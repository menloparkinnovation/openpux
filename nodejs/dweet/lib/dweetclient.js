
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
// DweetClient
//
// 01/02/2014
//

var dweetHandlerFactory = require('./dweethandler.js');

var myutil = require('./myutil.js');

//
// Dweet Client
//
// A Dweet client can be opened on a physical or virtual
// serial port.
//
// Properties:
//
//  portName
//  prefix
//  dweet  - dweethandler.js instance
//
// Methods:
//
//  createInstance
//  openPort
//  close
//  setTrace
//  setTraceError
//  tracelog
//  traceerror
//
// Internal: (don't call from outside)
//
//  DweetClient
//  openDweetSerialPort
//  openRadioSerial
//  openDweetTestDevice
//  receiveHandler
//  processReceivedData
//
function DweetClient(trace, traceerror, prefix) {
    
    this.moduleName = "DweetClient";

    this.prefix = prefix;

    this.portName = null;
    this.serialPortFactory = null;

    this.trace = trace;
    this.traceerrorValue = traceerror;

    this.buffer = null;

    // dweethandler.js
    this.dweet = dweetHandlerFactory.createInstance(trace, traceerror, prefix);

    // For detailed debugging
    this.dweet.setTrace(this.trace);
    this.dweet.setTraceError(this.traceerrorValue);

    this.dweetHandlers = null;

    this.appHandlers = null;

    //
    // Data properties added later:
    //
    // openDweetSerialPort()
    //   this.serialPortFactory
    //   this.portName
    //
    // openRadioSerial()
    //   this.dweetGatewayChannel
    //   this.serialPortFactory
    //   this.radioSerialFactory
    //   this.radioSerial
    //   this.packetradio
    //   this.portName
    //
    // openDweetTestDevice
    //   this.serialPortFactory
    //   this.testDevice
    //   this.virtualSerialModule
    //   this.virtualSerialFactory
    //   this.virtualSerial
    //   this.portName
    //
    // loadDweetHandlers()
    //   this.dweetHandlers
    //
    // loadAppHandlers()
    //   this.appHandlers
    //

    return this;
}

//
// dweetconsole.js, interactiveConsole  // this sets options
//   dweetconsole.js, openDweetClient
//     dweetclient.js, openPort         // this sets options.apphandler
//
// options:
//
// options.trace
// options.traceerror
// options.apphandler
// options.config       // application config file such as lighthouse.json
//
// Sets:
//
// options.dweetclient = this
//
// callback(error)
//
DweetClient.prototype.openPort = function(portName, options, openCallback) {

    var self = this;

    options.dweetclient = this;

    //
    // Load Configured Dweet Handlers
    //
    if (!self.loadDweetHandlers(options)) {
        openCallback("error loading configured Dweet Handlers");
        return;
    }

    //
    // Load Configured App Handlers
    //
    if (!self.loadAppHandlers(options)) {
        openCallback("error loading configured Application Handlers");
        return;
    }

    //
    // Each type of serial port requires unique setup, and
    // factory function for the SerialPort contract.
    //
    // This function determines which one to load based on the
    // portName prefix.
    //

    if (portName.indexOf("radio") == 0) {

        //
        // RadioSerial requires creation of the RadioSerial handler which
        // also creates a VirtualSerial instance to model a serial stream
        // over a small packet radio.
        //
        this.openRadioSerial(portName, options, openCallback);
    }
    else if (portName.indexOf("testdevice") == 0) {

        //
        // Test device requires creation of the device and VirtualSerial master.
        //
        this.openDweetTestDevice(portName, options, openCallback);
    }
    else {

        //
        // A hardware serial port handled through npm SerialPort package.
        //
        this.openDweetSerialPort(portName, options, openCallback);
    }
}

DweetClient.prototype.close = function() {

    // TODO:
    // this.dweet.openSerialPort -> this.dweet.close()
    // this.dweet -> dweethandler.js
    // this.portName
    // this.receiveCallback

    // If testdevice is opened:
    // this.testDevice
    // this.virtualSerialModule
    // this.virtualSerialFactory
    // this.virtualSerial
    //this.virtualSerial.on('error');
    //this.virtualSerial.on('data');
}

//
// Common receive data handler
//
DweetClient.prototype.receiveHandler = function(openError, readerData) {

    if (openError != null) {
	this.traceerror(openError);
	return;
    }

    //
    // function indicates reader data
    //
    if (readerData != null) {
	this.processReceivedData(readerData);
    }
}

//
// Open Dweet on a hardware serial port.
//
// This uses the npm SerialPort package.
//
// callback in invoked when data is indicated from the port.
//
// openCallback(error);
//
DweetClient.prototype.openDweetSerialPort = function(portName, options, openCallback) {

    //
    // If serialport is selected, then a real device with the
    // proper test firmware is expected. (Arduino).
    //

    // Get the factory for the SerialPort client side contract
    this.serialPortFactory = require("serialport").SerialPort;

    var captured_this = this;

    var receiveHandler = function(openError, readerData) {

        if (openError != null) {
            if (captured_this.tracerrror) console.error(openError);
            openCallback(openError);
            return;
        }
        else if (readerData != null) {

            //
            // function indicates reader data
            //
            // This supplies a whole received NMEA 0183 sentence that starts
            // with $ and ends with '\r''\n' to the NMEA 0183 handler.
            //
            // The serial port must be in line mode with '\n' as the delimeter.
            //
            captured_this.processReceivedData(readerData);
        }
        else {
            // error == null, data == null is fired on successful open

            // Set our port name
            captured_this.portName = portName;

            openCallback(null);
        }
    }

    // this.dweet == dweethandler.js
    this.dweet.openSerialPort(this.serialPortFactory, portName, receiveHandler);
}

//
// dweetconsole.js, interactiveConsole  // creates options
//   dweetconsole.js, openDweetClient
//     dweetclient.js, openPort         // this sets options.apphandler
//       dweetclient.js, openRadioSerial
//
// Input:
//
//   options.trace
//   options.traceerror
//   options.apphandler
//   options.config
//
// Output:
//
//   callback(error)
//
DweetClient.prototype.openRadioSerial = function(portName, options, openCallback) {

    var self = this;

    //
    // *** RadioSerial setup ***
    //

    if (options == null) {
        openCallback("options not set for dweet gateway channel");
        return;
    }

    if (options.dweetchannel == null) {
        openCallback("options.dweetchannel not set");
        return;
    }

    //
    // The radio gateway channel has extended timeout
    // and retry options.
    //
    self.dweet.setTimeoutBaseline(2 * 1000); // 2 seconds

    self.dweet.setRetryMultiplier(2);

    //
    // Save the Dweet Gateway channel used.
    //
    // This an instance of dweetlclient.js that is connected to the
    // Dweet channel that is the gateway to the radio. Typically this
    // is Dweet over a USB serial connection to a gateway board or
    // dongle hosting the packet radio.
    //
    // Invoked from:
    //
    //  dweetclient.js, radioGatewayCommandFunction()
    //
    self.dweetGatewayChannel = options.dweetchannel;

    // Get the factory for the SerialPort client side contract
    self.serialPortFactory = require("./radioserial.js").SerialPort;

    // Get the factory for the RadioSerial master side
    self.radioSerialFactory = require('./radioserial.js').RadioSerial;

    //
    // Lookup the packetradio module we will use
    //
    self.packetradio = self.getDweetHandlerModule("packetradio");
    if (self.packetradio == null) {
        openCallback("radioserial requires a packet radio module to be configured");
        return;
    }

    //
    // Setup the radioWrite function for radioserial
    //
    var radioWriteFunction = function(data, callback) {
        self.packetradio.write(self.dweetGatewayChannel, "0", data, callback);
        return;
    }

    // Create port in 'raw' mode with no parser.
    var radioSerialConfig = {};
    radioSerialConfig.portName = portName;
    radioSerialConfig.packetradio = self.packetradio;
    radioSerialConfig.trace = self.trace;
    radioSerialConfig.traceerror = self.traceerrorValue;
    radioSerialConfig.radioWriteFunction = radioWriteFunction;

    self.radioSerial = new self.radioSerialFactory(radioSerialConfig);

    self.radioSerial.on ('error', function(error) {
        // Error opening device
        openCallback(error);
        return;
    });

    self.radioSerial.on ('open', function(error) {

        if (error != null) {
            // Error opening device
            openCallback(error);
            return;
        }

        //
        // Note: radioSerial.on('data') is not used as its a
        // serial bytestream interface handled by RadioSerial.
        //
        // RadioSerial converts this serial bytestream into
        // a series of radio packets indicated on the radioWriteFunction()
        // above.
        //
    });

    //
    // Setup the packet radio to receive radio Dweets
    //
    self.packetradio.registerForReceive(self.dweetGatewayChannel);

    //
    // *** SerialPort setup ***
    //
    // The rest of the code here is the standard setup for a device
    // on a SerialPort instance. The RadioSerial provides a SerialPort
    // connection to the remote device over the radio with the above
    // supporting code.
    //

    // This indicates data on the SerialPort
    var receiveHandler = function(openError, readerData) {

        if (openError != null) {
            if (self.tracerrror) console.error(openError);
            openCallback(openError);
            return;
        }
        else if (readerData != null) {

            //
            // function indicates reader data
            //
            // This supplies a whole received NMEA 0183 sentence that starts
            // with $ and ends with '\r''\n' to the NMEA 0183 handler.
            //
            // The serial port must be in line mode with '\n' as the delimeter.
            //
            self.processReceivedData(readerData);
        }
        else {
            // error == null, data == null is fired on successful open

            // Set our port name
            self.portName = portName;

            openCallback(null);
        }
    }

    //
    // dweet is type dweethandler.js
    //
    // dweethandler.js
    //   serialhandler.js
    //
    self.dweet.openSerialPort(self.serialPortFactory, portName, receiveHandler);
}

//
// Open Dweet on a TestDevice
//
// callback in invoked when data is indicated from the device.
//
// openCallback(error);
//
DweetClient.prototype.openDweetTestDevice = function(portName, options, openCallback) {

    //
    // *** TestDevice/VirtualSerial setup ***
    //

    // Get the factory for the SerialPort client side contract
    this.serialPortFactory = require("./virtualserial.js").SerialPort;

    //
    // if TestDevice mode is selected a simulation with a device
    // is implemented.
    //

    //
    // Model:
    //
    // dweet: This is an instance of DweetHandler that
    //        communicates with a NMEA 0183 formatted device over
    //        a virtual serial connection.
    //
    // device: This is a software implementation of a device that
    //        responds to and sends Dweet commands over NMEA 0183.
    //

    //
    // Create the test device
    //
    // Since this is a test device we don't want to pull this in
    // on every server.
    //
    var testDeviceFactory = require('./DweetTestDevice.js');

    // Create an instance of the test device
    var device = testDeviceFactory.createInstance(this.trace, this.traceerrorValue,
                     this.throwOnError, this.prefix);

    this.testDevice = device;

    //
    // Create the VirtualSerial control side which operates similar to
    // a SerialPort contract.
    //
    this.virtualSerialModule = require('./virtualserial.js');

    this.virtualSerialFactory = this.virtualSerialModule.VirtualSerial;

    //
    // create port in readline mode to be compatible with expected
    // line oriented contract.
    //
    var serialportModule = require('serialport');

    this.virtualSerial = new this.virtualSerialFactory(portName,
		{ parser: serialportModule.parsers.readline("\n")});

    this.virtualSerial.setTrace(this.trace);
    this.virtualSerial.setTraceError(this.traceerrorValue);

    var captured_this = this;

    captured_this.virtualSerial.on ('error', function(error) {
        // Error opening device
        openCallback(error);
        return;
    });

    captured_this.virtualSerial.on ('open', function(error) {

        //
        // Now register our receive data handler
        //
        // This is data written to the SerialPort, which indicates
        // on VirtualSerial. In this case its device input data.
        //
        captured_this.virtualSerial.on ('data', function(data) {

            // sendData to device
            captured_this.testDevice.sendDataToDevice(data)
        });
    });

    //
    // Open the TestDevice.
    //
    this.testDevice.open(function(error, data) {

        //
        // Data indicated from the device that will be receive data
        // on SerialPort.
        //
        captured_this.virtualSerial.write(data, function(error, results) {
        });
    });

    //
    // *** SerialPort setup ***
    //
    // The rest here we treat the test device as if its on a serial port
    //

    // This indicates data on the SerialPort
    var receiveHandler = function(openError, readerData) {

        if (openError != null) {
            if (captured_this.tracerrror) console.error(openError);
            openCallback(openError);
            return;
        }
        else if (readerData != null) {

            //
            // function indicates reader data
            //
            // This supplies a whole received NMEA 0183 sentence that starts
            // with $ and ends with '\r''\n' to the NMEA 0183 handler.
            //
            // The serial port must be in line mode with '\n' as the delimeter.
            //
            captured_this.processReceivedData(readerData);
        }
        else {
            // error == null, data == null is fired on successful open

            // Set our port name
            captured_this.portName = portName;

            openCallback(null);
        }
    }

    //
    // This performs a require() on the VirtualSerial port module
    // from serialhandler().
    //
    // dweet is type dweethandler.js
    //
    this.dweet.openSerialPort(this.serialPortFactory, portName, receiveHandler);
}

//
// Data received from the device input stream.
//
// This supplies a whole received NMEA 0183 sentence that starts
// with $ and ends with '\r''\n' to the NMEA 0183 handler.
//
// A serial port in line mode with the delimeter '\n' performs this operation.
//
DweetClient.prototype.processReceivedData = function(data) {

    this.tracelog("DweetClient.processReceivedData: Received data=");
    this.tracelog(data.toString());

    this.dweet.processReceivedData(data);
}

//
// Load Dweet Handlers
//
// options.trace
// options.traceerror
// options.apphandler
// options.config       // application config file such as lighthouse.json
// options.dweetclient
//
DweetClient.prototype.loadDweetHandlers = function(options) {

    if (this.dweetHandlers != null) {
        // Already loaded
	this.traceerror("loadDweetHandlers: already loaded");
        return true;
    }

    if (options.config == null) {
        // Not configured
	this.traceerror("loadDweetHandlers: not configured");
        return true;
    }

    if (options.config.DweetHandlers.length == 0) {
        // Not configured
	this.traceerror("loadDweetHandlers: not configured, length = 0");
        return true;
    }

    this.dweetHandlers = options.config.DweetHandlers;

    var handlers = this.dweetHandlers;

    for (var index = 0; index < handlers.length; index++) {

        this.loadHandlerModule(options, handlers[index]);

        if (handlers[index].moduleInstance == null) {
            this.traceerror("loadDweetHandlers: could not load module");
            return false;
        }
    }

    return true;
}

//
// Return an already loaded Dweet Handler module by name.
//
DweetClient.prototype.getDweetHandlerModule = function(name) {

    if (this.dweetHandlers == null) {
        return null;
    }

    if (this.dweetHandlers.length == 0) {
        return null;
    }

    var handlers = this.dweetHandlers;

    for (var index = 0; index < handlers.length; index++) {

        if (handlers[index].name.search(name) == 0) {
            return handlers[index].moduleInstance;
        }
    }

    return null;
}

//
// Load Application Handlers
//
//   options.trace
//   options.traceerror
//   options.config       // application config file such as lighthouse.json
//
// dweetclient.js, DweetClient.openPort()
//
DweetClient.prototype.loadAppHandlers = function(options) {

    if (this.appHandlers != null) {
        // already loaded
        return true;
    }

    if (options.config == null) {
        // Not configured
        return true;
    }

    if (options.config.Applications.length == 0) {
        // Not configured
        return true;
    }

    this.appHandlers = options.config.Applications;

    var handlers = this.appHandlers;

    for (var index = 0; index < handlers.length; index++) {

        this.loadHandlerModule(options, handlers[index]);

        if (handlers[index].moduleInstance == null) {
            return false;
        }
    }

    return true;
}

//
// Return an already loaded App Handler module by name.
//
DweetClient.prototype.getAppHandlerModule = function(name) {

    if (this.appHandlers == null) {
        return null;
    }

    if (this.appHandlers.length == 0) {
        return null;
    }

    var handlers = this.appHandlers;

    for (var index = 0; index < handlers.length; index++) {

        if (handlers[index].name.search(name) == 0) {
            return handlers[index].moduleInstance;
        }
    }

    return null;
}

//
// Load the module
//
// It loads directly into the dweet!app Handlers[] so that modules that
// are loading can find currently loaded modules in the chain from
// Dweet Handlers to App Handlers.
//
// options:
//
// options.trace
// options.traceerror
// options.apphandler
// options.config       // application config file such as lighthouse.json
// options.dweetclient
//
// Caller:
//
// dweetclient.js, loadAppHandlers()
//
DweetClient.prototype.loadHandlerModule = function(options, entry) {

    try {
        entry.factory = require(entry.module);

        entry.moduleInstance = entry.factory.createInstance(options);

        // Tell the module to register its event handlers
        entry.moduleInstance.register();

     	this.tracelog("loadDweetHandlers: loaded module " + entry.module);
    }
    catch(e) {
        console.log("error loading module " + module + " e=" + e);
    }
}

DweetClient.prototype.setTrace = function(value) {
    this.trace = value;

    //
    // Set it on the dweet connection.
    //
    this.dweet.setTrace(this.trace);
}

DweetClient.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

DweetClient.prototype.consoleLog = function(message) {
    console.log(this.moduleName + ": " + message);
}

DweetClient.prototype.consoleError = function(message) {
    console.error(this.moduleName + ": " + message);
}

DweetClient.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

DweetClient.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(trace, traceerrorValue, prefix) {
    moduleInstance = new DweetClient(trace, traceerrorValue, prefix);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};

