
rem TODO: figure out what smartpux.com is complaining about in the query string
rem look at logs

rem this is 400 failure
node client.js SETSENSOR www.smartpux.com 80 querystring="A=1&S=1&P=12345678&M0=0&M1=0000&M2=0000&M3=0000"

rem bad password is 404 failure
rem node client.js SETSENSOR www.smartpux.com 80 querystring="A=1&S=1&P=87654321&M0=0&M1=0000&M2=0000&M3=0000"

rem node client.js SETSENSOR www.smartpux.com 80 querystring="A=1&S=1&P=12345678&M0=0&M1=0000&M2=0000&M3=0000"
rem node client.js SETSENSOR www.smartpux.com 80 querystring="A=1&S=1&P=12345678&C=0&U=30&M0=0&M1=0000&M2=0000&M3=0000"
rem node client.js SETSENSOR www.smartpux.com 80 querystring="A=1&S=1&P=87654321&C=0&U=30&M0=0&M1=0000&M2=0000&M3=0000"



