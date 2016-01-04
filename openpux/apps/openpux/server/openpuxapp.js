
//
// REST Root:      /api/v1/accounts
//
// Storage Root:   /accounts/
//
// see g_appConfig
//

//
//   openpuxapp.js
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
// This contains the business logic of the Openpux IoT sensor
// application.
//
// It utilizes the general purpose cached storage services from
// storage.js.
//

//
// Authentication Rules
//
// All authentication occurs at the top level validated before the
// operation procedes using the Ticket/Token system.
//
// Tickets/Token's are in the Authentication header.
//
// No passcodes are allowed in querystring except for some special cases of
// GET to allow browser access to sensor settings and readings.
//
// No data access routines or their workers are involved in authentication.
// Once a data access occurs, the request has been validated and is allowed.
// This is so passcodes and ticketid's don't get placed into the sensor data
// storage, but as managed separately in a a unique path or data store.
//
//

//
// Smartpux Schema Model and Enforcement
//
// Openpux allows general open ended creation of objects without
// any parent/child requirements. This means "orphan" objects can
// be created without their parents, such as sensor readings with
// no sensor object in the data store.
//
//   - This could be accepted as technically correct just returning
//     empty property sets for ItemDoesNotExist.
//
// Account must exist to create a sensor.
//
// Sensor must exist to create settings, or add readings.
//
// As a result no object exists without its parent object.
//
// Schema enforcement is done at object create. Update, query, delete
// operations attempt against the object name path, and will fail if it
// does not exist.
//
// This saves having to do extra queries and caching for update, query, delete.
//
// Delete is not implemented yet, and will have to walk/delete child objects
// to keep the schema consistent.
//

var g_appConfig = {
    name:     "openpux",
    url_root: "/api/v2/",
    storage_root: "/accounts"
};

var url = require('url');

var querystring = require('querystring');

function OpenpuxApp(config)
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
OpenpuxApp.prototype.processAppRequest = function (req, res, app, app_url, appserver) {

    var self = this;

    var error = null;

    var errormsg = null;

    if (req.url.search(this.appconfig.url_root) != 0) {
        // Not our root
        return false;
    }

    self.tracelog("processOpenpuxApiRequest: req.headers[]=");
    self.tracelog(req.headers);

    //
    // Validate the access.
    //
    // If the request is allowed, the ticket is returned as the response
    // object to allow further application level validation.
    //
    self.config.ticketserver.validateAccessForRequest(req, req.method, function(error, ticket) {

        if (error != null) {
            errormsg = "authentication error " + error + " for " + req.method;
            self.utility.logSendErrorAsJSON(self.logger, req, res, 401, errormsg);
            return;
        }

        self.processAppRequest2(req, res, app, app_url, appserver, ticket);
    });

    // return true to accept the URL
    return true;
}

OpenpuxApp.prototype.processAppRequest2 = function (req, res, app, app_url, appserver, ticket) {

    var self = this;

    var error = null;

    try {

       // We rely on the infrastructure to limit the maximum lenght of req.url
       var requestObject = self.parseApiRequestStringFromRequest(req, res);

       self.tracelog("requestObject=");
       self.tracelog(requestObject);

       if (requestObject.error != null) {

            // Pass the default request to the REST service provider with our configuration
            self.config.restserver.processObjectRequest(this.appconfig, req, res);

            //self.traceerror("processOpenpuxApiRequest: status=" + requestObject.status + 
            //                " error=" + requestObject.error);
            //self.utility.logSendErrorAsJSON(self.logger, req, res, requestObject.status, requestObject.error);
            return true;
       }

       if (requestObject.apiobject == "account") {
           self.processAccountRequest(req, res, requestObject, ticket);
           return true;
       }
       else if (requestObject.apiobject == "sensor") {
           self.processSensorRequest(req, res, requestObject, ticket);
           return true;
       }
       else {

           // Pass the default request to the REST service provider with our configuration
           self.config.restserver.processObjectRequest(this.appconfig, req, res);

           //error = "unrecognized api object specified";
           //self.traceerror("processOpenpuxApiRequest 400 " + error);
           //self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
           return true;

       }

    } catch(e) {
        self.tracelog("exception dispatching api request e=" + e.toString());
        self.tracelog("e.stack=" + e.stack.toString());
        error = "Bad Request";
        self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
        return true;
    }
}

//
// Process a request to operate on an account.
//
// URL parameters and querystring have been decoded into
// request..
//
OpenpuxApp.prototype.processAccountRequest = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;
    var command = null;

    var resultObject = {};
    resultObject.status = 200;
    resultObject.error = null;

    if (items.Command != null) {
        command = items.Command;
    }

    if (req.method == "GET") {

        if (command == null) {
            error = "no command specified on account";
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
        }
        else if (command == "properties") {
            // /api/v1/accounts/<AccountID>/properties?passcode=xxx
            self.queryAccountProperties(req, res, request, ticket);
        }
        else {
            resultObject.status = 400;
            resultObject.error = "unsupported account command=" + command;
            self.utility.sendResultAsJSON(req, res, resultObject);
        }
    }
    else if (req.method == "POST") {

        if (command == null) {
            error = "no command specified on account";
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
        }
        else if (command == "addaccount") {
            // /api/v1/accounts/<AccountID>/addaccount
            self.addAccount(req, res, request, ticket);
        }
        else if (command == "addsensor") {
            // /api/v1/accounts/<AccountID>/addsensor
            self.addSensor(req, res, request, ticket);
        }
        else if (command == "properties") {
            // /api/v1/accounts/<AccountID>/properties
            self.updateAccountProperties(req, res, request, ticket);
        }
        else {
            resultObject.status = 400;
            resultObject.error = "unsupported account command=" + command;
            self.utility.sendResultAsJSON(req, res, resultObject);
        }
    }
    else {
        resultObject.status = 400;
        resultObject.error = "unsupported method specified";
        self.utility.sendResultAsJSON(req, res, resultObject);
        return;
    }
}

//
// Process a request to operate on a sensor.
//
// URL parameters and querystring have been decoded into request.
//
OpenpuxApp.prototype.processSensorRequest = function(req, res, request, ticket) {

    var self = this;

    var error = null;
    var command = null;

    var items = request.items;
    if (items.Command != null) {
        command = items.Command;
    }

    //
    // If a handler is found, its responsible for eventually
    // replying on the HTTP res object on success or failure.
    //
    // This routine only generates a response when it determines
    // the request can no longer be processed.
    //
    if (req.method == "GET") {

        //
        // 200 OK
        // 404 Not Found
        // 400 Bad Request
        //

        if (command == null) {
            error = "no command specified on sensor";
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
        }
        else if (command == "properties") {
            // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/properties?passcode=xxx
            self.querySensorProperties(req, res, request, ticket);
        }
        else if (command == "settings") {
            // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/settings?passcode=xxx
            self.querySensorSettings(req, res, request, ticket);
        }
        else if (command == "readings") {
            // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/readings?passcode=xxx
            self.querySensorReadings(req, res, request, ticket);
        }
        else {
            error = "GET: unsupported sensor command=" + command;
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
        }
    }
    else if (req.method == "POST") {

        //
        // 201 Created
        // 404 Not Found
        // 409 Conflict
        //
        // "Location" header with URI on success, esp. if name generated by server
        //
        // Content body can contain client selected name
        //
        // POST to parent directory creates child entry
        //
        if (command == null) {
            error = "no command specified on sensor";
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
        }
        else if (command == "properties") {
            // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/properties
            self.updateSensorProperties(req, res, request, ticket);
        }
        else if (command == "settings") {
            // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/settings
            self.createSensorSettings(req, res, request, ticket);
        }
        else if (command == "readings") {
            // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/readings
            self.addSensorReading(req, res, request, ticket);
        }
        else {
            error = "POST: unsupported sensor command=" + command;
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
        }
    }
    else if (req.method == "PUT") {
        // 200 OK
        // 204 No Content
        // 404 Not Found
        if (command == "settings") {
            // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/settings
            self.updateSensorSettings(req, res, request, ticket);
        }
        else {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", "PUT not supported");
        }
    }
    else if (req.method == "DELETE") {
        // 200 OK
        // 404 Not Found
        self.utility.logSendErrorAsJSON(self.logger, req, res, "400", "DELETE not supported");
    }
    else {
        error = "unsupported method specified";
        self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
    }
}

OpenpuxApp.prototype.addAccount = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;

    // TODO: Add ticket validation for administrator account!

    // Receive the request JSON document as a parsed object
    self.utility.receiveJSONObject(req, res, self.logger, function(error, data) {

        if (error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
            return;
        }

        if ((data.NewAccountID == null) || (data.NewAccountID == "")) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", "NewAccountID not specified");
            return;
        }

        if ((data.NewAccountTicketID == null) || (data.NewAccountTicketID == "")) {
            self.tracelog("400 Failed: NewAccountTicketID not specified");
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", "NewAccountTicketID not specified");
            return;
        }

        //
        // Can't specify the administrator accountid
        //
        if (data.NewAccountID == g_config.AdministratorAccountID) {
            self.tracelog("400 Failed: Attempt to create account as non-Administrator");
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", "AccountID not available");
            return;
        }
    
        var reqItems = {};
        reqItems.AccountID = data.NewAccountID;

        self.addAccountWorker(reqItems, function(error, result) {

            var returnBlock = {status: null, error: null};

            if (error == null) {
                returnBlock.status = 200;
            }
            else {
                self.tracelog("error adding account to storage error=" + error);
                returnBlock.status = 404;
                returnBlock.error = error;
            }

            self.utility.sendResultAsJSON(req, res, returnBlock);
        });
    });
}

//
// Add a sensor to an account.
//
// If config.AllowCreateSensorByAccount is set add the sensor
// to the specified account in the authorization items.
//
// If authorization items refer to a valid administrator account, allow
// sensors to be added to a specified account.
//
//  Authorization/Request URL items:
//
//  data from received JSON document:
//
//  data["NewSensorID"] - New SensorID
//  data["NewSensorAccountID"] - Optional. Account to create new sensor under
//
OpenpuxApp.prototype.addSensor = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;

    // TODO: Add ticket validation!

    // Receive the request JSON document as a parsed object
    self.utility.receiveJSONObject(req, res, self.logger, function(error, data) {

        if (error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
            return;
        }

        if ((data.NewSensorID == null) || (data.NewSensorID == "")) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", "NewSensorID not specified");
            return;
        }
    
        self.addSensorWorker(data, function(error, result) {

            var returnBlock = {status: null, error: null};

            if (error == null) {
                returnBlock.status = 200;
            }
            else {
                returnBlock.status = 404;
                returnBlock.error = error;
            }

            self.utility.sendResultAsJSON(req, res, returnBlock);
        });
    });
}

OpenpuxApp.prototype.queryAccountProperties = function(req, res, request, ticket) {
    this.utility.logSendErrorAsJSON(this.logger, req, res, "400", "not implemented");
}

OpenpuxApp.prototype.updateAccountProperties = function(req, res, request, ticket) {
    this.utility.logSendErrorAsJSON(this.logger, req, res, "400", "not implemented");
}

//
// Sensor Properties are items not sent in regular exchanges
// with the sensor. Examples are location, detailed description,
// calibration settings, etc.
//
OpenpuxApp.prototype.querySensorProperties = function(req, res, request, ticket) {
    this.utility.logSendErrorAsJSON(this.logger, req, res, "400", "not implemented");
}

OpenpuxApp.prototype.updateSensorProperties = function(req, res, request, ticket) {
    this.utility.logSendErrorAsJSON(this.logger, req, res, "400", "not implemented");
}

//
// Sensor Settings are items sent in regular exchanges with
// the sensor. These represent operating modes, commands, etc.
//
// request contains the sensor and account being addressed
//
OpenpuxApp.prototype.querySensorSettings = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;

    // TODO: Add ticket validation!

    var args = new Object();
    args.AccountID = items["AccountID"];
    args.SensorID = items["SensorID"];

    self.getSensorSettings(args, function(error, data) {

        var returnBlock = {status: null, error: null, sensorsettings: null};

        if (error == null) {
            returnBlock.status = 200;
        }
        else {
            returnBlock.status = 404;
            returnBlock.error = error;
        }

        if (data != null) {
    	    returnBlock.sensorsettings = data;

            if (self.trace) {
                self.logger.info("querySensorSettings: settings=");
                self.logger.info(data);
            }
        }

        self.utility.sendResultAsJSON(req, res, returnBlock);
    });

    return;
}

//
// Create Sensor Settings
//
// JSON document contains the sensor settings attributes.
//
// request contains the sensor and account being addressed
//
OpenpuxApp.prototype.createSensorSettings = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;

    // TODO: Add ticket validation!

    // Receive the request JSON document as a parsed object
    self.utility.receiveJSONObject(req, res, self.logger, function(error, data) {

        if (error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
            return;
        }

        // New entry is identified with AccountID and SensorID
        var o = new Object();
        o["AccountID"] = items.AccountID;
        o["SensorID"] = items.SensorID;

        // data contains the name:value properties being posted
        for(var prop in data) {
            o[prop] = data[prop];
        }

        //
        // Data Object:
        //
        //  o["AccountID"] = "99999"
        //  o["SensorID"] = "99999"
        //  o["SleepTime"] = "30"
        //  o["TargetMask0"] = "0"
        //  o["TargetMask1"] = "1"
        //        ...
        //
        self.createSensorSettingsWorker(o, function(error, result) {

            var returnBlock = {status: null, error: null};

            if (error == null) {
                returnBlock.status = 200;
            }
            else {
                returnBlock.status = 404;
                returnBlock.error = error;
            }

            self.utility.sendResultAsJSON(req, res, returnBlock);
        });
    });
}

//
// Update Sensor Settings
//
// JSON document contains the sensor settings attributes.
//
// request contains the sensor and account being addressed
//
OpenpuxApp.prototype.updateSensorSettings = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;

    // TODO: Add ticket validation!

    // Receive the request JSON document as a parsed object
    self.utility.receiveJSONObject(req, res, self.logger, function(error, data) {

        if (error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
            return;
        }

        // New entry is identified with AccountID and SensorID
        var o = new Object();
        o["AccountID"] = items.AccountID;
        o["SensorID"] = items.SensorID;

        // data contains the name:value properties being posted
        for(var prop in data) {
            o[prop] = data[prop];
        }

        //
        // Data Object:
        //
        //  o["AccountID"] = "99999"
        //  o["SensorID"] = "99999"
        //  o["SleepTime"] = "30"
        //  o["TargetMask0"] = "0"
        //  o["TargetMask1"] = "1"
        //        ...
        //
        self.updateSensorSettingsWorker(o, function(error, result) {

            var returnBlock = {status: null, error: null};

            if (error == null) {
                returnBlock.status = 200;
            }
            else {
                self.tracelog("updateSensorSettings: error=" + error);
                returnBlock.status = 404;
                returnBlock.error = error;
            }

            self.utility.sendResultAsJSON(req, res, returnBlock);
        });
    });
}

OpenpuxApp.prototype.querySensorReadings = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;

    // TODO: Add ticket validation!

    var args = new Object();
    args.AccountID = items["AccountID"];
    args.SensorID = items["SensorID"];

    var latestCount = request.query["latestcount"];
    if ((typeof(latestCount) == "undefined") || (latestCount == null)) {
        latestCount = 1;
    }

    //
    // Get data from the data store.
    //
    self.queryLastReadings(args, latestCount, function(error, data) {

        //
        // Our result is an object in JSON format with:
        //
        // status: "200 OK" or error string
        // items: [{itemName: "name", item: obj}, ...]
        //
        var returnBlock = {status: null, error: null, items: null};

        if (error == null) {
	    returnBlock.status = 200;
        }
        else {
            returnBlock.status = 404;
            returnBlock.error = error;
        }

        if (data != null) {
    	    returnBlock.items = data;
        }

        self.utility.sendResultAsJSON(req, res, returnBlock);
    });

    //
    // Request has been successfully submitted, the previous lambda block
    // will handle final request disposition.
    //

    return;
}

//
// openpux application logic and schema:
//
// The rest of the routines just perform basic schema
// transformations from the Javascript objects representing the openpux
// data items and the backing store model.
//

// /accounts/xxx
OpenpuxApp.prototype.generateAccountName = function(accountid) {
    if (accountid == null) return null;
    var itemName = this.appconfig.storage_root + "/" + accountid;
    return itemName;
}

// /accounts/xxx/sensors/xxx
OpenpuxApp.prototype.generateSensorName = function(accountid, sensorid) {

    if (sensorid == null) return null;

    var itemName = this.generateAccountName(accountid);
    if (itemName == null) return null;    

    itemName = itemName + "/sensors/" + sensorid;
    return itemName;
}

// /accounts/xxx/sensors/xxx/settings
OpenpuxApp.prototype.generateSensorSettingsName = function(accountid, sensorid) {

    if (sensorid == null) return null;

    var itemName = this.generateSensorName(accountid, sensorid);
    if (itemName == null) return null;

    itemName = itemName + "/settings";
    return itemName;
}

// /accounts/xxx/sensors/xxx/readings/zzzz
OpenpuxApp.prototype.generateSensorReadingName = function(accountid, sensorid, timestamp) {

    if (timestamp == null) return null;

    var itemName = this.generateSensorName(accountid, sensorid);
    if (itemName == null) return null;

    itemName = itemName + "/readings/" + timestamp;
    return itemName;
}

//
// Create entry for account
//
// AccountID being created:
//   itemsArray['AccountID'] == new Account
//
// callback(error, accountEntry)
//
OpenpuxApp.prototype.addAccountWorker = function(itemsArray, callback) {

    var self = this;

    // Account to be created
    if (itemsArray.AccountID == null) {
        self.traceerror("missing AccountID");
        callback("missing AccountID", null);
        return;
    }

    //
    // Can't specify the administrator accountid for the new account.
    //
    if (itemsArray.AccountID == g_config.AdministratorAccountID) {
        callback("AccountID not available", null);
        return;
    }

    var itemName = self.generateAccountName(itemsArray.AccountID);

    self.storage.createItem(itemName, itemsArray, callback);
}

//
// Get Account entry
//
// args['AccountID'] == Account
//
// callback(error, accountEntry)
//
OpenpuxApp.prototype.getAccount = function(args, callback) {

    var self = this;
    
    var itemName = self.generateAccountName(args.AccountID);
    if (itemName == null) {
        self.traceerror("missing AccountID");
        callback("missing AccountID", null);
        return;
    }

    self.storage.getItem(itemName, function(error, accountEntry) {

        // error == "ItemDoesNotExist" if no account configured by that name
        if (error != null) {
            callback(error, accountEntry);
            return;
        }

        callback(null, accountEntry);
    });
}

//
// Create entry for sensor
//
//  itemsArrray['NewSensorID'] == SensorID being created
//  itemsArray['NewSensorAccountID'] == Account to create sensor under
//
// callback(error, sensorEntry)
//
OpenpuxApp.prototype.addSensorWorker = function(itemsArray, callback) {

    var self = this;

    if (itemsArray.NewSensorID == null) {
        self.traceerror("missing NewSensorID");
        callback("missing NewSensorID", null);
        return;
    }

    if (itemsArray.NewSensorAccountID == null) {
        self.traceerror("missing NewSensorAccountID");
        callback("missing NewSensorAccountID", null);
        return;
    }

    //console.log("addSensorWorker: itemsArray");
    //self.utility.dumpasjsonToConsole(itemsArray);

    var args = {};

    args.SensorID = itemsArray.NewSensorID;
    args.AccountID = itemsArray.NewSensorAccountID;

    var accountName = self.generateAccountName(args.AccountID);

    //
    // Smartpux Schema requires the account to exist before a sensor can be created.
    //
    self.storage.getItem(accountName, function(error, accountEntry) {

        if (error != null) {
            self.tracelog("addSensor: could not get account " + accountName + " error=" + error);
            callback(error, accountEntry);
            return;
        }

        var itemName = self.generateSensorName(args.AccountID, args.SensorID);

        // Generate the new entry
        var sensorEntry = new Object();
        sensorEntry.AccountID = args.AccountID;
        sensorEntry.SensorID = args.SensorID;

        self.storage.createItem(itemName, sensorEntry, callback);
    });
}

//
// Get Sensor Entry
//
// args['AccountID'] == Account
// args['SensorID'] == Sensor
//
// callback(error, sensorEntry)
//
OpenpuxApp.prototype.getSensor = function(args, callback) {

    var self = this;

    var sensorName = self.generateSensorName(args.AccountID, args.SensorID);
    if (sensorName == null) {
        self.traceerror("missing AccountID or SensorID");
        callback("missing AccountID or SensorID", null);
        return;
    }

    self.storage.getItem(sensorName, function(error, sensorEntry) {

        // error == "ItemDoesNotExist" if missing
        if (error != null) {
            callback(error, sensorEntry);
            return;
        }

        callback(error, sensorEntry);
    });
}

//
// args['AccountID'] == Account
// args['SensorID'] == Sensor
//
// The callback function is provided:
//
// callback(error, sensorSettings);
//
OpenpuxApp.prototype.getSensorSettings = function(args, callback) {

    var self = this;

    var itemName = self.generateSensorSettingsName(args.AccountID, args.SensorID);
    if (itemName == null) {
        self.traceerror("missing AccountID or SensorID");
        callback("missing AccountID or SensorID", null);
        return;
    }

    self.storage.getItem(itemName, callback);
}

//
// Create sensor settings.
//
// itemsArray['AccountID'] == Account
// itemsArray['SensorID'] == Sensor
//
// The callback function is provided:
//
// callback(error, result)
//
OpenpuxApp.prototype.createSensorSettingsWorker = function(itemsArray, callback) {

    var self = this;

    if (itemsArray.AccountID == null) {
        self.traceerror("missing AccountID");
        callback("missing AccountID", null);
        return;
    }

    if (itemsArray.SensorID == null) {
        self.traceerror("missing SensorID");
        callback("missing SensorID", null);
        return;
    }

    //
    // Smartpux Schema requires the sensor to exist before settings can be added.
    //
    self.getSensor(itemsArray, function(error, sensorEntry) {

        if (error != null) {
            callback("ItemDoesNotExist SensorID does not exist", null);
            return;
        }

        var itemName = self.generateSensorSettingsName(itemsArray.AccountID, itemsArray.SensorID);

        self.storage.createItem(itemName, itemsArray, callback);
   });
}

//
// Update sensor settings.
//
// If sensor settings do not exist, they are created.
//
// itemsArray['AccountID'] == Account
// itemsArray['SensorID'] == Sensor
//
// The callback function is provided:
//
// callback(error, result)
//
OpenpuxApp.prototype.updateSensorSettingsWorker = function(itemsArray, callback) {

    var self = this;

    var itemName = self.generateSensorSettingsName(itemsArray.AccountID, itemsArray.SensorID);
    if (itemName == null) {
        self.traceerror("missing AccountID or SensorID");
        callback("missing AccountID or SensorID", null);
        return;
    }

    self.storage.updateItem(itemName, itemsArray, function(error, result) {

        if (error == "ItemDoesNotExist") {

            // First time update, create the entry
            self.storage.createItem(itemName, itemsArray, callback);
            return;
        }

        callback(error, result);
    });
}

//
// itemsArray['AccountID'] == Account
// itemsArray['SensorID'] == Sensor
//
// The callback function is provided:
//
// callback(error, data)
//
OpenpuxApp.prototype.addReadings = function(itemsArray, callback) {

    var self = this;

    if (itemsArray.AccountID == null) {
        self.traceerror("missing AccountID");
        callback("missing AccountID", null);
        return;
    }

    if (itemsArray.SensorID == null) {
        self.traceerror("missing SensorID");
        callback("missing SensorID", null);
        return;
    }

    //
    // Reading TimeStamp may be supplied by a remote sensor subject to delays
    // so if one is supplied it is used instead of the current time.
    //
    if ((typeof(itemsArray.TimeStamp) == "undefined") || 
        (itemsArray.TimeStamp == null) ||
        (itemsArray.TimeStamp == "")) {
        // Add the reading insertation time

        // Date format is ISO 8601 which sorts in lexographical order
        itemsArray.TimeStamp = new Date().toISOString();
    }

    //
    // Smartpux Schema requires the sensor to exist before readings can be added.
    //
    self.getSensor(itemsArray, function(error, sensorEntry) {

        if (error != null) {
            callback("ItemDoesNotExist SensorID does not exist", null);
            return;
        }

        var itemName = self.generateSensorReadingName(
            itemsArray.AccountID, itemsArray.SensorID, itemsArray.TimeStamp);

        if (itemName == null) {
            callback("missing TimeStamp for readings", null);
            return;
        }

        self.storage.createItem(itemName, itemsArray, callback);
    });
}

//
// Query the most recent readings.
//
// args['AccountID'] == Account
// args['SensorID'] == Sensor
//
// The callback function is provided:
//
// callback(error, result)
//
// Returns:
//
// Array of objects. object properties are the set of readings.
//
OpenpuxApp.prototype.queryLastReadings = function(args, readingCount, callback) {

    var self = this;

    var sensorName = self.generateSensorName(args.AccountID, args.SensorID);
    if (sensorName == null) {
        self.traceerror("missing AccountID or SensorID");
        callback("missing AccountID or SensorID", null);
        return;
    }

    var queryString = self.storage.buildSelectLastReadingsStatement(
        sensorName,
        "",  // TimeStamp
        readingCount
        );

    var consistentRead = true;

    self.storage.queryItems(queryString, consistentRead, callback);

    self.getSensor(args, function(error, sensorEntry) {

        if (error != null) {
            self.tracelog("queryLastReadings: getSensor error=" + error);
            callback(error, sensorEntry);
            return;
        }

    });
}

OpenpuxApp.prototype.parseApiRequestStringFromRequest = function (req, res) {
    return this.parseApiRequestString(req.url);
}

//
// This is the main api rest request parser.
//
// It's responsible for determining which REST api object is being
// acted on and its identifiers. In addition it extracts querystring
// parameters if present.
//
// The returned object can be used by api dispatch/api handling code
// without worrying about the specific concerns of URL and querystring
// handling, isolating them from changes in the protocol.
//
// In addition this module is concerned with ensuring improperly
// formatted api requests do not make it to the main application logic.
//
// callback(error, result)
//
// Protocol details are in docs/rest_protocol.txt
//
// url forms:
//
// /api/<version>/accounts/<AccountID>
// /api/<version>/accounts/<AccountID>/addaccount?passcode=xxx
// /api/<version>/accounts/<AccountID>/addsensor?passcode=xxx
// /api/<version>/accounts/<AccountID>/sensors/<SensorID>/settings?passcode=xxx
// /api/<version>/accounts/<AccountID>/sensors/<SensorID>/readings?passcode=xxx
//
// Returns:
//
//  Object with details about the parsed request.
//
//  {
//    status: "200 OK",
//    error:  null,
//    apiobject: "account",
//    querypasscode: "passcode",
//
//    items: {
//        Command: "command",
//        AccountID: "1",
//        SensorID: "1"
//    }
//
//    query: {
//        queryItem1: "item1",
//        queryItem2: "item2"
//    },
//  }
//
// apiobject - contains object addressed by the REST url
//
// querypasscode - contains optional passcode supplied in a querystring
//                 ?passcode=12345678
//
//                 Optional querypasscode allows browser interaction, since
//                 typical authentication uses either HTTP headers
//                 or in the posting JSON document.
//
// items contains information parsed form the REST object path.
//
// query entries contains any values from the querystring
//
OpenpuxApp.prototype.parseApiRequestString = function (urlPath) {

    var self = this;

    var obj = {};
    obj.status = 200;
    obj.error = null;
    obj.apiobject = null;
    obj.ticket = null;
    obj.items = null;
    obj.query = null;

    try {
        var items = {};

        var parsedUrl = url.parse(urlPath, true);

        self.tracelog("parsedUrl=");
        self.tracelog(parsedUrl);

        var tokens = parsedUrl.pathname.substring(1, urlPath.length).split("/");
        if (tokens == null) {
            obj.status = 400;
            obj.error = "Error parsing request url";
            obj.error += " url=:" + urlPath + ":";
            self.tracelog(obj.error);
            return obj;
        }

        if (tokens.length < 4) {
            obj.status = 400;
            obj.error = "url missing components length=" + tokens.length;
            obj.error += " url=:" + urlPath + ":";
            self.tracelog(obj.error);
            return obj;
        }

        //
        // Set query items if search path is not null
        // Otherwise its an empty object {}
        //
        if ((parsedUrl.search != null) && (parsedUrl.search != "")) {
    
            //
            // Validate that all query items are string
            // This prevents duplicate properties from creating unexpected arrays, etc.
            //
            for (var prop in parsedUrl.query) {
    
                if (typeof(parsedUrl.query[prop]) != "string") {
                    obj.error = "querystring may only contain simple single instance items";
                    self.tracelog(obj.error);
                    return obj;
                }
    
                // Save passcode if present as override ?passcode=12345678
                if (prop.toLowerCase() == "passcode") {
                    obj.querypasscode = parsedUrl.query[prop];
                }
            }
    
            obj.query = parsedUrl.query;
        }

        //
        // /api/<version>/accounts/<AccountID>
        //
        if (tokens[2].toLowerCase() != 'accounts') {
            obj.status = 400;
            obj.error = "missing accounts";
            obj.error += " url=:" + urlPath + ":";
            self.tracelog(obj.error);
            return obj;
        }

        items["AccountID"] = tokens[3];

        if (tokens.length == 4) {
            obj.status = 200;
            obj.apiobject = "account";
            obj.items = items;
            obj.query = parsedUrl.query;
            return obj;
        }

        //
        // /api/<version>/accounts/<AccountID>/command
        //
        // /api/<version>/accounts/<AccountID>/properties
        //
        // /api/<version>/accounts/<AccountID>/settings
        //
        // /api/<version>/accounts/<AccountID>/privatesettings
        //
        if (tokens.length == 5) {

            // Sensors without an id
            if (tokens[4].toLowerCase() == 'sensors') {
                obj.status = 400;
                obj.error = "missing sensorid";
                obj.error += " url=:" + urlPath + ":";
                self.tracelog(obj.error);
                return obj;
            }

            // Command on account
            items["Command"] = tokens[4];
            obj.status = 200;
            obj.apiobject = "account";
            obj.items = items;
            obj.query = parsedUrl.query;
            obj.error = null;
            return obj;
        }

        //
        // /api/<version>/accounts/<AccountID>/sensors/<SensorID>
        //
        if (tokens.length == 6) {

            if (tokens[4].toLowerCase() != 'sensors') {
                obj.error = "missing sensors";
                obj.error += " url=:" + urlPath + ":";
                self.tracelog(obj.error);
                return obj;
            }

            items["SensorID"] = tokens[5];
            obj.status = 200;
            obj.apiobject = "sensor";
            obj.items = items;
            obj.query = parsedUrl.query;
            obj.error = null;
            return obj;
        }

        //
        // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/command
        //
        // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/readings/time
        //
        // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/readings?latestcount=1
        //
        // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/settings
        //
        // /api/<version>/accounts/<AccountID>/sensors/<SensorID>/privatesettings
        //
        if (tokens.length == 7) {

            // Command on sensor
            items["SensorID"] = tokens[5];
            items["Command"] = tokens[6];

            obj.status = 200;
            obj.apiobject = "sensor";
            obj.items = items;
            obj.query = parsedUrl.query;
            obj.error = null;
            return obj;
        }

        obj.status = 400;
        obj.error = "bad url";
        obj.apiobject = null;
        obj.items = null;
        obj.query = null;

        return obj;

    } catch(e) {
        obj.status = 500;
        obj.error = "bad url"; // don't supply exception data to remote

        // log exception and url data locally
        var exceptionInfo = e.toString() + " " + e.stack.toString();

        self.logger.error(obj.error + " exception: " + exceptionInfo + " url=" + urlPath);
        return obj;
    }
}

//
// Add a sensor reading.
//
// Unlike addSensorReadingShortForm it does not respond with
// the latest sensor settings.
//
OpenpuxApp.prototype.addSensorReading = function(req, res, request, ticket) {

    var self = this;

    var items = request.items;

    // TODO: Add ticket validation!

    // Receive the request JSON document as a parsed object
    self.utility.receiveJSONObject(req, res, self.logger, function(error, data) {

        if (error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, "400", error);
            return;
        }

        // New entry is identified with AccountID and SensorID
        var o = new Object();
        o["AccountID"] = items.AccountID;
        o["SensorID"] = items.SensorID;

        // data contains the name:value properties being posted
        for(var prop in data) {
            o[prop] = data[prop];
        }

        //
        // addReadings() will ensure that the sensor exits.
        //
        self.addReadings(o, function(error, result) {

            var returnBlock = {status: null, error: null};

            if (error == null) {
                returnBlock.status = 201;
            }
            else {
                returnBlock.status = 404;
                returnBlock.error = error;
            }

            self.utility.sendResultAsJSON(req, res, returnBlock);
        });
    });
}

OpenpuxApp.prototype.setTrace = function(value) {
    this.trace = value;
}

OpenpuxApp.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

OpenpuxApp.prototype.tracelog = function(message) {
    if (this.trace) {
        this.logger.info(this.moduleName + ": " + message);
    }
}

OpenpuxApp.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        this.logger.error(this.moduleName + ": " + message);
    }
}

OpenpuxApp.prototype.debuglog = function(message) {
    if (this.debug) {
        this.logger.log("debug", this.moduleName + ": " + message);
    }
}

module.exports = {
  App: OpenpuxApp
};
