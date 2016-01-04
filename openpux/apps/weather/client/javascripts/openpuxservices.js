
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

//
// This file incorporates the openpuxclient.js library as an AngularJS Service.
//

'use strict';

//
// function getOpenpuxConfig() {} is implemented in the main
// web page, or other configuration service.
//
// The returned configuration identifies the token, and optional account,
// and sensor
//
// var openpux_config =
// {
//    cloud_token: "12345678",
//    cloud_account: "1",
//    cloud_sensor_id: "1"
// };
//
//

//
// Custom Services:
//
// https://docs.angularjs.org/tutorial/step_11
//

var openpuxServices = angular.module('openpuxServices', []);

openpuxServices.factory('Openpux', [

  // No service dependencies in this requires array

  function() {

      // Create the service object
      var o = {};    

      o.opdata = null;
      o.opclient = null;

      var config = getOpenpuxConfig();

      //
      // Simple data display pages use this worker class
      //
      o.getLatestReading = function(callback) {
          if (this.opdata == null) {
              o.opdata = new OpenpuxData(config);
          }

          this.opdata.getLatestReading(callback);
      };

      //
      // More complex applications use the full library directly
      //
      o.getOpClient = function() {
          if (this.opclient == null) {
              o.opclient = new OpenpuxClient();

              // null for host_args will load the origin from window.location
              this.opclient.defaultTicket =
                  this.opclient.createTicket(config.cloud_token, "/", null);
          }

          // This provides access to all the functions in openpuxclient.js
          return o.opclient;
      };

      return o;
  }]);
