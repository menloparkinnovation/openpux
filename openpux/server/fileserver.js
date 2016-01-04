
//
// TODO:
//  10/13/2015
//   - Validation administrator account for logs
//   - Remote statSync -> use callback
//
//   fileserver.js
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

var fs = require('fs');

var path = require('path');

var utility = require('./utility.js');

function FileServer(config)
{
    this.moduleName = "FileServer";
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
// Returns true if the request is handled, false if not.
//
FileServer.prototype.processFileServer = function (req, res) {

    //
    // No pre-configured files right now. Driven by webserver.js
    // and other modules invoking serveFile or serveDirectoryFile
    // as required.
    //

    return false;
}

//
// Static file serving support
//

//
// Serve a basic file.
//
// content type is set based on the files extension.
//
// If the extension is not registered, the call fails.
//
// Caller is responsible for validating credentials before invoking
// this routine.
//
FileServer.prototype.serveFile = function (req, res, filePath) {

    var self = this;

    if (req.method != "GET") {
        utility.sendErrorAsJSON(req, res, 400, "not GET request");
        return;
    }

    var extname = path.extname(filePath);

    var contentType = utility.extensionToMimeType(extname);
    if (contentType == null) {
        utility.sendErrorAsJSON(req, res, 404, "no file extension");
        return;
    }
    
    fs.exists(filePath, function(exists) {
	
	if (exists) {
            var s = fs.createReadStream(filePath);
            s.on('error', function() {
                utility.sendErrorAsJSON(req, res, 404, "servFile: file read stream error for " + filePath);
            })

            self.logger.info("servFile: sending file " + filePath + " contentType=" + contentType);

	    res.writeHead(200, { 'Content-Type': contentType });

            s.pipe(res);
	}
	else {
            utility.sendErrorAsJSON(req, res, 404, "servFile: file not found path= + filePath");
	}
    });
}

//
// Serves a JSON listing of a directories contents.
//
// filePath - path to directory
//
// uriPath - root for URI to refer back to files in the directory
//
// Caller is responsible for validating credentials before invoking
// this routine.
//
FileServer.prototype.serveDirectoryFile = function(req, res, filePath, uriPath) {

    var self = this;

    var remoteAddr = req.socket.remoteAddress;

    //
    // http://nodejs.org/api/fs.html#fs_fs_readdir_path_callback
    //
    // fs.readdir(path, callback);
    // fs.readdirSync(path);
    //
    fs.readdir(filePath, function(error, files) {

         if (error != null) {
             var er = remoteAddr + " attempt to enumerate directory failed path=" + 
                      filePath + " error=" + error;
             self.logger.error(er);
             utility.sendErrorAsJSON(req, res, 404, "error reading directory");
             return;
         }

         // Add a log entry that the directory was enumerated
         self.logger.info(remoteAddr + " enumerated directory path=" + filePath);

         var filesIndex = 0;     // index into files[] array in readdir() callback
         var responseArray = []; // file information response entries

         //
         // This is invoked to stat a file from the files[] array.
         //
         var statEntry = function() {

             if (filesIndex >= files.length) {
                 // done
                 sendResponse(filePath, responseArray);
                 return;
             }

             var entry = files[filesIndex];

             // Path is the local file system path for stat, not the remote path
             var path;

             if (filePath[filePath.length-1] != '/') {
                 path = filePath + "/" + entry;
             }
             else {
                 path = filePath + entry;
             }

             fs.stat(path, function(error, stats) {
                 statEntryCallback(error, stats, entry);
             });
         }

         //
         // This is executed when we get a callback for a stat
         // entry.
         //
         // It places the information into the next array entry
         // and continues if not finished with all files[].
         //
         var statEntryCallback = function(error, stats, fileName) {

             if (error) {
                 // Skip and process next entry
                 filesIndex++;
                 statEntry();
                 return;
             }

             var fileType = null;

             if (stats.isFile()) {
                 fileType = "file";
             }
             else if (stats.isDirectory()) {
                 fileType = "directory";
             }

             if (fileType == null) {
                 // skip unknown file types
                 filesIndex++;
                 statEntry();
                 return;
             }

             //
             // construct the URL reference back to our server
             //
             var href = uriPath + "/" + fileName;

             var fileInfo = {};

             fileInfo.name = fileName;
             fileInfo.uri = href;

             fileInfo.type = fileType;
             fileInfo.size = stats.size;

             fileInfo.atime = stats.atime;
             fileInfo.mtime = stats.mtime;
             fileInfo.ctime = stats.ctime;

             fileInfo.birthtime = stats.birthtime;

             responseArray.push(fileInfo);

             // Start the next one
             filesIndex++;
             statEntry();
         }

         // Start off the state machine
         statEntry();
    });

    var sendResponse = function(path, responseFiles) {

         // Convert to JSON and respond
         var response = {};
         response.path = path;
         response.status = 200;
         response.files = responseFiles;

         var json = JSON.stringify(response);

         res.writeHead(200, { 'Content-Type': "application/json" });

         res.end(json);
    }

    return;
}

module.exports = {
  Server: FileServer
};
