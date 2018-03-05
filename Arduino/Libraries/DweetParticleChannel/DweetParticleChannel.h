
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
 *  Date: 03/05/2016
 *  File: DweetParticleChannel.h
 *
 *  Dweet channel on the Particle Cloud.
 *
 * Used for Smartpux DWEET's.
 */

#ifndef DweetParticleChannel_h
#define DweetParticleChannel_h

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
// such as serial, radioserial, HTTP, TCP, COAP, SMS, etc.
//
// This allows each instance to act independently when receiving
// and processing MenloDweet requests as they may arrive as
// fragments, etc.
//

//
// This class DweetParticleChannel handles messages
// sent over the Particle Cloud and represents the state of
// a unique channel instance in which Dweet requests and responses flow.
//
// An application may have multiple independent Dweet channels
// each with its own buffering and intermediate processing
// and command dispatch states.
//
// How an application resolves conflicts from Dweet requests
// from multiple channels is application defined.
//
// The model is similar to an RPC server which may receive
// requests from multiple independent transports and clients
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

//
// Tracing support
//
// Particle controllers have more memory than small Arduino style
// controllers and can afford to have memory for tracing.
//
// The particle cloud is tailor made for retrieving information such
// as logs in the field.
//
// https://docs.particle.io/reference/firmware/photon/#particle-variable-
//
// Note: Particle.variable() supports up to a 622 byte message, so a maximum
// buffer is allocated given the 32k of available memory.
//

const int ParticleFormatBufferSize = 622;

class DweetParticleChannel : public DweetChannel {

public:

    DweetParticleChannel() {
    }

    int
    Initialize(
        char* prefix
        );

    //
    // This virtual allows the caller to override the port write
    // function for transports that don't model an Arduino
    // Stream* (serial port) style port.
    //
    // In this case m_port is set to NULL at the initialize call
    // to MenloDweet.
    //
    virtual size_t WritePort(const uint8_t *buffer, size_t size);

    int DweetReceived(String arg);

    int TraceCommandReceived(String arg);

    int CaptureTraceBuffer(String arg);

    static DweetParticleChannel* s_self;

private:

    void InitializeTracing();

    char* m_prefix;

    //
    // input buffer is used as data arrives until the NMEA 0183 end of
    // delimiters are received.
    //
    int m_inputBufferIndex;
    int m_inputBufferMaxIndex;

    //
    // Maximum size is:
    //
    // $PDWT -    5 chars
    // message - 63 chars
    // *00 -      3 chars
    // '\0'-      1 char
    // ---------------------
    //            72 chars
    //

    char m_inputBuffer[84];

    //
    // output buffer is used by NMEA0183 to format sentences before send
    // This is passed to DweetChannel::m_nmea.Initialize()
    //
    char m_outputBuffer[84];

    //
    // The DweetParticleChannel registers for PollEvents in order
    // to pull data from the serial port in a timely manner.
    //
    MenloEventRegistration m_pollEvent;

    // PollEvent function
    unsigned long PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Format buffer is what is published to Particle.variable()
    //
    uint8_t m_traceFormatBuffer[ParticleFormatBufferSize];
};

#endif // DweetParticleChannel_h
