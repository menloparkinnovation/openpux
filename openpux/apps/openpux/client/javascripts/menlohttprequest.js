
//
//   menlohttprequest.js
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
// MenloHttpRequest provides a simple replacement for
// XMLHttpRequest available in web browsers, and gives an
// example of using node.js's built in http client.
//
// This allows the same code to be used for a command
// line client, unit testing.
//
// XMLHttpRequest document is at
// https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest
//
//

module.exports = {
  createHttpRequest: function () { return new MenloHttpRequest(); }
};

var http = require('http');
var url = require('url');

// Constructor function
function MenloHttpRequest() {

    // fields
    this.debugTrace = 0;
    this.debugTraceError = 1;

    this.readyState = 0;
    this.status = 401;
    this.responseText = null;
    this.requestHeaders = null;
    this.timeout = 0;

    // callbacks
    this.onreadystatechange = null;
}

MenloHttpRequest.prototype.open = function(http_method, request_url, async, username, password)
{
    //
    // The contract is that we just setup the parameters on open()
    //
    this.method = http_method;
    this.url = request_url;
    this.async = async;
    this.username = username;
    this.password = password;
}

MenloHttpRequest.prototype.setRequestHeader = function(header, value)
{
    //console.log("setRequestHeader: " + header + " value=" + value);

    if (this.requestHeaders == null) {
        this.requestHeaders = new Object();
    }

    this.requestHeaders[header] = value;
}

//
// This actually initiates the request
//
MenloHttpRequest.prototype.send = function(postDocument)
{
    this.sendit(postDocument, function(req) {
        req.readyState = 4;

        // Shows MenloHttpRequest
        //console.log("_send sendit callback: req.constructor.name: " + req.constructor.name);

        if (req.onreadystatechange != null) {
            req.onreadystatechange();
        }
        else {
            if (req.debugTraceError) console.log("_send(_sendit()): onreadystatechange not set!");
        }
    });
}

//
// Actually send the request
//
// Note: This has commented out details that shows how "this" changes
// in a node.js environment due to closures and lamba's on callbacks.
//
// Handy to better understand what is actually going on.
//
MenloHttpRequest.prototype.sendit = function(postDocument, callback)
{
    //
    // We must save our this for proper callback during the closure
    // as this gets redefined to the closures callback object.
    //
    var self = this;

    // http://nodejs.org/api/url.html
    // This requires "http://hostname/path" to fully parse
    var parsed = url.parse(self.url);

    // If its not supplied, its null, so default to 80
    if (parsed.port == null) {
        parsed.port = 80;
    }

    // http://nodejs.org/docs/latest/api/http.html#http_http_request_options_callback
    var options = {
        hostname: parsed.hostname,
        port: parsed.port,
        path: parsed.path,
        method: self.method,
        headers: self.requestHeaders
    };

    // Reset responseText to null string
    self.responseText = "";

    request = http.request(options, function(res) {

        //
	// callback is the "response" Event.
	// http://nodejs.org/docs/latest/api/http.html#http_event_response
        //
        // The res parameter is type http.IncomingMessage
	// http://nodejs.org/docs/latest/api/http.html#http_http_incomingmessage
        //
        // IncomingMessage implements Readable Stream
        // http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
        //

        // This an integer, not a string
        self.status = res.statusCode;

        // Part of the readable Stream interface
        // Class: stream.Readable
        // http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
        res.setEncoding('utf8');

        // http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
        // The documentation says this will get all of the data out of the
        // stream as fast as possible
        res.on('data', function (chunk) {
            if (self.debugTrace) {
                console.log("BODY:");
                console.log(chunk);
            }
            self.responseText += chunk;
	});

        res.on('end', function () {
            if (self.debugTrace) {
                console.log("sendit: end Event invoked! Callingback callback(self)");
            }
            callback(self);
        });
    });

    // https://nodejs.org/api/http.html#http_request_settimeout_timeout_callback
    // timeout is in milliseconds
    if (self.timeout != 0) {
        request.setTimeout(self.timeout);
    }

    self.request = request;

    // http://nodejs.org/docs/latest/api/http.html#http_class_http_clientrequest
    self.request.on('error', function(e) {
        if (self.debugTraceError) {
            console.log('problem with request: ' + e.message);
            console.log(e);
        }
        self.responseText += e.message;
        callback(self);
    });

    // write data to request body
    if (postDocument != null) {
        self.request.write(postDocument);
    }

    //
    // Note: Request will hang until this is done
    //
    self.request.end();
}
