"""
pigpio is a Python module for the Raspberry which allows control
of the general purpose input outputs (gpios).

[http://abyz.co.uk/rpi/pigpio/python.html]

*Features*

o pigpio Python module can be running on Windows, Macs, or Linux

o controls one or more Pi's

o independent PWM on any of gpios 0-31 simultaneously

o independent servo pulses on any of gpios 0-31 simultaneously

o callbacks when any of gpios 0-31 change state

o creating and transmitting precisely timed waveforms

o reading/writing gpios and setting their modes

o wrappers for I2C, SPI, and serial links

o creating and running scripts on the pigpio daemon

*gpios*

ALL gpios are identified by their Broadcom number.

*Notes*

Transmitted waveforms are accurate to a microsecond.

Callback level changes are time-stamped and will be
accurate to within a few microseconds.

*Settings*

A number of settings are determined when the pigpio daemon is started.

o the sample rate (1, 2, 4, 5, 8, or 10us, default 5us).

o the set of gpios which may be updated (generally written to).  The
  default set is those available on the Pi board revision.

o the available PWM frequencies (see [*set_PWM_frequency*]).

*Exceptions*

By default a fatal exception is raised if you pass an invalid
argument to a pigpio function.

If you wish to handle the returned status yourself you should set
pigpio.exceptions to False.

You may prefer to check the returned status in only a few parts
of your code.  In that case do the following.

...
pigpio.exceptions = False

# Code where you want to test the error status.

pigpio.exceptions = True
...

*Usage*

This module uses the services of the C pigpio library.  pigpio
must be running on the Pi(s) whose gpios are to be manipulated.

The normal way to start pigpio is as a daemon (during system
start).

sudo pigpiod

Your Python program must import pigpio and create one or more
instances of the pigpio.pi class.  This class gives access to
a specified Pi's gpios.

...
pi1 = pigpio.pi()       # pi1 accesses the local Pi's gpios
pi2 = pigpio.pi('tom')  # pi2 accesses tom's gpios
pi3 = pigpio.pi('dick') # pi3 accesses dick's gpios

pi1.write(4, 0) # set local Pi's gpio 4 low
pi2.write(4, 1) # set tom's gpio 4 to high
pi3.read(4)     # get level of dick's gpio 4
...

The later example code snippets assume that pi is an instance of
the pigpio.pi class.

OVERVIEW

Essential

pigpio.pi                 Initialise Pi connection
stop                      Stop a Pi connection

Beginner

set_mode                  Set a gpio mode
get_mode                  Get a gpio mode
set_pull_up_down          Set/clear gpio pull up/down resistor

read                      Read a gpio
write                     Write a gpio

set_PWM_dutycycle         Start/stop PWM pulses on a gpio
set_servo_pulsewidth      Start/Stop servo pulses on a gpio

callback                  Create gpio level change callback
wait_for_edge             Wait for gpio level change

Intermediate

gpio_trigger              Send a trigger pulse to a gpio

set_watchdog              Set a watchdog on a gpio

set_PWM_range             Configure PWM range of a gpio
get_PWM_range             Get configured PWM range of a gpio

set_PWM_frequency         Set PWM frequency of a gpio
get_PWM_frequency         Get PWM frequency of a gpio

read_bank_1               Read all bank 1 gpios
read_bank_2               Read all bank 2 gpios

clear_bank_1              Clear selected gpios in bank 1
clear_bank_2              Clear selected gpios in bank 2

set_bank_1                Set selected gpios in bank 1
set_bank_2                Set selected gpios in bank 2

Advanced

get_PWM_real_range        Get underlying PWM range for a gpio

notify_open               Request a notification handle
notify_begin              Start notifications for selected gpios
notify_pause              Pause notifications
notify_close              Close a notification

bb_serial_read_open       Open a gpio for bit bang serial reads
bb_serial_read            Read bit bang serial data from  a gpio
bb_serial_read_close      Close a gpio for bit bang serial reads

Scripts

store_script              Store a script
run_script                Run a stored script
script_status             Get script status and parameters
stop_script               Stop a running script
delete_script             Delete a stored script

Waves

wave_clear                Deletes all waveforms

wave_add_new              Starts a new waveform
wave_add_generic          Adds a series of pulses to the waveform
wave_add_serial           Adds serial data to the waveform

wave_create               Creates a waveform from added data
wave_delete               Deletes one or more waveforms

wave_send_once            Transmits a waveform once
wave_send_repeat          Transmits a waveform repeatedly
wave_tx_busy              Checks to see if a waveform has ended
wave_tx_stop              Aborts the current waveform

wave_tx_repeat            Creates/transmits a waveform (DEPRECATED)
wave_tx_start             Creates/transmits a waveform (DEPRECATED)

wave_get_micros           Length in microseconds of the current waveform
wave_get_max_micros       Absolute maximum allowed micros
wave_get_pulses           Length in pulses of the current waveform
wave_get_max_pulses       Absolute maximum allowed pulses
wave_get_cbs              Length in cbs of the current waveform
wave_get_max_cbs          Absolute maximum allowed cbs

I2C

i2c_open                  Opens an I2C device
i2c_close                 Closes an I2C device

i2c_read_device           Reads the raw I2C device
i2c_write_device          Writes the raw I2C device

i2c_write_quick           smbus write quick
i2c_write_byte            smbus write byte
i2c_read_byte             smbus read byte
i2c_write_byte_data       smbus write byte data
i2c_write_word_data       smbus write word data
i2c_read_byte_data        smbus read byte data
i2c_read_word_data        smbus read word data
i2c_process_call          smbus process call
i2c_write_block_data      smbus write block data
i2c_read_block_data       smbus read block data
i2c_block_process_call    smbus block process call

i2c_read_i2c_block_data   smbus read I2C block data
i2c_write_i2c_block_data  smbus write I2C block data

SPI

spi_open                  Opens a SPI device
spi_close                 Closes a SPI device

spi_read                  Reads bytes from a SPI device
spi_write                 Writes bytes to a SPI device
spi_xfer                  Transfers bytes with a SPI device

Serial

serial_open               Opens a serial device (/dev/tty*)
serial_close              Closes a serial device

serial_read               Reads bytes from a serial device
serial_read_byte          Reads a byte from a serial device

serial_write              Writes bytes to a serial device
serial_write_byte         Writes a byte to a serial device

serial_data_available     Returns number of bytes ready to be read

Utility

get_current_tick          Get current tick (microseconds)

get_hardware_revision     Get hardware revision
get_pigpio_version        Get the pigpio version

pigpio.error_text         Gets error text from error number
pigpio.tickDiff           Returns difference between two ticks
"""
import sys
import socket
import struct
import time
import threading
import os
import atexit
import codecs

VERSION = "1.8"

exceptions = True

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

# script run status

PI_SCRIPT_INITING=0
PI_SCRIPT_HALTED =1
PI_SCRIPT_RUNNING=2
PI_SCRIPT_WAITING=3
PI_SCRIPT_FAILED =4

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

_PI_CMD_PROCP=45
_PI_CMD_MICRO=46
_PI_CMD_MILLI=47
_PI_CMD_PARSE=48

_PI_CMD_WVCRE=49
_PI_CMD_WVDEL=50
_PI_CMD_WVTX =51
_PI_CMD_WVTXR=52
_PI_CMD_WVNEW=53

_PI_CMD_I2CO =54
_PI_CMD_I2CC =55
_PI_CMD_I2CRD=56
_PI_CMD_I2CWD=57
_PI_CMD_I2CWQ=58
_PI_CMD_I2CRS=59
_PI_CMD_I2CWS=60
_PI_CMD_I2CRB=61
_PI_CMD_I2CWB=62
_PI_CMD_I2CRW=63
_PI_CMD_I2CWW=64
_PI_CMD_I2CRK=65
_PI_CMD_I2CWK=66
_PI_CMD_I2CRI=67
_PI_CMD_I2CWI=68
_PI_CMD_I2CPC=69
_PI_CMD_I2CPK=70

_PI_CMD_SPIO =71
_PI_CMD_SPIC =72
_PI_CMD_SPIR =73
_PI_CMD_SPIW =74
_PI_CMD_SPIX =75

_PI_CMD_SERO =76
_PI_CMD_SERC =77
_PI_CMD_SERRB=78
_PI_CMD_SERWB=79
_PI_CMD_SERR =80
_PI_CMD_SERW =81
_PI_CMD_SERDA=82


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
PI_NOT_SERIAL_GPIO  =-38
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
PI_BAD_SERIAL_COUNT =-51
PI_BAD_PARAM_NUM    =-52
PI_DUP_TAG          =-53
PI_TOO_MANY_TAGS    =-54
PI_BAD_SCRIPT_CMD   =-55
PI_BAD_VAR_NUM      =-56
PI_NO_SCRIPT_ROOM   =-57
PI_NO_MEMORY        =-58
PI_SOCK_READ_FAILED =-59
PI_SOCK_WRIT_FAILED =-60
PI_TOO_MANY_PARAM   =-61
PI_NOT_HALTED       =-62
PI_BAD_TAG          =-63
PI_BAD_MICS_DELAY   =-64
PI_BAD_MILS_DELAY   =-65
PI_BAD_WAVE_ID      =-66
PI_TOO_MANY_CBS     =-67
PI_TOO_MANY_OOL     =-68
PI_EMPTY_WAVEFORM   =-69
PI_NO_WAVEFORM_ID   =-70
PI_I2C_OPEN_FAILED  =-71
PI_SER_OPEN_FAILED  =-72
PI_SPI_OPEN_FAILED  =-73
PI_BAD_I2C_BUS      =-74
PI_BAD_I2C_ADDR     =-75
PI_BAD_SPI_CHANNEL  =-76
PI_BAD_FLAGS        =-77
PI_BAD_SPI_SPEED    =-78
PI_BAD_SER_DEVICE   =-79
PI_BAD_SER_SPEED    =-80
PI_BAD_PARAM        =-81
PI_I2C_WRITE_FAILED =-82
PI_I2C_READ_FAILED  =-83
PI_BAD_SPI_COUNT    =-84
PI_SER_WRITE_FAILED =-85
PI_SER_READ_FAILED  =-86
PI_SER_READ_NO_DATA =-87
PI_UNKNOWN_COMMAND  =-88
PI_SPI_XFER_FAILED  =-89

# pigpio error text

_errors=[
   [_PI_INIT_FAILED      , "pigpio initialisation failed"],
   [PI_BAD_USER_GPIO     , "gpio not 0-31"],
   [PI_BAD_GPIO          , "gpio not 0-53"],
   [PI_BAD_MODE          , "mode not 0-7"],
   [PI_BAD_LEVEL         , "level not 0-1"],
   [PI_BAD_PUD           , "pud not 0-2"],
   [PI_BAD_PULSEWIDTH    , "pulsewidth not 0 or 500-2500"],
   [PI_BAD_DUTYCYCLE     , "dutycycle not 0-range (default 255)"],
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
   [PI_BAD_HANDLE        , "unknown handle"],
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
   [PI_BAD_SERIAL_COUNT  , "must read at least a byte at a time"],
   [PI_BAD_PARAM_NUM     , "script parameter must be 0-9"],
   [PI_DUP_TAG           , "script has duplicate tag"],
   [PI_TOO_MANY_TAGS     , "script has too many tags"],
   [PI_BAD_SCRIPT_CMD    , "illegal script command"],
   [PI_BAD_VAR_NUM       , "script variable must be 0-149"],
   [PI_NO_SCRIPT_ROOM    , "no more room for scripts"],
   [PI_NO_MEMORY         , "can't allocate temporary memory"],
   [PI_SOCK_READ_FAILED  , "socket read failed"],
   [PI_SOCK_WRIT_FAILED  , "socket write failed"],
   [PI_TOO_MANY_PARAM    , "too many script parameters (> 10)"],
   [PI_NOT_HALTED        , "script already running or failed"],
   [PI_BAD_TAG           , "script has unresolved tag"],
   [PI_BAD_MICS_DELAY    , "bad MICS delay (too large)"],
   [PI_BAD_MILS_DELAY    , "bad MILS delay (too large)"],
   [PI_BAD_WAVE_ID       , "non existent wave id"],
   [PI_TOO_MANY_CBS      , "No more CBs for waveform"],
   [PI_TOO_MANY_OOL      , "No more OOL for waveform"],
   [PI_EMPTY_WAVEFORM    , "attempt to create an empty waveform"],
   [PI_NO_WAVEFORM_ID    , "No more waveform ids"],
   [PI_I2C_OPEN_FAILED   , "can't open I2C device"],
   [PI_SER_OPEN_FAILED   , "can't open serial device"],
   [PI_SPI_OPEN_FAILED   , "can't open SPI device"],
   [PI_BAD_I2C_BUS       , "bad I2C bus"],
   [PI_BAD_I2C_ADDR      , "bad I2C address"],
   [PI_BAD_SPI_CHANNEL   , "bad SPI channel"],
   [PI_BAD_FLAGS         , "bad i2c/spi/ser open flags"],
   [PI_BAD_SPI_SPEED     , "bad SPI speed"],
   [PI_BAD_SER_DEVICE    , "bad serial device name"],
   [PI_BAD_SER_SPEED     , "bad serial baud rate"],
   [PI_BAD_PARAM         , "bad i2c/spi/ser parameter"],
   [PI_I2C_WRITE_FAILED  , "I2C write failed"],
   [PI_I2C_READ_FAILED   , "I2C read failed"],
   [PI_BAD_SPI_COUNT     , "bad SPI count"],
   [PI_SER_WRITE_FAILED  , "ser write failed"],
   [PI_SER_READ_FAILED   , "ser read failed"],
   [PI_SER_READ_NO_DATA  , "ser read no data available"],
   [PI_UNKNOWN_COMMAND   , "unknown command"],
   [PI_SPI_XFER_FAILED   , "SPI xfer/read/write failed"],
]

class error(Exception):
   """pigpio module exception"""
   def __init__(self, value):
      self.value = value
   def __str__(self):
      return repr(self.value)

class pulse:
   """
   A class to store pulse information.
   """

   def __init__(self, gpio_on, gpio_off, delay):
      """
      Initialises a pulse.

       gpio_on:= the gpios to switch on at the start of the pulse.
      gpio_off:= the gpios to switch off at the start of the pulse.
         delay:= the delay in microseconds before the next pulse.

      """
      self.gpio_on = gpio_on
      self.gpio_off = gpio_off
      self.delay = delay

def error_text(errnum):
   """
   Returns a text description of a pigpio error.

   errnum:= <0, the error number

   ...
   print(pigpio.error_text(-5))
   level not 0-1
   ...
   """
   for e in _errors:
      if e[0] == errnum:
         return e[1]
   return "unknown error ({})".format(errnum)

def tickDiff(t1, t2):
   """
   Returns the microsecond difference between two ticks.

   t1:= the earlier tick
   t2:= the later tick

   ...
   print(pigpio.tickDiff(4294967272, 12))
   36
   ...
   """
   tDiff = t2 - t1
   if tDiff < 0:
      tDiff += (1 << 32)
   return tDiff

if sys.version < '3':
   def _b(x):
      return x
else:
   def _b(x):
      return codecs.latin_1_encode(x)[0]

def _u2i(number):
   """Converts a 32 bit unsigned number to signed."""
   mask = (2 ** 32) - 1
   if number & (1 << 31):
      v = number | ~mask
   else:
      v = number & mask
   if v >= 0:
      return v;
   else:
      if exceptions:
         raise error(error_text(v))
      else:
         return v

def _pigpio_command(sock, cmd, p1, p2):
   """
   Runs a pigpio socket command.

   sock:= command socket.
    cmd:= the command to be executed.
     p1:= command parameter 1 (if applicable).
     p2:=  command parameter 2 (if applicable).
   """
   sock.send(struct.pack('IIII', cmd, p1, p2, 0))
   dummy, res = struct.unpack('12sI', sock.recv(16))
   return res

def _pigpio_command_ext(sock, cmd, p1, p2, p3, extents):
   """
   Runs an extended pigpio socket command.

      sock:= command socket.
       cmd:= the command to be executed.
        p1:= command parameter 1 (if applicable).
        p2:= command parameter 2 (if applicable).
        p3:= total size in bytes of following extents
   extents:= additional data blocks
   """
   ext = bytearray(struct.pack('IIII', cmd, p1, p2, p3))
   for x in extents:
      if type(x) == type(""):
         ext.extend(_b(x))
      else:
         ext.extend(x)
   sock.sendall(ext)
   dummy, res = struct.unpack('12sI', sock.recv(16))
   return res

class _callback_ADT:
   """An ADT class to hold callback information."""

   def __init__(self, gpio, edge, func):
      """
      Initialises a callback ADT.

      gpio:= Broadcom gpio number.
      edge:= EITHER_EDGE, RISING_EDGE, or FALLING_EDGE.
      func:= a user function taking three arguments (gpio, level, tick).
      """
      self.gpio = gpio
      self.edge = edge
      self.func = func
      self.bit = 1<<gpio

class _callback_thread(threading.Thread):
   """A class to encapsulate pigpio notification callbacks."""
   def __init__(self, control, host, port):
      """Initialises notifications."""
      threading.Thread.__init__(self)
      self.control = control
      self.go = False
      self.daemon = True
      self.monitor = 0
      self.callbacks = []
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((host, port))
      self.handle = _pigpio_command(self.sock, _PI_CMD_NOIB, 0, 0)
      self.go = True
      self.start()

   def stop(self):
      """Stops notifications."""
      if self.go:
         self.go = False
         self.sock.send(struct.pack('IIII', _PI_CMD_NC, self.handle, 0, 0))

   def append(self, callb):
      """Adds a callback to the notification thread."""
      self.callbacks.append(callb)
      self.monitor = self.monitor | callb.bit
      _pigpio_command(self.control, _PI_CMD_NB, self.handle, self.monitor)

   def remove(self, callb):
      """Removes a callback from the notification thread."""
      if callb in self.callbacks:
         self.callbacks.remove(callb)
         newMonitor = 0
         for c in self.callbacks:
            newMonitor |= c.bit
         if newMonitor != self.monitor:
            self.monitor = newMonitor
            _pigpio_command(
               self.control, _PI_CMD_NB, self.handle, self.monitor)

   def run(self):
      """Runs the notification thread."""

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

class _callback:
   """A class to provide gpio level change callbacks."""

   def __init__(self, notify, user_gpio, edge=RISING_EDGE, func=None):
      """
      Initialise a callback and adds it to the notification thread.
      """
      self._notify = notify
      self.count=0
      if func is None:
         func=self._tally
      self.callb = _callback_ADT(user_gpio, edge, func)
      self._notify.append(self.callb)

   def cancel(self):
      """Cancels a callback by removing it from the notification thread."""
      self._notify.remove(self.callb)

   def _tally(self, user_gpio, level, tick):
      """Increment the callback called count."""
      self.count += 1

   def tally(self):
      """
      Provides a count of how many times the default tally
      callback has triggered.

      The count will be zero if the user has supplied their own
      callback function.
      """
      return self.count

class _wait_for_edge:
   """Encapsulates waiting for gpio edges."""

   def __init__(self, notify, gpio, edge, timeout):
      """Initialises a wait_for_edge."""
      self._notify = notify
      self.callb = _callback_ADT(gpio, edge, self.func)
      self.trigger = False
      self._notify.append(self.callb)
      self.start = time.time()
      while (self.trigger == False) and ((time.time()-self.start) < timeout):
         time.sleep(0.05)
      self._notify.remove(self.callb)

   def func(self, gpio, level, tick):
      """Sets wait_for_edge triggered."""
      self.trigger = True

class pi():

   def _rxbuf(self, count):
      """Returns count bytes from the command socket."""
      ext = bytearray(self._control.recv(count))
      while len(ext) < count:
         ext.extend(self._control.recv(count - len(ext)))
      return ext

   def set_mode(self, gpio, mode):
      """
      Sets the gpio mode.

      gpio:= 0-53.
      mode:= INPUT, OUTPUT, ALT0, ALT1, ALT2, ALT3, ALT4, ALT5.

      ...
      pi.set_mode( 4, pigpio.INPUT)  # gpio  4 as input
      pi.set_mode(17, pigpio.OUTPUT) # gpio 17 as output
      pi.set_mode(24, pigpio.ALT2)   # gpio 24 as ALT2
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_MODES, gpio, mode))

   def get_mode(self, gpio):
      """
      Returns the gpio mode.

      gpio:= 0-53.

      Returns a value as follows

      . .
      0 = INPUT
      1 = OUTPUT
      2 = ALT5
      3 = ALT4
      4 = ALT0
      5 = ALT1
      6 = ALT2
      7 = ALT3
      . .

      ...
      print(pi.get_mode(0))
      4
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_MODEG, gpio, 0))

   def set_pull_up_down(self, gpio, pud):
      """
      Sets or clears the internal gpio pull-up/down resistor.

      gpio:= 0-53.
       pud:= PUD_UP, PUD_DOWN, PUD_OFF.

      ...
      pi.set_pull_up_down(17, pigpio.PUD_OFF)
      pi.set_pull_up_down(23, pigpio.PUD_UP)
      pi.set_pull_up_down(24, pigpio.PUD_DOWN)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PUD, gpio, pud))

   def read(self, gpio):
      """
      Returns the gpio level.

      gpio:= 0-53.

      ...
      pi.set_mode(23, pigpio.INPUT)

      pi.set_pull_up_down(23, pigpio.PUD_DOWN)
      print(pi.read(23))
      0

      pi.set_pull_up_down(23, pigpio.PUD_UP)
      print(pi.read(23))
      1
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_READ, gpio, 0))

   def write(self, gpio, level):
      """
      Sets the gpio level.

       gpio:= 0-53.
      level:= 0, 1.

      If PWM or servo pulses are active on the gpio they are
      switched off.

      ...
      pi.set_mode(17, pigpio.OUTPUT)

      pi.write(17,0)
      print(pi.read(17))
      0

      pi.write(17,1)
      print(pi.read(17))
      1
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WRITE, gpio, level))

   def set_PWM_dutycycle(self, user_gpio, dutycycle):
      """
      Starts (non-zero dutycycle) or stops (0) PWM pulses on the gpio.

      user_gpio:= 0-31.
      dutycycle:= 0-range (range defaults to 255).

      The [*set_PWM_range*] function can change the default range of 255.

      ...
      pi.set_PWM_dutycycle(4,   0) # PWM off
      pi.set_PWM_dutycycle(4,  64) # PWM 1/4 on
      pi.set_PWM_dutycycle(4, 128) # PWM 1/2 on
      pi.set_PWM_dutycycle(4, 192) # PWM 3/4 on
      pi.set_PWM_dutycycle(4, 255) # PWM full on
      ...
      """
      return _u2i(_pigpio_command(
         self._control, _PI_CMD_PWM, user_gpio, int(dutycycle)))

   def set_PWM_range(self, user_gpio, range_):
      """
      Sets the range of PWM values to be used on the gpio.

      user_gpio:= 0-31.
         range_:= 25-40000.

      ...
      pi.set_PWM_range(9, 100)  # now  25 1/4,   50 1/2,   75 3/4 on
      pi.set_PWM_range(9, 500)  # now 125 1/4,  250 1/2,  375 3/4 on
      pi.set_PWM_range(9, 3000) # now 750 1/4, 1500 1/2, 2250 3/4 on
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PRS, user_gpio, range_))

   def get_PWM_range(self, user_gpio):
      """
      Returns the range of PWM values being used on the gpio.

      user_gpio:= 0-31.

      ...
      pi.set_PWM_range(9, 500)
      print(pi.get_PWM_range(9))
      500
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PRG, user_gpio, 0))

   def get_PWM_real_range(self, user_gpio):
      """
      Returns the real (underlying) range of PWM values being
      used on the gpio.

      user_gpio:= 0-31.

      ...
      pi.set_PWM_frequency(4, 800)
      print(pi.get_PWM_real_range(4))
      250
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PRRG, user_gpio, 0))

   def set_PWM_frequency(self, user_gpio, frequency):
      """
      Sets the frequency (in Hz) of the PWM to be used on the gpio.

      user_gpio:= 0-31.
      frequency:= >=0 Hz

      Returns the frequency actually set.

      ...
      pi.set_PWM_frequency(4,0)
      print(pi.get_PWM_frequency(4))
      10

      pi.set_PWM_frequency(4,100000)
      print(pi.get_PWM_frequency(4))
      8000
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PFS, user_gpio, frequency))

   def get_PWM_frequency(self, user_gpio):
      """
      Returns the frequency of PWM being used on the gpio.

      user_gpio:= 0-31.

      Returns the frequency (in Hz) used for the gpio.

      ...
      pi.set_PWM_frequency(4,0)
      print(pi.get_PWM_frequency(4))
      10

      pi.set_PWM_frequency(4, 800)
      print(pi.get_PWM_frequency(4))
      800
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PFG, user_gpio, 0))

   def set_servo_pulsewidth(self, user_gpio, pulsewidth):
      """
      Starts (500-2500) or stops (0) servo pulses on the gpio.

       user_gpio:= 0-31.
      pulsewidth:= 0 (off),
                   500 (most anti-clockwise) - 2500 (most clockwise).

      The selected pulsewidth will continue to be transmitted until
      changed by a subsequent call to set_servo_pulsewidth.

      The pulsewidths supported by servos varies and should probably
      be determined by experiment. A value of 1500 should always be
      safe and represents the mid-point of rotation.

      You can DAMAGE a servo if you command it to move beyond its
      limits.

      ...
      pi.set_servo_pulsewidth(17, 0)    # off
      pi.set_servo_pulsewidth(17, 1000) # safe anti-clockwise
      pi.set_servo_pulsewidth(17, 1500) # centre
      pi.set_servo_pulsewidth(17, 2000) # safe clockwise
      ...
   """
      return _u2i(_pigpio_command(
         self._control, _PI_CMD_SERVO, user_gpio, int(pulsewidth)))

   def notify_open(self):
      """
      Returns a notification handle (>=0).

      A notification is a method for being notified of gpio state
      changes via a pipe.

      Pipes are only accessible from the local machine so this
      function serves no purpose if you are using Python from a
      remote machine.  The in-built (socket) notifications
      provided by [*callback*] should be used instead.

      Notifications for handle x will be available at the pipe
      named /dev/pigpiox (where x is the handle number).

      E.g. if the function returns 15 then the notifications must be
      read from /dev/pigpio15.

      Notifications have the following structure.

      . .
      I seqno - increments for each report
      I flags - flags, if bit 5 is set then bits 0-4 of the flags
                indicate a gpio which has had a watchdog timeout.
      I tick  - time of sample.
      I level - 32 bits of levels for gpios 0-31.
      . .

      ...
      h = pi.notify_open()
      if h >= 0:
         pi.notify_begin(h, 1234)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_NO, 0, 0))

   def notify_begin(self, handle, bits):
      """
      Starts notifications on a handle.

      handle:= >=0 (as returned by a prior call to [*notify_open*])
        bits:= a 32 bit mask indicating the gpios to be notified.

      The notification sends state changes for each gpio whose
      corresponding bit in bits is set.

      The following code starts notifications for gpios 1, 4,
      6, 7, and 10 (1234 = 0x04D2 = 0b0000010011010010).

      ...
      h = pi.notify_open()
      if h >= 0:
         pi.notify_begin(h, 1234)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_NB, handle, bits))

   def notify_pause(self, handle):
      """
      Pauses notifications on a handle.

      handle:= >=0 (as returned by a prior call to [*notify_open*])

      Notifications for the handle are suspended until
      [*notify_begin*] is called again.

      ...
      h = pi.notify_open()
      if h >= 0:
         pi.notify_begin(h, 1234)
         ...
         pi.notify_pause(h)
         ...
         pi.notify_begin(h, 1234)
         ...
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_NB, handle, 0))

   def notify_close(self, handle):
      """
      Stops notifications on a handle and releases the handle for reuse.

      handle:= >=0 (as returned by a prior call to [*notify_open*])

      ...
      h = pi.notify_open()
      if h >= 0:
         pi.notify_begin(h, 1234)
         ...
         pi.notify_close(h)
         ...
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_NC, handle, 0))

   def set_watchdog(self, user_gpio, wdog_timeout):
      """
      Sets a watchdog timeout for a gpio.

         user_gpio:= 0-31.
      wdog_timeout:= 0-60000.

      The watchdog is nominally in milliseconds.

      Only one watchdog may be registered per gpio.

      The watchdog may be cancelled by setting timeout to 0.

      If no level change has been detected for the gpio for timeout
      milliseconds any notification for the gpio has a report written
      to the fifo with the flags set to indicate a watchdog timeout.

      The callback class interprets the flags and will
      call registered callbacks for the gpio with level TIMEOUT.

      ...
      pi.set_watchdog(23, 1000) # 1000 ms watchdog on gpio 23
      pi.set_watchdog(23, 0)    # cancel watchdog on gpio 23
      ...
      """
      return _u2i(_pigpio_command(
         self._control, _PI_CMD_WDOG, user_gpio, int(wdog_timeout)))

   def read_bank_1(self):
      """
      Returns the levels of the bank 1 gpios (gpios 0-31).

      The returned 32 bit integer has a bit set if the corresponding
      gpio is high.  Gpio n has bit value (1<<n).

      ...
      print(bin(pi.read_bank_1()))
      0b10010100000011100100001001111
      ...
      """
      return _pigpio_command(self._control, _PI_CMD_BR1, 0, 0)

   def read_bank_2(self):
      """
      Returns the levels of the bank 2 gpios (gpios 32-53).

      The returned 32 bit integer has a bit set if the corresponding
      gpio is high.  Gpio n has bit value (1<<(n-32)).

      ...
      print(bin(pi.read_bank_2()))
      0b1111110000000000000000
      ...
      """
      return _pigpio_command(self._control, _PI_CMD_BR2, 0, 0)

   def clear_bank_1(self, bits):
      """
      Clears gpios 0-31 if the corresponding bit in bits is set.

      bits:= a 32 bit mask with 1 set if the corresponding gpio is
             to be cleared.

      A returned status of PI_SOME_PERMITTED indicates that the user
      is not allowed to write to one or more of the gpios.

      ...
      pi.clear_bank_1(int("111110010000",2))
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_BC1, bits, 0))

   def clear_bank_2(self, bits):
      """
      Clears gpios 32-53 if the corresponding bit (0-21) in bits is set.

      bits:= a 32 bit mask with 1 set if the corresponding gpio is
             to be cleared.

      A returned status of PI_SOME_PERMITTED indicates that the user
      is not allowed to write to one or more of the gpios.

      ...
      pi.clear_bank_2(0x1010)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_BC2, bits, 0))

   def set_bank_1(self, bits):
      """
      Sets gpios 0-31 if the corresponding bit in bits is set.

      bits:= a 32 bit mask with 1 set if the corresponding gpio is
             to be set.

      A returned status of PI_SOME_PERMITTED indicates that the user
      is not allowed to write to one or more of the gpios.

      ...
      pi.set_bank_1(int("111110010000",2))
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_BS1, bits, 0))

   def set_bank_2(self, bits):
      """
      Sets gpios 32-53 if the corresponding bit (0-21) in bits is set.

      bits:= a 32 bit mask with 1 set if the corresponding gpio is
             to be set.

      A returned status of PI_SOME_PERMITTED indicates that the user
      is not allowed to write to one or more of the gpios.

      ...
      pi.set_bank_2(0x303)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_BS2, bits, 0))

   def get_current_tick(self):
      """
      Returns the current system tick.

      Tick is the number of microseconds since system boot.  As an
      unsigned 32 bit quantity tick wraps around approximately
      every 71.6 minutes.

      ...
      t1 = pi.get_current_tick()
      time.sleep(1)
      t2 = pi.get_current_tick()
      ...
      """
      return _pigpio_command(self._control, _PI_CMD_TICK, 0, 0)

   def get_hardware_revision(self):
      """
      Returns the Pi's hardware revision number.

      The hardware revision is the last 4 characters on the Revision
      line of /proc/cpuinfo.

      The revision number can be used to determine the assignment
      of gpios to pins.

      There are at least three types of board.

      Type 1 has gpio 0 on P1-3, gpio 1 on P1-5, and gpio 21 on P1-13
      (revision numbers 2 and 3).

      Type 2 has gpio 2 on P1-3, gpio 3 on P1-5, gpio 27 on P1-13,
      and gpios 28-31 on P5 (revision numbers of 4, 5, 6, and 15).

      Type 3 has a 40 pin connector rather than the 26 pin connector
      of the earlier boards.  Gpios 0 to 27 are brought out to the
      connector (revision number 16).

      If the hardware revision can not be found or is not a valid
      hexadecimal number the function returns 0.

      ...
      print(pi.get_hardware_revision())
      2
      ...
      """
      return _pigpio_command(self._control, _PI_CMD_HWVER, 0, 0)

   def get_pigpio_version(self):
      """
      Returns the pigpio software version.

      ...
      v = pi.get_pigpio_version()
      ...
      """
      return _pigpio_command(self._control, _PI_CMD_PIGPV, 0, 0)

   def wave_clear(self):
      """
      Clears all waveforms and any data added by calls to the
      [*wave_add_**] functions.

      ...
      pi.wave_clear()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVCLR, 0, 0))

   def wave_add_new(self):
      """
      Starts a new empty waveform.

      You would not normally need to call this function as it is
      automatically called after a waveform is created with the
      [*wave_create*] function.

      ...
      pi.wave_add_new()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVNEW, 0, 0))

   def wave_add_generic(self, pulses):
      """
      Adds a list of pulses to the current waveform.

      pulses:= list of pulses to add to the waveform.

      Returns the new total number of pulses in the current waveform.

      The pulses are interleaved in time order within the existing
      waveform (if any).

      Merging allows the waveform to be built in parts, that is the
      settings for gpio#1 can be added, and then gpio#2 etc.

      If the added waveform is intended to start after or within
      the existing waveform then the first pulse should consist
      solely of a delay.

      ...
      G1=4
      G2=24

      pi.set_mode(G1, pigpio.OUTPUT)
      pi.set_mode(G2, pigpio.OUTPUT)

      flash_500=[] # flash every 500 ms
      flash_100=[] # flash every 100 ms

      #                              ON     OFF  DELAY

      flash_500.append(pigpio.pulse(1<<G1, 1<<G2, 500000))
      flash_500.append(pigpio.pulse(1<<G2, 1<<G1, 500000))

      flash_100.append(pigpio.pulse(1<<G1, 1<<G2, 100000))
      flash_100.append(pigpio.pulse(1<<G2, 1<<G1, 100000))

      pi.wave_clear() # clear any existing waveforms

      pi.wave_add_generic(flash_500) # 500 ms flashes
      f500 = pi.wave_create() # create and save id

      pi.wave_add_generic(flash_100) # 100 ms flashes
      f100 = pi.wave_create() # create and save id

      pi.wave_send_repeat(f500)

      time.sleep(4)

      pi.wave_send_repeat(f100)

      time.sleep(4)

      pi.wave_send_repeat(f500)

      time.sleep(4)

      pi.wave_tx_stop() # stop waveform

      pi.wave_clear() # clear all waveforms
      ...
      """
      # pigpio message format

      # I p1 0
      # I p2 0
      # I p3 pulses * 12
      ## extension ##
      # III on/off/delay * pulses
      if len(pulses):
         ext = bytearray()
         for p in pulses:
            ext.extend(struct.pack("III", p.gpio_on, p.gpio_off, p.delay))
         extents = [ext]
         return _u2i(_pigpio_command_ext(
            self._control, _PI_CMD_WVAG, 0, 0, len(pulses)*12, extents))
      else:
         return 0

   def wave_add_serial(self, user_gpio, bb_baud, offset, data):
      """
      Adds a waveform representing serial data to the existing
      waveform (if any).  The serial data starts offset
      microseconds from the start of the waveform.

      user_gpio:= gpio to transmit data.  You must set the gpio mode
                  to output.
        bb_baud:= baud rate to use.
         offset:= number of microseconds from the starts of the
                  waveform.
           data:= the bytes to write.

      Returns the new total number of pulses in the current waveform.

      The serial data is formatted as one start bit, eight data bits,
      and one stop bit.

      It is legal to add serial data streams with different baud
      rates to the same waveform.

      ...
      pi.wave_add_serial(4, 300, 0, 'Hello world')

      pi.wave_add_serial(4, 300, 0, b"Hello world")

      pi.wave_add_serial(4, 300, 0, b'\\x23\\x01\\x00\\x45')

      pi.wave_add_serial(17, 38400, 5000, [23, 128, 234])
      ...
      """
      # pigpio message format

      # I p1 gpio
      # I p2 bb_baud
      # I p3 len+4
      ## extension ##
      # I offset
      # s len data bytes
      if len(data):
         extents = [struct.pack("I", offset), data]
         return _u2i(_pigpio_command_ext(
            self._control, _PI_CMD_WVAS, user_gpio, bb_baud, len(data)+4, extents))
      else:
         return 0

   def wave_create(self):
      """
      Creates a waveform from the data provided by the prior calls
      to the [*wave_add_**] functions.

      Returns a wave id (>=0) if OK.

      The data provided by the [*wave_add_**] functions is consumed by
      this function.

      As many waveforms may be created as there is space available.
      The wave id is passed to [*wave_send_**] to specify the waveform
      to transmit.

      Normal usage would be

      Step 1. [*wave_clear*] to clear all waveforms and added data.

      Step 2. [*wave_add_**] calls to supply the waveform data.

      Step 3. [*wave_create*] to create the waveform and get a unique id

      Repeat steps 2 and 3 as needed.

      Step 4. [*wave_send_**] with the id of the waveform to transmit.

      A waveform comprises one or more pulses.

      A pulse specifies

      1) the gpios to be switched on at the start of the pulse. 
      2) the gpios to be switched off at the start of the pulse. 
      3) the delay in microseconds before the next pulse.

      Any or all the fields can be zero.  It doesn't make any sense
      to set all the fields to zero (the pulse will be ignored).

      When a waveform is started each pulse is executed in order with
      the specified delay between the pulse and the next.

      ...
      wid = pi.wave_create()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVCRE, 0, 0))

   def wave_delete(self, wave_id):
      """
      Deletes all created waveforms with ids greater than or equal
      to wave_id.

      wave_id:= >=0 (as returned by a prior call to [*wave_create*]).

      Wave ids are allocated in order, 0, 1, 2, etc.

      ...
      pi.wave_delete(6) # delete all waves with id 6 or greater

      pi.wave_delete(0) # delete all waves
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVDEL, wave_id, 0))

   def wave_tx_start(self): # DEPRECATED
      """
      This function is deprecated and will be removed.

      Use [*wave_create*]/[*wave_send_**] instead.
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVGO, 0, 0))

   def wave_tx_repeat(self): # DEPRECATED
      """
      This function is deprecated and will be removed.

      Use [*wave_create*]/[*wave_send_**] instead.
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVGOR, 0, 0))

   def wave_send_once(self, wave_id):
      """
      Transmits the waveform with id wave_id.  The waveform is sent
      once.

      wave_id:= >=0 (as returned by a prior call to [*wave_create*]).

      Returns the number of DMA control blocks used in the waveform.

      ...
      cbs = pi.wave_send_once(wid)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVTX, wave_id, 0))

   def wave_send_repeat(self, wave_id):
      """
      Transmits the waveform with id wave_id.  The waveform repeats
      until wave_tx_stop is called or another call to [*wave_send_**]
      is made.

      wave_id:= >=0 (as returned by a prior call to [*wave_create*]).

      Returns the number of DMA control blocks used in the waveform.

      ...
      cbs = pi.wave_send_repeat(wid)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVTXR, wave_id, 0))

   def wave_tx_busy(self):
      """
      Returns 1 if a waveform is currently being transmitted,
      otherwise 0.

      ...
      pi.wave_send_once(0) # send first waveform

      while pi.wave_tx_busy(): # wait for waveform to be sent
         time.sleep(0.1)

      pi.wave_send_once(1) # send next waveform
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVBSY, 0, 0))

   def wave_tx_stop(self):
      """
      Stops the transmission of the current waveform.

      This function is intended to stop a waveform started with
      wave_send_repeat.

      ...
      pi.wave_send_repeat(3)

      time.sleep(5)

      pi.wave_tx_stop()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVHLT, 0, 0))

   def wave_get_micros(self):
      """
      Returns the length in microseconds of the current waveform.

      ...
      micros = pi.wave_get_micros()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVSM, 0, 0))

   def wave_get_max_micros(self):
      """
      Returns the maximum possible size of a waveform in microseconds.

      ...
      micros = pi.wave_get_max_micros()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVSM, 2, 0))

   def wave_get_pulses(self):
      """
      Returns the length in pulses of the current waveform.

      ...
      pulses = pi.wave_get_pulses()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVSP, 0, 0))

   def wave_get_max_pulses(self):
      """
      Returns the maximum possible size of a waveform in pulses.

      ...
      pulses = pi.wave_get_max_pulses()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVSP, 2, 0))

   def wave_get_cbs(self):
      """
      Returns the length in DMA control blocks of the current
      waveform.

      ...
      cbs = pi.wave_get_cbs()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVSC, 0, 0))

   def wave_get_max_cbs(self):
      """
      Returns the maximum possible size of a waveform in DMA
      control blocks.

      ...
      cbs = pi.wave_get_max_cbs()
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_WVSC, 2, 0))

   def i2c_open(self, i2c_bus, i2c_address, i2c_flags=0):
      """
      Returns a handle (>=0) for the device at the I2C bus address.

          i2c_bus:= 0-1.
      i2c_address:= 0x08-0x77.
        i2c_flags:= 0, no flags are currently defined.

      Normally you would only use the [*i2c_**] functions if
      you are or will be connecting to the Pi over a network.  If
      you will always run on the local Pi use the standard smbus
      modules instead.

      ...
      h = pi.i2c_open(1, 0x53) # open device at address 0x53 on bus 1
      ...
      """
      # I p1 i2c_bus
      # I p2 i2c_addr
      # I p3 4
      ## extension ##
      # I i2c_flags
      extents = [struct.pack("I", i2c_flags)]
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_I2CO, i2c_bus, i2c_address, 4, extents))

   def i2c_close(self, handle):
      """
      Closes the I2C device associated with handle.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).

      ...
      pi.i2c_close(h)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_I2CC, handle, 0))

   def i2c_read_device(self, handle, count):
      """
      Returns count bytes read from the raw device associated
      with handle.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
       count:= >0, the number of bytes to read.

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (count, data) = pi.i2c_read_device(h, 12)
      ...
      """
      bytes = _u2i(_pigpio_command(self._control, _PI_CMD_I2CRD, handle, count))
      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   def i2c_write_device(self, handle, data):
      """
      Writes the data bytes to the raw device associated with handle.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
        data:= the bytes to write.

      ...
      pi.i2c_write_device(h, b"\\x12\\x34\\xA8")

      pi.i2c_write_device(h, b"help")

      pi.i2c_write_device(h, 'help')

      pi.i2c_write_device(h, [23, 56, 231])
      ...
      """
      # I p1 handle
      # I p2 0
      # I p3 len
      ## extension ##
      # s len data bytes
      if len(data):
         return _u2i(_pigpio_command_ext(
            self._control, _PI_CMD_I2CWD, handle, 0, len(data), [data]))
      else:
         return 0

   def i2c_write_quick(self, handle, bit):
      """
      Sends a single bit to the device associated with handle.

      smbus 2.0 5.5.1 - Quick command.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         bit:= 0 or 1, the value to write.

      ...
      pi.i2c_write_quick(0, 1) # send 1 to device 0
      pi.i2c_write_quick(3, 0) # send 0 to device 3
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_I2CWQ, handle, bit))

   def i2c_write_byte(self, handle, byte_val):
      """
      Sends a single byte to the device associated with handle.

      smbus 2.0 5.5.2 - Send byte.

        handle:= >=0 (as returned by a prior call to [*i2c_open*]).
      byte_val:= 0-255, the value to write.

      ...
      pi.i2c_write_byte(1, 17)   # send byte   17 to device 1
      pi.i2c_write_byte(2, 0x23) # send byte 0x23 to device 2
      ...
      """
      return _u2i(
         _pigpio_command(self._control, _PI_CMD_I2CWS, handle, byte_val))

   def i2c_read_byte(self, handle):
      """
      Reads a single byte from the device associated with handle.

      smbus 2.0 5.5.3 - Receive byte.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).

      ...
      b = pi.i2c_read_byte(2) # read a byte from device 2
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_I2CRS, handle, 0))

   def i2c_write_byte_data(self, handle, reg, byte_val):
      """
      Writes a single byte to the specified register of the device
      associated with handle.

      smbus 2.0 5.5.4 - Write byte.

        handle:= >=0 (as returned by a prior call to [*i2c_open*]).
           reg:= >=0, the device register.
      byte_val:= 0-255, the value to write.

      ...
      # send byte 0xC5 to reg 2 of device 1
      pi.i2c_write_byte_data(1, 2, 0xC5)

      # send byte 9 to reg 4 of device 2
      pi.i2c_write_byte_data(2, 4, 9)
      ...
      """
      # I p1 handle
      # I p2 reg
      # I p3 4
      ## extension ##
      # I byte_val
      extents = [struct.pack("I", byte_val)]
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_I2CWB, handle, reg, 4, extents))

   def i2c_write_word_data(self, handle, reg, word_val):
      """
      Writes a single 16 bit word to the specified register of the
      device associated with handle.

      smbus 2.0 5.5.4 - Write word.

        handle:= >=0 (as returned by a prior call to [*i2c_open*]).
           reg:= >=0, the device register.
      word_val:= 0-65535, the value to write.

      ...
      # send word 0xA0C5 to reg 5 of device 4
      pi.i2c_write_word_data(4, 5, 0xA0C5)

      # send word 2 to reg 2 of device 5
      pi.i2c_write_word_data(5, 2, 23)
      ...
      """
      # I p1 handle
      # I p2 reg
      # I p3 4
      ## extension ##
      # I word_val
      extents = [struct.pack("I", word_val)]
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_I2CWW, handle, reg, 4, extents))

   def i2c_read_byte_data(self, handle, reg):
      """
      Reads a single byte from the specified register of the device
      associated with handle.

      smbus 2.0 5.5.5 - Read byte.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         reg:= >=0, the device register.

      ...
      # read byte from reg 17 of device 2
      b = pi.i2c_read_byte_data(2, 17)

      # read byte from reg  1 of device 0
      b = pi.i2c_read_byte_data(0, 1)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_I2CRB, handle, reg))

   def i2c_read_word_data(self, handle, reg):
      """
      Reads a single 16 bit word from the specified register of the
      device associated with handle.

      smbus 2.0 5.5.5 - Read word.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         reg:= >=0, the device register.

      ...
      # read word from reg 2 of device 3
      w = pi.i2c_read_word_data(3, 2)

      # read word from reg 7 of device 2
      w = pi.i2c_read_word_data(2, 7)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_I2CRW, handle, reg))

   def i2c_process_call(self, handle, reg, word_val):
      """
      Writes 16 bits of data to the specified register of the device
      associated with handle and reads 16 bits of data in return.

      smbus 2.0 5.5.6 - Process call.

        handle:= >=0 (as returned by a prior call to [*i2c_open*]).
           reg:= >=0, the device register.
      word_val:= 0-65535, the value to write.

      ...
      r = pi.i2c_process_call(h, 4, 0x1231)
      r = pi.i2c_process_call(h, 6, 0)
      ...
      """
      # I p1 handle
      # I p2 reg
      # I p3 4
      ## extension ##
      # I word_val
      extents = [struct.pack("I", word_val)]
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_I2CPC, handle, reg, 4, extents))

   def i2c_write_block_data(self, handle, reg, data):
      """
      Writes up to 32 bytes to the specified register of the device
      associated with handle.

      smbus 2.0 5.5.7 - Block write.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         reg:= >=0, the device register.
        data:= the bytes to write.

      ...
      pi.i2c_write_block_data(4, 5, b'hello')

      pi.i2c_write_block_data(4, 5, "data bytes")

      pi.i2c_write_block_data(5, 0, b'\\x00\\x01\\x22')

      pi.i2c_write_block_data(6, 2, [0, 1, 0x22])
      ...
      """
      # I p1 handle
      # I p2 reg
      # I p3 len
      ## extension ##
      # s len data bytes
      if len(data):
         return _u2i(_pigpio_command_ext(
            self._control, _PI_CMD_I2CWK, handle, reg, len(data), [data]))
      else:
         return 0

   def i2c_read_block_data(self, handle, reg):
      """
      Reads a block of up to 32 bytes from the specified register of
      the device associated with handle.

      smbus 2.0 5.5.7 - Block read.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         reg:= >=0, the device register.

      The amount of returned data is set by the device.

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (b, d) = pi.i2c_read_block_data(h, 10)
      if b >= 0:
         # process data
      else:
         # process read failure
      ...
      """
      bytes = _u2i(_pigpio_command(self._control, _PI_CMD_I2CRK, handle, reg))
      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   def i2c_block_process_call(self, handle, reg, data):
      """
      Writes data bytes to the specified register of the device
      associated with handle and reads a device specified number
      of bytes of data in return.

      smbus 2.0 5.5.8 - Block write-block read.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         reg:= >=0, the device register.
        data:= the bytes to write.

      The smbus 2.0 documentation states that a minimum of 1 byte may
      be sent and a minimum of 1 byte may be received.  The total
      number of bytes sent/received must be 32 or less.

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (b, d) = pi.i2c_block_process_call(h, 10, b'\\x02\\x05\\x00')

      (b, d) = pi.i2c_block_process_call(h, 10, b'abcdr')

      (b, d) = pi.i2c_block_process_call(h, 10, "abracad")

      (b, d) = pi.i2c_block_process_call(h, 10, [2, 5, 16])
      ...
      """
      # I p1 handle
      # I p2 reg
      # I p3 len
      ## extension ##
      # s len data bytes
      bytes = _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_I2CPK, handle, reg, len(data), [data]))
      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   def i2c_write_i2c_block_data(self, handle, reg, data):
      """
      Writes data bytes to the specified register of the device
      associated with handle .  1-32 bytes may be written.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         reg:= >=0, the device register.
        data:= the bytes to write.

      ...
      pi.i2c_write_i2c_block_data(4, 5, 'hello')

      pi.i2c_write_i2c_block_data(4, 5, b'hello')

      pi.i2c_write_i2c_block_data(5, 0, b'\\x00\\x01\\x22')

      pi.i2c_write_i2c_block_data(6, 2, [0, 1, 0x22])
      ...
      """
      # I p1 handle
      # I p2 reg
      # I p3 len
      ## extension ##
      # s len data bytes
      if len(data):
         return _u2i(_pigpio_command_ext(
            self._control, _PI_CMD_I2CWI, handle, reg, len(data), [data]))
      else:
         return 0

   def i2c_read_i2c_block_data(self, handle, reg, count):
      """
      Reads count bytes from the specified register of the device
      associated with handle .  The count may be 1-32.

      handle:= >=0 (as returned by a prior call to [*i2c_open*]).
         reg:= >=0, the device register.
       count:= >0, the number of bytes to read.

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (b, d) = pi.i2c_read_i2c_block_data(h, 4, 32)
      if b >= 0:
         # process data
      else:
         # process read failure
      ...
      """
      # I p1 handle
      # I p2 reg
      # I p3 4
      ## extension ##
      # I count
      extents = [struct.pack("I", count)]
      bytes = _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_I2CRI, handle, reg, 4, extents))
      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   def spi_open(self, spi_channel, spi_baud, spi_flags=0):
      """
      Returns a handle for the SPI device on channel.  Data will be
      transferred at baud bits per second.  The flags may be used to
      modify the default behaviour of 4-wire operation, mode 0,
      active low chip select.

      spi_channel:= 0 or 1, the SPI channel.
         spi_baud:= >0, the transmission rate in bits per second.
        spi_flags:= see below.

      Normally you would only use the [*spi_**] functions if
      you are or will be connecting to the Pi over a network.  If
      you will always run on the local Pi use the standard SPI
      modules instead.

      spiFlags consists of the least significant 8 bits.

      . .
      7 6 5 4 3 2 1 0
      n n n n W P m m
      . .

      mm defines the SPI mode.

      . .
      Mode POL PHA
      0    0   0
      1    0   1
      2    1   0
      3    1   1
      . .

      P is 0 for active low chip select (normal) and 1 for active high.

      W is 0 if the device is not 3-wire, 1 if the device is 3-wire.

      nnnn defines the number of bytes (0-15) to write before switching
      the MOSI line to MISO to read data.  This field is ignored
      if W is not set.

      The other bits in flags should be set to zero.

      ...
      # open SPI device on channel 1 in mode 3 at 20000 bits per second

      h = pi.spi_open(1, 20000, 3)
      ...
      """
      # I p1 spi_channel
      # I p2 spi_baud
      # I p3 4
      ## extension ##
      # I spi_flags
      extents = [struct.pack("I", spi_flags)]
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_SPIO, spi_channel, spi_baud, 4, extents))

   def spi_close(self, handle):
      """
      Closes the SPI device associated with handle.

      handle:= >=0 (as returned by a prior call to [*spi_open*]).

      ...
      pi.spi_close(h)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_SPIC, handle, 0))

   def spi_read(self, handle, count):
      """
      Reads count bytes from the SPI device associated with handle.

      handle:= >=0 (as returned by a prior call to [*spi_open*]).
       count:= >0, the number of bytes to read.

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (b, d) = pi.spi_read(h, 60) # read 60 bytes from device h
      if b == 60:
         # process read data
      else:
         # error path
      ...
      """
      bytes = _u2i(_pigpio_command(
         self._control, _PI_CMD_SPIR, handle, count))
      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   def spi_write(self, handle, data):
      """
      Writes the data bytes to the SPI device associated with handle.

      handle:= >=0 (as returned by a prior call to [*spi_open*]).
        data:= the bytes to write.

      ...
      pi.spi_write(0, b'\\x02\\xc0\\x80') # write 3 bytes to device 0

      pi.spi_write(0, b'defgh')        # write 5 bytes to device 0

      pi.spi_write(0, "def")           # write 3 bytes to device 0

      pi.spi_write(1, [2, 192, 128])   # write 3 bytes to device 1
      ...
      """
      # I p1 handle
      # I p2 0
      # I p3 len
      ## extension ##
      # s len data bytes
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_SPIW, handle, 0, len(data), [data]))

   def spi_xfer(self, handle, data):
      """
      Writes the data bytes to the SPI device associated with handle,
      returning the data bytes read from the device.

      handle:= >=0 (as returned by a prior call to [*spi_open*]).
        data:= the bytes to write.

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (count, rx_data) = pi.spi_xfer(h, b'\\x01\\x80\\x00')

      (count, rx_data) = pi.spi_xfer(h, [1, 128, 0])

      (count, rx_data) = pi.spi_xfer(h, b"hello")

      (count, rx_data) = pi.spi_xfer(h, "hello")
      ...
      """
      # I p1 handle
      # I p2 0
      # I p3 len
      ## extension ##
      # s len data bytes
      bytes = _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_SPIX, handle, 0, len(data), [data]))
      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   def serial_open(self, tty, ser_baud, ser_flags=0):
      """
      Returns a handle for the serial tty device opened
      at ser_baud bits per second.

            tty:= the serial device to open.
       ser_baud:= baud rate
      ser_flags:= 0, no flags are currently defined.

      Normally you would only use the [*serial_**] functions if
      you are or will be connecting to the Pi over a network.  If
      you will always run on the local Pi use the standard serial
      modules instead.

      ...
      h1 = pi.serial_open("/dev/ttyAMA0", 300)

      h2 = pi.serial_open("/dev/ttyUSB1", 19200, 0)
      ...
      """
      # I p1 ser_baud
      # I p2 ser_flags
      # I p3 len
      ## extension ##
      # s len data bytes
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_SERO, ser_baud, ser_flags, len(tty), [tty]))

   def serial_close(self, handle):
      """
      Closes the serial device associated with handle.

      handle:= >=0 (as returned by a prior call to [*serial_open*]).

      ...
      pi.serial_close(h1)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_SERC, handle, 0))

   def serial_read_byte(self, handle):
      """
      Returns a single byte from the device associated with handle.

      handle:= >=0 (as returned by a prior call to [*serial_open*]).

      ...
      b = pi.serial_read_byte(h1)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_SERRB, handle, 0))

   def serial_write_byte(self, handle, byte_val):
      """
      Writes a single byte to the device associated with handle.

        handle:= >=0 (as returned by a prior call to [*serial_open*]).
      byte_val:= 0-255, the value to write.

      ...
      pi.serial_write_byte(h1, 23)

      pi.serial_write_byte(h1, ord('Z'))
      ...
      """
      return _u2i(
         _pigpio_command(self._control, _PI_CMD_SERWB, handle, byte_val))

   def serial_read(self, handle, count):
      """
      Reads up to count bytes from the device associated with handle.

      handle:= >=0 (as returned by a prior call to [*serial_open*]).
       count:= >0, the number of bytes to read.

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (b, d) = pi.serial_read(h2, 100)
      if b > 0:
         # process read data
      ...
      """
      bytes = _u2i(_pigpio_command(self._control, _PI_CMD_SERR, handle, count))

      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   def serial_write(self, handle, data):
      """
      Writes the data bytes to the device associated with handle.

      handle:= >=0 (as returned by a prior call to [*serial_open*]).
        data:= the bytes to write.

      ...
      pi.serial_write(h1, b'\\x02\\x03\\x04')

      pi.serial_write(h2, b'help')

      pi.serial_write(h2, "hello")

      pi.serial_write(h1, [2, 3, 4])
      ...
      """
      # I p1 handle
      # I p2 0
      # I p3 len
      ## extension ##
      # s len data bytes

      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_SERW, handle, 0, len(data), [data]))

   def serial_data_available(self, handle):
      """
      Returns the number of bytes available to be read from the
      device associated with handle.

      handle:= >=0 (as returned by a prior call to [*serial_open*]).

      ...
      rdy = pi.serial_data_available(h1)

      if rdy > 0:
         (b, d) = pi.serial_read(h1, rdy)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_SERDA, handle, 0))

   def gpio_trigger(self, user_gpio, pulse_len=10, level=1):
      """
      Send a trigger pulse to a gpio.  The gpio is set to
      level for pulse_len microseconds and then reset to not level.

      user_gpio:= 0-31
      pulse_len:= 1-50
          level:= 0-1

      ...
      pi.gpio_trigger(23, 10, 1)
      ...
      """
      # pigpio message format

      # I p1 user_gpio
      # I p2 pulse_len
      # I p3 4
      ## extension ##
      # I level
      extents = [struct.pack("I", level)]
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_TRIG, user_gpio, pulse_len, 4, extents))

   def store_script(self, script):
      """
      Store a script for later execution.

      script:= the script text as a series of bytes.

      Returns a >=0 script id if OK.

      ...
      sid = pi.store_script(
         b'tag 0 w 22 1 mils 100 w 22 0 mils 100 dcr p0 jp 0')
      ...
      """
      # I p1 0
      # I p2 0
      # I p3 len
      ## extension ##
      # s len data bytes
      if len(script):
         return _u2i(_pigpio_command_ext(
            self._control, _PI_CMD_PROC, 0, 0, len(script), [script]))
      else:
         return 0

   def run_script(self, script_id, params=None):
      """
      Runs a stored script.

      script_id:= id of stored script.
         params:= up to 10 parameters required by the script.

      ...
      s = pi.run_script(sid, [par1, par2])

      s = pi.run_script(sid)

      s = pi.run_script(sid, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
      ...
      """
      # I p1 script id
      # I p2 0
      # I p3 params * 4 (0-10 params)
      ## (optional) extension ##
      # I[] params
      if params is not None:
         ext = bytearray()
         for p in params:
            ext.extend(struct.pack("I", p))
         nump = len(params)
         extents = [ext]
      else:
         nump = 0
         extents = []
      return _u2i(_pigpio_command_ext(
         self._control, _PI_CMD_PROCR, script_id, 0, nump*4, extents))

   def script_status(self, script_id):
      """
      Returns the run status of a stored script as well as the
      current values of parameters 0 to 9.

      script_id:= id of stored script.

      The run status may be

      . .
      PI_SCRIPT_INITING
      PI_SCRIPT_HALTED
      PI_SCRIPT_RUNNING
      PI_SCRIPT_WAITING
      PI_SCRIPT_FAILED
      . .

      The return value is a tuple of run status and a list of
      the 10 parameters.  On error the run status will be negative
      and the parameter list will be empty.

      ...
      (s, pars) = pi.script_status(sid)
      ...
      """
      status = _u2i(_pigpio_command(self._control, _PI_CMD_PROCP, script_id, 0))
      if status > 0:
         params = struct.unpack('I10i', self._control.recv(44))
         return params[0], params[1:]
      return status, ()

   def stop_script(self, script_id):
      """
      Stops a running script.

      script_id:= id of stored script.

      ...
      status = pi.stop_script(sid)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PROCS, script_id, 0))

   def delete_script(self, script_id):
      """
      Deletes a stored script.

      script_id:= id of stored script.

      ...
      status = pi.delete_script(sid)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_PROCD, script_id, 0))

   def bb_serial_read_open(self, user_gpio, bb_baud):
      """
      Opens a gpio for bit bang reading of serial data.

      user_gpio:= 0-31, the gpio to use.
        bb_baud:= 300-250000, the baud rate.

      The serial data is held in a cyclic buffer and is read using
      bb_serial_read.

      It is the caller's responsibility to read data from the cyclic
      buffer in a timely fashion.

      ...
      status = pi.bb_serial_read_open(4, 19200)
      status = pi.bb_serial_read_open(17, 9600)
      ...
      """
      return _u2i(_pigpio_command(
         self._control, _PI_CMD_SLRO, user_gpio, bb_baud))

   def bb_serial_read(self, user_gpio):
      """
      Returns data from the bit bang serial cyclic buffer.

      user_gpio:= 0-31 (opened in a prior call to [*bb_serial_read_open*])

      The returned value is a tuple of the number of bytes read and a
      bytearray containing the bytes.  If there was an error the
      number of bytes read will be less than zero (and will contain
      the error code).

      ...
      (count, data) = pi.bb_serial_read(4)
      ...
      """
      bytes = _u2i(_pigpio_command(self._control, _PI_CMD_SLR, user_gpio, 10000))
      if bytes > 0:
         return bytes, self._rxbuf(bytes)
      return bytes, ""

   
   def bb_serial_read_close(self, user_gpio):
      """
      Closes a gpio for bit bang reading of serial data.

      user_gpio:= 0-31 (opened in a prior call to [*bb_serial_read_open*])

      ...
      status = pi.bb_serial_read_close(17)
      ...
      """
      return _u2i(_pigpio_command(self._control, _PI_CMD_SLRC, user_gpio, 0))

   def callback(self, user_gpio, edge=RISING_EDGE, func=None):
      """
      Calls a user supplied function (a callback) whenever the
      specified gpio edge is detected.

      user_gpio:= 0-31.
           edge:= EITHER_EDGE, RISING_EDGE (default), or FALLING_EDGE.
           func:= user supplied callback function.

      If a user callback is not specified a default tally callback is
      provided which simply counts edges.

      The user supplied callback receives three parameters, the gpio,
      the level, and the tick.

      ...
      def cbf(gpio, level, tick):
         print(gpio, level, tick)

      cb1 = pi.callback(22, pigpio.EITHER_EDGE, cbf)

      cb2 = pi.callback(4, pigpio.EITHER_EDGE)

      cb3 = pi.callback(17)

      print(cb3.tally())
      ...
      """
      return _callback(self._notify, user_gpio, edge, func)

   def wait_for_edge(self, user_gpio, edge=RISING_EDGE, wait_timeout=60.0):
      """
      Wait for an edge event on a gpio.

         user_gpio:= 0-31.
              edge:= EITHER_EDGE, RISING_EDGE (default), or
                     FALLING_EDGE.
      wait_timeout:= 0.0- (default 60.0).

      The function returns as soon as the edge is detected
      or after the number of seconds specified by timeout has
      expired.

      The function returns True if the edge is detected,
      otherwise False.

      ...
      if pi.wait_for_edge(23):
         print("Rising edge detected")
      else:
         print("wait for edge timed out")

      if pi.wait_for_edge(23, pigpio.FALLING_EDGE, 5.0):
         print("Falling edge detected")
      else:
         print("wait for falling edge timed out")
      ...
      """
      a = _wait_for_edge(self._notify, user_gpio, edge, wait_timeout)
      return a.trigger

   def __init__(self,
                host = os.getenv("PIGPIO_ADDR", ''),
                port = os.getenv("PIGPIO_PORT", 8888)):
      """
      Grants access to a Pi's gpios.

      host:= the host name of the Pi on which the pigpio daemon is
             running.  The default is localhost unless overridden by
             the PIGPIO_ADDR environment variable.
       
      port:= the port number on which the pigpio daemon is listening.
             The default is 8888 unless overridden by the PIGPIO_PORT
             environment variable.  The pigpio daemon must have been
             started with the same port number.

      This connects to the pigpio daemon and reserves resources
      to be used for sending commands and receiving notifications.

      ...
      pi = pigio.pi()              # use defaults
      pi = pigpio.pi('mypi')       # specify host, default port
      pi = pigpio.pi('mypi', 7777) # specify host and port
      ...
      """
      self.connected = True

      self._control = None
      self._notify  = None

      self._host = ''
      self._port = 8888

      self._host = host
      self._port = int(port)

      self._control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

      # Disable the Nagle algorithm.
      self._control.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

      try:
         self._control.connect((self._host, self._port))
         self._notify = _callback_thread(self._control, self._host, self._port)

      except socket.error:
         self.connected = False
         if self._control is not None:
            self._control = None
         if self._host == '':
            h = "localhost"
         else:
            h = self._host

         errStr = "Can't connect to pigpio on {}({})".format(
            str(h), str(self._port))

         print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
         print(errStr)
         print("")
         print("Did you start the pigpio daemon? E.g. sudo pigpiod")
         print("")
         print("Did you specify the correct Pi host/port in the environment")
         print("variables PIGPIO_ADDR/PIGPIO_PORT?")
         print("E.g. export PIGPIO_ADDR=soft, export PIGPIO_PORT=8888")
         print("")
         print("Did you specify the correct Pi host/port in the")
         print("pigpio.pi() function? E.g. pigpio.pi('soft', 8888))")
         print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
      else:
         atexit.register(self.stop)

   def stop(self):
      """Release pigpio resources.

      ...
      pi.stop()
      ...
      """

      if self._notify is not None:
         self._notify.stop()
         self._notify = None

      if self._control is not None:
         self._control.close()
         self._control = None

def xref():
   """
   bb_baud: 100 - 250000
   The baud rate used for the transmission of bit bang serial data.

   bit: 0-1
   A value of 0 or 1.

   bits: 32 bit number
   A mask used to select gpios to be operated on.  If bit n is set
   then gpio n is selected.  A convenient way of setting bit n is to
   bit or in the value (1<<n).

   To select gpios 1, 7, 23

   bits = (1<<1) | (1<<7) | (1<<23)

   byte_val: 0-255
   A whole number.

   count:
   The number of bytes of data to be transferred.

   data:
   Data to be transmitted, a series of bytes.

   delay: 1-
   The length of a pulse in microseconds.

   dutycycle: 0-range_
   A number between 0 and range_.

   The dutycycle sets the proportion of time on versus time off during each
   PWM cycle.

   Dutycycle     @ On time
   0             @ Off
   range_ * 0.25 @ 25% On
   range_ * 0.50 @ 50% On
   range_ * 0.75 @ 75% On
   range_        @ Fully On

   edge: 0-2
   EITHER_EDGE = 2 
   FALLING_EDGE = 1 
   RISING_EDGE = 0

   errnum: <0
   PI_BAD_DUTYCYCLE = -8 
   PI_BAD_DUTYRANGE = -21 
   PI_BAD_FLAGS = -77 
   PI_BAD_GPIO = -3 
   PI_BAD_HANDLE = -25 
   PI_BAD_I2C_ADDR = -75 
   PI_BAD_I2C_BUS = -74 
   PI_BAD_LEVEL = -5 
   PI_BAD_MICS_DELAY = -64 
   PI_BAD_MILS_DELAY = -65 
   PI_BAD_MODE = -4 
   PI_BAD_PARAM = -81 
   PI_BAD_PARAM_NUM = -52 
   PI_BAD_PUD = -6 
   PI_BAD_PULSELEN = -46 
   PI_BAD_PULSEWIDTH = -7 
   PI_BAD_SCRIPT = -47 
   PI_BAD_SCRIPT_CMD = -55 
   PI_BAD_SCRIPT_ID = -48 
   PI_BAD_SERIAL_COUNT = -51 
   PI_BAD_SER_DEVICE = -79 
   PI_BAD_SER_OFFSET = -49 
   PI_BAD_SER_SPEED = -80 
   PI_BAD_SPI_CHANNEL = -76 
   PI_BAD_SPI_COUNT = -84 
   PI_BAD_SPI_SPEED = -78 
   PI_BAD_TAG = -63 
   PI_BAD_USER_GPIO = -2 
   PI_BAD_VAR_NUM = -56 
   PI_BAD_WAVE_BAUD = -35 
   PI_BAD_WAVE_ID = -66 
   PI_BAD_WDOG_TIMEOUT = -15 
   PI_BAD_WVSC_COMMND = -43 
   PI_BAD_WVSM_COMMND = -44 
   PI_BAD_WVSP_COMMND = -45 
   PI_DUP_TAG = -53 
   PI_EMPTY_WAVEFORM = -69 
   PI_GPIO_IN_USE = -50 
   PI_I2C_OPEN_FAILED = -71 
   PI_I2C_READ_FAILED = -83 
   PI_I2C_WRITE_FAILED = -82 
   PI_NOT_HALTED = -62 
   PI_NOT_PERMITTED = -41 
   PI_NOT_SERIAL_GPIO = -38 
   PI_NO_HANDLE = -24 
   PI_NO_MEMORY = -58 
   PI_NO_SCRIPT_ROOM = -57 
   PI_NO_WAVEFORM_ID = -70 
   PI_SER_OPEN_FAILED = -72 
   PI_SER_READ_FAILED = -86 
   PI_SER_READ_NO_DATA = -87 
   PI_SER_WRITE_FAILED = -85 
   PI_SOCK_READ_FAILED = -59 
   PI_SOCK_WRIT_FAILED = -60 
   PI_SOME_PERMITTED = -42 
   PI_SPI_OPEN_FAILED = -73 
   PI_SPI_XFER_FAILED = -89 
   PI_TOO_MANY_CBS = -67 
   PI_TOO_MANY_CHARS = -37 
   PI_TOO_MANY_OOL = -68 
   PI_TOO_MANY_PARAM = -61 
   PI_TOO_MANY_PULSES = -36 
   PI_TOO_MANY_TAGS = -54 
   PI_UNKNOWN_COMMAND = -88 

   frequency: 0-40000
   Defines the frequency to be used for PWM on a gpio.
   The closest permitted frequency will be used.

   func:
   A user supplied callback function.

   gpio: 0-53
   A Broadcom numbered gpio.  All the user gpios are in the range 0-31.

   gpio_off:
   A mask used to select gpios to be operated on.  See [*bits*].

   This mask selects the gpios to be switched off at the start
   of a pulse.

   gpio_on:
   A mask used to select gpios to be operated on.  See [*bits*].

   This mask selects the gpios to be switched on at the start
   of a pulse.

   handle: 0-
   A number referencing an object opened by one of [*i2c_open*],
   [*notify_open*], [*serial_open*], [*spi_open*].

   host:
   The name or IP address of the Pi running the pigpio daemon.

   i2c_*:
   One of the i2c_ functions.

   i2c_address:
   The address of a device on the I2C bus (0x08 - 0x77)

   i2c_bus: 0-1
   An I2C bus number.

   i2c_flags: 32 bit
   No I2C flags are currently defined.

   level: 0-1 (2)
   CLEAR = 0 
   HIGH = 1 
   LOW = 0 
   OFF = 0 
   ON = 1 
   SET = 1 
   TIMEOUT = 2 # only returned for a watchdog timeout

   mode: 0-7
   ALT0 = 4 
   ALT1 = 5 
   ALT2 = 6 
   ALT3 = 7 
   ALT4 = 3 
   ALT5 = 2 
   INPUT = 0 
   OUTPUT = 1

   offset: 0-
   The offset wave data starts from the beginning of the waveform
   being currently defined.

   params: 32 bit number
   When scripts are started they can receive up to 10 parameters
   to define their operation.

   port: 
   The port used by the pigpio daemon, defaults to 8888.

   pud: 0-2
   PUD_DOWN = 1 
   PUD_OFF = 0 
   PUD_UP = 2 

   pulse_len: 1-50
   A whole number.

   pulses:
   A list of class pulse objects defining the characteristics of a
   waveform.

   pulsewidth:
   The servo pulsewidth in microseconds.  0 switches pulses off.

   range_: 25-40000
   Defines the limits for the [*dutycycle*] parameter.

   range_ defaults to 255.

   reg: 0-255
   An I2C device register.  The usable registers depend on the
   actual device.

   script:
   The text of a script to store on the pigpio daemon.

   script_id: 0-
   A number referencing a script created by [*store_script*].

   ser_baud:
   The transmission rate in bits per second.

   The allowable values are 50, 75, 110, 134, 150, 200, 300,
   600, 1200, 1800, 2400, 4800, 9600, 19200, 38400,
   57600, 115200, or 230400.

   ser_flags: 32 bit
   No serial flags are currently defined.

   serial_*:
   One of the serial_ functions.

   spi_*:
   One of the spi_ functions.

   spi_baud: 1-
   The transmission rate in bits per second.

   spi_channel: 0-1
   A SPI channel.

   spi_flags: 32 bit

   spi_flags consists of the least significant 8 bits.

   . .
   7 6 5 4 3 2 1 0
   n n n n W P m m
   . .

   mm defines the SPI mode.

   . .
   Mode POL PHA
    0    0   0
    1    0   1
    2    1   0
    3    1   1
   . .

   P is 0 for active low chip select (normal) and 1 for active high.

   W is 0 if the device is not 3-wire, 1 if the device is 3-wire.

   nnnn defines the number of bytes (0-15) to write before switching
   the MOSI line to MISO to read data.  This field is ignored
   if W is not set.

   The other bits in flags should be set to zero.

   t1:
   A tick (earlier).

   t2:
   A tick (later).

   tty:
   A Pi serial tty device, e.g. /dev/ttyAMA0, /dev/ttyUSB0

   user_gpio: 0-31
   A Broadcom numbered gpio.

   All the user gpios are in the range 0-31.

   Not all the gpios within this range are usable, some are reserved
   for system use.

   wait_timeout: 0.0 -
   The number of seconds to wait in wait_for_edge before timing out.

   wave_add_*:
   One of [*wave_add_new*] , [*wave_add_generic*], [*wave_add_serial*].

   wave_id: 0-
   A number referencing a wave created by [*wave_create*].

   wave_send_*:
   One of [*wave_send_once*], [*wave_send_repeat*].

   wdog_timeout: 0-60000
   Defines a gpio watchdog timeout in milliseconds.  If no level
   change is detected on the gpio for timeout millisecond a watchdog
   timeout report is issued (with level TIMEOUT).

   word_val: 0-65535
   A whole number.
   """
   pass

