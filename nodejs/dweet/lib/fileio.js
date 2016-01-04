
//
// File I/O handling
//
// John Richardson
//
// 12/19/2014
//

//
// The FileIo module documents and encapsulates some of involved
// node.js file contracts. It can be used as a module, or
// as example code snippets.
//
// Note: The simple wrappers that also exist in this module provide
// programming examples more than value add. Useful for cut + paste
// or fast lookup on how to perform a given operation.
//

// http://nodejs.org/api/fs.html
var fs = require('fs');

function FileIo(trace, traceerrorValue) {

    this.moduleName = "FileIo";
    this.trace = false;
    this.traceerrorValue = false;

    this.filespec = '/';

    if (process.platform == "win32") {
        this.filespec = '\\';
    }

    if (typeof(trace) != "undefined") {
        this.trace = trace;
    }

    if (typeof(traceerrorValue) != "undefined") {
        this.traceerrorValue = traceerrorValue;
    }
}

//
// Ensures all the directory components in path up to, but
// not including the final path component exist.
//
// Creates any directories required to allow the last
// component to be created as a file or directory.
//
// Example:
//
//   /full/path/to/file.txt
//
//     Will create directories "full", "path", "to" if required.
//
//     Will *not* create "file.txt".
//
//   /file.txt
//
//     Will do nothing.
//
// Returns the full directory path without the ending file name.
//
FileIo.prototype.createPathToFileSync = function(filePath) {

    if (filePath[0] != this.filespec) {
        throw "can't handle relative paths " + filePath;
    }

    var components = filePath.split(this.filespec);

    // "foo"
    if (components.length == 0) {
        throw "can't handle relative paths " + filePath;
    }

    // "/foo"
    if (components.length == 1) {
        // nothing to do
        return this.filespec;
    }

    // "/foo/bar" or longer
    var endIndex = components.length - 1;

    var createPath = "";

    for (var index = 0; index < endIndex; index++) {

        // parse gives us an empty first entry
        if (components[index] == "") {
            continue;
        }

        createPath += this.filespec;
        createPath += components[index];

        try {
            this.mkdirSync(createPath);
        } catch(e) {

            if (e.toString().search("Error: EISDIR") == 0) {
                this.tracelog("Continuing EISDIR error");
                continue;
            }

            if (e.toString().search("Error: EEXIST") == 0) {
                this.tracelog("Continuing EEXIST error");
                continue;
            }

            this.traceerror("mkdir exception e=" + e.toString());

            // Rethrow if we don't handle it
            throw e;
        }
    }

    return createPath;
}

//
// Note: sync file operations are useful for a simple
// utility, but should not be used when processing
// web requests as you backup other processing.
//

//
// This returns the modified time in milliseconds since
//
FileIo.prototype.getFileModifiedTimeSync = function(filePath) {

    var stats = this.statSync(filePath);

    //
    // File times are instances of the Date() object
    //
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date
    //
    var mDate = new Date(stats.mtime);

    return mDate.getTime();
}

//
// Return if the file is a directory
//
FileIo.prototype.isDirectorySync = function(filePath) {
    var stats = this.statSync(filePath);
    return stats.isDirectory();
}

FileIo.prototype.isFileSync = function(filePath) {
    var stats = this.statSync(filePath);
    return stats.isFile();
}

FileIo.prototype.readFileToBufferSync = function(filePath) {
    var data = fs.readFileSync(filePath, {encoding: 'utf8'});
    return data;
}

FileIo.prototype.writeBufferToFileSync = function(filePath, buffer) {
    fs.writeFileSync(filePath, buffer);
}

// No return value, throws on error
FileIo.prototype.mkdirSync = function(filePath) {
    fs.mkdirSync(filePath);
}

// no return, throws on error
FileIo.prototype.deleteFileSync = function(filePath) {
    fs.unlink(path);
}

// http://nodejs.org/api/fs.html#fs_class_fs_stats
FileIo.prototype.statSync = function(filePath) {

    // Note: fs.lstat will return symbolic link info
    var stats = fs.statSync(filePath);
    return stats;
}

//
// Not supposed to call fs.exists() on file you are going
// to open/create since there are races in which it can be
// created or deleted. So you just perform the operation.
//
// But its still good for cases in which you must test
// the presence of a marker file.
//
FileIo.prototype.fileExistsSync = function(filePath) {
    return fs.existsSync(filePath);
}

// callback(exists) // boolean true if exists
FileIo.prototype.fileExists = function(filePath, callback) {
    fs.exits(filePath, callback);
}

// http://nodejs.org/api/fs.html#fs_class_fs_stats
// callback(err, stats)
FileIo.prototype.stat = function(filePath, callback) {

    // Note: fs.lstat will return symbolic link info
    fs.stat(filePath, callback);
}

FileIo.prototype.setTrace = function(value) {
    this.trace = value;
}

FileIo.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

FileIo.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

FileIo.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.error(this.moduleName + ": " + message);
    }
}

function createInstance(trace, traceerrorValue) {
    moduleInstance = new FileIo(trace, traceerrorValue);
    return moduleInstance;
}

module.exports = {
  createInstance: createInstance
};
