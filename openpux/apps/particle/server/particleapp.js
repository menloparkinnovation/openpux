
//
//   particleapp.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2016 Menlo Park Innovation LLC
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

var particleFactory = require('./particlecloud.js');

var path = require('path');

var utility = require('../../../server/utility.js');

//
// This application provides support for the Particle cloud
// and allows registering for Particle Events, as well as sending
// requests to Particle Photons, Electron's, and other Particle
// cloud managed IoT devices and applications.
//

function App(config, appConfiguration)
{
    var self = this;

    this.config = config;
    this.appconfig = appConfiguration;

    console.log(this.appconfig);

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

    this.configserver = this.config.configserver;

    // particle_token must be set by login before use of the API's
    this.particle_token = null;

    self.particle_module_config = {
        Trace: self.trace,
        TraceError: self.traceerrorValue
    };

    // Load the particlemodule.js
    self.appmodule = new particleFactory.Module(self.particle_module_config);

    self.logger.info("Particle service loaded");
}

//
// Returns the reference to the particle API's
//
// No need to wrap them for direct/simple access.
//
App.prototype.getParticleApi = function() {
    return this.appmodule;
}

App.prototype.getParticleToken = function() {
    return this.particle_token;
}

//
// Start a login to the particle cloud.
//
// Communications can't be done till the login completes, which is asyc.
//
// Callback is invoked once logged in, or error.
//
// callback(error);
//
App.prototype.login = function(username, password, callback) {

    var self = this;

    var credentials = {
        username: username,
        password: password
    };

    self.appmodule.login(credentials, function(error, token) {
        if (error != null) {
            self.logger.error("particleapp: error logging into particle cloud " + error);
            callback(error);
            return;
        }

        self.logger.info("particleapp: logged into particle cloud");

        // We can now call particle API's
        self.particle_token = token;

        callback(null);
    });
}

//
// Handle an application message
//
App.prototype.HandleMessage = function(body, message, callback) {
    var response_message = "unhandled message";

    //
    // TODO: EventEmitter design for arriving messages from
    // the particle cloud.
    //

    callback(response_message);
}

//
// callback(error, sid)
//
App.prototype.SendMessage = function(to, from, message, callback) {

    callback("not implemented", null);
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

  // Specific configured application path not found.
  return false;
}

module.exports = {
  App: App
};
