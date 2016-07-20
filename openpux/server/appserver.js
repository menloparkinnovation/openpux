
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
  { extension: ".html",   path: "client/html/" },
  { extension: ".png",    path: "client/images/" },
  { extension: ".jpg",    path: "client/images/" }
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

    this.configserver = this.config.configserver;
}

//
// Initialize the application server.
//
AppServer.prototype.initialize = function () {

    var self = this;

    //
    // First load any external application configuration file.
    // Typically config/applications.json
    //
    self.loadApplicationsConfigurationFile();

    //
    // Now that all of the statically configured application information
    // is loaded now configure the applications by looking up their
    // configuration.json file if present.
    //
    self.configureApplications();

    //
    // Load any application that request to be loaded at startup.
    //
    self.loadBootApps();
}

//
// Load the applications.json configuration file if it exits
// and add any Applications[] entries to the main one.
//
AppServer.prototype.loadApplicationsConfigurationFile = function () {

    var self = this;

    if ((typeof(self.config.ApplicationsFile) == "undefined") ||
        (self.config.ApplicationsFile == null)) {

        self.logger.info("no external ApplicationsConfigurationFile specified");

        // No applications.json file configured
        return;
    }

    var fileDirectory = __dirname + "/../config/";
    var filePath = fileDirectory + self.config.ApplicationsFile;

    // Try and load the config file from config directory
    var result = self.configserver.loadConfigFileSync(filePath);
    if (result.error != null) {
        self.logger.error("error loading external application configuation file " + result.error);
        return;
    }

    if ((typeof(result.config.Applications) == "undefined") ||
        (result.config.Applications.length == 0)) {

        self.logger.info("no entries specified in external ApplicationsConfigurationFile");

        // no entries
        return;
    }

    //
    // Append the array to the ones inside the main configuration file
    // if configured.
    //
    // Note: It's ensured at least an empty array exists in
    // self.config.Applications
    //
    self.config.Applications = self.config.Applications.concat(result.config.Applications);

    self.logger.info("loaded external application configuration file " + filePath);

    return;
}

//
// Configure applications.
//
// Lookup if an application has a configuration.json entry, and if
// so, load it and use its configuration in place of the current app entry.
//
AppServer.prototype.configureApplications = function () {

    var self = this;

    var apps = this.config.Applications;

    for (var prop in apps) {
        var app = apps[prop];

        if (!self.isAppEnabled(app)) {

            // skip disabled applications
            self.logger.info("appserver: " + app.name + " is disabled");
            continue;
        }

        var appconfig = self.loadApplicationConfigSync(app.name);
        if (appconfig.error != null) {

            //
            // Application does not have an application.json, its configuration
            // will be taken from the values in the Applications[] entry.
            //
            self.logger.info("appserver: " + app.name + " has no configuration.json");
            continue;
        }

        //
        // Replace the Applications[] entry with the updated
        // applications values.
        //
        self.logger.info("appserver: using " + app.name + " configuration.json");

        self.config.Applications[prop] = appconfig.config;
    }
}

//
// Load the applications configuration.json file.
//
// Returns:
//
// {
//   error: "error message or null",
//   config: null // or pointer to obj
// }
//
AppServer.prototype.loadApplicationConfigSync = function (appname) {
    var self = this;

    // fs.open uses the path where app.js resides, not where this file resides
    var appDirectoryPath = "apps/" + appname + "/";

    var appConfigurationPath = appDirectoryPath + "configuration.json";

    var config = self.configserver.loadConfigFileSync(appConfigurationPath);
    if (config.error != null) {
        var err = "AppServer: Error loading configuration.json for " + appname;
        err += " error=" + config.error;
        self.logger.info(err);
        return config;
    }

    //
    // If the application config specifies a separate credentials file load that.
    //
    // This allows applications to place credentials in a common location
    // for deployment.
    //
    var appconf = config.config;

    if ((typeof(appconf.credentials_file) == "undefined") ||
               (appconf.credentials_file == null) ||
               (appconf.credentials_file == "")) {

        // No credentials file entry
        return config;
    }

    //
    // All loaded credentials are in credentials/.. not the
    // application directory.
    //
    // This makes it easer to manage credentials in deployments
    // rather than chasing them down in each app directory.
    //
    var credentialsDirectoryPath = "credentials/";

    // Try and load the applications credentials file
    var credentialsFilePath = credentialsDirectoryPath + appconf.credentials_file;

    var creds = self.configserver.loadConfigFileSync(credentialsFilePath);
    if (creds.error != null) {
        var err = "appserver: could not load credentials file for " + appname;
        err += " filePath=" + credentialsFilePath;
        self.logger.error(err);
        return config;
    }
    else {
        // replace (or add) the credentials entry
        self.logger.info("using credentials file for app " + appname);
        config.config.credentials = creds.config;
    }

    return config;
}

//
// Load boot loaded apps.
//
// This is called at server initialization time.
//
// Loading applications at boot time ensures any initialization
// errors occur at server load, vs. when either the first url
// comes in for the application, or when another application
// requests it.
//
// Since loading uses require(), its synchronous, so boot loading
// applications avoids holding up the server for app load during
// online operations.
//
AppServer.prototype.loadBootApps = function () {

    var self = this;

    var apps = this.config.Applications;

    for (var prop in apps) {
        var app = apps[prop];

        if (self.isAppBootLoaded(app)) {
            var appInstance = self.getAppInstance(app);
            if (appInstance == null) {
                var logMessage = "server app boot load error name= " + app.name;
                self.logger.error(logMessage);
            }
        }
    }
}

//
// Returns true if the app is boot/initialization/startup time loaded.
//
AppServer.prototype.isAppBootLoaded = function (app) {

    //
    // If enabled is "true", the module is loaded at server
    // initialization.
    //
    // If enabled is "dynamic", the module is loaded at either
    // the first URL request, or if another application module
    // requests it (for service provider application modules).
    //
    if ((typeof(app.enabled) == "undefined") || (app.enabled == null)) {

        // Entry is disabled by not having an "enabled" entry.
        return false;
    }

    if ((typeof(app.server) == "undefined") || (app.server == null)) {

        // No appserver.js configured
        return false;
    }

    if (app.enabled == "true") {
        // Entry is boot loaded
        return true;
    }

    if ((typeof(this.config.ForceAllBootLoaded) == "undefined") ||
        (this.config.ForceAllBootLoaded == null)) {

        // No force boot loaded setting
        return false;
    }

    //
    // Dynamically loaded applications are loaded at boot if
    // ForceAllBootLoaded is true.
    //
    if ((app.enabled == "dynamic") &&
        (this.config.ForceAllBootLoaded == "true")) {
        return true;
    }

    return false;
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
// Lookup an applications configuration entry by name.
//
// This will return applications who are disabled, but
// present in the Applications configuration database.
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
// Returns whether the app is enabled
//
AppServer.prototype.isAppEnabled = function (app) {

    //
    // If enabled is "true", the module is loaded at server
    // initialization.
    //
    // If enabled is "dynamic", the module is loaded at either
    // the first URL request, or if another application module
    // requests it (for service provider application modules).
    //
    if ((typeof(app.enabled) == "undefined") || (app.enabled == null)) {

        // Entry is disabled by not having an "enabled" entry.
        return false;
    }

    if ((app.enabled == "true") || (app.enabled == "dynamic")) {
        return true;
    }

    // Entry is disabled.
    return false;
}

//
// This returns a loaded instance of the application.
//
// It attempts to load the server for the application.
//
// If the application is disabled, its not loaded.
//
AppServer.prototype.getAppInstance = function (app) {

    var self = this;

    if (!self.isAppEnabled(app)) {
        // Application is not enabled
        return null;
    }

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
        app.serverInstance = new appFactory.App(self.config, app);
        return app;
    } catch (e) {
        var logMessage = "e=" + e.toString();
        logMessage += " server app configuration error, exception loading app module " + appServerPath;
        self.logger.error(logMessage);
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

        if ((apps[prop].url == "undefined") ||
            (apps[prop].url == null) ||
            (apps[prop].url == "")) {

            // No url setting, continue.
            continue;
        }

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
    
    var self = this;

    //
    // Currently each app is responsible for validating its url
    // and access type in the ticket.
    //

    var handled = false;

    //
    // See if a server side application is configured
    //
    if ((typeof(app.server) != "undefined") && (app.server != null)) {

        var appServer = self.getAppInstance(app);
        if (appServer == null) {

            //
            // A server was configured, but could not be loaded.
            // return an error.
            //
            self.utility.logSendErrorAsJSON(self.logger, req, res, 503,
                    "could not load application server " + app.server);

            return true;;
        }

        handled = appServer.serverInstance.processAppRequest(req, res, app, app_url, self);
        return handled;
    }

    //
    // If the application is not enabled, we don't serve any static
    // content from its application directory as well.
    //
    if (!self.isAppEnabled(app)) {
        self.logger.error("application is disabled app.name=" + app.name + " url=" + app_url);
        self.utility.logSendErrorAsJSON(self.logger, req, res, 400, "application is disabled");
        return true;
    }

    if (req.method != "GET") {
        self.utility.logSendErrorAsJSON(self.logger, req, res, 400, "request not supported by app");
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
    return self.processAppGetRequest(req, res, app, app_url);
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
// to map file extensions to paths.
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
