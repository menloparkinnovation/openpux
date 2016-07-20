
#
# First must provision on the empty in memory database
#
#util/provision.sh

# create user account 1
# Note: This specifies an explicit ticketId for the account as "12345678"
node apps/openpux/client/javascripts/client.js createaccount localhost 8080 jsonfile=util/createaccount.json

# create a ticket for user account 1
#node apps/openpux/client/javascripts/client.js createtoken localhost 8080 jsonfile=util/createaccount1token.json

echo "*** account provisioning done"

node apps/openpux/client/javascripts/client.js createsensor localhost 8080 jsonfile=util/createsensor.json

echo "*** sensor provisioning done"
echo "*** sensor provisioning done"
echo "*** sensor provisioning done"

# Now perform the normal test loop on a configured database
echo "updatesensorsettings.sh"
util/updatesensorsettings.sh

echo "querysensorsettings.sh"
util/querysensorsettings.sh

echo "addsensorreading.sh"
util/addsensorreading.sh

echo "querysensorreadings.sh"
util/querysensorreadings.sh

echo "querylatestreading.sh"
util/querylatestreading.sh

#echo "listlogs.sh"
#util/listlogs.sh
