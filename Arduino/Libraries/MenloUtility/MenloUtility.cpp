
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
 *  File: MenloUtility.cpp
 *
 * Utility functions in support of Menlo sensor network libraries
 */

#include "MenloPlatform.h"
#include "MenloUtility.h"
#include "MenloDebug.h"

// 17 bytes
const char*
MenloUtility::hexDigits = "0123456789ABCDEF";

void
MenloUtility::DelaySecondsWithWatchdog(long secondsToDelay)
{
    // Maximum delay is 1S between watchdog resets

    while(secondsToDelay-- > 0) {

       // Tell watchdog we are still alive
       ResetWatchdog();

       // 1 second
       delay(1000);

       // Tell watchdog we are still alive
       ResetWatchdog();
    }
}

//
// char* is a string with hex digits such as "0F".
//
// The first two chars are consumed from the string
//
uint8_t
MenloUtility::HexToByte(char* str)
{
    unsigned long val;
    char* endptr;
    char buf[3];

    buf[0] = *str++; // upper nibble
    if (*str == '\0') {
        // error, should be even # of chars
        return 0;
    }

    buf[1] = *str++; // lower nibble
    buf[2] = '\0';

    endptr = NULL;
    val = strtoul(buf, &endptr, 16);
    if (endptr == buf) {
        // conversion failure
        return 0;
    }

    return (uint8_t)val;
}

//
// char* is a string with hex digits such as "ABCD".
//
// The first four chars are consumed from the string
//
uint16_t
MenloUtility::HexToUShort(char* str)
{
    unsigned long val;
    char* endptr;
    char buf[5];

    buf[0] = *str++;
    if (*str == '\0') {
        // error, should be 4 chars
        return 0;
    }

    buf[1] = *str++;
    if (*str == '\0') {
        // error, should be 4 chars
        return 0;
    }

    buf[2] = *str++;
    if (*str == '\0') {
        // error, should be 4 chars
        return 0;
    }

    buf[3] = *str++;

    buf[4] = '\0';

    endptr = NULL;
    val = strtoul(buf, &endptr, 16);
    if (endptr == buf) {
        // conversion failure
        return 0;
    }

    return (uint16_t)val;
}

unsigned long
MenloUtility::HexToULong(char* str, bool* error)
{
  unsigned long val;
  char* endptr = NULL;
  *error = false;

   val  = strtoul(str, &endptr, 16);
   if (endptr == str) {
       // conversion failure
       *error = true;
       return 0;
   }

   return val;
}

//
// This returns exactly two ASCII characters
// for an 8 bit input.
//
void
MenloUtility::UInt8ToHexBuffer(uint8_t i, char* buffer)
{
  buffer[0] = (hexDigits[(i >> 4) & 0x0F]);
  buffer[1] = (hexDigits[i & 0x0F]);
}

//
// This returns exactly four ASCII characters
// for an 16 bit input.
//
void
MenloUtility::UInt16ToHexBuffer(uint16_t i, char* buffer)
{
  buffer[0] = (hexDigits[(i >> 12) & 0x0F]);
  buffer[1] = (hexDigits[(i >> 8) & 0x0F]);
  buffer[2] = (hexDigits[(i >> 4) & 0x0F]);
  buffer[3] = (hexDigits[i & 0x0F]);
}

//
// This returns exactly eight ASCII characters
// for an 32 bit input.
//
void
MenloUtility::UInt32ToHexBuffer(unsigned long i, char* buffer)
{
  buffer[0] = (hexDigits[(i >> 28) & 0x0F]);
  buffer[1] = (hexDigits[(i >> 24) & 0x0F]);
  buffer[2] = (hexDigits[(i >> 20) & 0x0F]);
  buffer[3] = (hexDigits[(i >> 16) & 0x0F]);
  buffer[4] = (hexDigits[(i >> 12) & 0x0F]);
  buffer[5] = (hexDigits[(i >> 8) & 0x0F]);
  buffer[6] = (hexDigits[(i >> 4) & 0x0F]);
  buffer[7] = (hexDigits[i & 0x0F]);
}

//
// Given a string and a length ensure the actual length
// of the NULL terminated string does not exceed length.
//
// Ensure the NULL '\0' is maintained.
//
// Return actual length of chars without the NULL terminator.
//
int
MenloUtility::TruncateStringToLength(char* string, int maxLength)
{
  int length = strlen(string);

  if ((length + 1) > maxLength) {
      length = maxLength - 1;
      // Ensure NULL
      string[length - 1] = '\0';
  }

  return length;
}

//
// Convert unsigned 16 bit value to decimal.
//
// Buffer must be 6 to include up to 5 digits and '\0'
//
// Buffer will contain leading 0's.
//
// The return pointer will skip leading 0's.
//
// Dual use functions such as this saves code space in call sites
// over two calls.
//
// These functions avoid use of sprintf() which costs
// 1536 bytes of flash on an AtMega328.
//
// sprintf costs 1536 bytes of flash, 3 bytes of RAM
//
// Before 29,020 flash, 1598 RAM
// After  30,566 flash  1602 RAM
//
// MenloUtility::UInt16ToDecimalBuffer
//
// Before 32,156 flash, 1610 RAM
// After  30,780 flash, 1608 RAM
//    
// Savings 1376 bytes of flash, 2 bytes of RAM
//    
char*
MenloUtility::UInt16ToDecimalBuffer(uint16_t i, char* bufferArg)
{
    int index;
    uint16_t tmp;
    uint16_t pow;
    char* buffer;

    buffer = bufferArg;

    //
    //                10**index
    // 65535 - 10000  - 4
    //  5535 -  1000  - 3
    //   535 -   100  - 2
    //    35 -    10  - 1
    //     5 -     1  - 0
    //
    //              index
    // 10**4        43210
    //
    pow = (uint16_t)10000;

    for (index = 4; index >= 0; index--) {

        tmp = i / pow;

        *buffer++ = hexDigits[tmp];

        if (i >= pow) {
            i = i - (pow * tmp);
        }

        pow = pow / 10;
    }

    *buffer = '\0';

    return SkipLeadingZeros(bufferArg);
}

//
// Skip leading '0''s
//
// Buffer is expected to be '\0' terminated.
//
char*
MenloUtility::SkipLeadingZeros(char* buffer)
{
    while (*buffer == '0') {

        // If we are the last char, stop
        if (buffer[1] == '\0') return buffer;

        buffer++;
    }

    return buffer;
}
