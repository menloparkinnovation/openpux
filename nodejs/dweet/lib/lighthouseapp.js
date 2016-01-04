
/*
 * Copyright (C) 2015 Menlo Park Innovation LLC
 *
 * This is licensed software, all rights as to the software
 * is reserved by Menlo Park Innovation LLC.
 *
 * A license included with the distribution provides certain limited
 * rights to a given distribution of the work.
 *
 * This distribution includes a copy of the license agreement and must be
 * provided along with any further distribution or copy thereof.
 *
 * If this license is missing, or you wish to license under different
 * terms please contact:
 *
 * menloparkinnovation.com
 * menloparkinnovation@gmail.com
 */

//
// Application handler for LightHouse.
//
// 06/17/2015
//
// Each application type has an application handler module
// for application specific dweets and low level radio
// communications packets.
//
// It operates as a plug in for Menlo Dweet.
//

// http://nodejs.org/api/events.html
var events = require('events');

// http://nodejs.org/docs/latest/api/util.html
var util = require('util');

//
// Openpux/Smartpux client support.
//
// Note: The files openpuxclient.js, and menlohttprequest.js
//       are direct copies form the openpux project.
//
//       These files are normally downloaded to web browsers
//       dynamically, but a copy is placed locally.
//
//       TODO: Add support for a node.js based http: require()
//       that dynamically downloads client side API contract files.
//
//       This way projects such as this do not need to be
//       manually kept up to date.
//
//       A half way place would be to offer the openpuxclient.js
//       as an npm module.
//
//       Path from openpux project:
//
//       gitrepo/openpux/apps/openpux/client/javascripts/openpuxclient.js
//       gitrepo/openpux/apps/openpux/client/javascripts/menlohttpreuqest.js
//
//       They can also be retrieved by:
//
//       wget http://smartpux.com/openpuxclient.js
//       wget http://smartpux.com/menlohttprequest.js
//
//       Or the locally running version during development:
//
//       wget http://localhost:8080/openpuxclient.js
//       wget http://localhost:8080/menlohttprequest.js
//

//
// Input:
//
// options on input:
//
// options.trace
// options.traceerror
// options.apphandler
// options.config       // application config file such as lighthouse.json
// options.dweetclient
//
// options on output:
//   ...
//
function LightHouseApp(options) {

    var self = this;

    self.config = options;
    self.moduleName = "LightHouseApp";
    self.trace = false;
    self.traceerrorValue = false;

    if (typeof(self.config.trace) != "undefined") {
        self.trace = self.config.trace;
    }

    if (typeof(self.config.traceerror) != "undefined") {
        self.traceerrorValue = self.config.traceerror;
    }

    //
    // Load Openpux/Smartpux IoT cloud gateway support.
    //
    self.opclient = null;

    self.loadOpenpuxSupport();
}

//
// LightHouseApp is an EventEmitter
//

util.inherits(LightHouseApp, events.EventEmitter);

//
// config // from application config such as lighthouse.json
//
LightHouseApp.prototype.loadOpenpuxSupport = function() {

    var self = this;

    if (self.opclient != null) throw "double initialize";

    // Cloud parameters
    self.cloudHost = null;
    self.cloudAccount = null;
    self.cloudPassCode = null;
    self.cloudSensorID = null;

    // set initial ticket to null
    self.ticket = null;

    var appConfig = self.config.config;

    //console.log("appConfig=");
    //console.log(appConfig);

    if (typeof(appConfig.cloud_host) != "undefined") {
        self.cloudHost = appConfig.cloud_host;
    }
    else {
        console.log("no cloud_host configured in application.json, so Cloud sending is disabled");
        return;
    }

    if (typeof(appConfig.cloud_account) != "undefined") {
        self.cloudAccount = appConfig.cloud_account;
    }

    if (typeof(appConfig.cloud_passcode) != "undefined") {
        self.cloudPassCode = appConfig.cloud_passcode;
    }

    if (typeof(appConfig.cloud_sensor_id) != "undefined") {
        self.cloudSensorID = appConfig.cloud_sensor_id;
    }

    var opclientFactory = require('./openpuxclient.js');

    // Note: This will require('./menlohttprequest.js')
    self.opclient = new opclientFactory.OpenpuxClient();

    console.log("Openpux client support loaded");

    return;
}

//
// input: readings:
//
//  {
//     // Defines type of packet, who sent it, how to interpret
//     type: byte,
//     subType: byte,
//
//     light:         word,  // untranslated
//     battery:       float, // actual voltage in 0 - 3.3V range
//     solar:         float, // actual solar volatage in 0 - 3.3V range
//     moisture:      word,  // untranslated, app specific ranges, trigger
//     temperature:   float, // temperature in F
//     windspeed:     float, // windspeed in x.xx
//     winddirection: float, // winddirection in 0 - 360 degrees
//     barometer:     float, // kilopascals in x.xx format
//     rainfall:      float, // rainfail in x.xx inches
//     humidity:      float, // humidity in x.xx percent
//     fog:           word,  // untranslated
//     seastate:      word,  // untranslated
//     radiomonitor:  word,  // untranslated
//     userdata0:     word,  // untranslated
//     userdata1:     word   // untranslated
//  }
//
LightHouseApp.prototype.sendOpenpuxSensorReadings = function(readings) {

    var self = this;

    // No cloud provider configured
    if (self.opclient == null) {
        return;
    }

    var props = {};

    // Everything is transferred as strings in JSON
    props.light = readings.light.toString();
    props.battery = readings.battery.toString();
    props.solar = readings.solar.toString();
    props.moisture = readings.moisture.toString();
    props.temperature = readings.temperature.toString();
    props.windspeed = readings.windspeed.toString();
    props.winddirection = readings.winddirection.toString();
    props.barometer = readings.barometer.toString();
    props.rainfall = readings.rainfall.toString();
    props.humidity = readings.humidity.toString();
    props.fog = readings.fog.toString();
    props.seastate = readings.seastate.toString();
    props.radiomonitor = readings.radiomonitor.toString();
    props.userdata0 = readings.userdata0.toString();
    props.userdata1 = readings.userdata1.toString();

    // Scheme is unsecured http vs. https
    var scheme = "http:";

    var hostname = self.cloudHost;
    var port = null;

    var account  = self.cloudAccount;
    var token = self.cloudPassCode;
    var sensorID = self.cloudSensorID;

    // no http auth information
    var httpauthusername = null;
    var httpauthpassword = null;

    if (self.ticket == null) {

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

        self.ticket = self.opclient.createTicket(token, "/", host_args);
        if (self.ticket == null) {
            console.log("error creating ticket");
            return;
        }
    }

    var args = {
        AccountID: account,
        SensorID: sensorID,
        items: props
    };

    self.opclient.addSensorReading(self.ticket, args, function(error, response) {
        if (error != null) {
            console.log("error sending readings to cloud error=" + error);

            // openpuxclient.js returns an object with error details
            if (response != null) {
                dumpasjson(response);
            }

            return;
        }

        if ((response.status != 200) && (response.status != 201)) {
            console.log("error response from cloud server status=" + response.status);
            if (response.error != null) {
                console.log("error=" + response.error);
            }
        }
        else {
            console.log("response from cloud status=" + response.status);
        }
    });
}

var util = require('util');

function dumpasjson (ob) {

      // Dump data as JSON
      // null is full depth, default is 2
      //var inspectOptions = { showHidden: true, depth: null };
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      console.log(dumpdata);
}

//
// Register our event handlers
//
// Caller:
//
// dweetclient.js, loadHandlerModule()
//
LightHouseApp.prototype.register = function() {

    var self = this;

    //
    // Lookup the packetradio Dweet Handler which we use to
    // register for lighthouse packets.
    //
    self.packetradio = self.config.dweetclient.getDweetHandlerModule("packetradio");
    if (self.packetradio == null) {
        console.log("lighthouseapp: packetradio module not loaded, not packets will be received");
        return;
    }

    //
    // Setup to receive packet events from the packet radio
    //
    self.packetradio.on ('data', function(channel, packet) {
        self.processReceivedPacket(channel, packet);
    });
}

//
// Process a received packet.
//
// channel - object describing channel the packet is received on
//
// packet - binary Buffer type with raw packet data as received from the radio
//
LightHouseApp.prototype.processReceivedPacket = function(channel, packet) {

    //console.log("LightHouseApp.processReceivedPacket packet received!");

    //
    // MenloRadioSerial.h
    //
    // This defines the main envelope
    //
    // #define MENLO_RADIO_SERIAL_SENSORDATA 0xC2
    // #define MENLO_RADIO_SERIAL_SENSORDATA_OVERHEAD 1
    //
    // Note: By convention applications can use data[0]
    // (second byte) as a sub-type code for display of the format.
    //
    // These subtype defines assists in decoding packets
    //
    // Libraries/LightHouseApp.h
    // #define LIGHTHOUSE_SENSOR_DATA_SUBTYPE  0xFF
    //
    // Libraries/MotionLightApp.h
    // #define MOTIONLIGHT_SENSOR_DATA_SUBTYPE 0xFE
    //
    // struct MenloRadioSerialSensorData {
    //   uint8_t type;
    //   uint8_t data[31];
    // };
    //
    // #define MENLO_RADIO_SERIAL_SENSORDATA_SIZE sizeof(struct MenloRadioSerialSensorData)
    //

    if ((packet[0] & 0xC2) == 0xC2) {
        this.tracelog("LightHouseApp: LightHouse sensor data packet received");
    }
    else {
        this.tracelog("LightHouseApp: not LightHouse sensor data");
        return;
    }

    // Check that it is the LightHouse application subtype format
    var subType = packet[1];
    if (subType != 0xFF) {
        this.tracelog("LightHouseApp: not LightHouse packet subtype " + subType);
        return;
    }

    //
    // Unmarshall the binary packet data into data types
    //
    var sensorData = this.unmarshallLightHouseData(packet);

    //
    // Perform any data translations for scaling, ranging, and calibration.
    //
    var translated = this.translateLightHouseData(sensorData);

    //
    // Perform calibration.
    //
    var calibrated = this.calibrateLightHouseData(translated);

    //
    // Display LightHouse Data
    //
    this.displayLightHouseData(calibrated);

    //
    // Display Weather Data
    //
    this.displayWeatherData(calibrated);

    //
    // Send data to the IoT Openpux Cloud Gateway
    //
    this.sendToCloud(calibrated);
}

//
// TODO: Place this in a main place. It's ok to be here now
// as LightHouse Weather station is is the first end to end
// demonstration of the model.
//
// Smartpux Sensor Philosophy
//
// The packet radios used typically send short messages of 16 or 32 bytes
// with the compacted readings data. This is a time, space, and energy
// efficient form for battery powered sensors. Gateway applications
// are responsible for translating from this form into something
// usable.
//
// Not only is the reading data compacted into the binary buffer, but
// the value ranges are typically converted from float values within
// a specified domain into small, scaled integers.
//
// What does this mean?
//
// Encoding: Scaled Integers to represent fractional values
//
//  Take a windspeed which can vary from 0 - 100 MPH, and is accurate
//  to a tenth of an MPH. This would typically be represented by
//  a float/double so that values such as 0.1, 2.3, 5.0, 99.9 are
//  all valid.
//  
//  Since the sensors useful range is 0 - 100 MPH in tenths of a MPH,
//  its mathmatical domain is 0 - 99.9. Rather than encode this in a 32
//  bit (4 bytes) or 64 bit (8 bytes) float, its encoded into a single
//  16 bit (2 bytes) scaled integer.
//
//  In order to remove, but not lose the fractional part we multiply
//  the reading (at the sensor) by 10. If accuracy was in 100's of
//  an MPH, we would multiple by 100 and use an unsigned integer
//  of sufficient precision.
//
//  unsigned short sensordata = (unsigned short)((float)reading * (float)10);
//
//  Since the domain is 99.9 => 9999 is the maximum value. This requires
//  a 16 bit unsigned integer.
//
//  For 100's, it would be 99.9 => 99999 which would require a 32 bit
//  unsigned integer taking up four bytes. Note: that in this case
//  a 4 byte float could be used, but there could be translation
//  problems between the platforms from the embedeed device and its
//  compilers, and the Javascript based gateway applications. Sending
//  it as a portable ASCII string representation is OK for TCP/HTTP connected
//  higher power sensors, but would essentially fill the entire small radio
//  packet in this case, in addition to the additional firmware space on
//  the small sensor (32k code typically on an AtMega 328 (Arduino Uno)).
//
//  Ranging: Communicating the actual domain of a sensor
//
//  A barometric pressure sensor can give native readings from 70,000 -130,000.
//  This is a domain of 60,000 and fits within an unsigned 16 bit integer.
//  This is accomplished by subtracting the base of the range (70,000)
//  from the reading, and sending that. Note that scaling is not required
//  in this case since the small units used in its domain accommodate
//  the useful/practical precision from the sensor.
//
//  unsigned int sensordata = (unsigned short)((float)reading - (float)70,000);
//
//  Calibration: Calibration and fixups applied at the application layer.
//
//  Another aspect of a sensor is that it will either come with a calibration
//  chart, or this calibration may be done by testing against a known
//  reference. This calibration chart would typically be "burnt into firmware"
//  in a larger "smart sensor" implementation.
//
//  But since we are optimizing for low cost, low (code) space sensors,
//  its not expected that such calibration charts are applied to the sensors
//  firmware. This lifts the burden of not only its storage space in the
//  sensors tiny microcontroller, but its cpu time/energy to calculate
//  these calibrations. In addition is more user friendly to update
//  calibrations in the IoT cloud/web site rather than have to physically
//  go to the sensor and plug it into a programmer to reflash its firmware
//  calibrations.
//
//  So the data processed above from scaling and ranging is "raw" uncalibrated
//  readings from the sensor, and the application layer applies the calibration
//  values (as a json file).
//
//  Note: Sensors that are calibrated from the factory, or use the
//  traditional firmware based calibration tables just provide either no,
//  or a "null" 1:1 calibration as their readings (after encoding/scaling)
//  can be used directly.
//


//
// Since data is received as small, tightly encoded binary packets
// processing data is in three steps:
//
// 1) Pull raw data from packet into Javascript object with properties
//
//    This is an "unmarshalling" step
//
// 2) Perform any value translations, processing
//    - Values may be scaled and ranged.
//    - Optional calibration tables may be applied for a specific sensor.
//    - This may create new entries with calculated values
//
// 3) Convert to some format such as JSON, querysstring, CSV, etc.
//    for communication and optional display.
//

//
// Process raw data from the lighthouse into a Javascript object
//
//
// LightHouseApp.h
//
// This defines the specific application usage of the packet
//
// This is an overlay for MenloRadioSerialSensorData which
// is a general use application level radio packet form
//
/*
struct LightHouseSensorData {

  // 2 bytes
  uint8_t  type;          // [0]
  uint8_t  subType;       // [1] subtype for applications

  //
  // Multibyte types are stored LSB first as on the AtMega series.
  // Other big endian processors need to reverse the bytes.
  //
  // (These have more code space to accomodate)
  //

  // 16 bytes
  uint16_t light;         // [2]
  uint16_t battery;       // [4]    
  uint16_t solar;         // [6]
  uint16_t moisture;      // [8]
  uint16_t temperature;   // [10]
  uint16_t windspeed;     // [12]
  uint16_t winddirection; // [14]
  uint16_t barometer;     // [16]

  // 14 bytes
  uint16_t rainfall;      // [18]
  uint16_t humidity;      // [20]
  uint16_t fog;           // [22]
  uint16_t seastate;      // [24]
  uint16_t radiomonitor;  // [26]
  uint16_t userdata0;     // [28]
  uint16_t userdata1;     // [30]
};

#define LIGHTHOUSE_SENSOR_DATA_SUBTYPE  0xFF

#define LIGHTHOUSE_SENSOR_DATA_SIZE sizeof(struct LightHouseSensorData)

#define LIGHTHOUSE_SENSOR_DATA_PACKET   MENLO_RADIO_SERIAL_SENSORDATA
#define LIGHTHOUSE_SENSOR_DATA_OVEHEAD  MENLO_RADIO_SERIAL_SENSORDATA_OVERHEAD

*/

//
// Unmarshal the received packet as a raw binary Buffer into a
// Javascript object whose properties represent the binary values
// of the data items being communicated.
//
// Note: The raw sensor data as binary words are not translated or
//       calibrated here, just unmarshalled from an array of binary bytes
//       into the Javascript value representations as properties on
//       the returned object.
//
// output:
//
// Note: Though its a javascript object, the following definitions
//       are used:
//
//       byte - 8 bit unsigned value
//       word - 16 bit unsigned value
//
//  This packet is 32 bytes. It has two bytes for
//  header/type information, and 15 16 bit data words.
//
//  {
//
//     // Defines type of packet, who sent it, how to interpret
//     type: byte,
//     subType: byte,
//
//     // These are the raw 16 bit word readings, not as named values
//     readings: [15],
//
//     // These are the raw, untranslated named values
//     light: word,
//     battery: word,
//     solar: word,
//     moisture: word,
//     temperature: word,
//     windspeed: word,
//     winddirection: word,
//     barometer: word,
//     rainfall: word,
//     humidity: word,
//     fog: word,
//     seastate: word,
//     radiomonitor: word,
//     userdata0: word,
//     userdata1: word
//  }
//
LightHouseApp.prototype.unmarshallLightHouseData = function(packet) {

    var tmpWord;

    var readings = [];

    var sensorData = new Object();

    var packetType = packet[0];

    sensorData.type = packetType;

    // packet format subType
    var subType = packet[1];

    sensorData.subType = subType;

    // light
    tmpWord = (packet[3] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[2];    
    sensorData.light = tmpWord;
    readings.push(tmpWord);

    // battery
    tmpWord = (packet[5] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[4];    
    sensorData.battery = tmpWord;
    readings.push(tmpWord);

    // solar
    tmpWord = (packet[7] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[6];    
    sensorData.solar = tmpWord;
    readings.push(tmpWord);

    // moisture
    tmpWord = (packet[9] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[8];
    sensorData.moisture = tmpWord;
    readings.push(tmpWord);

    // temperature
    tmpWord = (packet[11] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[10];
    sensorData.temperature = tmpWord;
    readings.push(tmpWord);

    // windspeed
    tmpWord = (packet[13] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[12];
    sensorData.windspeed = tmpWord;
    readings.push(tmpWord);

    // winddirection
    tmpWord = (packet[15] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[14];
    sensorData.winddirection = tmpWord;
    readings.push(tmpWord);

    //console.log("winddirection: (raw)" + tmpWord);
    //console.log("windirection: packet[15]" + packet[15]);
    //console.log("windirection: packet[14]" + packet[14]);

    // barometer
    tmpWord = (packet[17] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[16];
    sensorData.barometer = tmpWord;
    readings.push(tmpWord);

    // rainfall
    tmpWord = (packet[19] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[18];
    sensorData.rainfall = tmpWord;
    readings.push(tmpWord);

    // humidity
    tmpWord = (packet[21] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[20];
    sensorData.humidity = tmpWord;
    readings.push(tmpWord);

    //console.log("humidity: (raw)" + tmpWord);
    //console.log("humidity: packet[21]" + packet[21]);
    //console.log("humidity: packet[20]" + packet[20]);

    // fog
    tmpWord = (packet[23] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[22];
    sensorData.fog = tmpWord;
    readings.push(tmpWord);

    // seastate
    tmpWord = (packet[25] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[24];
    sensorData.seastate = tmpWord;
    readings.push(tmpWord);

    // radiomonitor
    tmpWord = (packet[27] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[26];
    sensorData.radiomonitor = tmpWord;
    readings.push(tmpWord);

    // userdata0
    tmpWord = (packet[29] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[28];
    sensorData.userdata0 = tmpWord;
    readings.push(tmpWord);

    // userdata1
    tmpWord = (packet[31] << 8) & 0xFF00;
    tmpWord = tmpWord |= packet[30];
    sensorData.userdata1 = tmpWord;
    readings.push(tmpWord);

    sensorData.readings = readings;

    return sensorData;
}

//
// This routine transforms any raw data values received by the
// sensor into higher level values after applying the scaling
// and ranging specific to the sensor.
//
// input: Object with unmarshalled raw sensorData
//
//  {
//     // Defines type of packet, who sent it, how to interpret
//     type: byte,
//     subType: byte,
//
//     // These are the raw 16 bit word readings, not as named values
//     readings: [15],
//
//     // These are the raw, untranslated named values
//     light: word,
//     battery: word,
//     solar: word,
//     moisture: word,
//     temperature: word,
//     windspeed: word,
//     winddirection: word,
//     barometer: word,
//     rainfall: word,
//     humidity: word,
//     fog: word,
//     seastate: word,
//     radiomonitor: word,
//     userdata0: word,
//     userdata1: word
//  }
//
// output: new object with the following fields:
//
//  {
//     // Defines type of packet, who sent it, how to interpret
//     type: byte,
//     subType: byte,
//
//     light:         word,  // untranslated
//     battery:       float, // actual voltage in 0 - 3.3V range
//     solar:         float, // actual solar volatage in 0 - 3.3V range
//     moisture:      word,  // untranslated, app specific ranges, trigger
//     temperature:   float, // temperature in F
//     windspeed:     float, // windspeed in x.xx
//     winddirection: float, // winddirection in 0 - 360 degrees
//     barometer:     float, // kilopascals in x.xx format
//     rainfall:      float, // rainfail in x.xx inches
//     humidity:      float, // humidity in x.xx percent
//     fog:           word,  // untranslated
//     seastate:      word,  // untranslated
//     radiomonitor:  word,  // untranslated
//     userdata0:     word,  // untranslated
//     userdata1:     word   // untranslated
//  }
//
LightHouseApp.prototype.translateLightHouseData = function(sensorData) {

    //
    // Processed data has the same format as the originating class
    // so it can be displayed by the same functions.
    //
    var p = new Object();

    //
    // Process voltages, units, apply calibrations, etc.
    //

    p.type = sensorData.type;
    p.subType = sensorData.subType;

    //
    // We do not translate light sensor readings as they are
    // translated by host/application calibration tables based on
    // sensor type and voltage.
    //
    p.light = sensorData.light;

    //
    // Calculate battery voltage based on 10 bit A/D on a 3.3V reference
    //
    var bat_voltage =  (sensorData.battery / 1023) * 3.3;
    p.battery = bat_voltage;

    var solar_voltage =  (sensorData.solar / 1023) * 3.3;
    p.solar = solar_voltage;

    //
    // We do not translate moisture readings as they are calibrated
    // in host/application calibration tables based on type of soil, etc.
    //
    p.moisture = sensorData.moisture;

    // Temperature is in degrees F scaled by 100
    p.temperature = sensorData.temperature / 100;

    // Wind speed is 0 - nnn MPH scaled by 100
    p.windspeed = sensorData.windspeed / 100;

    // Wind Direction is 0 - 360 degrees and is scaled by 100
    p.winddirection = sensorData.winddirection / 100;

    // Barometer is in kilopascals scaled by 100
    p.barometer = sensorData.barometer / 100;

    // Rainfall is in inches scaled by 100
    p.rainfall = sensorData.rainfall / 100;

    // Humidity is in percent scaled by 100
    p.humidity = sensorData.humidity / 100;

    //
    // The following are not translated
    //
    p.fog = sensorData.fog;
    p.seastate = sensorData.seastate;
    p.radiomonitor = sensorData.radiomonitor;
    p.userdata0 = sensorData.userdata0;
    p.userdata1 = sensorData.userdata1;

    return p;
}

//
// Perform sensor specific calibrations.
//
// input: Object with translated sensorData
//
// output: Same object with calibrated fields.
//
LightHouseApp.prototype.calibrateLightHouseData = function(sensorData) {

    // This particular version of the application does not apply calibrations
    return sensorData;
}

//
// Display LightHouse portion of the translated data.
//
LightHouseApp.prototype.displayLightHouseData = function(sensorData) {

    // csv for now
    var separator = ",";

    var str = "LightHouse";
    str += separator;
    str += getDateTime();

    //
    // LightHouseSensorData from LightHouseApp.h
    //

    // light
    str += separator;
    str += "light="
    str += sensorData.light;

    // battery
    str += separator;
    str += "battery="
    str += sensorData.battery.toFixed(3);
    str += "V";

    // solar
    str += separator;
    str += "solar="
    str += sensorData.solar.toFixed(3);
    str += "V";

    // moisture
    str += separator;
    str += "moisture="
    str += sensorData.moisture;

    // temperature
    str += separator;
    str += "temperature="
    str += sensorData.temperature;

    console.log(str);
}

//
// Display WeatherStation portion of the translated LightHouse data.
//
LightHouseApp.prototype.displayWeatherData = function(sensorData) {
    var tmpWord;

    // csv for now
    var separator = ",";

    var str = "WeatherStation";
    str += separator;
    str += getDateTime();

    str += separator;
    str += "windspeed="
    str += sensorData.windspeed;

    str += separator;
    str += "winddirection="
    str += sensorData.winddirection;

    str += separator;
    str += "barometer="
    str += sensorData.barometer;

    str += separator;
    str += "rainfall="
    str += sensorData.rainfall;

    str += separator;
    str += "humidity="
    str += sensorData.humidity;

    console.log(str);

    str = "WeatherStationExtra";
    str += separator;
    str += getDateTime();

    str += separator;
    str += "fog="
    str += sensorData.fog;

    str += separator;
    str += "seastate="
    str += sensorData.seastate;

    str += separator;
    str += "radiomonitor="
    str += sensorData.radiomonitor;

    str += separator;
    str += "userdata0="
    str += sensorData.userdata0;

    str += separator;
    str += "userdata1="
    str += sensorData.userdata1;

    console.log(str);
}

LightHouseApp.prototype.sendToCloud = function(sensorData) {

    //
    // Place it into Smartpux format
    //

    this.sendOpenpuxSensorReadings(sensorData);
}

function getDateTime() {
    var now     = new Date(); 
    var year    = now.getFullYear();
    var month   = now.getMonth()+1; 
    var day     = now.getDate();
    var hour    = now.getHours();
    var minute  = now.getMinutes();
    var second  = now.getSeconds(); 
    if(month.toString().length == 1) {
        var month = '0'+month;
    }
    if(day.toString().length == 1) {
        var day = '0'+day;
    }   
    if(hour.toString().length == 1) {
        var hour = '0'+hour;
    }
    if(minute.toString().length == 1) {
        var minute = '0'+minute;
    }
    if(second.toString().length == 1) {
        var second = '0'+second;
    }   
    var dateTime = year+'/'+month+'/'+day+' '+hour+':'+minute+':'+second;   
     return dateTime;
}

LightHouseApp.prototype.setTrace = function(value) {
    this.trace = value;
}

LightHouseApp.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

LightHouseApp.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

LightHouseApp.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(config) {
    var moduleInstance = new LightHouseApp(config);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
