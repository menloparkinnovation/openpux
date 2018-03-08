

#
# Build on Due, validating ARM32/SAM
#
# build from Arduino path.
#
# Note: Changes to Arduino 1.6.8 breaks Due, fixing this is a work in progress.
#

# On mac
MY_ARDUINO_INSTALLATION=/Applications/Arduino_1_6_8.app

#
# Build on Due, validating ARM.
#

MY_BOARD_NAME=arduino:avr:due

MY_SKETCH_PATH=$PWD

MY_LIBRARIES_PATH=$PWD/Libraries

MY_ARDUINO_BUILDER_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/

MY_HARDWARE_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/hardware

MY_TOOLS_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/hardware/tools/avr

MY_TOOLS_PATH2=$MY_ARDUINO_INSTALLATION/Contents/Java/tools-builder

# This does not build on UNO, but does on MEGA or Due since its larger than the UNO can fit.
#Module=RadioWeatherStation
#$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
#rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo "**** ALL PASSED ****"
