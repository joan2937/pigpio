/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

/*
This version is for pigpio version 16
*/

#ifndef PIGPIO_H
#define PIGPIO_H

#include <stdint.h>
#include <pthread.h>

#define PIGPIO_VERSION 16

/* INTRO

pigpio is a C library for the Raspberry Pi which allows 
control of the gpios.

Its main features are:

o PWM on any of gpios 0-31 
o servo pulses on any of gpios 0-31 
o callbacks when any of gpios 0-31 change state 
o callbacks at timed intervals 
o reading/writing all of the gpios in a bank as one operation 
o individually setting gpio modes, reading and writing 
o notifications when any of gpios 0-31 change state 
o the construction of output waveforms with microsecond timing 
o rudimentary permission control over gpios 
o a simple interface to start and stop new threads 
o I2C, SPI, and serial link wrappers

ALL gpios are identified by their Broadcom number.

The PWM and servo pulses are timed using the DMA and PWM peripherals.

This use was inspired by Richard Hirst's servoblaster kernel module. 
See https://github.com/richardghirst/PiBits 
Tag rgh on the Raspberry Pi forums http://www.raspberrypi.org/phpBB3/ 

INTRO */

/* USAGE
Usage

Include <pigpio.h> in your source files.

Assuming your source is in example.c use the following command to build

. .
gcc -o example example.c -lpigpio -lpthread -lrt
. .

For examples see checklib.c, demolib.c, pigpio.c, pigpiod.c, pig2vcd.c,
and pigs.c

All the functions which return an int return < 0 on error.

If the library isn't initialised all but the gpioCfg*, gpioVersion,
and gpioHardwareRevision functions will return error PI_NOT_INITIALISED.

If the library is initialised the gpioCfg* functions will
return error PI_INITIALISED.

USAGE */

/* OVERVIEW

ESSENTIAL

gpioInitialise             Initialise library
gpioTerminate              Stop library

BEGINNER

gpioSetMode                Set a gpio mode
gpioGetMode                Get a gpio mode

gpioSetPullUpDown          Set/clear gpio pull up/down resistor

gpioRead                   Read a gpio
gpioWrite                  Write a gpio

gpioPWM                    Start/stop PWM pulses on a gpio

gpioServo                  Start/stop servo pulses on a gpio

gpioDelay                  Delay for a number of microseconds

gpioSetAlertFunc           Request a gpio level change callback

gpioSetTimerFunc           Request a regular timed callback

INTERMEDIATE

gpioTrigger                Send a trigger pulse to a gpio.

gpioSetWatchdog            Set a watchdog on a gpio.

gpioSetPWMrange            Configure PWM range for a gpio
gpioGetPWMrange            Get configured PWM range for a gpio

gpioSetPWMfrequency        Configure PWM frequency for a gpio
gpioGetPWMfrequency        Get configured PWM frequency for a gpio

gpioRead_Bits_0_31         Read all gpios in bank 1
gpioRead_Bits_32_53        Read all gpios in bank 2

gpioWrite_Bits_0_31_Clear  Clear selected gpios in bank 1
gpioWrite_Bits_32_53_Clear Clear selected gpios in bank 2

gpioWrite_Bits_0_31_Set    Set selected gpios in bank 1
gpioWrite_Bits_32_53_Set   Set selected gpios in bank 2

gpioStartThread            Start a new thread
gpioStopThread             Stop a previously started thread

ADVANCED

gpioGetPWMrealRange        Get underlying PWM range for a gpio

gpioSetAlertFuncEx         Request a gpio change callback, extended

gpioSetSignalFunc          Request a signal callback
gpioSetSignalFuncEx        Request a signal callback, extended

gpioSetGetSamplesFunc      Requests a gpio samples callback
gpioSetGetSamplesFuncEx    Requests a gpio samples callback, extended

gpioSetTimerFuncEx         Request a regular timed callback, extended

gpioNotifyOpen             Request a notification handle
gpioNotifyBegin            Start notifications for selected gpios
gpioNotifyPause            Pause notifications
gpioNotifyClose            Close a notification

gpioSerialReadOpen         Opens a gpio for bit bang serial reads
gpioSerialRead             Reads bit bang serial data from a gpio
gpioSerialReadClose        Closes a gpio for bit bang serial reads

SCRIPTS

gpioStoreScript            Store a script
gpioRunScript              Run a stored script
gpioScriptStatus           Get script status and parameters
gpioStopScript             Stop a running script
gpioDeleteScript           Delete a stored script

WAVES

gpioWaveClear              Deletes all waveforms

gpioWaveAddNew             Starts a new waveform
gpioWaveAddGeneric         Adds a series of pulses to the waveform
gpioWaveAddSerial          Adds serial data to the waveform

gpioWaveCreate             Creates a waveform from added data
gpioWaveDelete             Deletes one or more waveforms

gpioWaveTxSend             Transmits a waveform
gpioWaveTxBusy             Checks to see if the waveform has ended
gpioWaveTxStop             Aborts the current waveform

gpioWaveGetMicros          Length in microseconds of the current waveform
gpioWaveGetHighMicros      Length of longest waveform so far
gpioWaveGetMaxMicros       Absolute maximum allowed micros

gpioWaveGetPulses          Length in pulses of the current waveform
gpioWaveGetHighPulses      Length of longest waveform so far
gpioWaveGetMaxPulses       Absolute maximum allowed pulses

gpioWaveGetCbs             Length in cbs of the current waveform
gpioWaveGetHighCbs         Length of longest waveform so far
gpioWaveGetMaxCbs          Absolute maximum allowed cbs

gpioWaveTxStart            Creates/transmits a waveform (DEPRECATED)

I2C

i2cOpen                    Opens an I2C device
i2cClose                   Closes an I2C device

i2cReadDevice              Reads the raw I2C device
i2cWriteDevice             Writes the raw I2C device

i2cWriteQuick              smbus write quick
i2cWriteByte               smbus write byte
i2cReadByte                smbus read byte
i2cWriteByteData           smbus write byte data
i2cWriteWordData           smbus write word data
i2cReadByteData            smbus read byte data
i2cReadWordData            smbus read word data
i2cProcessCall             smbus process call
i2cWriteBlockData          smbus write block data
i2cReadBlockData           smbus read block data
i2cBlockProcessCall        smbus block process call

i2cWriteI2CBlockData       smbus write I2C block data
i2cReadI2CBlockData        smbus read I2C block data

SPI

spiOpen                    Opens a SPI device
spiClose                   Closes a SPI device

spiRead                    Reads bytes from a SPI device
spiWrite                   Writes bytes to a SPI device
spiXfer                    Transfers bytes with a SPI device

SERIAL

serOpen                    Opens a serial device (/dev/tty*)
serClose                   Closes a serial device

serWriteByte               Writes a byte to a serial device
serReadByte                Reads a byte from a serial device
serWrite                   Writes bytes to a serial device
serRead                    Reads bytes from a serial device

serDataAvailable           Returns number of bytes ready to be read

CONFIGURATION

gpioCfgBufferSize          Configure the gpio sample buffer size
gpioCfgClock               Configure the gpio sample rate
gpioCfgDMAchannel          Configure the DMA channel (DEPRECATED)
gpioCfgDMAchannels         Configure the DMA channels
gpioCfgPermissions         Configure the gpio access permissions
gpioCfgInterfaces          Configure user interfaces
gpioCfgInternals           Configure miscellaneous internals
gpioCfgSocketPort          Configure socket port

UTILITIES

gpioTick                   Get current tick (microseconds)

gpioHardwareRevision       Get hardware revision
gpioVersion                Get the pigpio version

getBitInBytes              Get the value of a bit
putBitInBytes              Set the value of a bit

gpioTime                   Get current time
gpioSleep                  Sleep for specified time

time_sleep                 Sleeps for a float number of seconds
time_time                  Float number of seconds since the epoch

EXPERT

rawWaveAddSPI              Not intended for general use
rawWaveAddGeneric          Not intended for general use
rawWaveGetOut              Not intended for general use
rawWaveSetOut              Not intended for general use
rawWaveGetIn               Not intended for general use
rawWaveSetIn               Not intended for general use
rawDumpWave                Not intended for general use
rawDumpScript              Not intended for general use

OVERVIEW */

#define PI_INPFIFO "/dev/pigpio"
#define PI_OUTFIFO "/dev/pigout"
#define PI_ERRFIFO "/dev/pigerr"

#define PI_ENVPORT "PIGPIO_PORT"
#define PI_ENVADDR "PIGPIO_ADDR"

#define PI_LOCKFILE "/var/run/pigpio.pid"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
uint16_t func;
uint16_t size;
} gpioHeader_t;

typedef struct
{
size_t size;
void *ptr;
uint32_t data;
} gpioExtent_t;

typedef struct
{
uint32_t tick;
uint32_t level;
} gpioSample_t;

typedef struct
{
uint16_t seqno;
uint16_t flags;
uint32_t tick;
uint32_t level;
} gpioReport_t;

typedef struct
{
uint32_t gpioOn;
uint32_t gpioOff;
uint32_t usDelay;
} gpioPulse_t;

#define WAVE_FLAG_READ 1
#define WAVE_FLAG_TICK 2

typedef struct
{
uint32_t gpioOn;
uint32_t gpioOff;
uint32_t usDelay;
uint32_t flags;
} rawWave_t;

typedef struct
{
int clk;     /* gpio for clock           */
int mosi;    /* gpio for MOSI            */
int miso;    /* gpio for MISO            */
int ss_pol;  /* slave select off state   */
int ss_us;   /* delay after slave select */
int clk_pol; /* clock off state          */
int clk_pha; /* clock phase              */
int clk_us;  /* clock micros             */
} rawSPI_t;

typedef struct { /* linux/arch/arm/mach-bcm2708/include/mach/dma.h */
unsigned long info;
unsigned long src;
unsigned long dst;
unsigned long length;
unsigned long stride;
unsigned long next;
unsigned long pad[2];
} rawCbs_t;

typedef void (*gpioAlertFunc_t)    (int      gpio,
                                 int      level,
                                 uint32_t tick);

typedef void (*gpioAlertFuncEx_t)  (int      gpio,
                                 int      level,
                                 uint32_t tick,
                                 void *userdata);

typedef void (*gpioTimerFunc_t)    (void);

typedef void (*gpioTimerFuncEx_t)  (void *userdata);

typedef void (*gpioSignalFunc_t)   (int    signum);

typedef void (*gpioSignalFuncEx_t) (int    signum,
                                 void *userdata);

typedef void (*gpioGetSamplesFunc_t)   (const gpioSample_t *samples,
                                     int                  numSamples);

typedef void (*gpioGetSamplesFuncEx_t) (const gpioSample_t *samples,
                                     int                  numSamples,
                                     void *userdata);

typedef void *(gpioThreadFunc_t) (void *);


/* FUNC */
int gpioInitialise(void);
/* DESC
Initialises the library.

Call before using the other library functions.

Returns the pigpio version number if OK, otherwise PI_INIT_FAILED.

The only exception is the optional gpioCfg* functions, see later.
DESC */



/* FUNC */
void gpioTerminate(void);
/* DESC
Terminates the library.

Returns nothing.

Call before program exit.

This function resets the DMA and PWM peripherals, releases memory, and
terminates any running threads.
DESC */



/* FUNC */
int gpioSetMode(unsigned gpio, unsigned mode);
/* DESC
Sets the gpio mode, typically input or output.

Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_MODE.

Arduino style: pinMode.

...
gpioSetMode(17, PI_INPUT);  // set gpio17 as input
gpioSetMode(18, PI_OUTPUT); // set gpio18 as output
gpioSetMode(22,PI_ALT0);    // set gpio22 to alternative mode 0
...
DESC */

/* gpio: 0-53 */

#define PI_MIN_GPIO       0
#define PI_MAX_GPIO      53

/* mode: 0-7 */

#define PI_INPUT  0
#define PI_OUTPUT 1
#define PI_ALT0   4
#define PI_ALT1   5
#define PI_ALT2   6
#define PI_ALT3   7
#define PI_ALT4   3
#define PI_ALT5   2



/* FUNC */
int gpioGetMode(unsigned gpio);
/* DESC
Gets the gpio mode.

Returns the gpio mode if OK, otherwise PI_BAD_GPIO.

...
if (gpioGetMode(17) != PI_ALT0)
{
   gpioSetMode(17, PI_ALT0);  // set gpio17 to ALT0
}
...
DESC */



/* FUNC */
int gpioSetPullUpDown(unsigned gpio, unsigned pud);
/* DESC
Sets or clears resistor pull ups or downs on the gpio.

Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_PUD.

...
gpioSetPullUpDown(17, PI_PUD_UP);   // sets a pull-up on gpio17
gpioSetPullUpDown(18, PI_PUD_DOWN); // sets a pull-down on gpio18
gpioSetPullUpDown(23, PI_PUD_OFF);  // clear pull-ups/downs on gpio23
...
DESC */

/* pud: 0-2 */

#define PI_PUD_OFF  0
#define PI_PUD_DOWN 1
#define PI_PUD_UP   2



/* FUNC */
int gpioRead (unsigned gpio);
/* DESC
Reads the gpio level, on or off.

Returns the gpio level if OK, otherwise PI_BAD_GPIO.

Arduino style: digitalRead.

...
printf("gpio24 is level %d\n", gpioRead(24));
...
DESC */

/* level: 0-1 */

#define PI_OFF   0
#define PI_ON    1

#define PI_CLEAR 0
#define PI_SET   1

#define PI_LOW   0
#define PI_HIGH  1

/* level: only reported for gpio timeout, see gpioSetWatchdogTimeout */

#define PI_TIMEOUT 2



/* FUNC */
int gpioWrite(unsigned gpio, unsigned level);
/* DESC
Sets the gpio level, on or off.

Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_LEVEL.

If PWM or servo pulses are active on the gpio they are switched off.

Arduino style: digitalWrite

...
gpioWrite(24, 1); // sets gpio24 high
...
DESC */



/* FUNC */
int gpioPWM(unsigned user_gpio, unsigned dutycycle);
/* DESC
Starts PWM on the gpio, dutycycle between 0 (off) and range (fully on).
Range defaults to 255.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_BAD_DUTYCYCLE.

Arduino style: analogWrite

This and the servo functionality use the DMA and PWM or PCM peripherals
to control and schedule the pulse lengths and duty cycles.

The gpioSetPWMrange funtion can change the default range of 255.

...
gpioPWM(17, 255); // sets gpio17 full on
gpioPWM(18, 128); // sets gpio18 half on
gpioPWM(23, 0);   // sets gpio23 full off
...
DESC */

/* user_gpio: 0-31 */

#define PI_MAX_USER_GPIO 31

/* dutycycle: 0-range */

#define PI_DEFAULT_DUTYCYCLE_RANGE   255



/* FUNC */
int gpioSetPWMrange(unsigned user_gpio, unsigned range);
/* DESC
Selects the dutycycle range to be used for the gpio.  Subsequent calls
to gpioPWM will use a dutycycle between 0 (off) and range (fully on).

Returns the real range for the given gpio's frequency if OK,
otherwise PI_BAD_USER_GPIO or PI_BAD_DUTYRANGE.

If PWM is currently active on the gpio its dutycycle will be scaled
to reflect the new range.

The real range, the number of steps between fully off and fully
on for each frequency, is given in the following table.

. .
  25,   50,  100,  125,  200,  250,  400,   500,   625,
 800, 1000, 1250, 2000, 2500, 4000, 5000, 10000, 20000
. .

The real value set by gpioPWM is

(dutycycle * real range) / range.

...
gpioSetPWMrange(24, 2000); // now 2000 is fully on, 1000 is half on etc.
...
DESC */

/* range: 25-40000 */

#define PI_MIN_DUTYCYCLE_RANGE        25
#define PI_MAX_DUTYCYCLE_RANGE     40000



/* FUNC */
int gpioGetPWMrange(unsigned user_gpio);
/* DESC
Returns the dutycycle range used for the gpio if OK, otherwise
PI_BAD_USER_GPIO.
DESC */



/* FUNC */
int gpioGetPWMrealRange(unsigned user_gpio);
/* DESC
Returns the real range used for the gpio if OK, otherwise
PI_BAD_USER_GPIO.
DESC */



/* FUNC */
int gpioSetPWMfrequency(unsigned user_gpio, unsigned frequency);
/* DESC
Sets the frequency in hertz to be used for the gpio.

Returns the numerically closest frequency if OK, otherwise
PI_BAD_USER_GPIO.

The selectable frequencies depend upon the sample rate which
may be 1, 2, 4, 5, 8, or 10 microseconds (default 5).

Each gpio can be independently set to one of 18 different PWM
frequencies.

If PWM is currently active on the gpio it will be
switched off and then back on at the new frequency.

The frequencies for each sample rate are:

. .
                       Hertz

       1: 40000 20000 10000 8000 5000 4000 2500 2000 1600
           1250  1000   800  500  400  250  200  100   50

       2: 20000 10000  5000 4000 2500 2000 1250 1000  800
            625   500   400  250  200  125  100   50   25

       4: 10000  5000  2500 2000 1250 1000  625  500  400
            313   250   200  125  100   63   50   25   13
sample
 rate
 (us)  5:  8000  4000  2000 1600 1000  800  500  400  320
            250   200   160  100   80   50   40   20   10

       8:  5000  2500  1250 1000  625  500  313  250  200
            156   125   100   63   50   31   25   13    6

      10:  4000  2000  1000  800  500  400  250  200  160
            125   100    80   50   40   25   20   10    5
. .
DESC */



/* FUNC */
int gpioGetPWMfrequency(unsigned user_gpio);
/* DESC
Returns the frequency (in hertz) used for the gpio if OK, otherwise
PI_BAD_USER_GPIO.
DESC */



/* FUNC */
int gpioServo(unsigned user_gpio, unsigned pulsewidth);
/* DESC
Starts servo pulses on the gpio, 0 (off), 500 (most anti-clockwise) to
2500 (most clockwise).

Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_BAD_PULSEWIDTH.

The range supported by servos varies and should probably be determined
by experiment.  A value of 1500 should always be safe and represents
the mid-point of rotation.  You can DAMAGE a servo if you command it
to move beyond its limits.

The following causes an on pulse of 1500 microseconds duration to be
transmitted on gpio 17 at a rate of 50 times per second. This will
 command a servo connected to gpio 17 to rotate to its mid-point.

...
gpioServo(17, 1500);
...

OTHER UPDATE RATES:

This function updates servos at 50Hz.  If you wish to use a different
update frequency you will have to use the PWM functions.

. .
PWM Hz    50   100  200  400  500
1E6/Hz 20000 10000 5000 2500 2000
. .

Firstly set the desired PWM frequency using gpioSetPWMfrequency.

Then set the PWM range using gpioSetPWMrange to 1E6/frequency.
Doing this allows you to use units of microseconds when setting
the servo pulse width.

E.g. If you want to update a servo connected to gpio 25 at 400Hz

. .
gpioSetPWMfrequency(25, 400);
gpioSetPWMrange(25, 2500);
. .

Thereafter use the PWM command to move the servo,
e.g. gpioPWM(25, 1500) will set a 1500 us pulse.

DESC */

/* pulsewidth: 0, 500-2500 */

#define PI_SERVO_OFF 0
#define PI_MIN_SERVO_PULSEWIDTH 500
#define PI_MAX_SERVO_PULSEWIDTH 2500


/* FUNC */
int gpioSetAlertFunc(unsigned user_gpio, gpioAlertFunc_t f);
/* DESC
Registers a function to be called (a callback) when the specified
gpio changes state.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO.

One function may be registered per gpio.

The function is passed the gpio, the new level, and the tick.

The alert may be cancelled by passing NULL as the function.

The gpios are sampled at a rate set when the library is started.

If a value isn't specifically set the default of 5 us is used.

The number of samples per second is given in the following table.

. .
              samples
              per sec

         1  1,000,000
         2    500,000
sample   4    250,000
rate     5    200,000
(us)     8    125,000
        10    100,000
. .

Level changes of length less than the sample rate may be missed.

The thread which calls the alert functions is triggered nominally
1000 times per second.  The active alert functions will be called
once per level change since the last time the thread was activated.
i.e. The active alert functions will get all level changes but there
will be a latency.

The tick value is the time stamp of the sample in microseconds, see
gpioTick for more details.

...
void aFunction(int gpio, int level, uint32_t tick)
{
   printf("gpio %d became %d at %d\n", gpio, level, tick);
}

// call aFunction whenever gpio 4 changes state
gpioSetAlertFunc(4, aFunction);
...
DESC */


/* FUNC */
int gpioSetAlertFuncEx(
   unsigned user_gpio, gpioAlertFuncEx_t f, void *userdata);
/* DESC
Registers a function to be called (a callback) when the specified
gpio changes state.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO.

One function may be registered per gpio.

The function is passed the gpio, the new level, the tick, and
the userdata pointer.

Only one of gpioSetAlertFunc or gpioSetAlertFuncEx can be
registered per gpio.

See gpioSetAlertFunc for further details.
DESC */



/* FUNC */
int gpioNotifyOpen(void);
/* DESC
This function requests a free notification handle.

Returns a handle greater than or equal to zero if OK,
otherwise PI_NO_HANDLE.

A notification is a method for being notified of gpio state changes
via a pipe or socket.

Pipe notifications for handle x will be available at the pipe
named /dev/pigpiox (where x is the handle number).  E.g. if the
function returns 15 then the notifications must be read
from /dev/pigpio15.

Socket notifications are returned to the socket which requested the
handle.
DESC */

#define PI_NOTIFY_SLOTS  32



/* FUNC */
int gpioNotifyBegin(unsigned handle, uint32_t bits);
/* DESC
This function starts notifications on a previously opened handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE.

The notification sends state changes for each gpio whose corresponding
bit in bits is set.

Each notification occupies 12 bytes in the fifo and has the
following structure.

. .
typedef struct
{
   uint16_t seqno;
   uint16_t flags;
   uint32_t tick;
   uint32_t level;
} gpioReport_t;
. .

seqno starts at 0 each time the handle is opened and then increments
by one for each report.

flags, if bit 5 is set then bits 0-4 of the flags indicate a gpio
which has had a watchdog timeout.

tick is the number of microseconds since system boot.

level indicates the level of each gpio.

...
// start notifications for gpios 1, 4, 6, 7, 10
// (1234 = 0x04D2 = 0b0000010011010010)

gpioNotifyBegin(0, 1234);
...
DESC */

#define PI_NTFY_FLAGS_WDOG     (1 <<5)
#define PI_NTFY_FLAGS_BIT(x) (((x)<<0)&31)



/* FUNC */
int gpioNotifyPause(unsigned handle);
/* DESC
This function pauses notifications on a previously opened handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE.

Notifications for the handle are suspended until gpioNotifyBegin
is called again.
DESC */



/* FUNC */
int gpioNotifyClose(unsigned handle);
/* DESC
This function stops notifications on a previously opened handle
and releases the handle for reuse.

Returns 0 if OK, otherwise PI_BAD_HANDLE.
DESC */


/* FUNC */
int gpioWaveClear(void);
/* DESC
This function clears all waveforms and any data added by calls to the
gpioWaveAdd* functions.

Returns 0 if OK.
DESC */



/* FUNC */
int gpioWaveAddNew(void);
/* DESC
This function starts a new empty waveform.  You wouldn't normally need
to call this function as it is automatically called after a waveform is
created with the gpioWaveCreate function.

Returns 0 if OK.
DESC */



/* FUNC */
int gpioWaveAddGeneric(unsigned numPulses, gpioPulse_t *pulses);
/* DESC
This function adds a number of pulses to the current waveform.

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_TOO_MANY_PULSES.

The pulses are interleaved in time order within the existing waveform
(if any).

Merging allows the waveform to be built in parts, that is the settings
for gpio#1 can be added, and then gpio#2 etc.

If the added waveform is intended to start after or within the existing
waveform then the first pulse should consist of a delay.
DESC */



/* FUNC */
int gpioWaveAddSerial(unsigned user_gpio,
                      unsigned bbBaud,
                      unsigned offset,
                      unsigned numChar,
                      char *str);
/* DESC
This function adds a waveform representing serial data to the
existing waveform (if any).  The serial data starts offset microseconds
from the start of the waveform.

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD, PI_TOO_MANY_CHARS,
PI_BAD_SER_OFFSET, or PI_TOO_MANY_PULSES.

The serial data is formatted as one start bit, eight data bits, and one
stop bit.

It is legal to add serial data streams with different baud rates to
the same waveform.
DESC */

#define PI_WAVE_BLOCKS     4
#define PI_WAVE_MAX_PULSES (PI_WAVE_BLOCKS * 3000)
#define PI_WAVE_MAX_CHARS  (PI_WAVE_BLOCKS *  256)

#define PI_WAVE_MIN_BAUD      100
#define PI_WAVE_MAX_BAUD      250000

#define PI_WAVE_MAX_MICROS (30 * 60 * 1000000) /* half an hour */

#define PI_MAX_WAVES 512


/* FUNC */
int gpioWaveCreate(void);
/* DESC
This function creates a waveform from the data provided by the prior
calls to the gpioWaveAdd* functions.  Upon success a positive wave id
is returned.

The data provided by the gpioWaveAdd* functions is consumed by this
function.

As many waveforms may be created as there is space available.  The
wave id is passed to gpioWaveTxSend to specify the waveform to transmit.

Normal usage would be

Step 1. gpioWaveClear to clear all waveforms and added data.

Step 2. gpioWaveAdd* calls to supply the waveform data.

Step 3. gpioWaveCreate to create the waveform and get a unique id

Repeat steps 2 and 3 as needed.

Step 4. gpioWaveTxSend with the id of the waveform to transmit.

A waveform comprises one of more pulses.  Each pulse consists of a
gpioPulse_t structure.

typedef struct
{
   uint32_t gpioOn;
   uint32_t gpioOff;
   uint32_t usDelay;
} gpioPulse_t;

The fields specify

1) the gpios to be switched on at the start of the pulse.
2) the gpios to be switched off at the start of the pulse.
3) the delay in microseconds before the next pulse.

Any or all the fields can be zero.  It doesn't make any sense to
set all the fields to zero (the pulse will be ignored).

When a waveform is started each pulse is executed in order with the
specified delay between the pulse and the next.

Returns the new waveform id if OK, otherwise PI_EMPTY_WAVEFORM,
PI_NO_WAVEFORM_ID, PI_TOO_MANY_CBS, or PI_TOO_MANY_OOL.
DESC */


/* FUNC */
int gpioWaveDelete(unsigned wave_id);
/* DESC
This function deletes all created waveforms with ids greater than or
equal to wave_id.

Wave ids are allocated in order, 0, 1, 2, etc.

Returns 0 if OK, otherwise PI_BAD_WAVE_ID.
DESC */


/* FUNC */
int gpioWaveTxStart(unsigned mode); /* DEPRECATED */
/* DESC
This function creates and then transmits a waveform.  The mode
determines whether the waveform is sent once or cycles endlessly.

This function is deprecated and should no longer be used.  Use
gpioWaveCreate/gpioWaveTxSend instead.

Returns the number of DMA control blocks in the waveform if OK,
otherwise PI_BAD_WAVE_MODE.
DESC */

/* FUNC */
int gpioWaveTxSend(unsigned wave_id, unsigned mode);
/* DESC
This function transmits the waveform with id wave_id.  The mode
determines whether the waveform is sent once or cycles endlessly.

Returns the number of DMA control blocks in the waveform if OK,
otherwise PI_BAD_WAVE_ID, or PI_BAD_WAVE_MODE.
DESC */

#define PI_WAVE_MODE_ONE_SHOT 0
#define PI_WAVE_MODE_REPEAT   1



/* FUNC */
int gpioWaveTxBusy(void);
/* DESC
This function checks to see if a waveform is currently being
transmitted.

Returns 1 if a waveform is currently being transmitted, otherwise 0.
DESC */



/* FUNC */
int gpioWaveTxStop(void);
/* DESC
This function aborts the transmission of the current waveform.

Returns 0 if OK.

This function is intended to stop a waveform started in repeat mode.
DESC */



/* FUNC */
int gpioWaveGetMicros(void);
/* DESC
This function returns the length in microseconds of the current
waveform.
DESC */



/* FUNC */
int gpioWaveGetHighMicros(void);
/* DESC
This function returns the length in microseconds of the longest waveform
created since gpioInitialise was called.
DESC */



/* FUNC */
int gpioWaveGetMaxMicros(void);
/* DESC
This function returns the maximum possible size of a waveform in
microseconds.
DESC */



/* FUNC */
int gpioWaveGetPulses(void);
/* DESC
This function returns the length in pulses of the current waveform.
DESC */



/* FUNC */
int gpioWaveGetHighPulses(void);
/* DESC
This function returns the length in pulses of the longest waveform
created since gpioInitialise was called.
DESC */



/* FUNC */
int gpioWaveGetMaxPulses(void);
/* DESC
This function returns the maximum possible size of a waveform in pulses.
DESC */



/* FUNC */
int gpioWaveGetCbs(void);
/* DESC
This function returns the length in DMA control blocks of the current
waveform.
DESC */



/* FUNC */
int gpioWaveGetHighCbs(void);
/* DESC
This function returns the length in DMA control blocks of the longest
waveform created since gpioInitialise was called.
DESC */



/* FUNC */
int gpioWaveGetMaxCbs(void);
/* DESC
This function returns the maximum possible size of a waveform in DMA
control blocks.
DESC */



/* FUNC */
int gpioSerialReadOpen(unsigned user_gpio, unsigned bbBaud);
/* DESC
This function opens a gpio for reading serial data.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD,
or PI_GPIO_IN_USE.

The serial data is returned in a cyclic buffer and is read using
gpioSerialRead().

It is the caller's responsibility to read data from the cyclic buffer
in a timely fashion.
DESC */



/* FUNC */
int gpioSerialRead(unsigned user_gpio, void *buf, size_t bufSize);
/* DESC
This function copies up to bufSize bytes of data read from the
serial cyclic buffer to the buffer starting at buf.

Returns the number of bytes copied if OK, otherwise PI_BAD_USER_GPIO
or PI_NOT_SERIAL_GPIO.
DESC */



/* FUNC */
int gpioSerialReadClose(unsigned user_gpio);
/* DESC
This function closes a gpio for reading serial data.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_SERIAL_GPIO.
DESC */

#define PI_I2C_SLOTS 32
#define PI_SPI_SLOTS 4
#define PI_SER_SLOTS 4


#define PI_NUM_I2C_BUS 2
#define PI_NUM_SPI_CHANNEL 2

#define PI_MAX_I2C_DEVICE_COUNT 1024
#define PI_MAX_SPI_DEVICE_COUNT 1024


/* FUNC */
int i2cOpen(unsigned i2cBus, unsigned i2cAddr, unsigned i2cFlags);
/* DESC
This returns a handle for the device at the address on the I2C bus.

No flags are currently defined.  This parameter should be set to zero.

Returns a handle (>=0) if OK, otherwise PI_BAD_I2C_BUS, PI_BAD_I2C_ADDR,
PI_BAD_FLAGS, PI_NO_HANDLE, or PI_I2C_OPEN_FAILED.
DESC */

/* FUNC */
int i2cClose(unsigned handle);
/* DESC
This closes the I2C device associated with the handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE.
DESC */

/* FUNC */
int i2cReadDevice(unsigned handle, char *buf, unsigned count);
/* DESC
This reads count bytes from the raw device into buf.

Returns count (>0) if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_READ_FAILED.
DESC */

/* FUNC */
int i2cWriteDevice(unsigned handle, char *buf, unsigned count);
/* DESC
This writes count bytes from buf to the raw device.

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
DESC */

/* FUNC */
int i2cWriteQuick(unsigned handle, unsigned bit);
/* DESC
Sends a single bit to the device associated with handle
(smbus 2.0 5.5.1: Quick command).

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
DESC */

/* FUNC */
int i2cWriteByte(unsigned handle, unsigned bVal);
/* DESC
Sends a single byte to the device associated with handle
(smbus 2.0 5.5.2: Send byte).

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
DESC */

/* FUNC */
int i2cReadByte(unsigned handle);
/* DESC
Reads a single byte from the device associated with handle
(smbus 2.0 5.5.3: Receive byte).

Returns the byte read (>=0) if OK, otherwise PI_BAD_HANDLE,
or PI_I2C_READ_FAILED.
DESC */

/* FUNC */
int i2cWriteByteData(unsigned handle, unsigned reg, unsigned bVal);
/* DESC
Writes a single byte to the specified register of the device
associated with handle (smbus 2.0 5.5.4: Write byte).

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
DESC */

/* FUNC */
int i2cWriteWordData(unsigned handle, unsigned reg, unsigned wVal);
/* DESC
Writes a single 16 bit word to the specified register of the device
associated with handle (smbus 2.0 5.5.4: Write word).

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
DESC */

/* FUNC */
int i2cReadByteData(unsigned handle, unsigned reg);
/* DESC
Reads a single byte from the specified register of the device
associated with handle (smbus 2.0 5.5.5: Read byte).

Returns the byte read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.
DESC */

/* FUNC */
int i2cReadWordData(unsigned handle, unsigned reg);
/* DESC
Reads a single 16 bit word from the specified register of the device
associated with handle (smbus 2.0 5.5.5: Read word).

Returns the word read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.
DESC */

/* FUNC */
int i2cProcessCall(unsigned handle, unsigned reg, unsigned wVal);
/* DESC
Writes 16 bits of data to the specified register of the device
associated with handle and and reads 16 bits of data in return
(smbus 2.0 5.5.6: Process call).

Returns the word read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.
DESC */

/* FUNC */
int i2cWriteBlockData(
unsigned handle, unsigned reg, char *buf, unsigned count);
/* DESC
Writes up to 32 bytes to the specified register of the device
associated with handle (smbus 2.0 5.5.7: Block write).

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
DESC */

/* FUNC */
int i2cReadBlockData(unsigned handle, unsigned reg, char *buf);
/* DESC
Reads a block of up to 32 bytes from the specified register of
the device associated with handle (smbus 2.0 5.5.7: Block read).

The amount of returned data is set by the device.

Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.
DESC */

/* FUNC */
int i2cBlockProcessCall(
unsigned handle, unsigned reg, char *buf, unsigned count);
/* DESC
Writes data bytes to the specified register of the device
associated with handle and reads a device specified number
of bytes of data in return (smbus 2.0 5.5.8:
Block write-block read).

Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

The smbus 2.0 documentation states that a minimum of 1 byte may be
sent and a minimum of 1 byte may be received.  The total number of
bytes sent/received must be 32 or less.
DESC */


/* FUNC */
int i2cReadI2CBlockData(
unsigned handle, unsigned reg, char *buf, unsigned count);
/* DESC
Reads count bytes from the specified register of the device
associated with handle .  The count may be 1-32.

Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.
DESC */


/* FUNC */
int i2cWriteI2CBlockData(
unsigned handle, unsigned reg, char *buf, unsigned count);
/* DESC
Writes data bytes to the specified register of the device
associated with handle .  1-32 bytes may be written.

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
DESC */

/* FUNC */
int spiOpen(unsigned spiChan, unsigned spiBaud, unsigned spiFlags);
/* DESC
This function returns a handle for the SPI device on the channel
Data will be transferred at baud bits per second.

The least significant two bits of flags define the SPI mode.

. .
Mode POL PHA
 0    0   0
 1    0   1
 2    1   0
 3    1   1
. .

The other bits in flags should be set to zero.

Returns a handle (>=0) if OK, otherwise PI_BAD_SPI_CHANNEL,
PI_BAD_SPI_SPEED, PI_BAD_FLAGS, or PI_SPI_OPEN_FAILED.
DESC */

/* FUNC */
int spiClose(unsigned handle);
/* DESC
This functions closes the SPI device identified by the handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE.
DESC */

/* FUNC */
int spiRead(unsigned handle, char *buf, unsigned count);
/* DESC
This function reads count bytes of data from the SPI
device associated with the handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or
PI_SPI_XFER_FAILED.
DESC */

/* FUNC */
int spiWrite(unsigned handle, char *buf, unsigned count);
/* DESC
This function writes count bytes of data from buf to the SPI
device associated with the handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or
PI_SPI_XFER_FAILED.
DESC */

/* FUNC */
int spiXfer(unsigned handle, char *txBuf, char *rxBuf, unsigned count);
/* DESC
This function transfers count bytes of data from txBuf to the SPI
device associated with the handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or
PI_SPI_XFER_FAILED.

The data read from the device is written to rxBuf.
DESC */

/* FUNC */
int serOpen(char *tty, unsigned serBaud, unsigned serFlags);
/* DESC
This function open the serial device named tty at baud
bits per second.

No flags are currently defined.  This parameter should be set to zero.

Returns a handle (>=0) if OK, otherwise PI_NO_HANDLE, or
PI_SER_OPEN_FAILED.
DESC */

/* FUNC */
int serClose(unsigned handle);
/* DESC
This function closes the serial device associated with handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE.
DESC */

/* FUNC */
int serWriteByte(unsigned handle, unsigned bVal);
/* DESC
This function writes bVal to the serial port associated with handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_SER_WRITE_FAILED.
DESC */

/* FUNC */
int serReadByte(unsigned handle);
/* DESC
This function reads a byte from the serial port associated with handle.

Returns the read byte (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_SER_READ_NO_DATA, or PI_SER_READ_FAILED.
DESC */

/* FUNC */
int serWrite(unsigned handle, char *buf, unsigned count);
/* DESC
This function writes count bytes from buf to the the serial port
associated with handle.

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_SER_WRITE_FAILED.
DESC */

/* FUNC */
int serRead(unsigned handle, char *buf, unsigned count);
/* DESC
This function reads count bytes from the the serial port
associated with handle and writes them to buf.

Returns the number of bytes read (>0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, PI_SER_READ_NO_DATA, or PI_SER_WRITE_FAILED.
DESC */

/* FUNC */
int serDataAvailable(unsigned handle);
/* DESC
This function returns the number of bytes of serial data available
to be read from the serial device associated with handle.

Returns the number of bytes of data available (>=0) if OK,
otherwise PI_BAD_HANDLE.
DESC */


/* FUNC */
int gpioTrigger(unsigned user_gpio, unsigned pulseLen, unsigned level);
/* DESC
This function sends a trigger pulse to a gpio.  The gpio is set to
level for pulseLen microseconds and then reset to not level.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_LEVEL,
or PI_BAD_PULSELEN.
DESC */

#define PI_MAX_PULSELEN 50


/* FUNC */
int gpioSetWatchdog(unsigned user_gpio, unsigned timeout);
/* DESC
Sets a watchdog for a gpio.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_BAD_WDOG_TIMEOUT.

The watchdog is nominally in milliseconds.

One watchdog may be registered per gpio.

The watchdog may be cancelled by setting timeout to 0.

If no level change has been detected for the gpio for timeout
milliseconds:-

1) any registered alert function for the gpio is called with
   the level set to PI_TIMEOUT. 
2) any notification for the gpio has a report written to the
   fifo with the flags set to indicate a watchdog timeout.

...
void aFunction(int gpio, int level, uint32_t tick)
{
   printf("gpio %d became %d at %d\n", gpio, level, tick);
}

// call aFunction whenever gpio 4 changes state
gpioSetAlertFunc(4, aFunction);

//  or approximately every 5 millis
gpioSetWatchdogTimeout(4, 5);
...

DESC */

/* timeout: 0-60000 */

#define PI_MIN_WDOG_TIMEOUT 0
#define PI_MAX_WDOG_TIMEOUT 60000



/* FUNC */
int gpioSetGetSamplesFunc(gpioGetSamplesFunc_t f, uint32_t bits);
/* DESC
Registers a function to be called (a callback) every millisecond
with the latest gpio samples.

Returns 0 if OK.

The function is passed a pointer to the samples and the number
of samples.

Only one function can be registered.

The callback may be cancelled by passing NULL as the function.

The samples returned will be the union of bits, plus any active alerts,
plus any active notifications.

e.g.  if there are alerts for gpios 7, 8, and 9, notifications for gpios
8, 10, 23, 24, and bits is (1<<23)|(1<<17) then samples for gpios
7, 8, 9, 10, 17, 23, and 24 will be reported.
DESC */



/* FUNC */
int gpioSetGetSamplesFuncEx(
   gpioGetSamplesFuncEx_t f, uint32_t bits, void *userdata);
/* DESC
Registers a function to be called (a callback) every millisecond
with the latest gpio samples.

Returns 0 if OK.

The function is passed a pointer to the samples, the number
of samples, and the userdata pointer.

Only one of gpioGetSamplesFunc or gpioGetSamplesFuncEx can be
registered.

See gpioSetGetSamplesFunc for further details.
DESC */



/* FUNC */
int gpioSetTimerFunc(unsigned timer, unsigned millis, gpioTimerFunc_t f);
/* DESC
Registers a function to be called (a callback) every millis milliseconds.

Returns 0 if OK, otherwise PI_BAD_TIMER, PI_BAD_MS, or PI_TIMER_FAILED.

10 timers are supported numbered 0 to 9.

One function may be registered per timer.

The timer may be cancelled by passing NULL as the function.

...
void bFunction(void)
{
   printf("two seconds have elapsed\n");
}

// call bFunction every 2000 milliseconds
gpioSetTimerFunc(0, 2000, bFunction);
...
DESC */

/* timer: 0-9 */

#define PI_MIN_TIMER 0
#define PI_MAX_TIMER 9

/* millis: 10-60000 */

#define PI_MIN_MS 10
#define PI_MAX_MS 60000



/* FUNC */
int gpioSetTimerFuncEx(
   unsigned timer, unsigned millis, gpioTimerFuncEx_t f, void *userdata);
/* DESC
Registers a function to be called (a callback) every millis milliseconds.

Returns 0 if OK, otherwise PI_BAD_TIMER, PI_BAD_MS, or PI_TIMER_FAILED.

The function is passed the userdata pointer.

Only one of gpioSetTimerFunc or gpioSetTimerFuncEx can be
registered per timer.

See gpioSetTimerFunc for further details.
DESC */



/* FUNC */
pthread_t *gpioStartThread(gpioThreadFunc_t f, void *arg);
/* DESC
Starts a new thread of execution with f as the main routine.

Returns a pointer to pthread_t if OK, otherwise NULL.

The function is passed the single argument arg.

The thread can be cancelled by passing the pointer to pthread_t to
gpioStopThread().

...
#include <stdio.h>
#include <pigpio.h>

void *myfunc(void *arg)
{
   while (1)
   {
      printf("%s\n", arg);
      sleep(1);
   }
}

int main(int argc, char *argv[])
{
   pthread_t *p1, *p2, *p3;

   if (gpioInitialise() < 0) return 1;

   p1 = gpioStartThread(myfunc, "thread 1"); sleep(3);

   p2 = gpioStartThread(myfunc, "thread 2"); sleep(3);

   p3 = gpioStartThread(myfunc, "thread 3"); sleep(3);

   gpioStopThread(p3); sleep(3);

   gpioStopThread(p2); sleep(3);

   gpioStopThread(p1); sleep(3);

   gpioTerminate();
}
...
DESC */



/* FUNC */
void gpioStopThread(pthread_t *pth);
/* DESC
Cancels the thread pointed at by pth.

No value is returned.

The thread to be stopped should have been started with gpioStartThread().
DESC */


/* FUNC */
int gpioStoreScript(char *script);
/* DESC
This function stores a null terminated script for later execution.

The function returns a script id if the script is valid,
otherwise PI_BAD_SCRIPT.
DESC */

#define PI_MAX_SCRIPTS       32

#define PI_MAX_SCRIPT_TAGS   50
#define PI_MAX_SCRIPT_VARS  150
#define PI_MAX_SCRIPT_PARAMS 10


/* FUNC */
int gpioRunScript(unsigned script_id, unsigned numParam, uint32_t *param);
/* DESC
This function runs a stored script.

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID, or
PI_TOO_MANY_PARAM.

param is an array of up to 10 parameters which may be referenced in
the script as param 0 to param 9.
DESC */



/* FUNC */
int gpioScriptStatus(unsigned script_id, uint32_t *param);
/* DESC
This function returns the run status of a stored script as well as
the current values of parameters 0 to 9.

The function returns greater than or equal to 0 if OK,
otherwise PI_BAD_SCRIPT_ID.

The run status may be

PI_SCRIPT_HALTED
PI_SCRIPT_RUNNING
PI_SCRIPT_WAITING
PI_SCRIPT_FAILED

The current value of script parameters 0 to 9 are returned in param.
DESC */

/* script status */

#define PI_SCRIPT_HALTED  0
#define PI_SCRIPT_RUNNING 1
#define PI_SCRIPT_WAITING 2
#define PI_SCRIPT_FAILED  3



/* FUNC */
int gpioStopScript(unsigned script_id);
/* DESC
This function stops a running script.

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.
DESC */



/* FUNC */
int gpioDeleteScript(unsigned script_id);
/* DESC
This function deletes a stored script.

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.
DESC */



/* FUNC */
int gpioSetSignalFunc(unsigned signum, gpioSignalFunc_t f);
/* DESC
Registers a function to be called (a callback) when a signal occurs.

Returns 0 if OK, otherwise PI_BAD_SIGNUM.

The function is passed the signal number.

One function may be registered per signal.

The callback may be cancelled by passing NULL.

By default all signals are treated as fatal and cause the library
to call gpioTerminate and then exit.
DESC */

/* signum: 0-63 */

#define PI_MIN_SIGNUM 0
#define PI_MAX_SIGNUM 63



/* FUNC */
int gpioSetSignalFuncEx(
   unsigned signum, gpioSignalFuncEx_t f, void *userdata);
/* DESC
Registers a function to be called (a callback) when a signal occurs.

Returns 0 if OK, otherwise PI_BAD_SIGNUM.

The function is passed the signal number and the userdata pointer.

Only one of gpioSetSignalFunc or gpioSetSignalFuncEx can be
registered per signal.

See gpioSetSignalFunc for further details.
DESC */



/* FUNC */
uint32_t gpioRead_Bits_0_31(void);
/* DESC
Returns the current level of gpios 0-31.
DESC */



/* FUNC */
uint32_t gpioRead_Bits_32_53(void);
/* DESC
Returns the current level of gpios 32-53.
DESC */



/* FUNC */
int gpioWrite_Bits_0_31_Clear(uint32_t bits);
/* DESC
Clears gpios 0-31 if the corresponding bit in bits is set.

Returns 0 if OK.

...
// To clear (set to 0) gpios 4, 7, and 15
gpioWrite_Bits_0_31_Clear( (1<<4) | (1<<7) | (1<<15) );
...
DESC */



/* FUNC */
int gpioWrite_Bits_32_53_Clear(uint32_t bits);
/* DESC
Clears gpios 32-53 if the corresponding bit (0-21) in bits is set.

Returns 0 if OK.
DESC */



/* FUNC */
int gpioWrite_Bits_0_31_Set(uint32_t bits);
/* DESC
Sets gpios 0-31 if the corresponding bit in bits is set.

Returns 0 if OK.
DESC */



/* FUNC */
int gpioWrite_Bits_32_53_Set(uint32_t bits);
/* DESC
Sets gpios 32-53 if the corresponding bit (0-21) in bits is set.

Returns 0 if OK.

...
// To set (set to 1) gpios 32, 40, and 53
gpioWrite_Bits_32_53_Set( (1<<(32-32)) | (1<<(40-32)) | (1<<(53-32)) );
...
DESC */


/* FUNC */
int gpioTime(unsigned timetype, int *seconds, int *micros);
/* DESC
Updates the seconds and micros variables with the current time.

Returns 0 if OK, otherwise PI_BAD_TIMETYPE.

If timetype is PI_TIME_ABSOLUTE updates seconds and micros with the
number of seconds and microseconds since the epoch (1st January 1970).

If timetype is PI_TIME_RELATIVE updates seconds and micros with the
number of seconds and microseconds since the library was initialised.

...
int secs, mics;

// print the number of seconds since the library was started
gpioTime(PI_TIME_RELATIVE, &secs, &mics);
printf("library started %d.%03d seconds ago\n", secs, mics/1000);
...
DESC */

/* timetype: 0-1 */

#define PI_TIME_RELATIVE 0
#define PI_TIME_ABSOLUTE 1



/* FUNC */
int gpioSleep(unsigned timetype, int seconds, int micros);
/* DESC
Sleeps for the number of seconds and microseconds specified by seconds
and micros.

Returns 0 if OK, otherwise PI_BAD_TIMETYPE, PI_BAD_SECONDS,
or PI_BAD_MICROS.

If timetype is PI_TIME_ABSOLUTE the sleep ends when the number of seconds
and microseconds since the epoch (1st January 1970) has elapsed.  System
clock changes are taken into account.

If timetype is PI_TIME_RELATIVE the sleep is for the specified number
of seconds and microseconds.  System clock changes do not effect the
sleep length.

For short delays (say, 50 microseonds or less) use gpioDelayMicroseconds.

...
gpioSleep(PI_TIME_RELATIVE, 2, 500000); // sleep for 2.5 seconds

gpioSleep(PI_TIME_RELATIVE, 0, 100000); // sleep for 1/10th of a second

gpioSleep(PI_TIME_RELATIVE, 60, 0);     // sleep for one minute
...
DESC */



/* FUNC */
uint32_t gpioDelay(uint32_t micros);
/* DESC
Delays for at least the number of microseconds specified by micros.

Returns the actual length of the delay in microseconds.
DESC */

#define PI_MAX_MICS_DELAY 1000000 /* 1 second */
#define PI_MAX_MILS_DELAY 60000   /* 60 seconds */

/* FUNC */
uint32_t gpioTick(void);
/* DESC
Returns the current system tick.

Tick is the number of microseconds since system boot.

As tick is an unsigned 32 bit quantity it wraps around after
2^32 microseconds, which is approximately 1 hour 12 minutes.

You don't need to worry about the wrap around as long as you
take a tick (uint32_t) from another tick, i.e. the following
code will always provide the correct difference.

...
uint32_t startTick, endTick;
int diffTick;

startTick = gpioTick();

// do some processing

endTick = gpioTick();

diffTick = endTick - startTick;

printf("some processing took %d microseconds\n", diffTick);
...
DESC */



/* FUNC */
unsigned gpioHardwareRevision(void);
/* DESC
Returns the hardware revision.

If the hardware revision can not be found or is not a valid hexadecimal
number the function returns 0.

The hardware revision is the last 4 characters on the Revision line of
/proc/cpuinfo.

The revision number can be used to determine the assignment of gpios
to pins.

There are at least two types of board.

Type 1 has gpio 0 on P1-3, gpio 1 on P1-5, and gpio 21 on P1-13.

Type 2 has gpio 2 on P1-3, gpio 3 on P1-5, gpio 27 on P1-13, and
gpios 28-31 on P5.

Type 1 boards have hardware revision numbers of 2 and 3.

Type 2 boards have hardware revision numbers of 4, 5, 6, and 15.

for "Revision       : 0002" the function returns 2. 
for "Revision       : 000f" the function returns 15. 
for "Revision       : 000g" the function returns 0.
DESC */



/* FUNC */
unsigned gpioVersion(void);
/* DESC
Returns the pigpio version.
DESC */



/* FUNC */
int gpioCfgBufferSize(unsigned millis);
/* DESC
Configures pigpio to buffer millis milliseconds of gpio samples.

The default setting is 120 milliseconds.

The intention is to allow for bursts of data and protection against
other processes hogging cpu time.

I haven't seen a process locked out for more than 100 milliseconds.

Making the buffer bigger uses a LOT of memory at the more frequent
sampling rates as shown in the following table in MBs.

. .
                     buffer milliseconds
               120 250 500 1sec 2sec 4sec 8sec

         1      16  31  55  107  ---  ---  ---
         2      10  18  31   55  107  ---  ---
sample   4       8  12  18   31   55  107  ---
 rate    5       8  10  14   24   45   87  ---
 (us)    8       6   8  12   18   31   55  107
        10       6   8  10   14   24   45   87
. .
DESC */

/* millis */

#define PI_BUF_MILLIS_MIN 100
#define PI_BUF_MILLIS_MAX 10000



/* FUNC */
int gpioCfgClock(
   unsigned micros, unsigned peripheral, unsigned source);
/* DESC
Configures pigpio to use a sample rate of micros microseconds,
permitted values are 1, 2, 4, 5, 8 and 10.

The timings are provided by the specified peripheral (PWM or PCM)
using the frequency source (OSC or PLLD).

The default setting is 5 microseconds using the PCM peripheral
with the PLLD source.

The approximate CPU percentage used for each sample rate is:

. .
sample  cpu
 rate    %

  1     25
  2     16
  4     11
  5     10
  8     15
 10     14
. .

A sample rate of 5 microseconds seeems to be the sweet spot.
DESC */

/* micros: 1, 2, 4, 5, 8, or 10 */

/* peripheral: 0-1 */

#define PI_CLOCK_PWM 0
#define PI_CLOCK_PCM 1

/* source: 0-1 */

#define PI_CLOCK_OSC  0
#define PI_CLOCK_PLLD 1



/* FUNC */
int gpioCfgDMAchannel(unsigned DMAchannel); /* DEPRECATED */
/* DESC
Configures pigpio to use the specified DMA channel.

The default setting is to use channel 14.
DESC */

/* DMA channel: 0-14 */

#define PI_MIN_DMA_CHANNEL 0
#define PI_MAX_DMA_CHANNEL 14



/* FUNC */
int gpioCfgDMAchannels(
   unsigned primaryChannel, unsigned secondaryChannel);
/* DESC
Configures pigpio to use the specified DMA channels.

The default setting is to use channel 14 for the primary channel and
channel 5 for the secondary channel.
DESC */

#define PI_MAX_PRIMARY_CHANNEL   14
#define PI_MAX_SECONDARY_CHANNEL  6



/* FUNC */
int gpioCfgPermissions(uint64_t updateMask);
/* DESC
Configures pigpio to only allow updates (writes or mode changes) for the
gpios specified by the mask.

The default setting depends upon the board revision (Type 1 or Type 2).
The user gpios are added to the mask.  If the board revision is not
recognised then the mask is formed by or'ing the bits for the two
board revisions.

Unknown board 
. .
PI_DEFAULT_UPDATE_MASK_R0 0xFBE6CF9F 
. .
Type 1 board 
. .
PI_DEFAULT_UPDATE_MASK_R1 0x03E6CF93 
. .
Type 2 board 
. .
PI_DEFAULT_UPDATE_MASK_R2 0xFBC6CF9C
. .
DESC */



/* FUNC */
int gpioCfgSocketPort(unsigned port);
/* DESC
Configures pigpio to use the specified socket port.

The default setting is to use port 8888.
DESC */

/* port: 1024-9999 */

#define PI_MIN_SOCKET_PORT 1024
#define PI_MAX_SOCKET_PORT 32000



/* FUNC */
int gpioCfgInterfaces(unsigned ifFlags);
/* DESC
Configures pigpio support of the fifo and socket interfaces.

The default setting is that both interfaces are enabled.
DESC */

/* ifFlags: */

#define PI_DISABLE_FIFO_IF 1
#define PI_DISABLE_SOCK_IF 2

/* FUNC */
int gpioCfgInternals(unsigned cfgWhat, int cfgVal);
/* DESC
Used to tune internal settings.

Not intended for general use.
DESC */

/* FUNC */
int rawWaveAddSPI(
   rawSPI_t *spi,
   unsigned offset,
   unsigned spiSS,
   char *buf,
   unsigned spiTxBits,
   unsigned spiBitFirst,
   unsigned spiBitLast,
   unsigned spiBits);
/* DESC
This function adds a waveform representing SPI data to the
existing waveform (if any).

The SPI data starts offset microseconds from the start of the
waveform.  spiSS is the slave select gpio.  spiBits bits
are transferred.  spiTxBits are transmitted starting at the
first bit.  The bits to transmit are read, most significant bit
first, from buf. Gpio reads are made from spiBitFirst to
spiBitLast.

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_BAD_USER_GPIO, PI_BAD_SER_OFFSET, or PI_TOO_MANY_PULSES.

Not intended for general use.
DESC */

/* FUNC */
int rawWaveAddGeneric(unsigned numPulses, rawWave_t *pulses);
/* DESC
This function adds a number of pulses to the current waveform.

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_TOO_MANY_PULSES.

The advantage of this function over gpioWaveAddGeneric is that it
allows the setting of the flags field.

The pulses are interleaved in time order within the existing waveform
(if any).

Merging allows the waveform to be built in parts, that is the settings
for gpio#1 can be added, and then gpio#2 etc.

If the added waveform is intended to start after or within the existing
waveform then the first pulse should consist of a delay.

Not intended for general use.
DESC */

/* FUNC */
unsigned rawWaveCB(void);
/* DESC
Returns the number of the cb being currently output.

Not intended for general use.
DESC */

/* FUNC */
rawCbs_t *rawWaveCBAdr(int n);
/* DESC
Return the Linux address of contol block n.

Not intended for general use.
DESC */

/* FUNC */
uint32_t rawWaveGetOut(int pos);
/* DESC
Gets the wave output parameter stored at pos.

Not intended for general use.
DESC */


/* FUNC */
void rawWaveSetOut(int pos, uint32_t value);
/* DESC
Sets the wave output parameter stored at pos to value.

Not intended for general use.
DESC */

/* FUNC */
uint32_t rawWaveGetIn(int pos);
/* DESC
Gets the wave input value parameter stored at pos.

Not intended for general use.
DESC */


/* FUNC */
void rawWaveSetIn(int pos, uint32_t value);
/* DESC
Sets the wave input value stored at pos to value.

Not intended for general use.
DESC */

/* FUNC */
int getBitInBytes(int bitPos, char *buf, int numBits);
/* DESC
Returns the value of the bit bitPos bits from the start of buf.  Returns
0 if bitPos is greater than or equal to numBits.
DESC */

/* FUNC */
void putBitInBytes(int bitPos, char *buf, int bit);
/* DESC
Sets the bit bitPos bits from the start of buf to bit.
DESC */

/* FUNC */
double time_time(void);
/* DESC
Return the current time in seconds since the Epoch.
DESC */


/* FUNC */
void time_sleep(double seconds);
/* DESC
Delay execution for a given number of seconds
DESC */


/* FUNC */
void rawDumpWave(void);
/* DESC
Used to print a readable version of the current waveform to stderr.

Not intended for general use.
DESC */


/* FUNC */
void rawDumpScript(unsigned script_id);
/* DESC
Used to print a readable version of a script to stderr.

Not intended for general use.
DESC */


#ifdef __cplusplus
}
#endif

/* PARAMS

*arg::
A pointer to a void object passed to a thread started by gpioStartThread.

bbBaud::100-250000
The baud rate used for the transmission and reception of bit banged
serial data.

...
PI_WAVE_MIN_BAUD 100
PI_WAVE_MAX_BAUD 250000
...

bit::0-1
A value of 0 or 1.

bitPos::
A bit position within a byte or word.  The least significant bit is
position 0.

bits::
A value used to select gpios.  If bit n of bits is set then gpio n is
selected.

A convenient way to set bit n is to or in (1<<n).

e.g. to select bits 5, 9, 23 you could use (1<<5) | (1<<9) | (1<<23).

*buf::
A buffer to hold data being sent or being received.

bufSize::
The size in bytes of a buffer.

bVal::0-255
A byte value, 0-255.

char::
A single character, an 8 bit quantity able to store 0-255.

count::
The number of bytes to be transferred in aN I2C, SPI, or Serial
command.

DMAchannel::0-14
...
#define PI_MIN_DMA_CHANNEL 0
#define PI_MAX_DMA_CHANNEL 14
...
double::
A floating point number.

dutycycle::0-range
A number representing the ratio of on time to off time for PWM.

The number may vary between 0 and range (default 255) where
0 is off and 255 is fully on.

f::
A function.

frequency::0-
The number of times a gpio is swiched on and off per second.  This
can be set per gpio and may be as little as 5Hz or as much as
40KHz.  The gpio will be on for a proportion of the time as defined
by its dutycycle.

gpio::0-53
A Broadcom numbered gpio, in the range 0-53.

gpioAlertFuncEx_t::
. .
typedef void (*gpioAlertFuncEx_t)
   (int gpio, int level, uint32_t tick, void *userdata);
. .

gpioAlertFunc_t::
. .
typedef void (*gpioAlertFunc_t) (int gpio, int level, uint32_t tick);
. .

gpioExtent_t::
. .
typedef struct
{
   size_t size;
   void *ptr;
   uint32_t data;
} gpioExtent_t;
. .

gpioGetSamplesFuncEx_t::
. .
typedef void (*gpioGetSamplesFuncEx_t)
   (const gpioSample_t *samples, int numSamples, void *userdata);
. .

gpioGetSamplesFunc_t::
. .
typedef void (*gpioGetSamplesFunc_t)
   (const gpioSample_t *samples, int numSamples);
. .

gpioPulse_t::
. .
typedef struct
{
   uint32_t gpioOn;
   uint32_t gpioOff;
   uint32_t usDelay;
} gpioPulse_t;
. .

gpioSignalFuncEx_t::
. .
typedef void (*gpioSignalFuncEx_t) (int signum, void *userdata);
. .

gpioSignalFunc_t::
. .
typedef void (*gpioSignalFunc_t) (int signum);
. .

gpioThreadFunc_t::
. .
typedef void *(gpioThreadFunc_t) (void *);
. .

gpioTimerFuncEx_t::
. .
typedef void (*gpioTimerFuncEx_t) (void *userdata);
. .

gpioTimerFunc_t::
. .
typedef void (*gpioTimerFunc_t) (void);
. .

handle::0-
A number referencing an object opened by one of

[*i2cOpen*] 
[*gpioNotifyOpen*] 
[*serOpen*] 
[*spiOpen*]

i2cAddr::0x08-0x77
The address of a device on the I2C bus (0x08 - 0x77)

i2cBus::0-1
An I2C bus, 0 or 1.

i2cFlags::0
Flags which modify an I2C open command.  None are currently defined.

ifFlags::0-3
. .
PI_DISABLE_FIFO_IF 1
PI_DISABLE_SOCK_IF 2
. .

int::
A whole number, negative or positive.

level::0-1
. .
PI_OFF 0
PI_ON 1

PI_CLEAR 0
PI_SET 1

PI_LOW 0
PI_HIGH 1
. .

*micros::
A value representing microseconds.

micros::
A value representing microseconds.

millis::
A value representing milliseconds.

mode::0-7
The operational mode of a gpio, normally INPUT or OUTPUT.
. .
PI_INPUT 0
PI_OUTPUT 1
PI_ALT0 4
PI_ALT1 5
PI_ALT2 6
PI_ALT3 7
PI_ALT4 3
PI_ALT5 2
. .

numBits::

numChar::

numParam::

numPulses::

spiTxBits::

offset::

*param::

peripheral::

port::

pos::

primaryChannel::

*pth::
pthread_t::

pud::0-2
. .
PI_PUD_OFF 0
PI_PUD_DOWN 1
PI_PUD_UP 2
. .
pulseLen::
1-50, the length of a trigger pulse in microseconds.

*pulses::

pulsewidth::0, 500-2500
. .
PI_SERVO_OFF 0
PI_MIN_SERVO_PULSEWIDTH 500
PI_MAX_SERVO_PULSEWIDTH 2500
. .

range::25-40000
. .
PI_MIN_DUTYCYCLE_RANGE 25
PI_MAX_DUTYCYCLE_RANGE 40000
. .

rawCbs_t::
. .
typedef struct // linux/arch/arm/mach-bcm2708/include/mach/dma.h
{
   unsigned long info;
   unsigned long src;
   unsigned long dst;
   unsigned long length;
   unsigned long stride;
   unsigned long next;
   unsigned long pad[2];
} rawCbs_t;
. .

rawSPI_t::
. .
typedef struct
{
   int clk;     // gpio for clock
   int mosi;    // gpio for MOSI
   int miso;    // gpio for MISO
   int ss_pol;  // slave select off state
   int ss_us;   // delay after slave select
   int clk_pol; // clock off state
   int clk_pha; // clock phase
   int clk_us;  // clock micros
} rawSPI_t;
. .

rawWave_t::
. .
typedef struct
{
   uint32_t gpioOn;
   uint32_t gpioOff;
   uint32_t usDelay;
   uint32_t flags;
} rawWave_t;
. .

reg::

*rxBuf::

spiBitFirst::

spiBitLast::

*script::

script_id::
An id of a stored script as returned by gpioStoreScript.

secondaryChannel::

*seconds::

A pointer to a uint32_t to store the second component of
a returned time.

seconds::

serFlags::
Flags which modify a serial open command.  None are currently defined.

signum::0-63
. .
PI_MIN_SIGNUM 0
PI_MAX_SIGNUM 63
. .
size_t::

source::
. .
PI_CLOCK_OSC 0
PI_CLOCK_PLLD 1
. .
speed::
*spi::

spi_chan::
A SPI channel, 0 or 1.

spiFlags::
Flags which modify a SPI open command. The two least significant bits
define the SPI mode.  The other bits are undefined.

spiSS::
*str::
 A pointer to a null terminated ASCII string.
timeout::
. .
PI_MIN_WDOG_TIMEOUT 0
PI_MAX_WDOG_TIMEOUT 60000
. .
timer::
. .
PI_MIN_TIMER 0
PI_MAX_TIMER 9
. .
timetype::
. .
PI_TIME_RELATIVE 0
PI_TIME_ABSOLUTE 1
. .
*tty::
A Pi serial tty device, e.g. /dev/ttyAMA0, /dev/ttyUSB0

*txBuf::

uint32_t::
A whole number in the range 0-4,294,967,295

uint64_t::
A whole number in the range 0-(2^64)-1.

unsigned::
A whole number >= 0.

updateMask::
A 64 bit mask indicating which gpios may be written to by the user.

If gpio#n may be written then bit (1<<n) is set.

user_gpio::
0-31, a Broadcom numbered gpio.

*userdata::

bVal::

value::

void::

wave_id::
A number representing a waveform created by [*gpioWaveCreate*].
what::

PARAMS */

#define PI_CMD_MODES  0
#define PI_CMD_MODEG  1
#define PI_CMD_PUD    2
#define PI_CMD_READ   3
#define PI_CMD_WRITE  4
#define PI_CMD_PWM    5
#define PI_CMD_PRS    6
#define PI_CMD_PFS    7
#define PI_CMD_SERVO  8
#define PI_CMD_WDOG   9
#define PI_CMD_BR1   10
#define PI_CMD_BR2   11
#define PI_CMD_BC1   12
#define PI_CMD_BC2   13
#define PI_CMD_BS1   14
#define PI_CMD_BS2   15
#define PI_CMD_TICK  16
#define PI_CMD_HWVER 17
#define PI_CMD_NO    18
#define PI_CMD_NB    19
#define PI_CMD_NP    20
#define PI_CMD_NC    21
#define PI_CMD_PRG   22
#define PI_CMD_PFG   23
#define PI_CMD_PRRG  24
#define PI_CMD_HELP  25
#define PI_CMD_PIGPV 26
#define PI_CMD_WVCLR 27
#define PI_CMD_WVAG  28
#define PI_CMD_WVAS  29
#define PI_CMD_WVGO  30
#define PI_CMD_WVGOR 31
#define PI_CMD_WVBSY 32
#define PI_CMD_WVHLT 33
#define PI_CMD_WVSM  34
#define PI_CMD_WVSP  35
#define PI_CMD_WVSC  36
#define PI_CMD_TRIG  37
#define PI_CMD_PROC  38
#define PI_CMD_PROCD 39
#define PI_CMD_PROCR 40
#define PI_CMD_PROCS 41
#define PI_CMD_SLRO  42
#define PI_CMD_SLR   43
#define PI_CMD_SLRC  44
#define PI_CMD_PROCP 45
#define PI_CMD_MICS  46
#define PI_CMD_MILS  47
#define PI_CMD_PARSE 48
#define PI_CMD_WVCRE 49
#define PI_CMD_WVDEL 50
#define PI_CMD_WVTX  51
#define PI_CMD_WVTXR 52
#define PI_CMD_WVNEW 53

#define PI_CMD_I2CO  54
#define PI_CMD_I2CC  55
#define PI_CMD_I2CRD 56
#define PI_CMD_I2CWD 57
#define PI_CMD_I2CWQ 58
#define PI_CMD_I2CRS 59
#define PI_CMD_I2CWS 60
#define PI_CMD_I2CRB 61
#define PI_CMD_I2CWB 62
#define PI_CMD_I2CRW 63
#define PI_CMD_I2CWW 64
#define PI_CMD_I2CRK 65
#define PI_CMD_I2CWK 66
#define PI_CMD_I2CRI 67
#define PI_CMD_I2CWI 68
#define PI_CMD_I2CPC 69
#define PI_CMD_I2CPK 70

#define PI_CMD_SPIO  71
#define PI_CMD_SPIC  72
#define PI_CMD_SPIR  73
#define PI_CMD_SPIW  74
#define PI_CMD_SPIX  75

#define PI_CMD_SERO  76
#define PI_CMD_SERC  77
#define PI_CMD_SERRB 78
#define PI_CMD_SERWB 79
#define PI_CMD_SERR  80
#define PI_CMD_SERW  81
#define PI_CMD_SERDA 82

/*
The following command only works on the socket interface.
It returns a spare notification handle.  Notifications for
that handle will be sent to the socket (rather than a
/dev/pigpiox pipe).

The socket should be dedicated to receiving notifications
after this command is issued.
*/

#define PI_CMD_NOIB  99

/* pseudo commands */

#define PI_CMD_SCRIPT 800

#define PI_CMD_ADD   800
#define PI_CMD_AND   801
#define PI_CMD_CALL  802
#define PI_CMD_CMDR  803
#define PI_CMD_CMDW  804
#define PI_CMD_CMP   805
#define PI_CMD_DCR   806
#define PI_CMD_DCRA  807
#define PI_CMD_DIV   808
#define PI_CMD_HALT  809
#define PI_CMD_INR   810
#define PI_CMD_INRA  811
#define PI_CMD_JM    812
#define PI_CMD_JMP   813
#define PI_CMD_JNZ   814
#define PI_CMD_JP    815
#define PI_CMD_JZ    816
#define PI_CMD_TAG   817
#define PI_CMD_LD    818
#define PI_CMD_LDA   819
#define PI_CMD_LDAB  820
#define PI_CMD_MLT   821
#define PI_CMD_MOD   822
#define PI_CMD_NOP   823
#define PI_CMD_OR    824
#define PI_CMD_POP   825
#define PI_CMD_POPA  826
#define PI_CMD_PUSH  827
#define PI_CMD_PUSHA 828
#define PI_CMD_RET   829
#define PI_CMD_RL    830
#define PI_CMD_RLA   831
#define PI_CMD_RR    832
#define PI_CMD_RRA   833
#define PI_CMD_STA   834
#define PI_CMD_STAB  835
#define PI_CMD_SUB   836
#define PI_CMD_SYS   837
#define PI_CMD_WAIT  838
#define PI_CMD_X     839
#define PI_CMD_XA    840
#define PI_CMD_XOR   841

/* error numbers reported by functions */

#define PI_INIT_FAILED       -1 /* gpioInitialise failed                   */
#define PI_BAD_USER_GPIO     -2 /* gpio not 0-31                           */
#define PI_BAD_GPIO          -3 /* gpio not 0-53                           */
#define PI_BAD_MODE          -4 /* mode not 0-7                            */
#define PI_BAD_LEVEL         -5 /* level not 0-1                           */
#define PI_BAD_PUD           -6 /* pud not 0-2                             */
#define PI_BAD_PULSEWIDTH    -7 /* pulsewidth not 0 or 500-2500            */
#define PI_BAD_DUTYCYCLE     -8 /* dutycycle outside set range             */
#define PI_BAD_TIMER         -9 /* timer not 0-9                           */
#define PI_BAD_MS           -10 /* ms not 10-60000                         */
#define PI_BAD_TIMETYPE     -11 /* timetype not 0-1                        */
#define PI_BAD_SECONDS      -12 /* seconds < 0                             */
#define PI_BAD_MICROS       -13 /* micros not 0-999999                     */
#define PI_TIMER_FAILED     -14 /* gpioSetTimerFunc failed                 */
#define PI_BAD_WDOG_TIMEOUT -15 /* timeout not 0-60000                     */
#define PI_NO_ALERT_FUNC    -16 /* DEPRECATED                              */
#define PI_BAD_CLK_PERIPH   -17 /* clock peripheral not 0-1                */
#define PI_BAD_CLK_SOURCE   -18 /* clock source not 0-1                    */
#define PI_BAD_CLK_MICROS   -19 /* clock micros not 1, 2, 4, 5, 8, or 10   */
#define PI_BAD_BUF_MILLIS   -20 /* buf millis not 100-10000                */
#define PI_BAD_DUTYRANGE    -21 /* dutycycle range not 25-40000            */
#define PI_BAD_DUTY_RANGE   -21 /* DEPRECATED (use PI_BAD_DUTYRANGE)       */
#define PI_BAD_SIGNUM       -22 /* signum not 0-63                         */
#define PI_BAD_PATHNAME     -23 /* can't open pathname                     */
#define PI_NO_HANDLE        -24 /* no handle available                     */
#define PI_BAD_HANDLE       -25 /* unknown notify handle                   */
#define PI_BAD_IF_FLAGS     -26 /* ifFlags > 3                             */
#define PI_BAD_CHANNEL      -27 /* DMA channel not 0-14                    */
#define PI_BAD_PRIM_CHANNEL -27 /* DMA primary channel not 0-14            */
#define PI_BAD_SOCKET_PORT  -28 /* socket port not 1024-32000              */
#define PI_BAD_FIFO_COMMAND -29 /* unrecognized fifo command               */
#define PI_BAD_SECO_CHANNEL -30 /* DMA secondary channel not 0-6           */
#define PI_NOT_INITIALISED  -31 /* function called before gpioInitialise   */
#define PI_INITIALISED      -32 /* function called after gpioInitialise    */
#define PI_BAD_WAVE_MODE    -33 /* waveform mode not 0-1                   */
#define PI_BAD_CFG_INTERNAL -34 /* bad parameter in gpioCfgInternals call  */
#define PI_BAD_WAVE_BAUD    -35 /* baud rate not 100-250000                */
#define PI_TOO_MANY_PULSES  -36 /* waveform has too many pulses            */
#define PI_TOO_MANY_CHARS   -37 /* waveform has too many chars             */
#define PI_NOT_SERIAL_GPIO  -38 /* no serial read in progress on gpio      */
#define PI_BAD_SERIAL_STRUC -39 /* bad (null) serial structure parameter   */
#define PI_BAD_SERIAL_BUF   -40 /* bad (null) serial buf parameter         */
#define PI_NOT_PERMITTED    -41 /* gpio operation not permitted            */
#define PI_SOME_PERMITTED   -42 /* one or more gpios not permitted         */
#define PI_BAD_WVSC_COMMND  -43 /* bad WVSC subcommand                     */
#define PI_BAD_WVSM_COMMND  -44 /* bad WVSM subcommand                     */
#define PI_BAD_WVSP_COMMND  -45 /* bad WVSP subcommand                     */
#define PI_BAD_PULSELEN     -46 /* trigger pulse length > 50               */
#define PI_BAD_SCRIPT       -47 /* invalid script                          */
#define PI_BAD_SCRIPT_ID    -48 /* unknown script id                       */
#define PI_BAD_SER_OFFSET   -49 /* add serial data offset > 30 minutes     */
#define PI_GPIO_IN_USE      -50 /* gpio already in use                     */
#define PI_BAD_SERIAL_COUNT -51 /* must read at least a byte at a time     */
#define PI_BAD_PARAM_NUM    -52 /* script parameter must be 0-9            */
#define PI_DUP_TAG          -53 /* script has duplicate tag                */
#define PI_TOO_MANY_TAGS    -54 /* script has too many tags                */
#define PI_BAD_SCRIPT_CMD   -55 /* illegal script command                  */
#define PI_BAD_VAR_NUM      -56 /* script variable must be 0-149           */
#define PI_NO_SCRIPT_ROOM   -57 /* no more room for scripts                */
#define PI_NO_MEMORY        -58 /* can't allocate temporary memory         */
#define PI_SOCK_READ_FAILED -59 /* socket read failed                      */
#define PI_SOCK_WRIT_FAILED -60 /* socket write failed                     */
#define PI_TOO_MANY_PARAM   -61 /* too many script parameters > 10         */
#define PI_NOT_HALTED       -62 /* script already running or failed        */
#define PI_BAD_TAG          -63 /* script has unresolved tag               */
#define PI_BAD_MICS_DELAY   -64 /* bad MICS delay (too large)              */
#define PI_BAD_MILS_DELAY   -65 /* bad MILS delay (too large)              */
#define PI_BAD_WAVE_ID      -66 /* non existent wave id                    */
#define PI_TOO_MANY_CBS     -67 /* No more CBs for waveform                */
#define PI_TOO_MANY_OOL     -68 /* No more OOL for waveform                */
#define PI_EMPTY_WAVEFORM   -69 /* attempt to create an empty waveform     */
#define PI_NO_WAVEFORM_ID   -70 /* no more waveforms                       */
#define PI_I2C_OPEN_FAILED  -71 /* can't open I2C device                   */
#define PI_SER_OPEN_FAILED  -72 /* can't open serial device                */
#define PI_SPI_OPEN_FAILED  -73 /* can't open SPI device                   */
#define PI_BAD_I2C_BUS      -74 /* bad I2C bus                             */
#define PI_BAD_I2C_ADDR     -75 /* bad I2C address                         */
#define PI_BAD_SPI_CHANNEL  -76 /* bad SPI channel                         */
#define PI_BAD_FLAGS        -77 /* bad i2c/spi/ser open flags              */
#define PI_BAD_SPI_SPEED    -78 /* bad SPI speed                           */
#define PI_BAD_SER_DEVICE   -79 /* bad serial device name                  */
#define PI_BAD_SER_SPEED    -80 /* bad serial baud rate                    */
#define PI_BAD_PARAM        -81 /* bad i2c/spi/ser parameter               */
#define PI_I2C_WRITE_FAILED -82 /* i2c write failed                        */
#define PI_I2C_READ_FAILED  -83 /* i2c read failed                         */
#define PI_BAD_SPI_COUNT    -84 /* bad SPI count                           */
#define PI_SER_WRITE_FAILED -85 /* ser write failed                        */
#define PI_SER_READ_FAILED  -86 /* ser read failed                         */
#define PI_SER_READ_NO_DATA -87 /* ser read no data available              */
#define PI_UNKNOWN_COMMAND  -88 /* unknown command                         */
#define PI_SPI_XFER_FAILED  -89 /* spi xfer/read/write failed              */
#define PI_BAD_POINTER      -90 /* bad (NULL) pointer                      */



#define PI_DEFAULT_BUFFER_MILLIS         120
#define PI_DEFAULT_CLK_MICROS            5
#define PI_DEFAULT_CLK_PERIPHERAL        PI_CLOCK_PCM
#define PI_DEFAULT_CLK_SOURCE            PI_CLOCK_PLLD
#define PI_DEFAULT_IF_FLAGS              0
#define PI_DEFAULT_DMA_CHANNEL           14
#define PI_DEFAULT_DMA_PRIMARY_CHANNEL   14
#define PI_DEFAULT_DMA_SECONDARY_CHANNEL 5
#define PI_DEFAULT_SOCKET_PORT           8888
#define PI_DEFAULT_SOCKET_PORT_STR       "8888"
#define PI_DEFAULT_SOCKET_ADDR_STR       "127.0.0.1"
#define PI_DEFAULT_UPDATE_MASK_R0        0xFBE6CF9F
#define PI_DEFAULT_UPDATE_MASK_R1        0x03E6CF93
#define PI_DEFAULT_UPDATE_MASK_R2        0xFBC6CF9C

#endif

