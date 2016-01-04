
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
// Note: Since this module loads independently of Dweet
// its responsible for its own libraries.
//

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

//
// LightHouseApp is an EventEmitter
//

function LightHouseApp(trace, traceerrorValue) {

    this.moduleName = "LightHouseApp";
    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }
}

// LightHouseApp is an EventEmitter
util.inherits(LightHouseApp, events.EventEmitter);

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
    // Perform any data translations
    //
    var translated = this.translateLightHouseData(sensorData);

    //
    // Display LightHouse Data
    //
    this.displayLightHouseData(translated);

    //
    // Display Weather Data
    //
    this.displayWeatherData(translated);
}

//
// Processing data is in three steps
//
// 1) Pull raw data from packet into Javascript object
//
// 2) Perform any value translations, processing
//    - This may create new entries with calculated values
//
// 3) Display in some format such as JSON, CSV, web query form, etc.
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
// Javascript object.
//
// Note: Raw sensor data is not translated or calibrated here, just
//       unmarshalled from an array of binary bytes into the Javascript
//       value representations.
//
LightHouseApp.prototype.unmarshallLightHouseData = function(packet) {
    var tmpWord;

    var sensorData = new Object();

    var packetType = packet[0];

    sensorData.type = packetType;

    // packet format subType
    var subType = packet[1];

    sensorData.subType = subType;

    // light
    tmpWord = (packet[3] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[2];    
    sensorData.light = tmpWord;

    // battery
    tmpWord = (packet[5] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[4];    
    sensorData.battery = tmpWord;

    // solar
    tmpWord = (packet[7] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[6];    
    sensorData.solar = tmpWord;

    // moisture
    tmpWord = (packet[9] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[8];
    sensorData.moisture = tmpWord;

    // temperature
    tmpWord = (packet[11] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[10];
    sensorData.temperature = tmpWord;

    // windspeed
    tmpWord = (packet[13] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[12];
    sensorData.windspeed = tmpWord;

    // winddirection
    tmpWord = (packet[15] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[14];
    sensorData.winddirection = tmpWord;

    // barometer
    tmpWord = (packet[17] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[16];
    sensorData.barometer = tmpWord;

    // rainfall
    tmpWord = (packet[19] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[18];
    sensorData.rainfall = tmpWord;

    // humidity
    tmpWord = (packet[21] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[20];
    sensorData.humidity = tmpWord;

    // fog
    tmpWord = (packet[23] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[22];
    sensorData.fog = tmpWord;

    // seastate
    tmpWord = (packet[25] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[24];
    sensorData.seastate = tmpWord;

    // radiomonitor
    tmpWord = (packet[27] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[26];
    sensorData.radiomonitor = tmpWord;

    // userdata0
    tmpWord = (packet[29] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[28];
    sensorData.userdata0 = tmpWord;

    // userdata1
    tmpWord = (packet[31] << 4) & 0xF0;
    tmpWord = tmpWord |= packet[30];
    sensorData.userdata1 = tmpWord;

    return sensorData;
}

//
// This routine transforms any raw data values received by the
// sensor into higher level values.
//
// Any application applied calibarations would be applied here
// as well.
//
// input: Object with unmarshslled raw sensorData
//
// output: Same Object with additional fields with translated data.
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

    // Wind Direction is 0 - 360 degrees and is not scaled
    p.winddirection = sensorData.winddirection;

    // Barometer is in kilopascals scaled by 100
    p.barometer = sensorData.barometer / 100;

    // Rainfall is in inches scaled by 100
    p.rainfall = sensorData.rainfall / 100;

    // Humidity is in percent scaled by 100
    p.humidity = sensorData.humidity;

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

function createInstance(trace, traceerrorValue) {
    moduleInstance = new LightHouseApp(trace, traceerrorValue);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
