
/*
 * Copyright (C) 2015 Menlo Park Innovation LLC
 *
 * This is licensed software, all rights as to the software
 * is reserved by Menlo Park Innovation LLC.
 *
 * A license included with the distribution provides certain limited
 * rights to a given distribution of the work.
 *
 * This distribution includes a copy of the license agreement and must be
 * provided along with any further distribution or copy thereof.
 *
 * If this license is missing, or you wish to license under different
 * terms please contact:
 *
 * menloparkinnovation.com
 * menloparkinnovation@gmail.com
 */

// Local testing
var g_testHost = {
    scheme:   "http",
    hostname: "localhost",
    port:     8080
};

//
// This can be any token with createToken rights to "/test/"
//
// Administrator bootstrap/test ticket
// Warning! Disable in config/serverconfig.json during deployment.
//
var g_adminToken = "0123456789";

var g_adminBadToken = "01234567890";

var util = require('util');

var openpuxFactory = require('../apps/openpux/client/javascripts/openpuxclient.js');

var g_opclient = new openpuxFactory.OpenpuxClient();

var g_verbose = false;

function ResterTest(config) {
    this.config = config;
    this.opclient = config.opclient;
    this.adminToken = config.adminToken;
    this.adminBadToken = config.adminBadToken;
}

//
// Create an account
//
// name - account name. If not supplied a randomly generated one will be assigned
//        by the server.
//
// tokenid - tokenid to assign that will have full rights to the account. If
//        one is not supplied, a randomly generated one is returned.
//
// password - password to store with the account.
//            This is optional as most applications use token for access, not password.
//
// callback(error, result) - result.
//
// Returns:
//
// { name: '4qdcnsSsAbRocN6D', token: 'oq60efIhJNhJI328' }
//
// Note: Account name and token must be saved. If lost recovery would
// need to be invoked, which is application dependent.
//
// Note for writing recovery code:
//
//  The administrator token has access to all accounts, so it should issue
//  a query against customer supplied/specific attributes to call up accounts
//  created by them.
//
ResterTest.prototype.createAccount = function(ticket, name, tokenid, password, callback) {

    var args = {};

    if (name != null) {
        args.name = name;
    }

    if (tokenid != null) {
        args.id = tokenid;
    }

    if (password != null) {
        args.recovery_password = password;
    }

    this.opclient.createAccount(ticket, args, function(error, result) {
        callback(error, result);
    });
}

//
// callback (error, result)
//
ResterTest.prototype.accountTests = function(callback) {

    var self = this;

    var objectRootUrl = "/";

    var accountName = "TestAccount";

    var ticketId = "012345";

    var recovery = "recovery";

    var adminTicket = self.opclient.createTicket(this.adminToken, objectRootUrl, g_testHost);
    if (adminTicket == null) {
        if (displayErrors) {
            callback("error creating adminTicket", null);
        }
        return;
    }

    //
    // Creating an account with null will assign a new account ID
    //
    // This should never have an account collision as the server uses
    // random ID's that are not supposed to duplicate.
    //
    self.createAccount(adminTicket, null, null, null, function(error, result) {

        if (error != null) {
            console.log("**** error **** createAccount for auto assigned name failed! error1=" + error);
            return;
        }

        console.log("createAccount: name=" + result.name + " token=" + result.token);
        console.log("result=");
        console.log(result);

        // Create account with a name and defined ticketid
        self.createAccount(adminTicket, accountName, ticketId, recovery, function(error2, result2) {

            //
            // This is expected to fail after the first time run against a new
            // database since the test account name is fixed.
            //
            if (error2 != null) {

                if (error2.search("ItemAlreadyExists") == 0) {
                    console.log("**** PASS **** test account " + accountName + 
                                " allready exists on existing database error2=" + error2);
                    return;
                }

                console.log("**** error **** createAccount failed! error2=" + error2);
                return;
            }

            console.log("createAccount: name=" + result2.name + " token=" + result2.token);

            // We return the first auto created account
            callback(null, result);
        });
    });
}

//
// callback (error, result)
//
ResterTest.prototype.tokenTests = function(args, callback) {

    var self = this;

    var displayDetails = true;
    var displayErrors = true;

    if (typeof(args.display_errors) != "undefined") {
        displayErrors = args.display_errors;
    }

    if (typeof(args.display_details) != "undefined") {
        displayDetails = args.display_details;
    }

    // object root which we want the admin ticket valid for
    var objectRootUrl = args.object_root_url;

    // This is the url of the resource we want to create the new ticket for
    var resourceUrl = args.resource_url;

    console.log("TOKEN test objectRootUrl=" + objectRootUrl + " resourceUrl=" + resourceUrl);

    var adminTicket = self.opclient.createTicket(self.adminToken, objectRootUrl, g_testHost);
    if (adminTicket == null) {
        if (displayErrors) {
            callback("error creating adminTicket", null);
        }
        return;
    }

    var args = {
        object: resourceUrl,
        allow_sub_path: true,

        access: [
            "AddReading",
            "InvokePing"
        ]
    };

    self.opclient.createToken(adminTicket, args, function(error, newToken) {

        if (error != null) {
            if (displayErrors) {
                console.log("createToken error=" + error);
                dumpasjson(error);
            }
            callback(error, newToken);
            return;
        }

        if (displayDetails) {
            console.log("createToken: newToken=" + newToken);
        }

        callback(null, newToken);
    });
}

//
// Run Object Create, Read, Update, Delete test against a ticket
// and resource URL.
//
// ticket - ticket to use
//
// resourceUrl - Name of the object to create/read/update/delete
//
// args - other arguments as required
//
// callback (error, result)
//
ResterTest.prototype.object_CRUD_Test = function(ticket, resourceUrl, args, callback) {

    var self = this;

    var displayDetails = true;
    var displayErrors = true;

    if (args != null) {
        if (typeof(args.display_errors) != "undefined") {
            displayErrors = args.display_errors;
        }

        if (typeof(args.display_details) != "undefined") {
            displayDetails = args.display_details;
        }
    }

    var errormsg = " object_CRUD_Test failed on url=" + resourceUrl;

    console.log("CRUD test on " + resourceUrl + " token=" + ticket.token);

    var itemsArray = {
        Property1: "Data1",
        Property2: "Data2",
        Property3: "Data3"
    };

    var updateItems = {
        Property3: "Updated_Data3", // show that Property3 gets a new value
        Property4: "Data4"          // add a new property
    };

    // TODO: make this an array of functions called in sequence...

    //
    // Verify it does not exist, previous test should have cleaned up.
    //
    // Note: It's not an error if it does since a previous test could have
    // aborted early. This status is indicated, and the test will attempt
    // to delete it at the end.
    //
    var args_dont_display = { display_errors: false, display_details: false };

    self.getObject(ticket, resourceUrl, args_dont_display, function(error1, result1) {

        if (error1 == "ItemDoesNotExist") {
            // expected result
        }
        else if (error1 == "ItemAlreadyExists") {

            // TODO: provide cleanup so it will not fail again..

            if (displayErrors) {
                console.log("first getObject: *** object already exists! (FAIL) ***");
            }
            callback(error1 + errormsg, result1);
            return;
        }
        else if (error1.search("authentication error" != -1) ) {
            if (displayErrors) {
                console.log("first getObject: *** authentication error! (FAIL) ***");
            }
            callback(error1 + errormsg, result1);
            return;
        }
        else {
            if (displayErrors) {
                console.log("first getObject: *** unknown error=" + error1 + " (FAIL)");
            }
            callback(error1 + errormsg, result1);
            return;
        }

        self.createObject(ticket, resourceUrl, itemsArray, args, function(error2, result2) {

            // TODO: provide cleanup so it will not fail again..

            if (error2 != null) {
                callback(error2 + " createObject failed!", result2);
                return;
            }

            self.getObject(ticket, resourceUrl, args, function(error3, result3) {

                if (error3 != null) {
                    callback(error3 + " getObject failed!", result3);
                    return;
                }

                // TODO: Add another create object for the expected already exits failure
    
                self.updateObject(ticket, resourceUrl, updateItems, args, function(error4, result4) {

                    if (error4 != null) {
                        callback(error4, " updateObject failed!", result4);
                        return;
                    }

                    self.getObject(ticket, resourceUrl, args, function(error5, result5) {

                        if (error5 != null) {
                            callback(error5 + " getObject failed!", result5);
                            return;
                        }

                        self.deleteObject(ticket, resourceUrl, args, function(error6, result6) {

                            if (error6 != null) {
                                callback(error6 + " deleteObject failed!", result6);
                            }
                            else {
                                callback(error6, result6);
                            }

                            return;
                        });
                    });
                });
            });
        });
    });
}

//
// Wrapers for the client side library calls that allow test program
// controlled output and error display.
//
// They also provide programming examples for applications.
//

//
// callback(error, result)
//
ResterTest.prototype.createObject = function(ticket, resourceUrl, itemsArray, args, callback) {

    var self = this;

    var displayDetails = true;
    var displayErrors = true;

    if (args != null) {
        if (typeof(args.display_errors) != "undefined") {
            displayErrors = args.display_errors;
        }

        if (typeof(args.display_details) != "undefined") {
            displayDetails = args.display_details;
        }
    }

    self.opclient.createObject(ticket, resourceUrl, itemsArray, function(error, result) {

        if (error != null) {
            console.log("createObject error=" + error);
            if (result != null) {
                console.log("createObject error result=");
                dumpasjson(result);
            }

            callback(error, result);
            return;
        }

        if (displayDetails) {
            console.log("createObject: success");

            if (result != null) {
                console.log("createObject result=");
                dumpasjson(result);
            }
        }

        callback(error, result);
    });
}

//
// callback(error, result)
//
ResterTest.prototype.getObject = function(ticket, resourceUrl, args, callback) {

    var self = this;

    var displayDetails = true;
    var displayErrors = true;

    if (args != null) {
        if (typeof(args.display_errors) != "undefined") {
            displayErrors = args.display_errors;
        }

        if (typeof(args.display_details) != "undefined") {
            displayDetails = args.display_details;
        }
    }

    self.opclient.getObject(ticket, resourceUrl, function(error, result) {

        if (error != null) {

            if (displayErrors) {
                console.log("getObject error=" + error);

                if (result != null) {
                    console.log("getObject error result=");
                    dumpasjson(result);
                }
            }

            callback(error, result);
            return;
        }

        if (displayDetails) {
            console.log("getObject: success");

            if (result != null) {
                console.log("getObject result=");
                dumpasjson(result);
            }
        }

        callback(error, result);
    });
}

//
// callback(error, result)
//
ResterTest.prototype.updateObject = function(ticket, resourceUrl, itemsArray, args, callback) {

    var self = this;

    var displayDetails = true;
    var displayErrors = true;

    if (args != null) {
        if (typeof(args.display_errors) != "undefined") {
            displayErrors = args.display_errors;
        }

        if (typeof(args.display_details) != "undefined") {
            displayDetails = args.display_details;
        }
    }

    self.opclient.updateObject(ticket, resourceUrl, itemsArray, function(error, result) {

        if (error != null) {
            console.log("updateObject error=" + error);
            if (result != null) {
                console.log("updateObject error result=");
                dumpasjson(result);
            }

            callback(error, result);
            return;
        }

        if (displayDetails) {
            console.log("updateObject: success");

            if (result != null) {
                console.log("updateObject result=");
                dumpasjson(result);
            }
        }

        callback(error, result);
    });
}

//
// callback(error, result)
//
ResterTest.prototype.deleteObject = function(ticket, resourceUrl, args, callback) {

    var self = this;

    var displayDetails = true;
    var displayErrors = true;

    if (args != null) {
        if (typeof(args.display_errors) != "undefined") {
            displayErrors = args.display_errors;
        }

        if (typeof(args.display_details) != "undefined") {
            displayDetails = args.display_details;
        }
    }

    self.opclient.deleteObject(ticket, resourceUrl, function(error, result) {

        if (error != null) {
            console.log("deleteObject error=" + error);
            if (result != null) {
                console.log("deleteObject error result=");
                dumpasjson(result);
            }

            callback(error, result);
            return;
        }

        if (displayDetails) {
            console.log("deleteObject: success");

            if (result != null) {
                console.log("deleteObject result=" + result);
                dumpasjson(result);
            }
        }

        callback(error, result);
    });
}

ResterTest.prototype.runTests = function(ac, av) {

    var self = this;

    //
    // First run tests against the adminToken and
    // the /test/ object url.
    //

    var objectRootUrl = "/test/";

    // This is the url of the resource we want to create
    var resourceUrl = "/test/firstItem";

    //
    // A ticket represents an access token and a resource.
    //
    var adminTicket = self.opclient.createTicket(self.adminToken, objectRootUrl, g_testHost);
    if (adminTicket == null) {
        console.error("error creating adminTicket");
        return;
    }

    var badAdminTicket = self.opclient.createTicket(self.adminBadToken, objectRootUrl, g_testHost);
    if (badAdminTicket == null) {
        console.error("error creating badAdminTicket");
        return;
    }

    var args_display_errors = { display_errors: true, display_details: false };
    var args_dont_display_errors = { display_errors: false, display_details: false };

    self.object_CRUD_Test(badAdminTicket, resourceUrl, args_dont_display_errors, function(error, result) {

        if (error != null) {

            if (error.search("authentication error") == 0) {
                console.log("Expected Authentication Error with bad Admin Token Passed (PASS)");
            }
            else {
                console.error("unknown error=" + error);
                return;
            }

            console.log("");
        }

        if (error == null) {
            console.error("*** SERIOUS AUTHENTICATION ERROR **** BAD Token succeeded in access!");
            return;
        }

        self.object_CRUD_Test(adminTicket, resourceUrl, args_display_errors, function(error1, result1) {

            reportTestResults("  CRUD on " + resourceUrl, error1, result1);
            console.log("");

            var token_test_args = { 
                object_root_url: "/test/",
                resource_url: "/test/child/",
                display_errors: true,
                display_details: false
                };

            self.tokenTests(token_test_args, function(error2, result2) {
                reportTestResults("  Token on " + token_test_args.resource_url, error2, result2);

                self.accountTests(function(error3, result3) {
                    console.log("");
                    reportTestResults("Account Test", error3, result3);
                });
            });
        });
    });
}

function reportTestResults(name, error, result) {

    if (error != null) {
        console.log(name + " test failed! error=" + error);
    }
    else {
        console.log(name + " test success!");
    }

    if (result != null) {
        console.log(name + " test result=");
        dumpasjson(result);
    }
}

function dumpasjson(ob) {

      //
      // Dump data as JSON
      // null is full depth, default is 2
      // var inspectOptions = { showHidden: true, depth: null };
      //
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      console.log(dumpdata);
}

function main(ac, av) {

    var config = { opclient: g_opclient, adminToken: g_adminToken, adminBadToken: g_adminBadToken };

    var rt = new ResterTest(config);

    rt.runTests(ac, av);
}

function usage(message) {

    if (message != null) {
        console.error(message);
    }

    console.error("restertester args");

    process.exit(1);
}

function tracelog(config, message) {
    if (g_verbose) {
        // console.log connects to stdout
        console.log(message);
    }
}

function errlog(message) {
    // console.error connects to stderr
    console.error(message);
}

//
// Remove argv[0] to get to the base of the standard arguments.
// The first argument will now be the script name.
//
var args = process.argv.slice(1);

// Invoke main
main(args.length, args);
