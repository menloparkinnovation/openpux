
//
// TODO:
// 10/29/2015
//
// - remote shutdown rpc. Allow use of forever command to perform restart
//   - gracefull, log it, flush log, reply to client, initiate on next turn.
//
// - automatically cycle logs on a schedule. Daily, etc.
//
// - mechanism for uploading/updating applications to apps directory.
//   - best to not require a restart
//   - can I avoid a messy multiple part form upload?
//     - Applications are only text files
//       - .js, .css. html, .json
//       - maybe a simple POST to the admin interface targeting the apps/.. directory
//       - client side uploader can do multiple POST's
//       - admin way to shutdown, restart a specific application.
//         - for example stop it when its being updated.
//           - how to determine no current requests?
//             - without coding manual reference/activity counts?
//
//  - admin page
//
//  - command line tools around client.js, resterclient.js, etc.
//
//  - for updating server, can I do A==B
//
//    - upload new files to updates new dir
//    - then send command to switch over
//
//    - make sure data bases, config are still correct, in common
//      - config changes should be part of A==B
//
//    - my own smart launcher run from forever can make openpux
//      deployment decisions as far as which directory to launch on,
//      roll back on new server failure (last known good), etc.
//
//    - really start to think about and nail consumer remote RaspberryPi's
//      and not just devops on a cloud service using Docker.
//
//      - consumer case must be 100% automated
//        - or it does not scale (cost wise)
//
//    - think about dumper/backup for customer data
//
//     - can I rely on datastore specific ones such as tools for
//       AWS SimpleDB, Mongo, etc.?
//
//    - Need a customer data disclosure, license that there are
//      no guarantees, etc.
//
//      - Beta, as in gmail for years...
//
//    - Make it clear the service is for lab/experimentation/development/
//      educational use only.
//
//    - People can take the code and offer their own service, and its up
//      to them to decide on these policies. There is no warranty in that
//      they agree to use the code as is and are fully responsible for
//      their service.
//
//    - Makes no guarantees on privacy.
//
//    - Makes no guarantees on stability, whether it would be lost, corrupt
//
//    - Makes no guarantees on errors that result, accidents, spoiled cigars,
//      water bills, etc.
//
//   adminapp.js
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

var appConfig = {
    name:     "admin",
    url_root: "/administration/",

    logs_url: "/administration/logs",
    exec_url: "/administration/exec",

    exec_binpath: "./cgi-bin/"
};

function App(config)
{
    this.appconfig = appConfig;
    this.config = config;

    this.moduleName = this.appconfig.name;

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

    this.utility = this.config.utility;
}

//
// Process an App Request
//
// This is invoked by the generic application handler to
// query for, and handle an application path URL.
//
// Parameters:
//
// req - http request
//
// res - http response
//
// app - Applications config entry in config/serverconfig.json
//
// app_url - Specific application URL that was hit that caused the invoke
//
// appserver - instance to appserver.js for utility functions to aid
//       in generic request handling.
//
// Returns:
//
//  true - Path was handled, even if there was an error.
//
//  false - Path was not handled, continue looking for another handler.
//
App.prototype.processAppRequest = function (req, res, app, app_url, appserver) {

  if (req.method != "GET") {
      this.utility.sendErrorAsJSON(req, res, 400, "invalid admin request");
      return true;
  }

  //
  // TODO: Validate credentials!
  //

  if (req.url.search(this.appconfig.logs_url) == 0) {
      return this.processLogsRequest(req, res);
  }
  else if (req.url.search(this.appconfig.exec_url) == 0) {

      // All execute requests go to ./cgi-bin
      return this.config.execserver.processExecRequest(
          req, res, this.appconfig.exec_url, this.appconfig.exec_binpath);
  }
  else {
      this.utility.sendErrorAsJSON(req, res, 400, "invalid admin request");
      return true;
  }
}

//
// Retrieve administration logs and their information.
//
// curl http://localhost:8080/administration/logs
//
// curl http://localhost:8080/administration/logs/openpux_2015-10-13T15:35:26.156Z.log
//
// Returns:
//
//  true = processed request and sent response to the client, error or not.
//
//  false = did not recognize request, continue to other handlers.
//
App.prototype.processLogsRequest = function (req, res) {

  // No log files configured
  if (this.config.LogFileDirectory == null) {
      // 404 Not Found
      res.writeHead(404);
      res.end();
      return true;
  }

  var filePath = null;

  var path = req.url.substring(this.appconfig.logs_url.length, req.url.length);
  if (path == null) {
      res.writeHead(404);
      res.end();
      return true;
  }

  if ((path == "") || (path == "/")) {

      // request for logs directory

      // Skip over '/'
      filePath = path.substring(1, path.length);

      filePath = this.config.LogFileDirectory + filePath;

      // logsUri is supplied to allow href= links generated in response
      this.config.fileserver.serveDirectoryFile(req, res, filePath, this.appconfig.logs_url);
      return true;
  }

  filePath = this.config.LogFileDirectory + path;

  // Add a log entry that the log file was retrieved
  var remoteAddr = req.socket.remoteAddress;
  this.logger.info(remoteAddr + " retrieved log file path=" + filePath);

  this.config.fileserver.serveFile(req, res, filePath);

  return true;
}

module.exports = {
  App: App
};
