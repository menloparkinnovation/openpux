
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

#ifndef MenloNMEA0183_h
#define MenloNMEA0183_h

class MenloNMEA0183 {
 public:

  //
  // 6 per NMEA 0183 specification which includes $
  // $WIWMV, $PDWT, $PDBG
  //
  static const int MaxPrefixSize;

  // 80 per NMEA 0183 specification
  static const int PayLoadSize;

  //
  // Overhead is the minimum sentence payload ovehead
  // $WIWMV,*00 -> 10 characters
  //
  static const int PayLoadOverhead;

  //
  // Overhead is the memory in addition to the NMEA sentence.
  // This is the <CR><LF>'\0'
  //
  static const int TrailerOverhead;

  //
  // The CheckSum length is "*00" for the checksum.
  //
  // This is included in NMEA 0183's maximum string size of 80 characters
  //
  static const int CheckSumLength;

  // Need <CR><LF> and $,* + prefix and '\0'
  static const int MinimumBufferLength;

  static int MaximumUserPayload() {
    return PayLoadSize - PayLoadOverhead;
  }

  MenloNMEA0183();

  int Initialize(char* prefix, char* buffer, int bufferLength);

  int SetOutputBuffer(char* buffer, int bufferLength);

  //
  // Note: These match the node.js implementation nmea0183.js
  //

  // Resets the buffer to start a new one
  void reset();

  // Start a new buffer with a specific prefix
  void reset(char* prefix);

  int addCommand(char* cmd);

  int addCommand_P(PGM_P cmd);

  //
  // This allows a command to be placed into the buffer
  // with multiple back to back calls.
  //
  // This is to allow sending a command from a series
  // of buffers, some in data space, some in program space.
  //
  // It's geared for tight memory systems.
  //
  int addCommandStart();

  int addCommandPart(char* cmd);

  int addCommandPart_P(PGM_P cmd);

  int addCommandComplete();

  // return characters left in buffer for new commands
  int capacity();

  // Generate sentence into the internal buffer
  char* generateSentence();

  //
  // Parse the supplied buffer. It is expected to
  // have a complete NMEA 0183 sentence with the ending
  // \r\n.
  //
  // This modifies the output buffer to null terminate
  // the commands string where the '*' is.
  //
  // Returns a char* to the start of the commands after
  // the prefix.
  //
  // The prefix and checksum status is returned as well.
  //
  char* parse(
      char* buffer, 
      char* prefixOut,
      unsigned char* checksumOut,
      int* checksumOKOut
      );

  //
  // Calculate checksum for NMEA 0183 string.
  //
  // Returns false if the string format does not conform to a
  // NMEA 0183 sentence.
  //
  // Sentence must start with $, and data area terminated by *,
  // and have at least one data character.
  //
  bool checksum(char *str, unsigned char* checksum);

 protected:

  //
  // This is the send buffer. It is not used for receive.
  //
  // This must hold m_configuredMaxLength + <CR><LF>
  //
  char* m_buffer;

  // Caller supplied prefix. Typically $PSPX, $PDBG, etc.
  char* m_prefix;

  //
  // The raw total length of the buffer
  //
  int m_bufferLength;

  //
  // Buffer index that is currently filled
  //
  // its value is the length value for the string in the buffer.
  //
  int m_bufferIndex;

  //
  // This is the maximum buffer index for normal commands
  // leaving room in the buffer for the check sum and
  // overheads.
  //
  int m_maxBufferIndex;

  int m_maxPrefixLength;

 private:

  //
  // Internal calculate buffer lengths based on available
  // space, prefix, and required overheads.
  //
  void CalculateBufferLengths(char* prefix);

}; // 12 bytes

#endif // MenloNMEA0183_h
