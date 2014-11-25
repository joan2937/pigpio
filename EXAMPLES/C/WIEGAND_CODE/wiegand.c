#include <stdlib.h>

#include <pigpio.h>

#include "wiegand.h"

struct _Pi_Wieg_s
{
   int mygpio_0;
   int mygpio_1;
   int mytimeout;
   int in_code;
   int bits;
   Pi_Wieg_CB_t mycallback;
   uint32_t num;
   uint32_t code_timeout;
};

void _cb(int gpio, int level, uint32_t tick, void *user)
{
   /*
      Accumulate bits until both gpios 0 and 1 timeout.
   */

   Pi_Wieg_t *wieg;

   wieg = user;

   if (level == 0) /* a falling edge indicates a new bit */
   {
      if (!wieg->in_code)
      {
         wieg->bits = 1;
         wieg->num = 0;

         wieg->in_code = 1;
         wieg->code_timeout = 0;

         gpioSetWatchdog(wieg->mygpio_0, wieg->mytimeout);
         gpioSetWatchdog(wieg->mygpio_1, wieg->mytimeout);
      }
      else
      {
         wieg->bits++;
         wieg->num <<= 1;
      }

      if (gpio == wieg->mygpio_0)
      {
         wieg->code_timeout &= 2; /* clear gpio 0 timeout */
      }
      else
      {
         wieg->code_timeout &= 1; /* clear gpio 1 timeout */
         wieg->num |= 1;
      }
   }
   else if (level == PI_TIMEOUT)
   {
      if (wieg->in_code)
      {
         if (gpio == wieg->mygpio_0)
         {
            wieg->code_timeout |= 1; /* timeout gpio 0 */
         }
         else
         {
            wieg->code_timeout |= 2; /* timeout gpio 1 */
         }

         if (wieg->code_timeout == 3) /* both gpios timed out */
         {
            gpioSetWatchdog(wieg->mygpio_0, 0);
            gpioSetWatchdog(wieg->mygpio_1, 0);

            wieg->in_code = 0;

            (wieg->mycallback)(wieg->bits, wieg->num);
         }
      }
   }
}

Pi_Wieg_t * Pi_Wieg(
   int gpio_0,
   int gpio_1,
   Pi_Wieg_CB_t callback,
   int timeout)
{
   /*
      Instantiate with the gpio for 0 (green wire), the gpio for 1
      (white wire), the callback function, and the timeout in
      milliseconds which indicates the end of a code.

      The callback is passed the code length in bits and the value.
   */

   Pi_Wieg_t *wieg;

   wieg = malloc(sizeof(Pi_Wieg_t));

   wieg->mygpio_0 = gpio_0;
   wieg->mygpio_1 = gpio_1;

   wieg->mycallback = callback;

   wieg->mytimeout = timeout;

   wieg->in_code = 0;

   gpioSetMode(gpio_0, PI_INPUT);
   gpioSetMode(gpio_1, PI_INPUT);

   gpioSetPullUpDown(gpio_0, PI_PUD_UP);
   gpioSetPullUpDown(gpio_1, PI_PUD_UP);

   gpioSetAlertFuncEx(gpio_0, _cb, wieg);
   gpioSetAlertFuncEx(gpio_1, _cb, wieg);

   return wieg;
}

void Pi_Wieg_cancel(Pi_Wieg_t *wieg)
{
   /*
      Cancel the Wiegand decoder.
   */

   if (wieg)
   {
      gpioSetAlertFunc(wieg->mygpio_0, 0);
      gpioSetAlertFunc(wieg->mygpio_1, 0);

      free(wieg);
   }
}

