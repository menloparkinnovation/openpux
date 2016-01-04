

//
// 12/11/2015
//
// Tickets, Openpux, and Smartpux
//
// Both Openpux and Smartpux conform to REST and address the resource
// starting from /accounts/<accountid> since the data model design
// is that all objects and data are owned by a given account.
//
// The ticket/token can be used for authentication and contains the
// REST root path its valid for (example: "/accounts/1/") and the
// operations that are allowed (example: query readings, add new reading, but not delete).
//
// In many cases it would be more secure, convenient, or flexible to now have to
// give out account details, sensor id's, or REST path organization. In these cases
// the root path of the URL for the operation is the REST root path in the ticket's
// persistent server side data.
// 
// In all cases the preferred transport for the ticket is as the Authentication:
// header in the HTTP request. It is possible to optionally allow querystring
// form such as ?ticket=xxxxx. This is useful for GET to allow browser based
// data access without javascript libraries.
//
// For example, to update a given account without giving its name/id:
//
// Ticket:
//   /accounts/1/
//
// REST URL request:
//   PUT /api/v1/ticket?changelocation=bellevue
//
//
// Or add a reading to a sensor
//
// Ticket:
//   /accounts/1/sensors/2/readings/
//
// REST URL request:
//   POST /api/v1/ticket/{reading-time}
//
// This has the advantage is the user could redirect the
// request to sensor 3 if required, or even interpret the
// request as a group update by updating the server side
// information for the ticket.
//
// Clarify:
//
// Smartpux is:
//
//   A consistent base schema that is extensible and fits fully within
//   the openpux/rester model. Applications can rely on at least the
//   basic schema elements.
//  
//   Javascript client side routines focused on direct object interaction.
//  
//   Provisions for low power sensors communicating using simpler methods.
//  
//   All protocols are 100% openpux/rester compatible, but may target the
//   smartpux application URL vs. the openpux application URL.
//  
//   The Smartpux application server leverages openpux while enforcing
//   its minimal rules.
//  
//   Smartpux can keep its application data in its own tree, but the
//   pattern is the same for openpux and can be used as such.
//
//  /accounts/<accountid>/smartpux/...
//
//  Application id/path is really up to the caller.
//
// 12/10/2015
//
// Schema patterns:
//
// Openpux:
//
//  Anything goes. Any valid path after account is passed to create object and
//  will be created. On the account object parent must exit.
//
//  /accounts/<accountid>/...
//
//  Object will be created even if path components do not exist.
//
//  If you want to find sparse objects, you must know their paths, or seach
//  on object type supplied when createObject was called.
//
//  Substring queries to match other parts of the path can be used such
//  as query all objects whose parent path == account root.
//
// Smartpux:
//
//  Smartpux requires the existance of parents.
//
//  /accounts
//  /accounts/<accountid>
//  /accounts/<accountid>/sensors
//  /accounts/<accountid>/sensors/<sensorid>
//  /accounts/<accountid>/sensors/<sensorid>/sensorsettings
//  /accounts/<accountid>/sensors/<sensorid>/sensorreadings/{time}
//
// so a createObject of "/accounts/<accountid>/sensors/<sensorid>/sensorreadings/{time}"
// will fail if sensorid and accountid do not exist.
//
// A simple algorithm for such an appliation is to refuse to create an object
// if the (cached) get of its parent fails. This will require the building
// up of such trees. This would be an application restriction, not one against
// the whole openpux data storage model.
//
// You can always "walk" the tree of objects starting with
// "/accounts/<accountid>" by querying objects whose parent the path
// is and working your way down.
//
// This means that intermediate stages must exist as place holder nodes
// such as /accounts, sensors, sensorreadings, etc.
//
// sensorsettings is an object with settings as properties.
//
// sensorreadings is a directory object in which readings are child
// objects. The sensorreadings directory object must exist for readings
// to be placed.
//
// The directory object itself may have properties as required by
// the application such as access control, modifiers, etc.
//
// The type directory object is an application concept here, and not
// a required part of the openpux core data schema design.
//
// Smartpux API's
//
// Smartpux API's provide a uniform format for starting an IoT application
// and storing sensor lab data. The openpux general API's may be used
// to create additiona object types and paths as required.
//
// The Smartpux API's themselves enforce a basic schema documented above
// to allow a consistent, uniform set of queries. Additional information
// and objects maybe children of the Smartpux basic objects, parallel to,
// etc.
//
// TODO: How would a customer incorporate the concept of a gateway
// grouping sensors? Parallel, or in the REST path?
//
// Parallel:
//
//  sensorid is a property of the gateway object
//
//  In this case sensors are standalone, but happen to have gateways
//  which have additional configuration (group) data.
//
//  It's application defined to maintain per sensor gateway settings, or
//  pointers back to the gateway for communication parameters.
//
//  /accounts/<accountid>/gateways/<gatewayid>
//     /accounts/<accountid>/gateways/<gatewayid>/sensors/<sensorid>
//
// In Rest Path:
//
//  In this case gateways are the first class addressible entity
//  and sensors must be "reached" through them for all operations.
//
//  /accounts/<accountid>/gateways/<gatewayid>/sensors/<sensorid>
//
// addAccount()
//
// POST "http://host/api/v1/accounts/<accountid>/addaccount?passcode=passcode"
//
// { "NewAccountID": "2", "NewAccountPassCode":"12345678" }
//
// response: json document { ... }
//
// addSensor()
//
// POST "http://host/api/v1/accounts/<accountid>/addsensor?passcode=passcode"
//
// { "NewSensorID": "SensorID", "NewSensorAccountID": "NewSensorAccountID" }
//
// response: json document { ... }
//
// querySensorReadings()
//
// createSensorSettings()
//
// querySensorSettings()
//
// updateSensorSettings()
//
// addSensorReading()
//
// listLogs()
//
// getLog()
//
// addSensorReadingShortForm()
//
// remoteExec()
//
// POST http://host/administration/exec";
//
//    { "command": "cmd",
//      "args": []
//    }
//
// response:
//
// {
//    error:  "response or null on success"
//    stdout: "stdout stream as string"
//    stderr: "stderr stream as string"
// }
//




//
// 12/09/2015
//
// Integration of resterclient.js
//  - merge openpux/smartpux routines to rester routines
//  - figure out the backend schema's, app division
//
// 10/20/2015
//
// allow credentials to be set on opclient instance to be provided
// in all exchanges.
//
// Think about what rpc invoke on openpux objects means.
//   - Allow rpc("method", params) invoke on any object to be sent
//   - Up to "application" to determined what to do with it.
//

//
// For protocol see rest_protocol.txt
//

//
//   openpuxclient.js
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
// This allows the listed functions to be available from this
// module when not loaded in the browser, but run from node.js.
//
// This is used by the client.js utility, which is also used for
// unit and regression testing.
//

if (typeof window == "undefined") {

    //
    // window is undefined we are not running inside of
    // browser, but running inside of node.js so fill in
    // an exports table.
    //

    // module is only available for Node.js
    module.exports = {
        OpenpuxClient: OpenpuxClient
    };
}
else {
    // We are executing within a browser.
    // Browsers use new OpenpuxClient() directly.
}

OpenpuxClient.prototype.createRequest = function() {
  var result = null;

  if (typeof window == "undefined") {

      //
      // If window is undefined we are not running inside a browser,
      // but as a client inside of a Node.js [test] program.
      //
      // So a local replacement for XMLHttpRequest is created.
      //
      result = this.createLocalHttpRequest();

      return result;
  }
  else if (window.XMLHttpRequest) {

    //
    // FireFox, Safari, etc.
    // https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest
    //
    result = new XMLHttpRequest();
    if (typeof result.overrideMimeType != 'undefined') {
      result.overrideMimeType('application/x-www-form-urlencoded'); // TODO: Update to JSON
    }
  }
  else if (window.ActiveXObject) {
    // MSIE
    result = new ActiveXObject("Microsoft.XMLHTTP");
  } 
  else {
    // No known mechanism -- consider aborting the application
    alert("your browser does not support AJAX/REST");
  }
  return result;
}

OpenpuxClient.prototype.createLocalHttpRequest = function() {

    if (typeof window == "undefined") {
        var menlohttp = require('./menlohttprequest.js');
        return menlohttp.createHttpRequest();
    }

    return null;
}

// Constructor function
function OpenpuxClient() {
}

//
// This is a compatibility function for Esquilo.io style
// RPC to their boards programmed in Squirrel, but available
// to Javascript clients through erpc.
//
// erpc(function, [params], [successHandler(result)], [errorHandler(errstr)], [timeout])
//
if (typeof window != "undefined") {

    g_erpcClient = null;

    var erpc = function(method, params, successHandler, errorHandler, timeout)
    {
        // erpc creates its own OpenpuxClient() on demand
        if (g_erpcClient == null) {
            g_erpcClient = new OpenpuxClient();
        }

        var args = {};
        args.scheme = "http";
        args.url = "/erpc";
        args.httpauthusername = null;
        args.httpauthpassword = null;

        args.host = location.host;

        if (typeof(timeout) != "undefined") {
            args.timeout = timeout;
        }

        g_erpcClient.rpc(args, method, params, function(error, result) {

            if (error != null) {
                if (typeof(errorHandler) != "undefined") {
                    errorHandler(error);
                    return;
                }
            }
            else {
                if (typeof(successHandler) != "undefined") {
                    successHandler(result);
                    return;
                }
            }
        });
    }
}

//
// rpc
//
//  var args = {
//      url: url,
//      timeout: timeout
//  };
//
// POST:
//
// { method: "method" }
// { method: "method", params: "value" }
//
// Returns:
//
// callback(error, result)
//   error - is always a string
//   result - is an object or simple type
//
// { error: "error" }
// { result: "result" }
//
//
OpenpuxClient.prototype.rpc = function(ticket, args, method, params, callback)
{
    var self = this;

    // Format the request object
    var obj = {};

    obj.method = method;

    // Only define params entry if supplied.
    if ( (typeof(params) != "undefined") && (params != null)) {
        obj.params = params;
    }

    var timeout = 0;

    if ( (typeof(args.timeout) != "undefined") && (args.timeout != 0)) {
        timeout = args.timeout;
    }

    var json_document = JSON.stringify(obj);

    var url = this.buildUrl(ticket.scheme, ticket.host, args.url);

    var querystring = null;

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json",
            content_document: json_document,
            timeout: timeout
         },

         function(error, response) {
            
            if (error != null) {
                // Error from the transport
                callback(error, null);
                return;
            }

            try {
                var res = JSON.parse(response.responseText);
                if (res.error != null) {
                    // remote method response indicated an error
                    callback(res.error, null);
                }
                else {
                    callback(null, res.result);
                }
            } catch(e) {
                // Badly formatted response message
                callback(e.toString(), null);
            }
        });
}

//
// Add Sensor
//
// callback(error, response) - Function to invoke with response JSON document
//
//  var args = {
//      AccountID: account["AccountID"],
//      items: {
//          NewSensorID: newSensorID
//          NewSensorAccountID: newSensorAccountID
//      }
//  };
//
// POST:
//
// document = JSON
//
// {"AccountID":"1","PassCode":"12345678","items":{"NewSensorID":"2","NewSensorAccountID":"1"}}
//
OpenpuxClient.prototype.addSensor = function(ticket, args, callback)
{
    var self = this;

    if ((args.AccountID == null) || (args.AccountID == "")) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    var s = this.buildApiAccountPath(args.AccountID);

    // Command
    s = s + "/addsensor";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    // Format the request object
    var obj = new Object();

    obj.NewSensorID = args.items.NewSensorID;
    obj.NewSensorAccountID = args.items.NewSensorAccountID;

    // Convert to JSON document for POST
    var json_document = JSON.stringify(obj);

    var url = this.buildUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
         {
            url: url,
            token: args.ticket,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json",
            content_document: json_document
         },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });
}

//
// Query sensor readings.
//
// callback(error, response) - Function to invoke with response JSON document
//
// If latestcount != null the specified count of most recent sensor readings
// is returned.
//
// If latestcount == null, the startdate and enddate must be supplied.
//
//    var args = {
//        AccountID: account["AccountID"],
//        SensorID: account["SensorID"],
//        readingcount: readingcount,
//        startdate: startdate,
//        enddate: enddate
//    };
//
// /api/v1/accounts/<AccountID>/sensors/<SensorID>/readings?passcode=xx&latestcount=1
//
// /api/v1/accounts/<AccountID>/sensors/<SensorID>/readings?passcode=xx& +
//    startDate=2013:01:01:00:00:00&endDate=2013:01:01:00:00:01
//
// Returns:
//
// A JSON document of the form:
//
// { status: 200,
//   error: null,
//   items: 
//    [
//      {
//        itemName: '/accounts/1/sensors/1/readings/2015-11-11T14:48:40.304Z',
//        item: 
//         { AccountID: '1',
//           SensorID: '1',
//           TargetMask0: '0',
//           TargetMask1: '0001',
//           TargetMask2: '0002',
//           TargetMask3: '0003',
//           SensorReading0: '0000',
//           SensorReading1: '0001',
//           SensorReading2: '0002',
//           SensorReading3: '3',
//           TimeStamp: '2015-11-11T14:48:40.304Z' 
//         }
//      },
//      { 
//        itemName: '/accounts/1/sensors/1/readings/2015-11-11T14:46:08.002Z',
//        item: 
//         { AccountID: '1',
//           SensorID: '1',
//           TargetMask0: '0',
//           TargetMask1: '0001',
//           TargetMask2: '0002',
//           TargetMask3: '0003',
//           SensorReading0: '0000',
//           SensorReading1: '0001',
//           SensorReading2: '0002',
//           SensorReading3: '3',
//           TimeStamp: '2015-11-11T14:46:08.002Z' 
//         }
//      },
//      [length]: 2
//    ]
// }
//
OpenpuxClient.prototype.querySensorReadings = function(ticket, args, callback)
{
    var self = this;

    if ((args.AccountID == null) || (args.AccountID == "")) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    if (args.SensorID == null) {
        callback(null, {status: "400", error: "SensorID not specified"});
        return;
    }

    var s = this.buildApiSensorPath(args.AccountID, args.SensorID);

    // Add readings to path
    s = s + "/readings";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = appendQueryString(querystring, "passcode=" + args.PassCode);
    }

    // Add readingcount, start date to querystring if present
    if (args.readingcount != null) {
        // latestcount=1
        querystring = appendQueryString(querystring, "latestcount=" + args.readingcount);
    }
    else {
        if (args.startdate == null) {
            callback(null, {status: "400", error: "startdate == null"});
            return;
        }

        if (args.enddate == null) {
            callback(null, {status: "400", error: "enddate == null"});
            return;
        }

        // ISO 8601 data format is used.
        // 2013:01:01:00:00:00&endDate=2022:01:01:00:00:01
        querystring = appendQueryString(querystring, "&startdate=" + args.startdate);
        querystring = appendQueryString(querystring, "&enddate=" + args.enddate);
    }

    var url = this.buildUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
        {
            url: url,
            token: ticket.token,
            method: "GET",
            httpauthusername: ticket.httpauthusername,
            httpauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json"
        },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });

    return;
}

//
// Create Sensor Settings
//
// callback(error, response) - Function to invoke with response JSON document
//
//  var args = {
//      AccountID: account["AccountID"],
//      SensorID: account["SensorID"],
//      items: items
//  };
//
OpenpuxClient.prototype.createSensorSettings = function(ticket, args, callback)
{
    var self = this;

    if ((args.AccountID == null) || (args.AccountID == "")) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    if (args.SensorID == null) {
        callback(null, {status: "400", error: "SensorID not specified"});
        return;
    }

    var s = this.buildApiSensorPath(args.AccountID, args.SensorID);

    // Add settings to path
    s = s + "/settings";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    // Build a JSON document with the updated sensor settings
    var document = this.buildUpdateSensorSettingsDocument(args.items);
    if (document == null) {
        callback(null, {status: "204"});
        return;
    }

    var url = this.buildUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json",
            content_document: document
         },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });
}

//
// Query sensor settings.
//
// callback(error, response) - Function to invoke with response JSON document
//
// The default return format is JSON.
//
// ticket - access ticket to use
//
//  var ticket = {
//      token: token,
//      url: url
//  };
//
//  var args = {
//      AccountID: account["AccountID"],
//      SensorID: account["SensorID"],
//      items: masks
//  };
//
OpenpuxClient.prototype.querySensorSettings = function(ticket, args, callback)
{
    var self = this;

    if ((args.AccountID == null) || (args.AccountID == "")) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    if (args.SensorID == null) {
        callback(null, {status: "400", error: "SensorID not specified"});
        return;
    }

    var s = this.buildApiSensorPath(args.AccountID, args.SensorID);

    // Add settings to path
    s = s + "/settings";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    var url = this.buildUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
        {
            url: url,
            token: ticket.token,
            method: "GET",
            httpauthusername: ticket.httpauthusername,
            httpauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json"
        },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });
}

//
// Update Sensor Settings
//
// callback(error, response) - Function to invoke with response JSON document
//
//  var args = {
//      AccountID: account["AccountID"],
//      SensorID: account["SensorID"],
//      items: items
//  };
//
OpenpuxClient.prototype.updateSensorSettings = function(ticket, args, callback)
{
    var self = this;

    if ((args.AccountID == null) || (args.AccountID == "")) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    if (args.SensorID == null) {
        callback(null, {status: "400", error: "SensorID not specified"});
        return;
    }

    var s = this.buildApiSensorPath(args.AccountID, args.SensorID);

    // Add settings to path
    s = s + "/settings";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    // Build a JSON document with the updated sensor settings
    var document = this.buildUpdateSensorSettingsDocument(args.items);
    if (document == null) {
        callback(null, {status: "204"});
        return;
    }

    var url = this.buildUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "PUT",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json",
            content_document: document
         },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });
}

//
// Add Sensor Reading
//
// callback(error, response) - Function to invoke with response JSON document
//
//  var args = {
//      AccountID: account["AccountID"],
//      SensorID: account["SensorID"],
//      items: o
//  };
//
OpenpuxClient.prototype.addSensorReading = function(ticket, args, callback)
{
    var self = this;

    if ((args.AccountID == null) || (args.AccountID == "")) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    if (args.SensorID == null) {
        callback(null, {status: "400", error: "SensorID not specified"});
        return;
    }

    var s = this.buildApiSensorPath(args.AccountID, args.SensorID);

    // Add readings to path
    s = s + "/readings";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    // Build a JSON document with the sensor readingss
    var document = this.buildAddSensorReadingDocument(args.items);
    if (document == null) {
        callback(null, {status: "204"});
        return;
    }

    var url = this.buildUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json",
            content_document: document
         },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });
}

//
// List Logs
//
// callback(error, response) - Function to invoke with response object
//
//  var args = {
//      AccountID: account["AccountID"],
//  };
//
OpenpuxClient.prototype.listLogs = function(ticket, args, callback)
{
    var self = this;

    if (args.AccountID == null) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    var s = "/administration/logs";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    var url = this.buildRawUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
        {
            url: url,
            token: ticket.token,
            method: "GET",
            httpauthusername: ticket.httpauthusername,
            httpauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json"
        },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });
}

//
// Get Log
//
// callback(error, response) - Function to invoke with response text
//
//  var args = {
//      AccountID: account["AccountID"],
//      logfile: logFileName
//  };
//
OpenpuxClient.prototype.getLog = function(ticket, args, callback)
{
    var self = this;

    if (args.AccountID == null) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    var s = "/administration/logs/" + args.logfile;

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    var url = this.buildRawUrl(ticket.scheme, ticket.host, s);

    //
    // We don't attempt to parse the JSON since it could be an incomplete log,
    // so we let the caller deal with it.
    //
    this.executeHttpRequest(
        {
            url: url,
            token: ticket.token,
            method: "GET",
            httpauthusername: ticket.httpauthusername,
            httpauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json"
        },

        callback
        );
}

//
// remoteExec - run remote command/script
//
// callback(error, response) - Function to invoke with response object
//
//  var args = {
//      AccountID: account["AccountID"],
//      command: commandToExecute
//      args[]:  args to command
//  };
//
OpenpuxClient.prototype.remoteExec = function(ticket, args, callback)
{
    var self = this;

    if (args.AccountID == null) {
        callback(null, {status: "400", error: "AccountID not specified"});
        return;
    }

    var s = "/administration/exec";

    var querystring = null;

    // Add passcode as querystring if supplied
    if ((args.PassCode != null) && (args.PassCode != "")) {
        querystring = "?passcode=" + args.PassCode;
    }

    var url = this.buildRawUrl(ticket.scheme, ticket.host, s);

    // build JSON document for request
    var obj = {};

    obj.command = args.command;
    obj.args = [];

    for (var prop in args.args) {
        obj.args.push(args.args[prop]);
    }

    var json_document = JSON.stringify(obj);

    //
    // response:
    //
    // {
    //    error:  "response or null on success"
    //    stdout: "stdout stream as string"
    //    stderr: "stderr stream as string"
    // }
    //
    //

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json",
            content_document: json_document
         },

        function(error, returnBlock) {
            self.processJSONServerResponse(error, returnBlock, callback);
        });
}

//
// Add Sensor Reading Short Form
//
// This uses abbreviated values for very small strings sent from low memory
// devices or over SMS text messages.
//
// It acts as a gateway for a subset of the higher level REST protocol
// with translation of a limited number of well known attributes.
//
// Attributes not in the translation list as sent as their original names.
//
// callback(error, response) - Function to invoke with response JSON document
//
//  var args = {
//      AccountID: account["AccountID"],
//      SensorID: account["SensorID"],
//      items: o
//  };
//
// POST:
//   url = "http://localhost:8080/smartpuxdata/data
//   document = A=1&P=12345678&S=1&D0=1
//

OpenpuxClient.prototype.addSensorReadingShortForm = function(ticket, args, callback)
{
    var s = "/smartpuxdata/data";

    //
    // For shortform request we place the AccountID, PassCode, and SensorID into
    // the items object. This gets encoded into its compact form (A=, P=, S=).
    //
    // Currently string order is dependent to have A, P, S as the first items
    // in the string. This is to allow low memory controllers to process the string
    // as a stream without buffering. So we ensure this here as this special
    // case only applies to short form requests.
    //
    var items = {};

    // Javascript places these in order as they are inserted into the new object
    items["AccountID"] = args.AccountID;
    items["SensorID"] = args.SensorID;
    items["Ticket"] = ticket.token;

    // Get the rest of the properties being sent
    for (var prop in args.items) {
        var i = args.items[prop];
        if ((i != null) && (i != "")) {
            items[prop] = i;
        }
    }

    var shortForm = this.convertToShortForm(items);

    var sensorValuesString = this.generateSensorValues(shortForm);

    // Use raw url as it has a different path than the default REST path
    var url = this.buildRawUrl(ticket.scheme, ticket.host, s);

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: null, // querystring was add to url manually
            content_type: "application/x-www-form-urlencoded",
            content_document: sensorValuesString
         },

        callback        // urlencoded string is returned directly, unparsed
        );
}

//
// Worker functions for shortform add reading
//

OpenpuxClient.prototype.processQueryString = function (queryString) {

    if (queryString == null) return null;

    var parms = queryString.split('&');

    var nameValueArray = new Array();

    for (var i = 0; i < parms.length; i++) {

        var str = parms[i];

        var position = str.indexOf('=');

        nameValueArray[str.substring(0, position)] = str.substring(position + 1);

        //console.log(i + ': ' + parms[i]);
    }

    return nameValueArray;
}

OpenpuxClient.prototype.generateSensorValues = function (sensorValues) {

    var responseString = '';
    var index = 0;

    for (var prop in sensorValues) {
        if (index > 0) responseString += '&';

        responseString += prop;
        responseString += '=';
        responseString += sensorValues[prop];
        index++;
    }
    
    return responseString;
}

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
OpenpuxClient.prototype.convertToShortForm = function (settings) {
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
OpenpuxClient.prototype.convertFromShortForm = function (settings) {
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
// Worker functions for JSON exchanges
//

//
// General handler for server response.
//
// Handles error case if set. If not an error attempts to
// parse the returned JSON document.
//
// Returns the parsed object, or an error object.
//
// Arguments:
//
// error - error string
//
// response - Javascript object with response
//
//    { status: null, error: null, responseText: "response document" }
//
// Returns:
//
// callback(error, response)
//
//
OpenpuxClient.prototype.processJSONServerResponse = function(error, response, callback) {

    if (error != null) {
        callback(error, response);
        return;
    }

    // Must have response. This will throw on reference if not.

    //
    // Even on an error there may be a response JSON document with
    // error details. Attempt to parse it.
    //    
    var obj = null;
    if (response.responseText != null) {

        if (response.responseText == "") {
            obj = { status: "415", error: "empty response document" };
            callback("empty response document", null);
            return;
        }

        try {
            obj = JSON.parse(response.responseText);
            callback(null, obj);
        } catch(e) {
            obj = { status: "415", error: "exception processing server response" };
            obj.responseText = response.responseText;
            obj.message = "responseText may be corrupt or incorrect JSON format";
            callback("exception parsing json document", obj);
        }
    }
    else {
        // Pass the response through directly
        callback(null, response);
    }
}

//
// Return a JSON document with updated sensor settings
//
// All properties in items are placed in the document.
//
OpenpuxClient.prototype.buildUpdateSensorSettingsDocument = function(items)
{
    // If any mask is set, we will POST
    var anymaskset = false;

    var obj = {};

    for (var prop in items) {
        anymaskset = true;
        obj[prop] = items[prop];
    }

    // If no masks are set, do not return a document
    if (anymaskset == null) {
        return null;
    }

    var json_document = JSON.stringify(obj);
    return json_document;
}

//
// Return a JSON document with sensor reading attributes
//
// All properties in items are placed in the document.
//
OpenpuxClient.prototype.buildAddSensorReadingDocument = function(items)
{
    var obj = {};

    for (var prop in items) {
        obj[prop] = items[prop];
    }

    var json_document = JSON.stringify(obj);
    return json_document;
}

//
// Build an acount object path for the API
//
OpenpuxClient.prototype.buildApiAccountPath = function(accountid)
{
    if (accountid==null || accountid=="") {
        throw "null AccountID";
    }

    var s = "/accounts/" + accountid;

    return s;
}

//
// Build a sensor object path for the API
//
// /accounts/<accountid>/sensors/<sensorid>
//
OpenpuxClient.prototype.buildApiSensorPath = function(
    accountid,
    sensorid
    )
{
    var s = this.buildApiAccountPath(accountid);

    s += "/sensors/" + sensorid;

    return s;
}

//
// ***** NEW rester.js code merge ****
//
// This  new code should replace previous Smartpux usage above.
//

//
// Create ticket
//
// A ticket represents an access token and a resource URL.
//
// token: string that represent authentication code.
//
// objectUrl: base URL for application REST operations.
//
// args: [optional]
//
// {
//    scheme: "http",
//    hostname: "hostname",
//    port: 80,
//    httpauthusername: "httpauthusername",
//    httpauthpassword: "httpauthpassword"
// }
//
//
OpenpuxClient.prototype.createTicket = function(token, resource, args) {

    var scheme = "http";
    var hostname = "localhost";
    var port = 80;

    var httpauthusername = null;
    var httpauthpassword = null;

    //
    // Optional args can override origin.
    //
    // Node.js callers must supply args unless they are only talking
    // to the localhost with its default settings.
    //
    if ((typeof(args) != "undefined") && (args != null)) {

        if ((typeof(args.scheme) != "undefined") && (args.scheme != null)) {
            scheme = args.scheme;
        }

        // Note: Host could be "host" or "host:port".
        if ((typeof(args.hostname) != "undefined") && (args.hostname != null)) {
            hostname = args.hostname;
        }

        // Note: Do not set this if port is embedded in hostname above
        if ((typeof(args.port) != "undefined") && (args.port != 0)) {
            port = args.port;
        }

        if ((typeof(args.httpauthusername) != "undefined") && (args.httpauthusername != null)) {
            httpauthusername = args.httpauthusername;
        }

        if ((typeof(args.httpauthpassword) != "undefined") && (args.httpauthpassword != null)) {
            httpauthpassword = args.httpauthpassword;
        }
    }
    else {

        //
        // If running inside the browser we must connect back to the host
        // who served the page.
        //

        if (typeof(window) != "undefined") {
            scheme = window.location.protocol;
            hostname = window.location.hostname;
            port = window.location.port;
        }
    }

    if ((port != null) && (port != 80)) {
        hostname = hostname + ":" + port;
    }

    var ticket = {
        token: token,
        url: resource,
        scheme: scheme,
        host: hostname,
        httpauthusername: null,
        httpauthpassword: null
    };

    return ticket;
}

//
// Update the ticket to the new resource/object name
//
OpenpuxClient.prototype.updateTicket = function(ticket, resource) {
    ticket.url = resource;
}

//
// Create a token that can be used for delegated access to a resource
//
// args:
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
OpenpuxClient.prototype.createToken = function(ticket, args, callback) {

    //
    // The createToken invokeObjectMethod call parameters are:
    //
    // url: "/api/v2/token/"
    //
    // method: "createToken"
    //
    // params: -> args passed to this method, see method header above
    //
    var url = "/token/";
    var method = "createToken";

    var params = args;

    var invokeArgs = { url: url, method: method, params: params };

    this.invokeObjectMethod(ticket, invokeArgs, callback);
}

//
// Delete a token.
//
// args:
//
// {
//   id: "id"
// }
//
OpenpuxClient.prototype.deleteToken = function(ticket, args, callback) {

    //
    // The deleteToken invokeObjectMethod call parameters are:
    //
    // url: "/api/v2/token/"
    //
    // method: "deleteToken"
    //
    // params: -> args passed to this method, see method header above
    //
    var url = "/token/";
    var method = "deleteToken";

    var invokeArgs = { url: url, method: method, params: args };

    this.invokeObjectMethod(ticket, invokeArgs, callback);
}

//
// Create an account. 
//
// Returns a token that can be used to access the account with Full
// acccess rights.
//
// ticket - access ticket to use
//
//  var ticket = {
//      token: token,
//      url: url,
//      scheme: scheme,
//      host: host,
//      httpauthusername: httpauthusername,
//      httpauthpassword: httpauthpassword
//  };
//
// args:
//
// {
//   name:     "optional_account_name",
//   id:       "optional_ticket_id",
//   recovery_password: "optional_recovery_password",
// }
//
// returns:
//
// callback(error, result)
//
// return on success:
//
//   { name: 'e0321IOgaaULNzI3', token: 'xVoo0OLa1MBNDZ01' }
//
OpenpuxClient.prototype.createAccount = function(ticket, args, callback) {

    //
    // The createAccount invokeObjectMethod call parameters are:
    //
    // url: "/api/v2/token/"
    //
    // method: "createAccount"
    //
    // params: -> args passed to this method, see method header above
    //
    var url = "/token/";
    var method = "createAccount";

    var invokeArgs = { url: url, method: method, params: args };

    this.invokeObjectMethod(ticket, invokeArgs, callback);
}

//
// args:
//
// {
//   name:     "account_name",
//   id:       "ticket_id"
// }
//
OpenpuxClient.prototype.deleteAccount = function(ticket, args, callback) {

    //
    // The deleteAccount invokeObjectMethod call parameters are:
    //
    // url: "/api/v2/token/"
    //
    // method: "deleteAccount"
    //
    // params: -> args passed to this method, see method header above
    //
    var url = "/token/";
    var method = "deleteAccount";

    var invokeArgs = { url: url, method: method, params: args };

    this.invokeObjectMethod(ticket, invokeArgs, callback);
}

//
// createObject
//
//  var ticket = {
//      token: token,
//      url: url,
//      scheme: scheme,
//      host: host,
//      httpauthusername: httpauthusername,
//      httpauthpassword: httpauthpassword
//  };
//
OpenpuxClient.prototype.createObject = function(ticket, url, itemsArray, callback)
{
    var self = this;

    if (itemsArray == null) {
        callback("items must have at least one entry", null);
        return;
    }

    var objectUrl = null;

    if (url != null) {
        objectUrl = url;
    }    
    else {
        objectUrl = ticket.url;
    }

    // Convert to JSON document for POST
    var json_document = JSON.stringify(itemsArray);

    // http://host/api/v2/objectUrl
    var url = this.buildRestUrl(ticket.scheme, ticket.host, objectUrl);

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: null,
            content_type: "application/json",
            content_document: json_document
         },

        function(error, responseDocument) {
            self.processRestServerResponse(error, responseDocument, callback);
        });
}

//
// getObject
//
// callback(error, itemArray)
//
//  var ticket = {
//      token: token,
//      url: url,
//      scheme: scheme,
//      host: host,
//      httpauthusername: httpauthusername,
//      httpauthpassword: httpauthpassword
//  };
//
OpenpuxClient.prototype.getObject = function(ticket, url, callback)
{
    var self = this;

    var objectUrl = null;

    if (url != null) {
        objectUrl = url;
    }    
    else {
        objectUrl = ticket.url;
    }

    var url = this.buildRestUrl(ticket.scheme, ticket.host, objectUrl);

    this.executeHttpRequest(
        {
            url: url,
            token: ticket.token,
            method: "GET",
            httpauthusername: ticket.httpauthusername,
            httpauthpassword: ticket.httpauthpassword,
            querystring: null,
            content_type: "application/json"
        },

        function(error, responseDocument) {
            self.processRestServerResponse(error, responseDocument, callback);
        });
}

//
// updateObject
//
// callback(error, itemArray)
//
//  var ticket = {
//      token: token,
//      url: url,
//      scheme: scheme,
//      host: host,
//      httpauthusername: httpauthusername,
//      httpauthpassword: httpauthpassword
//  };
//
OpenpuxClient.prototype.updateObject = function(ticket, url, itemsArray, callback)
{
    var self = this;

    if (itemsArray == null) {
        callback("items must have at least one entry", null);
        return;
    }

    var objectUrl = null;

    if (url != null) {
        objectUrl = url;
    }    
    else {
        objectUrl = ticket.url;
    }

    // Convert to JSON document for POST
    var json_document = JSON.stringify(itemsArray);

    var url = this.buildRestUrl(ticket.scheme, ticket.host, objectUrl);

    this.executeHttpRequest(
         {
            url: url,
            token: ticket.token,
            method: "PUT",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: null,
            content_type: "application/json",
            content_document: json_document
         },

        function(error, responseDocument) {
            self.processRestServerResponse(error, responseDocument, callback);
        });
}

//
// deleteObject
//
// callback(error, null)
//
//  var ticket = {
//      token: token,
//      url: url,
//      scheme: scheme,
//      host: host,
//      httpauthusername: httpauthusername,
//      httpauthpassword: httpauthpassword
//  };
//
OpenpuxClient.prototype.deleteObject = function(ticket, url, callback)
{
    var self = this;

    var objectUrl = null;

    if (url != null) {
        objectUrl = url;
    }    
    else {
        objectUrl = ticket.url;
    }

    var url = this.buildRestUrl(ticket.scheme, ticket.host, objectUrl);

    this.executeHttpRequest(
        {
            url: url,
            token: ticket.token,
            method: "DELETE",
            httpauthusername: ticket.httpauthusername,
            httpauthpassword: ticket.httpauthpassword,
            querystring: null,
            content_type: "application/json"
        },

        function(error, responseDocument) {
            self.processRestServerResponse(error, responseDocument, callback);
        });
}

//
// invokeObjectMethod
//
// Arguments:
//
// ticket - access ticket to use
//
//  var ticket = {
//      token: token,
//      url: url,
//      scheme: scheme,
//      host: host,
//      httpauthusername: httpauthusername,
//      httpauthpassword: httpauthpassword
//  };
//
// args - arguments object
//  {
//      url: "objectUrl",      // optional URL. If null its taken from the ticket.
//      method:  "methodName", // Method name to invoke as string
//      params:  parameters,   // optional parameters, may be object or value type
//      timeout: 10000         // optional timeout in milliseconds
//  }
//
// callback(error, result) 
//  - if error != null, error status as string.
//  - if error == null, return from remote method as an object, or value
//
// POST document content:
//
// { method: "method" }
// { method: "method", params: object/value }
//
// POST return document content:
//
// { error: "error" }
// { result: "result" }
//
OpenpuxClient.prototype.invokeObjectMethod = function(ticket, args, callback)
{
    var self = this;

    // Format the request object
    var obj = {};

    var timeout = 0;

    var objectUrl = null;

    if ( (typeof(args.url) != "undefined") && (args.url != null)) {
        objectUrl = args.url;
    }    
    else {
        objectUrl = ticket.url;
    }

    // Only define params entry if supplied.
    if ( (typeof(args.params) != "undefined") && (args.params != null)) {
        obj.params = args.params;
    }

    if ( (typeof(args.timeout) != "undefined") && (args.url != timeout)) {
        timeout = args.timeout;
    }    

    obj.method = args.method;

    //
    // This qualifies the invokeObjectMethod POST from an objectCreate
    // The JSON document contains the controlling method for the invoke,
    // the method here is only informative for logs.
    // 
    var querystring = "?invokeObjectMethod=" + args.method;

    var json_document = JSON.stringify(obj);

    var requestUrl = this.buildRestUrl(ticket.scheme, ticket.host, objectUrl);

    this.executeHttpRequest(
         {
            url: requestUrl,
            token: ticket.token,
            method: "POST",
            httpauthusername: ticket.httpauthusername,
            httpauthauthpassword: ticket.httpauthpassword,
            querystring: querystring,
            content_type: "application/json",
            content_document: json_document,
            timeout: timeout
         },

         function(error, response) {
            
            if (error != null) {
                // Error from the transport
                callback(error, null);
                return;
            }

            try {
                var res = JSON.parse(response.responseText);
                if (res.error != null) {
                    // remote method response indicated an error
                    callback(res.error, null);
                }
                else {
                    callback(null, res.result);
                }
            } catch(e) {
                // Badly formatted response message
                callback(e.toString(), null);
            }
        });
}

//
// Worker functions for JSON exchanges
//

//
// General handler for server response.
//
// Handles error case if set. If not an error attempts to
// parse the returned JSON document.
//
// Returns the parsed object, or an error object.
//
// Arguments:
//
// error - error string
//
// response - Javascript object with response
//
OpenpuxClient.prototype.processRestServerResponse = function(error, response, callback) {

    if (error != null) {
        callback(error, response);
        return;
    }

    // Must have response. This will throw on reference if not.

    //
    // Even on an error there may be a response JSON document with
    // error details. Attempt to parse it.
    //    
    var obj = null;

    if (response.responseText != null) {

        try {
            obj = JSON.parse(response.responseText);

            if ((obj.status != 200) && (obj.status != 201)) {
                callback(obj.error, null);
                return;
            }

            if (typeof(obj.items) != "undefined") {
                callback(null, obj.items);
            }
            else {
                callback(null, null);
            }

        } catch(e) {
            obj = { status: "415", error: "exception processing server response" };
            obj.responseText = response.responseText;
            obj.message = "responseText may be corrupt or incorrect JSON format";
            obj.exception = e.toString();
            obj.stack = e.stack.toString();
            callback("exception parsing json document", obj);
        }
    }
    else {
        // Pass the response through directly
        callback(null, response);
    }
}

//
// Execute an HTTP request.
//
// args:
//         {
//            url: requestUrl,
//            token: ticket.token,
//            method: "POST",
//            httpauthusername: ticket.httpauthusername,
//            httpauthauthpassword: ticket.httpauthpassword,
//            querystring: querystring,
//            content_type: "application/json",
//            content_document: json_document,
//            timeout: timeout
//         }
//
// Returns:
// 
//  callback(null, returnBlock) - success
//    { status: null, error: null, responseText: "response document" }
// 
//  callback(error, returnBlock) - HTTP returned an error status
//  but may optionally still provide information.
//
//    { status: "error message", error: "error details", responseText: "optional response document" }
//
OpenpuxClient.prototype.executeHttpRequest = function(args, callback)
{
    var returnBlock = { status: null, error: null, responseText: null };

    // Create browser independent request
    var req = this.createRequest();
    if (req == null) {
        returnBlock.status = 400;
        returnBlock.error = "NULL HTTPXmlRequest. Unsupported Browser";
        callback(null, returnBlock);
        return;
    }

    if (typeof(args.timeout) != "undefined") {
        // timeout is in milliseconds
        req.timeout = args.timeout;
    }

    // Create the callback function and register it on the request object
    req.onreadystatechange = function() {

      if (req.readyState != 4) {
          return; // Not there yet
      }

      returnBlock.status = req.status;
      returnBlock.responseText = req.responseText;

      // Done processing request
      callback(null, returnBlock);
    }

    var fullUrl = args.url;

    if (args.querystring != null) {
        fullUrl += args.querystring;
    }

    // Now send it to the cloud server
    // open(method, url, async, user, password);
    req.open(args.method, fullUrl, true, args.httpauthusername, args.httpauthpassword);

    req.setRequestHeader("Content-Type", args.content_type);

    //
    // Set the token as a Bearer token in an Authorization header
    //
    // http://self-issued.info/docs/draft-ietf-oauth-v2-bearer.html#authz-header
    // http://self-issued.info/docs/draft-ietf-oauth-v2-bearer.html
    // https://en.wikipedia.org/wiki/Basic_access_authentication
    //
    if (args.token != null) {
        req.setRequestHeader("Authorization", "Bearer " + args.token);
    }

    if (args.content_document != null) {
        req.send(args.content_document);
    }
    else {
        req.send();
    }

    // The above anonymous/lambda function will execute with the request results
}

//
// Simplify the state machine of starting, or adding to a querystring
//
function appendQueryString(querystring, newitem) {

    if (querystring == null) {
        querystring = "?" + newitem;
    }
    else {
        querystring = querystring + "&" + newitem;
    }

    return querystring;
}

//
// Build URL handling scheme and host path
//
OpenpuxClient.prototype.buildRestUrl = function(scheme, host, url) {
    var url = scheme + "//" + host + "/api/v2" + url;
    return url;
}

//
// Build URL handling scheme and host path
//
OpenpuxClient.prototype.buildUrl = function(scheme, host, url) {
    var url = scheme + "//" + host + "/api/v2" + url;
    return url;
}

//
// Build a Raw URL supplied by the caller.
//
OpenpuxClient.prototype.buildRawUrl = function(scheme, host, url) {
    var url = scheme + "//" + host + url;
    return url;
}
