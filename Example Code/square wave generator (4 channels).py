#!/usr/bin/env python

# 2014-11-15
# tarte.py
# Public Domain

import time

import pigpio

LED1=21
LED2=26
LED3=12
LED4=6

"""
7.5 10 12 16

Find the number of cycles needed for an integral switch on/off for each LED.

That's 2*7.5 which is 2 seconds worth.

In 2 seconds there will be this many cycles of on/off

15  20 24 32

How many micros for each cycle?

15 66666 on 66667 off = 1999995
20 50000 on 50000 off = 2000000
24 41666 on 41667 off = 1999992
32 31250 on 31250 off = 2000000

There will be a slight error which will not be detectable by most means.
"""

def wave(pi, gpio, hz, secs, on=1, offset=0):
   """
   Generate a hz cycles per second square wave on gpio for
   secs seconds.  The first transition is to level on at
   offset microseconds from the start.
   """
   micros_left = int(secs * 1000000)
   transitions = int(2 * hz * secs)
   micros = micros_left / transitions

   if (offset < 0) or (offset > micros):
      print("Illegal offset {} for hz {}".format(offset, hz))
      exit()

   wf = [] # Empty waveform.

   if offset:
      wf.append(pigpio.pulse(0, 0, offset))
      micros_left -= micros
      last_micros = micros - offset
      transitions -= 1

   for t in range(transitions, 0, -1):
      micros = micros_left / t
      if (t & 1) == (on & 1):
         wf.append(pigpio.pulse(0, 1<<gpio, micros))
      else:
         wf.append(pigpio.pulse(1<<gpio, 0, micros))
      micros_left -= micros

   if offset:
      if on:
         wf.append(pigpio.pulse(1<<gpio, 0, last_micros))
      else:
         wf.append(pigpio.pulse(0, 1<<gpio, last_micros))

   pi.wave_add_generic(wf)

pi = pigpio.pi() # Connect to local host.

pi.wave_clear()

wave(pi, LED1, 7.5, 2, 0, 23456)
wave(pi, LED2, 10,  2)
wave(pi, LED3, 12,  2, 1,17123)
wave(pi, LED4, 16,  2)

wid = pi.wave_create()

pi.set_mode(LED1, pigpio.OUTPUT)
pi.set_mode(LED2, pigpio.OUTPUT)
pi.set_mode(LED3, pigpio.OUTPUT)
pi.set_mode(LED4, pigpio.OUTPUT)

pi.wave_send_repeat(wid)

pi.stop()

