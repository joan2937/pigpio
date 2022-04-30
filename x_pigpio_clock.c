
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* use local header for testing */
#include "pigpio.h"

#define DELAY_MS 50

int main(int argc, char *argv[])
{
  int ret, testClock;
  float ratio;
  unsigned cfgMicros, cfgPeripheral;

  testClock = -1;  /* -1: test both configurations; 0: test PWM; 1: test PCM */

  if ( 1 < argc ) {
    if (!strcmp(argv[1], "pcm"))
      testClock = 1;
    else if (!strcmp(argv[1], "pwm"))
      testClock = 0;
    else
      testClock = atoi(argv[1]);
    if (testClock)
      testClock = 1;
  }

  for ( cfgPeripheral = 0; cfgPeripheral < 2; ++cfgPeripheral ) {
    if ( !( testClock == -1 || testClock == cfgPeripheral ) )
      continue;

    cfgMicros = 5;
    fprintf(stdout, "testing %s clock\n", (cfgPeripheral ? "PCM" : "PWM") );

    gpioCfgClock(cfgMicros, cfgPeripheral, 0);
    if (gpioInitialise() < 0)
    {
      fprintf(stderr, "pigpio initialisation failed.\n");
      return (cfgPeripheral+1)*10;
    }

    ret = gpioTestClockTiming(DELAY_MS, &ratio);
    if (ret < 0) {
       fprintf(stderr, "error at gpioTestClockTiming()\n");
       return 1;
    }

    gpioTerminate();

    fprintf(stdout, "  measured clock ratio = %.3f\n", ratio);
    fprintf(stdout, "  %s: clock is %s\n", (ret ? "FAIL" : "PASS" ), (ret ? "skewed!" : "OK" ));
  }

  return 0;
}

