
#
# Build Arduino command line on mac (Mega2560)
#
# 12/30/2015
#

# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

#
# *** NOTE ***
#
# Must set the board to "Mega 2560" in the menu, then close/exit the Arduino
# IDE. This sets the environment settings which the command line build
# appears to rely on.
#
# If confusing/random errors occur, its because of a confusion between
# the settings from the command line and the saved environment.
#
# *** NOTE ***
#

#
# Build on Mega 2560, validating AVR/Mega chips
#

#
# RadioWeatherStation requires AtMega due to code size. (39k code, 2280 bytes ram)
#
/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:mega:cpu=atmega2560 $PWD/RadioWeatherStation/RadioWeatherStation.ino
