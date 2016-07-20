
//
//   jsondb.js
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
// jsondb is intended as a simple, append only, logging data store
// for local data storage.
//
// It can be readily used to keep local settings and data up to date
// with the cloud.
//
// It's designed to have low impact on fragile embedded file systems
// such as SD cards. It avoids block re-allocations, and with proper
// support can write to and reuse pre-allocated files.
//
// Since its append only, synchronization and re-allocation is not required.
//
// It's optimized for an embedded IoT gateway scenario in which a series of
// events/data readings are constantly appended to the data, but not
// often queried locally.
//
// Some number of records require the latest value to be available, such
// as sensor settings, and these are kept in a small in memory cache, and
// updated as more recent records are added.
//
// Datatypes may be mixed in the database at will, as long as the type
// name differs.
//
// Currently most I/O is sync, since its expected to be buffered. This solves
// ordering problems simply. It's expected that potentially lengthy operations
// such as getLatestEntry() on a re-opened file with contents occur at
// application initialization time so that at run time only the fast in memory
// cached operations occur.
//

//
// Overall file system model:
//
// A file is opened for append only, and all records are appended
// to the end. This is similar to a log structured file system.
//
// File space can be pre-allocated according to the application. This may
// be more robust than growing the file at runtime on fragile systems
// such as SD cards in embedded devices and gateways. Config options
// can tell it to open an existing file, rather than creating a new
// file (the default).
//
// An in memory cache makes the most recent update for a given type
// available for immediate access.
//
// Updates to types are written to the append log, and their
// most recently cached entry is updated. As a log is not kept in
// memory the footprint is optimized to be memory efficient for
// use on small embedded systems.
//
// Deletes of types are done by writing a delete log record of
// the type, and invalidating the cache.
//
// Other than getting the most recent entry of each type at
// startup, its expected that applications do not need to perform
// any file lookups or scans at runtime.
//
// It is expected that the application at runtime does not need to
// perform deep query's, scans, or data analysis while the log is
// running. Only a simple enumerate by type, or all types API is provided.
//
// It's expected that deeper analysis is done offline, or the
// data is imported into a tradtional relational, a noSQL, or
// analytical database instance.
//
// The design is optimized for "log shipping" data replication
// and updates to the cloud from small IoT end nodes and gateways.
//
// The simple, sequential enumerate, or enumerate by type API
// facilitates this. It's efficient to enumerate in the back ground
// when the goal is to update a remote cloud.
//

//
// Note: A higher level module manages sequential, or circular
// queues of database/log files. This module does not concern
// itself with such complications.
//
// Such a high level database/log file management can allow a
// set of pre-allocated files against a given pre-defined space/quota
// limit with file re-use so that the underlying embedded file system
// does not have to perform file allocation or grow operations
// keeping metadata operations to a minimum. This helps to prevent
// fragmentation, and reliability/wear leveling issues with
// embedded flash style file systems such as SD cards.
//

//
// File Format
//
// - All records are in json.
//
// - A type is always required.
//
// - Each record represents a self contained javascript object separated by a "\"
//
// - The other json fields can be any data the application requires.
//
// - Time/Date, unique ID's, etc. are the responsibility of the application, upper layer
//
// - The file ends with \n\n to allow file re-use without having to clear it.
//    - This is similar to HTTP transfers.
//
// - Only valid JSON characters may be used.
//
// { "type": "settings", ... }\n
// { "type": "readings", ... }\n
// { "type": "readings", "__deleted": true, ... }\n
// \n\n
//

//
// Typical Schemas
//
// { "type": "object", "name": "/accounts/1", "TimeStamp": "xxxxxx", "__Parent": "/accounts"}\n
//

//
// Files are in a sub-directory specified by the caller.
//
// They have the current date/time of the creation in their name along with
// any applications supplied name.
//
// The data/time allows file uniqueness, and determination of sequential
// order.
//

var fs = require('fs');

var path = require('path');

// For util.inspect()
var util = require('util');

var memoryCache = require('./memorycache.js');

// Constructor Function
function jsondb(config)
{
    this.moduleName = "jsondb";
    this.config = config;

    // schema prefix
    this.schemaPrefix = "__";

    this.trace = false;
    this.traceerrorValue = false;

    // This is the directory the file is in.
    this.dbDirectory = null;

    //
    // This is the name currently open for streaming the output.
    // Its also used for enumeration scans if requested.
    //
    this.dbFilePath = null;

    //
    // Instance name is used when file names are auto generated
    // with the time.
    //
    this.instanceName = "jsondb";

    //
    // explicitFileName is supplied when the caller wants to
    // control the file name, and not have an auto generated one.
    //
    this.explicitFileName = null;

    this.dbFileFD = -1;

    this.dbFileStream = null;

    this.consoleOutput = false;

    this.openExisting = false;

    if (this.config.directory == null) {
        throw "directory must be defined in config";
    }

    this.dbDirectory = this.config.directory;

    //
    // If openExisting is set, explicitFileName must also be
    // set since an autogenerated name would not find the correct
    // file the caller desires, esp. in regards to time being
    // used in the auto generated file name.
    //
    if (typeof(this.config.openExisting) != "undefined") {
        this.openExisting = this.config.openExisting;
    }

    //
    // TODO: This is not implemented fully yet. Must be used
    // to test file existance, etc.
    //
    if (typeof(this.config.createIfMissing) != "undefined") {
        this.createIfMissing = this.config.createIfMissing;
    }

    if (typeof(this.config.explicitFileName) != "undefined") {
        this.explicitFileName = this.config.explicitFileName;
    }

    // Allow caller to set instance/application name for database files
    if (typeof(this.config.instanceName) != "undefined") {
        this.instanceName = this.config.instanceName;
    }

    if (typeof(this.config.Trace) != "undefined") {
        this.trace = this.config.Trace;
    }

    if (typeof(this.config.TraceError) != "undefined") {
        this.traceerrorValue = this.config.TraceError;
    }

    if (typeof(this.config.ConsoleOutput) != "undefined") {
        this.consoleOutput = this.config.ConsoleOutput;
    }

    // Setup an exit handler
    var self = this;

    // This is the standard ^C handler
    process.once('SIGINT', function () {
      self.shutdown(function () {
        process.kill(process.pid, 'SIGINT');
      });
    });

    // SIGUSR2 is used by nodemon for recycle
    process.once('SIGUSR2', function () {
      self.shutdown(function () {
        process.kill(process.pid, 'SIGUSR2');
      });
    });

    //
    // The cache contains the most recent data for a given
    // record type.
    //
    // The application can control how many record types it
    // wants to create to maintain reasonable cache size.
    //
    // Cache tuning could be exposed through config to allow
    // a working set of cache entries to be used for the
    // "hot" set. The memoryCache supports this, but is
    // not currently exposed to keep most usage of jsondb
    // simple. Expected deployments have a reasonable
    // number of record types.
    //
    // Note: Only the most recent instance of a record type
    // is stored. This means one instance * unique_record_types.
    //

    // 0 means no limit
    this.maxCacheEntries = 0;

    //
    // 0 means no trimming.
    // If not 0, represents the number of entries purged per
    // time when maxCacheEntries are exceeded.
    //   
    this.cacheTrimCount = 0;

    this.ItemsTableCache = new memoryCache.MemoryCache(
        this.maxCacheEntries,
        this.cacheTrimCount
        );
}

//
// open an existing database or create a new one.
//
jsondb.prototype.open = function(callback) {
    this.openFileStream(callback);
}

//
// callback(error)
//
jsondb.prototype.shutdown = function(callback) {

    // Safe even if not open currently
    this.closeFileStream(callback);
}

//
// A checkpoint closes the current file based log (if open)
// and starts a new one.
//
// Done by an application on a periodic basis such as hourly,
// daily, weekly, etc.
//
// An application determines based on its data rate, file size,
// etc. when to checkpoint.
//
jsondb.prototype.checkPoint = function(callback) {

    if (this.dbFileStream != null) {
        // will automatically close the current one
        this.openFileStream(callback);
    }
    else {
        callback(null, null);
    }
}

//
// entry is a javascript object with the entry information
//
// callback(error)
//
jsondb.prototype.appendEntry = function(entry, callback) {

    if ((typeof(entry["type"]) == "undefined") || (entry["type"] == null)) {
        callback("type must be supplied");
        return;
    }

    this.updateCache(entry);

    var buffer = JSON.stringify(entry);

    //
    // This is async on the writeStream so we don't register
    // a callback.
    //
    this.dbFileStream.write(buffer + "\n");

    // Optional console output
    if (this.consoleOutput) {
        console.log(buffer);
    }

    callback(null);
}

//
// Delete all instances of the given type from the database.
//
// A log record is written with the __deleted attribute set to true.
//
// The entry is removed from the cache.
//
// callback(error)
//
jsondb.prototype.deleteType = function(record_type, callback) {

    // See if its cached. If its not cached, it does not exit.
    var result = this.ItemsTableCache.get(record_type);
    if (result == null) {
        callback("ItemNotFound");
        return;
    }

    // Remove from the cache
    this.uncacheEntry(record_type);

    // Created a __deleted record for the type
    var record = {
        type: record_type,
        __deleted: true
    };

    // Write record
    var buffer = JSON.stringify(record);

    //
    // This is async on the writeStream so we don't register
    // a callback.
    //
    this.dbFileStream.write(buffer + "\n");

    callback(null);
}

//
// If the record type is cached from previous gets, or caching
// request, the cache is invalidated.
//
// This allows an application that manages lots of different record
// types to manage cache growth.
//
jsondb.prototype.uncacheEntry = function(record_type)
{
    // look up entry in cache, invalidate if it exists.
    this.ItemsTableCache.remove(record_type);
}

//
// Place the entry into the cache.
//
// If the entry does not exist, it is created.
//
// If an entry already exists, it is overwritten.
//
// This results in the cache always having the latest value.
//
jsondb.prototype.updateCache = function(entry)
{
    var res = this.ItemsTableCache.set(entry.type, entry);
    return res;
}

//
// This returns the latest entry if its cached.
//
// Returns null if its not in the cache for immediate access.
//
// Non-blocking, so no callback required.
//
// Unlike getLatestEntry(), it will not initiate a search for
// the latest version of the record_type in the file.
//
jsondb.prototype.getLatestEntryIfCached = function(record_type)
{
    // Look in the cache. If present return it.
    var itemEntry = this.ItemsTableCache.get(record_type);

    // null if not found
    return itemEntry;
}

//
// This gets the latest entry.
//
// callback(error, itemEntry)
//
jsondb.prototype.getLatestEntry = function(record_type, callback)
{
    var self = this;

    // Look in the cache. If present return it.
    var itemEntry = self.getLatestEntryIfCached(record_type);
    if (itemEntry != null) {
        callback(null, itemEntry);
        return;
    }

    //
    // Current model always has the latestEntry in the cache.
    //
    // It was either started from a new file, with all appends
    // updating the cache, or it was an open of an existing file
    // and open time scanning updated the latest values of all
    // data types found in the cache.
    //

    callback("ItemNotFound", null);
}

//
// Perform a mount time scan through the file updating
// the cache with the latest version of each record type
// found.
//
// When done the cache has the latest version of each unique
// type in the database.
//
// callback(error)
//
jsondb.prototype.mountTimeScan = function(callback)
{
    var self = this;

    self.enumerateBegin("*", function(error, enumContext) {

        if (error != null) {
            self.traceerror("mountTimeScan: enumerateBegin error=" + error);
            callback(error);
            return;
        }

        var nextFunction = function(error2, entry) {

            if (error2 != null) {

                if (error2 == "ItemNotFound") {
                    // done
                    self.tracelog("mountTime done with enumeration, file mounted");
                    callback(null);
                    return;
                }

                self.tracelog("mountTime enumeration error=" + error2);
                callback(error2);
                return;
            }

            // Process the record
            if ((typeof(entry.__deleted) != "undefined") && entry.__deleted) {

                //
                // A deleted record has no cache entry.
                //
                // Note: Processing a later record may re-cache it with the
                // latest (re-created) value.
                //
                self.uncacheEntry(entry.type);
            }
            else {
                self.updateCache(entry);
            }

            // Continue enumeration
            self.enumerateGetNext(enumContext, nextFunction);
        };

        // kick it off
        self.enumerateGetNext(enumContext, nextFunction);
    });
}

//
// Begin an enumeration of the given record type.
//
// If record type == "*", all records are returned.
// 
//  All enumerates are returned in sequence from the
//  begining of the file.
// 
//  The returned enumeration object must be closed
//  when done, as it represents an open file stream
//  index into the jsondb file.
// 
jsondb.prototype.enumerateBegin = function(record_type, callback)
{
    var self = this;

    //
    // An enumeration context contains the currently open file stream,
    // I/O buffer, and line that is being completed from the stream.
    //
    var enumContext = {};

    enumContext.record_type = record_type;

    enumContext.fileFD = -1;
    enumContext.fileIndex = 0;
    enumContext.buffer = null; // this a string
    enumContext.bufferIndex = 0;
    enumContext.bufferSize = 0;
    enumContext.lineBuffer = null;
    enumContext.lineBufferIndex = 0;
    enumContext.ioBuffer = null; // This is node Buffer type
    enumContext.filePosition = 0;

    //
    // Open the file readonly, it must exist.
    //

    // https://nodejs.org/api/fs.html#fs_fs_open_path_flags_mode_callback
    fs.open(self.dbFilePath, "r", "0666", function(error, fd) {

        if (error != null) {
            self.tracelog("error opening stream error=" + error);
            callback(error, null);
            return;
        }

        enumContext.fileFD = fd;
        self.tracelog("opened enumContext");
        callback(null, enumContext);
    });
}

jsondb.prototype.enumerateGetNext = function(enumContext, callback)
{
    var self = this;

    // read records till record_type is found, or end

    var getNextEntry = function(error, line) {

            // No more entries
            if (line == null) {
                self.tracelog("enumerateGetNext: getline == null");
                callback("ItemNotFound", null);
                return;
            }

            //
            // \n\n ends all records, since a given file may be re-used and have
            // older data content underneath. This is indicated by a zero length line.
            //
            if (line.length == 0) {
                self.tracelog("zero length line, end of database marker found");
                callback("ItemNotFound", null);
                return;
            }

            //
            // Convert to object from json
            // Note: if an exception occurs due to a correct record we let it bubble up
            //
            var obj = JSON.parse(line);

            if ((enumContext.record_type == "*") || (enumContext.record_type == obj.type)) {
                if (self.consoleOutput) {
                    console.log("returning obj=");
                    console.log(obj);
                }

                callback(null, obj);
                return;
            }
    
            self.getLine(enumContext, getNextEntry);
        };

        // Kick it off
        self.getLine(enumContext, getNextEntry);
}

jsondb.prototype.enumerateEnd = function(enumContext, callback)
{
    var self = this;

    if (enumContext.fileFD == (-1)) {
        callback(null, null);
        return;
    }

    // Close the FD
    fs.close(enumContext.fileFD, function(error) {

        enumContext.fileFD = -1;

        enumContext.index = 0;
        enumContext.record_type = null;
        enumContext.ioBuffer = null;

        //
        // filePosition represents the current read position in the
        // file. It may be ahead of the characters read due to buffering
        // in ioBuffer.
        //
        // Character position is difficult to determine due to UTF-8/Unicode
        // expansion in which the filePosition/byte offset in file may be ahead
        // of the number of characters seen and processed.
        //
        // This has problems in trying to record file positions to determine
        // the start of any given record.
        //
        enumContext.filePosition = 0;

        if (error != null) {
            self.traceerror("error on close error=" + error);
        }

        callback(null, null);
    });
}

//
// Open a jsondb file stream
//
// Creates a file with the current time and data as its name.
//
// callback(error)
//
jsondb.prototype.openFileStream = function(callback) {

   var self = this;

   var msg = null;

   //
   // If a db file stream is currently open start
   // closing it. Note that this is async as we will
   // start the new stream in parallel.
   //
   if (self.dbFileStream != null) {
       self.dbFileStream();
   }

   try {

       var dbTime = new Date().toISOString();
       var dbPathTime = dbTime;
  
       // If its Win32/Windows we must eliminate the ":" characters.
       if (process.platform == "win32") {
           dbPathTime = dbPathTime.replace(/:/g, "_");
       }

       var fileName = null;

       if (self.explicitFileName != null) {
           fileName = self.explicitFileName;
       }
       else {
           var fileName = this.dbDirectory + "/" + self.instanceName;
           fileName += "_" + dbPathTime + ".jsondb";
       }

       var dbFilePath = fileName;

       //
       // Keep a captured copy of the FD since self.dbFileFD is
       // invalidated by closeFileStream() asynchronously.
       //

       //
       // We open the file in append mode
       //
       // https://nodejs.org/api/fs.html#fs_fs_open_path_flags_mode_callback
       //
       var captured_dbFileFD = null;

       var openMode = null;

       if (self.openExisting) {

           if (!self.explicitFileName) {
               msg = "open existing must supply file name";
               self.traceerror(msg);
               callback(msg);
               return;
           }

           //
           // Request is to open an existing file. Validate that it
           // does exist as "a" will create an empty file if it does
           // not, and the caller may have made a mistake in naming.
           //
           try {
               fs.accessSync(dbFilePath, fs.R_OK | fs.W_OK);
           } catch(e) {

               // File does not exist
               msg = "FileDoesNotExist and specified openExisting";
               this.traceerror(msg);
               callback(msg);
               return;
           }
           
           openMode = "a";
       }
       else {

           //
           // Request is for a new file. A scan does not have to be done
           // since all new data entries will be placed into the cache automatically.
           //
           // "ax" will cause the create for append to fail if the file already
           // exists.
           //
           openMode = "ax";
       }

       try {
           captured_dbFileFD = fs.openSync(dbFilePath, openMode, "0666");
       } catch(e) {
           // file does not exist or could not be created
           callback(e.toString());
           return;
       }

       // create the buffered stream we will use for I/O
       var dbFileStream =
           fs.createWriteStream(null, {flags: 'w', fd: captured_dbFileFD, mode: "0o666"});

       dbFileStream.on('end', function() {
           fs.closeSync(captured_dbFileFD);
       });

       dbFileStream.on('error', function(error) {
           console.error("error on db stream: error=" + error);
       });

       //
       // We write a start_record to identify recording of new data in a session.
       //
       var record =  { type: "start_record",
                       time: dbTime,
                       module: this.instanceName,
                       message: "db session started"
                     };

       this.updateCache(record);

       var buffer = JSON.stringify(record) + "\n";

       dbFileStream.write(buffer);

       if (this.consoleOutput) {
            console.log(buffer);
       }

       //
       // Now that an exception has not occured publish
       // the logging variables.
       //
       this.dbFileStream = dbFileStream;
       this.dbFileFD = captured_dbFileFD;
       this.dbFilePath = dbFilePath;

       //
       // If openExisting, perform a mount time scan to cache the latest
       // value of each record type.
       //
       // It will call the open completion callback when done.
       //
       if (this.openExisting) {
           this.mountTimeScan(callback);
           return;
       }

   } catch (e) {

       // Error setting up stream, report error and return.
       console.error("exception setting up file stream " + dbFilePath);
       console.error("e=" + e.toString());

       callback(e.toString());
       return;
   }

    callback(null);
}

//
// Close the database file stream
//
// callback(error)
//
jsondb.prototype.closeFileStream = function(callback) {

   if (this.dbFileStream == null) {
       if (callback != null) {
           callback(null);
       }
       return;
   }

   var dbTime = new Date().toISOString();

   var record =  { level: "stop_record",
                   time: dbTime,
                   module: this.instanceName,
                   message: "db session ended"
                 };

   this.updateCache(record);

   // Last record is terminated with a double "\n"
   var buffer = JSON.stringify(record) + "\n\n";

   //
   // Do this first in case the writeStream end fires before console finishes
   //
   if (this.consoleOutput) {
        console.log(buffer);
   }

   var captured_dbFileStream = this.dbFileStream;

   //
   // We null out the previous values now as they have been
   // captured by the 'end" event handler as lamba variables.
   //
   // This is to allow an immediate re-open of the log on a new
   // file stream. This is done perodically by applications
   // when their log interval (day, hour, week, etc.) occurs.
   //
   this.dbFileStream = null;
   this.dbFileFD = -1;

   //
   // Rundown the cache
   //
   this.ItemsTableCache = null;

   //
   // The will be closed by the 'end' event on the logFileStream when
   // all of the buffers have been written out.
   //

   // Note: .end is only available on sync streams
   // https://github.com/nodejs/node-v0.x-archive/blob/cfcb1de130867197cbc9c6012b7e84e08e53d032/lib/fs.js#L1597-L1620
   captured_dbFileStream.end(buffer, callback);
}

//
// getFileChunk - read a chunk from the file if buffer == null
//
// callback(error, buffer)
//
jsondb.prototype.getFileChunk = function(enumContext, callback)
{
    var self = this;

    //
    // enumContext.buffer represents a chunk of bytes being processed
    // from a file.
    //
    // Its set to null when fully consumed.
    //
    if (enumContext.buffer != null) {
        callback(null);
        return;
    }

    //
    // Attempt to get the next chunk
    //

    if (enumContext.ioBuffer == null) {
        // First time allocation of IoBuffer. Its re-used across I/O's.
        enumContext.ioBuffer = new Buffer(4096);
    }

    // https://nodejs.org/api/fs.html#fs_fs_read_fd_buffer_offset_length_position_callback
    fs.read(
        enumContext.fileFD,
        enumContext.ioBuffer,
        0,                           // Buffer Offset
        enumContext.ioBuffer.length, // Read Length
        enumContext.filePosition,
        function(error, bytesRead, buffer) {

            if (error != null) {
                self.traceerror("fs.read error=" + error);
                callback(error);
                return;
            }

            if (bytesRead == 0) {
                self.tracelog("bytesRead == 0 (EOF)");
                callback("EOF");
                return;
            }

            if (bytesRead == -1) {
                var err = "unexpected bytesRead == -1";
                self.traceerror(err);
                callback(err);
                return;
            }

            enumContext.filePosition += bytesRead;

            // Buffer returns octets (bytes) not characters
            enumContext.buffer = enumContext.ioBuffer;
            enumContext.bufferSize = bytesRead;
            enumContext.bufferIndex = 0;

            callback(null);
    });
}

//
// callback(error, c)
//
jsondb.prototype.getChar = function(enumContext, callback)
{
    var self = this;

    // Get chunk or existing buffer
    self.getFileChunk(enumContext, function(error) {

        if (error != null) {
            callback(error, null);
            return;
        }

        var c = enumContext.buffer[enumContext.bufferIndex];
        enumContext.bufferIndex++;
        enumContext.fileIndex++; // represents position within the file

        if (enumContext.bufferIndex >= enumContext.bufferSize) {
            // reset the buffer which indicates a need for a new chunk from the file
            enumContext.buffer = null;
            enumContext.bufferSize = 0;
        }

        callback(null, c);
    });
}

//
// callback(error, line)
//
jsondb.prototype.getLine = function(enumContext, callback)
{
    var self = this;

    if (enumContext.lineBuffer == null) {

        //
        // start new linebuffer as a dynamic array since we don't know
        // how many bytes until end of string is reached.
        //
        enumContext.lineBuffer = [];
        enumContext.lineBufferIndex = 0;
    }

    var nextCharFunction = function(error, c) {

        if (error == "EOF") {

            //
            // EOF
            // Note: last record should have terminated with '\n', so the line
            // is not complete. We leave any residue in lineBuffer though.
            //
            self.tracelog("getLine: EOF on file");
            callback("NoMoreEntries", null);
            return;
        }

        if (error != null) {
            self.traceerror("getLine: error on getChar error=" + error);
            callback(error, null);
            return;
        }

        // This compares against the byte char code for '\n'
        if (c == '\n'.charCodeAt(0)) {

            //
            // end of line, return it
            //

            // First must convert the array of bytes into a Buffer
            var b = new Buffer(enumContext.lineBuffer);

            // Then convert from bytes to a utf8 string
            var s = b.toString();

            enumContext.lineBuffer = null;
            enumContext.lineBufferIndex = 0;

            self.tracelog("getLine: end of line s=" + s);

            callback(null, s);
            return;
        }

        //console.log("getLine: got char " + c);

        enumContext.lineBuffer.push(c);
        enumContext.lineBufferIndex++;

        // Get next character in the state machine
        self.getChar(enumContext, nextCharFunction);
    };

    // Kick off the state machine
    self.getChar(enumContext, nextCharFunction);
}

//
// Perform a shallow, top level clone.
//
// Intended for simple objects consisting of a series
// of properties and values.
//
// References to objects are copied, not duplicated.
//
// Does not allow any schema items in object. Returns null on error.
//
jsondb.prototype.cloneObjectNoSchemaItems = function(a) {

    if (a == null) return null;

    var b = {};
    for (var prop in a) {
        if (this.isSchemaItem(prop)) {
            return null;
        }
        b[prop] = a[prop];
    }
    return b;
}

// Returns true if its an internal schema item not returned to the caller
jsondb.prototype.isSchemaItem = function(item) {
    if (item.search(this.schemaPrefix) == 0) {
        return true;
    }
    else {
        return false;
    }
}

jsondb.prototype.setTrace = function(value) {

    this.trace = value;
}

jsondb.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

jsondb.prototype.tracelog = function(message) {
    if (this.trace) {
        console.log(this.moduleName + ": " + message);
    }
}

jsondb.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        console.log(this.moduleName + ": " + message);
    }
}

module.exports = {
  Server: jsondb
};
