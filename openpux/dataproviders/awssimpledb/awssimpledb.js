
//
// TODO: Make queries general, not specific to openpux schema
//
//   awssimpledb.js
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
// Amazon AWS SimpleDB storage support
//
// This interfaces the openpux schema to the specific requirements
// of AWS SimpleDB.
//
// https://aws.amazon.com/sdk-for-node-js/
//
// npm install aws-sdk
//
// Note: You must setup your AWS SimpleDB authentication to the SimpleDB
// cloud service and place the authentication file somewhere in your setups
// root, its parent, or a configured directory as per the AWS instructions.
//
// Provision openpux data domain with:
//
//   node dataproviders/awssimpledb/util/simpledb_util.js createdomain
//
//   util/bin/provision.sh
//
// Verify openpux data domain with:
//
//   dataproviders/awssimpledb/util/queryall.sh
//
//   node dataproviders/awssimpledb/util/simpledb_util.js query all
//
// General Utility:
//
//   node dataproviders/awssimpledb/util/simpledb_util.js help
//
// Delete openpux domain *** AND ALL DATA *** with:
//
//   node dataproviders/awssimpledb/util/simpledb_util.js dEleteDomaiN
//
// Regenerate openpux domain deleting *** ALL DATA ***
//
//   dataproviders/awssimpledb/util/regenerate.sh
//
//   util/bin/provision.sh
//

//
// Schema:
//
// ItemName to AWS SimpleDB uniquely identifies an item.
//
// The fastest, lowest overhead lookup/retrieval is through ItemName
// as its a direct access.
//
// All other lookups are based on property values and performed with
// SQL query syntax. These involve searching through records for matches
// which consumes more server resources.
//
// A schema design that confines frequent access lookups to ItemName will
// minimize SimpleDB service resources which are billed to the account.
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
// AWS SimpleDB has restrictions on what characters can be inside
// an ItemName, an Attribute Name, or attribute data. To ensure
// portability with other storage mechanisms, the following restrictions
// are applied, even though AWS SimpleDB has more flexibility:
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
// Caching is handled by the storage.js layer. All requests here
// route out to HTTP REST requests against the AWS SimpleDB storage service.
//

// Load Amazon AWS SimpleDB handler
var simpledbFactory = require('./simpledb_module.js');

function SimpleDB_Store(config)
{

    this.moduleName = "SimpleDBStore";
    this.config = config;

    this.domain = "openpux";

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

    // Configure and load AWS SimpleDB module
    var simpledb_config = {
        Trace: this.trace,
        TraceError: this.traceerrorValue,

        // AWS Region
        region: 'us-west-1',

        logger: this.logger
        };

    this.simpledb = new simpledbFactory.SimpleDB(simpledb_config);

    //
    // Administrator token/password is passed in by the caller.
    //
    this.administratorToken = this.config.AdministratorToken;

    //
    // Dynamic deployments configure for automatic account
    // and per account sensor entry creation.
    //
    // Public deployments restrict these operations to manage
    // server and storage resources to known users.
    //
    this.autoCreateAccounts = this.config.AutoCreateAccounts;
    this.autoCreateSensors = this.config.AutoCreateSensorsByAccount;
}

SimpleDB_Store.prototype.getName = function() {
    return "Amazon AWS SimpleDB";
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
SimpleDB_Store.prototype.createItem = function(itemName, itemsArray, callback) {

    var requestArray = this.cloneObjectNoSchemaItems(itemsArray);
    if (requestArray == null) {
        callback("No attributes with __ prefix allowed (schema reserved)", null);
        return;
    }

    // Current openpux schema relies on __ItemName for searches
    requestArray.__ItemName = itemName;

    // Account is a child of accounts to allow search for all accounts
    requestArray.__Parent = getParentName(itemName);

    this.simpledb.createItem(this.domain, itemName, requestArray, callback);
}

//
// readItem
//
// itemName - name to read
//
// callback(error, result)
//
SimpleDB_Store.prototype.readItem = function(itemName, consistentRead, callback) {

    var self = this;

    self.simpledb.readItem(self.domain, itemName, consistentRead, function(error, result) {

        if (error != null) {
            callback(error, result);
            return;
        }

        // Filter out internal schema items
        var data = self.cloneObjectSkipSchemaItems(result);

        callback(error, data);
    });
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
SimpleDB_Store.prototype.updateItem = function(itemName, itemsArray, callback) {

    var requestArray = this.cloneObjectNoSchemaItems(itemsArray);
    if (requestArray == null) {
        callback("No attributes with __ prefix allowed (schema reserved)", null);
        return;
    }

    this.simpledb.updateItem(this.domain, itemName, requestArray, callback);
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
SimpleDB_Store.prototype.queryItems = function(querystring, consistentRead, callback) {

    var self = this;

    self.simpledb.queryItems(querystring, consistentRead, "", function(error, result) {

        if (error != null) {
            self.tracelog("queryItems: error=" + error);
            self.tracelog("querystring=" + querystring);
            callback(error, result);
            return;
        }

        //
        // result is an array of {itemName: "name", item: {...}}
        //

        // Filter out internal schema items
        var data = self.processNameValueArraySkipSchemaItems(result);

        callback(error, data);
    });
}

//
// Query child objects for the given "parent" path
//
SimpleDB_Store.prototype.queryChildren = function(parentPath, readingCount, callback) {

    var queryString = this.buildSelectChildrenStatement(
        this.domain,
        parentPath,
        readingCount
        );

    this.queryItems(queryString, true, callback);
}

//
// deleteItem
//
// itemName - name to update
//
// callback(error, result)
//
SimpleDB_Store.prototype.deleteItem = function(itemName, callback) {
    this.simpledb.deleteItem(this.domain, itemName, callback);
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
SimpleDB_Store.prototype.cloneObjectNoSchemaItems = function(a) {

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
SimpleDB_Store.prototype.cloneObjectSkipSchemaItems = function(a) {

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

SimpleDB_Store.prototype.cloneArraySkipSchemaItems = function(a) {
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
SimpleDB_Store.prototype.processNameValueArraySkipSchemaItems = function(a) {

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
// TODO: Make this not openpux specific!
//
// Select last itemcount items
//
// domain - SimpleDB domain to query
//
// sensorid - fully qualified sensor name such as "Account_1.Sensor_1"
//
// timestamp - timestamp to begin search by
//
SimpleDB_Store.prototype.buildSelectLastReadingsStatement = 
    function(sensorid, timestamp, itemcount) {

    var parent = sensorid + "/readings/";

    var select = "select * from ";
    select += this.domain;
    select += " where";

    select += " (__Parent = '" + parent + "')";
    select += " and (TimeStamp > '" + timestamp + "')";

    select += " order by TimeStamp desc limit " + itemcount;

    return select;
}

//
// Select the Immediate children of the parent path
//
// domain - SimpleDB domain to query
//
// parent - parent path such as /parent/sensor
//
// timestamp - timestamp to begin search by
//
SimpleDB_Store.prototype.buildSelectChildrenStatement =
    function(domain, parent, itemcount) {

    var select = "select * from ";
    select += domain;
    select += " where";

    select += " (__Parent = '" + parent + "')";
    select += " limit " + itemcount;

    return select;
}

//
// Select the all descendents of the parent path
//
// domain - SimpleDB domain to query
//
// parent - parent path such as /parent/sensor
//
// timestamp - timestamp to begin search by
//
SimpleDB_Store.prototype.buildSelectChildrenAndDescendantsStatement =
    function(domain, parent, itemcount) {

    var select = "select * from ";
    select += domain;
    select += " where";

    select += " (__Parent = '" + parent + "*')";
    select += " limit " + itemcount;

    return select;
}

//
// Template driven query builder.
//
// The purpose of the template driven query builder is to:
//
// 1) Isolate different storage backends and query/access syntax
//
// 2) Prevent SQL Injection attacks by not accepting queries
//    directly from remote clients.
//
//  compareArray is an array of logical constructs
//  consisting of the following used to build a combined where clause:
//
//  {
//    Name: "attributeName",
//    Value: "attributeValue",
//    Compare: "<"
//    Junction: "and"
//  }
//
// orderby if supplied
//
//  {
//   Name: "attributeName",
//   Desc: true,   // true if descending, false otherwise
//   Limit: "10"   // limit number.
//  }
//
//  Name can not have an characters not a-zA-Z0-9 and no spaces,
//  or other punctuation. This is to prevent SQL injection attacks.
//
//  Value can be any allowed character in the schema since its quoted in
//  the SQL statements.
//
//  Boolean must be one of "<", ">", "=", "<=", ">="
//
//  Junction must be one of "and", "or"
//
//  calback(error, querystring)
//
SimpleDB_Store.prototype.buildQuery = function(compareArray, orderby, callback) {

    var domain = this.domain;

    var select = "select * from ";
    select += this.domain;
    select += " where ";

    for (var index = 0; index < compareArray.length; index++) {
        var e = compareArray[index];

        if (!validateAttributeName(e.Name)) {
            callback("InvalidAttributeName", null);
            return;
        }
        
        if (!validateAttributeValue(e.Value)) {
            callback("InvalidAttributeValue", null);
            return;
        }

        if (!validateCompare(e.Compare)) {
            callback("InvalidCompareOperator", null);
            return;
        }
        
        if (!validateJunction(e.Junction)) {
            callback("InvalidJunctionOperator", null);
            return;
        }

        // TODO: An array of entries within can mean compound
        // statements. (foo >= "value") and ((foo == "bar") or (foo == "bat"))

        select += "(";

        select += e.Name;

        select += " ";
        select += e.Compare;
        select += "\"";
        select += e.Value;
        select += "\"";
        select += ") ";

        if (e.Junction != null) {
            select += "";
            select += e.junction;
            select += "";
        }
    }

    if (orderby == null) {
        // done;
        callback(null, select);
        return;
    }

    //
    // build optional order by
    //
    select += "order by ";
    
    if (!validateAttributeName(orderby.Name)) {
        callback("InvalidOrderByAttributeName", null);
        return;
    }

    select += orderby.Name;
    select += " ";

    if (orderby.Desc) {
        select += "desc ";
    }

    if (orderby.Limit != null) {

        // May not be a number, but at leasts its not sql injection.
        if (!validateAttributeName(orderby.Limit)) {
            callback("InvalidLimitValue", null);
            return;
        }

        select += "limit ";
        select += orderby.Limit;
    }
}

//
// Validate junction operatiors for query
//
SimpleDB_Store.prototype.validateJunction = function(j) {

    // null is valid for junction as its optional
    if (j == null) return true;

    if (j == "and") return true;
    if (j == "or") return true;

    return false;
}

//
// Validate boolean operations for query
//
SimpleDB_Store.prototype.validateCompare = function(b) {

    if (b == "<") return true;
    if (b == ">") return true;
    if (b == "<=") return true;
    if (b == ">=") return true;
    if (b == "=") return true;

    return false;
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

SimpleDB_Store.prototype.validateAttributeName = function(n) {

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

SimpleDB_Store.prototype.validateAttributeValue = function(n) {

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

SimpleDB_Store.prototype.setTrace = function(value) {
    this.trace = value;
}

SimpleDB_Store.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

SimpleDB_Store.prototype.tracelog = function(message) {
    if (this.trace) {
        this.logger.info(this.moduleName + ": " + message);
    }
}

SimpleDB_Store.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        this.logger.error(this.moduleName + ": " + message);
    }
}

SimpleDB_Store.prototype.dumpasjson = function(ob) {

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
  BackingStore: SimpleDB_Store
};
