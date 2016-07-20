
//
// 04/28/2016
//
// Design of automatic device SMS push notifications.
//
// The device on board detects an event and sends it out as
// a Dweet. This is normal function for any device that
// generates device events.
//
// For the Particle cloud this generates a "dweet" Particle cloud event,
// which is received by apps/server/particleapp.js, HandleMessage().
//
//   This means that particleapp.js has an outstanding web hook event
//   listener on Particle cloud "dweet" event name.
//
// This Dweet message is routed to dweetweatherapp.js, decoded
// as required.
//
//   This means some sort of decoding from Dweet syntax to
//   something that can be acted on.
//
// If a user message is to be initiated, it is sent using
// apps/twilio/server/twilioapp.js, SendMessage() to generate an
// async SMS text message.
//
//   The user can reply, which generates a new incoming message
//   handled as it is today.
//
// Things to make generic:
//
//   Need a way to generally dispatch dweet events received on
//   a transport to interested apps. They need to know the transport,
//   device id, etc. This allows dweet serial transport, smartpux cloud
//   transport, Particle cloud transport, etc.
//
//   Dweet should be its own app module. All transports receiving
//   dweets send them to the module for dispatch.
//
//   The Dweet app module raises events for received dweets and
//   allows interested apps to register for them.
//
//   All apps respond to a dweet, or initiate one to the dweet app
//   module.
//
//   This means all dweet events have a context to identify
//   transport, device, etc. For example on Twilio it may be different
//   SMS phone numbers, while on Particle cloud its the particle device
//   ID. On the openpux/smartpux cloud its account, sensorid.
//
//   Need a way to generically dispatch Twilio SMS events as either
//   user application commands, or as dweets themselves. The transport
//   and device identifiers need to be supplied so an application
//   handler can respond to the proper device in the proper context.
//
//   Note: The above model will be inside this app initially, then
//   moved out to infrastructure, specifically with the creation
//   of openpux event handler lists.
//

//
//   dweetweatherapp.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2016 Menlo Park Innovation LLC
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

var path = require('path');

var utility = require('../../../server/utility.js');

//
// g_dweet_commands: full name matches to the Dweet GETSTATE
// commands in a standard form.
//

//
// GETSTATE=COMMAND
//
// Note: Dweet GETSTATE=<nmea_prefix> elicits a one time stream
// of the given prefix. Note that the sentence start character "$"
// is dropped from the NMEA prefix you want to elicit as its a reserved
// character already used to start the DWEET sentence when sent.
//
// This is used solicit general NMEA sentences from a remote device
// without having it stream the messages, which is expensive over
// a channel such as SMS and occasionally connected sensors.
//
// Example:
//
// $PDWT,GETSTATE=WIMWV*00
//
// Response:
//
// $WIMWV,270,T,3.5,M,A*00
//
var g_dweet_getstate_commands = {
    "wimwv": "WIMWV",
    "wimda": "WIMDA",
    "iixdr": "IIXDR",
    "yxxdr": "YXXDR",
    "windspeed": "WINDSPEED",
    "winddirection": "WINDDIR",
    "temperature": "TEMPERATURE",
    "barometer": "BAROMETER",
    "humidity": "HUMIDITY",
    "rainfall": "RAINFALL",
    "freememory": "FREEMEMORY"
};

//
// These are substitutions for short hand, text lingo, and common misspellings.
//
// This keeps from polluting the dweet commands table with application specific
// shorthand.
//
var g_weatherapp_aliases = {

    "t": "temperature",
    "temp": "temperature",
    "temperature": "temperature",

    "w": "windspeed",
    "wind": "windspeed",
    "windspeed": "windspeed",

    "d": "winddirection",
    "dir": "winddirection",
    "winddir": "winddirection",
    "winddirection": "winddirection",

    "p": "barometer",
    "pressure": "barometer",
    "baro": "barometer",
    "barometer": "barometer",
    "b": "barometer",
    "baro": "barometer",
    "barometer": "barometer",

    "h": "humidity",
    "humid": "humidity",
    "humidity": "humidty",

    "r": "rainfall",
    "rain": "rainfall",
    "rainfall": "rainfall",

    "m": "freememory",
    "fm": "freememory",
    "memory": "freememory",
    "freememory": "freememory"
};

//
// appConfiguration.credentials contains the credentials loaded by
// the app module loader from credentials/appname_credentials.json
//
function App(config, appConfiguration)
{
    var self = this;

    //
    // Settings for sending event SMS messages
    //
    this.sendReadingsSMS = false;


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

    this.storage = config.storage;

    this.utility = config.utility;

    this.configserver = this.config.configserver;

    //
    // Note: dependency load order of application is handled by the
    // app server infrastructure loading the application module
    // on demand if it would otherwise be configured to be loaded
    // later depending on its order in the applications.json configuration.
    //
    // The dependencies{} entry in configuration.json is currently
    // informative, and not used currently for sequencing load orders.
    //

    // Locate the Twilio app whose services we consume
    this.twilio_app = this.config.appserver.getAppInstanceByName("twilio");
    if (this.twilio_app == null) {
        this.logger.error("DweetWeatherApp: Could not load Twilio app dependency");
        this.logger.info("DweetWeather service failed to start");
        return;
    }

    // DeviceId comes from the credentials configuration
    if (this.appconfig.credentials == null) {
        this.particle_deviceid = -1;
        this.logger.error("DweetWeatherApp: No credentials for particle cloud");
        this.logger.info("DweetWeather service failed to start");
        return;
    }
    else {
        this.particle_deviceid = this.appconfig.credentials.particle.deviceid;
    }

    // Get the Particle cloud support application
    this.particle_app = this.config.appserver.getAppInstanceByName("particle");
    if (this.particle_app == null) {
        this.logger.error("DweetWeatherApp: Could not load Particle app dependency");
        this.logger.info("DweetWeather service failed to start");
        return;
    }
    else {

        // Acquire the lower level Particle API's
        this.particle_api = this.particle_app.getParticleApi();
        if (this.particle_api == null) {
            this.logger.error("DweetWeatherApp: Could not load Particle api dependency");
            this.logger.info("DweetWeather service failed to start");
            return;
        }
    }

    this.Initialize();

    this.logger.info("DweetWeather service started");
}

App.prototype.Initialize = function () {

    var self = this;

    //
    // Register a Twilio message handler
    //
    self.twilio_app.RegisterMessageHandler(function(body, message, callback) {
        self.HandleTwilioMessage(body, message, callback);
    });

    //
    // Start async login to the particle cloud
    //
    var name = self.appconfig.credentials.particle.cloud.username;
    var password = self.appconfig.credentials.particle.cloud.password;

    self.particle_app.login(name, password, function(error) {
        if (error != null) {
            self.logger.error("dweetweather: could not acquire particle login");
            return;
        }

        self.logger.info("dweetweather: particle cloud is ready");

        //
        // Register to receive Dweet events from the particle cloud
        //
        var token = self.particle_app.getParticleToken();

        //
        // null device id since it using a bearer token with path:
        // https://api.particle.io/v1/events/readings instead of:
        // /v1/devices/deviceId/events/name/
        //

        var deviceid = null;

        var eventName = "dweet";
        //var eventName = "readings";

        // Get the underlying API handler
        var particle_api = self.particle_app.getParticleApi();

        try {

            self.particle_api.geteventstream(token, deviceid, eventName,
                function(error2, stream, data) {
                    self.HandleParticleDweetEvent(error2, stream, data);
                });
        }
        catch(e) {
            self.logger.error("Exception! dweetweatherapp: particle_api.geteventstream");
            self.logger.error(e);
        }
    });
}

//
// Handle a Dweet event from the particle cloud
//
App.prototype.HandleParticleDweetEvent = function(error, stream, data) {

    var self = this;

    if (error != null) {
        self.logger.error("ParticleDweetEvent: error=" + error);
        return;
    }

    if (data == null) {
        self.logger.warn("ParticleDweetEvent data=null");
        return;
    }

    if (data.data == null) {
        self.logger.warn("ParticleDweetEvent=null");
        return;
    }

    self.logger.info("ParticleDweetEvent:");

    //
    // data.data
    // data.coreid
    // data.ttl
    // data.published_at
    // data.name
    //

    //
    // {
    // data: '&winddirection=3&windgustdir=0&moisture=1367&windspeed=2.9721&windgust=0.0000&rainfall=0.0000&humidity=53.6008&temperature=62.7292&barometer=102173.0000&soiltemp=0.0000',
    //  ttl: '60',
    //  published_at: '2016-04-30T16:40:34.034Z',
    //  coreid: '280041000347343337373738',
    //  name: 'readings' }
    //

    if (data.name == "readings") {

        // Readings are sent every 30 seconds
        self.logger.info(data.data);

        if (!self.sendReadingsSMS) {
            // Readings are sent every 30 seconds
            self.logger.info("readings SMS not configured");
            return;
        }

        self.SendTwilioMessage(data.data, function(error, sid) {
            if (error != null) {
                self.logger.warn("error sending SMS message " + error);
            }

            if (sid != null) {
                self.logger.info("readings SMS sent sid=" + sid);
            }
        });

        return;
    }
    else if (data.name == "dweet") {

        // dweet is initiated by the device
        self.logger.info(data.data);

        self.SendTwilioMessage(data.data, function(error, sid) {
            if (error != null) {
                self.logger.warn("error sending SMS message " + error);
            }

            if (sid != null) {
                self.logger.info("dweet SMS sent sid=" + sid);
            }
        });

        return;
    }
    else {
        self.logger.warn("ParticleDweetEvent unknown event name=" + data.name);
        self.logger.warn(data.data);
        return;
    }
}

//
// callback(error, sid)
//
App.prototype.SendTwilioMessage = function(message, callback) {

    var self = this;

    //
    // null for to and from will use default numbers in
    // the configured twilio credentials file.
    //
    var to = null;
    var from = null;

    self.twilio_app.SendMessage(to, from, message, function(error, sid) {

        if (error != null) {
            callback(error, null);
            return;
        }

        callback(null, sid);
    });
}

//
// This is the function we register with Twilio
//
App.prototype.HandleTwilioMessage = function(body, message, callback) {

    var self = this;

    var response_message = "dweet weather: unrecognized request";

    if (message == "status") {
        response_message = "dweet weather ready";
        callback(response_message);
        return;
    }

    self.PerformApplicationCommand(
        message, g_weatherapp_aliases, g_dweet_getstate_commands, function(error, result) {

            if (error != null) {
                response_message = error;
            }
            else {
                response_message = result;
            }

            callback(response_message);
    });
}

//
// Replace a character at the given position in the string.
//
// A new string is returned as strings are immutable.
//
function ReplaceAt(str, index, character) {
    return str.substr(0, index) + character + str.substr(index+character.length);
}

//
// Convert a string to Upcasefirst format
//
function UpCaseFirst(str) {
    var lower = str.toLowerCase();
    var firstChar = lower[0].toUpperCase();
    var upcaseFirst = ReplaceAt(lower, 0, firstChar);
    return upcaseFirst;
}


//
// Look up the Dweet command for the application specific command alias
//
// Execute the Dweet command if found.
//
App.prototype.PerformApplicationCommand = function(user_command, alias_table, dweet_table, callback) {

    var self = this;

    var dweet_command = null;

    var deviceid = self.particle_deviceid;

    if (deviceid == -1) {
        callback("no device id or credentails", null);
        return;
    }

    for (var prop in alias_table) {
        if (user_command.toLowerCase() == prop) {
            dweet_command = alias_table[prop];
            break;
        }
    }

    if (dweet_command == null) {
        callback("unrecognized command " + user_command, null);
        return;
    }

    self.DweetGetValue(deviceid, dweet_command, g_dweet_getstate_commands, function(error, data) {

        if (error) {
            callback(error, null);
            return;
        }

        // The data is a hex number as a string
        var reply = self.HexStringToDecimalString(data);

        // Convert dweet_command from "TEMPERATURE" to "Temperature"
        var result = UpCaseFirst(dweet_command) + "=" + reply;
        callback(null, result);
    });
}

//
// Lookup the specific Dweet value in the table and execute it if found.
//
App.prototype.DweetGetValue = function(deviceid, user_command, table, callback) {

    var command = null;

    for (var prop in table) {

        if (user_command.toLowerCase() == prop) {
            command = table[prop];
            break;
        }
    }

    if (command == null) {
        callback("command not found", null);
        return;
    }

    this.DweetGetState(deviceid, command, callback);
}

//
// Perform a Dweet Exchange
//
// This sends a dweet using the Particle function invoke cloud API
// and then reads the response variable from the target device.
//
// callback(error, dweetreply)
//
App.prototype.DweetExchange = function(deviceid, dweet_command, callback) {

    // Command to get temperature
    var functionName = "dweet";
    var functionArg = dweet_command;

    // Particle variable that stores the reply
    var variableName = "dweetreply";

    var self = this;

    var token = self.particle_app.getParticleToken();
    if (token == null) {
        callback("DweetExchange: particle cloud service not logged in or online", null);
        return;
    }

    self.particle_api.callParticleFunction(
        token, deviceid, functionName, functionArg, function(error, data) {

        if (error != null) {
            callback("DweetExchange: particle dweet function call failure " + error, null);
            return;
        }

        //
        // Get the Dweet reply
        //
        // Note: Dweet is async, but many Dweet to generate a reply
        // which is saved in the dweetreply particle variable.
        //
        self.particle_api.getvariable(token, deviceid, variableName, function(error2, data2) {

            if (error2 != null) {
                callback("DweetExchange: particle getvariable dweetreply failure " + error2, null);
                return;
            }

            /*
            { body: 
               { cmd: 'VarReturn',
                 name: 'dweetreply',
                 result: '$PDWT,GETSTATE_REPLY=TEMPERATURE:0047*7F\r\n',
                 coreInfo: 
                  { last_app: '',
                    last_heard: '2016-03-27T23:53:53.457Z',
                    connected: true,
                    last_handshake_at: '2016-03-27T14:24:18.833Z',
                    deviceID: 'device_id',
                    product_id: 6 } },
                statusCode: 200 }
            */
            callback(null, data2.body.result);
        });
    });
}

//
// Perform a Dweet GETSTATE= exchange
//
// callback(error, reply_value)
//
App.prototype.DweetGetState = function(deviceid, command, callback) {

    var self = this;

    var dweet = "GETSTATE=" + command;
    var dweet_reply = "GETSTATE_REPLY=" + command + ":";

    if (self.particle_deviceid == -1) {
        callback("no device id or credentails", null);
        return;
    }

    self.DweetExchange(self.particle_deviceid, dweet, function(error, dweetreply) {

        if (error != null) {
            callback(error, null);
            return;
        }

        //
        // $PDWT,GETSTATE_REPLY=TEMPERATURE:003A*0E
        //
        var parsed = null;

        try {
            parsed = self.ParseDweet(dweetreply);
        } catch(e) {
            console.log("exception e=" + e);
            // fallthrough with parsed == null
        }

        if ((parsed == null) || (parsed.error != null) ||
            (parsed.commands == null) || (parsed.commands.length == 0)) {
            callback(null, "invalid response received from device");
            return;
        }

        // GETSTATE_REPLY=TEMPERATURE:003A
        var tmp = parsed.commands[0];
        if (tmp.search(dweet_reply) != 0) {
            callback(null, "wrong device response");
            return;
        }

        var position = tmp.indexOf(':');
        var reply_value = tmp.substring(position+1, tmp.length+1);

        callback(null, reply_value);
    });
}

//
// Convert a hex number as a string to a decimal number
// as a string.
//
App.prototype.HexStringToDecimalString = function(hexnum) {
    var value = parseInt(hexnum, 16);
    var decimalString = value.toString();
    return decimalString;
}

//
// Parse a Dweet message.
//
// Returns an object with the following data:
//
// o.error - != null is a parse error
// o.sentence = raw sentence
// o.prefix - prefix
// o.commands - array of commands/words that were separated by "," in sentence
// o.checksumOK - true if checksum is ok.
// o.checksumIsPlusOne - true if checksum is one greater than the sentence checksum
// o.calculatedChecksum - check sum calculated.
// o.checksumMsb - received checksum msb
// o.checksumLsb - received checksum lsb
//
// This routine is from dweet/lib/nmea0183.js
//
App.prototype.ParseDweet = function(str) {

    var o = new Object();
    o.error = null;
    o.prefix = null;
    o.commands = null;
    o.checksumOK = false;
    o.sentence = str;

    if (str[0] != '$') {
        o.error = "Sentence does not start with $";
        return o;
    }

    // Find first "," which marks the end of the received prefix
    var index = str.indexOf(',');
    if (index == (-1)) {
        o.error = "Sentence missing separator , after prefix";
        return o;
    }

    o.prefix = str.substring(0, index);

    if (o.prefix.length > this.maxPrefixLength) {
        o.error = "maximum prefix length exceeded";
        return o;
    }

    var index = str.indexOf('*');
    if (index == (-1)) {
        o.error = "missing commands terminator *";
        return o;
    }

    //
    // This returns the check sum as hex with upper case characters
    // since this is the representation in NMEA 0183.
    //
    try {
        o.calculatedChecksum = this.Nmea0183Checksum(str);
    }
    catch (e) {
        //
        // This occurs due to bad formatting, not due to a bad checksum
        // since that comparison is later.
        //
        o.error = "checksum calculation exception e=" + e.toString();
        return o;
    }

    // Prefix + command separator ","
    // $PDWT,
    var start = o.prefix.length + 1;

    var commands = str.substring(start, index);

    var checkdigits = str.substring(index+1, index+3);

    o.checksumMsb = checkdigits[0];
    o.checksumLsb = checkdigits[1];

    // Create an array of the commands split on "," separator
    o.commands = commands.split(',');

    // split can return a "" entry[0]
    if ((o.commands.length > 0) && (o.commands[0].length == 0)) {
        o.commands = o.commands.slice(1, o.commands.length);
    }

    if (checkdigits == o.calculatedChecksum) {
        o.checksumOK = true;
    }
    else {

       //
       // Check if the checksum inside the sentence is one
       // greater than the calculated checksum. This occurs
       // for routed messages on purpose to artificially
       // "poison" them if they get separated from their
       // routing header. We indicate this to the caller
       // since they may be a router expecting this case.
       //
       o.checksumIsPlusOne = false;

       if (checkdigits == (o.calculatedChecksum + 1)) {
           o.checksumIsPlusOne = true;
       }

        o.error = "checksum error " + o.calculatedChecksum + 
                  " does not match check digits " + checkdigits;

        //
        // We still return the commands to let the caller
        // decide if they want to accept the message.
        //
    }

    return o;
}

//
// http://en.wikipedia.org/wiki/NMEA_0183#C_implementation_of_checksum_generation
//
// Checksum is returned as a string in upper case.
//
// This routine is from dweet/lib/nmea0183.c, checksum
//
App.prototype.Nmea0183Checksum = function(str) {
    var err;
    var chksum = 0;
    var index = 0;

    // <CR><LF> is not counted in maximum length
    if (str.length > this.configuredMaxLength + 2) {
        throw "string length to long";
    }

    //
    // Note: CHKSUM is a bytewise operation, make sure
    // this works out corrrectly in Javascript.
    //

    index = 0;

    if (str[index] != '$') {
        err =  "message must start with $";
        throw err;
    }

    //
    // Check sum starts at 0.
    //
    // Includes all parts of the string but $ and *
    //
    // If any <CR><LF> exists they are ignored as
    // well.
    //
    // Example:
    //   $PSPX,STRICT=ON*
    //

    // skip any '$'
    index++;

    for (; index < str.length; index++) {

        // done if we reach the '*' character at the end
        if (str[index] == '*') {
            break;
        }

        //console.log("before: chksum=" + chksum + " str[index]=" + str[index]);

        // keep it at two digits
        chksum = ((chksum & 0x00FF) ^ str.charCodeAt(index)) & 0x00FF;

        //console.log("after: chksum=" + chksum + " str[index]=" + str[index]);
    }

    if (str[index] != '*') {
        err = "message must end with *";
        throw err;
    }

    var chksumString = chksum.toString(16);

    // 0 pad to two digits
    if (chksumString.length == 1) {
        chksumString = "0" + chksumString[0];
    }
    else {
        // assert
        if (chksumString.length != 2) {
            //console.log("chhksumString=" + chksumString);
            //console.log("chhksumString.length=" + chksumString.length);
            //console.log("str.length=" + str.length);
            throw "chksum digits overflow";
        }
    }

    return chksumString.toUpperCase();
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

  // Specific configured application path not found.
  return false;
}

module.exports = {
  App: App
};
