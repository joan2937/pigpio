#include <iostream>

#include <pigpio.h>

#include "ir_hasher.hpp"

/*

REQUIRES

An IR receiver output pin connected to a Pi gpio.

TO BUILD

g++ -o ir_hash_cpp test_ir_hasher.cpp ir_hasher.cpp -lpigpio -lrt -lpthread

TO RUN

sudo ./ir_hash_cpp

*/

void callback(uint32_t hash)
{
   std::cout << "hash=" << hash << std::endl;
}

int main(int argc, char *argv[])
{
   if (gpioInitialise() >= 0)
   {
      /* Can't instantiate a Hasher before pigpio is initialised. */

      /* 
         This assumes the output pin of an IR receiver is
         connected to gpio 7.
      */

      Hasher ir(7, callback);

      sleep(300);
   }
}

