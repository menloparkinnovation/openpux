
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/local/games:/usr/games:/opt/node/bin

DEPLOY=/home/pi/Smartpux/openpux
cd $DEPLOY

node openpux.js $DEPLOY/deploy/raspberrypi/openpux/pi_awssimpledb_config.json
