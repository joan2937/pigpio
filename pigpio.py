"""
pigpio is a Python module for the Raspberry Pi which allows control
of the general purpose input outputs (gpios).

There are 54 gpios in total, arranged in two banks. Bank 1 contains
gpios 0-31. Bank 2 contains gpios 32-54.

Most of the gpios are dedicated to system use.

A user should only manipulate gpios in bank 1.

For a Rev.1 board only use gpios 0, 1, 4, 7, 8, 9, 10, 11, 14, 15, 17,
18, 21, 22, 23, 24, 25.

For a Rev.2 board only use gpios 2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 17,
18, 22, 23, 24, 25, 27, 28, 29, 30, 31.

It is safe to read all the gpios. If you try to write a system gpio or
change its mode you can crash the Pi or corrupt the data on the SD card.

Features

The pigpio module's main features are:

- provision of PWM on any number of gpios 0-31 simultaneously.

- provision of servo pulses on any number of gpios 0-31 simultaneously.

- callbacks when any of gpios 0-31 change state.

- reading/writing gpios and setting their modes (typically input
  or output).

- reading/writing all of the gpios in a bank (0-31, 32-53) as a single
  operation.

Notes

ALL gpios are identified by their Broadcom number.

This module uses the services of the C pigpio library.  That library
must be running on the Pi whose gpios are to be manipulated.

The normal way to start the library is as a daemon (during system
start).

sudo pigpiod

Your Python program should wrap the use of the module up in calls
to pigpio.start() and pigpio.stop().

Settings

A number of settings are determined when the pigpiod daemon is started.

- the sample rate (1, 2, 4, 5, 8, or 10us, default 5us).

- the set of gpios which may be updated (generally written to).  The
  default set is those listed above for the Rev.1 or Rev.2 boards.

- the available PWM frequencies (see set_PWM_frequency()).

Exceptions

By default a fatal exception is raised if you pass an invalid
argument to a pigpio function.

If you wish to handle the returned status yourself you should set
pigpio.exceptions = False.

"""
import socket
import struct
import time
import threading
import os
import atexit

VERSION = "1.3"

# gpio levels

OFF   = 0
LOW   = 0
CLEAR = 0

ON   = 1
HIGH = 1
SET  = 1

TIMEOUT = 2

# gpio edges

RISING_EDGE  = 0
FALLING_EDGE = 1
EITHER_EDGE  = 2

# gpio modes

INPUT  = 0
OUTPUT = 1
ALT0   = 4
ALT1   = 5
ALT2   = 6
ALT3   = 7
ALT4   = 3
ALT5   = 2

# gpio Pull Up Down

PUD_OFF  = 0
PUD_DOWN = 1
PUD_UP   = 2

# pigpio command numbers

_PI_CMD_MODES= 0
_PI_CMD_MODEG= 1
_PI_CMD_PUD=   2
_PI_CMD_READ=  3
_PI_CMD_WRITE= 4
_PI_CMD_PWM=   5
_PI_CMD_PRS=   6
_PI_CMD_PFS=   7
_PI_CMD_SERVO= 8
_PI_CMD_WDOG=  9
_PI_CMD_BR1=  10
_PI_CMD_BR2=  11
_PI_CMD_BC1=  12
_PI_CMD_BC2=  13
_PI_CMD_BS1=  14
_PI_CMD_BS2=  15
_PI_CMD_TICK= 16
_PI_CMD_HWVER=17
_PI_CMD_NO=   18
_PI_CMD_NB=   19
_PI_CMD_NP=   20
_PI_CMD_NC=   21
_PI_CMD_PRG=  22
_PI_CMD_PFG=  23
_PI_CMD_PRRG= 24
_PI_CMD_HELP= 25
_PI_CMD_PIGPV=26
_PI_CMD_WVCLR=27
_PI_CMD_WVAG= 28
_PI_CMD_WVAS= 29
_PI_CMD_WVGO= 30
_PI_CMD_WVGOR=31
_PI_CMD_WVBSY=32
_PI_CMD_WVHLT=33
_PI_CMD_WVSM= 34
_PI_CMD_WVSP= 35
_PI_CMD_WVSC= 36
_PI_CMD_TRIG= 37
_PI_CMD_PROC= 38
_PI_CMD_PROCD=39
_PI_CMD_PROCR=40
_PI_CMD_PROCS=41
_PI_CMD_SLRO= 42
_PI_CMD_SLR=  43
_PI_CMD_SLRC= 44


_PI_CMD_NOIB= 99


# pigpio error numbers

_PI_INIT_FAILED     =-1
PI_BAD_USER_GPIO    =-2
PI_BAD_GPIO         =-3
PI_BAD_MODE         =-4
PI_BAD_LEVEL        =-5
PI_BAD_PUD          =-6
PI_BAD_PULSEWIDTH   =-7
PI_BAD_DUTYCYCLE    =-8
_PI_BAD_TIMER       =-9
_PI_BAD_MS          =-10
_PI_BAD_TIMETYPE    =-11
_PI_BAD_SECONDS     =-12
_PI_BAD_MICROS      =-13
_PI_TIMER_FAILED    =-14
PI_BAD_WDOG_TIMEOUT =-15
_PI_NO_ALERT_FUNC   =-16
_PI_BAD_CLK_PERIPH  =-17
_PI_BAD_CLK_SOURCE  =-18
_PI_BAD_CLK_MICROS  =-19
_PI_BAD_BUF_MILLIS  =-20
PI_BAD_DUTYRANGE    =-21
_PI_BAD_SIGNUM      =-22
_PI_BAD_PATHNAME    =-23
PI_NO_HANDLE        =-24
PI_BAD_HANDLE       =-25
_PI_BAD_IF_FLAGS    =-26
_PI_BAD_CHANNEL     =-27
_PI_BAD_PRIM_CHANNEL=-27
_PI_BAD_SOCKET_PORT =-28
_PI_BAD_FIFO_COMMAND=-29
_PI_BAD_SECO_CHANNEL=-30
_PI_NOT_INITIALISED =-31
_PI_INITIALISED     =-32
_PI_BAD_WAVE_MODE   =-33
_PI_BAD_CFG_INTERNAL=-34
PI_BAD_WAVE_BAUD    =-35
PI_TOO_MANY_PULSES  =-36
PI_TOO_MANY_CHARS   =-37
PI_NOT_SERIAL_GPIO =-38
_PI_BAD_SERIAL_STRUC=-39
_PI_BAD_SERIAL_BUF  =-40
PI_NOT_PERMITTED    =-41
PI_SOME_PERMITTED   =-42
PI_BAD_WVSC_COMMND  =-43
PI_BAD_WVSM_COMMND  =-44
PI_BAD_WVSP_COMMND  =-45
PI_BAD_PULSELEN     =-46
PI_BAD_SCRIPT       =-47
PI_BAD_SCRIPT_ID    =-48
PI_BAD_SER_OFFSET   =-49
PI_GPIO_IN_USE      =-50

# pigpio error text

_errors=[
   [_PI_INIT_FAILED      , "pigpio initialisation failed"],
   [PI_BAD_USER_GPIO     , "gpio not 0-31"],
   [PI_BAD_GPIO          , "gpio not 0-53"],
   [PI_BAD_MODE          , "mode not 0-7"],
   [PI_BAD_LEVEL         , "level not 0-1"],
   [PI_BAD_PUD           , "pud not 0-2"],
   [PI_BAD_PULSEWIDTH    , "pulsewidth not 0 or 500-2500"],
   [PI_BAD_DUTYCYCLE     , "dutycycle not 0-255"],
   [_PI_BAD_TIMER        , "timer not 0-9"],
   [_PI_BAD_MS           , "ms not 10-60000"],
   [_PI_BAD_TIMETYPE     , "timetype not 0-1"],
   [_PI_BAD_SECONDS      , "seconds < 0"],
   [_PI_BAD_MICROS       , "micros not 0-999999"],
   [_PI_TIMER_FAILED     , "gpioSetTimerFunc failed"],
   [PI_BAD_WDOG_TIMEOUT  , "timeout not 0-60000"],
   [_PI_NO_ALERT_FUNC    , "DEPRECATED"],
   [_PI_BAD_CLK_PERIPH   , "clock peripheral not 0-1"],
   [_PI_BAD_CLK_SOURCE   , "clock source not 0-1"],
   [_PI_BAD_CLK_MICROS   , "clock micros not 1, 2, 4, 5, 8, or 10"],
   [_PI_BAD_BUF_MILLIS   , "buf millis not 100-10000"],
   [PI_BAD_DUTYRANGE     , "dutycycle range not 25-40000"],
   [_PI_BAD_SIGNUM       , "signum not 0-63"],
   [_PI_BAD_PATHNAME     , "can't open pathname"],
   [PI_NO_HANDLE         , "no handle available"],
   [PI_BAD_HANDLE        , "unknown notify handle"],
   [_PI_BAD_IF_FLAGS     , "ifFlags > 3"],
   [_PI_BAD_CHANNEL      , "DMA channel not 0-14"],
   [_PI_BAD_SOCKET_PORT  , "socket port not 1024-30000"],
   [_PI_BAD_FIFO_COMMAND , "unknown fifo command"],
   [_PI_BAD_SECO_CHANNEL , "DMA secondary channel not 0-6"],
   [_PI_NOT_INITIALISED  , "function called before gpioInitialise"],
   [_PI_INITIALISED      , "function called after gpioInitialise"],
   [_PI_BAD_WAVE_MODE    , "waveform mode not 0-1"],
   [_PI_BAD_CFG_INTERNAL , "bad parameter in gpioCfgInternals call"],
   [PI_BAD_WAVE_BAUD     , "baud rate not 100-250000"],
   [PI_TOO_MANY_PULSES   , "waveform has too many pulses"],
   [PI_TOO_MANY_CHARS    , "waveform has too many chars"],
   [PI_NOT_SERIAL_GPIO   , "no serial read in progress on gpio"],
   [PI_NOT_PERMITTED     , "no permission to update gpio"],
   [PI_SOME_PERMITTED    , "no permission to update one or more gpios"],
   [PI_BAD_WVSC_COMMND   , "bad WVSC subcommand"],
   [PI_BAD_WVSM_COMMND   , "bad WVSM subcommand"],
   [PI_BAD_WVSP_COMMND   , "bad WVSP subcommand"],
   [PI_BAD_PULSELEN      , "trigger pulse length > 100"],
   [PI_BAD_SCRIPT        , "invalid script"],
   [PI_BAD_SCRIPT_ID     , "unknown script id"],
   [PI_BAD_SER_OFFSET    , "add serial data offset > 30 minute"],
   [PI_GPIO_IN_USE       , "gpio already in use"],
]

_control = None
_notify  = None

_host = ''
_port = 8888

exceptions = True

class _pigpioError(Exception):
   """pigpio module exception"""
   def __init__(self, value):
      self.value = value
   def __str__(self):
      return repr(self.value)

def error(pigpio_error):
   """Converts a pigpio error number to a text description.

   pigpio_error: an error number (<0) returned by pigpio.

   Example
   ...
   print(pigpio.error(-5))
   level not 0-1
   ...
   """
   for e in _errors:
      if e[0] == pigpio_error:
         return e[1]
   return "unknown error ({})".format(pigpio_error)

def tickDiff(tStart, tEnd):
   """Calculate the time difference between two ticks.

   tStart: the earlier tick.
   tEnd:   the later tick.

   The function handles wrap around as the tick overflows 32 bits.

   The returned value is in microseconds.

   Example
   ...
   print(pigpio.tickDiff(4294967272, 12))
   36
   ...
   """
   tDiff = tEnd - tStart
   if tDiff < 0:
      tDiff += (1 << 32)
   return tDiff

def _u2i(number):
   """Converts a number from unsigned to signed.

   number: a 32 bit unsigned number
   """
   mask = (2 ** 32) - 1
   if number & (1 << 31):
      v = number | ~mask
   else:
      v = number & mask
   if v >= 0:
      return v;
   else:
      if exceptions:
         raise _pigpioError(error(v))
      else:
         return v

def _pigpio_command(sock, cmd, p1, p2):
   """
   Executes a pigpio socket command.

   sock: command socket.
   cmd:  the command to be executed.
   p1:   command paramter 1 (if applicable).
   p2:   command paramter 2 (if applicable).
   """
   if sock is not None:
      sock.send(struct.pack('IIII', cmd, p1, p2, 0))
      x, y, z, res = struct.unpack('IIII', sock.recv(16))
      return res
   else:
      raise  _pigpioError("*** Module not started, call pigpio.start() ***")

def _pigpio_command_ext(sock, cmd, p1, p2, extents):
   """
   Executes an extended pigpio socket command.

   sock: command socket.
   cmd:  the command to be executed.
   p1:   command paramter 1 (if applicable).
   p2:   command paramter 2 (if applicable).
   extents: additional data blocks
   """
   if sock is not None:
      sock.send(struct.pack('IIII', cmd, p1, p2, 0))

      for ext in extents:
         sock.sendall(ext)

      x, y, z, res = struct.unpack('IIII', sock.recv(16))
      return res
   else:
      raise  _pigpioError("*** Module not started, call pigpio.start() ***")

class _callback:
   """An ADT class to hold callback information."""

   def __init__(self, gpio, edge, func):
      """Initialises a callback ADT.

      gpio: Broadcom gpio number.
      edge: EITHER_EDGE, RISING_EDGE, or FALLING_EDGE.
      func: a user function taking three arguments (gpio, level, tick).
      """
      self.gpio = gpio
      self.edge = edge
      self.func = func
      self.bit = 1<<gpio

class _callback_thread(threading.Thread):
   """A class to encapsulate pigpio notification callbacks."""
   def __init__(self):
      """Initialises notifications."""
      threading.Thread.__init__(self)
      self.go = False
      self.daemon = True
      self.monitor = 0
      self.callbacks = []
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((_host,_port))
      self.handle = _pigpio_command(self.sock, _PI_CMD_NOIB, 0, 0)
      self.go = True
      self.start()

   def stop(self):
      """Stops notifications."""
      if self.go:
         self.go = False
         self.sock.send(struct.pack('IIII', _PI_CMD_NC, self.handle, 0, 0))

   def append(self, callb):
      """Adds a callback to the notification thread.

      callb:
      """
      self.callbacks.append(callb)
      self.monitor = self.monitor | callb.bit
      notify_begin(self.handle, self.monitor)

   def remove(self, callb):
      """Removes a callback from the notification thread.

      callb:
      """
      if callb in self.callbacks:
         self.callbacks.remove(callb)
         newMonitor = 0
         for c in self.callbacks:
            newMonitor |= c.bit
         if newMonitor != self.monitor:
            self.monitor = newMonitor
            notify_begin(self.handle, self.monitor)

   def run(self):

      """Execute the notification thread."""

      lastLevel = 0

      MSG_SIZ = 12

      while self.go:

         buf = self.sock.recv(MSG_SIZ)

         while self.go and len(buf) < MSG_SIZ:
            buf += self.sock.recv(MSG_SIZ-len(buf))

         if self.go:
            seq, flags, tick, level = (struct.unpack('HHII', buf))

            if flags == 0:
               changed = level ^ lastLevel
               lastLevel = level
               for cb in self.callbacks:
                  if cb.bit & changed:
                     newLevel = 0
                     if cb.bit & level:
                        newLevel = 1
                     if (cb.edge ^ newLevel):
                         cb.func(cb.gpio, newLevel, tick)
            else:
               gpio = flags & 31
               for cb in self.callbacks:
                  if cb.gpio == gpio:
                     cb.func(cb.gpio, TIMEOUT, tick)

      self.sock.close()

class _wait_for_edge:

   """A class to encapsulate waiting for gpio edges."""

   def __init__(self, gpio, edge, timeout):
      """Initialise a wait_for_edge.

      gpio:
      edge:
      timeout:
      """
      self.callb = _callback(gpio, edge, self.func)
      self.trigger = False
      _notify.append(self.callb)
      self.start = time.time()
      while (self.trigger == False) and ((time.time()-self.start) < timeout):
         time.sleep(0.1)
      _notify.remove(self.callb)

   def func(self, gpio, level, tick):
      """Set wait_for_edge triggered.

      gpio:
      level:
      tick:
      """
      self.trigger = True


def set_mode(gpio, mode):
   """Set the gpio mode.

   gpio: 0-53.
   mode: INPUT, OUTPUT, ALT0, ALT1, ALT2, ALT3, ALT4, ALT5.

   Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_MODE,
   or PI_NOT_PERMITTED.

   Notes

   Arduino style: pinMode.

   Example
   ...
   pigpio.set_mode(4, pigpio.INPUT) # gpio 4 as input
   pigpio.set_mode(7, pigpio.OUTPUT) # gpio 7 as output
   pigpio.set_mode(10, pigpio.ALT2) # gpio 10 as ALT2
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_MODES, gpio, mode))

def get_mode(gpio):
   """Get the gpio mode.

   Returns the gpio mode if OK, otherwise PI_BAD_GPIO.

   gpio: 0-53.

   Example
   ...
   print(pigpio.get_mode(4))
   0
   print(pigpio.get_mode(7))
   1
   print(pigpio.get_mode(10))
   6
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_MODEG, gpio, 0))

def set_pull_up_down(gpio, pud):
   """Set or clear the gpio pull-up/down resistor.

   Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_PUD,
   or PI_NOT_PERMITTED.

   gpio: 0-53.
   pud:  PUD_UP, PUD_DOWN, PUD_OFF.

   Example
   ...
   pigpio.set_mode(23, pigpio.INPUT)
   pigpio.set_mode(24, pigpio.INPUT)

   pigpio.set_pull_up_down(23, pigpio.PUD_UP)
   pigpio.set_pull_up_down(24, pigpio.PUD_DOWN)

   print(pigpio.read(23))
   1
   print(pigpio.read(24))
   0

   pigpio.set_pull_up_down(23, pigpio.PUD_DOWN)
   pigpio.set_pull_up_down(24, pigpio.PUD_UP)

   print(pigpio.read(23))
   0
   print(pigpio.read(24))
   1
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PUD, gpio, pud))

def read(gpio):
   """Read the gpio level.

   Returns the gpio level if OK, otherwise PI_BAD_GPIO.

   gpio:0-53.

   Notes

   Arduino style: digitalRead.

   Example
   ...
   pigpio.set_mode(25, pigpio.INPUT)

   pigpio.set_pull_up_down(25, pigpio.PUD_DOWN)

   print(pigpio.read(25))
   0

   pigpio.set_pull_up_down(25, pigpio.PUD_UP)

   print(pigpio.read(25))
   1
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_READ, gpio, 0))

def write(gpio, level):
   """Write the gpio level.

   Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_LEVEL,
   or PI_NOT_PERMITTED.

   gpio:  0-53.
   level: 0, 1.

   Notes

   If PWM or servo pulses are active on the gpio they are switched off.

   Arduino style: digitalWrite

   Example
   ...
   pigpio.set_mode(11, pigpio.OUTPUT)

   pigpio.write(11,0)

   print(pigpio.read(11))
   0

   pigpio.write(11,1)

   print(pigpio.read(11))
   1
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WRITE, gpio, level))

def set_PWM_dutycycle(user_gpio, dutycycle):
   """Start (non-zero dutycycle) or stop (0) PWM pulses on the gpio.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_DUTYCYCLE,
   or PI_NOT_PERMITTED.

   user_gpio: 0-31.
   dutycycle: 0-range (range defaults to 255).

   Notes

   Arduino style: analogWrite

   This and the servo functionality use the DMA and PWM or PCM
   peripherals to control and schedule the pulse lengths and
   duty cycles.

   The set_PWM_range() function can change the default range of 255.

   Example
   ...
   set_PWM_dutycycle(4,   0) # PWM off
   set_PWM_dutycycle(4,  64) # PWM 1/4 on
   set_PWM_dutycycle(4, 128) # PWM 1/2 on
   set_PWM_dutycycle(4, 192) # PWM 3/4 on
   set_PWM_dutycycle(4, 255) # PWM full on
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PWM, user_gpio, dutycycle))

def set_PWM_range(user_gpio, range_):
   """Set the range of PWM values to be used on the gpio.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_DUTYRANGE,
   or PI_NOT_PERMITTED.

   user_gpio: 0-31.
   range_:    25-40000.

   Notes

   If PWM is currently active on the gpio its dutycycle will be
   scaled to reflect the new range.

   The real range, the number of steps between fully off and fully on
   for each of the 18 available gpio frequencies is

   25(#1), 50(#2), 100(#3), 125(#4), 200(#5), 250(#6), 400(#7),
   500(#8), 625(#9), 800(#10), 1000(#11), 1250(#12), 2000(#13),
   2500(#14), 4000(#15), 5000(#16), 10000(#17), 20000(#18)

   The real value set by set_PWM_range is
   (dutycycle * real range) / range.

   Example
   ...
   pigpio.set_PWM_range(9, 100) # now 25 1/4, 50 1/2, 75 3/4 on

   pigpio.set_PWM_range(9, 500) # now 125 1/4, 250 1/2, 375 3/4 on

   pigpio.set_PWM_range(9, 3000) # now 750 1/4, 1500 1/2, 2250 3/4 on
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PRS, user_gpio, range_))

def get_PWM_range(user_gpio):
   """Get the range of PWM values being used on the gpio.

   Returns the dutycycle range used for the gpio if OK,
   otherwise PI_BAD_USER_GPIO.

   user_gpio: 0-31.

   Example
   ...
   print(pigpio.get_PWM_range(9))
   255

   pigpio.set_PWM_range(9, 100)
   print(pigpio.get_PWM_range(9))
   100

   pigpio.set_PWM_range(9, 500)
   print(pigpio.get_PWM_range(9))
   500

   pigpio.set_PWM_range(9, 3000)
   print(pigpio.get_PWM_range(9))
   3000
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PRG, user_gpio, 0))

def get_PWM_real_range(user_gpio):
   """Get the real underlying range of PWM values being used on the gpio.

   Returns the real range used for the gpio if OK,
   otherwise PI_BAD_USER_GPIO.

   user_gpio: 0-31.

   Example
   ...
   pigpio.set_PWM_frequency(4,0)

   print(pigpio.get_PWM_real_range(4))
   20000

   pigpio.set_PWM_frequency(4,800)
   print(pigpio.get_PWM_real_range(4))
   250

   pigpio.set_PWM_frequency(4,100000)
   print(pigpio.get_PWM_real_range(4))
   25
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PRRG, user_gpio, 0))

def set_PWM_frequency(user_gpio, frequency):
   """Set the frequency (in Hz) of the PWM to be used on the gpio.

   Returns the numerically closest frequency if OK, otherwise
   PI_BAD_USER_GPIO or PI_NOT_PERMITTED.

   user_gpio: 0-31.
   frequency: 0- (Hz).

   The selectable frequencies depend upon the sample rate which
   may be 1, 2, 4, 5, 8, or 10 microseconds (default 5).  The
   sample rate is set when the C pigpio library is started.

   Each gpio can be independently set to one of 18 different
   PWM frequencies.

   If PWM is currently active on the gpio it will be switched
   off and then back on at the new frequency.

   1us 40000, 20000, 10000, 8000, 5000, 4000, 2500, 2000, 1600,
       1250, 1000, 800, 500, 400, 250, 200, 100, 50

   2us 20000, 10000,  5000, 4000, 2500, 2000, 1250, 1000, 800,
       625, 500, 400, 250, 200, 125, 100, 50, 25

   4us 10000, 5000, 2500, 2000, 1250, 1000, 625, 500, 400,
       313, 250, 200, 125, 100, 63, 50, 25, 13

   5us 8000, 4000, 2000, 1600, 1000, 800, 500, 400, 320,
       250, 200, 160, 100, 80, 50, 40, 20, 10

   8us 5000, 2500, 1250, 1000, 625, 500, 313, 250, 200,
       156, 125, 100, 63, 50, 31, 25, 13, 6

   10us 4000, 2000, 1000, 800, 500, 400, 250, 200, 160,
        125, 100, 80, 50, 40, 25, 20, 10, 5

   Example
   ...
   pigpio.set_PWM_frequency(4,0)

   print(pigpio.get_PWM_frequency(4))
   10

   pigpio.set_PWM_frequency(4,800)

   print(pigpio.get_PWM_frequency(4))
   800

   pigpio.set_PWM_frequency(4,100000)

   print(pigpio.get_PWM_frequency(4))
   8000
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PFS, user_gpio, frequency))

def get_PWM_frequency(user_gpio):
   """Get the frequency of PWM being used on the gpio.

   Returns the frequency (in hertz) used for the gpio if OK,
   otherwise PI_BAD_USER_GPIO.

   user_gpio: 0-31.

   Example
   ...
   pigpio.set_PWM_frequency(4,0)

   print(pigpio.get_PWM_frequency(4))
   10

   pigpio.set_PWM_frequency(4,800)

   print(pigpio.get_PWM_frequency(4))
   800

   pigpio.set_PWM_frequency(4,100000)

   print(pigpio.get_PWM_frequency(4))
   8000
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PFG, user_gpio, 0))

def set_servo_pulsewidth(user_gpio, pulsewidth):
   """Start (500-2500) or stop (0) servo pulses on the gpio.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_PULSEWIDTH or
   PI_NOT_PERMITTED.

   user_gpio:  0-31.
   pulsewidth: 0 (off), 500 (most anti-clockwise) - 2500 (most clockwise).

   The selected pulsewidth will continue to be transmitted until
   changed by a subsequent call to set_servo_pulsewidth().

   The pulsewidths supported by servos varies and should probably be
   determined by experiment. A value of 1500 should always be safe and
   represents the mid-point of rotation.

   You can DAMAGE a servo if you command it to move beyond its limits.

   OTHER UPDATE RATES:

   This function updates servos at 50Hz.  If you wish to use a different
   update frequency you will have to use the PWM functions.

   Update Rate (Hz) 50   100  200  400  500
   1E6/Hz        20000 10000 5000 2500 2000

   Firstly set the desired PWM frequency using set_PWM_frequency().

   Then set the PWM range using set_PWM_range() to 1E6/Hz.
   Doing this allows you to use units of microseconds when setting
   the servo pulse width.

   E.g. If you want to update a servo connected to gpio 25 at 400Hz

   set_PWM_frequency(25, 400)
   set_PWM_range(25, 2500)

   Thereafter use the set_PWM_dutycycle() function to move the servo,
   e.g. set_PWM_dutycycle(25, 1500) will set a 1500 us pulse. 

   Example 1: standard 50 Hz hobby servo updates

   #!/usr/bin/python

   import pigpio
   import time

   moves = [[1000, 5],[1200,3],[1500,2],[2000,5],[1000,0]]

   pigpio.start()

   for m in moves:
      pigpio.set_servo_pulsewidth(24, m[0]);
      time.sleep(m[1])
      s = str(m[1]) + " seconds @ " + str(m[0]) + " us"
      print(s)

   pigpio.stop()

   will print lines

   5 seconds @ 1000 us
   3 seconds @ 1200 us
   2 seconds @ 1500 us
   5 seconds @ 2000 us
   0 seconds @ 1000 us

   Example 2: 400 Hz ESC type servo updates

   #!/usr/bin/python

   import pigpio
   import time

   moves = [[1000, 5],[1200,3],[1500,2],[2000,5],[1000,0]]

   pigpio.start()

   pigpio.set_PWM_frequency(25, 400)
   pigpio.set_PWM_range(25, 2500)

   for m in moves:
      pigpio.set_PWM_dutycycle(25, m[0]);
      time.sleep(m[1])
      s = str(m[1]) + " seconds @ " + str(m[0]) + " us"
      print(s)

   pigpio.stop()

   will print lines

   5 seconds @ 1000 us
   3 seconds @ 1200 us
   2 seconds @ 1500 us
   5 seconds @ 2000 us
   0 seconds @ 1000 us
"""
   return _u2i(_pigpio_command(_control, _PI_CMD_SERVO, user_gpio, pulsewidth))

def notify_open():
   """Get a free notification handle.

   Returns a handle greater than or equal to zero if OK,
   otherwise PI_NO_HANDLE.

   A notification is a method for being notified of gpio state
   changes via a pipe.

   Pipes are only accessible from the local machine so this function
   serves no purpose if you are using Python from a remote machine.
   The in-built (socket) notifications provided by callback()
   should be used instead.

   Notifications for handle x will be available at the pipe
   named /dev/pigpiox (where x is the handle number).
   E.g. if the function returns 15 then the notifications must be
   read from /dev/pigpio15.

   Example
   ...
   h = pigpio.notify_open()
   if h >= 0:
      pigpio.notify_begin(h, 1234)
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_NO, 0, 0))

def notify_begin(handle, bits):
   """Start notifications on a previously opened handle.

   Returns 0 if OK, otherwise PI_BAD_HANDLE.

   handle: 0-31 (as returned by notify_open())
   bits:   a mask indicating the gpios to be notified.

   The notification sends state changes for each gpio whose
   corresponding bit in bits is set.

   Example
   ...
   h = pigpio.notify_open()
   if h >= 0:
      pigpio.notify_begin(h, 1234)
   ...

   This will start notifications for gpios 1, 4, 6, 7, 10
   (1234 = 0x04D2 = 0b0000010011010010).

   Notes

   Each notification occupies 12 bytes in the fifo as follows:

   H (16 bit) seqno
   H (16 bit) flags
   I (32 bit) tick
   I (32 bit) level

   """
   return _u2i(_pigpio_command(_control, _PI_CMD_NB, handle, bits))

def notify_pause(handle):
   """Pause notifications on a previously opened handle.

   Returns 0 if OK, otherwise PI_BAD_HANDLE.

   handle: 0-31 (as returned by notify_open())

   Notifications for the handle are suspended until
   notify_begin() is called again.

   Example
   ...
   h = pigpio.notify_open()
   if h >= 0:
      pigpio.notify_begin(h, 1234)
      ...
      pigpio.notify_pause(h)
      ...
      pigpio.notify_begin(h, 1234)
      ...
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_NB, handle, 0))

def notify_close(handle):
   """Stop notifications on a previously opened handle and
   release the handle for reuse.

   Returns 0 if OK, otherwise PI_BAD_HANDLE.

   handle: 0-31 (as returned by notify_open())

   Example
   ...
   h = pigpio.notify_open()
   if h >= 0:
      pigpio.notify_begin(h, 1234)
      ...
      pigpio.notify_close(h)
      ...
   ...
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_NC, handle, 0))

def set_watchdog(user_gpio, timeout):
   """Sets a watchdog for a gpio.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO
   or PI_BAD_WDOG_TIMEOUT.

   user_gpio: 0-31.
   timeout:   0-60000.

   The watchdog is nominally in milliseconds.

   Only one watchdog may be registered per gpio.

   The watchdog may be cancelled by setting timeout to 0.

   If no level change has been detected for the gpio for timeout
   milliseconds any notification for the gpio has a report written
   to the fifo with the flags set to indicate a watchdog timeout.

   The callback() class interprets the flags and will
   call registered callbacks for the gpio with level TIMEOUT.

   Example

   #!/usr/bin/python

   import pigpio
   import time

   def cbf(g, L, t):
      s = "gpio=" + str(g) + " level=" + str(L) + " at " + str(t)
      print(s)

   pigpio.start()

   cb = pigpio.callback(22, pigpio.EITHER_EDGE, cbf)

   print("callback started, 5 second delay")

   time.sleep(5)

   pigpio.set_watchdog(22, 1000) # 1000ms watchdog

   print("watchdog started, 5 second delay")

   time.sleep(5)

   pigpio.set_watchdog(22, 0) # cancel watchdog

   print("watchdog cancelled, 5 second delay")

   time.sleep(5)

   cb.cancel()

   pigpio.stop()

   will print lines such as

   callback started, 5 second delay
   watchdog started, 5 second delay
   gpio=22 level=2 at 3547411617
   gpio=22 level=2 at 3548411254
   gpio=22 level=2 at 3549411927
   gpio=22 level=2 at 3550412060
   gpio=22 level=2 at 3551411622
   watchdog cancelled, 5 second delay
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WDOG, user_gpio, timeout))

def read_bank_1():
   """Read the levels of the bank 1 gpios (gpios 0-31).

   The returned 32 bit integer has a bit set if the corresponding
   gpio is logic 1.  Gpio n has bit value (1<<n).

   Example
   ...
   print(bin(pigpio.read_bank_1()))
   0b10010100000011100100001001111
   ...
   """
   return _pigpio_command(_control, _PI_CMD_BR1, 0, 0)

def read_bank_2():
   """Read the levels of the bank 2 gpios (gpios 32-53).

   The returned 32 bit integer has a bit set if the corresponding
   gpio is logic 1.  Gpio n has bit value (1<<(n-32)).

   Example
   ...
   print(bin(pigpio.read_bank_2()))
   0b1111110000000000000000
   ...
   """
   return _pigpio_command(_control, _PI_CMD_BR2, 0, 0)

def clear_bank_1(levels):
   """Clears gpios 0-31 if the corresponding bit in levels is set.

   Returns 0 if OK, otherwise PI_SOME_PERMITTED.

   A status of PI_SOME_PERMITTED indicates that the user is not
   allowed to write to one or more of the gpios.

   levels: a bit mask with 1 set if the corresponding gpio is
           to be cleared.

   Example

   #!/usr/bin/python

   import pigpio

   pigpio.start()

   pigpio.set_mode(4,  pigpio.OUTPUT)
   pigpio.set_mode(7,  pigpio.OUTPUT)
   pigpio.set_mode(8,  pigpio.OUTPUT)
   pigpio.set_mode(9,  pigpio.OUTPUT)
   pigpio.set_mode(10, pigpio.OUTPUT)
   pigpio.set_mode(11, pigpio.OUTPUT)

   pigpio.set_bank_1(int("111110010000",2))

   # 0x1000 is added so that all numbers are aligned

   b1 = (pigpio.read_bank_1() & 0xFFF) + 0x1000
   print(bin(b1))

   pigpio.clear_bank_1(int("111110010000",2))

   b2 = (pigpio.read_bank_1() & 0xFFF) + 0x1000
   print(bin(b2))

   print(bin((b1^b2) + 0x1000))

   pigpio.stop()

   displays

   0b1111111011111
   0b1000001001111
   0b1111110010000

   """
   return _u2i(_pigpio_command(_control, _PI_CMD_BC1, levels, 0))

def clear_bank_2(levels):
   """Clears gpios 32-53 if the corresponding bit (0-21) in levels is set.

   Returns 0 if OK, otherwise PI_SOME_PERMITTED.

   A status of PI_SOME_PERMITTED indicates that the user is not
   allowed to write to one or more of the gpios.

   levels: a bit mask with 1 set if the corresponding gpio is
           to be cleared.

   See clear_bank_1() for an example.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_BC2, levels, 0))

def set_bank_1(levels):
   """Sets gpios 0-31 if the corresponding bit in levels is set.

   Returns 0 if OK, otherwise PI_SOME_PERMITTED.

   A status of PI_SOME_PERMITTED indicates that the user is not
   allowed to write to one or more of the gpios.

   levels: a bit mask with 1 set if the corresponding gpio is
           to be set.

   Example

   #!/usr/bin/python

   import pigpio

   pigpio.start()

   pigpio.set_mode(4,  pigpio.OUTPUT)
   pigpio.set_mode(7,  pigpio.OUTPUT)
   pigpio.set_mode(8,  pigpio.OUTPUT)
   pigpio.set_mode(9,  pigpio.OUTPUT)
   pigpio.set_mode(10, pigpio.OUTPUT)
   pigpio.set_mode(11, pigpio.OUTPUT)

   pigpio.clear_bank_1(int("111110010000",2))

   # 0x1000 is added so that all numbers are aligned

   b1 = (pigpio.read_bank_1() & 0xFFF) + 0x1000
   print(bin(b1))

   pigpio.set_bank_1(int("111110010000",2))

   b2 = (pigpio.read_bank_1() & 0xFFF) + 0x1000
   print(bin(b2))

   print(bin((b1^b2) + 0x1000))

   pigpio.stop()

   displays

   0b1000001001111
   0b1111111011111
   0b1111110010000
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_BS1, levels, 0))

def set_bank_2(levels):
   """Sets gpios 32-53 if the corresponding bit (0-21) in levels is set.

   Returns 0 if OK, otherwise PI_SOME_PERMITTED.

   A status of PI_SOME_PERMITTED indicates that the user is not
   allowed to write to one or more of the gpios.

   levels: a bit mask with 1 set if the corresponding gpio is
           to be set.

   See set_bank_1() for an example.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_BS2, levels, 0))

def get_current_tick():
   """Gets the current system tick.

   Tick is the number of microseconds since system boot.

   As tick is an unsigned 32 bit quantity it wraps around after
   2**32 microseconds, which is approximately 1 hour 12 minutes.

   Example

   #!/usr/bin/python

   import pigpio
   import time

   pigpio.start()

   t1 = pigpio.get_current_tick()

   time.sleep(5)

   t2 = pigpio.get_current_tick()

   s = "5 seconds is " + str(pigpio.tickDiff(t1, t2)) + " ticks"

   print(s) 

   pigpio.stop()

   displays

   5 seconds is 5003398 ticks
   """
   return _pigpio_command(_control, _PI_CMD_TICK, 0, 0)

def get_hardware_revision():
   """Get the Pi's hardware revision number.

   It is unfortunate that Pi boards have been named Revision.1 and
   Revision.2.  That use of the word revision is distinct from the
   Pi's hardware revision number.'

   The hardware revision is the last 4 characters on the Revision line
   of /proc/cpuinfo.

   The revision number can be used to determine the assignment of gpios
   to pins.

   There are at least two types of board.

   Type 1 has gpio 0 on P1-3, gpio 1 on P1-5, and gpio 21 on P1-13.
   Type 2 has gpio 2 on P1-3, gpio 3 on P1-5, gpio 27 on P1-13, and
   gpios 28-31 on P5.

   Type 1 boards have hardware revision numbers of 2 and 3.

   Type 2 boards have hardware revision numbers of 4, 5, 6, and 15.

   If the hardware revision can not be found or is not a valid
   hexadecimal number the function returns 0.

   Example 1:
   ...
   print(pigpio.get_hardware_revision())
   2
   ...
   Example 2:

   for "Revision : 0002" the function returns 2.
   for "Revision : 000f" the function returns 15.
   for "Revision : 000g" the function returns 0.
   """
   return _pigpio_command(_control, _PI_CMD_HWVER, 0, 0)

def get_pigpio_version():
   """
   Returns the pigpio software version.
   """
   return _pigpio_command(_control, _PI_CMD_PIGPV, 0, 0)

class pulse:
   """
   An ADT class to hold pulse information.
   """

   def __init__(self, gpio_on, gpio_off, delay):
      """
      Initialises a pulse ADT.

      gpio_on: the gpios to switch on at the start of the pulse.
      gpio_off: the gpios to switch off at the start of the pulse.
      delay: the delay in microseconds before the next pulse.
      """
      self.gpio_on = gpio_on
      self.gpio_off = gpio_off
      self.delay = delay

def wave_clear():
   """
   Initialises a new waveform.

   Returns 0 if OK.

   A waveform comprises one of more pulses.

   A pulse specifies

   1) the gpios to be switched on at the start of the pulse.
   2) the gpios to be switched off at the start of the pulse.
   3) the delay in microseconds before the next pulse.

   Any or all the fields can be zero.  It doesn't make any sense
   to set all the fields to zero (the pulse will be ignored).

   When a waveform is started each pulse is executed in order with
   the specified delay between the pulse and the next.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVCLR, 0, 0))

def wave_add_generic(pulses):
   """
   Adds a list of pulses to the current waveform.

   Returns the new total number of pulses in the current waveform
   if OK, otherwise PI_TOO_MANY_PULSES.

   pulses: list of pulses to add to the waveform.

   The pulses are interleaved in time order within the existing
   waveform (if any).

   Merging allows the waveform to be built in parts, that is the
   settings for gpio#1 can be added, and then gpio#2 etc.

   If the added waveform is intended to start after or within
   the existing waveform then the first pulse should consist
   solely of a delay.

   Example

   #!/usr/bin/env python

   import time
   import pigpio

   class stepper:

      def __init__(self, g1, g2, g3, g4):
         self.g1 = g1
         self.g2 = g2
         self.g3 = g3
         self.g4 = g4
         self.all = (1<<g1 | 1<<g2 | 1<<g3 | 1<<g4)

         pigpio.set_mode(g1, pigpio.OUTPUT)
         pigpio.set_mode(g2, pigpio.OUTPUT)
         pigpio.set_mode(g3, pigpio.OUTPUT)
         pigpio.set_mode(g4, pigpio.OUTPUT)

      def step_on(self, pos):
         if   pos == 0: return (1<<self.g4)
         elif pos == 1: return (1<<self.g3 | 1<<self.g4)
         elif pos == 2: return (1<<self.g3)
         elif pos == 3: return (1<<self.g2 | 1<<self.g3)
         elif pos == 4: return (1<<self.g2)
         elif pos == 5: return (1<<self.g1 | 1<<self.g2)
         elif pos == 6: return (1<<self.g1)
         elif pos == 7: return (1<<self.g1 | 1<<self.g4)
         else:          return 0

      def step_off(self, pos):
         return self.step_on(pos) ^ self.all

   pigpio.start()

   s1 = stepper(14, 15, 18, 17)
   s2 = stepper(24, 25,  8,  7)

   f1=[] # pulses to drive stepper 1 forward
   b2=[] # pulses to drive stepper 2 backward

   for i in range(8):
      f1.append(pigpio.pulse(s1.step_on(i), s1.step_off(i), 1200))
      b2.append(pigpio.pulse(s2.step_on(7-i), s2.step_off(7-i), 1200))

   pigpio.wave_clear() # initialise a new waveform

   pigpio.wave_add_generic(f1) # add stepper 1 forward
   pigpio.wave_add_generic(b2) # add stepper 2 backward

   pigpio.wave_tx_repeat() # repeately transmit pulses

   time.sleep(10)

   pigpio.wave_tx_stop() # stop waveform

   pigpio.stop()
   """
   # pigpio message format

   # I p1 number of pulses
   # I p2 0
   ## extension ##
   # III on/off/delay * number of pulses
   msg = ""
   for p in pulses:
      msg += struct.pack("III", p.gpio_on, p.gpio_off, p.delay)
   extents = [msg]
   return _u2i(_pigpio_command_ext(
      _control, _PI_CMD_WVAG, len(pulses), 0, extents))

def wave_add_serial(user_gpio, baud, offset, data):
   """
   Adds a waveform representing serial data to the existing waveform
   (if any).  The serial data starts offset microseconds from the
   start of the waveform.

   Returns the new total number of pulses in the current waveform
   if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD,
   PI_TOO_MANY_CHARS, PI_BAD_SER_OFFSET, or PI_TOO_MANY_PULSES.

   user_gpio: gpio to transmit data.  You must set the gpio mode
              to output.
   baud: baud rate to use.
   offset: number of microseconds from the starts of the waveform.
   data: the data to transmit.

   The serial data is formatted as one start bit, eight data bits,
   and one stop bit.

   It is legal to add serial data streams with different baud rates
   to the same waveform.

   Example

   #!/usr/bin/env python

   import time

   import pigpio

   GPIO=24

   pigpio.start()

   pigpio.set_mode(TX_GPIO, pigpio.OUTPUT)

   pigpio.wave_clear() # initialise waveform

   for i in range(10):
      pigpio.wave_add_serial(
         GPIO, 9600, i*2000000, "{} seconds in.\r\n".format(i*2))

   pigpio.wave_tx_start()

   time.sleep(22)

   pigpio.stop()
   """
   # pigpio message format

   # I p1 user_gpio
   # I p2 len(data)
   ## extension ##
   # I baud
   # I offset
   # s data
   extents = [struct.pack("I", baud),struct.pack("I", offset), data]
   return _u2i(_pigpio_command_ext(
      _control, _PI_CMD_WVAS, user_gpio, len(data), extents))

def wave_tx_busy():
   """
   Checks to see if a waveform is currently being transmitted.

   Returns 1 if a waveform is currently being transmitted, otherwise 0.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVBSY, 0, 0))

def wave_tx_stop():
   """
   Stops the transmission of the current waveform.

   Returns 0 if OK.

   This function is intended to stop a waveform started with
   wave_tx_repeat().
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVHLT, 0, 0))

def wave_tx_start():
   """
   Transmits the current waveform.  The waveform is sent once.

   Returns the number of cbs in the waveform if OK,
   otherwise PI_BAD_WAVE_MODE.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVGO, 0, 0))

def wave_tx_repeat():
   """
   Transmits the current waveform.  The waveform repeats until
   wave_tx_stop is called.

   Returns the number of cbs in the waveform if OK,
   otherwise PI_BAD_WAVE_MODE.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVGOR, 0, 0))

def wave_get_micros():
   """
   Returns the length in microseconds of the current waveform.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVSM, 0, 0))

def wave_get_max_micros():
   """
   Returns the maximum possible size of a waveform in microseconds.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVSM, 2, 0))

def wave_get_pulses():
   """
   Returns the length in pulses of the current waveform.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVSP, 0, 0))

def wave_get_max_pulses():
   """
   Returns the maximum possible size of a waveform in pulses.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVSP, 2, 0))

def wave_get_cbs():
   """
   Returns the length in DMA control blocks of the current waveform.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVSC, 0, 0))

def wave_get_max_cbs():
   """
   Returns the maximum possible size of a waveform in DMA control blocks.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_WVSC, 2, 0))

def gpio_trigger(user_gpio, pulse_len=10, level=1):
   """
   Send a trigger pulse to a gpio.  The gpio is set to
   level for pulse_len microseconds and then reset to not level.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_LEVEL,
   PI_BAD_PULSELEN, or PI_NOT_PERMITTED.

   user_gpio: gpio to pulse.
   pulse_len: length of pulse in microseconds.
   level: whether the pulse should be high or low.

   Example

   #!/usr/bin/env python

   import time

   import pigpio

   GPIO=24

   pigpio.start()

   for i in range(10):
      pigpio.gpio_trigger(GPIO, (i*5)+10, 1)
      time.sleep(1)

   pigpio.stop()

   """
   # pigpio message format

   # I p1 user_gpio
   # I p2 pulse_len
   ## extension ##
   # I level

   extents = [struct.pack("I", level)]

   return _u2i(_pigpio_command_ext(
      _control, _PI_CMD_TRIG, user_gpio, pulse_len, extents))

def store_script(script):
   """
   Store a script for later execution.

   Returns a script id if OK, otherwise PI_BAD_SCRIPT.
   """
   # I p1 script length
   # I p2 0
   ## extension ##
   # s script

   return _u2i(_pigpio_command_ext(
      _control, _PI_CMD_PROC, len(script), 0, script))

def run_script(script_id):
   """
   Runs a stored script.

   Returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.

   script_id: script_id of stored script.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PROCR, script_id, 0))

def stop_script(script_id):
   """
   Stops a running script.

   Returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.

   script_id: script_id of stored script.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PROCS, script_id, 0))

def delete_script(script_id):
   """
   Deletes a stored script.

   Returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.

   script_id: script_id of stored script.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_PROCD, script_id, 0))

def serial_read_open(user_gpio, baud):
   """
   This function opens a gpio for reading serial data.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD,
   or PI_GPIO_IN_USE.

   The serial data is held in a cyclic buffer and is read using
   gpioSerialRead().

   It is the caller's responsibility to read data from the cyclic buffer
   in a timely fashion.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_SLRO, user_gpio, baud))

def serial_read(user_gpio):
   """
   This function returns data from the serial cyclic buffer.

   It returns a tuple of status and string.  The status will be the
   length, possibly 0, of the returned string if OK.  Otherwise a
   negative error code will be returned in which case the string
   will be null.
   """
   bytes = _u2i(_pigpio_command(_control, _PI_CMD_SLR, user_gpio, 10000))
   if bytes > 0:
      buf = ""
      while len(buf) < bytes: buf += _control.recv(bytes-len(buf))
      return bytes, buf
   return bytes, ""


def serial_read_close(user_gpio):
   """
   This function closes a gpio for reading serial data.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_SERIAL_GPIO.
   """
   return _u2i(_pigpio_command(_control, _PI_CMD_SLRC, user_gpio, 0))


class callback:
   """A class to provide gpio level change callbacks."""

   def __init__(self, user_gpio, edge=RISING_EDGE, func=None):
      """Initialise a callback and adds it to the notification thread.

      user_gpio: 0-31.
      edge:      EITHER_EDGE, RISING_EDGE (default), or FALLING_EDGE.
      func:      user supplied callback function.

      If a user callback is not specified a default tally callback is
      provided which simply counts edges.

      The user supplied callback receives three parameters, the gpio,
      the level, and the tick.

      Example 1: user supplied edge and callback

      #!/usr/bin/python

      import pigpio
      import time

      def cbf(g, L, t):
         s = "gpio=" + str(g) + " level=" + str(L) + " at " + str(t)
         print(s)

      pigpio.start()

      cb = pigpio.callback(22, pigpio.EITHER_EDGE, cbf)

      time.sleep(30)

      cb.cancel()

      pigpio.stop()

      will print lines such as

      gpio=22 level=1 at 548556842
      gpio=22 level=0 at 551316679
      gpio=22 level=1 at 553411795
      gpio=22 level=0 at 555269219
      gpio=22 level=1 at 557689701

      Example 2: user supplied edge, default (tally) callback

      #!/usr/bin/python

      import pigpio
      import time

      pigpio.start()

      pigpio.set_PWM_dutycycle(4, 0)

      pigpio.set_PWM_frequency(4, 2000)

      cb = pigpio.callback(4, pigpio.EITHER_EDGE)

      pigpio.set_PWM_dutycycle(4, 128) # half power

      tally_1 = cb.tally()

      time.sleep(50)

      tally_2 = cb.tally()

      s = "counted " + str(tally_2 - tally_1) + " edges"

      print(s)

      cb.cancel()

      pigpio.stop()

      will print a line such as

      counted 200200 edges

      Example 3: default edge and (tally) callback

      #!/usr/bin/python

      import pigpio
      import time

      pigpio.start()

      pigpio.set_PWM_dutycycle(17, 0)

      pigpio.set_PWM_frequency(17, 2000)

      cb = pigpio.callback(17)

      pigpio.set_PWM_dutycycle(17, 64) # quarter power

      tally_1 = cb.tally()

      time.sleep(50)

      tally_2 = cb.tally()

      s = "counted " + str(tally_2 - tally_1) + " rising edges"

      print(s)

      cb.cancel()

      pigpio.stop()

      will print a line such as

      counted 100101 rising edges

      """
      self.count=0
      if func is None:
         func=self._tally
      self.callb = _callback(user_gpio, edge, func)
      _notify.append(self.callb)

   def cancel(self):
      """Cancels a callback by removing it from the notification thread."""
      _notify.remove(self.callb)

   def _tally(self, user_gpio, level, tick):
      """Increment the callback called count.

      user_gpio:
      level:
      tick:
      """
      self.count += 1

   def tally(self):
      """Provides a count of how many times the default tally
      callback has triggered.

      The count will be zero if the user has supplied their own
      callback function.
      """
      return self.count


def wait_for_edge(user_gpio, edge=RISING_EDGE, timeout=60.0):
   """Wait for an edge event on a gpio.

   The function returns as soon as the edge is detected
   or after the number of seconds specified by timeout has
   expired.

   user_gpio: 0-31.
   edge:      EITHER_EDGE, RISING_EDGE (default), or FALLING_EDGE.
   timeout:   0.0- (default 60.0).

   The function returns True if the edge is detected,
   otherwise False.

   Example 1: default edge and timeout

   #!/usr/bin/python

   import pigpio
   import time

   pigpio.start()

   if pigpio.wait_for_edge(23):
      print("Rising edge detected")
   else:
      print("wait for edge timed out")

   pigpio.stop()

   will print

   Rising edge detected

   or

   wait for edge timed out

   Example 2: user supplied edge and timeout

   #!/usr/bin/python

   import pigpio
   import time

   pigpio.start()

   if pigpio.wait_for_edge(23, pigpio.FALLING_EDGE, 5.0):
      print("Falling edge detected")
   else:
      print("wait for falling edge timed out")

   pigpio.stop()


   will print

   Falling edge detected

   or

   wait for falling edge timed out

   """
   a = _wait_for_edge(user_gpio, edge, timeout)
   return a.trigger

def start(host = os.getenv("PIGPIO_ADDR", ''),
          port = os.getenv("PIGPIO_PORT", 8888)):
   """Start the pigpio module.

   host: the host name of the Pi on which the pigpio daemon is running.
         The default is localhost unless overwritten by the PIGPIO_ADDR
         environment variable.
   port: the port number on which the pigpio daemon is listening.
         The default is 8888 unless overwritten by the PIGPIO_PORT
         environment variable.  The pigpiod must have been started
         with the same port number.

   The function connects to the pigpio daemon and reserves resources
   to be used for sending commands and receiving notifications.

   EXAMPLES:
   ...
   pigpio.start() # use defaults

   pigpio.start('mypi') # specify host, default port

   pigpio.start('mypi', 7777) # specify host and port
   ...
   """

   global _control, _notify
   global _host, _port

   _host = host
   _port = int(port)

   _control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

   try:
      _control.connect((_host, _port))
      _notify = _callback_thread()
   except socket.error:
      if _control is not None:
         _control = None
      if _host == '':
         h = "localhost"
      else:
         h = _host
      errStr = "Can't connect to pigpio on " + str(h) + "(" + str(_port) + ")"
      print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
      print(errStr)
      print("")
      print("Did you start the pigpio daemon? E.g. sudo pigpiod")
      print("")
      print("Did you specify the correct Pi host/port in the environment")
      print("variables PIGPIO_ADDR/PIGPIO_PORT?")
      print("E.g. export PIGPIO_ADDR=soft, export PIGPIO_PORT=8888")
      print("")
      print("Did you specify the correct Pi host/port in the")
      print("pigpio.start() function? E.g. pigpio.start('soft', 8888))")
      print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
      raise

def stop():
   """Release pigpio resources.

   Example
   ...
   pigpio.stop()
   ...
   """
   global _control, _notify

   if _notify is not None:
      _notify.stop()
      _notify = None

   if _control is not None:
      _control.close()
      _control = None

atexit.register(stop)

