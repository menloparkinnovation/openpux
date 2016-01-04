Dweet Support + Testing

02/14/2015

 nmea0183.js is a general NMEA 0183 handler.

 It is an EventEmitter for sent and received NMEA 0183 streams, and
 provides utility functions for buffering a NMEA 0183 sentence,
 checksums, etc.

 dweethandler.js implements the Dweet command/response protocol on top
 of NMEA 0183. It uses the NMEA 0183 event receiver and the NMEA 0183
 command buffering functions to receive and send Dweet semantic
 messages within NMEA 0183 sentences.

 dweetclient.js provides utility functions for binding to ports,
 virtual buffers, etc.

 Dweet.js is the main program function.

 dweetconsole.js provides a command line oriented console to allow
 high level Dweet commands and NMEA 0183 messages to be sent
 and received.

 It's intended for interactive operation and debugging, as well
 as a framework for remote "Conversational Dweet" service
 construction.

 What is a "Conversational Dweet"? - Something that someone
 would use every day. Its intended for SMS text messages or
 similar and would provide operations such as:

 status - return status of the device, door, alarm, etc.

 arm alarm - arm an alarm

 disarm alarm [pin] - disarm an alarm

 etc.

01/02/2015

 dweethandler.js is the library module for Dweet support. It relies upon
 nmea0183.js in a separate module directory.

 dweettestdevice.js is a node.js implementation of a "Device" that
 responds to Dweets. It can be used as a template for implementing a
 Node.js NMEA 0183 "Dweet" device target.

 devicemodel.js is the "hardware" device model exposed by dweettestdevice.js
 and approximates the capabilities of an "Arduino" embedded controller development
 board as these are very common and use for Dweet testing.

 dweet.js is the main test driver program that has two modes:

 1) Operate in memory against the dweettestdevice using simulated commands.

    node dweet.js testdevice

 2) Operate against a real device over the npm serialport package using Dweet.

    Test firmware may be programmed into an "Arduino" low cost embedded
    development board using a free IDE and connected over USB to the
    host computer.

    The Arduino firmware is part of an embedded Dweet library in C/C++.

    node dweet.js
