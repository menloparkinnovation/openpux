#
# Must install forever with:
#
# sudo npm install forever -g
#
# check status with:
#
# forever list
#
# stop with:
#
# forever stopall
# forever stop <pid>
#
# other commands:
#
# forever logs
# forever --help
#
DEPLOY=$HOME/Dropbox/code/macgitrepos/Smartpux/openpux

cd $DEPLOY

forever start openpux.js config/awssimpledb_config.json
