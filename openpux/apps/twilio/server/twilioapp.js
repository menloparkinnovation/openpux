
//
//   twilioapp.js
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

// Twilio API handler
var twilioFactory = require('./twiliomodule.js');

var path = require('path');

var utility = require('../../../server/utility.js');

//
// This application supports Twilio for SMS text message send
// and receive.
//

//
// Parameters:
//
// config: - main server configuration with references to subsystem services
// such as logging, appserver, httpserver, data provider, etc.
//
// appConfiguration - Application configuration from either its Applications[]
// entry in the master configuration file, or the local configuration.json
// if the application supplies one.
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

    //
    // These are the registered handlers to notify applications
    // of SMS message arrival.
    //
    this.app_client_handler = null;

    //
    // Credentials are loaded by the application module loader from
    // credentials/twilio_credentials.json
    //
    this.credentials = this.appconfig.credentials;

    self.twilio_module_config = {
        Trace: self.trace,
        TraceError: self.traceerrorValue,
        logger: self.logger
    };

    // Load the Twilio handler module
    self.appmodule = new twilioFactory.Module(self.twilio_module_config);

    // Register SMS client API
    self.appmodule.RegisterClient(self.credentials.accountid, self.credentials.token);

    self.logger.info("Twilio service started and listening");

    // Register the message handling function
    self.appmodule.StartService(function(body, message, callback) {
        self.HandleMessage(body, message, callback);
    });
}

//
// Client's register interest in SMS text messages
// by invoking this function.
//
// TODO: Make this an Event registration
//
App.prototype.RegisterMessageHandler = function(handler) {
    this.app_client_handler = handler;
}

//
// Handle an application message dispatching it to
// app_client_handler if registered.
//
// TODO: Make this an Event registration
//
App.prototype.HandleMessage = function(body, message, callback) {

    var self = this;

    var response_message = "openpux: no applications registered";

    //
    // TODO: This should be an event emitter to allow applications
    // to register for the message.
    //
    // See what the pattern in node.js is for event emitters which
    // must provide a response so the connection does not hang.
    //
    // Could have an event receiver set a flag it has been handled,
    // but event delivery could be delayed till future turns, so
    // you can't rely on return from the event emitter routine
    // as an indication of whether someone claimed it or not.
    //
    // Timeout would be awful. Since there is no order, I can't put my
    // own at the end of the chain. Besides, what keeps someone from
    // having an event handlers and deferring to its own turn.
    //
    // I don't want to write/invent my own event mechanism for
    // dispatching such requests just because I need to know
    // that someone handled it.
    //

    if (self.app_client_handler != null) {
        self.app_client_handler(body, message, callback);
        return;
    }

    self.logger.info("Twilio: unhandled message=" + message);

    callback(response_message);
}

//
// to - SMS text number in the form of "4250001234"
//
//      If null default_target_number in credentials is used
//
// from - Twilio registered number in the form of "+14250004321"
//
//      If null source_number in credentials is used
//
// message - Text message body that conforms to character standards.
//
// callback(error, sid)
//
App.prototype.SendMessage = function(to, from, message, callback) {

    var self = this;

    if (to == null) {
        to = self.credentials.default_target_number;
        if (to == null) {
            callback("No to number set", null);
            return;
        }
    }

    if (from == null) {
        from = self.credentials.source_number;
        if (from == null) {
            callback("No from number set", null);
            return;
        }
    }

    self.appmodule.SendMessage(to, from, message, function(error, sid) {

        if (error != null) {
            self.logger.error("error sending message=");
            self.logger.error(error);
            callback(error, null);
            return;
        }

        self.logger.info("message sent, sid=" + sid);

        callback(null, sid);
    });
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

  var self = this;

  //
  // This application only handles POST, so tell the appserver
  // to keep searching for a handler.
  //
  if (req.method != "POST") {
      return false;
  }

  //
  // Twilio does a POST with an SMS request
  //
  // /twilio/smartpux
  //
  if (req.url.search(self.appconfig.app_path) == 0) {

      //
      // Process Twilio request.
      //
      // DispatchRequest will respond to the HTTP request with
      // its TwiML language and close the HTTP connection.
      //
      self.appmodule.DispatchRequest(req, res);
      return true;
  }
  else {
      // Specific configured application path not found.
      return false;
  }
}

module.exports = {
  App: App
};
