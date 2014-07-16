
//
// Node Template 05/26/2014
//
// Statements are executed right off as in any script
//
// Run as "node hello.js"
//
// "dir | node hello.js -" to test input pipe function
//
// rem runit.cmd
// node hello.js arg2 arg3 arg4
//

// On a Unix machine this would allow automatic execution after chmod a+x hello.js
// #!/usr/bin/env node

console.log("Hello World");

// The process object allows access to arguments
console.log("process.argv");
console.log(process.argv);

//
// The arguments passed to node.exe are:
//
// [0] executable_name
// [1] node_script_path.js
// [2] first user arg
// [3] second user arg
//    ...
//
// Remove argv[0], argv[1] to get to the base of the user supplied arguments
var args = process.argv.slice(2);

// This will dump the args in JSON
console.log("process.argv.slice(2)");
console.log(args);

//
// Process pipe data if argument is '-'
//
if ((args != null) &&
    (args[0] == '-')) {

    console.log("Receiving Pipe Data");

    process.stdin.resume();
    process.stdin.setEncoding('utf8');

    //
    // Set exit handler to exit the process when done
    // Can't do this as a fall through, since script execution in
    // node.js sets up an event handler to be processed later.
    //
    // Exiting now would kill the process before the pipe stream
    // receive event handler gets a chance to run, or is still
    // running.
    //

    process.stdin.on('end', function(data) {
        console.log("Exiting");
        process.exit(0);
    });

    process.stdin.on('data', function(data) {
        console.log("Pipe Data event");
        process.stdout.write(data);
    });
}

//
// Do not exit the script here if any async event
// handlers were setup above. Otherwise the program
// will terminate before they get a chance to run, or
// finish running.
//
// Doing a process.exit(0); here will stop before any
// processing occurs.
//
// Node.js will continue to run while handlers are outstanding.
//
// It appears to exit when everything is done.
//