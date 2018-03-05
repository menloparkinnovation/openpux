
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
 *  Date: 12/13/2014
 *  File: MenloNMEA0183.h
 *
 * NMEA 0183 handling.
 *
 * Used for Smartpux DWEET's.
 */

#include "MenloPlatform.h"
#include "MenloUtility.h"
#include "MenloDebug.h"
#include "MenloNMEA0183Stream.h"
#include "MenloNMEA0183.h"

//
// 6 per NMEA 0183 specification which includes $
// $WIWMV, $PDWT, $PDBG
//
const int
MenloNMEA0183::MaxPrefixSize = 6;

//
// Overhead is the minimum sentence payload ovehead
  // $WIWMV,*00 -> 10 characters
//
const int
MenloNMEA0183::PayLoadOverhead = 10;

//
// 80 per NMEA 0183 specification
//
const int
MenloNMEA0183::PayLoadSize = 80;

// Trailing <CR><LF>'\0'
const int
MenloNMEA0183::TrailerOverhead = 3;

// The CheckSum length is "*00" for the checksum.
const int
MenloNMEA0183::CheckSumLength = 3;

const int 
MenloNMEA0183::MinimumBufferLength = 10;

//
// Note: We use the common constants from MenloNMEA0183Stream
// where possible.
//

MenloNMEA0183::MenloNMEA0183()
{
  m_buffer = NULL;
  m_bufferIndex = 0;
  m_prefix = NULL;
  m_maxPrefixLength = MenloNMEA0183::MaxPrefixSize;
}

void
MenloNMEA0183::CalculateBufferLengths(char* prefix)
{
  int length;

  m_bufferIndex = 0;

  // Pre-copy the prefix into the buffer
  length = strlen(prefix);
  memcpy(&m_buffer[m_bufferIndex], prefix, length);
  m_bufferIndex += length;
  m_buffer[m_bufferIndex] = '\0';

  // Calculate amount left after the prefix
  length = m_bufferLength - m_bufferIndex;
  
  // Must leave "*00" for the checksum, and <CR><LF>'\0'
  m_maxBufferIndex = m_bufferLength - (CheckSumLength + TrailerOverhead);

  // m_maxBufferIndex - m_bufferIndex is what is available for commands
}

int
MenloNMEA0183::capacity()
{
  return m_maxBufferIndex - m_bufferIndex;
}

//
// This allow reseting the buffer at runtime for
// systems with tight memory that share the buffer with
// other uses such as sending HTTP requests, etc.
//
int
MenloNMEA0183::SetOutputBuffer(char* buffer, int bufferLength)
{

  if (buffer == NULL) {
    return 0;
  }

  if (bufferLength < MinimumBufferLength) {
    return 0;
  }

  //
  // The bufferLength supplied is the maximum full buffer we
  // have. So calculate our NMEA maximum sentence accordingly.
  //

  m_buffer = buffer;
  m_bufferLength = bufferLength;

  m_bufferIndex = 0;
  m_maxBufferIndex = 0;

  // A reset() or reset(char* prefix) will recalculate the buffer
  CalculateBufferLengths(m_prefix);

  return 1;
}

int
MenloNMEA0183::Initialize(char* prefix, char* buffer, int bufferLength)
{
  int result;

  // Set prefix first since it is used in the buffer calculations
  m_prefix = prefix;

  result = SetOutputBuffer(buffer, bufferLength);

  return result;
}

void
MenloNMEA0183::reset()
{
  // This resets the buffer to the default prefix
  CalculateBufferLengths(m_prefix);
}

void
MenloNMEA0183::reset(char* prefix)
{
  // This resets the buffer to the supplied prefix
  CalculateBufferLengths(prefix);
}

int
MenloNMEA0183::addCommand(char *cmd)
{
  if (addCommandStart() == 0) return 0;
  if (addCommandPart(cmd) == 0) return 0;

  return addCommandComplete();
}

int
MenloNMEA0183::addCommand_P(PGM_P cmd)
{
  if (addCommandStart() == 0) return 0;
  if (addCommandPart_P(cmd) == 0) return 0;

  return addCommandComplete();
}

//
// Start a new command by appending a ',' to the already
// reset buffer pre-loaded with the prefix.
//
int
MenloNMEA0183::addCommandStart()
{
  if (m_buffer == NULL) return 0;

   // Add one for the "," separator
   if (1 > capacity()) {
     return 0;
   }

#if OLD_WAY
   m_buffer[m_bufferIndex++] = ',';
   m_buffer[m_bufferIndex] = '\0';
#else
  char* ptr = &m_buffer[m_bufferIndex];
   *ptr++ = ',';
   m_bufferIndex++;
   *ptr = '\0';
#endif

   return 1;
}

int
MenloNMEA0183::addCommandComplete()
{
  // nothing to do in this version
  return 1;
}

//
// Add a command part. addCommandStart() must be called first.
//
int
MenloNMEA0183::addCommandPart(char* cmd)
{
  int cmdLength;

  if (m_buffer == NULL) return 0;

  cmdLength = strlen(cmd);

  if (cmdLength > capacity()) {
    return 0;
  }

  memcpy(&m_buffer[m_bufferIndex], cmd, cmdLength);
  m_bufferIndex += cmdLength;

  m_buffer[m_bufferIndex] = '\0';

  return cmdLength;
}

int
MenloNMEA0183::addCommandPart_P(PGM_P cmd)
{
  int cmdLength;

  if (m_buffer == NULL) return 0;

  cmdLength = strlen_P(cmd);

  if (cmdLength > capacity()) {
    return 0;
  }

  memcpy_P(&m_buffer[m_bufferIndex], cmd, cmdLength);
  m_bufferIndex += cmdLength;

  m_buffer[m_bufferIndex] = '\0';

  return cmdLength;
}

//
// To save space the sentence is generated into the
// configured buffer in which the caller can use it for
// the send, or copy into another buffer.
//
// When done they call reset() to allow a new buffer to be started.
//
char*
MenloNMEA0183::generateSentence()
{
  int length;
  unsigned char chksum;
  char buffer[2];
  char* ptr;

  if (m_buffer == NULL) {
    return NULL;
  }

  length = strlen(m_prefix);  

  if (m_bufferIndex == length) {
    // just prefix in buffer
    return NULL;
  }

  //
  // Note: Using OLD_WAY costs 122 bytes of code space more than the alternative
  // on AtMega328!
  //


  // Add checksum symbol
#if OLD_WAY
  m_buffer[m_bufferIndex++] = '*';
  m_buffer[m_bufferIndex] = '\0';
#else
  ptr = &m_buffer[m_bufferIndex];
  *ptr++ = '*';
  m_bufferIndex++;
  *ptr = '\0';
#endif

  // Calculate checksum
  if (!checksum(&m_buffer[0], &chksum)) {
      // format is incorrect

      // NOTE: This includes the entire source path in the image!
      // 30,110 with this, 29,936 without. 174 bytes!
      //DEBUG_SHIP_ASSERT(DEBUG_FALSE);

      return NULL;
  }

  MenloUtility::UInt8ToHexBuffer(chksum, &buffer[0]);

#if OLD_WAY
  m_buffer[m_bufferIndex++] = buffer[0];
  m_buffer[m_bufferIndex++] = buffer[1];
  m_buffer[m_bufferIndex++] = '\r';
  m_buffer[m_bufferIndex++] = '\n';
  m_buffer[m_bufferIndex] = '\0';
#else
  *ptr++ = buffer[0];
  *ptr++ = buffer[1];
  *ptr++ = '\r';
  *ptr++ = '\n';
  *ptr = '\0';

  m_bufferIndex += 4;
#endif

  return m_buffer;
}

//
// This modifies the output buffer to null terminate
// the commands string where the '*' is.
//
// Returns a char* to the start of the commands after
// the prefix.
//
// The prefix and checksum status is returned as well.
//
char*
MenloNMEA0183::parse(
    char* buffer,
    char* prefixOut,
    unsigned char* checksumOut,
    int* checksumOKOut
    )
{
  int index;
  char *firstCommand = NULL;
  unsigned char chksum;
  char checksumAscii[2];

  // If string format is not valid error out now
  if (!checksum(buffer, &chksum)) {
      return NULL;
  }

  //
  // Since NMEA 0183 is used by MenloDebug, we must perform
  // direct writing to the serial port when debugging here.
  //  
  // This messes up the protocol, but at least allows low level
  // infrastructure debugging.
  //  
  // Note: Must turn on traceerror for the dweet utility so that
  //       bad checksum messages get reported.
  //  
  //Serial.print("$PDBG,checksum=");
  //Serial.print(chksum);
  //Serial.println("*00");

  prefixOut[0] = '\0';
  *checksumOKOut = 0;

  //
  // Checksum may still be invalid, but at least the string
  // was formatted correctly to be able to calculate it.
  //
  *checksumOut = chksum;

  //
  // Convert the checksum binary byte to ASCII hex digits
  // for comparison later. Cheaper than doing atoi with hex conversion
  // or worse yet scanf()
  //

  MenloUtility::UInt8ToHexBuffer(chksum, &checksumAscii[0]);

  //
  // parse the buffer indexing by ',' characters
  //
  // return each word between each pair of ',' characters
  //
  // model in strtok which allows moving a cursor through
  // to minimize buffering and complex callbacks.
  //
  if (buffer[0] != '$') {
      return NULL;
  }
  
  // Load prefix
  for (index = 0;; index++) {

    // str to short
    if (buffer[index] == '\0') {
      prefixOut[0] = '\0';
      return NULL;
    }

    if (buffer[index] == ',') break;

    if (index >= m_maxPrefixLength) {
      // prefix too long
      prefixOut[0] = '\0';
      return NULL;
    }

    prefixOut[index] = buffer[index];
  }

  prefixOut[index] = '\0';

  // skip ','
  index++;

  // this points to the first command buffer[index]
  firstCommand = &buffer[index];

  // Now look for sentence end '*'
  while(buffer[index] != '\0') {
    if (buffer[index] == '*') break;
    index++;
  }

  if (buffer[index] != '*') {
    prefixOut[0] = '\0';
    return NULL;
  }

  // Mark the end of the commands string
  buffer[index] = '\0';

  // skip '*' entry
  index++;

  //
  // We could hit end of string '\0', but checksum digits
  // won't compare.
  //
  if ((checksumAscii[0] == buffer[index]) &&
      (checksumAscii[1] == buffer[index+1])) {
    // Check sum is valid
    *checksumOKOut = 1;
  }
  else {
    // Check sum is not valid. We return to the caller
    // the command pointer so they can decide the policy.
  }

  return firstCommand;
}

//
// Validate basic string format and calculate a checksum
// for the data payload.
//
// It does not actually validate this checksum, but returns
// it to the caller.
//
//
bool
MenloNMEA0183::checksum(char* str, unsigned char* checksumOut)
{
  int index = 0;
  unsigned char chksum = 0;

  int strLength = strlen(str);

  //
  // Must have at least $ for start, * for end, plus a char to
  // be a valid NMEA message.
  //
  if (strLength < 3) {
      return false;
  }

  //
  // String must start with $
  //
  // A * must be found in the string during checksum that terminates
  // the data part of the string.
  //
  // Check sum starts with an initial value of 0.
  //
  // Includes all parts of the string but $ and *
  //
  // If any <CR><LF> exists they are ignored as
  // well.
  //
  // Example:
  //   $PSPX,STRICT=ON*
  //

  index = 0;

  if (str[index] != '$') {
    return false;
  }

  // skip '$'
  index++;

  for (; index < strLength; index++) {

    if (str[index] == '*') {
      break;
    }

    chksum = (chksum ^ str[index]);
  }

  if (str[index] != '*') {
    return false;
  }

  *checksumOut = chksum;

  return true;
}
