
//
//   memorycache.js
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

var utility = require('./utility.js');

//
// Simple in memory caching
//
// A simple, synchronous in memory cache is used to implement "working memory"
// of a server. This is intended to be a small, per node.js instance for
// caching frequently accessed data.
//
// It implements a struct LRU based on the ordering of Javascript object map
// property inserts, and Object.keys() indexes.
//

//
// Usage:
//
// var memoryCache = require('./memorycache.js');
//
// var maxEntries = 100;
// var trimCount = 10;
//
// var cache = new memoryCache.MemoryCache(maxEntries, trimCount);
//
// cache.set(key, val);
//
// var val = cache.get(key);
//

//
// maxEntries - Maximum entries for cache.
//
//              A value of 0 means no limit.
//
//              Remember, a cache without reasonable bounds is a memory leak.
//
//              Since this is a strict LRU cache, a maxEntries value too small in
//              relation to total unique requests may constantly push valid entries out
//              of the cache.
//
// trimCount - Number of entries to trim when maxEntries is reached.
//
//             A reasonable fraction of maxEntries ensures CPU time
//             is not spent trimming the cache for each entry added.
//
//             maxEntries/10, maxEntries/4, etc. is recommended.
//
//             If trimCount == 0, no trimming occurs and set will fail when
//             at capacity. trim(count) may be called manually in this case.
//
//             This is useful for support storage models in which deletion
//             of older entries is not desired, such as an in memory database
//             with a fixed capacity.
//
function MemoryCache(maxEntries, trimCount) {

    this.moduleName = "MemoryCache";
    this.maxEntries = maxEntries;
    this.trimCount = trimCount;
    this.entryCount = 0;

    // A Javascript object is a map
    this.cacheMap = new Object();
}

//
// Set a value into the cache.
//
// If the cache is at capacity, timeCount entries are
// removed.
//
// A reference to the object is stored by the key, the object
// is not copied.
//
// val - object reference
//
// Returns:
//  true - entry was entered
//  false - cache is full, and trimCount was specified as 0 at construction
//
MemoryCache.prototype.set = function(key, val) {

    if ((this.maxEntries != 0) && (this.entryCount >= this.maxEntries)) {

        if (this.trimCount == 0) return false;

        this.trim(this.trimCount);
    }

    // map is indexed by string value
    this.cacheMap[key] = val;
    this.entryCount++;
    return true;
}

//
// key - string key value to use
//
// Returns "undefined" if no entry.
//
MemoryCache.prototype.get = function(key) {
    var val = this.cacheMap[key];
    return val;
}

//
// key - string key value to use
//
// No return value
//
MemoryCache.prototype.remove = function(key) {
    delete this.cacheMap[key];
}

//
// Return entryCount
//
MemoryCache.prototype.getEntryCount = function() {
    return this.entryCount;
}

//
// A trimCount keeps from having to process the trim operation
// for each individual overflow.
//
MemoryCache.prototype.trim = function(trimCount) {

    var thisTrim = 0;
    var key;

    var keys = Object.keys(this.cacheMap);

    var thisTrim = keys.length;
    if (thisTrim > trimCount) {
        thisTrim = trimCount;
    }

    // Delete the first keys since they are the oldest
    for (var index = 0; index < thisTrim; index++) {
        key = keys[index];
        delete this.cacheMap[key];
    }

    this.entryCount = this.entryCount - thisTrim;
}

//
// Return the array of keys
//
MemoryCache.prototype.getKeys = function() {
    return Object.keys(this.cacheMap);
}

//
// enumerateChildNames
//
// Enumerate and return child names. This just returns the names, not
// the objects themselves. The object themselves can be retrieved by
// calling getItem() on the returned names.
//
// If includeDescendants == false, just the names of the direct children
// are returned.
//
// If includeDescendants == true, the names of the children and any
// descendants are included. This can result in a large number of items
// returned if the data set is large.
//
// This is used to walk, or discover objects in the sparse namespace
// maintained by the data store.
//
// itemNamePrefix - Path name to enumerate children for.
//
// startingIndex - Index to start from to allow enumeration through a large set
//
// itemCount - Maximum count of items to return.
//
// includeDescendants - If false, only immediate children are returned.
//                      If true, all descendants of parent path are returned.
//
// callback(error, result)
//
// result is an array of childNames.
//
// Example:
//
// If the object store has the following entries:
//
// /accounts/1/sensors/1
// /accounts/1/sensors/1/settings
// /accounts/1/sensors/1/readings/2015-11-11T15:41:26.969Z
// /accounts/1/sensors/1/readings/2015-11-11T15:41:27.992Z
//
// Then a query of parentName /accounts/1/sensors/1 with
// includeDescendants == false would return:
//
// /accounts/1/sensors/1/settings
// /accounts/1/sensors/1/readings
//
// If includeDescendants == true, then the additional items
// below would also be returned:
//
// /accounts/1/sensors/1/readings/2015-11-11T15:41:26.969Z
// /accounts/1/sensors/1/readings/2015-11-11T15:41:27.992Z
//
MemoryCache.prototype.enumerateChildNames = 
    function(parentName, startIndex, itemCount, includeDescendants) {
    
    //
    // In order to return just one instance of a name an
    // object is used as a string indexed array/hashtable
    //
    var o = {};

    var keys = Object.keys(this.cacheMap);

    if (startIndex > keys.length) {
        return null;
    }

    var itemsLeft = keys.length - startIndex;

    if (itemCount > itemsLeft) {
        itemCount = itemsLeft;
    }

    for (var index = startIndex; (index < keys.length) && (itemCount > 0); index++) {
        var key = keys[index];

        if (includeDescendants) {
            if (key.search(parentName) == 0) {
                o[key] = true; // a boolean is lowest possible storage
            }
        }
        else {
            // Validate if name is an immediate decendent
            child = utility.getImmediateChildObject(parentName, key);
            if (child != null) {
                o[child] = true; // a boolean is lowest possible storage
            }
        }
    }

    // Now convert to the property names
    var childNames = Object.keys(o);

    return childNames;
}

//
// Enumerate up to itemCount items that are immediate decendents of parentName.
//
// parentName - Path name to match item entries on.
//
// startingIndex - Index to start from to allow enumeration through a large set
//
// itemCount - Maximum count of items to return.
//
// includeDescendants - If false, only immediate children are returned.
//                      If true, all descendants of parent path are returned.
//
// callback(error, result)
//
// result is an array of {itemName: "name", item: {...}}
//
MemoryCache.prototype.enumerateItems = 
    function(parentName, startIndex, itemCount, includeDescendants) {

    var items = [];

    var keys = Object.keys(this.cacheMap);

    if (startIndex > keys.length) {
        return items;
    }

    var itemsLeft = keys.length - startIndex;

    if (itemCount > itemsLeft) {
        itemCount = itemsLeft;
    }

    for (var index = startIndex; (index < keys.length) && (itemCount > 0); index++) {
        var key = keys[index];

        if (includeDescendants) {
            if (key.search(parentName) == 0) {
                var o = {itemName: key, item: this.cacheMap[key]}
                items.push(o);
                itemCount--;
            }
        }
        else {

            // Validate if name is an immediate decendent and an object
            var child = utility.getImmediateChildObject(parentName, key);
            if (child != null) {

                var o = {itemName: key, item: this.cacheMap[key]}
                items.push(o);
                itemCount--;
            }
        }
    }

    return items;
}

//
// Return the last n items that are immediate children of parentName.
//
// parentName - Path name to match item entries on
//
// itemCount - Count of items to return.
//
// callback(error, result)
//
// result is an array of {itemName: "name", item: {...}}
//
MemoryCache.prototype.enumerateLastItems = function(parentName, itemCount) {

    var items = [];

    var keys = Object.keys(this.cacheMap);

    if (itemCount > keys.length) {
        itemCount = keys.length;
    }

    for (var index = keys.length - 1; (index >= 0) && (itemCount > 0); index--) {
        var key = keys[index];

        // Validate if name is an immediate decendent and an object
        var child = utility.getImmediateChildObject(parentName, key);
        if (child != null) {

            var o = {itemName: key, item: this.cacheMap[key]}
            items.push(o);
            itemCount--;
        }
    }

    return items;
}

//
// Select last itemCount items
//
// itemName - itemName such as "/account/1/sensor/1/readings"
//
// timestamp - timestamp to begin search by
//
MemoryCache.prototype.buildSelectLastItemsStatement =
    function(itemName, timestamp, itemcount) {

    // To support in memory query, return a JSON object with the parameters.
    var parent = itemName + "/";

    var query = { query: "enumerate", 
                  parent: parent,
                  timestamp: timestamp,
                  itemcount: itemcount };

    return JSON.stringify(query);
}

//
// Query items from the in memory cache
//
// querystring - object with query parameters from buildSelectLastReadingsStatement()
//
MemoryCache.prototype.queryItems = function(querystring, consistentRead, callback) {

    var items = null;

    // querystring is a JSON object with the in memory query parameters
    var query = JSON.parse(querystring);

    if (query.query == "enumerate") {

        items = this.enumerateLastItems(query.parent, query.itemcount);

        callback(null, items);
    }
    else {
        this.traceerror("queryItems: unrecognized query=" + query.query + " for in memory cache");
        callback("ItemDoesNotExist", null);
    }
}

//
// Invoke callback for each entry
//
// callback(key, val)
//
MemoryCache.prototype.Dump = function(callback) {

    var key;
    var obj;

    var keys = Object.keys(this.cacheMap);

    for (var index = 0; index < keys.length; index++) {

        key = keys[index];

        // Extract object by key
        obj = this.cacheMap[key];

        callback(key, obj);
    }
}

module.exports = {
  MemoryCache: MemoryCache
};
