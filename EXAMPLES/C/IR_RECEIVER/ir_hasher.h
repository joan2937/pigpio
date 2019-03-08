#ifndef IR_HASHER_H
#define IR_HASHER_H

#include <stdint.h>

typedef void (*Pi_Hasher_CB_t)(uint32_t);

struct _Pi_Hasher_s;

typedef struct _Pi_Hasher_s Pi_Hasher_t;

Pi_Hasher_t * Pi_Hasher(int gpio, Pi_Hasher_CB_t callback, int timeout);
   /*
      This function establishes an IR hasher on the gpio.

      A gap of timeout milliseconds without a new bit indicates
      the end of a code.

      When code end is detected the callback function is called
      with the code hash.

      A pointer to a private data type is returned.  This should be passed
      to Pi_Hasher_cancel if the hasher is to be cancelled.
   */


void Pi_Hasher_cancel(Pi_Hasher_t *hasher);
   /*
      This function releases the resources used by the hasher.
   */


#endif
