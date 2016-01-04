
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
 *  Date: 01/26/2014
 *  File: MenloUtility.h
 *
 * Utility functions in support of Menlo sensor network libraries
 */

#ifndef MenloUtility_h
#define MenloUtility_h

class MenloUtility {
 public:

  //
  // Perform delay keeping the watchdog reset
  //
  static void DelaySecondsWithWatchdog(long secondsToDelay);

  //
  // Convert two hex digits to a byte.
  //
  static uint8_t HexToByte(char* str);

  //
  // Convert four hex digits to an unsigned short
  //
  static uint16_t HexToUShort(char* str);

  //
  // Convert up to 8 hex digits to an unsigned long
  //
  // Uses C library strtoul()
  //
  static unsigned long HexToULong(char* str, bool* error);

  //
  // Convert uint8_t to two ASCII hex digits
  // The buffer takes two characters and is not NULL terminated
  //
  static void UInt8ToHexBuffer(uint8_t c, char* buffer);

  //
  // Convert uint16_t to four ASCII hex digits
  // The buffer takes four characters and is not NULL terminated
  //
  static void UInt16ToHexBuffer(uint16_t i, char* buffer);

  //
  // Convert uint32_t to eight ASCII hex digits
  // The buffer takes eight characters and is not NULL terminated
  //
  static void UInt32ToHexBuffer(unsigned long i, char* buffer);

  //
  // Convert uint16_t to 5 decimal digits.
  //
  // The buffer takes 6 characters and is NULL terminated.
  //
  // The buffer will have any leading 0's in it.
  //
  // The returned char* will point into the buffer skipping
  // any leading 0's.
  //
  static char* UInt16ToDecimalBuffer(uint16_t i, char* buffer);

  //
  // Skip leading '0''s
  //
  // Buffer is expected to be '\0' terminated.
  //
  static char* SkipLeadingZeros(char* buffer);

  //
  // Array of digits 0-9, A-F. Exposed to share memory.
  //
  static const char* hexDigits;

  //
  // Given a string and a length ensure the actual length
  // of the NULL terminated string does not exceed length.
  //
  // Ensure the NULL '\0' is maintained.
  //
  // Return actual length of chars without the NULL terminator.
  //
  static int TruncateStringToLength(char* string, int maxLength);
};

#endif // MenloUtility_h
