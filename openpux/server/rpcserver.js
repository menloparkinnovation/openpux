
//
//   rpcserver.js
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
// This provides support to applications for general RPC handling.
//

var utility = require('./utility.js');

function RpcServer(config)
{
    this.moduleName = "RpcServer";
    this.config = config;

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
// rpc support:
//
// Handle a remote application request in an RPC style
// fashion.
//
// Parameters:
//
// req - http request
//
// res - http response
//
// remote_method_table - remote method table
//
// remote_method_object - object to invoke method on
//
// Input:
//
// { method: "getWeather" }
// { method: "getWeather", params: "value" }
//
// Response:
//
// {error: "error"}
// {result: "result"}
//
RpcServer.prototype.process_rpc_Request = 
    function (req, res, remote_method_table, remote_method_object) {

    var self = this;

    var json = null;

    // Receive the request JSON document as a parsed object
    utility.receiveJSONObject(req, res, self.logger, function(error, data) {

        if (error != null) {
            // erpc has a specific response document format
            json = JSON.stringify({ error: error });
            utility.logSendError(self.logger, req, res, error, json);
            return;
        }

        var result = 0;
        var params = null;

        if (typeof(data.params) != "undefined") {
            // parameters specified
            params = data.params;
        }

        // Logging for debug
        //console.log("process_rpc_request: params=");
        //utility.dumpasjson(params);

        //
        // data.method
        //
        // data.method could contain script, but without an exact name
        // match exists in the table, its rejected. This is why the simpler
        // eval() is not used and registration of remote methods is required.
        //
        for (var prop in remote_method_table) {

            if (remote_method_table[prop].name == data.method) {

                var func = remote_method_table[prop].func;

                try {

                    // We pass the request object for validation of tickets
                    func.call(remote_method_object, params, req, function(error, result) {

                        if (error != null) {
                            json = JSON.stringify({ error: error });
                        }
                        else {
                            json = JSON.stringify({ result: result });
                        }
                        res.writeHead(200, {'Content-Type': 'application/json'});            
                        res.write(json);
                        res.end();
                        return;
                    });

                } catch(e) {
                    console.log("exception e=");
                    console.log(e.stack.toString());
                    json = JSON.stringify({ error: "exception invoking rpc method", e: e.toString() });
                    utility.logSendError(self.logger, req, res, error, json);
                    return;
                }

                return;
            }
        }

        json = JSON.stringify({ error: "method not found" });
        utility.logSendError(self.logger, req, res, error, json);
        return;
     });
}

module.exports = {
  Server: RpcServer
};
