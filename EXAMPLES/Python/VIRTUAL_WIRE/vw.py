#!/usr/bin/env python
"""
This module provides a 313MHz/434MHz radio interface compatible
with the Virtual Wire library used on Arduinos.

It has been tested between a Pi, TI Launchpad, and Arduino Pro Mini.
"""
# 2014-08-14
# vw.py

import time

import pigpio

MAX_MESSAGE_BYTES=77

MIN_BPS=50
MAX_BPS=10000

_HEADER=[0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x38, 0x2c]

_CTL=3

_SYMBOL=[
   0x0d, 0x0e, 0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
   0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34]


def _sym2nibble(symbol):
   for nibble in range(16):
      if symbol == _SYMBOL[nibble]:
         return nibble
   return 0

def _crc_ccitt_update(crc, data):

   data = data ^ (crc & 0xFF);

   data = (data ^ (data << 4)) & 0xFF;

   return (
             (((data << 8) & 0xFFFF) | (crc >> 8)) ^
              ((data >> 4) & 0x00FF) ^ ((data << 3) & 0xFFFF)
          )

class tx():

   def __init__(self, pi, txgpio, bps=2000):
      """
      Instantiate a transmitter with the Pi, the transmit gpio,
      and the bits per second (bps).  The bps defaults to 2000.
      The bps is constrained to be within MIN_BPS to MAX_BPS.
      """
      self.pi = pi

      self.txbit = (1<<txgpio)

      if bps < MIN_BPS:
         bps = MIN_BPS
      elif bps > MAX_BPS:
         bps = MAX_BPS

      self.mics = int(1000000 / bps)

      self.wave_id = None

      pi.wave_add_new()

      pi.set_mode(txgpio, pigpio.OUTPUT)


   def _nibble(self, nibble):

      for i in range(6):
         if nibble & (1<<i):
            self.wf.append(pigpio.pulse(self.txbit, 0, self.mics))
         else:
            self.wf.append(pigpio.pulse(0, self.txbit, self.mics))

   def _byte(self, crc, byte):
      self._nibble(_SYMBOL[byte>>4])
      self._nibble(_SYMBOL[byte&0x0F])
      return _crc_ccitt_update(crc, byte)

   def put(self, data):
      """
      Transmit a message.  If the message is more than
      MAX_MESSAGE_BYTES in size it is discarded.  If a message
      is currently being transmitted it is aborted and replaced
      with the new message.  True is returned if message
      transmission has successfully started.  False indicates
      an error.
      """
      if len(data) > MAX_MESSAGE_BYTES:
         return False

      self.wf = []

      self.cancel()

      for i in _HEADER:
         self._nibble(i)

      crc = self._byte(0xFFFF, len(data)+_CTL)

      for i in data:

         if type(i) == type(""):
            v = ord(i)
         else:
            v = i

         crc = self._byte(crc, v)

      crc = ~crc

      self._byte(0, crc&0xFF)
      self._byte(0, crc>>8)

      self.pi.wave_add_generic(self.wf)

      self.wave_id = self.pi.wave_create()

      if self.wave_id >= 0:
         self.pi.wave_send_once(self.wave_id)
         return True
      else:
         return False


   def ready(self):
      """
      Returns True if a new message may be transmitted.
      """
      return not self.pi.wave_tx_busy()

   def cancel(self):
      """
      Cancels the wireless transmitter, aborting any message
      in progress.
      """
      if self.wave_id is not None:
         self.pi.wave_tx_stop()
         self.pi.wave_delete(self.wave_id)
         self.pi.wave_add_new()

      self.wave_id = None

class rx():

   def __init__(self, pi, rxgpio, bps=2000):
      """
      Instantiate a receiver with the Pi, the receive gpio, and
      the bits per second (bps).  The bps defaults to 2000.
      The bps is constrained to be within MIN_BPS to MAX_BPS.
      """
      self.pi = pi
      self.rxgpio = rxgpio

      self.messages = []
      self.bad_CRC = 0

      if bps < MIN_BPS:
         bps = MIN_BPS
      elif bps > MAX_BPS:
         bps = MAX_BPS

      slack = 0.20
      self.mics = int(1000000 / bps)
      slack_mics = int(slack * self.mics)
      self.min_mics = self.mics - slack_mics       # Shortest legal edge.
      self.max_mics = (self.mics + slack_mics) * 4 # Longest legal edge.

      self.timeout =  8 * self.mics / 1000 # 8 bits time in ms.
      if self.timeout < 8:
         self.timeout = 8

      self.last_tick = None
      self.good = 0
      self.bits = 0
      self.token = 0
      self.in_message = False
      self.message = [0]*(MAX_MESSAGE_BYTES+_CTL)
      self.message_len = 0
      self.byte = 0

      pi.set_mode(rxgpio, pigpio.INPUT)

      self.cb = pi.callback(rxgpio, pigpio.EITHER_EDGE, self._cb)

   def _calc_crc(self):

      crc = 0xFFFF
      for i in range(self.message_length):
         crc = _crc_ccitt_update(crc, self.message[i])
      return crc

   def _insert(self, bits, level):

      for i in range(bits):

         self.token >>= 1

         if level == 0:
            self.token |= 0x800

         if self.in_message:

            self.bits += 1

            if self.bits >= 12: # Complete token.

               byte = (
                  _sym2nibble(self.token & 0x3f) << 4 |
                  _sym2nibble(self.token >> 6))

               if self.byte == 0:
                  self.message_length = byte

                  if byte > (MAX_MESSAGE_BYTES+_CTL):
                     self.in_message = False # Abort message.
                     return

               self.message[self.byte] = byte

               self.byte += 1
               self.bits = 0

               if self.byte >= self.message_length:
                  self.in_message = False
                  self.pi.set_watchdog(self.rxgpio, 0)

                  crc = self._calc_crc()

                  if crc == 0xF0B8: # Valid CRC.
                     self.messages.append(
                        self.message[1:self.message_length-2])
                  else:
                     self.bad_CRC += 1

         else:
            if self.token == 0xB38: # Start message token.
               self.in_message = True
               self.pi.set_watchdog(self.rxgpio, self.timeout)
               self.bits = 0
               self.byte = 0

   def _cb(self, gpio, level, tick):

      if self.last_tick is not None:

         if level == pigpio.TIMEOUT:

            self.pi.set_watchdog(self.rxgpio, 0) # Switch watchdog off.

            if self.in_message:
               self._insert(4, not self.last_level)

            self.good = 0
            self.in_message = False

         else:

            edge = pigpio.tickDiff(self.last_tick, tick)

            if edge < self.min_mics:

               self.good = 0
               self.in_message = False

            elif edge > self.max_mics:

               if self.in_message:
                  self._insert(4, level)

               self.good = 0
               self.in_message = False

            else:

               self.good += 1

               if self.good > 8: 

                  bitlen = (100 * edge) / self.mics

                  if   bitlen < 140:
                     bits = 1
                  elif bitlen < 240:
                     bits = 2
                  elif bitlen < 340:
                     bits = 3
                  else:
                     bits = 4

                  self._insert(bits, level)

      self.last_tick = tick
      self.last_level = level

   def get(self):
      """
      Returns the next unread message, or None if none is avaiable.
      """
      if len(self.messages):
         return self.messages.pop(0)
      else:
         return None

   def ready(self):
      """
      Returns True if there is a message available to be read.
      """
      return len(self.messages)

   def cancel(self):
      """
      Cancels the wireless receiver.
      """
      if self.cb is not None:
         self.cb.cancel()
         self.pi.set_watchdog(self.rxgpio, 0)
      self.cb = None

if __name__ == "__main__":

   import time

   import pigpio

   import vw

   RX=11
   TX=25

   BPS=2000

   pi = pigpio.pi() # Connect to local Pi.

   rx = vw.rx(pi, RX, BPS) # Specify Pi, rx gpio, and baud.
   tx = vw.tx(pi, TX, BPS) # Specify Pi, tx gpio, and baud.

   msg = 0

   start = time.time()

   while (time.time()-start) < 300:

      msg += 1

      while not tx.ready():
         time.sleep(0.1)

      time.sleep(0.2)

      tx.put([48, 49, 65, ((msg>>6)&0x3F)+32, (msg&0x3F)+32])

      while not tx.ready():
         time.sleep(0.1)

      time.sleep(0.2)

      tx.put("Hello World #{}!".format(msg))

      while rx.ready():
         print("".join(chr (c) for c in rx.get()))

   rx.cancel()
   tx.cancel()

   pi.stop()

