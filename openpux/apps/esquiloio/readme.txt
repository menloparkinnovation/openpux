
http://localhost:8080/esquiloio/weather.html

Weather station example using HTML calling a remote server
using a Javascript RPC (erpc) from Esquilo.io.

AngularJS version.

Note: Weather.nut is the Squirrel language which runs in the Esquilo.io
board. erpc is a basic binding for the client side javascript to the
getWeather() function on the board.

Openpux provides an alternate server side implemention in this
example application in app/esquiloio/server/weatherapp.js
and is invoked directly from the AngularJS $http service.

Currently this is just a simulator with random readings
to demonstrate how to support existing HTTP RPC formats
on the openpux server.

A more complete example hooked up to openpux/smartpux IoT protocol
is in apps/weather/..

12/29/2015

//
// From Esquilo.io (Squirrel board). Open Commons License.
//
https://github.com/esquiloio/lib/tree/master/demos/weather

  This work is released under the Creative Commons Zero (CC0) license
  http://creativecommons.org/publicdomain/zero/1.0/

