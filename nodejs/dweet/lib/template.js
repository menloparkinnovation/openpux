
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
// Node.js template command line scripts/applications.
//
// 06/05/2015
//

Template.prototype.tracelog = function(message) {
    if (this.trace) {
        this.log(message);
    }
}

Template.prototype.tracerror = function(message) {
    if (this.traceerrorValue) {
        this.error(message);
    }
}

function Template(createArgs) {
    this.trace = createArgs.trace;
    this.traceerrorValue = createArgs.traceerror;
    this.log = createArgs.log;
    this.error = createArgs.error;
}

function createInstance(createArgs) {
    template = new Template(createArgs);
    return template;
}

module.exports = {
  createInstance: createInstance
};

//
// Invoked from dweet.js
//
// callback(error, result)
//
Template.prototype.programEntry = function(config, args, callback) {

    // Invoke template main
    this.templateMain(config, args.length, args, callback);
}

//
// Generate usage() summary from the configured
// options table.
//
Template.prototype.generateUsage = function(config) {

    var argsTable = config.argsTable;

    var usageString = "Usage: " + config.name;

    // required options
    for( var entry in argsTable) {
        t = argsTable[entry];
        if (t.option && t.required) {
            if (typeof t.altname != "undefined") {
                usageString += " " + t.altname;
            }
            usageString += " " + t.name;
        }
    }

    // optional options
    for( var entry in argsTable) {
        t = argsTable[entry];
        if (t.option && !t.required) {
            if (typeof t.altname != "undefined") {
                usageString += " [" + t.altname + "]";
            }
            usageString += " [" + t.name + "]";
        }
    }

    // required arguments
    for( var entry in argsTable) {
        t = argsTable[entry];
        if (!t.option && t.required) {
            if (typeof t.altname != "undefined") {
                usageString += " " + t.altname;
            }
            usageString += " " + t.name;
        }
    }

    // optional arguments
    for( var entry in argsTable) {
        t = argsTable[entry];
        if (!t.option && !t.required) {
            if (typeof t.altname != "undefined") {
                usageString += " [" + t.altname + "]";
            }
            usageString += " [" + t.name + "]";
        }
    }

    return usageString;
}

//
// templateMain() that handles processing on behalf of the caller.
//
// Invoked from:
//
// bin/dweet.sh
//   #!/usr/bin/env node
//   require('../lib/dweet.js').launch();  // loads dweet.js and executes launch()
//     dweet.js, module.exports.launch -> LaunchCommandLine()
//       dweet.js, LaunchCommandLine()
//         dweet.js, RunCommandLineMain()
//           template.js, programEntry
//             template.js, templateMain
//
// callback(error, result)
//
Template.prototype.templateMain = function (config, argc, argv, callback)
{
    this.tracelog("argc=");
    this.tracelog(argc);

    // This will dump the args in JSON
    this.tracelog("argv=");
    this.tracelog(argv);

    var captured_this = this;

    captured_this.processArgs(config, argc, argv, function(options, args) {
   
        //
        // Common default options are processed here.
        //
        // userMain can process more specific options
        //

        if (config.argsTable["help"].value != false) {
            config.userMainInstance.usage(config);
        }

        if (config.argsTable["verbose"].value != false) {
            config.verbose = true;
        }

        //
        // Ensure all required options are specified
        //
        for (var entry in config.argsTable) {
            t = config.argsTable[entry];

            if (t.required && !t.present) {

                captured_this.tracelog("entry=");
                captured_this.tracelog(t);
                var err = null;

                if (t.option) {
                    err = "required option " + t.name + " not specified";
                    config.userMainInstance.usage(config, err);
                }
                else {
                    err = "required argument " + t.name + " not specified";
                    config.userMainInstance.usage(config, err);
                }

                callback(err, null);
                return;
            }
        }

        // usermain.js
        config.userMainInstance.userMain(config, args, callback);
    });
}

//
// Process arguments and options from the argc, argv command line.
//
// argsTable provides the configuration for valid options, and
// is updated with any options found.
//
// argv contains the full argument list from the command line
// which includes the script name.
//
// argc is the argument count from the command line.
//
// callback is the function to call with the processed
// arguments and options.
//
// Options to the script start with '-' and must preceede the
// command. These are placed in the options array provided
// to the callback.
//
// All arguments after the first non-'-' entry are treated
// as arguments/options to the command and are placed as is
// into the args array in the callback.
//
// Example:
//
// script -option1 -option2 cmd -cmdOpt1 -cmdOpt2 cmdArg1 cmdArg2 ...
//
// -option1, -option2 belong to this script and are interpreted by
// it from the options[] array.
//
// cmd, -cmdOpt1, -cmdOpt2, cmdArg1, cmdArg2 are considered arguments
// that represent the cmd to run and its options. These are provided
// in the args[] array in the callback.
//

Template.prototype.processArgs = function(config, argc, argv, callback) {

    var args = new Array();

    var args_index = 0;
    var processing_options = true;

    var argsTable = config.argsTable;

    // We skip the first argument which is the name of this script
    for (var index = 1; index < argc; index++) {

        // First process options till we get an entry without a '-'
        if (processing_options) {

            var entry = this.processSingleOption(config, argv[index]);
            if (entry == null) {

                //
                // If we are still processing options and it does not
                // have an entry, its an error.
                //
    	        if (argv[index][0] == '-') {
                    config.userMainInstance.usage(config, "unknown option " + argv[index]);
                }
            }

    	    if (argv[index][0] != '-') {

                // entry does not start with '-' done with options processing
                this.tracelog("processArgs: switching to argument processing");
                processing_options = false;
    
                // fall through
            }
        }

        //
        // This can't be an else since the above falls through when
        // it encounters the first non-'-' entry
        //
        if (!processing_options) {
            // The rest of the entries are considered arguments
            this.tracelog("adding arg to list arg=" + argv[index]);
            args[args_index++] = argv[index];
        }
    }

    callback(config, args);
}

//
// valueoption == false:
//
// -name
// -n
//
// valueoption == true:
//
//  -name=value
//  -n=value
//
Template.prototype.processSingleOption = function(config, arg) {

    var argsTable = config.argsTable;

    //
    // Look through the args table for the option
    //
    for (var entry in argsTable) {

        if ((argsTable[entry].valueoption != false) &&
            ((arg.indexOf(argsTable[entry].name) == 0) ||
             (arg.indexOf(argsTable[entry].altname) == 0))) {

            var length = 0;
            if (arg.indexOf(argsTable[entry].name) == 0) {
                length = argsTable[entry].name.length;
            }
            else {
                length = argsTable[entry].altname.length;
            }

            argsTable[entry].value = arg.substring(length);

            if (argsTable[entry].value.length == 0) {
                config.userMainInstance.usage(config, "value entry " + arg + " has no value setting");
                return null;
            }

            // -name=value
            argsTable[entry].present = true;

            this.tracelog("processSingleOption: found value option entry=" + entry);
            this.tracelog(argsTable[entry]);

            return entry;
        }
        else if ((argsTable[entry].valueoption == false) &&
            ((arg == argsTable[entry].name) || (arg == argsTable[entry].altname))) {
            // -name
            argsTable[entry].present = true;
            argsTable[entry].value = true;
            this.tracelog("processSingleOption: found boolean option entry=" + entry);
            this.tracelog(argsTable[entry]);
            return entry;
        }
        else {
	    this.tracelog("processSingleOption: no match for arg=" + arg + " entry=" + entry);
            this.tracelog(argsTable[entry]);
        }
    }

    // option not found in the table
    return null;
};
