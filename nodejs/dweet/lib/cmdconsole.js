
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
// cmdconsole module
//
// 01/25/2015
//

//
// This modules implements a command console.
//

var dh = require("./dumphex.js").createInstance();

var lb = require("./linebuffer.js");

var help = {
    "name": "help",
    "function": helpFunction
};

var question_mark_help = {
    "name": "?",
    "function": helpFunction
};

var exit = {
    "name": "exit",
    "function": exitFunction
};

var quit = {
    "name": "quit",
    "function": exitFunction
};

var q_abbrev = {
    "name": "q",
    "function": exitFunction
};

var bye = {
    "name": "bye",
    "function": exitFunction
};

var localCommandTable = [
    help,
    question_mark_help,
    exit,
    quit,
    q_abbrev,
    bye
];

//
// callback(error, result)
//
function helpFunction(cmd, context, entry, callback) {
    console.log("");
    console.log("help, ? - help");
    console.log("exit, quit, q, bye - exit");
    console.log("");

    callback(null, null);
}

//
// callback(error, result)
//
function exitFunction(cmd, context, entry, callback) {
    console.log("exit:");
    callback("quit", null);
}

//
// This is invoked from the command line or scripts
//
// callback(error, result)
//
CmdConsole.prototype.executeCommand = function (commandTableArray, cmd, context, callback) {
    this.processCommand(commandTableArray, cmd, context, callback);
}

//
// This allows a standard command line to be passed along
//
// cmd arg1 arg2 ...
//
// argv[0] == "cmd"
// argv[1] == "arg1"
// argv[2] == "arg2"
//     ...
//
// callback(error, result)
//
CmdConsole.prototype.executeCommandArgv = function(commandTableArray, argv, context, callback) {

    if (argv.length == 0) {
        callback("no command specified", null);
        return;
    }

    var cmd = argv[0];

    for (var entryIndex = 1; entryIndex < argv.length; entryIndex++) {
        cmd += " ";
        cmd += argv[entryIndex];
    }

    this.processCommand(commandTableArray, cmd, context, callback);
}

//
// commandTable is the caller specified commands and functions
// to execute when they are present.
//
// context.client        // dweetclient.js instance
// context.script
// context.stopOnError
// context.exitWhenDone
//
// context.commandTable[]
// context.showDweetCommands
// context.showNMEACommands
// 
// callback(error, result)
// callback("quit", result) // done
// 
CmdConsole.prototype.cmdConsole = function(commandTableArray, context, callback) {

    var captured_this = this;

    if (context.script != null) {

        // process script
        captured_this.consoleScriptLoop(commandTableArray, context, function(error, result) {

            if (error != null) {
                callback(error, null);
                return;
            }

            // Exit when done unless specified otherwise
            if (context.exitWhenDone) {
                callback("quit", result);
                return;
            }

            // Launch the console reader after the script is done
            context.stdout.write("script done, starting interactive console\n\n");

            captured_this.consoleReaderLoop(commandTableArray, context, callback);
        });
    }
    else {

        // Start the console reader right away
        captured_this.consoleReaderLoop(commandTableArray, context, callback);
    }
}

//
// This is a console reader loop
//
// Note: callback is only invoked on quit.
//
// callback("quit", null)
//
CmdConsole.prototype.consoleReaderLoop = function(commandTableArray, context, callback) {

    // Start a console reader until quit is entered
    console.info("Enter console. Enter ? or help for help.");
    console.info("q, quit, exit to exit.");
    console.log("");

    context.stdout.write("> ");

    var captured_this = this;

    // Function needs a name so we can remove the event listener on quit
    var consoleInputFunction = function(consoleData) {

        consoleData = captured_this.processInputLine(consoleData);

        if (consoleData == "") {
            context.stdout.write("> ");
            return;
        }

        captured_this.processCommand(commandTableArray, consoleData, context, function(error, result) {

            if (error != null) {
                if (error == "quit") {

                    //
                    // Remove the console listener as its no longer being monitored
                    //
                    context.stdin.removeListener('data', consoleInputFunction);
                    callback(error, result);
                    return;
                }

                context.stdout.write(error + "\n");
            }

            // Supplying result is optional
            if ((typeof(result) != "undefined") && (result != null)) {
                context.stdout.write(result + "\n");
            }

            context.stdout.write("> ");
        });
    };

    // Start the reader
    context.stdin.on('data', consoleInputFunction);
}

//
// Process a series of cmdConsole commands from the input script file.
//
// context.client        // dweetclient.js instance
// context.script
// context.stopOnError
// context.exitWhenDone
//
// context.commandTable[]
// context.showDweetCommands
// context.showNMEACommands
//
// callback("quit", null)
//
CmdConsole.prototype.consoleScriptLoop = function(commandTableArray, context, scriptDoneCallback) {

    // Console data is each line processed
    var consoleData = null;

    var captured_this = this;

    var script = context.script;

    var stopOnError = context.stopOnError;

    if (script == null) {
        scriptDoneCallback("null script", null);
        return;
    }

    context.stdout.write("\nexecuting script file actions:\n");

    var linebuffer = lb.createInstance();

    try {
        linebuffer.loadBufferFromFile(script);
    }
    catch(e) {
        scriptDoneCallback("could not load file " + script + " e=" + e);
        return;
    }

    // Load default arguments
    var commandArgs = new Object();
    commandArgs.commandTableArray = commandTableArray;
    commandArgs.context = context;

    //
    // Process a single command line
    //
    var performCommand = function (args) {

        captured_this.processCommand(args.commandTableArray, args.commandLine, args.context,
            function(error, result) {

            if (error != null) {
                if (error == "quit") {
                    args.callback(error, result);
                    return;
                }

                context.stdout.write(error + "\n");

                if (stopOnError) {
                    context.stdout.write("\nstopOnError specified, exiting script\n");
                    args.callback("quit", result);
                    return;
                }
            }

            // Supplying result is optional
            if ((typeof(result) != "undefined") && (result != null)) {
                context.stdout.write(result + "\n");
            }

            // command done
            args.callback(null, result);
        });
    };

    //
    // Get the next valid line from the buffer and process it
    // in the command processor.
    //
    var processLine = function() {

        //
        // linebuffer, commandArgs, performCommand from lambda capture
        // of outer scope function
        //
        consoleData = captured_this.getValidLine(linebuffer);
        if (consoleData == null) {
            // no more lines
            scriptDoneCallback(null, null);
            return;
        }

        // Echo the line from the script
        context.stdout.write("> ");
        context.stdout.write(consoleData + "\n");

        commandArgs.commandLine = consoleData;

        // This callback is invoked when the command is done
        commandArgs.callback = function(error, result) {

            if (error == "quit") {
                scriptDoneCallback(error, result);
                return;
            }

            // process next line
            processLine(linebuffer);
        };

        //
        // Run each command at the top of the turn loop rather than
        // recurse on the stack for each command in the script.
        //
        // Such recursion can also mess up the turn processing
        // of the communications infrastructure as well.
        //
        // commandArgs.commandLine
        //
        setImmediate(performCommand, commandArgs);
    }

    // Kick it off
    processLine(linebuffer);
}

//
// Get a line that is valid for execution.
//
CmdConsole.prototype.getValidLine = function(linebuffer) {

    var consoleData = null;

    while((consoleData = linebuffer.getNextLine()) != null) {

        // Cleanup line
        consoleData = this.processInputLine(consoleData);
        if (consoleData == "") {
            continue;
        }

        // '#' is a script comment line, skip
        if (consoleData[0] == "#") {
            continue;
        }

        // valid line, return it
        return consoleData;
    }

    return consoleData;
}

//
// Process a given command
//
// callback(error, result)
//
CmdConsole.prototype.processCommand = function(commandTableArray, cmd, context, callback) {

    var commandTable = null;

    //
    // Lookup a command in an array of command tables.
    //
    // If a command is found processing stops.
    //
    // This allows earlier commandTable array entries to
    // override base commands that come later.
    //
    // cmd is a full command line such as "cmd options args, etc."
    //
    // This function only matches on the first entry in the table
    // in the order of the table. This avoids argument processing
    // in this or earlier routines and leaves it up to the command.
    //
    // This allows the following patterns:
    // "cmd arg1 arg2"
    // "cmd=arg1
    //  w,0:1
    // etc.
    //
    // Short named commands which can match as a prefix of a longer
    // command must come after the longer commands. This is to ensure
    // the longer command is not prematurely matched with the shorter
    // name, which may be unrelated.
    //
    // Example:
    //   "help",
    //   "h"
    //

    //
    // commandTableArray[] is an array of commandTables, each of which
    // is an array themselves.
    //
    // Note: See localCommandTable for an example of the fields of the objects
    // within a commandTable[].
    //
    // {
    //   "name": "quit",
    //   "function": exitFunction
    // };
    //

    for (var arrayIndex in commandTableArray) {

        commandTable = commandTableArray[arrayIndex];

	for (var entryIndex in commandTable) {

	    var entry = commandTable[entryIndex];

	    if (cmd.indexOf(entry.name) == 0) {
		entry.function(cmd, context, entry, callback);
		return;
	    }
	}
    }

    callback("unknown command: " + cmd, null);
}

function dumpBuffer(buffer) {

    //
    // If buffer is from an I/O stream it won't have charCodeAt()
    // which dumpHex requires.
    //
    buffer = buffer.toString();

    dh.dumpHex(buffer, 0, buffer.length, function(data) {
        console.log(data);
    });
}

//
// Process a data buffer line that came in on a stream
// into a string without any ending '\r' or '\n'.
//
// It's expected that this represents a whole command line
// with arguments.
//
CmdConsole.prototype.processInputLine = function(line) {

    //dumpBuffer(line);

    //
    // Data came in from an I/O stream and is not a normal string.
    // The below character comparisons won't work unless its
    // converted to a string.
    //
    var s = line.toString();

    if ((s.length > 0) && (s[s.length - 1] == '\n')) {
        s = s.substring(0, s.length - 1);
    }

    if ((s.length > 0) && (s[s.length - 1] == '\r')) {
        s = s.substring(0, s.length - 1);
    }

    return s;
}

CmdConsole.prototype.setTrace = function(value) {
    this.trace = value;
}

CmdConsole.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

CmdConsole.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

CmdConsole.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

// The module declares a constructor function for its API object
function CmdConsole(trace, traceerrorValue) {

    this.moduleName = "cmdconsole";

    this.defaultCommandTable = localCommandTable;

    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }
}

function createInstance(trace, traceerrorValue) {
    moduleInstance = new CmdConsole(trace, traceerrorValue);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};

