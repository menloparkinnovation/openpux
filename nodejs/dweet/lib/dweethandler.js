
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
// DWEET Handler.
//
// 12/20/2014
//

//
// Note: See end of file for overall design details.
//

// http://nodejs.org/api/events.html
var events = require('events');

// http://nodejs.org/docs/latest/api/util.html
var util = require('util');

// Dweet uses NMEA 0183 formatted commands
var nmeaFactory = require('./nmea0183.js');

var dh = require("./dumphex.js").createInstance();

//
// DweetHandler is an EventEmitter
//
// Calling dweet.processSentenceAsEvents(dweet.parse(str)) will raise
// an event for each unique commnad=value in sentence.
//
// See processSentenceAsEvents/processCommandAsEvent for details.
//
// Properties:
//
// prefix
// serialPort
// nmea - nmea0183.js instance
// nmea() - getter for this.nmea property
//
// Methods:
//
// openSerialPort
// setMaxLength
// sendAndFlush - send basic command right away
// exchangeCommand - send command and expect a reply
// exchangeCommandList - send list of commands, each which expects a reply
// commandTransaction
// flush - flush any commands out now
// sendSync - send a sync command to (re) establish communication
// processReceivedData - indicate a fully received NMEA 0183 sentence line
// splitCommandValue - utility
// splitCommandEventReply - utility
// sendNMEASentence
// setTrace
// setTraceError
// tracelog
// traceerror
// createInstance
//
// Internal: (don't call from outside)
//
// onNMEAReceiveSentence
// generateCompareObjectFunction
// categorizeResponse
// processSentenceAsEvents
// processCommandAsEvent
//
function DweetHandler(trace, traceerrorValue, prefix) {

    this.moduleName = "DweetHandler";
    this.trace = false;
    this.traceerrorValue = false;

    this.prefix = prefix;

    //
    // A given channel can set new timeout and retries
    // baselines to allow for slower channels such as
    // multi-hop radios.
    //

    // This is added to the supplied commands timeout
    this.timeoutBaseline = 0;

    //
    // This is multiplied by the commands retry baseline.
    // This is to allow 0 retry commands to remain 0 retry
    // and rely on extended timeout alone.
    //
    this.retryMultiplier = 1;

    this.buffer = null;
    this.serialPort = null;

    // nmea0183.js instance
    this.nmea = nmeaFactory.createInstance(trace, traceerrorValue, prefix);

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }
}

// DweetHandler is an EventEmitter for received Dweet's such as COMMAND=VALUE
util.inherits(DweetHandler, events.EventEmitter);

DweetHandler.prototype.setTimeoutBaseline = function(value) {
    this.timeoutBaseline = value;
}

DweetHandler.prototype.setRetryMultiplier = function(value) {
    this.retryMultiplier = value;
}

//
// NMEA 0183 sentence has arrived, break it down and process it as
// Dweet's.
//
DweetHandler.prototype.onNMEAReceiveSentence = function(eventName, error, sentence, o) {

    //
    // o is the parsed sentence which has broken all NEMA 0183 words separated
    // by "," into separate array entries as o.commands[]
    //

    // Want to return even on error
    o.nmeaReplySentence = o.sentence; // o.sentence set by nmea.parse()

    if (error != null) {
        this.traceerror("onNMEAReceiveSentence: error=" + error);
        return;
    }

    // if o.error != null the event emitter has set error != null above

    if (!o.checksumOK) {
        this.traceerror("onNMEAReceiveSentence: bad checksum on sentence=" + o.sentence);
        return;
    }

    this.processSentenceAsEvents(o);
}

//
// serialport must be brought in with npm
// npm install serialport
//
// As such we don't execute require() until someone
// wants to open a serial port.
//
// openCallback is invoked on open error, data error, or data
// arrival indications if open is successful.
//
// Successful open is indicated with (error == null && data == null);
//
//    openCallback(error, data);
//
//      error != null -> error as per Node.js pattern for callbacks
//
//      data != null -> data that has arrived
//
// TODO: Fix this!
//      error == null && data == null -> First callback on successful open
//
// See comments for serialhandler.js, startSerialReader()
//
DweetHandler.prototype.openSerialPort = function(serialPortFactory, portName, openCallback) {

    this.serialPort = require('./serialhandler.js').createInstance(serialPortFactory);

    //
    // this can bind to the calling object, so get a stable instance
    // to our current instance into the lambda variable.
    //
    var dweetInstance = this;

    //
    // Set the nmea write handler to send data out the serialport
    //
    dweetInstance.nmea.setWriteHandler(function(data, callback) {
        var length = 0;

        if (data != null) {
            length = data.length;
            dweetInstance.serialPort.write(data, function(error, results) {
                // deeper down the rabbit hole...
                if (error != null) {
                    dweetInstance.traceerror("Dweet serial write error=" + error);
                    callback(error, 0); // writeHandler results callback
                    return
                }
            });
        }

        callback(null, length); // writeHandler results callback
    }); // writeHandler

    // Receives all NMEA 0183 sentences, error or not
    dweetInstance.nmea.on('receive', function(eventName, error, sentence, parsed_sentence) {
        dweetInstance.onNMEAReceiveSentence(eventName, error, sentence, parsed_sentence);
    });

    //
    // start off the serial reader which opens the port.
    // On open error it will invoke the callback with the error.
    // If success, a series of data arrival indications will occur
    // through the callback with data != null.
    //
    dweetInstance.serialPort.startSerialReader(portName, true, openCallback);
}

DweetHandler.prototype.setMaxLength = function(maxLength) {
    this.nmea.setMaxLength(maxLength);
}

//
// Parameterized compare function useful for exchangeCommand when
// a common Dweet command works with multiple objects of the form:
// 
// command=object:value
// 
// This allows matching replies with requests.
// 
// Command structure this supports:
//
// GETCONFIG=VERSION  -> sent by host to device
//   
// GETCONFIG_REPLY=VERSION:1.0.1 -> sent by device to host
// GETCONFIG_ERROR=VERSION -> sent by device to host
// GETCONFIG_UNSUP=VERSION -> sent by device to host
//
// Arguments:
//
// argPrefix=$PDWT
//
//   This allows different NMEA 0183 sentence structures to be used
//   such as $PDWT for general Dweet commands, $PDBG for debug commands
//   or other application/device specific commands.
//
// command=GETCONFIG
//
// This is the command we are looking for.
//
// object=VERSION
//
// This is the command object we are looking for.
//
DweetHandler.prototype.generateCompareObjectFunction = function(argPrefix, command, objectName) {

    var captured_this = this;

    var compareFunction = function(name, value, prefix) {

       //captured_this.tracelog("comparefunction: name=" + name + " value=" + value);

       var cr = captured_this.splitCommandEventReply(name);

       //captured_this.tracelog("cr.event=" + cr.event + " command=" + command);

        if ((prefix == argPrefix) && (cr.event == command)) {

            if (objectName == null) {
                // Not comparing objectName we are done
                return true;
            }

            // objectName is required to match by the caller

            // NOTE: Some responses will be just value and not value:name
            ar = value.split(':');
            if (ar[0] == objectName) {
                return true;
            }
            else {
                return false;
            }
        }

        return false;
    }

    return compareFunction;
}

//
// This categorizes the response.
//
//  COMMAND       - ""
//  COMMAND_REPLY - "REPLY"
//  COMMAND_ERROR - "ERROR"
//  COMMAND_UNSUP - "UNSUP"
//
DweetHandler.prototype.categorizeResponse= function(resp) {
    var nv = resp.split('_');

    if (nv.length == 1) {
        // just COMMAND
        return "";
    }

    this.tracelog("categorizeResponse command=" + nv[0] + " reply=" + nv[1]);

    return nv[1];
}

//
// Send a Dweet command in which a response
// is expected.
//
// If a response is not received by timeout, re-send
// the command until either retryLimit is reached, or
// a response is received.
//
// It is the callers responsibility to ensure that the devices
// state is not corrupted if multiple commands are sent, or
// replies are lost.
//
// Arguments:
//
// args - arguments object which contains all required parameters
//     for the command transaction and stores the results.
//
// Input:
//   args.fullCommand
//   args.timeout
//   args.retryLimit
//   args.command
//   args.retryLimit
//
// Output:
//   args.error
//   args.nmeaReplySentence
//
// Description of fields:
//
// fullCommand - Full command to send.
//   Example: "GETCONFIG=LIGHT"
//
// timeout - Time in milliseconds before the command is resent.
//
// retryLimit - Maximum number of retries the command will be sent
//           before returning an error.
//
// compareFunc - Function that returns true if a given response
//          is accepted as the commands response.
//
// callback(error, reply, retryCount) - Function invoked when
//          an acceptable response is received, or the retryLimit
//          has been reached.
//
//     dweethandler.exchangeCommand(args, compareFunc, callback)
//       dweethandler.sendAndFlush(args, callback)
//         nmea0183.sendAndFlush(args, callback)
//           nmea0183.nmeaSender(args, callback)
//           nmea0183.flush(callback);
//
DweetHandler.prototype.exchangeCommand = function(args, compareFunc, callback) {

    var o = args;

    var command = o.fullCommand;
    var timeout = o.timeout + this.timeoutBaseline;
    var retryLimit = o.retryLimit * this.retryMultiplier;

    var retryCount = 0;
    var intervalObject = null;

    var captured_this = this;

    var nv = this.splitCommandValue(command);

    //
    // Configure receiver/event for response
    //
    // We use a Node.js event listener on the receive Dweet
    // stream. Since EvenEmitters can have multiple listeners
    // we can be an additional party to the receive Dweet stream.
    //

    if (o.command.indexOf("_") != (-1)) {
        o.error = "COMMAND can not have _ in main name";
        callback(o.error, null, 0);
        return;
    }

    var eventName = o.command;

    // Function requires a name so we can perform a removeListener(event, func)
    var eventFunction = function(name, value, prefix, reply_o) {

        captured_this.tracelog("exchangeCommand: received Dweet response Event");
        captured_this.tracelog("name=" + name + " value=" + value + " prefix=" + prefix);

        if (compareFunc(name, value, prefix)) {

            var error = null;

            //
            // The received Dweet matches the pattern we are looking
            // for. So cancel the timer and our Dweet receive event
            // stream listener.
            //

            // Cancel the timer and event listener
            clearInterval(intervalObject);
            captured_this.removeListener(eventName, eventFunction);

            //
            // Though we got a valid response, it could be an error
            // such as _ERROR or _UNSUP, so validate it.
            //
            var resp = captured_this.categorizeResponse(name);
            if (resp != "REPLY") {
                captured_this.tracelog("exchangeCommand: error response=" + resp +
                    " retryCount=" + retryCount);
                error = resp;
            }
            else {
                captured_this.tracelog("exchangeCommand: got success response retryCount=" + retryCount);
            }

            // format the reply object
            nv.name = name;
            nv.value = value;
            nv.prefix = prefix;
            nv.nmeaReplySentence = reply_o.nmeaReplySentence;

            callback(error, nv, retryCount);
            return;
        }
        else {
            captured_this.tracelog("exchangeCommand: response does not match");
            captured_this.tracelog("eventName=" + command + " retryCount=" + retryCount);
            captured_this.tracelog("name=" + name + " value=" + value + " prefix=" + prefix);

            // We just drop it
        }
    }

    //
    // Listen for replies on the Dweet receive event stream
    // 
    // Events raised by this.processCommandAsEvent()
    //
    captured_this.on(eventName, eventFunction);

    // This is called by the timer to retry sends
    var intervalFunction = function() {

        retryCount++;
        if (retryCount > retryLimit) {

            clearInterval(intervalObject);
            captured_this.removeListener(eventName, eventFunction);
            var err = "no response received in timeout";
            captured_this.traceerror("exchangeCommand: " + err);
            callback(err, null, null);
            return;
        }

        // Resend command after a sync
        captured_this.sendSync(function(syncError) {

            captured_this.sendAndFlush(o, function(error) {

                if (error != null) {
                    captured_this.traceerror("exchangeCommand: sendAndFlush error=" + error);
                }
                else {
                    captured_this.tracelog("exchangeCommand: retry send succeeded: Retries=" + retryCount);
                }
            });
        });

    }

    intervalObject = setInterval(intervalFunction, timeout);

    //
    // Note a command exchange acts as a barrier and
    // any previously queued commands are flushed along
    // with the requested command.
   //
    // This is because we are on a timer and expect
    // timely delivery.
    //

    // Send the first command
    captured_this.sendAndFlush(o, function(error) {
        if (error != null) {
            captured_this.traceerror("exchangeCommand: sendAndFlush error=" + error);
        }
    });
}

//
// Perform a command transaction to retrieve a result
// from a Dweet device.
//
// Dweet command structure:
//
// command=object
// command=object:value
//
// Responses are:
//
// command_REPLY=object:value
// command_ERROR=object:value
// command_UNSUP=object:value
//
// Examples:
//
// $PDWT,GETCONFIG=VERSION*00\r\n  -> sent by host to device
//
// $PDWT,GETCONFIG_REPLY=VERSION:1.0.1*00\r\n -> sent by device to host
// o.prefix == "$PDWT"
//
// Input:
//
//   args.prefix
//   args.command
//   args.object
//   args.value
//   args.compareObject
//   args.timeout
//   args.retryLimit
//
// Output:
//   callback(error, args, args.result);
//
//   args.error
//   args.result
//   args.fullCommand
//   args.retryCount
//   args.fullReply
//
//   args.nmeaSentence
//   args.nmeaReplySentence
//
// dweetconsole.commandFunctionArgs(args, context, callback)
//   dweethandler.commandTransaction(args, callback)
//     dweethandler.exchangeCommand(args, compareFunc, callback)
//       dweethandler.sendAndFlush(args, callback)
//         nmea0183.sendAndFlush(args, callback)
//           nmea0183.nmeaSender(args, callback)
//           nmea0183.flush(callback);
//
DweetHandler.prototype.commandTransaction = function(args, callback) {

    var o = args;

    //
    // NOTE: We receive a stream of DWEET's from the device
    // and our response may be sent between many others. So
    // we need to look for our expected reply and if a given
    // event is not ours return a status so that the Dweet receive
    // event handler may continue processing.
    //
    // exchangeCommand() implements a timeout on behalf of
    // caller if we do not detect a reply that matches
    // the pattern we are looking for within the specified
    // interval.
    //

    // command=name
    o.fullCommand = o.command + "=" + o.object;

    if (o.value != null) {
        // command=object:value
        o.fullCommand += ":";
        o.fullCommand += o.value;
    }

    o.error = null;
    o.result = null;
    o.retryCount = 0;
    o.fullReply = null;

    // capture this
    var dweet = this;

    //
    // invoked by dweet.exchangeComamnd() when done
    //
    //  nv.prefix
    //  nv.name
    //  nv.value
    //  nv.nmeaReplySentence
    //
    var completionFunction = function(error, nv, retryCount) {

        if (error != null) {
            o.error = error;

            if (nv != null) {
                // Show reply details even if error if provided
                o.fullReply = nv.name + "=" + nv.value;
                o.nmeaReplySentence = nv.nmeaReplySentence;
            }

            dweet.traceerror("commandTransaction: Error sending Dweet error=" + error);
            callback(error, o, null);
            return;
        }
        else {

            o.fullReply = nv.name + "=" + nv.value;
            o.nmeaReplySentence = nv.nmeaReplySentence;

            dweet.tracelog("commandTransaction: nv.name=" + nv.name + " nv.value=" + nv.value);

            ar = nv.value.split(':');

            if (o.object.indexOf(ar[0]) != 0) {
                var es = "not expected name " + ar[0] + " sb " + o.object;
                o.error = es;
                dweet.traceerror(es);
                callback(es, o, null);
                return;
            }

            o.result = ar[1];
            o.retryCount = retryCount;

            callback(null, o, o.result);
            return;
        }
    }

    //
    // Generate a compare function that will match received Dweet events
    // to discover our reply.
    //
    // o.prefix=$PDWT
    // o.command=GETCONFIG
    // o.compareObject=VERSION
    //
    // Note: compareObject could be null, or a different value in what
    // is sent. It's the name matched for if != null.
    //
    var compareFunc = dweet.generateCompareObjectFunction(o.prefix, o.command, o.compareObject);

    dweet.exchangeCommand(
        o,
        compareFunc,
        completionFunction
        );
}

//
// exchangeCommandList
//
// Send a series of Dweet commands in order from an array
// of per command configuration objects.
//
// Each configuration object specifies the commands parameters
// and holds any responses.
//
// Some commands may expect a response from the remote device
// and if so the sequence will not progress until a response
// is received. The caller configures a timeout and
// a retry count.
//
// The caller can specify whether to progress on errors,
// or fail early.
//
// commandArray has the same format as exchangeCommand.
//
// error != null is an error.
//
//   callback(error, o, o.result);
//
DweetHandler.prototype.exchangeCommandList = function(commandArray, stopOnError, callback) {

    var o = null;

    //
    // Send a series of commands by using continuations.
    //
    var arrayIndex = 0;

    //
    // capture our "this" to ensure its consistency in callbacks
    //
    var captured_this = this;

    //
    // Note: Must be very careful to ensure unique variables are
    // used since their scope is across the current method and
    // sub-functions regardless of scope blocks ({})'s.
    //
    var sendComplete = function(error_x, o_x, result_x) {

        o_x.error = error_x;
        o_x.result = result_x;

        if (error_x != null) {
            captured_this.traceerror("error queuing dweet command error=" + error_x);
            captured_this.traceerror("command=" + commandArray[arrayIndex].fullCommand);

            if (stopOnError) {
                callback(error_x, o_x, o.result);
                return;
            }

            // continue sending commands on error
        }

        arrayIndex++;
        if (arrayIndex >= commandArray.length) {

            // We are done, flush
            captured_this.flush(function(error_x1) {
                if (error_x1 != null)  {
                    captured_this.traceerror("error flushing Dweet sentence error=" + error_x1);
                }

                // done
                callback(error_x1, o_x, o.result);

            }); // flush

            return;
        }

        // Send the next command
        o = commandArray[arrayIndex];
        captured_this.commandTransaction(o, sendComplete);
    };

    // kick it off
    o = commandArray[arrayIndex];
    captured_this.commandTransaction(o, sendComplete);
}

//
// Return the nmea0183 instance to allow Event handlers
// to be configured.
//
DweetHandler.prototype.nmea = function() {
    return this.nmea;
}

//
// invoked from dweetclient.js, processReceivedData(data)
//
DweetHandler.prototype.processReceivedData = function(data) {

    //
    // This supplies a whole received NMEA 0183 sentence that starts
    // with $ and ends with '\r''\n' to the NMEA 0183 handler.
    //
    // This will cause the NMEA 0183 handler to raise the
    // the sentence received event after processing it.
    //
    this.nmea.processReceivedSentence(data);
}

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
DweetHandler.prototype.splitCommandValue = function(str) {
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
// Note: Since DweetHandler is the EventEmitter itself
// these can't be delegated to nmea0183.
//
// This is called by low level Dweet protocol handlers that
// have received a sentence and broke it down into an
// array of command=value entries separated by "," in
// the NMEA 0183 sentence as o.commands[]
//
// dweetclient.processReceivedData(data)
//   var o = dweethandler.parse(data)
//   dweethandler.processSentenceAsEvents(o)
//
//   o.nmeaReplySentence
//
DweetHandler.prototype.processSentenceAsEvents = function(o) {
    var nv = null;
    var command = null;

    this.tracelog("DweetHandler.processSentenceAsEvents: received sentence=");
    if (this.trace) {
        // Get the JSON output of console.log directly
        console.log(o);
    }

    for (var index = 0; index < o.commands.length; index++) {
        command = o.commands[index];
        nv = this.splitCommandValue(command);
        this.processCommandAsEvent(o, command, nv);
    }
}

//
// Raise an event on the DweetHandler.EventEmitter for the
// command.
//
// "_" is used as a separator between a base COMMAND which is
// its event name that is raised and its possible responses
// which represent success or error conditions.
//
// COMMAND=value
// COMMAND_REPLY=value
// COMMAND_ERROR=value
// COMMAND_UNSUP=value
//      ...
//
// These all raise the "COMMAND" event with:
//
// nv.name  = "COMMAND"
//         *or*
// nv.name  = "COMMAND_REPLY"
//         *or*
// nv.name  = "COMMAND_ERROR"
//         *or*
// nv.name  = "COMMAND_UNSUP"
//         *and*
// nv.value = "value"
//
// Dweet.js:
//    dweet.on('GETCONFIG_REPLY', function(name, value, prefix, o) {
//        processDeviceDweet(name, value, prefix);
//    });
//
//   o.nmeaReplySentence
//
DweetHandler.prototype.processCommandAsEvent = function(o, command, nv) {

    this.tracelog("processCommandAsEvent: received event name=" + nv.name +
        " value=" + nv.value + " prefix=" + o.prefix);

    //
    // We emit the general Dweet receive event for general receive
    // Dweet stream listeners.
    //
    this.emit("receive", "receive", null, command, o);

    //
    // The event's name is the base name of the COMMAND.
    //
    // The listener is responsible for any further processing
    // of the received nv.name, nv.value.
    //
    var cr = this.splitCommandEventReply(nv.name);

    this.tracelog("raising event=" + cr.event + " nv.name=" + nv.name);

    //
    // This just emits the base name of the received Dweet
    // for specific listeners.
    //
    // GETCONFIG_REPLY=name:value => GETCONFIG event is emitted.
    //
    this.emit(cr.event, nv.name, nv.value, o.prefix, o);
}

//
// COMMAND
//   o.event = "COMMAND"
//   o.reply = null;
//
// COMMAND_REPLY
//   o.event = "COMMAND"
//   o.reply = "REPLY"
//
// Output:
//
DweetHandler.prototype.splitCommandEventReply = function(str) {
    var o = new Object();

    if (str.indexOf('_') == (-1)) {
        // treat as a single entry
        o.event = str;
        o.reply = null;
        return o;
    }

    var nv = str.split('_');
    o.event = nv[0];
    o.reply = nv[1];

    return o;
}

//
// Send a sync to the channel to (re) establish communications.
//
// callback(error)
//
DweetHandler.prototype.sendSync = function(callback) {
    this.nmea.sendSync(callback);
}

//
// Send a low level raw NMEA sentence with no Dweet command
// formating.
//
// This worker function ensures we have flushed any previous
// Dweet commands before sending the direct NMEA 0183 sentence.
//
// If you don't want a flush error preventing the NMEA sentence
// from being sent call flush() before this function. This
// is automatic if Dweet exchangeCommand() is used.
//
// No response is expected, or waited for from the device
// and its up the caller to arrange for handling any NMEA messages
// received from the device as a result.
//
// callback(error)
//
DweetHandler.prototype.sendNMEASentence = function(sentence, callback) {

    var error = null;
    var captured_this = this;

    //
    // To ensure that flush errors do not prevent the NMEA sentence from
    // being sent callers can call dweethandler.flush(callback) first
    // to ensure any previously queued Dweet messages have been sent,
    // and deal with any Dweet resends as required. This is done
    // automatically if the higher level Dweet handler exchangeCommand() is
    // used.
    //
    var args = new Object();
    captured_this.nmea.flush(args, function(flushError) {

        if (flushError) {
            flushError = "sendNMEASentence: flush error for previous data=" + flushError;
            captured_this.traceerror(flushError);
            callback(flushError);
            return;
        }

        captured_this.nmea.sendNMEASentence(sentence, callback);
    });
}

//
// See nmeaHandler.sendAndFlush() for contract.
//
//       dweethandler.sendAndFlush(args, callback)
//         nmea0183.sendAndFlush(args, callback)
//           nmea0183.nmeaSender(args, callback)
//           nmea0183.flush(callback);
//
// args used:
//
// Input:
//   args.fullCommand
//
// Output:
//   args.nmeaSentence
//
// callback(error)
//
DweetHandler.prototype.sendAndFlush = function(args, callback) {

    var captured_this = this;

    this.nmea.sendAndFlush(args, function(error) {

        // general send event
        captured_this.emit("send", "send", error, args.fullCommand, args);

        callback(error);
    });
}

DweetHandler.prototype.setTrace = function(value) {
    this.trace = value;
    this.nmea.setTrace(value);
}

DweetHandler.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
    this.nmea.setTraceError(value);
}

DweetHandler.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

DweetHandler.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(trace, traceerrorValue, prefix) {
    moduleInstance = new DweetHandler(trace, traceerrorValue, prefix);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
