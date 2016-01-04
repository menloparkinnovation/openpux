
// Cable with marker is rain sensor.

09/03/2015

WeatherStation which uses SparkFun WeatherShield.

It outputs the weather data as NMEA 0183 on the standard serial port.

This is intended to hook its serial connection to another device
to receive streaming weather station data over serial using standard
NMEA 0183 weather instrument verbs.

The other device would forward the received weather data to a radio, etc.
A demonstration of this technique is in the project "LightHouse" which provides
a radio connection to a gateway along with controlling lighthouse functions.

The WeatherStation device also responds to the Menlo Framework Dweet
protocol over the serial port, which is compatible with NMEA 0183.

To build:

Launch Arduino 1.6.4, select board:

  Arduino Uno 

  Open WeatherStation.ino
  
  Select "Build" (check mark upper left corner)

To Program:

  Connect your device, and select the serial port in Arduino/Tools/Port

  Click program (right arrow in upper left corner)

To Configure:

As a fully functioning device configuration must be stored in
the devices EEPROM.

The node.js utility "dweet" is used to upload this configuration
over the serial port from a PC/Mac/Linux/RaspberryPi. This uses the
NMEA 0183 compatible Dweet protocol.

This utility is in Smartpux/nodejs/dweet.
  - It is installed with npm install -g Smartpux/nodejs/dweet

Configuration is done from .dweet scripts which contain
the series of configuration commands.

scripts/configureweatherstation.sh

To Interact/Debug WeatherStation:

// Launch the Dweet console
scripts/dweetweatherstation.sh

//
// Calibration data in 0labbook.txt
//

