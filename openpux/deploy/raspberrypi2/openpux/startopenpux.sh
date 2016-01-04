
DEPLOY=/home/pi/Smartpux/openpux

cd $DEPLOY/deploy/raspberrypi2/openpux

forever start --spinSleepTime 60000 -a --uid "openpux" -c sh openpux.sh
