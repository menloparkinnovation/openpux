
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
    processSensorQuery();
}

//
// This is invoked with the response JSON document.
//
// The document may be an error response, or the data requested.
//
function processSensorQueryFormResponse(responseDocument) {
      if (responseDocument == null) {
          responseDocument = "null";
      }

      //showSensorQueryResponseForm();

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

function processSensorQuery() {

    // Scheme is unsecured http vs. https
    var scheme = "http";

    // Get host from the browsers environment
    var host = "www.smartpux.com"

    // no http auth information
    var httpauthusername = null;
    var httpauthpassword = null;

    // Get form inputs
    var accountid = "1";
    var passcode = "12345678";
    var sensorid = "1";

    var readingcount = 10;
    var startdate = "";
    var enddate = "";

    if (readingcount == null) readingcount = 1;

    // openpuxclient.js
    result = opclient.querySensorReadings(
        scheme,
        host,
        httpauthusername,
        httpauthpassword,
        processSensorQueryFormResponse,
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
