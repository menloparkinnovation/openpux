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
//   Openpux.js
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
// Storage Support
//

var SensorReadingsTable = new Array();

var SensorSettingsTable = new Array();

// RFC 2822 http://tools.ietf.org/html/rfc2822#page-14
// http://msdn.microsoft.com/en-us/library/ie/ff743760
// http://stackoverflow.com/questions/14914739/which-date-formats-are-ietf-compliant-rfc-2822-timestamps
// http://en.wikipedia.org/wiki/Year_2038_problem
var MaxDate = Date.parse("01/01/2038 00:00:00");

//
// Reverse dates are used so that the sort order is reversed
// which has the latest readings at the "top" if standard collating
// order is used.
//
var calculateReverseDate = function (date) {
    return MaxDate - date;
}

// http://stackoverflow.com/questions/1344500/efficient-way-to-insert-a-number-into-a-sorted-array-of-numbers
var sortedInsert = function (element, array) {
    array.splice(locationOf(element, array) + 1, 0, element);
}

// http://stackoverflow.com/questions/1344500/efficient-way-to-insert-a-number-into-a-sorted-array-of-numbers
var locationOf = function (element, array, start, end) {
  start = start || 0;
  end = end || array.length;
  var pivot = parseInt(start + (end - start) / 2, 10);
  if (end-start <= 1 || array[pivot] === element) return pivot;
  if (array[pivot] < element) {
    return locationOf(element, array, pivot, end);
  } else {
    return locationOf(element, array, start, pivot);
  }
}

var dumpSensorReadingsTable = function() {

    for (var prop in SensorReadingsTable) {
        console.log("prop=" + prop);
        console.log("value=" + SensorReadingsTable[prop]);

        var sensors = SensorReadingsTable[prop];

        for (var prop in sensors) {
            console.log("sensors_prop=" + prop);
            console.log("sensors_value=" + sensors[prop]);

            var readings = sensors[prop];

            for (var prop in readings) {
                console.log("readings_prop=" + prop);
                console.log("readings_value=" + readings[prop]);

                var values = readings[prop];
                console.log(values);
            }
        }
    }
}

var dumpSensorSettingsTable = function() {

    for (var prop in SensorSettingsTable) {
        console.log("prop=" + prop);
        console.log("value=" + SensorSettingsTable[prop]);

        var sensors = SensorSettingsTable[prop];

        for (var prop in sensors) {
            console.log("sensors_prop=" + prop);
            console.log("sensors_value=" + sensors[prop]);

            var settings = sensors[prop];

            for (var prop in settings) {
                console.log("readings_prop=" + prop);
                console.log("readings_value=" + settings[prop]);
            }
        }
    }
}

var querySensorSettingsFromStorage = function(itemsArray) {

    // itemsArray['AccountID'] == Account
    // itemsArray['Password'] == PassCode
    // itemsArray['SensorID'] == Sensor

    var account = itemsArray['AccountID'];
    if (account == null) {
        console.log("missing AccountID");
        return false;
    }

    var sensor = itemsArray['SensorID'];
    if (sensor == null) {
        console.log("missing SensorID");
        return false;
    }

    var sensors = SensorSettingsTable[account];
    if (sensors == null) {
        return null;
    }

    var settings = sensors[sensor];
    if (settings == null) {
        return null;
    }

    return settings;
}

var addSensorReadingsToStorage = function(itemsArray) {

    // itemsArray['AccountID'] == Account
    // itemsArray['PassCode'] == PassCode
    // itemsArray['SensorID'] == Sensor

    var account = itemsArray['AccountID'];
    if (account == null) {
        console.log("missing AccountID");
        return false;
    }

    var sensor = itemsArray['SensorID'];
    if (sensor == null) {
        console.log("missing SensorID");
        return false;
    }

    var sensors = SensorReadingsTable[account];
    if (sensors == null) {
        console.log("creating sensors for account=" + account);
        sensors = new Array();
        SensorReadingsTable[account] = sensors;
    }

    var readings = sensors[sensor];
    if (readings == null) {
        console.log("creating readings for sensor=" + sensor);
        readings = new Array();
        sensors[sensor] = readings;
    }

    var millis = Date.now();

    console.log("Date.now()=" + millis);

    // Add the reading insertation time
    itemsArray.TimeAdded = millis;

    // Add the readings to the end of the time indexed array
    readings.push(itemsArray);

    // Dump all the readings
    dumpSensorReadingsTable();

    return true;
}

var queryLastReading = function(account, sensor) {
    
    var sensors = SensorReadingsTable[account];
    if (sensors == null) {
        return null;
    }

    var readings = sensors[sensor];
    if (readings == null) {
        return null;
    }

    return readings[readings.length - 1];
}

var queryLastReadings = function(account, sensor, readingCount) {
    
    var sensors = SensorReadingsTable[account];
    if (sensors == null) {
        return null;
    }

    var readings = sensors[sensor];
    if (readings == null) {
        return null;
    }

    return readings.slice(readings.length - readingCount);
}

var updateSensorSettingsToStorage = function(itemsArray) {

    // itemsArray['AccountID'] == Account
    // itemsArray['PassCode'] == PassCode
    // itemsArray['SensorID'] == Sensor

    var account = itemsArray['AccountID'];
    if (account == null) {
        console.log("missing AccountID");
        return false;
    }

    var sensor = itemsArray['SensorID'];
    if (sensor == null) {
        console.log("missing SensorID");
        return false;
    }

    var sensors = SensorSettingsTable[account];
    if (sensors == null) {
        console.log("creating sensors settings for account=" + account);
        sensors = new Array();
        SensorSettingsTable[account] = sensors;
    }

    var settings = sensors[sensor];
    if (settings == null) {
        console.log("creating settings for sensor=" + sensor);

        // A single set of settings updated as required.
        settings = new Object();
        sensors[sensor] = settings;
    }

    var millis = Date.now();

    console.log("Date.now()=" + millis);

    // Add the settings update time
    itemsArray.TimeAdded = millis;

    // Place the settings into the object
    for(var prop in itemsArray) {
        settings[prop] = itemsArray[prop];
    }

    // Dump the settings
    dumpSensorSettingsTable();

    return true;
}

var processHeadersAndDispatch = function (req, res) {

  // Note: This is handy, but verbose as it walks the object graph
  //console.log("Request: ObjectDumpBegin:\n\n");
  //console.log(req);
  //console.log("Request: ObjectDumpEnd:\n\n");

  if (req.url == '/smartpuxdata/data') {

      if (req.method != "POST") {
          sendError(req, res, 400, "only POST accepted");
          return false;
      }

      //
      // The data URL is an simple x-www-form-urlencoded short form
      // sensor update POST exchange.
      //
      // This allows the sensor to send its latest readings, and
      // returned the sensors latest settings.
      //
      // It's designed for simple sensors with minimal memory.
      //
      // The short hand notation is expanded before placing into
      // the sensor readings data store.
      //

      if (req.headers['content-type'] != 'application/x-www-form-urlencoded') {
          sendError(req, res, 400, "content-type != application/x-www.form-urlencoded");
          console.log("content-type != application/x-www.form-urlencoded");
          console.log(req.headers);
          return false;
      }

      //
      // read and process the input. A response is generated
      // when the input end event occurs.
      //
      readAndProcessSimpleSensorExchange(req, res);
  }
  else if (req.url.search('/smartpuxdata/dataapp') == 0) {

      //
      // dataapp URL is a REST request to either set the sensor's
      // cloud settings, or query its previously sent readings.
      //

      if (req.method == "POST") {
          readAndProcessSensorSettings(req, res);
      }
      else if (req.method == "GET") {
          processSensorQuery(req, res);
      }
      else {
          sendError(req, res, 400, "Unknown request " + req.method);
          return false;
      }
  }
  else if (req.url == '/') {
      serveFile(req, res, "sensor.html");
  }
  else if (req.url == '/sensor.html') {
      serveFile(req, res, "sensor.html");
  }
  else if (req.url == '/openpuxclient.js') {
      serveFile(req, res, "openpuxclient.js");
  }
  else {
      console.log("Unknown URL: " + req.url + "\n");
      sendError(req, res, 400, "Unknown URL " + req.url);
      return false;
  }

  return true;
}

//
// This is a REST request to set the sensor settings table.
//
var readAndProcessSensorSettings = function (req, res) {

    // /smartpuxdata/dataapp/REST/account/passcode/SensorID/settargetmask?sleeptime=30&targetmask0=0000&targetmask1=0000&targetmask2=0000&targetmask3=0000

    var tokens = processRESTString(req.url);
    if (tokens == null) {
        sendError(req, res, 400, "Error parsing REST string");
        return;
    }

    if (tokens[2] != 'REST') {
        console.log("RESTSTRING:" + req.url + ":");
        sendError(req, res, 400, "Missing REST indicator");
        return;
    }

    console.log(req.url);
    console.log(tokens);

    var account = tokens[3];
    var passcode = tokens[4];
    var sensor = tokens[5];
    var cmd = tokens[6];  // 'settargetmask?targetmask1=1'

    if (cmd.search("settargetmask?") != 0) {
        console.log("no querystring on request " + req.url);
        sendError(req, res, 400, "no querystring");
    }

    queryString = cmd.substring(14, cmd.length);

    console.log("queryString=" + queryString);

    var sensorSettings = processQueryString(queryString);

    console.log(sensorSettings);

    var o = new Object();

    o.AccountID = account;
    o.PassCode = passcode;
    o.SensorID = sensor;

    // Copy querystring parameters
    for(var prop in sensorSettings) {
        o[prop] = sensorSettings[prop];
    }

    updateSensorSettingsToStorage(o);

    var returnBlock = new Object();
    returnBlock.status = "200 OK";

    res.writeHead(200, {'Content-Type': 'application/json'});

    var jsonString = JSON.stringify(returnBlock);

    console.log(jsonString);

    res.write(jsonString);

    res.end();
}

//
// Convert an Array() to an Object as JSON's return format differs
// based on this.
//
var arrayToObject = function (array) {
    var o = new Object();

    for (var prop in array) {
        o[prop] = array[prop];
    }

    return o;
}

//
// This is a REST request to query the sensor readings.
//
var processSensorQuery = function (req, res) {

    // /smartpuxdata/dataapp/REST/1/12345678/1/querydata?latestCount=1
    // http://www.smartpux.com/smartpuxdata/dataapp/REST/1/12345678/1/querydata?latestCount=1
    // http://localhost:8080/smartpuxdata/dataapp/REST/1/12345678/1/querydata?latestCount=1

    // http://localhost:8080/smartpuxdata/dataapp/REST/1/12345678/1/querydata?$format=json&latestCount=1
    // http://localhost:8080/smartpuxdata/dataapp/REST/account/passcode/SensorID/querydata?startDate=2013:01:01:00:00:00&endDate=2022:01:01:00:00:01

    var tokens = processRESTString(req.url);
    if (tokens == null) {
        console.log("dataapp: Bad Query " + req.url);
        sendError(req, res, 400, "Error parsing REST string");
        return;
    }

    if (tokens[2] != 'REST') {
        console.log("RESTSTRING:" + req.url + ":");
        sendError(req, res, 400, "Missing REST indicator");
        return;
    }

    console.log(req.url);
    console.log(tokens);

    var account = tokens[3];
    var passcode = tokens[4];
    var sensor = tokens[5];
    var cmd = tokens[6];  // 'querydata?$format=json&latestCount=1'

    // Get data
    var reading = queryLastReading(account, sensor);
    if (reading == null) {
        console.log("Unknown account=" + account + " or sensor=" + sensor);
        sendError(req, res, 400, "unknown account/sensor");
    }

    //
    // Our result is an Object an array of sensoreading
    // and result status in JSON format.
    //
    var returnBlock = new Object();

    returnBlock.sensorreading = new Array();
    returnBlock.status = "200 OK";

    //
    // Convert from an Array() of readings to an Object with members so that
    // our JSON format is what is expected.
    //
    returnBlock.sensorreading[0] = arrayToObject(reading);

    res.writeHead(200, {'Content-Type': 'application/json'});

    var jsonString = JSON.stringify(returnBlock);

    res.write(jsonString);

    res.end();
}

//
// This is the simple sensor exchange that uses a shorthand
// syntax as a x-www-form-urlencoded query string.
//
var readAndProcessSimpleSensorExchange = function (req, res) {

  // http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_readable

  //
  // Note: Node.js is fully async. The req.on('data') and
  // req.on('end') events only register the function handlers
  // when this function executes.
  //
  // Upon return from this function they are only scheduled to
  // execute on future data arrival and connected closed events
  // and their data is not yet available.
  // 

  // Start with empty string
  var body = '';

  // Set encoding to utf8, otherwise default is binary
  req.setEncoding('utf8');

  // request fires 'data' events with the data chunk(s)
  req.on('data', function(chunk) {
	  body += chunk;
  })

  // 'end' event indicates entire body has been delivered
  req.on('end', function() {
    processSensorInputAndSendResponse(req, res, body);
  })
}

var processSensorInputAndSendResponse = function (req, res, queryString) {

    console.log('QueryString:' + queryString + ':\n');

    var sensorReadings = processQueryString(queryString);

    // The values will be dumped as JSON
    console.log(sensorReadings);

    var longFormReadings = convertFromShortForm(sensorReadings);

    console.log("Short Form:");
    console.log(sensorReadings);

    console.log("Long Form:");
    console.log(longFormReadings);

    if (!addSensorReadingsToStorage(longFormReadings)) {
        sendError(req, res, 500, "storage update error");
    }

    var sensorValues = querySensorSettingsFromStorage(longFormReadings);

    console.log("querySensorSettingsFromStorage:");

    if (sensorValues != null) {
        console.log(sensorValues);
    }
    else {
        console.log("sensorValues == null");
    }

    sendSimpleSensorResponse(req, res, sensorValues);
}

var sendSimpleSensorResponse = function (req, res, sensorValues) {

  // http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_class_http_serverresponse
  // http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_writable

  // It's OK if there are no values set yet
  if (sensorValues == null) {
      res.writeHead(200, {'Content-Type': 'application/x-www-form-urlencoded'});
      res.end();
      return;
  }

  var shortFormValues = convertToShortForm(sensorValues);

  var responseString = generateSensorValues(shortFormValues);

  console.log("long form values:");
  console.log(sensorValues);

  console.log("short form values:");
  console.log(shortFormValues);

  res.writeHead(200, {'Content-Type': 'application/x-www-form-urlencoded'});

  res.end(responseString);
}

//
// Simple sensor exchange uses limited number of short form values.
//
var convertToShortForm = function (settings) {

    var o = new Object();

    if (settings["Command"] != null) {
        o.C = settings["Command"];
    }

    if (settings["SleepTime"] != null) {
        o.S = settings["SleepTime"];
    }

    if (settings["TargetMask0"] != null) {
        o.M0 = settings["TargetMask0"];
    }

    if (settings["TargetMask1"] != null) {
        o.M1 = settings["TargetMask1"];
    }

    if (settings["TargetMask2"] != null) {
        o.M2 = settings["TargetMask2"];
    }

    if (settings["TargetMask3"] != null) {
        o.M3 = settings["TargetMask3"];
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
var convertFromShortForm = function (sensorReading) {

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

var generateSensorValues = function (sensorValues) {

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

var processQueryString = function (queryString) {

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

var processRESTString = function (restString) {

    // Minimum entries are /REST/account/passcode/sensorid/command
    var MinimumRestEntries = 5;
    var MaximumRestEntries = 10;

    var tokens = restString.substring(1, restString.length).split("/", MaximumRestEntries);

    if (tokens.length < MinimumRestEntries) {
        return null;
    }

    return tokens;
}

var sendError = function (req, res, errorCode, errorMessage) {

  res.writeHead(errorCode, {'Content-Type': 'application/x-www-form-urlencoded'});

  res.write(errorMessage, 'utf8');

  res.end();
}

//
// This is invoked by the httpServer when a new request arrives
//
var request_func = function (req, res) {

  // http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_http_createserver_requestlistener
  //
  // http.IncomingMessage request;
  //   http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_http_incomingmessage
  //   http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_readable
  //
  // http.ServerResponse response;
  //   http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_class_http_serverresponse
  //   http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_writable

  processHeadersAndDispatch(req, res);
}

//
// Static file serving support
//

var fs = require('fs');
var path = require('path');

var serveFile = function (req, res, filePath) {

    if (req.method != "GET") {
        res.writeHead(404);
        res.end();
        return;
    }

    var extname = path.extname(filePath);

    var contentType = 'text/html';
    switch (extname) {
	case '.js':
            contentType = 'text/javascript';
	    break;
	case '.css':
	    contentType = 'text/css';
	    break;
    }
    
    path.exists(filePath, function(exists) {
	
	if (exists) {
            var s = fs.createReadStream(filePath);
            s.on('error', function() {
                res.writeHead(404);
                res.end();
            })

            s.once('fd', function() {
	        res.writeHead(200, { 'Content-Type': contentType });
            });

            s.pipe(res);
	}
	else {
	    res.writeHead(404);
            res.end();
	}
    });
}

//
// Setup server
//

var http = require('http');

// http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_http_createserver_requestlistener
http.createServer(request_func).listen(8080, '127.0.0.1');

console.log('HttpTest Server version 3 running at http://127.0.0.1:8080/');
