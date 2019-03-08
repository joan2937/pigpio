#include <stdio.h>

#include <pigpio.h>

#include "wiegand.h"

/*

REQUIRES

Wiegand contacts 0 and 1 connected to separate gpios.

TO BUILD

gcc -o wiegand_c test_wiegand.c wiegand.c -lpigpio -lrt

TO RUN

sudo ./wiegand_c

*/

void callback(int bits, uint32_t value)
{
   printf("bits=%d value=%u\n", bits, value);
}

int main(int argc, char *argv[])
{
   Pi_Wieg_t * w;

   if (gpioInitialise() < 0) return 1;

   w = Pi_Wieg(14, 15, callback, 5);

   sleep(300);

   Pi_Wieg_cancel(w);

   gpioTerminate();
}

