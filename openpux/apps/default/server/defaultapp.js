
//
//   defaultapp.js
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
    name:     "default",
    url_root: "/"
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

  var url = req.url;

  //
  // This does not forward Api, rpc, or other types of requests, just basic data file content.
  //
  if (req.method != "GET") {
      return false;
  }

  if (url[0] != '/') {
      return false;
  }

  if (url.length == 1) {

      // use configured default landing page
      url = app.default_url;

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

      // Fall through to standard processing
  }

  //
  // Forward the request to the configured application
  //
  var forward_app = appserver.findAppByName(app.app);
  if (forward_app == null) {
      return false;
  }

  // /appname is the root of the url
  return appserver.processUrlEntry(req, res, forward_app, name);
}

module.exports = {
  App: App
};
