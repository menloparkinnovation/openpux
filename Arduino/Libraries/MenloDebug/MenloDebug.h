
//
// Where various internal debug settings may be set
//
// This is generally left on since its cost is small and list corruptions
// can be very hard to debug.
//
// MenloDispatchObject.h
// #define MENLOEVENT_DEBUG 1
//

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

//
// Master Debug for on off
//
// This is really an effort to enable controlled debugging
// for small memory controllers.
//

#if defined(ESP8266)
// Debugging ESP8266 platform
#define MASTER_DEBUG 1
#define MASTER_DEBUG_DETAILED 1
#define MASTER_DEBUG_DETAILED_MEMORY_TRACING 1
#define MASTER_DEBUG_ERROR_PATHS 1
#define MASTER_DEBUG_DWEET_ERROR_PATHS 1
#else
#define MASTER_DEBUG 0
#define MASTER_DEBUG_DETAILED 0
#define MASTER_DEBUG_DETAILED_MEMORY_TRACING 0
#define MASTER_DEBUG_ERROR_PATHS 0
#define MASTER_DEBUG_DWEET_ERROR_PATHS 0
#endif

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

extern "C" void MenloDebug_AssertionFailed(int module, int line);

#define DEBUG_FALSE 0

#if DEBUG
#define DEBUG_ASSERT(module, x) { if(!(x)) MenloDebug_AssertionFailed(module, __LINE__); }
#else
#define DEBUG_ASSERT(x) 
#endif

//
// Tracing Defines
//

#define TRACE_ALWAYS    0x01
#define TRACE_FRAMEWORK 0x02
#define TRACE_DWEET     0x04
#define TRACE_RADIO     0x08
#define TRACE_APP1      0x10
#define TRACE_APP2      0x20
#define TRACE_APP3      0x40
#define TRACE_APP4      0x80

//
// These tracing templates are selectively compiled based
// on the size of the target platform, and/or project
// settings.
//
// This allows general field tracing to be available for
// platforms that can afford the small overhead, but compile
// out for large projects on tiny platforms that may use
// very selective tracing.
//

#if BIG_MEM
#define SHIP_TRACE_ENABLED 1
#else
#define SHIP_TRACE_ENABLED 0
#endif

#if SHIP_TRACE_ENABLED
#define SHIP_TRACE(l, x)           (MenloDebug::Trace(l, x))
#define SHIP_TRACE_STRING(l, x, d) (MenloDebug::TraceString(l, x, d))
#define SHIP_TRACE_INT(l, x, d)    (MenloDebug::TraceInt(l, x, d))
#define SHIP_TRACE_LONG(l, x, d)   (MenloDebug::TraceLong(l, x, d))
#define SHIP_TRACE_BYTE(l, x, d)   (MenloDebug::TraceByte(l, x, d))
#else
#define SHIP_TRACE(l, x)
#define SHIP_TRACE_STRING(l, x, d)
#define SHIP_TRACE_INT(l, x, d)
#define SHIP_TRACE_LONG(l, x, d)
#define SHIP_TRACE_BYTE(l, x, d)
#endif

//
// Note: Introducing Trace() calls adds 1622 bytes to AtMega328
// builds. If all Trace() calls are not compiled by conditional
// macro's this code is not referenced.
//
// Core tracing is on by default when BIG_MEM is defined, as these
// are represented as larger program space microcontrollers that
// can support field tracing.
//
// Smaller applications can support field tracing with AtMega328's
// by accounting for this overhead.
//

// Debug class that can be called from C++
class MenloDebug : MenloObject {
 public:

  // Call from setup()
  static void Init();

  // Caller can specify a pre-initialized port
  static void Init(Stream*);

  // Allows setting a Prefix in front of new messages
  static void SetPrefix(char*);

  //
  // Set synchronous output. Useful when a hard to debug
  // crash or reset occurs before debugging messages have
  // been sent out the serial port.
  //
  static uint8_t SetSynchronous(uint8_t syncOn);

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

  //
  // Note: This always prints 16 bits "0000" even if the
  // architecture is a 32 bit int.
  //
  static size_t PrintHex(int number);
  static size_t PrintHexNoNewline(int number);

  //
  // This will print the natural size of a pointer as int16 or int32
  //
  static size_t PrintHexPtr(void* ptr);
  static size_t PrintHexPtrNoNewline(void* ptr);

  static size_t PrintHexByte(uint8_t number);
  static size_t PrintHexByteNoNewline(uint8_t number);

//
// Always defined now. AtMega328 will not link routines into your
// code unless called on Arduino IDE 1.6.8 or later.
//
// Costs 174 bytes of code space for first call/reference, none
// after, except for call site invoke code.
//
//#if defined(MENLO_BOARD_RFDUINO) || (MENLO_ESP8266) || (MENLO_ARM32)
  //
  // For 32 bit architectures print the natural value in one call.
  //
  static size_t PrintHex32(uint32_t number);
  static size_t PrintHex32NoNewline(uint32_t number);
//#endif

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

#if defined(MENLO_ATMEGA) || (MENLO_BOARD_RFDUINO) || (MENLO_ESP8266) || (MENLO_ARM32)

  //
  // AtMega's with Harvard architecture and little RAM place
  // most strings in code space, which must be accessed using
  // special instructions generated through __FlashStringHelper().
  //
  // This is done with the F() macro such as:
  //
  // Print(F("string"));
  //
  // Other architectures even though ARM or Tensilica based provide
  // similar routines for accessing strings in flash memory, which
  // may have restrictions compared to general load/store instructions.
  //

  static size_t Print_P(PGM_P);
  static size_t Print(const __FlashStringHelper*);
  static size_t Print(const __FlashStringHelper* string, int value);
  static size_t PrintNoNewline(const __FlashStringHelper* string);

  static void TraceString(uint8_t level, uint8_t code, PGM_P string);
#else
  static size_t Print_P(PGM_P s) {
      return MenloDebug::Print(s);
  }
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

  //
  // Tracing
  //
  // Note: update tracecodes.json to indicate mapping from trace
  // code and data to its message and data type.
  //

  static uint8_t GetTraceMask();

  static void SetTraceMask(uint8_t mask);

  static void SetTraceBuffer(uint8_t* buffer, int size, int index);

  static void GetTraceBuffer(uint8_t** buffer, int* size, int* index);

  static void SetFormatBuffer(uint8_t* formatBuffer, int formatBufferSize);

  static void GetFormatBuffer(uint8_t** buffer, int* size);

  //
  // The upper bit of the trace code is reserved to indicate
  // whether it has data (bit 7 == 1) or not (bit 7 == 0).
  //
  // This is set automatically by the trace routines so don't use
  // it in your modules or message confusion may result.
  //

  //
  // Simplest trace with just a code
  //
  static void Trace(uint8_t level, uint8_t code);

  // Trace with a buffer and size
  static void Trace(uint8_t level, uint8_t code, uint8_t size, uint8_t* data);

  //
  // Trace functions with simple data
  //

  static void TraceByte(uint8_t level, uint8_t code, uint8_t data);
  static void TraceInt(uint8_t level, uint8_t code, int data);
  static void TraceLong(uint8_t level, uint8_t code, unsigned long data);
  static void TraceString(uint8_t level, uint8_t code, char* data);

  // TraceMask is public to allow efficient macro testing
  static uint8_t TraceMask;

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

  // Synchronous
  static uint8_t Synchronous;

  // Default to no pin defined as 0xFF
  static uint8_t PanicPinNumber;

  // Display loops during a panic before reset
  static uint8_t PanicDisplayLoops;

  static uint8_t* TraceBuffer;
  static int TraceBufferSize;
  static int TraceBufferIndex;

  static uint8_t* TraceFormatBuffer;
  static int TraceFormatBufferSize;

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
