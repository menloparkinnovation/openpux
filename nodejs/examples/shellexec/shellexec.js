
//
// Shell execution wrapper
//
// 12/13/2014
//

// http://nodejs.org/api/child_process.html
var exec = require('child_process');

//
// Execute a program using the systems command shell.
//
// The callback returns the results from child_process.exec()
//
// callback(error, stdout, stderr)
//
//   if error, callback is invoked with the error.
//
//   callback is invoked when the child exits with strings representing
//   its stdout, stderr output during its execution.
//
// This function returns the child object created which may
// be executing till callback() is invoked.
//
function shellExec(cmdLine, callback) {

    var child;

    // child_process.exec() uses the OS shell
    child = exec.exec(cmdLine, callback);

    return child;
}

//
// Execute a series of commands sequentially.
//
function executeList() {

    // This allows variable arguments to be supported
    var commandlist;
    var stdoutFunc;
    var stderrFunc;
    var callback;
    var options = null;

    if (arguments.length == 4) {
        commandList = arguments[0];
        stdoutFunc = arguments[1];
        stderrFunc = arguments[2];
        callback = arguments[3];
    }
    else {
        commandList = arguments[0];
        options = arguments[1];
        stdoutFunc = arguments[2];
        stderrFunc = arguments[3];
        callback = arguments[4];
    }

    //
    // Since execution needs to be sequential and
    // and asynchronous it is done by unrolling
    // the loop into a series of ordered callbacks.
    //
    // There are actually two asynchronous operations involved:
    // 
    // 1: Getting the result of the process exec request which
    //    is asynchronous and indicates if a child is running or not.
    //
    // 2: If a child process was successfully created, waiting
    //    until it finishes processing and exists.
    //

    var child;
    var stopExecuting = false;
    var currentIndex = 0;
    var cmd = commandList[currentIndex].commandLine;

    console.log("currentIndex=" + currentIndex + " command=" + cmd);

    // executeResult function
    var executeResult = function(error, stdout, stderr) {

        //
        // This contains the result of an attempt to execute
        // a child process. It could have failed, or be running
        // when this is invoked.
        //

        if (error) {
            // child process create failed
            console.log("execute error, currentIndex=" + currentIndex);
            console.log("error=");
            console.log(error);
            callback(error);
            return;
        }

        // The child on('exit') handler will get our final results now
    }

    // http://nodejs.org/api/child_process.html#child_process_event_exit    
    var exitFunction = function(code, signal) {

        //
        // This is invoked on a successful child process creation
        // when it finally completes executing.
        //

        console.log("child " + currentIndex + " exited code=" + code + " signal=" + signal);

        currentIndex++;
        if (currentIndex >= commandList.length) {
            console.log("executeCommandList: done");
            callback(null);
            return;
        }

        // next command
        cmd = commandList[currentIndex].commandLine;

        console.log("currentIndex=" + currentIndex + " command=" + cmd);

        // Next command in sequence
        child = exec.exec(cmd, options, executeResult);
        child.on('exit', exitFunction);

        // reconnect the new childs streams with the callback functions
        child.stdout.on('data', stdoutFunc);
        child.stderr.on('data', stderrFunc);
    }

    // start it off
    child = exec.exec(cmd, options, executeResult);
    child.on('exit', exitFunction);

    //
    // Register the callers output handlers to allow real time
    // streaming or buffering of command execution if desired.
    //
    child.stdout.on('data', stdoutFunc);
    child.stderr.on('data', stderrFunc);
}

module.exports = {
  shellExec: shellExec,
  executeList: executeList
};
