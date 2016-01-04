
//
//   accounting.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014,2015 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Snapshot License
//
//   This license is for a specific snapshot of a base work of
//   Menlo Park Innovation LLC on a non-exclusive basis with no warranty
//   or obligation for future updates. This work, any portion, or derivative
//   of it may be made available under other license terms by
//   Menlo Park Innovation LLC without notice or obligation to this license.
//
//   There is no warranty, statement of fitness, statement of
//   fitness for any purpose, and no statements as to infringements
//   on any patents.
//
//   Menlo Park Innovation has no obligation to offer support, updates,
//   future revisions and improvements, source code, source code downloads,
//   media, etc.
//
//   This specific snapshot is made available under the following license:
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

// http://nodejs.org/api/fs.html
var fs = require('fs');

//
// Accounting for openpux objects and operations.
//
// This allows limits to be applied to accounts, etc.
//
// The current implementation is file based, writing a JSON based
// log. This allows post processing utilities to assign charges/usage
// to accounts after the fact.
//
// The framework is hooked into the openpux system in real time, so
// alternate implementations could keep real time information in order
// to block aggressive users of resources.
//

//
// Arguments:
//
// config: configuration object.
//
// {
//     "InstanceName": "name_of_module_or_application",
//     "LogDirectory" : "./accounting_logs",
//     "ConsoleOutput": true
// }
//
// var accountingFactory = require('accounting');
// var accounting = new accountingFactory.Server({InstanceName: "app", ConsoleOutput: true});
//
function Accounting(config) {

    this.config = config;

    this.instanceName = "accounting";

    this.logDirectory = null;

    this.logFilePath = null;

    this.logFileFD = -1;

    this.logFileStream = null;

    this.consoleOutput = false;

    if (typeof(config.InstanceName) != "undefined") {
        this.instanceName = config.InstanceName;
    }

    if (typeof(config.ConsoleOutput) != "undefined") {
        this.consoleOutput = config.ConsoleOutput;
    }

    if (typeof(config.LogDirectory) != "undefined") {
        this.logDirectory = this.config.LogDirectory;
    }

    //
    // If logDirectory is set start the file based accounting logger
    //
    if (this.logDirectory != null) {
        this.openFileStream();
    }

    // Setup an exit handler
    var self = this;

    // This is the standard ^C handler
    process.once('SIGINT', function () {
      self.shutdown(function () {
        process.kill(process.pid, 'SIGINT');
      });
    });

    // SIGUSR2 is used by nodemon for recycle
    process.once('SIGUSR2', function () {
      self.shutdown(function () {
        process.kill(process.pid, 'SIGUSR2');
      });
    });
}

Accounting.prototype.shutdown = function(callback) {

    // Safe even if not open currently
    this.closeFileStream(callback);
}

//
// Log an account event
//
// args = {
//     account:     "account_to_charge",
//     object:      "subject_of_charge",
//     operation:   "operation_of_charge",
//     data_stored: "data_stored_in_bytes",
//     data_io:     "data transferred_in_bytes",
//     network:     "network_transfer_in_bytes",
//     cpu:         "cpu_used_in_milliseconds"
// };
//
Accounting.prototype.log = function(args) {
    var logTime = new Date().toISOString();

    var entry = {time: logTime, module: this.instanceName, message: args};
    this.writeLog(entry);
}

//
// These workers are used to place important operations
// account policy in one place.
//
// charge_to_object: represents the account/resource who is charged
//
// object_of_charge: object whose resources are the subject of the charge
//
// This is a basic addObject charge
//
Accounting.prototype.createObjectBasicCharge = function(charge_to_object, object_of_charge) {
    var args = {};    

    args.account = charge_to_object;
    args.object = object_of_charge;

    args.operation = "create";
    args.data_stored = 100;
    args.data_io = 100;
    args.network = 100;
    args.cpu = 10;      // 10 ms

    this.log(args);
}

//
// Set whether console output is done
//
Accounting.prototype.setConsoleOutput = function(value) {
    this.consoleOutput = value;
}

//
// entry is a javascript object with log information
//
Accounting.prototype.writeLog = function(entry) {

    var buffer = JSON.stringify(entry);

    // Write to log file
    if (this.logFileStream != null) {

        //
        // This is async on the writeStream so we don't register
        // a callback.
        //
        this.logFileStream.write(buffer + ",\n");
    }

    // Optional console output
    if (this.consoleOutput) {
        console.log(buffer);
    }
}

//
// A checkpoint closes the current file based log (if open)
// and starts a new one.
//
// Done by an application on a periodic basis such as hourly,
// daily, weekly, etc.
//
Accounting.prototype.checkPointLog = function() {

    if (this.logFileStream != null) {
        // will automatically close the current one
        this.openFileStream();
    }
}

//
// Open a logger file stream
//
Accounting.prototype.openFileStream = function() {

   //
   // If a log file stream is currently open start
   // closing it. Note that this is async as we will
   // start the new stream in parallel.
   //
   if (this.logFileStream != null) {
       this.closeFileStream();
   }

   try {

       var logTime = new Date().toISOString();
       var logPathTime = logTime;
  
       // If its Win32/Windows we must eliminate the ":" characters.
       if (process.platform == "win32") {
           logPathTime = logPathTime.replace(/:/g, "_");
       }

       var s = this.logDirectory + "/" + this.instanceName;
           s += "_" + logPathTime + ".log";

       var logFilePath = s;

       //
       // Keep a captured copy of the FD since self.logFileFD is
       // invalidated by closeFileStream() asynchronously.
       //
       var captured_logFileFD = fs.openSync(logFilePath, "a", "0666");

       var logFileStream =
           fs.createWriteStream(null, {flags: 'w', fd: captured_logFileFD, mode: "0o666"});

       logFileStream.on('end', function() {
           fs.closeSync(captured_logFileFD);
       });

       logFileStream.on('error', function(error) {
           console.error("error on log stream: error=" + error);
       });

       var record =  { time: logTime,
                       module: this.instanceName,
                       message: "accounting session started"
                     };

       var start = "[\n";

       var buffer = start + JSON.stringify(record) + ",\n";

       logFileStream.write(buffer);

       if (this.consoleOutput) {
            console.log(buffer);
       }

       //
       // Now that an exception has not occured publish
       // the logging variables.
       //
       this.logFileStream = logFileStream;
       this.logFileFD = captured_logFileFD;
       this.logFilePath = logFilePath;

   } catch (e) {

       // Error setting up stream, report error and return.
       console.error("exception setting up file stream " + logFilePath);
       console.error("e=" + e.toString());

       //
       // we rethrow since operating a server without logs when
       // requested is dangerous.
       //
       throw e;
   }
}

//
// Close the logger file stream
//
Accounting.prototype.closeFileStream = function(callback) {

   if (this.logFileStream == null) {
       if (callback != null) {
           callback();
       }
   }

   var logTime = new Date().toISOString();

   var record =  { time: logTime,
                   module: this.instanceName,
                   message: "accounting session ended"
                 };

   var stop = "]\n";

   var buffer = JSON.stringify(record) + "\n" + stop;

   //
   // Do this first in case the writeStream end fires before console finishes
   //
   if (this.consoleOutput) {
        console.log(buffer);
   }

   var captured_logFileStream = this.logFileStream;

   //
   // We null out the previous values now as they have been
   // captured by the 'end" event handler as lamba variables.
   //
   // This is to allow an immediate re-open of the log on a new
   // file stream. This is done perodically by applications
   // when their log interval (day, hour, week, etc.) occurs.
   //
   this.logFileStream = null;
   this.logFileFD = -1;

   //
   // The will be closed by the 'end' event on the logFileStream when
   // all of the buffers have been written out.
   //
   captured_logFileStream.end(buffer, callback);
}

module.exports = {
  Server: Accounting
};
