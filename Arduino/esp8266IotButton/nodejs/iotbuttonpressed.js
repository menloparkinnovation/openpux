
//
// Simulate a button press from the IotButton device.
//

var g_serviceUrl = "http://iotbutton.azurewebsites.net";

var g_buttonMessage = '{"device_id": "123", "button_state": "1 pressed", "battery_level": "90"}';

//var g_serviceUrl = "http://192.168.1.7:8080";
//var g_buttonMessage = '{"device_id": "123", "button_state": "1 pressed", "battery_level": "90"}';

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

var http = require('http');
var url = require('url');

//
// Perform an HTTP POST
//
// callback(error, data)
//
function PerformHttpPost(targetUrl, postData, callback) {

    var parsed = url.parse(targetUrl);

    var responseText = "";

    var requestHeaders = new Object();
    requestHeaders["Content-Type"] =  "application/json";

    var options = {
        protocol: "http:",
        method: "POST",
        hostname: parsed.hostname,
        port: parsed.port,
        path: parsed.path,
        headers: requestHeaders
    };

    // https://nodejs.org/api/http.html#http_http_request_options_callback
    request = http.request(options, function(res) {

        res.setEncoding('utf8');

        // http://nodejs.org/docs/latest/api/stream.html#stream_class_stream_readable
        res.on('data', function (chunk) {
            console.log("received data...");
            responseText += chunk;
	});

        res.on('end', function () {
            console.log("received end");
            callback(null, responseText);
        });
    });

    // Error will occur right away and not invoke the 'data' or 'end' lamblas above
    request.on('error', function(e) {
        console.log("received error");
        callback(e, null);
    });

    if (postData != null) {
        request.write(postData);
        console.log("posted data...");
    }

    // Send it
    request.end();
}

function SendButton1Message(serviceUrl, buttonMessage)
{

    var url = g_serviceUrl + "/iotbutton";

    var jsonDocument = buttonMessage;

    PerformHttpPost(url, jsonDocument, function(error, data) {

        if (error != null) {
	    console.error("HTTP POST: error posting");
	    console.error(error);
	    return;
	}

	console.error("HTTP_POST: received file from url=" + url + ":");
	console.log(data);
    });
}

// Runit
SendButton1Message(g_serviceUrl, g_buttonMessage);
