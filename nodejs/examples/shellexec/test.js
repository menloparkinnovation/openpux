//
// Shell exec testing
//
// 12/13/2014
//

var shellexec = require('./shellexec');

var g_verbose = false;

//
// These are run sequentially
//

//
// This set generates a lot of output to allow testing the maxBuffer
// option.
//
// This demonstates conditions when maxBuffer in options
// is required.
//
var testCommandListLargeOutput = 
[
    {"commandLine": "find / -print"},
    {"commandLine": "find / -print"},
    {"commandLine": "find / -print"},
    {"commandLine": "find / -print"}
];

var testCommandListSmallOutput = 
[
    {"commandLine": "ls"},
    {"commandLine": "ls -l"},
    {"commandLine": "ps"},
    {"commandLine": "ps -edlf"}
];

var testCommandList = testCommandListSmallOutput;

//
// This will result in "Error: stdout maxBuffer exceeded." unless
// maxBuffer in options is made very large.
//
//var testCommandList = testCommandListLargeOutput;

function testExecuteList(commandList) {

    //
    // We provide functions to get real time notification
    // of the combined stdout, stderr streams from the series
    // of child processes we execute sequentially.
    //
    var stdoutFunc = function(data) {
        console.log(data);
    };

    var stderrFunc = function(data) {
        console.error(data);
    };

    //
    // Set a large buffer for long output commands
    //
    // shellExec() uses .exec() which buffers stdout + stderr
    // until the child process exits and then notifies us
    // by invoking the callback. options allows us to set this
    // buffer size for commands with a lot of output before
    // exiting.
    //
    // This problem can be avoided for very long running/large output
    // commands by using .spawn() (shellSpawn()) which consumes the
    // stdout and stderr streams in real time.
    //
    var options = {
        maxBuffer:  1024 * 1024
    };

    console.log("options=");
    console.log(options);

    shellexec.executeList(commandList, options, stdoutFunc, stderrFunc, function(result) {
        console.log("executeList result=" + result);
    });
}

function main(ac, arv) {

    var cmdLine = "ls -l";

    console.log("shellExec: running commandline=" + cmdLine);

    shellexec.shellExec(cmdLine, function(error, stdout, stderr) {

	if (error !== null) {
	    console.error('exec error: ' + error);
	}

	console.log(stdout);
	console.error(stderr);

        // Now execute a series of commands sequentially
        enumerateCommandList(testCommandList)

        testExecuteList(testCommandList);
    });
}

function enumerateCommandList(commandList) {
    var index;

    for (index = 0; index < 4; index++) {
        console.log("index=" + index + " commandLine=" + commandList[index].commandLine);
    }
}

function dumpOutput(stdout, stderror) {
    console.log("stdout=");
    console.log(stdout);

    if (typeof(stderr) != "undefined") {
        console.log("stderr=");
        console.log(stderr);
    }
}

var usage = function(bad_arg) {

    if (typeof bad_arg != "undefined") {
        console.error("Bad argument " + bad_arg);
    }

    console.error(g_usage);

    process.exit(1);
}

function tracelog(config, message) {
    if (g_verbose) {
        // console.log connects to stdout
        console.log(message);
    }
}

function errlog(message) {
    // console.error connects to stderr
    console.error(message);
}

//
// Remove argv[0] to get to the base of the standard arguments.
// The first argument will now be the script name.
//
var args = process.argv.slice(1);

// Invoke main
main(args.length, args);
