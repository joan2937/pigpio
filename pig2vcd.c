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
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

#include "pigpio.h"

/*
This software converts pigpio notification reports
into a VCD format understood by GTKWave.
*/

#define RS (sizeof(gpioReport_t))

static char * timeStamp()
{
   static char buf[32];

   struct timeval now;
   struct tm tmp;

   gettimeofday(&now, NULL);

   localtime_r(&now.tv_sec, &tmp);
   strftime(buf, sizeof(buf), "%F %T", &tmp);

   return buf;
}

int symbol(int bit)
{
   if (bit < 26) return ('A' + bit);
   else          return ('a' + bit - 26);
}

int main(int argc, char * argv[])
{
   int b, r, v;
   uint32_t t0;
   uint32_t lastLevel, changed;

   gpioReport_t report;

   r=read(STDIN_FILENO, &report, RS);

   if (r != RS) exit(-1);

   printf("$date %s $end\n", timeStamp());
   printf("$version pig2vcd V1 $end\n");
   printf("$timescale 1 us $end\n");
   printf("$scope module top $end\n");

   for (b=0; b<32; b++)
      printf("$var wire 1 %c %d $end\n", symbol(b), b);
        
   printf("$upscope $end\n");
   printf("$enddefinitions $end\n");
         
   t0 = report.tick;
   lastLevel =0;

   while ((r=read(STDIN_FILENO, &report, RS)) == RS)
   {
      if (report.level != lastLevel)
      {
         printf("#%u\n", report.tick - t0);

         changed = report.level ^ lastLevel;

         lastLevel = report.level;

         for (b=0; b<32; b++)
         {
            if (changed & (1<<b))
            {
               if (report.level & (1<<b)) v='1'; else v='0';

               printf("%c%c\n", v, symbol(b));
            }
         }
      }
   }
   return 0;
}

