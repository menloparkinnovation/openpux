

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
 *  Date: 04/28/2015
 *  File: DweetSensorApp.cpp
 *
 *  Sensor application framework.
 */

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
#include <MenloConfigStore.h>

// Dweet Support
#include <MenloNMEA0183.h>
#include <MenloDweet.h>

#include <MenloRadio.h>
#include <MenloRadioSerial.h>

#if USE_DS18S20
// Dallas Digital 1 wire temperature sensor
#include <OS_DS18S20.h>
#endif

#include "DweetSensorApp.h"

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define DBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_HEX_STRING(x, l)
#define DBG_PRINT_HEX_STRING_NNL(x, l)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define xDBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_HEX_STRING(x, l)
#define xDBG_PRINT_HEX_STRING_NNL(x, l)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

//
// Strings used in DweetSensor
//

extern const char dweet_lightcolor_string[] PROGMEM = "LIGHTCOLOR";
extern const char dweet_lightonlevel_string[] PROGMEM = "LIGHTONLEVEL";
extern const char dweet_sensorrate_string[] PROGMEM = "SENSORRATE";

// Sensor/environmental support
extern const char dweet_sensors_string[] PROGMEM = "SENSORS";

const char* const sensor_string_table[] PROGMEM =
{
  dweet_lightcolor_string,
  dweet_lightonlevel_string,
  dweet_sensorrate_string,
  dweet_sensors_string
};

// Locally typed version of state dispatch function
typedef int (DweetSensorApp::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod sensor_function_table[] =
{
    &DweetSensorApp::LightColor,
    &DweetSensorApp::LightOnLevel,
    &DweetSensorApp::SensorUpdateRate,
    &DweetSensorApp::ProcessSensors
};

PROGMEM const int sensor_index_table[] =
{
  SENSOR_LIGHT_COLOR,
  SENSOR_LIGHT_ONLEVEL,
  SENSOR_SENSORRATE,
  0                 // SENSORS does not have an EEPROM setting
};

PROGMEM const int sensor_size_table[] =
{
  SENSOR_LIGHT_COLOR_SIZE,
  SENSOR_LIGHT_ONLEVEL_SIZE,
  SENSOR_SENSORRATE_SIZE,
  0                 // SENSORS does not have an EEPROM setting
};

//
// Lighthouse commands processing.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetSensorApp::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[SENSOR_MAX_SIZE+1];

    int tableEntries = sizeof(sensor_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    DBG_PRINT("DweetSensorApp calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.stringTable = (PGM_P)sensor_string_table;
    parms.functionTable = (PGM_P)sensor_function_table;
    parms.object =  this;
    parms.indexTable = sensor_index_table;
    parms.sizeTable = sensor_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = SENSOR_CHECKSUM;
    parms.checksumBlockStart = SENSOR_CHECKSUM_BEGIN;
    parms.checksumBlockSize = SENSOR_CHECKSUM_END - SENSOR_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
}

//
// RGB saturation values
// LIGHTCOLOR:00.00.00
//
// LIGHTCOLOR:GREEN
// LIGHTCOLOR:RED
// LIGHTCOLOR:AMBER
// LIGHTCOLOR:WHITE
// LIGHTCOLOR:BLUE
//
int
DweetSensorApp::LightColor(char* buf, int size, bool isSet)
{
    char* ptr;
    int length;

    if (isSet) {

        //
        // RR.GG.BB
        //
        // Parse the argument string
        // action == rr.gg.bb RGB 8 bit hex values for PWM
        //

        length = strlen(buf);
        if (length < 8) {
            DBG_PRINT("LightColor: bad length on set");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        ptr = buf;

        m_redIntensity = MenloUtility::HexToByte(ptr);
        ptr += 2;
        if (*ptr != '.') {
            DBG_PRINT("LightColor: not . after RR");
            return DWEET_INVALID_PARAMETER;
        }
        ptr++; // skip '.'

        m_greenIntensity = MenloUtility::HexToByte(ptr);
        ptr += 2;
        if (*ptr != '.') {
            DBG_PRINT("LightColor: not . after GG");
            return DWEET_INVALID_PARAMETER;
        }
        ptr++; // skip '.'

        m_blueIntensity = MenloUtility::HexToByte(ptr);
        ptr += 2;

        //
        // The light intensity will be set at the next sensor
        // update interval.
        //

        DBG_PRINT("LightColor set");

        return 0;
    }
    else {

        if (size < 9) {
            DBG_PRINT("LightColor: bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        ptr = buf;

        MenloUtility::UInt8ToHexBuffer(m_redIntensity, ptr);
        ptr += 2;

        *ptr++ = '.';

        MenloUtility::UInt8ToHexBuffer(m_greenIntensity, ptr);
        ptr += 2;

        *ptr++ = '.';

        MenloUtility::UInt8ToHexBuffer(m_blueIntensity, ptr);
        ptr += 2;

        *ptr = '\0';

        return 0;
    }
    return 0;
}

//
// Parse the argument string
// buf == 0000 16 bit hex values for light on level
//
// LIGHTONLEVEL:0000
//
int
DweetSensorApp::LightOnLevel(char* buf, int size, bool isSet)
{
    int length;
    char* ptr;

    if (isSet) {

        //
        // Parse the argument string
        // action == 0000 16 bit hex values for light intensity level
        // which triggers "nighttime mode".
        //
        length = strlen(buf);
        if (length < 4) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        ptr = buf;

        // Reads first 4 characters
        m_lightOnLevel = MenloUtility::HexToUShort(ptr);

        //
        // The light level we be re-evaluated at the next
        // sensor update interval.
        //

        return 0;
    }
    else {
        // TODO:
        return DWEET_ERROR_UNSUP;
    }
}

//
// SENSORRATE:0000
//
int
DweetSensorApp::SensorUpdateRate(char* buf, int size, bool isSet)
{
    int length;
    char* ptr;
    uint16_t updateRate;

    if (isSet) {

        //
        // Parse the argument string
        // action == 0000 16 bit hex values for update rate in seconds
        //
        length = strlen(buf);
        if (length < 4) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        ptr = buf;

        // Reads first 4 characters
        updateRate = MenloUtility::HexToUShort(ptr);

        SetSensorUpdateRate(updateRate);

        return 0;
    }
    else {
        // TODO:
        return DWEET_ERROR_UNSUP;
    }
}

//
// SENSORS:0000.0000.0000.0000
// Each sensor responds in position. Based on configuration.
// This is designed to be generic, and not take up much code space.
//
int
DweetSensorApp::ProcessSensors(char* buffer, int size, bool isSet)
{
    int index;

    SensorsData sensors;

    // SETSTATE is not supported
    if (isSet) return DWEET_ERROR_UNSUP;

    //
    // This function returns the following full Dweet
    //
    //                        lite bat  slr  mois temp
    // GETSTATE_REPLY=SENSORS:0000.0000.0000.0000.0000
    //
    // this is 24 characters for basic data.
    // Allocate extra char for ':' and '\0'
    //

    index = 0;

    if (!readSensors(&sensors)) {
        return DWEET_APP_FAILURE;
    }

    MenloUtility::UInt16ToHexBuffer(sensors.lightIntensity, &buffer[index]);
    index += 4;
    buffer[index++]  = '.';

    MenloUtility::UInt16ToHexBuffer(sensors.battery, &buffer[index]);
    index += 4;
    buffer[index++]  = '.';

    MenloUtility::UInt16ToHexBuffer(sensors.solar, &buffer[index]);
    index += 4;
    buffer[index++]  = '.';

    MenloUtility::UInt16ToHexBuffer(sensors.moisture, &buffer[index]);
    index += 4;
    buffer[index++]  = '.';

    MenloUtility::UInt16ToHexBuffer(sensors.temperature, &buffer[index]);
    index += 4;

    buffer[index++] = '\0';

    // 0000.0000.0000.0000

    return 1;
}

void
DweetSensorApp::SetRadio(
    MenloRadio* radio
    )
{
    // This could be NULL, which means no radio
    m_radio = radio;
}

//
// Enable use of Dallas DS18S20 OneWire temperature sensor.
//
// Careful when you enable this. It can cause hardware lock ups,
// code size growth of 2114 bytes, and at least 2 seconds spinning
// around waiting for a reading and for the thing to settle at
// every sample interval.
// 
//#define USE_DS18S20 1
#define USE_DS18S20 0

DweetSensorApp::DweetSensorApp ()
{
        m_initialized = false;
        m_sendSensorUpdates = false;
        m_lightLevel = 0; // default is darkness
        m_lightOnLevel = (-1); // default is always on.


        m_lightState = false;
        m_lightPolarity = true;
        m_rgbEnabled = false;

        m_redIntensity = 0xFF;
        m_greenIntensity = 0xFF;
        m_blueIntensity = 0xFF;

        // m_sensors are initialized during Initialize()
}

int
DweetSensorApp::Initialize(SensorsConfiguration* sensors)
{
    //
    // struct copy to local field as this is a stack
    // structure to the caller.
    //
    m_sensors = *sensors;

    //
    // Application state setup
    //

    //
    // Setup the environmental monitoring timer
    //

    m_monitorInterval = SENSOR_DEFAULT_INTERVAL;

    // Standard args
    m_timerEvent.object = this;

    m_timerEvent.method = (MenloEventMethod)&DweetSensorApp::TimerEvent;

    // Object type specific args
    m_timerEvent.m_interval = m_monitorInterval;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    //
    // Setup the hardware
    //

    // Setup our application hardware pins
    if (m_sensors.lightPin != (-1)) {
        pinMode(m_sensors.lightPin, OUTPUT);
    }

    if (m_sensors.redPin != (-1)) {
        pinMode(m_sensors.redPin, OUTPUT);

        // Set test short cut
        m_rgbEnabled = true;
    }

    if (m_sensors.greenPin != (-1)) {
        pinMode(m_sensors.greenPin, OUTPUT);
        m_rgbEnabled = true;
    }

    if (m_sensors.bluePin != (-1)) {
        pinMode(m_sensors.bluePin, OUTPUT);
        m_rgbEnabled = true;
    }

    if (m_sensors.sensorPower != (-1)) {

        pinMode(m_sensors.sensorPower, OUTPUT);
        digitalWrite(m_sensors.sensorPower, 0);
    }

#if USE_DS18S20
    if (m_sensors.temperature != (-1)) {
        m_temperatureSensor.Initialize(m_sensors.temperature);
    }
#endif

    //
    // These pins are analog input which by default need
    // no configuration.
    //
    // m_sensors.lightIntensity
    // m_sensors.battery
    // m_sensors.solar
    // m_sensors.moisture
    // m_sensors.temperature
    //

    //
    // Read the EEPROM for power on/reset state
    // settings.
    //
    InitializeStateFromStoredConfig();

    //
    // Now register our events now we are configured and ready
    // to receive them.
    //

    // Timer
    m_timer.RegisterIntervalTimer(&m_timerEvent);

    // Indicate we are initialized and can perform async processing
    m_initialized = true;

    return 0;
}

void
DweetSensorApp::InitializeStateFromStoredConfig()
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[SENSOR_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(sensor_string_table) / sizeof(char*);

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (DweetSensorApp) since
    // the handlers are on this class.
    //
    // Improve: These stubs can be eliminated and direct calls
    // to the application class used.
    //

    parms.stringTable = (PGM_P)sensor_string_table;
    parms.functionTable = (PGM_P)sensor_function_table;
    parms.object =  this;
    parms.indexTable = sensor_index_table;
    parms.sizeTable = sensor_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = SENSOR_CHECKSUM;
    parms.checksumBlockStart = SENSOR_CHECKSUM_BEGIN;
    parms.checksumBlockSize = SENSOR_CHECKSUM_END - SENSOR_CHECKSUM_BEGIN;
    parms.name = NULL;
    parms.value = NULL;

    // DweetState.cpp
    result = MenloDweet::LoadConfigurationSettingsTable(&parms);
    if (result != 0) {
        if (result == DWEET_INVALID_CHECKSUM) {
            MenloDebug::Print(F("DweetSensorApp Stored settings checksum is invalid"));
        }
        else {
            MenloDebug::Print(F("DweetSensorApp Stored settings are invalid"));
        }
    }
    else {
        MenloDebug::Print(F("DweetSensor Stored settings are valid"));
    }
}

unsigned long
DweetSensorApp::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    SensorsData sensors;
    SensorDataPacket data;

    xDBG_PRINT("DweetSensorApp TimerEvent");

    // Check the environmental monitor hardware
    if (readSensors(&sensors)) {

        //
        // We expect "sane" readings from the hardware support
        // even if something is disconnected or optional.
        //

        //
        // valid reading(s), so store the current light intensity
        // for automatic light handling.
        //
        m_lightLevel = sensors.lightIntensity;

        //
        // If radio sensor updates is configured attempt
        // to send the new readings.
        //
        if (m_sendSensorUpdates) {

	    xDBG_PRINT("Sending radio sensor update");

    	    //
	    // Queue a radio sensor update packet if configured
	    //

	    data.type = SENSOR_DATA_PACKET;
	    data.flags = (SENSOR_DATA_LIGHT    |
			  SENSOR_DATA_BATTERY  |
			  SENSOR_DATA_SOLAR    |
			  SENSOR_DATA_MOISTURE |
			  SENSOR_DATA_TEMPERATURE);

	    data.light = sensors.lightIntensity;
	    data.battery = sensors.battery;
	    data.solar = sensors.solar;
	    data.moisture = sensors.moisture;
	    data.temperature = sensors.temperature;

            if (m_radio != NULL) {

                xDBG_PRINT("Writing to radio");

	        m_radio->Write(
   		    NULL, // Use the configured address
                    (uint8_t*)&data,
                    (uint8_t)sizeof(data),
                    250  // 250 ms
                    );
            }

        } // if (m_sendSensorUpdates)

        //
        // If the light on level is greater than current ambient light
        // we do not enable the light
        //
        // Values 0x0 and 0xFFFF are always enabled due to either no
        // sensor, or not being initialized in the EEPROM.
        //
        if ((m_lightOnLevel == 0xFFFF) ||
            (m_lightOnLevel == 0) ||
            (m_lightLevel < m_lightOnLevel)) {

            m_lightState = true;
        }
        else {
            m_lightState = false;
        }

        // Process the current light state settings
        SetLightOnState();

        //
        // Queue A Dweet update if configured:
        //
        // TODO:
        //
        // NOTE: This is on both radio serial or serial interfaces.
        // The incoming request works from either transport.
        // But we need configuration of where to send the periodic
        // sensor updates.
        //
        // DweetSensor::ProcessGetSensors(NULL, "SENSORS")
        //
        // Could make this radio packet only for updates, but
        // that would preclude USB connected sensor gateway
        // applications.
        //
    }

    return MAX_POLL_TIME;
}

void
DweetSensorApp::SetSensorUpdateRate(uint16_t value)
{
    //
    // If a sensor update rate is specified other
    // than zero sensor updates are scheduled at
    // the specified rate in seconds.
    //
    // The sensor update handler also performs local
    // timer based autonomous functions such as
    // day/night handling of the light sequence.
    //
    // If an update rate value of 0 is specified, no
    // sensor updates are sent.
    //
    // The timer event is re-registered to the default
    // rate so that local autonomous processing may
    // continue at the default.
    //
    // Improve: Could allow the default rate to be stored
    // in the config store and not locked into 30 seconds.
    //
    // Improve: The side effect of setting longer environmental
    // update times increase the response time to day/night
    // configuration.
    //
    if (value != 0) {
        m_monitorInterval = (value * 1000L);
        m_sendSensorUpdates = true;
    }
    else {

        //
        // set m_monitorInterval back to default as its
        // used for autonomous light on, light off handing.
        //
        m_monitorInterval = SENSOR_DEFAULT_INTERVAL;
        m_sendSensorUpdates = false;
    }

    //
    // If not initialized timer is not set and we are being
    // set based on EEPROM configuraiton.
    //
    // So update our interval, but don't attempt to touch
    // the timer. When Initialize() finally starts the timer
    // it will have the correct updated interval.
    //

    if (m_initialized) {
        // Unregister our current timer
        xDBG_PRINT("SENSORRATE Unregistering timer");
        m_timer.UnregisterIntervalTimer(&m_timerEvent);
    }

    // Update interval
    m_timerEvent.m_interval = m_monitorInterval;

    if (m_initialized) {
        // Re-register timer
        xDBG_PRINT("SENSORRATE Re-registering timer");
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }
}

//
// Set light state according to current light
// control values
//
// m_lightState
// m_lightPolarity
//
//
void
DweetSensorApp::SetLightOnState()
{
  bool onState;

  //
  // Set the light state. Polarity determines whether
  // TRUE == ON (m_polarity == TRUE) or OFF (m_polarity == FALSE)
  //
  if (m_lightState) {
      // ON
      onState = m_lightPolarity;
  }
  else {
      // OFF
      onState = !m_lightPolarity;
  }

  if (m_sensors.lightPin != (-1)) {
      digitalWrite(m_sensors.lightPin, onState);
  }

  // Handle RGB. Polarity determines whether true == ON or OFF
  if (m_rgbEnabled) {
      SetRGB(onState);
  }

  return;
}

void
DweetSensorApp::SetRGB(bool state)
{
    // Pins are allowed to be individually set
    if (m_sensors.redPin != (-1)) {
      if (state) {
          analogWrite(m_sensors.redPin, m_redIntensity);
      }
      else {
          analogWrite(m_sensors.redPin, 0);
      }
    }

    if (m_sensors.bluePin != (-1)) {
      if (state) {
          analogWrite(m_sensors.bluePin, m_blueIntensity);
      }
      else {
          analogWrite(m_sensors.bluePin, 0);
      }
    }

    if (m_sensors.greenPin != (-1)) {
      if (state) {
          analogWrite(m_sensors.greenPin, m_greenIntensity);
      }
      else {
          analogWrite(m_sensors.greenPin, 0);
      }
    }
}

//
// Read as many sensors as are configured
//
// One function saves code space on small micros such
// as the Atmega328.
//
bool
DweetSensorApp::readSensors(SensorsData* sensors)
{
     uint16_t tmp;

    // Power on if required
    if (m_sensors.sensorPower != (-1)) {
        digitalWrite(m_sensors.sensorPower, 1);
        delay(10); // wait for settling
    }

    // m_sensors contains the configured pin numbers.

    // sensors* is the output for the current readings.

    //
    // To test sensor hardware send the following Dweet:
    //
    //                        lite bat  slr  mois temp
    // GETSTATE_REPLY=SENSORS:0000.0000.0000.0000.0000
    // dweet GETSTATE=SENSORS
    //

    //
    // MenloSensor2 returns with 1.2v NiCD battery almost dead
    // and solar cell facing down/blocked.
    //
    // lite bat  slr  mois temp
    // 03B1.015B.016C.0120.0000
    //
    //

    if (m_sensors.lightIntensity != (-1)) {

        //
        //
        // MenloSensor1
        //
        // light:
        //
        // 10 bit A/D. Brighter light is a lower reading.
        //
        // Light is 0x27B (635) dark 0x01B (27) light
        //
        // So we subtract from the A/D range (0-1023) to
        // get a value that increases with light intensity.
        //
        tmp = analogRead(m_sensors.lightIntensity);

        sensors->lightIntensity = 1023 - tmp;
    }
    else {
        sensors->lightIntensity = 0;
    }

    if (m_sensors.battery != (-1)) {

        //
        // MenloSensor1
        //
        // battery:
        //
        // currently read 0x1B3 (435) on a fairly full Alkaline battery
        //
        // (435/1023) == 0.4252 ratio of a 3.3v reference == 1.403 volts
        //
        sensors->battery = analogRead(m_sensors.battery);
    }
    else {
        sensors->battery = 0;
    }

    if (m_sensors.solar != (-1)) {

        //
        // MenloSensor1
        //
        // solar:
        //
        // currently read 0x1B5 (437) on a disconnected solar cell and
        // a fairly full Alkaline battery installed.
        //
        // Even though there is a blocking diode we could be reading
        // back leakage since there is no load.
        //
        sensors->solar = analogRead(m_sensors.solar);
    }
    else {
        sensors->solar = 0;
    }

    if (m_sensors.moisture != (-1)) {

        //
        // MenloSensor1
        //
        // moisture:
        //
        // Currently read 0x015C () while disconnected (open ended)
        //
        sensors->moisture = analogRead(m_sensors.moisture);
    }
    else {
        sensors->moisture = 0;
    }

    if (m_sensors.temperature != (-1)) {

#if USE_DS18S20

        //
        // This sensor has the following issues:
        //
        // Cost: About $3.50
        // Slow: Must be awake for 1-2 seconds bouncing its signal
        //       and waiting to get a reading. Hurts battery life when
        //       could be sleeping.
        //
        // Weird signals: It's use has dropped the Nordic Radio/SPI offline
        // requiring re-initialization, and signal coupling has caused other
        // problems. It must always be on the power rail, though it goes into
        // it own low power sleep. Can't put it on sensor power pin.
        //
        // Code Size Cost: 2114 bytes for OS_DS18S20 library + OneWire support library.
        //            vs. a single analogRead() of a < $1.00 TMP36.
        //
        // 30,094 enabled
        // 27,980 disabled
        // -------
        //  2114 bytes
        //

        //
        // Currently a Dallas DS18B20 One Wire Temperature Sensor
        //
        sensors->temperature = m_temperatureSensor.GetTemperature();
#else
        //
        // A TMP36 is an analog sensor which is cheaper (< $1.00)
        // and does not burn xxx bytes of code space.
        //
        // sensors->temperature = analogRead(m_sensors.temperature);
        sensors->temperature = 0;
#endif
    }
    else {
        sensors->temperature = 0;
    }

    // Power off if required
    if (m_sensors.sensorPower != (-1)) {
        digitalWrite(m_sensors.sensorPower, 0);
    }

    return true;
}
