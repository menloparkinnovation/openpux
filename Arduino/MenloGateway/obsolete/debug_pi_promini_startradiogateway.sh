
# Start radio gateway

# Note: lighthouseapp.js is installed in the dweet lib directory with npm

# This uses a per-project customized handler
dweet -verbose -traceerror -tracelog -console -script=PiProMiniStartRadioGateway.dweet -apphandler=$PWD/../lighthouseapp.js $*

# This references a handler installed with dweet using npm -g
#dweet -console -script=PiProMiniStartRadioGateway.dweet -apphandler=./lighthouseapp.js $*
