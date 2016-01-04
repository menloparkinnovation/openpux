
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

// Specified configuration file
var g_configFile = null;

//
// Default Configuration:
//
// This is overriden by config.json if present.
//
var g_defaultConfig = {

    // Logging support
    LogFileDirectory: null,
    LogToConsole: true,

    AccountingFileDirectory: null,
    AccountingLogToConsole: true,

    // Trace level support
    Trace: true,
    TraceError: true,
    DebugTrace: true,

    //
    // Default Administrator account id
    //
    AdministratorAccountID: "0",

    //
    // Default administrator token/ticket. This is used for
    // provisioning and testing and *must* be removed for
    // deployment.
    //
    // It's not set here since best practice is to use a
    // specific maintainence serverconfig.json that is manually launched
    // when provisioning, or maintaining a data store.
    //
    AdministratorPassCode: null,

    //
    // Allow regular user accounts to create their own
    // sensors.
    //
    AllowCreateSensorsByAccount: true,

    // Configurable backing stores
    BackingStoreProvider: null,

    CacheTimeout: 30,

   //
   // Maximum Account entries allowed in storage cache
   //
   // Note: When no backing store and AccountTrimCount == 0 this
   // is the maximum accounts allowed.
   //
   AccountMaxEntries: 100,

   //
   // Number of entries to automatically trim when AccountMaxEntries
   // is hit. If the value is 0 then existing entries are not replaced.
   //
   // If the value != 0 then entries are replaced in an LRU fashion
   // AccountTrimCount at a time.
   //
   AccountTrimCount: 0,  // Existing account entries are not replaced on overflow

   //
   // Maximum Items entries in the cache
   //
   ItemsMaxEntries: 100,

   //
   // Number of entries to automatically trim when ItemsMaxEntries
   // is hit. If the value is 0 then existing entries are not replaced.
   //
   // If the value != 0 then entries are replaced in an LRU fashion
   // ItemsTrimCount at a time.
   //
   ItemsTrimCount: 10,

   //
   // This limits how many sensors an account may auto create.
   //
   SensorsMaxEntriesPerAccount: 100,

   //
   // Number of entries to automatically trim when SensorMaxEntriesPerAccount
   // is hit. If the value is 0 then existing entries are not replaced.
   //
   // If the value != 0 then entries are replaced in an LRU fashion
   // SensorsTrimCount at a time.
   //
   SensorsTrimCount: 0,   // Existing sensor entries are not replaced on overflow

   //
   // Maximum cached readings per sensor
   //
   ReadingsMaxPerSensor: 100,

   //
   // This configures how many readings to trim when the readings
   // table is full.
   //
   ReadingsTrimCount: 10,   // LRU replacement of readings, trim 10% per overflow

    ticket_cache: {
        max_entries: 100,
        trim_count: 10
    },

    Applications:  [
        { name: "openpux",    url: "/api/v1/",         server: "openpuxapp.js" },
        { name: "rester",     url: "/api/v2/",         server: "resterapp.js" },
        { name: "admin",      url: "/administration/", server: "adminapp.js" },
        { name: "ticket",     url: "/ticket/",         server: "ticketapp.js" },
        { name: "smartpux",   url: "/smartpuxdata/",   server: "smartpuxapp.js" },
        { name: "default",    url: "/",                server: "defaultapp.js", 
            app: "openpux", default_url: "/sensor" }
     ]
};

var loggerFactory = require('./server/logger');

var accountingFactory = require('./server/accounting');

var storageFactory = require('./server/storage.js');

var ticketServerFactory = require('./server/ticket.js');

var fileServerFactory = require('./server/fileserver.js');

var appServerFactory = require('./server/appserver.js');

var rpcServerFactory = require('./server/rpcserver.js');

var restServerFactory = require('./server/restserver.js');

var execServerFactory = require('./server/execserver.js');

var openpux = null; // constructed at bottom of this file

//
// Main Server Start
//
function Openpux(config)
{
    this.moduleName = "Openpux";
    this.config = config;

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

    this.config.utility = require('./server/utility.js');

    this.config.logger = this.logger;

    this.config.accounting = this.accounting;

    this.config.storage = new storageFactory.Server(this.config);

    this.config.ticketserver = new ticketServerFactory.Server(this.config);

    this.config.fileserver = new fileServerFactory.Server(this.config);

    this.config.execserver = new execServerFactory.Server(this.config);

    this.config.rpcserver = new rpcServerFactory.Server(this.config);

    this.config.restserver = new restServerFactory.Server(this.config);

    this.config.appserver = new appServerFactory.Server(this.config);
}

Openpux.prototype.StartLogger = function() {

    var loggerConfig = {
        InstanceName: "openpux",
        LogDirectory : this.config.LogFileDirectory,
        ConsoleOutput: this.config.LogToConsole
    };

    this.logger = new loggerFactory.Server(loggerConfig);

    // Create entry describing the configuration file used
    if (g_configFile != null) {
        this.logger.info("using configuration file " + g_configFile);
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
Openpux.prototype.processHeadersAndDispatch = function (req, res) {

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

//
// Direct Execution block to load the configuration file, construct the
// openpux instance, and start the web server.
//

//
// Remove argv[0] to get to the base of the standard arguments.
// The first argument will now be the script name.
//
var g_args = process.argv.slice(1);

// Lookup the configuration file
if (g_args.length > 1) {
    g_configFile = g_args[1];
    console.log("using supplied configuration file " + g_configFile);
}

if (g_configFile != null) {
    try {
        var jsonText = fs.readFileSync(g_configFile);

        try {
            g_config = JSON.parse(jsonText);

            // Successfully loaded
            console.log("loaded parameters from " + g_configFile);
            console.log(g_config);

        } catch(ee) {
          console.error("error=" + ee);
          console.error("error parsing specified configuration file " + g_configFile);
          process.exit(1);         
        }

    } catch (e) {
        console.error("error=" + e);
        console.error("can't read specified configuration file " + g_configFile);
        process.exit(1);         
    }
}
else {
    console.log("no config file specified, using built in configuration");
    g_config = g_defaultConfig;
}

openpux = new Openpux(g_config);

//
// Http Server
//

var http = require('http');

// http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_http_createserver_requestlistener
http.createServer(function(req, res) {
    openpux.processHeadersAndDispatch(req, res);
}).listen(g_config.http_ipv4_listen_port, g_config.http_ipv4_listen_address);

console.log("HttpTest Server version 3 running at http://" + 
            g_config.http_ipv4_listen_address + ":" + g_config.http_ipv4_listen_port);
