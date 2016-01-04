
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

// http://nodejs.org/api/events.html
var events = require('events');

// http://nodejs.org/docs/latest/api/util.html
var util = require('util');

//
// Usage:
//
// var virtualSerialFactory = require('../virtualserial/virtualserial.js').VirtualSerial;
// new virtualSerialFactory.VirtualSerial(portName) - Registers portName
// VirtualSerial.write(data, callback(error, results)) - data becomes SerialPort.on('data', data_func(data));
// VirtualSerial.on('data', function(data) {}); - data from SerialPort.write(data, callback)
//
// var SerialFactory = require('../virtualserial/virtualserial.js').SerialPort;
// new SerialFactory.SerialPort(portName) - Finds portName registered from VirtualSerial(portName), or error
// SerialPort.write(data, callback(error, results)) - data becomes VirtualSerial.on('data', data_func(data));
// SerialPort.on('data', function(data) {}); - data from VirtualSerial.write(data, callback)
//

//
// VirtualSerial
//
// Provides a node.js "npm SerialPort" style contract.
//
// 03/03/2015
//
// VirtualSerial implements two main classes tied together similar to
// a Unix PTY.
//
// VirtualSerial -> This is the control interface in which writes become
// serial port data input, and serial port data output becomes data indications
// from from virtual serial.
//
// It models the SerialPort interface so programming is similar.
//
// SerialPort -> This models a sub set of the npm SerialPort package which
// operates similar to a serial port to applications.
//
// Usage/Contract:
//
// A VirtualSerial port handler is created by invoking:
//
// var virtualSerialFactory = require('./virtualserial.js').VirtualSerial;
// var virtualSerialPort = new VirtualSerial("portName");
//
// virtualSerialPort.on ('error', function(error) {
//     // Handle error
// });
//
// virtualSerialPort.on ('open', function(data) {
//
//     virtualSerialPort.on ('data', function(data) {
//           // write handler
//     });
//  });
//
// Then a normal serialport user can call:
//
// var serialportFactory = require('./virtualserial.js').SerialPort;
//
// Use the default raw parser:
//
// var port = new SerialPortFactory("portName");
//
// Or use one of SerialPorts parsers:
//
// var port = new SerialPortFactory("portName",
//        { parser: this.serialport.parsers.readline("\n")});
//
// port follows the SerialPort contract
//
function SerialPort(portName, options)
{
    this.moduleName = "SerialPort[VirtualSerial]";

    this.trace = false;

    this.traceErrorValue = false;

    this.lineMode = false;

    //
    // Default "raw" parser
    //
    this.parser = function(emitter, data) {
        this.emit('data', data);
    }

    //
    // See if the caller specified a parser
    //
    // This module accepts caller supplied parsers from SerialPort if supplied.
    //
    // port = new this.serialportFactory(portName,
    //        { parser: this.serialport.parsers.readline("\n")});
    //
    if (typeof(options) != "undefined") {
        if (typeof(options.parser) != "undefined") {
            if (options.parser != null) {
                this.parser = options.parser;
            }
        }
    }

    // Lookup port name
    if (module.Ports[portName] == null) {

        // Error
        this.emit('error', "port " + portName + " not found");
        return;
    }

    // Set our virtual serial handler
    this.virtualSerial = module.Ports[portName];

    // Tell the handler about us
    this.virtualSerial.announceSerialPort(this);
    
    // Set our tracing according to the master settings
    this.trace = this.virtualSerial.trace;
    this.traceerrorValue = this.virtualSerial.traceerrorValue;

    //
    // Announce ourselves to the application on the next turn.
    // This is required since the caller calls new() of our class
    // and then registers for the event on the result. If we emit
    // the event right away it will be missed.
    //
    var captured_this = this;
    setImmediate(function() {
        captured_this.emit('open', null);
    });
}

// SerialPort is an EventEmitter for open, error, and data events
util.inherits(SerialPort, events.EventEmitter);

SerialPort.prototype.setTrace = function(flag) {
    this.trace = flag;
}

SerialPort.prototype.setTraceError = function(flag) {
    this.traceerrorValue = flag;
}

SerialPort.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

SerialPort.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

//
// write data to the serial port
//
// callback(error, results)
//
SerialPort.prototype.write = function(data, callback) {

    //
    // The SerialPort contract provides for automatic conversion
    // of strings to Buffer[] binary data.
    //
    // http://nodejs.org/api/buffer.html
    //

    if (!Buffer.isBuffer(data)) {
        data = new Buffer(data);
    }

    this.tracelog("SerialPort.write(data)");
    this.tracelog(data);

    // This will raise a data event on VirtualSerial
    this.virtualSerial.announceDataFromSerial(data, function(error) {

        if (error != null) {
            callback(error, 0);
        }
        else {
            callback(null, data.length);
        }
    });
}

//
// This announces data arrival on the serialport from VirtualSerial.write().
//
// data is buffer type from // http://nodejs.org/api/buffer.html
//
// callback(error)
//
SerialPort.prototype.announceDataFromVirtualSerial = function(data, callback) {

    this.tracelog("SerialPort.announceFromVirtualSerial");

    // The parser will perform the data emit on our behalf
    this.parser(this, data);

    callback(null);
}

//
// portName is the port name that will be found by SerialPort.open()
//
// writeFunction is invoked with data that was written to the serial port
//
// Use the default raw parser:
//
// var port = new VirtualSerialFactory("portName");
//
// Or use one of SerialPorts parsers:
//
// var serialportModule = require('serialport');
// var port = new VirtualSerialFactory("portName",
//        { parser: serialportModule.parsers.readline("\n")});
//
function VirtualSerial(
    portName,
    options
    )
{
    this.moduleName = "VirtualSerial[VirtualSerial]";

    this.trace = false;

    this.traceErrorValue = false;

    this.portName = portName;

    // Our serial port pairing is not determined yet
    this.serialport = null;

    this.trace = false;
    this.traceerrorValue = false;

    var captured_this = this;

    //
    // Default "raw" parser
    //
    this.parser = function(emitter, data) {
        this.emit('data', data);
    }

    //
    // See if the caller specified a parser
    //
    // This module accepts caller supplied parsers from SerialPort if supplied.
    //
    // port = new this.serialportFactory(portName,
    //        { parser: this.serialport.parsers.readline("\n")});
    //
    if (typeof(options) != "undefined") {
        if (typeof(options.parser) != "undefined") {
            if (options.parser != null) {
                this.parser = options.parser;
            }
        }
    }

    //
    // Add to the module hashmap of port names to allow lookup
    // of the virtual serial handler.
    //
    if (typeof(module.Ports) == "undefined") {
        module.Ports = new Object();
    }

    if (module.Ports[this.portName] != null) {

        setImmediate(function() {
            captured_this.emit('open', "Name Already Defined");
        });

        return;
    }

    module.Ports[this.portName] = this;

    //
    // Announce ourselves to the application on the next turn.
    // This is required since the caller calls new() of our class
    // and then registers for the event on the result. If we emit
    // the event right away it will be missed.
    //
    var captured_this = this;
    setImmediate(function() {
        captured_this.emit('open', null);
    });
}

//
// VirtualSerial is an EventEmitter for open, error, and data events
// just like SerialPort.
//
util.inherits(VirtualSerial, events.EventEmitter);

VirtualSerial.prototype.setTrace = function(flag) {
    this.trace = flag;
}

VirtualSerial.prototype.setTraceError = function(flag) {
    this.traceerrorValue = flag;
}

VirtualSerial.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

VirtualSerial.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

//
// write data to the virtual serial port which becomes
// read data on SerialPort.
//
// callback(error, results)
//
VirtualSerial.prototype.write = function(data, callback) {

    this.tracelog("VirtualSerial.write(data)");

    //
    // The SerialPort contract provides for automatic conversion
    // of strings to Buffer[] binary data.
    //
    // http://nodejs.org/api/buffer.html
    //

    if (!Buffer.isBuffer(data)) {
        data = new Buffer(data);
    }

    if (this.serialport != null) {
        this.serialport.announceDataFromVirtualSerial(data, function(error) {
            
            if (error != null) {
                callback(error, 0);
            }
            else {
                callback(null, data.length);
            }
        });
    }
    else {
        // No listener
        callback("no listener", 0);
    }
}

//
// This announces the serialport paired with this virtual
// serial instance.
//
VirtualSerial.prototype.announceSerialPort = function(serialport) {
    this.serialport = serialport;
}

//
// This is write data from the serialport that is raised
// as a data event on VirtualSerial.
//
// callback(error)
//
VirtualSerial.prototype.announceDataFromSerial = function(data, callback) {

    this.tracelog("VirtualSerial.announceDataFromSerial(data)");
    this.tracelog(data);

    // The parser will perform the data emit on our behalf
    this.parser(this, data);

    callback(null);
}

module.exports = {

  // We export the same name as npm SerialPort
  SerialPort: SerialPort,

  // VirtualSerial support
  VirtualSerial: VirtualSerial
};
