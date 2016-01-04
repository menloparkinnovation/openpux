
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
// DeviceModel
//
// 02/02/2015
//
// This implemnents a "sensor" type of device closely
// modeled on the capabilities of an Arduino.
//

//
// This implements a device model.
//
// It can be used for node.js implementations of sensor devices,
// or provide a model for a real world device implemented in
// a lower level programming language such as C/C++ such as
// on an Arduino.
//
function DeviceModel(trace, traceerrorValue) {

    // Module support
    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }

    //
    // Standard device information
    //
    this.deviceModel = "DweetTestDevice";
    this.deviceName = "Device1";
    this.deviceVersion = "1.0";
    this.deviceFirmwareVersion = "1.0.1";
    this.deviceSerialNumber = "01-02-2015-12345678";

    //
    // This models 100 bytes of EEPROM configuration data
    //
    // Specific entries provide configuration and operating
    // modes for the device.
    //
    this.eeprom = new Array(100);

    //
    // This models the 2048 bytes of RAM in a typical
    // embedded device such as an Arduino
    //
    this.ram = new Array(2048);

    //
    // These values are exported to represent
    // configuration addresses in the EEPROM.
    //
    this.AlarmStateConfig = 90;
    this.LightStateConfig = 91;
    this.DoorStateConfig = 92;
    this.GarageDoorStateConfig = 93;

    //
    // These represent hardware I/O ports
    //

    // D0 - D13
    this.digitalPorts = new Array(14);

    // A0 - A5
    this.analogPorts = new Array(6);

    // PWM ports/settings
    // Note: Each entry is a PWM configuration object when set
    this.pwmPorts = new Array(4);

    // Timer0 - Timer2
    // Note: Each entry is a TIMER configuration object when set
    this.timers = new Array(3);

    //
    // Note: Many small devices may just supply a 16 bit
    // hex number to represent some quantity range
    // and expects the support infrastructure to
    // translate it against a calibration table.
    //
    // This keeps the device simple as a collection
    // of sensors.
    //
    // Devices that have onboard calibration and can
    // handle floating point may offer higher level values.
    //
    // The values choosen here are a representation of
    // the different possibilities.
    //
    // Note: Sending Dweets with floating point or difficult
    // to convert values may put pressure on the code + memory
    // space of limited devices. So the Dweet protocol at
    // the device exchange level may keep it simple using
    // values and ranges the device can use directly without
    // costly conversions.
    //
    // An intermediate level either at a gateway or cloud service
    // can perform any Dweet command to Dweetn command conversions
    // as required, such as using any locally available
    // calibration tables.
    //
    // Of course this conversion is expected to take place
    // with node.js on a device such as a RaspberryPi or
    // the cloud service.
    //
    // It's the designers call as to whether a device supports
    // "conversational mode" directly with human readable numbers,
    // or relies on intermediate services for their conversion/presentation.
    //
    // Of course an applications "App" or web page can do the conversions
    // and calibrations as well.
    //

    //
    // These are direct read values in decimal with floating point
    // representations where accuracy requires.
    //
    // This is the preferred Dweet form for "conversational mode"
    // devices which can support direct user interaction, as well
    // as programming by developers not familar to the embedded
    // space and leveling and conversion of A-D and binary/hex
    // values.
    //
    // Any device that represents itself using a higher level language
    // such as Node.js should use "conversational" values.
    //
    // It is recommended for low level embedded devices to do so
    // if their memory/code space allows for the conversions.
    //
    // These values though directly readable may require calibration
    // tables for accurate readings.
    //
    this.battery = 100; // decimal percent "BATTERY=100"
    this.batteryVoltage = 2.2; // floating point value "BATTERYVOLTAGE=2.2"
    this.solar = 70;    // percent "SOLAR=70"

    //
    // A raw Analog to Digital conversion value is sent untranslated
    // by the sensor.
    //
    // The deploying application understands the reference voltage,
    // response curve of the sensor to light, and the number of bits
    // of conversion.
    //
    // This could be an 8 bit, 10 bit, 12 bit, or 16 bit number.
    //
    // Note that "0x" is not required if the receiver of the
    // information knows its always sent in hex. 0x does remind
    // a human its a hex value.
    //
    // It could always pad the values as well, that is optional.
    //
    this.light = 0xA0;   // raw A to D value in hex "LIGHT=0xA0", or "LIGHT=A0"

    //
    // This is an 10 bit A-D reading of a voltage with
    // a range from 0 - 3.3v represented in a 16 bit hex value.
    //
    // voltage reading 2.6 out of 3.3v == 2.6/3.3 == 0.7878
    // 10 bit A-D value == (0.7878 * 1024) == 806
    // 16 bit hex value of 806 == 0x0326
    // 
    this.soil = 0x0326; // "SOIL=0x0326" or "SOIL=326" if th protocol assumes it.

    //
    // Temperature here is a scale from the sensors minimum
    // reading temperature (-40 degrees C) to its highest
    // (+160 degrees C). This is a range of 200.
    //
    // By scaling this 10x, this allow tenths of a degree
    // to be represented as an integer with no floating point.
    //
    // Values are sent in hex, since typically a binary
    // word is read directly from the sensor and transmitted.
    //
    // This type of scaling range sensor is common in embedded
    // systems. The example sensor here has its own voltage
    // reference and is considered an accurate, calibrated
    // sensor without additional table translations. An example
    // is the Dallas Semiconductor DS18B20 though read its
    // data sheet for its actual transmission range.
    //
    // 0x0   -> -40 degrees C
    //
    // 0x28  -> 0 degrees C
    //
    // 0x7D0 -> 160 degrees C
    //
    this.temp = 0x28;   // 0 degrees C "TEMP=028" assumed to be hex range 0 - 7D0
}

//
// Configuration support
//
// Configuration is typically in a small EEPROM for storing
// instance serial number, default operating mode, network
// keys and addresses, etc.
//

DeviceModel.prototype.getDeviceModel = function() {
    return this.deviceModel;
}

DeviceModel.prototype.getDeviceName = function() {
    return this.deviceName;
}

DeviceModel.prototype.getDeviceSerialNumber = function() {
    return this.deviceSerialNumber;
}

DeviceModel.prototype.getDeviceVersion = function() {
    return this.deviceVersion;
}

DeviceModel.prototype.getDeviceFirmwareVersion = function() {
    return this.deviceFirmwareVersion;
}

DeviceModel.prototype.getConfigUInt8 = function(address) {
    var value = this.eeprom[address];
    return value;
}

DeviceModel.prototype.getConfigUInt8 = function(address) {
    var value = 0;
    value = value | (this.eeprom[address] & 0xFF);
    return value;
}

DeviceModel.prototype.getConfigUInt16 = function(address) {
    var value = 0;

    value = value | (this.eeprom[address] & 0xFF);
    value = value | ((this.eeprom[address+1] << 8) & 0xFF);

    return value;
}

DeviceModel.prototype.getConfigUInt8 = function(address) {
    var value = 0;

    value = value | (this.eeprom[address] & 0xFF);
    value = value | ((this.eeprom[address+1] << 8) & 0xFF);
    value = value | ((this.eeprom[address+2] << 16) & 0xFF);
    value = value | ((this.eeprom[address+3] << 24) & 0xFF);

    return value;
}

DeviceModel.prototype.setConfigUInt8 = function(address, value) {
    this.eeprom[address] = value;
}

DeviceModel.prototype.setConfigUInt16 = function(address, value) {
    this.eeprom[address] = value & 0x00FF;
    this.eeprom[address+1] = (value >> 8) & 0x00FF;
}

DeviceModel.prototype.setConfigUInt32 = function(address, value) {
    this.eeprom[address] = value & 0x00FF;
    this.eeprom[address+1] = (value >> 8) & 0x00FF;
    this.eeprom[address+2] = (value >> 16) & 0x00FF;
    this.eeprom[address+3] = (value >> 24) & 0x00FF;
}

DeviceModel.prototype.setTrace = function(value) {
    this.trace = value;
}

DeviceModel.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

DeviceModel.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

DeviceModel.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(trace, traceerrorValue) {
    moduleInstance = new DeviceModel(trace, traceerrorValue);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
