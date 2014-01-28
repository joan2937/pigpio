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

/* PIGPIOD_IF_VERSION 3 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <arpa/inet.h>

#include "pigpio.h"
#include "command.h"

#include "pigpiod_if.h"

#define PISCOPE_MAX_REPORTS_PER_READ 4096

#define STACK_SIZE (256*1024)

typedef void (*CBF_t) ();

struct callback_s
{

   int id;
   int gpio;
   int edge;
   CBF_t f;
   void * user;
   int ex;
   callback_t *prev;
   callback_t *next;
};

/* GLOBALS ---------------------------------------------------------------- */

static gpioReport_t gReport[PISCOPE_MAX_REPORTS_PER_READ];

static int gPigCommand = -1;
static int gPigHandle = -1;
static int gPigNotify = -1;

static uint32_t gNotifyBits;

callback_t *gCallBackFirst = 0;
callback_t *gCallBackLast = 0;

static int gPigStarted = 0;

static pthread_t *pthNotify;

/* PRIVATE ---------------------------------------------------------------- */

static int pigpio_command(int fd, int command, int p1, int p2)
{
   cmdCmd_t cmd;

   cmd.cmd = command;
   cmd.p1  = p1;
   cmd.p2  = p2;
   cmd.res = 0;

   if (send(fd, &cmd, sizeof(cmd), 0) != sizeof(cmd)) return pigif_bad_send;

   if (recv(fd, &cmd, sizeof(cmd), MSG_WAITALL) != sizeof(cmd))
      return pigif_bad_recv;

   return cmd.res;
}

static int pigpio_command_ext
   (int fd, int command, int p1, int p2, int extents, gpioExtent_t *ext)
{
   int i;
   cmdCmd_t cmd;

   cmd.cmd = command;
   cmd.p1  = p1;
   cmd.p2  = p2;
   cmd.res = 0;

   if (send(fd, &cmd, sizeof(cmd), 0) != sizeof(cmd)) return pigif_bad_send;

   for (i=0; i<extents; i++)
   {
      if (send(fd, ext[i].ptr, ext[i].size, 0) != ext[i].size)
         return pigif_bad_send;
   }

   if (recv(fd, &cmd, sizeof(cmd), MSG_WAITALL) != sizeof(cmd))
      return pigif_bad_recv;

   return cmd.res;
}

static int pigpioOpenSocket(char *addr, char *port)
{
   int sock, err;
   struct addrinfo hints, *res, *rp;
   const char *addrStr, *portStr;

   if (!addr)
   {
      addrStr = getenv(PI_ENVADDR);

      if ((!addrStr) || (!strlen(addrStr)))
      {
         addrStr = PI_DEFAULT_SOCKET_ADDR_STR;
      }
   }
   else addrStr = addr;

   if (!port)
   {
      portStr = getenv(PI_ENVPORT);

      if ((!portStr) || (!strlen(portStr)))
      {
         portStr = PI_DEFAULT_SOCKET_PORT_STR;
      }
   }
   else portStr = port;

   memset (&hints, 0, sizeof (hints));

   hints.ai_family   = PF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags   |= AI_CANONNAME;

   err = getaddrinfo (addrStr, portStr, &hints, &res);

   if (err) return pigif_bad_getaddrinfo;

   for (rp=res; rp!=NULL; rp=rp->ai_next)
   {
      sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

      if (sock == -1) continue;

      if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1) break;
   }

   freeaddrinfo(res);

   if (rp == NULL) return pigif_bad_connect;

   return sock;
}

static void dispatch_notification(gpioReport_t *r)
{
   static uint32_t lastLevel = 0;

   callback_t *p;
   uint32_t changed;
   int l, g;

   /*
   printf("s=%d f=%d l=%8X, t=%10u\n",
      r->seqno, r->flags, r->level, r->tick);
   */

   if (r->flags == 0)
   {
      changed = (r->level ^ lastLevel) & gNotifyBits;

      lastLevel = r->level;

      p = gCallBackFirst;

      while (p)
      {
         if (changed & (1<<(p->gpio)))
         {
            if ((r->level) & (1<<(p->gpio))) l = 1; else l = 0;
            if ((p->edge) ^ l)
            {
               if (p->ex) (p->f)(p->gpio, l, r->tick, p->user);
               else       (p->f)(p->gpio, l, r->tick);
            }
         }
         p = p->next;
      }
   }
   else
   {
      g = (r->flags) & 31;

      p = gCallBackFirst;

      while (p)
      {
         if ((p->gpio) == g)
         {
            if (p->ex) (p->f)(g, PI_TIMEOUT, r->tick, p->user);
            else       (p->f)(g, PI_TIMEOUT, r->tick);
         }
         p = p->next;
      }
   }
}

static void *pthNotifyThread(void *x)
{
   static int got = 0;

   int bytes, r;

   while (1)
   {
      bytes = read(gPigNotify, (char*)&gReport+got, sizeof(gReport)-got);

      if (bytes > 0) got += bytes;
      else break;

      r = 0;

      while (got >= sizeof(gpioReport_t))
      {
         dispatch_notification(&gReport[r]);

         r++;

         got -= sizeof(gpioReport_t);
      }

      /* copy any partial report to start of array */
      
      if (got && r) gReport[0] = gReport[r];
   }
   return 0;
}

static void findNotifyBits(void)
{
   callback_t *p;
   uint32_t bits = 0;

   p = gCallBackFirst;

   while (p)
   {
      bits |= (1<<(p->gpio));
      p = p->next;
   }

   if (bits != gNotifyBits)
   {
      gNotifyBits = bits;
      pigpio_command(gPigCommand, PI_CMD_NB, gPigHandle, gNotifyBits);
   }
}

static void _wfe(unsigned gpio, unsigned level, uint32_t tick, void *user)
{
   *(int *)user = 1;
}

static int intCallback(unsigned gpio, unsigned edge, void *f, void *user, int ex)
{
   static int id = 0;
   callback_t *p;

   if ((gpio >=0) && (gpio < 32) && (edge >=0) && (edge <= 2) && f)
   {
      /* prevent duplicates */

      p = gCallBackFirst;

      while (p)
      {
         if ((p->gpio == gpio) && (p->edge == edge) && (p->f == f))
         {
            return pigif_duplicate_callback;
         }
         p = p->next;
      }

      p = malloc(sizeof(callback_t));

      if (p)
      {
         if (!gCallBackFirst) gCallBackFirst = p;

         p->id = id++;
         p->gpio = gpio;
         p->edge = edge;
         p->f = f;
         p->user = user;
         p->ex = ex;
         p->next = 0;
         p->prev = gCallBackLast;

         if (p->prev) (p->prev)->next = p;
         gCallBackLast = p;

         findNotifyBits();

         return p->id;
      }

      return pigif_bad_malloc;
   }

   return pigif_bad_callback;
}

/* PUBLIC ----------------------------------------------------------------- */

double time_time(void)
{
   struct timeval tv;
   double t;

   gettimeofday(&tv, 0);

   t = (double)tv.tv_sec + ((double)tv.tv_usec / 1E6);

   return t;
}

void time_sleep(double seconds)
{
   struct timespec ts, rem;

   if (seconds > 0.0)
   {
      ts.tv_sec = seconds;
      ts.tv_nsec = (seconds-(double)ts.tv_sec) * 1E9;

      while (clock_nanosleep(CLOCK_REALTIME, 0, &ts, &rem))
      {
         /* copy remaining time to ts */
         ts.tv_sec  = rem.tv_sec;
         ts.tv_nsec = rem.tv_nsec;
      }
   }
}

const char *pigpio_error(int error)
{
   if (error > -1000) return cmdErrStr(error);
   else
   {
      switch(error)
      {
         case pigif_bad_send:
            return "failed to send to pigpiod";
         case pigif_bad_recv:
            return "failed to receive from pigpiod";
         case pigif_bad_getaddrinfo:
            return "failed to find address of pigpiod";
         case pigif_bad_connect:
            return "failed to connect to pigpiod";
         case pigif_bad_socket:
            return "failed to create socket";
         case pigif_bad_noib:
            return "failed to open noib";
         case pigif_duplicate_callback:
            return "identical callback exists";
         case pigif_bad_malloc:
            return "failed to malloc";
         case pigif_bad_callback:
            return "bad callback parameter";
         case pigif_notify_failed:
            return "failed to create notification thread";
         case pigif_callback_not_found:
            return "callback not found";
         default:
            return "unknown error";
      }
   }
}

unsigned pigpiod_if_version(void)
{
   return PIGPIOD_IF_VERSION;
}

pthread_t *start_thread(gpioThreadFunc_t func, void *arg)
{
   pthread_t *pth;
   pthread_attr_t pthAttr;

   pth = malloc(sizeof(pthread_t));

   if (pth)
   {
      if (pthread_attr_init(&pthAttr))
      {
         perror("pthread_attr_init failed");
         free(pth);
         return NULL;
      }

      if (pthread_attr_setstacksize(&pthAttr, STACK_SIZE))
      {
         perror("pthread_attr_setstacksize failed");
         free(pth);
         return NULL;
      }

      if (pthread_create(pth, &pthAttr, func, arg))
      {
         perror("pthread_create socket failed");
         free(pth);
         return NULL;
      }
   }
   return pth;
}

void stop_thread(pthread_t *pth)
{
   if (pth)
   {
      pthread_cancel(*pth);
      pthread_join(*pth, NULL);
   }
}

int pigpio_start(char *addrStr, char *portStr)
{
   if (!gPigStarted)
   {
      gPigCommand = pigpioOpenSocket(addrStr, portStr);

      if (gPigCommand >= 0)
      {
         gPigNotify = pigpioOpenSocket(addrStr, portStr);

         if (gPigNotify >= 0)
         {
            gPigHandle = pigpio_command(gPigNotify, PI_CMD_NOIB, 0, 0);

            if (gPigHandle < 0) return pigif_bad_noib;
            else
            {
               pthNotify = start_thread(pthNotifyThread, 0);
               if (pthNotify)
               {
                  gPigStarted = 1;
                  return 0;
               }
               else return pigif_notify_failed;
            }
         }
         else return gPigNotify;
      }
      else return gPigCommand;
   }
   return 0;
}

void pigpio_stop(void)
{
   gPigStarted = 0;

   if (pthNotify)
   {
      stop_thread(pthNotify);
      pthNotify = 0;
   }

   if (gPigNotify >= 0)
   {
      if (gPigHandle >= 0)
      {
         pigpio_command(gPigNotify, PI_CMD_NC, gPigHandle, 0);
         gPigHandle = -1;
      }

      close(gPigNotify);
      gPigNotify = -1;
   }

   if (gPigCommand >= 0)
   {
      if (gPigHandle >= 0)
      {
         pigpio_command(gPigCommand, PI_CMD_NC, gPigHandle, 0);
         gPigHandle = -1;
      }

      close(gPigCommand);
      gPigCommand = -1;
   }
}

int set_mode(unsigned gpio, unsigned mode)
   {return pigpio_command(gPigCommand, PI_CMD_MODES, gpio, mode);}

int get_mode(unsigned gpio)
   {return pigpio_command(gPigCommand, PI_CMD_MODEG, gpio, 0);}

int set_pull_up_down(unsigned gpio, unsigned pud)
   {return pigpio_command(gPigCommand, PI_CMD_PUD, gpio, pud);}

int read_gpio(unsigned gpio)
   {return pigpio_command(gPigCommand, PI_CMD_READ, gpio, 0);}

int write_gpio(unsigned gpio, unsigned level)
   {return pigpio_command(gPigCommand, PI_CMD_WRITE, gpio, level);}

int set_PWM_dutycycle(unsigned user_gpio, unsigned dutycycle)
   {return pigpio_command(gPigCommand, PI_CMD_PWM, user_gpio, dutycycle);}

int set_PWM_range(unsigned user_gpio, unsigned range_)
   {return pigpio_command(gPigCommand, PI_CMD_PRS, user_gpio, range_);}

int get_PWM_range(unsigned user_gpio)
   {return pigpio_command(gPigCommand, PI_CMD_PRG, user_gpio, 0);}

int get_PWM_real_range(unsigned user_gpio)
   {return pigpio_command(gPigCommand, PI_CMD_PRRG, user_gpio, 0);}

int set_PWM_frequency(unsigned user_gpio, unsigned frequency)
   {return pigpio_command(gPigCommand, PI_CMD_PFS, user_gpio, frequency);}

int get_PWM_frequency(unsigned user_gpio)
   {return pigpio_command(gPigCommand, PI_CMD_PFG, user_gpio, 0);}

int set_servo_pulsewidth(unsigned user_gpio, unsigned pulsewidth)
   {return pigpio_command(gPigCommand, PI_CMD_SERVO, user_gpio, pulsewidth);}

int notify_open(void)
   {return pigpio_command(gPigCommand, PI_CMD_NO, 0, 0);}

int notify_begin(unsigned handle, uint32_t bits)
   {return pigpio_command(gPigCommand, PI_CMD_NB, handle, bits);}

int notify_pause(unsigned handle)
   {return pigpio_command(gPigCommand, PI_CMD_NB, handle, 0);}

int notify_close(unsigned handle)
   {return pigpio_command(gPigCommand, PI_CMD_NC, handle, 0);}

int set_watchdog(unsigned user_gpio, unsigned timeout)
   {return pigpio_command(gPigCommand, PI_CMD_WDOG, user_gpio, timeout);}

uint32_t read_bank_1(void)
   {return pigpio_command(gPigCommand, PI_CMD_BR1, 0, 0);}

uint32_t read_bank_2(void)
   {return pigpio_command(gPigCommand, PI_CMD_BR2, 0, 0);}

int clear_bank_1(uint32_t levels)
   {return pigpio_command(gPigCommand, PI_CMD_BC1, levels, 0);}

int clear_bank_2(uint32_t levels)
   {return pigpio_command(gPigCommand, PI_CMD_BC2, levels, 0);}

int set_bank_1(uint32_t levels)
   {return pigpio_command(gPigCommand, PI_CMD_BS1, levels, 0);}

int set_bank_2(uint32_t levels)
   {return pigpio_command(gPigCommand, PI_CMD_BS2, levels, 0);}

uint32_t get_current_tick(void)
   {return pigpio_command(gPigCommand, PI_CMD_TICK, 0, 0);}

uint32_t get_hardware_revision(void)
   {return pigpio_command(gPigCommand, PI_CMD_HWVER, 0, 0);}

unsigned get_pigpio_version(void)
   {return pigpio_command(gPigCommand, PI_CMD_PIGPV, 0, 0);}

int wave_clear(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVCLR, 0, 0);}

int wave_add_generic(unsigned numPulses, gpioPulse_t *pulses)
{
   gpioExtent_t ext[1];
   
   /*
   p1=numPulses
   p2=0
   ## extension ##
   gpioPulse_t[] pulses
   */

   ext[0].size = numPulses * sizeof(gpioPulse_t);
   ext[0].ptr = pulses;

   return pigpio_command_ext(gPigCommand, PI_CMD_WVAG, numPulses, 0, 1, ext);
}

int wave_add_serial(
   unsigned gpio, unsigned baud, unsigned offset, unsigned numChar, char *str)
{
   gpioExtent_t ext[3];

   /*
   p1=gpio
   p2=numChar
   ## extension ##
   unsigned baud
   unsigned offset
   char[] str
   */

   ext[0].size = sizeof(unsigned);
   ext[0].ptr = &baud;

   ext[1].size = sizeof(unsigned);
   ext[1].ptr = &offset;

   ext[2].size = numChar;
   ext[2].ptr = str;

   return pigpio_command_ext(gPigCommand, PI_CMD_WVAS, gpio, numChar, 3, ext);
}

int wave_tx_busy(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVBSY, 0, 0);}

int wave_tx_stop(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVHLT, 0, 0);}

int wave_tx_start(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVGO, 0, 0);}

int wave_tx_repeat(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVGOR, 0, 0);}

int wave_get_micros(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSM, 0, 0);}

int wave_get_high_micros(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSM, 1, 0);}

int wave_get_max_micros(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSM, 2, 0);}

int wave_get_pulses(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSP, 0, 0);}

int wave_get_high_pulses(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSP, 1, 0);}

int wave_get_max_pulses(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSP, 2, 0);}

int wave_get_cbs(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSC, 0, 0);}

int wave_get_high_cbs(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSC, 1, 0);}

int wave_get_max_cbs(void)
   {return pigpio_command(gPigCommand, PI_CMD_WVSC, 2, 0);}

int gpio_trigger(unsigned gpio, unsigned pulseLen, unsigned level)
{
   gpioExtent_t ext[1];
   
   /*
   p1=gpio
   p2=pulseLen
   ## extension ##
   unsigned level
   */

   ext[0].size = sizeof(level);
   ext[0].ptr = &level;

   return pigpio_command_ext(gPigCommand, PI_CMD_TRIG, gpio, pulseLen, 1, ext);
}

int store_script(char *script)
{
   unsigned len;
   gpioExtent_t ext[1];

   /*
   p1=script length
   p2=0
   ## extension ##
   char[] script
   */

   len = strlen(script);

   ext[0].size = len;
   ext[0].ptr = script;

   return pigpio_command_ext(gPigCommand, PI_CMD_PROC, len, 0, 1, ext);
}

int run_script(unsigned script_id)
   {return pigpio_command(gPigCommand, PI_CMD_PROCR, script_id, 0);}

int stop_script(unsigned script_id)
   {return pigpio_command(gPigCommand, PI_CMD_PROCS, script_id, 0);}

int delete_script(unsigned script_id)
   {return pigpio_command(gPigCommand, PI_CMD_PROCD, script_id, 0);}

int serial_read_open(unsigned gpio, unsigned baud)
   {return pigpio_command(gPigCommand, PI_CMD_SLRO, gpio, baud);}

int serial_read(unsigned gpio, void *buf, size_t bufSize)
{
   int bytes;

   bytes = pigpio_command(gPigCommand, PI_CMD_SLR, gpio, bufSize);

   if (bytes > 0)
   {
      /* get the data */
      recv(gPigCommand, buf, bytes, MSG_WAITALL);
   }
   return bytes;
}

int serial_read_close(unsigned gpio)
   {return pigpio_command(gPigCommand, PI_CMD_SLRC, gpio, 0);}

int callback(unsigned gpio, unsigned edge, CBFunc_t f)
   {return intCallback(gpio, edge, f, 0, 0);}

int callback_ex(unsigned gpio, unsigned edge, CBFuncEx_t f, void *user)
   {return intCallback(gpio, edge, f, user, 1);}

int callback_cancel(unsigned id)
{
   callback_t *p;

   p = gCallBackFirst;

   while (p)
   {
      if (p->id == id)
      {
         if (p->prev) p->prev->next = p->next;
         else gCallBackFirst = p->next;

         if (p->next) p->next->prev = p->prev;
         else gCallBackLast = p->prev;

         free(p);

         findNotifyBits();

         return 0;
      }
      p = p->next;
   }
   return pigif_callback_not_found;
}

int wait_for_edge(unsigned gpio, unsigned edge, double timeout)
{
   int triggered = 0;
   int id;
   double due;

   if (timeout <= 0.0) return 0;

   due = time_time() + timeout;

   id = callback_ex(gpio, edge, _wfe, &triggered);

   while (!triggered && (time_time() < due)) time_sleep(0.1);

   callback_cancel(id);

   return triggered;
}

