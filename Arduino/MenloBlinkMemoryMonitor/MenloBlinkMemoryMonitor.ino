
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
 * Date: 05/20/2015
 * File: MenloBlinkMemoryMonitor.ino
 *
 * Most basic use of MenloFramework for debug and memory monitor
 *
 */

#if ARDUINO_ARCH_AVR

//
// AVR Libraries
//

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#endif

//
// MenloFramework
//

//
// Note: Arduino requires all these includes even though
// MenloFramework.h already includes them.
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloPower.h>
#include <MenloFramework.h>
#include <MenloDispatchObject.h>
#include <MenloTimer.h>
#include <MenloNMEA0183.h>
#include <MenloDweet.h>

int g_panicPin = 13;

//
// Setup
//
void setup()
{
    //
    // Setup the hardware
    //
    pinMode(g_panicPin, OUTPUT);     
    digitalWrite(g_panicPin, LOW);        

    //
    // Setup the MenloFramework
    //
    Serial.begin(9600);

    MenloDebug::Init(&Serial);

    MenloDebug::ConfigurePanicPin(g_panicPin);

    //
    // Memory Monitor
    //
    MenloMemoryMonitor::Init(
        128, // maximum dynamic heap size
        380, // maximum dynamic stack size
        16,  // buffer space to aid in detection of deep overlows
        true // enable detailed memory usage profiling
        );

    MenloMemoryMonitor::CheckMemory(__LINE__);
}

//
// Loop
//
void loop()
{
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
}
