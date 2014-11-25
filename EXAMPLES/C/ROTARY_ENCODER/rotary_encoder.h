#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

typedef void (*Pi_Renc_CB_t)(int);

struct _Pi_Renc_s;

typedef struct _Pi_Renc_s Pi_Renc_t;

Pi_Renc_t * Pi_Renc(int gpioA, int gpioB, Pi_Renc_CB_t callback);
/*
   This function establishes a rotary encoder on gpioA and gpioB.

   When the encoder is turned the callback function is called.

   A pointer to a private data type is returned.  This should be passed
   to Pi_Renc_cancel if the rotary encoder is to be cancelled.
*/

void Pi_Renc_cancel(Pi_Renc_t *renc);
/*
   This function releases the resources used by the decoder.
*/

#endif
