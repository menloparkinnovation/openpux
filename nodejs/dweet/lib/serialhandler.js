
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

function SerialHandler(serialPortFactory, trace, traceerror)
{
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
    this.serialportReader = null;
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
SerialHandler.prototype.openInternal = function(portName, linemode, callback)
{
    var self = this;

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

        // *********
        // TODO: remove at somepoint.
        //
        // NOTE: Since we select either the npm SerialPort or our own
        // handlers we bring in the npm SerialPort parser regardless
        // to re-use its already implemented function.
        //
        // node_modules/serialport/parsers.js
        //
        //var serialportModule = require('serialport');

	//this.serialport = new this.serialportFactory(portName,
	//	{ parser: serialportModule.parsers.readline("\n")});

        // TODO: remove at somepoint.
        // *********

        //
        // Create an instance of the port from the factory
        //
        // 03/11/2018 this has changed on nodeserial 6.x
        // https://node-serialport.github.io/node-serialport/global.html#openOptions
        //

        // Make variables match example
        // 
        self.serialportFactory = null; // put a bomb in it to force interface updates.

        self.SerialPort = require('serialport');

        self.Readline = self.SerialPort.parsers.Readline;

	self.serialport = new self.SerialPort(portName);

        self.parser = new self.Readline();

        self.serialport.pipe(self.parser);

        //
        // This now requires read to be on the parser
        //

        self.serialportReader = self.parser;
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

        //
        // Keep the usage consistent. 03/11/2018.
        //

        self.serialportFactory = null; // put a bomb in it to force interface updates.

        self.SerialPort = require('serialport');

	self.serialport = new self.SerialPort(portName);

        self.serialportReader = self.serialport;
    }

    self.serialport.on ('error', function(error) {

        // See comment above about "this" on callbacks
        self.traceerror("open open error=" + error);

        callback(error, null);
    });

    self.serialport.on ('open', function(error) {

        //
        // Note: In raw mode data is indicated as its
        // received from the serial port and this can cause
        // the display to be broken up vs. line mode.
        //
        //self.serialport.on ('data', function(data) {

        //
        // 03/08/2018 nodeserial 6.x update.
        // Reader could be serialport (RAW) or parser (ReadLine).
        //
        // setup ensures that serialportReader points to the right
        // object.
        //
        self.serialportReader.on ('data', function(data) {

            // http://nodejs.org/api/buffer.html
            // data is a raw Buffer type

            //if (self.trace) {
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
