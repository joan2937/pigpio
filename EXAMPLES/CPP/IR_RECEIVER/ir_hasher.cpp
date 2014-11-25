#include <pigpio.h>

#include "ir_hasher.hpp"

void Hasher::_hash(int old_val, int new_val)
{
   int val;

   if      (new_val < (old_val * 0.60)) val = 13;
   else if (old_val < (new_val * 0.60)) val = 23;
   else                                 val = 2;

   hash_val ^= val;
   hash_val *= 16777619; /* FNV_PRIME_32 */
}

void Hasher::_callback(int gpio, int level, uint32_t tick)
{
   if (level != PI_TIMEOUT)
   {
      if (in_code == 0)
      {
         in_code = 1;

         gpioSetWatchdog(mygpio, mytimeout);

         hash_val = 2166136261U; /* FNV_BASIS_32 */

         edges = 1;

         t1 = 0;
         t2 = 0;
         t3 = 0;
         t4 = tick;
      }
      else
      {
         edges++;

         t1 = t2;
         t2 = t3;
         t3 = t4;
         t4 = tick;

         if (edges > 3) _hash(t2-t1, t4-t3);
      }
   }
   else
   {
      if (in_code)
      {
         in_code = 0;

         gpioSetWatchdog(mygpio, 0);

         if (edges > 12) /* Anything less is probably noise. */
         {
            (mycallback)(hash_val);
         }
      }
   }
}

void Hasher::_callbackExt(int gpio, int level, uint32_t tick, void *user)
{
   /*
      Need a static callback to link with C.
   */

   Hasher *mySelf = (Hasher *) user;

   mySelf->_callback(gpio, level, tick); /* Call the instance callback. */
}

Hasher::Hasher(int gpio, HasherCB_t callback, int timeout)
{
   /*
      Initialises an IR remote hasher on a gpio.  A gap of timeout
      milliseconds indicates the end of the remote key press.
   */
   mygpio     = gpio;
   mycallback = callback;
   mytimeout  = timeout;

   in_code = 0;

   gpioSetMode(gpio, PI_INPUT);

   gpioSetAlertFuncEx(gpio, _callbackExt, (void *)this);
}

