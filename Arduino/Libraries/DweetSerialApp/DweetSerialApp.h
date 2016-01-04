
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
 *  File: DweetSerialApp.h
 *
 * Template for MenloDweet Application on a serial connection.
 *
 */

#ifndef DweetSerialApp_h
#define DweetSerialApp_h

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

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>
#include <DweetApp.h>

#include <DweetSerialChannel.h>

// Dispatch + Timers
#include <MenloDispatchObject.h>
#include <MenloTimer.h>

struct DweetSerialAppConfiguration {

    //
    // The application object.
    //
    // A single application object is invoked from potentially
    // multiple Dweet channels/transports.
    //
    DweetApp* dweetApp;
};

class DweetSerialApp : public MenloObject  {

public:

    DweetSerialApp();

    int Initialize(DweetSerialAppConfiguration* config);

    // Default is the Dweet protocol over NMEA 0183
    static char* m_dweetPrefix;

    // Return gDweet channel
    MenloDweet* GetDweet() {
        return &m_dweetSerialChannel;
    }

    //
    // DweetEvent function
    //
    // This allows subclasses to use a common Dweet event delivery function.
    //
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

protected:

    //
    // The application object we dispatch events on
    //
    DweetApp* m_dweetApp;

    //
    // Dweet can support multiple channels active at one time
    // to allow an application to be reachable by HTTP, Serial port,
    // RadioSerial, COAP, etc.
    //

    //
    // Dweet channel support over serial.
    //
    DweetSerialChannel m_dweetSerialChannel;

private:

    //
    // Event registration for transports.
    //
    // For each transport an unhandled Dweet event handler
    // is registered to receive application commands from
    // the transport.
    //
    // A common event handler function is used since each
    // events arguments include the Dweet channel it came
    // in on.
    //

    // Serial Event registration
    MenloDweetEventRegistration m_serialDweetEvent;
};

#endif // DweetSerialApp_h
