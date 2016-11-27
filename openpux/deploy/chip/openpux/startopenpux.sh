
DEPLOY=/home/chip/openpux/openpux

cd $DEPLOY/deploy/chip/openpux

forever start --spinSleepTime 60000 -a --uid "openpux" -c sh openpux.sh
