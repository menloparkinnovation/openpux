
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
    targetmask0,
    targetmask1,
    targetmask2,
    targetmask3
    )
{
    var s = buildRestSensorPath(accountid, passcode, sensorid);
    if (s == null) {
        return "{\"status\": \"400 BAD_REQUEST\"}";
    }

    var command = buildSetTargetMaskUrl(
        sleeptime,
        targetmask0,
        targetmask1,
        targetmask2,
        targetmask3
        );

    if (command == null) {
        return "{\"status\": \"400 BAD_REQUEST\"}";
    }

    s = s + command;

    var url = buildUrl(scheme, host, s);

    executePostRequest(url, httpauthusername, httpauthpassword, null, callback);

    return null;
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
    targetmask0,
    targetmask1,
    targetmask2,
    targetmask3
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

  if (!(targetmask0==null || targetmask0=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = targetmask0;
      reststring += "TargetMask0=";
      reststring += targetmask0;
  }

  if (!(targetmask1==null || targetmask1=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = targetmask1;
      reststring += "TargetMask1=";
      reststring += targetmask1;
  }

  if (!(targetmask2==null || targetmask2=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = targetmask2;
      reststring += "TargetMask2=";
      reststring += targetmask2;
  }

  if (!(targetmask3==null || targetmask3=="")) {
      if (anymaskset != null) {
          reststring += "&";
      }

      anymaskset = targetmask3;
      reststring += "TargetMask3=";
      reststring += targetmask3;
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

//
// Create an HTTP request object for different browsers
//
function createRequest() {
  var result = null;
  if (window.XMLHttpRequest) {

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
