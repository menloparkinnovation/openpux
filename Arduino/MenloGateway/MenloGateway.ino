
//
// 04/17/2016
//
// dweet SETCONFIG=BLINKINTERVAL:5000 is currently not supported
// and returns an unhandled dweet error.
//
//  - Maybe a result of the new Dweet dispatch model and need to update
//    the app to use the DweetApp registration class.
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
 * Date: 05/23/2015
 * File: MenloGateway.ino
 *
 * Main MenloGateway for nRF24L01 radios for sensors utilizing MenloFramework.
 *
 * Utilizes Smartpux DWEET's and Menlo Framework.
 */

//
// 04/17/2016
//
// Arduino Pro Mini 3.3V 8Mhz memory consumption
//
// 24,164 bytes flash, 966 bytes ram
//
// 05/24/2015
//
// Arduino Uno memory consumption
//  25,914 flash, 1565 ram
//
// SparkFun Pro Micro 3.3V 8Mhz Arduino Leonardo clone
// 28,542 flash, 1534 ram
//  - Net 2628 bytes of flash for USB Serial library
//  - 4K bootloader, so only 28,672 available
//
// After Rework for specialized GatewayApp classes:
//
// Arduino Uno memory consumption
// 24,296, 958 ram
//
// SparkFun Pro Micro 3.3V 8Mhz Arduino Leonardo clone
// 26,924 flash, 927 ram
// - available 1748 bytes flash
//

//
// MenloFramework
//

//
// Note: Arduino requires all these includes even though
// MenloFramework.h already includes them.
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloDispatchObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloPower.h>
#include <MenloTimer.h>

#include <MenloFramework.h>

// MenloRadio support
#include <MenloRadio.h>
#include <MenloRadioSerial.h>

// Libraries used by the Nordic nRF24L01 radio
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <OS_nRF24L01.h>

// Menlo Dweet Support
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetSerialChannel.h>
#include <DweetRadio.h>

// Application Framework support
#include <DweetSerialApp.h>
#include <DweetRadioGatewayApp.h>

//
// Application Class
//
#include <MenloGatewayApp.h>
#include "MenloGatewayAppHardware.h"

// This is the default interval since its now configurable over Dweet
#define DEFAULT_BLINK_INTERVAL (1 * 1000) // 1 second

//
// Nordic Radio Configuration
//
// In addition, the radio uses the SPI bus in a shared
// fashion on pins 11 (MOSI), 12 (MISO), 13 (SCK).
//

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

char* nRF24L01_defaultReceiveAddress = ADDRESS_GATE0;

// DweetRadioGatewayApp sends to the gateway from the sensor
char* g_radioSerialTransmitAddress  = ADDRESS_GATE0;

unsigned long g_sendTimeout = 1000; // 1 second

//
// Currently an 8MHZ CPU means a build for the 3.3v MenloNRFDongle
//
#if F_CPU == 8000000UL

//
// SparkFun 8Mhz 3.3V Arduino Leonardo mini-board clone
//

//
// Nordic Radio Configuration
//
// In addition, the radio uses the SPI bus in a shared
// fashion on pins 11 (MOSI), 12 (MISO), 13 (SCK).
//
// Currently IRQ (pin 2) is not connected.
//
const uint8_t nRF24L01_CSN = 7;
const uint8_t nRF24L01_CE = 8;
const uint8_t nRF24L01_IRQ = 2;

//
// Note: Pin13 which has the LED is shared with SPI
// SCK, but the MenloDebug will only (re) program it on
// a panic situation.
//
//int g_panicPin = -1;
int g_panicPin = 13;

#else

#if ARDUINO_AVR_MEGA2560

//
// Sometimes pin 10 for CSN has conflicts/errors.
// Pin 4 is more reliable.
//
// A jumper on the Menlo NRF24L01Shield allows selection of either.
//
//

const uint8_t nRF24L01_CSN = 10;  // Warning: Could be flakey in some configurations

//const uint8_t nRF24L01_CSN = 4; // alternate selection

//
// Pin 9 which is free when the SparkFun WeatherShield is installed
//
// Access it with a small jumper wire from the center pin of the CSN
// selection jumper block on the Menlo NRF24L01Shield and D9 on
// the end of the shields Arduino headers.
//

//const uint8_t nRF24L01_CSN = 9;

#else
const uint8_t nRF24L01_CSN = 10;
#endif

//
// Arduino UNO or clones
//
// 16Mhz, 5V AtMega328
//

const uint8_t nRF24L01_CE = 9;
const uint8_t nRF24L01_IRQ = 8;

#if ARDUINO_AVR_MEGA2560
int g_panicPin = (-1);
#else
int g_panicPin = (-1);
#endif

#endif

//
// Main Application class
//
//
// class DweetApp : public MenloObject
//   virtual ProcessAppCommands = 0;
//
//   class MenloGatewayApp : public DweetApp
//       ProcessAppCommands
//       m_lightToggle;
//       MenloTimer m_timer;
//
//     class MenloGatewayHardware : public MenloGatewayApp
//       SetLightState
//
MenloGatewayAppHardware g_App;

//
// Application Framework class for MenloDweet on a radio and serial port.
//
// DweetSerialApp : public MenloObject
//   DweetApp* m_dweetApp;
//   DweetSerialChannel m_dweetSerialChannel;
//   MenloDweetEventRegistration m_serialDweetEvent;
//
//   DweetRadioGatewayApp : public DweetSerialApp
//     OS_nRF24L01 m_nordic;
//     MirfHardwareSpiDriver m_MirfHardwareSpi;
//     DweetRadio m_dweetRadio;
//
DweetRadioGatewayApp g_AppFramework;

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

    MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

    //
    // Setup application framework
    //
    ApplicationSetup();

    MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);
}

//
// Loop
//
void loop()
{
    MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

    //
    // We don't have any other processing to do so we set the maximum
    // polltime.
    //
    // Sleep can be shorter due to waking up due to application set timers
    // such as the blink interval.
    //
    MenloFramework::loop(MAX_POLL_TIME);

    MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);
}

//
// Setupt the hardware
//
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
  MenloGatewayConfiguration appConfig;

  //
  // Application Framework Configuration
  //
  DweetRadioGatewayAppConfiguration config;

  //
  // Initialize the Application State
  //
  appConfig.pinNumber = g_panicPin;
  appConfig.interval = DEFAULT_BLINK_INTERVAL;

  MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

  g_App.Initialize(&appConfig);

  MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

  ResetWatchdog();

  //
  // Initialize the Application Framework
  //

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

  MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

  g_AppFramework.Initialize(&config);

  MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

  MenloDebug::Print(F("MenloGateway 04/10/2016"));

  ResetWatchdog();

  //
  // We check the memory monitor at the end of initialization
  // in case any initialization routines overflowed the stack or heap.
  //
  MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

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
  frameworkConfig.stackSize = 370;
  frameworkConfig.guardRegionSize = 16;
  frameworkConfig.enableUsageProfiling = true;

  // Serial port configuration. Defaults to Serial0
  frameworkConfig.baudRate = 9600;

  // Panic Pin
  frameworkConfig.panicPin = g_panicPin;

  MenloFramework::setup(&frameworkConfig);

  MenloMemoryMonitor::CheckMemory(LineNumberBaseApp + __LINE__);

#if ARDUINO_AVR_PROMICRO8
  //
  // Without delay tends to hang on startup
  // Delay based smash of guard region at 0x800, 0x801 to 0000
  //
  ResetWatchdog();
  delay(5000);
  ResetWatchdog();
#endif
}
