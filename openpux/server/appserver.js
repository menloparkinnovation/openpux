
//
//   appserver.js
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
// This provides support for application modules which consist
// of client content and optional server back end app.js file
// as a module.
//

var path = require('path');

//
// This represents generic locations for client content.
//
// Each application is expected to place its content in the
// proper location rooted by apps/<applicationName>/ to
// server this content to the client automatically.
//
// Example:
//
// /appurl/app.html -> apps/appname/client/html/app.html
//
var extensionToPathTable = [
  { extension: ".js",     path: "client/javascripts/" },
  { extension: ".css",    path: "client/css/" },
  { extension: ".html",   path: "client/html/" }
];

//
// If an application provides an optional server module
// it must be the following path in the applicaiton content directory.
//
// Example: apps/weather/server/app.js
//
var serverPath = "server/";

//
// An application entry has the following format:
//
// { "name": "weather",    "url": "/weather/",     "server": "weather.js" },
//
//

function AppServer(config)
{
    this.moduleName = "AppServer";
    this.config = config;

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

    this.utility = this.config.utility;
}

//
// This returns a loaded instance of the application.
//
// It attempts to load the server for the application if
// its not already loaded.
//
// This can be called by applications which layer on top of
// other applications to ensure their server module is loaded
// and have a configured instance to it.
//
AppServer.prototype.getAppInstanceByName = function (name) {
    var app = this.findAppByName(name);
    if (app == null) return null;

    // Returns the app on success, null on failure.
    app = this.getAppInstance(app);
    if (app == null) return null;

    return app.serverInstance;
}

//
// Lookup an applications entry by name
//
AppServer.prototype.findAppByName = function (name) {

    if ((typeof(this.config.Applications) == "undefined") ||
        (this.config.Applications.length == 0)) {

        // No configured applications
        return null;
    }

    var apps = this.config.Applications;

    for (var prop in apps) {
        if (apps[prop].name == name) {
            return apps[prop];
        }
    }

    return null;
}

//
// This returns a loaded instance of the application.
//
// It attempts to load the server for the application.
//
AppServer.prototype.getAppInstance = function (app) {

    if ((typeof(app.server) == "undefined") || (app.server == null)) {

        // No appserver.js configured
        return null;
    }

    // If this was set a load was already attempted.
    if (typeof(app.serverInstance) != "undefined") {
        
        if (app.serverInstance == null) {
            // did not successfully load appserver.js previously
            return null;
        }

        // we have a valid, loaded appserver.js
        return app;
    }

    //
    // No attempt has been made to load the app server, try it now
    //

    //
    // We set instance to null on the first attempt so we don't
    // try and reload on error for every request.
    //
    app.serverInstance = null;
             
    var appServerPath = "../apps/" + app.name + "/" + serverPath + app.server;

    try {
        var appFactory = new require(appServerPath);
        app.serverInstance = new appFactory.App(this.config);
        return app;
    } catch (e) {
        var logMessage = "e=" + e.toString() + " stack=" + e.stack.toString();
        logMessage += " server app configuration error, exception loading app module " + appServerPath;
        this.logger.error(logMessage);
        return null;
    }
}

//
// Process an App Request
//
// Attempt to lookup an application that handles the req.url
//
// Returns:
//
//  true - Path was handled, even if there was an error.
//
//  false - Path was not handled, continue looking for another handler.
//
AppServer.prototype.processAppRequest = function (req, res) {

    if ((typeof(this.config.Applications) == "undefined") ||
        (this.config.Applications.length == 0)) {

        // No configured applications
        return false;
    }

    var apps = this.config.Applications;

    //
    // If we return false the appserver logic will continue searching for
    // another module to handle the url.
    //
    // Application modules that return true have "claimed" the url and are
    // responsible for the eventual async completion of the request, whether
    // with success or error by writing to the response object.
    //
    // This allows one application module to handles "GET" for example, and
    // another to handle "POST" on the same url if it makes sense for the
    // applications organization.
    //
    var handled = false;

    //
    // Application configuration entries have the following form:
    //
    // "Applications":  [
    //    { "name": "weather",    "url": "/weather/",        "server": "weatherapp.js" },
    //    { "name": "domweather", "url": ["/domweather/", "/erpc", "/js/erpc.js"], "server": "domweatherapp.js"}
    //  ]
    //
    // Note that multiple url entries are allowed for an application the
    // handles multiple roots. In the example above the domweather application
    // server also handles the compatibility erpc url /erpc and /js/erpc.js
    //
    // A last entry with url "/" could handle root level requests such
    // "/default.html" and redirect to the proper application, including
    // looking them up and requesting that they be dynamically loaded
    // through findAppByName(), getAppInstanceByName().
    //
    // A name of "*" could be used to lookup an application in the file
    // system that is not present in serverconfig.json.
    //
    // This would allow development/lab scenarios in which uploading a new
    // application to the apps/ directory makes it available immediately
    // without updating config/serverconfig.json and possibly restarting
    // the server.
    //
    // Note that in this case if app.js is missing from the directory
    // standard file serving will still occur since ad-hoc applications
    // may consist of client side only content using the standard
    // infrastructure providers.
    //
    //    { "*": "", "url": "", "server": "app.js"}
    //
    for (var prop in apps) {

        // url can be an array of url entries the application handles
        if (Array.isArray(apps[prop].url)) {
            var ar = apps[prop].url;

            for (var index in ar) {
                if (req.url.search(ar[index]) == 0) {
                    handled = this.processAppUrlRequest(req, res, apps[prop], ar[index]);
                    return handled;
                }
            }
        }
        else {
            if (req.url.search(apps[prop].url) == 0) {
                handled = this.processAppUrlRequest(req, res, apps[prop], apps[prop].url);
                return handled;
            }
        }
    }

    return false;
}

//
// Process a request for a specific application module
//
// This is called when the URL matches the supplied application
// entry.
//
// app - application configuration entry
//
// app_url - URL that caused the app to be invoked
//
// Returns:
//
//  true - Path was handled, even if there was an error.
//
//  false - Path was not handled, continue looking for another handler.
//
AppServer.prototype.processAppUrlRequest = function (req, res, app, app_url) {
    
    //
    // TODO: Validate credentials to app domain!
    //

    var handled = false;

    //
    // See if a server side application is configured
    //
    if ((typeof(app.server) != "undefined") && (app.server != null)) {

        var appServer = this.getAppInstance(app);
        if (appServer == null) {

            //
            // A server was configured, but could not be loaded.
            // return an error.
            //
            this.utility.logSendErrorAsJSON(this.logger, req, res, 503,
                    "could not load application server " + app.server);

            return true;;
        }

        handled = appServer.serverInstance.processAppRequest(req, res, app, app_url, this);
        return handled;
    }

    if (req.method != "GET") {
        this.utility.logSendErrorAsJSON(this.logger, req, res, 400, "request not supported by app");
        return true;
    }

    //
    // if we get here there is no application server side module
    // configured. Serve default client side requests to its
    // application client directory.
    //
    // This allows the simple construction of applications that consist
    // of just client side programming against the standard Openpux API's
    // and data services using tools such as jQuery, AngularJS, or plain
    // old Javascript + DOM.
    //
    return this.processAppGetRequest(req, res, app, app_url);
}

//
// Process standard get requests for an application.
//
// app - Applications configuration entry
//
// app_url - Base url that was matched for the application
//
// Returns:
//
//  true - Path was handled, even if there was an error.
//
//  false - Path was not handled, continue looking for another handler.
//
AppServer.prototype.processAppGetRequest = function (req, res, app, app_url) {

    // trim root path
    var name = req.url.substring(app_url.length, req.url.length);
    if (name == "") {
        this.utility.logSendErrorAsJSON(this.logger, req, res, 400, "bad path");
        return true;
    }

    return this.processUrlEntry(req, res, app, name);
}

//
// Process a simple file entry
//
// app - Applications configuration entry
//
// name - Simple resource name such as sensor.html, lib.js, sensor.css
//
// Returns:
//
//  true - Path was handled, even if there was an error.
//
//  false - Path was not handled, continue looking for another handler.
//
AppServer.prototype.processUrlEntry = function (req, res, app, name) {

    //
    // Only simple paths such as file, file.html are allowed, not subdirectories such
    // as file/child.html
    //
    if (!this.utility.validateSimpleFilePath(name)) {
        this.utility.logSendErrorAsJSON(this.logger, req, res, 400, "bad path");
        return true;
    }

    // get extension
    var ext = path.extname(name);
    var base = name.substring(0, name.length - ext.length);

    // Get its content directory
    var filePath = this.extensionToPath(ext, base, app);
    if (filePath != null) {

        // Add a log entry that the log file was retrieved
        var remoteAddr = req.socket.remoteAddress;
        this.logger.info(remoteAddr + " retrieved file path=" + filePath);

        this.config.fileserver.serveFile(req, res, filePath);
        return true;
    }

    this.utility.logSendErrorAsJSON(this.logger, req, res, 404, "path not found");

    return true;
}

//
// This is invoked by the generic application handler
// to map file extentions to paths.
//
AppServer.prototype.extensionToPath = function (ext, base, app) {

    var appRoot = "apps/" + app.name + "/";

    // Get its content directory
    for (var prop in extensionToPathTable) {
       if (ext == extensionToPathTable[prop].extension) {

            //
            // /appuri/sensor.html -> apps/appname/client/sensor.html
            // /appuri/library.js  -> apps/appname/client/javascripts/library.js
            // /appuri/library.css -> apps/appname/client/css/library.cs
            //
            return appRoot + extensionToPathTable[prop].path + base + ext;
       }
    }

    return null;
}

module.exports = {
  Server: AppServer
};
