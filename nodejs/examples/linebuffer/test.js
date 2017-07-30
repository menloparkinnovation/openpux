
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
// Test LineBuffer
//
// 12/26/2014
//

var fs = require('fs');

var g_verbose = false;

function main(ac, arv) {
    simpleTest();

    testFileFormatting();
}

//
// Input files in their native format.
// Never modified.
//
var unixInputFileName = "unixfile.txt";
var dosInputFileName = "dosfile.txt";

//
// Output files from 1:1 input/output loop
//
var unix_to_unix_OutputFileName = "unix_to_unix_outputfile.txt";

var dos_to_dos_OutputFileName = "dos_to_dos_outputfile.txt";

//
// Output files from cross platform conversion
//
var unix_to_dos_OutputFileName = "unix_to_dos_outputfile.txt";

var dos_to_unix_OutputFileName = "dos_to_unix_outputfile.txt";

//
// This tests the Unix <-> DOS line formatting
//
// Use dumphex utility on the files to examine their
// formatting, paying attention to the 0xA (Unix) vs.
// 0x0D 0x0A (DOS) line endings.
//
function testFileFormatting() {
    
    //
    // Load the files
    //
    var unix_lb = loadFileToLineBuffer(unixInputFileName);

    var dos_lb = loadFileToLineBuffer(dosInputFileName);

    console.log("unix_lb.getMode() = " + unix_lb.getMode());

    // Force it to Unix since we could be on dos
    unix_lb.setMode("unix");
    saveLineBufferToFile(unix_lb, unix_to_unix_OutputFileName);

    // Force it to Dos since we could be on unix
    dos_lb.setMode("dos");
    saveLineBufferToFile(dos_lb, dos_to_dos_OutputFileName);

    // Convert to the other format
    unix_lb.setMode("dos");
    console.log("after: unix_lb.getMode() = " + unix_lb.getMode());
    saveLineBufferToFile(unix_lb, unix_to_dos_OutputFileName);

    dos_lb.setMode("unix");
    console.log("after: dos_lb.getMode() = " + dos_lb.getMode());
    saveLineBufferToFile(dos_lb, dos_to_unix_OutputFileName);
}

//
// Handy code snippets for using linebuffer
//
function isWin32() {
    if (process.platform == "win32") return true;
    return false;
}

function createLineBuffer(mode) {
    var linebuffer = require('./linebuffer.js').createInstance(mode);
    return linebuffer;
}

function loadFileToLineBuffer(filePath, mode) {
    var buffer = fs.readFileSync(filePath, {encoding: 'utf8'});
    var linebuffer = require('./linebuffer.js').createInstance(buffer, mode);
    return linebuffer;
}

function saveLineBufferToFile(linebuffer, filePath) {
    var buffer = linebuffer.toBuffer();
    fs.writeFileSync(filePath, buffer);
}

function displayLineBuffer(linebuffer) {
    var buffer = linebuffer.toBuffer();
    console.log(buffer);
}

function displayLineBufferLines(linebuffer, prefix) {
    var s;
    var lineno = 0;
    var buffer = "";

    //
    // Lines are retrieved without line endings which
    // is the linebuffer contract.
    //
    while ((s = linebuffer.getNextLine()) != null) {
        console.log(prefix + lineno + ": " + s);
        lineno++;
    }
}

function simpleTest() {
    var sourceFile = "testfile.txt";

    var linebuffer = loadFileToLineBuffer(sourceFile);

    linebuffer.putLine("+++ THIS IS AN ADDED LINE ++++");

    displayLineBufferLines(linebuffer, "sourceFile ");

    console.log("writebuffer:")

    var writebuffer = createLineBuffer();

    writebuffer.putLine("this");
    writebuffer.putLine(" is");
    writebuffer.putLine(" a");
    writebuffer.putLine(" test\n");
    
    displayLineBufferLines(writebuffer, "putline ");
}

var usage = function(bad_arg) {

    if (typeof bad_arg != "undefined") {
        console.error("Bad argument " + bad_arg);
    }

    console.error(g_usage);

    process.exit(1);
}

function tracelog(config, message) {
    if (g_verbose) {
        // console.log connects to stdout
        console.log(message);
    }
}

function errlog(message) {
    // console.error connects to stderr
    console.error(message);
}

//
// Remove argv[0] to get to the base of the standard arguments.
// The first argument will now be the script name.
//
var args = process.argv.slice(1);

// Invoke main
main(args.length, args);
