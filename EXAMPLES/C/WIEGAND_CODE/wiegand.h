#ifndef WIEGAND_H
#define WIEGAND_H

#include <stdint.h>

typedef void (*Pi_Wieg_CB_t)(int, uint32_t);

struct _Pi_Wieg_s;

typedef struct _Pi_Wieg_s Pi_Wieg_t;

Pi_Wieg_t *Pi_Wieg(int gpio_0, int gpio_1, Pi_Wieg_CB_t callback, int timeout);
   /*
      This function establishes a Wiegand decoder on gpio_0 and gpio_1.

      A gap of timeout milliseconds without a new bit indicates the
      end of a code.

      When the code is ended the callback function is called with the code
      bit length and value.

      A pointer to a private data type is returned.  This should be passed
      to Pi_Wieg_cancel if the decoder is to be cancelled.
   */

void Pi_Wieg_cancel(Pi_Wieg_t *wieg);
   /*
      This function releases the resources used by the decoder.
   */

#endif

