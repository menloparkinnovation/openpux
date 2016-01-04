
//
// TODO: Storage handles these:
//
//  Add property name, data validation.
//
//  Add accounting for data of itemName, property name, property data
//  bytes for charging/resource limiting.
//
//
// Add validation of item names.
// Add validation of property names.
// Add validation of property values.
//
// cleanup TODO:'s

//
//   Copyright (C) 2015 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
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
//   storage.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2015 Menlo Park Innovation LLC
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
//   This specific snapshot is made available under the terms of
//   the Apache License Version 2.0 of January 2004
//   http://www.apache.org/licenses/
//

//
// Storage support.
//
// Provides generic storage support for objects either in memory,
// or persistently through configurable data storage providers.
//
// Without a persistent storage provider it provides basic in memory storage
// for local/transient deployments.
//
// With a persistent storage provider it provides a simple caching
// layer as well as a generic interface to the storage provider.
//
// Persistent storage providers can provide backing can be the local file
// system, a service, gateway, proxy, or private/public cloud.
//

//
// Schema:
//
// The storage schema is designed to adapt well with cloud based
// "NoSQL" data stores such as Amazon SimpleDB, as well as local data
// stores such as MongoDB.
//
// Objects are simple "Plain Old Javascript Objects" (POJO) whose
// top level public properties are the attributes that are stored.
//
// The Javascript property name becomes the storage object's attribute,
// and the Javascript property value becomes the attribute value.
//
// The names of the attributes are restricted to character ranges
// compatible with a broad range of storage providers without translation.
//
// These ranges are: a-z, A-Z, 0-9, _
//
// The values are strings composed of the non-control ASCII character
// set in the ranges of " ".charCodeAt(0) to "~".charCodeAt(0) which
// include the characters allowed for attribute names above.
//
// Applications need to convert any numbers into strings for storage
// as its not done by this layer. This gives the application full control
// its representation and precision.
//
// All objects have a unique ItemName which consists of the characters
// a-z, A-Z, 0-9, and '/'. Names are typically constructed hierarchiically
// similar to file system pathnames or REST object path names. But this
// is up to the application as its free to use other method such
// as generation of sequential or random GUID's as many databases
// do.
//
// The ItemName is used as the fastest retrieval key from
// the data service provider. This is the key preferred by the server
// for most transactions due to its efficient retrieval from the storage
// service without a query or search being performed.
//

//
// Search Keys:
//
// Any attribute may be used for searching. Character string collating
// is used for order and search patterns in SQL select, order, etc.
// statements.
// 
// It is up to the application to appropriately pad any strings or
// numbers to make sequences operate properly, such as with numbers
// such as 10, 100, 1000, which should be represented as 0010, 0100,
// 1000 is 1000 is the highest sequence.
//
// Character string collating does not handle negative numbers, so
// numbers should be "rebased" to only positive values using a base
// to represent the most negative number allowed by the applications
// schema.
//
// The previous guidelines are not required on every attribute, only
// ones that are acting as primary/search/sort keys in the application
// and the application desires a logical sorting.
//
// Searching is implemented by an object based "query builder"
// API which provides abstraction from the query syntax of a particular
// data storage provider, and protection against SQL injection attacks
// by disallowing direct use of strings that can potentially contain
// SQL statements.
//

//
// Caching:
//
// If there is no configured backing store provider all data
// is kept in memory. The configuration sets any limits on
// the size of this data set. This is useful for ad-hoc deployments.
//
// If there is a configured backing store provider caching is
// performed for a valid account and sensor so that validation does
// not require a lookup for every request.
//
// If an entry is not in the cache, an attempt is made to acquire
// it from the data store.
//
// If the entry is not in the data store, a configurable attempt is
// made to create the entry.
//
// For ad-hoc IoT setups, automatic creation of accounts and sensors
// is handy for a "no maintenance" server.
//
// For a public access or account limited server automatic
// account creation is not desireable. Automatic sensor creation
// may be limited, or accounted for in total data storage limits/charges.
//
// Sensor readings are not cached if a backing store is present so
// the most recent data is provided to the backing store. As a
// result sensor reading queries always flow through to the
// backing store. This is to allow common deployments in which
// multiple openpux server processes are handling requests
// against a common backing store such as a cloud provider's
// data store, MongoDB, Redis, etc. Any caching and data synchronization
// policy is the responsibility of the data provider tier.
//
// If caching for readings is a performance requirement, a
// data provider for one of the many available data caching servers
// may be provided.
//

var memoryCache = require('./memorycache.js');

function Storage(config)
{
    this.moduleName = "Storage";
    this.config = config;

    this.trace = false;
    this.debug = false;
    this.traceerrorValue = false;

    if (typeof(this.config.Trace) != "undefined") {
        this.trace = this.config.Trace;
    }

    if (typeof(this.config.TraceError) != "undefined") {
        this.traceerrorValue = this.config.TraceError;
    }

    if (typeof(this.config.DebugTrace) != "undefined") {
        this.debug = this.config.DebugTrace;
    }

    // Setup the inherited logger from config
    this.logger = this.config.logger;

    //
    // Administrator token/password is passed in by the caller.
    //
    // config.AdministratorToken;
    //

    //
    // The ItemsTableCache is a general cache for all item
    // entries.
    //
    // If no backing store is configured, it becomes the primary
    // in memory storage.
    //
    this.ItemsTableCache = new memoryCache.MemoryCache(
        this.config.ItemsMaxEntries,
        this.config.ItemsTrimCount
        );

    //
    // Attempt to load optional backing store provider
    //
    this.backingStoreFactory = null;
    this.backingStore = null;

    if (this.config.BackingStoreProvider != null) {
       this.backingStoreFactory = require(this.config.BackingStoreProvider);

       this.backingStore = 
           new this.backingStoreFactory.BackingStore(this.config);

       this.tracelog("BackingStore " + this.backingStore.getName() + " configured")
    }
    else {
        this.tracelog("No BackingStore configured, using in memory database only");
    }
}

//
// Crud - Create
//
// Arguments:
//
//   itemName - unique name of item
//
//   itemsArray - Array of attributes for item
//
//   callback - callback function
//
// Returns:
//
//   On error: callback(error, result)
//
//   On success: callback(null, itemsArray);
//
Storage.prototype.createItem = function(itemName, itemsArray, callback) {

    var self = this;

    // name == itemName, ie == itemEntry, cb == callback
    var createCacheEntry = function(name, ie, cb) {

        var res = self.ItemsTableCache.set(name, ie);
        if (!res) {
            self.traceerror("failure to add items entry");
            cb("Items table full", null);
        }
        else {
            cb(null, ie);
        }
    };

    // First lookup in the cache
    var itemEntry = self.ItemsTableCache.get(itemName);
    if (itemEntry != null) {
        // Cache Hit, return "ItemAlreadyExists" failure
        self.tracelog("createItem error item in cache hit itemName=" + itemName);
        callback("ItemAlreadyExists", null);
        return;
    }

    self.tracelog("creating item " + itemName);

    if (self.backingStore == null) {
        // No backing store, in memory storage only
        createCacheEntry(itemName, itemsArray, callback);
        return;
    }

    //
    // If a backing store is configured we only add items
    // to the cache on successful get from the backing store.
    //
    self.backingStore.createItem(itemName, itemsArray, function(error, result) {

        if (error != null) {
            self.tracelog(error + " createItem: error adding item to datastore error=");
            callback(error, result);
            return;
        }

        //
        // We don't attempt to delete the item in the backing
        // store if create cache entry fails locally.
        //
        createCacheEntry(itemName, itemsArray, callback);
    });
}

//
// cRud - Read
//
// Arguments:
//
//   itemName - unique name of item
//
//   callback - callback function
//
// Returns:
//
//   On error: callback(error, result)
//
//   On success: callback(null, itemsArray);
//
Storage.prototype.getItem = function(itemName, callback) {

    var self = this;

    // First lookup in the cache
    var itemEntry = self.ItemsTableCache.get(itemName);
    if (itemEntry != null) {
        // Cache Hit, return entry
        self.tracelog("getItem cache hit itemName=" + itemName);
        callback(null, itemEntry);
        return;
    }

    if (self.backingStore == null) {
        // no backing store, there is no item
        callback("ItemDoesNotExist", null);
        return;
    }

    // attempt to read the item from the backing store
    self.backingStore.readItem(itemName, true, function (error, itemsArray) {

        if (error != null) {
            callback(error, itemsArray);
            return;
        }

        // Success, attempt to place it into the cache
        // Note: If cache placement fails, we still return the object.
        var res = self.ItemsTableCache.set(itemName, itemsArray);
        if (!res) {
            var msg = "getItem: attempt to cache returned item failed " + itemName;
            self.tracelog(msg);
        }

        callback(error, itemsArray);
    });
}

//
// cRud - Query
//
// Arguments:
//
//   querystring - object with query parameters from
//                 buildSelectLastReadingsStatement(), etc.
//
//   consistentRead - True if reads should be consistent, or from the nearest
//                    replica for clustered configurations with eventually consistent
//                    semantics.
//
//   callback - callback function
//
// Returns:
//
//   On error: callback(error, result)
//
//   On success: callback(null, nameValueArray);
//
//   result is an array of {itemName: "name", item: {...}}
//
Storage.prototype.queryItems = function(querystring, consistentRead, callback) {

    if (this.backingStore == null) {
        this.ItemsTableCache.queryItems(querystring, consistentRead, callback);
        return;
    }

    //
    // Note: The query is sent to the backing store and will read through
    // the cache. This is ok since the cache always pushes back updates.
    //
    this.backingStore.queryItems(querystring, consistentRead, callback);
}

//
// crUd - Update
//
// Arguments:
//
//   itemName - unique name of item
//
//   itemsArray - Array of attributes to update on item
//
//   callback - callback function
//
// Returns:
//
//   On error: callback(error, result)
//
//   On success: callback(null, itemsArray);
//
Storage.prototype.updateItem = function(itemName, itemsArray, callback) {

    var self = this;

    //
    // Item Attributes to deal with:
    //
    // itemsArray - New attributes to add/update. This is potentially a subset
    //              of settings currently in the cache or backing store.
    //
    // cachedItems - Current settings from cache, or empty object if
    //                  no settings currently in cache.
    //
    // backingStoreItems - Settings retrieved from any backingStore.
    //
    // updatedItems - This is a new object that consists of the union of
    //              current settings and updated/new settings.
    //
    // When updating settings in backing store we only want to send the
    // updated properties and not the entire record. The cache should always
    // reflect all of the most recent, valid attributes.
    //
    var mergeItemAttributes = function(from, to) {

        // Merge/update the attributes
        for(var prop in from) {
           to[prop] = from[prop];
        }
    };

    //
    // get the item. This will read from cache, or backing store.
    //
    self.getItem(itemName, function(error, cachedItems) {

        // Item may not exist, callers responsbility to create it first.
        if (error != null) {
            callback(error, cachedItems);
            return;
        }

        // Place the cached items into a new object
        var newObject = {};
        mergeItemAttributes(cachedItems, newObject);

        // merge the updated/added items
        mergeItemAttributes(itemsArray, newObject);

        if (self.backingStore == null) {
            // No backing store we are done so set the new object in the cache
            self.ItemsTableCache.set(itemName, newObject);
            callback(null, newObject);
            return;
        }

        // Now attempt to write them back to the backing store
        self.backingStore.updateItem(itemName, cachedItems, function(error2, result) {

            if (error2 != null) {
                callback(error2, result);
                return;
            }

            // Now set the update object in the cache
            self.ItemsTableCache.set(itemName, newObject);
            callback(null, newObject);
        });
    });
}

//
// cruD - Delete
//
// Arguments:
//
//   itemName - unique name of item
//
//   callback - callback function
//
// Returns:
//
//   On error: callback(error, result)
//
//   On success: callback(null, itemsArray);
//
Storage.prototype.deleteItem = function(itemName, callback) {

    var self = this;

    // Delete from cache first
    var res = self.ItemsTableCache.remove(itemName);

    if (self.backingStore == null) {
        // no backing store. If failed, return error result.
        if (!res) {
            callback("ItemDoesNotExist", null);
        }
        else {
            callback(null, null);
        }

        return;      
    }

    //
    // Attempt to delete from backing store. It may not have been
    // in the cache.
    //

    self.backingStore.deleteItem(itemName, callback);
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
Storage.prototype.buildSelectChildrenStatement =
    function(domain, parent, itemcount) {

    if (this.backingStore == null) {
        return this.ItemsTableCache.buildSelectChildrenStatement(sensorid, timestamp, itemcount);
    }

    return this.backingStore.buildSelectChildrenStatement(sensorid, timestamp, itemcount);
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
Storage.prototype.buildSelectChildrenAndDescendantsStatement =
    function(domain, parent, itemcount) {

    if (this.backingStore == null) {
        return this.ItemsTableCache.buildSelectChildrenAndDescendantsStatement(sensorid, timestamp, itemcount);
    }

    return this.backingStore.buildSelectChildrenAndDescendantsStatement(sensorid, timestamp, itemcount);
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
Storage.prototype.buildSelectLastReadingsStatement = function(sensorid, timestamp, itemcount) {

    if (this.backingStore == null) {
        return this.ItemsTableCache.buildSelectLastReadingsStatement(sensorid, timestamp, itemcount);
    }

    return this.backingStore.buildSelectLastReadingsStatement(sensorid, timestamp, itemcount);
}

Storage.prototype.dumpAccounts = function(callback) {
    callback(null, null);
}

Storage.prototype.dumpSensors = function(accountEntry, callback) {
    callback(null, null);
}

Storage.prototype.dumpReadings = function(sensorEntry, callback) {
    callback(null, null);
}

Storage.prototype.dumpSensorSettings = function(sensorEntry, callback) {

    var settings = sensorEntry.SensorSettings;

    for (var prop in settings) {
        this.tracelog("readings_prop=" + prop);
        this.tracelog("readings_value=" + settings[prop]);
    }

    callback(null, null);
}

Storage.prototype.setTrace = function(value) {
    this.trace = value;
}

Storage.prototype.setTraceError = function(value) {
    this.traceerrorValue = value;
}

Storage.prototype.tracelog = function(message) {
    if (this.trace) {
        this.logger.info(this.moduleName + ": " + message);
    }
}

Storage.prototype.traceerror = function(message) {
    if (this.traceerrorValue) {
        this.logger.error(this.moduleName + ": " + message);
    }
}

Storage.prototype.debuglog = function(message) {
    if (this.debug) {
        this.logger.log("debug", this.moduleName + ": " + message);
    }
}

module.exports = {
  Server: Storage
};
