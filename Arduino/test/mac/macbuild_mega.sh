
#
# Arduino Builder Testing
#
# build from Arduino path.
#

# On mac
MY_ARDUINO_INSTALLATION=/Applications/Arduino_1_6_8.app

#
# Note: If build fails with "missing -mmcu=" error the following line is missing
# from the "mega" entry in
# /Applications/Arduino_1_6_8.app/Contents/Java/hardware/arduino/avr/boards.txt
#
# MenloParkInnovation LLC 03/08/2018 This was missing preventing mega builds with arduino-builder
# It was in the menu. Somehow this line got deleted.
#mega.build.mcu=atmega2560
#


#
# Build on Mega 2560, validating AVR/Mega chips
#

MY_BOARD_NAME=arduino:avr:mega

MY_SKETCH_PATH=$PWD

MY_LIBRARIES_PATH=$PWD/Libraries

MY_ARDUINO_BUILDER_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/

MY_HARDWARE_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/hardware

MY_TOOLS_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/hardware/tools/avr

MY_TOOLS_PATH2=$MY_ARDUINO_INSTALLATION/Contents/Java/tools-builder

#
# RadioWeatherStation requires Arduino MEGA due to code size. (39k code, 2280 bytes ram)
#
Module=RadioWeatherStation
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo "**** ALL PASSED ****"
