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
// Each function returns true or false if it was successfully
// queued.
//
// Callback is invoked when the results are complete.
//
module.exports = {

  querySettings: function (itemsArray, callback) {
        var returnValue;
	returnValue = querySensorSettingsFromStorage(itemsArray);
        callback(returnValue);
        return 1;
  },

  addReadings: function (itemsArray, callback) {
        var returnValue;
        returnValue = addSensorReadingsToStorage(itemsArray);
        callback(returnValue);
        return 1;
  },

  queryLastReading: function (account, sensor, callback) {
        var returnValue;
        returnValue = queryLastReading(account, sensor);
        callback(returnValue);
        return 1;
  },

  queryLastReadings: function (account, sensor, readingCount, callback) {
        var returnValue;
        returnValue = queryLastReadings(account, sensor, readingCount);
        callback(returnValue);
        return 1;
  },

  updateSensorSetting: function (itemsArray, callback) {
        var returnValue;
	returnValue =  updateSensorSettingsToStorage(itemsArray);
        callback(returnValue);
        return 1;
  },

  dumpReadings: function (callback) {
        var returnValue;
	returnValue = dumpSensorReadingsTable();
        callback(returnValue);
        return 1;
  },

  dumpSettings: function (callback) {
        var returnValue;
	returnValue = dumpSensorSettingsTable();
        callback(returnValue);
        return 1;
  }
};

var SensorReadingsTable = new Array();

var SensorSettingsTable = new Array();

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
    //dumpSensorReadingsTable();

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
