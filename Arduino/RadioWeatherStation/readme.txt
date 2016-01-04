
09/03/2015

This attempts to integrate both the Nordic nRF24L01+ radio
and the WeatherShield support in one project.

It currently overflows an Arduino Uno.

It builds for:

 Arduino Mega, Mega 2560

It currently fails to build for the Due since Software Serial is missing,
which is used for the GPS connection. Need investigate/port if Due has
more than one serial port available.

To build:

Launch Arduino 1.6.4, select board:

  Arduino Mega, Mega 2560

  Open RadioWeatherStation.ino
  
  Select "Build" (check mark upper left)


