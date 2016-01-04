
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
// DWEET Console
//
// 02/01/2015
//

//
// NOTE: Only create instances of modules which are
// safe for multiple context access (no globals or shared
// instance data).
//
// If there is shared instance data then just load the factory
// method here and use createInstance() when a new instance
// is launched.
//

// DweetClient is the interface to device Dweet streams
var dweetClientFactory = require('./dweetclient.js');

// This is a table of valid Dweet commands
var dweetcmds = require('./dweetcmds.js');

var cmdconsoleFactory = require('./cmdconsole.js');

var myutil = require("./myutil.js");

var dh = require("./dumphex.js").createInstance();

function DweetConsole(trace, traceerrorValue) {

    this.moduleName = "DweetConsole";
    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }
}

//
// Launch Dweet console
//
// callback(error)
//
//    qrgs.trace       // true to log/trace verbose activity/debug information
//    args.traceerror  // true to log/trace errors
//    args.log         // log stream, similar to console.log
//    args.error       // error stream, similar to console.error
//    args.stdin       // std input stream
//    args.stdout      // std output stream
//    args.stderr      // std error stream
//    args.prefix      // NMEA 0183 prefix, such as $PDWT
//    args.script      // Script to run if != null
//    args.stopOnError // stop processing script on error line if true
//    args.openConsole // open interactive console at end of script if true
//    args.apphandler  // Application specific handler
//    args.config      // application config.json file such as lighthouse.json
//    args.deviceName  // device name to open if != null
//
// dweet.js, Launch(),
//   dweet.js, RunCommandLineMain
//     template.js, programEntry
//       template.js, templateMain
//         usermain.js, usermain.userMain   // this creates args
//           dweetconsole.js, ConsoleMain
//
// This entry point launches dweet from a command line.
//
DweetConsole.prototype.ConsoleMain = function(args, exitFunction) {

    // Context for state that the invoked functions can access
    var context = new Object();

    context.trace = args.trace;
    context.traceerror = args.traceerror;

    // Streams
    context.log = args.log;
    context.error = args.error;
    context.stdin = args.stdin;
    context.stdout = args.stdout;
    context.stderr = args.stderr;

    // Control variables
    context.defaultPrefix = args.prefix;
    context.defaultPortName = args.deviceName;
    context.script = args.script;
    context.exitWhenDone = !args.openConsole;
    context.stopOnError = args.stopOnError;

    // User application handlers
    context.apphandler = args.apphandler;

    // application config file
    context.config = args.config;

    // start the console command processing loop
    // dweetconsole.js, interactiveConsole
    this.interactiveConsole(context, function(error) {
        exitFunction(error);
    });
}

//
// Dump buffer that is ASCII hex value as a binary hexodec dump.
//
function dumpAsciiHexBuffer(buffer, addressBase) {

    var pair;
    var binBuffer = new Array();

    var binBufferIndex = 0;

    for (var index = 0; index < buffer.length; index += 2) {

        pair = "";
        pair += buffer[index];
        pair += buffer[index+1];

        binBuffer[binBufferIndex] = myutil.asciiHexToBinary(pair);

        binBufferIndex++;
    }

    //console.log("binBuffer=");
    //console.log(binBuffer);

    // Make the Array() into a string
    //binBuffer = binBuffer.toString();

    dh.dumpHexWithAddressBase(binBuffer, 0, binBuffer.length, addressBase, function(data) {
        console.log(data);
    });
}

//
// Set Dweet Client as the current default.
//
// context.client        // dweetclient.js instance
// context.client.dweet  // dweethandler.js instance
//
DweetConsole.prototype.SetCurrentChannel = function(context, client) {
    context.client = client;
    context.current_channel = client;
}

//
// context.trace
// context.traceerror
// context.defaultPortName
// context.defaultPrefix
// context.script
// context.exitWhenDone
// context.stopOnError
//
// Set by this function:
//
// context.dweetConsole = this
// context.showDweetCommands
// context.showNMEACommands
// context.current_channel = null;
// context.channels
// context.channels["default"] = context.current_channel
//
DweetConsole.prototype.ContextInitialize = function(context) {

    //
    // We set the reference to the console instance
    // for non-bound functions to use.
    //
    context.dweetConsole = this;

    // Add specific context information

    //
    // Note: Since these set up an event listener on the NMEA 0183
    // or Dweet stream we can't register till the stream exists by opening
    // the port.
    //
    // see toggleShowCommandsFunction()/toggleShownmeaFunction() for details.
    //
    context.showDweetCommands = false;

    context.showNMEACommands = false;

    //
    // Add support for multiple Dweet channels
    //
    context.current_channel = null;

    // Named array for additional channels
    // These are instances of dweetclient.js
    context.channels = new Object();

    context.channels["default"] = null;
}

//
// Launch interactive console.
//
// Note: it could be scripted, or from an interactive device such as stdin.
//
// context.trace
// context.traceerror
// context.log
// context.error 
// context.stdin
// context.stdout
// context.stderr
// context.defaultPrefix
// context.defaultPortName
// context.script
// context.exitWhenDone
// context.stopOnError
// context.apphandler
// context.config       // application config file such as lighthouse.json
//
// Set by this function:
//
// context.dweetconsole  // dweetconsole.js instance (this module)
// context.client        // dweetclient.js instance
// context.client.dweet  // dweethandler.js instance
// context.commandTable
// context.showDweetCommands
// context.showNMEACommands
// context.current_channel = null;
// context.channels
// context.channels["default"] = context.current_channel
//
// Return:
//
// callback(error)
//  error != null => error open port, initializing, etc.
//  error == null => normal exit process due to quit, end of command file, etc.
//
// dweetconsole.js, ConsoleMain   // this creates the context
//   dweetconsole.js, interactiveConsole
//
DweetConsole.prototype.interactiveConsole = function(context, callback) {

    var msg = null;
    var options = null;

    context.dweetconsole = this;

    // Create the cmdconsole
    context.cmdconsole = cmdconsoleFactory.createInstance();

    options = new Object();
    options.trace = context.trace;
    options.traceerror = context.traceerror;

    //
    // Setup a command table to execute.
    //
    var commandTable = new Array();

    commandTable[0] = dweetCommandTable;

    // Use the default built in command table for help, ?, etc.
    commandTable[1] = context.cmdconsole.defaultCommandTable;

    // Save our local state in context
    context.commandTable = commandTable;

    this.ContextInitialize(context);

    var captured_this = this;

    // Open the default port if specified
    if (context.defaultPortName != null) {

	openDweetClient(context, context.defaultPortName, context.tag, context.defaultPrefix,
	    options, context.trace, context.traceerror, function(error, dweetClient) {

	    if (error != null) {
		msg = "can't open port " + context.defaultPortName + " error=" + error;
		callback(msg);
		return;
	    }

	    //
	    // Set dweetClient as the current default
	    //
	    // context.client        // dweetclient.js instance
	    // context.client.dweet  // dweethandler.js instance
	    //
	    context.channels["default"] = dweetClient;

	    captured_this.SetCurrentChannel(context, dweetClient);

	    // Start a console reader to process input commands
	    context.cmdconsole.cmdConsole(commandTable, context, function(error) {
		callback(error);
	    });
	});
    }
    else {
        // Launch the console right away
        context.cmdconsole.cmdConsole(commandTable, context, function(error) {
            callback(error);
        });
    }
}

//
// function commands are called with:
//
// function(cmd, context, entry, callback);
//
//   cmd     - The full command line
//
//   context - context from caller. context.client == dweetclient
//
//   entry   - The entry in the command table
//             Allows extra local parameters in entry
//
//   callback - function to invoke with result as callback(error)
//

var dweetHelp = {
    "name": "help",
    "function": dweetHelpFunction
};

var dweetHelp_questionmark = {
    "name": "?",
    "function": dweetHelpFunction
};

var settrace = {
    "name": "settrace",
    "function": settraceFunction,
    "helpFunction": singleCommandHelpFunction
};

// Open Dweet channel
var dweetopen = {
    "name": "open",
    "function": openCommandFunction,
    "helpFunction": singleCommandHelpFunction
};

// Close a Dweet channel
var dweetclose = {
    "name": "close",
    "function": closeCommandFunction,
    "helpFunction": singleCommandHelpFunction
};

// Switch a Dweet channel
var dweetswitch = {
    "name": "switch",
    "function": switchCommandFunction,
    "helpFunction": singleCommandHelpFunction
};

// Display which Dweet channel is currently the target of console input commands
var dweetwho = {
    "name": "who",
    "function": whoCommandFunction,
    "helpFunction": singleCommandHelpFunction
};

// Configure a Dweet channel as a radio gateway
var radiogateway = {
    "name": "radiogateway",
    "function": radiogatewayCommandFunction,
    "helpFunction": singleCommandHelpFunction
};

// Show Dweet command exchange
var showcommands = {
    "name": "showcommands",
    "function": toggleShowCommandsFunction,
    "helpFunction": singleCommandHelpFunction
};

// alias for above
var showdweet = {
    "name": "showdweet",
    "function": toggleShowCommandsFunction,
    "helpFunction": singleCommandHelpFunction
};

// Show NMEA0183 command exchange
var shownmea = {
    "name": "shownmea",
    "function": toggleShownmeaFunction,
    "helpFunction": singleCommandHelpFunction
};

// Send a single hand coded Dweet such as "GETCONFIG=NAME"
var sendDweet = {
    "name": "dweet",
    "function": dweetCommandFunction,
    "helpFunction": singleCommandHelpFunction
};

//
// Send a single hand coded NMEA 0183 message
// "$PDWT,GETCONFIG=NAME*00"
//
// Checksum *00 is automatically filled in with the correct
// value.
//
var sendNMEA0183 = {
    "name": "nmea",
    "function": nmeaCommandFunction,
    "helpFunction": singleCommandHelpFunction
};

//
// Prototype of new command format:
// setstate object="value"
//
var getstate = {
    "name": "getstate",
    "function": getStateFunction,
    "helpFunction": operationHelpFunction
};

var setstate = {
    "name": "setstate",
    "function": setStateFunction,
    "helpFunction": operationHelpFunction
};

var getconfig = {
    "name": "getconfig",
    "function": getConfigFunction,
    "helpFunction": operationHelpFunction
};

var setconfig = {
    "name": "setconfig",
    "function": setConfigFunction,
    "helpFunction": operationHelpFunction
};

var read = {
    "name": "read",
    "function": readFunction,
    "helpFunction": operationHelpFunction
};

var write = {
    "name": "write",
    "function": writeFunction,
    "helpFunction": operationHelpFunction
};

var pinmode = {
    "name": "pinmode",
    "function": stubFunction,
    "helpFunction": singleCommandHelpFunction
};

var digitalread = {
    "name": "digitalread",
    "function": stubFunction,
    "helpFunction": singleCommandHelpFunction
};

var digitalwrite = {
    "name": "digitalwrite",
    "function": stubFunction,
    "helpFunction": singleCommandHelpFunction
};

var aref = {
    "name": "analogreference",
    "function": stubFunction,
    "helpFunction": singleCommandHelpFunction
};

var analogread = {
    "name": "analogread",
    "function": stubFunction,
    "helpFunction": singleCommandHelpFunction
};

var analogwrite = {
    "name": "analogwrite",
    "function": stubFunction,
    "helpFunction": singleCommandHelpFunction
};

var dweetCommandTable = [

    // arduino
    pinmode,
    digitalread,
    digitalwrite,
    aref,
    analogread,
    analogwrite,

    // read/write memory, registers, eeprom, flash
    read,
    write,

    // get/set config
    getconfig,
    setconfig,

    // get/set state
    getstate,
    setstate,

    // built ins
    showcommands,
    showdweet,
    shownmea,
    sendDweet,
    sendNMEA0183,

    // Open/close Dweet channels
    dweetopen,
    dweetclose,
    dweetswitch,
    dweetwho,

    // Radio gateway
    radiogateway,

    settrace,
    dweetHelp,
    dweetHelp_questionmark,
];

function stubFunction(cmd, context, entry, callback) {
    console.log("cmd=" + cmd);
    callback(null);
}

function singleCommandHelpFunction(cmdEntry, context) {
    console.log("    " + cmdEntry.name);
}

function settraceFunction(cmd, context, entry, callback) {
    var operation = "settrace";
    var usage = "usage: settrace level\nSets trace level on current channel. 0 == off";
    var msg = null;

    var c = processCommandLine(cmd);
    if (c.error != null) {
        msg = c.error + " " + usage;
        callback(msg, null);
        return;
    }

    if (c.args.length <= 1) {
        msg = "level parameter missing " + usage;
        callback(msg, null);
        return;
    }

    var traceLevel  = c.args[1];

    if (context.current_channel == null) {
         msg = "no channel open";
         callback(msg, null);
         return;
    }

    //
    // See SetCurrentChannel()
    //
    // context.current_channel        // dweetclient.js instance
    // context.current_channel.dweet  // dweethandler.js instance
    //
    context.current_channel.setTrace(traceLevel);

    msg = "new tracelevel " + traceLevel;
    msg += " set on " + context.current_channel.tag;

    callback(null, msg);
}

//
// Open Dweet Client
//
// callback(error, dweetclient)
//
// dweetclient is an instance of dweetclient.js
//
// context.config       // application config file such as lighthouse.json
//
// options:
//    options.trace = context.trace;
//    options.traceerror = context.traceerror;
//
// caller:
//
// dweetconsole.js, interactiveConsole()
//
function openDweetClient(context, portName, tag, prefix, options, trace, traceerror, callback) {

    //
    // Open Dweet client on supplied port
    // this is an instance of dweetclient.js
    //

    // Set application handler on options
    options.apphandler = context.apphandler;

    // application config file such as lighthouse.json
    options.config = context.config;

    // dweetclient.js
    var client = dweetClientFactory.createInstance(trace, traceerror, prefix);

    //
    // Register Dweet callbacks for PRINT, PRINT2 debug outputs
    //
    setupDweetCommandEvents(client, client.dweet);

    // dweetclient.js, openPort
    client.openPort(portName, options, function(error) {

        var msg = null;

        if (error != null) {
            msg = "can't open port " + portName + " error=" + error;
            console.log(msg);
            callback(msg, null);
            return;
        }

        // Set our tag on the client instance
        client.tag = tag;

        //
        // Send a sync on newly opened channels to allow the target
        // to know that commands will be arriving.
        //
        client.dweet.sendSync(function(syncError) {

            if (syncError != null) {
                console.log("sendSync: error=" + syncError);
            }

            // Processing will occcur as events are received
            callback(null, client);
        });
    });
}

//
// Setup default streamed commands from the device.
//
// When performing command/response the expected reply events are
// dynamically setup and then cancelled after the reply.
//
// But general long running "streaming" Dweets are unsolicited, so we
// register for those here. Debugging stream is the most common, but
// others maybe added through the command execution system.
//
function setupDweetCommandEvents(client, dweet) {

    //
    // client.dweet is an EventEmitter which indicates on a completed
    // dweet command. A received Dweet sentence may contain multiple
    // commands or command replies and this will generate multiple
    // events. This allows applications to ignore issues such as the
    // number of commands packed into any given Dweet sentence.
    //
    // var client = require('./dweetclient.js').createInstance(prefix, portName)
    //   client.dweet = require('./dweethandler.js').createInstance(prefix);
    //     // DweetHandler is an EventEmitter
    //     util.inherits(client.dweet, events.EventEmitter);
    //       client.dweet.processSentenceAsEvents()
    //         client.dweet.processCommandAsEvent()
    //           client.dweet.emit(nv.name, nv.name, nv.value, o.prefix);
    //

    // Debug support on prefix $PDBG
    dweet.on('PRINT', function(name, value, prefix) {
        processDebugDweet(client, name, value, prefix);
    });

    dweet.on('PRINT2', function(name, value, prefix) {
        processDebugDweet(client, name, value, prefix);
    });
}

//
// Process Dweets arriving from the device
//
function processDeviceDweet(name, value, prefix) {

    var logString = "processDeviceDweet " + prefix + " " + name + "=" + value;
    var ar;

    if (name == "GETCONFIG_REPLY") {
        ar = value.split(':');
        console.log(ar[0] + " is " + ar[1]);
    }
    else {
        console.log(logString);
    }
}

//
// Shows the debugger print output on the console.
//
function processDebugDweet(client, name, value, prefix) {

    var logString = "processDebugDweet " + prefix + " " + name + "=" + value;

    var tag = "";

    //
    // tag is set after port open, though we should not get
    // Dweet events until then.
    //
    if ((typeof(client.tag) != "undefined") &&
        (client.tag != null)) {
        tag = client.tag;
    }

    //console.log(logString);

    if (name == "PRINT") {
        console.log(tag + " DBG: " + value);
    }
    else if(name == "PRINT2") {
        console.log(tag + " DBG2: " + value);
    }
    else {
        logString += tag;
        logString += " unknown debug Dweet";
        console.log(logString);
    }
}

//
// Toggle the showing of all Dweet send/receive traffice
//
function toggleShowCommandsFunction(cmd, context, entry, callback) {
    var operation = "showcommands";

    if (context.client == null) {
        callback("no device open, use open device_name or open testdevice", args, null);
        return;
    }

    var captured_this = this;

    // dweethandler.js instance
    var dweetHandler = context.client.dweet;

    var dweetReceiveListener = function(eventName, error, fullCommand, reply_o) {
            var msg = "DWEET<:" + fullCommand;

            if (error != null) {
                msg += "(ERROR=" + error + ")";
            }

            console.log(msg);
    };

    var dweetSendListener = function(eventName, error, fullCommand, reply_o) {
            var msg = "DWEET>:" + fullCommand;

            if (error != null) {
                msg += "(ERROR=" + error + ")";
            }

            console.log(msg);
    };

    if (context.showDweetCommands) {

        console.log(operation + " Dweet commands and responses will not be shown");
        context.showDweetCommands = false;

        dweetHandler.removeListener('receive', dweetReceiveListener);
        dweetHandler.removeListener('send', dweetSendListener);
    }
    else {
        console.log(operation + " Dweet commands and responses will be shown");
        context.showDweetCommands = true;


        dweetHandler.on('receive', dweetReceiveListener);
        dweetHandler.on('send', dweetSendListener);
    }

    callback(null);
}

//
// Toggle the showing of all NMEA 0183 send/receive traffice
//
function toggleShownmeaFunction(cmd, context, entry, callback) {
    var operation = "shownmea";

    if (context.client == null) {
        callback("no device open, use open device_name or open testdevice", args, null);
        return;
    }

    var captured_this = this;

    // dweethandler.js instance
    var dweetHandler = context.client.dweet;

    var nmeaReceiveListener = function(eventName, error, sentence, parsed_sentence) {
            var chksum = null;

            //
            // Note: Raw NMEA 0183 sentence contains the ending "\r\n" which
            // must be removed for display.
            //
            sentence = removeLineEndings(sentence);

            if (parsed_sentence.checksumOK) {
                chksum = "(checkSumOK)";
            }
            else {
                chksum = "(badCheckSum)";
            }

            var msg = "NMEA<:" + sentence + chksum;
            console.log(msg);
    };

    var nmeaSendListener = function(eventName, error, sentence) {
            var chksum = null;

            //
            // Note: Raw NMEA 0183 sentence contains the ending "\r\n" which
            // must be removed for display.
            //
            sentence = removeLineEndings(sentence);

            var msg = "NMEA>:" + sentence;
            console.log(msg);
    };

    if (context.showNMEACommands) {

        console.log(operation + " NMEA 0183 commands and responses will not be shown");
        context.showNMEACommands = false;

        dweetHandler.nmea.removeListener('receive', nmeaReceiveListener);
        dweetHandler.nmea.removeListener('send', nmeaSendListener);
    }
    else {
        console.log(operation + " NMEA 0183 commands and responses will be shown");
        context.showNMEACommands = true;

        dweetHandler.nmea.on('receive', nmeaReceiveListener);
        dweetHandler.nmea.on('send', nmeaSendListener);
    }

    callback(null);
}

//
// Remove "\r\n" if present.
//
function removeLineEndings(s) {

    // First trim '\n'
    if ((s.length > 0) && (s[s.length - 1] == '\n')) {
        s = s.substring(0, s.length - 1);
    }

    // Next trim '\r'
    if ((s.length > 0) && (s[s.length - 1] == '\r')) {
        s = s.substring(0, s.length - 1);
    }

    return s;
}

//
// dweet command
// dweet dweet=command
// dweet dweet=command:value
//
// dweetFunction
//
// callback(error, result)
//
function dweetCommandFunction(cmd, context, entry, callback) {
    var operation = "dweetcommand";
    var error;

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(c.error, null);
        return;
    }

    if (c.args.length <= 1) {
        callback("dweet: command parameter missing", null);
        return;
    }

    // Create args object
    var args = new Object();
    args.prefix = "$PDWT";

    // Set defaults. They can be overridden by a table entry
    args.timeout = 10 * 1000;
    args.retryLimit = 1;

    // Parse the command into the Dweet component parts
    var command  = c.args[1];

    // Most Dweet commands are command=operation
    var c_nv = processNameValueArgument(command);
    if (c_nv.error != null) {
        error = "dweet command argument error " + c_nv.error;
        console.log(error);
        callback(error, null);
        return;
    }

    // fill in the command values
    args.command = c_nv.name;

    // Value could be null if command stands along without "="
    if (c_nv.value != null) {

        var v_nv = processNameValueArgument(c_nv.value);
        if (v_nv.error != null) {
            error = "dweet command argument error " + v_nv.error;
            console.log(error);
            callback(error, null);
            return;
        }

        args.object = v_nv.name;
        args.value = v_nv.value; // could be null
    }
    else {
        args.object = null;
        args.value = null;
    }

    // We don't have a compare object for the responses
    args.compareObject = null;

    commandFunctionArgs(args, context, function(error, o, result) {

        if (error != null) {
            console.log(error);
            callback(error, null);
            return;
        }

        console.log(result);

        callback(error, null);
    });
}

//
// Allows sending low level NMEA 0183 messages.
//
// The messages are in NMEA 0183 format and don't have
// to follow the Dweet format. This allows standard NMEA
// 0183 messages and prefixes to be sent to existing
// instruments.
//
// Note: *00 is allowed as a place holder for the check
// sum and is filled in by the NMEA 0183 sender with
// the correctly calculated one.
//
// nmea "$PDWT,word,data,word*00"
//
function nmeaCommandFunction(cmd, context, entry, callback) {
    var operation = "nmeacommand";
    var error;

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(c.error);
        return;
    }

    if (c.args.length <= 1) {
        callback("nmea: NMEA 0183 sentence");
        return;
    }

    // Get the sentence.
    var sentence  = c.args[1];

    //
    // We use the dweethandler to pass through the NMEA 0183
    // request since its managing the connection to the device.
    //

    // dweethandler.js instance
    var dweetHandler = context.client.dweet;

    //
    // Since any NMEA 0183 command may be entered we can't
    // expect a response, or whether any future received NMEA 0183
    // sentences are related in any way to the sentence
    // we send.
    //
    // In order to handle this general NMEA 0183 messages not recognized
    // as part of a command transactions should be output similar
    // to the debug messages.
    //
    // Settings can control whether these should be visible or not.
    //

    dweetHandler.sendNMEASentence(sentence, function(error) {
        var err = null;

        if (error != null) {
            err = "error sending NMEA sentence " + error;
            console.log(err);
        }

        callback(err);
    });
}

//
// getstate object
//
function getStateFunction(cmd, context, entry, callback) {
    var operation = "getstate";

    return commandFunction(cmd, operation, context, callback);
}

//
// setstate object=value
// setstate object="value"
//
function setStateFunction(cmd, context, entry, callback) {
    var operation = "setstate";

    return commandFunction(cmd, operation, context, callback);
}

//
// getconfig object
//
function getConfigFunction(cmd, context, entry, callback) {
    var operation = "getconfig";

    return commandFunction(cmd, operation, context, callback);
}

//
// setconfig object=value
// setconfig object="value"
//
function setConfigFunction(cmd, context, entry, callback) {
    var operation = "setconfig";

    return commandFunction(cmd, operation, context, callback);
}

//
// read mem addr [size]
// read eeprom addr [size]
// read register addr [size]
// read flash addr [size]
//
// size can be length in bytes, maximum 16.
//
// can also be b, w, d, dq for 1, 2, 4, 8 bytes
//
// Returns:
//
//   callback(error);
//
//   prints to the command console any output
//
function readFunction(cmd, context, entry, callback) {
    var operation = "read";

    var table = dweetcmds.commandtable;

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(c.error);
        return;
    }

    // read mem addr [size]
    if (c.args.length <= 1) {
        callback("read: operation type parameter missing");
        return;
    }

    if (c.args.length <= 2) {
        callback("read: addr parameter missing");
        return;
    }

    if (c.operation != operation) {
       callback(operation + " no space before arguments");
       return;
    }

    // Default is 1 byte
    var size = "1";

    // TODO: Must map size from "d" -> 4, "w" -> 2, etc.

    if (c.args.length >= 4) {
        size = c.args[3];
    }

    // type of memory space to read is the second argument
    var spaceType = c.args[1];

    // address is the third argument
    var address = c.args[2];

    //console.log("read: spaceType=" + spaceType + " address= " + address + " size=" + size);

    //
    // Map c.operation (read) + spaceType to:
    //
    // read mem -> operation=read dataName=memory
    // read eeprom -> operation=read dataName=eeprom
    // read register -> operation=read dataName=register
    // read flash -> operation=read dataName=flash
    // 
    var lookupDataName = mapSpaceType(spaceType);
    if (lookupDataName == "unknown") {
        error = "error: unknown space type " + spaceType;
        console.log(error);
        callback(error);
        return;
    }

    //
    // Args describe the low level Dweet command exchange
    //
    var args = new Object();
    args.prefix = "$PDWT";

    //
    // Lookup our DWEET command entry
    //
    // Our dataName == mapped space type
    //
    // table has:
    //  entry.operation == "read"
    //  entry.dataName == "memory"
    //
    var entry = lookupOperationObjectEntry(
        table,
        args.prefix,
        operation,
        lookupDataName
        );

    if (entry == null) {
        error = c.operation + ": " + lookupDataName + " object type not found";
        error += " prefix=" + args.prefix;
        callback(error);
        return;
    }

    // fill in the command values
    args.command = entry.command;
    args.timeout = entry.timeout;
    args.retryLimit = entry.retryLimit;

    // context.client is our connection and is a dweetclient.js instance
    // context.client.dweet is an dweethandler.js instance

    args.object = address;
    args.value = size;

    // We don't have a compare object for the responses
    args.compareObject = null;

    //console.log("read: args.command=" + args.command);

    //console.log("args.object=" + args.object + " args.value=" + args.value);

    commandFunctionArgs(args, context, function(error, o, result) {

        if (error != null) {
            console.log(error);
            callback(error);
            return;
        }

        // o.object is the address being operated on
        dumpAsciiHexBuffer(result, o.object);
        console.log("");

        var str = o.object + ": ";

        for (var index = 0; index < result.length; index += 2) {

            if (index != 0) {
                str += " ";
            }

            str += result[index];
            str += result[index+1];
        }

        console.log(str);

        callback(error);
    });
}

//
// write mem addr=value
// write eeprom addr=value
// write register addr=value
//
// size is determined by digits specified, rounded up to natural
// type size.
//
//        0 = byte
//       00 = byte
//      000 = word
//     0000 = word
//    00000 = 32 bit longword
//   000000 = ""
//  0000000 = ""
// 00000000 = ""
//
// Returns:
//
//   callback(error);
//
function writeFunction(cmd, context, entry, callback) {

    var operation = "write";

    var table = dweetcmds.commandtable;

    var error;

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(c.error);
        return;
    }

    // write type addr=value
    if (c.args.length <= 1) {
        callback("write: operation type parameter missing");
        return;
    }

    if (c.args.length <= 2) {
        callback("write: addr parameter missing");
        return;
    }

    var nv = processNameValueArgument(c.args[2]);
    if (nv.value == null) {
        callback("missing data value");
        return;
    }

    // type of memory space to read is the second argument
    var spaceType = c.args[1];

    // address is the third argument
    var address = nv.name;
    var dataValue = nv.value;

    // Default is 1 byte
    // The size is based on digits sent in the dataValue
    var size = (dataValue.length / 2) + 1;

    console.log("write: spaceType=" + spaceType + " address= " + 
                 address + " value=" + dataValue + " size=" + size);

    //
    // Map c.operation (write) + spaceType to:
    //
    // write mem -> operation=read dataName=memory
    // write eeprom -> operation=read dataName=eeprom
    // write register -> operation=read dataName=register
    // write flash -> operation=read dataName=flash
    // 
    var lookupDataName = mapSpaceType(spaceType);
    if (lookupDataName == "unknown") {
        error = "error: unknown space type " + spaceType;
        console.log(error);
        callback(error);
        return;
    }

    //
    // Args describe the low level Dweet command exchange
    //
    // MEMWRITE=addr:value
    //
    var args = new Object();
    args.prefix = "$PDWT";

    //
    // Lookup our DWEET command entry
    //
    // Our dataName == mapped space type
    //
    // table has:
    //  entry.operation == "write"
    //  entry.dataName == "memory"
    //
    var entry = lookupOperationObjectEntry(
        table,
        args.prefix,
        operation,
        lookupDataName
        );

    if (entry == null) {
        error = c.operation + ": " + lookupDataName + " object type not found";
        error += " prefix=" + args.prefix;
        callback(error);
        return;
    }

    // fill in the command values
    args.command = entry.command;
    args.timeout = entry.timeout;
    args.retryLimit = entry.retryLimit;

    args.object = address;
    args.value = dataValue;

    // We don't have a compare object for the responses
    args.compareObject = null;

    //console.log("write: args.command=" + args.command);

    //console.log("args.object=" + args.object + " args.value=" + args.value);

    commandFunctionArgs(args, context, function(error, o, result) {

        console.log(o.object + ": " + o.value);

        callback(error);
    });
}

function mapSpaceType(op) {
    var operationType = "unknown";

    if (op == "mem") {
        operationType = "memory";
    }
    else if (op == "memory") {
        operationType = "memory";
    }
    else if (op == "eeprom") {
        operationType = "eeprom";
    }
    else if (op == "reg") {
        operationType = "register";
    }
    else if (op == "register") {
        operationType = "register";
    }
    else if (op == "flash") {
        operationType = "flash";
    }

    return operationType;
}

//
// dweetHelpFunction: Show list of available commands
// from the command table.
//
// callback(error, result)
//
function dweetHelpFunction(cmd, context, entry, callback) {
    console.log("dweetHelp: Available Commands:");

    for (var entryIndex in dweetCommandTable) {
        var entry = dweetCommandTable[entryIndex];

        // If the entry has a help function, run it
        if (typeof(entry.helpFunction) != "undefined") {
            entry.helpFunction(entry, context);
        }
        else {
            console.log(entry.name);
        }
    }

    //
    // Run help against the default table
    // to get its base help output
    //
    var ar = new Array();

    ar[0] = context.cmdconsole.defaultCommandTable;
    context.cmdconsole.executeCommand(ar, cmd, context, callback);
}

//
// Help function that displays the commands available
// for the given operation given in cmdEntry.name.
//
// It does this by matching it with .operation
// in the Dweet protocol commands table.
//
function operationHelpFunction(cmdEntry, context) {

    console.log(cmdEntry.name + " available commands:");

    // dweetcmds.js
    var table = dweetcmds.commandtable;

    var entry = null;

    for (var entryIndex in table) {
        entry = table[entryIndex];

        // dweetcmd.operation == cmdentry.name
        if (entry.operation.toLowerCase() == cmdEntry.name) {
            console.log("    " + entry.dataName);
        }
    }
}

//
// Returns:
//
//   nv.error
//   nv.name
//   nv.value
//   nv.quoted
//
function processNameValueArgument(arg) {
    var nv = new Object();

    nv.quoted = false;
    nv.error = null;

    if (arg.indexOf('=') == (-1)) {
        // just name, no value
        nv.name = arg;
        nv.value = null;
    }
    else {
        // name=value
        r = arg.split('=');
        nv.name = r[0];
        nv.value = r[1];
    }

    //
    // If there is a value, process it to remove any
    // quotes and ensure there are no spaces.
    //
    if (nv.value != null) {

        var qa = processQuotedArgument(nv.value);
        if (qa.error != null) {
            nv.error = qa.error;
            return nv;
        }

        nv.value = qa.arg;
        nv.quoted = qa.quoted;
    }

    return nv;
}

//
// Process an argument that may have quotes.
//
// Removes the quotes and returns the base string.
//
// Returns:
//
//   result.error
//   result.arg
//   result.quoted
//
function processQuotedArgument(arg) {

    var result = new Object();
    result.error = null;
    result.arg = null;
    result.quoted = false;

    //
    // Perform input validation.
    //
    // If the parameter value is in quotes, we currently
    // don't allow spaces.
    //
    // We remove the quotes before returning it.
    //
    if ((arg[0] != '"') && (arg[arg.length - 1] != '"')) {

        // no quotes
        result.arg = arg;
        return result;
    }

    // There was a least one quote, make sure both are there
    if ((arg[0] != '"') || (arg[arg.length - 1] != '"')) {
        result.error = "quotes unbalanced in argument value";
        return result;
    }

    // Remove the quotes ""
    arg = arg.substring(1, arg.length - 1);

    // Current do not allow spaces
    if (arg.indexOf(' ') != (-1)) {
        result.error = "spaces not allowed in argument value"
        return result;
    }

    result.arg = arg;
    result.quoted = true;

    return result;
}

//
// Lookup a command=object:value entry in which
// the operation and object must match.
//
function lookupOperationObjectEntry(table, prefix, operation, dataName) {

    if ((dataName == null) || (dataName == "")) {
        var error = operation + ": no object specified";
        console.log(error);
        return null;
    }

    //
    // lookup the name in commandtable
    // dataName is the object being operated on
    //
    var entry = null;

    for (var entryIndex in table) {
        entry = table[entryIndex];

        // Command entry must match prefix, get operation, data being requested
        if (entry.prefix == prefix) {
            if (entry.operation.toLowerCase() == operation.toLowerCase()) {
                if (entry.dataName.toLowerCase() == dataName.toLowerCase()) {
                    break;
                }
            }
        }

        entry = null;
    }

    return entry;
}

//
// getstate object
//   args.operation "getstate"
//   args.object = "object"
//   args.value = null;
//
// setstate object="value"
//   args.operation == "setstate"
//   args.object = "object"
//   args.value = "value"
//
// memread addr
//   args.operation == "memread"
//   args.object = "addr"
//   args.value = null
//
// db addr
//   args.operation == "db"
//   args.object = "addr"
//   args.value = null
//
// memwrite addr=data
//   args.operation == "memwrite"
//   args.object = "addr"
//   args.value = data
//
// dumpmemory addr=length
//   args.operation == "dumpmemory"
//   args.object = addr
//   args.value = length
//
// dumpeeprom addr=length
//   args.operation == "dumpeeprom"
//   args.object = addr
//   args.value = length

//
// command object object=value ...
//
// Returns:
//
//   args.error
//   args.operation
//   args.args
//     args[0] == command
//     args[1]  == object=value
//       ...
//
function processCommandLine(cmd) {

    var args = new Object();
    args.error = null;

    var a = cmd.split(' ');
    if (a.length <= 1) {
        args.error = "no arguments specified";
        return args;
    }

    var args = new Object();
    args.args = a;
    args.operation = a[0];

    return args;
}

//
// Process arguments then call worker function.
//
// This takes command lines in the following forms and
// looks up the operation in the Dweet command table for
// the message exchange parameters with the remote device.
//
// operation object
// operation object="value"
// operation object=value
//
// callback(error)
// 
// KEYWORDS: processCommandFunction
//
function commandFunction(cmd, operation, context, callback) {

    var table = dweetcmds.commandtable;

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(c.error);
        return;
    }

    if (c.args.length < 2) {
        callback("parameter missing");
        return;
    }

    if (c.operation != operation) {
       callback(operation + " no space before arguments");
       return;
    }

    //
    // Args describe the low level Dweet command exchange
    //
    var args = new Object();
    args.prefix = "$PDWT";

    // parameter to operation is the second argument
    var arg1 = c.args[1];

    var nv = processNameValueArgument(arg1);
    if (nv.value == null) {
        args.object = nv.name;
        args.value = null;
    }
    else {
        args.object = nv.name;
        args.value = nv.value;
    }

    // nv.quoted indicates if quotes were present

    // Lookup our DWEET command entry
    var entry = null;
    entry = lookupOperationObjectEntry(table, args.prefix, c.operation, args.object);
    if (entry == null) {
        error = c.operation + ": " + args.object + " object type not found";
        callback(error);
        return;
    }

    if (entry.valuerequired && (args.value == null)) {

        // If the command requests a value, its an error if its missing.
        error = "= required, example: " + args.operation + " " + args.object + "=\"value\"";
        callback(error);
        return;

       //console.log("cmdLine=" + options.operation + " " + options.object + " " + entry.dataValue);
    }

    // fill in the command values
    args.command = entry.command;
    args.timeout = entry.timeout;
    args.retryLimit = entry.retryLimit;

    //
    // We fill in the name from the table since its typically
    // case sensitive from what was typed on the command line.
    //
    // getconfig name -> GETCONFIG=NAME
    //
    args.object = entry.dataName;

    // We look for the object name in responses
    args.compareObject = args.object;

    commandFunctionArgs(args, context, function(error, o, result) {

        //
        // o is args + added fields
        //
        // o.object == "NAME"
        // o.result == reply
        //
        // o.fullCommand  -> Dweet Sent
        // o.fullResponse -> Dweet Received
        //
        if (error == null) {
            // result == reply data from remote such as "Device1"
            console.log(o.object + ": " + result);
        }

        callback(error);
    });
}

//
// The worker function is not concerned with command
// and argument format as the caller takes care of it.
//
// Arguments:
//
// args.error
// args.result
// args.fullCommand
// args.fullReply
// args.retryCount
// args.prefix
// args.timeout
// args.retryLimit
// args.command
// args.object
// args.value
// args.compareObject
//
// Fields updated by transaction:
//
// args.fullCommand
// args.error
// args.result
// args.retryCount
//
// dweetconsole.dweetCommandFunction(cmd, context, entry, callback)
//   - allocates args
//
// dweetconsole.commandFunctionArgs(args, context, callback)
//   dweethandler.commandTransaction(args, callback)
//     dweethandler.exchangeCommand(args, compareFunc, callback)
//       dweethandler.sendAndFlush(args, callback)
//         nmea0183.sendAndFlush(args, callback)
//           nmea0183.nmeaSender(args, callback)
//           nmea0183.flush(callback);
//
// Return:
//
// callback(error, args, result)
//
//   - args contains updated fields
//
function commandFunctionArgs(args, context, callback) {

    var error = null;
    var dweet = null;

    if (context.client == null) {
        callback("no device open, use open device_name or open testdevice", args, null);
        return;
    }

    dweet = context.client.dweet;

    // dweethandler.js
    dweet.commandTransaction(args,

        function(error, o, result) {

            //
            // o == args + updated fields
            // o.fullCommand
            // o.error
            // o.result
            // o.retryCount
            //
            if (error != null) {
                error = "Error: " + o.fullCommand + " " + error + " fullReply=" + o.fullReply;
                callback(error, o, result);
                return;
            }

            callback(null, o, result);
    });

    return;
}

//
// See ContextInitialize() for full context contents
//
// callback(error, result)
//
function openCommandFunction(cmd, context, entry, callback) {

    var options = null;
    var tag = null;
    var portName  = null;
    var usage = "open portName [channel_tag]";

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(usage, null);
        return;
    }

    if (c.args.length <= 1) {
        callback("dweet: missing device port name", null);
        return;
    }

    portName  = c.args[1];

    if (c.args.length >= 2) {
        tag  = c.args[2];
    }

    //
    // Ensure the portName is not already registered
    //
    entryName = lookupChannelEntry(context, portName);
    if (entryName != null) {
        callback("port name already open", null);
        return;
    }

    // Set apphandler
    var options = new Object();
    options.apphandler = context.apphandler;

    openDweetClient(context, portName, tag, context.defaultPrefix, options,
	    context.trace, context.traceerror, function(error, dweetClient) {

        if (error != null) {
            msg = "can't open port " + context.defaultPortName + " error=" + error;
            callback(msg, null);
	    return;
	}
      
	context.channels[tag] = dweetClient;
	context.dweetConsole.SetCurrentChannel(context, dweetClient);

        msg = "port " + portName + " opened";
        callback(null, msg);
    });

    return;
}

//
// Display the currently active channel receiving console
// input commands.
//
function whoCommandFunction(cmd, context, entry, callback) {

    var msg = null;

    if (context.current_channel == null) {
        msg = "No port is current";
        callback(null, msg);
        return;
    }

    msg = context.current_channel.tag + " [" + context.current_channel.portName + "]";

    msg += displayAvailableChannels(context);

    callback(null, msg);
    return;
}

function displayAvailableChannels(context) {

    //
    // List available channels for selection
    //
    var msg = "";

    msg += "\n\nAvailable channels:\n";

    for (var index in context.channels) {

        var d = context.channels[index];
        if (d == null) {
            continue;
        }

        msg += d.tag + " " + d.portName + "\n";
    }

    return msg;
}

//
// Switch Dweet channel.
//
// switch portName
// switch channelTag
//
function switchCommandFunction(cmd, context, entry, callback) {

    var tag = null;
    var portName  = null;
    var usage = "switch portName|tag";
    var msg;

    var c = processCommandLine(cmd);
    if ((c.error != null) || (c.args.length <= 1)) {

        msg = "dweet: missing device port name\n";
        msg += displayAvailableChannels(context);
        msg += " ";
        msg += usage;

        callback(msg, null);
        return;
    }

    portName  = c.args[1];

    // Lookup the entryName which could be a port or tag
    var entryName = lookupChannelEntry(context, portName);
    if (entryName == null) {
        callback("name " + portName + " not open", null);
        return;
    }

    var dweet = context.channels[entryName];
    if (dweet != null) {

        // Switch current channel to selected one
        context.client = dweet;
	context.current_channel = dweet;
    }

    var msg = portName + " now active";

    callback(null, msg);
    return;
}

//
// callback(error, result)
//
function closeCommandFunction(cmd, context, entry, callback) {

    var usage = "close portName|tag";

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(usage, null);
        return;
    }

    if (c.args.length <= 1) {
        callback("dweet: missing device port name or tag", null);
        return;
    }

    var portName  = c.args[1];

    // Lookup the entryName which could be a port or tag
    var entryName = lookupChannelEntry(context, portName);

    var dweet = context.channels[entryName];
    if (dweet != null) {
    
	context.channels[entryName] = null;

	if (context.current_channel == dweet) {
	
	     context.client = null;
	     context.current_channel = null;
	}

	dweet.close();
    }

    callback(null, "port name " + portName + " closed");
    return;
}

//
// Configure a Dweet channel as a radio gateway
//
function radiogatewayCommandFunction(cmd, context, entry, callback) {

    var options = null;
    var entryName = null;
    var msg = null;
    var tag = null;
    var dweetPortName  = null;
    var radioPortName = null;
    var usage = "radiogateway radioportname portName|tag [radioport_tag]";

    var c = processCommandLine(cmd);
    if (c.error != null) {
        callback(usage, null);
        return;
    }

    if (c.args.length <= 1) {
        callback("dweet: missing radio port name", null);
        return;
    }

    if (c.args.length <= 2) {
        callback("dweet: missing dweet portName or tag", null);
        return;
    }

    // This is the name given to the radio port such as radio0
    radioPortName  = c.args[1];

    if (radioPortName.indexOf("radio") != 0) {
        callback("radio port name must beging with radio prefix such as radio0", null);
        return;
    }

    //
    // Ensure the radioPortName is not already registered
    //
    entryName = lookupChannelEntry(context, radioPortName);
    if (entryName != null) {
        callback("radio port name already open", null);
        return;
    }

    //
    // This is the portName or tag for the already open dweet channel
    // which will be used as the radio gateway using Radio Dweet's.
    //
    dweetPortName  = c.args[2];

    // Set or default the tag
    if (c.args.length > 3) {
        tag = c.args[3];
    }
    else {
        tag = radioPortName;
    }

    // Lookup the entryName which could be a port or tag
    var entryName = lookupChannelEntry(context, dweetPortName);
    if (entryName == null) {
        callback("name " + dweetPortName + " not open", null);
        return;
    }

    var dweetGatewayChannel = context.channels[entryName];
    if (dweetGatewayChannel == null) {
        callback("name " + dweetPortName + " not open", null);
        return;
    }

    //
    // We communicate the dweet client channel to use
    // in options.dweetchannel.
    //
    options = new Object();
    options.dweetchannel = dweetGatewayChannel;

    // Set apphandler
    options.apphandler = context.apphandler;

    openDweetClient(context, radioPortName, tag, context.defaultPrefix, options,
	    context.trace, context.traceerror, function(error, dweetClient) {

        if (error != null) {
            msg = "can't open port " + radioPortName + " error=" + error;
            callback(msg, null);
	    return;
	}
      
        // Insert the radio into the current open dweet channels
	context.channels[tag] = dweetClient;
	context.dweetConsole.SetCurrentChannel(context, dweetClient);

        msg = radioPortName + " now being routed to " + dweetPortName;

        callback(null, msg);
    });
}

//
// Lookup a given channel by name or tag
//
function lookupChannelEntry(context, nameOrTag) {

    var entryName = null;

    if (nameOrTag == null) {
        return null;
    }

    // See if its the tag first
    if (context.channels[nameOrTag] != null) {
        entryName = nameOrTag;
    }
    else {
    
        // Must search for the portName
        for (var index in context.channels) {
    
            var d = context.channels[index];
            if (d == null) {
                continue;
            }
    
            if (d.portName == nameOrTag) {
                entryName = index;
                break;
            }
        }
    }

    // This could null if not found
    return entryName;
}

DweetConsole.prototype.setTrace = function(value) {
    this.trace = value;
}

DweetConsole.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
    this.nmea.setTraceError(value);
}

DweetConsole.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

DweetConsole.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(trace, traceerrorValue) {
    moduleInstance = new DweetConsole(trace, traceerrorValue);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
