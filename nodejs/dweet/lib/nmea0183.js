
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
// NMEA0183 support
//
// 12/10/2014
//

//
// Only printable ASCII characters are allowed plus CR, LF.
//
// Special characters/symbols
// 
//  $    - sentence start
//  ,    - word separator in sentence
//  *    - checksum follows
//  <CR> - part of sentence end
//  <LF> - part of sentence end
//
// Lines are limited to 80 printable characters not including the line
// terminators <CR><LF>
//
// There are three kinds of sentences:
// 1 - Talker
// 2 - Proprietary
// 3 - query
//
// Proprietary Sentences:
// 
// $Pxxx
//
// $   - sentence start
// P   - proprietary sentence identifier
// xxx - manufacturer ID
//
// $ - sentence start
// P - proprietary sentence identifier
// SPX - Smartpux three letter manufacturer id
// 
// NMEA SPX sentence (note checksum example is not right calculated value at *00)
// $PSPX,S=00,D0=0000,D1=0000,D2=0000,D3=0000,M0=0000,M1=0000,M2=0000,M3=0000*00<CR><LF>
//

//
// Very short commands
//
//     4 16 bit sensor readings
//     total = 27 characters
//  6    2       16            3
// $PDWT,a=0000000000000000*00
//
//    1 16 bit sensor reading
//
//     15 characters
//  6    2  4  3
// $PDWT,a=0000*00
//
//     14 characters
//  6    1  4  3
// $PDWT,a0000*00
//

// http://nodejs.org/api/events.html
var events = require('events');

// http://nodejs.org/docs/latest/api/util.html
var util = require('util');

//
// Nmea0183Instance is an EventEmitter
//
// Calling nmea.processSentence(nmea.parse(str)) will raise
// an event for each unique commnad=value in sentence.
//
// See processSentence/processCommand for details.
//
function Nmea0183Instance(trace, traceerrorValue, prefix) {

    // Module support
    this.moduleName = "Nmea0183Instance";
    this.trace = false;
    this.traceerrorValue = true;

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }

    if (typeof(prefix) != "undefined") {
        this.prefix = prefix;
    }
    else {
        this.prefix = "$PSPX";
    }

    this.buffer = "";

    //
    // Configured maximum length for NMEA 0183 is 80
    // characters not including the <CR><LF>.
    //
    this.configuredMaxLength = 80;

    //
    // Maximum prefix length is 6 for standard NMEA.
    //
    // $PDWT -  5 including $
    // $WIWMV - 6 including $
    //
    this.maxPrefixLength = 6;

    //
    // maximum length includes the prefix and the checksum
    // which includes the * before the two checksum digits.
    //
    // It does not include the <CR><LF>
    //
    this.maxLength = this.configuredMaxLength - (this.prefix.length + 3);

    //
    // create a default handler for the streaming
    // API's.
    //
    this.channelWrite = function(data, callback) {
        error = "Nmea0183Instance: channelWrite not implemented";
        callback(error, null);
    }
}

// Nmea0183Instance is an EventEmitter for received and sent NMEA 0183 messages
util.inherits(Nmea0183Instance, events.EventEmitter);

Nmea0183Instance.prototype.emitReceivedEvent = function(error, sentence, parsed_sentence) {
    this.emit("receive", "receive", error, sentence, parsed_sentence);
}

Nmea0183Instance.prototype.emitSendEvent = function(error, sentence) {
    this.emit("send", "send", error, sentence);
}

//
// This is invoked by the driver/serial handler for a complete
// NMEA 0183 buffer received that starts with $ and ends with \r\n
//
Nmea0183Instance.prototype.processReceivedSentence = function(sentence) {

    var o = this.parse(sentence);

    // Validate checksum, structure, pass along the results
    this.emitReceivedEvent(o.error, sentence, o);
}

//
// Set the write handler for the streaming API's.
//
// writeHandler has the following signature:
//
// void writeHandler(data, callback)
//
//   data != null  -> data to send out
//
//   callback(error, results)
//
//     writeHandler is expected to invoke the following signature
//     on I/O complete or error:
//
//       error != null -> error as per Node.js pattern for callbacks
//
//       results != null  -> count of bytes transferred
//
Nmea0183Instance.prototype.setWriteHandler = function(writeHandler) {
    this.channelWrite = writeHandler;
}

Nmea0183Instance.prototype.setMaxLength = function(maxLength) {

    if (maxLength > 80) throw "bad max length";

    this.configuredMaxLength = maxLength;
    this.maxLength = this.configuredMaxLength - (this.prefix.length + 3);
}

// Reset the buffer
Nmea0183Instance.prototype.reset = function() {
    this.buffer = "";
}

//
// NMEA commands/words are separated by ",". They are placed
// into a sentence that meets the total length restrictions.
//
Nmea0183Instance.prototype.addCommand = function(cmd) {

    // Add one for the "," separator
    if (((cmd.length + 1) + this.buffer.length) > this.maxLength) {
        return false;
    }

    this.buffer += ",";
    this.buffer += cmd;

    return true;
}

//
// Capacity left for commands/words.
//
Nmea0183Instance.prototype.capacity = function() {
    return this.maxLength - this.buffer.length;
}

//
// Generate NMEA 0183 sentence from the buffered commands.
//
// If no commands are buffered, returns null.
//
Nmea0183Instance.prototype.generateSentence = function() {
    var chksum;

    // If no commands in the buffer, we return null.
    if ((this.buffer == null) || (this.buffer.length == 0)) {
        return null;
    }

    var sentence = this.prefix + this.buffer + "*";

    chksum = this.checksum(sentence);

    sentence += chksum;
    sentence += "\r\n";

    if (sentence.length > 82) throw "generateSentence bad max length";

    this.reset();

    return sentence;
}

//
// Parse the message.
//
// Returns an object with the following data:
//
// o.error - != null is a parse error
// o.sentence = raw sentence
// o.prefix - prefix
// o.commands - array of commands/words that were separated by "," in sentence
// o.checksumOK - true if checksum is ok.
// o.checksumIsPlusOne - true if checksum is one greater than the sentence checksum
// o.calculatedChecksum - check sum calculated.
// o.checksumMsb - received checksum msb
// o.checksumLsb - received checksum lsb
//
Nmea0183Instance.prototype.parse = function(str) {

    var o = new Object();
    o.error = null;
    o.prefix = null;
    o.commands = null;
    o.checksumOK = false;
    o.sentence = str;

    this.tracelogLevel(10, "parse: str=" + str);

    if (str[0] != '$') {
        o.error = "Sentence does not start with $";
        this.traceerror(o.error);
        return o;
    }

    // Find first "," which marks the end of the received prefix
    var index = str.indexOf(',');
    if (index == (-1)) {
        o.error = "Sentence missing separator , after prefix";
        this.traceerror(this.error);
        return o;
    }

    o.prefix = str.substring(0, index);

    if (o.prefix.length > this.maxPrefixLength) {
        o.error = "maximum prefix length exceeded";
        this.traceerror(o.error);
        return o;
    }

    var index = str.indexOf('*');
    if (index == (-1)) {
        o.error = "missing commands terminator *";
        this.traceerror(o.error);
        return o;
    }

    //
    // This returns the check sum as hex with upper case characters
    // since this is the representation in NMEA 0183.
    //
    try {
        o.calculatedChecksum = this.checksum(str);
    }
    catch (e) {
        //
        // This occurs due to bad formatting, not due to a bad checksum
        // since that comparison is later.
        //
        o.error = "checksum calculation exception e=" + e.toString();
        this.traceerror(o.error);
        return o;
    }

    // Prefix + command separator ","
    // $PDWT,
    var start = o.prefix.length + 1;

    var commands = str.substring(start, index);

    var checkdigits = str.substring(index+1, index+3);
    this.tracelog("checkdigits=" + checkdigits);

    o.checksumMsb = checkdigits[0];
    o.checksumLsb = checkdigits[1];

    // Create an array of the commands split on "," separator
    o.commands = commands.split(',');

    // split can return a "" entry[0]
    if ((o.commands.length > 0) && (o.commands[0].length == 0)) {
        o.commands = o.commands.slice(1, o.commands.length);
    }

    if (checkdigits == o.calculatedChecksum) {
        o.checksumOK = true;
        this.tracelog("checksumOK");
    }
    else {

       //
       // Check if the checksum inside the sentence is one
       // greater than the calculated checksum. This occurs
       // for routed messages on purpose to artificially
       // "poison" them if they get separated from their
       // routing header. We indicate this to the caller
       // since they may be a router expecting this case.
       //
       o.checksumIsPlusOne = false;

       if (checkdigits == (o.calculatedChecksum + 1)) {
           o.checksumIsPlusOne = true;
       }

        o.error = "checksum error " + o.calculatedChecksum + 
                  " does not match check digits " + checkdigits;

        this.traceerror(o.error);

        //
        // We still return the commands to let the caller
        // decide if they want to accept the message.
        //
    }

    return o;
}

//
// http://en.wikipedia.org/wiki/NMEA_0183#C_implementation_of_checksum_generation
//
// Checksum is returned as a string in upper case.
//
Nmea0183Instance.prototype.checksum = function(str) {
    var err;
    var chksum = 0;
    var index = 0;

    // <CR><LF> is not counted in maximum length
    if (str.length > this.configuredMaxLength + 2) {
        this.traceerror("checksum: sentence length to long sentence.length=" + str.length);
        throw "string length to long";
    }

    //
    // Note: CHKSUM is a bytewise operation, make sure
    // this works out corrrectly in Javascript.
    //

    index = 0;

    if (str[index] != '$') {
        err =  "message must start with $";
        this.traceerror(err);
        throw err;
    }

    //
    // Check sum starts at 0.
    //
    // Includes all parts of the string but $ and *
    //
    // If any <CR><LF> exists they are ignored as
    // well.
    //
    // Example:
    //   $PSPX,STRICT=ON*
    //

    // skip any '$'
    index++;

    this.tracelog("checksum: sentence=" + str);

    for (; index < str.length; index++) {

        // done if we reach the '*' character at the end
        if (str[index] == '*') {
            break;
        }

        //this.tracelog("before: chksum=" + chksum + " str[index]=" + str[index]);

        // keep it at two digits
        chksum = ((chksum & 0x00FF) ^ str.charCodeAt(index)) & 0x00FF;

        //this.tracelog("after: chksum=" + chksum + " str[index]=" + str[index]);
    }

    if (str[index] != '*') {
        err = "message must end with *";
        this.traceerror(err);
        throw err;
    }

    var chksumString = chksum.toString(16);

    // 0 pad to two digits
    if (chksumString.length == 1) {
        chksumString = "0" + chksumString[0];
    }
    else {
        // assert
        if (chksumString.length != 2) {
            this.traceerror("chhksumString=" + chksumString);
            this.traceerror("chhksumString.length=" + chksumString.length);
            this.traceerror("str.length=" + str.length);
            throw "chksum digits overflow";
        }
    }

    return chksumString.toUpperCase();
}

//
// NMEA 0183 command or words are  composed
// of values separated by ",". Multiple commands
// or words may be in an sentence.
//
// Example valid NMEA 0183 words are:
//
// COMMAND1=VALUE,COMMAND=NAME:VALUE,COMMAND0
//
// $HCHDM,238,M<CR><LF>
//
//   HC - Magnetic compass
//
//   HDM - Magnetic heading
//
//   238 - Heading value
//
//   M - Designates a magnetic heading value
//
// These are placed into NMEA 0183 sentences from the
// configurated prefix such as "$PDWT" in the case
// of the "Dweet" device control prototocol.
//
// This routine is designed to efficiently buffer as
// many commands within a given sentence as possible.
//
// This handles the breaking up of long
// sentences to fit within the NMEA 0183 specification
// of a maximum sentence length of 80.
//
// It's the callers responsibility to ensure that each
// command can fit into a sentence by itself.
//
// This works with indempotent commands which can
// be split between multiple sentences without loss
// of fidelity. Of course either of the sentences
// could be lost, and the callers data model must be
// prepared to handle this.
//
// If the buffer is full, the current set of buffered
// commands are sent out on the channel, and the requested
// command is buffered for the next sentence.
//
// Commands can remain buffered in the currently
// being constructed sentence. The caller is
// responsible for eventually flushing it to send it on
// the configured channel.
//
// callback(error);
//
//           nmea0183.nmeaSender(args, callback)
//           nmea0183.flush(args, callback);
//
// Input to this function:
//
// args.error
// args.result
// args.fullCommand
// args.fullReply
// args.retryCount
//
// Added by this function:
//
// args.nmeaSentence
//
Nmea0183Instance.prototype.nmeaSender = function(args, callback) {

    var result = this.addCommand(args.fullCommand);
    if (result)  {
        // Command fit into current buffer, so return success
        callback(null);
        return;
    }

    this.tracelog("nmea buffer full, sending current sentence");

    // current sentence buffer is full, send it
    var sentence = this.generateSentence();

    this.tracelog("sentence=" + sentence);
 
    args.nmeaSentence = sentence;

    //
    // Must capture the outer "this" into a lamba variable
    // since callbacks bind "this" to their invoking object.
    //
    var nmeaInstance = this;

    //
    // send the buffer on the channel.
    // write is async, so we must queue the next send as the callback
    //
    nmeaInstance.flush(args, function(error) {
     
        if (error != null) {
            nmeaInstance.traceerror("nmeaSender: error=" + error);
            callback(error);
            return;
        }

        // No error, setup the next sentence

        // reset the buffer to begin the next sentence
        nmeaInstance.reset();

        var result = nmeaInstance.addCommand(command);
        if (!result) {
            var msg  = "error adding command to NMEA 0183 buffer, cmd=" + command + "\n";
                msg += "nmeaSender: error=" + msg + "\n";
                msg += "nmea.capacity()=" + this.capacity() +
                       " command.length=" + command.length + "\n";
                msg += "command may be to long for the configured capacity";
            nmeaInstance.traceerror(msg);
            callback(msg);
            return;
        }
        else {
            callback(null);
            return;
        }

    }); // nmeaInstance.flush()
}

//
// Flush any commands in the currently buffered sentence if
// any.
//
// Note: The buffer is reset only on write success to allow
// the caller to decide to retry or reset the buffer.
//
// callback(error);
//
Nmea0183Instance.prototype.flush = function(args, callback) {

    var sentence = this.generateSentence();
    if (sentence == null) {
        // no commands buffered to send
        callback(null);
        return;
    }

    args.nmeaSentence = sentence;

    //
    // Must capture the outer "this" into a lamba variable
    // since callbacks bind "this" to their invoking object.
    //
    var nmeaInstance = this;

    this.channelWrite(sentence, function(error, results) {

        // Emit the sentence and its results for listeners
        nmeaInstance.emitSendEvent(error, sentence);

        if (error != null) {
            nmeaInstance.traceerror("dweetSender: error=" + error);
            callback(error);
            return;
        }
        else {
            nmeaInstance.reset();
            callback(null);
        }
    });
}

//
// Send a sync to the channel to (re) establish communications.
//
Nmea0183Instance.prototype.sendSync = function(callback) {

    //
    // Send the '\n' the is the final delimeter twice
    //
    var sentence = "\n";

    var nmeaInstance = this;

    this.channelWrite(sentence, function(error, results) {

        // Emit the sentence and its results for listeners
        nmeaInstance.emitSendEvent(error, sentence);

        if (error != null) {
            nmeaInstance.traceerror("nmea0183.sendSync error=" + error);
            callback(error);
            return;
        }
        else {
            callback(null);
            return;
        }
    });
}

//
// Combine the send + flush operations in a single function
// with only one callback to deal with.
//
// callback(error)
//
//         nmea0183.sendAndFlush(args, callback)
//           nmea0183.nmeaSender(args, callback)
//           nmea0183.flush(args, callback);
//
Nmea0183Instance.prototype.sendAndFlush = function(args, callback) {

    var captured_this = this;

    this.nmeaSender(args, function(error) {
        if (error != null) {
            callback(error);
            return;
        }
        captured_this.flush(args, callback);
    });
}

//
// Send a NMEA 0183 sentence.
//
// Command buffer should have been flushed.
//
// The sentence contains the prefix, separators, and
// must end with "*".
//
// The required checksum and "\r\n" are added by this function.
//
// callback(error);
//
Nmea0183Instance.prototype.sendNMEASentence = function(sentence, callback) {

    if (this.buffer.length != 0) {
        callback("non-zero length command buffer, flush first");
        return;
    }

    //
    // prefix is supplied
    //
    // Sentence ends with *, without checksum digits which are added
    // by this routine.
    //

    chksum = this.checksum(sentence);

    sentence += chksum;
    sentence += "\r\n";

    var nmeaInstance = this;

    this.channelWrite(sentence, function(error, results) {

        // Emit the sentence and its results for listeners
        nmeaInstance.emitSendEvent(error, sentence);

        if (error != null) {
            nmeaInstance.traceerror("dweetSender: error=" + error);
            callback(error);
            return;
        }
        else {
            callback(null);
        }
    });
}


//
//
// The following special characters must be escaped
// on a NMEA 0183 stream.
//
// '$' ',' '*' '\r' '\n'
//
// '%'  = %25
//' $'  = %24
//' ,'  = %2c
// '*'  = %2a
// '\r' = %0d
// '\n' = $0a
//
// Also Dweet relies on '=' and ':' being special as well
//
// '='  = %3d
// ':'  = %3a
//
Nmea0183Instance.prototype.escapeString = function(input) {
    var output = new Array();

    var inputIndex = 0;
    var outputIndex = 0;
    var c;

    for (; inputIndex < input.length; inputIndex++) {

        c = input[inputIndex];

        switch(c) {

           case '%':
               output[outputIndex++] = '%';
               output[outputIndex++] = '2';
               output[outputIndex++] = '5';
               break;

           case '$':
               output[outputIndex++] = '%';
               output[outputIndex++] = '2';
               output[outputIndex++] = '4';
               break;

           case ',':
               output[outputIndex++] = '%';
               output[outputIndex++] = '2';
               output[outputIndex++] = 'C';
               break;

           case '*':
               output[outputIndex++] = '%';
               output[outputIndex++] = '2';
               output[outputIndex++] = 'A';
               break;

           case '\r':
               output[outputIndex++] = '%';
               output[outputIndex++] = '0';
               output[outputIndex++] = 'D';
               break;

           case '\n':
               output[outputIndex++] = '%';
               output[outputIndex++] = '0';
               output[outputIndex++] = 'A';
               break;

           case '=':
               output[outputIndex++] = '%';
               output[outputIndex++] = '3';
               output[outputIndex++] = 'D';
               break;

           case ':':
               output[outputIndex++] = '%';
               output[outputIndex++] = '3';
               output[outputIndex++] = 'A';
               break;

           default:
               output[outputIndex++] = input[inputIndex];
               break;
        }
    }

    return output;
}

Nmea0183Instance.prototype.unescapeString = function(input) {
    return input;
}

Nmea0183Instance.prototype.setTrace = function(value) {
    this.trace = value;

    if (this.trace) {
        this.setTraceError(true);
    }
}

Nmea0183Instance.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

//
// Allows controlled detailed tracing.
//
Nmea0183Instance.prototype.tracelogLevel = function(level, message) {
    if (this.trace > level) {
        console.log(this.moduleName + ": " + message);
    }
}

Nmea0183Instance.prototype.tracelog = function(message) {
    if (this.trace > 0) {
        console.log(this.moduleName + ": " + message);
    }
}

Nmea0183Instance.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.log(this.moduleName + ": " + message);
    }
}

function createInstance(trace, traceerrorValue, prefix) {
    moduleInstance = new Nmea0183Instance(trace, traceerrorValue, prefix);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
