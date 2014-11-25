#include <pigpio.h>

#include "wiegand.hpp"

Wiegand::Wiegand(int gpio_0, int gpio_1, WiegandCB_t callback, int timeout)
{
   /*
      Instantiate with the gpio for 0 (green wire), the gpio for 1
      (white wire), the callback function, and the bit timeout in
      milliseconds which indicates the end of a code.

      The callback is passed the code length in bits and the value.
   */

   mygpio_0 = gpio_0;
   mygpio_1 = gpio_1;

   mycallback = callback;

   mytimeout = timeout;

   in_code = 0;

   gpioSetMode(gpio_0, PI_INPUT);
   gpioSetMode(gpio_1, PI_INPUT);

   gpioSetPullUpDown(gpio_0, PI_PUD_UP);
   gpioSetPullUpDown(gpio_1, PI_PUD_UP);

   gpioSetAlertFuncEx(gpio_0, _cbEx, this);
   gpioSetAlertFuncEx(gpio_1, _cbEx, this);
}

void Wiegand::_cb(int gpio, int level, uint32_t tick)
{
   /*
      Accumulate bits until both gpios 0 and 1 timeout.
   */

   if (level == 0) /* a falling edge indicates a new bit */
   {
      if (!in_code)
      {
         bits = 1;
         num = 0;

         in_code = 1;
         code_timeout = 0;

         gpioSetWatchdog(mygpio_0, mytimeout);
         gpioSetWatchdog(mygpio_1, mytimeout);
      }
      else
      {
         bits++;
         num <<= 1;
      }

      if (gpio == mygpio_0)
      {
         code_timeout &= 2; /* clear gpio 0 timeout */
      }
      else
      {
         code_timeout &= 1; /* clear gpio 1 timeout */
         num |= 1;
      }
   }
   else if (level == PI_TIMEOUT)
   {
      if (in_code)
      {
         if (gpio == mygpio_0)
         {
            code_timeout |= 1; /* timeout gpio 0 */
         }
         else
         {
            code_timeout |= 2; /* timeout gpio 1 */
         }

         if (code_timeout == 3) /* both gpios timed out */
         {
            gpioSetWatchdog(mygpio_0, 0);
            gpioSetWatchdog(mygpio_1, 0);

            in_code = 0;

            (mycallback)(bits, num);
         }
      }
   }
}

void Wiegand::_cbEx(int gpio, int level, uint32_t tick, void *user)
{
   /*
      Need a static callback to link with C.
   */

   Wiegand *mySelf = (Wiegand *) user;

   mySelf->_cb(gpio, level, tick); /* Call the instance callback. */
}


void Wiegand::cancel(void)
{
   /*
      Cancel the Wiegand decoder.
   */

   gpioSetAlertFuncEx(mygpio_0, 0, this);
   gpioSetAlertFuncEx(mygpio_1, 0, this);
}

