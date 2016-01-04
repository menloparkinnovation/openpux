
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
// Common pre-written Dweet commands.
//
// These are commands sent to and received from the device.
//
// 01/03/2015
//

//
// Though Dweet is an open ended protocol in which the developer
// of a device can invent new commands as required, there are
// many built in commands with an existing implementation that
// fit many IoT scenarios.
//
// This module provides pre-configured access and
// examples for these common commands.
//

//
// This is not a true module in the npm module style, but
// is a Javascript file that exports functions and variables
// directly.
//

//
// var dweetcmds = require('./dweetcmds.js');
// var table = dweetcmds.commandtable;
//

//
// Dweet mapping:
//
// COMMAND=object:value
//
// command=dataName:dataValue
//
// command -> DWEET command message
// object  -> typical the item being acted on by the Dweet command
// value   -> Typically value used as part of the operation such as set.
//

var getstate_light = {
    "operation": "getstate",
    "dataName": "LIGHT",
    "valuerequired": false,
    "helpstring": "getstate light",

    "prefix": "$PDWT",
    "command": "GETSTATE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var getstate_alarm = {
    "operation": "getstate",
    "dataName": "ALARM",
    "valuerequired": false,
    "helpstring": "getstate alarm",

    "prefix": "$PDWT",
    "command": "GETSTATE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var getstate_door = {
    "operation": "getstate",
    "dataName": "DOOR",
    "valuerequired": false,
    "helpstring": "getstate door",

    "prefix": "$PDWT",
    "command": "GETSTATE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var setstate_light = {
    "operation": "setstate",
    "dataName": "LIGHT",
    "valuerequired": true,
    "helpstring": "setstate light=\"ON|OFF\"",

    "prefix": "$PDWT",
    "command": "SETSTATE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var setstate_alarm = {
    "operation": "setstate",
    "dataName": "ALARM",
    "valuerequired": true,
    "helpstring": "setstate alarm=\"ON|OFF\"",

    "prefix": "$PDWT",
    "command": "SETSTATE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// GETCONFIG=MODEL
// GETCONFIG_REPLY=MODEL:DweetTestDevice
//
var getconfig_model = {
    "operation": "getconfig",
    "dataName": "MODEL",
    "valuerequired": false,

    "prefix": "$PDWT",
    "command": "GETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// GETCONFIG=NAME
// GETCONFIG_REPLY=NAME:Device1
//
// This function used at startup, so it has a longer
// delay than the defaults.
//
var getconfig_name = {
    "operation": "getconfig",
    "dataName": "NAME",
    "valuerequired": false,

    "prefix": "$PDWT",
    "command": "GETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// GETCONFIG=SERIAL
// GETCONFIG_REPLY=SERIAL:01-02-2015-12345678
//
var getconfig_serial = {
    "operation": "getconfig",
    "dataName": "SERIAL",
    "valuerequired": false,

    "prefix": "$PDWT",
    "command": "GETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// GETCONFIG=VERSION
// GETCONFIG_REPLY=VERSION:1.0
//
var getconfig_version = {
    "operation": "getconfig",
    "dataName": "VERSION",
    "valuerequired": false,

    "prefix": "$PDWT",
    "command": "GETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// GETCONFIG=FIRMWAREVERSION
// GETCONFIG_REPLY=FIRMWAREVERSION:1.0
//
var getconfig_firmwareversion = {
    "operation": "getconfig",
    "dataName": "FIRMWAREVERSION",
    "valuerequired": false,

    "prefix": "$PDWT",
    "command": "GETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// SETCONFIG=MODEL:name
// SETCONFIG_REPLY=MODEL:name
//
var setconfig_model = {
    "operation": "setconfig",
    "dataName": "MODEL",
    "valuerequired": true,
    "helpstring": "setconfig model=\"name\"",

    "prefix": "$PDWT",
    "command": "SETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// SETCONFIG=NAME:Device1
// SETCONFIG_REPLY=NAME:Device1
//
var setconfig_name = {
    "operation": "setconfig",
    "dataName": "NAME",
    "valuerequired": true,

    "prefix": "$PDWT",
    "command": "SETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// SETCONFIG=SERIAL:number
// SETCONFIG_REPLY=SERIAL:number
//
var setconfig_serial = {
    "operation": "setconfig",
    "dataName": "SERIAL",
    "valuerequired": true,

    "prefix": "$PDWT",
    "command": "SETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// SETCONFIG=VERSION:1.0
// SETCONFIG_REPLY=VERSION:1.0
//
var setconfig_version = {
    "operation": "setconfig",
    "dataName": "VERSION",
    "valuerequired": true,

    "prefix": "$PDWT",
    "command": "SETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// SETCONFIG=FIRMWAREVERSION:1.0
// SETCONFIG_REPLY=FIRMWAREVERSION:1.0
//
var setconfig_firmwareversion = {
    "operation": "setconfig",
    "dataName": "FIRMWAREVERSION",
    "valuerequired": true,

    "prefix": "$PDWT",
    "command": "SETCONFIG",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// Debug functions
//
var read_mem = {
    "operation": "read",
    "dataName": "memory",
    "valuerequired": false,
    "helpstring": "read mem",

    "prefix": "$PDWT",
    "command": "MEMREAD",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var read_eeprom = {
    "operation": "read",
    "dataName": "eeprom",
    "valuerequired": false,
    "helpstring": "read eeprom",

    "prefix": "$PDWT",
    "command": "EEPROMREAD",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var read_reg = {
    "operation": "read",
    "dataName": "register",
    "valuerequired": false,
    "helpstring": "read register",

    "prefix": "$PDWT",
    "command": "REGREAD",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var read_flash= {
    "operation": "read",
    "dataName": "flash",
    "valuerequired": false,
    "helpstring": "read flash",

    "prefix": "$PDWT",
    "command": "FLASHREAD",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var write_mem = {
    "operation": "write",
    "dataName": "memory",
    "valuerequired": true,
    "helpstring": "write mem",

    "prefix": "$PDWT",
    "command": "MEMWRITE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var write_eeprom = {
    "operation": "write",
    "dataName": "eeprom",
    "valuerequired": true,
    "helpstring": "write eeprom",

    "prefix": "$PDWT",
    "command": "EEPROMWRITE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var write_reg = {
    "operation": "write",
    "dataName": "register",
    "valuerequired": true,
    "helpstring": "write register",

    "prefix": "$PDWT",
    "command": "REGWRITE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var pinmode = {
    "operation": "pinmode",
    "dataName": "",
    "valuerequired": true,
    "helpstring": "pinmode Dx:MODE",

    "prefix": "$PDWT",
    "command": "PINMODE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var dread = {
    "operation": "digitalread",
    "dataName": "",
    "valuerequired": false,
    "helpstring": "digitalread port",

    "prefix": "$PDWT",
    "command": "DREAD",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var dwrite = {
    "operation": "digitalwrite",
    "dataName": "",
    "valuerequired": true,
    "helpstring": "digitalwrite port=value",

    "prefix": "$PDWT",
    "command": "DWRITE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var aref = {
    "operation": "analogreference",
    "dataName": "",
    "valuerequired": true,
    "helpstring": "analogreference value",

    "prefix": "$PDWT",
    "command": "AREF",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var aread = {
    "operation": "analogread",
    "dataName": "",
    "valuerequired": false,
    "helpstring": "analogread port",

    "prefix": "$PDWT",
    "command": "AREAD",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

var awrite = {
    "operation": "analogwrite",
    "dataName": "",
    "valuerequired": true,
    "helpstring": "analogwrite port=value",

    "prefix": "$PDWT",
    "command": "AWRITE",
    "timeout": 10 * 1000,
    "retryLimit": 1,

     // These are filled in by the command transaction
    "dataValue": null,
    "fullCommand": null,
    "retryCount": 0,
    "error": null,
    "result": null
};

//
// The table of exported commands from this module.
//
var commandtable = [

    // GETSTATE entries
    // getstate object
    getstate_light,
    getstate_alarm,
    getstate_door,

    // SETSTATE entries
    // setstate object="name"
    setstate_light,
    setstate_alarm,

    // GETCONFIG entries
    // getconfig object
    getconfig_model,
    getconfig_name,
    getconfig_serial,
    getconfig_version,
    getconfig_firmwareversion,

    // SETCONFIG entries
    // setconfig object="name"
    setconfig_model,
    setconfig_name,
    setconfig_serial,
    setconfig_version,
    setconfig_firmwareversion,

    // debug entries
    read_mem,
    read_eeprom,
    read_reg,
    read_flash,

    write_mem,
    write_eeprom,
    write_reg,

    // Arduino support
    pinmode,
    dread,
    dwrite,
    aref,
    aread,
    awrite
];

//
// The command group we export from this module
//
exports.commandtable = commandtable;
