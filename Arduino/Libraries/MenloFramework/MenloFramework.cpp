
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
 *  Date: 04/27/2015
 *  File: MenloFramework.cpp
 *
 *  MenloFramework initialization.
 */

#include "MenloFramework.h"

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

char* MenloFramework::m_debugPrefix = "$PDBG,PRINT=";

MenloFramework::MenloFramework()
{
}

void
MenloFramework::setup(MenloFrameworkConfiguration* config)
{
    if (config->enableWatchdog) {
        Power.SetWatchdog(true);
    }

    ResetWatchdog();

    //
    // We setup our serial port manually
    //
    // Serial is a global
    //
    Serial.begin(config->baudRate);

    // Get the serial port setup for debugging first.
    MenloDebug::Init(&Serial);

    //
    // Set synchronous mode for debugging hard problems
    // since a reset/reboot will lose the last messages.
    //
    if (config->synchronousDebug) {
        MenloDebug::SetSynchronous(0x01);
    }

    //
    // Set the debug prefix as we are using NMEA0183 serial
    // control commands over the same serial channel.
    //
    // This allows debug output to be in NMEA0183 message
    // format. (Except for possible length and check sum issues)
    //
    // MenloDebug uses a streaming mode version of NMEA 0183
    // without any additional buffering. Be careful when
    // using Debug::Print's while in the middle of constructing
    // and sending a Dweet/NMEA 0183 message to avoid mixing
    // the output and confusing the host.
    //
    MenloDebug::SetPrefix(m_debugPrefix);

    //
    // Set our panic pin number to the default Arduino
    // LED on the SCK pin. See notes above for SPI interaction.
    //
    // Note: This only configures the port number, it does
    // not initialize it.
    //
    MenloDebug::ConfigurePanicPin(config->panicPin);

    MenloMemoryMonitor::Init(
        config->heapSize, // maximum dynamic heap size
        config->stackSize, // maximum dynamic stack size
        config->guardRegionSize,  // buffer space to aid in detection of deep overlows
        config->enableUsageProfiling // enable detailed memory usage profiling
        );

    //
    // Test current memory environment.
    //
    // Note: This is handy to do around suspect routines
    // which may use to much memory. Good candiates are any
    // kind of complex external library such as WiFi or
    // anything that uses interrupts.
    //
    MenloMemoryMonitor::CheckMemory(LineNumberBaseFramework + __LINE__);

#if xDBG_PRINT_ENABLED
    //
    // Report current detailed memory usage
    //
    // This consumes 56 bytes on Atmega 328
    //
    // Must also turn on MEMORY_MONITOR_FULL
    // in MenloMemoryMonitor.cpp
    //
    MenloMemoryMonitor::ReportMemoryUsage(__LINE__);
#endif

    return;
}

unsigned long
MenloFramework::loop(unsigned long maxSleepTime)
{
    return MenloDispatchObject::loop(maxSleepTime);
}
