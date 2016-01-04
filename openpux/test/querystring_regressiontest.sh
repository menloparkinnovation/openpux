
# Configure basic sensor settings
# Note: This depends on automatic account and sensor creation

# updatesensorsettings.sh
node client/javascripts/client.js UPDATESENSORSETTINGS localhost 8080 querystring="A=1&S=1&P=12345678&C=0&U=30&M0=0&M1=0001&M2=0002&M3=0003"

# querysensorsettings.sh
node client/javascripts/client.js QUERYSENSORSETTINGS localhost 8080 querystring="A=1&S=1&P=12345678"

# addsensorreading.sh
node client/javascripts/client.js ADDSENSORREADING localhost 8080 querystring="A=1&S=1&P=12345678&D0=0000&D1=0001&D2=0002&D3=3&M0=0&M1=0001&M2=0002&M3=0003"

# querysensorreadings.sh
node client/javascripts/client.js QUERYSENSORREADINGS localhost 8080 querystring="A=1&S=1&P=12345678"
