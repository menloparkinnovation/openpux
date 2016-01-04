
#
# Build Arduino command line on mac
#
# 12/30/2015
#

# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

#
# *** NOTE ***
#
# Must set the board to "Uno" in the menu, then close/exit the Arduino
# IDE. This sets the environment settings which the command line build
# appears to rely on.
#
# If confusing/random errors occur, its because of a confusion between
# the settings from the command line and the saved environment.
#
# *** NOTE ***
#

#
# Build on Uno, validating AVR/AtMega328
#

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:uno $PWD/MenloGateway/MenloGateway.ino

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:uno $PWD/WeatherStation/WeatherStation.ino

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:uno $PWD/LightHouse/LightHouse.ino

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:uno $PWD/FencePostLight/FencePostLight.ino

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:uno $PWD/MenloBlink/MenloBlink.ino

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:uno $PWD/MenloRadioDweetBlink/MenloRadioDweetBlink.ino

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --verify --verbose --board arduino:avr:uno $PWD/MenloDweetBlink/MenloDweetBlink.ino
