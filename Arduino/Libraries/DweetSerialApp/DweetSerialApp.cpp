
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
 *  Date: 05/02/2015
 *  File: DweetSerialApp.cpp
 *
 * Template for MenloDweet Application on a serial connection.
 *
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
#include <DweetSerialChannel.h>

#include <MenloDispatchObject.h>
#include <MenloTimer.h>

#include "DweetSerialApp.h"

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

char* DweetSerialApp::m_dweetPrefix = "$PDWT";

DweetSerialApp::DweetSerialApp()
{
}

int
DweetSerialApp::Initialize(DweetSerialAppConfiguration* config)
{
    ResetWatchdog();

    //
    // Initialize Dweet with the NMEA0183 handler over hardware Serial 0
    //
    m_dweetSerialChannel.Initialize(&Serial, m_dweetPrefix);

    ResetWatchdog();

    //
    // We check the memory monitor at the end of initialization
    // in case any initialization routines overflowed the stack or heap.
    //
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDweetSerialApp + __LINE__);

    return 0;
}
