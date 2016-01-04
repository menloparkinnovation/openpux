
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
 *  Date: 01/24/2015
 *  File: DweetSerialChannel.h
 *
 * Handle a Dweet channel instance
 *
 * Used for Smartpux DWEET's.
 */

#ifndef DweetSerialChannel_h
#define DweetSerialChannel_h

#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>
#include <DweetChannel.h>

//
// Each MenloDweet instance has its own independent receive
// and transmit buffers as well as internal processing states
// in both MenloDweet common support and NMEA0183 transport. As
// such each MenloDweet instance is bound to a transport channel
// such as serial, radioserial, HTTP, TCP, COAP, etc.
//
// This allows each instance to act independently when receiving
// and processing MenloDweet requests as they may arrive as
// fragments, etc.
//

//
// This class DweetSerialChannel handles Arduino style Serial
// interfaces, and represents the state of a unique channel instance
// in which Dweet requests and responses flow.
//
// An application may have multiple independent Dweet channels
// each with its own buffering and intermediate processing
// and command dispatch states.
//
// How an application resolves conflicts from Dweet requests
// from multiple channels is application defined.
//
// The model is similar to an RPC server which may receive
// requests from multiple indepenent transports and clients
// at the same time.
//
// Most applications will point the multiple Dweet channels
// at a single application class instance which responds to
// all commands regardless of transport.
//
// The application class may qualify actions based on which
// transport by customizing the command dispatch handlers,
// or tracking which requests come from which channels when
// using the Dweet event handler.
//
// The Lighthouse.ino application provides an example in
// which two Dweet channels (serial + radioserial) are
// supported against a single application instance class.
//

class DweetSerialChannel : public DweetChannel {

public:

    DweetSerialChannel() {
    }

   int
   Initialize(
       Stream* port,
       char* prefix
       );

private:

    void ProcessSerialInput();

    Stream* m_port;

    //
    // input buffer is used as data arrives until the NMEA 0183 end of
    // delimiters are received.
    //
    int m_inputBufferIndex;
    int m_inputBufferMaxIndex;

    char m_inputBuffer[84];

    //
    // output buffer is used by NMEA0183 to format sentences before send
    // This is passed to DweetChannel::m_nmea.Initialize()
    //
    char m_outputBuffer[84];

    //
    // The DweetSerialChannel registers for PollEvents in order
    // to pull data from the serial port in a timely manner.
    //
    MenloEventRegistration m_pollEvent;

    // PollEvent function
    unsigned long PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // DweetSerialChannel_h
