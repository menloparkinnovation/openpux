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
forever start openpux.js config/awssimpledb_config.json
