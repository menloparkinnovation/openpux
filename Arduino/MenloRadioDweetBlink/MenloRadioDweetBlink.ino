
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
 * Date: 05/04/2015
 * File: MenloRadioDweetBlink.ino
 *
 * Blink with Dweet over a radio and serial port.
 *
 * Provides the basis for a simple remote control application.
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

// MenloRadio support
#include <MenloRadio.h>
#include <MenloRadioSerial.h>

// Libraries used by the Nordic nRF24L01 radio
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <OS_nRF24L01.h>

//
// MenloDweet Support
//
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetApp.h>
#include <DweetSerialChannel.h>

// Dweet Radio
#include <DweetRadio.h>

//
// Application Framework support
//
#include <DweetSerialApp.h>
#include <DweetRadioSerialApp.h>

//
// Application Class
//
#include <DweetBlinkApp.h>
#include "DweetBlinkAppArduino.h"

// This is the default interval since its now configurable over Dweet
#define DEFAULT_BLINK_INTERVAL (1 * 1000) // 1 second

int g_panicPin = 13;

//
// Nordic Radio Configuration
//
// In addition, the radio uses the SPI bus in a shared
// fashion on pins 11 (MOSI), 12 (MISO), 13 (SCK).
//

// This is the wiring on an Arduino Uno using the Menlo Radio Shield
const uint8_t nRF24L01_CSN = 10;
const uint8_t nRF24L01_CE = 9;
const uint8_t nRF24L01_IRQ = 8;

const uint8_t nRF24L01_payLoadSize = 32;

// Force defaults when firmware is corrupt or for initialize
bool nRF24L01_forceDefaults = false;

// Note: Channel must be 84 or under in the US
char* nRF24L01_defaultChannel = (char*)"01";

//
// Radio endpoint addresses
//
// receiveAddress - receive address of the radio.
//
// transmitAddress - transmit address for point to point links
// such as RadioSerial.
//

// "sens0" in ASCII hex
#define ADDRESS_SENS0 (char*)"73656E7330"

// "gate0" in ASCII hex
#define ADDRESS_GATE0 (char*)"6761746530"

//
// Sensor receives on ADDRESS_SENS0
//   Sensor transmits to ADDRESS_GATE0
//
// Gateway receives on ADDRESS_GATE0
//   Gateway transmits to ADDRESS_SENS0
//

//
// The following defaults will take the gateway view
//
// Note: For a product default could be "sensor" to allow ready setup
// for an already configured gateway. To be a gateway it must be hooked
// up to USB so in that case an enable Gateway Dweet can be sent.
//
// This way out of the box sensors don't need to be connected to
// a computer for auto-configuration over the air. By definition
// gateways are, even if the computer is another embedded device
// such as a RaspberryPi, Galileo, Edison, BeagleBone, etc.
//

char* nRF24L01_defaultReceiveAddress = ADDRESS_GATE0;

// MenloRadioSerial sends to the gateway from the sensor
char* g_radioSerialTransmitAddress  = ADDRESS_GATE0;

unsigned long g_sendTimeout = 1000; // 1 second

//
// Main Application class
//
DweetBlinkAppArduino g_App;

//
// Application Framework class for MenloDweet on a radio and serial port.
//
DweetRadioSerialApp g_AppFramework;

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
  DweetRadioSerialAppConfiguration config;

  // Radio hardware configuration
  config.csnPin = nRF24L01_CSN;
  config.cePin = nRF24L01_CE;
  config.irqPin = nRF24L01_IRQ;
  config.payLoadSize = nRF24L01_payLoadSize;
  config.forceDefaults = nRF24L01_forceDefaults;
  config.defaultChannel = nRF24L01_defaultChannel;
  config.defaultReceiveAddress = nRF24L01_defaultReceiveAddress;
  config.sendTimeout = g_sendTimeout;

  // Radio Serial
  config.radioSerialTransmitAddress = g_radioSerialTransmitAddress;

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
