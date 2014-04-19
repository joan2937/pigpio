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
This version is for pigpio version 14+
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "pigpio.h"
#include "command.h"

/*
This program provides a socket interface to some of
the commands available from pigpio.
*/

char command_buf[8192];
char response_buf[8192];

#define SOCKET_OPEN_FAILED -1

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

static int openSocket(void)
{
   int sock, err;
   struct addrinfo hints, *res, *rp;
   const char *addrStr, *portStr;

   portStr = getenv(PI_ENVPORT);

   if (!portStr) portStr = PI_DEFAULT_SOCKET_PORT_STR;

   addrStr = getenv(PI_ENVADDR);

   if (!addrStr) addrStr = PI_DEFAULT_SOCKET_ADDR_STR;

   memset (&hints, 0, sizeof (hints));

   hints.ai_family   = PF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags   |= AI_CANONNAME;

   err = getaddrinfo(addrStr, portStr, &hints, &res);

   if (err) return SOCKET_OPEN_FAILED;

   for (rp=res; rp!=NULL; rp=rp->ai_next)
   {
      sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

      if (sock == -1) continue;

      if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1) break;
   }

   freeaddrinfo(res);

   if (rp == NULL) return SOCKET_OPEN_FAILED;

   return sock;
}

void print_result(int sock, int rv, cmdCmd_t cmd)
{
   int i, r;
   uint32_t *p;

   r = cmd.res;

   switch (rv)
   {
      case 0:
         if (r < 0) fatal("ERROR: %s", cmdErrStr(r));
         break;

      case 1:
         if (r < 0) fatal("ERROR: %s", cmdErrStr(r));
         break;

      case 2:
         if (r < 0) fatal("ERROR: %s", cmdErrStr(r));
         else printf("%d\n", r);
         break;

      case 3:
         printf("%08X\n", cmd.res);
         break;

      case 4:
         printf("%u\n", cmd.res);
         break;

      case 5:
         printf(cmdUsage);
         break;

      case 6: /* SLR */
         if (r < 0) fatal("ERROR: %s", cmdErrStr(r));
         else if (r > 0)
         {
            printf("%s", response_buf);
         }
         break;

      case 7: /* PROCP */
         if (r < 0) fatal("ERROR: %s", cmdErrStr(r));
         else
         {
            printf("%d", r);

            p = (uint32_t *)response_buf;

            for (i=0; i<PI_MAX_SCRIPT_PARAMS; i++)
            {
               printf(" %d", p[i]);
            }

            printf("\n");
         }
         break;
   }
}

void get_extensions(int sock, int command, int res)
{
   switch (command)
   {
      case PI_CMD_PROCP: /* PROCP */
         if (res >= 0)
         {
            recv(sock,
                 response_buf,
                 sizeof(uint32_t)*PI_MAX_SCRIPT_PARAMS,
                 MSG_WAITALL);
         }
         break;

      case PI_CMD_SLR: /* SLR */
         if (res > 0)
         {
            recv(sock, response_buf, res, MSG_WAITALL);
            response_buf[res] = 0;
         }
         break;
   }
}

void put_extensions(int sock, int command, uint32_t *p, void *v[])
{
   switch (command)
   {
      case PI_CMD_PROC:
         /*
         p1=script length w[1]
         p2=0
         ## extension ##
         char[] script    v[1]
         */
         send(sock, v[1], p[1], 0);
         break;

      case PI_CMD_PROCR:
         /*
         p1=script id     w[1]
         p2=numParam      w[2]
         ## extension ##
         int[] param      v[1]
         */
         if (p[2]) send(sock, v[1], p[2]*sizeof(uint32_t), 0);
         break;

      case PI_CMD_TRIG:
         /*
         p1=user_gpio      w[1]
         p2=pulseLen       w[2]
         ## extension ##
         unsigned level    w[3]
         */
         send(sock, &p[3], 4, 0);
         break;

      case PI_CMD_WVAG:
         /*
         p1=pulses        w[1]
         p2=0
         ## extension ##
         int[] param      v[1]
         */
         if (p[1]) send(sock, v[1], p[1]*sizeof(gpioPulse_t), 0);
         break;

      case PI_CMD_WVAS:
         /*
         p1=user_gpio       w[1]
         p2=numChar         w[4]
         ## extension ##
         unsigned baud      w[2]
         unsigned offset    w[3]
         char[] str         v[1]
         */
         send(sock, &p[2], 4, 0);
         send(sock, &p[3], 4, 0);
         send(sock, v[1], p[4], 0);
         break;
   }

}

int main(int argc , char *argv[])
{
   int sock, command;
   int idx, i, pp, l, len;
   cmdCmd_t cmd;
   uint32_t p[10];
   void *v[10];
   cmdCtlParse_t ctl;
   cmdScript_t s;

   sock = openSocket();

   command_buf[0] = 0;
   l = 0;
   pp = 0;

   for (i=1; i<argc; i++)
   {
      l += (strlen(argv[i]) + 1);
      if (l < sizeof(command_buf))
         {sprintf(command_buf+pp, "%s ", argv[i]); pp=l;}
   }

   if (pp) {command_buf[--pp] = 0;}

   ctl.eaten = 0;

   len = strlen(command_buf);
   idx = 0;

   while ((idx >= 0) && (ctl.eaten < len))
   {
      if ((idx=cmdParse(command_buf, p, v, &ctl)) >= 0)
      {
         command = p[0];

         if (command < PI_CMD_SCRIPT)
         {
            if (command == PI_CMD_HELP)
            {
               printf(cmdUsage);
            }
            else if (command == PI_CMD_PARSE)
            {
               cmdParseScript(v[1], &s, 1);
               if (s.par) free (s.par);
            }
            else
            {
               cmd.cmd = command;
               cmd.p1 = p[1];
               cmd.p2 = p[2];

               switch (command)
               {
                  case PI_CMD_WVAS:
                     cmd.p2 = p[4];
                     break;

                  case PI_CMD_PROC:
                     cmd.p2 = 0;
                     break;
               }

               if (sock != SOCKET_OPEN_FAILED)
               {
                  if (send(sock, &cmd, sizeof(cmdCmd_t), 0) ==
                     sizeof(cmdCmd_t))
                  {
                     put_extensions(sock, command, p, v);

                     if (recv(sock, &cmd, sizeof(cmdCmd_t), MSG_WAITALL) ==
                        sizeof(cmdCmd_t))
                     {
                        get_extensions(sock, command, cmd.res);

                        print_result(sock, cmdInfo[idx].rv, cmd);
                     }
                     else fatal("recv failed, %m");
                  }
                  else fatal("send failed, %m");
               }
               else fatal("connect failed");
            }
         }
         else fatal("%s only allowed within a script", cmdInfo[idx].name);
      }
      else
      {
         if (idx == CMD_UNKNOWN_CMD)
            fatal("%s? unknown command, pigs h for help", cmdStr());
         else
            fatal("%s: bad parameter, pigs h for help", cmdStr());
      }
   }

   if (sock >= 0) close(sock);

   return 0;
}

