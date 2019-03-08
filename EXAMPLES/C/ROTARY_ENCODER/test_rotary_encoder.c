#include <stdio.h>

#include <pigpio.h>

#include "rotary_encoder.h"

/*

REQUIRES

A rotary encoder contacts A and B connected to separate gpios and
the common contact connected to Pi ground.

TO BUILD

gcc -o rot_enc_c test_rotary_encoder.c rotary_encoder.c -lpigpio -lrt

TO RUN

sudo ./rot_enc_c

*/

void callback(int way)
{
   static int pos = 0;

   pos += way;

   printf("pos=%d\n", pos);
}

int main(int argc, char *argv[])
{
   Pi_Renc_t * renc;

   if (gpioInitialise() < 0) return 1;

   renc = Pi_Renc(7, 8, callback);

   sleep(300);

   Pi_Renc_cancel(renc);

   gpioTerminate();
}

