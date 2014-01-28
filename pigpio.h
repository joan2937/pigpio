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
This version is for pigpio version 12
*/

#ifndef PIGPIO_H
#define PIGPIO_H

/************************************************************************** /
/                                                                           /
/ pigpio is a C library for the Raspberry Pi which allows                   /
/ control of the gpios.                                                     /
/                                                                           /
/ Its main features are:                                                    /
/                                                                           /
/ 1) provision of PWM on any number of gpios 0-31 simultaneously.           /
/ 2) provision of servo pulses on any number of gpios 0-31 simultaneously.  /
/ 3) callbacks when any of gpios 0-31 change state.                         /
/ 4) callbacks at timed intervals.                                          /
/ 5) reading/writing all of the gpios in a bank (0-31, 32-53) as a          /
/    single operation.                                                      /
/ 6) individually setting gpio modes, reading and writing.                  /
/ 7) notifications when any of gpios 0-31 change state.                     /
/ 8) the construction of arbitrary waveforms to give precise timing of      /
/    output gpio level changes.                                             /
/ 9) rudimentary permission control through the socket and pipe interfaces  /
/    so users can be prevented from "updating" inappropriate gpios.         /
/ 10) a simple interface to start and stop new threads.                     /
/                                                                           /
/ NOTE:                                                                     /
/                                                                           /
/ ALL gpios are identified by their Broadcom number.                        /
/                                                                           /
*************************************************************************** /
/                                                                           /
/ The PWM and servo pulses are timed using the DMA and PWM peripherals.     /
/                                                                           /
/ This use was inspired by Richard Hirst's servoblaster kernel module.      /
/ See https://github.com/richardghirst/PiBits                               /
/ Tag rgh on the Raspberry Pi forums http://www.raspberrypi.org/phpBB3/     /
/                                                                           /
*************************************************************************** /
/                                                                           /
/ Usage:                                                                    /
/                                                                           /
/ copy libpigpio.a to /usr/local/lib                                        /
/ copy pigpio.h    to /usr/local/include                                    /
/                                                                           /
/ #include <pigpio.h> in your source files                                  /
/                                                                           /
/ Assuming your source is in example.c use the following command to build   /
/                                                                           /
/ gcc -o example example.c -lpigpio -lpthread -lrt                          /
/                                                                           /
/ For examples see checklib.c, demolib.c, pigpio.c, pigpiod.c, pig2vcd.c,   /
/ and pigs.c                                                                /
/                                                                           /
****************************************************************************/

#include <stdint.h>
#include <pthread.h>

#define PIGPIO_VERSION 12

/*-------------------------------------------------------------------------*/

/* 

Function                   Usage
--------                   -----

gpioInitialise             Initialise library.
gpioTerminate              Terminate library.

gpioSetMode                Set a gpio mode.
gpioGetMode                Get a gpio mode.

gpioSetPullUpDown          Set/clear gpio pull up/down resistor.

gpioRead                   Read a gpio.
gpioWrite                  Write a gpio.

gpioPWM                    Start/stop PWM pulses on a gpio.

gpioSetPWMrange            Configure PWM range for a gpio.
gpioGetPWMrange            Get configured PWM range for a gpio.
gpioGetPWMrealRange        Get underlying PWM range for a gpio.

gpioSetPWMfrequency        Configure PWM frequency for a gpio.
gpioGetPWMfrequency        Get configured PWM frequency for a gpio.

gpioServo                  Start/stop servo pulses on a gpio.

gpioSetAlertFunc           Request a gpio change callback.
gpioSetAlertFuncEx         Request a gpio change callback, extended.

gpioNotifyOpen             Open a gpio(s) changed notification.
gpioNotifyBegin            Begin a gpio(s) changed notification.
gpioNotifyPause            Pause a gpio(s) changed notification.
gpioNotifyClose            Close a gpio(s) changed notification.

gpioWaveClear              Initialises a new waveform.
gpioWaveAddGeneric         Adds a series of pulses to the waveform.
gpioWaveAddSerial          Adds serial data to the waveform.

gpioWaveTxStart            Transmits the waveform.
gpioWaveTxBusy             Checks to see if the waveform has ended.
gpioWaveTxStop             Aborts the current waveform.

gpioSerialReadOpen         Opens a gpio for reading serial data.
gpioSerialRead             Reads serial data from a gpio.
gpioSerialReadClose        Closes a gpio for reading serial data.

gpioWaveGetMicros          Length in microseconds of the current waveform.
gpioWaveGetHighMicros      Length of longest waveform so far.
gpioWaveGetMaxMicros       Absolute maximum allowed micros.

gpioWaveGetPulses          Length in pulses of the current waveform.
gpioWaveGetHighPulses      Length of longest waveform so far.
gpioWaveGetMaxPulses       Absolute maximum allowed pulses.

gpioWaveGetCbs             Length in cbs of the current waveform.
gpioWaveGetHighCbs         Length of longest waveform so far.
gpioWaveGetMaxCbs          Absolute maximum allowed cbs.

gpioTrigger                Send a trigger pulse to a gpio.

gpioSetWatchdog            Set a watchdog on a gpio.

gpioSetGetSamplesFunc      Requests a gpio samples callback.
gpioSetGetSamplesFuncEx    Requests a gpio samples callback, extended.

gpioSetTimerFunc           Request a regular timed callback.
gpioSetTimerFuncEx         Request a regular timed callback, extended.

gpioStartThread            Start a new thread.
gpioStopThread             Stop a previously started thread.

gpioStoreScript            Store a script.
gpioRunScript              Run a stored script.
gpioStopScript             Stop a running script.
gpioDeleteScript           Delete a stored script.

gpioSetSignalFunc          Request a signal callback.
gpioSetSignalFuncEx        Request a signal callback, extended.

gpioRead_Bits_0_31         Read gpios in bank 1.
gpioRead_Bits_32_53        Read gpios in bank 2.

gpioWrite_Bits_0_31_Clear  Clear gpios in bank 1.
gpioWrite_Bits_32_53_Clear Clear gpios in bank 2.

gpioWrite_Bits_0_31_Set    Set gpios in bank 1.
gpioWrite_Bits_32_53_Set   Set gpios in bank 2.

gpioTime                   Get current time.

gpioSleep                  Sleep for specified time.
gpioDelay                  Delay for microseconds.

gpioTick                   Get current tick (microseconds).

gpioHardwareRevision       Get hardware version.

gpioVersion                Get the pigpio version.

gpioCfgBufferSize          Configure the gpio sample buffer size.
gpioCfgClock               Configure the gpio sample rate.
gpioCfgDMAchannel          Configure the DMA channel (DEPRECATED).
gpioCfgDMAchannels         Configure the DMA channels.
gpioCfgPermissions         Configure the gpio access permissions.
gpioCfgInterfaces          Configure user interfaces.
gpioCfgInternals           Configure miscellaneous internals.
gpioCfgSocketPort          Configure socket port.

*/

/*-------------------------------------------------------------------------*/


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
   uint32_t cmd;
   uint32_t p1;
   uint32_t p2;
   uint32_t res;
} cmdCmd_t;

typedef struct
{
   size_t size;
   void *ptr;
   int data;
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

typedef void (*gpioAlertFunc_t)    (int      gpio,
                                    int      level,
                                    uint32_t tick);

typedef void (*gpioAlertFuncEx_t)  (int      gpio,
                                    int      level,
                                    uint32_t tick,
                                    void *   userdata);

typedef void (*gpioTimerFunc_t)    (void);

typedef void (*gpioTimerFuncEx_t)  (void * userdata);

typedef void (*gpioSignalFunc_t)   (int    signum);

typedef void (*gpioSignalFuncEx_t) (int    signum,
                                    void * userdata);

typedef void (*gpioGetSamplesFunc_t)   (const gpioSample_t * samples,
                                        int                  numSamples);

typedef void (*gpioGetSamplesFuncEx_t) (const gpioSample_t * samples,
                                        int                  numSamples,
                                        void *               userdata);

typedef void *(gpioThreadFunc_t) (void *);


/*
   All the functions which return an int return < 0 on error.

   If the library isn't initialised all but the gpioCfg*, gpioVersion,
   and gpioHardwareRevision functions will return error PI_NOT_INITIALISED.

   If the library is initialised the gpioCfg* functions will
   return error PI_INITIALISED.  
*/

 

/*-------------------------------------------------------------------------*/
int gpioInitialise(void);
/*-------------------------------------------------------------------------*/
/* Initialises the library.

   Call before using the other library functions.

   Returns the pigpio version number if OK, otherwise PI_INIT_FAILED.

   NOTES:

   The only exception is the optional gpioCfg* functions, see later.
*/



/*-------------------------------------------------------------------------*/
void gpioTerminate(void);
/*-------------------------------------------------------------------------*/
/* Terminates the library.

   Returns nothing.

   Call before program exit.

   NOTES:

   This function resets the DMA and PWM peripherals, releases memory, and
   terminates any running threads.
*/



/*-------------------------------------------------------------------------*/
int gpioSetMode(unsigned gpio,
                unsigned mode);
/*-------------------------------------------------------------------------*/
/* Sets the gpio mode, typically input or output.

   Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_MODE.

   Arduino style: pinMode.

   EXAMPLE:
   ...
   gpioSetMode(17, PI_INPUT);  // set gpio17 as input
   gpioSetMode(18, PI_OUTPUT); // set gpio18 as output
   gpioSetMode(22,PI_ALT0);    // set gpio22 to alternative mode 0
   ... 
*/

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



/*-------------------------------------------------------------------------*/
int gpioGetMode(unsigned gpio);
/*-------------------------------------------------------------------------*/
/* Gets the gpio mode.

   Returns the gpio mode if OK, otherwise PI_BAD_GPIO.

   EXAMPLE:
   ...
   if (gpioGetMode(17) != PI_ALT0)
   {
      gpioSetMode(17, PI_ALT0);  // set gpio17 to ALT0  
   }
   ...
*/



/*-------------------------------------------------------------------------*/
int gpioSetPullUpDown(unsigned gpio,
                      unsigned pud);
/*-------------------------------------------------------------------------*/
/* Sets or clears resistor pull ups or downs on the gpio.

   Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_PUD.

   EXAMPLE:
   ...
   gpioSetPullUpDown(17, PI_PUD_UP);   // sets a pull-up on gpio17
   gpioSetPullUpDown(18, PI_PUD_DOWN); // sets a pull-down on gpio18
   gpioSetPullUpDown(23, PI_PUD_OFF);  // clear pull-ups/downs on gpio23
   ...
*/

/* pud: 0-2 */

#define PI_PUD_OFF  0
#define PI_PUD_DOWN 1
#define PI_PUD_UP   2



/*-------------------------------------------------------------------------*/
int gpioRead (unsigned gpio);
/*-------------------------------------------------------------------------*/
/* Reads the gpio level, on or off.

   Returns the gpio level if OK, otherwise PI_BAD_GPIO.

   EXAMPLE:
   ...
   printf("gpio24 is level %d\n", gpioRead(24));
   ...

   NOTES:

   Arduino style: digitalRead.
*/

/* level: 0-1 */

#define PI_OFF   0
#define PI_ON    1

#define PI_CLEAR 0
#define PI_SET   1

#define PI_LOW   0
#define PI_HIGH  1

/* level: only reported for gpio timeout, see gpioSetWatchdogTimeout */

#define PI_TIMEOUT 2



/*-------------------------------------------------------------------------*/
int gpioWrite(unsigned gpio,
              unsigned level);
/*-------------------------------------------------------------------------*/
/* Sets the gpio level, on or off.

   Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_LEVEL.

   EXAMPLE:
   ...
   gpioWrite(24, 1); // sets gpio24 high
   ...

   NOTES:

   If PWM or servo pulses are active on the gpio they are switched off.

   Arduino style: digitalWrite
*/



/*-------------------------------------------------------------------------*/
int gpioPWM(unsigned user_gpio,
            unsigned dutycycle);
/*-------------------------------------------------------------------------*/
/* Starts PWM on the gpio, dutycycle between 0 (off) and range (fully on).
   Range defaults to 255.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_BAD_DUTYCYCLE.

   EXAMPLE:
   ...
   gpioPWM(17, 255); // sets gpio17 full on
   gpioPWM(18, 128); // sets gpio18 half on
   gpioPWM(23, 0);   // sets gpio23 full off
   ...

   NOTES:

   Arduino style: analogWrite

   This and the servo functionality use the DMA and PWM or PCM peripherals
   to control and schedule the pulse lengths and duty cycles.

   The gpioSetPWMrange funtion can change the default range of 255.
*/

/* user_gpio: 0-31 */

#define PI_MAX_USER_GPIO 31

/* dutycycle: 0-range */

#define PI_DEFAULT_DUTYCYCLE_RANGE   255



/*-------------------------------------------------------------------------*/
int gpioSetPWMrange(unsigned user_gpio,
                    unsigned range);
/*-------------------------------------------------------------------------*/
/* Selects the dutycycle range to be used for the gpio.  Subsequent calls
   to gpioPWM will use a dutycycle between 0 (off) and range (fully on).

   Returns the real range for the given gpio's frequency if OK,
   otherwise PI_BAD_USER_GPIO or PI_BAD_DUTYRANGE.

   EXAMPLE:
   ...
   gpioSetPWMrange(24, 2000); // now 2000 is fully on, 1000 is half on etc.
   ...

   NOTES:

   If PWM is currently active on the gpio its dutycycle will be scaled
   to reflect the new range.

   The real range, the number of steps between fully off and fully
   on for each frequency, is given in the following table.

     25,   50,  100,  125,  200,  250,  400,   500,   625,
    800, 1000, 1250, 2000, 2500, 4000, 5000, 10000, 20000

    The real value set by gpioPWM is

       (dutycycle * real range) / range.
*/

/* range: 25-40000 */

#define PI_MIN_DUTYCYCLE_RANGE        25
#define PI_MAX_DUTYCYCLE_RANGE     40000



/*-------------------------------------------------------------------------*/
int gpioGetPWMrange(unsigned user_gpio);
/*-------------------------------------------------------------------------*/
/* Returns the dutycycle range used for the gpio if OK, otherwise
   PI_BAD_USER_GPIO.
*/



/*-------------------------------------------------------------------------*/
int gpioGetPWMrealRange(unsigned user_gpio);
/*-------------------------------------------------------------------------*/
/* Returns the real range used for the gpio if OK, otherwise
   PI_BAD_USER_GPIO.
*/



/*-------------------------------------------------------------------------*/
int gpioSetPWMfrequency(unsigned user_gpio,
                        unsigned frequency);
/*-------------------------------------------------------------------------*/
/* Sets the frequency in hertz to be used for the gpio.

   Returns the numerically closest frequency if OK, otherwise
   PI_BAD_USER_GPIO.

   The selectable frequencies depend upon the sample rate which
   may be 1, 2, 4, 5, 8, or 10 microseconds (default 5).

   Each gpio can be independently set to one of 18 different PWM
   frequencies.

   If PWM is currently active on the gpio it will be
   switched off and then back on at the new frequency.

   NOTES:

   The frequencies for each sample rate are:

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
*/



/*-------------------------------------------------------------------------*/
int gpioGetPWMfrequency(unsigned user_gpio);
/*-------------------------------------------------------------------------*/
/* Returns the frequency (in hertz) used for the gpio if OK, otherwise
   PI_BAD_USER_GPIO.
*/



/*-------------------------------------------------------------------------*/
int gpioServo(unsigned user_gpio,
              unsigned pulsewidth);
/*-------------------------------------------------------------------------*/
/* Starts servo pulses on the gpio, 0 (off), 500 (most anti-clockwise) to
   2500 (most clockwise).

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_BAD_PULSEWIDTH.

   NOTES:

   The range supported by servos varies and should probably be determined
   by experiment.  A value of 1500 should always be safe and represents
   the mid-point of rotation.  You can DAMAGE a servo if you command it
   to move beyond its limits.

   EXAMPLE:

   ...
   gpioServo(17, 1500);
   ...

   This example causes an on pulse of 1500 microseconds duration to be
   transmitted on gpio 17 at a rate of 50 times per second.

   This will command a servo connected to gpio 17 to rotate to
   its mid-point.

   OTHER UPDATE RATES:

   This function updates servos at 50Hz.  If you wish to use a different
   update frequency you will have to use the PWM functions.

   PWM Hz    50   100  200  400  500
   1E6/Hz 20000 10000 5000 2500 2000

   Firstly set the desired PWM frequency using gpioSetPWMfrequency.

   Then set the PWM range using gpioSetPWMrange to 1E6/frequency.
   Doing this allows you to use units of microseconds when setting
   the servo pulse width.

   E.g. If you want to update a servo connected to gpio 25 at 400Hz

   gpioSetPWMfrequency(25, 400);
   gpioSetPWMrange(25, 2500);

   Thereafter use the PWM command to move the servo,
   e.g. gpioPWM(25, 1500) will set a 1500 us pulse. 

*/

/* pulsewidth: 0, 500-2500 */

#define PI_SERVO_OFF 0
#define PI_MIN_SERVO_PULSEWIDTH 500
#define PI_MAX_SERVO_PULSEWIDTH 2500


/*-------------------------------------------------------------------------*/
int gpioSetAlertFunc(unsigned        user_gpio,
                     gpioAlertFunc_t f);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) when the specified
   gpio changes state.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO.

   One function may be registered per gpio.

   The function is passed the gpio, the new level, and the tick.

   The alert may be cancelled by passing NULL as the function.

   EXAMPLE:
   ...
   void aFunction(int gpio, int level, uint32_t tick)
   {
      printf("gpio %d became %d at %d\n", gpio, level, tick);
   }
   ...
   gpioSetAlertFunc(4, aFunction);
   ...

   This example causes aFunction to be called whenever
   gpio 4 changes state.

   NOTES:

   The gpios are sampled at a rate set when the library is started.

   If a value isn't specifically set the default of 5 us is used.

   The number of samples per second is given in the following table.

                 samples
                 per sec

            1  1,000,000
            2    500,000
   sample   4    250,000
   rate     5    200,000
   (us)     8    125,000
           10    100,000

   Level changes of length less than the sample rate may be missed.

   The thread which calls the alert functions is triggered nominally
   1000 times per second.  The active alert functions will be called
   once per level change since the last time the thread was activated.
   i.e. The active alert functions will get all level changes but there
   will be a latency.

   The tick value is the time stamp of the sample in microseconds, see
   gpioTick for more details.
*/


/*-------------------------------------------------------------------------*/
int gpioSetAlertFuncEx(unsigned          user_gpio,
                       gpioAlertFuncEx_t f,
                       void *            userdata);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) when the specified
   gpio changes state.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO.

   One function may be registered per gpio.

   The function is passed the gpio, the new level, the tick, and
   the userdata pointer.

   Only one of gpioSetAlertFunc or gpioSetAlertFuncEx can be
   registered per gpio.

   See gpioSetAlertFunc for further details.
*/



/*-------------------------------------------------------------------------*/
int gpioNotifyOpen(void);
/*-------------------------------------------------------------------------*/
/* This function requests a free notification handle.

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
*/



/*-------------------------------------------------------------------------*/
int gpioNotifyBegin(unsigned handle,
                    uint32_t bits);
/*-------------------------------------------------------------------------*/
/* This function starts notifications on a previously opened handle.

   Returns 0 if OK, otherwise PI_BAD_HANDLE.

   The notification sends state changes for each gpio whose corresponding
   bit in bits is set.

   EXAMPLE:

   gpioNotifyBegin(0, 1234) will start notifications for gpios 1, 4, 6,
   7, 10 (1234 = 0x04D2 = 0b0000010011010010).

   NOTES:

   Each notification occupies 12 bytes in the fifo and has the
   following structure.

   typedef struct
   {
      uint16_t seqno;
      uint16_t flags;
      uint32_t tick;
      uint32_t level;
   } gpioReport_t;

   seqno starts at 0 each time the handle is opened and then increments
   by one for each report.

   flags, if bit 5 is set then bits 0-4 of the flags indicate a gpio
   which has had a watchdog timeout.

   tick is the number of microseconds since system boot.

   level indicates the level of each gpio.
*/

#define PI_NTFY_FLAGS_WDOG     (1 <<5)
#define PI_NTFY_FLAGS_BIT(x) (((x)<<0)&31)



/*-------------------------------------------------------------------------*/
int gpioNotifyPause(unsigned handle);
/*-------------------------------------------------------------------------*/
/* This function pauses notifications on a previously opened handle.

   Returns 0 if OK, otherwise PI_BAD_HANDLE.

   Notifications for the handle are suspended until gpioNotifyBegin
   is called again.
*/



/*-------------------------------------------------------------------------*/
int gpioNotifyClose(unsigned handle);
/*-------------------------------------------------------------------------*/
/* This function stops notifications on a previously opened handle
   and releases the handle for reuse.

   Returns 0 if OK, otherwise PI_BAD_HANDLE.
*/


/*-------------------------------------------------------------------------*/
int gpioWaveClear(void);
/*-------------------------------------------------------------------------*/
/* This function initialises a new waveform.

   Returns 0 if OK.

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
*/


/*-------------------------------------------------------------------------*/
int gpioWaveAddGeneric(unsigned numPulses, gpioPulse_t * pulses);
/*-------------------------------------------------------------------------*/
/* This function adds a number of pulses to the current waveform.

   Returns the new total number of pulses in the current waveform if OK,
   otherwise PI_TOO_MANY_PULSES.

   NOTES:

   The   pulses are interleaved in time order within the existing waveform
   (if any).

   Merging allows the waveform to be built in parts, that is the settings
   for gpio#1 can be added, and then gpio#2 etc.

   If the added waveform is intended to start after or within the existing
   waveform then the first pulse should consist of a delay.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveAddSerial(unsigned user_gpio,
                      unsigned baud,
                      unsigned offset,
                      unsigned numChar,
                      char *   str);
/*-------------------------------------------------------------------------*/
/* This function adds a waveform representing serial data to the
   existing waveform (if any).  The serial data starts offset microseconds
   from the start of the waveform.

   Returns the new total number of pulses in the current waveform if OK,
   otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD, PI_TOO_MANY_CHARS,
   PI_BAD_SER_OFFSET, or PI_TOO_MANY_PULSES.

   NOTES:

   The serial data is formatted as one start bit, eight data bits, and one
   stop bit.

   It is legal to add serial data streams with different baud rates to
   the same waveform.
*/

#define PI_WAVE_BLOCKS     3
#define PI_WAVE_MAX_PULSES (PI_WAVE_BLOCKS * 3000)
#define PI_WAVE_MAX_CHARS  (PI_WAVE_BLOCKS *  256)

#define PI_WAVE_MIN_BAUD      100
#define PI_WAVE_MAX_BAUD      250000

#define PI_WAVE_MAX_MICROS (30 * 60 * 1000000) /* half an hour */



/*-------------------------------------------------------------------------*/
int gpioWaveTxStart(unsigned mode);
/*-------------------------------------------------------------------------*/
/* This function transmits the current waveform.  The mode determines
   whether the waveform is sent once or cycles endlessly.

   Returns the number of DMA control blocks in the waveform if OK,
   otherwise PI_BAD_WAVE_MODE.
*/

#define PI_WAVE_MODE_ONE_SHOT 0
#define PI_WAVE_MODE_REPEAT   1



/*-------------------------------------------------------------------------*/
int gpioWaveTxBusy(void);
/*-------------------------------------------------------------------------*/
/* This function checks to see if a waveform is currently being
   transmitted.

   Returns 1 if a waveform is currently being transmitted, otherwise 0.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveTxStop(void);
/*-------------------------------------------------------------------------*/
/* This function aborts the transmission of the current waveform.

   Returns 0 if OK.

   NOTES:

   This function is intended to stop a waveform started with the repeat mode.
*/



/*-------------------------------------------------------------------------*/
int gpioSerialReadOpen(unsigned user_gpio, unsigned baud);
/*-------------------------------------------------------------------------*/
/* This function opens a gpio for reading serial data.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD,
   or PI_GPIO_IN_USE.

   The serial data is returned in a cyclic buffer and is read using
   gpioSerialRead().

   It is the caller's responsibility to read data from the cyclic buffer
   in a timely fashion.
*/



/*-------------------------------------------------------------------------*/
int gpioSerialRead(unsigned user_gpio, void *buf, size_t bufSize);
/*-------------------------------------------------------------------------*/
/* This function copies up to bufSize bytes of data read from the
   serial cyclic buffer to the buffer starting at buf.

   Returns the number of bytes copied if OK, otherwise PI_BAD_USER_GPIO
   or PI_NOT_SERIAL_GPIO.
*/



/*-------------------------------------------------------------------------*/
int gpioSerialReadClose(unsigned user_gpio);
/*-------------------------------------------------------------------------*/
/* This function closes a gpio for reading serial data.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_SERIAL_GPIO.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetMicros(void);
/*-------------------------------------------------------------------------*/
/* This function returns the length in microseconds of the current
   waveform.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetHighMicros(void);
/*-------------------------------------------------------------------------*/
/* This function returns the length in microseconds of the longest waveform
   created since gpioInitialise was called.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetMaxMicros(void);
/*-------------------------------------------------------------------------*/
/* This function returns the maximum possible size of a waveform in 
   microseconds.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetPulses(void);
/*-------------------------------------------------------------------------*/
/* This function returns the length in pulses of the current waveform.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetHighPulses(void);
/*-------------------------------------------------------------------------*/
/* This function returns the length in pulses of the longest waveform
   created since gpioInitialise was called.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetMaxPulses(void);
/*-------------------------------------------------------------------------*/
/* This function returns the maximum possible size of a waveform in pulses.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetCbs(void);
/*-------------------------------------------------------------------------*/
/* This function returns the length in DMA control blocks of the current
   waveform.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetHighCbs(void);
/*-------------------------------------------------------------------------*/
/* This function returns the length in DMA control blocks of the longest
   waveform created since gpioInitialise was called.
*/



/*-------------------------------------------------------------------------*/
int gpioWaveGetMaxCbs(void);
/*-------------------------------------------------------------------------*/
/* This function returns the maximum possible size of a waveform in DMA
   control blocks.
*/



/*-------------------------------------------------------------------------*/
int gpioTrigger(unsigned user_gpio, unsigned pulseLen, unsigned level);
/*-------------------------------------------------------------------------*/
/* This function sends a trigger pulse to a gpio.  The gpio is set to
   level for pulseLen microseconds and then reset to not level.

   Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_LEVEL,
   or PI_BAD_PULSELEN.
*/

#define PI_MAX_PULSELEN 100


/*-------------------------------------------------------------------------*/
int gpioSetWatchdog(unsigned user_gpio,
                    unsigned timeout);
/*-------------------------------------------------------------------------*/
/* Sets a watchdog for a gpio.

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

   EXAMPLE:

   void aFunction(int gpio, int level, uint32_t tick)
   {
      printf("gpio %d became %d at %d\n", gpio, level, tick);
   }
   ...
   gpioSetAlertFunc(4, aFunction);
   gpioSetWatchdogTimeout(4, 5);
   ...

   This example causes aFunction to be called whenever
   gpio 4 changes state or approximately every 5 ms.
*/

/* timeout: 0-60000 */

#define PI_MIN_WDOG_TIMEOUT 0
#define PI_MAX_WDOG_TIMEOUT 60000



/*-------------------------------------------------------------------------*/
int gpioSetGetSamplesFunc(gpioGetSamplesFunc_t f,
                          uint32_t             bits);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) every millisecond
   with the latest gpio samples.

   Returns 0 if OK.

   The function is passed a pointer to the samples and the number
   of samples.

   Only one function can be registered.

   The callback may be cancelled by passing NULL as the function.

   NOTES:

   The samples returned will be the union of bits, plus any active alerts,
   plus any active notifications.

   e.g.  if there are alerts for gpios 7, 8, and 9, notifications for gpios
   8, 10, 23, 24, and bits is (1<<23)|(1<<17) then samples for gpios
   7, 8, 9, 10, 17, 23, and 24 will be reported.
*/



/*-------------------------------------------------------------------------*/
int gpioSetGetSamplesFuncEx(gpioGetSamplesFuncEx_t f,
                            uint32_t               bits,
                            void *                 userdata);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) every millisecond
   with the latest gpio samples.

   Returns 0 if OK.

   The function is passed a pointer to the samples, the number
   of samples, and the userdata pointer.

   Only one of gpioGetSamplesFunc or gpioGetSamplesFuncEx can be
   registered.

   See gpioSetGetSamplesFunc for further details.
*/



/*-------------------------------------------------------------------------*/
int gpioSetTimerFunc(unsigned        timer,
                     unsigned        ms,
                     gpioTimerFunc_t f);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) every ms milliseconds.

   Returns 0 if OK, otherwise PI_BAD_TIMER, PI_BAD_MS, or PI_TIMER_FAILED.

   10 timers are supported numbered 0 to 9.

   One function may be registered per timer.

   The timer may be cancelled by passing NULL as the function.

   EXAMPLE:

   ...
   void bFunction(void)
   {
      printf("two seconds have elapsed\n");
   }
   ...
   gpioSetTimerFunc(0, 2000, bFunction);
   ...

   This example causes bFunction to be called every 2000 milliseconds.
*/

/* timer: 0-9 */

#define PI_MIN_TIMER 0
#define PI_MAX_TIMER 9

/* ms: 10-60000 */

#define PI_MIN_MS 10
#define PI_MAX_MS 60000



/*-------------------------------------------------------------------------*/
int gpioSetTimerFuncEx(unsigned          timer,
                       unsigned          ms,
                       gpioTimerFuncEx_t f,
                       void *            userdata);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) every ms milliseconds.

   Returns 0 if OK, otherwise PI_BAD_TIMER, PI_BAD_MS, or PI_TIMER_FAILED.

   The function is passed the userdata pointer.

   Only one of gpioSetTimerFunc or gpioSetTimerFuncEx can be
   registered per timer.

   See gpioSetTimerFunc for further details.
*/



/* ----------------------------------------------------------------------- */
pthread_t *gpioStartThread(gpioThreadFunc_t func, void *arg);
/*-------------------------------------------------------------------------*/
/* Starts a new thread of execution with func as the main routine.

   Returns a pointer to pthread_t if OK, otherwise NULL.

   The function is passed the single argument arg.

   The thread can be cancelled by passing the pointer to pthread_t to
   gpioStopThread().

   EXAMPLE:

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
*/



/* ----------------------------------------------------------------------- */
void gpioStopThread(pthread_t *pth);
/*-------------------------------------------------------------------------*/
/* Cancels the thread pointed at by pth.

   No value is returned.

   The thread to be stopped should have been started with gpioStartThread().
*/


/* ----------------------------------------------------------------------- */
int gpioStoreScript(char *script);
/* ----------------------------------------------------------------------- */
/* This function stores a null terminated script for later execution.

   The function returns a script id if the script is valid,
   otherwise PI_BAD_SCRIPT.
*/



/* ----------------------------------------------------------------------- */
int gpioRunScript(int script_id);
/* ----------------------------------------------------------------------- */
/* This function runs a stored script.

   The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.
*/



/* ----------------------------------------------------------------------- */
int gpioStopScript(int script_id);
/* ----------------------------------------------------------------------- */
/* This function stops a running script.

   The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.
*/



/* ----------------------------------------------------------------------- */
int gpioDeleteScript(int script_id);
/* ----------------------------------------------------------------------- */
/* This function deletes a stored script.

   The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.
*/



/*-------------------------------------------------------------------------*/
int gpioSetSignalFunc(unsigned         signum,
                      gpioSignalFunc_t f);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) when a signal occurs.

   Returns 0 if OK, otherwise PI_BAD_SIGNUM.

   The function is passed the signal number.

   One function may be registered per signal.

   The callback may be cancelled by passing NULL.

   NOTES:

   By default all signals are treated as fatal and cause the library
   to call gpioTerminate and then exit.
*/

/* signum: 0-63 */

#define PI_MIN_SIGNUM 0
#define PI_MAX_SIGNUM 63



/*-------------------------------------------------------------------------*/
int gpioSetSignalFuncEx(unsigned           signum,
                        gpioSignalFuncEx_t f,
                        void *             userdata);
/*-------------------------------------------------------------------------*/
/* Registers a function to be called (a callback) when a signal occurs.

   Returns 0 if OK, otherwise PI_BAD_SIGNUM.

   The function is passed the signal number and the userdata pointer.

   Only one of gpioSetSignalFunc or gpioSetSignalFuncEx can be
   registered per signal.

   See gpioSetSignalFunc for further details.
*/



/*-------------------------------------------------------------------------*/
uint32_t gpioRead_Bits_0_31(void);
/*-------------------------------------------------------------------------*/
/* Returns the current level of gpios 0-31.
*/



/*-------------------------------------------------------------------------*/
uint32_t gpioRead_Bits_32_53(void);
/*-------------------------------------------------------------------------*/
/* Returns the current level of gpios 32-53.
*/



/*-------------------------------------------------------------------------*/
int gpioWrite_Bits_0_31_Clear(uint32_t levels);
/*-------------------------------------------------------------------------*/
/* Clears gpios 0-31 if the corresponding bit in levels is set.

   Returns 0 if OK.

   EXAMPLE:

   To clear (set to 0) gpios 4, 7, and 15.

   ...
   gpioWrite_Bits_0_31_Clear( (1<<4) | (1<<7) | (1<<15) );
   ...
*/



/*-------------------------------------------------------------------------*/
int gpioWrite_Bits_32_53_Clear(uint32_t levels);
/*-------------------------------------------------------------------------*/
/* Clears gpios 32-53 if the corresponding bit (0-21) in levels is set.

   Returns 0 if OK.
*/



/*-------------------------------------------------------------------------*/
int gpioWrite_Bits_0_31_Set(uint32_t levels);
/*-------------------------------------------------------------------------*/
/* Sets gpios 0-31 if the corresponding bit in levels is set.

   Returns 0 if OK.
*/



/*-------------------------------------------------------------------------*/
int gpioWrite_Bits_32_53_Set(uint32_t levels);
/*-------------------------------------------------------------------------*/
/* Sets gpios 32-53 if the corresponding bit (0-21) in levels is set.

   Returns 0 if OK.

   EXAMPLE:

   To set (set to 1) gpios 32, 40, and 53.

   ...
   gpioWrite_Bits_32_53_Set( (1<<(32-32)) | (1<<(40-32)) | (1<<(53-32)) );
   ...
*/

/*-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*/
int gpioTime(unsigned timetype,
             int *    seconds,
             int *    micros);
/*-------------------------------------------------------------------------*/
/* Updates the seconds and micros variables with the current time.

   Returns 0 if OK, otherwise PI_BAD_TIMETYPE.

   If timetype is PI_TIME_ABSOLUTE updates seconds and micros with the
   number of seconds and microseconds since the epoch (1st January 1970).

   If timetype is PI_TIME_RELATIVE updates seconds and micros with the
   number of seconds and microseconds since the library was initialised.

   EXAMPLE:

   ...
   int secs, mics;
   ...
   gpioTime(PI_TIME_RELATIVE, &secs, &mics);
   printf("library started %d.%03d seconds ago\n", secs, mics/1000);
   ...
   prints the number of seconds since the library was started.
*/

/* timetype: 0-1 */

#define PI_TIME_RELATIVE 0
#define PI_TIME_ABSOLUTE 1



/*-------------------------------------------------------------------------*/
int gpioSleep(unsigned timetype,
              int      seconds,
              int      micros);
/*-------------------------------------------------------------------------*/
/* Sleeps for the number of seconds and microseconds specified by seconds
   and micros.

   Returns 0 if OK, otherwise PI_BAD_TIMETYPE, PI_BAD_SECONDS,
   or PI_BAD_MICROS.
  
   If timetype is PI_TIME_ABSOLUTE the sleep ends when the number of seconds
   and microseconds since the epoch (1st January 1970) has elapsed.  System
   clock changes are taken into account.

   If timetype is PI_TIME_RELATIVE the sleep is for the specified number
   of seconds and microseconds.  System clock changes do not effect the
   sleep length.

   NOTES:

   For short delays (say, 250 microseonds or less) use gpioDelayMicroseconds.

   EXAMPLE:

   ...
   gpioSleep(PI_TIME_RELATIVE, 2, 500000); // sleep for 2.5 seconds
   ...
   gpioSleep(PI_TIME_RELATIVE, 0, 100000); // sleep for 1/10th of a second
   ...
   gpioSleep(PI_TIME_RELATIVE, 60, 0);     // sleep for one minute
   ...
*/



/*-------------------------------------------------------------------------*/
uint32_t gpioDelay(uint32_t micros);
/*-------------------------------------------------------------------------*/
/* Delays for at least the number of microseconds specified by micros.

   Returns the actual length of the delay in microseconds.  
*/



/*-------------------------------------------------------------------------*/
uint32_t gpioTick(void);
/*-------------------------------------------------------------------------*/
/* Returns the current system tick.

   Tick is the number of microseconds since system boot.

   NOTES:

   As tick is an unsigned 32 bit quantity it wraps around after
   2^32 microseconds, which is approximately 1 hour 12 minutes.

   You don't need to worry about the wrap around as long as you
   take a tick (uint32_t) from another tick, i.e. the following
   code will always provide the correct difference.

   EXAMPLE:

   uint32_t startTick, endTick;
   int diffTick;
   ...
   startTick = gpioTick();
   ...
   // do some processing
   ...
   endTick = gpioTick();

   diffTick = endTick - startTick;

   printf("some processing took %d microseconds\n", diffTick);
   ...
*/



/*-------------------------------------------------------------------------*/
unsigned gpioHardwareRevision(void);
/*-------------------------------------------------------------------------*/
/* Returns the hardware revision.

   If the hardware revision can not be found or is not a valid hexadecimal
   number the function returns 0.

   NOTES:

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

   EXAMPLES:

   for "Revision       : 0002" the function returns 2.
   for "Revision       : 000f" the function returns 15.
   for "Revision       : 000g" the function returns 0.
*/



/*-------------------------------------------------------------------------*/
unsigned gpioVersion(void);
/*-------------------------------------------------------------------------*/
/* Returns the pigpio version.
*/



/*-------------------------------------------------------------------------*/
int gpioCfgBufferSize(unsigned millis);
/*-------------------------------------------------------------------------*/
/* Configures pigpio to buffer millis milliseconds of gpio samples.

   The default setting is 120 milliseconds.

   NOTES:

   The intention is to allow for bursts of data and protection against
   other processes hogging cpu time.

   I haven't seen a process locked out for more than 100 milliseconds.

   Making the buffer bigger uses a LOT of memory at the more frequent
   sampling rates as shown in the following table in MBs.

                     buffer milliseconds
               120 250 500 1sec 2sec 4sec 8sec

         1      16  31  55  107  ---  ---  ---
         2      10  18  31   55  107  ---  ---
sample   4       8  12  18   31   55  107  ---
 rate    5       8  10  14   24   45   87  ---
 (us)    8       6   8  12   18   31   55  107
        10       6   8  10   14   24   45   87
*/

/* millis */

#define PI_BUF_MILLIS_MIN 100
#define PI_BUF_MILLIS_MAX 10000



/*-------------------------------------------------------------------------*/
int gpioCfgClock(unsigned micros,
                 unsigned peripheral,
                 unsigned source);
/*-------------------------------------------------------------------------*/
/* Configures pigpio to use a sample rate of micros microseconds,
   permitted values are 1, 2, 4, 5, 8 and 10.

   The timings are provided by the specified peripheral (PWM or PCM)
   using the frequency source (OSC or PLLD).

   The default setting is 5 microseconds using the PCM peripheral
   with the PLLD source.

   NOTES:

   The approximate CPU percentage used for each sample rate is:

   sample  cpu
    rate    %

     1     25
     2     16
     4     11
     5     10
     8     15
    10     14

    A sample rate of 5 microseconds seeems to be the sweet spot.

    These readings were done by checking the resources used by
    the demolib program (which is reasonably busy).
*/

/* micros: 1, 2, 4, 5, 8, or 10 */

/* peripheral: 0-1 */

#define PI_CLOCK_PWM 0
#define PI_CLOCK_PCM 1

/* source: 0-1 */

#define PI_CLOCK_OSC  0
#define PI_CLOCK_PLLD 1



/*-------------------------------------------------------------------------*/
int gpioCfgDMAchannel(unsigned channel); /* DEPRECATED */
/*-------------------------------------------------------------------------*/
/* Configures pigpio to use the specified DMA channel.

   The default setting is to use channel 14.
*/

/* channel: 0-14 */

#define PI_MIN_DMA_CHANNEL 0
#define PI_MAX_DMA_CHANNEL 14



/*-------------------------------------------------------------------------*/
int gpioCfgDMAchannels(unsigned primaryChannel,
                       unsigned secondaryChannel);
/*-------------------------------------------------------------------------*/
/* Configures pigpio to use the specified DMA channels.

   The default setting is to use channel 14 for the primary channel and
   channel 5 for the secondary channel.
*/

#define PI_MAX_PRIMARY_CHANNEL   14
#define PI_MAX_SECONDARY_CHANNEL  6



/*-------------------------------------------------------------------------*/
int gpioCfgPermissions(uint64_t updateMask);
/*-------------------------------------------------------------------------*/
/* Configures pigpio to only allow updates (writes or mode changes) for the
   gpios specified by the mask.

   The default setting depends upon the board revision (Type 1 or Type 2).
   The user gpios are added to the mask.  If the board revision is not
   recognised then the mask is formed by or'ing the bits for the two
   board revisions.

   Unknown board: PI_DEFAULT_UPDATE_MASK_R0        0xFBE6CF9F
   Type 1 board:  PI_DEFAULT_UPDATE_MASK_R1        0x03E6CF93
   Type 2 board:  PI_DEFAULT_UPDATE_MASK_R2        0xFBC6CF9C
*/



/*-------------------------------------------------------------------------*/
int gpioCfgSocketPort(unsigned port);
/*-------------------------------------------------------------------------*/
/* Configures pigpio to use the specified socket port.

   The default setting is to use port 8888.
*/

/* port: 1024-9999 */

#define PI_MIN_SOCKET_PORT 1024
#define PI_MAX_SOCKET_PORT 32000



/*-------------------------------------------------------------------------*/
int gpioCfgInterfaces(unsigned ifFlags);
/*-------------------------------------------------------------------------*/
/* Configures pigpio support of the fifo and socket interfaces.

   The default setting is that both interfaces are enabled.
*/

/* ifFlags: */

#define PI_DISABLE_FIFO_IF 1
#define PI_DISABLE_SOCK_IF 2

/*-------------------------------------------------------------------------*/
int gpioCfgInternals(unsigned what,
                     int      value);
/*-------------------------------------------------------------------------*/
/* Used to tune internal settings.
   Not intended for general use.
*/



/*-------------------------------------------------------------------------*/
void gpioWaveDump(void);
/*-------------------------------------------------------------------------*/
/* Used to print a readable version of the current waveform to stdout.
   Not intended for general use.
*/


#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------------*/

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

/*
The following command only works on the socket interface.
It returns a spare notification handle.  Notifications for
that handle will be sent to the socket (rather than a
/dev/pigpiox pipe).

The socket should be dedicated to receiving notifications
after this command is issued.
*/

#define PI_CMD_NOIB  99


/*-------------------------------------------------------------------------*/

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
#define PI_BAD_PULSELEN     -46 /* trigger pulse length > 100              */
#define PI_BAD_SCRIPT       -47 /* invalid script                          */
#define PI_BAD_SCRIPT_ID    -48 /* unknown script id                       */
#define PI_BAD_SER_OFFSET   -49 /* add serial data offset > 30 minutes     */
#define PI_GPIO_IN_USE      -50 /* gpio already in use                     */
#define PI_BAD_SERIAL_COUNT -51 /* must read at least a byte at a time     */


/*-------------------------------------------------------------------------*/

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

