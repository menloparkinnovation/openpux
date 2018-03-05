
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

/*
 *  Date: 11/21/2012
 *  File: MenloXBee.h
 */

#ifndef MenloXBee_h
#define MenloXBee_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#include <Arduino.h>
#include <inttypes.h>

// MenloXBee implements the MenloRadio class
#include <MenloRadio.h>

// MenloXBee uses SoftwareSerial for pin flexibility
#include <SoftwareSerial.h>

//
// Binary over ASCII Protocol
//
// For simplicity with XBee's pass through or protocol mode,
// binary information is sent as an ASCII hex string in the following form:
//
// There is a frame start character 'X' that can be searched for to sync
// the stream and denote the begining of a new message.
//
// All bytes are 8 bits represented by two ASCII upper case hex digits
//  '0' - '9', 'A-F'
//
// The string contains 0 or more bytes represented.
//
// The string ends with an end delimeter ';'
//
// No protocol level checksums, counts, etc, are included. This
// is all the responsibility of an upper level.
//
// The begin and end delimeter make this a framed, packet protocol
// which is easier to deal with for simple sensor protocols at the
// upper layer. (Less buffering required)
//
// Any XBee addressing is outside of this protocol
//
// The following example is a 13 byte binary message:
//
// X0102030405060708FFFAFCF0;
//
// Pretty much this is modeling a small binary packet protocol
// over the ASCII stream oriented XBee's in transparent
// passthrough mode, which is easier for experimenters.
//
#define XBEE_PACKET_BEGIN 'X'
#define XBEE_PACKET_END   ';'

// 1 second
#define XBEE_READ_TIMEOUT 1000

class MenloXBee : MenloRadio {

public:
  
  MenloXBee(int rxpin, int txpin);

  int Initialize(int speed);

  // Inherited from MenloRadio
  virtual int Read(byte *buffer, int length);
  virtual int Write(byte *targetAddress, byte* buffer, int length);
 
  int DataAvailable();

  int WaitForDataAvailable(int timeout);

  int SyncBinaryOnAsciiStream(int timeout);

  int WriteBinaryOnAsciiStream(
    char* buffer,
    int bufferLength
    );

  int ReadBinaryOnAsciiStream(
    char* buffer,
    int bufferLength,
    int timeout
    );

private:

  // TODO: The Hex<->binary stuff could be put into a utility library
  // Also need to see what's already getting linked in for println support
  // as we have only 32k.
  void WriteAsHex(char data);

  void WriteNibAsAscii(char nib);

  char ToNibble(char data);

  int IsHexDigit(char data);

  int ReadChar(char* buffer, int timeout);

  int ReadByteAsAsciiHex(char* buffer, int timeout);

  int FromHexDigits(char upper, char lower, char* binaryOutput);

  SoftwareSerial m_port;
  int m_speed;

};

#endif // MenloXBee_h
