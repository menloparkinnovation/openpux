
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
 * Date: 05/26/2015
 * File: MotionLightApp.h
 *
 * Application class.
 *
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

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>

#include "MotionLightApp.h"

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
//   dweet SETCONFIG=MOTIONLIGHTINTERVAL:00007530
//
// Get the power on interval in EEPROM
//   dweet GETCONFIG=MOTIONLIGHTINTERVAL
//
// Set the temporary interval in memory. This setting
// is lost on power on/reset or watchdog timeout.
//
//   dweet SETSTATE=MOTIONLIGHTINTERVAL:00007530
//
// Get the interval that is active regardless of from
// EEPROM or a previous SETSTATE=MOTIONLIGHTINTERVAL command.
//
//   dweet GETSTATE=MOTIONLIGHTINTERVAL
//

const char motionlight_module_name_string[] PROGMEM = "MotionLight";

//
// Time to remain on after a trigger
//
const char dweet_ontime_string[] PROGMEM = "ONTIME";

//
// Light Mode
//
// Mode 0 - 
//
//  Light turns on for ONTIME when trigger is seen.
//
//  Light turns off when ONTIME expires.
//
//  Light will turn back on again if another trigger occurs.
//
//  A LIGHTOFF command turns light off, but will turn back on again if
//  another trigger occurs.
//
// Mode 1 - 
//
//  Flashes at BLINKRATE for BLINKINTERVAL when initially triggered.
//
//  Light is steady for ONTIME.
//
//  Flashes at BLINKRATE for BLINKINTERVAL just before turning off.
//
//  A LIGHTOFF command turns light off, but will turn back on again if
//  another trigger occurs.
//
// Mode 2 - 
//
//  Flashes at BLINKRATE for for ONTIME.
//
//  A LIGHTOFF command turns light off, but will turn back on again if
//  another trigger occurs.
//
const char dweet_lightmode_string[] PROGMEM = "LIGHTMODE";

//
// Interval in ms that the light flashes when in blink mode
//
const char dweet_blinkrate_string[] PROGMEM = "BLINKRATE";

//
// The interval that it will flash in blink mode
//
const char dweet_blinkinterval_string[] PROGMEM = "BLINKINTERVAL";

//
// Time in ms to keep the light on
//
const char dweet_lighton_string[] PROGMEM = "LIGHTON";

//
// Time  in ms that the light will be off.
//
const char dweet_lightoff_string[] PROGMEM = "LIGHTOFF";

const char* const motionlight_string_table[] PROGMEM =
{
    dweet_ontime_string,
    dweet_lightmode_string,
    dweet_blinkrate_string,
    dweet_blinkinterval_string,
    dweet_lighton_string,
    dweet_lightoff_string
};

// Locally typed version of state dispatch function
typedef int (MotionLightApp::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod motionlight_function_table[] =
{
    &MotionLightApp::OnTime,
    &MotionLightApp::LightMode,
    &MotionLightApp::BlinkRate,
    &MotionLightApp::BlinkInterval,
    &MotionLightApp::LightOn,
    &MotionLightApp::LightOff
};

PROGMEM const int motionlight_index_table[] =
{
  MOTIONLIGHT_ONTIME_INDEX,
  MOTIONLIGHT_LIGHTMODE_INDEX,
  MOTIONLIGHT_BLINKRATE_INDEX,
  MOTIONLIGHT_BLINKINTERVAL_INDEX,
  0, // no config entry
  0  // no config entry
};

PROGMEM const int motionlight_size_table[] =
{
  MOTIONLIGHT_ONTIME_SIZE,
  MOTIONLIGHT_LIGHTMODE_SIZE,
  MOTIONLIGHT_BLINKRATE_SIZE,
  MOTIONLIGHT_BLINKINTERVAL_SIZE,
  MOTIONLIGHT_LIGHTON_SIZE,
  MOTIONLIGHT_LIGHTOFF_SIZE
};

//
// Decode the interval which is up to 8 ASCII hex characters
// representing the 32 bit light on interval in ms.
//
int
MotionLightApp::OnTime(char* buf, int size, bool isSet)
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

        if (interval < MOTIONLIGHT_MINIMUM_ON_TIME) {
            interval = MOTIONLIGHT_MINIMUM_ON_TIME;
        }

        // Update our application state
        m_ontime = interval;

        //
        // ProcessEvents() is not called since the new value
        // will be used by the timer if its running at its next interval.
        //
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_ontime, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MotionLightApp::LightMode(char* buf, int size, bool isSet)
{
    int length;
    uint8_t mode;

    if (isSet) {
        mode = MenloUtility::HexToByte(buf);

        // Update our application state
        m_lightMode = mode;

        //
        // ProcessEvents() is not called since the new value
        // will be used by the timer if its running at its next interval.
        //
    }
    else {
        // get current value
        if (size < 3) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 2 hex chars without '\0'
        MenloUtility::UInt8ToHexBuffer(m_lightMode, buf);
        buf[2] = '\0';
    }

    return 0;
}

int
MotionLightApp::BlinkRate(char* buf, int size, bool isSet)
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
        m_blinkrate = interval;

        //
        // ProcessEvents() is not called since the new value
        // will be used by the blink timer if its running at
        // its next interval.
        //
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_blinkrate, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MotionLightApp::BlinkInterval(char* buf, int size, bool isSet)
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
        m_blinkinterval = interval;

        //
        // ProcessEvents() is not called since the new value
        // will be used by the timer if its running at its next interval.
        //
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_blinkinterval, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MotionLightApp::LightOn(char* buf, int size, bool isSet)
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

        if (interval < MOTIONLIGHT_MINIMUM_ON_TIME) {
            interval = MOTIONLIGHT_MINIMUM_ON_TIME;
        }

        m_lightOnInterval = interval;

        m_state = StateTransitionToLightOn;
        m_currentStateEndTime = 0;

        // Process the change of state
        ProcessLightEvents();
    }
    else {

        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_lightOnInterval, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MotionLightApp::LightOff(char* buf, int size, bool isSet)
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

        if (interval < MOTIONLIGHT_MINIMUM_OFF_TIME) {
            interval = MOTIONLIGHT_MINIMUM_OFF_TIME;
        }

        m_lightOffInterval = interval;

        m_state = StateTransitionToLightOff;
        m_currentStateEndTime = 0;

        // Process the change of state
        ProcessLightEvents();
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_lightOffInterval, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MotionLightApp::Initialize(MotionLightConfiguration* config)
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[MOTIONLIGHT_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(motionlight_string_table) / sizeof(char*);

    m_config = *config;

    m_state = StateIdle;

    m_sendSensorUpdates = true;

    m_ontime = m_config.defaultOnTime;

    if (m_ontime < MOTIONLIGHT_MINIMUM_ON_TIME) {
         m_ontime = MOTIONLIGHT_MINIMUM_ON_TIME;
    }

    // Re-use the default on time
    m_sensorUpdateTime = m_config.defaultOnTime;

    // Setup a TimerEvent for the state machine
    m_stateTimerEvent.object = this;
    m_stateTimerEvent.method = (MenloEventMethod)&MotionLightApp::StateTimerEvent;

    m_stateTimerEvent.m_interval = MOTIONLIGHT_MINIMUM_ON_TIME;
    m_stateTimerEvent.m_dueTime = 0L; // indicate not registered

    // Setup a TimerEvent for the blink state machine
    m_blinkTimerEvent.object = this;
    m_blinkTimerEvent.method = (MenloEventMethod)&MotionLightApp::BlinkTimerEvent;

    m_blinkTimerEvent.m_interval = MOTIONLIGHT_MINIMUM_ON_TIME;
    m_blinkTimerEvent.m_dueTime = 0L; // indicate not registered

    // Setup a TimerEvent for the sensor updates
    m_sensorTimerEvent.object = this;
    m_sensorTimerEvent.method = (MenloEventMethod)&MotionLightApp::SensorTimerEvent;

    m_sensorTimerEvent.m_interval = MOTIONLIGHT_DEFAULT_SENSOR_INTERVAL;
    m_sensorTimerEvent.m_dueTime = 0L; // indicate not registered

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (DweetMotionlight) since
    // the handlers are on this class.
    //
    parms.ModuleName = (PGM_P)motionlight_module_name_string;
    parms.stringTable = (PGM_P)motionlight_string_table;
    parms.functionTable = (PGM_P)motionlight_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = motionlight_index_table;
    parms.sizeTable = motionlight_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = MOTIONLIGHT_CHECKSUM;
    parms.checksumBlockStart = MOTIONLIGHT_CHECKSUM_BEGIN;
    parms.checksumBlockSize = MOTIONLIGHT_CHECKSUM_END - MOTIONLIGHT_CHECKSUM_BEGIN;
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

            MenloDebug::Print(F("DweetMotionlight Stored settings checksum is invalid"));
        }
        else {

            //
            // This error occurs if one of the property handlers such as
            // Interval() rejects the value due to misconfigured or otherwise
            // corrupted value.
            //

            MenloDebug::Print(F("DweetMotionlight Stored settings are invalid"));
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

        MenloDebug::Print(F("DweetMotionlight Stored settings are valid"));
    }

    //
    // Register sensor update timer if not already set
    //
    if (!m_sensorTimer.IsTimerRegistered(&m_sensorTimerEvent)) {
        m_sensorTimer.RegisterIntervalTimer(&m_sensorTimerEvent);
    }

    return 0;
}

//
// State Machine TimerEvent
//
unsigned long
MotionLightApp::StateTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    ProcessLightEvents();

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

//
// This will run till cancelled by ProcessLightEvents()
//
unsigned long
MotionLightApp::BlinkTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
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
MotionLightApp::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[MOTIONLIGHT_MAX_SIZE+1];

    int tableEntries = sizeof(motionlight_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    DBG_PRINT("DweetMotionlight calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.ModuleName = (PGM_P)motionlight_module_name_string;
    parms.stringTable = (PGM_P)motionlight_string_table;
    parms.functionTable = (PGM_P)motionlight_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = motionlight_index_table;
    parms.sizeTable = motionlight_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = MOTIONLIGHT_CHECKSUM;
    parms.checksumBlockStart = MOTIONLIGHT_CHECKSUM_BEGIN;
    parms.checksumBlockSize = MOTIONLIGHT_CHECKSUM_END - MOTIONLIGHT_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
}

void
MotionLightApp::SetLightTriggerEvent()
{
    xDBG_PRINT("SetLightTriggerEvent");

    //
    // Triggers are ignored during force off time
    //
    if (m_state == StateLightOff) {
        return;
    }

    //
    // Normal mode flashes the light for each new trigger
    // event even if it is still on from a previous trigger
    // event. This is "follow me" mode.
    //

    if (m_lightMode == LightModeNormal) {

        //
        // This has the effect of interrupting a trigger
        // on with a restart of the blink -> on sequence.
        //
        m_state = StateTransitionToBlinkTriggered;
    }
    else {

        if (m_state == StateLightOnTriggered) {

            // If light is on, extend the time without blink
            m_state = StateTransitionToTriggered;
        }
        else {
            // Light is not already on, start blink sequence
            m_state = StateTransitionToBlinkTriggered;
        }
    }

    // Force re-evauluation of state
    m_currentStateEndTime = 0;

    ProcessLightEvents();
}

//
// This processes the light events and sets the proper state
// based on the mode.
//
void
MotionLightApp::ProcessLightEvents()
{
    unsigned long stateMachineInterval = 0;
    unsigned long current_time = millis();

    if (current_time < m_currentStateEndTime) {
        // Waiting until next state machine edge time
        return;
    }

    //
    // Process the light state
    //
    switch (m_state) {

        case StateIdle:
            break;

        case StateTransitionToLightOn:
            SetLightState(true);
            stateMachineInterval = m_lightOnInterval;
            m_state = StateLightOn;
            break;

        case StateLightOn:
            // Transition light to off as on time has expired
            SetLightState(false);
            m_currentStateEndTime = 0;
            m_state = StateIdle;
            break;

        case StateTransitionToLightOff:
            SetLightState(false);
            stateMachineInterval = m_lightOffInterval;
            m_state = StateLightOff;
            break;

        case StateLightOff:
            // Cancel the force light off as the time has expired
            m_currentStateEndTime = 0;
            m_state = StateIdle;
            break;

        case StateTransitionToTriggered:
            SetLightState(true);
            stateMachineInterval = m_ontime;
            m_state = StateLightOnTriggered;
            break;

        case StateLightOnTriggered:
            // Cancel after light on time has expired
            SetLightState(false);
            m_currentStateEndTime = 0;
            m_state = StateIdle;
            break;

        case StateTransitionToBlinkTriggered:
            StartBlink(m_blinkrate);
            stateMachineInterval = m_blinkinterval;
            m_state = StateBlinkTriggered;
            break;

        case StateBlinkTriggered:
            StopBlink();
            SetLightState(true);
            stateMachineInterval = m_ontime;
            m_state = StateLightOnTriggered;
            break;

        default:

        // Illegal state, reset to off to save battery
        m_state = StateIdle;
        SetLightState(false);
        break;
    }

    //
    // Process State machine timer
    //
    if (stateMachineInterval != 0) {

        // Reset timer interval to new value
        if (m_stateTimer.IsTimerRegistered(&m_stateTimerEvent)) {
            m_stateTimer.UnregisterIntervalTimer(&m_stateTimerEvent);
        }

        // Update interval
        m_stateTimerEvent.m_interval = stateMachineInterval;

        // Restart timer on new interval
        m_stateTimer.RegisterIntervalTimer(&m_stateTimerEvent);
   }
   else {
        // Shutdown the timer
        if (m_stateTimer.IsTimerRegistered(&m_stateTimerEvent)) {
            m_stateTimer.UnregisterIntervalTimer(&m_stateTimerEvent);
        }
   }
}

//
// State machine workers
//
void
MotionLightApp::StartBlink(unsigned long blinkInterval)
{
    if (blinkInterval != 0) {

        // Reset timer interval to new value
        if (m_blinkTimer.IsTimerRegistered(&m_blinkTimerEvent)) {
            m_blinkTimer.UnregisterIntervalTimer(&m_blinkTimerEvent);
        }

        // Update interval
        m_blinkTimerEvent.m_interval = blinkInterval;

        // Restart timer on new interval
        m_blinkTimer.RegisterIntervalTimer(&m_blinkTimerEvent);
   }
   else {
        // Shutdown the timer
        if (m_blinkTimer.IsTimerRegistered(&m_blinkTimerEvent)) {
            m_blinkTimer.UnregisterIntervalTimer(&m_blinkTimerEvent);
        }
   }
}

void
MotionLightApp::StopBlink()
{
    // Shutdown the timer
    if (m_blinkTimer.IsTimerRegistered(&m_blinkTimerEvent)) {
        m_blinkTimer.UnregisterIntervalTimer(&m_blinkTimerEvent);
    }

    // Don't know the final light state, so turn it off
    SetLightState(false);
}

unsigned long
MotionLightApp::SensorTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    MotionLightSensorData data;

    xDBG_PRINT("MotionLight SensorTimerEvent");

    memset(&data, 0, sizeof(data));

    if (m_sendSensorUpdates) {

        xDBG_PRINT("Sending radio sensor update");

        //
        // Queue a radio sensor update packet if configured
        //

        data.type = MOTIONLIGHT_SENSOR_DATA_PACKET;
        data.subType = MOTIONLIGHT_SENSOR_DATA_SUBTYPE;

        if (m_state == StateLightOnTriggered) {
            data.lightTriggered = 1;
            data.lightState = 1;
        }
        else {
            data.lightTriggered = 0;
            data.lightState = 0;
        }

        // Get current status of the trigger
        data.triggerState = GetTriggerState();
        data.battery = 0;
        data.solar = 0;

        if (m_radio != NULL) {

            xDBG_PRINT("Writing to radio");

	    m_radio->Write(
   		NULL, // Use the configured address
                (uint8_t*)&data,
                (uint8_t)sizeof(data),
                250  // 250 ms
                );
        }
    }

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

void
MotionLightApp::SensorUpdateRate(uint16_t* value, bool isSet)
{
    if (!isSet) {
        *value = m_sensorUpdateTime / 1000L;
        return;
    }

    //
    // If a sensor update rate is specified other
    // than zero sensor updates are scheduled at
    // the specified rate in seconds.
    //
    // If an update rate value of 0 is specified, no
    // sensor updates are sent.
    //
    if (*value != 0) {
        m_sensorUpdateTime = (*value * 1000L);
        m_sendSensorUpdates = true;
    }
    else {

        //
        // Set back to the default
        //
        m_sensorUpdateTime = m_config.defaultOnTime;
        m_sendSensorUpdates = false;
    }

    // Reset timer interval to new value
    if (m_sensorTimer.IsTimerRegistered(&m_sensorTimerEvent)) {
        m_sensorTimer.UnregisterIntervalTimer(&m_sensorTimerEvent);
    }

    // Update interval
    m_sensorTimerEvent.m_interval = m_sensorUpdateTime;

    // Restart timer on new interval
    m_sensorTimer.RegisterIntervalTimer(&m_sensorTimerEvent);

    return;
}

void
MotionLightApp::SetRadio(
    MenloRadio* radio
    )
{
    // This could be NULL, which means no radio
    m_radio = radio;
}

