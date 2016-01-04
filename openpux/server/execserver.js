
//
//   execserver.js
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

var utility = require('./utility.js');

function ExecServer(config)
{
    this.moduleName = "ExecServer";
    this.config = config;

    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(this.config.Trace) != "undefined") {
        this.trace = this.config.Trace;
    }

    if (typeof(this.config.TraceError) != "undefined") {
        this.traceerrorValue = this.config.TraceError;
    }

    // Setup the inherited logger
    this.logger = this.config.logger;
}

//
// baseUri - URI base that leads to the exec request.
//
// basePath - Base of file system path to find executable file/script
//
// Caller is responsible for validating credentials before invoking
// this routine.
//
ExecServer.prototype.processExecRequest = function (req, res, baseUri, basePath) {

   var self = this;

   utility.receiveJSONObject(req, res, this.logger, function(error, data) {

      //
      // data contains the command and parameters
      //
      // data.command
      // data.args[]
      //

      if (error != null) {
          self.logger.info(remoteAddr + " failed to execute path=" + filePath);
          utility.sendErrorAsJSON(req, res, "400", error);
          return;
      }

      var path = req.url.substring(baseUri.length, req.url.length);
      if (path == null) {
          res.writeHead(404);
          res.end();
          return true;
      }

      var filePath = "./cgi-bin/" + path;

      console.log("uri=" + req.url);

      console.log("filePath=" + filePath);
      
      cmdLine = "./cgi-bin/";

      cmdLine += data.command;
    
      for (var entry in data.args) {
          cmdLine += " ";
          cmdLine += data.args[entry];
      }

      var result = {};
    
      // Add a log entry that exec was attempted
      var remoteAddr = req.socket.remoteAddress;
      self.logger.info(remoteAddr + " executed cmdline=" + cmdLine);

      // http://nodejs.org/api/child_process.html
      var exec = require('child_process').exec;

      // child_process.exec() uses the OS shell
      var child = exec(cmdLine, function(error, stdout, stderr) {

        result.status = 200;
        result.error = error;
        result.stdout = stdout;
        result.stderr = stderr;

        var jsonString = JSON.stringify(result);

        res.writeHead(200, {'Content-Type': 'application/json'});
        res.write(jsonString);
        res.end();
      });
  });
}

module.exports = {
  Server: ExecServer
};

