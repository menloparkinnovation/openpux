
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
 *  Date: 02/22/2015
 *  File: DweetLightHouse.cpp
 *
 *  Lighthouse Dweet support
 *
 *   Refactored from example app as a separate module to support
 *   multiple Dweet channels.
 */

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//
#include <MenloPlatform.h>
#include <MenloUtility.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>

#include <DweetSerialChannel.h>

// Lighthouse support
#include <MenloLightHouse.h>

#include "LightHouseApp.h"
#include <DweetApp.h>
#include "DweetLightHouse.h"

#include "LightHouseHardwareBase.h"

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
// Strings used in DweetLightHouse
//

extern const char dweet_lightperiod_string[] PROGMEM = "LIGHTPERIOD";
extern const char dweet_lighttick_string[] PROGMEM = "LIGHTTICK";
extern const char dweet_lightcolor_string[] PROGMEM = "LIGHTCOLOR";
extern const char dweet_lightramp_string[] PROGMEM = "LIGHTRAMP";
extern const char dweet_lightonlevel_string[] PROGMEM = "LIGHTONLEVEL";
extern const char dweet_sensorrate_string[] PROGMEM = "SENSORRATE";

// Sensor/environmental support
extern const char dweet_sensors_string[] PROGMEM = "SENSORS";

//
// These try to keep the command short to maximize encoding of the sequence
//
extern const char dweet_lightsq_string[] PROGMEM = "LIGHTSQ";  // Set Light Sequence
extern const char dweet_lightsp_string[] PROGMEM = "LIGHTSP";  // Set Light Sequence Persistent
extern const char dweet_lightgq_string[] PROGMEM = "LIGHTGQ";  // Get Light Sequence
extern const char dweet_lightgp_string[] PROGMEM = "LIGHTGP";  // Get Light Sequence Persistent

// IMPROVE: Make these PSTR("")'s
// NOTE: Currently used in call to char* function
char*
DweetLightHouse::onState = "ON";

char*
DweetLightHouse::offState = "OFF";

const char* const lighthouse_string_table[] PROGMEM =
{
  dweet_lightperiod_string,
  dweet_lighttick_string,
  dweet_lightcolor_string,
  dweet_lightramp_string,
  dweet_lightonlevel_string,
  dweet_sensorrate_string,
  dweet_sensors_string
};

// Locally typed version of state dispatch function
typedef int (DweetLightHouse::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod lighthouse_function_table[] =
{
    &DweetLightHouse::LightPeriod,
    &DweetLightHouse::LightTick,
    &DweetLightHouse::LightColor,
    &DweetLightHouse::LightRamp,
    &DweetLightHouse::LightOnLevel,
    &DweetLightHouse::SensorUpdateRate,
    &DweetLightHouse::ProcessSensors
};

PROGMEM const int lighthouse_index_table[] =
{
  LIGHT_PERIOD,
  LIGHT_TICK,
  LIGHT_COLOR,
  LIGHT_RAMP,
  LIGHT_ONLEVEL,
  LIGHT_SENSORRATE,
  0                 // SENSORS does not have an EEPROM setting
};

PROGMEM const int lighthouse_size_table[] =
{
  LIGHT_PERIOD_SIZE,
  LIGHT_TICK_SIZE,
  LIGHT_COLOR_SIZE,
  LIGHT_RAMP_SIZE,
  LIGHT_ONLEVEL_SIZE,
  LIGHT_SENSORRATE_SIZE,
  0                 // SENSORS does not have an EEPROM setting
};

//
// LIGHTPERIOD:00000000
//
int
DweetLightHouse::LightPeriod(char* buf, int size, bool isSet)
{
    char* ptr;
    bool error;
    int length;
    unsigned long tick;

    if (isSet) {

        // buf == lightperiod digits
        error = false;
        tick = MenloUtility::HexToULong(buf, &error);

        if (error) {
            return DWEET_INVALID_PARAMETER;
        }

        m_lightHouseApp->LightPeriod(&tick, isSet);

        return 0;
    }
    else {
        if (size < 9) {
            DBG_PRINT("LightPeriod: bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        // Get current tick
        m_lightHouseApp->LightPeriod(&tick, false);

        ptr = buf;
        MenloUtility::UInt32ToHexBuffer(tick, ptr);
        ptr += 8;

        *ptr = '\0';

        return 0;
    }
}

//
// LIGHTTICK:0000
//
int
DweetLightHouse::LightTick(char* buf, int size, bool isSet)
{
    char* ptr;
    bool error;
    int length;
    unsigned long tick;

    if (isSet) {

        // action == tick count digits
        error = false;
        tick = MenloUtility::HexToULong(buf, &error);
        if (error) {
            return DWEET_INVALID_PARAMETER;
        }

        m_lightHouseApp->LightTick(&tick, isSet);

        return 0;
    }
    else {
        if (size < 9) {
            DBG_PRINT("LightPeriod: bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        // Get current tick
        m_lightHouseApp->LightTick(&tick, false);

        ptr = buf;
        MenloUtility::UInt32ToHexBuffer(tick, ptr);
        ptr += 8;

        *ptr = '\0';

        return 0;
    }
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
DweetLightHouse::LightColor(char* buf, int size, bool isSet)
{
    char* ptr;
    int length;
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    if (isSet) {

        //
        // RR.GG.BB
        //
        // Parse the argument string
        // action == rr.gg.bb RGB 8 bit hex values for PWM
        //

        length = strlen(buf);
        if (length < 8) {
            DBG_PRINT("LightColor: bad length");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        ptr = buf;

        red = MenloUtility::HexToByte(ptr);
        ptr += 2;
        if (*ptr != '.') {
            DBG_PRINT("LightColor: not . after RR");
            return DWEET_INVALID_PARAMETER;
        }
        ptr++; // skip '.'

        green = MenloUtility::HexToByte(ptr);
        ptr += 2;
        if (*ptr != '.') {
            DBG_PRINT("LightColor: not . after GG");
            return DWEET_INVALID_PARAMETER;
        }
        ptr++; // skip '.'

        blue = MenloUtility::HexToByte(ptr);
        ptr += 2;

        m_lightHouseApp->LightColor(&red, &green, &blue, isSet);

        DBG_PRINT("LightColor set NP");

        return 0;
    }
    else {

        if (size < 9) {
            DBG_PRINT("LightColor: bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        m_lightHouseApp->LightColor(&red, &green, &blue, isSet);

        ptr = buf;

        MenloUtility::UInt8ToHexBuffer(red, ptr);
        ptr += 2;

        *ptr++ = '.';

        MenloUtility::UInt8ToHexBuffer(green, ptr);
        ptr += 2;

        *ptr++ = '.';

        MenloUtility::UInt8ToHexBuffer(blue, ptr);
        ptr += 2;

        *ptr = '\0';

        return 0;
    }
}

//
// LIGHTRAMP:00.00 rampup, ram down
//
int
DweetLightHouse::LightRamp(char* buf, int size, bool isSet)
{
    int length;
    char* ptr;
    uint16_t rampUpPeriod;
    uint16_t rampDownPeriod;

    if (isSet) {

        //
        // Parse the argument string
        // action == 0000.0000 16 bit hex values for ramp up + ramp down
        //
        length = strlen(buf);
        if (length < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        ptr = buf;

        // Reads first 4 characters
        rampUpPeriod = MenloUtility::HexToUShort(ptr);
        ptr += 4;
        if (*ptr != '.') {
            return DWEET_INVALID_PARAMETER;
        }
        ptr++; // skip '.'

        rampDownPeriod = MenloUtility::HexToUShort(ptr);

        m_lightHouseApp->RampPeriod(&rampUpPeriod, &rampDownPeriod, isSet);

        return 0;
    }
    else {

        if (size < 10) {
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        m_lightHouseApp->RampPeriod(&rampUpPeriod, &rampDownPeriod, false);

        ptr = buf;

        MenloUtility::UInt16ToHexBuffer(rampUpPeriod, ptr);
        ptr += 4;

        *ptr++ = '.';

        MenloUtility::UInt16ToHexBuffer(rampDownPeriod, ptr);
        ptr += 4;

        *ptr = '\0';

        return 0;
    }
}

//
// Parse the argument string
// buf == 0000 16 bit hex values for light on level
//
// LIGHTONLEVEL:0000
//
int
DweetLightHouse::LightOnLevel(char* buf, int size, bool isSet)
{
    int length;
    char* ptr;
    uint16_t onLevel;

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
        onLevel = MenloUtility::HexToUShort(ptr);

        m_lightHouseApp->LightOnLevel(&onLevel, isSet);

        return 0;
    }
    else {

        if (size < 5) {
            DBG_PRINT("LightOnLevel: bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        m_lightHouseApp->LightOnLevel(&onLevel, false);

        ptr = buf;
        MenloUtility::UInt16ToHexBuffer(onLevel, ptr);
        ptr += 4;

        *ptr = '\0';

        return 0;
    }
}

//
// SENSORRATE:0000
//
int
DweetLightHouse::SensorUpdateRate(char* buf, int size, bool isSet)
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

        m_lightHouseApp->SensorUpdateRate(&updateRate, isSet);

        return 0;
    }
    else {

        if (size < 5) {
            DBG_PRINT("SensorUpdateRate: bad length on get");
            return DWEET_INVALID_PARAMETER_LENGTH;
        }

        m_lightHouseApp->SensorUpdateRate(&updateRate, false);

        ptr = buf;
        MenloUtility::UInt16ToHexBuffer(updateRate, ptr);
        ptr += 4;

        *ptr = '\0';

        return 0;
    }
}

//
// Lighthouse commands processing.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetLightHouse::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[LIGHT_MAX_SIZE+1];

    int tableEntries = sizeof(lighthouse_string_table) / sizeof(char*);

    //
    // To maximum space for the sequence the set/get light
    // sequence commands don't use GETSTATE/SETSTATE to
    // have a slightly more compact representation.
    //
    if (strncmp_P(name, dweet_lightsq_string, 7) == 0)  {
          return ProcessSetLightSequence(dweet, name, value, false);
    }
    else if (strncmp_P(name, dweet_lightsp_string, 7) == 0)  {
          // true for set persistent
          return ProcessSetLightSequence(dweet, name, value, true);
    }
    else if (strncmp_P(name, dweet_lightgq_string, 7) == 0)  {
          return ProcessGetLightSequence(dweet, name, value, false);
    }
    else if (strncmp_P(name, dweet_lightgp_string, 7) == 0)  {
          // true for set persistent
          return ProcessGetLightSequence(dweet, name, value, true);
    }
    else {

        //
        // Most lighthouse operationg modes and sensor commands
        // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
        // and use the table driven common code which saves at least 3K bytes
        // on an Atmeg328 for this one set of commands alone.
        //

        DBG_PRINT("DweetLightHouse calling table processor");

        //
        // We dispatch on "this" because the method is part of the current
        // class instance as this function performs the specialization
        // required for DweetLightHouse
        //
        parms.stringTable = (PGM_P)lighthouse_string_table;
        parms.functionTable = (PGM_P)lighthouse_function_table;
        parms.object =  this;
        parms.indexTable = lighthouse_index_table;
        parms.sizeTable = lighthouse_size_table;
        parms.tableEntries = tableEntries;
        parms.workingBuffer = workingBuffer;
        parms.checksumIndex = LIGHT_CHECKSUM;
        parms.checksumBlockStart = LIGHT_CHECKSUM_BEGIN;
        parms.checksumBlockSize = LIGHT_CHECKSUM_END - LIGHT_CHECKSUM_BEGIN;
        parms.name = name;
        parms.value = value;

        return dweet->ProcessStateCommandsTable(&parms);
    }
}

//
// Application Structure:
//
// LightHouse.ino      - Arduino setup, resource and hardware assignments.
//                       Executes main loop.
//
// LightHouseApp.h
// LightHouseApp.cpp   - Implement application state.
//                       Application state queried/set by DweetLightHouse.
//                       Handle power on configuration from config store.
//
// LightHouseHardware.h
// LightHouseHardware.cpp   - Implement hardware state
//                
// DweetLightHouse.h
// DweetLightHouse.cpp - Implement the application specific Dweet commands
//                       which allow query state, set state, and query and
//                       modify power on/reset state configuration of the
//                       application and hardware.
//

int
DweetLightHouse::Initialize(
    LightHouseApp* lightHouseApp
    )
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[LIGHT_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(lighthouse_string_table) / sizeof(char*);

    //
    // Initialize our application state
    //
    m_lightHouseApp = lightHouseApp;

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (DweetLightHouse) since
    // the handlers are on this class.
    //
    // Improve: These stubs can be eliminated and direct calls
    // to the application class used.
    //

    parms.stringTable = (PGM_P)lighthouse_string_table;
    parms.functionTable = (PGM_P)lighthouse_function_table;
    parms.object =  this;
    parms.indexTable = lighthouse_index_table;
    parms.sizeTable = lighthouse_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = LIGHT_CHECKSUM;
    parms.checksumBlockStart = LIGHT_CHECKSUM_BEGIN;
    parms.checksumBlockSize = LIGHT_CHECKSUM_END - LIGHT_CHECKSUM_BEGIN;
    parms.name = NULL;
    parms.value = NULL;

    // DweetState.cpp
    result = MenloDweet::LoadConfigurationSettingsTable(&parms);
    if (result != 0) {
        if (result == DWEET_INVALID_CHECKSUM) {
            MenloDebug::Print(F("DweetLightHouse Stored settings checksum is invalid"));
        }
        else {
            MenloDebug::Print(F("DweetLightHouse Stored settings are invalid"));
        }
    }
    else {
        MenloDebug::Print(F("DweetLightHouse Stored settings are valid"));
    }

    return result;
}

int
DweetLightHouse::ProcessGetLightSequence(MenloDweet* dweet, char* name, char* value, bool persistent)
{
    PGM_P command;
    int size;
    bool result;

    // Allocate extra char for '\0'
    char buffer[LIGHT_SEQUENCE_SIZE + 1];

    //
    // No value with LIGHTGP | LIGHTGQ
    //

    if (persistent) {
        // LIGHTGP
        command = dweet_lightgp_string;
    }
    else {
        // LIGHTGQ
        command = dweet_lightgq_string;
    }

    result = m_lightHouseApp->GetLightSequence(buffer, sizeof(buffer), persistent);
    if (result) {

        //
        // A configured sequence can exceed the maximum NMEA 0183 buffer size
        //
        // LIGHTGQ_REPLY=
        // LIGHTGP_REPLY=
        //
        size = dweet->CalculateMaximumValueReply(command, dweet_reply_string, buffer);
        if ((int)strlen(buffer) > size) {
            // Update buffer size
            buffer[size] = '\0';
        }

        dweet->SendDweetItemReplyType_P(
             command,
             dweet_reply_string,
             buffer
             );
    }
    else {
        dweet->SendDweetItemReplyType_P(
            command,
            dweet_error_string,
            value
            );
    }

    return 1;
}

//
// Examples:
//
// All examples are using the default 250ms clock and 0 wait repeating period
// $PDWT,SETSTATE=LIGHTTICK:00FA
// $PDWT,SETSTATE=LIGHTPERIOD:0000
//
// $PDWT,SETCONFIG=LIGHTTICK:00FA   // save to EEPROM
// $PDWT,SETCONFIG=LIGHTPERIOD:0000 // save to EEPROM
//
// New Dungeness Light is quick flash every 5 seconds
// http://en.wikipedia.org/wiki/New_Dungeness_Light
//
// This setting uses 1/4 second for quick flash
//
// $PDWT,LIGHTSQ=28:0100000000*00
// $PDWT,LIGHTSP=28:0100000000*00   // save to EEPROM
//
// 1 second on, 1 second off (3) times, then 5 seconds pause
//
// $PDWT,LIGHTSQ=2C:0F0F0F000000*00
// $PDWT,LIGHTSP=2C:0F0F0F000000*00 // save to EEPROM
//
// 1/2 second intervals ISO
//
// $PDWT,LIGHTSQ=08:33*00
//
int
DweetLightHouse::ProcessSetLightSequence(MenloDweet* dweet, char* name, char* value, bool persistent)
{
    int size;
    bool result;
    PGM_P command;
    char buf[3];

    if (persistent) {
        command = dweet_lightsp_string;
    }
    else {
        command = dweet_lightsq_string;
    }

    //
    // LIGHTSQ=00:0000000
    // LIGHTSQ_ERROR=00:00
    //
    result = m_lightHouseApp->SetLightSequence(value, persistent);
    if (!result) {

        //
        // error in format
        //
        // Note: The reply to this request can be long enough to exceed the maximum
        // NMEA 0183 sentence so we truncate it.
        //
        // LIGHTSQ_ERROR=00:00...
        //
        size = dweet->CalculateMaximumValueReply(command, dweet_error_string, value);
        if ((int)strlen(value) > size) {
            // Update in buffer
            value[size] = '\0';
        }

        dweet->SendDweetItemReplyType_P(
	    command,
            dweet_error_string,
            value
            );

            return 1;
    }

    // We just reply with the bitCount
    buf[0] = value[0];
    buf[1] = value[1];
    buf[2] = '\0';

    dweet->SendDweetItemReplyType_P(
        command,
        dweet_reply_string,
        buf
        );

    return 1;
}

//
// SENSORS:0000.0000.0000.0000
// Each sensor responds in position. Based on configuration.
// This is designed to be generic, and not take up much code space.
//
int
DweetLightHouse::ProcessSensors(char* buffer, int size, bool isSet)
{
    int index;

    LightHouseSensors sensors;

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

    if (!m_lightHouseApp->readSensors(&sensors)) {
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
