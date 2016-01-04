
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
// LineBuffer
//
// 12/26/2014
//
// See Model: at the end for details.
//

// File 
var fileio = require("./fileio.js").createInstance();

function LineBuffer(buffer, mode) {

    this.lines = null;
    this.index = 0;
    this.fillIndex = 0;

    // Default is to output buffers based on the current platform
    if (typeof(mode) == "undefined") {
        mode = "auto";
    }

    this.setMode(mode);

    //
    // If a buffer is supplied initialize the lines
    //
    if (typeof(buffer) != "undefined") {
        this.loadBuffer(buffer);
    }
    else {
        // empty array
        this.lines = new Array();
    }
}

LineBuffer.prototype.length = function() {
    return this.lines.length;
}

LineBuffer.prototype.getLine = function(index) {
    return this.lines[index];
}

LineBuffer.prototype.putLine = function(str) {
    return this.lines[this.fillIndex++] = str;
}

LineBuffer.prototype.updateLine = function(str, index) {
    return this.lines[index] = str;
}

// Reset the line enumerator
LineBuffer.prototype.reset = function() {
    this.index = 0;
}

//
// Remove "\r\n" if present.
//
LineBuffer.prototype.removeLineEndings = function(s) {

    // First trim '\n'
    if ((s.length > 0) && (s[s.length - 1] == '\n')) {
        s = s.substring(0, s.length - 1);
    }

    // Next trim '\r'
    if ((s.length > 0) && (s[s.length - 1] == '\r')) {
        s = s.substring(0, s.length - 1);
    }

    return s;
}

// Enumerate through lines in the buffer
LineBuffer.prototype.getNextLine = function() {
    if (this.index > this.lines.length) return null;
    return this.lines[this.index++];
}

LineBuffer.prototype.getMode = function() {
    return this.mode;
}

//
// Set the operating mode.
//
// Mode controls the handling of '\n vs. '\r\n' for Unix
// vs. Windows line endings when the buffer is output.
//
LineBuffer.prototype.setMode = function(value) {

    var mode = value.toLowerCase();

    if (mode == "auto") {
        if (process.platform == "win32") {
            this.mode = "dos";
        }
        else {
            this.mode = "unix";
        }
    }
    else if (mode == "raw") {
        this.mode = mode;
    }
    else if (mode == "unix") {
        this.mode = mode;
    }
    else if (mode == "dos") {
        this.mode = mode;
    }
    else if (mode == "win32") {
        // "win32" is an alias for dos mode as its known in unix land
	this.mode = "dos";
    }
    else {
	throw "unknown mode " + mode;
    }

    //
    // Note: No changes are made to the lines already
    // in the buffer.
    //

    return;
}

//
// load the buffer from the file
//
LineBuffer.prototype.loadBufferFromFile = function(file) {
    var buffer = fileio.readFileToBufferSync(file);
    this.loadBuffer(buffer);
}

//
// Load the supplied buffer.
//
LineBuffer.prototype.loadBuffer = function(buffer) {

    //
    // The buffer is maintained in no line endings mode unless raw.
    //

    // This splits the lines on '\n' and removes them.
    this.lines = buffer.split('\n');    

    //
    // If mode == "raw" we replace the removed '\n''s
    //
    // This supports the contract that the buffered lines
    // in memory represent the original buffer/file loaded for
    // "raw" mode.
    //
    if (this.mode == "raw") {
        for (var index = 0; index < this.lines.length; index++) {
            this.lines[index] = (this.lines[index]) += '\n';
        }
    }
    else {

        //
        // We could have been input dos formatted lines. The '\n'
        // was removed, but the '\r' may still be present at
        // the end of the line.
        //
        for (var index = 0; index < this.lines.length; index++) {
            var s = this.lines[index];
            if ((s.length > 0) && (s[s.length - 1] == '\r')) {
                s = s.substring(0, s.length - 1);
            }

            this.lines[index] = s;
        }
    }

    // Allow append
    this.fillIndex = this.lines.length;
}

//
// Lines are returned in target format
//
LineBuffer.prototype.toBuffer = function() {
    var buf = "";

    for (var index=0; index < this.lines.length; index++) {

        var s = this.lines[index];

        // If not "raw" add back the removed line ending
        if (this.mode != "raw") {
            s += '\n';
        }

        // if mode is "dos", ensure lines end with its format
        if (this.mode == "dos") {
            s = this.unixToDos(s);
        }

        buf += s;
    }

    return buf;
}

//
// This converts all occurances of a single instance of
// '\n' into the double character sequence '\r''\n' to
// conform to dos/win32 line ending expectations.
//
LineBuffer.prototype.unixToDos = function(str) {

    var strIndex;
    var buffer = "";

    strIndex = 0;

    //
    // Open question is to whether using buffer or string cat
    // is more efficient with Node.js
    //
    // http://www.clintharris.net/2011/nodejs-101-use-buffer-to-append-strings-instead-of-concatenating-them/
    //

    for (; strIndex < str.length;) {

        if (str[strIndex] == '\n') {

            if ((strIndex > 0) && (str[strIndex - 1] == '\r')) {
                // already has '\r''\n'
            }
            else {
                // Insert extra '\r' before '\n'
                buffer += '\r';
            }
        }

        buffer += str[strIndex++];
    }

    return buffer.toString();
}

//
// Convert a line that ends with '\r''\n' into one
// that ends with '\n'
//
// Note: This just deals with the line endings, and does
// not look through the entire string.
//
LineBuffer.prototype.dosToUnix = function(str) {

    //
    // If the file came from Windows there may be an extra
    // '\r' (0x0D) before the '\n' (0x0A)
    //
    if ((str != null) && (str.length > 1)) {
	if ((str[str.length - 1] == '\n') &&
	    (str[str.length - 2] == '\r')) {

            // strip off the '\r''\n'
	    str = str.substring(0, str.length - 2);

           // add single '\n'
           str += '\n';
	}
    }

    return str;
}

//
// Valid modes are:
//
// "auto" -> (default) select based on current platform executing on
//
// "raw" -> do nothing to buffer and added lines.
//
// "unix" -> lines end with '\n'
//
// "dos" -> lines end with '\r''\n'
//
// "win32" -> (alias for "dos") lines end with '\r''\n'
//
function createInstance(buffer, mode) {
    myModule = new LineBuffer(buffer, mode);
    return myModule;
}

module.exports = {
  createInstance: createInstance
};

//
// Model:
//
// Line Endings:
//
// mode == "raw"
//
//  Nothing is done to the formatting of the lines.
//
//  Line boundaries are still determined by '\n', but
//  these remain in the returned lines.
//
//  This is useful for manipulating lines
//  that are part of an external protocol such as
//  NMEA 0183 which have the line ending requirement of
//  '\r''\n' regardless of platform.
//
// getNextLine/putLine:
//
// Lines are internally stored as an array of lines that
// have no line terminator. When individual lines are
// get or put they are handled without the terminators and
// the caller determines line endings for any I/O.
//
// loadBuffer/toBuffer:
//
// When lines are loaded from a buffer lines are split
// on the '\n' line ending character with the '\n' removed
// to be compatible with putLine().
//
// If they are Dos formatted lines the extra '\r' is also removed.
//
// When lines are retrieved to a buffer their line endings are
// inserted based on the current mode.
//
// Conversions between Unix and Dos:
//
// Loading a buffer always places lines in a line terminator
// free mode, removing any terminator characters regardless
// of the source unix or DOS.
//
// When individual lines are retrieved with getNextLine()
// they are always in terminator free mode and its up to
// the caller to deal with the target I/O properly.
//
// If a buffer is returned it is formatted according to
// the current mode with the line terminators added. This
// makes it convenient to stream to a file.
//


// Usage Patterns:
//
// buffer could have been sourced from a unix or dos formatted file
//
//  // autoselect platform for how the lines should be formatted
//  var linebuffer = require('./linebuffer.js').createInstance(buffer);
//  var linebuffer = require('./linebuffer.js').createInstance(buffer, "auto");
//
//  // lines are to be in unix format, regardless of current platform
//  var linebuffer = require('./linebuffer.js').createInstance(buffer, "unix");
//
//  // lines are left in original format regardless of platform
//  var linebuffer = require('./linebuffer.js').createInstance(buffer, "raw");
//
//  // lines are to be in dos format, regardless of current platform
//  var linebuffer = require('./linebuffer.js').createInstance(buffer, "dos");
//  var linebuffer = require('./linebuffer.js').createInstance(buffer, "win32");
//
//  // Existing lines in the buffer are then converted to the target:
//  linebuffer.setMode("auto");
//  linebuffer.setMode("dos");
//  linebuffer.setMode("unix");
//
