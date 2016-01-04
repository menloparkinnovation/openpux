
http://localhost:8080/domweather/domweather.html

DOM version of the weather station example using HTML calling a
remote server using a Javascript RPC (erpc) from Esquilo.io.

Note: Weather.nut is the Squirrel language which runs in the Esquilo.io
board. erpc is a basic binding for the client side javascript to the
getWeather() function on the board.

Openpux provides an alternate client side implementation by mapping
the client request for "/js/erpc.js" to "/openpuxclient.js", which provides
a client side compatible erpc() function.

In addition a server side implementation of the erpc under the openpux
REST rpc protocol is handled by the provided server in the application.

The configuration mappings that accomplish this are:

//
// openpux/config/xxx_config.json, "Applications": [ ...
//

{ "name": "domweather", "url": ["/domweather/", "/erpc", "/js/erpc.js"], "server": "domweatherapp.js"},

apps/domweather/server/domweatherapp.js:

var appConfig = {
    name:     "domweather",
    url_root: "/domweather/",
    erpc_url_root: "/domweather/erpc",

    erpc_client_compat_url: "/js/erpc.js",
    erpc_compat_url: "/erpc",

    remote_method_table: [
        { name: "getWeather",  func: App.prototype.getWeather }
    ]
};

This results in client requests for /js/erpc.js -> openpuxclient.js
which contains the function erpc(...) which communicates with
URL /erpc.

12/29/2015

//
// From Esquilo.io (Squirrel board). Open Commons License.
//
https://github.com/esquiloio/lib/tree/master/demos/weather

  This work is released under the Creative Commons Zero (CC0) license
  http://creativecommons.org/publicdomain/zero/1.0/


