
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
// Arguments Table for Dweet.
//
// 01/25/2015
//

var programname = "main";

//
// This table defines which options are available
//

var help = {
    "name": "-help",
    "altname": "-h",
    "option": true,
    "valueoption": false,
    "required": false,
    "present": false,
    "value": 0
};

var tracelog = {
    "name": "-tracelog",
    "altname": "-tl",
    "option": true,
    "valueoption": false,
    "required": false,
    "present": false,
    "value": 0
};

var traceerror = {
    "name": "-traceerror",
    "altname": "-te",
    "option": true,
    "valueoption": false,
    "required": false,
    "present": false,
    "value": 0
};

var verbose = {
    "name": "-verbose",
    "altname": "-v",
    "option": true,
    "valueoption": false,
    "required": false,
    "present": false,
    "value": 0
};

// Specify the port
var portoption = {
    "name": "-port",
    "altname": "-p",
    "option": true,
    "valueoption": true,
    "required": false,
    "present": false,
    "value": 0
};

// Run a script of Dweet commands
var scriptoption = {
    "name": "-script",
    "altname": "-s",
    "option": true,
    "valueoption": true,
    "required": false,
    "present": false,
    "value": 0
};

var apphandleroption = {
    "name": "-apphandler",
    "altname": "-a",
    "option": true,
    "valueoption": true,
    "required": false,
    "present": false,
    "value": 0
};

var configoption = {
    "name": "-config",
    "altname": "-a",
    "option": true,
    "valueoption": true,
    "required": false,
    "present": false,
    "value": 0
};

// Dont stop script on errors
var dontstoponerror = {
    "name": "-dontstoponerror",
    "altname": "-dontstoponerror",
    "option": true,
    "valueoption": false,
    "required": false,
    "present": false,
    "value": false
};

// Open an interactive console
var consoleoption = {
    "name": "-console",
    "altname": "-c",
    "option": true,
    "valueoption": false,
    "required": false,
    "present": false,
    "value": false
};

var checksumtest = {
    "name": "-checksumtest",
    "altname": null,
    "option": true,
    "valueoption": false,
    "required": false,
    "present": false,
    "value": 0
};

var configuredOptions = {
    "help":    help,
    "verbose": verbose,
    "tracelog": tracelog,
    "traceerror": traceerror,
    "checksumtest": checksumtest,
    "consoleoption": consoleoption,
    "port": portoption,
    "script": scriptoption,
    "apphandler": apphandleroption,
    "config": configoption,
    "dontstoponerror": dontstoponerror
};

function getArgsTable() {
    return configuredOptions;
}

function getProgramName() {
    return programname;
}

function ArgsTable(createArgs) {
    this.getArgsTable = getArgsTable;
    this.getProgramName = getProgramName;
}

function createInstance(createArgs) {
    var argsTable = new ArgsTable(createArgs);
    return argsTable;
}

module.exports = {
  createInstance: createInstance
};
