
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
 *  File: MenloXBee.cpp
 */

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

//
// Include Menlo Debug library support
//
#include <Debug.h>

// This libraries header
#include <MenloXBee.h>

// Constructor
MenloXBee::MenloXBee(int rxpin, int txpin) :
  m_port(rxpin, txpin)
{
}

int
MenloXBee::Initialize(int speed)
{
  m_speed = speed;
  m_port.begin(m_speed);
  return 0;
}

int
MenloXBee::Read(
    byte *buffer,
    int length)
{
  return 0;
}

int
MenloXBee::Write(
    byte *targetAddress,
    byte *buffer,
    int length)
{
  return 0;
}

// Returns whether data is available from the XBee
int
MenloXBee::DataAvailable()
{
  return m_port.available();
}

int
MenloXBee::WaitForDataAvailable(int timeout)
{
    unsigned long now;
    unsigned long begin;

    if (m_port.available()) return 1;

    begin = now = millis();

    while((now - begin) < timeout) {
        if (m_port.available()) return 1;
        now = millis();
    }

    return -1;
}

//
// Sync the stream by reading till it gets the XBEE_PACKET_BEGIN character.
//
// This should only be called once data is expected as it will block waiting
// for data.
//
// It will return -1 on timeout, 1 on success. The XBEE_PACKET_BEGIN character
// is consumed.
//
int
MenloXBee::SyncBinaryOnAsciiStream(int timeout)
{
    char data;

    do {
        // Wait for data
      if (WaitForDataAvailable(timeout) == (-1)) return -1;
        data = m_port.read();
    } while(data != XBEE_PACKET_BEGIN);
    
   return 1;
}

void MenloXBee::WriteNibAsAscii(char nib)
{
    if (nib > 9)
      nib = (nib - 10) + 'A';
    else 
      nib = nib + '0';

    m_port.write(nib);
}

void MenloXBee::WriteAsHex(char data)
{
  WriteNibAsAscii(data & 0x0F);
  WriteNibAsAscii((data >> 8) & 0x0F);
}

int MenloXBee::IsHexDigit(char data)
{
  if (((data >= '0') && (data <= '9')) 
       ||
      ((data >= 'A') && (data <= 'F'))) {
    return 1;
  }

  return 0;
}

char
MenloXBee::ToNibble(char data)
{
    if ((data >= '0') && (data <= '9')) {
        return '9' - data;
    }
    else {
      return ('F' - data) + 10;
    }
}

int
MenloXBee::FromHexDigits(
    char upper,
    char lower,
    char* binaryOutput
    )
{
  char value;

  if (!IsHexDigit(upper)) return -1;
  if (!IsHexDigit(lower)) return -1;

  value = ToNibble(lower);  
  value |= (ToNibble(upper) << 8);

  *binaryOutput = value;

  return 1;
}

//
// Read a byte represented as ASCII hex from the stream. If its a characeter is
// received which is not a not a hex digit, or a timeout, returns -1.
//
// Value in *buffer
//
//  return status == 1 -> hex digit from 0x0-0xFF
//  return status == (-1) 0xFF == timeout, or character in error if not a timeout to
//                        caller to see if its a protocol delimeter to re-sync the stream.
//
int
MenloXBee::ReadByteAsAsciiHex(
    char* buffer,
    int timeout
    )
{
  char upper, lower;

  *buffer = 0xFF;

  if (ReadChar(&upper, timeout) == (-1)) return -1;

  //
  // Even though FromHexDigits validates the input,
  // returning (-1) right away on a sync error helps
  // get back in sync quicker on character loss.\
  //
  // We also return the digit read so it can see if its
  // an end of frame, or begin of frame.
  //
  if (!IsHexDigit(upper)) {
      *buffer = upper;
      return -1;
  }

  if (ReadChar(&lower, timeout) == (-1)) return -1;

  if (!IsHexDigit(lower)) {
      *buffer = lower;
      return -1;
  }

  return FromHexDigits(upper, lower, buffer);
}

int
MenloXBee::WriteBinaryOnAsciiStream(
    char* buffer,
    int bufferLength
    )
{
    int count;

    // First write the packet start
    m_port.write(XBEE_PACKET_BEGIN);

    for(count = 0; count < bufferLength; count++) {
        WriteAsHex(buffer[count]);
    }

    m_port.write(XBEE_PACKET_END);

  return bufferLength;
}

int
MenloXBee::ReadChar(
    char* buffer,
    int timeout
    )
{
  char data;
  if (WaitForDataAvailable(timeout) == (-1)) return -1;
  *buffer = m_port.read();
  return 1;
}

//
// Read a packet terminated by the delimiter.
//
// Returns -1 on overflow or timeout
//
int
MenloXBee::ReadBinaryOnAsciiStream(
    char* buffer,
    int bufferLength,
    int timeout
    )
{
    char data;
    int count;

    if (SyncBinaryOnAsciiStream(timeout) == (-1)) return -1;

    for(count = 0; count < bufferLength; count++) {

        // Read a byte encoded as two ASCII hex digits
        if (ReadByteAsAsciiHex(&data, timeout) == (-1)) {

	  if (data == XBEE_PACKET_END) {
              buffer[count] = 0;
              return count;
	  }

          return -1;
        }

        buffer[count] = data;
    }

    // Overflow before packet end character read
    return -1;
}
