
//
//   webserver.js
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

//
// This module is the basic web server for html, .js, .css content
// to clients.
//
// It redirects specific content type requests to their proper
// directory.
//
// It only serves file directed at the "root" of URI such
// as "/", "/file.html", "library.js", etc.
//
// It protects against paths such as /foo.html/../../../etc/passwd, etc.
//
// /foo.js   -> client/javascripts/foo.js
// /foo.css  -> client/css/foo.css
// /foo.html -> client/foo.html
//
// No extension assumes HTML.
//
// /foo      -> client/foo.html
//

// Default file served for '/' is /sensor application.
var defaultRootFile = "/sensor";

//
// This configures where different file types are served from.
//
var extensionToPathTable = [
  { extension: "js",     path: "client/javascripts/" },
  { extension: "css",    path: "client/css/" },
  { extension: "html",   path: "client/html/" }
];

var fs = require('fs');

function WebServer(config)
{
    this.moduleName = "WebServer";
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
// Returns true if the request is handled, false if not.
//
WebServer.prototype.processWebServer = function (req, res) {

  var url = req.url;

  if (req.method != "GET") {
      return false;
  }

  if (url[0] != '/') {
      return false;
  }

  if (url.length == 1) {

      // use configured default landing page
      url = defaultRootFile;

      // Fall through to standard processing
  }

  // trim root '/'
  var name = url.substring(1, url.length);
  
  //
  // Only simple paths such as /file are allowed, not subdirectories such
  // as /file/child.html
  //
  if (name.search(/\//g) != (-1)) {
      // Found another '/' in string
      return false;
  }

  // no ".." allowed in path
  if (name.search(/\.\./g) != (-1)) {
      // Found ".." in string
      return false;
  }

  var index = -1;

  if ((index = name.search(/\./)) == (-1)) {

      //
      // no extension, just try and serve the file with the .html extension
      // from the client dir
      //
      // sensor -> sensor.html
      //
      name = name + ".html";

      // set the index
      index = name.search(/\./);

      // Fall through to standard processing
  }

  // An extension is present, validate it.
  var ext = name.substring(index+1, name.length);
  var base = name.substring(0, index);

  if (ext.search(/\./) != (-1)) {
      // only one '.' allowed
      return false;
  }

  // Get its content directory
  for (var prop in extensionToPathTable) {
     if (ext == extensionToPathTable[prop].extension) {

          //
          // /sensor.html -> client/sensor.html
          // /library.js  -> client/javascripts/library.js
          // /library.css -> client/css/library.cs
          //
          var filePath = extensionToPathTable[prop].path + base + "." + ext;

          // Add a log entry that the log file was retrieved
          var remoteAddr = req.socket.remoteAddress;
          this.logger.info(remoteAddr + " retrieved file path=" + filePath);

          this.config.fileserver.serveFile(req, res, filePath);
          return true;
     }
  }

  return false;
}

module.exports = {
  WebServer: WebServer
};
