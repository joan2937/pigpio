#!/usr/bin/env python

import pigpio

morse={
'a':'.-'   , 'b':'-...' , 'c':'-.-.' , 'd':'-..'  , 'e':'.'    ,
'f':'..-.' , 'g':'--.'  , 'h':'....' , 'i':'..'   , 'j':'.---' ,
'k':'-.-'  , 'l':'.-..' , 'm':'--'   , 'n':'-.'   , 'o':'---'  ,
'p':'.--.' , 'q':'--.-' , 'r':'.-.'  , 's':'...'  , 't':'-'    ,
'u':'..-'  , 'v':'...-' , 'w':'.--'  , 'x':'-..-' , 'y':'-.--' ,
'z':'--..' , '1':'.----', '2':'..---', '3':'...--', '4':'....-',
'5':'.....', '6':'-....', '7':'--...', '8':'---..', '9':'----.',
'0':'-----'}

GPIO=22

MICROS=100000

NONE=0

DASH=3
DOT=1

GAP=1
LETTER_GAP=3-GAP
WORD_GAP=7-LETTER_GAP

def transmit_string(pi, gpio, str):

   pi.wave_clear() # start a new waveform

   wf=[]

   for C in str:
      c=C.lower()
      print(c)
      if c in morse:
         k = morse[c]
         for x in k:

            if x == '.':
               wf.append(pigpio.pulse(1<<gpio, NONE, DOT * MICROS))
            else:
               wf.append(pigpio.pulse(1<<gpio, NONE, DASH * MICROS))

            wf.append(pigpio.pulse(NONE, 1<<gpio, GAP * MICROS))

         wf.append(pigpio.pulse(NONE, 1<<gpio, LETTER_GAP * MICROS))

      elif c == ' ':
         wf.append(pigpio.pulse(NONE, 1<<gpio, WORD_GAP * MICROS))

   pi.wave_add_generic(wf)

   pi.wave_tx_start()

pi = pigpio.pi()

pi.set_mode(GPIO, pigpio.OUTPUT)

transmit_string(pi, GPIO, "Now is the winter of our discontent")

while pi.wave_tx_busy():
   pass

transmit_string(pi, GPIO, "made glorious summer by this sun of York")

while pi.wave_tx_busy():
   pass

pi.stop()

