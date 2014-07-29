
rem
rem Configure basic sensor settings
rem

node client.js SETSENSOR localhost

rem
rem  Send some readings for query to work
rem

node client.js SENDREADINGS localhost

rem
rem Validate that query works
rem

node client.js QUERYSENSOR localhost
