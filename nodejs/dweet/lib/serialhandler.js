
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
// Handler for SerialPort npm package for node.js
//
// 12/19/2014
//

function SerialHandler(serialPortFactory, trace, traceerror) {

    // Factory class for SerialPort
    this.serialportFactory = serialPortFactory;

    this.moduleName = "SerialHandler";
    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerror) != "undefined") {
        this.traceerrorValue = traceerror;
    }

    //
    // This is deferred till open()
    //

    // Actual port instance
    this.serialport = null;
}

//
// Write data on serial port
//
SerialHandler.prototype.write = function(data, callback) {
    this.serialport.write(data, function(error, results) {
        callback(error, results);
    });
}

//
// Open serial port. Invoked from this.startSerialReader().
//
// callback(error, data);
//
//      error != null -> error as per Node.js pattern for callbacks
//
//      data != null -> data that has arrived
//
//      error == null && data == null -> First callback on successful open
//
SerialHandler.prototype.openInternal = function(portName, linemode, callback) {

    //
    // This does any require() to lookup the proper SerialPort
    // package for the given portName.
    //

    if (linemode) {

	//
	// This opens the readline parser. The default without the
	// object parameter is to open raw, which returns Buffer as data
	// which does not have CharCodeAt() and other string functions.
	//

        //
        // NOTE: Since we select either the npm SerialPort or our own
        // handlers we bring in the npm SerialPort parser regardless
        // to re-use its already implemented function.
        //
        // node_modules/serialport/parsers.js
        //
        var serialportModule = require('serialport');

        // Create an instance of the port from the factory
	this.serialport = new this.serialportFactory(portName,
		{ parser: serialportModule.parsers.readline("\n")});
    }
    else {

    	//
	// This opens the serialport in raw mode, which returns
	// Buffer as data.
	//
	// Buffer does not have CharCodeAt() and other string functions.
	//
	// You must use data.toString() or String.fromCharCode() to generate
	// a string from the raw binary data received.
	//
	this.serialport = new this.serialportFactory(portName);
    }

    //
    // Note: "this" is bound to SerialPort on event callbacks,
    // so must call using an outerscope variable
    //
    var serialHandler = this;

    this.serialport.on ('error', function(error) {

        // See comment above about "this" on callbacks
        serialHandler.traceerror("open open error=" + error);

        callback(error, null);
    });

    this.serialport.on ('open', function(error) {

        //
        // Note: In raw mode data is indicated as its
        // received from the serial port and this can cause
        // the display to be broken up vs. line mode.
        //
        serialHandler.serialport.on ('data', function(data) {

            // http://nodejs.org/api/buffer.html
            // data is a raw Buffer type

            //if (serialHandler.trace) {
                //dumpHexBuffer(data.toString());
                //console.log(data.toString());
            //}

            callback(null, data);
            return;

        }); // data event

        callback(error, null);
        return;

    }); // open event
}

//
// This opens the given serial port and starts
// a serial reader.
//
// callback(error, data);
//
// callback is only invoked if:
//
//   error != null, error on open
//
//   data != null, data arrival indication
//
//   data type is based on linemode.
//
//    linemode == true
//       string data buffer with character data
//
//    linemode == false
//       Buffer object with byte data
//       Buffer.toString() returns string data
//
//    callback(error, data);
//
//      error != null -> error as per Node.js pattern for callbacks
//
//      data != null -> data that has arrived
//
//      error == null && data == null -> First callback on successful open
//
// Consider: EventEmitter for above open indication?
//
SerialHandler.prototype.startSerialReader = function(portName, linemode, callback) {

    this.openInternal(portName, linemode, function(error, data) {

        if (error != null) {
            callback(error, null);
            return;
        }

        //
        // The first callback without data indicates that
        // the port has opened.
        //
        if (data == null) {
            // Serial port is now opened
            //console.log('Serial Port Opened');
            callback(null, null);
            return;
        }

        if (data != null) {

            //
            // if linemode == false:
            //
            // http://nodejs.org/api/buffer.html
            // data is a raw Buffer type
            //
            // if linemode == true:
            //
            // data is a string. .toString() still
            // works on a string allowing either buffer
            // to be handled the same way.
            //

            //dumpHexBuffer(data.toString());
            //console.log(data.toString());
            callback(null, data);
            return;
        }
    });
}

SerialHandler.prototype.setTrace = function(value) {
    this.trace = value;
}

SerialHandler.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

SerialHandler.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

SerialHandler.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(serialPortFactory, trace, traceerror) {
    moduleInstance = new SerialHandler(serialPortFactory, trace, traceerror);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
