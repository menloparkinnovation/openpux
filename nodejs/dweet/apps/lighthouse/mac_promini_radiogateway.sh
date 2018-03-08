
#
# Start Radio Gateway to handle the Menlo LightHouse WeatherStation project
#
# lighthouse.json loads packetradio.js, lighthouseapp.js
#

bin/dweet -console -script=apps/lighthouse/mac_promini_radiogateway.dweet -config=apps/lighthouse/lighthouse.json -apphandler=./lighthouseapp.js $*
