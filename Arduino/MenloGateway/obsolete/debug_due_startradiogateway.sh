
# Configure RadioGateway device
# /dev/tty.usbmodem1421 for Arduino Due

dweet -verbose -traceerror -tracelog -console -script=DueStartRadioGateway.dweet /dev/tty.usbmodem1421

#dweet -verbose -console -script=DueStartRadioGateway.dweet /dev/tty.usbmodem1421
#dweet -console -script=DueStartRadioGateway.dweet $*
