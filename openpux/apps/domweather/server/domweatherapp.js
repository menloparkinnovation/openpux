
//
//   domweatherapp.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014,2015 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Snapshot License
//
//   This license is for a specific snapshot of a base work of
//   Menlo Park Innovation LLC on a non-exclusive basis with no warranty
//   or obligation for future updates. This work, any portion, or derivative
//   of it may be made available under other license terms by
//   Menlo Park Innovation LLC without notice or obligation to this license.
//
//   There is no warranty, statement of fitness, statement of
//   fitness for any purpose, and no statements as to infringements
//   on any patents.
//
//   Menlo Park Innovation has no obligation to offer support, updates,
//   future revisions and improvements, source code, source code downloads,
//   media, etc.
//
//   This specific snapshot is made available under the following license:
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

var path = require('path');

var utility = require('../../../server/utility.js');

//
// This is a demonstration of support for a generic application
// that combines client and specific backend logic as a module.
//

//
// Remote Methods are invoked by the applications client side
// to retrieve data and perform the applications functions.
//

//
// All methods take params, callback.
//
// params could be null.
//
App.prototype.getWeather = function(params, req, callback) {

    // C, percent, millibars

    var temp = 43 * Math.random();
    var humidity = 130 * Math.random();

    var pressure_floor = 98250;
    var pressure_max = 108200;
    var pressure_diff = pressure_max - pressure_floor;

    var pressure = pressure_floor + (pressure_diff * Math.random());

    callback(null,  { temp: temp, humidity: humidity, pressure: pressure });
}

//
// erpc root for Weather App
//
// Note: Unmodified Esquilo.io test clients just invoke
// on /erpc expecting the embedded board to be dedicated
// to the task.
//
// For serving/testing unmodfied applications we provide an
// alternate entry at the root. The first application configured
// in config/serverconfig.json will handle the request.
//
// A small modification to the clients to invoke "/appname/erpc"
// allows separation.
//

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

function App(config)
{
    this.appconfig = appConfig;
    this.config = config;

    this.moduleName = this.appconfig.name;

    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(this.config.Trace) != "undefined") {
        this.trace = this.config.Trace;
    }

    if (typeof(this.config.TraceError) != "undefined") {
        this.traceerrorValue = this.config.TraceError;
    }

    // Setup the inherited logger
    this.logger = this.config.logger;
}

//
// Process an App Request
//
// This is invoked by the generic application handler to
// query for, and handle an application path URL.
//
// Parameters:
//
// req - http request
//
// res - http response
//
// app - Applications config entry in config/serverconfig.json
//
// app_url - Specific application URL that was hit that caused the invoke
//
// appserver - instance to appserver.js for utility functions to aid
//       in generic request handling.
//
// Returns:
//
//  true - Path was handled, even if there was an error.
//
//  false - Path was not handled, continue looking for another handler.
//
App.prototype.processAppRequest = function (req, res, app, app_url, appserver) {

  if (req.method == "GET") {

      //
      // This demonstrates handling special case client side application
      // compatiblity url's
      //
      if (req.url.search(this.appconfig.erpc_client_compat_url) == 0) {

          //
          // request for compatibility erpc client "/js/erpc.js", so redirect
          // to the openpux client which offers an erpc compatibility interface.
          //
          this.config.fileserver.serveFile(req, res, "apps/openpux/client/javascripts/openpuxclient.js");
          return true;
      }

      // GET requests a handled by the default handler and mime type mappings
      if (req.url.search(this.appconfig.url_root) == 0) {
          return appserver.processAppGetRequest(req, res, app, app_url);
      }

      return false;
  }  

  //
  // This application only handles GET, POST, so tell the appserver
  // to keep searching for a handler.
  //
  if (req.method != "POST") {
      return false;
  }

  //
  // See if "/erpc" application compatibility is enabled.
  //
  // "/erpc" is designed for small sensor boards with dedicated
  // applications. The first application configured which handles
  // the "/erpc" entry services it and claims the URL.
  //
  // For this application it's enabled with an alias entry in
  // config/serverconfig.json to route "/erpc" requests to this module.
  //
  if (this.appconfig.erpc_compat_url != null) {
      if (req.url.search(this.appconfig.erpc_compat_url) == 0) {
          this.config.rpcserver.process_rpc_Request(req, res, this.appconfig.remote_method_table, this);
          return true;
      }
  }

  //
  // The preferred rpc/erpc mechanism is to use application
  // specific URL's which allow multiple applications to operate
  // side by side on the same server instance.
  //
  // This requires a small change to erpc demonstration applications
  // to use openpuxclient.js instead of erpc.js which allows application
  // URL invoke.
  //
  // Some applications such as DomWeather.html perform erpc inline
  // and a simple change to the HTTP POST path accomplishes this.
  //
  if (req.url.search(this.appconfig.erpc_url_root) == 0) {
      this.config.rpcserver.process_erpc_Request(req, res, this.appconfig.remote_method_table, this);
      return true;
  }
  else {
      return false;
  }
}

module.exports = {
  App: App
};
