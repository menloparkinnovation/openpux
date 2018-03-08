
//
// Cable with marker is rain sensor.
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

/*
 * Date: 05/13/2015
 * File: WeatherStation
 *
 * WeatherStation using SparkFun WeatherShield or equivalent.
 */

//
// Arduino Headers
//
#include <Arduino.h>
#include <Wire.h>

#include "SparkFunMPL3115A2.h" //Pressure sensor
#include "HTU21D.h" //Humidity sensor
#include <SoftwareSerial.h> //Needed for GPS
#include <TinyGPS++.h> //GPS parsing

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
// Application Framework support
//
#include <DweetSerialApp.h>

//
// Application Framework Class
//
#include <WeatherStationApp.h>

//
// Hardware specialization class
//
// Library/WeatherShieldHardware/WeatherShieldHardware.h
//
#include "WeatherShieldHardware.h"

// This is the default interval but is configurable over Dweet
#define DEFAULT_UPDATE_INTERVAL (30 * 1000) // 30 seconds

// This is the sensor sample/averaging interval but is configurable over Dweet
#define DEFAULT_SAMPLE_INTERVAL (1 * 1000) // 1 second

// Pin 13 is currently unused by WeatherShield
int g_panicPin = 13;

//
// Hardware implementation class
//

WeatherShieldHardware g_hardware;

//
// Main Application class
//
WeatherStationApp g_App;

//
// Application Framework class for MenloDweet of a serial port
//
DweetSerialApp g_AppFramework;

// Arduino 1.6.8 now requires forward declarations like a proper C/C++ compiler.
void HardwareSetup();
void ApplicationSetup();
void MenloFrameworkSetup();

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
    WeatherShieldHardwareConfig config;

    // set pinmode as we are using it for flashing the light in our main loop
    if (g_panicPin != (-1)) {
        pinMode(g_panicPin, OUTPUT);     
        digitalWrite(g_panicPin, LOW);        
    }

    //
    // Setup the hardware configuration
    //
    // Note: SDA/SDC pins are used for the Two Wire (Wire.h)
    // communication to the temperature, humidity, and altitude sensors.
    //
    // In addition the GPS pins are statically defined within
    // the hardware class due to configuration issues with SoftwareSerial.
    // gpsTxPin == 4, gpsRxPin == 5
    //

    // Analog pins
    config.windDirPin = A0;
    config.lightPin = A1;
    config.batPin = A2;
    config.referencePin = A3;
    config.solarPin = A4;

    // Digital pins
    config.rainPin = 2;
    config.windSpeedPin = 3;
    config.gpsTxPin = 4; // Fixed in WeatherShieldHardware.cpp
    config.gpsRxPin = 5; // Fixed in WeatherShieldHardware.cpp
    config.gpsPowerPin = 6;
    config.stat1Pin = 7; // blue LED
    config.stat2Pin = 8; // green LED

    //
    // Initialize the hardware specialization class
    //

    g_hardware.Initialize(&config);
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
  WeatherStationConfiguration appConfig;

  //
  // Application Framework Configuration
  //
  DweetSerialAppConfiguration config;

  MenloDebug::Print(F("WeatherStation"));

  //
  // Initialize the WeatherStation application class
  //
  appConfig.updateInterval = DEFAULT_UPDATE_INTERVAL;
  appConfig.sampleInterval = DEFAULT_SAMPLE_INTERVAL;

  g_App.Initialize(&appConfig, &g_hardware);

  ResetWatchdog();

  //
  // Initialize the Application Framework
  //

  g_AppFramework.Initialize(&config);

  //
  // Set our Dweet channel on the application object.
  //
  // This is used by the WeatherStation application to send standard
  // NMEA 0183 formatted messages asynchronously from received
  // Dweet requests.
  //
  g_App.SetDweet(g_AppFramework.GetDweet());

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
  frameworkConfig.heapSize = 64;
  frameworkConfig.stackSize = 320;
  frameworkConfig.guardRegionSize = 16;
  frameworkConfig.enableUsageProfiling = true;

  // Serial port configuration. Defaults to Serial0
  frameworkConfig.baudRate = 9600;

  // Panic Pin
  frameworkConfig.panicPin = g_panicPin;

  MenloFramework::setup(&frameworkConfig);
}
