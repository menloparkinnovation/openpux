
/*
 * Copyright (C) 2018 Menlo Park Innovation LLC
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
 * Date: 02/28/2018
 * File: MenloWatchDogApp.cpp
 *
 * MenloWatchDogApp.
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

#include "MenloWatchDogApp.h"

#define DBG_PRINT_ENABLED 1

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_HEX32(x)   (MenloDebug::PrintHex32(x))
#define DBG_PRINT_HEX32_NNL(x) (MenloDebug::PrintHex32NoNewline(x))
#define DBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define DBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_HEX32(x)
#define DBG_PRINT_HEX32_NNL(x)
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
// Hardware functions defined in MenloWatchDog.ino
//
void WatchDogSetResetPin(int value);

void WatchDogSetPowerPin(int value);

int WatchDogGetAndResetWatchDogCount();

void WatchDogIncrementCount();

void WatchDogSetLightState(bool state);

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

const char watchdog_module_name_string[] PROGMEM = "MenloWatchDogApp";

const char watchdog_timeout_string[] PROGMEM = "WATCHDOGTIMEOUT";

const char watchdog_reset_string[] PROGMEM = "WATCHDOGRESET";

const char watchdog_power_string[] PROGMEM = "WATCHDOGPOWER";

const char watchdog_ind_string[] PROGMEM = "WATCHDOGIND";

const char watchdog_resets_string[] PROGMEM = "WATCHDOGRESETS";

const char watchdog_poke_string[] PROGMEM = "WATCHDOGPOKE";

const char* const watchdog_string_table[] PROGMEM =
{
    watchdog_timeout_string,
    watchdog_reset_string,
    watchdog_power_string,
    watchdog_ind_string,
    watchdog_resets_string,
    watchdog_poke_string
};

// Locally typed version of state dispatch function
typedef int (MenloWatchDogApp::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod watchdog_function_table[] =
{
    &MenloWatchDogApp::Timeout,
    &MenloWatchDogApp::Reset,
    &MenloWatchDogApp::Power,
    &MenloWatchDogApp::Indicator,
    &MenloWatchDogApp::Resets,
    &MenloWatchDogApp::Poke
};

PROGMEM const int watchdog_index_table[] =
{
    WATCHDOG_TIMEOUT_INDEX,
    WATCHDOG_RESET_INDEX,
    WATCHDOG_POWER_INDEX,
    WATCHDOG_IND_INDEX,
    WATCHDOG_RESETS_INDEX,
    0 // no Configuration store entry
};

PROGMEM const int watchdog_size_table[] =
{
    WATCHDOG_TIMEOUT_SIZE,
    WATCHDOG_RESET_SIZE,
    WATCHDOG_POWER_SIZE,
    WATCHDOG_IND_SIZE,
    WATCHDOG_RESETS_SIZE,
    0 // no configuration store entry
};

// This ends up with all zero's, must debug.
// AutoConfig
// Auto Config
#define DWEET_STATE_ENABLE_DEFAULT_TABLE_SUPPORT 1

#if DWEET_STATE_ENABLE_DEFAULT_TABLE_SUPPORT

// Default value used for the interval (300 == 5 minutes)
const char watchdog_default_timeout[] PROGMEM = "0000012C";

// 200ms
const char watchdog_default_reset[] PROGMEM = "000000C8";

// 5000 milliseconds
const char watchdog_default_power[] PROGMEM = "00001388";

// 1 (TRUE)
const char watchdog_default_ind[] PROGMEM = "00000001";

// This causes a clear of the resets count on an EEPROM checksum failure or new initialization.
const char watchdog_default_resets[] PROGMEM = "00000000";

//
// Default values table.
//
// Entries without default values will not invoke the set function
// when there is a checksum error on the EEPROM configuration block.
//
// A valid EEPROM checksum, but rejected value will attempt to provide
// a default entry if its available in this table.
//
PROGMEM const struct StateSettingsDefaultValue watchdog_default_table[] =
{
    {WATCHDOG_TIMEOUT_INDEX, watchdog_default_timeout},
    {WATCHDOG_RESET_INDEX, watchdog_default_reset},
    {WATCHDOG_POWER_INDEX, watchdog_default_power},
    {WATCHDOG_IND_INDEX, watchdog_default_ind},
    {WATCHDOG_RESETS_INDEX, watchdog_default_resets},

    // This is required and indicates end of table
    {0, (PGM_P)NULL}
};

#endif

//
// The following routines are invoked automatically by the DWEET dispatcher
// based on the above tables for the following:
//
// 1) GETSTATE= and SETSTATE= DWEET routines to manipulate operating state.
//
// 2) Invoked at SETCONFIG= time to validate a setting and update application
//    state before setting thew new config value to the EEPROM.
//
// 3) Invoked at initialization time when default values are read from EEPROM
//    with a valid checksum to set them automatically into the operating state,
//    overriding any initialization values passed in.
//
// 4) Invoked at initialization time when EEPROM checksum is invalid, and a
//    defaults table, and setting entry for the value has been supplied.
//
// This model attempts to keep application state "automatically up to date"
// with changes to the runtime state (SETSTATE) and power on state (SETCONFIG),
// in addition to answering DWEET queries as to the operating state variables.
//
// This prevents writing the same code for this critical aspect of an IoT
// device deployment for every application and module.
//

//
// Set the watchdog timeout
//
int
MenloWatchDogApp::Timeout(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {
        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            MenloDebug::Print(F("MenloWatchDog invalid setting for WatchDog Timeout"));
            return DWEET_INVALID_PARAMETER;
        }

        MenloDebug::PrintNoNewline(F("MenloWatchDog setting WatchDog Timeout "));
        MenloDebug::PrintHex32(interval); 

        // Update our application state
        m_config.timeout_time = interval;

        // Reset timer interval to new value
        if (m_timer.IsTimerRegistered(&m_timerEvent)) {
            m_timer.UnregisterIntervalTimer(&m_timerEvent);
        }

        // Watchdog timeout time is in seconds
        m_timerEvent.m_interval = m_config.timeout_time * 1000L;

        // Restart timer on new interval
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.timeout_time, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloWatchDogApp::Reset(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {
        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            MenloDebug::Print(F("MenloWatchDog invalid setting for WatchDog RESET interval"));
            return DWEET_INVALID_PARAMETER;
        }

        MenloDebug::PrintNoNewline(F("MenloWatchDog setting WatchDog RESET Interval "));
        MenloDebug::PrintHex32(interval); 

        // Update our application state
        m_config.reset_time = interval;

        //
        // Timer is not touched since its programmed with this value during
        // a watchdog timeout reset cycle.
        //
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.reset_time, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloWatchDogApp::Power(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {
        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            MenloDebug::Print(F("MenloWatchDog invalid setting for WatchDog POWER interval"));
            return DWEET_INVALID_PARAMETER;
        }

        MenloDebug::PrintNoNewline(F("MenloWatchDog setting WatchDog POWER Interval "));
        MenloDebug::PrintHex32(interval); 

        // Update our application state
        m_config.power_time = interval;

        //
        // Timer is not touched since its programmed with this value during
        // a watchdog timeout reset/power cycle.
        //
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.power_time, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloWatchDogApp::Indicator(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {
        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            MenloDebug::Print(F("MenloWatchDog invalid setting for WatchDog indicator status"));
            return DWEET_INVALID_PARAMETER;
        }

        MenloDebug::PrintNoNewline(F("MenloWatchDog setting WatchDog indicator status "));
        MenloDebug::PrintHex32(interval); 

        // Update our application state
        m_config.indicator = interval;
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.indicator, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
MenloWatchDogApp::Resets(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {
        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            MenloDebug::Print(F("MenloWatchDog invalid setting for WatchDog Resets"));
            return DWEET_INVALID_PARAMETER;
        }

        MenloDebug::PrintNoNewline(F("MenloWatchDog setting WatchDog Resets "));
        MenloDebug::PrintHex32(interval); 

        // This resets the number of resets
        m_config.number_of_resets = interval;
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.number_of_resets, buf);
        buf[8] = '\0';
    }

    return 0;
}

//
// Poke the watchdog so it does not reset the target.
//
// SETSTATE=WATCHDOGPOKE:1
//
int
MenloWatchDogApp::Poke(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {
        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            MenloDebug::Print(F("MenloWatchDog invalid setting for WatchDog Poke"));
            return DWEET_INVALID_PARAMETER;
        }

        MenloDebug::Print(F("MenloWatchDog setting WatchDog Poke "));

        // Increment watchdog handshake
        WatchDogIncrementCount();
    }
    else {
        return DWEET_ERROR_UNSUP;
    }

    return 0;
}

int
MenloWatchDogApp::WriteResetsToEEPROM()
{
    char buf[10];

    // 8 hex chars without '\0'
    MenloUtility::UInt32ToHexBuffer(m_config.number_of_resets, buf);
    buf[8] = '\0';

    // Note: We add +1 to bufferLength to store the null
    ConfigStore.WriteConfig(WATCHDOG_RESETS_INDEX, (uint8_t*)buf, 9);

    ConfigStore.CalculateAndStoreCheckSumRange(
        WATCHDOG_CHECKSUM,
        WATCHDOG_CHECKSUM_BEGIN,
        WATCHDOG_CHECKSUM_END - WATCHDOG_CHECKSUM_BEGIN
        );

    return 0;
}

int
MenloWatchDogApp::Initialize(WatchdogConfiguration* config)
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[WATCHDOG_MAX_SIZE+1]; // Must be larger than any config values we fetch

    m_performingReset = false;
    m_performingPowerCycle = false;

    int tableEntries = sizeof(watchdog_string_table) / sizeof(char*);

    // Copy configuration entry defaults from caller.
    m_config = *config;

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog timeout_time "));
    MenloDebug::PrintHex32(m_config.timeout_time); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog reset_time "));
    MenloDebug::PrintHex32(m_config.reset_time); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog power_time "));
    MenloDebug::PrintHex32(m_config.power_time); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog number_of_resets "));
    MenloDebug::PrintHex32(m_config.number_of_resets); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog indicator "));
    MenloDebug::PrintHex32(m_config.indicator); 

    // Setup Dweet handler callback
    DweetApp::Initialize();

    // Setup a TimerEvent for the watchdog timeout period
    m_timerEvent.object = this;
    m_timerEvent.method = (MenloEventMethod)&MenloWatchDogApp::TimerEvent;

    // Watchdog timeout time is in seconds
    m_timerEvent.m_interval = m_config.timeout_time * 1000L;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    //
    // Setup the timer used when reset/power cycle event is occuring.
    //
    m_resetTimerEvent.object = this;
    m_resetTimerEvent.method = (MenloEventMethod)&MenloWatchDogApp::ResetTimerEvent;

    m_resetTimerEvent.m_interval = m_config.reset_time;
    m_resetTimerEvent.m_dueTime = 0L; // indicate not registered

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (MenloWatchDog) since
    // the handlers are on this class.
    //
    parms.ModuleName = (PGM_P)watchdog_module_name_string;
    parms.stringTable = (PGM_P)watchdog_string_table;
    parms.functionTable = (PGM_P)watchdog_function_table;
#if DWEET_STATE_ENABLE_DEFAULT_TABLE_SUPPORT
    parms.defaultsTable = watchdog_default_table;
#else
    parms.defaultsTable = NULL;
#endif
    parms.object =  this;
    parms.indexTable = watchdog_index_table;
    parms.sizeTable = watchdog_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = WATCHDOG_CHECKSUM;
    parms.checksumBlockStart = WATCHDOG_CHECKSUM_BEGIN;
    parms.checksumBlockSize = WATCHDOG_CHECKSUM_END - WATCHDOG_CHECKSUM_BEGIN;
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

            MenloDebug::Print(F("MenloWatchDog Stored settings checksum is invalid"));
        }
        else {

            //
            // This error occurs if one of the property handlers such as
            // Interval() rejects the value due to misconfigured or otherwise
            // corrupted value.
            //

            MenloDebug::Print(F("MenloWatchDog Stored settings are invalid"));
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

        MenloDebug::Print(F("MenloWatchDog Stored settings are valid"));
    }

    MenloDebug::Print(F("After EEPROM Setttings applied: "));

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog timeout_time "));
    MenloDebug::PrintHex32(m_config.timeout_time); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog reset_time "));
    MenloDebug::PrintHex32(m_config.reset_time); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog power_time "));
    MenloDebug::PrintHex32(m_config.power_time); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog number_of_resets "));
    MenloDebug::PrintHex32(m_config.number_of_resets); 

    MenloDebug::PrintNoNewline(F("Config MenloWatchDog setting WatchDog indicator "));
    MenloDebug::PrintHex32(m_config.indicator); 

    //
    // Start the timer running if it was not already started
    // by Timeout() being invoked by the EEPROM configuration
    // routines.
    //
    if (!m_timer.IsTimerRegistered(&m_timerEvent)) {
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }

    return 0;
}

//
// This occurs when the watchdog timeout period has expired.
//
unsigned long
MenloWatchDogApp::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    xDBG_PRINT("Watchdog TimerEvent");

    //
    // Test to see if any keep alive events have occurred from the
    // system being monitored.
    //
    // The routine WatchDogGetAndResetWatchDogCount() synchronizes with
    // the interrupt handler.
    //
    int keepAlivesSeen = WatchDogGetAndResetWatchDogCount();
    if (keepAlivesSeen != 0) {
        DBG_PRINT_NNL("Watchdog 0x");
        DBG_PRINT_HEX32_NNL(keepAlivesSeen); 
        DBG_PRINT(" keep alives seen from device reseting watchdog timer.");
        return MAX_POLL_TIME;
    }

    //
    // A watchdog timeout has occurred since no keep alives
    // have been seen.
    //
    // Perform the reset/power cycle.
    //

    //
    // Reset the reset timer interval to new value
    //
    if (m_resetTimer.IsTimerRegistered(&m_resetTimerEvent)) {
        m_resetTimer.UnregisterIntervalTimer(&m_resetTimerEvent);
    }

    //
    // WATCHDOG TIMEOUT:
    //

    m_config.number_of_resets++;

    //
    // Write the number_of_resets to EEPROM
    //

    WriteResetsToEEPROM();

    MenloDebug::Print(F(""));
    MenloDebug::Print(F("WATCHDOG TIMEOUT ON DEVICE"));
    MenloDebug::PrintNoNewline(F("NUMBER OF RESETS "));
    MenloDebug::PrintHex32(m_config.number_of_resets); 

    //
    // Start the reset cycle
    //

    // Light on indicates reset/power cycle is in process.
    SetLightState(true);

    MenloDebug::Print(F("WatchDog Timeout Starting Reset Cycle"));

    m_performingReset = true;
    m_performingPowerCycle = false;

    WatchDogSetResetPin(true);

    // Update interval
    m_resetTimerEvent.m_interval = m_config.reset_time;

    // Start timer on new interval
    m_resetTimer.RegisterIntervalTimer(&m_resetTimerEvent);

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

//
// This is used for the state machine when a reset/power cycle
// is being performed.
//
unsigned long
MenloWatchDogApp::ResetTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    xDBG_PRINT("Watchdog ResetTimerEvent");

    //
    // Disable self after executing
    //
    if (m_resetTimer.IsTimerRegistered(&m_resetTimerEvent)) {
        m_resetTimer.UnregisterIntervalTimer(&m_resetTimerEvent);
    }

    if (m_performingReset) {

        MenloDebug::Print(F("WatchDog Reset Cycle Complete, Starting Power Cycle"));

        // set power cycle pin to power down
        WatchDogSetPowerPin(false);

        // cancel reset pin
        WatchDogSetResetPin(false);

        m_performingReset = false;
        m_performingPowerCycle = true;

        // Update interval
        m_resetTimerEvent.m_interval = m_config.power_time;

        // Start timer on new interval
        m_resetTimer.RegisterIntervalTimer(&m_resetTimerEvent);
    }
    else if (m_performingPowerCycle) {

        MenloDebug::Print(F("WatchDog Power Cycle Complete"));
        MenloDebug::Print(F("WATCHDOG DEVICE RESET/POWER CYCLE COMPLETE"));
        MenloDebug::Print(F(""));

        // Set power pin back on
        WatchDogSetPowerPin(true);

        m_performingReset = false;
        m_performingPowerCycle = false;

        SetLightState(false);
    }

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

int
MenloWatchDogApp::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[WATCHDOG_MAX_SIZE+1];

    int tableEntries = sizeof(watchdog_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    xDBG_PRINT("Watchdog calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.ModuleName = (PGM_P)watchdog_module_name_string;
    parms.stringTable = (PGM_P)watchdog_string_table;
    parms.functionTable = (PGM_P)watchdog_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = watchdog_index_table;
    parms.sizeTable = watchdog_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = WATCHDOG_CHECKSUM;
    parms.checksumBlockStart = WATCHDOG_CHECKSUM_BEGIN;
    parms.checksumBlockSize = WATCHDOG_CHECKSUM_END - WATCHDOG_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
}

void
MenloWatchDogApp::SetLightState(bool state)
{
    WatchDogSetLightState(state);
}

