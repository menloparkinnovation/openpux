
//
// REST Root:      /api/v2/token/
//
// Storage Root for Accounts:
//                 /accounts
//
// Storage Root for Tokens:
//                 /Tickets
//
// see g_appconfig
//

//
//   ticketapp.js
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

function App(config)
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

    //
    // Setup references to the services we use
    //

    // Setup the inherited logger
    this.logger = this.config.logger;

    this.accounting = this.config.accounting;

    this.utility = this.config.utility;

    this.ticketserver = this.config.ticketserver;

    this.storage = this.config.storage;
}

//
// Create Account
//
// An account is required before resources can be created manipulated.
//
// Once an account has been created its settings/attributes maybe
// updated using PUT requests in the same manner as REST resource objects.
//
// params:
//
// {
//   name:   "optional_account_name",
//   id:     "optional_ticket_id_",
//   object:   "/objectpath/the/ticket/allows/access/to",
//   recovery_password: "recovery_password"
// }
//
//   name: If account name is not supplied, one is generated/assigned.
//
//         The account name must not exist. If it does the error return
//         is ItemAlreadyExists.
//
//   id:   Optional name for the ticket assigned to the account on
//         success. If a name is not supplied one will be randomly
//         assigned similar to createTicket.
//
//         The id if supplied must not exist. If it does the error return
//         is ItemAlreadyExists.
//
//   object: Application URI path to allow access to.
//
//         Example: "/api/v2". With the created account, the full
//         application path will be "/api/v2/accounts/account_name"
//
//         Note that Full access is granted to new accounts for
//         all object paths underneath.
//
//   recovery_password: Password or code used to recover a ticket if its
//         id is lost. This should be as secure as the ticket itself.
//
//         If not set, recovery must be from an administration account
//         with "TicketRecovery" rights.
//
// Returns:
//
// {
//   token: newTicket.id
// }
//
// Returns a new ticket which can be used to access the account
// which has Full rights to the account.
//
App.prototype.createAccount = function(params, req, callback) {

    var self = this;

    var ticketArgs = {};

    // A new account has full access to its resources
    ticketArgs.access = ["Full"];

    ticketArgs.allow_sub_path = true;

    if ((typeof(params.id) != "undefined") && (params.id != null)) {
        ticketArgs.id = params.id;
    }

    var accountName = null;
    var accountProperties = {};

    if ((typeof(params.name) != "undefined") && (params.name != null)) {
        accountName = params.name;
    }
    else {
        // Generate a random name
        accountName = self.ticketserver.generateToken();
    }

    accountProperties.Name = accountName;

    // TODO: Pass this in during create
    var objectPath = "/api/v2" + self.appconfig.storage_root + "/" + accountName;
    //var objectPath = self.appconfig.storage_root + "/" + accountName;

    ticketArgs.object = objectPath;

    // Validate that the auth ticket in the request has CreateAccount access
    self.ticketserver.validateAccessForRequest(req, "CreateAccount", function(error, authTicket) {

        if (error != null) {
            callback(error, null);
            return;
        }

        //
        // Create the ticket for the account
        //
        self.ticketserver.createTicket(ticketArgs, function(error2, newTicket) {

            if (error2 != null) {
                callback(error2 + " error creating ticket for account", null);
                return;
            }

            self.storage.createItem(objectPath, accountProperties, function(error3, result) {

                if (error3 != null) {

                    //
                    // We created a ticket, which is now leaked unless we delete it.
                    // If this call fails, the ticket.id leaks. Not a problem for a generated
                    // one, but keeps the name busy for the user.
                    //
                    // We could handle the ItemAlreadyExists error above and use it
                    // since tickets should be unique. If someone guesses anothers ticket that
                    // would be bad, but a consequence of using simple id's.
                    //
                    // We could keep an accounts creation transaction log that records
                    // attempts and successes, and a maintenance task can periodically
                    // look for incomplete account creations and clean up.
                    //
                    // We could also support ticket recovery passwords to not only
                    // handle a creation reply message being lost, but to allow users
                    // recovery of tickets.
                    //
                    self.ticketserver.deleteTicket(newTicket, function(error4, result4) {
                        callback(error3 + " conflict on account name " + accountName, null);
                    });

                    return;
                }

                // Account for the basic addAccount resources
                self.accounting.createObjectBasicCharge(authTicket.object, objectPath);

                // return the account name and new ticket id as the result
                callback(null,  { status: 201, error: null, name: accountName, token: newTicket.id });
            });
        });
    });
}

App.prototype.deleteAccount = function(params, req, callback) {

    var self = this;
    var msg = null;

    if ((typeof(params.name) == "undefined") && (params.name == "")) {
        callback("account name is missing", null);
    }

    if ((typeof(params.id) == "undefined") || (params.id == "")) {
        callback("ticket id is missing", null);
        return;
    }

    var accountName = params.name;
    var ticketToDelete = params.id;

    var objectPath = self.appconfig.storage_root + "/" + accountName;

    // Validate that the auth ticket in the request has CreateAccount access
    self.ticketserver.validateAccessForRequest(req, "DeleteAccount", function(error, authTicket) {

        if (error != null) {
            self.traceerror("deleteAccount error=" + error);
            callback(error, null);
            return;
        }

        // Delete the account
        self.storage.deleteItem(objectPath, function(error2, result) {

            if (error2 != null) {
                msg = error2 + " error deleting account " + objectPath;

                self.traceerror("deleteAccount error deleting from storage error=" + msg);
                callback(msg, null);
                return;
            }

            self.ticketserver.deleteTicket(ticketToDelete, function(error3, result3) {

                if (error3 != null) {
                    msg = error3 + " error deleting ticket " + ticketToDelete;
                    callback(msg, null);
                    return;
                }

                self.tracelog("Account " + objectPath + " deleted");
                self.tracelog("Ticket " + ticketToDelete + " deleted");

                callback(null, null);
            });
        });
    });
}

//
// params:
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
// Returns:
//
// {
//   token: newTicket.id
// }
//
//
// TODO: Need to start billing account resources.
//
// New tickets should be charged against the account it was
// created on behalf of. This prevents the filling up of the storage with
// abandoned tickets.
//
App.prototype.createToken = function(params, req, callback) {

    var self = this;

    var args = {};

    if ((typeof(params.id) != "undefined") && (params.id != null)) {
        args.id = params.id;
    }

    args.allow_sub_path = false;

    if (typeof(params.allow_sub_path) != "undefined") {
        args.allow_sub_path = params.allow_sub_path;
    }

    args.object = params.object;

    args.access = params.access;

    // Validate that the auth ticket in the request has CreateToken access
    self.ticketserver.validateAccessForRequest(req, "CreateToken", function(error, authTicket) {

        if (error != null) {
            callback(error, null);
            return;
        }

        // auth ticket is valid, try and create delegate ticket
        self.ticketserver.createDelegatedTicket(authTicket, args, function(error2, newTicket) {

            if (error2 != null) {
                callback(error2, null);
                return;
            }

            // return the new ticket id as the result
            callback(null,  { status: 201, token: newTicket.id });
        });
    });
}

App.prototype.deleteToken = function(params, req, callback) {

    var self = this;

    if ((typeof(params.id) == "undefined") || (params.id == "")) {
        callback("token id is missing", null);
        return;
    }

    // Create a fake ticket for deleting
    var ticketToDelete = {
        id: params.id
    };

    // Validate that the auth ticket in the request has DeleteToken access
    self.ticketserver.validateAccessForRequest(req, "DeleteToken", function(error, authTicket) {

        if (error != null) {
            self.traceerror("deleteToken access error=" + error);
            callback(error, null);
            return;
        }

        self.ticketserver.deleteTicket(ticketToDelete, function(error2, result2) {

            if (error2 != null) {
                self.traceerror("deleteToken error=" + error2);
                callback(error2, null);
                return;
            }

            self.tracelog("Token " + ticketToDelete.id + " deleted");
 
            callback(null, null);
        });        
    });
}

//
// This allows the creation and management of tickets
//

var g_appConfig = {
    name:     "ticket",

    rpc_url_root: "/api/v2/token/",

    storage_root: "/accounts",

    remote_method_table: [
        { name: "createToken",   func: App.prototype.createToken },
        { name: "deleteToken",   func: App.prototype.deleteToken },
        { name: "createAccount", func: App.prototype.createAccount },
        { name: "deleteAccount", func: App.prototype.deleteAccount }
    ]
};

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

  if (req.url.search(this.appconfig.rpc_url_root) == 0) {

      if (req.method != "POST") {
          this.utility.sendErrorAsJSON(req, res, 400, "invalid ticket request");
          return true;
      }

      //
      // All Ticket requesta are RPC's since they process complex state
      //

      this.config.rpcserver.process_rpc_Request(req, res, this.appconfig.remote_method_table, this);
      return true;
  }
  else {
      // keep searching
      return false;
  }
}

App.prototype.setTrace = function(value) {
    this.trace = value;
}

App.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
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

App.prototype.debuglog = function(message) {
    if (this.debug) {
        this.logger.log("debug", this.moduleName + ": " + message);
    }
}

module.exports = {
  App: App
};


