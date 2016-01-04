
11/25/2015

Version 2 of FencePostLight using update MenloFramework.

This version is based on Dweet, and its hardware/software separation classes.

The FencePostLight acts as a programmable day/night light which can report on
light level, temperature, and its battery state.

It can be programmed to automatically turn on and off based on night or day
and has a programmable three color LED. Any color can be programmed as well
as flashing or periodic display modes.

Building:

Currently Arduino 1.0.6 

Board - MenloSensor 8Mhz/3.3V (No BootLoader)

Serial port - As required based on FTDI USB -> serial adapter

11/25/2015 - Arduino 1.6.4 builds, needs to be tested.

Note:

FencePostLight leverages the LightHouse library and classes. This is to
provide the programmable light sequence and day/night support.

Even though the program is similar to the LightHouse application, it
removes support for NMEA 0183 streaming weather sensors, which saves
about 1.5K of code space enabling this program to fit into an AtMega328
using standard boot loaders. Currently, LightHouse is ICSP programmed
as it just fits into the AtMega328 without a boot loader.

This programs size is currently 31,572 bytes out of 32,768 maximum
using Arduino 1.0.6. This is 1196 bytes free.

With Arduino 1.6.4 program size is 30,850 bytes out of 32,256 with
Arduino Uno and a 0.5K bootloader. This is 1406 bytes free.

Arduino 1.6.4 set for Arduino Pro Mini 8Mhz/3.3V is 30,838 bytes
out of 30,720 available with its 2K bootloader.

