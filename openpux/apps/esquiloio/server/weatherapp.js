
//
//   weatherapp.js
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
// This server simulates weather data in a simple rpc format
// compatible with examples from
//
// https://github.com/esquiloio/lib/blob/master/demos/weather/angular/weather.html
//

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

    var windspeed = 90 * Math.random();
    var winddirection = 359 * Math.random();

    var data = {
        temp: temp,
        humidity: humidity,
        pressure: pressure,
        windspeed: windspeed,
        winddirection: winddirection
    };

    //callback(null,  { temp: temp, humidity: humidity, pressure: pressure });
    callback(null, data);

}

var appConfig = {
    name:     "esquiloio",
    url_root: "/esquiloio/",
    erpc_url_root: "/esquiloio/weather/erpc",

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

      // GET requests a handled by the default handler and mime type mappings
      if (req.url.search(this.appconfig.url_root) == 0) {
          appserver.processAppGetRequest(req, res, app, app_url);
          return true;
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
  // The preferred rpc/erpc mechanism is to use application
  // specific URL's which allow multiple applications to operate
  // side by side on the same server instance.
  //
  // This requires a small change to erpc demonstration applications
  // to use openpuxclient.js instead of erpc.js which allows application
  // URL invoke.
  //
  // See apps/domweather for an example of a 100% compatible server for
  // unmodified erpc applications.
  //
  if (req.url.search(this.appconfig.erpc_url_root) == 0) {
      this.config.rpcserver.process_rpc_Request(req, res, this.appconfig.remote_method_table, this);
      return true;
  }
  else {
      return false;
  }
}

module.exports = {
  App: App
};
