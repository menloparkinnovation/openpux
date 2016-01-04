
//
//   simpledb_util.js
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
// 09/01/2015
//
// SimpleDB utility
//
// node simpledb_until.js get ItemName
// node simpledb_until.js query "Select Statement"
//

// For util.inspect()
var util = require('util');

// Load Amazon AWS SimpleDB handler
var simpledbFactory = require('../simpledb_module.js');

var g_config = {
    Trace: true,
    TraceError: true,

    // AWS Region
    region: 'us-west-1',

    // SimpleDB Domain
    domain: "openpux"
    };

function Module(config) {
    this.config = config;
    this.simpledb = new simpledbFactory.SimpleDB(config);
}

Module.prototype.main = function(ac, av) {

    var operation = null;
    var itemName = null;

    if (ac > 1) {
        operation = av[1];

        if (ac > 2) {
            itemName = av[2];
        }
    }
    else {
        this.usage("incorrect argument number " + ac);
        return;
    }

    if (operation == "get") {

        if (itemName == null) {
            this.usage("must specify ItemName");
            return;
        }

        console.log("get: ItemName=" + itemName + " domain=" + this.config.domain);
        this.readItem(this.config.domain, itemName);
        return;
    }
    else if (operation == "query") {

        if (itemName == null) {
            this.usage("must specify select statement");
            return;
        }

        var selectStatement = null;

        if ((itemName == "all") || (itemName == "*")) {
            selectStatement = this.buildSelectAllStatement(this.config.domain, null);
        }
        else {
            selectStatement = itemName;
        }

        console.log("select statement: " + selectStatement);
        this.query(selectStatement);
        return;
    }
    else if (operation == "listdomains") {
        console.log("listdomains");
        this.listDomains();
        return;
    }
    else if (operation == "createdomain") {
        console.log("createdomain");
        this.createDomain(this.config.domain);
        return;
    }
    else if (operation == "dEleteDomaiN") {
        console.log("dEleteDomaiN!!!!");
        this.deleteDomain(this.config.domain);
        return;
    }
    else {
        this.usage("unknown operation " + operation);
        return;
    }
}

Module.prototype.listDomains = function() {

    var self = this;

    self.simpledb.listDomains(100, '', function(error, data) {

        if (error) {
            console.log(error);
        }
        else {
            self.processResultSet(data);
        }
    });
}

Module.prototype.createDomain = function(domain) {

    var self = this;

    self.simpledb.createDomain(domain, function(error, data) {
        if (error) {
            console.log(error);
        }
        else {
            self.processResultSet(data);
        }
    });
}

Module.prototype.deleteDomain = function(domain) {

    var self = this;

    self.simpledb.deleteDomain(domain, function(error, data) {
        if (error) {
            console.log(error);
        }
        else {
            self.processResultSet(data);
        }
    });
}

Module.prototype.readItem = function(domain, itemName) {

    var self = this;

    self.simpledb.readItem(domain, itemName, true, function(error, data) {
        if (error) {
            console.log(error);
        }
        else {
            self.processResultSet(data);
        }
    });
}

Module.prototype.query = function(selectStatement) {

    var self = this;

    self.simpledb.queryItems(selectStatement, true, '', function(error, data) {

      if (error) {
          console.log(error);
          console.log("select statement that errored: " + selectStatement);
      }
      else {
          self.processResultSet(data);
      }
    });
}

//
// Process the result set returned from the SimpleDB service.
//
Module.prototype.processResultSet = function(data) {

    var self = this;

    console.log("processResultSet (RAW):");
    if (data == null) {
        console.log("data == null");
        return;
    }
    
    self.dumpasjson(data);

    var ResponseMetadata = data.ResponseMetadata;

    console.log("");

    if (ResponseMetadata == null) {
        console.log("no ResponseMetadata");

        // Ensure we understand responses fully
        if (data.Items != null) throw "unexpected items when ResponseMetaData == null";

        return;
    }

    console.log("RequestId: " + data.ResponseMetadata.RequestId);
    console.log("BoxUsage: " + data.ResponseMetadata.BoxUsage);

    console.log("");

    //
    // Two forms of return:
    //
    // One form from readItem (getAttributes)
    //
    // Another form from query (search)
    //
    if (data.Items != null) {

        //
        // result set is an array of Items with Name and attributes collection
        // which results in multiple objects
        //
        var objects = self.simpledb.getObjectNameValueArrayFromResultSet(data);
        if (objects == null) {
            console.log("No Items in result set");
            return;
        }

        console.log("object[]=");

        self.dumpasjson(objects);
    }
    else if (data.Attributes != null) {
        
        //
        // getAttributes results are just the attributes
        //
        var obj = self.simpledb.getObjectFromGetAttributes(data);
        if (obj == null) {
            console.log("No attributes in result set");
            return;
        }

        console.log("obj=");

        self.dumpasjson(obj);
    }
    else {
        console.log("*** do not know how to process result set ***");
    }

    return;
}

//
// Build select all statement from domain and where clause
//
// select * means return all field/attributes of the record
//
Module.prototype.buildSelectAllStatement = function(domain, where) {

    var select = "select * from ";

    select += domain;

    select + where;

    return select;
}

Module.prototype.dumpasjson = function(ob) {

      // Dump data as JSON
      // null is full depth, default is 2
      //var inspectOptions = { showHidden: true, depth: null };
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      console.log(dumpdata);
}

Module.prototype.usage = function(message) {

    if (message != null) {
        console.error(message);
    }

    console.error("");
    console.error("get/query AWS SimpleDB for openpux");
    console.error("");
    console.error("    node simpledb_util get ItemName");
    console.error('    node simpledb_util query "Select Statement"');
    console.error("    node simpledb_util createdomain");
    console.error("    node simpledb_util dEleteDomaiN");
    console.error("");
    console.error("Examples:");
    console.error("");
    console.error("    node simpledb_util get Account_1");
    console.error("    node simpledb_util get Account_1.Sensor_1");
    console.error("    node simpledb_util get Account_1.Sensor_1.Reading_141130364498");
    console.error("");
    console.error('    node simpledb_util query all');
    console.error('    node simpledb_util query "*"');
    console.error('    node simpledb_util query "select * from openpux"');
    console.error('    node simpledb_util query "select * from openpux where SleepTime = \'30\'"');
    console.error('    node simpledb_util query "select TargetMask from openpux where SleepTime = \'30\'"');
    console.error('    node simpledb_util query "select * from openpux where SensorID = \'Account_1.Sensor_1\'"');
    console.error("");
    console.error("    node simpledb_util createdomain");
    console.error("");
    console.error("    Note: This is upper and lower case to avoid mistakes");
    console.error("          since it will delete all data");
    console.error("");
    console.error("    node simpledb_util dEleteDomaiN");
    console.error("");

    process.exit(1);
}

var args = process.argv.slice(1);

var module = new Module(g_config);
module.main(args.length, args);
