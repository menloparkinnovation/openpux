
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
// dweet.js
//
// Rewrote 06/24/2015 to be main entry point for both command
// line and library/module usage.
//

//
// bin/dweet.sh
//   #!/usr/bin/env node
//   require('../lib/dweet').launch();  // loads dweet.js and executes launch()
//     dweet.js, module.exports.launch -> LaunchCommandLine()
//       dweet.js, LaunchCommandLine()
//         dweet.js, createInstance(console.log, console.error)
//           dweet.js, Dweet(logger, errorlogger)
//
function Dweet(logger, errorlogger) {
    this.logger = logger;
    this.errorlogger = errorlogger;
}

//
// Run main using the command line processing system.
//
// This allows use of the command line version of Dweet from either
// an interactive OS command line/shell, or from a service that loads
// dweet as an npm module/library.
//
// bin/dweet.sh
//   #!/usr/bin/env node
//   require('../lib/dweet').launch();  // loads dweet.js and executes launch()
//     module.exports.launch -> LaunchCommandLine()
//       LaunchCommandLine()
//         RunCommandLineMain()
//
Dweet.prototype.RunCommandLineMain = function(args) {

    var createArgs = new Object();

    //
    // Note:
    //
    // Dweet may be loaded as a module within another process.
    // As a result it does not directly reference the process object
    // and functions such as process.exit(), process.stdout, etc.
    //
    // These functions should only be accessed here within this
    // method whose purpose is to allow execution of dweet from
    // an interactive operating system shell/command line with
    // its own dedicated process.
    //
    var stdin = process.stdin;
    var stdout = process.stdout;
    var stderr = process.stderr;

    // loggers were setup by createInstance()

    //
    // These are manually set to true to debug the command line
    // startup infrastructure, vs. tracing enabled by options processed
    // on the command line for the application itself.
    //
    createArgs.trace = false;
    createArgs.traceerror = false;

    // We pass references to the active logger
    createArgs.log = this.logger;

    // console.error connects to stderr
    createArgs.error = this.errorlogger;

    // The template module handles program argument handling, etc.
    this.template = require('./template.js').createInstance(createArgs);

    // The arguments table is JSON configured as a separate module
    this.argsTable = require('./argstable.js').createInstance(createArgs);

    // User main is in a separate module
    this.userMainInstance = require('./usermain.js').createInstance(createArgs);

    // Create Config object instance using the process streams
    this.config = new Config(this.argsTable, this.template, this.userMainInstance,
                              this.logger, this.errorlogger,
                              stdin, stdout, stderr);

    //
    // Execute template.js, programEntry() with the configuration object
    //
    // template.js, programEntry
    //   template.js, this.templateMain(config, args.length, args, callback);
    //     template.js, captured_this.processArgs(config, argc, argv, function(options, args) {
    //       usermain.js, config.userMainInstance.userMain(config, args, callback);
    //
    this.template.programEntry(this.config, args, function(error, result) {
        if (error != null) {
            stderr.write("exit status=" + error + "\n");
            process.exit(1);
        }
        else {
            process.exit(0);
        }
    });
}

//
// This supports the direct launching from a command line startup script
//
// This is exposed as modules.exports.launch
//
// See argstable.js for program name and valid arguments.
//
// Loading, startup order:
//
// bin/dweet.sh
//   #!/usr/bin/env node
//   require('../lib/dweet').launch();  // loads dweet.js and executes launch()
//     module.exports.launch -> LaunchCommandLine()
//
function LaunchCommandLine() {

    //
    // Remove argv[0] to get to the base of the standard arguments.
    // The first argument will now be the script name.
    //
    var args = process.argv.slice(1);

    //
    // Create Main object instance using console.out, console.error for the loggers
    //
    // Main object is the entry
    //
    var dweet = createInstance(console.log, console.error);

    // Run it
    dweet.RunCommandLineMain(args);
}

//
// Constructor function for configuration options
//
function Config(argsTable, templ, userMain, logger, errorlogger,
                stdin, stdout, stderr)
{

    // data
    this.verbose = false;
    this.name = argsTable.getProgramName();
    this.argsTable = argsTable.getArgsTable();

    // methods
    this.userMainInstance = userMain;

    // references
    this.template = templ;

    // Streams, loggers
    this.log = logger;
    this.error = errorlogger;
    this.stdin = stdin;
    this.stdout = stdout;
    this.stderr = stderr;
}

//
// Create Instance of Dweet.
//
// dweet.js, LaunchCommandLine()
//
function createInstance(logger, errorlogger) {
    var dweet = new Dweet(logger, errorlogger);
    return dweet;
}

module.exports = {

  //
  // Interface for library/npm loading.
  // Also used by command line launcher.
  //
  createInstance: createInstance,

  // Interface to direct launch from the command line
  launch: LaunchCommandLine
};
