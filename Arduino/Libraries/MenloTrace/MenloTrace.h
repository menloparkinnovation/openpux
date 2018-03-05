
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

#ifndef MenloTrace_h
#define MenloTrace_h

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
#include <MenloTimer.h>
#include <MenloPower.h>

//
// MenloDweet Support
//
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetApp.h>
#include <DweetSerialChannel.h>

//
// Configuration store entries store power on values
// in the EEPROM. They are automatically read and
// applied to the application objects properties
// at power on/reset.
//
// The values are checked summed to prevent incorrect
// device operation on a missing or damaged configuration.
//
// Note: MENLOTRACE is stored in MenloConfigStore.h
//


// Capture command can optionally have up to an 8 byte parameter
#define MENLOTRACE_CAPTURE_SIZE 8

//
// DweetApp allows standard signatures for event callbacks
// and ProcessAppCommands.
//
class MenloTrace : public DweetApp  {

public:

    //
    // This is a static worker routine available from any caller.
    //
    static int CaptureTraceBuffer(char* formatBuffer, int formatBufferSize);

    MenloTrace();

    //
    // Tracing providers allow early initialization of the tracing
    // infrastructure.
    //
    void InitializeTracing(uint8_t* buffer, int bufferSize);

    int Initialize();

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // GET/SET property commands
    //

    int SetMask(char* buf, int size, bool isSet);
    int Capture(char* buf, int size, bool isSet);

protected:

private:

    //
    // Tracing support
    //
    bool m_tracingInitialized;

    int m_traceBufferSize;

    uint8_t* m_traceBuffer;

    // DweetEvent function
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloTrace_h
