
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

//
// Launcher
//

var openpuxFactory = require('./server/openpux.js');

var fs = require('fs');

// Configuration file specified on the command line
var g_configFile = null;

// Server instance
var g_openpux = null;

//
// Default Configuration:
//
// This is overriden by g_configFile if present.
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

g_openpux = new openpuxFactory.Server(g_config, g_configFile);

//
// Setup Http Server
//

var http = require('http');

// http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_http_createserver_requestlistener
http.createServer(function(req, res) {

    g_openpux.processRequest(req, res);

}).listen(g_config.http_ipv4_listen_port, g_config.http_ipv4_listen_address);

console.log(
    "HttpTest Server version 3 running at http://" + 
        g_config.http_ipv4_listen_address + ":" +
        g_config.http_ipv4_listen_port);

