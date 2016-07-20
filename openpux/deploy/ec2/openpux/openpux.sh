
DEPLOY=/home/ec2-user/Smartpux/openpux
cd $DEPLOY

node app.js $DEPLOY/config/ec2_awssimpledb_config.json
