
//
// rem client.cmd
// node client.js website_url ...
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

    // Set sensor state
    var doSetSensor = false;

    // Query sensor readings
    var doQuerySensor = false;

    // Send sensor readings
    var doSendReadings = false;

    var hostname = "localhost";
    var port = 8080;

    var username = "username";
    var password = "password";

    // Scheme is unsecured http vs. https
    var scheme = "http";

    // Get form inputs
    var accountid = "99999";
    var passcode = "12345678";
    var sensorid = "99999";

    var readingcount = 1;
    var startdate = "";
    var enddate = "";

    // Set Sensor inputs
    var sleeptime = 30;

    // Process arguments
    if (argc >= 1) {
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

    if (argc >= 2) {
        if (argv[2] == "smartpux") {

            hostname = "www.smartpux.com";
            port = 80;

            accountid = "1";
            passcode = "12345678";
            sensorid = "1";
        }
        else if (argv[2] == "localhost") {
            hostname = "localhost";
            port = 8080;

            accountid = "99999";
            passcode = "12345678";
            sensorid = "99999";

        }
        else {
            console.log("Please Select Host");
            usage();
            return;
        }
    }
    else {
        console.log("Please Select Host");
        usage();
        return;
    }

    if (doSetSensor) {
        console.log("SETSENSOR selected");

        var setSensorItems = setupSetSensorItems();

        performSetSensor(scheme, hostname, port, username, password,
		         accountid, passcode, sensorid,
                         sleeptime, setSensorItems);
    }
    else if (doSendReadings) {
        console.log("SENDREADINGS selected");

        var sendReadingsItems = setupSendReadingsItems();

        performSendReadings(scheme, hostname, port, username, password,
                            sendReadingsItems);
    }
    else if (doQuerySensor) {
        console.log("QUERYSENSOR selected");
        performQuerySensorReadings(scheme, hostname, port, username, password,
            accountid, passcode, sensorid, readingcount, startdate, enddate);
    }
    else {
        console.log("No action specified: QUERYSENSOR | SENDREADINGS | SETSENSOR");
        usage();
        return;
    }
}

function usage()
{
    console.log("client.js action host");
    console.log("action one of: QUERYSENSOR | SENDREADINGS | SETSENSOR");
    console.log("host one of: www.smartpux.com localhost");
}

function setupSetSensorItems()
{
    var o = new Object();

    o["AccountID"] = "99999";
    o["PassCode"] = "12345678";
    o["SensorID"] = "99999";

    o["SleepTime"] = 30;

    o["Command"] = 0;

    o["TargetMask0"] = 0x100;
    o["TargetMask1"] = 0x101;
    o["TargetMask2"] = 0x102;
    o["TargetMask3"] = 0x103;

    return o;
}

function setupSendReadingsItems()
{
    var o = new Object();

    var o = new Object();

    o["AccountID"] = "99999";
    o["PassCode"] = "12345678";
    o["SensorID"] = "99999";

    o["SleepTime"] = 30;

    o["Command"] = 0;

    o["SensorReading0"] = 0;
    o["SensorReading1"] = 1;
    o["SensorReading3"] = 2;
    o["SensorReading3"] = 3;

    o["TargetMask0"] = 10;
    o["TargetMask1"] = 11;
    o["TargetMask2"] = 12;
    o["TargetMask3"] = 13;

    return o;
}

function performSetSensor(
    scheme, hostname, port, username, password,
    accountid, passcode, sensorid,
    sleeptime, items
    )
{
    if ((port != null) && (port != 80)) {
        hostname = hostname + ":" + port;
    }

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

function processUpdateSensorResponse(responseDocument) {
      if (responseDocument == null) {
          responseDocument = "null";
      }

      var obj = JSON.parse(responseDocument);

      var status = obj.status;
      if (status != "200 OK") {
          console.log("Error " + status);
          return;
      }

      console.log(obj);
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

function processAddSensorReadingResponse(responseDocument) {
      if (responseDocument == null) {
          responseDocument = "null";
      }

      var obj = JSON.parse(responseDocument);

      var status = obj.status;
      if (status != "200 OK") {
          console.log("Error " + status);
          return;
      }

      console.log(obj);
}

function performQuerySensorReadings(
    scheme, hostname, port, username, password,
    accountid, passcode, sensorid,
    readingcount, startdate, enddate
    )
{
    if (readingcount == null) readingcount = 1;

    if ((port != null) && (port != 80)) {
        hostname = hostname + ":" + port;
    }

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
