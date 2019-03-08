#include <iostream>

#include <pigpio.h>

#include "rotary_encoder.hpp"

/*

             +---------+         +---------+      0
             |         |         |         |
   A         |         |         |         |
             |         |         |         |
   +---------+         +---------+         +----- 1

       +---------+         +---------+            0
       |         |         |         |
   B   |         |         |         |
       |         |         |         |
   ----+         +---------+         +---------+  1

*/

void re_decoder::_pulse(int gpio, int level, uint32_t tick)
{
   if (gpio == mygpioA) levA = level; else levB = level;

   if (gpio != lastGpio) /* debounce */
   {
      lastGpio = gpio;

      if ((gpio == mygpioA) && (level == 1))
      {
         if (levB) (mycallback)(1);
      }
      else if ((gpio == mygpioB) && (level == 1))
      {
         if (levA) (mycallback)(-1);
      }
   }
}

void re_decoder::_pulseEx(int gpio, int level, uint32_t tick, void *user)
{
   /*
      Need a static callback to link with C.
   */

   re_decoder *mySelf = (re_decoder *) user;

   mySelf->_pulse(gpio, level, tick); /* Call the instance callback. */
}

re_decoder::re_decoder(int gpioA, int gpioB, re_decoderCB_t callback)
{
   mygpioA = gpioA;
   mygpioB = gpioB;

   mycallback = callback;

   levA=0;
   levB=0;

   lastGpio = -1;

   gpioSetMode(gpioA, PI_INPUT);
   gpioSetMode(gpioB, PI_INPUT);

   /* pull up is needed as encoder common is grounded */

   gpioSetPullUpDown(gpioA, PI_PUD_UP);
   gpioSetPullUpDown(gpioB, PI_PUD_UP);

   /* monitor encoder level changes */

   gpioSetAlertFuncEx(gpioA, _pulseEx, this);
   gpioSetAlertFuncEx(gpioB, _pulseEx, this);
}

void re_decoder::re_cancel(void)
{
   gpioSetAlertFuncEx(mygpioA, 0, this);
   gpioSetAlertFuncEx(mygpioB, 0, this);
}

