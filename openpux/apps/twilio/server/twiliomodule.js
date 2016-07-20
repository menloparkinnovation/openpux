
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

//
// Handle simple twilio SMS text messages sent to an application
// SMS phone number.
//
// 03/20/2016
//
//
// This version integrates with an existing HTTP server.
//
// install dependencies with:
//
// npm install
//
// Use ngrok to expose a port over the internet for testing.
//
// /web/ngrok/ngrok http 8080
//
// This is because twilio requires an HTTP address reachable over
// the internet to forward SMS message notifications to your application.
//

//
// NOTE: process.env.TWILIO_AUTH_TOKEN must be set.
//
// The twilio account must be setup to route to the
// URL for the applications assigned SMS phone number.
//

//
// http://twilio.github.io/twilio-node/
//

var twilio = require('twilio');

var qs = require('querystring');

function TwilioModule(config) {

    this.moduleName = "TwilioModule";
    this.config = config;

    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(config.Trace) != "undefined") {
        this.trace = config.Trace;
    }

    if (typeof(config.TraceError) != "undefined") {
        this.traceerrorValue = config.TraceError;
    }

    this.logger = config.logger;

    // customer message handler function
    this.messageFunc = null;

    // Initially no client is setup
    this.client = null;
}

TwilioModule.prototype.RegisterClient = function(accountSid, authToken) {
    this.client = twilio(accountSid, authToken);
}

//
// Send a message
//
// callback(error, response_sid)
//
TwilioModule.prototype.SendMessage = function(to, from , message, callback) {

    var self = this;

    var params = { 
        to: to, 
        from: from, 
        body: message
    };

    this.client.messages.create(params, function(error, responseMessage) {

        if (error != null) {
            callback(error, null);
            return;
        }

        callback(null, responseMessage.sid);
    });
}

//
// The infrastructure this module is loaded by dispatches
// requests to our URL.
//
TwilioModule.prototype.DispatchRequest = function(req, res) {

    var self = this;
    var body = null;
    var data = "";

    // Receive body content
    req.setEncoding('utf8'); // default is binary

    req.on('data', function(chunk) {
        data += chunk;
    });

    req.on('end', function() {

        //console.log("Twilio response document");
        //console.log(data);
      
        body = qs.parse(data);

        //console.log("Twilio response document parsed");
        //console.log(body);

        self.handleMessage(body, body.Body, function(response_message) {

            var twiml = new twilio.TwimlResponse();
            twiml.message(response_message);

            // output the response
            res.writeHead(200, {'Content-Type':'text/xml'});
            res.end(twiml.toString());
        });
    });
}

//
// Register the application handler function.
//
TwilioModule.prototype.StartService = function(messageFunc) {
    this.messageFunc = messageFunc;
}

//
// Handle a message
//
// callback(response_message)
//
TwilioModule.prototype.handleMessage = function(body, message, callback) {

    var self = this;

    var response_message = "unhandled message";

    self.logger.info("Twilio: received message=" + message + " From: " + body.From);

    //
    // caller supplied a message handler to StartServer
    //
    if (self.messageFunc != null) {
        self.messageFunc(body, message, callback);
        return;
    }
    else {
        callback(response_message);
        return;
    }

/*
 The post body content looks like this:

{ ToCountry: 'US',
  ToState: 'WA',
  SmsMessageSid: 'smsmessagesid',
  NumMedia: '0',
  ToCity: 'RENTON',
  FromZip: '98007',
  SmsSid: 'smdsid',
  FromState: 'WA',
  SmsStatus: 'received',
  FromCity: 'BELLEVUE',
  Body: 'status',
  FromCountry: 'US',
  To: '+1xxxxxxxxx',
  MessagingServiceSid: 'messagingservicesid',
  ToZip: '98055',
  NumSegments: '1',
  MessageSid: 'SM50607e047681d0dbcacaf90d37672d49',
  AccountSid: 'accountsid',
  From: '+1xxxxxxxxx',
  ApiVersion: '2010-04-01' }
*/
}

TwilioModule.prototype.setTrace = function(value) {
    this.trace = value;
}

TwilioModule.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

TwilioModule.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

TwilioModule.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

TwilioModule.prototype.errlog = function(message) {
    // console.error connects to stderr
    console.error(message);
}

module.exports = {
  Module: TwilioModule
};


