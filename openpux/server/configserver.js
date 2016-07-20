
//
//   config.js
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

//
// Configuration handling routines for main server and applications.
//

var fs = require('fs');

function ConfigServer(config)
{
    this.moduleName = "ConfigServer";
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

    this.storage = this.config.storage;
}

//
// Load a configuration file as json.
//
// This is sync since application configuration files are used
// just before require() which is sync.
//
// Returns a Javascript object parsed from the json on success.
//
// Returns:
//
// {
//   error: "error message or null",
//   config: null // or pointer to obj
// }
//
ConfigServer.prototype.loadConfigFileSync = function(filePath) {

    var errorText;
    var result = {
        error: null,
        config: null
        };

    try {
        var jsonText = fs.readFileSync(filePath);

        try {
            var local_config = JSON.parse(jsonText);

            // Successfully loaded
            result.config = local_config;
            return result;

        } catch(ee) {
            errorText = "ConfigServer: json parsing error file=" + filePath + " exception=" + ee;
            result.error = errorText;
            return result;
        }

    } catch (e) {
        errorText = "ConfigServer: load error file=" + filePath + " exception=" + e;
        result.error = errorText;
        return result;
    }
}

module.exports = {
  Server: ConfigServer
};
