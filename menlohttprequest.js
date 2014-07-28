
//
//   Copyright (C) 2014 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
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
//   MenloHttpRequest.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014 Menlo Park Innovation LLC
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
//   This specific snapshot is made available under the terms of
//   the Apache License Version 2.0 of January 2004
//   http://www.apache.org/licenses/
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
  createHttpRequest: function () {
      return createMenloHttpRequest();
  }
};

var http = require('http');
var url = require('url');

// Constructor function
function MenloHttpRequest() {
}

function createMenloHttpRequest() {

    var obj = new MenloHttpRequest();

    // fields
    obj.debugTrace = 0;
    obj.debugTraceError = 1;

    obj.readyState = 0;
    obj.status = 401;
    obj.responseText = null;
    obj.requestHeaders = null;

    // methods
    obj.open = localHttpRequest_open;
    obj.setRequestHeader = localHttpRequest_setRequestHeader;
    obj.send = localHttpRequest_send;

    // Our worker
    obj.sendit = localHttpRequest_sendit;

    // callbacks
    obj.onreadystatechange = null;

    return obj;
}

function localHttpRequest_open(http_method, request_url, async, username, password)
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

function localHttpRequest_setRequestHeader(header)
{
    if (this.requestHeaders == null) {
        this.requestHeaders = new Object();
    }

    this.requestHeaders = header;
}

//
// This actually initiates the request
//
function localHttpRequest_send()
{
    this.sendit(function(req) {
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
function localHttpRequest_sendit(callback)
{
    // http://nodejs.org/api/url.html
    // This requires "http://hostname/path" to fully parse
    var parsed = url.parse(this.url);

    //console.log(parsed);

    // If its not supplied, its null, so default to 80
    if (parsed.port = null) {
        parsed.port = 80;
    }

    // http://nodejs.org/docs/latest/api/http.html#http_http_request_options_callback
    var options = {
        hostname: parsed.host,
        port: parsed.port,
        path: parsed.path,
        method: this.method,
        headers: this.headers
    };

    // Reset responseText to null string
    this.responseText = "";

    // Shows MenloHttpRequest
    //console.log("_sendit()Mainline this.constructor.name: " + this.constructor.name);

    //
    // We must save our this for proper callback during the closure
    // as this gets redefined to the closures callback object.
    //
    httpRequestThis = this;

    request = http.request(options, function(res) {

        //
        // "this" is now defined by the closure here, not the "this" that is in effect
        // localHttpRequest_sendit()
        //

        // This shows ClientRequest
        //console.log("_sendit() request.constructor.name: " + request.constructor.name);

        // This shows ClientRequest
        //console.log("_sendit_request_closure: this.constructor.name: " + this.constructor.name);

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

	//console.log('STATUS: ' + res.statusCode);
        //console.log('HEADERS: ' + JSON.stringify(res.headers));

        httpRequestThis.status = res.statusCode;

        // Part of the readable Stream interface
        // Class: stream.Readable
        // http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
        res.setEncoding('utf8');


        // http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
        // The documentation says this will get all of the data out of the
        // stream as fast as possible
        res.on('data', function (chunk) {
            if (httpRequestThis.debugTrace) {
                console.log("BODY:");
                console.log(chunk);
            }
            httpRequestThis.responseText += chunk;
	});

        res.on('end', function () {
            if (httpRequestThis.debugTrace) console.log("sendit: end Event invoked! Callingback callback(httpRequestThis)");

            // Shows IncomingMessage
            //console.log("SB IncomingMessage: " + this.constructor.name);

            // Shows ClientRequest
            //console.log("SB ClientRequest: " + request.constructor.name);

            // Shows MenloHttpRequest
            //console.log("SB MenloHttpRequest: " + httpRequestThis.constructor.name);

            callback(httpRequestThis);
        });
    });

    // Shows Object when "var obj = new Object()" is used.
    // Shows MenloHttpRequest when "var obj = new MenloHttpRequest()" is used
    // The second pattern is the constructor function pattern and is recommended.
    //console.log("this.constructor.name: " + this.constructor.name);

    this.request = request;

    // http://nodejs.org/docs/latest/api/http.html#http_class_http_clientrequest
    this.request.on('error', function(e) {
        if (httpRequestThis.debugTraceError) {
            console.log('problem with request: ' + e.message);
            console.log(e);
        }
        httpRequestThis.responseText += e.message;
        callback(httpRequestThis);
    });

    // TODO: Support this for POST/PUT
    // write data to request body
    //req.write('data\n');

    if (httpRequestThis.debugTrace) console.log("sendit: calling this.request.end()");

    this.request.end();
}
