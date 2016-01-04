
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
// Usage:
//
// - Packet data received from radio which becomes serial input
//   processReceivedPacket(channel, packet)
//
// - Serial write data that becomes packet(s) sent to the radio
//   through radioWrite(data, callback(error) {}
//

//
// RadioSerial
//
// Provides serial port operations over small packet radios.
//
// 03/03/2015
//
// First byte of the packet is encoded as follows:
// (From MenloRadioSerial.h in Smartpux/Arduino tree)
//
// type_and_size byte
//
// The upper 2 bits (7 + 6) define the packet type.
//
// Bit 5 is a sequence number
//
// The 5 lower bits 4 - 0 indicate packet size.
//
// The upper 2 bits set (0xC0 - 0xFF) are reserved as packet types
//
// #define PACKET_TYPE_MASK(t) (t & 0xC0)
// #define PACKET_SEQUENCE_MASK(t) (t & 0x20)
// #define PACKET_SIZE_MASK(t) (t & 0x1F)
//

//
// Use the VirtualSerial module to provide the serial port contract
// and its control side interface.
//

// http://nodejs.org/api/events.html
var events = require('events');

// http://nodejs.org/docs/latest/api/util.html
var util = require('util');

var dh = require("./dumphex.js").createInstance();

var virtualSerialModule = require('./virtualserial.js');

var myutil = require("./myutil.js");

//
// config.portName
// config.packetradio
// config.trace
// config.traceerror
//
function RadioSerial(config)
{
    var self = this;

    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(config.trace) != "undefined") {
        this.trace = config.trace;
    }

    if (typeof(config.traceerror) != "undefined") {
        this.traceerrorValue = config.traceerror;
    }

    this.portName = config.portName;

    this.packetradio = config.packetradio;

    this.radioWriteFunction = config.radioWriteFunction;

    //
    // This is the sequence of the next packet we expect to receive.
    //
    this.receiveSequence = false;

    //
    // This is the sequence of the next packet we will transmit.
    //
    this.transmitSequence = false;

    this.moduleName = "radioserial";

    //
    // Create the VirtualSerial control side which operates similar to
    // a SerialPort contract.
    //

    this.virtualSerialFactory = virtualSerialModule.VirtualSerial;

    // create port in "raw" mode with no parser
    this.virtualSerial = new this.virtualSerialFactory(this.portName);

    this.virtualSerial.setTrace(this.trace);
    this.virtualSerial.setTraceError(this.traceerrorValue);

    self.virtualSerial.on ('error', function(error) {

        //
        // Error opening device
        //
        // Note: VirtualSerial has already done a SetImmediate() to ensure
        // the error does not occur during the initial setup call.
        //
        self.emit('error', null);
        return;
    });

    self.virtualSerial.on ('open', function(open_error) {

        if (open_error) {

            // Error opening device
            self.emit('error', open_error);
            return;
        }

        // Now register our receive data handler
        self.virtualSerial.on ('data', function(data) {

            // sendData to radio
            self.internalSendDataToRadio(data, function(senddata_error) {

                //
                // Swallow the error. If we emit an 'error' status
                // it can cause abandonment of the connection. Serial
                // connections are allowed to lose data without
                // abandonment.
                //
                if (senddata_error) {
                    self.errorlog("RadioSerial.internalSendDataToRadio error " + senddata_error);
                    //self.emit('error', sendata_error);
                }
            });
        });
    });

    self.registerForReceive();
}

//
// RadioSerial is an EventEmitter for open, error, and data events
// just like SerialPort/VirtualSerial.
//
util.inherits(RadioSerial, events.EventEmitter);

RadioSerial.prototype.registerForReceive = function() {

    var self = this;

    //
    // Setup to receive packet events from the packet radio
    //
    self.packetradio.on ('data', function(channel, packet) {
        self.processReceivedPacket(channel, packet);
    });
}

//
// Process a packet received from a radio.
//
// Its in binary, not string/char format.
//
// It becomes virtual serial input to the application.
//
// channel.portNumber
//
// packet is type buffer from http://nodejs.org/api/buffer.html
//
RadioSerial.prototype.processReceivedPacket = function(channel, packet) {

    var data = null;
    var dataLength = 0;
    var lineStatus = 0;
    var lineStatusValid = false;
    var sequence = false;

    //console.log("RadioSerial processReceivedPacket");

    if (!Buffer.isBuffer(packet)) {
        throw "packet needs to be Buffer";
    }

    if (packet.length == 0) {
        console.log("zerolengthpacket");
        return;
    }

    //
    // Process radio packet
    //
    if ((packet[0] & 0xC0) == 0x40) {

        if (packet.length < 2) {
            console.log("packettoshort");
            return;
        }

        //
        // The radio packet may have more bytes than
        // the payload header says is valid. This is because small radios
        // typically have a fixed packet length.
        //
        dataLength = packet[0] & 0x1F;

        // Look for malformed packets
        if (dataLength > (packet.length - 1)) {
            console.log("headerlengthlongerthanpacketlength");
            return;
        }

        // A zero data length packet is allowed.
        if (dataLength != 0) {
            // Buffer.slice points to the same data in packet
            data = packet.slice(1, dataLength + 1);
        }
    }
    else if ((packet[0] & 0xC0) == 0x80) {

        if (packet.length < 2) {
            console.log("packettoshort with status");
            return;
        }

        dataLength = packet[0] & 0x1F;

        // Look for malformed packets
        if (dataLength > (packet.length - 1)) {
            console.log("headerlengthlongerthanpacketlength with status");
            return;
        }

        // data + line_status packet is allowed to be line_status only.
        if (dataLength != 0) {
            // Buffer.slice points to the same data in packet
            data = packet.slice(2, dataLength + 2);
        }

        lineStatus = packet[1];
        lineStatusValid = true;
    }
    else {

        //
        // Not a radio serial packet.
        //
        // We don't touch it as we share a radio channel with other
        // handlers for different packet types.
        //
        return;
    }

    //
    // Packet is correctly formed, look at its receiveSequence.
    //
    if ((packet[0] & 0x20) != 0) {
        sequence = true;
    }

    if (sequence != this.receiveSequence) {
       var msg = "receivePacket out of sequence, dropping expect ";
       msg += this.receiveSequence;
       msg += " got ";
       msg += sequence;
       console.log(msg); // TODO: traceerror(msg);
       return;
    }

    // Toggle receive sequence for next packet receive
    if (this.receiveSequence) {
        this.receiveSequence = false;
    }
    else {
        this.receiveSequence = true;
    }

    //
    // A zero length data packet, or line status only, so there is
    // no data to deliver
    //
    if (data == null) {
        console.log("zero length data packet");
        return;
    }

    //
    // Announce the data to the VirtualSerial master side
    // This will become SerialPort receive data input.
    //
    this.virtualSerial.write(data, function(error, results) {
    });
}

//
// Send the data in buffer/length out the radio as one or more
// radio packets.
//
// callback(error)
//
RadioSerial.prototype.internalSendDataToRadio = function(argBuffer, callback) {

    var captured_this = this;

    if (captured_this.trace) {

        console.log("RadioSerial.internalSendDataToRadio buffer=");
        console.log(argBuffer);

        dh.dumpHexWithAddressBase(argBuffer, 0, argBuffer.length, 0, function(data) {
            console.log(data);
        });
    }

    //
    // TODO: Could cache small packets until a '\n' or flush to
    // minimize short radio packets.
    //
    // But current Dweet code is structured to send a full string
    // down to avoid this. So we will use data write sizes as
    // intended by the caller.
    //

    var argLength = argBuffer.length;

    //
    // sendPacketCallback(error)
    //
    var sendPacket = function(dataBuffer, dataIndex, dataLength, sendPacketCallback) {

        captured_this.tracelog("sendPacket dataLength=" + dataLength + " dataindex=" + dataIndex);

	if (captured_this.trace) {

	    console.log("RadioSerial.sendPacket dataBuffer=");
	    console.log(dataBuffer);

	    dh.dumpHexWithAddressBase(dataBuffer, 0, dataBuffer.length, 0, function(data) {
		console.log(data);
	    });
	}

        var packet = new Buffer(dataLength + 1);
        var packetByteCount = 0;
        var b = 0x40;
        b |= (dataLength & 0x1F);

        // Set transmitSequence, and toggle it for the next transmission.
        if (captured_this.transmitSequence) {
            b |= 0x20;
            captured_this.transmitSequence = false;
        }
        else {
            // Packet already has bit 0x20 clear
            captured_this.transmitSequence = true;
        }

        packet[0] = b;
        packetByteCount++;

        for (var index = 0; index < dataLength; index++) {
            packet[packetByteCount] = dataBuffer[dataIndex + index];
            packetByteCount++;
        }

	if (captured_this.trace) {

	    console.log("RadioSerial.radioWrite packet=");
	    console.log(packet);

	    dh.dumpHexWithAddressBase(packet, 0, packet.length, 0, function(data) {
		console.log(data);
	    });
	}

        captured_this.radioWriteFunction(packet, function(error) {
            sendPacketCallback(error);
        });
    }

    //
    // These are outside of transferMachine to provide
    // lambda state/operation context
    //   
    var bytesToGo = argLength;
    var dataBufferIndex = 0;

    //
    // This implements the typical transfer state machine
    //
    var transferMachine = function(machineCallback) {

       var thisTransfer = bytesToGo;
       if (thisTransfer > 31) {
           thisTransfer = 31;
       }

       bytesToGo -= thisTransfer;

       sendPacket(argBuffer, dataBufferIndex, thisTransfer, function(error) {
           dataBufferIndex += thisTransfer;

           if (bytesToGo == 0) {

               // Done
               machineCallback(null);
           }
           else {

               //
               // Continue
               //
               // Recursion/stack usage concerns:
               //
               // For small serial transfers no. But a large amount of data
               // being moved could blow the stack.
               //
               // A recursion depth can keep track and queue a
               // setImmediate() when reached.
               //
               // setImmediate(immediateFunction, "immediateArg");
               //
               transferMachine(machineCallback);
           }
       });
    }

    // Kick it off
   transferMachine(function(error) {
       callback(error);
   });
}

RadioSerial.prototype.setTrace = function(flag) {
    this.trace = flag;
}

RadioSerial.prototype.setTraceError = function(flag) {
    this.traceerrorValue = flag;
}

RadioSerial.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

RadioSerial.prototype.errorlog = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

module.exports = {

  //
  // We export the same name as npm SerialPort, using the serial port
  // package. The caller must have setup a RadioSerial first to register
  // the name for it to find.
  //
  SerialPort: function(portName, options) {
      return new virtualSerialModule.SerialPort(portName, options);
  },

  // RadioSerial support
  RadioSerial: RadioSerial
};

