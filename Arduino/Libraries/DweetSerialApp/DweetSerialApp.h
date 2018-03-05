
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

//
// This class handles setting up the Dweet channel registration
// for a given application instance.
//

struct DweetSerialAppConfiguration {
    // PlaceHolder
};

class DweetSerialApp : public MenloObject  {

public:

    DweetSerialApp();

    int Initialize(DweetSerialAppConfiguration* config = NULL);

    // Default is the Dweet protocol over NMEA 0183
    static char* m_dweetPrefix;
    
    // Return Dweet channel
    MenloDweet* GetDweet() {
        return &m_dweetSerialChannel;
    }

protected:

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

};

#endif // DweetSerialApp_h
