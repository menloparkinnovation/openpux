
DEPLOY=/home/ec2-user/Smartpux/openpux
cd $DEPLOY

node openpux.js $DEPLOY/deploy/ec2/openpux/ec2_awssimpledb_config.json
