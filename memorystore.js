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
//   memorystorage.js
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
// In memory storage support
//

//
// This is the contract for openpux storage providers.
//
// The contract is async as real I/O can be required
// for storage providers.
//
// Each function returns 0 if it was successfully queued
// and the callback will be invoked.
//
// Callback is invoked when the results are complete with
// their final results, which may contain a storage system error.
//
// Callback arguments:
//
// error == null with no errors, string otherwise.
//
// result depends on the request, typically a storage document.
//
// callback(error, result)
//
module.exports = {

  querySettings: function (itemsArray, callback) {
	return querySensorSettingsFromStorage(itemsArray, callback);
  },

  addReadings: function (itemsArray, callback) {
        return addSensorReadingsToStorage(itemsArray, callback);
  },

  queryLastReading: function (account, sensor, callback) {
        return queryLastReading(account, sensor, callback);
  },

  queryLastReadings: function (account, sensor, readingCount, callback) {
        return queryLastReadings(account, sensor, readingCount, callback);
  },

  updateSensorSettings: function (itemsArray, callback) {
	return updateSensorSettingsToStorage(itemsArray, callback);
  },

  dumpReadings: function (callback) {
	return dumpSensorReadingsTable(callback);
  },

  dumpSettings: function (callback) {
	return dumpSensorSettingsTable(callback);
  }
};

var SensorReadingsTable = new Array();

var SensorSettingsTable = new Array();

var dumpSensorReadingsTable = function(callback) {

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

    // No results here
    callback(null, null);

    return 0;
}

var dumpSensorSettingsTable = function(callback) {

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

    // No results here
    callback(null, null);

    return 0;
}

//
// Returns 0 on successful submission. A callback will be generated
// error or not.
//
// Returns != 0 if a submission request occurs and a callback will
// not be invoked.
//
// The callback function is provided:
//
// callback(error, result)
//

var querySensorSettingsFromStorage = function(itemsArray, callback) {

    // itemsArray['AccountID'] == Account
    // itemsArray['Password'] == PassCode
    // itemsArray['SensorID'] == Sensor

    var account = itemsArray['AccountID'];
    if (account == null) {
        console.log("missing AccountID");
        callback("missing AccountID", null);
        return 0;
    }

    var sensor = itemsArray['SensorID'];
    if (sensor == null) {
        console.log("missing SensorID");
        callback("missing SensorID", null);
        return 0;
    }

    var sensors = SensorSettingsTable[account];
    if (sensors == null) {
        callback("no sensors present for account", account);
        return 0;
    }

    var settings = sensors[sensor];
    if (settings == null) {
        callback("no setting present for sensor", sensor);
        return 0;
    }

    callback(null, settings);

    return 0;
}

//
// Returns 0 on successful submission. A callback will be generated
// error or not.
//
// Returns != 0 if a submission request occurs and a callback will
// not be invoked.
//
// The callback function is provided:
//
// callback(error, result)
//

var addSensorReadingsToStorage = function(itemsArray, callback) {

    // itemsArray['AccountID'] == Account
    // itemsArray['PassCode'] == PassCode
    // itemsArray['SensorID'] == Sensor

    var account = itemsArray['AccountID'];
    if (account == null) {
        console.log("missing AccountID");
        callback("missing AccountID", null);
        return 0;
    }

    var sensor = itemsArray['SensorID'];
    if (sensor == null) {
        console.log("missing SensorID");
        callback("missing SensorID", null);
        return 0;
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
    //dumpSensorReadingsTable();

    callback(null, null);

    return 0;
}

//
// Returns 0 on successful submission. A callback will be generated
// error or not.
//
// Returns != 0 if a submission request occurs and a callback will
// not be invoked.
//
// The callback function is provided:
//
// callback(error, result)
//
var queryLastReading = function(account, sensor, callback) {
    
    var sensors = SensorReadingsTable[account];
    if (sensors == null) {
        console.log("queryLastReading: null sensors for account=" + account);
        callback("404 Not found", account);
        return 0;
    }

    var readings = sensors[sensor];
    if (readings == null) {
        console.log("queryLastReading: null readings for account=" + account + " sensors=" + sensors);
        callback("404 Not found", sensor);
        return 0;
    }

    console.log("queryLastReading: readings=");
    console.log(readings[readings.length - 1]);

    var reading = readings[readings.length - 1];
    if (reading == null) {
        callback("404 Not found", "No readings");
        return 0;
    }

    callback(null, reading);

    return 0;
}

//
// Returns 0 on successful submission. A callback will be generated
// error or not.
//
// Returns != 0 if a submission request occurs and a callback will
// not be invoked.
//
// The callback function is provided:
//
// callback(error, result)
//
var queryLastReadings = function(account, sensor, readingCount, callback) {
    
    var sensors = SensorReadingsTable[account];
    if (sensors == null) {
        console.log("queryLastReading: null sensors for account=" + account);
        callback("404 Not found", account);
        return 0;
    }

    var readings = sensors[sensor];
    if (readings == null) {
        console.log("queryLastReading: null readings for account=" + account + " sensors=" + sensors);
        callback("404 Not found", sensor);
        return 0;
    }

    var returnSet = readings.slice(readings.length - readingCount);
    if (returnSet == null) {
        callback("404 Not found", "No readings");
        return 0;
    }

    callback(null, returnSet);

    return 0;
}

//
// Returns 0 on successful submission. A callback will be generated
// error or not.
//
// Returns != 0 if a submission request occurs and a callback will
// not be invoked.
//
// The callback function is provided:
//
// callback(error, result)
//
var updateSensorSettingsToStorage = function(itemsArray, callback) {

    // itemsArray['AccountID'] == Account
    // itemsArray['PassCode'] == PassCode
    // itemsArray['SensorID'] == Sensor

    var account = itemsArray['AccountID'];
    if (account == null) {
        console.log("missing AccountID");
        callback("missing AccountID", null);
        return 0;
    }

    var sensor = itemsArray['SensorID'];
    if (sensor == null) {
        console.log("missing SensorID");
        callback("missing SensorID", null);
        return 0;
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
    //dumpSensorSettingsTable();

    callback(null, null);

    return 0;
}
