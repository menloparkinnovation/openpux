
export PATH=$PATH:/opt/node/bin

DEPLOY=/home/pi/Smartpux/openpux

cd $DEPLOY/deploy/raspberrypi/dweet

# Start Radio Gateway on a per-project customized handler
dweet -console -script=PiProMiniStartRadioGateway.dweet -config=lighthouse.json -apphandler=./lighthouseapp.js $*
