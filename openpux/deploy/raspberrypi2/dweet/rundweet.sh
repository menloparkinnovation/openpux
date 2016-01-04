
DEPLOY=/home/pi/Smartpux/openpux

cd $DEPLOY/deploy/raspberrypi2/dweet

# Start Radio Gateway on a per-project customized handler
dweet -console -script=Pi2ProMiniStartRadioGateway.dweet -config=lighthouse.json -apphandler=./lighthouseapp.js $*
