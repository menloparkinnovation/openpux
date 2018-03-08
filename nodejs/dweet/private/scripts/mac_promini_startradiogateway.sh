
#
# Start Radio Gateway to handle the Menlo LightHouse WeatherStation project
#
# lighthouse.json loads packetradio.js, lighthouseapp.js
#

dweet -console -script=scripts/MacProMiniStartRadioGateway.dweet -config=scripts/lighthouse.json -apphandler=./lighthouseapp.js $*
