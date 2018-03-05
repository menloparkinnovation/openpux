
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
 *  Date: 05/28/2016 rewrote.
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
// Strings used in DweetSensorApp
//
const char sensorapp_module_name_string[] PROGMEM = "DweetSensorApp";

extern const char dweet_sensorrate_string[] PROGMEM = "SENSORRATE";

const char* const sensorapp_string_table[] PROGMEM =
{
  dweet_sensorrate_string
};

// Locally typed version of state dispatch function
typedef int (DweetSensorApp::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod sensorapp_function_table[] =
{
    &DweetSensorApp::SensorUpdateRate
};

PROGMEM const int sensorapp_index_table[] =
{
  SENSORAPP_SENSORRATE
};

PROGMEM const int sensorapp_size_table[] =
{
  SENSORAPP_SENSORRATE_SIZE
};

//
// Dweet commands processing.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetSensorApp::SubClassProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    // Dweet not handled
    xDBG_PRINT("DweetSensorApp: Default base SubClassProcessAppCommands invoked");
    return DWEET_EVENT_NOT_PROCESSED;
}

//
// Dweet commands processing.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetSensorApp::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    int retVal;
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[SENSORAPP_MAX_SIZE+1];

    //
    // The regular Dweet contract is that all modules process
    // the Dweet, but in the subclassing model the subclass
    // can override the handler.
    //
    retVal = SubClassProcessAppCommands(dweet, name, value);
    if (retVal != 0) {
        xDBG_PRINT("DweetSensorApp: SubClassProcessAppCommands returned 1");
        return retVal;
    }

    int tableEntries = sizeof(sensorapp_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    xDBG_PRINT("DweetSensorApp calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.ModuleName = (PGM_P)sensorapp_module_name_string;
    parms.stringTable = (PGM_P)sensorapp_string_table;
    parms.functionTable = (PGM_P)sensorapp_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = sensorapp_index_table;
    parms.sizeTable = sensorapp_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = SENSORAPP_CHECKSUM;
    parms.checksumBlockStart = SENSORAPP_CHECKSUM_BEGIN;
    parms.checksumBlockSize = SENSORAPP_CHECKSUM_END - SENSORAPP_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
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

        if (size < 5) {
            return DWEET_PARAMETER_TO_LONG;
        }

        updateRate = m_sensorTimerInterval / 1000L;

        MenloUtility::UInt16ToHexBuffer(updateRate, buf);

        return 0;
    }
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
    m_dweet = NULL;
    m_initialized = false;
    m_sendSensorUpdates = false;
}

int
DweetSensorApp::Initialize()
{
    //
    // Invoke base initialize
    //
    DweetApp::Initialize();

    xDBG_PRINT("DweetSensorApp Initialize returned from DweetApp base");

    //
    // Application state setup
    //

    //
    // Setup the sensor timer
    //

    m_sensorTimerInterval = SENSOR_DEFAULT_INTERVAL;

    // Standard args
    m_sensorTimerEvent.object = this;

    m_sensorTimerEvent.method = (MenloEventMethod)&DweetSensorApp::LocalSensorTimerEvent;

    // Object type specific args
    m_sensorTimerEvent.m_interval = m_sensorTimerInterval;
    m_sensorTimerEvent.m_dueTime = 0L; // indicate not registered

    xDBG_PRINT("DweetSensorApp Initialize invoking stored state config");

    //
    // Read the EEPROM for power on/reset state
    // settings.
    //
    InitializeStateFromStoredConfig();

    xDBG_PRINT("DweetSensorApp Initialize returned from stored state config");

    //
    // Now register our events now we are configured and ready
    // to receive them.
    //

    // Timer
    m_sensorTimer.RegisterIntervalTimer(&m_sensorTimerEvent);

    // Indicate we are initialized and can perform async processing
    m_initialized = true;

    return 0;
}

void
DweetSensorApp::InitializeStateFromStoredConfig()
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[SENSORAPP_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(sensorapp_string_table) / sizeof(char*);

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (DweetSensorApp) since
    // the handlers are on this class.
    //
    // Improve: These stubs can be eliminated and direct calls
    // to the application class used.
    //

    parms.ModuleName = (PGM_P)sensorapp_module_name_string;
    parms.stringTable = (PGM_P)sensorapp_string_table;
    parms.functionTable = (PGM_P)sensorapp_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = sensorapp_index_table;
    parms.sizeTable = sensorapp_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = SENSORAPP_CHECKSUM;
    parms.checksumBlockStart = SENSORAPP_CHECKSUM_BEGIN;
    parms.checksumBlockSize = SENSORAPP_CHECKSUM_END - SENSORAPP_CHECKSUM_BEGIN;
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
DweetSensorApp::LocalSensorTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    xDBG_PRINT("DweetSensorApp LocalSensorTimerEvent");

    // Allow sensor processing by subclasses.
    return SensorTimerEvent();
}

//
// This is typically overriden by subclasses that handle
// sensor hardware.
//
unsigned long
DweetSensorApp::SensorTimerEvent()
{
    unsigned long pollTime = MAX_POLL_TIME;

    return pollTime;
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
        m_sensorTimerInterval = (value * 1000L);
        m_sendSensorUpdates = true;
    }
    else {

        //
        // set m_sensorTimerInterval back to default as its
        // used for autonomous light on, light off handing.
        //
        m_sensorTimerInterval = SENSOR_DEFAULT_INTERVAL;
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
        m_sensorTimer.UnregisterIntervalTimer(&m_sensorTimerEvent);
    }

    // Update interval
    m_sensorTimerEvent.m_interval = m_sensorTimerInterval;

    if (m_initialized) {
        // Re-register timer
        xDBG_PRINT("SENSORRATE Re-registering timer");
        m_sensorTimer.RegisterIntervalTimer(&m_sensorTimerEvent);
    }
}
