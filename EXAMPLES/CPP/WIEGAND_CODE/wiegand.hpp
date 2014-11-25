#ifndef WIEGAND_HPP
#define WIEGAND_HPP

#include <stdint.h>

typedef void (*WiegandCB_t)(int, uint32_t);

class Wiegand
{
   int mygpio_0, mygpio_1, mytimeout, in_code, bits;

   WiegandCB_t mycallback;

   uint32_t num;

   uint32_t code_timeout;

   void _cb(int gpio, int level, uint32_t tick);

   /* Need a static callback to link with C. */
   static void _cbEx(int gpio, int level, uint32_t tick, void *user);

   public:

   Wiegand(int gpio_0, int gpio_1, WiegandCB_t callback, int timeout=5);
   /*
      This function establishes a Wiegand decoder on gpio_0 and gpio_1.

      A gap of timeout milliseconds without a new bit indicates
      the end of a code.

      When the code is ended the callback function is called with the code
      bit length and value.
   */

   void cancel(void);
   /*
      This function releases the resources used by the decoder.
   */
};

#endif

