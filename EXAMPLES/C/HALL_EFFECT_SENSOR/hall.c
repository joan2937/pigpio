#include <stdio.h>

#include <pigpio.h>

/*
OH3144E or equivalent Hall effect sensor

Pin 1 - 5V
Pin 2 - Ground
Pin 3 - gpio (here P1-8, gpio 14, TXD is used)

The internal gpio pull-up is enabled so that the sensor
normally reads high.  It reads low when a magnet is close.

gcc -o hall hall.c -lpigpio -lrt -lpthread
sudo ./hall
*/

#define HALL 14

void alert(int gpio, int level, uint32_t tick)
{
   static uint32_t lastTick=0;

   if (lastTick) printf("%d %.2f\n", level, (float)(tick-lastTick)/1000000.0);
   else          printf("%d 0.00\n", level);

   lastTick = tick;
}

int main(int argc, char *argv[])
{
   int secs=60;

   if (argc>1) secs = atoi(argv[1]); /* program run seconds */

   if ((secs<1) || (secs>3600)) secs = 3600;

   if (gpioInitialise()<0) return 1;

   gpioSetMode(HALL, PI_INPUT);

   gpioSetPullUpDown(HALL, PI_PUD_UP);

   gpioSetAlertFunc(HALL, alert);

   sleep(secs);

   gpioTerminate();
}

