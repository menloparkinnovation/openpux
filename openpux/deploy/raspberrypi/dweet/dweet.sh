PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/local/games:/usr/games:/opt/node/bin

DEPLOY=/home/pi/Smartpux/openpux/deploy/raspberrypi/dweet
cd $DEPLOY

# dweet is installed globally
dweet $*
