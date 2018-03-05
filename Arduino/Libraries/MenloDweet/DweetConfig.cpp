
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
 *  Date: 01/07/2015
 *  File: DweetConfig.cpp
 *
 * Standard Dweet Configuration commands
 *
 * Used for Smartpux DWEET's.
 */

#include "MenloPlatform.h"
#include "MenloPower.h"
#include "MenloDebug.h"
#include "MenloMemoryMonitor.h"
#include "MenloNMEA0183.h"
#include "MenloDweet.h"

#include <MenloConfigStore.h>

// Debug tracing for development
#define DBG_TRACE_ENABLED 0

#if DBG_TRACE_ENABLED
#define DBG_TRACE(l, x)           (MenloDebug::Trace(l, x))
#define DBG_TRACE_STRING(l, x, d) (MenloDebug::TraceString(l, x, d))
#define DBG_TRACE_INT(l, x, d)    (MenloDebug::TraceInt(l, x, d))
#define DBG_TRACE_LONG(l, x, d)   (MenloDebug::TraceLong(l, x, d))
#define DBG_TRACE_BYTE(l, x, d)   (MenloDebug::TraceByte(l, x, d))
#else
#define DBG_TRACE(l, x)
#define DBG_TRACE_STRING(l, x, d)
#define DBG_TRACE_INT(l, x, d)
#define DBG_TRACE_LONG(l, x, d)
#define DBG_TRACE_BYTE(l, x, d)
#endif

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_STRING_P(x)  (MenloDebug::Print_P(x))
#define DBG_PRINT_NNL(x)  (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x) (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_STRING_P(x)
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
#define xDBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_STRING_P(x)  (MenloDebug::Print_P(x))
#define xDBG_PRINT_NNL(x)  (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x) (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_STRING_P(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

//
// Strings are declared once and re-used to save program space
//
const char config_module_name_string[] PROGMEM = "DweetConfig";

const char dweet_model_string[] PROGMEM = "MODEL";
const char dweet_name_string[] PROGMEM = "NAME";
const char dweet_serial_string[] PROGMEM = "SERIAL";
const char dweet_version_string[] PROGMEM = "VERSION";
const char dweet_firmwareversion_string[] PROGMEM = "FIRMWAREVERSION";

// CPU Power Support
const char dweet_sleepmode_string[] PROGMEM = "SLEEPMODE";
const char dweet_cpuspeed_string[] PROGMEM = "CPUSPEED";
const char dweet_sleeptime_string[] PROGMEM = "SLEEPTIME";
const char dweet_awaketime_string[] PROGMEM = "AWAKETIME";

//
// State commands
//
const char dweet_tracelevel_string[] PROGMEM = "TRACELEVEL";

// Ignore NMEA 0183 checksum errors (allows easy command console input)
const char dweet_nochecksum_string[] PROGMEM = "NOCHECKSUM";

//
// MenloDweet only deals with normal character
// configuration settings. This avoids having to
// deal with escapes, non-printables, etc.
//
// It uses the ASCII validation routines of
// the MenloConfigStore.
//

//
// These are the builtin SETCONFIG/GETCONFIG and
// SETSTATE/GETSTATE Dweet commands
//

const char* const config_string_table[] PROGMEM =
{
  dweet_model_string,
  dweet_name_string,
  dweet_serial_string,
  dweet_version_string,
  dweet_firmwareversion_string,
  dweet_tracelevel_string,
  dweet_nochecksum_string,
  dweet_sleepmode_string,
  dweet_cpuspeed_string,
  dweet_sleeptime_string,
  dweet_awaketime_string
};

// Locally typed version of state dispatch function
typedef int (MenloDweet::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod config_function_table[] =
{
    0,   // model
    0,   // name
    0,   // serial
    0,   // version
    0,   // firmwareversion
    &MenloDweet::TraceLevelHandler,
    &MenloDweet::NoChecksumHandler,
    &MenloDweet::SleepModeHandler,
    &MenloDweet::CpuSpeedHandler,
    &MenloDweet::SleepTimeHandler,
    &MenloDweet::AwakeTimeHandler
};

PROGMEM const int config_index_table[] =
{
    MODEL_INDEX,
    NAME_INDEX,
    SERIAL_INDEX,
    VERSION_INDEX,
    FIRMWARE_VERSION_INDEX,
    0,    // No persistent storage of trace level
    0,    // No persistent storage of checksum ignore
    SLEEPMODE_INDEX,
    CPUSPEED_INDEX,
    0,   // No persistent storage of sleep time
    0    // No persistent storage of awake time
};

PROGMEM const int config_size_table[] =
{
    MODEL_SIZE,
    NAME_SIZE,
    SERIAL_SIZE,
    VERSION_SIZE,
    FIRMWARE_VERSION_SIZE,
    TRACELEVEL_SIZE,
    NOCHECKSUM_SIZE,
    SLEEPMODE_SIZE,
    CPUSPEED_SIZE,
    SLEEPTIME_SIZE,
    AWAKETIME_SIZE
};

int
MenloDweet::NoChecksumHandler(char* buf, int size, bool isSet)
{
    if (isSet) {

        if (size < 2) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        if (buf[1] == '0') {
            m_ignoreChecksumErrors = false;
        }
        else if (buf[1] == '1') {
            m_ignoreChecksumErrors = true;
        }
        else {
            return DWEET_INVALID_PARAMETER;
        }
    }
    else {

        if (size < 3) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        buf[0] = '0';
        if (m_ignoreChecksumErrors) {
            buf[1] = '1';
        }
        else {
            buf[1] = '0';
        }

        buf[2] = '\0';
    }

    return 0;
}

int
MenloDweet::TraceLevelHandler(char* buf, int size, bool isSet)
{
    if (size < 2) {
        return DWEET_PARAMETER_TO_SHORT;
    }

    if (isSet) {

        if (!((buf[0] >= '0') && (buf[0] <= '9'))) {
            return DWEET_INVALID_PARAMETER;
        }

        // Convert from ASCII to a byte binary value 0 - 9
        SetTraceLevel(buf[0] - '0');
    }
    else {
        buf[0] = GetTraceLevel() + '0';
        buf[1] = '\0';
    }

    return 0;
}

int
MenloDweet::SleepModeHandler(char* buf, int size, bool isSet)
{
    return Power.SleepMode(buf, size, isSet);
}

int
MenloDweet::CpuSpeedHandler(char* buf, int size, bool isSet)
{
    return Power.CpuSpeed(buf, size, isSet);
}

int
MenloDweet::SleepTimeHandler(char* buf, int size, bool isSet)
{
    return Power.SleepTime(buf, size, isSet);
}

int
MenloDweet::AwakeTimeHandler(char* buf, int size, bool isSet)
{
    return Power.AwakeTime(buf, size, isSet);
}

//
// Handler for built in commands for model, serial number,
// firmware, CPU control, etc.
//
int
MenloDweet::ProcessBuiltInCommands(char* name, char* value)
{
    struct StateSettingsParameters parms;

    char workingBuffer[BUILTIN_MAX_SIZE+1]; // MAX_SIZE + plus '\0'
    int tableEntries = sizeof(config_string_table) / sizeof(char*);

    //
    // Note: No function table is specified. One could be added
    // to validate the format of the values placed into the configuration
    // store. Currently we allow any valid configuration store character
    // up to the configured length.
    //

    parms.ModuleName = (PGM_P)config_module_name_string;
    parms.stringTable = (PGM_P)config_string_table;
    parms.functionTable = (PGM_P)config_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = config_index_table;
    parms.sizeTable = config_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = BUILTIN_CHECKSUM;
    parms.checksumBlockStart = BUILTIN_CHECKSUM_BEGIN;
    parms.checksumBlockSize = BUILTIN_CHECKSUM_END - BUILTIN_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    // DweetState.cpp
    return ProcessStateCommandsTable(&parms);
}

//
// GETCONFIG handler that is table driven to save code space and
// make adding new GETCONFIG commands easier.
//
// This is used by both this module and application modules.
//
// Caller is responsible for making sure working buffer
// is as large as the largest entry in sizeTable.
//
int
MenloDweet::ProcessGetConfigCommandsTable(
    PGM_P configTable,         // table of PGM character strings
    const int* indexTable,     // table of PGM int's
    const int* sizeTable,      // table of PGM int's
    int tableEntries,          // number of entries in the above tables
    char* workingBuffer,
    char* name,
    char* value
    )
{
    PGM_P p;
    int index;
    int length;
    int dataIndex;
    int bufferLength;

    // GETCONFIG=
    if (strncmp_P(name, dweet_getconfig_string, 9) != 0)  {
        // Continue looking for a handler
        return 0;
    }

    // Look for the entry
    // MenloDweet.cpp
    index = LookupStringPrefixTableIndex(configTable, tableEntries, value);
    if (index == (-1)) {
        // No matching entry, continue looking for a handler
        return 0;
    }

    // MenloPlatform.h
    p = (PGM_P)MenloPlatform::GetStringPointerFromStringArray((char**)configTable, index);
    length = strlen_P(p);

    dataIndex = pgm_read_word(&indexTable[index]);
    bufferLength = pgm_read_word(&sizeTable[index]);

    //
    // A zero in the configuration index or size means
    // GETCONFIG/SETCONFIG is not supported.
    //
    if ((dataIndex == 0) || (bufferLength == 0)) {
        // We matched on the string, but did not handle. Allow app override.
        return 0;
    }

    ConfigStore.ReadConfig(dataIndex, (uint8_t*)&workingBuffer[0], bufferLength);

     //
     // This will translate any invalid chars to '0', but leave
     // '\0''s in place.
     //
     ConfigStore.ProcessConfigBufferForValidChars(&workingBuffer[0], bufferLength);

     workingBuffer[bufferLength] = '\0';

     value[length] = '\0';

     // GETCONFIG_REPLY=item:value
     SendDweetItemValueReplyType(
         dweet_getconfig_string,
         dweet_reply_string,
         value,
         workingBuffer
         );

     // Handled
     return 1;
}

//
// SETCONFIG handler that is table driven to save
// code space and make adding new SETCONFIG commands easier.
//
// This is used by both this module and application modules.
//
// Caller is responsible for making sure working buffer
// is as large as the largest entry in sizeTable.
//
int
MenloDweet::ProcessSetConfigCommandsTable(
    PGM_P configTable,         // table of PGM character strings
    const int* indexTable,     // table of PGM int's
    const int* sizeTable,      // table of PGM int's
    int tableEntries,          // number of entries in the above tables
    int checksumIndex,         // index of checksum (always 2 ASCII chars)
    int checksumBlockStart,    // start block that is checksummed
    int checksumBlockSize,     // size of block that is checksummed
    char* name,                // SETCONFIG
    char* item,                // item name such as RADIOCHANNEL
    char* action               // action/value such as 01
    )
{
    int index;
    int configDataIndex;
    int configDataLength;
    int bufferLength;

#if MASTER_DEBUG_DETAILED_MEMORY_TRACING
    //
    // Suspect overflows here on Uno, but not a lot of memory left.
    // This call takes 10 bytes on AtMega328.
    //
    MenloMemoryMonitor::CheckMemory(__LINE__);
#endif

    // SETCONFIG=
    if (strncmp_P(name, dweet_setconfig_string, 9) != 0)  {
        // Continue looking for a handler
        return 0;
    }

    xDBG_PRINT_NNL("SETCONFIG item is ");
    xDBG_PRINT_STRING(item);

    // Look for the entry
    index = LookupStringPrefixTableIndex(configTable, tableEntries, item);
    if (index == (-1)) {
        xDBG_PRINT("SETCONFIG no table entry");
        // No matching entry, continue looking for a handler

        // "message": "ProcessSetConfigCommandsTable: No SETCONFIG entry in table"
        SHIP_TRACE(TRACE_DWEET, 0x20);
        return 0;
    }

    // Debug code
    // MenloPlatform.h
    //int length;
    //PGM_P p;
    //p = (PGM_P)MenloPlatform::GetStringPointerFromStringArray((char**)configTable, index);
    //length = strlen_P(p);
    //xDBG_PRINT_NNL("length is ");
    //xDBG_PRINT_INT(length);

    configDataIndex = pgm_read_word(&indexTable[index]);
    configDataLength = pgm_read_word(&sizeTable[index]);

    //
    // A zero in the configuration index or size means
    // GETCONFIG/SETCONFIG is not supported.
    //
    if ((configDataIndex == 0) || (configDataLength == 0)) {
        // We matched on the string, but did not handle. Allow app override.
        xDBG_PRINT("SETCONFIG no config data index or length");
        // "message": "ProcessSetConfigCommandsTable: No GET/SET CONFIG supported for entry"
        SHIP_TRACE(TRACE_DWEET, 0x21);
        return 0;
    }

    // action is value to store
    bufferLength = strlen(action);
    if (bufferLength > configDataLength) {
        bufferLength = configDataLength;
    }

    action[bufferLength] = '\0';

    // Don't let invalid config chars be written to the store
    if (ConfigStore.ConfigBufferHasInvalidChars(action, bufferLength)) {

        MenloDebug::Print(F("SETCONFIG invalid chars"));

        // "message": "ProcessSetConfigCommandsTable: SETCONFIG has invalid chars"
        SHIP_TRACE(TRACE_DWEET, 0x22);

        SendDweetItemValueReplyType(
            dweet_setconfig_string,
            dweet_error_string,
            item,
            action
            );

        // Handled
        return 1;
    }

    // Note: We add +1 to bufferLength to store the null
    ConfigStore.WriteConfig(configDataIndex, (uint8_t*)action, bufferLength + 1);

    //
    // See if the caller has specified a checksum range.
    //
    // If so re-calculate the checksum for the entire block the
    // caller specified to include the new contents.
    //
    if (checksumIndex != 0) {
        ConfigStore.CalculateAndStoreCheckSumRange(
            checksumIndex,
            checksumBlockStart,
            checksumBlockSize
            );
    }

    //
    // We return the actual buffer set which aids in diagnostics
    //

    // SETCONFIG_REPLY=item:value
    SendDweetItemValueReplyType(
        dweet_setconfig_string,
        dweet_reply_string,
        item,
        action
        );

    // Handled
    return 1;
}
