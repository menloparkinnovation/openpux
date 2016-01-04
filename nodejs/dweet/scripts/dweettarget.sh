
# Dweet Radio Target
# Arduino Uno with board #2
# /dev/tty.usbmodem1411 for Arduino Uno on MacBookAir right side USB port
# Run ./configureradiotarget.sh first after new firmware upload

dweet /dev/tty.usbmodem1411 $*

#
# getconfig name
#  - shows device name, if no name its not configured
#
