
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
 *  Date: 01/31/2015
 *  File: DweetState.cpp
 *
 * Standard Dweet device state commands
 *
 * Used for Smartpux DWEET's.
 */

#include "MenloPlatform.h"
#include "MenloDebug.h"
#include "MenloMemoryMonitor.h"
#include "MenloUtility.h"
#include "MenloNMEA0183.h"
#include "MenloDweet.h"

#include <MenloConfigStore.h>

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
// This dispatches the given Set/Get state handler commands at
// the specified index.
//
// It passes the boolean argument.
//
// This dispatch is typed to the current class by using the "this" pointer.
//
int
MenloDweet::DispatchFunctionFromTable(
    PGM_P functionTable,       // table of PGM function pointers
    MenloObject* object,       // object "this" to invoke on
    int index,
    char* buf,
    int size_arg,
    bool bool_arg
    )
{
    int retVal;

    // Allows common use for SETSTATE/SETCONFIG
    if (functionTable == NULL) {
        xDBG_PRINT("DispatchFunction from table is null");
        return DWEET_NO_FUNCTION;
    }

    //
    // Compiler (GCC) does not allow this, so resort to the union trick.
    //
    // method = (StateMethod)m;
    //
    union {
      unsigned long p;
      StateMethod m;
    } method;

    // Can't do this on an AtMega due to separate I + D space
    // method = functionTable[index];

    //
    // MenloPlatform.h
    //

    method.p = MenloPlatform::GetMethodPointerFromMethodArray(
        (char**)functionTable, index);

    DBG_PRINT_NNL("function address ");
    DBG_PRINT_INT((uint16_t)method.p);

    // If the entry is null, return not implemented code
    if (method.p == NULL) {
        xDBG_PRINT("DispatchFunction from table entry is null");
        return DWEET_NO_FUNCTION;
    }

    retVal = ((object)->*(method.m))(buf, size_arg, bool_arg);

    return retVal;
}

//
// GETSTATE/SETSTATE, GETCONFIG/SETCONFIG are handled
// by a common table driven function.
//
// GETSTATE/SETSTATE impact the runtime operation of the
// device/application.
//
// GETCONFIG/SETCONFIG impact the stored power on configuration
// of the device/application.
//
// SETCONFIG first passes through SETSTATE if a function exists
// in the supplied STATE function table to validate the setting.
// If it succeeeds the new setting is saved to the power on
// persistent storage.
//
int
MenloDweet::ProcessStateCommandsTable(
    struct StateSettingsParameters* parms
    )
{
    PGM_P replyType;
    int index;
    int dataIndex;
    bool isSet;
    int retVal;
    int configDataLength;
    char errorBuffer[5];
    char* item = NULL;
    char* argumentBuffer = NULL;
    bool isSetConfig = false;

    DBG_PRINT("DweetState dispatch state function");

    // Check memory
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDweet + __LINE__);

    if (strncmp_P(parms->name, dweet_getconfig_string, 9) == 0)  {

        //
        // GETCONFIG=
        //
        // Read from the persistent storage to return the power on
        // state setting.
        //
        xDBG_PRINT("GETCONFIG table");

        retVal = ProcessGetConfigCommandsTable(
            parms->stringTable,
            parms->indexTable,
            parms->sizeTable,
            parms->tableEntries,
            parms->workingBuffer,
            parms->name,
            parms->value
            );

        return retVal;
    }
    else if (strncmp_P(parms->name, dweet_setconfig_string, 9) == 0)  {

        //
        // SETCONFIG=
        //
        // If there is a SETSTATE function invoke it first to
        // validate the proposed setting. If success, write it to
        // the permanent storage.
        //

        xDBG_PRINT("SETCONFIG table");

        isSet = true;
        isSetConfig = true;
        replyType = dweet_setconfig_string;

        //
        // ProcessSetConfigCommandTable has already processed
        // and queued the Dweet reply.
        //
        // We could execute the SETSTATE half in order to bring
        // the runtime state up to the configured state without
        // requiring a device restart.
        //
        // At the current time this is not done, as an application
        // that wants the state of the device to match the configuration
        // it just set can just send a SETSTATE command.
        //

        // Fall through to further processing
    }
    else if (strncmp_P(parms->name, dweet_getstate_string, 8) == 0)  {
        // GETSTATE=VALUE
        isSet = false;
        replyType = dweet_getstate_string;
    }
    else if (strncmp_P(parms->name, dweet_setstate_string, 8) == 0)  {
        // SETSTATE=
        isSet = true;
        replyType = dweet_setstate_string;
    }
    else {
        // Continue looking for a handler
        return 0;
    }

    //
    // GETSTATE/SETSTATE use a function dispatch table to handle
    // each active state request.
    //

    xDBG_PRINT_NNL("name is ");
    xDBG_PRINT_STRING(parms->name);

    xDBG_PRINT_NNL("value is ");
    xDBG_PRINT_STRING(parms->value);

    // Look for the entry
    index = LookupStringPrefixTableIndex(parms->stringTable, parms->tableEntries, parms->value);

    if (index == (-1)) {
        // No matching entry, continue looking for a handler
        xDBG_PRINT("statecommands no matching entry in table");
        return 0;
    }

    if (isSet) {

        //
        // All SETSTATE= parameters are item:action format so decode it now
        // and handle any errors.
        //
        xDBG_PRINT("SetState ProcessItemAction");

        xDBG_PRINT_NNL("name is ");
        xDBG_PRINT_STRING(parms->name);

        xDBG_PRINT_NNL("value is ");
        xDBG_PRINT_STRING(parms->value);

        //
        // This updates the string in place, so only should be called
        // once we know we have a match, which we did above with
        // LookupStringPrefixTableIndex()
        //
        if (ProcessItemAction(parms->value, &item, &argumentBuffer) == 0) {

            xDBG_PRINT("ProcessItemAction error");

            // error, not item:action format
            SendDweetItemReplyType_P(
                replyType,    // could be SETSTATE or SETCONFIG
                dweet_error_string,
                parms->value
                );

            return 1;
        }

        xDBG_PRINT("ProcessItemAction returned success");
    }
    else {
        // value is the object being requested
        item = parms->value;
        argumentBuffer = parms->workingBuffer;
    }

    configDataLength = pgm_read_word(&parms->sizeTable[index]);

    xDBG_PRINT("calling function dispatch");

    //
    // Now dispatch to the right function handler
    //

    // DweetState.cpp
    retVal = MenloDweet::DispatchFunctionFromTable(
        parms->functionTable,
        parms->object,
        index,
        argumentBuffer,
        configDataLength,
        isSet
        );

    if (isSetConfig) {

        // No function is ok for a SETCONFIG
        if ((retVal == 0) || (retVal == DWEET_NO_FUNCTION)) {

            //
            // Since we have split the value string into item:value
            // we must pass the components on
            //
            retVal = ProcessSetConfigCommandsTable(
                parms->stringTable,
                parms->indexTable,
                parms->sizeTable,
                parms->tableEntries,
                parms->checksumIndex,
                parms->checksumBlockStart,
                parms->checksumBlockSize,
                parms->name,
                item,
                argumentBuffer
                );

             // ProcessSetConfigCommandsTable sends the Dweet reply
             return retVal;
        }

        //
        // A SETSTATE function was specified, but it failed.
        // This indicates a problem with the proposed setting
        // and we don't want to write it to the configuration
        // store.
        //
        // Fall through to the rest of the error processing.
        // replyType has been set to SETCONFIG_REPLY
        //
    }

    if (retVal != 0) {

        xDBG_PRINT_NNL("function returned error ");
        xDBG_PRINT_INT(retVal);

        // error, return the four digit status code
        MenloUtility::UInt16ToHexBuffer(retVal, &errorBuffer[0]);
        errorBuffer[4] = '\0';

        SendDweetItemReplyType_P(
            replyType,
            dweet_error_string,
            errorBuffer
            );

        return 1;
    }
    else {

        xDBG_PRINT("function returned ok");

        SendDweetItemValueReply(
            replyType,
            item,
            argumentBuffer
            );

        return 1;
    }
}

//
// Load configuration settings re-using CONFIG and STATE commands
// tables of an application module.
//
int
MenloDweet::LoadConfigurationSettingsTable(
    struct StateSettingsParameters* parms
    )
{
    int index;
    int configDataIndex;
    int configDataLength;
    int retVal;
    bool result;
    int cachedError = 0;

    //
    // First validate the checksum range
    //
    result = ConfigStore.CalculateAndValidateCheckSumRange(
        parms->checksumIndex,
        parms->checksumBlockStart,
        parms->checksumBlockSize
        );

    if (!result) {
        xDBG_PRINT("Load Config Settings checksum range invalid");
        return DWEET_INVALID_CHECKSUM;
    }
    else {
        xDBG_PRINT("Load Config Settings checksum is valid");
    }

    //
    // For each entry read its CONFIG index + size and
    // function address. If all of these are not 0 or null
    // read the CONFIG data entry using the common MenloConfigStore
    // worker routine and then set it on the object using the set command.
    //
    for (index = 0; index < parms->tableEntries; index++) {

        configDataIndex = pgm_read_word(&parms->indexTable[index]);
        configDataLength = pgm_read_word(&parms->sizeTable[index]);
 
        if ((configDataIndex == 0) || (configDataLength == 0)) {
            xDBG_PRINT_NNL("Skipping entry index ");
            xDBG_PRINT_INT(index);
            continue;
        }

        // Get the config data
        ConfigStore.ReadConfig(configDataIndex, (uint8_t*)&parms->workingBuffer[0], configDataLength);
        ConfigStore.ProcessConfigBufferForValidChars(&parms->workingBuffer[0], configDataLength);
        parms->workingBuffer[configDataLength] = '\0';

        // Now try and set it on the object using the SETSTATE function
        retVal = MenloDweet::DispatchFunctionFromTable(
            parms->functionTable,
            parms->object,
            index,
            parms->workingBuffer,
            configDataLength,
            true
            );

#if XDBG_PRINT_ENABLED
        if (retVal == DWEET_NO_FUNCTION) {
            xDBG_PRINT_NNL("Load Config settings no function from table index is ");
            xDBG_PRINT_INT(index);
        }

        if (retVal == DWEET_INVALID_PARAMETER) {
            xDBG_PRINT_NNL("Load Config invalid parameter index ");
            xDBG_PRINT_INT(index);
        }
#endif
        if (retVal != 0) {
            PGM_P p;
            cachedError = retVal;

            //
            // This occurs because the SETSTATE function did not like
            // the parameter that was stored in the EEPROM even though
            // the checksum for the block is valid.
            //
            // This occurs when a setting was not configured, but
            // the block checksum was calculated on the default values.
            //
            // The application class then rejects the out of range value.
            //

            MenloDebug::PrintNoNewline(F("Config Error "));
            MenloDebug::PrintHexNoNewline(retVal);
            MenloDebug::PrintNoNewline(F(" index "));
            MenloDebug::PrintHex(index);

            // To help with diagnostics see if there is a string table entry
            if (parms->stringTable != NULL) {
                p = (PGM_P)MenloPlatform::GetStringPointerFromStringArray((char**)parms->stringTable, index);
                if (p != NULL) {
                   MenloDebug::PrintNoNewline(F("name "));
                   MenloDebug::Print_P(p); // need to be PGM_P
                }
            }
        }
    }

    return cachedError;
}
