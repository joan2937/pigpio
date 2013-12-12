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
This version is for pigpio version 3+
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "pigpio.h"
#include "command.h"

/*
This program provides a socket interface
to the commands available from pigpio.
*/

int main(int argc , char *argv[])
{
   int sock, r, idx, port;
   struct sockaddr_in server;
   cmdCmd_t cmd;
   char * portStr, * addrStr;
   char buf[128];
 
   sock = socket(AF_INET, SOCK_STREAM, 0);
 
   if (sock != -1)
   {
      portStr = getenv(PI_ENVPORT);

      if (portStr) port = atoi(portStr);
      else         port = PI_DEFAULT_SOCKET_PORT;

      addrStr = getenv(PI_ENVADDR);

      if (!addrStr) addrStr="127.0.0.1";

      server.sin_addr.s_addr = inet_addr(addrStr);
      server.sin_family = AF_INET;
      server.sin_port = htons(port);

      if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == 0)
      {
         switch(argc)
         {
            case 1:
               exit(0);

            case 2:
               sprintf(buf, "%10s", argv[1]);
               break;

            case 3:
               sprintf(buf, "%10s %10s", argv[1], argv[2]);
               break;

            case 4:
               sprintf(buf, "%10s %10s %10s", argv[1], argv[2], argv[3]);
               break;

            default:
               cmdFatal("what?");
         }

         if ((idx=cmdParse(buf, &cmd)) >= 0)
         {
            if (send(sock, &cmd, sizeof(cmdCmd_t), 0) == sizeof(cmdCmd_t))
            {
               if (recv(sock, &cmd, sizeof(cmdCmd_t), 0) == sizeof(cmdCmd_t))
               {
                  switch (cmdInfo[idx].rv)
                  {
                     case 0:
                        r = cmd.res;
                        if (r < 0) cmdFatal("ERROR: %s", cmdErrStr(r));
                        break;

                     case 1:
                        break;

                     case 2:
                        r = cmd.res;
                        if (r < 0) cmdFatal("ERROR: %s", cmdErrStr(r));
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
                  }
               }
               else cmdFatal("recv failed, %m");
            }
            else cmdFatal("send failed, %m");
         }
         else cmdFatal("what?");
      }
      else cmdFatal("connect failed, %m");

      close(sock);
   }
   else cmdFatal("socket failed, %m");

   return 0;
}

