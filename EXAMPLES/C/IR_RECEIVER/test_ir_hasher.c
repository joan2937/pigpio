#include <stdio.h>

#include <pigpio.h>

#include "ir_hasher.h"

/*

REQUIRES

An IR receiver output pin connected to a Pi gpio.

TO BUILD

gcc -o ir_hash_c test_ir_hasher.c  ir_hasher.c -lpigpio -lrt -lpthread

TO RUN

sudo ./ir_hash_c

*/

void callback(uint32_t hash)
{
   printf("hash=%u\n", hash);
}

int main(int argc, char *argv[])
{
   Pi_Hasher_t *hasher;

   if (gpioInitialise() < 0) return 1;

   /* 
      This assumes the output pin of an IR receiver is
      connected to gpio 7.
   */

   hasher = Pi_Hasher(7, callback, 5);

   sleep(300);

   Pi_Hasher_cancel(hasher);

   gpioTerminate();
}

