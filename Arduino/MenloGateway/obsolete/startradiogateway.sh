
# Configure RadioGateway device
# /dev/tty.usbmodem1421 for Arduino Uno on MacBookAir

# Use the following command:
# open /dev/tty.usbmodem1421 gateway

# Note: lighthouseapp.js is installed in the dweet lib directory with npm
dweet -console -script=StartRadioGateway.dweet -apphandler=./lighthouseapp.js $*
