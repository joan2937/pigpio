#include <stdio.h>

#include <pigpio.h>

/*
   Measure how long a capacitor takes to charge through a resistance.

   A potentimeter is used to vary the resistance.

   The time taken will be proportional to the resistance.

   3V3 ----- Potentiometer --+-- Capacitor ----- Ground
                             |
                             +-- gpio


  gcc -o pot_cap_charge pot_cap_charge.c -lpigpio -lpthread -lrt
  sudo ./pot_cap_charge

*/

#define GPIO 25

#define MAX_READING 1000

static uint32_t rechargeTick = 0;

void callback(int gpio, int level, uint32_t tick)
{
   static uint32_t smooth = 0;
   static int reading = 0;

   uint32_t raw;

   if (level == 1) /* measure recharge time */
   {
      ++reading;

      if (rechargeTick)
      {
         raw = tick - rechargeTick; /* set in main */

         if (raw < MAX_READING) /* ignore outliers */
         {
            /* smooth using 0.8 * smooth + 0.2 * raw */

            smooth = (raw + (4 * smooth)) / 5;

            printf("%d %d %d\n", reading, raw, smooth);
         }
         else
         {
            /* ignore outlier, set dot at fixed position */
            printf("%d %d %d\n", reading, 40, smooth);
         }
      }
      else
      {
         /* ignore reschedule, set dot at fixed position */
         printf("%d %d %d\n", reading, 20, smooth);
      }
   }
}

int main (int argc, char *argv[])
{
   uint32_t t1, t2;
   int tDiff;

   if (gpioInitialise()<0) return 1;

   gpioSetAlertFunc(GPIO, callback); /* callback when GPIO changes state */
    
   while (1)
   {
      gpioWrite(GPIO, PI_OFF); /* drain capacitor */

      gpioDelay(200); /* microseconds */

      t1 = gpioTick();

      gpioSetMode(GPIO, PI_INPUT); /* start capacitor recharge */

      t2 = gpioTick();

      /* dump reading if rechargeTick not accurate to 3 micros */

      if ((t2 - t1) < 3) rechargeTick = t1; else rechargeTick = 0;

      gpioDelay(5000); /* microseconds, nominal 200 readings per second */
   }

   gpioTerminate();
}
