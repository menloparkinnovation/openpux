
//
// TODO: support JSON contentType
//
// App.prototype.sendSensorResponseShortForm = function (req, res, sensorValues, contentType) {
//

//
// REST Root:      /smartpuxdata
//
// Storage Root:   /accounts/
//                 invokes openpuxapp.js
//
// Validation Root: /api/v2/accounts/...
//

//
//   smartpuxapp.js
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

//
// Smartpux supports supports low power, low memory, infrequently connected
// sensors with very short messages. It is targeted at AtMega 328 class
// devices with 2K RAM, 32K flash, 1K eeprom.
//
// It layers on top of the standard Smartpux REST/JSON oriented messages used
// by sensors with more memory and power.
//
// It provides a limited facility for a sensor to make periodic transactions
// with the server in which it sends its current readings, and retrieves its
// target state and settings in a simple transaction.
//
// The message encoding uses url query path encoding, and the messages are
// short enough to be sent/forwarded by the SMS text messaging system.
//
// Smartpux sensors, and sensor applications are managed through the full
// openpux application and management consoles. This module just provides
// an expedited sensor message update and settings retrieval path.
//

var appConfig = {
    name:     "smartpux",
    url_root: "/smartpuxdata/"
};

function App(config)
{
    this.appconfig = appConfig;
    this.config = config;

    this.moduleName = this.appconfig.name;

    this.trace = false;
    this.traceerrorValue = false;

    this.openpux_app = null;

    if (typeof(this.config.Trace) != "undefined") {
        this.trace = this.config.Trace;
    }

    if (typeof(this.config.TraceError) != "undefined") {
        this.traceerrorValue = this.config.TraceError;
    }

    this.utility = config.utility;

    // Setup the inherited logger
    this.logger = this.config.logger;

    // Locate the openpux app we will layer on top of
    this.openpux_app = config.appserver.getAppInstanceByName("openpux");
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

  //
  // Only POST to the data url is supported.
  //
  // More complete management of a sensor occurs through the full openpux
  // REST/JSON based API.
  //
  if (req.method != "POST") {
      return false;
  }

  // Get content type so we can response properly to the caller
  var contentType = this.getContentType(req);
  if (contentType == null) {
      this.tracelog("unknown content-type");
      this.tracelog("content-type=" + req.headers['content-type']);
      this.tracelog(req.headers);

      // This handles null content type by sending as json
      this.logSendError(contentType, req, res, 400, "unknown content-type");
      return;
  }

  if (req.url.search(this.appconfig.url_root) == 0) {

      //
      // We claim our url "/smartpuxdata/data", but reject requests
      // with extra information.
      //
      // This allows future expansion.
      //
      if (req.url != '/smartpuxdata/data') {
          this.logSendError(contentType, req, res, 400, "url with extra path not supported");
          return true;
      }

      //
      // Smartpux front-ends openpuxapp for low power sensors and token
      // based single operations.
      //
      // If the openpux_app is not loaded, return an error.
      //
      if (this.openpux_app == null) {
          this.logSendError(contentType, req, res, 500, "openpux app not configured for server");
          return true;
      }

      this.processDataRequest(req, res, app, app_url, appserver, contentType);
      return true;
  }

  // url not recognized or claimed
  return false;
}

App.prototype.getContentType = function (req) {

    var contentType = null;

    if (req.headers['content-type'] == 'application/x-www-form-urlencoded') {
        contentType = "urlencoded";
    }
    else if (req.headers['content-type'] == 'application/json') {
        contentType = "json";
    }

    return contentType;
}

//
// Process a short form sensor data request.
//
// The data URL is an simple x-www-form-urlencoded short form
// sensor update POST exchange.
//
// This allows the sensor to send its latest readings, and
// returned the sensors latest settings.
//
// It's designed for simple sensors with minimal memory.
//
// The short hand notation is expanded before placing into
// the sensor readings data store.
//
App.prototype.processDataRequest = function (req, res, app, app_url, appserver, contentType) {

    if (req.method != "POST") {
        this.tracelog("/smartpuxdata/data method not POST\n");
        this.logSendError(contentType, req, res, 400, "only POST accepted");
        return false;
    }

    //
    // read and process the input. A response is generated
    // when the input end event occurs.
    //
    // Note: All methods in which processing is handed off
    // to is responsible for a final response to the
    // request, error or not.
    //
    this.addSensorReadingShortForm(req, res, contentType);
}

//
// Support low power sensors.
//
// Here "low power" means any sensor with limited memory, computation,
// communications bandwidth, or energy. Such a sensor is typically
// mostly sleeping, wakes up periodicially, and communicates using
// short messages.
//
// Protocol:
//
// Sensors using this interface are expected to be mostly sleeping
// and periodically wakeup and send readings to the service. The service
// then replies with the current sensor settings and commands.
// 
// As part of the sensor readings data sent to the service sensor
// settings and internal state is sent as well so that the service
// has the latest sensor states.
//
// This exchange of current sensor settings and returned new sensor target
// settings allow the service to converge sensors to a target state
// as they wakeup and send readings. This protocol is designed to be
// tolerant of lost messages as a sensor will always report its current
// settings and receive its latest target settings on each exchange.
//
// The exchange is expected to occur quickly in which the sensor goes back to
// sleep after retrieving its updated settings to minimize energy usage.
//
// Format:
//
// This is the simple sensor exchange that uses a shorthand syntax as a
// x-www-form-urlencoded query string.
//
// This query string is POST'ed to send data readings to the service.
// The service responds with an x-www-form-urlencoded query string with
// the latest sensor settings data. This sensor settings data define
// operational modes, sleeptime for sensor power savings, etc.
//
// Any request attributes (data items) that are not recognized by
// the translation table are passed through as is. This allows sensors
// to provide new features with custom settings and data. A mix of short
// form syntax and longer custom forms is allowed in order to provide
// local specific optimizations. In most cases sending extended attributes
// only consumes small amounts of local code space for the string itself
// which is streamed out.
//
// Responses are strictly limited to the defined short form mask set
// responses since firmware on low memory devices is a state machine to
// process a narrow range of responses with minimal buffering.
//
// TODO: Fix default MenloFramework HTTP state machine to allow open
//       ended attributes with an unrecognized attribute function
//       call for application specific attributes.
//
// Firmware with more memory/code space can use the JSON
// interfaces to handle open ended schemas.
//
// Identification data for the sensor such as account, passcode,
// sensor id is included in the message itself. This is because
// these messages may have passed through various gateways over
// low bandwith radios (BLE, 2.4Ghz ISM, etc.), or through
// services such as SMS text messaging.
//
// A Ticket, encoded as K=xxxx can supplied account, passcode, and
// sensorid based on the backend server and the type of ticket.
//
// In cases a Ticket is not supplied, PassCode is used. If
// Account is supplied, it must match the ticket.
//
// Data Schema:
//
// Ticket is encoded as K.
//
// AccountID is encoded as A.
//
// PassCode is encoded as P.
//
// SensorID is encoded as S.
//
// SleepTime is encoded as T.
//
// Command is encoded as C.
//
// Well known Sensor data readings are known as SensorReading0 - SensorReading9.
//
// These are encoded in short hand as D0 - D9.
//
// Well known Sensor data settings are known as TargetMask0 - TargetMask9.
//
// These are encoded in short hand as M0 - M9.
//
// Sent Data:
//
// When sending sensors identify themselves in the short message along
// with their current state (Mx readings) and sensor readings (Dx readings).
//
//   A=1&P=12345678&S=1&T=30&D0=1234&D1=5678M0=0&M1=ff&M2=0&M3=0
//
// Response Data:
//
// Response data contains the new target state(s) (Mx) and sleep time (T=xx)
//
//   T=30&&M0=0&M1=ff&M2=0&M3=0
//
App.prototype.addSensorReadingShortForm = function (req, res, contentType) {

  this.tracelog("addSensorReadingShortForm invoked!");

  var self = this;

  //
  // Receive the http document. The querystring form is POST'ed
  // to the server.
  //

  // Start with empty string
  var body = '';

  // Set encoding to utf8, otherwise default is binary
  req.setEncoding('utf8');

  // request fires 'data' events with the data chunk(s)
  req.on('data', function(chunk) {
	  body += chunk;
  });

  // 'end' event indicates entire body has been delivered
  req.on('end', function() {
    self.processAddSensorReadingShortForm(req, res, contentType, body);
  });
}

// This is here to catch unbalanced parens, etc.
// TODO: Remove!
App.prototype.fooFunc = function (req, res) {

    var ar = [];
    var ob = {};

    var str = "st";
    var str2 = 'st';

    var value = ((10 * 10) * 10);
}


App.prototype.processAddSensorReadingShortForm = function (req, res, contentType, document) {

    var self = this;

    this.tracelog("processAddSensorReadingShortForm:");
    this.tracelog('document:' + document + ':\n');

    var sensorReadings = self.processRequestDocument(contentType, document);

    // The values will be dumped as JSON
    this.tracelog(sensorReadings);

    //
    // Convert readings from short form
    //
    var longFormReadings = self.convertFromShortForm(sensorReadings);

    this.tracelog("Short Form:");
    this.tracelog(sensorReadings);

    this.tracelog("Long Form:");
    this.tracelog(longFormReadings);

    var args = new Object();
    args.AccountID = longFormReadings["AccountID"];
    args.SensorID = longFormReadings["SensorID"];

    //
    // Note: Current low memory sensors expect all responses, including
    // errors to be of type "application/x-www-form-urlencoded".
    //

    var token = null;

    //
    // First attempt to get a ticket from the request headers.
    //
    // This is the preferred way to send the authentication ticket, but
    // some sensor libraries may not readily support custom headers.
    //

    token = self.config.ticketserver.getTokenFromRequestHeaders(req);

    if (token == null) {

        //
        // Attempt to get a token from the document supplied with the request.
        //

        if (longFormReadings["Ticket"] != null) {
            token = longFormReadings.Ticket;
        }
        else {

            //
            // Sensor clients that do not provide a ticket must
            // supply AccountID, SensorID, and PassCode.
            //

            if (args.AccountID == null) {
                self.logSendError(contentType, req, res, 401,
                    "no AccountID (A) supplied without ticket");
                return true;
            }

            if (args.SensorID == null) {
                self.logSendError(contentType, req, res, 401,
                    "no SensorID (S) supplied without ticket");
                return true;
            }

            if (longFormReadings.PassCode == null) {
                self.logSendError(contentType, req, res, 401,
                    "no PassCode (P) supplied without ticket");
                return true;
            }

            token = longFormReadings.PassCode;
        }
    }

    // Ticket or PassCode if present should not be stored in readings
    delete longFormReadings.Ticket;
    delete longFormReadings.PassCode;

    var url = "/api/v2/accounts/";

    //
    // Determine if the supplied token represents a valid ticket
    //
    self.config.ticketserver.getTicketById(token, function(error, ticket) {

        if (error != null) {
            // Authentication error, ticket not valid
            self.logSendError(contentType, req, res, 401,
                "authentication error ticket not valid" + error);
            return;
        }

        //
        // if AccountID or SensorID is not supplied attempt to get it from
        // the tokens URL.
        //
        // Options:
        //
        // The ticketid contains the account, the sensor id is in the document
        //
        // The ticketid contains both the account and the sensor.
        //
        // This allows sensors to be configured with a single ticket and are
        // then not required to store the AccountID, PassCode, and SensorID
        // which minimizes storage requirements and data transfers.
        //
        // In addition the ticket becomes a "serial number" for the device
        // and allows an application to change its account, sensorid and
        // other attributes.
        //
        if ((args.AccountID == null) ||
            (args.SensorID == null)) {

            // get objectpath from ticket
            var objectPath = self.config.ticketserver.GetObjectPathFromTicket(ticket);

            var requestObject = self.openpux_app.parseApiRequestString(objectPath);
            if (requestObject.error != null) {
                self.logSendError(contentType, req, res, 401,
                    "authentication error ticket does not represent the account and sensor");
                return;
            }

            if (requestObject.apiobject == "account") {

                //
                // path in token identifies an account
                //

                // If AccountID was supplied, it must match
                if ((args.AccountID != null) &&
                    (requestObject.items.AccountID != args.AccountID)) {
                        self.logSendError(contentType, req, res, 401,
                            "AccountID in ticket does not match supplied");
                        return;
                }

                //
                // SensorID must be supplied in the document
                //
                if (args.SensorID == null) {
                    self.logSendError(contentType, req, res, 401, "SensorID not supplied");
                    return;
                }

                // Get the AccountID from the ticket's version
                args.AccountID = requestObject.items.AccountID;
            }
            else if (requestObject.apiobject == "sensor") {

                //
                // path in token identifies the account and sensor.
                //
                // If either are supplied, they must match
                //

                if ((args.AccountID != null) &&
                    (requestObject.items.AccountID != args.AccountID)) {
                        self.logSendError(contentType, req, res, 401,
                            "AccountID in ticket does not match supplied");
                        return;
                }

                if ((args.SensorID != null) &&
                    (requestObject.items.SensorID != args.SensorID)) {
                        self.logSendError(contentType, req, res, 401,
                            "SensorID in ticket does not match supplied");
                        return;
                }

                // Get the AccountID and SensorID from the ticket's version
                args.AccountID = requestObject.items.AccountID;
                args.SensorID = requestObject.items.SensorID;
            }
            else {
                // path in token identifies a different object type
                self.logSendError(contentType, req, res, 401,
                    "ticket does not represent account or sensor");
                return;
            }
        }

        //
        // Full url path to validate request against is:
        //
        // /api/v2/accounts/<AccountID>/sensors/<SensorID>/readings
        //

        // Add AccountID
        url = url + args.AccountID;

        // Add SensorID
        url = url + "/sensors/" + args.SensorID;

        // Add readings
        url = url + "/readings";

        var accessRequest = "addreading";

        var accessState = {object: url, request: accessRequest};

        self.config.ticketserver.validateTicket(ticket, accessState, function(error2, accessGranted) {

            if (error2 != null) {
                // Authentication error
                self.logSendError(contentType, req, res, 401, "authentication error " + error2);
                return;
            }

            self.openpux_app.addReadings(longFormReadings, function(error3, result) {

                if (error2 != null) {
                    self.tracelog("storage update error " + error3);
                    self.logSendError(contentType, req, res, 400, "storage update error=" + error3);
                    return;
                }

                //
                // Now retrieve the settings, if any
                //
                self.openpux_app.getSensorSettings(args, function(error4, sensorValues) {

                    if (error4 != null) {
                        self.tracelog("error getting sensor settings");
                        sensorValues = null;
                    }

                    if (sensorValues != null) {
                        if (self.trace) {
                            self.logger.info("addSensorReadingShortForm: settings=");
                            self.logger.info(sensorValues);
                        }

                        self.tracelog(sensorValues);
                    }
                    else {
                        self.tracelog("sensorValues == null");
                    }

                    self.sendSensorResponseShortForm(req, res, sensorValues, contentType);
                });
            });
        });
    });

    return;
}

//
// Newer sensors have built in JSON libraries and its fast becoming
// the preference by programmers even for embedded C/C++ firmware.
//
// So support JSON or UrlEncoded format in the simple sensor exchange.
//
// Currently no need is anticipated for XML as that format is quickly
// being left behind for IoT/sensor applications.
//
// Menlo Dweet format is another that can be added in the future
// to support multiple transport message onboarding such as over
// SMS text message system.
//
App.prototype.processRequestDocument = function (contentType, document) {

    if (contentType == "urlencoded") {
        return this.processNameValueQueryString(document);
    }
    else {
        // we treat the rest as json
        return this.utility.processJSONDocument(document);
    }
}

//
// TODO: support JSON contentType
//
App.prototype.sendSensorResponseShortForm = function (req, res, sensorValues, contentType) {

  // http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_class_http_serverresponse
  // http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_writable

  // It's OK if there are no values set yet
  if (sensorValues == null) {
      this.tracelog("No sensor values to send");
      res.writeHead(201, {'Content-Type': 'application/x-www-form-urlencoded'});
      res.end();
      return;
  }

  //
  //  - This is passing through SimpleDB schema
  //    items in which these should be kept locally.
  //
  //  - Problem is really getSensorSettings() and which parts of the
  //    data schema to keep local and send remotely.
  //
  var shortFormValues = this.convertToShortForm(sensorValues);

  var responseString = this.generateQueryString(shortFormValues);

  this.tracelog("long form values:");
  this.tracelog(sensorValues);

  this.tracelog("short form values:");
  this.tracelog(shortFormValues);

  res.writeHead(201, {'Content-Type': 'application/x-www-form-urlencoded'});

  this.tracelog("responseString=" + responseString + "\n");

  res.end(responseString);

  return;
}

//
// Shortform translation table
//
var g_shortFormTable = {
    "Ticket": "K",
    "AccountID": "A",
    "PassCode": "P",
    "SensorID": "S",
    "Command": "C",
    "SleepTime": "T",
    "TargetMask0": "M0",
    "TargetMask1": "M1",
    "TargetMask2": "M2",
    "TargetMask3": "M3",
    "TargetMask4": "M4",
    "TargetMask5": "M5",
    "TargetMask6": "M6",
    "TargetMask7": "M7",
    "TargetMask8": "M8",
    "TargetMask9": "M9",
    "SensorReading0": "D0",
    "SensorReading1": "D1",
    "SensorReading2": "D2",
    "SensorReading3": "D3",
    "SensorReading4": "D4",
    "SensorReading5": "D5",
    "SensorReading6": "D6",
    "SensorReading7": "D7",
    "SensorReading8": "D8",
    "SensorReading9": "D9"
};

//
// Use a reverse table that is hashed by the runtime rather than
// searching the previous one for reverse order lookup.
//

var g_longFormTable = {
    "K": "Ticket",
    "A": "AccountID",
    "P": "PassCode",
    "S": "SensorID",
    "C": "Command",
    "T": "SleepTime",
    "M0": "TargetMask0",
    "M1": "TargetMask1",
    "M2": "TargetMask2",
    "M3": "TargetMask3",
    "M4": "TargetMask4",
    "M5": "TargetMask5",
    "M6": "TargetMask6",
    "M7": "TargetMask7",
    "M8": "TargetMask8",
    "M9": "TargetMask9",
    "D0": "SensorReading0",
    "D1": "SensorReading1",
    "D2": "SensorReading2",
    "D3": "SensorReading3",
    "D4": "SensorReading4",
    "D5": "SensorReading5",
    "D6": "SensorReading6",
    "D7": "SensorReading7",
    "D8": "SensorReading8",
    "D9": "SensorReading9"
};

//
// Simple sensor exchange uses limited number of short form values.
//
App.prototype.convertToShortForm = function (settings) {
    var o = new Object();
    for (var propName in settings) {
        var newPropName = g_shortFormTable[propName];
        if (newPropName == null) {
            // Passthrough unknown properties without translations
            newPropName = propName;
        }

        o[newPropName] = settings[propName];
    }

    return o;
}

//
// Short form is used by the minimal sensor readings POST function
// as 'application/x-www-form-urlencoded'.
//
// We convert to the full names that we actually store in the
// data store and return/manipulate for applications.
//
App.prototype.convertFromShortForm = function (settings) {
    var o = new Object();
    for (var propName in settings) {
        var newPropName = g_longFormTable[propName];
        if (newPropName == null) {
            // Passthrough unknown properties without translations
            newPropName = propName;
        }

        o[newPropName] = settings[propName];
    }

    return o;
}

//
// Encode the data portion of a querystring.
//
App.prototype.generateQueryString = function (values) {

    var queryString = '';
    var index = 0;

    for (var prop in values) {
        if (index > 0) queryString += '&';

        queryString += prop;
        queryString += '=';
        queryString += values[prop];
        index++;
    }
    
    return queryString;
}

//
// Input:
//
//   M0=0&M1=0001&M2=0002&M3=0003"
//
// Output:
//
//   array["M0"] = "0"
//   array["M1"] = "0001"
//        ...
//
App.prototype.processNameValueQueryString = function (queryString) {

    //
    // Input:
    //
    //   M0=0&M1=0001&M2=0002&M3=0003"
    //
    // Output:
    //
    //   array[0] = "M0=0"
    //   array[1] = "M1=0001"
    //        ...
    //
    try {
        var parms = queryString.split('&');

        var nameValueArray = new Array();

        for (var i = 0; i < parms.length; i++) {

            var str = parms[i];

            var position = str.indexOf('=');

            nameValueArray[str.substring(0, position)] = str.substring(position + 1);

            //this.tracelog(i + ': ' + parms[i]);
        }

        return nameValueArray;

    } catch(e) {
        this.tracelog("processNameValueQueryString exception e=" + e.toString() + " querystring=" + queryString);
        return null;
    }
}

//
// The message exchange with low power sensors is as url encoded messages.
//
App.prototype.logSendError = function (contentType, req, res, errorCode, errorMessage) {

    if (contentType == "json") {
        this.utility.logSendErrorAsJSON(this.logger, req, res, errorCode, errorMessage);
    }
    else if (contentType == "urlencoded") {
        this.logSendErrorAsUrlEncoded(this.logger, req, res, errorCode, errorMessage);
    }
    else {
        // Unknown types default to json
        this.utility.logSendErrorAsJSON(this.logger, req, res, errorCode, errorMessage);
    }
}

App.prototype.logSendErrorAsUrlEncoded = function (logger, req, res, errorCode, errorMessage) {

    var remoteAddr = req.socket.remoteAddress;
    logger.error(remoteAddr + " error=" + errorCode + " errorMessage=" + errorMessage);

    var queryString = "status=" + errorCode + "&errorMessage=" + errorMessage;

    res.writeHead(errorCode, {'Content-Type': 'application/x-www-form-urlencoded'});
    res.write(queryString, 'utf8');
    res.end();
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

module.exports = {
  App: App
};
