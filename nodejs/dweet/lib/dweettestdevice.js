
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
// Simulate a Dweet Device
//
// 12/29/2014
//

// http://nodejs.org/api/events.html
var events = require('events');

// http://nodejs.org/docs/latest/api/util.html
var util = require('util');

var nmeaFactory = require('./nmea0183.js');

var dm = require('./devicemodel.js');

//
//
// DweetTestDevice
//
// The device uses a device model to simulate real
// world sensors with different data formats for testing.
//
// Similar to a real device, it provides the following:
//
// 1) A configurable update interval (default 30 seconds) in which
//    it sends it current state of its sensors as a Dweet.
//
// 2) It responds to commands which may arrive at anytime.
//
// 3) It provides device configuration store such as EEPROM
//    in which device operating modes, charactersitics, and
//    calibrations may be updated.
//

//
// Implementation of a Dweet Device.
//
// This device responds to pre-configured commands
// used for testing the eventloop of Dweet.
//
// Also a code example of a node.js Dweet device implementation.
//
// DweetTestDevice is an EventEmitter.
//
// Calling processSentence() will raise an event for each unique commnad=value
// in sentence.
//
// See processSentence/processCommand for details.
// 
function DweetTestDevice(trace, traceerrorValue, throwOnError, prefix) {
    this.moduleName = "DweetTestDevice_ModuleName";

    // A Dweet device is a NMEA 0183 device
    this.prefix = "$PDWT";

    this.trace = false;
    this.traceerrorValue = false;
    this.throwOnError = false;

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }

    if (typeof(thrownOnError) != "undefined") {
        this.throwOnError = throwOnError;
    }

    if (typeof(prefix) != "undefined") {
        this.prefix = prefix;
    }

    //
    // Create an instance of our device model which talks to our
    // simulates the applicatins hardware.
    //
    this.deviceModel = dm.createInstance(this.trace, this.traceerrorValue);

    this.deviceToCallerDataIndication = null;

    this.nmea = nmeaFactory.createInstance(this.trace, this.traceerrorValue, this.prefix);
}

// DweetTestDevice is an EventEmitter for received Dweet commands
util.inherits(DweetTestDevice, events.EventEmitter);

DweetTestDevice.prototype.onNMEAReceiveSentence = function(eventName, error, sentence, o) {

    // o is the parsed sentence

    if (error != null) {
        this.traceerror("onNMEAReceiveSentence: error=" + error);
        return;
    }

    // if o.error != null the event emitter has set error != null above

    if (!o.checksumOK) {
        this.traceerror("onNMEAReceiveSentence: bad checksum on sentence=" + o.sentence);
        return;
    }

    this.processSentence(o);
}

//
// Setup the command events we are going to process.
//
DweetTestDevice.prototype.setupCommandEvents = function() {

    var captured_this = this;

    // Receives all NMEA 0183 sentences, error or not
    this.nmea.on('receive', function(eventName, error, sentence, parsed_sentence) {
        captured_this.onNMEAReceiveSentence(eventName, error, sentence, parsed_sentence);
    });

    //
    // Register to received this devices higher level Dweet
    // command events
    //
    this.on('GETCONFIG', function(name, value, prefix) {
        captured_this.getConfigHandler(name, value, prefix);
    });


    // Setup optional debug package
    this.setupDebugPackage();

    // Setup the conversational package
    this.setupConversationalPackage();
}

//
// This sets up a default conversational package
//
DweetTestDevice.prototype.setupConversationalPackage = function() {

    //
    // Must capture the device objects "this" since callbacks
    // bind "this" to the event emitter.
    //
    var myDevice = this;

    this.on('GETCONFIG', function(name, value, prefix) {
        myDevice.getConfigHandler(name, value, prefix);
    });

    this.on('SETCONFIG', function(name, value, prefix) {
        myDevice.setConfigHandler(name, value, prefix);
    });
}

//
// This sets up the optional debug package.
//
// It's used here to allow low level access to device state.
//
DweetTestDevice.prototype.setupDebugPackage = function() {

    //
    // Note: In this example PGM, MSR, and IO space are
    // not registered for but have similar patterns.
    //

    //
    // Must capture the device objects "this" since callbacks
    // bind "this" to the event emitter.
    //
    var myDevice = this;

    //myDevice.on('GETCONFIG', function(name, value, prefix) {
    //    myDevice.defaultHandler(name, value, prefix);
    //});
}

//
// This is the default handler.
//
DweetTestDevice.prototype.defaultHandler = function(name, value, prefix) {
    var logString = "defaultHandler " + prefix + " " + name + "=" + value;
    this.traceerror(logString);
}

DweetTestDevice.prototype.getConfigHandler = function(name, value, prefix) {
    var logString = "getConfigHandler " + prefix + " " + name + "=" + value;
    var data;
    var response = "GETCONFIG_REPLY=" + value + ":";
    var args = new Object();

    // Create args object
    var args = new Object();
    args.prefix = "$PDWT";
    args.timeout = 10 * 1000;
    args.retryLimit = 1;

    // fill in the command values
    args.command = null;
    args.object = null;
    args.value = null;

    // We don't have a compare object for the responses
    args.compareObject = null;

    args.fullCommand = null;

    args.error = null;
    args.result = null;
    args.retryCount = 0;
    args.fullReply = null;
    args.nmeaReplySentence = null;

    if (prefix != "$PDWT") {
        // not a Dweet command
        logString += " unknown Dweet";
        this.traceerror(logString);
        if (this.throwOnError) throw logString;
        return;
    }

    this.tracelog(logString);

    if (value == "MODEL") {
        data = this.deviceModel.getDeviceModel();
        response += data;
    }
    else if (value == "NAME") {
        data = this.deviceModel.getDeviceName();
        response += data;
    }
    else if (value == "SERIAL") {
        data = this.deviceModel.getDeviceSerialNumber();
        response += data;
    }
    else if (value == "VERSION") {
        data = this.deviceModel.getDeviceVersion();
        response += data;
    }
    else if (value == "FIRMWAREVERSION") {
        data = this.deviceModel.getDeviceFirmwareVersion();
        response += data;
    }
    else if (value == "LIGHT") {
        data = this.deviceModel.getConfigUInt8(this.deviceModel.LightStateConfig);
        if (data == 0) {
            response += "OFF";
        }
        else {
            response += "ON";
        }
    }
    else if (value == "ALARM") {
        data = this.deviceModel.getConfigUInt8(this.deviceModel.AlarmStateConfig);
        if (data == 0) {
            response += "OFF";
        }
        else {
            response += "ON";
        }
    }
    else if (value == "DOOR") {
        data = this.deviceModel.getConfigUInt8(this.deviceModel.DoorStateConfig);
        if (data == 0) {
            response += "CLOSED";
        }
        else {
            response += "OPEN";
        }
    }
    else {
        logString += " unknown GETCONFIG";
        this.traceerror(logString);
        if (this.throwOnError) throw logString;
        return;
    }

    args.fullCommand = response;

    //
    // Now queue the response
    //
    this.queueDweetToHost(args);
}

//
// SETCONFIG= is device specific.
//
DweetTestDevice.prototype.setConfigHandler = function(name, value, prefix) {

    var logString = "setConfigHandler " + prefix + " " + name + "=" + value;

    if (prefix != "$PDWT") {
        // not a Dweet command
        logString += " unknown Dweet";
        this.traceerror(logString);
        if (this.throwOnError) throw logString;
        return;
    }

    this.tracelog(logString);

    // "SETCONFIG=ALARM:ON"
    if (value == "ALARM:ON") {
        this.deviceModel.setConfigUInt8(this.deviceModel.AlarmStateConfig, 1);
    }
    else if (value == "ALARM:OFF") {
        this.deviceModel.setConfigUInt8(this.deviceModel.AlarmStateConfig, 0);
    }
    else if (value == "LIGHT:ON") {
        this.deviceModel.setConfigUInt8(this.deviceModel.LightStateConfig, 1);
    }
    else if (value == "LIGHT:OFF") {
        this.deviceModel.setConfigUInt8(this.deviceModel.LightStateConfig, 0);
    }
    else if (value == "DOOR:OPEN") {
        this.deviceModel.setConfigUInt8(this.deviceModel.DoorStateConfig, 1);
    }
    else if (value == "DOOR:CLOSED") {
        this.deviceModel.setConfigUInt8(this.deviceModel.DoorStateConfig, 0);
    }
    else if (value == "GARAGEDOOR:OPEN") {
        this.deviceModel.setConfigUInt8(this.deviceModel.GarageDoorStateConfig, 1);
    }
    else if (value == "GARAGEDOOR:CLOSED") {
        this.deviceModel.setConfigUInt8(this.deviceModel.GarageDoorStateConfig, 0);
    }
    else {
        logString += " unknown SETCONFIG";
        this.traceerror(logString);
        if (this.throwOnError) throw logString;
        return;
    }
}

//
// Device Stream I/O Model
//
// Similar to PTY's on Unix systems a virtual serial port interface
// is modeled. A caller of the DweetTestDevice provides the actual
// serial data transport and indicates it to us at its leasure.
//
// At the same time a caller who opens this device provides a callback
// handler so that the device may indicate serial data streams to it.
//
// DweetTestDevice.send(data)
//
//   This is similar to serial.write() and is the data stream
//   of bytes generated from the device.
//
//   It invokes this.deviceToCallerDataIndication(null, data)
//
//   which is a callback supplied by the caller to DweetTestDevice.open(callback)
//
//   It represents the serial byte stream from the device.
//
// DweetTestDevice.receive(data)
//
//   This is similar to serial.read() and is the data stream
//   arriving to the device.
//
// DweetTestDevice.sendDataToDevice(data)
//
//   This allows the data transport handler to inject data as
//   DweetTestDevice.receive(data) indications as if it arrived
//   from the serial port.
//
//   Similar to a PTY, "send" here becomes "receive" to the virtual
//   serial port for the device.
//

//
// This is from the perspective of the device
// and the function the device uses to communicate
// data that it sends.
//
// For example a real device will send data on
// a serial port to the remote Dweet listener.
//
DweetTestDevice.prototype.send = function(data) {

    var testDevice = this;

    // Process at the top of the event loop
    setImmediate(function() {
        testDevice.deviceToCallerDataIndication(null, data);
    });
}

//
// This is from the perspective of the device
// and the function on the device that indicates
// received Dweet commands.
//
// On a real device this will be called from
// the serial read handler when a completed
// line is available.
//
DweetTestDevice.prototype.receive = function(data) {

    //
    // We pass this to the NMEA receive handler which
    // will emit the receive event we registered for
    // at setup.
    //
    this.nmea.processReceivedSentence(data);
}

//
// callbackOuter is invoked when data indicates on the device.
//
// callbackOuter(error, data);
//
DweetTestDevice.prototype.open = function(callbackOuter) {

    this.deviceToCallerDataIndication = callbackOuter;

    var myDevice = this;

    //
    // Setup the writeHandler for nmea that sends bytes out
    // the virtual serial port as a result of nmea writes.
    //
    myDevice.nmea.setWriteHandler(function(data, callbackInner) {
        var length = 0;

        if (data != null) {
            length = data.length;
            myDevice.send(data);
        }

        callbackInner(null, length); // writeHandler results callback
    });

    // Setup the command events we will process
    this.setupCommandEvents();
}

//
// This allows a caller to send data to the device
//
DweetTestDevice.prototype.sendDataToDevice = function(data) {

    //
    // Note: Since we use setImmediate() to send the data
    // "this" gets rebound on the callback to "Object" from
    // "DweetTestDevice".
    //
    // So we must capture our "this" into a lambda variable
    // and supply it on the callback.
    //
    // This is one of the more annoying aspects of Javascript
    // over Java, C# languages in response to "delegates".
    //
    // We also capture data in the lambda rather than supplying
    // it as an additional argument to setImmediate().
    //

    var testDevice = this;

    // Process at the top of the event loop
    setImmediate(function() {
        testDevice.receive(data);
    });
}

//
// Queue a Dweet to the host
//
DweetTestDevice.prototype.queueDweetToHost = function(args) {
    var myDevice = this;
    myDevice.nmea.nmeaSender(args, function(error) {
        myDevice.nmea.flush(args, function(errorflush) {
        });
    });
}

//
// Worker functions that parse a NMEA 0183 sentence into a
// series of command=arguments pairs.
//
// arguments may further be passed into name:value as well.
//
// This is a Dweet pattern, not specific to NMEA 0183.
//
// Note: Use of dweethandler.js by Dweet clients and applications
// handle this automatically. At this level we are implementing
// a Dweet device in node.js.
//
// TODO: Separate this into a more formal dweetdevice.js library
// for Dweet device implementers, vs. Dweet client's.
//
// See devicemodel.js for a work in progress.
//
//
// Split to name/value pair
//
// Input: command=value
//
// Output: nv.name  = "command"
//         nv.value = "value"
//
// Input: command
//
// Output: nv.name = "command"
//         nv.value = null
//
DweetTestDevice.prototype.splitCommandValue = function(str) {
    var o = new Object();

    if (str.indexOf('=') == (-1)) {
        // treat as a single entry
        o.name = str;
        o.value = null;
        return o;
    }

    var nv = str.split('=');

    o.name = nv[0];
    o.value = nv[1];
    return o;
}

//
// Process individual commands within the sentence as events
//
DweetTestDevice.prototype.processSentence = function(o) {
    var nv = null;

    for (var index = 0; index < o.commands.length; index++) {
        nv = this.splitCommandValue(o.commands[index]);
        this.processCommand(o, nv);
    }
}

DweetTestDevice.prototype.processCommand = function(o, nv) {
    this.emit(nv.name, nv.name, nv.value, o.prefix);
}

DweetTestDevice.prototype.setTrace = function(value) {
    this.trace = value;
}

DweetTestDevice.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

DweetTestDevice.prototype.tracelog = function(message) {
    if (this.trace) {
       console.log(this.moduleName + ": " + message);
    }
}

DweetTestDevice.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(trace, traceerrorValue, throwOnError, prefix) {
    moduleInstance = new DweetTestDevice(trace, traceerrorValue, throwOnError, prefix);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
