
//
//   Openpux: The Operating System for IoT
//
//   Openpux.js
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

var fs = require('fs');

var url = require('url');

var querystring = require('querystring');

var loggerFactory = require('./logger');

var accountingFactory = require('./accounting');

var storageFactory = require('./storage.js');

var ticketServerFactory = require('./ticket.js');

var fileServerFactory = require('./fileserver.js');

var appServerFactory = require('./appserver.js');

var credentialsServerFactory = require('./credentials.js');

var configServerFactory = require('./configserver.js');

var rpcServerFactory = require('./rpcserver.js');

var restServerFactory = require('./restserver.js');

var execServerFactory = require('./execserver.js');

var openpux = null; // constructed at bottom of this file

//
// Main Module Start
//
// This is invoked from app.js after processing the command line arguments.
//
// config - Javascript object created from the master configuration
//          file such as config/awssimpledb_config.json, or the built
//          in default configuration file in app.js.
//
// configFile - If null, the built in configuration file in app.js is used.
//              If != null, the name of the configuration file loaded from
//              the file system.
//
function Openpux(config, configFile)
{
    this.moduleName = "Openpux";
    this.config = config;
    this.configFile = configFile; // optional file path name

    this.trace = false;
    this.traceerrorValue = false;

    this.logger = null;

    this.accounting = null;

    if (typeof(this.config.Trace) != "undefined") {
        this.trace = this.config.Trace;
    }

    if (typeof(this.config.TraceError) != "undefined") {
        this.traceerrorValue = this.config.TraceError;
    }

    // Start the logger
    this.StartLogger();

    // Start accounting
    this.StartAccounting();

    //
    // Instrastructure services are exported through config
    // for use by other modules, and loadable applications.
    //
    // Note: Initialization order here conforms to layering as later
    // services are not available during initialization of early services.
    //

    this.config.utility = require('./utility.js');

    this.config.logger = this.logger;

    this.config.accounting = this.accounting;

    // credentials server is used by storage for signon credentials.
    this.config.credentialsserver = new credentialsServerFactory.Server(this.config);

    this.config.storage = new storageFactory.Server(this.config);

    // configserver requires storage for database held configuration
    this.config.configserver = new configServerFactory.Server(this.config);

    this.config.ticketserver = new ticketServerFactory.Server(this.config);

    this.config.fileserver = new fileServerFactory.Server(this.config);

    this.config.execserver = new execServerFactory.Server(this.config);

    this.config.rpcserver = new rpcServerFactory.Server(this.config);

    this.config.restserver = new restServerFactory.Server(this.config);

    //
    // Load and Initialize the application server
    //
    this.config.appserver = new appServerFactory.Server(this.config);

    this.config.appserver.initialize();
}

Openpux.prototype.StartLogger = function() {

    var loggerConfig = {
        InstanceName: "openpux",
        LogDirectory : this.config.LogFileDirectory,
        ConsoleOutput: this.config.LogToConsole
    };

    this.logger = new loggerFactory.Server(loggerConfig);

    // Create entry describing the configuration file used
    if (this.configFile != null) {
        this.logger.info("using configuration file " + this.configFile);
    }
    else {
        this.logger.info("using memory only built in configuration file");
    }
}

Openpux.prototype.StartAccounting = function() {

    var accountingConfig = {
        InstanceName: "openpux",
        LogDirectory : this.config.AccountingFileDirectory,
        ConsoleOutput: this.config.AccountingLogToConsole
    };

    this.accounting = new accountingFactory.Server(accountingConfig);
}

//
// Main dispatch function. This function is invoked for
// each HTTP request from http.createServer.listen()
//
// Process headers and dispatch according to the request.
//
// This function is responsible for ending the request
// with a response, error or not.
//
// There are two types of async processing in this module:
//
//  All http request functions with req, res are invoked as a result
//  of an async http request and the http connection is held open.
//
//  These functions are responsible for ensuring that an http
//  response is eventually sent on success or error. This can
//  be done by other functions asynchronously queued by
//  the http request function.
//
// The second class represent intermediate processing in which
// a full http response is not yet available. In these cases
// the common node.js processing pattern of callback(error, data)
// is used. Examples here are accessing external storage during the
// processing of an http request which requires I/O and thus
// "waiting", which is deferred processing to a future turn
// in node.js.
//
// http.IncomingMessage request;
//   http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_http_incomingmessage
//   http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_readable
//
// http.ServerResponse response;
//   http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_class_http_serverresponse
//   http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_writable
//
Openpux.prototype.processRequest = function (req, res) {

  this.tracelog("processHeadersAndDispatch:");
  this.tracelog("req.url=" + req.url);

  if (this.config.appserver.processAppRequest(req, res)) {
      // fallthrough
  }
  else if (this.config.fileserver.processFileServer(req, res)) {
      // fallthrough
  }
  else {

      //
      // Unrecognized request
      //
      this.tracelog("Unknown URL: " + req.url + "\n");
      var errormsg = req.socket.remoteAddress + " requested unknown url " + req.url;
      this.config.utility.logSendErrorAsJSON(this.logger, req, res, 400, errormsg);
      return false;
  }

  return true;
}

Openpux.prototype.setTrace = function(value) {
    this.trace = value;
}

Openpux.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

Openpux.prototype.tracelog = function(message) {
    if (this.trace) {
        this.logger.info(this.moduleName + ": " + message);
    }
}

Openpux.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        this.logger.error(this.moduleName + ": " + message);
    }
}

module.exports = {
  Server: Openpux
};
