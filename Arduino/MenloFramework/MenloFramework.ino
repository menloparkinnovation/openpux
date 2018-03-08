
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
 * File: MenloFramework.ino
 *
 * MenloFrameowrk template.
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

void MenloFrameworkSetup();

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
    MenloFrameworkSetup();
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

//
// Setup the core MenloFramework
//
void
MenloFrameworkSetup()
{

  //
  // MenloFramework configuration
  //
  MenloFrameworkConfiguration frameworkConfig;

  //
  // The initial framework is configured first
  // to allow use of framework facilities during
  // application hardware and state configuration.
  //

  // Set this to true for hard to debug hangs, resets, and reboots
  //frameworkConfig.synchronousDebug = true;
  frameworkConfig.synchronousDebug = false;

  //
  // Set watchdog which will reset the board after 8 seconds
  // if the application becomes unresponsive.
  //
  frameworkConfig.enableWatchdog = true;

  //
  // Setup memory monitor configuration
  //
  // These values work for MemoyRadioSensorApp, but may have
  // to be adjusted based on actual application heap and stack usage.
  //
  frameworkConfig.heapSize = 128;
  frameworkConfig.stackSize = 320;
  frameworkConfig.guardRegionSize = 16;
  frameworkConfig.enableUsageProfiling = true;

  // Serial port configuration. Defaults to Serial0
  frameworkConfig.baudRate = 9600;

  // Panic Pin
  frameworkConfig.panicPin = g_panicPin;

  MenloFramework::setup(&frameworkConfig);
}
