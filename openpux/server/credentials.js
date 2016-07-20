
//
//   credentials.js
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
// Common credential handling routines.
//
// Credentials stored in openpux/credentials
//
// Default Administrator credentials:
//
//   openpux/credentials/server_credentials
//
//   Available when no data service available.
//
//   TOOD: move out of the config/* files.
//
// Per app credentials:
//
//   openpux/credentials/appname_credentials.json
//
//   TODO: Have the apps call the routines in this file.
//
//   TODO: Have the appserver app loader call the routines in this file.
//

var fs = require('fs');

//
// Config is the master configuration file for Openpux as a Javascript object.
//
function CredentialsServer(config)
{
    this.moduleName = "CredentialsServer";
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

    //
    // Storage relies on file system based credentials during initialization
    // and is brought online later by a call to initializeStorage().
    //
    this.storage = null;

    //
    // The master configuration file allows deployment time credentials
    // outside of the deployment runtime root.
    //
    if ((typeof(this.config.credentialsDirectoryPath) != "undefined") &&
        (this.config.credentialsDirectoryPath != null)) {
        this.credentialsDirectoryPath = config.credentialsDirectoryPath;
    }
    else {
        this.credentialsDirectoryPath = "credentials/";
    }
}

//
// During first construction, storage is not available as the storage
// subsystem uses the credentials server for its connection.
//
// Later when credentials are placed in the data storage system
// it can be brought online by initializeStorage().
//
CredentialsServer.prototype.initializeStorage = function(storage)
{
    this.storage = storage;
}

//
// Load a credentials file as json.
//
// This is sync since file system based credential files are used
// just before require() which is sync.
//
// Returns a Javascript object parsed from the json on success.
//
// Returns:
//
// {
//   error: "error message or null",
//   credentials: null // or pointer to obj
// }
//
CredentialsServer.prototype.loadCredentialsFileSync = function(fileName) {

    var errorText;
    var result = {
        error: null,
        credentials: null
        };

    var credentialsFilePath = this.credentialsDirectoryPath + fileName;

    try {
        var jsonText = fs.readFileSync(credentialsFilePath);

        try {
            var local_config = JSON.parse(jsonText);

            // Successfully loaded
            result.credentials = local_config;
            return result;

        } catch(ee) {
            errorText = "CredentialsServer: json parsing error file=" +
            credentialsFilePath + " exception=" + ee;
            result.error = errorText;
            return result;
        }

    } catch (e) {
        errorText = "CredentialsServer: load error file=" +
            credentialsFilePath + " exception=" + e;
        result.error = errorText;
        return result;
    }
}

module.exports = {
  Server: CredentialsServer
};
