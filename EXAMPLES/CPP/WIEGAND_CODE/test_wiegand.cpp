#include <iostream>

#include <pigpio.h>

#include "wiegand.hpp"

/*

REQUIRES

Wiegand contacts 0 and 1 connected to separate gpios.

TO BUILD

g++ -o wiegand_cpp test_wiegand.cpp wiegand.cpp -lpigpio -lrt

TO RUN

sudo ./wiegand_cpp

*/

void callback(int bits, uint32_t value)
{
   std::cout << "bits=" << bits << " value=" << value << std::endl;
}

int main(int argc, char *argv[])
{
   if (gpioInitialise() < 0) return 1;

   Wiegand dec(14, 15, callback);

   sleep(300);

   dec.cancel();

   gpioTerminate();
}

