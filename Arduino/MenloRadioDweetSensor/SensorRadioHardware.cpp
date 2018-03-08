
/*
 * Copyright (C) 2016 Menlo Park Innovation LLC
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
 *  Date: 05/28/2016
 *  File: SensorHardware.cpp
 *
 *  Hardware handling for sensor projects.
 */

// Costs 304 bytes RAM
//#define USE_DS18S20 1

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

#include "SensorRadioHardware.h"

//
// Note: DBG_PRINT_ENABLED is set to one to validate operation
// even when hardware is hooked up.
//
#define DBG_PRINT_ENABLED 1

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
const char sensor_module_name_string[] PROGMEM = "SensorHardware";

extern const char dweet_lightcolor_string[] PROGMEM = "LIGHTCOLOR";
extern const char dweet_lightonlevel_string[] PROGMEM = "LIGHTONLEVEL";

// Sensor/environmental support
extern const char dweet_sensors_string[] PROGMEM = "SENSORS";

const char* const sensor_string_table[] PROGMEM =
{
  dweet_lightcolor_string,
  dweet_lightonlevel_string,
  dweet_sensors_string
};

// Locally typed version of state dispatch function
typedef int (SensorHardware::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod sensor_function_table[] =
{
    &SensorHardware::LightColor,
    &SensorHardware::LightOnLevel,
    &SensorHardware::Sensors
};

PROGMEM const int sensor_index_table[] =
{
  SENSOR_LIGHT_COLOR,
  SENSOR_LIGHT_ONLEVEL,
  0                 // SENSORS does not have an EEPROM setting
};

PROGMEM const int sensor_size_table[] =
{
  SENSOR_LIGHT_COLOR_SIZE,
  SENSOR_LIGHT_ONLEVEL_SIZE,
  0                 // SENSORS does not have an EEPROM setting
};

SensorHardware::SensorHardware ()
{
    m_state.Initialize();
}

int
SensorHardware::Initialize(
    SensorHardwareConfig* config
    )
{

    //
    // Invoke base class initialize
    //
    DweetSensorApp::Initialize();

    //
    // struct copy to local field as this is a stack
    // structure to the caller.
    //
    m_config = *config;

    // Initialize
    m_config.Initialize();

    //
    // Read the EEPROM for power on/reset state
    // settings.
    //
    InitializeStateFromStoredConfig();

    return 0;
}

//
// This is fired by the base class based on the configured
// sensor sampling interval.
//
unsigned long
SensorHardware::SensorTimerEvent()
{
    unsigned long pollTime = MAX_POLL_TIME;

    ProcessLocalSensors();

    return pollTime;
}

void
SensorHardware::InitializeStateFromStoredConfig()
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[SENSOR_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(sensor_string_table) / sizeof(char*);

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (SensorHardware) since
    // the handlers are on this class.
    //
    // Improve: These stubs can be eliminated and direct calls
    // to the application class used.
    //

    parms.ModuleName = (PGM_P)sensor_module_name_string;
    parms.stringTable = (PGM_P)sensor_string_table;
    parms.functionTable = (PGM_P)sensor_function_table;
    parms.defaultsTable = NULL;
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
            MenloDebug::Print(F("SensorHardware Stored settings checksum is invalid"));
        }
        else {
            MenloDebug::Print(F("SensorHardware Stored settings are invalid"));
        }
    }
    else {
        MenloDebug::Print(F("SensorHardware Stored settings are valid"));
    }
}

//
// Dweet commands processing.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
SensorHardware::SubClassProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    int retVal;
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[SENSOR_MAX_SIZE+1];

    int tableEntries = sizeof(sensor_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    xDBG_PRINT("MenloDweetSensor: calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.ModuleName = (PGM_P)sensor_module_name_string;
    parms.stringTable = (PGM_P)sensor_string_table;
    parms.functionTable = (PGM_P)sensor_function_table;
    parms.defaultsTable = NULL;
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

void
SensorHardware::ProcessLocalSensors()
{
    SensorHardwareData sensors;

    xDBG_PRINT("ProcessLocalSensors entered");

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
        m_state.lightLevel = sensors.lightIntensity;

        // Process the current light state settings
        ProcessSensorStateTransitions();

        // Report sensor readings
        ReportSensorReadings(&sensors);
    }

    return;
}

//
// Process Sensor State Transitions.
//
// Set light state according to current light
// control values
//
// m_state.lightState
// m_state.lightPolarity
// m_state.redIntensity
// m_state.blueIntensity
// m_state.greenIntensity
//
void
SensorHardware::ProcessSensorStateTransitions()
{
  bool onState;

    //
    // If the light on level is greater than current ambient light
    // we do not enable the light
    //
    // Values 0x0 and 0xFFFF are always enabled due to either no
    // sensor, or not being initialized in the EEPROM.
    //
    if ((m_state.lightOnLevel == 0xFFFF) ||
        (m_state.lightOnLevel == 0) ||
        (m_state.lightLevel < m_state.lightOnLevel)) {

        m_state.lightState = true;
        DBG_PRINT("LightState ON");
    }
    else {
        m_state.lightState = false;
        DBG_PRINT("LightState OFF");
    }

    //
    // Set the light state. Polarity determines whether
    // TRUE == ON (m_polarity == TRUE) or OFF (m_polarity == FALSE)
    //
    if (m_state.lightState) {
        // ON
        onState = m_config.lightPolarity;
    }
    else {
        // OFF
        onState = !m_config.lightPolarity;
    }

    if (m_config.whitePin != (-1)) {
        digitalWrite(m_config.whitePin, onState);
    }

    // Handle RGB. Polarity determines whether true == ON or OFF
    if (m_config.rgbEnabled) {
        SetRGB(onState);
    }

    return;
}

void
SensorHardware::ReportSensorReadings(SensorHardwareData* data)
{
    int bufferIndex;
    MenloDweet* dweet;

    //
    // Report sensor readings on the radio
    //
    ReportSensorReadingsOnRadio(data);

    //
    // Note: To show SENSORDATA message enabled "showdweet" in
    // the dweet console. Normally unrecognized messages are not
    // displayed.
    //
    // Full message:
    //
    // SENSORDATA=0000.0000.0000.0000.0000'\0'
    //
    // Buffer just has to contain the data string and '\0'.
    //
    // This is maximum for Dweet:
    // (80 - (strlen("SENSORDATA=") + (strlen("*00"))) + 1;
    //
    char buffer[68];

    //
    // Queue A Dweet update if configured:
    //
    dweet = GetDweet();
    if (dweet == NULL) {
        xDBG_PRINT("m_dweet is NULL");
        return;
    }

    // Re-use common routine
    Sensors(buffer, sizeof(buffer) - 1, false);

    // Format is: SENSORDATA=0000.0000.0000.0000
    dweet->SendDweetItemReplyType_P(
	PSTR("SENSORDATA"),
	PSTR("="), // Don't want _REPLY on it as its a first request
        buffer
        );
}

void
SensorHardware::ReportSensorReadingsOnRadio(SensorHardwareData* data)
{
    SensorDataPacket packet;

    //
    // If radio sensor updates is configured attempt
    // to send the new readings.
    //
    if (m_sendSensorUpdates) {

        xDBG_PRINT("Sending radio sensor update");

        memset(&packet, 0, sizeof(packet));

        //
        // Queue a radio sensor update packet if configured
        //

        packet.type = SENSOR_DATA_PACKET;

        //
        // See MenloRadioSerial.h for subType LIGHTHOUSE_SENSOR_DATA_SUBTYPE
        //
        packet.subType = LIGHTHOUSE_SENSOR_DATA_SUBTYPE;

        //
        //        packet.flags = (SENSOR_DATA_LIGHT    |
        //                        SENSOR_DATA_BATTERY  |
        //                        SENSOR_DATA_SOLAR    |
        //                        SENSOR_DATA_MOISTURE |
        //                        SENSOR_DATA_TEMPERATURE);

        packet.light = data->lightIntensity;
        packet.battery = data->battery;
        packet.solar = data->solar;
        packet.moisture = data->moisture;
        packet.temperature = data->temperature;

        packet.windspeed = 0;
        packet.winddirection = 0;
        packet.barometer = 0;
        packet.humidity = 0;
        packet.rainfall = 0;

        if (m_radio != NULL) {

            DBG_PRINT("Writing to radio");

            m_radio->Write(
                NULL, // Use the configured address
                (uint8_t*)&packet,
                (uint8_t)sizeof(packet),
                250  // 250 ms
                );
        }
    }
}

//
// Worker to set RGB state.
//
void
SensorHardware::SetRGB(bool state)
{
    //
    // Show state since hardware may not be hooked up
    //
    if (state) {
        DBG_PRINT_NNL("SetRGB ");
        DBG_PRINT_INT_NNL(m_state.redIntensity);
        DBG_PRINT_NNL(".");
        DBG_PRINT_INT_NNL(m_state.greenIntensity);
        DBG_PRINT_NNL(".");
        DBG_PRINT_INT(m_state.blueIntensity);
    }
    else {
        DBG_PRINT("SetRGB State OFF");
    }

    // Pins are allowed to be individually set
    if (m_config.redPin != (-1)) {
      if (state) {
          analogWrite(m_config.redPin, m_state.redIntensity);
      }
      else {
          analogWrite(m_config.redPin, 0);
      }
    }

    if (m_config.bluePin != (-1)) {
      if (state) {
          analogWrite(m_config.bluePin, m_state.blueIntensity);
      }
      else {
          analogWrite(m_config.bluePin, 0);
      }
    }

    if (m_config.greenPin != (-1)) {
      if (state) {
          analogWrite(m_config.greenPin, m_state.greenIntensity);
      }
      else {
          analogWrite(m_config.greenPin, 0);
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
SensorHardware::readSensors(SensorHardwareData* sensors)
{
     uint16_t tmp;

    // Power on if required
    if (m_config.sensorPower != (-1)) {
        digitalWrite(m_config.sensorPower, 1);
        delay(10); // wait for settling
    }

    // m_config contains the configured pin numbers.

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

    if (m_config.lightIntensity != (-1)) {

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
        tmp = analogRead(m_config.lightIntensity);

        sensors->lightIntensity = 1023 - tmp;
    }
    else {
        sensors->lightIntensity = 0;
    }

    if (m_config.battery != (-1)) {

        //
        // MenloSensor1
        //
        // battery:
        //
        // currently read 0x1B3 (435) on a fairly full Alkaline battery
        //
        // (435/1023) == 0.4252 ratio of a 3.3v reference == 1.403 volts
        //
        sensors->battery = analogRead(m_config.battery);
    }
    else {
        sensors->battery = 0;
    }

    if (m_config.solar != (-1)) {

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
        sensors->solar = analogRead(m_config.solar);
    }
    else {
        sensors->solar = 0;
    }

    if (m_config.moisture != (-1)) {

        //
        // MenloSensor1
        //
        // moisture:
        //
        // Currently read 0x015C () while disconnected (open ended)
        //
        sensors->moisture = analogRead(m_config.moisture);
    }
    else {
        sensors->moisture = 0;
    }

    if (m_config.temperature != (-1)) {

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
        // Real key is RAM.
        // 1,714 bytes enabled
        // 1,410 bytes disabled
        // -------
        //   304 bytes
        //

        //
        // Currently a Dallas DS18B20 One Wire Temperature Sensor
        //
        sensors->temperature = m_config.temperatureSensor.GetTemperature();
#else
        //
        // A TMP36 is an analog sensor which is cheaper (< $1.00)
        // and does not burn xxx bytes of code space.
        //
        // Note: Know which reference is being used and accuracy of voltage
        // regulator when battery powered. See Arduino analogReference()
        // and the AtMega328 data sheets.
        //
        // https://www.arduino.cc/en/Reference/analogReference
        //
        // sensors->temperature = analogRead(m_config.temperature);
        sensors->temperature = 0;
#endif
    }
    else {
        sensors->temperature = 0;
    }

    // Power off if required
    if (m_config.sensorPower != (-1)) {
        digitalWrite(m_config.sensorPower, 0);
    }

    return true;
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
SensorHardware::LightColor(char* buf, int size, bool isSet)
{
    char* ptr;
    int length;

    xDBG_PRINT("LightColor: entered");

    if (isSet) {

        //
        // RR.GG.BB
        //
        // Parse the argument string
        // action == rr.gg.bb RGB 8 bit hex values for PWM
        //

        length = strlen(buf);
        if (length < 8) {
            xDBG_PRINT("LightColor: bad length on set");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        ptr = buf;

        //
        // R:G:B
        //

        m_state.redIntensity = MenloUtility::HexToByte(ptr);
        ptr += 2;
        if (*ptr != '.') {
            xDBG_PRINT("LightColor: not . after RR");
            return DWEET_INVALID_PARAMETER;
        }
        ptr++; // skip '.'

        xDBG_PRINT_NNL("redIntensity ");
        xDBG_PRINT_INT(m_state.redIntensity);

        m_state.greenIntensity = MenloUtility::HexToByte(ptr);
        ptr += 2;
        if (*ptr != '.') {
            xDBG_PRINT("LightColor: not . after GG");
            return DWEET_INVALID_PARAMETER;
        }
        ptr++; // skip '.'

        m_state.blueIntensity = MenloUtility::HexToByte(ptr);
        ptr += 2;

        //
        // Process the new light state
        //
        ProcessSensorStateTransitions();

        xDBG_PRINT("LightColor set");

        return 0;
    }
    else {

        if (size < 9) {
            xDBG_PRINT("LightColor: bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        ptr = buf;

        MenloUtility::UInt8ToHexBuffer(m_state.redIntensity, ptr);
        ptr += 2;

        *ptr++ = '.';

        MenloUtility::UInt8ToHexBuffer(m_state.greenIntensity, ptr);
        ptr += 2;

        *ptr++ = '.';

        MenloUtility::UInt8ToHexBuffer(m_state.blueIntensity, ptr);
        ptr += 2;

        *ptr = '\0';

        xDBG_PRINT("LightColor: returned value");

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
SensorHardware::LightOnLevel(char* buffer, int size, bool isSet)
{
    int length;
    int index;
    char* ptr;

    xDBG_PRINT("LightOnLevel: entered");

    if (isSet) {

        //
        // Parse the argument string
        // action == 0000 16 bit hex values for light intensity level
        // which triggers "nighttime mode".
        //
        length = strlen(buffer);
        if (length < 4) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        ptr = buffer;

        // Reads first 4 characters
        m_state.lightOnLevel = MenloUtility::HexToUShort(ptr);

        //
        // Re-evaluate the sensor states.
        //
        ProcessLocalSensors();

        return 0;
    }
    else {

        if (size < 5) {
            xDBG_PRINT("LightOnLevel bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        index = 0;

        MenloUtility::UInt16ToHexBuffer(m_state.lightOnLevel, &buffer[index]);
        index += 4;

        *ptr = '\0';

        return 0;
    }
}

//
// SENSORS:0000.0000.0000.0000
//
// Each sensor responds in position. Based on configuration.
//
// This is designed to be generic, and not take up much code space.
//
// buffer size is defined by the maximum buffer size - 1 of the
// buffer supplied to ProcessStateCommandsTable().
//
// This value is SENSOR_MAX_SIZE in SensorHardware.h
//
int
SensorHardware::Sensors(char* buffer, int size, bool isSet)
{
    int index;

    SensorHardwareData sensors;

    xDBG_PRINT("Sensors: entered");

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

    DBG_PRINT_NNL("SENSORS ");
    DBG_PRINT_STRING(buffer);

    return 1;
}
