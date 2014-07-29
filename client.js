
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
// client.js provides two things:
//
//  1) A simple command line front end for test scripts, server provisioning,
//  sensor status monitoring, etc.
//
//
//  2) Library that can be used by intermediate agents that connect
//     to and communicate with openpux servers.
//

var opclient = require('./openpuxclient.js');

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

    //console.log("argc=" + argc + " argv=");
    //console.log(argv);

    // Set sensor state
    var doSetSensor = false;

    // Query sensor readings
    var doQuerySensor = false;

    // Send sensor readings
    var doSendReadings = false;

    // Caller specified querystring
    var queryString = null;

    // Scheme is unsecured http vs. https
    var scheme = "http";

    var hostname = null;
    var port = null;

    var username = "username";
    var password = "password";

    // TODO: Place these in the queryString format
    var readingcount = 1;
    var startdate = "";
    var enddate = "";

    // Process arguments
    if (argc == 1) {
        console.log("No arguments given");
        usage();
        return;
    }

    if (argc > 1) {
        if (argv[1] == "QUERYSENSOR") {
            doQuerySensor = true;
        }
        else if (argv[1] == "SETSENSOR") {
            doSetSensor = true;
        }
        else if (argv[1] == "SENDREADINGS") {
            doSendReadings = true;
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

    if ((argc > argIndex) && (argv[argIndex].length >= 12)) {

        var s = argv[argIndex].substring(0, 12);

        s = s.toLowerCase();

        if (s.search("querystring=") == 0) {
            queryString = argv[argIndex].substring(12, argv[argIndex].length);

            //console.log("QueryString Specified=");
            //console.log(":" + queryString + ":");
        }
    }

    if (doSetSensor) {
        console.log("SETSENSOR selected");

        if (queryString == null) {
            console.log("SENDREADINGS: Must supply querystring");
            usage();
            return;
        }

        var setSensorItems = setupSetSensorItems(queryString);
        if (setSensorItems == null) {
            console.log("Bad querystring format");
            queryStringUsage();
            return;
        }

        performSetSensor(scheme, hostname, port, username, password,
                         setSensorItems);
    }
    else if (doSendReadings) {
        console.log("SENDREADINGS selected");

        if (queryString == null) {
            console.log("SENDREADINGS: Must supply querystring");
            usage();
            return;
        }

        var sendReadingsItems = setupSendReadingsItems(queryString);
        if (sendReadingsItems == null) {
            console.log("Bad querystring format");
            queryStringUsage();
            return;
        }

        performSendReadings(scheme, hostname, port, username, password,
                            sendReadingsItems);
    }
    else if (doQuerySensor) {
        console.log("QUERYSENSOR selected");

        if (queryString == null) {
            console.log("QUERYSENSOR: Must supply querystring");
            usage();
            return;
        }

        var querySensorItems = setupQuerySensorItems(queryString);
        if (querySensorItems == null) {
            console.log("Bad querystring format");
            queryStringUsage();
            return;
        }

        performQuerySensorReadings(scheme, hostname, port, username, password,
            readingcount, startdate, enddate, querySensorItems);
    }
    else {
        console.log("No action specified: QUERYSENSOR | SENDREADINGS | SETSENSOR");
        usage();
        return;
    }
}

function usage()
{
    console.log("client.js action host port [querystring=string]");
    console.log("action one of: QUERYSENSOR | SENDREADINGS | SETSENSOR");
    console.log("querystring= used when sending commands and data for SENDREADINGS | SETSENSOR");
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

function setupQuerySensorItems(queryString)
{
    var sensorReadings = processQueryString(queryString);
    if (sensorReadings == null) {
        return null;
    }

    var longFormReadings = convertFromShortForm(sensorReadings);

    console.log("longFormReadings=");
    console.log(longFormReadings);

    return longFormReadings;
}

function setupSetSensorItems(queryString)
{
    var sensorReadings = processQueryString(queryString);
    if (sensorReadings == null) {
        return null;
    }

    var longFormReadings = convertFromShortForm(sensorReadings);

    console.log("longFormReadings=");
    console.log(longFormReadings);

    return longFormReadings;
}

function setupSendReadingsItems(queryString)
{
    var sensorReadings = processQueryString(queryString);
    if (sensorReadings == null) {
        return null;
    }

    var longFormReadings = convertFromShortForm(sensorReadings);

    console.log("longFormReadings=");
    console.log(longFormReadings);

    return longFormReadings;
}

function performSetSensor(
    scheme, hostname, port, username, password,
    items
    )
{
    if ((port != null) && (port != 80)) {
        hostname = hostname + ":" + port;
    }

    // Get the account, passcode, sensorid, sleeptime from the query string
    var accountid = items["AccountID"];
    var passcode = items["PassCode"];
    var sensorid = items["SensorID"];
    var sleeptime = items["SleepTime"];

    // openpuxclient.js
    result = opclient.updateSensorTargetState(
        scheme,
        hostname,
        username,
        password,
        processUpdateSensorResponse,
        accountid,
        passcode,
        sensorid,
        sleeptime,
        items
        );

    if (result != null) {
      alert("Local Error Status: " + result);
    }
}

function processUpdateSensorResponse(responseDocument)
{
      if (responseDocument == null) {
          console.log("responseDocument=null");
      }
      else {
          console.log("responseDocument=" + responseDocument);
      }
}

function performSendReadings(
    scheme, hostname, port, username, password,
    items
    )
{
    if ((port != null) && (port != 80)) {
        hostname = hostname + ":" + port;
    }

    // openpuxclient.js
    result = opclient.addSensorReadingShortForm(
        scheme,
        hostname,
        username,
        password,
        processAddSensorReadingResponse,
        items
        );

    if (result != null) {
      alert("Local Error Status: " + result);
    }
}

function processAddSensorReadingResponse(responseDocument)
{
      //console.log("processAddSensorReadingResponse: responseDocument=");

      if (responseDocument == null) {
          console.log("responseDocument=null");
      }
      else {
          console.log("responseDocument=" + responseDocument);
      }
}

function performQuerySensorReadings(
    scheme, hostname, port, username, password,
    readingcount, startdate, enddate, items
    )
{
    if (readingcount == null) readingcount = 1;

    if ((port != null) && (port != 80)) {
        hostname = hostname + ":" + port;
    }

    // Get the account, passcode, and sensorid from the querystring
    var accountid = items["AccountID"];
    var passcode = items["PassCode"];
    var sensorid = items["SensorID"];

    // openpuxclient.js
    result = opclient.querySensorReadings(
        scheme,
        hostname,
        username,
        password,
        processSensorQueryResponse,
        accountid,
        passcode,
        sensorid,
        readingcount,
        startdate,
        enddate
        );

    if (result != null) {
      alert("Local Error Status: " + result);
    }
}

//
// This is invoked with the response JSON document.
//
// The document may be an error response, or the data requested.
//
function processSensorQueryResponse(responseDocument) {
      if (responseDocument == null) {
          responseDocument = "null";
      }

      var obj = JSON.parse(responseDocument);

      var status = obj.status;
      if (status != "200 OK") {
          console.log("Error " + status);
          return;
      }

      for(var index = 0; index < obj.sensorreading.length; index++) {
          //processSensorReading(obj.sensorreading[index], status);
          console.log(obj);
      }
}

//
// QueryString form is used as a short hand to send sensor data
// and commands around.
//
// Since its compact, it can travel over SMS, be encoded as NMEA 0183,
// sent over serial ports, etc.
//
// send readings:
//
//  A=99999&S=99999&P=12345678&D0=0000&D1=0001&D2=0002&D3=3&M0=0&M1=0001&M2=0002&M3=0003
//
// set sensor state
//
//  A=99999&S=99999&P=12345678&C=0&U=30&M0=0&M1=0001&M2=0002&M3=0003
//

function processQueryString(queryString) {

    if (queryString == null) {
        return null;
    }

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

//
// Short form is used by the minimal sensor readings POST function
// as 'application/x-www-form-urlencoded'.
//
// We convert to the full names that we actually store in the
// data store and return/manipulate for applications.
//
function convertFromShortForm(sensorReading) {

/*

Short form from POST as 'application/x-www-form-urlencoded'

A is the account
P is the passcode
S is the sensorID

D0 - D9 are valid sensor readings
M0 - M3 are valid masks

{ A: '1',
  P: '12345678',
  S: '1',
  D0: '0',
  D1: '2',
  D2: 'f',
  M0: '0'
  TimeAdded: 1399997573344 }

  A=01234567&S=012345678&P=0123456789ABCDEF&D0=34f4&D1=3467&D2=ffff&D4=1234&D5=3467&D6=1234&M0=0

 http://www.smartpux.com/smartpuxdata/dataapp/REST/1/12345678/1/querydata?latestCount=1

{
  "sensorreading": [
    {
      "SensorID": "1",
      "SensorReading0": "67",
      "SensorReading1": "66",
      "SensorReading2": "0",
      "SensorReading3": "0",
      "SensorReading4": "0",
      "SensorReading5": "0",
      "SensorReading6": "0",
      "SensorReading7": "0",
      "SensorReading8": "1",
      "SensorReading9": "0",
      "SensorMask0": "0",
      "SensorMask1": "70",
      "SensorMask2": "145",
      "SensorMask3": "1",
      "TimeAdded": "Mon Apr 07 05:16:12 UTC 2014"
    }
  ],
  "status": "200 OK"
}
*/

    var o = new Object();

    if (sensorReading["TimeAdded"] != null) {
        o.TimeAdded = sensorReading["TimeAdded"];
    }

    if (sensorReading["A"] != null) {
        o.AccountID = sensorReading["A"];
    }

    if (sensorReading["P"] != null) {
        o.PassCode = sensorReading["P"];
    }

    if (sensorReading["S"] != null) {
        o.SensorID = sensorReading["S"];
    }

    if (sensorReading["U"] != null) {
        o.SleepTime = sensorReading["U"];
    }

    if (sensorReading["C"] != null) {
        o.Command = sensorReading["C"];
    }

    if (sensorReading["M0"] != null) {
        o.TargetMask0 = sensorReading["M0"];
    }

    if (sensorReading["M1"] != null) {
        o.TargetMask1 = sensorReading["M1"];
    }

    if (sensorReading["M2"] != null) {
        o.TargetMask2 = sensorReading["M2"];
    }

    if (sensorReading["M3"] != null) {
        o.TargetMask3 = sensorReading["M3"];
    }

    if (sensorReading["M4"] != null) {
        o.TargetMask4 = sensorReading["M4"];
    }

    if (sensorReading["M5"] != null) {
        o.TargetMask5 = sensorReading["M5"];
    }

    if (sensorReading["M6"] != null) {
        o.TargetMask6 = sensorReading["M6"];
    }

    if (sensorReading["M7"] != null) {
        o.TargetMask7 = sensorReading["M7"];
    }

    if (sensorReading["M8"] != null) {
        o.TargetMask8 = sensorReading["M8"];
    }

    if (sensorReading["M9"] != null) {
        o.TargetMask9 = sensorReading["M9"];
    }

    if (sensorReading["D0"] != null) {
        o.SensorReading0 = sensorReading["D0"];
    }

    if (sensorReading["D1"] != null) {
        o.SensorReading1 = sensorReading["D1"];
    }

    if (sensorReading["D2"] != null) {
        o.SensorReading2 = sensorReading["D2"];
    }

    if (sensorReading["D3"] != null) {
        o.SensorReading3 = sensorReading["D3"];
    }

    if (sensorReading["D4"] != null) {
        o.SensorReading4 = sensorReading["D4"];
    }

    if (sensorReading["D5"] != null) {
        o.SensorReading5 = sensorReading["D5"];
    }

    if (sensorReading["D6"] != null) {
        o.SensorReading6 = sensorReading["D6"];
    }

    if (sensorReading["D7"] != null) {
        o.SensorReading7 = sensorReading["D7"];
    }

    if (sensorReading["D8"] != null) {
        o.SensorReading8 = sensorReading["D8"];
    }

    if (sensorReading["D9"] != null) {
        o.SensorReading9 = sensorReading["D9"];
    }

    return o;
}
