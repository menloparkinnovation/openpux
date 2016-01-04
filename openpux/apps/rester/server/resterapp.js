
//
// REST Root:      /api/v2/
//
// Storage Root:   /accounts
//
// see appconfig
//

//
//   resterapp.js
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

var url = require('url');

//
// rester:
//
// Provides basic data only access through standard REST operations.
//
// Provides quick REST data/object operations against a data store for
// building custom IoT applications.
//

var g_appConfig = {
    name:     "rester",
    url_root: "/api/v2/",
    storage_root: "/accounts"
};

function App(config)
{
    this.appconfig = g_appConfig;
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

    this.storage = config.storage;

    this.utility = config.utility;
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

    if (req.url.search(this.appconfig.url_root) != 0) {
        // Not our root
        return false;
    }

    // Pass the request to the REST service provider with our configuration
    this.config.restserver.processObjectRequest(this.appconfig, req, res);
    return true;
}

App.prototype.setTrace = function(value) {
    this.trace = value;
}

App.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

App.prototype.tracelog = function(message) {
    if (this.trace) {
        this.logger.info(this.moduleName + ": " + message);
    }
}

App.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        this.logger.error(this.moduleName + ": " + message);
    }
}

App.prototype.debuglog = function(message) {
    if (this.debug) {
        this.logger.log("debug", this.moduleName + ": " + message);
    }
}

module.exports = {
  App: App
};
