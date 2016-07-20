
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

//
// Currently the defaultapp serves standard requests such as
// to /openpuxclient.js, etc. and redirects them to
// openpux/client/javascripts/openpuxclient.js as per the application
// handling for openpux app.
//
// This is based on the entries in the defaultapp's configuraiton.json file.
//

function App(config, appConfiguration)
{
    this.config = config;
    this.appconfig = appConfiguration;

    console.log(this.appconfig);

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
// app - Applications configuration entry
//
// app_url - Specific application URL that was hit that caused the invoke
//
// appserver - reference to appserver.js instance for utility functions to aid
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
    var forward_app = null;
    var name = null;

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

        //
        // If the URL is exactly "/" we load the default home app and page
        //

        //console.log("**** defaultapp home page request");

        if ((typeof(app.default_home_page) == "undefined") ||
            (app.default_home_page == null)) {
            return false;
        }

        if ((typeof(app.default_home_app) == "undefined") ||
            (app.default_home_app == null)) {
            return false;
        }

        // use configured default landing page
        name = app.default_home_page;

        forward_app = appserver.findAppByName(app.default_home_app);
        if (forward_app == null) {
            return false;
        }

        console.log("**** defaultapp name=" + name);
        console.log("**** defaultapp app.name=" + forward_app.name);

        //
        // Forward the request to the configured application
        //
        // NOTE: Problem here is its not rooted properly from the browser
        // so that all requests will be relative to "/" and not the
        // application such as "/weather".
        //
        return appserver.processUrlEntry(req, res, forward_app, name);
    }
    else {

        //
        // If the URL is exactly "/xxx" we load it as a default
        // against the configured default_app.
        //
        // This handles URL's such as "/openpuxclient.js" and redirects
        // them to "apps/openpux/client/javascript/openpuxclient.js"
        //

        if ((typeof(app.default_app) == "undefined") ||
            (app.default_app == null)) {

            return false;
        }

        forward_app = appserver.findAppByName(app.default_app);
        if (forward_app == null) {
            return false;
        }

        // trim root '/'
        name = url.substring(1, url.length);

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
        }

        //
        // This just processes a simple file URL from the targeted application.
        //
        return appserver.processUrlEntry(req, res, forward_app, name);
    }
}

module.exports = {
  App: App
};
