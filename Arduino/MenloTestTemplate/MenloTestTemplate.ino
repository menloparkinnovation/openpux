
/*
 * MenloTest Template.
 *
 * MenlPark Innovation LLC 01/07/2015
 *
 * Copyright (C) Menlo Park Innovation LLC 01/2015
 */

//
// For Arduino UNO
//

//
// Arduino libraries
//

// watchdog
//#include <avr/wdt.h>

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//

#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>

// This kills it. Note: Arduino includes anyway, even if commented out
// with /* */, but // will override.
#include <MenloConfigStore.h>

#define DBG_PRINT_ENABLED 1

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define DBG_PRINT_INT(x) (MenloDebug::Print(x))
#define DBG_PRINT2(x, y) (MenloDebug::Print(F(x)) && MenloDebug::Print(y))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT2(x, y)
#endif

const uint8_t PanicPin = 13;

// Loop Time
long g_loopWaitTime = (5 * 1000); // Delays are in ms

// Debug support
bool g_Debug = true;

char* g_debugPrefix = "$PDBG,PRINT=";

//
// Classes
//

// Default is the Dweet protocol over NMEA 0183
char* g_prefix = "$PDWT";

void setup()
{
  int result;

  Serial.begin(9600);

  Serial.println(F("Serial.begin initialized"));

  // Enable watchdog for the maximum 8 seconds
  wdt_enable(WDTO_8S);

  wdt_reset();

  pinMode(PanicPin, OUTPUT);     

  Runtime_Initialize();

  pinMode(PanicPin, OUTPUT);     

  // Tell watchdog we are still alive
  wdt_reset();

  MenloDebug::Print(F("MenloTestTemplate: 04/18/2015 08:44 AM"));

  // Tell watchdog we are still alive
  wdt_reset();

  MenloMemoryMonitor::CheckMemory(__LINE__);
}

void loop() {

//  Serial.println(F("loop..."));
//  delay(1000);

    bool hadData;
    static int loopCount = 0;

    wdt_reset();

    MenloMemoryMonitor::CheckMemory(__LINE__);
    
    digitalWrite(PanicPin, HIGH);

    MenloDebug::Print(F("LIGHT ON"));

    delay(g_loopWaitTime); // Delays are in ms
    wdt_reset();

    digitalWrite(PanicPin, LOW);

    MenloDebug::Print(F("LIGHT OFF"));

    delay(g_loopWaitTime); // Delays are in ms
    wdt_reset();

    MenloDebug::PrintNoNewline(F("loop() "));
    MenloDebug::PrintHex(loopCount);

    loopCount++;

    hadData = false;
}

//
// Initialize Runtime
//
void Runtime_Initialize()
{
  Serial.begin(9600);

  //Serial.println(F("first serial output"));
  //Serial.begin(9600);

  //Serial.println(F("first serial output"));

  MenloDebug::Init(&Serial);

  Serial.println(F("MenloDebug initialized"));

  //
  // Set synchronous mode for debugging hard problems
  // since a reset/reboot will lose the last messages.
  //
  MenloDebug::SetSynchronous(0x01);

  MenloDebug::SetPrefix(g_debugPrefix);

  MenloDebug::Print(F("Runtime_Initialize...."));

  MenloDebug::ConfigurePanicPin(PanicPin);

  MenloMemoryMonitor::Init(
      650, // maximum dynamic heap size
      192, // maximum dynamic stack size
      16,  // buffer space to aid in detection of deep overlows
      true // enable detailed memory usage profiling
      );

  MenloMemoryMonitor::CheckMemory(__LINE__);

  if (g_Debug) {
      // Report current detailed memory usage
      MenloMemoryMonitor::ReportMemoryUsage(__LINE__);
  }
}
