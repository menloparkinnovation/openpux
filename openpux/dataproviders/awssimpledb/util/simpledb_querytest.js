
/*
 * Copyright (C) 2015 Menlo Park Innovation LLC
 *
 * This is licensed software, all rights as to the software
 * is reserved by Menlo Park Innovation LLC.
 *
 * A license included with the distribution provides certain limited
 * rights to a given distribution of the work.
 *
 * This distribution includes a copy of the license agreement and must be
 * provided along with any further distribution or copy thereof.
 *
 * If this license is missing, or you wish to license under different
 * terms please contact:
 *
 * menloparkinnovation.com
 * menloparkinnovation@gmail.com
 */

//
// Test queries against an existing openpux SimpleDB domain
//

//
// npm install aws-sdk
// node simpledb_querytest.js
//

var util = require('util');

var g_domain = "openpux";

// Load Amazon AWS SimpleDB handler
var simpledbFactory = require('../simpledb_module.js');

// Create instance with full tracing
var g_config = {
    Trace: true,
    TraceError: true,

    // AWS Region
    region: 'us-west-1'
    };

var simpledb = new simpledbFactory.SimpleDB(g_config);

var selectstatement = null;

var directString  = "select * from openpux where ";
    directString += "(__Type = 'SensorReading')";
    directString += " and (__Parent = 'Account_1.Sensor_1')";
    directString += " and (TimeStamp > '0')";

selectStatement = buildSelectLastReadingsStatement(
    g_domain,
    "Account_1.Sensor_1",
    "0",
    100
    );

selectStatement = buildSelectAllStatement(
    g_domain,
    null
    );

performTest(selectStatement, function(error, result) {
});

function buildSelectLastReadingsStatement(domain, sensorid, timestamp, itemcount) {

    var select = "select * from ";
    select += domain;
    select += " where";

    select += " (Type = '__SensorReading')";
    select += " and (__Parent = '" + sensorid + "')";
    select += " and (TimeStamp > '" + timestamp + "')";

    select += " order by TimeStamp desc limit " + itemcount;

    return select;
}

//
// Select an item based on Type and Name
//
function buildSelectTypeAndNameStatement(domain, type, name) {

    var select = "select * from ";

    select += domain;

    select += " where (Type = '" + type + "') and (Name = '" + name + "')";

    return select;
}

function buildSelectPrimaryKeyStatement(domain, primaryKey) {
    var select = "select * from ";
    select += domain;

    select += " where (PrimaryKey = '" + primaryKey + "')";

    return select;
}

//
// Select first itemcount items
//
// "select * from " + domain + " where TimeStamp > '0' order by TimeStamp limit 2";
//
function buildSelectFirstItemsStatement(domain, itemcount) {
    var select = "select * from ";
    select += domain;
    select += " where TimeStamp > '0' order by TimeStamp limit " + itemcount;

    return select;
}

//
// Select last itemcount items
//
// "select * from " + domain + " where TimeStamp > '0' order by TimeStamp desc limit 2";
//
function buildSelectLastItemsStatement(domain, itemcount) {
    var select = "select * from ";
    select += domain;

    select += " where TimeStamp > '0' order by TimeStamp desc limit " + itemcount;

    return select;
}

//
// Build select all statement from domain and where clause
//
// select * means return all field/attributes of the record
//
function buildSelectAllStatement(domain, where) {

    var select = "select * from ";

    select += domain;

    select + where;

    return select;
}

function performTest(selectStatment, callback) {

    console.log("select statement: " + selectStatement);

    simpledb.queryItems(selectStatement, true, '', function(err, data) {

      if (err) {
          console.log(err, err.stack); // an error occurred
          console.log("statement that errored: " + selectStatement);
          throw err;
      }
      else {
          dumpasjson(data);
      }

      callback(err, data);
    });
}

function dumpasjson(ob) {

      // Dump data as JSON
      // null is full depth, default is 2
      //var inspectOptions = { showHidden: true, depth: null };
      var inspectOptions = { showHidden: true, depth: null,
                       customInspect: false, colors: true };

      var dumpdata = util.inspect(ob, inspectOptions);

      console.log(dumpdata);
}
