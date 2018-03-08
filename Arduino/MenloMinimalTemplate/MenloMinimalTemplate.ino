
/*
 * MenloMinimal Template.
 *
 * MenlPark Innovation LLC 01/28/2015
 *
 * Copyright (C) Menlo Park Innovation LLC 01/2015
 */

//
// This is to benchmark minimum Menlo classes sizes.
//

//
// For Arduino UNO
//

//
// Arduino libraries
//

// watchdog
#include <avr/wdt.h>

//
// MenloFramework
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>

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

//
// Classes
//

void setup()
{
  int result;

  // Enable watchdog for the maximum 8 seconds
  wdt_enable(WDTO_8S);

  wdt_reset();

  pinMode(PanicPin, OUTPUT);     

  Runtime_Initialize();

  // Tell watchdog we are still alive
  wdt_reset();

  // Tell watchdog we are still alive
  wdt_reset();

  MenloMemoryMonitor::CheckMemory(__LINE__);
}

void loop() {

    static int loopCount = 0;

    wdt_reset();

    MenloMemoryMonitor::CheckMemory(__LINE__);
    
    digitalWrite(PanicPin, HIGH);

    delay(g_loopWaitTime); // Delays are in ms
    wdt_reset();

    digitalWrite(PanicPin, LOW);

    delay(g_loopWaitTime); // Delays are in ms
    wdt_reset();

    MenloDebug::PrintNoNewline(F("loop() "));
    MenloDebug::PrintHex(loopCount);

    loopCount++;
}

//
// Initialize Runtime
//
void Runtime_Initialize()
{
  Serial.begin(9600);

  MenloDebug::Init(&Serial);

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
