/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

/*
This version is for pigpio version 11+
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include "pigpio.h"

/*
This program starts the pigpio library as a daemon.
*/

static unsigned bufferSizeMilliseconds = PI_DEFAULT_BUFFER_MILLIS;
static unsigned clockMicros            = PI_DEFAULT_CLK_MICROS;
static unsigned clockPeripheral        = PI_DEFAULT_CLK_PERIPHERAL;
static unsigned clockSource            = PI_DEFAULT_CLK_SOURCE;
static unsigned ifFlags                = PI_DEFAULT_IF_FLAGS;
static unsigned DMAprimaryChannel      = PI_DEFAULT_DMA_PRIMARY_CHANNEL;
static unsigned DMAsecondaryChannel    = PI_DEFAULT_DMA_SECONDARY_CHANNEL;
static unsigned socketPort             = PI_DEFAULT_SOCKET_PORT;
static uint64_t updateMask             = -1;

static FILE * errFifo;

void fatal(char *fmt, ...)
{
   char buf[128];
   va_list ap;

   va_start(ap, fmt);
   vsnprintf(buf, sizeof(buf), fmt, ap);
   va_end(ap);

   fprintf(stderr, "%s\n", buf);

   fflush(stderr);

   exit(EXIT_FAILURE);
}

void usage()
{
   fprintf(stderr, "\n" \
      "Usage: sudo pigpiod [OPTION] ...\n" \
      "   -b value, gpio sample buffer in milliseconds, default 120\n" \
      "   -d value, primary DMA channel, 0-14,          default 14\n" \
      "   -e value, secondary DMA channel, 0-6,         default 5\n" \
      "   -f,       disable fifo interface,             default enabled\n" \
      "   -k,       disable socket interface,           default enabled\n" \
      "   -p value, socket port, 1024-32000,            default 8888\n" \
      "   -s value, sample rate, 1, 2, 4, 5, 8, or 10,  default 5\n" \
      "   -t value, clock peripheral, 0=PWM 1=PCM,      default PCM\n" \
      "   -u value, clock source, 0=OSC 1=PLLD,         default PLLD\n" \
      "   -x mask,  gpios which may be updated,         default 0xFFFFFFFF\n" \
      "EXAMPLE\n" \
      "sudo pigpiod -s 2 -b 200 -f\n" \
      "  Set a sample rate of 2 microseconds with a 200 millisecond\n" \
      "  buffer.  Disable the fifo interface.\n" \
   "\n");
}

static void initOpts(int argc, char *argv[])
{
   int i, opt;
   uint64_t mask;
   char * endptr;

   while ((opt = getopt(argc, argv, "b:d:e:fkp:s:t:u:x:")) != -1)
   {
      i = -1;

      switch (opt)
      {
         case 'b':
            i = atoi(optarg);
            if ((i >= PI_BUF_MILLIS_MIN) && (i <= PI_BUF_MILLIS_MAX))
               bufferSizeMilliseconds = i;
            else fatal("invalid -b option (%d)", i);
            break;

         case 'd':
            i = atoi(optarg);
            if ((i >= PI_MIN_DMA_CHANNEL) && (i <= PI_MAX_PRIMARY_CHANNEL))
               DMAprimaryChannel = i;
            else fatal("invalid -d option (%d)", i);
            break;

         case 'e':
            i = atoi(optarg);
            if ((i >= PI_MIN_DMA_CHANNEL) && (i <= PI_MAX_SECONDARY_CHANNEL))
               DMAsecondaryChannel = i;
            else fatal("invalid -e option (%d)", i);
            break;

         case 'f':
            ifFlags |= PI_DISABLE_FIFO_IF;
            break; 

         case 'k':
            ifFlags |= PI_DISABLE_SOCK_IF;
            break; 

         case 'p':
            i = atoi(optarg);
            if ((i >= PI_MIN_SOCKET_PORT) && (i <= PI_MAX_SOCKET_PORT))
               socketPort = i;
            else fatal("invalid -p option (%d)", i);
            break;

         case 's':
            i = atoi(optarg);

            switch(i)
            {
               case 1:
               case 2:
               case 4:
               case 5:
               case 8:
               case 10:
                  clockMicros = i;
                  break;

               default:
                  fatal("invalid -s option (%d)", i);
                  break;
            }
            break;

         case 't':
            i = atoi(optarg);
            if ((i >= PI_CLOCK_PWM) && (i <= PI_CLOCK_PCM))
               clockPeripheral = i;
            else fatal("invalid -t option (%d)", i);
            break;

         case 'u':
            i = atoi(optarg);
            if ((i >= PI_CLOCK_OSC) && (i <= PI_CLOCK_PLLD))
               clockSource = i;
            else fatal("invalid -u option (%d)", i);
            break;

         case 'x':
            mask = strtoll(optarg, &endptr, 0);
            printf("mask=%llx\n", mask);
            if (!*endptr) updateMask = mask;
            else fatal("invalid -x option (%s)", optarg);
            break;

        default: /* '?' */
           usage();
           exit(-1);
        }
    }
}

void terminate(int signum)
{
   /* only registered for SIGHUP/SIGTERM */

   gpioTerminate();

   fprintf(errFifo, "SIGHUP/SIGTERM received\n");

   fflush(NULL);

   fclose(errFifo);

   unlink(PI_ERRFIFO);

   exit(0);
}


int main(int argc, char **argv)
{
   pid_t pid;
   int flags;

   /* Fork off the parent process */

   pid = fork();

   if (pid < 0) { exit(EXIT_FAILURE); }

   /* If we got a good PID, then we can exit the parent process. */

   if (pid > 0) { exit(EXIT_SUCCESS); }

   /* Change the file mode mask */

   umask(0);       
   
   /* Open any logs here */

   /* NONE */
   
   /* Create a new SID for the child process */

   if (setsid() < 0) fatal("setsid failed (%m)");

   /* Change the current working directory */

   if ((chdir("/")) < 0) fatal("chdir failed (%m)");
   
   /* check command line parameters */

   initOpts(argc, argv);
   
   /* Close out the standard file descriptors */

   fclose(stdin);
   fclose(stdout);

   /* configure library */

   gpioCfgBufferSize(bufferSizeMilliseconds);

   gpioCfgClock(clockMicros, clockPeripheral, clockSource);

   gpioCfgInterfaces(ifFlags);

   gpioCfgDMAchannels(DMAprimaryChannel, DMAsecondaryChannel);

   gpioCfgSocketPort(socketPort);

   if (updateMask != -1) gpioCfgPermissions(updateMask);

   /* start library */

   if (gpioInitialise()< 0) fatal("Can't initialise pigpio library");

   /* create pipe for error reporting */

   unlink(PI_ERRFIFO);

   mkfifo(PI_ERRFIFO, 0664);

   if (chmod(PI_ERRFIFO, 0664) < 0)
      fatal("chmod %s failed (%m)", PI_ERRFIFO);

   errFifo = freopen(PI_ERRFIFO, "w+", stderr);

   if (errFifo)
   {
      /* set stderr non-blocking */

      flags = fcntl(fileno(errFifo), F_GETFL, 0);
      fcntl(fileno(errFifo), F_SETFL, flags | O_NONBLOCK);

      /* request SIGHUP/SIGTERM from libarary for termination */

      gpioSetSignalFunc(SIGHUP, terminate);
      gpioSetSignalFunc(SIGTERM, terminate);

      /* sleep forever */

      while (1)
      {
         /* cat /dev/pigerr to view daemon errors */

         sleep(5);

         fflush(errFifo);
      }
   }
   else
   {
      fprintf(stderr, "freopen failed (%m)");

      gpioTerminate();
   }

   return 0;
}
