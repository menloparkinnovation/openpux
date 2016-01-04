
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
 *  File: MenloFramework.h
 *
 *  MenloFramework initialization.
 */

#ifndef MenloFramework_h
#define MenloFramework_h

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

//
// Configure the MenloFramework core platform.
//
struct MenloFrameworkConfiguration {

    // Serial0 configuration
    int baudRate;

    // Synchronous Debug is useful for tricky hangs, etc.
    bool synchronousDebug;

    // Watchdog will reset application after 8 seconds of no response
    bool enableWatchdog;

    // Panic pin to flash code on LED
    uint8_t panicPin;

    // Memory validation, tracing, and profiling
    int heapSize;
    int stackSize;
    int guardRegionSize;
    bool enableUsageProfiling;
};

class MenloFramework  {

public:

    static void setup(MenloFrameworkConfiguration* config);

    //
    // loop uses power efficient sleep if enabled.
    //
    // This is called from the main processing loop of the
    // application to drive event dispatching.
    //
    static unsigned long loop(unsigned long maxSleepTime);

protected:

private:

    // Don't allow it to be created
    MenloFramework();

    static char* m_debugPrefix;

};

#endif // MenloFramework_h
