/*
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
This software reads pigpio notification reports monitoring the I2C signals.

Notifications are pipe based so this software must be run on the Pi
being monitored.

It should be able to handle a 100kHz bus.  You are unlikely to get any
usable results if the bus is running at 400kHz.

gcc -o pig2i2c pig2i2c.c

Do something like

sudo pigpiod -s 2

# get a notification handle, assume handle 0 was returned

pigs no

# start notifications for SCL/SDA

e.g. pigs nb 0 0x3   # Rev. 1 select gpios 0/1
e.g. pigs nb 0 0xC   # Rev. 2 select gpios 2/3
e.g. pigs nb 0 0xA00 # select gpios 9/11 (1<<9|1<<11)

# run the program, specifying SCL/SDA and notification pipe

./pig2i2c SCL SDA </dev/pigpioN # specify gpios for SCL/SDA and pipe N

e.g. ./pig2i2c 1  0 </dev/pigpio0 # Rev.1 I2C gpios
e.g. ./pig2i2c 3  2 </dev/pigpio0 # Rev.2 I2C gpios
e.g. ./pig2i2c 9 11 </dev/pigpio0 # monitor external bus 
*/

#define RS (sizeof(gpioReport_t))

#define SCL_FALLING 0
#define SCL_RISING  1
#define SCL_STEADY  2

#define SDA_FALLING 0
#define SDA_RISING  4
#define SDA_STEADY  8

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

void parse_I2C(int SCL, int SDA)
{
   static int in_data=0, byte=0, bit=0;
   static int oldSCL=1, oldSDA=1;

   int xSCL, xSDA;

   if (SCL != oldSCL)
   {
      oldSCL = SCL;
      if (SCL) xSCL = SCL_RISING;
      else     xSCL = SCL_FALLING;
   }
   else        xSCL = SCL_STEADY;

   if (SDA != oldSDA)
   {
      oldSDA = SDA;
      if (SDA) xSDA = SDA_RISING;
      else     xSDA = SDA_FALLING;
   }
   else        xSDA = SDA_STEADY;

   switch (xSCL+xSDA)
   {
      case SCL_RISING + SDA_RISING:
      case SCL_RISING + SDA_FALLING:
      case SCL_RISING + SDA_STEADY:
         if (in_data)
         {
            if (bit++ < 8)
            {
               byte <<= 1;
               byte |= SDA;
            }
            else
            {
               printf("%02X", byte);
               if (SDA) printf("-"); else printf("+");
               bit = 0;
               byte = 0;
            }
         }
         break;

      case SCL_FALLING + SDA_RISING:
         break;

      case SCL_FALLING + SDA_FALLING:
         break;

      case SCL_FALLING + SDA_STEADY:
         break;

      case SCL_STEADY + SDA_RISING:
         if (SCL)
         {
            in_data = 0;
            byte = 0;
            bit = 0;

            printf("]\n"); // stop
            fflush(NULL);
         }
         break;

      case SCL_STEADY + SDA_FALLING:
         if (SCL)
         {
            in_data = 1;
            byte = 0;
            bit = 0;

            printf("["); // start
         }
         break;

      case SCL_STEADY + SDA_STEADY:
         break;

   }
}

int main(int argc, char * argv[])
{
   int gSCL, gSDA, SCL, SDA, xSCL;
   int r;
   uint32_t level, changed, bI2C, bSCL, bSDA;

   gpioReport_t report;

   if (argc > 2)
   {
      gSCL = atoi(argv[1]);
      gSDA = atoi(argv[2]);

      bSCL = 1<<gSCL;
      bSDA = 1<<gSDA;

      bI2C = bSCL | bSDA;
   }
   else
   {
      exit(-1);
   }

   /* default to SCL/SDA high */

   SCL = 1;
   SDA = 1;
   level = bI2C;

   while ((r=read(STDIN_FILENO, &report, RS)) == RS)
   {
      report.level &= bI2C;

      if (report.level != level)
      {
         changed = report.level ^ level;

         level = report.level;

         if (level & bSCL) SCL = 1; else SCL = 0;
         if (level & bSDA) SDA = 1; else SDA = 0;

         parse_I2C(SCL, SDA);
      }
   }
   return 0;
}

