
# Start radio gateway

# Note: lighthouseapp.js is installed in the dweet lib directory with npm

# This uses a per-project customized handler
dweet -console -script=ProMiniStartRadioGateway.dweet -apphandler=$PWD/../lighthouseapp.js $*

# This references a handler installed with dweet using npm -g
#dweet -console -script=ProMiniStartRadioGateway.dweet -apphandler=./lighthouseapp.js $*
