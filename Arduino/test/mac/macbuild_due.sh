
#
# Build Arduino command line on mac (Due)
#
# 12/30/2015
#

# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

#
# *** NOTE ***
#
# Must set the board to "Due" in the menu, then close/exit the Arduino
# IDE. This sets the environment settings which the command line build
# appears to rely on.
#
# If confusing/random errors occur, its because of a confusion between
# the settings from the command line and the saved environment.
#
# *** NOTE ***
#

#
# Build on Due, validating ARM32/SAM
#

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --board arduino:sam:arduino_due_x --verify --verbose $PWD/MenloGateway/MenloGateway.ino

/Applications/Arduino_1.6.4.app/Contents/MacOS/Arduino --board arduino:sam:arduino_due_x --verify --verbose $PWD/LightHouse/LightHouse.ino
