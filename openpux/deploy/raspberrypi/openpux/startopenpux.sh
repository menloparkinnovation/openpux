
export PATH=$PATH:/opt/node/bin

DEPLOY=/home/pi/Smartpux/openpux

cd $DEPLOY/deploy/raspberrypi/openpux

/opt/node/bin/forever start --spinSleepTime 60000 -a --uid "openpux" -c sh openpux.sh
