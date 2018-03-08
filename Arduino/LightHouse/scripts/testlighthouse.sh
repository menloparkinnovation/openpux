
# Configure LightHouse as a radio target device
# /dev/tty.usbmodem1411
# For Arduino Uno on MacBookAir Right side USB port
dweet -console -script=TestLightHouse.dweet /dev/cu.usbmodem1421 $*
