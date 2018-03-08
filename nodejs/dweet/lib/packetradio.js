
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
// Packet Radio handler for low level radio packets
//
// This module registers as a Dweet handler for R and T
// Dweets from attached packet radios.
//

//
// This module provides the interface to application specific handlers
// for these packets.
//

// http://nodejs.org/api/events.html
var events = require('events');

// http://nodejs.org/docs/latest/api/util.html
var util = require('util');

var myutil = require('./myutil.js');

//
// PacketRadio is an EventEmitter
//

//
//   options.trace
//   options.traceerror
//   options.apphandler
//   options.config
//   options.dweetclient
//
function PacketRadio(options) {

    this.options = options;
    this.dweetclient = options.dweetclient;

    this.moduleName = "PacketRadio";
    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(this.options.trace) != "undefined") {
        this.trace = this.options.trace;
    }

    if (typeof(this.options.traceerror) != "undefined") {
        this.traceerrorValue = this.options.traceerror;
    }
}

// PacketRadio is an EventEmitter for received packets
util.inherits(PacketRadio, events.EventEmitter);

//
// Register our application and event handlers
//
PacketRadio.prototype.register = function() {
}

//
// Register for R (packet radio receive) Dweets on the gateway Dweet Channel.
//
// dweetChannel - channel to receive packet radio Dweet's on.
//                instance of dweetclient.js
//
PacketRadio.prototype.registerForReceive = function(dweetChannel) {

    var self = this;

    self.tracelog("packetradio: register for receive");

    //
    // Register for packet radio receive Dweets
    //
    dweetChannel.dweet.on('R', function(name, value, prefix) {

        if (prefix != "$PDWT") {
            // Not for radio gateway
            self.tracelog("dweet received not for radio gateway");
            return;
        }

         self.tracelog("R dweet received");

        // This should be "R" based on our event registration
        if (name != "R") {
            var msg = "something is messed up name=" + name;
            throw msg;
        }

        if (value.length < 2) {
            self.traceerror("Radio Dweet Receive: Invalid value length");
            return;
        }

        // Channel marker
        if (value[1] != ':') {
            self.traceerror("Radio Dweet Receive: Invalid channel marker");
            return;
        }

        //
        // Process channel
        //
        var channel = new Object();

        if (value[0] == '0') {
            channel.portNumber = 0;
        }
        else {
            // TODO: Update for multi-channel support
            self.traceerror("Radio Dweet Receive: Invalid channel number " + value[0]);
            return;
        }

        value = value.substring(2, value.length);

        var packet = myutil.asciiHexStringToBinaryBuffer(value);

        self.tracelog("R calling processPacketFromRadio");

        //
        // Emit the event for the application handlers
        //
        self.emit('data', channel, packet);
    });
}

//
// Write a data packet out on the radio
//
// dweetChannel - channel to send Dweet's on to access packet radio
//                instance of dweetclient.js
//
// radioChannel - which channel to target over the radio
//
// data - binary Buffer[] packet data
//
// callback(error, result);
//
PacketRadio.prototype.write = function(dweetChannel, radioChannel, data, callback) {

    var self = this;

    //
    // send packet out radio with Radio Dweet
    //
    var asciiHex = myutil.binaryBufferToAsciiHexString(data);

    var args = new Object();

    args.prefix = "$PDWT";

    args.timeout = (2 * 1000);
    args.retryLimit = 4;

    args.command = "T";

    // T_REPLY=<radioChannel> will be matched with the reply
    args.object = radioChannel;

    args.value = asciiHex;

    args.compareObject = null;

    //
    // Note: this gets built by commandTransaction
    // args.fullCommand = "T=0:" + asciiHex;
    //  if radioChannel == "0"
    //

    // var dweet is an instance of dweethandler.js
    var dweet = dweetChannel.dweet;

    //
    // Send a sync since radio packet write is a lengthy command
    // and the Arduino may be processing something else, so we
    // send a "wakeup" first.
    //
    dweet.commandTransaction(args, function(sendError, o, result) {
        if (sendError != null) {
            self.traceerror("radioWriteFunction error=" + sendError);
        }
        callback(sendError, null);
    });

    return;
}

PacketRadio.prototype.setTrace = function(value) {
    this.trace = value;
}

PacketRadio.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

PacketRadio.prototype.consoleLog = function(message) {
    console.log(this.moduleName + ": " + message);
}

PacketRadio.prototype.consoleError = function(message) {
    console.error(this.moduleName + ": " + message);
}

PacketRadio.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

PacketRadio.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(options, trace, traceerrorValue) {
    moduleInstance = new PacketRadio(options, trace, traceerrorValue);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
