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

#ifndef PIGPIOD_IF_H
#define PIGPIOD_IF_H

#include "pigpio.h"

#define PIGPIOD_IF_VERSION 28

/*TEXT

THIS LIBRARY IS DEPRECATED.  NEW CODE SHOULD BE WRITTEN TO
USE THE MORE VERSATILE pigpiod_if2 LIBRARY.

pigpiod_if is a C library for the Raspberry which allows control
of the GPIO via the socket interface to the pigpio daemon. 

*Features*

o hardware timed PWM on any of GPIO 0-31

o hardware timed servo pulses on any of GPIO 0-31

o callbacks when any of GPIO 0-31 change state

o callbacks at timed intervals

o reading/writing all of the GPIO in a bank as one operation

o individually setting GPIO modes, reading and writing

o notifications when any of GPIO 0-31 change state

o the construction of output waveforms with microsecond timing

o rudimentary permission control over GPIO

o a simple interface to start and stop new threads

o I2C, SPI, and serial link wrappers

o creating and running scripts on the pigpio daemon

*GPIO*

ALL GPIO are identified by their Broadcom number.

*Notes*

The PWM and servo pulses are timed using the DMA and PWM/PCM peripherals.

*Usage*

Include <pigpiod_if.h> in your source files.

Assuming your source is in prog.c use the following command to build

. .
gcc -Wall -pthread -o prog prog.c -lpigpiod_if -lrt
. .

to run make sure the pigpio daemon is running

. .
sudo pigpiod

 ./prog # sudo is not required to run programs linked to pigpiod_if
. .

For examples see x_pigpiod_if.c within the pigpio archive file.

*Notes*

All the functions which return an int return < 0 on error

TEXT*/

/*OVERVIEW

ESSENTIAL

pigpio_start               Connects to the pigpio daemon
pigpio_stop                Disconnects from the pigpio daemon

BEGINNER

set_mode                   Set a GPIO mode
get_mode                   Get a GPIO mode

set_pull_up_down           Set/clear GPIO pull up/down resistor

gpio_read                  Read a GPIO
gpio_write                 Write a GPIO

set_PWM_dutycycle          Start/stop PWM pulses on a GPIO
get_PWM_dutycycle          Get the PWM dutycycle in use on a GPIO

set_servo_pulsewidth       Start/stop servo pulses on a GPIO
get_servo_pulsewidth       Get the servo pulsewidth in use on a GPIO

callback                   Create GPIO level change callback
callback_ex                Create GPIO level change callback
callback_cancel            Cancel a callback
wait_for_edge              Wait for GPIO level change

INTERMEDIATE

gpio_trigger               Send a trigger pulse to a GPIO.

set_watchdog               Set a watchdog on a GPIO.

set_PWM_range              Configure PWM range for a GPIO
get_PWM_range              Get configured PWM range for a GPIO

set_PWM_frequency          Configure PWM frequency for a GPIO
get_PWM_frequency          Get configured PWM frequency for a GPIO

read_bank_1                Read all GPIO in bank 1
read_bank_2                Read all GPIO in bank 2

clear_bank_1               Clear selected GPIO in bank 1
clear_bank_2               Clear selected GPIO in bank 2

set_bank_1                 Set selected GPIO in bank 1
set_bank_2                 Set selected GPIO in bank 2

start_thread               Start a new thread
stop_thread                Stop a previously started thread

ADVANCED

get_PWM_real_range         Get underlying PWM range for a GPIO

notify_open                Request a notification handle
notify_begin               Start notifications for selected GPIO
notify_pause               Pause notifications
notify_close               Close a notification

bb_serial_read_open        Opens a GPIO for bit bang serial reads
bb_serial_read             Reads bit bang serial data from a GPIO
bb_serial_read_close       Closes a GPIO for bit bang serial reads
bb_serial_invert           Invert serial logic (1 invert, 0 normal)

hardware_clock             Start hardware clock on supported GPIO
hardware_PWM               Start hardware PWM on supported GPIO

set_glitch_filter         Set a glitch filter on a GPIO
set_noise_filter          Set a noise filter on a GPIO

SCRIPTS

store_script               Store a script
run_script                 Run a stored script
script_status              Get script status and parameters
stop_script                Stop a running script
delete_script              Delete a stored script

WAVES

wave_clear                 Deletes all waveforms

wave_add_new               Starts a new waveform
wave_add_generic           Adds a series of pulses to the waveform
wave_add_serial            Adds serial data to the waveform

wave_create                Creates a waveform from added data
wave_delete                Deletes one or more waveforms

wave_send_once             Transmits a waveform once
wave_send_repeat           Transmits a waveform repeatedly

wave_chain                 Transmits a chain of waveforms

wave_tx_busy               Checks to see if the waveform has ended
wave_tx_stop               Aborts the current waveform

wave_get_micros            Length in microseconds of the current waveform
wave_get_high_micros       Length of longest waveform so far
wave_get_max_micros        Absolute maximum allowed micros

wave_get_pulses            Length in pulses of the current waveform
wave_get_high_pulses       Length of longest waveform so far
wave_get_max_pulses        Absolute maximum allowed pulses

wave_get_cbs               Length in cbs of the current waveform
wave_get_high_cbs          Length of longest waveform so far
wave_get_max_cbs           Absolute maximum allowed cbs

I2C

i2c_open                   Opens an I2C device
i2c_close                  Closes an I2C device

i2c_write_quick            smbus write quick
i2c_write_byte             smbus write byte
i2c_read_byte              smbus read byte
i2c_write_byte_data        smbus write byte data
i2c_write_word_data        smbus write word data
i2c_read_byte_data         smbus read byte data
i2c_read_word_data         smbus read word data
i2c_process_call           smbus process call
i2c_write_block_data       smbus write block data
i2c_read_block_data        smbus read block data
i2c_block_process_call     smbus block process call

i2c_write_i2c_block_data   smbus write I2C block data
i2c_read_i2c_block_data    smbus read I2C block data

i2c_read_device            Reads the raw I2C device
i2c_write_device           Writes the raw I2C device

i2c_zip                    Performs multiple I2C transactions

bb_i2c_open                Opens GPIO for bit banging I2C
bb_i2c_close               Closes GPIO for bit banging I2C
bb_i2c_zip                 Performs multiple bit banged I2C transactions

SPI

spi_open                   Opens a SPI device
spi_close                  Closes a SPI device

spi_read                   Reads bytes from a SPI device
spi_write                  Writes bytes to a SPI device
spi_xfer                   Transfers bytes with a SPI device

SERIAL

serial_open                Opens a serial device
serial_close               Closes a serial device

serial_write_byte          Writes a byte to a serial device
serial_read_byte           Reads a byte from a serial device
serial_write               Writes bytes to a serial device
serial_read                Reads bytes from a serial device

serial_data_available      Returns number of bytes ready to be read

CUSTOM

custom_1                   User custom function 1
custom_2                   User custom function 2

UTILITIES

get_current_tick           Get current tick (microseconds)

get_hardware_revision      Get hardware revision
get_pigpio_version         Get the pigpio version
pigpiod_if_version         Get the pigpiod_if version

pigpio_error               Get a text description of an error code.

time_sleep                 Sleeps for a float number of seconds
time_time                  Float number of seconds since the epoch

OVERVIEW*/

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CBFunc_t)  (unsigned user_gpio, unsigned level, uint32_t tick);

typedef void (*CBFuncEx_t)
   (unsigned user_gpio, unsigned level, uint32_t tick, void * user);

typedef struct callback_s callback_t;

/*F*/
double time_time(void);
/*D
Return the current time in seconds since the Epoch.
D*/

/*F*/
void time_sleep(double seconds);
/*D
Delay execution for a given number of seconds.

. .
seconds: the number of seconds to delay.
. .
D*/

/*F*/
char *pigpio_error(int errnum);
/*D
Return a text description for an error code.

. .
errnum: the error code.
. .
D*/

/*F*/
unsigned pigpiod_if_version(void);
/*D
Return the pigpiod_if version.
D*/

/*F*/
pthread_t *start_thread(gpioThreadFunc_t thread_func, void *userdata);
/*D
Starts a new thread of execution with thread_func as the main routine.

. .
thread_func: the main function for the new thread.
   userdata: a pointer to an arbitrary argument.
. .

Returns a pointer to pthread_t if OK, otherwise NULL.

The function is passed the single argument userdata.

The thread can be cancelled by passing the pointer to pthread_t to
[*stop_thread*].
D*/

/*F*/
void stop_thread(pthread_t *pth);
/*D
Cancels the thread pointed at by pth.

. .
pth: the thread to be stopped.
. .

No value is returned.

The thread to be stopped should have been started with [*start_thread*].
D*/

/*F*/
int pigpio_start(char *addrStr, char *portStr);
/*D
Connect to the pigpio daemon.  Reserving command and
notification streams.

. .
addrStr: specifies the host or IP address of the Pi running the
         pigpio daemon.  It may be NULL in which case localhost
         is used unless overridden by the PIGPIO_ADDR environment
         variable.

portStr: specifies the port address used by the Pi running the
         pigpio daemon.  It may be NULL in which case "8888"
         is used unless overridden by the PIGPIO_PORT environment
         variable.
. .
D*/

/*F*/
void pigpio_stop(void);
/*D
Terminates the connection to the pigpio daemon and releases
resources used by the library.
D*/

/*F*/
int set_mode(unsigned gpio, unsigned mode);
/*D
Set the GPIO mode.

. .
gpio: 0-53.
mode: PI_INPUT, PI_OUTPUT, PI_ALT0, PI_ALT1,
      PI_ALT2, PI_ALT3, PI_ALT4, PI_ALT5.
. .

Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_MODE,
or PI_NOT_PERMITTED.
D*/

/*F*/
int get_mode(unsigned gpio);
/*D
Get the GPIO mode.

. .
gpio: 0-53.
. .

Returns the GPIO mode if OK, otherwise PI_BAD_GPIO.
D*/

/*F*/
int set_pull_up_down(unsigned gpio, unsigned pud);
/*D
Set or clear the GPIO pull-up/down resistor.

. .
gpio: 0-53.
 pud: PI_PUD_UP, PI_PUD_DOWN, PI_PUD_OFF.
. .

Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_PUD,
or PI_NOT_PERMITTED.
D*/

/*F*/
int gpio_read(unsigned gpio);
/*D
Read the GPIO level.

. .
gpio:0-53.
. .

Returns the GPIO level if OK, otherwise PI_BAD_GPIO.
D*/

/*F*/
int gpio_write(unsigned gpio, unsigned level);
/*D
Write the GPIO level.

. .
 gpio: 0-53.
level: 0, 1.
. .

Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_LEVEL,
or PI_NOT_PERMITTED.

Notes

If PWM or servo pulses are active on the GPIO they are switched off.
D*/

/*F*/
int set_PWM_dutycycle(unsigned user_gpio, unsigned dutycycle);
/*D
Start (non-zero dutycycle) or stop (0) PWM pulses on the GPIO.

. .
user_gpio: 0-31.
dutycycle: 0-range (range defaults to 255).
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_DUTYCYCLE,
or PI_NOT_PERMITTED.
Notes

The [*set_PWM_range*] function may be used to change the
default range of 255.
D*/

/*F*/
int get_PWM_dutycycle(unsigned user_gpio);
/*D
Return the PWM dutycycle in use on a GPIO.

. .
user_gpio: 0-31.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_NOT_PWM_GPIO.

For normal PWM the dutycycle will be out of the defined range
for the GPIO (see [*get_PWM_range*]).

If a hardware clock is active on the GPIO the reported dutycycle
will be 500000 (500k) out of 1000000 (1M).

If hardware PWM is active on the GPIO the reported dutycycle
will be out of a 1000000 (1M).
D*/

/*F*/
int set_PWM_range(unsigned user_gpio, unsigned range);
/*D
Set the range of PWM values to be used on the GPIO.

. .
user_gpio: 0-31.
    range: 25-40000.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_DUTYRANGE,
or PI_NOT_PERMITTED.

Notes

If PWM is currently active on the GPIO its dutycycle will be
scaled to reflect the new range.

The real range, the number of steps between fully off and fully on
for each of the 18 available GPIO frequencies is

. .
  25(#1),    50(#2),   100(#3),   125(#4),    200(#5),    250(#6),
 400(#7),   500(#8),   625(#9),   800(#10),  1000(#11),  1250(#12),
2000(#13), 2500(#14), 4000(#15), 5000(#16), 10000(#17), 20000(#18)
. .

The real value set by set_PWM_range is (dutycycle * real range) / range.
D*/

/*F*/
int get_PWM_range(unsigned user_gpio);
/*D
Get the range of PWM values being used on the GPIO.

. .
user_gpio: 0-31.
. .

Returns the dutycycle range used for the GPIO if OK,
otherwise PI_BAD_USER_GPIO.

If a hardware clock or hardware PWM is active on the GPIO the
reported range will be 1000000 (1M).
D*/

/*F*/
int get_PWM_real_range(unsigned user_gpio);
/*D
Get the real underlying range of PWM values being used on the GPIO.

. .
user_gpio: 0-31.
. .

Returns the real range used for the GPIO if OK,
otherwise PI_BAD_USER_GPIO.

If a hardware clock is active on the GPIO the reported
real range will be 1000000 (1M).

If hardware PWM is active on the GPIO the reported real range
will be approximately 250M divided by the set PWM frequency.

D*/

/*F*/
int set_PWM_frequency(unsigned user_gpio, unsigned frequency);
/*D
Set the frequency (in Hz) of the PWM to be used on the GPIO.

. .
user_gpio: 0-31.
frequency: >=0 (Hz).
. .

Returns the numerically closest frequency if OK, otherwise
PI_BAD_USER_GPIO or PI_NOT_PERMITTED.

If PWM is currently active on the GPIO it will be switched
off and then back on at the new frequency.

Each GPIO can be independently set to one of 18 different
PWM frequencies.

The selectable frequencies depend upon the sample rate which
may be 1, 2, 4, 5, 8, or 10 microseconds (default 5).  The
sample rate is set when the pigpio daemon is started.

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
D*/

/*F*/
int get_PWM_frequency(unsigned user_gpio);
/*D
Get the frequency of PWM being used on the GPIO.

. .
user_gpio: 0-31.
. .

For normal PWM the frequency will be that defined for the GPIO by
[*set_PWM_frequency*].

If a hardware clock is active on the GPIO the reported frequency
will be that set by [*hardware_clock*].

If hardware PWM is active on the GPIO the reported frequency
will be that set by [*hardware_PWM*].

Returns the frequency (in hertz) used for the GPIO if OK,
otherwise PI_BAD_USER_GPIO.
D*/

/*F*/
int set_servo_pulsewidth(unsigned user_gpio, unsigned pulsewidth);
/*D
Start (500-2500) or stop (0) servo pulses on the GPIO.

. .
 user_gpio: 0-31.
pulsewidth: 0 (off), 500 (anti-clockwise) - 2500 (clockwise).
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_PULSEWIDTH or
PI_NOT_PERMITTED.

The selected pulsewidth will continue to be transmitted until
changed by a subsequent call to set_servo_pulsewidth.

The pulsewidths supported by servos varies and should probably be
determined by experiment. A value of 1500 should always be safe and
represents the mid-point of rotation.

You can DAMAGE a servo if you command it to move beyond its limits.

OTHER UPDATE RATES:

This function updates servos at 50Hz.  If you wish to use a different
update frequency you will have to use the PWM functions.

. .
Update Rate (Hz)     50   100  200  400  500
1E6/Hz            20000 10000 5000 2500 2000
. .

Firstly set the desired PWM frequency using [*set_PWM_frequency*].

Then set the PWM range using [*set_PWM_range*] to 1E6/Hz.
Doing this allows you to use units of microseconds when setting
the servo pulsewidth.

E.g. If you want to update a servo connected to GPIO 25 at 400Hz

. .
set_PWM_frequency(25, 400);
set_PWM_range(25, 2500);
. .

Thereafter use the [*set_PWM_dutycycle*] function to move the servo,
e.g. set_PWM_dutycycle(25, 1500) will set a 1500 us pulse. 
D*/

/*F*/
int get_servo_pulsewidth(unsigned user_gpio);
/*D
Return the servo pulsewidth in use on a GPIO.

. .
user_gpio: 0-31.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_NOT_SERVO_GPIO.
D*/

/*F*/
int notify_open(void);
/*D
Get a free notification handle.

Returns a handle greater than or equal to zero if OK,
otherwise PI_NO_HANDLE.

A notification is a method for being notified of GPIO state
changes via a pipe.

Pipes are only accessible from the local machine so this function
serves no purpose if you are using the library from a remote machine.
The in-built (socket) notifications provided by [*callback*]
should be used instead.

Notifications for handle x will be available at the pipe
named /dev/pigpiox (where x is the handle number).
E.g. if the function returns 15 then the notifications must be
read from /dev/pigpio15.
D*/

/*F*/
int notify_begin(unsigned handle, uint32_t bits);
/*D
Start notifications on a previously opened handle.

. .
handle: 0-31 (as returned by [*notify_open*])
  bits: a mask indicating the GPIO to be notified.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

The notification sends state changes for each GPIO whose
corresponding bit in bits is set.

Each notification occupies 12 bytes in the fifo as follows:

. .
typedef struct
{
   uint16_t seqno;
   uint16_t flags;
   uint32_t tick;
   uint32_t level;
} gpioReport_t;
. .

seqno: starts at 0 each time the handle is opened and then increments
by one for each report.

flags: two flags are defined, PI_NTFY_FLAGS_WDOG and PI_NTFY_FLAGS_ALIVE.

PI_NTFY_FLAGS_WDOG, if bit 5 is set then bits 0-4 of the flags
indicate a GPIO which has had a watchdog timeout.

PI_NTFY_FLAGS_ALIVE, if bit 6 is set this indicates a keep alive
signal on the pipe/socket and is sent once a minute in the absence
of other notification activity.

tick: the number of microseconds since system boot.  It wraps around
after 1h12m.

level: indicates the level of each GPIO.  If bit 1<<x is set then
GPIO x is high.
D*/

/*F*/
int notify_pause(unsigned handle);
/*D
Pause notifications on a previously opened handle.

. .
handle: 0-31 (as returned by [*notify_open*])
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

Notifications for the handle are suspended until
[*notify_begin*] is called again.
D*/

/*F*/
int notify_close(unsigned handle);
/*D
Stop notifications on a previously opened handle and
release the handle for reuse.

. .
handle: 0-31 (as returned by [*notify_open*])
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.
D*/

/*F*/
int set_watchdog(unsigned user_gpio, unsigned timeout);
/*D
Sets a watchdog for a GPIO.

. .
user_gpio: 0-31.
  timeout: 0-60000.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO
or PI_BAD_WDOG_TIMEOUT.

The watchdog is nominally in milliseconds.

Only one watchdog may be registered per GPIO.

The watchdog may be cancelled by setting timeout to 0.

Once a watchdog has been started callbacks for the GPIO will be
triggered every timeout interval after the last GPIO activity.

The callback will receive the special level PI_TIMEOUT.
D*/

/*F*/
int set_glitch_filter(unsigned user_gpio, unsigned steady);
/*D
Sets a glitch filter on a GPIO.

Level changes on the GPIO are not reported unless the level
has been stable for at least [*steady*] microseconds.  The
level is then reported.  Level changes of less than [*steady*]
microseconds are ignored.

. .
user_gpio: 0-31
   steady: 0-300000
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_BAD_FILTER.

This filter affects the GPIO samples returned to callbacks set up
with [*callback*], [*callback_ex*] and [*wait_for_edge*].

It does not affect levels read by [*gpio_read*],
[*read_bank_1*], or [*read_bank_2*].
Each (stable) edge will be timestamped [*steady*] microseconds
after it was first detected.
D*/

/*F*/
int set_noise_filter(unsigned user_gpio, unsigned steady, unsigned active);
/*D
Sets a noise filter on a GPIO.

Level changes on the GPIO are ignored until a level which has
been stable for [*steady*] microseconds is detected.  Level changes
on the GPIO are then reported for [*active*] microseconds after
which the process repeats.

. .
user_gpio: 0-31
   steady: 0-300000
   active: 0-1000000
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_BAD_FILTER.

This filter affects the GPIO samples returned to callbacks set up
with [*callback*], [*callback_ex*] and [*wait_for_edge*].

It does not affect levels read by [*gpio_read*],
[*read_bank_1*], or [*read_bank_2*].

Level changes before and after the active period may
be reported.  Your software must be designed to cope with
such reports.
D*/

/*F*/
uint32_t read_bank_1(void);
/*D
Read the levels of the bank 1 GPIO (GPIO 0-31).

The returned 32 bit integer has a bit set if the corresponding
GPIO is logic 1.  GPIO n has bit value (1<<n).
D*/

/*F*/
uint32_t read_bank_2(void);
/*D
Read the levels of the bank 2 GPIO (GPIO 32-53).

The returned 32 bit integer has a bit set if the corresponding
GPIO is logic 1.  GPIO n has bit value (1<<(n-32)).
D*/

/*F*/
int clear_bank_1(uint32_t bits);
/*D
Clears GPIO 0-31 if the corresponding bit in bits is set.

. .
bits: a bit mask with 1 set if the corresponding GPIO is
      to be cleared.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.
D*/

/*F*/
int clear_bank_2(uint32_t bits);
/*D
Clears GPIO 32-53 if the corresponding bit (0-21) in bits is set.

. .
bits: a bit mask with 1 set if the corresponding GPIO is
      to be cleared.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.
D*/

/*F*/
int set_bank_1(uint32_t bits);
/*D
Sets GPIO 0-31 if the corresponding bit in bits is set.

. .
bits: a bit mask with 1 set if the corresponding GPIO is
      to be set.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.
D*/

/*F*/
int set_bank_2(uint32_t bits);
/*D
Sets GPIO 32-53 if the corresponding bit (0-21) in bits is set.

. .
bits: a bit mask with 1 set if the corresponding GPIO is
      to be set.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.
D*/


/*F*/
int hardware_clock(unsigned gpio, unsigned clkfreq);
/*D
Starts a hardware clock on a GPIO at the specified frequency.
Frequencies above 30MHz are unlikely to work.

. .
     gpio: see description
frequency: 0 (off) or 4689-250000000 (250M)
. .

Returns 0 if OK, otherwise PI_NOT_PERMITTED, PI_BAD_GPIO,
PI_NOT_HCLK_GPIO, PI_BAD_HCLK_FREQ,or PI_BAD_HCLK_PASS.

The same clock is available on multiple GPIO.  The latest
frequency setting will be used by all GPIO which share a clock.

The GPIO must be one of the following.

. .
4   clock 0  All models
5   clock 1  All models but A and B (reserved for system use)
6   clock 2  All models but A and B
20  clock 0  All models but A and B
21  clock 1  All models but A and Rev.2 B (reserved for system use)

32  clock 0  Compute module only
34  clock 0  Compute module only
42  clock 1  Compute module only (reserved for system use)
43  clock 2  Compute module only
44  clock 1  Compute module only (reserved for system use)
. .

Access to clock 1 is protected by a password as its use will likely
crash the Pi.  The password is given by or'ing 0x5A000000 with the
GPIO number.
D*/


/*F*/
int hardware_PWM(unsigned gpio, unsigned PWMfreq, uint32_t PWMduty);
/*D
Starts hardware PWM on a GPIO at the specified frequency and dutycycle.
Frequencies above 30MHz are unlikely to work.

NOTE: Any waveform started by [*wave_send_once*], [*wave_send_repeat*],
or [*wave_chain*] will be cancelled.

This function is only valid if the pigpio main clock is PCM.  The
main clock defaults to PCM but may be overridden when the pigpio
daemon is started (option -t).

. .
   gpio: see descripton
PWMfreq: 0 (off) or 1-125000000 (125M)
PWMduty: 0 (off) to 1000000 (1M)(fully on)
. .

Returns 0 if OK, otherwise PI_NOT_PERMITTED, PI_BAD_GPIO,
PI_NOT_HPWM_GPIO, PI_BAD_HPWM_DUTY, PI_BAD_HPWM_FREQ,
or PI_HPWM_ILLEGAL.

The same PWM channel is available on multiple GPIO.  The latest
frequency and dutycycle setting will be used by all GPIO which
share a PWM channel.

The GPIO must be one of the following.

. .
12  PWM channel 0  All models but A and B
13  PWM channel 1  All models but A and B
18  PWM channel 0  All models
19  PWM channel 1  All models but A and B

40  PWM channel 0  Compute module only
41  PWM channel 1  Compute module only
45  PWM channel 1  Compute module only
52  PWM channel 0  Compute module only
53  PWM channel 1  Compute module only
. .
D*/


/*F*/
uint32_t get_current_tick(void);
/*D
Gets the current system tick.

Tick is the number of microseconds since system boot.

As tick is an unsigned 32 bit quantity it wraps around after
2**32 microseconds, which is approximately 1 hour 12 minutes.

D*/

/*F*/
uint32_t get_hardware_revision(void);
/*D
Get the Pi's hardware revision number.

The hardware revision is the last few characters on the Revision line
of /proc/cpuinfo.

If the hardware revision can not be found or is not a valid
hexadecimal number the function returns 0.

The revision number can be used to determine the assignment of GPIO
to pins (see [*gpio*]).

There are at least three types of board.

Type 1 boards have hardware revision numbers of 2 and 3.

Type 2 boards have hardware revision numbers of 4, 5, 6, and 15.

Type 3 boards have hardware revision numbers of 16 or greater.
D*/

/*F*/
uint32_t get_pigpio_version(void);
/*D
Returns the pigpio version.
D*/


/*F*/
int wave_clear(void);
/*D
This function clears all waveforms and any data added by calls to the
[*wave_add_**] functions.

Returns 0 if OK.
D*/

/*F*/
int wave_add_new(void);
/*D
This function starts a new empty waveform.  You wouldn't normally need
to call this function as it is automatically called after a waveform is
created with the [*wave_create*] function.

Returns 0 if OK.
D*/

/*F*/
int wave_add_generic(unsigned numPulses, gpioPulse_t *pulses);
/*D
This function adds a number of pulses to the current waveform.

. .
numPulses: the number of pulses.
   pulses: an array of pulses.
. .

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_TOO_MANY_PULSES.

The pulses are interleaved in time order within the existing waveform
(if any).

Merging allows the waveform to be built in parts, that is the settings
for GPIO#1 can be added, and then GPIO#2 etc.

If the added waveform is intended to start after or within the existing
waveform then the first pulse should consist solely of a delay.
D*/

/*F*/
int wave_add_serial
   (unsigned user_gpio, unsigned baud, unsigned data_bits,
    unsigned stop_bits, unsigned offset, unsigned numBytes, char *str);
/*D
This function adds a waveform representing serial data to the
existing waveform (if any).  The serial data starts offset
microseconds from the start of the waveform.

. .
user_gpio: 0-31.
     baud: 50-1000000
data_bits: number of data bits (1-32)
stop_bits: number of stop half bits (2-8)
   offset: >=0
 numBytes: >=1
      str: an array of chars.
. .

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD, PI_BAD_DATABITS,
PI_BAD_STOP_BITS, PI_TOO_MANY_CHARS, PI_BAD_SER_OFFSET,
or PI_TOO_MANY_PULSES.

NOTES:

The serial data is formatted as one start bit, [*data_bits*] data bits,
and [*stop_bits*]/2 stop bits.

It is legal to add serial data streams with different baud rates to
the same waveform.

[*numBytes*] is the number of bytes of data in str.

The bytes required for each character depend upon [*data_bits*].

For [*data_bits*] 1-8 there will be one byte per character. 
For [*data_bits*] 9-16 there will be two bytes per character. 
For [*data_bits*] 17-32 there will be four bytes per character.
D*/

/*F*/
int wave_create(void);
/*D
This function creates a waveform from the data provided by the prior
calls to the [*wave_add_**] functions.  Upon success a wave id
greater than or equal to 0 is returned, otherwise PI_EMPTY_WAVEFORM,
PI_TOO_MANY_CBS, PI_TOO_MANY_OOL, or PI_NO_WAVEFORM_ID.

The data provided by the [*wave_add_**] functions is consumed by this
function.

As many waveforms may be created as there is space available.  The
wave id is passed to [*wave_send_**] to specify the waveform to transmit.

Normal usage would be

Step 1. [*wave_clear*] to clear all waveforms and added data.

Step 2. [*wave_add_**] calls to supply the waveform data.

Step 3. [*wave_create*] to create the waveform and get a unique id

Repeat steps 2 and 3 as needed.

Step 4. [*wave_send_**] with the id of the waveform to transmit.

A waveform comprises one or more pulses.  Each pulse consists of a
[*gpioPulse_t*] structure.

. .
typedef struct
{
   uint32_t gpioOn;
   uint32_t gpioOff;
   uint32_t usDelay;
} gpioPulse_t;
. .

The fields specify

1) the GPIO to be switched on at the start of the pulse. 
2) the GPIO to be switched off at the start of the pulse. 
3) the delay in microseconds before the next pulse. 

Any or all the fields can be zero.  It doesn't make any sense to
set all the fields to zero (the pulse will be ignored).

When a waveform is started each pulse is executed in order with the
specified delay between the pulse and the next.

Returns the new waveform id if OK, otherwise PI_EMPTY_WAVEFORM,
PI_NO_WAVEFORM_ID, PI_TOO_MANY_CBS, or PI_TOO_MANY_OOL.
D*/


/*F*/
int wave_delete(unsigned wave_id);
/*D
This function deletes the waveform with id wave_id.

. .
wave_id: >=0, as returned by [*wave_create*].
. .

Wave ids are allocated in order, 0, 1, 2, etc.

The wave is flagged for deletion.  The resources used by the wave
will only be reused when either of the following apply.

- all waves with higher numbered wave ids have been deleted or have
been flagged for deletion.

- a new wave is created which uses exactly the same resources as
the current wave (see the C source for gpioWaveCreate for details).

Returns 0 if OK, otherwise PI_BAD_WAVE_ID.
D*/

/*F*/
int wave_send_once(unsigned wave_id);
/*D
This function transmits the waveform with id wave_id.  The waveform
is sent once.

NOTE: Any hardware PWM started by [*hardware_PWM*] will be cancelled.

. .
wave_id: >=0, as returned by [*wave_create*].
. .

Returns the number of DMA control blocks in the waveform if OK,
otherwise PI_BAD_WAVE_ID, or PI_BAD_WAVE_MODE.
D*/

/*F*/
int wave_send_repeat(unsigned wave_id);
/*D
This function transmits the waveform with id wave_id.  The waveform
cycles until cancelled (either by the sending of a new waveform or
by [*wave_tx_stop*]).

NOTE: Any hardware PWM started by [*hardware_PWM*] will be cancelled.

. .
wave_id: >=0, as returned by [*wave_create*].
. .

Returns the number of DMA control blocks in the waveform if OK,
otherwise PI_BAD_WAVE_ID, or PI_BAD_WAVE_MODE.
D*/

/*F*/
int wave_chain(char *buf, unsigned bufSize);
/*D
This function transmits a chain of waveforms.

NOTE: Any hardware PWM started by [*hardware_PWM*] will be cancelled.

The waves to be transmitted are specified by the contents of buf
which contains an ordered list of [*wave_id*]s and optional command
codes and related data.

. .
    buf: pointer to the wave_ids and optional command codes
bufSize: the number of bytes in buf
. .

Returns 0 if OK, otherwise PI_CHAIN_NESTING, PI_CHAIN_LOOP_CNT, PI_BAD_CHAIN_LOOP, PI_BAD_CHAIN_CMD, PI_CHAIN_COUNTER,
PI_BAD_CHAIN_DELAY, PI_CHAIN_TOO_BIG, or PI_BAD_WAVE_ID.

Each wave is transmitted in the order specified.  A wave may
occur multiple times per chain.

A blocks of waves may be transmitted multiple times by using
the loop commands. The block is bracketed by loop start and
end commands.  Loops may be nested.

Delays between waves may be added with the delay command.

The following command codes are supported:

Name         @ Cmd & Data @ Meaning
Loop Start   @ 255 0      @ Identify start of a wave block
Loop Repeat  @ 255 1 x y  @ loop x + y*256 times
Delay        @ 255 2 x y  @ delay x + y*256 microseconds
Loop Forever @ 255 3      @ loop forever

If present Loop Forever must be the last entry in the chain.

The code is currently dimensioned to support a chain with roughly
600 entries and 20 loop counters.

...
#include <stdio.h>
#include <pigpiod_if.h>

#define WAVES 5
#define GPIO 4

int main(int argc, char *argv[])
{
   int i, wid[WAVES];

   if (pigpio_start(0, 0)<0) return -1;

   set_mode(GPIO, PI_OUTPUT);

   for (i=0; i<WAVES; i++)
   {
      wave_add_generic(2, (gpioPulse_t[])
         {{1<<GPIO, 0,        20},
          {0, 1<<GPIO, (i+1)*200}});

      wid[i] = wave_create();
   }

   wave_chain((char []) {
      wid[4], wid[3], wid[2],       // transmit waves 4+3+2
      255, 0,                       // loop start
         wid[0], wid[0], wid[0],    // transmit waves 0+0+0
         255, 0,                    // loop start
            wid[0], wid[1],         // transmit waves 0+1
            255, 2, 0x88, 0x13,     // delay 5000us
         255, 1, 30, 0,             // loop end (repeat 30 times)
         255, 0,                    // loop start
            wid[2], wid[3], wid[0], // transmit waves 2+3+0
            wid[3], wid[1], wid[2], // transmit waves 3+1+2
         255, 1, 10, 0,             // loop end (repeat 10 times)
      255, 1, 5, 0,                 // loop end (repeat 5 times)
      wid[4], wid[4], wid[4],       // transmit waves 4+4+4
      255, 2, 0x20, 0x4E,           // delay 20000us
      wid[0], wid[0], wid[0],       // transmit waves 0+0+0

      }, 46);

   while (wave_tx_busy()) time_sleep(0.1);

   for (i=0; i<WAVES; i++) wave_delete(wid[i]);

   pigpio_stop();
}
...
D*/


/*F*/
int wave_tx_busy(void);
/*D
This function checks to see if a waveform is currently being
transmitted.

Returns 1 if a waveform is currently being transmitted, otherwise 0.
D*/

/*F*/
int wave_tx_stop(void);
/*D
This function stops the transmission of the current waveform.

Returns 0 if OK.

This function is intended to stop a waveform started with the repeat mode.
D*/

/*F*/
int wave_get_micros(void);
/*D
This function returns the length in microseconds of the current
waveform.
D*/

/*F*/
int wave_get_high_micros(void);
/*D
This function returns the length in microseconds of the longest waveform
created since the pigpio daemon was started.
D*/

/*F*/
int wave_get_max_micros(void);
/*D
This function returns the maximum possible size of a waveform in 
microseconds.
D*/

/*F*/
int wave_get_pulses(void);
/*D
This function returns the length in pulses of the current waveform.
D*/

/*F*/
int wave_get_high_pulses(void);
/*D
This function returns the length in pulses of the longest waveform
created since the pigpio daemon was started.
D*/

/*F*/
int wave_get_max_pulses(void);
/*D
This function returns the maximum possible size of a waveform in pulses.
D*/

/*F*/
int wave_get_cbs(void);
/*D
This function returns the length in DMA control blocks of the current
waveform.
D*/

/*F*/
int wave_get_high_cbs(void);
/*D
This function returns the length in DMA control blocks of the longest
waveform created since the pigpio daemon was started.
D*/

/*F*/
int wave_get_max_cbs(void);
/*D
This function returns the maximum possible size of a waveform in DMA
control blocks.
D*/

/*F*/
int gpio_trigger(unsigned user_gpio, unsigned pulseLen, unsigned level);
/*D
This function sends a trigger pulse to a GPIO.  The GPIO is set to
level for pulseLen microseconds and then reset to not level.

. .
user_gpio: 0-31.
 pulseLen: 1-100.
    level: 0,1.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_LEVEL,
PI_BAD_PULSELEN, or PI_NOT_PERMITTED.
D*/

/*F*/
int store_script(char *script);
/*D
This function stores a script for later execution.

See [[http://abyz.me.uk/rpi/pigpio/pigs.html#Scripts]] for details.

. .
script: the text of the script.
. .

The function returns a script id if the script is valid,
otherwise PI_BAD_SCRIPT.
D*/

/*F*/
int run_script(unsigned script_id, unsigned numPar, uint32_t *param);
/*D
This function runs a stored script.

. .
script_id: >=0, as returned by [*store_script*].
   numPar: 0-10, the number of parameters.
    param: an array of parameters.
. .

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID, or
PI_TOO_MANY_PARAM

param is an array of up to 10 parameters which may be referenced in
the script as p0 to p9.
D*/

/*F*/
int script_status(unsigned script_id, uint32_t *param);
/*D
This function returns the run status of a stored script as well
as the current values of parameters 0 to 9.

. .
script_id: >=0, as returned by [*store_script*].
    param: an array to hold the returned 10 parameters.
. .

The function returns greater than or equal to 0 if OK,
otherwise PI_BAD_SCRIPT_ID.

The run status may be

. .
PI_SCRIPT_INITING
PI_SCRIPT_HALTED
PI_SCRIPT_RUNNING
PI_SCRIPT_WAITING
PI_SCRIPT_FAILED
. .

The current value of script parameters 0 to 9 are returned in param.
D*/

/*F*/
int stop_script(unsigned script_id);
/*D
This function stops a running script.

. .
script_id: >=0, as returned by [*store_script*].
. .

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.
D*/

/*F*/
int delete_script(unsigned script_id);
/*D
This function deletes a stored script.

. .
script_id: >=0, as returned by [*store_script*].
. .

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.
D*/

/*F*/
int bb_serial_read_open(unsigned user_gpio, unsigned baud, unsigned data_bits);
/*D
This function opens a GPIO for bit bang reading of serial data.

. .
user_gpio: 0-31.
     baud: 50-250000
data_bits: 1-32
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD,
or PI_GPIO_IN_USE.

The serial data is returned in a cyclic buffer and is read using
bb_serial_read.

It is the caller's responsibility to read data from the cyclic buffer
in a timely fashion.
D*/

/*F*/
int bb_serial_read(unsigned user_gpio, void *buf, size_t bufSize);
/*D
This function copies up to bufSize bytes of data read from the
bit bang serial cyclic buffer to the buffer starting at buf.

. .
user_gpio: 0-31, previously opened with [*bb_serial_read_open*].
      buf: an array to receive the read bytes.
  bufSize: >=0
. .

Returns the number of bytes copied if OK, otherwise PI_BAD_USER_GPIO
or PI_NOT_SERIAL_GPIO.

The bytes returned for each character depend upon the number of
data bits [*data_bits*] specified in the [*bb_serial_read_open*] command.

For [*data_bits*] 1-8 there will be one byte per character. 
For [*data_bits*] 9-16 there will be two bytes per character. 
For [*data_bits*] 17-32 there will be four bytes per character.
D*/

/*F*/
int bb_serial_read_close(unsigned user_gpio);
/*D
This function closes a GPIO for bit bang reading of serial data.

. .
user_gpio: 0-31, previously opened with [*bb_serial_read_open*].
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_SERIAL_GPIO.
D*/

/*F*/
int bb_serial_invert(unsigned user_gpio, unsigned invert);
/*D
This function inverts serial logic for big bang serial reads.

. .
user_gpio: 0-31, previously opened with [*bb_serial_read_open*].
   invert: 0-1, 1 invert, 0 normal.
. .

Returns 0 if OK, otherwise PI_NOT_SERIAL_GPIO or PI_BAD_SER_INVERT.
D*/

/*F*/
int i2c_open(unsigned i2c_bus, unsigned i2c_addr, unsigned i2c_flags);
/*D
This returns a handle for the device at address i2c_addr on bus i2c_bus.

. .
  i2c_bus: >=0.
 i2c_addr: 0-0x7F.
i2c_flags: 0.
. .

No flags are currently defined.  This parameter should be set to zero.

Physically buses 0 and 1 are available on the Pi.  Higher numbered buses
will be available if a kernel supported bus multiplexor is being used.

The GPIO used are given in the following table.

      @ SDA @ SCL
I2C 0 @  0  @  1
I2C 1 @  2  @  3

Returns a handle (>=0) if OK, otherwise PI_BAD_I2C_BUS, PI_BAD_I2C_ADDR,
PI_BAD_FLAGS, PI_NO_HANDLE, or PI_I2C_OPEN_FAILED.

For the SMBus commands the low level transactions are shown at the end
of the function description.  The following abbreviations are used.

. .
S     (1 bit) : Start bit
P     (1 bit) : Stop bit
Rd/Wr (1 bit) : Read/Write bit. Rd equals 1, Wr equals 0.
A, NA (1 bit) : Accept and not accept bit. 
Addr  (7 bits): I2C 7 bit address.
Comm  (8 bits): Command byte, a data byte which often selects a register.
Data  (8 bits): A data byte.
Count (8 bits): A data byte containing the length of a block operation.

[..]: Data sent by the device.
. .
D*/

/*F*/
int i2c_close(unsigned handle);
/*D
This closes the I2C device associated with the handle.

. .
handle: >=0, as returned by a call to [*i2c_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.
D*/

/*F*/
int i2c_write_quick(unsigned handle, unsigned bit);
/*D
This sends a single bit (in the Rd/Wr bit) to the device associated
with handle.

. .
handle: >=0, as returned by a call to [*i2c_open*].
   bit: 0-1, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Quick command. SMBus 2.0 5.5.1
. .
S Addr Rd/Wr [A] P
. .
D*/

/*F*/
int i2c_write_byte(unsigned handle, unsigned bVal);
/*D
This sends a single byte to the device associated with handle.

. .
handle: >=0, as returned by a call to [*i2c_open*].
  bVal: 0-0xFF, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Send byte. SMBus 2.0 5.5.2
. .
S Addr Wr [A] Data [A] P
. .
D*/

/*F*/
int i2c_read_byte(unsigned handle);
/*D
This reads a single byte from the device associated with handle.

. .
handle: >=0, as returned by a call to [*i2c_open*].
. .

Returns the byte read (>=0) if OK, otherwise PI_BAD_HANDLE,
or PI_I2C_READ_FAILED.

Receive byte. SMBus 2.0 5.5.3
. .
S Addr Rd [A] [Data] NA P
. .
D*/

/*F*/
int i2c_write_byte_data(unsigned handle, unsigned i2c_reg, unsigned bVal);
/*D
This writes a single byte to the specified register of the device
associated with handle.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to write.
   bVal: 0-0xFF, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Write byte. SMBus 2.0 5.5.4
. .
S Addr Wr [A] Comm [A] Data [A] P
. .
D*/

/*F*/
int i2c_write_word_data(unsigned handle, unsigned i2c_reg, unsigned wVal);
/*D
This writes a single 16 bit word to the specified register of the device
associated with handle.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to write.
   wVal: 0-0xFFFF, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Write word. SMBus 2.0 5.5.4
. .
S Addr Wr [A] Comm [A] DataLow [A] DataHigh [A] P
. .
D*/

/*F*/
int i2c_read_byte_data(unsigned handle, unsigned i2c_reg);
/*D
This reads a single byte from the specified register of the device
associated with handle.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to read.
. .

Returns the byte read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Read byte. SMBus 2.0 5.5.5
. .
S Addr Wr [A] Comm [A] S Addr Rd [A] [Data] NA P
. .
D*/

/*F*/
int i2c_read_word_data(unsigned handle, unsigned i2c_reg);
/*D
This reads a single 16 bit word from the specified register of the device
associated with handle.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to read.
. .

Returns the word read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Read word. SMBus 2.0 5.5.5
. .
S Addr Wr [A] Comm [A] S Addr Rd [A] [DataLow] A [DataHigh] NA P
. .
D*/

/*F*/
int i2c_process_call(unsigned handle, unsigned i2c_reg, unsigned wVal);
/*D
This writes 16 bits of data to the specified register of the device
associated with handle and and reads 16 bits of data in return.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to write/read.
   wVal: 0-0xFFFF, the value to write.
. .

Returns the word read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Process call. SMBus 2.0 5.5.6
. .
S Addr Wr [A] Comm [A] DataLow [A] DataHigh [A]
   S Addr Rd [A] [DataLow] A [DataHigh] NA P
. .
D*/

/*F*/
int i2c_write_block_data(
unsigned handle, unsigned i2c_reg, char *buf, unsigned count);
/*D
This writes up to 32 bytes to the specified register of the device
associated with handle.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to write.
    buf: an array with the data to send.
  count: 1-32, the number of bytes to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Block write. SMBus 2.0 5.5.7
. .
S Addr Wr [A] Comm [A] Count [A] Data [A] Data [A] ... [A] Data [A] P
. .
D*/

/*F*/
int i2c_read_block_data(unsigned handle, unsigned i2c_reg, char *buf);
/*D
This reads a block of up to 32 bytes from the specified register of
the device associated with handle.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to read.
    buf: an array to receive the read data.
. .

The amount of returned data is set by the device.

Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Block read. SMBus 2.0 5.5.7
. .
S Addr Wr [A] Comm [A]
   S Addr Rd [A] [Count] A [Data] A [Data] A ... A [Data] NA P
. .
D*/

/*F*/
int i2c_block_process_call(
unsigned handle, unsigned i2c_reg, char *buf, unsigned count);
/*D
This writes data bytes to the specified register of the device
associated with handle and reads a device specified number
of bytes of data in return.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to write/read.
    buf: an array with the data to send and to receive the read data.
  count: 1-32, the number of bytes to write.
. .


Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

The smbus 2.0 documentation states that a minimum of 1 byte may be
sent and a minimum of 1 byte may be received.  The total number of
bytes sent/received must be 32 or less.

Block write-block read. SMBus 2.0 5.5.8
. .
S Addr Wr [A] Comm [A] Count [A] Data [A] ...
   S Addr Rd [A] [Count] A [Data] ... A P
. .
D*/

/*F*/
int i2c_read_i2c_block_data(
unsigned handle, unsigned i2c_reg, char *buf, unsigned count);
/*D
This reads count bytes from the specified register of the device
associated with handle .  The count may be 1-32.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to read.
    buf: an array to receive the read data.
  count: 1-32, the number of bytes to read.
. .

Returns the number of bytes read (>0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

. .
S Addr Wr [A] Comm [A]
   S Addr Rd [A] [Data] A [Data] A ... A [Data] NA P
. .
D*/


/*F*/
int i2c_write_i2c_block_data(
unsigned handle, unsigned i2c_reg, char *buf, unsigned count);
/*D
This writes 1 to 32 bytes to the specified register of the device
associated with handle.

. .
 handle: >=0, as returned by a call to [*i2c_open*].
i2c_reg: 0-255, the register to write.
    buf: the data to write.
  count: 1-32, the number of bytes to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

. .
S Addr Wr [A] Comm [A] Data [A] Data [A] ... [A] Data [A] P
. .
D*/

/*F*/
int i2c_read_device(unsigned handle, char *buf, unsigned count);
/*D
This reads count bytes from the raw device into buf.

. .
handle: >=0, as returned by a call to [*i2c_open*].
   buf: an array to receive the read data bytes.
 count: >0, the number of bytes to read.
. .

Returns count (>0) if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_READ_FAILED.
D*/

/*F*/
int i2c_write_device(unsigned handle, char *buf, unsigned count);
/*D
This writes count bytes from buf to the raw device.

. .
handle: >=0, as returned by a call to [*i2c_open*].
   buf: an array containing the data bytes to write.
 count: >0, the number of bytes to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.
D*/

/*F*/
int i2c_zip(
   unsigned handle,
   char    *inBuf,
   unsigned inLen,
   char    *outBuf,
   unsigned outLen);
/*D
This function executes a sequence of I2C operations.  The
operations to be performed are specified by the contents of inBuf
which contains the concatenated command codes and associated data.

. .
handle: >=0, as returned by a call to [*i2cOpen*]
 inBuf: pointer to the concatenated I2C commands, see below
 inLen: size of command buffer
outBuf: pointer to buffer to hold returned data
outLen: size of output buffer
. .

Returns >= 0 if OK (the number of bytes read), otherwise
PI_BAD_HANDLE, PI_BAD_POINTER, PI_BAD_I2C_CMD, PI_BAD_I2C_RLEN.
PI_BAD_I2C_WLEN, or PI_BAD_I2C_SEG.

The following command codes are supported:

Name    @ Cmd & Data @ Meaning
End     @ 0          @ No more commands
Escape  @ 1          @ Next P is two bytes
On      @ 2          @ Switch combined flag on
Off     @ 3          @ Switch combined flag off
Address @ 4 P        @ Set I2C address to P
Flags   @ 5 lsb msb  @ Set I2C flags to lsb + (msb << 8)
Read    @ 6 P        @ Read P bytes of data
Write   @ 7 P ...    @ Write P bytes of data

The address, read, and write commands take a parameter P.
Normally P is one byte (0-255).  If the command is preceded by
the Escape command then P is two bytes (0-65535, least significant
byte first).

The address defaults to that associated with the handle.
The flags default to 0.  The address and flags maintain their
previous value until updated.

The returned I2C data is stored in consecutive locations of outBuf.

...
Set address 0x53, write 0x32, read 6 bytes
Set address 0x1E, write 0x03, read 6 bytes
Set address 0x68, write 0x1B, read 8 bytes
End

0x04 0x53   0x07 0x01 0x32   0x06 0x06
0x04 0x1E   0x07 0x01 0x03   0x06 0x06
0x04 0x68   0x07 0x01 0x1B   0x06 0x08
0x00
...

D*/

/*F*/
int bb_i2c_open(unsigned SDA, unsigned SCL, unsigned baud);
/*D
This function selects a pair of GPIO for bit banging I2C at a
specified baud rate.

Bit banging I2C allows for certain operations which are not possible
with the standard I2C driver.

o baud rates as low as 50 
o repeated starts 
o clock stretching 
o I2C on any pair of spare GPIO

. .
 SDA: 0-31
 SCL: 0-31
baud: 50-500000
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_I2C_BAUD, or
PI_GPIO_IN_USE.

NOTE:

The GPIO used for SDA and SCL must have pull-ups to 3V3 connected.  As
a guide the hardware pull-ups on pins 3 and 5 are 1k8 in value.
D*/

/*F*/
int bb_i2c_close(unsigned SDA);
/*D
This function stops bit banging I2C on a pair of GPIO previously
opened with [*bb_i2c_open*].

. .
SDA: 0-31, the SDA GPIO used in a prior call to [*bb_i2c_open*]
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_I2C_GPIO.
D*/

/*F*/
int bb_i2c_zip(
   unsigned SDA,
   char    *inBuf,
   unsigned inLen,
   char    *outBuf,
   unsigned outLen);
/*D
This function executes a sequence of bit banged I2C operations.  The
operations to be performed are specified by the contents of inBuf
which contains the concatenated command codes and associated data.

. .
   SDA: 0-31 (as used in a prior call to [*bb_i2c_open*])
 inBuf: pointer to the concatenated I2C commands, see below
 inLen: size of command buffer
outBuf: pointer to buffer to hold returned data
outLen: size of output buffer
. .

Returns >= 0 if OK (the number of bytes read), otherwise
PI_BAD_USER_GPIO, PI_NOT_I2C_GPIO, PI_BAD_POINTER,
PI_BAD_I2C_CMD, PI_BAD_I2C_RLEN, PI_BAD_I2C_WLEN,
PI_I2C_READ_FAILED, or PI_I2C_WRITE_FAILED.

The following command codes are supported:

Name    @ Cmd & Data   @ Meaning
End     @ 0            @ No more commands
Escape  @ 1            @ Next P is two bytes
Start   @ 2            @ Start condition
Stop    @ 3            @ Stop condition
Address @ 4 P          @ Set I2C address to P
Flags   @ 5 lsb msb    @ Set I2C flags to lsb + (msb << 8)
Read    @ 6 P          @ Read P bytes of data
Write   @ 7 P ...      @ Write P bytes of data

The address, read, and write commands take a parameter P.
Normally P is one byte (0-255).  If the command is preceded by
the Escape command then P is two bytes (0-65535, least significant
byte first).

The address and flags default to 0.  The address and flags maintain
their previous value until updated.

No flags are currently defined.

The returned I2C data is stored in consecutive locations of outBuf.

...
Set address 0x53
start, write 0x32, (re)start, read 6 bytes, stop
Set address 0x1E
start, write 0x03, (re)start, read 6 bytes, stop
Set address 0x68
start, write 0x1B, (re)start, read 8 bytes, stop
End

0x04 0x53
0x02 0x07 0x01 0x32   0x02 0x06 0x06 0x03

0x04 0x1E
0x02 0x07 0x01 0x03   0x02 0x06 0x06 0x03

0x04 0x68
0x02 0x07 0x01 0x1B   0x02 0x06 0x08 0x03

0x00
...
D*/

/*F*/
int spi_open(unsigned spi_channel, unsigned baud, unsigned spi_flags);
/*D
This function returns a handle for the SPI device on the channel.
Data will be transferred at baud bits per second.  The flags may
be used to modify the default behaviour of 4-wire operation, mode 0,
active low chip select.

The Pi has two SPI peripherals: main and auxiliary.

The main SPI has two chip selects (channels), the auxiliary has
three.

The auxiliary SPI is available on all models but the A and B.

The GPIO used are given in the following table.

         @ MISO @ MOSI @ SCLK @ CE0 @ CE1 @ CE2
Main SPI @    9 @   10 @   11 @   8 @   7 @   -
Aux SPI  @   19 @   20 @   21 @  18 @  17 @  16

. .
spi_channel: 0-1 (0-2 for the auxiliary SPI).
       baud: 32K-125M (values above 30M are unlikely to work).
  spi_flags: see below.
. .

Returns a handle (>=0) if OK, otherwise PI_BAD_SPI_CHANNEL,
PI_BAD_SPI_SPEED, PI_BAD_FLAGS, PI_NO_AUX_SPI, or PI_SPI_OPEN_FAILED.

spi_flags consists of the least significant 22 bits.

. .
21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 b  b  b  b  b  b  R  T  n  n  n  n  W  A u2 u1 u0 p2 p1 p0  m  m
. .

mm defines the SPI mode.

Warning: modes 1 and 3 do not appear to work on the auxiliary SPI.

. .
Mode POL PHA
 0    0   0
 1    0   1
 2    1   0
 3    1   1
. .

px is 0 if CEx is active low (default) and 1 for active high.

ux is 0 if the CEx GPIO is reserved for SPI (default) and 1 otherwise.

A is 0 for the main SPI, 1 for the auxiliary SPI.

W is 0 if the device is not 3-wire, 1 if the device is 3-wire.  Main
SPI only.

nnnn defines the number of bytes (0-15) to write before switching
the MOSI line to MISO to read data.  This field is ignored
if W is not set.  Main SPI only.

T is 1 if the least significant bit is transmitted on MOSI first, the
default (0) shifts the most significant bit out first.  Auxiliary SPI
only.

R is 1 if the least significant bit is received on MISO first, the
default (0) receives the most significant bit first.  Auxiliary SPI
only.

bbbbbb defines the word size in bits (0-32).  The default (0)
sets 8 bits per word.  Auxiliary SPI only.

The [*spi_read*], [*spi_write*], and [*spi_xfer*] functions
transfer data packed into 1, 2, or 4 bytes according to
the word size in bits.

For bits 1-8 there will be one byte per word. 
For bits 9-16 there will be two bytes per word. 
For bits 17-32 there will be four bytes per word.

Multi-byte transfers are made in least significant byte first order.

E.g. to transfer 32 11-bit words buf should contain 64 bytes
and count should be 64.

E.g. to transfer the 14 bit value 0x1ABC send the bytes 0xBC followed
by 0x1A.

The other bits in flags should be set to zero.
D*/

/*F*/
int spi_close(unsigned handle);
/*D
This functions closes the SPI device identified by the handle.

. .
handle: >=0, as returned by a call to [*spi_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.
D*/

/*F*/
int spi_read(unsigned handle, char *buf, unsigned count);
/*D
This function reads count bytes of data from the SPI
device associated with the handle.

. .
handle: >=0, as returned by a call to [*spi_open*].
   buf: an array to receive the read data bytes.
 count: the number of bytes to read.
. .

Returns the number of bytes transferred if OK, otherwise
PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or PI_SPI_XFER_FAILED.
D*/

/*F*/
int spi_write(unsigned handle, char *buf, unsigned count);
/*D
This function writes count bytes of data from buf to the SPI
device associated with the handle.

. .
handle: >=0, as returned by a call to [*spi_open*].
   buf: the data bytes to write.
 count: the number of bytes to write.
. .

Returns the number of bytes transferred if OK, otherwise
PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or PI_SPI_XFER_FAILED.
D*/

/*F*/
int spi_xfer(unsigned handle, char *txBuf, char *rxBuf, unsigned count);
/*D
This function transfers count bytes of data from txBuf to the SPI
device associated with the handle.  Simultaneously count bytes of
data are read from the device and placed in rxBuf.

. .
handle: >=0, as returned by a call to [*spi_open*].
 txBuf: the data bytes to write.
 rxBuf: the received data bytes.
 count: the number of bytes to transfer.
. .

Returns the number of bytes transferred if OK, otherwise
PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or PI_SPI_XFER_FAILED.
D*/

/*F*/
int serial_open(char *ser_tty, unsigned baud, unsigned ser_flags);
/*D
This function opens a serial device at a specified baud rate
with specified flags.  The device name must start with
/dev/tty or /dev/serial.

. .
  ser_tty: the serial device to open.
     baud: the baud rate in bits per second, see below.
ser_flags: 0.
. .

Returns a handle (>=0) if OK, otherwise PI_NO_HANDLE, or
PI_SER_OPEN_FAILED.

The baud rate must be one of 50, 75, 110, 134, 150,
200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200,
38400, 57600, 115200, or 230400.

No flags are currently defined.  This parameter should be set to zero.
D*/

/*F*/
int serial_close(unsigned handle);
/*D
This function closes the serial device associated with handle.

. .
handle: >=0, as returned by a call to [*serial_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.
D*/

/*F*/
int serial_write_byte(unsigned handle, unsigned bVal);
/*D
This function writes bVal to the serial port associated with handle.

. .
handle: >=0, as returned by a call to [*serial_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_SER_WRITE_FAILED.
D*/

/*F*/
int serial_read_byte(unsigned handle);
/*D
This function reads a byte from the serial port associated with handle.

. .
handle: >=0, as returned by a call to [*serial_open*].
. .

Returns the read byte (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_SER_READ_NO_DATA, or PI_SER_READ_FAILED.

If no data is ready PI_SER_READ_NO_DATA is returned.
D*/

/*F*/
int serial_write(unsigned handle, char *buf, unsigned count);
/*D
This function writes count bytes from buf to the the serial port
associated with handle.

. .
handle: >=0, as returned by a call to [*serial_open*].
   buf: the array of bytes to write.
 count: the number of bytes to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_SER_WRITE_FAILED.
D*/

/*F*/
int serial_read(unsigned handle, char *buf, unsigned count);
/*D
This function reads up to count bytes from the the serial port
associated with handle and writes them to buf.

. .
handle: >=0, as returned by a call to [*serial_open*].
   buf: an array to receive the read data.
 count: the maximum number of bytes to read.
. .

Returns the number of bytes read (>0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, PI_SER_READ_NO_DATA, or PI_SER_WRITE_FAILED.

If no data is ready zero is returned.
D*/

/*F*/
int serial_data_available(unsigned handle);
/*D
Returns the number of bytes available to be read from the
device associated with handle.

. .
handle: >=0, as returned by a call to [*serial_open*].
. .

Returns the number of bytes of data available (>=0) if OK,
otherwise PI_BAD_HANDLE.
D*/

/*F*/
int custom_1(unsigned arg1, unsigned arg2, char *argx, unsigned argc);
/*D
This function is available for user customisation.

It returns a single integer value.

. .
arg1: >=0
arg2: >=0
argx: extra (byte) arguments
argc: number of extra arguments
. .

Returns >= 0 if OK, less than 0 indicates a user defined error.
D*/


/*F*/
int custom_2(unsigned arg1, char *argx, unsigned argc,
             char *retBuf, unsigned retMax);
/*D
This function is available for user customisation.

It differs from custom_1 in that it returns an array of bytes
rather than just an integer.

The return value is an integer indicating the number of returned bytes.
. .
  arg1: >=0
  argc: extra (byte) arguments
 count: number of extra arguments
retBuf: buffer for returned data
retMax: maximum number of bytes to return
. .

Returns >= 0 if OK, less than 0 indicates a user defined error.

Note, the number of returned bytes will be retMax or less.
D*/


/*F*/
int callback(unsigned user_gpio, unsigned edge, CBFunc_t f);
/*D
This function initialises a new callback.

. .
user_gpio: 0-31.
     edge: RISING_EDGE, FALLING_EDGE, or EITHER_EDGE.
        f: the callback function.
. .

The function returns a callback id if OK, otherwise pigif_bad_malloc,
pigif_duplicate_callback, or pigif_bad_callback.

The callback is called with the GPIO, edge, and tick, whenever the
GPIO has the identified edge.

. .
Parameter   Value    Meaning

GPIO        0-31     The GPIO which has changed state

edge        0-2      0 = change to low (a falling edge)
                     1 = change to high (a rising edge)
                     2 = no level change (a watchdog timeout)

tick        32 bit   The number of microseconds since boot
                     WARNING: this wraps around from
                     4294967295 to 0 roughly every 72 minutes
. .
D*/

/*F*/
int callback_ex
   (unsigned user_gpio, unsigned edge, CBFuncEx_t f, void *userdata);
/*D
This function initialises a new callback.

. .
user_gpio: 0-31.
     edge: RISING_EDGE, FALLING_EDGE, or EITHER_EDGE.
        f: the callback function.
 userdata: a pointer to arbitrary user data.
. .

The function returns a callback id if OK, otherwise pigif_bad_malloc,
pigif_duplicate_callback, or pigif_bad_callback.

The callback is called with the GPIO, edge, tick, and user, whenever
the GPIO has the identified edge.

. .
Parameter   Value    Meaning

GPIO        0-31     The GPIO which has changed state

edge        0-2      0 = change to low (a falling edge)
                     1 = change to high (a rising edge)
                     2 = no level change (a watchdog timeout)

tick        32 bit   The number of microseconds since boot
                     WARNING: this wraps around from
                     4294967295 to 0 roughly every 72 minutes

userdata    pointer  Pointer to an arbitrary object
. .
D*/

/*F*/
int callback_cancel(unsigned callback_id);
/*D
This function cancels a callback identified by its id.

. .
callback_id: >=0, as returned by a call to [*callback*] or [*callback_ex*].
. .

The function returns 0 if OK, otherwise pigif_callback_not_found.
D*/

/*F*/
int wait_for_edge(unsigned user_gpio, unsigned edge, double timeout);
/*D
This function waits for edge on the GPIO for up to timeout
seconds.

. .
user_gpio: 0-31.
     edge: RISING_EDGE, FALLING_EDGE, or EITHER_EDGE.
  timeout: >=0.
. .

The function returns 1 if the edge occurred, otherwise 0.

The function returns when the edge occurs or after the timeout.
D*/

/*PARAMS

active :: 0-1000000

The number of microseconds level changes are reported for once
a noise filter has been triggered (by [*steady*] microseconds of
a stable level).

*addrStr::
A string specifying the host or IP address of the Pi running
the pigpio daemon.  It may be NULL in which case localhost
is used unless overridden by the PIGPIO_ADDR environment
variable.

arg1::
An unsigned argument passed to a user customised function.  Its
meaning is defined by the customiser.

arg2::
An unsigned argument passed to a user customised function.  Its
meaning is defined by the customiser.

argc::
The count of bytes passed to a user customised function.

*argx::
A pointer to an array of bytes passed to a user customised function.
Its meaning and content is defined by the customiser.

baud::
The speed of serial communication (I2C, SPI, serial link, waves) in
bits per second.

bit::
A value of 0 or 1.

bits::
A value used to select GPIO.  If bit n of bits is set then GPIO n is
selected.

A convenient way to set bit n is to or in (1<<n).

e.g. to select bits 5, 9, 23 you could use (1<<5) | (1<<9) | (1<<23).

*buf::
A buffer to hold data being sent or being received.

bufSize::
The size in bytes of a buffer.


bVal::0-255 (Hex 0x0-0xFF, Octal 0-0377)
An 8-bit byte value.

callback_id::
A >=0, as returned by a call to [*callback*] or [*callback_ex*].  This is
passed to [*callback_cancel*] to cancel the callback.

CBFunc_t::
. .
typedef void (*CBFunc_t)
   (unsigned user_gpio, unsigned level, uint32_t tick);
. .

CBFuncEx_t::
. .
typedef void (*CBFuncEx_t)
   (unsigned user_gpio, unsigned level, uint32_t tick, void * user);
. .

char::
A single character, an 8 bit quantity able to store 0-255.

clkfreq::4689-250000000 (250M)
The hardware clock frequency.

count::
The number of bytes to be transferred in an I2C, SPI, or Serial
command.

data_bits::1-32
The number of data bits in each character of serial data.

. .
#define PI_MIN_WAVE_DATABITS 1
#define PI_MAX_WAVE_DATABITS 32
. .

double::
A floating point number.

dutycycle::0-range
A number representing the ratio of on time to off time for PWM.

The number may vary between 0 and range (default 255) where
0 is off and range is fully on.

edge::
Used to identify a GPIO level transition of interest.  A rising edge is
a level change from 0 to 1.  A falling edge is a level change from 1 to 0.

. .
RISING_EDGE  0
FALLING_EDGE 1
EITHER_EDGE. 2
. .

errnum::
A negative number indicating a function call failed and the nature
of the error.

f::
A function.

frequency::>=0
The number of times a GPIO is swiched on and off per second.  This
can be set per GPIO and may be as little as 5Hz or as much as
40KHz.  The GPIO will be on for a proportion of the time as defined
by its dutycycle.


gpio::
A Broadcom numbered GPIO, in the range 0-53.

There  are 54 General Purpose Input Outputs (GPIO) named gpio0 through
gpio53.

They are split into two  banks.   Bank  1  consists  of  gpio0  through
gpio31.  Bank 2 consists of gpio32 through gpio53.

All the GPIO which are safe for the user to read and write are in
bank 1.  Not all GPIO in bank 1 are safe though.  Type 1 boards
have 17  safe GPIO.  Type 2 boards have 21.  Type 3 boards have 26.

See [*get_hardware_revision*].

The user GPIO are marked with an X in the following table.

. .
          0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
Type 1    X  X  -  -  X  -  -  X  X  X  X  X  -  -  X  X
Type 2    -  -  X  X  X  -  -  X  X  X  X  X  -  -  X  X
Type 3          X  X  X  X  X  X  X  X  X  X  X  X  X  X

         16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
Type 1    -  X  X  -  -  X  X  X  X  X  -  -  -  -  -  -
Type 2    -  X  X  -  -  -  X  X  X  X  -  X  X  X  X  X
Type 3    X  X  X  X  X  X  X  X  X  X  X  X  -  -  -  -
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

gpioThreadFunc_t::
. .
typedef void *(gpioThreadFunc_t) (void *);
. .

handle::>=0
A number referencing an object opened by one of [*i2c_open*], [*notify_open*],
[*serial_open*], and [*spi_open*].

i2c_addr:: 0-0x7F
The address of a device on the I2C bus.

i2c_bus::>=0
An I2C bus number.

i2c_flags::0
Flags which modify an I2C open command.  None are currently defined.

i2c_reg:: 0-255
A register of an I2C device.

*inBuf::
A buffer used to pass data to a function.

inLen::
The number of bytes of data in a buffer.

int::
A whole number, negative or positive.

invert::
A flag used to set normal or inverted bit bang serial data level logic.

level::
The level of a GPIO.  Low or High.

. .
PI_OFF 0
PI_ON 1

PI_CLEAR 0
PI_SET 1

PI_LOW 0
PI_HIGH 1
. .

There is one exception.  If a watchdog expires on a GPIO the level will be
reported as PI_TIMEOUT.  See [*set_watchdog*].

. .
PI_TIMEOUT 2
. .

mode::0-7
The operational mode of a GPIO, normally INPUT or OUTPUT.

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

numBytes::
The number of bytes used to store characters in a string.  Depending
on the number of bits per character there may be 1, 2, or 4 bytes
per character.

numPar:: 0-10
The number of parameters passed to a script.

numPulses::
The number of pulses to be added to a waveform.

offset::
The associated data starts this number of microseconds from the start of
the waveform.

*outBuf::
A buffer used to return data from a function.

outLen::
The size in bytes of an output buffer.

*param::
An array of script parameters.

*portStr::
A string specifying the port address used by the Pi running
the pigpio daemon.  It may be NULL in which case "8888"
is used unless overridden by the PIGPIO_PORT environment
variable.

*pth::
A thread identifier, returned by [*start_thread*].


pthread_t::
A thread identifier.

pud::0-2
The setting of the pull up/down resistor for a GPIO, which may be off,
pull-up, or pull-down.
. .
PI_PUD_OFF 0
PI_PUD_DOWN 1
PI_PUD_UP 2
. .

pulseLen::
1-100, the length of a trigger pulse in microseconds.

*pulses::
An array of pulses to be added to a waveform.

pulsewidth::0, 500-2500
. .
PI_SERVO_OFF 0
PI_MIN_SERVO_PULSEWIDTH 500
PI_MAX_SERVO_PULSEWIDTH 2500
. .

PWMduty::0-1000000 (1M)
The hardware PWM dutycycle.

. .
#define PI_HW_PWM_RANGE 1000000
. .

PWMfreq::1-125000000 (125M)
The hardware PWM frequency.

. .
#define PI_HW_PWM_MIN_FREQ 1
#define PI_HW_PWM_MAX_FREQ 125000000
. .

range::25-40000
The permissible dutycycle values are 0-range.
. .
PI_MIN_DUTYCYCLE_RANGE 25
PI_MAX_DUTYCYCLE_RANGE 40000
. .

*retBuf::
A buffer to hold a number of bytes returned to a used customised function,

retMax::
The maximum number of bytes a user customised function should return.


*rxBuf::
A pointer to a buffer to receive data.

SCL::
The user GPIO to use for the clock when bit banging I2C.

*script::
A pointer to the text of a script.

script_id::
An id of a stored script as returned by [*store_script*].

SDA::
The user GPIO to use for data when bit banging I2C.

seconds::
The number of seconds.

ser_flags::
Flags which modify a serial open command.  None are currently defined.

*ser_tty::
The name of a serial tty device, e.g. /dev/ttyAMA0, /dev/ttyUSB0, /dev/tty1.

size_t::
A standard type used to indicate the size of an object in bytes.

spi_channel::
A SPI channel, 0-2.

spi_flags::
See [*spi_open*].

steady :: 0-300000

The number of microseconds level changes must be stable for
before reporting the level changed ([*set_glitch_filter*]) or triggering
the active part of a noise filter ([*set_noise_filter*]).

stop_bits::2-8
The number of (half) stop bits to be used when adding serial data
to a waveform.

. .
#define PI_MIN_WAVE_HALFSTOPBITS 2
#define PI_MAX_WAVE_HALFSTOPBITS 8
. .

*str::
 An array of characters.

thread_func::
A function of type gpioThreadFunc_t used as the main function of a
thread.

timeout::
A GPIO watchdog timeout in milliseconds.
. .
PI_MIN_WDOG_TIMEOUT 0
PI_MAX_WDOG_TIMEOUT 60000
. .

*txBuf::
An array of bytes to transmit.

uint32_t::0-0-4,294,967,295 (Hex 0x0-0xFFFFFFFF)
A 32-bit unsigned value.

unsigned::
A whole number >= 0.

user_gpio::
0-31, a Broadcom numbered GPIO.

See [*gpio*].

*userdata::
A pointer to arbitrary user data.  This may be used to identify the instance.

void::
Denoting no parameter is required

wave_add_*::
One of [*wave_add_new*], [*wave_add_generic*], [*wave_add_serial*].

wave_id::
A number representing a waveform created by [*wave_create*].

wave_send_*::
One of [*wave_send_once*], [*wave_send_repeat*].

wVal::0-65535 (Hex 0x0-0xFFFF, Octal 0-0177777)
A 16-bit word value.

PARAMS*/

/*DEF_S pigpiod_if Error Codes*/

typedef enum
{
   pigif_bad_send           = -2000,
   pigif_bad_recv           = -2001,
   pigif_bad_getaddrinfo    = -2002,
   pigif_bad_connect        = -2003,
   pigif_bad_socket         = -2004,
   pigif_bad_noib           = -2005,
   pigif_duplicate_callback = -2006,
   pigif_bad_malloc         = -2007,
   pigif_bad_callback       = -2008,
   pigif_notify_failed      = -2009,
   pigif_callback_not_found = -2010,
} pigifError_t;

/*DEF_E*/

#ifdef __cplusplus
}
#endif

#endif

