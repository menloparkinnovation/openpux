
/*
 * Copyright (C) 2015,2016 Menlo Park Innovation LLC
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
 * Date: 01/31/2016
 * File: MenloMotionControlApp
 *
 * Application class for motion control (stepper, encoder).
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

#include "MenloMotionControlApp.h"

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
// TODO: Fill in
//

const char motioncontrol_module_name_string[] PROGMEM = "MotionControl";

const char dweet_microstep_string[] PROGMEM = "MICROSTEP";

const char dweet_steprate_string[]  PROGMEM = "STEPRATE";

const char dweet_powermode_string[] PROGMEM = "POWERMODE";

const char dweet_stepopenloop_string[] PROGMEM = "STEPOPENLOOP";

const char dweet_stepclosedloop_string[] PROGMEM = "STEPCLOSEDLOOP";

const char* const motioncontrol_string_table[] PROGMEM =
{
    dweet_microstep_string,
    dweet_steprate_string,
    dweet_powermode_string,
    dweet_stepopenloop_string,
    dweet_stepclosedloop_string
};

// Locally typed version of state dispatch function
typedef int (MenloMotionControlApp::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod motioncontrol_function_table[] =
{

    &MenloMotionControlApp::MicroStep,
    &MenloMotionControlApp::StepRate,
    &MenloMotionControlApp::PowerMode,
    &MenloMotionControlApp::StepOpenLoop,
    &MenloMotionControlApp::StepClosedLoop
};

PROGMEM const int motioncontrol_index_table[] =
{
  MOTIONCONTROL_MICROSTEP_INDEX,
  MOTIONCONTROL_STEPRATE_INDEX,
  MOTIONCONTROL_POWERMODE_INDEX,
  0, // StepOpenLoop   - no config entry
  0  // StepClosedLoop - no config entry
};

PROGMEM const int motioncontrol_size_table[] =
{
  MOTIONCONTROL_MICROSTEP_SIZE,
  MOTIONCONTROL_STEPRATE_SIZE,
  MOTIONCONTROL_POWERMODE_SIZE,
  MOTIONCONTROL_STEP_OPENLOOP_SIZE,
  MOTIONCONTROL_STEP_CLOSEDLOOP_SIZE
};

int
MenloMotionControlApp::MicroStep(char* buf, int size, bool isSet)
{
    int length;
    uint8_t mode;

    if (isSet) {
        mode = MenloUtility::HexToByte(buf);

        // Update our application state
        m_microstep = mode;

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
        MenloUtility::UInt8ToHexBuffer(m_microstep, buf);
        buf[2] = '\0';
    }

    return 0;
}

//
// StepRate is a ULONG
//
int
MenloMotionControlApp::StepRate(char* buf, int size, bool isSet)
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
        m_steprate = interval;

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
        MenloUtility::UInt32ToHexBuffer(m_steprate, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloMotionControlApp::PowerMode(char* buf, int size, bool isSet)
{
    int length;
    uint8_t mode;

    if (isSet) {
        mode = MenloUtility::HexToByte(buf);

        // Update our application state
        m_powermode = mode;

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
        MenloUtility::UInt8ToHexBuffer(m_powermode, buf);
        buf[2] = '\0';
    }

    return 0;
}

int
MenloMotionControlApp::StepOpenLoop(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long steps;

    if (isSet) {
        steps = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            return DWEET_INVALID_PARAMETER;
        }

        // Update our application state
        m_openLoopStepsTarget = steps;

        m_state = StateTransitionToSteppingOpenLoop;
        m_currentStateEndTime = 0;

        // Process the change of state
        ProcessStateEvents();

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
        MenloUtility::UInt32ToHexBuffer(m_openLoopStepsTarget, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloMotionControlApp::StepClosedLoop(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long steps;

    if (isSet) {
        steps = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            return DWEET_INVALID_PARAMETER;
        }

        // Update our application state
        m_closedLoopStepsTarget = steps;

        m_state = StateTransitionToSteppingClosedLoop;
        m_currentStateEndTime = 0;

        // Process the change of state
        ProcessStateEvents();
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_closedLoopStepsTarget, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloMotionControlApp::Initialize(MotionControlConfiguration* config)
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[MOTIONCONTROL_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(motioncontrol_string_table) / sizeof(char*);

    // Make a copy of the configuration. Callers typically have this on their stack.
    m_config = *config;

    m_state = StateIdlePowerOff;

    //
    // Setup a TimerEvent for the state machine
    //
    m_stateTimerEvent.object = this;
    m_stateTimerEvent.method = (MenloEventMethod)&MenloMotionControlApp::StateTimerEvent;

    m_stateTimerEvent.m_interval = MOTION_CONTROL_MINIMUM_STATE_TIME;
    m_stateTimerEvent.m_dueTime = 0L; // indicate not registered

    //
    // Setup the motion timer
    //
    m_motionTimerEvent.object = this;
    m_motionTimerEvent.method = (MenloEventMethod)&MenloMotionControlApp::MotionTimerEvent;

    m_motionTimerEvent.m_interval = MOTION_CONTROL_MINIMUM_MOTION_TIME;
    m_motionTimerEvent.m_dueTime = 0L; // indicate not registered

    //
    // Initialize the hardware class.
    //
    // This must be done before calling the table driven configuration
    // since it may attempt to program settings into the hardware based
    // on the values read.
    //
    HardwareInitialize();

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (DweetMotionControl) since
    // the handlers are on this class.
    //
    parms.ModuleName = (PGM_P)motioncontrol_module_name_string;
    parms.stringTable = (PGM_P)motioncontrol_string_table;
    parms.functionTable = (PGM_P)motioncontrol_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = motioncontrol_index_table;
    parms.sizeTable = motioncontrol_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = MOTIONCONTROL_CHECKSUM;
    parms.checksumBlockStart = MOTIONCONTROL_CHECKSUM_BEGIN;
    parms.checksumBlockSize = MOTIONCONTROL_CHECKSUM_END - MOTIONCONTROL_CHECKSUM_BEGIN;
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

            MenloDebug::Print(F("DweetMotionControl Stored settings checksum is invalid"));
        }
        else {

            //
            // This error occurs if one of the property handlers such as
            // Interval() rejects the value due to misconfigured or otherwise
            // corrupted value.
            //

            MenloDebug::Print(F("DweetMotionControl Stored settings are invalid"));
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

        MenloDebug::Print(F("DweetMotionControl Stored settings are valid"));
    }

    return 0;
}

//
// State Machine TimerEvent
//
unsigned long
MenloMotionControlApp::StateTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    ProcessStateEvents();

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

//
// This is running when motion is occuring
//
unsigned long
MenloMotionControlApp::MotionTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    //
    // Process current motion state
    //
    // Usually a step(s) per timer interval
    //

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

int
MenloMotionControlApp::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[MOTIONCONTROL_MAX_SIZE+1];

    int tableEntries = sizeof(motioncontrol_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    DBG_PRINT("DweetMotionControl calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.ModuleName = (PGM_P)motioncontrol_module_name_string;
    parms.stringTable = (PGM_P)motioncontrol_string_table;
    parms.functionTable = (PGM_P)motioncontrol_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = motioncontrol_index_table;
    parms.sizeTable = motioncontrol_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = MOTIONCONTROL_CHECKSUM;
    parms.checksumBlockStart = MOTIONCONTROL_CHECKSUM_BEGIN;
    parms.checksumBlockSize = MOTIONCONTROL_CHECKSUM_END - MOTIONCONTROL_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
}

//
// This processes the state events and sets the proper state
// based on the mode.
//
void
MenloMotionControlApp::ProcessStateEvents()
{
    unsigned long stateMachineInterval = 0;
    unsigned long current_time = millis();

    if (current_time < m_currentStateEndTime) {
        // Waiting until next state machine edge time
        return;
    }

    //
    // Process the motion state
    //
    switch (m_state) {

        case StateIdlePowerOff:
            // Do nothing
            break;

        case StateTransitionToIdlePowerOff:
            // Power off motor
            StepperPowerEnable(false);
            m_state = StateIdlePowerOff;
            break;

        case StateIdlePowerOn:
            // Power off motor if stepping is done, and mode motor off on idle
            // Could have a motor power timer/timeout
            m_state = StateTransitionToIdlePowerOff;
            break;

        // From stepping closed loop, open loop to idle with power on
        case StateTransitionToIdlePowerOn:
            // Send no more step commands
            m_state = StateIdlePowerOn;
            break;

        case StateTransitionToSteppingOpenLoop:
            m_state = StateSteppingOpenLoop;
            // Start stepper...
            StepperPowerEnable(true);
            break;

        case StateSteppingOpenLoop:
            // When done, transition to idle power on
            m_state = StateTransitionToIdlePowerOn;
            break;

        case StateTransitionToSteppingClosedLoop:
            m_state = StateSteppingClosedLoop;
            // Start stepper with encoder watch...
            StepperPowerEnable(true);
            break;

        case StateSteppingClosedLoop:
            // When done, transition to idle power on
            m_state = StateTransitionToIdlePowerOn;
            break;

        default:

        // Illegal state, reset to off for safety
        m_state = StateIdlePowerOff;
        StepperPowerEnable(false);
        break;
    }

    //
    // Process State machine timer
    //
    // The state machine timer allows the next state to be entered
    // after a state machine determined time.
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
// Motion Timer Support
//
// The motion timer is set when motion is occuring, and the timer
// interval determines the motion rate.
//
void
MenloMotionControlApp::StartMotionTimer(unsigned long motionInterval)
{
    if (motionInterval != 0) {

        // Reset timer interval to new value
        if (m_motionTimer.IsTimerRegistered(&m_motionTimerEvent)) {
            m_motionTimer.UnregisterIntervalTimer(&m_motionTimerEvent);
        }

        // Update interval
        m_motionTimerEvent.m_interval = motionInterval;

        // Restart timer on new interval
        m_motionTimer.RegisterIntervalTimer(&m_motionTimerEvent);
   }
   else {
        // Shutdown the timer
        if (m_motionTimer.IsTimerRegistered(&m_motionTimerEvent)) {
            m_motionTimer.UnregisterIntervalTimer(&m_motionTimerEvent);
        }
   }
}

//
// Idle the stepper.
//
// Cancels any outstanding stepper operations.
//
// Requests to place the stepper in the idle, power off state.
//
void
MenloMotionControlApp::IdleStepper()
{
    // TODO: Handle idle to power on, power off depending on power mode
    m_state = StateTransitionToIdlePowerOff;
}

//
// Stop the motion timer since the current motion command has completed.
//
void
MenloMotionControlApp::StopMotionTimer()
{
    // Shutdown the timer
    if (m_motionTimer.IsTimerRegistered(&m_motionTimerEvent)) {
        m_motionTimer.UnregisterIntervalTimer(&m_motionTimerEvent);
    }

    // Don't know the final motion state, so idle the stepper
    IdleStepper();
}
