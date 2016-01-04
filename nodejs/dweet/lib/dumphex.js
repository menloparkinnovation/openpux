
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
// hex dump utility
//
// 12/05/2014
//

//
// Dump hex + ascii from the buffer at the starting index
// for the given length.
//
// callback is supplied each single line string to be processed
// by the caller.
//

//
// hexodec
//
// callback(data)
//
var dumpHexAscii = function(data, startIndex, length, callback) {

    var index = startIndex;
    var bytesPerLine = 8;

    var end = startIndex + length;
    if (end > data.length) end = data.length;

    for (var index = startIndex; index < end; index += bytesPerLine) {
        var line = addressEntry(index) + ": ";
        line += formatHexAsciiString(data, index, bytesPerLine);
        callback(line);
    }
}

//
// Allow the caller to choose the address base displayed
//
var dumpHexAsciiWithAddressBase = function(data, startIndex, length, addressbase, callback) {

    var index = startIndex;
    var bytesPerLine = 8;

    var address = parseInt(addressbase, 16);

    var end = startIndex + length;
    if (end > data.length) end = data.length;

    for (var index = startIndex; index < end; index += bytesPerLine) {

        var line = addressEntry(address + index) + ": ";

        line += formatHexAsciiString(data, index, bytesPerLine);
        callback(line);
    }
}

// Generate the address entry with padding
function addressEntry(address) {
    var line = "";
    var str = address.toString(16);
    var pad = 8 - str.length;

    line += str;
    for (var index = 0; index< pad; index++) {
        line += " ";
    }

    return line;
}

//
// Print hex + ascii line of 16 bytes
//
function formatHexAsciiString(data, startIndex, bytesPerLine) {

    var line = "";
    var end = startIndex + bytesPerLine;
    if (end > data.length) end = data.length;

    for (i=startIndex; i < end; i++) {
        var c;

        // Some buffers are raw without char representation
        if (typeof(data.charCodeAt) == "undefined") {
            c = data[i];
        }
        else {
            c = data.charCodeAt(i);
        }

        line += "0x";
        line += c.toString(16);
        if (c <= 0xF) {
            line += " "; // padding
        }
        line += " ";
    }

    line += "| ";

    for (i = startIndex; i < end; i++) {
        var code;

        // Some buffers are raw without char representation
        if (typeof(data.charCodeAt) == "undefined") {
            code = data[i];
        }
        else {
            code = data.charCodeAt(i);
        }

        if (isPrintable(code)) {
            line += String.fromCharCode(code);
        }
        else {
            line += '.';
        }
    }

    return line;
}

//
// code is the hex return from charCodeAt()
//
function isPrintable(code) {
        
    var template = "azAZ09";

    if ((code >= template.charCodeAt(0)) &&
        (code <= template.charCodeAt(1))) {
        return true;
    }
     
    if ((code >= template.charCodeAt(2)) &&
        (code <= template.charCodeAt(3))) {
        return true;
    }

    if ((code >= template.charCodeAt(4)) &&
        (code <= template.charCodeAt(5))) {
        return true;
    }

    // Handle printable special characters
    var printableChars = "~`!@#$%^&*()-_+={}[]|\\\'\";:/?><.,";

    var c  = String.fromCharCode(code);

    if (printableChars.indexOf(c) != (-1)) {
        return true;
    }

    return false;
}

function DumpHex() {
    this.dumpHex = dumpHexAscii;
    this.dumpHexWithAddressBase = dumpHexAsciiWithAddressBase;
}

function createInstance() {
    myModule = new DumpHex();
    return myModule;
}

module.exports = {
  createInstance: createInstance
};
