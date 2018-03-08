#
# Arduino Builder Testing
#
# build from Arduino path.
#

# On mac
MY_ARDUINO_INSTALLATION=/Applications/Arduino_1_6_8.app

MY_BOARD_NAME=arduino:avr:uno

MY_SKETCH_PATH=$PWD

MY_LIBRARIES_PATH=$PWD/Libraries

MY_ARDUINO_BUILDER_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/

MY_HARDWARE_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/hardware

MY_TOOLS_PATH=$MY_ARDUINO_INSTALLATION/Contents/Java/hardware/tools/avr

MY_TOOLS_PATH2=$MY_ARDUINO_INSTALLATION/Contents/Java/tools-builder

Module=MenloFramework
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloMinimalTemplate
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloBlink
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloBlinkMemoryMonitor
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloDweetBlink
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloRadioDweetBlink
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloRadioDweetSensor
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloGateway
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloWatchDog
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=MenloTestTemplate
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=FencePostLight
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=LightHouse
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

Module=WeatherStation
$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

# This does not build on UNO, but does on MEGA or Due since its larger than the UNO can fit.
#Module=RadioWeatherStation
#$MY_ARDUINO_BUILDER_PATH/arduino-builder -verbose -hardware $MY_HARDWARE_PATH -tools $MY_TOOLS_PATH -tools $MY_TOOLS_PATH2 -libraries $MY_LIBRARIES_PATH -fqbn $MY_BOARD_NAME $MY_SKETCH_PATH/$Module
#rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo "**** ALL PASSED ****"
