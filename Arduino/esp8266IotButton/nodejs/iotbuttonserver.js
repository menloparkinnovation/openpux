
/*
 * Copyright (C) 2016 Menlo Park Innovation LLC
 *
 * This is licensed software, all rights as to the software
 * is reserved by Menlo Park Innovation LLC.
 *
 * A license included with the distribution provides certain limited
 * rights to a given distribution of the work.
 *
 * This distribution includes a copy of the license agreement and must be
 * provided along with any further distribution or copy thereof.
 *
 * If this license is missing, or you wish to license under different
 * terms please contact:
 *
 * menloparkinnovation.com
 * menloparkinnovation@gmail.com
 */

/*
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2016 Menlo Park Innovation LLC
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

var g_serverPort = null;

if (typeof(process.env.port) != "undefined") {
    g_serverPort = process.env.port;
    console.log("using port from process.env.port");
}
else {
    console.log("using built in port");
    g_serverPort = 8080;
}

var g_serverInterface = "0.0.0.0";

//var g_serverInterface = "127.0.0.0";

var http = require('http');

//
// This handles a JSON post from an IoT device.
//
// Input:
//
// {
//   "device_id": "001",
//   "button_state": "pressed",
//   "battery_level": "90"
// }
//
// Response:
//
// {
//   "status": 200,
//   "message": "Success"
// }
//
function handleIotButtonPost (req, res) {

    var reqDocument = null;

    tracelog("IotButtonPost: getting document");

    reqDocument = receiveHttpStreamToBuffer(req, res, function(statusCode, document) {

        if (statusCode != 200) {
            sendJsonResponse(req, res, statusCode, "error receiving document");
            return;
        }

        //
        // Convert from JSON to an object
        //
        var reqObj = parseJSON(document);
        if (reqObj == null) {
            sendJsonResponse(req, res, 400, "error parsing document");
            return;
        }

        console.log("POST: document from IoT device=");
        console.log(reqObj);

        sendJsonResponse(req, res, 200, "Success");
    });
}

function httpPostFunction (req, res) {

    tracelog("request: url=" + req.url);
    tracelog("method=" + req.method);

    if (req.url == '/iotbutton') {
        handleIotButtonPost(req, res);
        return true;
    }
    else {
        tracelog("Unknown URL " + req.url);
        sendJsonResponse(req, res, 404, "Unknown url ");
        return false;
    }
}

function httpGetFunction (req, res) {

    tracelog("request: url=" + req.url);
    tracelog("method=" + req.method);

    sendJsonResponse(req, res, 405, "GET not supported");
}

//
// This is invoked as a callback from http.createServer()
// and dispatches the request.
//
// req is http.IncomingMessage and implements the stream.Readable interface
//   http://nodejs.org/docs/latest/api/http.html#http_http_incomingmessage
//   http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
//
// res is http.ServerResponse
//   http://nodejs.org/docs/latest/api/http.html#http_class_http_serverresponse
//
function processHttpRequest (req, res) {

   try {
	tracelog("httprequest dispatch: got request");

	DumpRequest(req, res);

	// Dispatch http request type
	if (req.method == "POST") {
	    return httpPostFunction(req, res);
	}
	else if (req.method == "GET") {
	    return httpGetFunction(req, res);
	}
	else {
	    sendJsonResponse(req, res, 405, "Not Supported");
	    return false;
	}
    } catch (e) {
        errorlog("servfile: Exception handling request e=");
        errorlog(e);
        sendJsonResponse(req, res, 400, "exception processing request");
        return false;
    }
}

function parseJSON(jsonText) {

    var obj = null;
    try {
        obj = JSON.parse(jsonText);
    } catch(e) {
        // handle error
        console.log("json parse error=" + e);
    }

    return obj;
}

//
// Send a response as JSON.
//
function sendJsonResponse (req, res, resultCode, resultMessage) {

    tracelog("sendJsonResponse: Result: " + resultCode + " " + resultMessage);

    var responseObj = {};

    responseObj.status = resultCode;
    responseObj.message = resultMessage;

    var responseDoc = JSON.stringify(responseObj);

    res.writeHead(resultCode, {'Content-Type': 'application/json'});
    res.write(responseDoc, 'utf8');
    res.end();
}

//
// Receive an HTTP file stream into a buffer.
//
// Note: The whole file is received into the buffer and only
// works for files that fit in memory.
//
// For large files or small platforms see pipeStreamToFile().
//
function receiveHttpStreamToBuffer(req, res, callback) {

    // Start with empty string
    var body = '';
    var statusCode = 200;

    // Set encoding to utf8, otherwise default is binary
    req.setEncoding('utf8');

    // request fires 'data' events with the data chunk(s)
    req.on('data', function(chunk) {
        body += chunk;
    });

    // error is invoked on a stream error
    // http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
    req.on('error', function(error) {
        errorlog("http read stream error:");
        errorlog(error);
        statusCode = 404;
        callback(statusCode, null);
    });

    // 'end' event indicates entire body has been delivered
    req.on('end', function() {
        errorlog("stream end received");
        callback(statusCode, body);
    });
}

// General information such as startup, shutdown, etc.
function infolog(message) {
    console.log(message);
}

// Errors such as bad requests, etc.
function errorlog(message) {
    console.error(message);
}

// Deeper tracing, debug client requests, local server actions, etc.
function traceerror(message) {
    console.error(message);
}

function tracelog(message) {
    console.log(message);
}

function printhex(value) {
    console.log(value.toString(16));
}

//
// This is useful for debugging form input
//
function DumpRequest(req, res) {
    tracelog("req.url=" + req.url);
    tracelog("req.method=" + req.method);
    tracelog("req.headers:");
    tracelog(req.headers);
}

//
// Create the Http Server
//

http.createServer(processHttpRequest).listen(g_serverPort, g_serverInterface);

console.log("iotbuttonserver started port=" + g_serverPort + " interface=" + g_serverInterface);
