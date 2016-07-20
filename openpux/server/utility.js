
//
// 10/23/2015
// TODO: Use a more specific search for paths using regex
//
//   utility.js
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

var util = require('util');

var path = require('path');

var mimeTypeConfiguration = [
  { extension: ".js",    mime_type: "application/javascript" },
  { extension: ".json",  mime_type: "application/json" },
  { extension: ".css",   mime_type: "text/css" },
  { extension: ".html",  mime_type: "text/html" },
  { extension: ".htm",   mime_type: "text/html" },
  { extension: ".text",  mime_type: "text/plain" },
  { extension: ".gif",   mime_type: "image/gif" },
  { extension: ".png",   mime_type: "image/png" },
  { extension: ".jpg",   mime_type: "image/jpeg" },
  { extension: ".log",   mime_type: "application/json" }
];

function extensionToMimeType (ext) {

    for (var prop in mimeTypeConfiguration) {
        if (ext == mimeTypeConfiguration[prop].extension) {
            return mimeTypeConfiguration[prop].mime_type;
        }
    }

    return null;
}

function mimeTypeToExtension (mimeType) {

    for (var prop in mimeTypeConfiguaration) {
        if (mimeType == mimeTypeConfiguration[prop].mime_type) {
            return mimeTypeConfiguration[prop].extension;
        }
    }

    return null;
}

//
// Validate string just uses simple characters and does not contain
// programming strings.
//
// Name must consist of:
//
// a-z, A-Z, 0-9, _
//
function isSimpleName(name) {

    // TODO: Use a more specific search for paths using regex

    // Currently use the file path validator for file specs
    if (!validateSimplePath(name)) {
        return false;
    }

    // no '\.' allowed in name
    if (name.search(/\./g) != (-1)) {
        // Found  '.' in string
        return false;
    }

    return true;
}

//
// Don't allow '/' or '..' in path
//
function validateSimpleFilePath(filePath) {

    //
    // Only simple paths such as file, file.html are allowed, not subdirectories such
    // as file/child.html
    //

    // no '/' allowed in path
    // path.sep (unix)
    if (filePath.search(/\//g) != (-1)) {
        // Found another '/' in string
        return false;
    }

    // no ".." allowed in path
    if (filePath.search(/\.\./g) != (-1)) {
        // Found ".." in string
        return false;
    }

    // no '\\' allowed in path
    // path.sep (windows)
    if (filePath.search(/\\/g) != (-1)) {
        // Found  \ in string
        return false;
    }

    // no '\:' allowed in path
    // path.delimiter
    if (filePath.search(/\:/g) != (-1)) {
        // Found  : in string
        return false;
    }

    return true;
}

//
// Process a JSON document to an object
//
function processJSONDocument(logger, document) {

    try {
        var jsonObject = JSON.parse(document);
        return jsonObject;
        callback(null, jsonObject);
    } catch (e) {
        var error = "exception processing received JSON document";
        error += " e=" + e.toString();
        //error += " e.stack=" + e.stack.toString();
        error += " json=" + document;
        logger.error(error);
        return null;
    }
}

//
// (Expect to) Receive a JSON document that will be converted
// to an object.
//
// logger - allows logging of errors. Important for debugging applications
//          and looking for attacks.
//
// callback(error, data)
//
function receiveJSONObject(req, res, logger, callback) {

    var self = this;

    //
    // Can include encoding type such as:
    //
    // "application/json;charset=UTF-8"
    //
    if (req.headers["content-type"].search("application/json") != 0) {
        callback("content-type not application/json", null);
        return;
    }

    var data = '';

    // Set encoding to utf8, otherwise default is binary
    req.setEncoding('utf8');

    // TODO: Is there a receive limit to keep for DOS'ing the server?
    req.on('data', function(chunk) {
        data += chunk;
    });

    req.on('end', function() {

        // Now try to parse it
        try {
            var jsonObject = JSON.parse(data);
            callback(null, jsonObject);
        } catch (e) {
            var error = "exception processing received JSON document";
            error += " e=" + e.toString();
            //error += " e.stack=" + e.stack.toString();
            error += " json=" + data;
            logger.error(error);
            callback("invalid JSON document received", null);
        }
    });
}

//
// Return result in JSON block
//
function sendResultAsJSON (req, res, resultObject) {
    var jsonString = JSON.stringify(resultObject);
    res.writeHead(resultObject.status, {'Content-Type': 'application/json'});
    res.write(jsonString);
    res.end();
}

//
// Return error in JSON block
//
function sendErrorAsJSON (req, res, status, error) {
    var result = new Object();
    result.status = status;
    result.error = error;
    this.sendResultAsJSON(req, res, result);
}

//
// Send error as plain text
//
// Used when a request URL is not recognized as to what type of response
// should be sent.
//
function sendErrorAsPlainText (req, res, errorCode, errorMessage) {
  res.writeHead(errorCode, {'Content-Type': 'text/plain'});
  res.write(errorMessage, 'utf8');
  res.end();
}


//
// Return error in JSON block after logging entry
//
function logSendErrorAsJSON (logger, req, res, status, error) {

    var remoteAddr = req.socket.remoteAddress;
    logger.error(remoteAddr + " error=" + error + " status=" + status);

    var result = {};
    result.status = status;
    result.error = error;
    this.sendResultAsJSON(req, res, result);
}

function logSendError (logger, req, res, error, document) {
    var remoteAddr = req.socket.remoteAddress;
    logger.error(remoteAddr + " error=" + error);

    res.writeHead(400, {'Content-Type': 'application/json'});            
    res.write(document);
    res.end();
}

//
// Perform a shallow (single level) copy of properties
// from object "from" to object  "to".
//
// No methods are copied.
//
function shallowCopyObject(from, to) {
    for (var prop in from) {
        to[prop] = from[prop];
    }
}

// dumpobjectasjson
// dumpobjasjson
function dumpasjson(ob) {

      //
      // Dump data as JSON
      // null is full depth, default is 2
      // var inspectOptions = { showHidden: true, depth: null };
      //
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      console.log(dumpdata);
}

// dumpobjectasjson
// dumpobjasjson
function dumpasjsonToConsole(ob) {

      //
      // Dump data as JSON
      // null is full depth, default is 2
      // var inspectOptions = { showHidden: true, depth: null };
      //
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      console.log(dumpdata);
}

//
// Return an array of the immediate children of the parentName
// from the names array.
//
// Only one instance of any given child name is returned.
//
// parentName = "/parent" returns for the following names:
//
// input names[]:
//
// [
//    "/parent",
//    "/parent/",
//    "/parent/1",
//    "/parent/1/2",
//    "/parent/child",
//    "/parent/child/",
//    "/parent/child/grandchild",
//    "/parent/child/grandchild/",
//    "/parent/child/grandchild/greatgrandchild",
//    "/parent/child/grandchild/greatgrandchild/",
//    "/parent/child1",
//    "/parent/child1/"
// ];
//
// returned childNames[]:
//
// [
//    "/parent/1",
//    "/parent/child",
//    "/parent/child1"
// ];
//
function enumerateChildNames(parentName, names) {

    //
    // In order to return just one instance of a name an
    // object is used as a string indexed array/hashtable
    //
    var o = {};

    for (var index = 0; index < names.length; index++) {
        var child = getImmediateChildName(parentName, names[index]);
        if (child != null) {
            o[child] = true; // a boolean is lowest possible storage
        }
    }

    // Now convert to the property names
    var childNames = Object.keys(o);

    return childNames;
}

//
// Return the immediate child of parent.
//
// Example:
//
// parentName = "/parent" returns for the following paths:
//
//   "/parent"           - null
//   "/parent/"          - null
//   "/parent/1",        - /parent/1
//   "/parent/1/2",      - /parent/1
//   "/parent/child",    - /parent/child
//   "/parent/child/",   - /parent/child
//   "/parent/child/grandchild" -  /parent/child
//   "/parent/child/grandchild/greatgrandchild" - /parent/child
//
function getImmediateChildName(parentName, childName) {

    //
    // Ensure all parent paths end in '/'
    //
    if (parentName[parentName.length - 1] != '/') {
        parentName = parentName + "/";
    }

    //
    // A child must be a least one character longer than
    // /parent/, such as /parent/1
    //
    if (childName.length <= parentName.length) return null;

    if (childName.search(parentName) != 0) {
        // child is not a prefix of the parent
        return null;
    }

    var index1 = parentName.length;

    var index2 = childName.indexOf("/", index1);
    if (index2 == (-1)) {
        // The name already is in child form with no further '/'s
        return childName;
    }

    var newChildName = childName.substring(0, index2);

    return newChildName;
}

//
// Return the immediate child of parent if its a direct object
// and not just part of a path to a grandchild.
//
// Example:
//
// parentName = "/parent" returns for the following paths:
//
//   "/parent"           - null
//   "/parent/"          - null
//   "/parent/1",        - /parent/1
//   "/parent/1/2",      - null
//   "/parent/child",    - /parent/child
//   "/parent/child/",   - null
//   "/parent/child/grandchild" -  null
//   "/parent/child/grandchild/greatgrandchild" - null
//
function getImmediateChildObject(parentName, childName) {

    //
    // Ensure all parent paths end in '/'
    //
    if (parentName[parentName.length - 1] != '/') {
        parentName = parentName + "/";
    }

    //
    // A child must be a least one character longer than
    // /parent/, such as /parent/1
    //
    if (childName.length <= parentName.length) return null;

    if (childName.search(parentName) != 0) {
        // child is not a prefix of the parent
        return null;
    }

    var index1 = parentName.length;

    var index2 = childName.indexOf("/", index1);
    if (index2 == (-1)) {
        // The name already is in child form with no further '/'s
        return childName;
    }

    // The path points to a decendent of child, not the immediate child
    return null;
}

module.exports = {
  receiveJSONObject:       receiveJSONObject,
  processJSONDocument:     processJSONDocument,
  sendResultAsJSON:        sendResultAsJSON,
  sendErrorAsJSON:         sendErrorAsJSON,
  extensionToMimeType:     extensionToMimeType,
  validateSimpleFilePath:  validateSimpleFilePath,
  logSendError:            logSendError,
  logSendErrorAsJSON:      logSendErrorAsJSON,
  shallowCopyObject:       shallowCopyObject,
  sendErrorAsPlainText:    sendErrorAsPlainText,
  dumpasjsonToConsole:     dumpasjsonToConsole,
  dumpasjson:              dumpasjson,
  getImmediateChildName:   getImmediateChildName,
  getImmediateChildObject: getImmediateChildObject,
  enumerateChildNames:     enumerateChildNames
};
