
# Dweet Radio Gateway
# Arduino Uno with board #1
# /dev/tty.usbmodem1421 for Arduino Uno on MacBookAir
# Run ./configureradiogateway.sh first after new firmware upload

dweet /dev/tty.usbmodem1421 $*

#
# getconfig name
#  - shows device name, if no name its not configured
#
# dweet SETSTATE=RADIOGATEWAY:ON
#  - Turns on radio gateway
#
# radiogateway radio0 /dev/tty.usbmodem1421
#  - Connect dweet console to radio0 over active device as gateway
#
# getconfig name
#  - This should be sent to radio0 as a Dweet over RadioSerial
#
