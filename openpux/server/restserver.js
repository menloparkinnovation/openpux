
//
//   restserver.js
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
// restserver:
//
// Provides REST services for data access and method invocatin (rpc)
// to application modules.
//

function RestServer(config)
{
    this.config = config;

    this.moduleName = "RestServer";

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
// Request for a REST object.
//
// Requests that a ticket is provided in the Authorization header
// that has access to the url and the operation.
//
// Could be a data request with GET, POST, PUT, DELETE, or an RPC
// request with POST url=/app_url/?invokeMethod=method
//
// appconfig - Provides applications url, storage root, and name
//
// {
//   name:         "appname",
//   url_root:     "/api/v2/appname/",
//   storage_root: "/appname"
//
//   rpc_url_root: "/api/v2/appname/",
//
//   remote_method_table: [
//       { name: "createAppItem",  func: App.prototype.createAppItem }
//   ]
// }
//
RestServer.prototype.processObjectRequest = function(appconfig, req, res) {

    var self = this;

    var errormsg = null;

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

        var objectName = self.generateObjectName(appconfig, req);
        if (objectName == null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, 400, "invalid url");
            return;
        }

        if (req.method == "GET") {
            // get an entity
            self.getObject(appconfig, req, res, objectName, ticket);
        }
        else if (req.method == "POST") {
            // a POST may be an object create, or an object method invoke
            self.processPostRequest(appconfig, req, res, objectName, ticket);
        }
        else if (req.method == "PUT") {
            // update an entity
            self.updateObject(appconfig, req, res, objectName, ticket);
        }
        else if (req.method == "DELETE") {
            // delete an entity
            self.deleteObject(appconfig, req, res, objectName, ticket);
        }
        else {
            errormsg = "unsupported HTTP method " + req.method + " for url " + req.url;
            self.utility.logSendErrorAsJSON(self.logger, req, res, 400, errormsg);
            return;
        }
    });

    return;
}

//
// Process an HTTP POST request.
//
RestServer.prototype.processPostRequest = function (appconfig, req, res, objectName, ticket) {

    var isRpc = this.isRpcRequest(req);

    if (isRpc) {
        this.invokeObjectMethod(appconfig, req, res, objectName, ticket);
    }
    else {
        this.createObject(appconfig, req, res, objectName, ticket);
    }
}

//
// Returns:
//
// 201 Created
// 404 Not Found
// 409 Conflict
//
RestServer.prototype.createObject = function (appconfig, req, res, itemName, ticket) {

    var self = this;

    self.utility.receiveJSONObject(req, res, self.logger, function(rcv_error, itemsArray) {

        if (rcv_error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, 400, rcv_error);
            return;
        }

        self.storage.createItem(itemName, itemsArray, function(error, ie) {

            if (error != null) {

                if (error == "ItemAlreadyExists") {
                    self.utility.logSendErrorAsJSON(self.logger, req, res, 409, error);
                }
                else {
                    self.utility.logSendErrorAsJSON(self.logger, req, res, 400, error);
                }

                return;
            }

            // send result as JSON
            self.utility.sendResultAsJSON(req, res, {status: 201, error: null});
        });
    });
}

//
// Returns:
//
// 200 OK
// 400 Bad Request
// 404 Not Found
//
RestServer.prototype.getObject = function (appconfig, req, res, itemName, ticket) {

    var self = this;

    self.storage.getItem(itemName, function(error, itemEntry) {

        if (error != null) {

            if (error == "ItemDoesNotExist") {
                self.utility.logSendErrorAsJSON(self.logger, req, res, 404, error);
            }
            else {
                self.utility.logSendErrorAsJSON(self.logger, req, res, 400, error);
            }

            return;
        }

        // send result as JSON
        var obj = {status: 200, error: null, items: itemEntry};

        self.utility.sendResultAsJSON(req, res, obj);
    });
}

//
// Returns:
//
// 200 OK
// 204 No Content
// 404 Not Found
//
RestServer.prototype.updateObject = function (appconfig, req, res, itemName, ticket) {

    var self = this;

    self.utility.receiveJSONObject(req, res, self.logger, function(rcv_error, itemsArray) {

        if (rcv_error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, 400, rcv_error);
            return;
        }

        self.storage.updateItem(itemName, itemsArray, function(error, itemEntry) {

            if (error != null) {

                if (error == "ItemsDoesNotExist") {
                    self.utility.logSendErrorAsJSON(self.logger, req, res, 404, error);
                }
                else {
                    self.utility.logSendErrorAsJSON(self.logger, req, res, 400, error);
                }

                return;
            }

            // send result as JSON
            self.utility.sendResultAsJSON(req, res, {status: 200, error: null});
        });
    });
}

//
// Returns:
//
// 200 OK
// 404 Not Found
//
RestServer.prototype.deleteObject = function (appconfig, req, res, itemName, ticket) {

    var self = this;

    self.storage.deleteItem(itemName, function(error, itemsEntry) {

        if (error != null) {

            if (error == "ItemsDoesNotExist") {
                self.utility.logSendErrorAsJSON(self.logger, req, res, 404, error);
            }
            else {
                self.utility.logSendErrorAsJSON(self.logger, req, res, 400, error);
            }

            return;
        }

        // send result as JSON
        self.utility.sendResultAsJSON(req, res, {status: 200, error: null});
    });
}

//
// Invoke an object method
//
RestServer.prototype.invokeObjectMethod = function (appconfig, req, res, objectName, ticket) {

    var self = this;

    // Validate that RPC access is allowed on the object
    var accessState = {object: req.url, request: "RPC"};

    self.validateTicket(ticket, accessState, function(error, accessGranted) {

        if (error != null) {
            self.utility.logSendErrorAsJSON(self.logger, req, res, 400, "ticket does not allow RPC to " + req.url);
            callback(error, null);
            return;
        }

        self.config.rpcserver.process_rpc_Request(req, res, appconfig.remote_method_table, this);
    });
}

//
// Generates the object name by trimming off the REST URL root
// and adding in the storage root.
//
// "/api/v2/foo/bar" = "/appname/foo/bar"
//
RestServer.prototype.generateObjectName = function (appconfig, req) {

    // Trim off the url root. Example "/api/v2/foo/bar" => "/foo/bar"
    var name = req.url.substring(appconfig.url_root.length, req.url.length);

    if ((name == null) || (name == "")) {
        return null;
    }

    // add it to the storage root "/foo/bar" => "/appname/foo/bar"
    return appconfig.storage_root + name;
}

RestServer.prototype.isRpcRequest = function (req) {

    var parsed = url.parse(req.url);

    var isRpc = false;

    //
    // ?invokeObjectMethod= in the querystring indicates an rpc request
    // on the object, not a data creation request.
    //
    // This allows the same objects defined as REST data items with
    // POST/PUT to have unambiguous actions invoked against them.
    // 
    if (parsed.query != null) {
        if (parsed.query["invokeMethod"] != null) {
            return true;
        }
    }

    return false;
}

RestServer.prototype.setTrace = function(value) {
    this.trace = value;
}

RestServer.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

RestServer.prototype.tracelog = function(message) {
    if (this.trace) {
        this.logger.info(this.moduleName + ": " + message);
    }
}

RestServer.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        this.logger.error(this.moduleName + ": " + message);
    }
}

RestServer.prototype.debuglog = function(message) {
    if (this.debug) {
        this.logger.log("debug", this.moduleName + ": " + message);
    }
}

module.exports = {
  Server: RestServer
};
