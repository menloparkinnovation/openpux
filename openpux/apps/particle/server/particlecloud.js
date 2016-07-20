
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
// Particle API
//
// "sudo npm install --save particle-api-js" // original registration
// "sudo npm install"                        // install per deployment
//
var Particle = require('particle-api-js');
var particle = new Particle();

//
// Particle API uses the promises pattern.
//
// https://promisesaplus.com/
//

function ParticleCloud(config) {

    this.moduleName = "ParticleCloud";
    this.config = config;

    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(config.Trace) != "undefined") {
        this.trace = config.Trace;
    }

    if (typeof(config.TraceError) != "undefined") {
        this.traceerrorValue = config.TraceError;
    }
}

//
// These worker routines not only provide code examples
// but keep the promises pattern used by the Particle API
// within this module. Promises is not the standard node.js
// programming style and does not get riddeled throughout
// my projects.
//
// Plus there are multiple competing implementations.
//
// All API's work with the standard node.js callback(error, data) pattern.
//

//
// Login to the Particle Cloud
//
// creds:
//
// {
//    username: "email",
//    password: "something"
// };
//
// callback(error, token)
//
ParticleCloud.prototype.login = function(creds, callback) {

    particle.login(creds).then(

      // promise.resolve
      function(data) {
        callback(null, data.body.access_token);
      },

      // promise.fail
      function(err) {
        callback(err, null);
      }
    );
}

//
// listdevices:
//
// parameters:
//
//  token - token from login
//
// returns:
//
// callback(error, devices)
//
// devices response object dumped as json:
/*
{ body: 
   [ { id: '280041000347343337373738',
       name: 'WeatherStation2',
       last_app: null,
       last_ip_address: '50.35.41.141',
       last_heard: '2016-03-18T22:33:33.050Z',
       product_id: 6,
       connected: true,
       platform_id: 6,
       cellular: false,
       status: 'normal' },
     { id: '500041000c51343334363138',
       name: 'menloelectron',
       last_app: null,
       last_ip_address: '176.83.208.56:4093',
       last_heard: '2016-02-24T04:18:54.452Z',
       product_id: 10,
       connected: false,
       platform_id: 10,
       cellular: true,
       status: 'normal',
       last_iccid: '8934076500002724276',
       imei: '353162071113006',
       current_build_target: '0.4.8' },
     [length]: 2 ],
  statusCode: 200 }
*/
//
ParticleCloud.prototype.listdevices = function(token, callback) {

    var auth = {auth: token};

    var devicesPr = particle.listDevices(auth);

    devicesPr.then(

      // promise.resolve
      function(devices){
        callback(null, devices);
      },

      // promise.fail
      function(err) {
        callback(err, null);
      }
    );
}

//
// callback(error, data)
//
/*
{ body: 
   { id: '280041000347343337373738',
     name: 'WeatherStation2',
     last_app: null,
     last_ip_address: '50.35.41.141',
     last_heard: '2016-03-18T22:33:33.050Z',
     product_id: 6,
     connected: true,
     platform_id: 6,
     cellular: false,
     status: 'normal',
     variables: { dweetreply: 'string' },
     functions: [ 'dweet', [length]: 1 ],
     cc3000_patch_version: null },
  statusCode: 200 }
*/
//
ParticleCloud.prototype.getattributes = function(token, deviceid, callback) {

    var devicesPr = particle.getDevice({ deviceId: deviceid, auth: token });

    // promise.resolve
    devicesPr.then(
      function(data){
        //console.log('Device attrs retrieved successfully:', data);
        callback(null, data);
      },

      // promise.fail
      function(err) {
        //console.log('API call failed: ', err);
        callback(err);
      }
    );
}

//
// callback(error, data)
//
/*
{ body: 
   { cmd: 'VarReturn',
     name: 'dweetreply',
     result: '$PDWT',
     coreInfo: 
      { last_app: '',
        last_heard: '2016-03-19T19:01:16.873Z',
        connected: true,
        last_handshake_at: '2016-03-18T22:33:33.050Z',
        deviceID: '280041000347343337373738',
        product_id: 6 } },
  statusCode: 200 }
*/
//
ParticleCloud.prototype.getvariable = function(token, deviceid, variable_name, callback) {

    var params = { deviceId: deviceid, name: variable_name, auth: token };

    particle.getVariable(params).then(

      // promise.resolve
      function(data) {
          //console.log('Device variable retrieved successfully:', data);
          callback(null, data);
          return;
      },

      // promise.fail
      function(err) {
          //console.log('An error occurred while getting variable:', err);
          callback(err, null);
          return;
      });
}

//
// callback(error, data)
//
ParticleCloud.prototype.callParticleFunction =
    function(token, deviceid, function_name, function_argument, callback) {

    var params = {
        deviceId: deviceid, name: function_name, argument: function_argument, auth: token
    };

    var fnPr = particle.callFunction(params);

    fnPr.then(

      // promise.resolve
      function(data) {
          callback(null, data);
      },

      // promise.fail
      function(err) {
          callback(err, null);
      });
}

//
// On success will invoke callback for each event that occurs.
//
// Note: an outstanding event stream registration should keep
// the node.js program loaded until canceled or exit explicitly.
//
// callback(error, stream, data)
//
ParticleCloud.prototype.geteventstream = function(token, deviceid, event_name, callback) {

    // This generates path https://api.particle.io/v1/events/readings
    var params = { name: event_name, auth: token };

    //var params = { deviceId: deviceid, name: event_name, auth: token };

    particle.getEventStream(params).then(

        // promise.resolve
        function(stream) {
            stream.on('event', function(data) {
                console.log("Event: " + data);
                callback(null, stream, data);
            },

         // promise.fail
         function(err) {
             //console.log('An error occurred:', err);
             callback(err, null, null);
          });
    });
}

module.exports = {
  Module: ParticleCloud
};
