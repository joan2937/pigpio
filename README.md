# pigpio
pigpio is a C library for the Raspberry which allows control of the
General Purpose Input Outputs (GPIO).

Features

    sampling and time-stamping of GPIO 0-31 between 100,000 and 1,000,000 times per second.

    provision of PWM on any number of the user GPIO simultaneously.

    provision of servo pulses on any number of the user GPIO simultaneously.

    callbacks when any of GPIO 0-31 change state (callbacks receive the time of the event
    accurate to a few microseconds).

    notifications via pipe when any of GPIO 0-31 change state.

    callbacks at timed intervals.

    reading/writing all of the GPIO in a bank (0-31, 32-53) as a single operation.

    individually setting GPIO modes, reading and writing.

    socket and pipe interfaces for the bulk of the functionality in addition to the
    underlying C library calls.

    the construction of arbitrary waveforms to give precise timing of output GPIO
    level changes (accurate to a few microseconds).

    software serial links, I2C, and SPI using any user GPIO.

    rudimentary permission control through the socket and pipe interfaces so users
    can be prevented from "updating" inappropriate GPIO.

    creating and running scripts on the pigpio daemon.

Interfaces

The library provides a number of control interfaces

    the C function interface

    the /dev/pigpio pipe interface

    the socket interface (used by the pigs utility and the Python module)

Utilities

A number of utility programs are provided

    the pigpiod daemon.
    the Python module.
    the piscope digital waveform viewer.
    the pigs command line utility.
    the pig2vcd utility which converts notifications into the value change dump (VCD)
    format (useful for viewing digital waveforms with GTKWave).

Example programs

See http://abyz.co.uk/rpi/pigpio/examples.html

GPIO

ALL GPIO are identified by their Broadcom number.  See elinux.org

There are 54 GPIO in total, arranged in two banks.

Bank 1 contains GPIO 0-31.  Bank 2 contains GPIO 32-54.

A user should only manipulate GPIO in bank 1.

There are at least three types of board.

Type 1

    26 pin header (P1).

    Hardware revision numbers of 2 and 3.

    User gpios 0-1, 4, 7-11, 14-15, 17-18, 21-25.

Type 2

    26 pin header (P1) and an additional 8 pin header (P5).

    Hardware revision numbers of 4, 5, 6, and 15.

    User gpios 2-4, 7-11, 14-15, 17-18, 22-25, 27-31.

Type 3

    40 pin expansion header (J8).

    Hardware revision numbers of 16 or greater.

    User gpios 2-27 (0 and 1 are reserved).

It is safe to read all the GPIO. If you try to write a system GPIO or change
its mode you can crash the Pi or corrupt the data on the SD card.
