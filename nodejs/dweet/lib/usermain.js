
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
// User Main for Dweet.
//
// 01/25/2015
//

function UserMain(createArgs) {
    this.trace = createArgs.trace;
    this.traceerrorValue = createArgs.traceerror;
    this.log = createArgs.log;
    this.error = createArgs.error;
}

//
// This sets up a dweet interactive console (repl) from the
// arguments supplied by argsTable[]
//
// // Setup by dweet.js, function Config(...)
// config.verbose
// config.name
// config.argsTable
// config.userMainInstance
// config.template
// config.log
// config.error
// config.stdin
// config.stdout
// config.stderr
//
// args - command line args to command processing loop
//
// bin/dweet.sh
//   #!/usr/bin/env node
//   require('../lib/dweet').launch();  // loads dweet.js and executes launch()
//     module.exports.launch -> LaunchCommandLine()
//       LaunchCommandLine()
//         RunCommandLineMain()// template.js, programEntry
//           template.js, this.templateMain(config, args.length, args, callback);
//             template.js, captured_this.processArgs(config, argc, argv, function(options, args) {
//               usermain.js, config.userMainInstance.userMain(config, args, callback);
//
// callback(error, result)
//
UserMain.prototype.userMain = function(config, args, callback) {

    var argsTable = config.argsTable;

    // /dev/cu.usbmodem1421 for Arduino Uno on MacBookAir
    //var portName = "/dev/cu.usbmodem1421";
    var portName = null;
    var script = null;
    var apphandler = null;
    var configFile = null;
    var openConsole = false;
    var stopOnError = true;

    // Default tracing
    var trace = false;
    var traceerror = false;

    // setup tracing
    config.verbose = argsTable["verbose"].value;

    this.tracelog(config, "userMain: entered");

    //
    // argsTable values are set by the command line arguments
    // processor.
    //
    // It's used when launched from the command line, or a programmatic
    // command line script launch.
    //
    if (argsTable["verbose"].value != false) {
        this.log("verbose specified");
        trace = true;
        traceerror = true;
    }

    if (argsTable["tracelog"].value != false) {
        trace = true;
    }

    if (argsTable["traceerror"].value != false) {
        traceerror = true;
    }

    if (argsTable["port"].present != false) {
        portName = argsTable["port"].value;
        portName = portName.substring(1, portName.length);
    }

    if (argsTable["script"].present != false) {
        script = argsTable["script"].value;
        script = script.substring(1, script.length);
        this.log("script=" + script);
    }

    if (argsTable["apphandler"].present != false) {
        apphandler = argsTable["apphandler"].value;
        apphandler = apphandler.substring(1, apphandler.length);
        this.log("apphandler=" + apphandler);
    }

    // lighthouse.json, etc.
    if (argsTable["config"].present != false) {
        configFile = argsTable["config"].value;
        configFile = configFile.substring(1, configFile.length);
        this.log("config=" + configFile);
    }

    if (argsTable["dontstoponerror"].value != false) {
        stopOnError = false;
    }

    if (argsTable["consoleoption"].value != false) {
        openConsole = true;
    }

    //
    // dweetArgs contains all parameters required
    //
    var dweetArgs = new Object();

    dweetArgs.trace = trace;
    dweetArgs.traceerror = traceerror;

    // Streams
    dweetArgs.log = this.log;
    dweetArgs.error = this.error;
    dweetArgs.stdin = config.stdin;
    dweetArgs.stdout = config.stdout;
    dweetArgs.stderr = config.stderr;

    // Default Prefix
    dweetArgs.prefix = "$PDWT"; 

    dweetArgs.script = script;
    dweetArgs.stopOnError = stopOnError;
    dweetArgs.openConsole = openConsole;

    //
    // User application handlers
    //
    // -apphandler=lighthouseapp.js
    //
    dweetArgs.apphandler = apphandler;

    //
    // Load config file if specified
    //
    // -config=config.json
    //
    if (configFile != null) {
        dweetArgs.config = loadJsonFile(configFile);
        if (dweetArgs.config == null) {
            callback("error loading config file " + configFile, null);
            return;
        }
    }
    else {
        dweetArgs.config = null;
    }

    if (args.length == 0) {
        dweetArgs.deviceName = portName;
    }
    else {
        dweetArgs.deviceName = args[0];
    }

    //
    // Create a Dweet Console instance from the factory
    //
    this.dweetConsole = 
        require('./dweetconsole.js').createInstance(dweetArgs.trace, dweetArgs.traceerror);

    //
    // Launch the command line console instance of Dweet
    //
    // dweetconsole.js, ConsoleMain
    //
    this.dweetConsole.ConsoleMain(dweetArgs, function(error) {
        callback(error, null);
    });
}

//
// usage() may be customized as required, but by default will
// generate a usage message from the configured options.
//
UserMain.prototype.usage = function(config, message) {

    if (typeof message != "undefined") {
        this.error(message);
    }

    //
    // Call the template helper which generates a default
    // usage display from the configured arguments table.
    //
    var usageString = config.template.generateUsage(config);

    this.error(usageString);
}

UserMain.prototype.getUserMain = function() {
    return this.userMain;
}

UserMain.prototype.getUsage = function() {
    return this.usage;
}

UserMain.prototype.tracelog = function(config, message) {
    if (config.verbose) {
        this.log(message);
    }
}

UserMain.prototype.errlog = function(message) {
    this.error(message);
}

function createInstance(createArgs) {
    var um = new UserMain(createArgs);
    return um;
}

module.exports = {
  createInstance: createInstance
};

var fs = require('fs');

function loadJsonFile(fileName) {

    try {
        var jsonText = fs.readFileSync(fileName);
        var obj = JSON.parse(jsonText);
        return obj;
    }
    catch(e) {
        return null;
    }
}
