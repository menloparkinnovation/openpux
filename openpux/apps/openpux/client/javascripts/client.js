
//
// On a Unix machine this would allow automatic execution after chmod a+x hello.js
// #!/usr/bin/env node
//

//
//   Copyright (C) 2014 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
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
//   client.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014 Menlo Park Innovation LLC
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
//   This specific snapshot is made available under the terms of
//   the Apache License Version 2.0 of January 2004
//   http://www.apache.org/licenses/
//

//
// client.js provides a node.js client interface to openpux HTTP requests.
//
// It is used for:
//
//  1) A simple command line front end for test scripts, server provisioning,
//  sensor status monitoring, etc. run from node.js
//
//  2) Library that can be used by intermediate agents that connect
//     to and communicate with openpux servers.
//

var fs = require('fs');

var util = require('util');

var opclientFactory = require('./openpuxclient.js');

var opclient = new opclientFactory.OpenpuxClient();

//
// Remove argv[0] to get to the base of the standard arguments.
// The first argument will now be the script name.
//
var args = process.argv.slice(1);

// Invoke main
main(args.length, args);

// This will dump the args in JSON
//console.log(args);

function main(argc, argv) {

    // Scheme is unsecured http vs. https
    var scheme = "http:";

    var hostname = null;
    var port = null;

    var httpauthusername = "httpusername";
    var httpauthpassword = "httppassword";

    // TODO: Place these in the queryString format
    var readingcount = 1;
    var startdate = "";
    var enddate = "";

    // Token
    var doCreateToken = false;

    var doDeleteToken = false;

    // Account
    var doCreateAccount = false;

    var doDeleteAccount = false;

    // Sensor
    var doCreateSensor = false;

    var doDeleteSensor = false;

    var doCreateSensorSettings = false;

    var doQuerySensorSettings = false;

    var doUpdateSensorSettings = false;

    // Readings
    var doAddSensorReading = false;

    var doAddSensorReadingShortForm = false;

    var doQuerySensorReadings = false;

    var doListLogs = false;

    var doGetLog = false;

    var doExec = false;

    var logFileName = null;

    var commandName = null;

    var commandArgs = [];

    // Caller specified querystring
    var queryString = null;

    // Process arguments
    if (argc == 1) {
        console.log("No arguments given");
        usage();
        return;
    }

    if (argc > 1) {
        if (argv[1].toLowerCase() == "createtoken") {
            doCreateToken = true;
        }
        else if (argv[1].toLowerCase() == "deletetoken") {
            doDeleteToken = true;
        }
        else if (argv[1].toLowerCase() == "createaccount") {
            doCreateAccount = true;
        }
        else if (argv[1].toLowerCase() == "deleteaccount") {
            doDeleteAccount = true;
        }
        else if (argv[1].toLowerCase() == "createsensor") {
            doCreateSensor = true;
        }
        else if (argv[1].toLowerCase() == "deletesensor") {
            doDeleteSensor = true;
        }
        else if (argv[1].toLowerCase() == "createsensorsettings") {
            doCreateSensorSettings = true;
        }
        else if (argv[1].toLowerCase() == "querysensorsettings") {
            doQuerySensorSettings = true;
        }
        else if (argv[1].toLowerCase() == "updatesensorsettings") {
            doUpdateSensorSettings = true;
        }
        else if (argv[1].toLowerCase() == "addsensorreading") {
            doAddSensorReading = true;
        }
        else if (argv[1].toLowerCase() == "addsensorreadingshortform") {
            doAddSensorReadingShortForm = true;
        }
        else if (argv[1].toLowerCase() == "querysensorreadings") {
            doQuerySensorReadings = true;
        }
        else if (argv[1].toLowerCase() == "listlogs") {
            doListLogs = true;
        }
        else if (argv[1].toLowerCase() == "getlog") {
            doGetLog = true;
        }
        else if (argv[1].toLowerCase() == "exec") {
            doExec = true;
        }
        else {
            console.log("Unrecognized argument: " + argv[1]);
            return;
        }
    }

    if (argc > 2) {
        hostname = argv[2];
    }
    else {
        console.log("Please Select Host");
        usage();
        return;
    }

    // Port
    if (argc > 3) {
        port = argv[3];
    }
    else {
        console.log("Must specify PORT");
        usage();
        return;
    }

    //
    // See if QueryString is specified
    //
    // Note: We want to compare querystring= as case insensitive,
    // but don't want to lower case the entire string as the querystring
    // capitialization may have meaning depending on the protocol/format.
    // 
    argIndex = 4;

    var items = null;

    if (argc > argIndex) {
        if (argv[argIndex].toLowerCase().search("querystring=") == 0) {
            queryString = argv[argIndex].substring(12, argv[argIndex].length);

            items = queryStringToItems(queryString);
            if (items == null) {
                console.log("Bad querystring format");
                queryStringUsage();
                return;
            }
        }
        else if (argv[argIndex].toLowerCase().search("jsonfile=") == 0) {
            jsonFile = argv[argIndex].substring(9, argv[argIndex].length);

            items = jsonFileToItems(jsonFile);
            if (items == null) {
                console.log("bad json file");
                usage();
                return;
            }
        }
        else {
            console.log("Must supply querystring= or jsonfile=");
            usage();
            return;
        }
    }

    argIndex = 5;

    if (argc > argIndex) {

        if (doGetLog) {
            logFileName = argv[argIndex];
        }
        else if (doExec) {
            commandName = argv[argIndex];
            argIndex++;

            // Rest of the arguments are the command parameters
            for(; argIndex < argv.length; argIndex++) {
                commandArgs.push(argv[argIndex]);
            }
        }
    }

    //
    // create the authentication ticket
    //
    var host_args = {
        scheme: scheme,
        hostname: hostname,
        port: port,
        httpauthusername: httpauthusername,
        httpauthpassword: httpauthpassword
    };

    var token = items["Ticket"];
    if (token == null) {
        throw "Ticket not supplied";
    }

    // posted items should not have Ticket
    delete items["Ticket"];

    var ticket = opclient.createTicket(token, "/", host_args);
    if (ticket == null) {
        throw "bad ticket";
    }

    if (doCreateToken) {
        console.log("create token selected");
        performCreateToken(ticket, items);
    }
    else if (doDeleteToken) {
        console.log("delete token selected");
        performDeleteToken(ticket, items);
    }
    else if (doCreateAccount) {
        console.log("create account selected");
        performCreateAccount(ticket, items);
    }
    else if (doDeleteAccount) {
        console.log("delete account selected");
        performDeleteAccount(ticket, items);
    }
    else if (doCreateSensor) {
        console.log("create sensor selected");
        performCreateSensor(ticket, items);
    }
    else if (doDeleteSensor) {
        console.log("delete sensor selected");
        performDeleteSensor(ticket, items);
    }
    else if (doCreateSensorSettings) {
        console.log("create sensor settings selected");

        performCreateSensorSettings(ticket, items);
    }
    else if (doQuerySensorSettings) {
        console.log("query sensor settings selected");

        performQuerySensorSettings(ticket, items);
    }
    else if (doUpdateSensorSettings) {
        console.log("update sensor settings selected");

        performUpdateSensorSettings(ticket, items);
    }
    else if (doAddSensorReading) {
        console.log("add sensor reading selected");

        performAddSensorReading(ticket, items);
    }
    else if (doAddSensorReadingShortForm) {
        console.log("add sensor reading short form selected");

        performAddSensorReadingShortForm(ticket, items);
    }
    else if (doQuerySensorReadings) {

        console.log("query sensor readings selected");

        if ((typeof(items.reading_count) != "undefined") && (items.reading_count != null)) {
            readingcount = items.reading_count;
            delete items.reading_count;
        }

        if ((typeof(items.start_date) != "undefined") && (items.start_date != null)) {
            startdate = items.start_date;
            delete items.start_date;
        }

        if ((typeof(items.end_date) != "undefined") && (items.end_date != null)) {
            enddate = items.end_date;
            delete items.end_date;
        }

        performQuerySensorReadings(ticket, readingcount, startdate, enddate, items);
    }
    else if (doListLogs) {
        console.log("list logs selected");
        performListLogs(ticket, items);
    }
    else if (doGetLog) {
        console.log("get log selected");
        performGetLog(ticket, items, logFileName);
    }
    else if (doExec) {
        console.log("exec selected");
        performExec(ticket, items, commandName, commandArgs);
    }
    else {
        console.log("No action specified:");
        usage();
        return;
    }
}

function usage()
{
    console.log("client.js action host port [querystring=string] | [jsonfile=filename.json]");
    console.log("querystring= data in urlencoded format");
    console.log("jsonfile= data in json file");

    console.log("action is one of:");
    console.log("addacount | deleteaccount");
    console.log("addsensor | deletesensor");
    console.log("addsensorsettings | querysensorsettings | updatesensorsettings | deletesensorsettings");
    console.log("addsensorreading | querysensorreadings");
}

function queryStringUsage()
{
    console.log("QueryString Usage:");

    console.log("SENDREADING:");

    console.log("A=99999&S=99999&P=12345678&D0=0000&D1=0001&D2=0002&D3=3&M0=0&M1=0001&M2=0002&M3=0003");
    console.log("\n");

    console.log("SETSENSOR:");
    console.log("A=99999&S=99999&P=12345678&C=0&U=30&M0=0&M1=0001&M2=0002&M3=0003");

    console.log("\n");
}

function performCreateToken(ticket, items)
{
    if (items.object == null) throw "missing object";
    if (items.allow_sub_path == null) throw "missing allow_sub_path indication";
    if (items.access == null) throw "missing access rights";

    // items.id is optional. If not supplied one is generated.

    var args = {
       id:   items.id,
       object: items.object,
       allow_sub_path: items.allow_sub_path,
       access: items.access
    };

    opclient.createToken(ticket, args, processServerResponse);
}

function performDeleteToken(ticket, items)
{
    if (items.DeleteTokenID == null) throw "missing DeleteTokenID";

    var args = {
       id:   items.DeleteTokenID
    };

    opclient.deleteToken(ticket, args, processServerResponse);
}

function performCreateAccount(ticket, items)
{
    if (items.NewAccountID == null) throw "missing NewAccountID";

    var ticketid = null;

    if (typeof(items.NewAccountTicketID) != "undefined") {
        ticketid = items.NewAccountTicketID;
    }

    var args = {
       name: items.NewAccountID,
       id: ticketid
    };

    opclient.createAccount(ticket, args, processServerResponse);
}

function performDeleteAccount(ticket, items)
{
    if (items.DeleteAccountID == null) throw "missing DeleteAccountID";
    if (items.DeleteAccountTicketID == null) throw "missing DeleteAccountTicketID";

    var args = {
       name: items.DeleteAccountID,
       id:   items.DeleteAccountTicketID
    };

    opclient.deleteAccount(ticket, args, processServerResponse);
}

function performCreateSensor(ticket, items)
{
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    delete items["AccountID"];
    delete items["SensorID"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid,
        items: items
    };

    opclient.addSensor(ticket, args, processServerResponse);
}

function performDeleteSensor(ticket, items)
{
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    delete items["AccountID"];
    delete items["SensorID"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid,
        items: items
    };

    opclient.deleteSensor(ticket, args, processServerResponse);
}

function performCreateSensorSettings(ticket, items)
{
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    delete items["AccountID"];
    delete items["SensorID"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid,
        items: items
    };

    opclient.createSensorSettings(ticket, args, processServerResponse);
}

function performQuerySensorSettings(ticket, items)
{
    // Get the account, passcode, and sensorid from the querystring
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid
    };

    // openpuxclient.js
    result = opclient.querySensorSettings(ticket, args, processServerResponse);
}

function performUpdateSensorSettings(ticket, items)
{
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    // posted items should not have AccountID, SensorID
    delete items["AccountID"];
    delete items["SensorID"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid,
        items: items
    };

    // openpuxclient.js
    opclient.updateSensorSettings(ticket, args, processServerResponse);
}

function performAddSensorReading(ticket, items)
{
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    // posted items should not have AccountID, SensorID
    delete items["AccountID"];
    delete items["SensorID"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid,
        items: items
    };

    // openpuxclient.js
    opclient.addSensorReading(ticket, args, processServerResponse);
}

function performAddSensorReadingShortForm(ticket, items)
{
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    // posted items should not have AccountID, SensorID, Ticket
    delete items["AccountID"];
    delete items["SensorID"];
    delete items["Ticket"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid,
        items: items
    };

    opclient.addSensorReadingShortForm(ticket, args, processAddSensorReadingShortFormResponse);
}

//
// ResponseDocument is a urlencoded querystring
//
function processAddSensorReadingShortFormResponse(error, responseDocument)
{
      if (error != null) {
          console.log("processAddSensorReadingShortFormResponse error=" + error);
          return;
      }

      if (responseDocument == null) {
          console.log("responseDocument=null");
      }
      else {
          console.log("responseDocument=" + responseDocument);
          dumpasjson(responseDocument);
      }
}

function performQuerySensorReadings(ticket, readingcount, startdate, enddate, items)
{
    if (readingcount == null) readingcount = 1;

    // Get the account, passcode, and sensorid from the querystring
    var accountid = items["AccountID"];
    var sensorid = items["SensorID"];

    var args = {
        AccountID: accountid,
        SensorID: sensorid,
        readingcount: readingcount,
        startdate: startdate,
        enddate: enddate
    };

    // openpuxclient.js
    opclient.querySensorReadings(ticket, args, processServerResponse);
}

function performListLogs(ticket, items)
{
    // Get the account, passcode, and sensorid from the querystring
    var accountid = items["AccountID"];

    var args = {
        AccountID: accountid
    };

    // openpuxclient.js
    result = opclient.listLogs(ticket, args, processServerResponse);
}

function performGetLog(ticket, items, logFileName)
{
    // Get the account, passcode, and sensorid from the querystring
    var accountid = items["AccountID"];

    var args = {
        AccountID: accountid,
        logfile: logFileName
    };

    // openpuxclient.js
    result = opclient.getLog(ticket, args, function(error, responseDocument) {

        if (error) {
            console.log("error=" + error);
            if (responseDocument != null) {
                console.log("responseDocument=");
                console.log(responseDocument);
            }
            return;
        }

        var jsonText = responseDocument.responseText;

        // jsonText could be an incomplete log so try and remove last ","
        if (jsonText[jsonText.length-2] == ",") {
            jsonText = jsonText.substring(0, jsonText.length - 2);

            // end the array
            jsonText += "]\n";
        }

        try {
            var obj = JSON.parse(jsonText);
            dumpasjson(obj);
            return obj;
        } catch(e) {
            console.log("error parsing json log, text=");
            console.log(jsonText);
        }
    });
}

function performExec(ticket, items, command, commandArgs)
{
    // Get the account, passcode, and sensorid from the querystring
    var accountid = items["AccountID"];

    var args = {
        AccountID: accountid,
        command: command,
        args: commandArgs
    };

    // openpuxclient.js
    result = opclient.remoteExec(ticket, args, function(error, response) {

      if (error != null) {
          console.log("ServerResponse: error=" + error);
          return;
      }

      if (response == null) {
          console.log("ServerResponse: null response document");
          return;
      }

      var status = response.status;
      if (status != 200) {
          console.log("status: " + status);
          if (response.error != null) {
              console.log("error: " + response.error);
          }
          return;
      }

      if (response.error != null) {
          console.log("error from command " + args.command + " error=" + response.error);
      }

      // Dump the stdout, stderror
      console.log("results from command " + args.command);

      console.log(response.stdout);

      if ((response.stderr != null) && (response.stderr != "")) {
          console.log("stderr:");
          console.log(response.stderr);
      }
    });
}

//
// This is invoked with the server response.
//
function processServerResponse(error, response) {

      console.log("");
      console.log("ServerResponse:");

      if (error != null) {
          console.log("ServerResponse: error=" + error);
          return;
      }

      if (response== null) {
          console.log("ServerResponse: null response document");
          return;
      }

      var status = response.status;
      if ((status != 200) && (status != 201)) {
          console.log("status: " + status);
          if (response.error != null) {
              console.log("error: " + response.error);
          }
      }

      //console.log(response);
      dumpasjson(response);
}

//
// Parses the json document in the specified file path
// to an object.
//
function jsonFileToItems(fileName) {
    try {
        var jsonText = fs.readFileSync(fileName);
        try {
            var obj = JSON.parse(jsonText);
            return obj;
        } catch(ee) {
            console.log("error parsing json from file " + fileName);
            console.log("e=" + ee);
            return;
        }

    } catch(e) {
        console.log("error reading file " + fileName);
        console.log("e=" + e);
        return;
    }
}

//
// Parses the short form query items such as:
// A=1&P=12345678&S=1 into an object with expanded values.
//
function queryStringToItems(queryString)
{
    console.log("queryString=");
    console.log(queryString);

    var shortForm = opclient.processQueryString(queryString);
    if (shortForm == null) {
        return null;
    }

    var longForm = opclient.convertFromShortForm(shortForm);

    console.log("longForm=");
    console.log(longForm);

    return longForm;
}

function dumpasjson (ob) {

      // Dump data as JSON
      // null is full depth, default is 2
      //var inspectOptions = { showHidden: true, depth: null };
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      console.log(dumpdata);
}

