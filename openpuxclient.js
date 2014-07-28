
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
//   Openpuxclient.js
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
// This allows the listed functions to be available from this
// module when not loaded in the browser, but run from node.js.
//
// This is used by the client.js utility, which is also used for
// unit and regression testing.
//

if (typeof window == "undefined") {

    //
    // We export our functions to Node.js
    // for command line testing.
    //

    // module is only available for Node.js
    module.exports = {
      querySensorReadings: function (
       scheme,
       host,
       httpauthusername,
       httpauthpassword,
       callback,
       accountid,
       passcode,
       sensorid,
       latestcount,
       startdate,
       enddate
      )
      {
        return querySensorReadings(
          scheme,
          host,
          httpauthusername,
          httpauthpassword,
          callback,
          accountid,
          passcode,
          sensorid,
          latestcount,
          startdate,
          enddate
	    );
      }
    };
}

function createLocalHttpRequest() {

    if (typeof window == "undefined") {
        var menlohttp = require('./menlohttprequest.js');
        return menlohttp.createHttpRequest();
    }

    return null;
}

//
// Query sensor readings.
//
// callback - Function to invoke with response JSON document
//
// If latestcount != null the specified count of most recent sensor readings
// is returned.
//
// If latestcount == null, the startdate and enddate must be supplied.
//
//
// The default return format is JSON.
//
// Return Value:
//
//   null - Request has been posted. Asynchronous callback function will
//          be invoked with JSON document with results or error.
//
//  !=null - Local error status inside a JSON document as string.
//
function querySensorReadings(
    scheme,
    host,
    httpauthusername,
    httpauthpassword,
    callback,
    accountid,
    passcode,
    sensorid,
    latestcount,
    startdate,
    enddate
    )
{
    var s = buildRestSensorPath(accountid, passcode, sensorid);
    if (s == null) {
        return "{\"status\": \"400 BAD_REQUEST\"}";
    }

    if (latestcount != null) {
        // /querydata?latestCount=1
        s = s + "/querydata?latestCount=" + latestcount;
    }
    else {
        if (startdate == null) return null;
        if (enddate == null) return null;

        // /querydata?startDate=2013:01:01:00:00:00&endDate=2022:01:01:00:00:01
        s = s + "/querydata?startDate=" + startdate;
        s = s + "&endtDate=" + enddate;
    }

    var url = buildUrl(scheme, host, s);

    executeGetRequest(url, httpauthusername, httpauthpassword, null, callback);

    return null;
}

//
// Update Sensor Target State
//
//  callback - Function to invoke with response JSON document
//
// Return Value:
//
//   null - Request has been posted. Asynchronous callback function will
//          be invoked with JSON document with results or error.
//
//  !=null - Local error status inside a JSON document as string.
//
function updateSensorTargetState(
    scheme,
    host,
    httpauthusername,
    httpauthpassword,
    callback,
    accountid,
    passcode,
    sensorid,
    sleeptime,
    items
    )
{
    var s = buildRestSensorPath(accountid, passcode, sensorid);
    if (s == null) {
        return "{\"status\": \"400 BAD_REQUEST\"}";
    }

    var command = buildSetTargetMaskUrl(sleeptime, items);
    if (command == null) {
        return "{\"status\": \"400 BAD_REQUEST\"}";
    }

    s = s + command;

    var url = buildUrl(scheme, host, s);

    executePostRequest(url, httpauthusername, httpauthpassword, null, callback);

    return null;
}

//
// Add Sensor Reading
//
//  callback - Function to invoke with response JSON document
//
// Return Value:
//
//   null - Request has been posted. Asynchronous callback function will
//          be invoked with JSON document with results or error.
//
//  !=null - Local error status inside a JSON document as string.
//
function addSensorReadingShortForm(
    scheme,
    host,
    httpauthusername,
    httpauthpassword,
    callback,
    items
    )
{
    var s = "/smartpuxdata/data";

    var shortForm = convertToShortForm(items);

    var sensorValuesString = generateSensorValues(shortForm);

    var url = buildUrl(scheme, host, s);

    executePostRequest(url, httpauthusername, httpauthpassword, sensorValuesString, callback);

    return null;
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

//
// Simple sensor exchange uses limited number of short form values.
//
var convertToShortForm = function (settings) {

    var o = new Object();

    if (settings["AccountID"] != null) {
        o.A = settings["AccountID"];
    }

    if (settings["PassCode"] != null) {
        o.P = settings["PassCode"];
    }

    if (settings["SensorID"] != null) {
        o.S = settings["SensorID"];
    }

    if (settings["Command"] != null) {
        o.C = settings["Command"];
    }

    if (settings["SleepTime"] != null) {
        o.T = settings["SleepTime"];
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

    if (settings["SensorReading0"] != null) {
        o.D0 = settings["SensorReading0"];
    }

    if (settings["SensorReading1"] != null) {
        o.D1 = settings["SensorReading1"];
    }

    if (settings["SensorReading2"] != null) {
        o.D2 = settings["SensorReading2"];
    }

    if (settings["SensorReading3"] != null) {
        o.D3 = settings["SensorReading3"];
    }

    if (settings["SensorReading4"] != null) {
        o.D4 = settings["SensorReading4"];
    }

    if (settings["SensorReading5"] != null) {
        o.D5 = settings["SensorReading5"];
    }

    if (settings["SensorReading6"] != null) {
        o.D6 = settings["SensorReading6"];
    }

    if (settings["SensorReading7"] != null) {
        o.D7 = settings["SensorReading7"];
    }

    if (settings["SensorReading8"] != null) {
        o.D8 = settings["SensorReading8"];
    }

    if (settings["SensorReading9"] != null) {
        o.D9 = settings["SensorReading9"];
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

    if (sensorReading["C"] != null) {
        o.Command = sensorReading["C"];
    }

    if (sensorReading["T"] != null) {
        o.SleepTime = sensorReading["T"];
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

//
// Build REST sensor object path
//
function buildRestSensorPath(
    accountid,
    passcode,
    sensorid
    )
{
    if (accountid==null || accountid=="") {
        return null;
    }

    if (passcode==null || passcode=="") {
        return null;
    }

    if (sensorid==null || sensorid=="") {
        return null;
    }

    // /smartpuxdata/dataapp/REST/account/passcode/SensorID
    var s = "/smartpuxdata/dataapp/REST/" + accountid + "/" + passcode + "/" + sensorid;

    return s;
}

function buildSetTargetMaskUrl(
    sleeptime,
    items
    )
{
  var reststring = null;

  // If any mask is set, we will POST
  var anymaskset = null;

  reststring = "/settargetmask?";

  if (!(sleeptime==null || sleeptime=="")) {
      anymaskset = sleeptime;
      reststring += "SleepTime=";
      reststring += sleeptime;
  }

  if (!(items.TargetMask0==null || items.TargetMask0=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask0;
      reststring += "TargetMask0=";
      reststring += items.TargetMask0;
  }

  if (!(items.TargetMask1==null || items.TargetMask1=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask1;
      reststring += "TargetMask1=";
      reststring += items.TargetMask1;
  }

  if (!(items.TargetMask2==null || items.TargetMask2=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask2;
      reststring += "TargetMask2=";
      reststring += items.TargetMask2;
  }

  if (!(items.TargetMask3==null || items.TargetMask3=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask3;
      reststring += "TargetMask3=";
      reststring += items.TargetMask3;
  }

  if (!(items.TargetMask4==null || items.TargetMask4=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.Targetmask4;
      reststring += "TargetMask4=";
      reststring += items.Targetmask4;
  }

  if (!(items.TargetMask5==null || items.TargetMask5=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask5;
      reststring += "TargetMask5=";
      reststring += items.TargetMask5;
  }

  if (!(items.TargetMask6==null || items.TargetMask6=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask6;
      reststring += "TargetMask6=";
      reststring += items.TargetMask6;
  }

  if (!(items.TargetMask7==null || items.TargetMask7=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask7;
      reststring += "TargetMask7=";
      reststring += items.TargetMask7;
  }

  if (!(items.TargetMask8==null || items.TargetMask8=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask8;
      reststring += "TargetMask8=";
      reststring += items.TargetMask8;
  }

  if (!(items.TargetMask9==null || items.TargetMask9=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = items.TargetMask9;
      reststring += "TargetMask9=";
      reststring += items.TargetMask9;
  }

  // If no masks are set, do not return a URL

  if (anymaskset == null) {
      return null;
  }

  return reststring;
}

//
// Build URL handling scheme and host path
//
function buildUrl(scheme, host, apppath) {
    var url = scheme + "://" + host + apppath;
    return url;
}

function createRequest() {
  var result = null;

  if (typeof window == "undefined") {

      //
      // If window is undefined we are not running inside a browser,
      // but as a client inside of a Node.js [test] program.
      //
      // So a local replacement for XMLHttpRequest is created.
      //
      result = createLocalHttpRequest();

      return result;
  }
  else if (window.XMLHttpRequest) {

    // FireFox, Safari, etc.
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

function executeGetRequest(
    url,
    httpauthusername,
    httpauthpassword,
    queryString,
    callback
    )
{
    // Create browser independent request
    var req = createRequest();
    if (req == null) {
        var responseText = "{\"status\": \"NULL HTTPXmlRequest. Unsupported Browser\"}";
        callback(responseText);
        return;
    }

    // Create the callback function and register it on the request object
    req.onreadystatechange = function() {

      if (req.readyState != 4) {
          // Not there yet
          return;
      }

      if (req.status != 200) {
        // Process failure status in JSON for main response path
        callback("{\"status\": \"" + req.status + " FAILED" + "\"}");
        return;
      }

      // Request successful, process the response
      callback(req.responseText);
    }

    var fullUrl = url;

    if (queryString != null) {
        fullUrl += queryString;
    }

    //
    // Now send it to the cloud server
    // open(method, url, async, user, password);
    //
    req.open("GET", fullUrl, true, httpauthusername, httpauthpassword);
    req.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

    req.send();

    // The above anonymous/lambda function will execute with the request results
}

function executePostRequest(
    url,
    httpauthusername,
    httpauthpassword,
    content_document,
    callback
    )
{
    // Create browser independent request
    var req = createRequest();

    // Create the callback function and register it on the request object
    req.onreadystatechange = function() {

      if (req.readyState != 4) {
          //alert("Not readyState 4: " + req.readyState + " Status=" + req.Status);
          return; // Not there yet
      }

      if (req.status != 200) {
        // Process failure status in JSON for main response path
        //alert("openpuxclient.js: executePostRequest: req.status failed " + req.status);
        callback("{\"status\": \"" + req.status + " FAILED" + "\"}");
        return;
      }

      // Request successful, process the response
      callback(req.responseText);
    }

    // Now send it to the cloud server
    // open(method, url, async, user, password);
    req.open("POST", url, true, httpauthusername, httpauthpassword);

    req.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

    if (content_document != null) {
        req.send(content_document);
    }
    else {
        req.send();
    }

    // The above anonymous/lambda function will execute with the request results
}

//
// Openpux Cloud Library
//
