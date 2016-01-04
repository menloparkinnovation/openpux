
# Configure basic sensor settings
# Note: This depends on automatic account and sensor creation

node apps/openpux/client/javascripts/client.js createaccount localhost 8080 jsonfile=util/createaccount.json

node apps/openpux/client/javascripts/client.js createsensor localhost 8080 jsonfile=util/createsensor.json

node apps/openpux/client/javascripts/client.js createsensorsettings localhost 8080 jsonfile=util/createsensorsettings.json

node apps/openpux/client/javascripts/client.js updatesensorsettings localhost 8080 jsonfile=util/updatesensorsettings.json

node apps/openpux/client/javascripts/client.js addsensorreading localhost 8080 jsonfile=util/sensorreading.json

node apps/openpux/client/javascripts/client.js querysensorreadings localhost 8080 jsonfile=util/querysensorreadings.json

