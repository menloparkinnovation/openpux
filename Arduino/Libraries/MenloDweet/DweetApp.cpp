
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
 *  Date: 03/02/106
 *
 *  File: DweetApp.cpp
 *
 *  Template class for Dweet applications.
 */

//
// MenloPlatform support
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloPower.h>
#include <MenloFramework.h>

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>

#include <DweetApp.h>

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
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
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

DweetApp::DweetApp()
{
}

int
DweetApp::Initialize()
{
    //
    // Setup the Dweet event handler
    //
    m_dweetEvent.object = this;
    m_dweetEvent.method = (MenloEventMethod)&DweetApp::DweetEvent;

    //
    // This registers for the global unhandled Dweet event
    // notifications which can come in on any transport.
    //
    MenloDweet::RegisterGlobalUnhandledDweetEvent(&m_dweetEvent);

    return 1;
}

//
//
// This event is a listener for Dweets
//
// This allows composition of Dweet handlers through registered events
// vrs. class relationships.
//
// This may be invoked by multiple transports since the common function
// is used in all Dweet transport event registrations for this module.
//
unsigned long
DweetApp::DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    MenloDweetEventArgs* dweetArgs = (MenloDweetEventArgs*)eventArgs;

    // Check memory
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDweetSerialApp + __LINE__);

    DBG_PRINT("DweetApp DweetEvent");

    if (ProcessAppCommands(dweetArgs->dweet, dweetArgs->name, dweetArgs->value) == 0) {

        // Not handled
        SHIP_TRACE(TRACE_DWEET, 0x31);
        DBG_PRINT("DweetApp DweetEvent NOT HANDLED");
        return MAX_POLL_TIME;
    }
    else {
        // handled
        SHIP_TRACE(TRACE_DWEET, 0x32);
        DBG_PRINT("DweetApp DweetEvent WAS HANDLED");
        return 0;
    }
}
