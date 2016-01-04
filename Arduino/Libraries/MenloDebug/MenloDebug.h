
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
// MenloDebug.h
//
//  Debug Support for Arduino
//
//  The main Arduino libraries do not provide support for debug tracing,
//  asserts, ship asserts, and program halt.
//

#ifndef MenloDebug_h
#define MenloDebug_h

#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloNMEA0183Stream.h>

#include <inttypes.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "WString.h" // __FlashStringHelper
#endif

#include <MenloPanicCodes.h>

//
// Note: MENLO_ARM32 is used to compile out definitions
// of __FlashStringHelper methods since these don't exist
// on 32 bit non-Harvard processors.
//

//
// ASSERT support
//
// Names are defined to avoid collisions with other
// assertion mechanisms in various libraries.
//

#if MENLO_ATMEGA
extern "C" void MenloDebug_AssertionFailed(const __FlashStringHelper* file, int line);
#else
extern "C" void MenloDebug_AssertionFailed(const char* file, int line);
#endif

#define DEBUG_FALSE 0

#if DEBUG
#define DEBUG_ASSERT(x) { if(!(x)) MenloDebug_AssertionFailed(F(__FILE__), __LINE__); }
#else
#define DEBUG_ASSERT(x) 
#endif

//
// This includes the entire source path in the generated object
// code in the text section of the microcontroller. Not a good idea.
//
// SHIP_ASSERT is always present, and will result in a halt
//#define DEBUG_SHIP_ASSERT(x) { if(!(x)) MenloDebug_AssertionFailed(F(__FILE__), __LINE__); }

// 8 bit trace message
#define DEBUG_MAKE_TRACE_MSG(ModuleId, MessageId) (ModuleId | (MessageId & 0x0F))

// 16 bit trace message
#define DEBUG_MAKE_TRACE_MSG16(ModuleId, MessageId) ((uint8_t)((ModuleId << 8) | (MessageId & 0xFF)))

//
// This allows a string to be associated with a message.
//
// The message string is not compiled into the target microcontroller
// to save space.
//
// Other processing could be used to scan a code base and build
// a dictionary of message id's to debug strings.
//
// Could also use compiler specific debug data emit functions
// which provide the information in the program symbols, but
// not in the code downloaded to the target.
//
// DEBUG_TRACE_MESSAGE(MODULE_MENLODEBUG, 1, "MessageString");
//
#define DEBUG_TRACE_MSG16(ModuleId, MessageId, Message)   \
  DebugPrintHex(DEBUG_MAKE_TRACE_MSG(ModuleId, MessageId))

// Encode two error codes into upper/lower nibbles of a byte
#define DEBUG_MAKE_ERROR_CODE(Major, Minor) ((uint8_t)((Major << 4) | (Minor & 0x0F)))

// Debug class that can be called from C++
class MenloDebug : MenloObject {
 public:

  // Call from setup()
  static void Init();

  // Caller can specify a pre-initialized port
  static void Init(Stream*);

  // Allows setting a Prefix in front of new messages
  static void SetPrefix(char*);

  // Single byte trace for smallest overhead
  static void Trace(uint8_t);

  //
  // Trace is used to log a word which is composed of a module ID
  // and a module specific message ID.
  // 
  static void Trace16(unsigned short);

  static uint8_t GetTraceLevel();

  static void SetTraceLevel(uint8_t level);

  //
  // Set synchronous output. Useful when a hard to debug
  // crash or reset occurs before debugging messages have
  // been sent out the serial port.
  //
  static uint8_t SetSynchronous(uint8_t syncOn);

  //
  // This variant of Trace allows a line to be output/recorded.
  //
#if defined(MENLO_ATMEGA)
  static void TracePrint(unsigned short, const __FlashStringHelper*);
#endif
  static void TracePrint(unsigned short, char*);

  //
  // Record/output hex number
  //
  static void TraceHex(unsigned short traceID, int number);
  static void TracePrintHex(unsigned short traceID, int number);

  // Panic with code. No return.
  static void Panic(uint8_t code);

  // This configures a pin with an LED connected to it to flash the panic code.
  // (-1) disable it.
  static void ConfigurePanicPin(int pinNumber);

  // This configures the number of display loops during a panic
  static void ConfigurePanicDisplayLoops(uint8_t number);

  //
  // Display Error Code and return.
  // This will flash on the LED if ConfigurePanicPin() has been called.
  //
  static void DisplayErrorCode(uint8_t code);

  //
  // Print a simple hex number as a string. No newline.
  //
  // Note: This is deprecated and replace by the C++
  // overload version of Print(int), PrintNoNewline(int)
  //
  static size_t PrintHex(int number);
  static size_t PrintHexNoNewline(int number);

  //
  // Print a hex string representing binary data.
  //
  static size_t PrintHexString(uint8_t *buffer, int length);
  static size_t PrintHexStringNoNewline(uint8_t* buffer, int length);

  //
  // General print formatting
  //  
  // Note: size_t return value of != 0 exists to model
  // base Arduino Print.h behavior in which compound macro's
  // depend upon.
  //
  // Note: If routine over NMEA 0183 embedded "=" may confuse
  // a monitor program which uses "=" as a command separator.
  //
  // TODO: Escape special chars such as "=" on the debug protocol.
  //
  static size_t Print(const char* string);
  static size_t PrintNoNewline(const char* string);

  static size_t Print(int number);
  static size_t PrintNoNewline(int number);

  static size_t Print(const char* string, int value);

#if defined(MENLO_ATMEGA)
  //
  // AtMega's with Harvard architecture and little RAM place
  // most strings in code space, which must be accessed using
  // special instructions generated through __FlashStringHelper().
  //
  // This is done with the F() macro such as:
  //
  // Print(F("string"));
  //
  static size_t Print_P(PGM_P);
  static size_t Print(const __FlashStringHelper*);
  static size_t Print(const __FlashStringHelper* string, int value);
  static size_t PrintNoNewline(const __FlashStringHelper* string);
#endif

  //
  // Flash panic code of a first number in the
  // upper 4 bits and a second number in the lower 4 bits.
  //
  // This returns to the caller, unlike Panic(). This allows
  // communication of attempts to configure in a retry loop
  // such as signing onto WiFi, etc.
  //
  static void FlashCode(uint8_t code);

 protected:

  static void StartSentenceIfRequired();

  static void EndSentence();

  // Determines if prefix is required
  static void PrintPrefixInternal();

  // Flash single number and return
  static void FlashNumber(uint8_t code);

  // Escape special characters as required
  static size_t EscapePrintChar(char c);

  //
  // Data
  //

  // TraceLevel
  static uint8_t TraceLevel;

  // Synchronous
  static uint8_t Synchronous;

  // Default to no pin defined as 0xFF
  static uint8_t PanicPinNumber;

  // Display loops during a panic before reset
  static uint8_t PanicDisplayLoops;

  //
  // If a prefix is configured its placed in front of each
  // message. This allows debug output to not confuse a
  // command protocol by configuring it to use an escape
  // sequence.
  //
  static char* Prefix;

  //
  // A NMEA 0183 debug message sentence is started for each
  // new debug line until a '\n' is received.
  //
  // Receiving a '\n' ends the current line and sentence.
  //
  static int SentenceStarted;

  // Serial object we use for I/O
  static Stream* Port;
};

#endif // MenloDebug_h
