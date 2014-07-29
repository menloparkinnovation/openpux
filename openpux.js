
//
// TODO:
//
// 07/26/2014 8:46AM
//
//  - Write/finish node.js client for automated testing
//
// Finish testing external storage module
//
// 07/24/2014
// IE is broken.
//
//  -> Use servfile.js, scriptform.html to test.
//      - IE works in this case. Must be something with
//        how the three file parts come together in
//        the server. May be ending the HTTP request
//        early, then streaming additional content in which
//        Chrome seems more tolerant of.
//
//        ?Fiddler?
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

var store = require('./memorystore.js');

var querySensorSettingsFromStorage = function(itemsArray, callback) {
    return store.querySettings(itemsArray, callback);
}

var addSensorReadingsToStorage = function(itemsArray, callback) {
    return store.addReadings(itemsArray, callback);
}

var queryLastReading = function(account, sensor, callback) {
    return store.queryLastReading(account, sensor, callback);
}

var queryLastReadings = function(account, sensor, readingCount, callback) {
    return store.queryLastReadings(account, sensor, readingCount, callback);
}

var updateSensorSettingsToStorage = function(itemsArray, callback) {
    return store.updateSensorSettings(itemsArray, callback);
}

var dumpSensorReadingsTable = function(callback) {
    return store.dumpReadings(callback);
}

var dumpSensorSettingsTable = function(callback) {
    return store.dumpSettings(callback);
}

//
// Process headers and dispatch according to the request.
//
// This function is responsible for ending the request
// with a response, error or not.
//
var processHeadersAndDispatch = function (req, res) {

  console.log("processHeadersAndDispatch:");
  console.log("req.url=" + req.url);

  // Note: This is handy, but verbose as it walks the object graph
  //console.log("Request: ObjectDumpBegin:\n\n");
  //console.log(req);
  //console.log("Request: ObjectDumpEnd:\n\n");

  if (req.url == '/smartpuxdata/data') {

      if (req.method != "POST") {
          console.log("/smartpuxdata/data method not POST\n");
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
          console.log("content-type != application/x-www.form-urlencoded");
          console.log("content-type=" + req.headers['content-type']);
          console.log(req.headers);
          sendError(req, res, 400, "content-type != application/x-www.form-urlencoded");
          return false;
      }

      //
      // read and process the input. A response is generated
      // when the input end event occurs.
      //
      // Note: All methods in which processing is handed off
      // to is responsible for a final response to the
      // request, error or not.
      //

      readAndProcessSimpleSensorExchange(req, res);

      // Fallthrough
  }
  else if (req.url.search('/smartpuxdata/dataapp') == 0) {

      //
      // dataapp URL is a REST request to either set the sensor's
      // cloud settings, or query its previously sent readings.
      //

      if (req.method == "POST") {
          readAndProcessSensorSettings(req, res);
          // Fallthrough
      }
      else if (req.method == "GET") {
          processSensorQuery(req, res);
          // Fallthrough
      }
      else {
          console.log("/smartpuxdata/dataapp unknown REST Method\n");
          sendError(req, res, 400, "Unknown request " + req.method);
          return false;
      }
  }

  //
  //
  // Automatic specialization of forms
  //
  // In the future an automatic lookup of "xxx.ocs" will load
  // sensorheader.html, xxx.ocs, sensorbody.html
  // .ocs stands for Openpux Client Specialization and is the javascript
  // configuration for the dynamically generated client side scripting form.
  //
  // xxx.oss would be Openpux Server Specialization if needed.
  //

  else if (req.url == '/') {
      // Serve the default file
      serveHtmlForm(req, res, "sensorheader.html", "sensorspecialization.js", "sensorbody.html");
  }
  else if (req.url == '/sensor') {
      serveHtmlForm(req, res, "sensorheader.html", "sensorspecialization.js", "sensorbody.html");
  }
  else if (req.url == '/humidor') {
      serveHtmlForm(req, res, "sensorheader.html", "humidorspecialization.js", "sensorbody.html");
  }
  else if (req.url == '/plantmonitor') {
      serveHtmlForm(req, res, "sensorheader.html", "plantmonitorspecialization.js", "sensorbody.html");
  }
  else if (req.url == '/drinkcooler') {
      serveHtmlForm(req, res, "sensorheader.html", "drinkcoolerspecialization.js", "sensorbody.html");
  }
  else if (req.url == '/galileo') {
      serveHtmlForm(req, res, "sensorheader.html", "galileospecialization.js", "sensorbody.html");
  }

  //
  // Client Library file
  //

  else if (req.url == '/openpuxclient.js') {
      serveFile(req, res, "openpuxclient.js");
  }

  //
  // These entries allow the openpux server and client files
  // to be served by the server itself using wget.
  //
  // This is handy for bootstrapping platforms such as RaspberryPI
  // or Intel Galileo.
  //
  // Note: Don't place any passwords in these files!
  //

  //
  // Server files
  //
  else if (req.url == '/openpux.js') {
      // Main server file
      serveFile(req, res, "openpux.js");
  }
  else if (req.url == '/memorystore.js') {
      // Memory storage support.
      serveFile(req, res, "memorystore.js");
  }

  //
  // Command line client for test/admin
  //
  else if (req.url == '/client.js') {
      // Command line client front end
      serveFile(req, res, "client.js");
  }

  //
  // Client side HTML templates
  //
  else if (req.url == '/sensorheader.html') {
      serveFile(req, res, "sensorheader.html");
  }
  else if (req.url == '/sensorbody.html') {
      serveFile(req, res, "sensorbody.html");
  }

  //
  // Client side specialization files for specific applications
  //
  else if (req.url == '/sensorpecialization.js') {
      serveFile(req, res, "sensorspecialization.js");
  }
  else if (req.url == '/humidorpecialization.js') {
      serveFile(req, res, "humidorspecialization.js");
  }
  else if (req.url == '/drinkcoolerspecialization.js') {
      serveFile(req, res, "drinkcoolerspecialization.js");
  }
  else if (req.url == '/plantmonitorspecialization.js') {
      serveFile(req, res, "plantmonitorspecialization.js");
  }
  else if (req.url == '/fencepostlightspecialization.js') {
      serveFile(req, res, "fencepostlightspecialization.js");
  }
  else if (req.url == '/galileospecialization.js') {
      serveFile(req, res, "galileospecialization.js");
  }

  //
  // Unrecognized request
  //

  else {
      console.log("Unknown URL: " + req.url + "\n");
      sendError(req, res, 400, "Unknown URL " + req.url);
      return false;
  }

  //
  // We have accepted the request, it is posted for async completion.
  //

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
        return;
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

    var retVal = updateSensorSettingsToStorage(o, function(error, result) {

        var returnBlock = new Object();

        if (error == null) {
            returnBlock.status = "200 OK";
        }
        else {
            returnBlock.status = "404 Not found" + error;
        }

        res.writeHead(200, {'Content-Type': 'application/json'});

        var jsonString = JSON.stringify(returnBlock);

        console.log(jsonString);

        res.write(jsonString);

        res.end();
    });

    if (retVal != 0) {
        // Error on submission
        console.log("readAndProcessSensorSettings: error submitting request to storage " + retVal);
        sendError(req, res, 400, "storage layer error=" + retVal);
        return;
    }

    return;
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
// This function is responsible for providing a response
// to the req, res stream regardless of error or not.
//
// Note: This function has comments about what is happening
// with the async request and lambda functions to better show
// what is happening under the covers with Node.js. The rest
// of the functions do not have this level of detail, unless
// required for clarity.
//
// Why Node.js?
//
// Note: In general Javascript and Node.js is a pretty advanced
//       system using leading edge concepts in computer science,
//       concurrency, and scaling. It's well suited to the type
//       of processing that occurs for web/http requests which
//       consist of moderate amounts of processing, waiting for I/O,
//       and parallel execution of (mostly) independent requests.
//
//       It does it in a way that the programmer does not have
//       to manage locking/synchronization, thread pools, request
//       throttling, etc. Of course the underlying system hosting
//       Node.js does, so Node.js can be looked at as a portable
//       mechanism to access an underlying web host platforms
//       power and scaleability, but without having to write
//       the complex scaling code yourself. This is its main power
//       and why many "simple" Node.js servers can outperform many
//       "native" or "Java" based servers.
//
//       Sure, there are many native (C/C++) and Java based server
//       implementations that have these patterns (Sun has many examples,
//       including processors designed to exploit the model with multithreaded
//       Sparc), but they are usually specific and tied to the container
//       and language. Node.js is the first (at least first popular) platform
//       to be transportable among these server implementations.
//
//       Some people may think the performance comes from Googles V8 Javascript
//       engine, but for Node.js is really about the language's performance
//       not getting in the way. V8 helps in this sense by significantly reducing
//       the tradtional gap between scripting and "compiled"
//       (is Java really compiled?, ok half compiled...) languages.
//
//       But if all Node.js did was to expose the existing programming models
//       available to C/C++/Java it would perform similar +- any gap still
//       remaining from V8 vs. native or JVM performance.
//
//       But this is not the case with Node.js, its a new async model which
//       is really what exposes the power of todays multi-core platforms.
//
//       So the way to think about Node.js performance and scaleability is
//       what power/scaleability is available on the platform underneath
//       that Node.js can exploit since its program representation is
//       in a form capable of exploiting this parallelism.
//
//       The source script does not encode a specific CPU instruction set
//       or compromise based portable/intermediate language/virtual machine
//       instruction set. This allows the target to compile based on its
//       most optimized patterns. For server side deployments compiling
//       the scripts and saving them for later re-use is a "no-brainer"
//       optimization. Adding Profile Guided Optimization (PGO) to
//       re-compile and re-cache over time based on actual execution history
//       is something an advanced platform can be expected to provide.
//
//       Since the programming pattern is fully async non-blocking its
//       adaptable to underlying systems that are either threaded, or
//       async. Each platform can choose what works best for it,
//       as programs in Node.js/Javascript do no impose a specific
//       threading and locking model. Without Node.js high performance
//       servers must invest heavily in multi-core scaling code which
//       in many cases is tied to narrow platform target definitions
//       such as 4 cores 4-8 cores, 4-16 cores, etc. Node.js allows
//       the configured runtime to decide the binding of Node.js
//       programs on a targeted machine.
//
//       It is expected that high performance Node.js implementations
//       would have runtimes optimized for such target machines.
//
//       And this all happens without re-designing the original
//       Node.js source program, something not available to todays
//       native or even Java implementations.
//

var processSensorQuery = function (req, res) {

    // /smartpuxdata/dataapp/REST/1/12345678/1/querydata?latestCount=1
    // http://www.smartpux.com/smartpuxdata/dataapp/REST/1/12345678/1/querydata?latestCount=1
    // http://localhost:8080/smartpuxdata/dataapp/REST/1/12345678/1/querydata?latestCount=1

    // http://localhost:8080/smartpuxdata/dataapp/REST/1/12345678/1/querydata?$format=json&latestCount=1
    // http://localhost:8080/smartpuxdata/dataapp/REST/account/passcode/SensorID/querydata?startDate=2013:01:01:00:00:00&endDate=2022:01:01:00:00:01

    //
    // sendError() completes processing of the Http stream and
    // generates a valid response to the client with the provided
    // errorCode and errorMessage.
    //

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

    //console.log(req.url);
    //console.log(tokens);

    var account = tokens[3];
    var passcode = tokens[4];
    var sensor = tokens[5];
    var cmd = tokens[6];  // 'querydata?$format=json&latestCount=1'

    //
    // Get data from the data store.
    //
    // Note: We use a lambda here instead of a separate function since
    // we need access to the req, res variables.
    //
    var result = queryLastReading(account, sensor, function(error, reading) {

        //
        // This is a lambda block which will only execute if result != 0
        // and contains the response from the storage layer, which could
        // include an error.
        //

        //
        // It has access to the outer scope local variables and
        // arguments even though it may execute before, or after
        // the enclosing method has finished.
        //

        //
        // Note: Neither function will execute at the same time,
        // such as on another CPU core. This is because Node.js is
        // designed to provide a concurrency safe single threaded
        // domain per request.
        //
        // Parallel processing is accomplished across multiple requests,
        // each with their own independent concurrency domains. This
        // makes the code in Node.js scripts "thread safe", or "concurrency safe".
        //

        //
        // For those interested in the low level details:
        //
        // The local variables in an enclosing function that are referenced
        // by a lambda block are promoted from the stack to a hidden object
        // instance (display class in C#). Both the enclosing function and
        // the lambda function reference these variables by pointer to their
        // entry in the display class object, not as local stack references.
        // This is because the enclosing function and the lambda can be on
        // completely unrelated stacks (or frames).
        //
        // The lifetime of the display class is determined by the standard
        // garbage collectors reference tracing and will remain live as
        // long as either local variables are refering to it, or the
        // object created to represent the lambda block function.
        //
        // Typically a lambda block functions object ends up on some
        // I/O queue for later processing while the enclosing function
        // returns to the caller. This is because Node.js is "non-blocking"
        // and fully async.
        //

        //
        // Our result is an Object an array of sensoreading
        // and result status in JSON format.
        //
        var returnBlock = new Object();

	returnBlock.sensorreading = new Array();

        if (error != null) {
   	    returnBlock.status = error;
        }
        else {
	    returnBlock.status = "200 OK";
        }

	//
	// Convert from an Array() of readings to an Object with members so that
	// our JSON format is what is expected.
	//
        if (reading != null) {
    	    returnBlock.sensorreading[0] = arrayToObject(reading);
        }

        //
        // 200 here represents we are provided a valid query response JSON
        // document though it may contain an error status from the storage layer.
        //
	res.writeHead(200, {'Content-Type': 'application/json'});

	var jsonString = JSON.stringify(returnBlock);

	res.write(jsonString);

	res.end();

        // There is an implied return here
    });

    //
    // Check if request submission failed. If it fails to even submit
    // the previous lambda block will not run.
    //
    // Note: ??? Is it Node.js pattern to always call the lambda
    // to unify error handling?
    //
    if (result != 0) {
        console.log("Error submitting request: account=" + account + " sensor=" + sensor);
        sendError(req, res, 400, "unknown storage error result=" + result);
        return;
    }

    //
    // Request has been successfully submitted, the previous lambda block
    // will handle final request disposition.
    //

    return;
}

var processSensorQueryResponse = function (reading) {

}

//
// This is the simple sensor exchange that uses a shorthand
// syntax as a x-www-form-urlencoded query string.
//
var readAndProcessSimpleSensorExchange = function (req, res) {

  console.log("readAndProcessSimpleSensorExchange invoked!");

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

    console.log("processSensorInputAndSendResponse:!");
    console.log('QueryString:' + queryString + ':\n');

    var sensorReadings = processQueryString(queryString);

    // The values will be dumped as JSON
    console.log(sensorReadings);

    var longFormReadings = convertFromShortForm(sensorReadings);

    console.log("Short Form:");
    console.log(sensorReadings);

    console.log("Long Form:");
    console.log(longFormReadings);

    var returnValue = addSensorReadingsToStorage(longFormReadings, function(error, result) {

        if (error != null) {
            console.log("storage update error " + error);
            sendError(req, res, 500, "storage update error=" + error);
            return;
        }

        // Now retrieve the settings, if any
        var result2 = querySensorSettingsFromStorage(longFormReadings, function(error2, sensorValues) {

            console.log("querySensorSettingsFromStorage response:");

            if (sensorValues != null) {
                console.log(sensorValues);
            }
            else {
                console.log("sensorValues == null");
            }

            sendSimpleSensorResponse(req, res, sensorValues);
        });
    });

    return returnValue;
}

var sendSimpleSensorResponse = function (req, res, sensorValues) {

  // http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_class_http_serverresponse
  // http://nodejs.org/dist/v0.10.28/docs/api/stream.html#stream_class_stream_writable

  // It's OK if there are no values set yet
  if (sensorValues == null) {
      console.log("No sensor values to send");
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

  console.log("responseString=" + responseString + "\n");

  res.end(responseString);

  return;
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

    if (settings["TargetMask4"] != null) {
        o.M4 = settings["TargetMask4"];
    }

    if (settings["TargetMask5"] != null) {
        o.M5 = settings["TargetMask5"];
    }

    if (settings["TargetMask6"] != null) {
        o.M6 = settings["TargetMask6"];
    }

    if (settings["TargetMask7"] != null) {
        o.M7 = settings["TargetMask7"];
    }

    if (settings["TargetMask8"] != null) {
        o.M8 = settings["TargetMask8"];
    }

    if (settings["TargetMask9"] != null) {
        o.M9 = settings["TargetMask9"];
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

    if (sensorReading["U"] != null) {
        o.SleepTIme = sensorReading["U"];
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
            // http://stackoverflow.com/questions/4101394/javascript-mime-type
            //contentType = 'text/javascript';
            contentType = 'application/javascript';
	    break;
	case '.css':
	    contentType = 'text/css';
	    break;
    }
    
    //console.log("sendFile: " + filePath + " contentType=" + contentType);

    // Generates warning, but works
    //path.exists(filePath, function(exists) {

    fs.exists(filePath, function(exists) {
	
	if (exists) {
            var s = fs.createReadStream(filePath);
            s.on('error', function() {
                res.writeHead(404);
                res.end();
            })

	    res.writeHead(200, { 'Content-Type': contentType });

            s.pipe(res);
	}
	else {
	    res.writeHead(404);
            res.end();
	}
    });
}

//
// Output an HTML header, then a specialization
// file (typically java script), then the main
// body.
//
var serveHtmlForm = function (
    req,
    res,
    htmlHeader,
    specialization,
    htmlBody
    ) {

    if (req.method != "GET") {
        res.writeHead(404);
        res.end();
        return;
    }

    var contentType = 'text/html';

    //
    // This assumes that htmlHeader, htmlBody exists as templates
    // The caller ensures this.
    //

    fs.exists(specialization, function(exists) {
	
	if (exists) {
            var specializationStream = fs.createReadStream(specialization);
            var htmlHeaderStream = fs.createReadStream(htmlHeader);
            var htmlBodyStream = fs.createReadStream(htmlBody);

            specializationStream.on('error', function() {
                res.writeHead(404);
                res.end();
            })

            htmlHeaderStream.once('fd', function() {
	        res.setHeader("Content-Type", contentType);
	        res.writeHead(200);
            });

            htmlHeaderStream.pipe(res, { end: false });

            htmlHeaderStream.on('end', function() {

	        specializationStream.pipe(res, { end: false });

                specializationStream.on('end', function() {

                    // This ends the chain
		    htmlBodyStream.pipe(res);
                    htmlBodyStream.on('end', function() {
                    });
                });
            });
	}
	else {
	    res.writeHead(404);
            res.end();
	}
    });
}

//
// Http Server
//

var http = require('http');

// http://nodejs.org/dist/v0.10.28/docs/api/http.html#http_http_createserver_requestlistener
//http.createServer(request_func).listen(8080, '127.0.0.1');
http.createServer(request_func).listen(8080, '0.0.0.0');

console.log('HttpTest Server version 3 running at http://0.0.0.0:8080/');
