
#
# script RadioSerial gateway tests against testdevice
#

# -script=name.dweet => script name to run
# -console => open console when script is done
# -dontstoponerror => keep executing script commands even on error
node main.js -verbose -console -script=radio0testdevice.dweet $*

# test without stopOnError, eval error in script
#node main.js -console -dontstoponerror -script=radio0testdevice.dweet $*
