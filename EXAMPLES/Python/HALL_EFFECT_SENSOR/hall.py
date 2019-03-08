#!/usr/bin/env python

import time

import pigpio

#
# OH3144E or equivalent Hall effect sensor
#
# Pin 1 - 5V
# Pin 2 - Ground
# Pin 3 - gpio (here P1-8, gpio 14, TXD is used)
#
# The internal gpio pull-up is enabled so that the sensor
# normally reads high.  It reads low when a magnet is close.
#

HALL=14

pi = pigpio.pi() # connect to local Pi

pi.set_mode(HALL, pigpio.INPUT)
pi.set_pull_up_down(HALL, pigpio.PUD_UP)

start = time.time()

while (time.time() - start) < 60:
   print("Hall = {}".format(pi.read(HALL)))
   time.sleep(0.2)

pi.stop()

