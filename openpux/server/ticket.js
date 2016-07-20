
//
// Storage Root:   /Tickets/
//

//
//   ticket.js
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

var memoryCache = require('./memorycache.js');

var crypto = require('crypto');

function TicketServer(config)
{
    this.moduleName = "TicketServer";
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

    this.storage = this.config.storage;

    //
    // Note:
    //
    // Small sensors may require a smaller token length due to storage
    // limitations.
    //
    // But risk for small entropy tickets is minimized by only giving them
    // limited permissions, such as add readings for a specific sensor.
    //
    // Larger length tickets with more entropy bytes can be issued for
    // administrative operations, account tokens, etc. which have
    // a wider impact.
    //
    // Proposed Policy: Shorter ticket lengths (lower entropy) are for limited
    // operations only for small sensors with limited memory space. (16 chars).
    //
    // Larger tokens are required for administration and application oriented
    // operations in which the platforms can support it.
    //
    //this.sensorEntropyBytes = 12; // 96 bits, or approx 16 characters in base64 encoding.

    //this.accountEntropyBytes = 32; // 256 bits, or approx 44 characters in base64 encoding.

    // This is the current default
    this.entropyBytes = 12; // 96 bits, or approx 16 characters in base64 encoding.

    //
    // Tickets data path
    //
    this.ticketsPath = "/Tickets/";

    this.ticketCache = new memoryCache.MemoryCache(
        this.config.ticket_cache.max_entries,
        this.config.ticket_cache.trim_count
        );
}

//
// This validates the authorization header, retrieves the token, and looks up the
// ticket.
// 
// callback(error, ticket)
//
TicketServer.prototype.acquireRequestTicket = function (req, callback) {

    var self = this;

    this.getTicketFromRequest(req, function(error, ticket) {

        if (error != null) {
            self.logger.error("request from " + req.socket.remoteAddress + 
                " has an invalid ticket/missing header");
            callback(error, null);
            return;
        }

        if (!self.validateTicketUrl(ticket, req.url)) {
            callback("incorrect object", null);
            return;
        }

        callback(null, ticket);
    });
}

//
// Validate access for a given HTTP request which containers a token
// inside an authorization header.
//
// This validates the header, retrieves the token, and looks up the
// ticket.
//
// It then validates whether the given HTTP verb is allowed by the
// ticket for the req.url path specified in the request.
//
// accessRequest specifies the HTTP verb, or type of access being requested.
//
// The request must have an authorization header and it must contain a ticket
// which grants authorization to the resource described in req.url for the
// access right which matches accessRequest.
//
// On success returns the ticket that generated the access.
//
// callback(error, ticket)
//
TicketServer.prototype.validateAccessForRequest = function (req, accessRequest, callback) {

    var self = this;

    this.getTicketFromRequest(req, function(error, ticket) {

        if (error != null) {
            self.logger.error("request from " + req.socket.remoteAddress + 
                " has an invalid ticket/missing header");
            callback(error, null);
            return;
        }

        var accessState = {object: req.url, request: accessRequest};

        self.validateTicket(ticket, accessState, function(error2, accessGranted) {

            if (error2 != null) {
                callback(error2, null);
                return;
            }

            callback(null, ticket);
        });
    });
}

//
// Given a token retrieved from some source lookup the ticket ID and
// see if its valid.
// 
// If the ticket is valid, validate that the accessRequest is allowed by
// the ticket for the supplied URL.
//
// This is for tokens received from other than an HTTP Authorization header
// on a request.
//
TicketServer.prototype.validateAccessForToken = function (token, url, accessRequest, callback) {

    var self = this;

    this.getTicketById(token, function(error, ticket) {

        if (error != null) {
            callback(error, null);
            return;
        }

        var accessState = {object: url, request: accessRequest};

        self.validateTicket(ticket, accessState, function(error2, accessGranted) {

            if (error2 != null) {
                callback(error2, null);
                return;
            }

            callback(null, ticket);
        });
    });
}

//
// Validate that the supplied ticket allows the requested access
// to the supplied resource url.
//
// The ticket has already been retrieved and validated from storage.
//
// callback(error, accessGranted)
//
//   Returns the actual access name granted on success.
//
TicketServer.prototype.validateAccessForTicket = function (ticket, url, accessRequest, callback) {

    var self = this;

    var accessState = {object: url, request: accessRequest};

    self.validateTicket(ticket, accessState, function(error, accessGranted) {

        if (error != null) {
            callback(error, null);
            return;
        }

        callback(null, accessGranted);
    });
}

//
// Return the object path for a ticket
//
TicketServer.prototype.getObjectPathFromTicket = function (ticket) {
    return ticket.object;
}

//
// Returns true if the ticket allows subpaths.
//
// False otherwise.
//
TicketServer.prototype.AllowSubPath = function (ticket) {

    if (ticket.allow_sub_path) {
        return true;
    }
    else {
        return false;
    }
}

//
// Get token from request header
//
TicketServer.prototype.getTokenFromRequestHeaders = function (req) {

    var auth = req.headers["authorization"];
    if (auth == null) {
        return null;
    }

    var bearer = "Bearer ";

    if (auth.length < bearer.lenght) {
        return null;
    }

    var ticket_id = auth.substring(bearer.length, auth.length);

    return ticket_id;
}

//
// Retrieve the ticket from the http request
//
// callback(error, ticket)
//
TicketServer.prototype.getTicketFromRequest = function (req, callback) {

    var auth = req.headers["authorization"];

    if (auth == null) {
        this.logger.error("request from " + req.socket.remoteAddress + 
          " is missing Authorization header");
        callback("no authorization header", null);
        return;
    }

    var bearer = "Bearer ";

    if (auth.length < bearer.lenght) {
        this.logger.error("request from " + req.socket.remoteAddress + 
          " has a short Authorization header length");
        callback("bad Authorization header", null);
        return;
    }

    var ticket_id = auth.substring(bearer.length, auth.length);

    this.getTicketById(ticket_id, callback);
}

//
// Retrieve a ticked by id
//
// callback(error, ticket)
//
TicketServer.prototype.getTicketById = function (ticket_id, callback) {

    var self = this;

    var ticket = null;

    var ticket_path = this.ticketsPath + ticket_id;

    // Try to get it first from the cache
    ticket = self.ticketCache.get(ticket_path);
    if (ticket != null) {
        // cache hit
        callback(null, ticket);
        return;
    }

    //
    // Try to retrieve from storage
    //
    self.config.storage.getItem(ticket_path, function(error, ticketEntry) {

        if (error != null) {

            //
            // We may allow a configured static ticket when the data store
            // is being provisioned, or is offline for local maintainence.
            //
            self.handleGetTicketByIdError(ticket_id, error, callback);
            return;
        }

        // Create ticket from storage version of the ticket
        ticket = self.createTicketFromStore(ticketEntry);
        if (ticket == null) {
            // some sort of storage corruption
            callback("internal storage error, could not retrieve ticket", null);
            return;
        }

        // This could fail, but we just re-get it from storage later
        self.ticketCache.set(ticket_path, ticket);

        callback(null, ticket);
    });
}

//
// Worker function to handle a getTicketById error in which a
// pre-configured ticket maybe available.
//
TicketServer.prototype.handleGetTicketByIdError = function (ticket_id, error, callback) {

    if (error == null) {
        // programming, error, fail
        callback("internal ticket error", null);
        return;
    }

    if ((typeof(this.config.AdministratorPassCode) != "undefined") &&
        (this.config.AdministratorPassCode != null) &&
        (this.config.AdministratorPassCode == ticket_id)) {

        var ticket = this.allocateAdminTicket();
        if (ticket != null) {
            callback(null, ticket);
        }
        else {
            // programming, error, fail
            callback("internal ticket error", null);
        }
        return;
    }

    if (error == "ItemDoesNotExist") {
        this.logger.error("invalid ticket id supplied id=" + ticket_id);
        callback("no ticket by id " + ticket_id, null);
        return;
    }

    this.logger.error("error retrieving ticket id " + ticket_id  + " from storage");

    callback("error retrieving ticket error=" + error, null);
    return;
}

//
// Attempt to allocate a built in admin ticket
//
// Used for boot strapping server configurations, and when
// the main data store is down, or requires provisioning.
//
// Deployer is responsible for removing this passcode from
// the server deployment configuration file.
//
TicketServer.prototype.allocateAdminTicket = function () {

    var ticket = {};

    if ((typeof(this.config.AdministratorPassCode) == "undefined") ||
        (this.config.AdministratorPassCode == null) ||
        (this.config.AdministratorPassCode == "")) {

        return null;
    }

    // We use the same ticket path as storage
    var ticket_path = this.ticketsPath + this.config.AdministratorPassCode;

    var ticket = this.allocateTicket(
        {
          id:       this.config.AdministratorPassCode,
          object:    "/",
          allow_sub_path: true,
          access:   ["Full"]
        }
    );

    this.logger.info("Admin ticket allocated with Full access");

    // This could fail, but we just re-create it
    this.ticketCache.set(ticket_path, ticket);

    return ticket;
}

//
// Create ticket from itemsArray retrieved from storage.
//
TicketServer.prototype.createTicketFromStore = function (itemsArray) {

    var ticket = {};

    ticket.id = itemsArray["id"];
    ticket.object = itemsArray["object"];
    
    if ((itemsArray["allow_sub_path"] != null) && 
        (itemsArray["allow_sub_path"] == "true")) {
        ticket.allow_sub_path = true;
    }
    else {
        ticket.allow_sub_path = false;
    }

    //
    // The access[] permissions array is encoded as a series
    // of properties that begin with "access_".
    //
    // This is to keep the storage object a simple set of name/value entries.
    //
    ticket.access = {};

    var accessSet = false;

    var accessStr = "access_";

    for (var prop in itemsArray) {

        if (prop.search("access_") != 0) {
            continue;
        }

        var name = itemsArray[prop];
        ticket.access[name] = name;

        accessSet = true;
    }

    // we will not return a bad ticket
    if (ticket.id == null) return null;
    if (ticket.object == null) return null;
    if (!accessSet) return null;

    return ticket;
}

//
// Create itemsArray from ticket.
//
// We use the simple storage model of cloud providers such as SimpleDB
// so that we can operate an practically any storage format.
//
TicketServer.prototype.createItemsArrayFromTicket = function (ticket) {

    var itemsArray = {};

    itemsArray["id"] = ticket.id;
    itemsArray["object"] = ticket.object;

    var allow_sub_path = "false";

    if (ticket.allow_sub_path) {
        allow_sub_path = "true";
    }

    itemsArray["allow_sub_path"] = allow_sub_path;

    for (var prop in ticket.access) {
        var str = "access_" + ticket.access[prop];
        itemsArray[str] = ticket.access[prop];
    }

    return itemsArray;
}

//
// Create a delegated ticket that is a subset of the authorization ticket
//
// Returns error if the delegated ticket could not be created since the
// authTicket does not authorize the access rights requested in args.
//
TicketServer.prototype.createDelegatedTicket = function (authTicket, args, callback) {

    var self = this;

    var newArgs = {};

    if (args.object == null) {
        callback("object path not defined for delegate ticket", null);
        return;
    }

    //
    // The authTicket.object must be the prefix, or equal to the requested
    // object path in args.
    //
    if (args.object.search(authTicket.object) != 0) {
        callback("delegated ticket is not a child path of creating ticket", null);
        return;
    }

    newArgs.object = args.object;

    newArgs.allow_sub_path = false;

    if (args.allow_sub_path) {
        if (!authTicket.allow_sub_path) {
            callback("creating ticket does not allow subpaths but delegate ticket is requesting them", null);
            return;
        }

        newArgs.allow_sub_path = true;
    }

    //
    // If the authTicket does not have Full access, then each
    // requested access type in the new ticket must be present in the
    // auth ticket. This prevents the upgrading of a ticket.
    //
    if (authTicket.access["Full"] == null) {
        for (var index in args.access) {
            var request = args.access[index];

            if (authTicket.access[request] == null) {
                // no entry for request granting it
                callback("requested access " + args.access[index] + " not in creating ticket", null);
                return;
            }
        }
    }

    newArgs.access = args.access;

    // if the caller specified a name use it
    if ((typeof(args.id) != "undefined") && (args.id != null)) {
        newArgs.id = args.id;
    }

    self.createTicket(newArgs, function(error, ticket) {

        if (error != null) {
            callback(error, ticket);
            return;
        }

        self.logger.info("delegated ticket " + ticket.id + " created from " + authTicket.id);

        callback(null, ticket);
    });
}

//
// Allocate a new ticket.
//
// A ticket allows specific access to an object.
//
// Parameters:
//
// args -
//
// {
//   id:       "optionalid",
//   object:   "/objectpath/the/ticket/allows/access/to",
//   allow_sub_path: true,
//   access:   [
//                "accessright1",
//                "accessright2",
//                "read",
//                "addreading",
//                "update",
//                "delete"
//             ]
// }
//
TicketServer.prototype.allocateTicket = function (args) {

    var ticket = {};

    //
    // If the caller supplies an id we use it, but its subject
    // to the restrictions for ticket identifiers.
    //
    // If the caller allocates the id, they are responsible to
    // ensure its sequence is cryptographically secure if their
    // application usage requires it.
    //
    if ((typeof(args.id) != "undefined") && (args.id != null)) {
        ticket.id = args.id;
    }
    else {
        // This is the itemName and returned to the clients.
        ticket.id = this.generateToken();
    }

    ticket.allow_sub_path = false;

    //
    // If subpaths are allowed a path such as "/accounts/1" will match any
    // object under the account such as "/accounts/1/sensors",
    // "/accounts/1/sensors/1/settings", etc.
    //
    if ((typeof(args.allow_sub_path) != "undefined") && (args.allow_sub_path)) {
        ticket.allow_sub_path = true;
    }

    // These are required
    ticket.object = args.object;

    // Copy the permissions entries
    ticket.access = {};

    for (var index in args.access) {
        var name = args.access[index];
        ticket.access[name] = name;
    }

    return ticket;
}

//
// Create a new ticket and store it in the data store.
//
// This is the internal create ticket that does not check credentials.
//
TicketServer.prototype.createTicket = function (args, callback) {

    var self = this;

    var ticket = self.allocateTicket(args);

    // Create the storage format for the ticket
    var itemsArray = self.createItemsArrayFromTicket(ticket);

    var ticket_path = this.ticketsPath + ticket.id;

    //
    // Attempt to write it to the store
    //
    self.storage.createItem(ticket_path, itemsArray, function(error, result) {

        if (error != null) {
            self.logger.error(error + " ticket: create ticket");
            callback(error + " create ticket", null);
            return;
        }

        self.logger.info("ticket: new ticket allocated id: " + ticket.id + 
                         " object " + args.object + " access " + args.access);

        callback(null, ticket);
    });
}

//
// Delete Ticket
//
// This is the internal delete ticket that does not check credentials.
//
TicketServer.prototype.deleteTicket = function (ticket, callback) {

    var ticket_path = this.ticketsPath + ticket.id;

    console.log("deleteTicket: ticket_path=" + ticket_path);

    this.storage.deleteItem(ticket_path, callback);
}

//
// Validate that the ticket allows access to the URL.
//
// If the ticket allows sub paths, the ticket.object must be a strict
// subset of url.
//
// If the ticket does not allow subpaths, then the ticket.object must
// exactly match the url.
//
// Returns:
//
//   true - access granted
//
//   false - access denied
//
TicketServer.prototype.validateTicketUrl = function (ticket, url, callback) {

    var object_path_granted = false;

    if ((typeof(ticket.allow_sub_path) != "undefined") && ticket.allow_sub_path) {

        // The ticket.object must be a prefix of the requested object path
        if (url.search(ticket.object) == 0) {
            object_path_granted = true;
        }
    }
    else {

        // ticket.object must refer specifically to the object
        if (ticket.object == url) {
            object_path_granted = true;
        }
    }

    if (!object_path_granted) {

        // Ticket does not refer to the target object
        this.logger.error("ticket: ticket presented for wrong object. ticket.object: " + ticket.object +
                     " target object: " + url);

        return false;
    }

    return true;
}

//
// accessState:
//
// {
//    object:   "object for target such as itemName path",
//    request:  "what access type is being requested",
//    allow_sub_path: true
// }
//
// callback(error, accessGranted);
//
TicketServer.prototype.validateTicket = function (ticket, accessState, callback) {

    var object_path_granted = false;

    if (!this.validateTicketUrl(accessState.object)) {
        callback("incorrect object", null);
        return;
    }

    //
    // See if the requested access is allowed
    //
    // Even if the ticket allows Full access we still look for a
    // specific grant first.
    //
    var req = accessState.request;

    if (ticket.access[req] != null) {

        // grant the specific access
        callback(null, ticket.access[req]);
        return;
    }

    //
    // The requested access right has not been found.
    //
    // "Full" access is a special case which allows all operations.
    // Assign with care.
    //
    if (ticket.access["Full"] != null) {

        //
        // grant. We return the requested access type so that
        // application logic does not have to interpret "Full"
        // access.
        //
        callback(null, accessState.request);
        return;
    }

    this.logger.error("ticket: ticket does not grant " + accessState.request +
                      " for object " + accessState.object);

    callback("access not granted", null);
}

//
// Return what ever length base64 string comes back from
// the requested entropy bytes.
//
// Caller can decide if the string is to long, may have
// characters they don't want (+, /), etc. The entropy
// risk is on the caller, not the function here.
//
// Note: This does rely on the platform entropy provided
// to node.js crypto.randomBytes(lengthInBytes);
//
// Common values:
//
// randomBase64String(8)  // 64 bit  entropy, approx 12 characters
// randomBase64String(12) // 96 bit  entropy, approx 16 characters
// randomBase64String(16) // 128 bit entropy, approx 24 characters
// randomBase64String(32) // 256 bit entropy, approx 44 characters
// randomBase64String(64) // 512 bit entropy, approx 88 characters
//
function randomBase64String (lengthInBytes) {

    // https://nodejs.org/api/crypto.html#crypto_crypto_randombytes_size_callback
    var buf = crypto.randomBytes(lengthInBytes);

    //
    // https://en.wikipedia.org/wiki/Base64
    //
    // https://nodejs.org/api/buffer.html#buffer_buf_tostring_encoding_start_end
    //
    var str = buf.toString('base64');

    return str;
}

//
// Generate a token which can be used in the data storage
// system as part an itemName.
//
TicketServer.prototype.generateToken = function () {

    //
    // This will become the tickets itemName in the data store
    //
    var str = randomBase64String(this.entropyBytes);

    //
    // Note: We must remove '/' and '+' characters with with a slight
    // (3.125%) reduction in entropy since these characters are not allowed
    // in itemName's.
    //
    str = str.replace(/\//g, '0');
    str = str.replace(/\+/g, '0');

    return str;
}

module.exports = {
  Server: TicketServer
};
