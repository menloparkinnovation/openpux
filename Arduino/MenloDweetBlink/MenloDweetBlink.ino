
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
 * Date: 04/30/2015
 * File: MenloDweetBlink.ino
 *
 * Blink with Dweet over serial port.
 *
 */

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

//
// MenloDweet Support
//
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetApp.h>
#include <DweetSerialChannel.h>

//
// Note: This example introduces the Application Framwork
// pattern used by MenloFramework to promote code-reuse
// and component based building of applications.
//
// Two Application Framework modules are used here:
//
// 1) DweetSerialApp - This provides support for Menlo Dweet over
//    standard serial ports.
//
// 2) DweetBlinkApp - This is the application broken down into
//    its component parts since it now may be re-used for:
//
//    Different transports/configuration such as Serial, RadioSerial, Http, etc.
//
//    Different hardware platforms that do not start with an Arduino Sketch/Project
//
//    As a component in further frameworks.
//
// All Application Framework components are in the Libraries directory.
//

//
// Application Framework support
//
#include <DweetSerialApp.h>

//
// Application Framework Class
//
#include <DweetBlinkApp.h>

//
// Hardware specialization class
//
#include "DweetBlinkAppArduino.h"

// This is the default interval since its now configurable over Dweet
#define DEFAULT_BLINK_INTERVAL (1 * 1000) // 1 second

int g_panicPin = 13;

//
// Main Application class
//
DweetBlinkAppArduino g_App;

//
// Application Framework class for MenloDweet of a serial port
//
DweetSerialApp g_AppFramework;

//
// Setup
//
void setup()
{
    //
    // Setup the hardware
    //
    HardwareSetup();

    //
    // Setup the MenloFramework
    //
    MenloFrameworkSetup();

    //
    // Setup application framework
    //
    ApplicationSetup();
}

//
// Loop
//
void loop()
{
    //
    // We don't have any other processing to do so we set the maximum
    // polltime.
    //
    // Sleep can be shorter due to waking up due to application set timers
    // such as the blink interval.
    //
    MenloFramework::loop(MAX_POLL_TIME);
}

void
HardwareSetup()
{
    // set pinmode as we are using it for flashing the light in our main loop
    if (g_panicPin != (-1)) {
        pinMode(g_panicPin, OUTPUT);     
        digitalWrite(g_panicPin, LOW);        
    }
}

//
// Setup application framework.
//
void
ApplicationSetup()
{
  //
  // Application class Configuration
  //
  BlinkConfiguration appConfig;

  //
  // Application Framework Configuration
  //
  DweetSerialAppConfiguration config;

  MenloDebug::Print(F("MenloDweetBlink"));

  //
  // Initialize the application class
  //
  appConfig.pinNumber = g_panicPin;
  appConfig.interval = DEFAULT_BLINK_INTERVAL;

  g_App.Initialize(&appConfig);

  ResetWatchdog();

  //
  // Initialize the Application Framework
  //

  config.dweetApp = &g_App;

  g_AppFramework.Initialize(&config);


  //
  // We check the memory monitor at the end of initialization
  // in case any initialization routines overflowed the stack or heap.
  //
  MenloMemoryMonitor::CheckMemory(__LINE__);

  return;
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
