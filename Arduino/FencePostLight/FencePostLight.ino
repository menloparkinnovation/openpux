
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
 * Date: 11/25/2015
 * File: FencePostLight.ino
 *
 * FencePostLight application.
 *
 * Utilizes Smartpux DWEET's and Menlo Framework.
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

//
// Arduino and Debug support
//
#include <DweetArduino.h>
#include <DweetDebug.h>

// LightHouse library is used for timed light sequences
#include <MenloDispatchObject.h>
#include <MenloTimer.h>
#include <MenloLightHouse.h>
#include <LightHouseHardwareBase.h>

// Application Framework class
#include <DweetSerialApp.h>
#include <DweetRadioSerialApp.h>

// Application and hardware
#include "FencePostLightHardware.h"
#include "LightHouseApp.h"
#include "DweetLightHouse.h" // used to program light sequences

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

// DweetRadioSerialApp sends to the gateway from the sensor
char* g_radioSerialTransmitAddress  = ADDRESS_GATE0;

unsigned long g_sendTimeout = 1000; // 1 second

//
// Currently an 8MHZ CPU means a build for the sensor target hardware.
//
#if F_CPU == 8000000UL

//
// MenloSensor Target Hardware
// AtMega 328 at 8Mhz, 3.3V
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
// Menlo Sensor hardware Configuration
//

//
// Board Configuration:
//
// Smartpux/Hardware/IntegratedSensor/Version8_FenceLight/IntegratedSensor.sch
//
// Light sensor with CDS photocell and 10k resistor with bridge
// on analog input 1
//
// Moisture sensor with a 2N2222A amplifier with base supplied
// by sensor through 100 ohm resistor. A 100 ohm resistor is
// from emitter to ground. Analog input 0 is at the junction of
// the emitter.
//
// Power for both is from DIO4 to enable/disable under software
// control. Max current draw for each sensor is about 2.5ma.
//
// DS18S20 1 wire temperature sensor is connected to pin DIO6,
// and is powered by VCC. Vampire power is not used with this
// board layout.
//

// Common cathode, so on == pin high
const uint8_t LedOn = HIGH;
const uint8_t LedOff = LOW;

int g_lightPin = (-1); // No white LED
int g_redPin = 9;
int g_greenPin = 3;
int g_bluePin = 5;

int g_panicPin = 9; // red pin

// Optional environmental sensors
int g_sensorPowerPin = 4;
int g_lightIntensityPin = A1;
int g_batteryPin = A2;
int g_solarPin = A3;
int g_moisturePin = A0;

// Current IntegratedSensor has a Dallas DS18S20 on pin 6
int g_temperaturePin = 6;

#else

#if ARDUINO_AVR_MEGA2560
// Pin 4 must be used for CSN as 10 has conflicts
const uint8_t nRF24L01_CSN = 4;
#else
const uint8_t nRF24L01_CSN = 10;
#endif

const uint8_t nRF24L01_CE = 9;
const uint8_t nRF24L01_IRQ = 8;

//
// LED configuration
//

// Common cathode, so on == pin high
const uint8_t LedOn = HIGH;
const uint8_t LedOff = LOW;

int g_lightPin = (-1); // Default onboard led is SCK so don't touch it.

#if ARDUINO_AVR_MEGA2560
int g_redPin = (-1);
int g_greenPin = (-1);
int g_bluePin = (-1);
int g_panicPin = (-1);
#else
int g_redPin = (3);   // PWM
int g_greenPin = (5); // PWM
int g_bluePin = (6);  // PWM

int g_panicPin = 3; // red pin
#endif

//
// Optional environmental sensors
//
int g_sensorPowerPin = (-1);
int g_lightIntensityPin = (-1);
int g_batteryPin = (-1);
int g_solarPin = (-1);
int g_moisturePin = (-1);
int g_temperaturePin = (-1);

#endif

//
// Application hardware model support
//
FencePostLightHardware g_fencePostLightHardware;

//
// Main Application class
//
LightHouseApp g_lightHouseApp;

//
// Application Dweet handler that invokes the application
// class above in an RPC style fashion.
//
DweetLightHouse g_dweetLightHouse;

//
// DweetRadioSerialApp provides a general framework for
// a remote IoT sensor over a low power radio.
//
// In addition it supports connection to the serial port/USB
// for configuration, debugging, and gateway support.
//
// This framework enables the MenloFramework features for persistent
// configuration storage, power on state configuration, etc.
//

DweetRadioSerialApp g_sensorApp;

//
// Setup
//
void setup()
{
    //
    // Setup base hardware environment, pinmodes, etc.
    //

    // set pinmode as we are using it for flashing the light in our main loop
    if (g_panicPin != (-1)) {
        pinMode(g_panicPin, OUTPUT);     
        digitalWrite(g_panicPin, LOW);        
    }

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

  //
  // 05/23/2015 values
  //
  // Total Data: 1593 bytes leaving 455 bytes free at runtime
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
}

//
// Setup application framework.
//
void
ApplicationSetup()
{
  //
  // Hardware Configuration
  //
  LightHouseLight light;
  LightHouseSensors sensors;

  //
  // Application Framework Configuration
  //
  DweetRadioSerialAppConfiguration config;

  //
  // Setup the LightHouse light hardware configuration
  //
  light.whitePin = g_lightPin;
  light.redPin = g_redPin;
  light.greenPin = g_greenPin;
  light.bluePin = g_bluePin;

  //
  // Setup our sensors hardware configuration
  //
  sensors.sensorPower = g_sensorPowerPin;
  sensors.lightIntensity = g_lightIntensityPin;
  sensors.battery = g_batteryPin;
  sensors.solar = g_solarPin;
  sensors.moisture = g_moisturePin;
  sensors.temperature = g_temperaturePin;

  // Initialize the application hardware
  g_fencePostLightHardware.Initialize(
      &light,
      &sensors
      );

  ResetWatchdog();

  //
  // Initialize the Application State
  //
  g_lightHouseApp.Initialize(&g_fencePostLightHardware);

  ResetWatchdog();

  //
  // Initialize the Dweet handler
  //
  g_dweetLightHouse.Initialize(&g_lightHouseApp);

  //
  // Setup our application framework parameters
  //

  // A single application instance may be invoked from multiple Dweet channels.
  config.serialConfig.dweetApp = &g_dweetLightHouse;

  //
  // Supply the Dweet channel to the FencePostLight hardware.
  //
  g_fencePostLightHardware.SetDweet(g_sensorApp.GetDweet());

#if ARDUINO_AVR_MEGA2560
    //
    // Some UNO shields may wire SPI on both pins and ICSP
    // so this is a defensive configuration.
    //
    pinMode(11, INPUT);   // MOSI
    pinMode(12, INPUT);   // MISO
    pinMode(13, INPUT);   // SCK
#endif

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

  //
  // Initialize the Application Framework
  //
  g_sensorApp.Initialize(&config);

  ResetWatchdog();

  // Set the radio to the application after its initialized.
  g_lightHouseApp.SetRadio(g_sensorApp.GetRadio());

  ResetWatchdog();

  //
  // The application is designed with one instance of the LightHouseApp
  // object and one instance of the LighthouseHardware object.
  //
  // The application and hardware state can be queried and manipulated
  // by Dweets over the radio or the hardware serial port. They both
  // manipulate the common application state, but have their own
  // independent Dweet states since messages may be independently
  // received and responded to.
  //
  // As this is not a multithreaded environment synchronization of
  // the application state and hardware state object is not required.
  //

  MenloDebug::Print(F("LightHouse 05/23/2015"));

  ResetWatchdog();

  //
  // We check the memory monitor at the end of initialization
  // in case any initialization routines overflowed the stack or heap.
  //
  MenloMemoryMonitor::CheckMemory(__LINE__);

  return;
}
