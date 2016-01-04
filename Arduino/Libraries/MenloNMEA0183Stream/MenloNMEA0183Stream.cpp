
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
 *  Date: 01/23/2015
 *  File: MenloNMEA0183Stream.cpp
 *
 * NMEA 0183 handling as an unbuffered stream.
 *
 * Used for low memory debug output streaming.
 */

#include "MenloPlatform.h"
#include "MenloUtility.h"
#include "MenloNMEA0183Stream.h"

//
// NMEA 0183 Stream Support
//

MenloNMEA0183Stream::MenloNMEA0183Stream()
{
  m_maxCharacters = 0;
  m_characterCount = 0;
  m_checksum = 0;
  m_overhead = 0;
}

int
MenloNMEA0183Stream::Initialize(unsigned char maxLength)
{
  m_maxCharacters = maxLength;
  return 1;
}

unsigned char
MenloNMEA0183Stream::capacity()
{
  unsigned char total = m_overhead + m_characterCount;
  return (m_maxCharacters - total);
}

void
MenloNMEA0183Stream::startSentence(const char* prefix)
{
  // 5 for "*00\r\n";
  m_overhead = strlen(prefix) + 5;

  m_checksum = 0;
  m_characterCount = strlen(prefix);

  // checksum does not include the '$' character per NMEA0183 specification
  checksum(&prefix[1]);
}

unsigned char
MenloNMEA0183Stream::addChar(char c)
{
  checksum(c);
  return 1;
}

unsigned char
MenloNMEA0183Stream::addCommand(const char* cmd)
{
  unsigned char length = strlen(cmd);

  m_characterCount += length;

  // Update streaming checksum
  checksum(cmd);

  return length;
}

unsigned char
MenloNMEA0183Stream::addSeparator()
{
  m_characterCount++;

  checksum(',');

  return ',';
}

void
MenloNMEA0183Stream::endSentence(char* buffer)
{
  char b[2];

  // Use the common static utility function
  MenloUtility::UInt8ToHexBuffer(m_checksum, &b[0]);

  buffer[0] = '*';
  buffer[1] = b[0];
  buffer[2] = b[1];
  buffer[3] = '\r';
  buffer[4] = '\n';
}

unsigned char
MenloNMEA0183Stream::checksum(char c)
{
  m_checksum = (m_checksum ^ (unsigned char)c);
  return m_checksum;
}

unsigned char
MenloNMEA0183Stream::checksum(const char* buffer)
{
  unsigned char index;
  unsigned char length = strlen(buffer);

  for (index=0; index < length; index++) {
    checksum(buffer[index]);
  }

  return m_checksum;
}
