
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
 * Date: 05/14/2016
 *
 * File: MenloTrace.h
 *
 * Tracing Dweet application class.
 *
 */

//
// MenloTrace Design:
//
//
// MenloTrace allows early application/firmware startup to configured
// a trace buffer that can record initialization, and runtime events
// in a compact format.
//
// This trace buffer can be set before any transports are initialized.
//
// A caller can supply a trace buffer, or malloc() allocation may be requested.
//
// This allows a deployment to use a static buffer it or a transport
// has allocated, or for it to switch buffers at runtime.
//
// The format buffer is supplied by a transport which will capture
// traces and transport them remotely for retrieval. This is transport
// dependent, and does not always have to be present, though commands
// to capture the current trace buffer would fail without a format
// buffer present. Typically a transport will retrieve the format
// buffer on command for remote telemetry and diagnostics.
//
// The format buffer must be twice the trace buffer, plus any additional
// overhead of the transport. This is because the trace format operation
// generates an ASCII hex string that uses two ASCII characters to
// represent each byte.
//
// Transports which wish to deal with binary transfers of the trace
// buffer may just retrieve the binary buffer directly and not
// reference the format routine. This will save code space as
// the format routine will not compile in if the Dweet portion is
// conditionally compiled as well.
//


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
#include <MenloTrace.h>
#include <MenloConfigStore.h>
#include <MenloPower.h>
#include <MenloTimer.h>

#include "MenloTrace.h"

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

const char menlotrace_module_name_string[] PROGMEM = "MenloTrace";

// SETCONFIG=TRACEMASK:00000000
const char menlotrace_setmask_string[] PROGMEM = "TRACESETMASK";

// SETSTATE=TRACECAPTURE:00
const char menlotrace_capture_string[] PROGMEM = "TRACECAPTURE";

const char* const menlotrace_string_table[] PROGMEM =
{
    menlotrace_setmask_string,
    menlotrace_capture_string
};

// Locally typed version of state dispatch function
typedef int (MenloTrace::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod menlotrace_function_table[] =
{
    &MenloTrace::SetMask,
    &MenloTrace::Capture
};

PROGMEM const int menlotrace_index_table[] =
{
    MENLOTRACE_SETMASK_INDEX,
    0 // Capture does not have an EEPROM entry
};

PROGMEM const int menlotrace_size_table[] =
{
    MENLOTRACE_SETMASK_SIZE,
    MENLOTRACE_CAPTURE_SIZE
};

MenloTrace::MenloTrace()
{
    m_tracingInitialized = false;
    m_traceBuffer = NULL;
    m_traceBufferSize = 0;
}

//
// Tracing providers allow early initialization of the tracing
// infrastructure.
//
void
MenloTrace::InitializeTracing(uint8_t* buffer, int bufferSize)
{
    if (m_tracingInitialized) {
        return;
    }

    m_tracingInitialized = true;

    if (bufferSize == 0) {
        // Done
        return;
    }

    //
    // If buffer is NULL, its a request for malloc()
    //
    if (buffer == NULL) {
        buffer = (uint8_t*)malloc(bufferSize);
        if (buffer == NULL) {
            // Failure to allocate trace buffer
            return;
        }
    }

    m_traceBuffer = buffer;
    m_traceBufferSize = bufferSize;

    bzero(m_traceBuffer, m_traceBufferSize);

    // By default enable tracing for startup time events
    MenloDebug::SetTraceBuffer(m_traceBuffer, m_traceBufferSize, 0);

    // "message": "Early Trace Buffer Started"
    SHIP_TRACE(TRACE_ALWAYS, 0x7D);
}

int
MenloTrace::Initialize()
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[MENLOTRACE_MAX_SIZE+1]; // Must be larger than any config values we fetch

    //
    // InitilizeTracing() is used to configure the tracing buffer.
    //
    // This initializes the MenloTrace Dweet commands support.
    //

    int tableEntries = sizeof(menlotrace_string_table) / sizeof(char*);

    // Setup Dweet handler callback
    DweetApp::Initialize();

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (MenloTrace) since
    // the handlers are on this class.
    //
    parms.ModuleName = (PGM_P)menlotrace_module_name_string;
    parms.stringTable = (PGM_P)menlotrace_string_table;
    parms.functionTable = (PGM_P)menlotrace_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = menlotrace_index_table;
    parms.sizeTable = menlotrace_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = MENLOTRACE_CHECKSUM;
    parms.checksumBlockStart = MENLOTRACE_CHECKSUM_BEGIN;
    parms.checksumBlockSize = MENLOTRACE_CHECKSUM_END - MENLOTRACE_CHECKSUM_BEGIN;
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

            MenloDebug::Print(F("MenloTrace Stored settings checksum is invalid"));
        }
        else {

            //
            // This error occurs if one of the property handlers such as
            // Interval() rejects the value due to misconfigured or otherwise
            // corrupted value.
            //

            MenloDebug::Print(F("MenloTrace A property handler rejected a stored setting"));
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

        MenloDebug::Print(F("MenloTrace Stored settings are valid"));
    }

    return 0;
}

int
MenloTrace::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[MENLOTRACE_MAX_SIZE+1];

    int tableEntries = sizeof(menlotrace_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    DBG_PRINT("MenloTrace calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.ModuleName = (PGM_P)menlotrace_module_name_string;
    parms.stringTable = (PGM_P)menlotrace_string_table;
    parms.functionTable = (PGM_P)menlotrace_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = menlotrace_index_table;
    parms.sizeTable = menlotrace_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = MENLOTRACE_CHECKSUM;
    parms.checksumBlockStart = MENLOTRACE_CHECKSUM_BEGIN;
    parms.checksumBlockSize = MENLOTRACE_CHECKSUM_END - MENLOTRACE_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
}

//
// Decode the mask which is up to 8 ASCII hex characters
// representing the 32 bit mask;
//
int
MenloTrace::SetMask(char* buf, int size, bool isSet)
{
    bool error;
    unsigned long mask;

    if (isSet) {
        mask = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            return DWEET_INVALID_PARAMETER;
        }

        // Update our application state
        MenloDebug::SetTraceMask((uint8_t)mask);
    }
    else {
        // get current value
        if (size < 9) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        mask = MenloDebug::GetTraceMask();

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(mask, buf);
        buf[8] = '\0';
    }

    return 0;
}

//
// Capture Trace Buffer Dweet command.
//
int
MenloTrace::Capture(char* buf, int size, bool isSet)
{
    int traceFormatSize;
    uint8_t* traceFormatBuffer;

    // This is a SETCONFIG only command
    if (!isSet) {
        return DWEET_GET_NOTSUPPORTED;
    }

    // Get the current trace format buffer
    MenloDebug::GetFormatBuffer(&traceFormatBuffer, &traceFormatSize);

    //
    // Only supported if a transport or application registers
    // a format buffer before hand.
    //
    if (traceFormatSize == 0) {
        return DWEET_NO_RESOURCE;
    }

    CaptureTraceBuffer((char*)traceFormatBuffer, traceFormatSize);

    return 0;
}

//
// Capture the trace buffer by formatting current tracing data
// into an ASCII hex format.
//
// It captures what ever is the current trace buffer.
//
// The format buffer is suppied by the caller so it can be invoked
// from a transport that retrieves the tracing data.
//
int
MenloTrace::CaptureTraceBuffer(char* formatBuffer, int formatBufferSize)
{
    int traceSize = 0;
    int traceIndex = 0;
    uint8_t* traceBuffer = NULL;

    if ((formatBuffer == NULL) || (formatBufferSize == 0)) {
        // No trace format buffer is available.
        return -1;
    }

    // Get the current trace buffer
    MenloDebug::GetTraceBuffer(&traceBuffer, &traceSize, &traceIndex);

    //
    // If non-zero, format it
    //
    if ((traceBuffer == NULL) || (traceSize == 0) || (traceIndex == 0)) {
        // No trace buffer is available
        return 0;
    }

    if (formatBufferSize < ((traceSize * 2) + 1)) {
        // Improper size
        return -1;
    }

    // Clear existing buffer, this will ensure null termination of the string
    bzero(formatBuffer, formatBufferSize);

    MenloUtility::UInt8ToHexBuffer(traceBuffer, traceSize, formatBuffer);

    //
    // Reset the trace buffer.
    //
    // Note: This resets the existing trace buffer, which may not be
    // the trace buffer supplied by this class.
    //
    bzero(traceBuffer, traceSize);

    // This clears the index back to zero
    MenloDebug::SetTraceBuffer(traceBuffer, traceSize, 0);

    return 1;
}

