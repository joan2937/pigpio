# pigpio
pigpio is a C library for the Raspberry which allows control of the
general purpose input outputs (gpios).

Features

    sampling and time-stamping of gpios 0-31 between 100,000 and 1,000,000 times per second.

    provision of PWM on any number of the user gpios simultaneously.

    provision of servo pulses on any number of the user gpios simultaneously.

    callbacks when any of gpios 0-31 change state (callbacks receive the time of the event
    accurate to a few microseconds).

    notifications via pipe when any of gpios 0-31 change state.

    callbacks at timed intervals.

    reading/writing all of the gpios in a bank (0-31, 32-53) as a single operation.

    individually setting gpio modes, reading and writing.

    socket and pipe interfaces for the bulk of the functionality in addition to the
    underlying C library calls.

    the construction of arbitrary waveforms to give precise timing of output gpio
    level changes (accurate to a few microseconds).

    software serial links using any user gpio.

    rudimentary permission control through the socket and pipe interfaces so users
    can be prevented from "updating" inappropriate gpios.

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

    the pigs command line utility.

    the pig2vcd utility which converts notifications into the value change dump (VCD)
    format (useful for viewing digital waveforms with GTKWave).

gpios

ALL gpios are identified by their Broadcom number.  See elinux.org

There are 54 gpios in total, arranged in two banks.

Bank 1 contains gpios 0-31.  Bank 2 contains gpios 32-54.

A user should only manipulate gpios in bank 1.

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

It is safe to read all the gpios. If you try to write a system gpio or change
its mode you can crash the Pi or corrupt the data on the SD card.
