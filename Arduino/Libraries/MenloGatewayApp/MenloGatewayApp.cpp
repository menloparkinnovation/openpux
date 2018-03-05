
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
 * File: MenloGatewayAppHardware.cpp
 *
 * MenloGateway Application class.
 */

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//
//
// MenloPlatform support
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

#include "MenloGatewayApp.h"

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
// Using Menlo Dweet Node.js application
//
// node dweet.js <portname>
//
// Note: Values are in hex. 0x7530 == 30000 milliseconds, or 30 seconds.
//                          0x1288 ==  5000 milliseconds, or  5 seconds.
//
// Set the power on interval in EEPROM
// This will also set a valid checksum for the configuration
// This also updates the runtime interval as if a reset had occured.
//
//   dweet SETCONFIG=BLINKINTERVAL:00007530
//
// Get the power on interval in EEPROM
//   dweet GETCONFIG=BLINKINTERVAL
//
// Set the temporary interval in memory. This setting
// is lost on power on/reset or watchdog timeout.
//
//   dweet SETSTATE=BLINKINTERVAL:00007530
//
// Get the interval that is active regardless of from
// EEPROM or a previous SETSTATE=BLINKINTERVAL command.
//
//   dweet GETSTATE=BLINKINTERVAL
//

const char gateway_module_name_string[] PROGMEM = "Gateway";

const char dweet_blinkinterval_string[] PROGMEM = "BLINKINTERVAL";

const char* const gateway_string_table[] PROGMEM =
{
    dweet_blinkinterval_string
};

// Locally typed version of state dispatch function
typedef int (MenloGatewayApp::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod gateway_function_table[] =
{
    &MenloGatewayApp::Interval
};

PROGMEM const int gateway_index_table[] =
{
  GATEWAY_INTERVAL_INDEX
};

PROGMEM const int gateway_size_table[] =
{
  GATEWAY_INTERVAL_SIZE
};

//
// Decode the interval which is up to 8 ASCII hex characters
// representing the 32 bit interval in milliseconds.
//
int
MenloGatewayApp::Interval(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {
        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            return DWEET_INVALID_PARAMETER;
        }

        // Update our application state
        m_config.interval = interval;

        // Reset timer interval to new value
        if (m_timer.IsTimerRegistered(&m_timerEvent)) {
            m_timer.UnregisterIntervalTimer(&m_timerEvent);
        }

        m_timerEvent.m_interval = m_config.interval;

        // Re-register timer if not zero
        if (m_timerEvent.m_interval != 0) {
            m_timer.RegisterIntervalTimer(&m_timerEvent);
        }
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.interval, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloGatewayApp::Initialize(MenloGatewayConfiguration* config)
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[GATEWAY_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(gateway_string_table) / sizeof(char*);

    MenloMemoryMonitor::CheckMemory(LineNumberBaseGatewayApp + __LINE__);

    m_config = *config;

    // Setup a TimerEvent for the requested period
    m_timerEvent.object = this;
    m_timerEvent.method = (MenloEventMethod)&MenloGatewayApp::TimerEvent;

    m_timerEvent.m_interval = m_config.interval;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    if (m_config.pinNumber != (-1)) {
        pinMode(m_config.pinNumber, OUTPUT);
    }

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (DweetBlink) since
    // the handlers are on this class.
    //
    parms.ModuleName = (PGM_P)gateway_module_name_string;
    parms.stringTable = (PGM_P)gateway_string_table;
    parms.functionTable = (PGM_P)gateway_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = gateway_index_table;
    parms.sizeTable = gateway_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = GATEWAY_CHECKSUM;
    parms.checksumBlockStart = GATEWAY_CHECKSUM_BEGIN;
    parms.checksumBlockSize = GATEWAY_CHECKSUM_END - GATEWAY_CHECKSUM_BEGIN;
    parms.name = NULL;
    parms.value = NULL;

    // DweetState.cpp
    result = MenloDweet::LoadConfigurationSettingsTable(&parms);
    if (result != 0) {
        if (result == DWEET_INVALID_CHECKSUM) {

            //
            // This error occurs if SETCONFIG has not been used to set
            // any valid settings since the device has been reflashed so
            // that the EEPROM is in an indeterminate state.
            //
            // It can also be the result of corruption of previously
            // valid EEPROM settings.
            //

            MenloDebug::Print(F("MenloGateway Stored settings checksum is invalid"));
        }
        else {

            //
            // This error occurs if one of the property handlers such as
            // Interval() rejects the value due to misconfigured or otherwise
            // corrupted value.
            //

            MenloDebug::Print(F("MenloGateway A stored property setting is invalid"));
        }
    }
    else {

        //
        // Configuration is loaded and enabled if:
        //
        // 1) The stored checksum for the applications configuration
        //    EEPROM block is valid.
        //
        // 2) The application property methods such as Interval() have
        //    accepted and activated the value on the application.
        //

        MenloDebug::Print(F("MenloGateway Stored settings are valid"));
    }

    MenloMemoryMonitor::CheckMemory(LineNumberBaseGatewayApp + __LINE__);

    //
    // Start the timer running if it was not already started
    // by Interval() being invoked by the EEPROM configuration
    // routines.
    //
    if (!m_timer.IsTimerRegistered(&m_timerEvent)) {
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }

    MenloMemoryMonitor::CheckMemory(LineNumberBaseGatewayApp + __LINE__);

    return 0;
}

unsigned long
MenloGatewayApp::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    DBG_PRINT("MenloGateway TimerEvent");

    if (m_lightToggle) {
        m_lightToggle = false;
    }
    else {
        m_lightToggle = true;
    }

    SetLightState(m_lightToggle);

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

int
MenloGatewayApp::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[GATEWAY_MAX_SIZE+1];

    int tableEntries = sizeof(gateway_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    DBG_PRINT("MenloGatewayApp calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.ModuleName = (PGM_P)gateway_module_name_string;
    parms.stringTable = (PGM_P)gateway_string_table;
    parms.functionTable = (PGM_P)gateway_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = gateway_index_table;
    parms.sizeTable = gateway_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = GATEWAY_CHECKSUM;
    parms.checksumBlockStart = GATEWAY_CHECKSUM_BEGIN;
    parms.checksumBlockSize = GATEWAY_CHECKSUM_END - GATEWAY_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
}
