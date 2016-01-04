
rem
rem Configure basic sensor settings
rem

node client\javascripts\client.js UPDATESENSORSETTINGS localhost

rem
rem  Send some readings for query to work
rem

node client\javascripts\client.js ADDSENSORREADING localhost

rem
rem Validate that query works
rem

node client\javascripts\client.js QUERYSENSORSETTINGS localhost

node client\javascripts\client.js QUERYSENSORREADINGS localhost
