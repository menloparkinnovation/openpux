
//
//   jsondbprovider.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014,2015,2016 Menlo Park Innovation LLC
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
// Local jsondb storage support.
//
// This uses the servers built in simple jsondb support and provides
// a data provider interface on top.
//

//
// Openpux Data Provider Api Contract
//
// Since Javascript does not support interfaces, the general Storage Provider
// API contract is provided here.
//
// Note: A new provider implementation would substitute its own name
// for SimpleDB used here. Though the names are private and don't
// need to be changed, or could use a uniform name across different
// providers.
//
// Constructor:
//
// function JsonDB_Store(config) {}
//
// // export:
// module.exports = {
//   BackingStore: JsonDB_Store
// };
//
// JsonDB_Store.prototype.getName = function()
//
// JsonDB_Store.prototype.logCreateItem = function(itemName, itemsArray, timeStamp, callback) {
//
// JsonDB_Store.prototype.logReadLatestItem = function(itemName, consistentRead, callback) {
//
// JsonDB_Store.prototype.logBuildSelectLastItemsStatement = 
//
// JsonDB_Store.prototype.createItem = function(itemName, itemsArray, callback) {
//
// JsonDB_Store.prototype.readItem = function(itemName, consistentRead, callback) {
//
// JsonDB_Store.prototype.updateItem = function(itemName, itemsArray, callback) {
//
// JsonDB_Store.prototype.queryItems = function(querystring, consistentRead, callback) {
//
// JsonDB_Store.prototype.queryChildren = function(parentPath, readingCount, callback) {
//
// JsonDB_Store.prototype.deleteItem = function(itemName, callback) {
//
// JsonDB_Store.prototype.buildSelectChildrenStatement =
//
// JsonDB_Store.prototype.buildSelectChildrenAndDescendantsStatement =
//
// JsonDB_Store.prototype.buildQuery = function(compareArray, orderby, callback) {
//
// JsonDB_Store.prototype.setTrace = function(value) {
//
// JsonDB_Store.prototype.setTraceError = function(value) {
//
// JsonDB_Store.prototype.tracelog = function(message) {
//
// JsonDB_Store.prototype.traceerror = function(message) {
//
// JsonDB_Store.prototype.dumpasjson = function(ob) {
//

//
// Schema:
//
// ItemName uniquely identifies an item similar to cloud oriented
// provider.s
//
// Since attributes can be used to represent structural relationships
// attribute names begining with "__" are reserved for internal schema usage.
// Callers trying to create an attribute with these names will get a failure.
//
// This is to clearly separate application object attributes from ones
// used by a given applications schema.
//
// Restrictions:
//
// Since data is stored as json, restrictions for allowed characters
// for names and string storage for json must be followed.
//
// In addition further restrictions are imposed to allow independence
// of the data provider for openpux in general.
//
// Encoding of characters outside of this set is the responsibility
// of the application/caller. Use something such as base64, etc.
//
// All Items allow characters a-z, A-Z, '0-9'
//
// Attribute names may only have the base characters above.
// Atributes used by the schema may include "__" prefix.
//
// ItemNames may also have '/', '.', "_'
//
// Attribute values may also have '/', '.', '=', ':'
//

//
// Describing Object Relationships:
//
// A way to describe object relationships is to use simple path
// names similar to file system path names and rest query
// strings. This syntax maps naturally to most REST based application
// models.
//
// An example path for an application such as openpux that has
// Accounts, Sensors, Settings, and Readings would be as follows:
// 
// /accounts/1
// /accounts/1/sensors/1
// /accounts/1/sensors/1/settings
// /accounts/1/sensors/1/readings/1001222000
// /accounts/1/sensors/1/readings/1001222001
//             ...
//
// To allow simple searches based on parent/child relationships
// a "__Parent" schema item can be used by implementations to allow
// searches based on a given parent.
//
// This allows efficient queries for frequent application domain operations
// such as:
//
//   Query All Accounts:
//     select * from domain where __Parent = "/accounts"
//     
//     returns ItemName's:
//     "/accounts/1"
//     "/accounts/2"
//         ...
//
//   Query All Sensors for Account 1
//     select * from domain where __Parent = "/accounts/1/sensors"
//
//     returns ItemName's:
//     "/accounts/1/sensors/1"
//     "/accounts/1/sensors/2"
//         ...
//
//   Query All Readings for Account 1, Sensor 2
//     select * from domain where __Parent = "/accounts/1/sensors/2/readings"
//
//     returns ItemName's:
//     "/accounts/1/sensors/2/readings/01044490440"
//     "/accounts/1/sensors/2/readings/01044490441"
//         ...
//
// Note: Directly accessing an item whose name is known
// remains fast since the application can always construct the
// direct path name such as "/accounts/1/sensors/2/readings/01044490441"
// and present it to getItem()/updateItem()/deleteItem().
//
// Query is only required when you want to "search for" items whose
// name is not known, or can be constructed by simple rules beforehand
// by the application.
//

// For util.inspect()
var util = require('util');


//
// Caching:
//
// Caching is handled by the storage.js layer above.
//

// Load the jsondb handler
var jsondb = require('../../server/jsondb.js');

function JsonDB_Store(config)
{

    this.moduleName = "JsonDBStore";
    this.config = config;

    this.trace = false;
    this.traceerrorValue = false;

    if (typeof(this.config.Trace) != "undefined") {
        this.trace = this.config.Trace;
    }

    if (typeof(this.config.TraceError) != "undefined") {
        this.traceerrorValue = this.config.TraceError;
    }

    // Inherit the logger from config
    this.logger = this.config.logger;

    // Configure and load JSON DB
    var jsondb_config = {
        Trace: this.trace,
        TraceError: this.traceerrorValue,
        ConsoleOutput: false,
        directory: "data_dir",
        instanceName: "jsondbprovider",
        explicitFileName: "./var/data/jsondb/localdatabase.jsondb",
        openExisting: true,
        createIfMissing: true
    };

    this.jsondb = new jsondb.Server(jsondb_config);

    //
    // Administrator token/password is passed in by the caller.
    //
    this.administratorToken = this.config.AdministratorToken;

    //
    // Open the database file
    //
    this.jsondb.open(function(error, result) {

        //
        // Since this executes after the constructor function
        // returns we throw an exception to the server causing
        // an exit.
        //
        if (error != null) {
            throw "error initializing jsondb __dirname=" + __dirname + " error=" + error;
        }

        return;
    });
}

JsonDB_Store.prototype.getName = function() {
    return "JsonDB Store";
}

//
// timeStamp - Date format is ISO 8601 which sorts in lexographical order
//
JsonDB_Store.prototype.logCreateItem = function(itemName, itemsArray, timeStamp, callback) {

    // Caller provided timestamp
    this.createItemWorker(itemName, itemsArray, timeStamp, callback);
}

//
// createItem
//
// itemName - must be unique.
//
// itemsArray - attributes to place on created item.
//
// callback(error, result)
//
JsonDB_Store.prototype.createItem = function(itemName, itemsArray, callback) {

    // Date format is ISO 8601
    var timeStamp = new Date().toISOString();

    this.createItemWorker(itemName, itemsArray, timeStamp, callback);
}

JsonDB_Store.prototype.createItemWorker = function(itemName, itemsArray, timeStamp, callback) {

    var requestArray = this.cloneObjectNoSchemaItems(itemsArray);
    if (requestArray == null) {
        callback("No attributes with __ prefix allowed (schema reserved)", null);
        return;
    }

    // Current openpux schema relies on __ItemName for searches
    requestArray.__ItemName = itemName;

    // Account is a child of accounts to allow search for all accounts
    requestArray.__Parent = getParentName(itemName);

    //
    // Each record type is the Item name
    // 
    // The record has a time stamp that records when created.
    //
    var record = {
        type: itemName,
        time: timeStamp,
        data: requestArray
    };

    this.jsondb.appendEntry(record, callback);
}

//
// logReadLatestItem
//
// Read the latest item of a time sequence.
//
// The itemName represents the resource URL, and all updates are written
// as a log style time sequence.
//
// This routine returns the latest item, and functions similar to readItem
// on a non-time sequence item.
//
// itemName - name to read
//
// callback(error, result)
//
JsonDB_Store.prototype.logReadLatestItem = function(itemName, consistentRead, callback) {
    this.readItem(itemName, consistentRead, callback);
}

//
// readItem
//
// itemName - name to read
//
// callback(error, result)
//
JsonDB_Store.prototype.readItem = function(itemName, consistentRead, callback) {

    var self = this;

    var le = this.jsondb.getLatestEntryIfCached(itemName);
    if (le != null) {

        // Filter out internal schema items
        var data = self.cloneObjectSkipSchemaItems(le.data);

        callback(null, data);
        return;
    }

    callback("ItemDoesNotExist", null);
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

//
// updateItem
//
// itemName - name to update
//
// itemsArray - attributes to update on item
//
// callback(error, result)
//
JsonDB_Store.prototype.updateItem = function(itemName, itemsArray, callback) {

    //
    // When an existing database is "mounted" it is scanned
    // for the latest version of the record so that the latest
    // entry has the complete record.
    //
    // When an update occurs, we take this latest version of the
    // record, update/add fields as required, and write the new
    // entry, which includes updating the cache of the latest
    // entry.
    //

    // Get the current items state
    var le = this.jsondb.getLatestEntryIfCached(itemName);
    if (le == null) {

        // Item has not yet been created
        callback("ItemDoesNotExist", null);
        return;
    }

    var requestArray = this.cloneObjectNoSchemaItems(itemsArray);
    if (requestArray == null) {
        callback("No attributes with __ prefix allowed (schema reserved)", null);
        return;
    }

    //
    // merge in the new items
    //
    // Note: This writes into the jsondb's latest item for this itemName,
    // but we overwrite it immediately below.
    //
    shallowCopyObject(requestArray, le.data);

    var timeStamp = new Date().toISOString();

    // Each record type is the Item name
    var record = {
        type: itemName,
        time: timeStamp,
        data: le.data
    };

    this.jsondb.appendEntry(record, callback);
}

//
// callback(error, namevaluearray)
//
// Returns:
//
// An object array of the form:
//
//    [
//      {
//        itemName: '/accounts/1/sensors/1/readings/2015-11-11T14:48:40.304Z',
//        item: 
//         { AccountID: '1',
//           SensorID: '1',
//           TargetMask0: '0',
//           TargetMask1: '0001',
//           TargetMask2: '0002',
//           TargetMask3: '0003',
//           SensorReading0: '0000',
//           SensorReading1: '0001',
//           SensorReading2: '0002',
//           SensorReading3: '3',
//           TimeStamp: '2015-11-11T14:48:40.304Z' 
//         }
//      },
//      { 
//        itemName: '/accounts/1/sensors/1/readings/2015-11-11T14:46:08.002Z',
//        item: 
//         { AccountID: '1',
//           SensorID: '1',
//           TargetMask0: '0',
//           TargetMask1: '0001',
//           TargetMask2: '0002',
//           TargetMask3: '0003',
//           SensorReading0: '0000',
//           SensorReading1: '0001',
//           SensorReading2: '0002',
//           SensorReading3: '3',
//           TimeStamp: '2015-11-11T14:46:08.002Z' 
//         }
//      }
//    ]
//
JsonDB_Store.prototype.queryItems = function(querystring, consistentRead, callback) {

    var items = null;

    // querystring is a JSON object with the in memory query parameters
    var query = JSON.parse(querystring);

    if (query.query == "enumerate_last") {

        var le = this.jsondb.getLatestEntryIfCached(query.itemName);
        if (le == null) {
            callback("ItemDoesNotExist", null);
            return;
        }

        // This query only ever returns one item
        var ar = [];
        var ob = {};

        ob.itemName = query.itemName;
        ob.item = le.data;
        ar.push(ob);

        callback(null, ar);
        return;
    }
    else {
        this.traceerror("queryItems: unrecognized query=" + query.query + " for in jsondbprovider");
        callback("ItemDoesNotExist", null);
        return;
    }
}

//
// Select last itemCount items
//
// itemName - itemName such as "/account/1/sensor/1/readings"
//
// timestamp - timestamp to begin search by
//
JsonDB_Store.prototype.buildSelectLastItemsStatement =
    function(itemName, timestamp, itemcount) {

    // To support in memory query, return a JSON object with the parameters.
    var query = { query: "enumerate_last", 
                  itemName: itemName,
                  timestamp: timestamp,
                  itemcount: itemcount };

    return JSON.stringify(query);
}

JsonDB_Store.prototype.logBuildSelectLastItemsStatement =
    function(itemName, timestamp, itemcount) {

        return this.buildSelectLastItemsStatement(itemName, timestamp, itemcount);
}

//
// Query child objects for the given "parent" path
//
JsonDB_Store.prototype.queryChildren = function(parentPath, readingCount, callback) {
    callback("NotImplemented", null);
}

//
// deleteItem
//
// itemName - name to update
//
// callback(error, result)
//
JsonDB_Store.prototype.deleteItem = function(itemName, callback) {
    this.jsondb.deleteType(itemName, callback);
}

// Add schema prefix
var g_schemaPrefix = "__";

// Returns true if its an internal schema item not returned to the caller
function isSchemaItem(item) {
    if (item.search(g_schemaPrefix) == 0) {
        return true;
    }
    else {
        return false;
    }
}

//
// Returns the parent of the path.
// 
// / -> /
// // -> /
// x/ -> x/
// /x -> /
// /accounts -> /
// /accounts/ -> /
// /accounts/xxx -> /accounts/
// /accounts/xxx/ -> /accounts/
// /accounts/xxx/sensors -> /accounts/xxx/
// /accounts/xxx/sensors/ -> /accounts/xxx/
// /accounts/xxx/sensors/xxx -> /accounts/xxx/sensors/
// /accounts/xxx/sensors/xxx/ -> /accounts/xxx/sensors/
// /accounts/xxx/sensors/xxx/readings -> /accounts/xxx/sensors/xxx/
// /accounts/xxx/sensors/xxx/readings/ -> /accounts/xxx/sensors/xxx/
// /accounts/xxx/sensors/xxx/readings/zzzz -> /accounts/xxx/sensors/xxx/readings/
// /accounts/xxx/sensors/xxx/readings/zzzz/ -> /accounts/xxx/sensors/xxx/readings/
//
function getParentName(path) {

    if (path == null) return null;

    var index = path.length;

    // No parent for single character path
    if (index == 1) return path;

    // If it ends with '/', start one before  
    if (path[index - 1] == '/') {
        index--;
    }

    for (; index > 0; index--) {
        if (path[index - 1] == '/') {
            return path.substring(0, index);
        }
    }

    return path;
}

//
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
JsonDB_Store.prototype.cloneObjectNoSchemaItems = function(a) {

    if (a == null) return null;

    var b = {};
    for (var prop in a) {
        if (isSchemaItem(prop)) {
            return null;
        }
        b[prop] = a[prop];
    }
    return b;
}

//
// Clone an object skipping schema items
//
// This handles the result set from query which is an object
// with fields and an array of object matching the query.
//
JsonDB_Store.prototype.cloneObjectSkipSchemaItems = function(a) {

    if (a == null) return null;

    if (util.isArray(a)) {
        return this.cloneArraySkipSchemaItems(a);
    }

    var b = {};

    for (var prop in a) {

        if (isSchemaItem(prop)) {
            continue;
        }

        if (util.isArray(a[prop])) {
            var array = a[prop];
            var newarray = [];

            // result set is an array of objects
            for (var index=0; index < array.length; index++) {
                newarray.push(this.cloneObjectSkipSchemaItems(array[index]));
            }

            b[prop] = newarray;
        }
        else {
            b[prop] = a[prop];
        }
    }

    return b;
}

JsonDB_Store.prototype.cloneArraySkipSchemaItems = function(a) {
    b = [];

    for (var index = 0; index < a.length; index++) {
        b.push(this.cloneObjectSkipSchemaItems(a[index]));
    }

    return b;
}

//
// Process a name, value array skipping schema items in the objects
// returned as value.
//
// Input: array of {itemName: "name", item: {...}}
//
// result is an array of {itemName: "name", item: {...}}
//
JsonDB_Store.prototype.processNameValueArraySkipSchemaItems = function(a) {

    if (a == null) return null;

    b = [];

    for (var index = 0; index < a.length; index++) {
        var nv = a[index];

        var newItem = this.cloneObjectSkipSchemaItems(nv.item);

        var after = { itemName: nv.itemName, item: newItem };

        b.push(after);
    }

    return b;
}

//
// Another strategy would be table driven. Is there
// a Javascript/node.js built in for validation?
//
// Note this relies on ASCII sequences which is the language
// used for implementation of the SQL queries. Anything outside
// the range is rejected and not supported in order to prevent
// UTF encoding from being used to launch SQL attacks.
//
var g_allowedAttributeNameCharsTable = [
    { lower: "a".charCodeAt(0), upper: "z".charCodeAt(0) },
    { lower: "A".charCodeAt(0), upper: "Z".charCodeAt(0) },
    { lower: "0".charCodeAt(0), upper: "9".charCodeAt(0) },

    // Allow schema items with __AttributeName
    { lower: "_".charCodeAt(0), upper: "_".charCodeAt(0) }
];

var g_allowedAttributeValueCharsTable = [
    // This includes a-z, A-Z, 0-9 and non control special chars
    { lower: " ".charCodeAt(0), upper: "~".charCodeAt(0) }
];

function isCharCodeInTable(table, charCode) {

    for (var index = 0; index < table.length; index++) {

        var entry = table[index];

        if ((charCode >= entry.lower) && (charCode <= entry.upper)) {
            return true;
        }
    }

    return false;
}

JsonDB_Store.prototype.validateAttributeName = function(n) {

    if (n == null) return false;

    // Use the table for allowed attribute names
    var table = g_allowedAttributeNameCharsTable;

    for (var index = 0; index < n.length; index++) {
        
        if (!isCharCodeInTable(table, n.charCodeAt(index))) {
            // Bad character
            return false;
        }
    }
    
    return false;
}

JsonDB_Store.prototype.validateAttributeValue = function(n) {

    if (n == null) return false;

    // Use the table for allowed attribute values
    var table = g_allowedAttributeValueCharsTable;

    for (var index = 0; index < n.length; index++) {
        
        if (!isCharCodeInTable(table, n.charCodeAt(index))) {
            // Bad character
            return false;
        }
    }
    
    return false;
}

JsonDB_Store.prototype.setTrace = function(value) {
    this.trace = value;
}

JsonDB_Store.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

JsonDB_Store.prototype.tracelog = function(message) {
    if (this.trace) {
        this.logger.info(this.moduleName + ": " + message);
    }
}

JsonDB_Store.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        this.logger.error(this.moduleName + ": " + message);
    }
}

JsonDB_Store.prototype.dumpasjson = function(ob) {

      //
      // Dump data as JSON
      // null is full depth, default is 2
      // var inspectOptions = { showHidden: true, depth: null };
      //
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      this.logger.info(dumpdata);
}

module.exports = {
  BackingStore: JsonDB_Store
};
