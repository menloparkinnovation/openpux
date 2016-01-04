
11/24/2015

Arduino 1.0.6

With NMEA 0183 weather instrument streaming support (approx 1.5K)
the program just fits into the AtMega328 without a boot loader. There
is not enough space of even the 0.5K boot loader, so it is ICSP
programmed into the board.

The program is currently 32,560 bytes out of 32,768 available without a
boot loader. This is 208 bytes left, a minimal bootloader is 512 bytes.

The MenloSensor 8Mhz/3.3V (no bootloader) selection is chosen which
provides the proper fuse settings for no boot loader and start address
within the main program area.

Arduino 1.6.4

The program set for Arduino Uno with 0.5K bootloader is 31,832 bytes
out of 32,256. This is 424 bytes free and represents Arduino 1.6.4
more efficient compiler/libraries.

MenloSensor 8Mhz/3.3 (no bootloader)

31,820 bytes used out of 32,768 available.

Free 948 bytes.

Others:

  - Get the 0.5K boot loader built and working for 8Mhz/3.3V and then
    Arduino Pro Mini 8Mhz/3.3V target can be used. Currently selecting
    this board results in a 2K bootloader being configured., leaving only
    30,720 bytes total available.

    - Not sure if the new 0.5K "optiboot" requires the new serial/USB
      interface vrs FTDI. It could be the firmware on the new serial
      interface (using another smaller AVR chip) makes it easy on
      the Arduino boot loader by coming up pre-configured.

  - SparkFun Pro Micro 8Mhz/3.3V is even worse since it has on board
    USB which consumes 4K of flash space leaving only 28,672 k for the
    application. Even worse it includes in your application larger libraries
    for talking to serial, which results in the program requiring
    34,740 bytes.

09/03/2015

This is a lighthouse that receives NMEA 0183 weather station data.

Its remote controlled over the Nordic nRF24L01+ radio.

It communicates with a Dweet gateway application to log weather readings.

It uses a Menlo Smartpux 8Mhz AtMega328 "LightHouse" sensor module
with three color LED, light sensor, and battery/solar monitor.

The NMEA 0183 data can come from anything, but in the example
configuration its from a SparkFun Red Board Arduino AtMega328
with the SparkFun Arduino WeatherShield. The software for this
configuration is in the project "WeatherStation".

--------

  Smartpux/Arduino/LightHouse/:

    - This provides a radio controlled lighthouse over a Nordic nRF24L01+
      radio. It controls the light sequence, and also sends light intensity
      data, temperature data, solar panel output voltage, battery status,
      and optional moisture level indications.

      The moisture level indication can be used as part of a sea level
      sensor in a real lighthouse, or as a soil moisture monitor when
      its sitting in your garden.

      The lighthouse light is a three color LED and has a configurable
      flashing interval, color, as well as automatic day/night operation.

    - It sends its environmental readings over the radio to a gateway on
      a perodic basis, and also receive configuration/management updates
      over the radio as well.

    - It also supports the streaming input of standard NMEA 0183 weather
      instrument sentences on the serial port and will integrate these
      readings with the lighthouses periodic radio report.

  Smartpux/Arduino/WeatherStation/:

    - SparkFun WeatherShield and Arduino Uno/SparkFun Redboard which
      provides a weather station with wind speed, wind direction,
      temperature, humidity, barometric pressure, and optional GPS readings.

    - This project outputs the weather data as standard NMEA 0183 weather
      instrument sentences and can be used as an input to the weatherstation
      function of the LightHouse application.

    - If mounted on a Buoy an accurate GPS can allow the weatherstation
      to calculate tide heights using the GPS altitude function.

      In addition, if the GPS update rate is fast enough, it can also
      calculate sea (wave) height and period and send this data as well.

    - The SparkFun GPS module is capable of 1/2 second updates and
      depending on GPS selective availability should have an altitude
      accurracy for determining at least approximate tide heights.

  Smartpux/Arduino/MenloGateway:

    - The Gateway provides communication to the light house over the Nordic
      nRF24L01+ radio. It communicates with a PC/Mac/RaspberryPi over standard
      Arduino USB serial connection. The gateway uses the Menlo Dweet NMEA 0183
      compatible protocol for control and data messages.

    - The gateway hardware can be an Arduino Uno with a Menlo nRF24L01+
      adapter shield, or manual wiring to the radio module with jumpers wires
      or other third party shield.

      The following script may need to be updated based on your local
      computers configuration.

      scripts/startradiogateway.sh

    - A small form factor gateway is constructed from an Arduino Pro Mini
      8Mhz/3.3v and a mini Menlo nRF24L01+ shield. This allows the gateway to
      be as small as a large USB "stick" or integrated into a short PVC pipe
      enclosure as a remote "antenna" fed by a USB cable of reasonable length.

      This allows placement of the antenna high enough to get good coverage.

      Since the Arduino Pro Mini on a SparkFun FTDI breakout has a different
      port it has its own script. This may need to be updated based on your
      local computers configuration.

      scripts/mini_startradiogateway.sh

    - The design target for the gateway computer is the RaspberryPi, but works
      fine on PC/Mac/Linux as well. The Dweet gateway software is a node.js
      application on all platforms just requiring the npm package "serialport".

----------------

To build:

Launch Arduino 1.0.6

  (Arduino 1.6.4 support is being debugged)

  Open LightHouse.ino
  
  Select board MenloSensor 3.3V/8MHz

    - This project requires no boot loader since its 32,560 bytes in size

    - Install MenloSensor configuration if not present.

  Select "Build" (check mark upper left corner)

    - If it overflows, check for debug code enabled in the libraries.

    - See Arduino/roadmap.txt, KEYWORDS: Memory overflow problems

To Program:

  Must use ICSP such as an Olimex due to no boot loader to save the 2K of space.

  File, Upload using Progammers...

To Configure:

As a fully functioning device configuration must be stored in
the devices EEPROM.

The node.js utility "dweet" is used to upload this configuration
over the serial port from a PC. This uses the lower level
NMEA 0183 compatible Dweet protocol.

This utility is in Smartpux/nodejs/dweet.

Configuration is done from .dweet scripts which contain
the series of configuration commands.

scripts/configureweatherstation.sh

To Interact/Debug WeatherStation:

// Launch the Dweet console
scripts/dweetweatherstation.sh

