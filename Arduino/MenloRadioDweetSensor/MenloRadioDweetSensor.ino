
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
 * Date: 04/28/2015
 * File: MenloRadioDweetSensor.ino
 *
 * Basic Sensor application.
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

// Dweet Support
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetSerialChannel.h>
#include <DweetRadio.h>

//
// Arduino and Debug support
//
#include <DweetArduino.h>
#include <DweetDebug.h>

// Application Framework class
#include <DweetSerialApp.h>

#include <DweetRadioSerialTransport.h>

// Dweet sensor application framework
#include <DweetSensorApp.h>

// Sensor Hardware
#include "SensorRadioHardware.h"

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
// The following defaults will take the sensor view
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

char* nRF24L01_defaultReceiveAddress = ADDRESS_SENS0;

// MenloRadioSerial sends to the gateway from the sensor
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

//
// Arduino Uno
//

const uint8_t nRF24L01_CSN = 10;
const uint8_t nRF24L01_CE = 9;
const uint8_t nRF24L01_IRQ = 8;

//
// LED configuration
//

// Common cathode, so on == pin high
const uint8_t LedOn = HIGH;
const uint8_t LedOff = LOW;

int g_lightPin = (-1); // Default onboard led is SCK so don't touch it.

int g_redPin = (3);   // PWM
int g_greenPin = (5); // PWM
int g_bluePin = (6);  // PWM

int g_panicPin = 3; // red pin

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
// Radio Hardware
//
OS_nRF24L01 g_nordic;
MirfHardwareSpiDriver g_MirfHardwareSpi;

void InitializePacketRadio()
{

    // Initialize the Nordic nRF24L01 radio
    g_nordic.Initialize(
        nRF24L01_forceDefaults,
        nRF24L01_defaultChannel,
        nRF24L01_defaultReceiveAddress,
        &g_MirfHardwareSpi,
        nRF24L01_payLoadSize,
        nRF24L01_CE,
        nRF24L01_CSN
        //nRF24L01_IRQ
        );
}

DweetRadio g_dweetRadio;

//
// Main Application class
//
// This is a generic SensorApp that responds to Dweet's for a Light
// flashing function, as well as basic monitoring.
//
// It registers to receive Dweet events from the common Dweet
// dispatcher regardless of which channel its received upon.
//
// It may be configured with a primary channel to send asynchronous
// Dweet updates.
//
SensorHardware g_sensorApp;

//
// DweetRadioSerialTransport provides a general transport for a remote IoT
// sensor over a low power radio.
//
// In addition it supports connection to the serial port/USB
// for configuration, debugging, and gateway support.
//
// This framework enables the MenloFramework features for persistent
// configuration storage, power on state configuration, etc.
//

//
// Without its 24,014 bytes used, 749 globals.
//
#if ENABLE_RADIO_SERIAL
DweetRadioSerialTransport g_radioSerialAppTransport;
#endif

// Arduino 1.6.8 now requires forward declarations like a proper C/C++ compiler.
void HardwareSetup(SensorHardwareConfig* configOut);
void ApplicationSetup();
void MenloFrameworkSetup();

//
// Setup
//
void setup()
{
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

void HardwareSetup(SensorHardwareConfig* config)
{

    // First initialize all pins to disabled.
    config->whitePin = (-1);
    config->redPin = (-1);
    config->greenPin = (-1);
    config->bluePin = (-1);
    config->lightPolarity = true;

    //
    // Enable hardware that is available
    //
    config->whitePin = 13;

    config->sensorPower = g_sensorPowerPin;
    config->lightIntensity = g_lightIntensityPin;
    config->battery = g_batteryPin;
    config->solar = g_solarPin;
    config->moisture = g_moisturePin;
    config->temperature = g_temperaturePin;
    config->whitePin = g_lightPin;
    config->redPin = g_redPin;
    config->greenPin = g_greenPin;
    config->bluePin = g_bluePin;

    // set pinmode as we are using it for flashing the light in our main loop
    if (g_panicPin != (-1)) {
        pinMode(g_panicPin, OUTPUT);     
        digitalWrite(g_panicPin, LOW);        
    }
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
  // These values work for the default app framework, but may have
  // to be adjusted based on actual application heap and stack usage.
  //
  // 638 bytes free 05/09/2017
  //
  frameworkConfig.heapSize = 32;
  frameworkConfig.stackSize = 570;
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
  // Hardware configuration
  //
  SensorHardwareConfig hardwareConfig;

  HardwareSetup(&hardwareConfig);

  //
  // Initialize the sensor application class.
  //
  // Note: The hardware configuration class derives from
  // the sensor application class.
  //
  g_sensorApp.Initialize(&hardwareConfig);

  ResetWatchdog();

  //
  // Setup our application transport(s) parameters
  //

  InitializePacketRadio();

  //
  // RadioSerial configuration
  //
  DweetRadioSerialTransportConfiguration config;

  config.payLoadSize = nRF24L01_payLoadSize;
  config.sendTimeout = g_sendTimeout;
  config.radio = &g_nordic;
  config.dweetRadio = &g_dweetRadio;
  config.radioSerialTransmitAddress = g_radioSerialTransmitAddress;

#if ENABLE_RADIO_SERIAL
  g_radioSerialAppTransport.Initialize(&config);
#endif

  //
  // Initialize Dweet Radio on the serial interface for configuration and
  // optional gateway usage.
  //

  // MenloDweet* needed from serial channel for generating R=0:xxx packets

#if ENABLE_RADIO_SERIAL
  g_dweetRadio.Initialize(g_radioSerialAppTransport.GetDweet(), &g_nordic);
#else
  g_dweetRadio.Initialize(NULL, &g_nordic);
#endif

  ResetWatchdog();

  // Set the default Dweet channel
#if ENABLE_RADIO_SERIAL
  g_sensorApp.SetDweet(g_radioSerialAppTransport.GetDweet());
#else
  g_sensorApp.SetDweet(NULL);
#endif

  // Set the radio to the application after initialized
#if ENABLE_RADIO_SERIAL
  g_sensorApp.SetRadio(g_radioSerialAppTransport.GetRadio());
#else
  g_sensorApp.SetRadio(&g_nordic);
#endif

  ResetWatchdog();

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

  MenloDebug::Print(F("MenloRadioDweetSensor Application 05/09/2017"));

  ResetWatchdog();

  //
  // We check the memory monitor at the end of initialization
  // in case any initialization routines overflowed the stack or heap.
  //
  MenloMemoryMonitor::CheckMemory(__LINE__);

  return;
}


//
// Internal 1.1V reference is +-10% accurate, but limited range.
// 
// Technique is to use VCC for the analog reference (default wiring for
// Arduino Uno, Arduino Pro Mini, etc.) and calibrate it against the
// 1.1V internal reference. Then use the calibration factor to determine
// voltage being measured, which reads as some 1023 bit fraction of VCC.
//
// From:
//
// Secret Arduino Voltmeter – Measure Battery Voltage by Provide
// Your Own is licensed under a
// Creative Commons Attribution-ShareAlike 4.0 International License.
//
// https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
//
long
readVcc()
{
  //
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  //
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif  

  delay(2); // Wait for Vref to settle

  ADCSRA |= _BV(ADSC); // Start conversion

  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  result = 1125300L / result;

  // Vcc in millivolts
  return result;
}

//
// Read AtMega CPU temperature.
//
// It may, or may not be close to ambient depending on whether the
// CPU is mostly sleeping or not, I/O pin currents, etc.
//
// https://code.google.com/archive/p/tinkerit/wikis/SecretThermometer.wiki
// http://www.instructables.com/id/Hidden-Arduino-Thermometer/
//
// Temperature is returned in milli-°C. So 25000 is 25°C.
//
long
readCpuTemp()
{
    long result;

#if defined(__AVR_ATmega32U4__)
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
    ADCSRB = _BV(MUX5); // the MUX5 bit is in the ADCSRB register
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(REFS1) | _BV(MUX5) | _BV(MUX1);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(REFS1) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
#else
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
#endif

    // Wait for Vref to settle
    delay(2);
    
    ADCSRA |= _BV(ADSC); // Start conversion

    // Wait for it
    while (bit_is_set(ADCSRA, ADSC));

    // must read ADCL first - it then locks ADCH
    result = ADCL;

    // reading ADCH unlocks both
    result |= ADCH << 8;

    // Note: Examine these constants, may require tweaking.
    // result = (result - 125) * 1075;

    return result;
}

//
// Normalize temperature against calibration
//
float
normalizeCpuTemperature(long rawData)
{ 
  //
  // replace these constants with your 2 data points
  // these are sample values that will get you in the ballpark (in degrees C)
  //
  float temp1 = 0;
  long data1 = 274;

  float temp2 = 25.0;
  long data2 = 304;
 
  // calculate the scale factor
  float scaleFactor = (temp2 - temp1) / (data2 - data1);

  // now calculate the temperature
  float temp = scaleFactor * (rawData - data1) + temp1;

  return temp;
}

