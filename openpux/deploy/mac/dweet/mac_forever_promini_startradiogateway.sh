
# Start Radio Gateway on a per-project customized handler
forever start -c sh dweet.sh -console -script=MacProMiniStartRadioGateway.dweet -config=lighthouse.json -apphandler=./lighthouseapp.js $*
