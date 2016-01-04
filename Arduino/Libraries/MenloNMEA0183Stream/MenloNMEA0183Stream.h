
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
 *  File: MenloNMEA0183Stream.h
 *
 * NMEA 0183 handling as an unbuffered stream.
 *
 * Used for low memory debug output streaming.
 */

#ifndef MenloNMEA0183Stream_h
#define MenloNMEA0183Stream_h

//
// Stream version with no buffering or I/O.
//
// Assists in generating correct sentences on behalf of
// the caller.
//
// The caller is responsible for any buffering or direct
// sending of the sentence.
//
// Sequence:
//
// nmea = new MenloNMEA0183Stream();
// nmea->Initialize(maxSentenceLength);
// nmea->startSentence(prefix);
//
// char* cmd = "LIGHT=ON";
//
// int capacity = nmea->capacity();
//
// unsigned char c = nmea->addSeparator();
// Serial.write(c);
//
// char* cmd = "LIGHT=ON";
// nmea->addCommand(cmd);
// Serial.write(cmd, strlen(cmd));
// 
// unsigned char buf[5];
//
// nmea->endSentence(&buf[0]);
// Serial.write(&buf[0], sizeof(buf));
//

class MenloNMEA0183Stream {

 public:

  MenloNMEA0183Stream();

  int Initialize(unsigned char maxLength);

  //
  // Returns the capacity of characters that can be used
  // for a new command.
  //
  // Note: caller must take into account the need for
  // the command separator character ',' in addition
  // to their command string.
  //
  unsigned char capacity();

  //
  // Start a new sentence by resetting the checksum
  // and starting its calculation with the supplied
  // prefix value that the caller is responsible for
  // placing into the front of the stream.
  //
  void startSentence(const char* prefix);

  //
  // Update checksum, character count from the bytes in cmd
  // Caller is responsible for sending the exact bytes.
  //
  // Caller is responsible for calling addSeparator()
  // and sending its single character return value into
  // the stream before calling addCommand();
  //
  // If a sentence length restriction
  //
  unsigned char addCommand(const char* cmd);

  unsigned char addChar(char c);

  //
  // This adds the command separator "," updating the
  // checksum to include it.
  //
  // It's returned to the caller who sends it.
  //
  unsigned char addSeparator();

  //
  // generate checksum, final characters for sentence
  // Buffer is 5 bytes to hold "*00\r\n" for checksum
  // and line ending sequence.
  //
  void endSentence(char* buffer);

 private:

  // streaming checksum value
  unsigned char checksum(char c);
  unsigned char checksum(const char*);

  //
  // NMEA specifies a maximum sentence of 80 characters.
  //
  // We use unsighed char for counts in order to keep
  // overhead small.
  //

  //
  // Maximum characters in a sentence used for capacity()
  // calculations and current overheads.
  //
  unsigned char m_maxCharacters;

  // Count of characters so far
  unsigned char m_characterCount;

  // overhead count
  unsigned char m_overhead;

  // Streaming checksum
  unsigned char m_checksum;

}; // 4 bytes

#endif // MenloNMEA0183Stream_h
