
//
//   logger.js
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
// Follows common logging pattherns for node.js applications.
//
// Interfaces to what ever logger is configured. Provides built
// in defaults.
//
// Arguments:
//
// config: configuration object.
//
// {
//     "InstanceName": "name_of_module_or_application",
//     "LogDirectory" : "./logs",
//     "ConsoleOutput": true
// }
//
// var loggerFactory = require('logger');
// var logger = new loggerFactory.Logger({InstanceName: "module", ConsoleOutput: true});
//
function Logger(config) {

    this.config = config;

    this.instanceName = "logger";

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
    // If logDirectory is set start the file based logger
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

Logger.prototype.shutdown = function(callback) {

    // Safe even if not open currently
    this.closeFileStream(callback);
}

Logger.prototype.log = function(level, message) {
    var entry = {level: level, name: this.instanceName, message: message};
    this.writeLog(entry);
}

Logger.prototype.info = function( message) {
    var entry = {level: "info", name: this.instanceName, message: message};
    this.writeLog(entry);
}

Logger.prototype.warn = function(message) {
    var entry = {level: "warn", name: this.instanceName, message: message};
    this.writeLog(entry);
}

Logger.prototype.error = function(message) {
    var entry = {level: "error", name: this.instanceName, message: message};
    this.writeLog(entry);
}

Logger.prototype.fatal = function(message) {
    var entry = {level: "fatal", name: this.instanceName, message: message};
    this.writeLog(entry);
}

//
// Set whether console output is done
//
Logger.prototype.setConsoleOutput = function(value) {
    this.consoleOutput = value;
}

//
// entry is a javascript object with log information
//
Logger.prototype.writeLog = function(entry) {

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
Logger.prototype.checkPointLog = function() {

    if (this.logFileStream != null) {
        // will automatically close the current one
        this.openFileStream();
    }
}

//
// Open a logger file stream
//
Logger.prototype.openFileStream = function() {

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

       var record =  { level: "info",
                       time: logTime,
                       module: this.instanceName,
                       message: "logger session started"
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
Logger.prototype.closeFileStream = function(callback) {

   if (this.logFileStream == null) {
       if (callback != null) {
           callback();
       }
   }

   var logTime = new Date().toISOString();

   var record =  { level: "info",
                   time: logTime,
                   module: this.instanceName,
                   message: "logger session ended"
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
  Server: Logger
};
