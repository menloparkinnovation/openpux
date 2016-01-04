
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
// Util lib
//
// 01/03/2015
//
// Last update 01/18/2015 1:28 PM
//

//
// Provide useful utility functions without all the overhead
// and bother of a full module instance.
//

//
// This is not a true module in the npm module style, but
// is a Javascript file that exports functions and variables
// directly.
//

//
// Usage:
//
// var myutil = require('./myutil.js');
//
// myutil.testFunction();
//

function testFunction() {
    console.log("myutil.js testFunction called");
}

function throwStack(message) {

    //
    //http://stackoverflow.com/questions/591857/how-can-i-get-a-javascript-stack-trace-when-i-throw-an-exception
    //

    var err = new Error();

    console.log("throw stack=");
    console.log(err.stack);

    throw message;
}

//
// More reliable is valid number.
//
function isValidNumber(x) {

    //
    // Notes on the issue:
    //
    // http://stackoverflow.com/questions/18082/validate-decimal-numbers-in-javascript-isnumeric
    //
    // http://stackoverflow.com/questions/18082/validate-decimal-numbers-in-javascript-isnumeric/174921#174921
    //
    // https://dl.dropboxusercontent.com/u/35146/js/tests/isNumber.html    //
    //
    if (!(!isNaN(parseFloat(x)) && isFinite(x))) {
        throw "!isNaN(parseFloat(x)) && isFinite(x) says invalid x=" + x;
    }

    //
    // Note: There are plenty of ways to test for NaN that
    // *do not* work.
    //
    // Note: Read about Number.isNaN(abs) to ensure NaN is detected without
    // aliases. For example Javascript will convert parsable strings and
    // values such as "bool" to numbers. (Bool translates to 0 or 1, both
    // valid numbers).
    //
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/isNaN
    //

    if (typeof(x) == "undefined") {
        throw "Number is invalid by typeof(x) == \"undefined\", x=" + x;
    }

    // Currently the universal (though not 100% reliable) test for a bad number.
    if (x != x) {
        throw "Number is invalid by x != x, x=" + x;
    }

    //
    // Some functions are considered global in browsers and widely supported
    //

    // Test against the global isNaN() function
    if (isNaN(x)) {
        throw "Number is invalid by isNaN(x) x=" + x;
    }

    //
    // Standard comparsion tests against values
    // is whats available on all (or most) Javascript engines.
    //
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number
    //

    //
    // We can't trust these are present on all platforms so
    // validate there presence.
    //
    // They throw so we can "know our platform" and its compromises
    // to number validation.
    //
    // Javascript has the interesting property that a test
    // against an undefined will be false, so you would not even
    // know that the platform does not allow us to indicate bad numbers.
    //
    if (typeof(Infinity) == "undefined") {
        throw "Infinity is undefined";
    }

    if (typeof(Number.NaN) == "undefined") {
        throw "Number.NaN is undefined";
    }

    if (typeof(Number.NEGATIVE_INFINITY) == "undefined") {
        throw "Number.NEGATIVE_INFINITY is undefined";
    }

    if (typeof(Number.POSITIVE_INFINITY) == "undefined") {
        throw "Number.POSITIVE_INFINITY is undefined";
    }

    if (typeof(Number.MAX_VALUE) == "undefined") {
        throw "Number.MAX_VALUE is undefined";
    }

    if (typeof(Number.MIN_VALUE) == "undefined") {
        throw "Number.MIN_VALUE is undefined";
    }

    if (typeof(Number.MAX_SAFE_INTEGER) == "undefined") {
        // Not on current Node.js 01/2015
        //throw "Number.MAX_SAFE_INTEGER is undefined";
    }
    else {
        if (!(x < Number.MAX_SAFE_INTEGER)) {
            throw "Number is invalid by < MAX_SAFE_INTEGER x=" + x;
        }
    }

    if (typeof(Number.MIN_SAFE_INTEGER) == "undefined") {
        // Not on current Node.js 01/2015
        //throw "Number.MIN_SAFE_INTEGER is undefined";
    }
    else {
        if (!(x > Number.MIN_SAFE_INTEGER)) {
            throw "Number is invalid by x < MIN_SAFE_INTEGER x=" + x;
        }
    }

    // Test for Infinity
    if (x == Infinity) {
        throw "Number is invalid by x == Infinity x=" + x;
    }

    if (x == Number.NaN) {
        throw "Number is invalid by x == Number.NaN x=" + x;
    }

    if (x == Number.NEGATIVE_INFINITY) {
        throw "Number is invalid by x == NEGATIVE_INFINITY x=" + x;
    }

    if (x == Number.POSITIVE_INFINITY) {
        throw "Number is invalid by x == POSITIVE_INFINITY x=" + x;
    }

    if (!(x < Number.MAX_VALUE)) {
        throw "Number is invalid by > MAX_VALUE x=" + x;
    }

    if (x == 0) {
       // we must take 0 as ok, since the tests below fail on 0
    }
    else if (x < 0) {
        var abs = Math.abs(x);
        if (isNaN(abs)) throw "Math.abs(x) == isNaN()";
        if (!(abs > Number.MIN_VALUE)) {
            throw "Number is invalid by < MIN_VALUE x=" + x;
        }
    }
    else {
        if (!(x > Number.MIN_VALUE)) {
            throw "Number is invalid by x > MIN_VALUE x=" + x;
        }
    }

    //
    // The presence of validation functions is currently (01/2015) considered
    // ECMA Version 6 experimental features. Support is really spotty
    // across browsers. Though the Chrome V8 engine is up to date. Not
    // sure where this maps to Node.js versions.
    //
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/isFinite
    //
/*
REMOVE 01/18/2015 does not work on Node.js as of 01/2015

    if (typeof(Number.isFinite) != "undefined") {

        if (!Number.isFinite(x)) {
            // This throws on 0, 0.00, 0.2188, etc. Node.js as of 01/2015
            //throw "number is not finite x=" + x;
            // Version, platform issues?
        }
    }
*/

/*
01/18/2015 not sure if we want this since strictly a float
value of 1.23 is not an integer.

    if (typeof(Number.isInteger) != "undefined") {

        // NOTE: As of Node.js 01/2015 0.4375, etc. pass this test!
        if (!Number.isInteger(x)) {
            throw "number is not integer x=" + x;
        }
    }
*/

    if (typeof(Number.isNaN) != "undefined") {
        if (Number.isNaN(x)) {
            throw "Number is invalid by Number.isNaN(x) x=" + x;
        }
    }

    if (typeof(Number.isSafeInteger) != "undefined") {
        if (!Number.isSafeInteger(x)) {
            throw "number is not safe integer x=" + x;
        }
    }

    return;
}

//
// Convert a single HEX character in the range of 0-f to
// its 4 bit binary number.
//
function charHexToBinary(buf) {
    var result;

    // First upper case the character so we don't have to test for "a-f"
    buf = buf.toUpperCase();

    var charCode = buf[0].charCodeAt(0);

    if ((charCode >= "0".charCodeAt(0)) && (charCode <= "9".charCodeAt(0))) {
        result = charCode - "0".charCodeAt(0);
    }
    else if ((charCode >= "A".charCodeAt(0)) && (charCode <= "F".charCodeAt(0))) {
        result = (charCode - "A".charCodeAt(0)) + 10;
    }
    else {
        throw "invalid HEX char";
    }

    return result;
}

//
// Convert to ASCII characters representing a hex value
// to binary.
//
// Input: 0x30 0x31 in ASCII
//
// Output: 01 in binary
//
function asciiHexToBinary(buf) {

    var upperNibble = charHexToBinary(buf[0]);
    var lowerNibble = charHexToBinary(buf[1]);

    var result = 0;
    result = (upperNibble << 4) & 0xF0;
    result |= lowerNibble & 0x0F;

    return result;
}

//
// Convert the nibble (4 bits) to a single ASCII hex digit
//
function binaryNibbleToAsciiHexDigit(nibble) {
    var charCode;
    var str = "";

    if ((nibble >= 0) && (nibble <= 9)) {
         charCode = ("0".charCodeAt(0)) + nibble;
         str += String.fromCharCode(charCode);
    }
    else if ((nibble >= 10) && (nibble <= 15)) {
         charCode = ("A".charCodeAt(0)) + (nibble - 10);
         str += String.fromCharCode(charCode);
    }
    
    return str;
}

//
// Convert a binary byte to two ASCII hex digits
//
function binaryByteToAsciiHex(binaryByte) {
    var str = "";

    var upperNibble = (binaryByte >> 4) & 0x0F;
    var lowerNibble = (binaryByte & 0x0F);

    str += binaryNibbleToAsciiHexDigit(upperNibble);
    str += binaryNibbleToAsciiHexDigit(lowerNibble);

    return str;
}

//
// Convert a binary buffer to an ASCII hex string
//
function binaryBufferToAsciiHexString(binBuffer) {
    var str = "";

    for (var index = 0; index < binBuffer.length; index++) {
        var b = binBuffer[index];
        str += binaryByteToAsciiHex(b);
    }    

    return str;
}

//
// Convert an ASCII Hex string to its binary buffer
//
function asciiHexStringToBinaryBuffer(buffer) {

    var pair;

    // Two ASCII chars make one binary octet
    var binBuffer = new Buffer(buffer.length / 2);

    var binBufferIndex = 0;

    for (var index = 0; index < buffer.length; index += 2) {

        pair = "";
        pair += buffer[index];
        pair += buffer[index+1];

        binBuffer[binBufferIndex] = asciiHexToBinary(pair);

        binBufferIndex++;
    }

    return binBuffer;
}

function getDateTime() {
    var now     = new Date(); 
    var year    = now.getFullYear();
    var month   = now.getMonth()+1; 
    var day     = now.getDate();
    var hour    = now.getHours();
    var minute  = now.getMinutes();
    var second  = now.getSeconds(); 
    if(month.toString().length == 1) {
        var month = '0'+month;
    }
    if(day.toString().length == 1) {
        var day = '0'+day;
    }   
    if(hour.toString().length == 1) {
        var hour = '0'+hour;
    }
    if(minute.toString().length == 1) {
        var minute = '0'+minute;
    }
    if(second.toString().length == 1) {
        var second = '0'+second;
    }   
    var dateTime = year+'/'+month+'/'+day+' '+hour+':'+minute+':'+second;   
     return dateTime;
}

//
// This is the list of utility functions exported
//
exports.throwStack = throwStack;
exports.isValidNumber = isValidNumber;
exports.testFunction = testFunction;

// character/buffer handling
exports.charHexToBinary = charHexToBinary;
exports.asciiHexToBinary = asciiHexToBinary;

// Buffer conversion between ASCII hex string
exports.binaryByteToAsciiHex = binaryByteToAsciiHex;
exports.binaryBufferToAsciiHexString = binaryBufferToAsciiHexString;
exports.asciiHexStringToBinaryBuffer = asciiHexStringToBinaryBuffer;

// Time/Date utilities
exports.getDateTime = getDateTime;
