# Python Class for Reading Single Edge Nibble Transmission (SENT) using the Raspberry Pi

A full description of this Python script is described at [www.surfncircuits.com](https://surfncircuits.com) in the blog entry: [Implementing a Single Edge Nibble Transmission (SENT) protocol in Python for the Raspberry Pi Zero](https://surfncircuits.com/?p=3725)


This python library will read a Raspberry Pi GPIO pin connected. Start the pigpiod daemon with one microsecond sampling to read SENT transmissions with three microsecond tick times.   

## To start the daemon on Raspberry Pi
- sudo pigpiod -s 1

## SENT packet frame summary

- Sync Pulse: 56 ticks
- 4 bit Status and Message Pulse: 17-32 ticks
- 4 bit (9:12) Data1 Field: 17-32 ticks
- 4 bit (5:8) Data1 Field: 17-32 ticks
- 4 bit (1:4) Data1 Field: 17-32 ticks
- 4 bit (9-12) Data2 Field: 17-32 ticks
- 4 bit (5-8) Data2 Field: 17-32 ticks
- 4 bit (1-4) Data2 Field: 17-32 ticks
- 4 bit CRC: 17-32 ticks


## requirements
[pigpiod](http://abyz.me.uk/rpi/pigpio/)  library required

## To run the script for a signal attached to GPIO BCD 18 (pin 12)
- python3 sent_READ.py
