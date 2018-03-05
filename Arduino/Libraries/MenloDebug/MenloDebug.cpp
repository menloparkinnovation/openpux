
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

#include <MenloPlatform.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>

// Defaults to 0x01, default tracing enabled
uint8_t MenloDebug::TraceMask = 0x01;
//uint8_t MenloDebug::TraceMask = 0xFF;

// Synchronous is off by default
uint8_t MenloDebug::Synchronous = 0x00;

// 0xFF defaults to no pin assigned.
uint8_t MenloDebug::PanicPinNumber = 0xFF;

// After 2 times displaying the panic code, reset
uint8_t MenloDebug::PanicDisplayLoops = 2;

// Tracing support
uint8_t* MenloDebug::TraceBuffer = NULL;
int MenloDebug::TraceBufferSize = 0;
int MenloDebug::TraceBufferIndex = 0;

// Tracing format buffer
uint8_t* MenloDebug::TraceFormatBuffer = NULL;
int MenloDebug::TraceFormatBufferSize = 0;

Stream* MenloDebug::Port = NULL;

char* MenloDebug::Prefix = NULL;

int MenloDebug::SentenceStarted = 0;

//
// If a Prefix is set, messages are sent in NMEA 0183
// format with checksums for reliable decoding by
// programs.
//
// To minimize memory the stream version of MenloNMEA0183
// is used.
//
MenloNMEA0183Stream g_debug_nmea;

uint8_t
MenloDebug::SetSynchronous(uint8_t syncOn)
{
  uint8_t prev = MenloDebug::Synchronous;
  MenloDebug::Synchronous = syncOn;
  return prev;
}

// Set default port
void
MenloDebug::Init()
{
  Serial.begin(9600);

  // Set the default port
  MenloDebug::Init(&Serial);
}

//
// The Serial port is already initialized.
//
void
MenloDebug::Init(Stream* port)
{
  MenloDebug::Port = port;
}

void
MenloDebug::SetPrefix(char* prefix)
{
  MenloDebug::Prefix = prefix;

  // Initialize to standard NMEA0183 maximum length
  g_debug_nmea.Initialize(80);
}

//
// Start a NMEA 0183 debug sentence if required.
//
void
MenloDebug::StartSentenceIfRequired()
{
  if ((MenloDebug::Prefix != NULL) &&
      (MenloDebug::SentenceStarted == 0)) {

    // Start the NMEA 0183 sentence
    g_debug_nmea.startSentence(MenloDebug::Prefix);

    MenloDebug::Port->print(MenloDebug::Prefix);
    MenloDebug::SentenceStarted = 1;
  }
}

//
// End the current Sentence
//
void
MenloDebug::EndSentence()
{
  char buffer[5];

  ResetWatchdog();

  if (MenloDebug::Prefix == NULL) {
    buffer[0] = '\n';
    buffer[1] = '\0';
    MenloDebug::Port->write((const uint8_t*)&buffer[0], 1);
    MenloDebug::SentenceStarted = 0;

    if (MenloDebug::Synchronous) {
        // 9600 baud == 960 cps, 64 byte buffer is out in 66ms
        delay(500);
    }

    return;
  }

  // Get the NMEA 0183 checksum and ending sequence.
  g_debug_nmea.endSentence(&buffer[0]);

  MenloDebug::Port->write((const uint8_t*)&buffer[0], sizeof(buffer));

  MenloDebug::SentenceStarted = 0;

  if (MenloDebug::Synchronous) {
      // 9600 baud == 960 cps, 64 byte buffer is out in 66ms
      delay(500);
  }
}

//
// Print a hex string representing binary data with newline at the end.
//
size_t
MenloDebug::PrintHexString(uint8_t* buffer, int length)
{
  MenloDebug::PrintHexStringNoNewline(buffer, length);
  MenloDebug::EndSentence();

  return 1;
}

//
// Print a hex string representing binary data.
//
size_t
MenloDebug::PrintHexStringNoNewline(uint8_t* buffer, int length)
{
  char buf[2];
  int index;
  uint8_t data;

  MenloDebug::StartSentenceIfRequired();

  for (index = 0; index < length; index++ ) {
      data = buffer[index];
      MenloUtility::UInt8ToHexBuffer(data, &buf[0]);

      g_debug_nmea.addChar(buf[0]);
      MenloDebug::Port->write(buf[0]);

      g_debug_nmea.addChar(buf[1]);
      MenloDebug::Port->write(buf[1]);
  }

  // The sentence is not ended as further data is expected
  return 1;
}

// Print with a newline which ends the current sentence
size_t
MenloDebug::PrintHex(int number)
{
  MenloDebug::PrintHexNoNewline(number);
  MenloDebug::EndSentence();

  return 1;
}

size_t
MenloDebug::PrintHexNoNewline(int number)
{
  char buf[4];
  char c;
  int index;

  MenloDebug::StartSentenceIfRequired();

  MenloUtility::UInt16ToHexBuffer(number, &buf[0]);

  for (index = 0; index < 4; index++ ) {
    c = buf[index];
    g_debug_nmea.addChar(c);
    MenloDebug::Port->write(c);
  }

  // The sentence is not ended as further data is expected
  return 1;
}

size_t
MenloDebug::PrintHexPtr(void* ptr)
{
  MenloDebug::PrintHexPtrNoNewline(ptr);
  MenloDebug::EndSentence();

  return 1;
}

size_t
MenloDebug::PrintHexPtrNoNewline(void* ptr)
{
#if defined(MENLO_BOARD_RFDUINO) || (MENLO_ESP8266) || (MENLO_ARM32)
    // 32 bit architecture
    MenloDebug::PrintHex32NoNewline((uint32_t)ptr);
#else
    // 16 bit architecture
    MenloDebug::PrintHexNoNewline((uint16_t)ptr);
#endif

  return 1;
}

//#if defined(MENLO_BOARD_RFDUINO) || (MENLO_ESP8266) || (MENLO_ARM32)
// Print with a newline which ends the current sentence
size_t
MenloDebug::PrintHex32(uint32_t number)
{
  MenloDebug::PrintHex32NoNewline(number);
  MenloDebug::EndSentence();

  return 1;
}

size_t
MenloDebug::PrintHex32NoNewline(uint32_t number)
{
  char buf[8];
  char c;
  int index;

  MenloDebug::StartSentenceIfRequired();

  MenloUtility::UInt32ToHexBuffer(number, &buf[0]);

  for (index = 0; index < 8; index++ ) {
    c = buf[index];
    g_debug_nmea.addChar(c);
    MenloDebug::Port->write(c);
  }

  // The sentence is not ended as further data is expected
  return 1;
}
//#endif

size_t
MenloDebug::PrintHexByte(uint8_t number)
{
  MenloDebug::PrintHexByteNoNewline(number);
  MenloDebug::EndSentence();

  return 1;
}

size_t
MenloDebug::PrintHexByteNoNewline(uint8_t number)
{
  char buf[4];
  char c;
  int index;

  MenloDebug::StartSentenceIfRequired();

  MenloUtility::UInt8ToHexBuffer(number, &buf[0]);

  for (index = 0; index < 2; index++ ) {
    c = buf[index];
    g_debug_nmea.addChar(c);
    MenloDebug::Port->write(c);
  }

  // The sentence is not ended as further data is expected
  return 1;
}

// Print with a newline which ends the current sentence
size_t
MenloDebug::Print(const char* string)
{
  MenloDebug::PrintNoNewline(string);
  MenloDebug::EndSentence();

  return 1;
}

//
// Escape any characters if NMEA 0183 is active.
//
// TODO: This impacts up front size calculations for strings.
//
size_t
MenloDebug::EscapePrintChar(char c)
{
  char* buf;

  // If we are not NMEA, just output it now
  if (MenloDebug::Prefix == NULL) {
    MenloDebug::Port->write(c);
    return 1;
  }

  //
  // The following special characters must be escaped
  // on a NMEA 0183 stream.
  //
  // '$' ',' '*' '\r' '\n'
  //
  // '%'  = %25
  //' $'  = %24
  //' ,'  = %2c
  // '*'  = %2a
  // '\r' = %0d
  // '\n' = $0a
  //
  // Also Dweet relies on '=' and ':' being special as well
  //
  // '='  = %3d
  // ':'  = %3a
  //

  switch(c) {

    // We add the '%' escape character as well
  case '%':
    buf = "%25";
    break;

    // NMEA 0183 special sentence, command structure characters
  case '$':
    buf = "%24";
    break;

  case '*':
    buf = "%2A";
    break;

  case ',':
    buf = "%2C";
    break;

  case '\r':
    buf = "%0D";
    break;

  case '\n':
    buf = "%0A";
    break;

  case '=':
    buf = "%3D";
    break;

  case ':':
    buf = "%3A";
    break;

  default:
    g_debug_nmea.addChar(c);
    MenloDebug::Port->write(c);
    return 1;
  }

  g_debug_nmea.addChar(buf[0]);
  MenloDebug::Port->write(buf[0]);

  g_debug_nmea.addChar(buf[1]);
  MenloDebug::Port->write(buf[1]);

  g_debug_nmea.addChar(buf[2]);
  MenloDebug::Port->write(buf[2]);

  return 3;
}

size_t
MenloDebug::PrintNoNewline(const char* string)
{
  char c;
  const char* p = string;

  MenloDebug::StartSentenceIfRequired();

  while(1) {
    c = *p++;
    if (c == 0) break;

    //
    // Escape special characters
    //
    EscapePrintChar(c);
  }

  // The sentence is not ended as further data is expected
  return 1;
}

#if defined(MENLO_ATMEGA) || (MENLO_BOARD_RFDUINO) || (MENLO_ESP8266) || (MENLO_ARM32)

//
// A PGM_P string in a table or other data is not setup as
// a _FlashStringHelper().
//
// So we pull the information directly from the instruction/flash space.
//
size_t
MenloDebug::Print_P(PGM_P string)
{
  int index;
  int length;
  char buf[2];

  length = strlen_P(string);

  for(index = 0; index < length; index++) {
 
      memcpy_P(&buf[0], string, 1);
      buf[1] = '\0';

      MenloDebug::PrintNoNewline(buf);

      string++;
  }

  //
  // Can't use the _FlashStringHelper version or we will call ourselves
  // again and recurse off the stack
  //
  MenloDebug::Print("");

  return 1;
}

//
// This function must pull characters from the code space.
//
size_t
MenloDebug::PrintNoNewline(const __FlashStringHelper* string)
{
  unsigned char c;
  PGM_P p = reinterpret_cast<PGM_P>(string);

  MenloDebug::StartSentenceIfRequired();

  while(1) {
    c = pgm_read_byte(p++);
    if (c == 0) break;

    //
    // Escape special characters
    //
    EscapePrintChar(c);
  }

  // The sentence is not ended as further data is expected
  return 1;
}

size_t
MenloDebug::Print(const __FlashStringHelper* string)
{
  MenloDebug::PrintNoNewline(string);

  //
  // Can't use the _FlashStringHelper version or we will call outselves
  // again and recurse off the stack
  //
  MenloDebug::Print("");

  return 1;
}

size_t
MenloDebug::Print(const __FlashStringHelper* string, int number)
{
  MenloDebug::PrintNoNewline(string);
  MenloDebug::PrintHex(number);

  return 1;
}

void
MenloDebug::TraceString(uint8_t level, uint8_t code, PGM_P string)
{
    uint8_t buf[16];
    int length;

    if (string == NULL) {
        Trace(level, code, 0, NULL);
    }
    else {

        length = strlen_P(string);
        if (length > sizeof(buf) - 1) {
            length = sizeof(buf) - 1;
        }

        memcpy_P(&buf[0], string, length);
        buf[length] = '\0';

        Trace(level, code, length, buf);
    }
}

#endif

size_t
MenloDebug::Print(const char* string, int number)
{
  MenloDebug::PrintNoNewline(string);
  MenloDebug::PrintHex(number);

  return 1;
}

size_t
MenloDebug::Print(int number)
{
  MenloDebug::PrintHex(number);
  return 1;
}

size_t
MenloDebug::PrintNoNewline(int number)
{
  MenloDebug::PrintHexNoNewline(number);
  return 1;
}

void
MenloDebug::ConfigurePanicPin(int pinNumber)
{
  if (pinNumber == (-1)) {
     MenloDebug::PanicPinNumber = 0xFF;
  }
  else {
     MenloDebug::PanicPinNumber = pinNumber;
  }
}

void
MenloDebug::ConfigurePanicDisplayLoops(uint8_t number)
{
  MenloDebug::PanicDisplayLoops = number;
}

void
MenloDebug::DisplayErrorCode(uint8_t code)
{
    // Provide serial output in case a debug monitor is attached
    ResetWatchdog();
    PrintNoNewline(F("ErrorCode: 0x"));
    PrintHex(code);
    Print(F("\n"));

    ResetWatchdog();

    // Flash the panic code on the light
    FlashCode(code);
}

void
MenloDebug::FlashCode(uint8_t code)
{
  uint8_t tmp;

  // High digit
  tmp = (code >> 4) & 0x0F;
  FlashNumber(tmp);

  // wait for a 2 second gap between each digit
  delay(2000);

  // Low digit
  tmp = code  & 0x0F;
  FlashNumber(tmp);
}

//
// Routines below here have platform specific calls
//

//
// Go into a no return loop displaying the error code.
//
void
MenloDebug::Panic(uint8_t code)
{
  int index;

  //
  // We don't now the watchdog state, so we turn it on
  // for its maximum of 8 seconds so we have enough
  // time to display our message. It also ensures its
  // on in case the message display hangs due to corruption.
  //
  EnableWatchdog();

  ResetWatchdog();

  for(index = 0; index < MenloDebug::PanicDisplayLoops; index++) {

    PrintNoNewline(F("Panic: Code 0x"));
    PrintHex(code);

    DisplayErrorCode(code);

    ResetWatchdog();

    // Wait 5 seconds and do it again
    delay(5000);

    ResetWatchdog();
  }

  //
  // We now want the watchdog to reset the device
  //
  while (1) {

    //
    // Display a rapid flash while waiting for reset
    // A rapid flash will mean the reset did not occur
    //
    if (MenloDebug::PanicPinNumber != 0xFF) {
        pinMode(MenloDebug::PanicPinNumber, OUTPUT);
        digitalWrite(MenloDebug::PanicPinNumber, HIGH);
        delay(100);
        digitalWrite(MenloDebug::PanicPinNumber, LOW);
        delay(100);
    }
  }
}

void
MenloDebug::FlashNumber(uint8_t code)
{
  if (MenloDebug::PanicPinNumber == 0xFF) return;

  // 
  // On many Arduino's the pin with the LED (13) is also the
  // SPI SCK pin. This pin must remain as an OUTPUT for
  // master mode which is what is used for driving WiFi, etc.
  // 
  // An SPI slave device will recognize transitions
  // on this pin if Slave Select is on. This should not
  // be happening if we are not trying to display
  // during a WiFi or other SPI transactions.
  // 
  // For SPI, SCK LOW is de-asserted, so we must always leave
  // it in this state upon return.
  //

  //
  // We always force the pin to output since it may have been
  // configured for other use, such as PWM light control, SPI
  // device such as WiFi or the Nordic radio, etc.
  //
  // But since we are in a panic mode, we override any hardware
  // settings.
  //
  pinMode(MenloDebug::PanicPinNumber, OUTPUT);

  while(code > 0) {

    ResetWatchdog();

    // Flash on for 1/2 second
    digitalWrite(MenloDebug::PanicPinNumber, HIGH);
    delay(500);

    // Flash off for 1/2 second
    digitalWrite(MenloDebug::PanicPinNumber, LOW);
    delay(500);

    code--;
  }

  // Ensure the pin is low on exit
  digitalWrite(MenloDebug::PanicPinNumber, LOW);
}

//
// Callable from C
//
extern "C" void MenloDebug_AssertionFailed(int module, int line)
{
  //
  // This effectively halts the system on error.
  // It repeats the message to give you a chance to open
  // the Serial Monitor and see it
  //
  for (;;) {

    MenloDebug::PrintNoNewline(F("assert! M "));
    MenloDebug::PrintHexNoNewline(module);
    MenloDebug::PrintNoNewline(F(" L "));
    MenloDebug::PrintHex(line);

    MenloDebug::Panic(MenloDebugUserAssertionFailed);
  }
}

// ====== Tracing Support ======

uint8_t
MenloDebug::GetTraceMask()
{
  return MenloDebug::TraceMask;
}

void
MenloDebug::SetTraceMask(uint8_t mask)
{
  MenloDebug::TraceMask = mask;
}

//
// SetTraceBuffer
//
// A trace buffer of NULL or 0 size disables tracing.
//
// A trace buffer once set is used immediately.
//
// This allows sequences of code to be traced into a module/application
// specific buffer.
//
// Parameters:
//
//  buffer - Trace buffer to use. Recommended to be statically allocated
//           unless you are careful with heap allocation/free.
//
//           If NULL tracing is disabled.
//
//  size   - Size in bytes of the trace buffer.
//
//           0 disables tracing.
//
//  index  - Index into buffer to begin the trace.
//
//           This allows use of GetTraceBuffer()/SetTraceBuffer() to save
//           the previous trace buffer pointers, set an application or module
//           specific trace buffer, and then restore when done.
//
//           A new buffer is initialized with index 0.
//
void
MenloDebug::SetTraceBuffer(uint8_t* buffer, int size, int index)
{
    // Note: NULL shuts it off
    MenloDebug::TraceBuffer = buffer;
    MenloDebug::TraceBufferSize = size;
    MenloDebug::TraceBufferIndex = index;

    //
    // The first location is reserved to indicate the tracebuffer status
    //
    if ((buffer != NULL) && (size > 0)) {
        MenloDebug::TraceBuffer[0] = 0x7E;
        MenloDebug::TraceBufferIndex++;
    }
}

//
// Return the current trace buffer
//
void
MenloDebug::GetTraceBuffer(uint8_t** buffer, int* size, int* index)
{
    *buffer = MenloDebug::TraceBuffer;
    *size = MenloDebug::TraceBufferSize;
    *index = MenloDebug::TraceBufferIndex;
}

//
// Set the format buffer.
//
// This is set by a transport, and used for retrieval.
//
// This allows a decoupling between the tracing facility and transports.
//
void
MenloDebug::SetFormatBuffer(uint8_t* formatBuffer, int formatBufferSize)
{
    MenloDebug::TraceFormatBuffer = formatBuffer;
    MenloDebug::TraceFormatBufferSize = formatBufferSize;
}

//
// Return the current format buffer
//
void
MenloDebug::GetFormatBuffer(uint8_t** buffer, int* size)
{
    *buffer = MenloDebug::TraceFormatBuffer;
    *size = MenloDebug::TraceFormatBufferSize;
}

//
// Note: Tracing is always in a compact binary format.
//
// Transports that acquire trace data can convert it to
// strings, or other data formats as required. The possible
// conversions are not burdened here to keep the core code small.
//
// The simplest conversion for a caller is to call
// GetTraceBuffer(), allocate a buffer that is 2X+1 tracebuffer
// contents indicated by index, and call MenloUtility::UInt8ToHexBuffer()
// to convert to ASCII hex format, and '\0' terminate it.
//
void
MenloDebug::Trace(uint8_t level, uint8_t code, uint8_t size, uint8_t* data)
{
    int total;
    int space;
    uint8_t* to;

    if ((MenloDebug::TraceMask & level) == 0) {
        return;
    }

    PrintNoNewline(F("T "));
    PrintHex(code);

    PrintNoNewline(F("D ptr "));
    PrintHex(((int)data) >> 16);
    PrintHex((int)data);

    PrintNoNewline(F("D size "));
    PrintHex(size);

    if (data != NULL) {
        PrintNoNewline(F("D "));
        PrintHexString(data, size);
    }

    if (MenloDebug::TraceBuffer == NULL) return;
    if (MenloDebug::TraceBufferSize == 0) return;

    if (size == 0) {

        // Just the code. Code must have upper bit clear to indicate no data.
        total = 1;
        code = code & 0x7F;
    }
    else {

        //
        // 2 bytes for code and data size byte. Code has the upper bit set
        // to indicate data.
        //

        total = size + 2;
        code = code | 0x80;
    }

    space = MenloDebug::TraceBufferSize - MenloDebug::TraceBufferIndex;
    if (total > space) {

        // MessageId 0x7F indicates a full tracebuffer
        MenloDebug::TraceBuffer[0] = 0x7F;

        Print(F("T OVF"));
        return;
    }

    to = &MenloDebug::TraceBuffer[MenloDebug::TraceBufferIndex];

    MenloDebug::TraceBufferIndex += total;

    // Code
    *to = code;
    to++;

    if (size == 0) {
        // Done
        return;
    }

    // Size
    *to = (uint8_t)size;
    to++;

    // Data
    memcpy(to, data, size);
}

//
// These allow potentially short call sites and re-used code
//

void
MenloDebug::Trace(uint8_t level, uint8_t code)
{
    Trace(level, code, 0, NULL);
}

void
MenloDebug::TraceByte(uint8_t level, uint8_t code, uint8_t data)
{
    Trace(level, code, sizeof(data), (uint8_t*)&data);
}

void
MenloDebug::TraceInt(uint8_t level, uint8_t code, int data)
{
    Trace(level, code, sizeof(data), (uint8_t*)&data);
}

void
MenloDebug::TraceLong(uint8_t level, uint8_t code, unsigned long data)
{
    Trace(level, code, sizeof(data), (uint8_t*)&data);
}

void
MenloDebug::TraceString(uint8_t level, uint8_t code, char* data)
{
    if (data == NULL) {
        Trace(level, code, 0, NULL);
    }
    else {
        Trace(level, code, strlen(data), (uint8_t*)data);
    }
}
