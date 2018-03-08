
# Dweet MenloSensor
# /dev/tty.usbserial-A10132OT
# On MacBookAir Right side USB port
# Run ./configuremenlosensor.sh first after new firmware upload

node main.js /dev/tty.usbserial-A10132OT $*

#
# getconfig name
#  - shows device name, if no name its not configured
#
