
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
 *  File: DweetParticleChannel.cpp
 *
 *  Dweet channel on the Particle Cloud.
 *
 * Used for Smartpux DWEET's.
 */

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloTrace.h>

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Particle library support
#include "application.h" 

// Dweet Support
#include <MenloDweet.h>
#include <DweetChannel.h>

//
// This must be "", not <>
// if the file is local to the .ino file and not in the libraries.
//
#include "DweetParticleChannel.h"

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
// Usage Model:
//
// Menlo Dweets are transported over the Particle cloud function,
// variable, and event support.
//
// The published function name "dweet" accepts either fully
// formatted Dweets, or a shorter form due to the 63 maximum
// characters in a particle function call argument. The optionally
// dropped prefix of "$PDWT," and suffix "*00" can be avoided
// since we know the topic of the conversation (function is dweet)
// and its a reliable transport in regards to data checksums not
// being required.
//
// The Dweet reply buffer is published as a particle variable
// that can be read remotely at any time using the Particle Cloud's
// REST commands. It will contain the last reply to a Dweet on
// the particle channel, or any locally generated Dweets intended
// for the particle channel.
//
// Note: It's better to use Particle events for locally generated
// asynchronous dweets. That can be added later as application
// scenarios require it.
//

DweetParticleChannel* DweetParticleChannel::s_self = NULL;

//
// Handle debug command from the Particle Cloud.
//
// Note: Since every MenloFramework application on the Particle
// devices is expected to use Dweet this module also provides
// Particle Cloud accessed remote debug functions as well.
//
int
DweetParticleChannel_DebugReceived(String arg)
{
    return DweetParticleChannel::s_self->TraceCommandReceived(arg);
}

int
DweetParticleChannel::TraceCommandReceived(String arg)
{
    if (arg == "trace_capture") {

        //
        // Capture the trace buffer into the buffer published to
        // Particle.variable().
        //
        // This resets the trace buffer.
        //

        return CaptureTraceBuffer(arg);
    }
    else {

        //
        // All other tracing commands such as setting the mask
        // and performing specific captures are done by Dweet's
        // on the MenloTrace module.
        //

        return -1;
    }
}

//
// Capture the trace buffer by formatting current tracing data
// into an ASCII hex format.
//
int
DweetParticleChannel::CaptureTraceBuffer(String arg)
{
    if (ParticleFormatBufferSize == 0) {
        // No trace format buffer is available.
        return -1;
    }

    // MenloTrace::CaptureTraceBuffer will zero the supplied format buffer
    MenloTrace::CaptureTraceBuffer((char*)&m_traceFormatBuffer, ParticleFormatBufferSize);

    // The m_traceFormatBuffer is published as a Particle.variable("tracebuffer")
    // during tracing initialize.

    return 1;
}

//
//
// Invoked when a dweet is received from the particle cloud
//
// The arg is the data value supplied by the REST caller
// and is up to 63 bytes.
//
// A Dweet is a NMEA 0183 formatted command.
//
// Note: Since NMEA 0183 maximum length is 82 characters
// and we are limited to 63 by Particle functions, the
// prefix, checksum, are optional given the particle infrastructure
// has assured complete message delivery.
//
// Example:
//
// $PDWT,SETCONFIG=UPDATERATE:30*00 -> SETCONFIG=UPDATERATE:30
//
// This was setup with:
//
//  Particle.function("dweet", dweetReceived);
//
//
// Any Dweet reply's generated in the m_outputBuffer are published
// as a Particle string variable named "dweetreply".
//
//  Particle.variable("dweetreply", m_outputBuffer, STRING);
//
int
DweetParticleChannel_DweetReceived(String arg)
{
    return DweetParticleChannel::s_self->DweetReceived(arg);
}

//
// The Particle channel provides support for retrieval of a
// trace format buffer and is published as a Particle Cloud variable.
//
void
DweetParticleChannel::InitializeTracing()
{
    s_self = this;

    if (ParticleFormatBufferSize != 0) {

        bzero(m_traceFormatBuffer, ParticleFormatBufferSize);

        //
        // Set the trace format buffer to the MenloTrace subsystem
        //
        MenloDebug::SetFormatBuffer((uint8_t*)&m_traceFormatBuffer, ParticleFormatBufferSize);

        //
        // Publish the trace format buffer as a variable for retrieval through
        // the particle cloud.
        //
        Particle.variable("tracebuffer", m_traceFormatBuffer, STRING);
    }

    // Register for Particle cloud "debug" commands
    Particle.function("debug", DweetParticleChannel_DebugReceived);
}

//
// called by application.
//
int
DweetParticleChannel::Initialize(
    char* prefix
    )
{
    int result;

    // this initializes s_self
    InitializeTracing();

    // We save the prefix to compare whether we have a short message or not
    m_prefix = prefix;

    m_inputBufferIndex = 0;
    m_inputBufferMaxIndex = sizeof(m_inputBuffer) - 1;

    m_inputBuffer[0] = '\0';
    m_outputBuffer[0] = '\0';

    result = m_nmea.Initialize(
        prefix,
        m_outputBuffer,
        sizeof(m_outputBuffer)
        );

    // Initialize base class.
    MenloDweet::Initialize(
        &m_nmea,
        NULL
    );

    //
    // Register for PollEvent to pull data from the cloud
    //
    m_pollEvent.object = this;
    m_pollEvent.method = (MenloEventMethod)&DweetParticleChannel::PollEvent;

    MenloDispatchObject::RegisterPollEvent(&m_pollEvent);

    // Register for Particle cloud "dweet" commands
    Particle.function("dweet", DweetParticleChannel_DweetReceived);

    // Publish the output buffer as a variable for Dweet Replies
    Particle.variable("dweetreply", m_outputBuffer, STRING);

    return result;
}

//
// This is an override of MenloDweet for an alternate port write implementation.
//
size_t
DweetParticleChannel::WritePort(const uint8_t *buffer, size_t size)
{
    size_t retVal = 0;

    //
    // Dweet's from the device are published as events.
    //
    // Note: TTL is always 60 right now.
    // Event data can be up to 255 bytes.
    //
    // PRIVATE could be used for private events
    //

    //
    // Note: Supplied buffer is NULL terminated
    //
    bool result = Particle.publish("dweet", (const char*)buffer, 60, PUBLIC);
    if (!result) {

        DBG_PRINT("DweetParticleChannel Particle.publish dweet event failed");

        //
        // failed to publish the event. Could be that the cloud
        // is disconnected.
        //
    }
    else {
        retVal = size;
        DBG_PRINT(("DweetParticleChannel:: Particle.publish dweet event succeeded"));
    }

    return retVal;
}

int
DweetParticleChannel::DweetReceived(String arg)
{
    int retVal;
    int dweetLength = arg.length();
    unsigned char checksum;
    char checksumBuffer[2];

    //
    // For particle cloud we take in whole messages.
    //

    xDBG_PRINT("Dweet DispatchMessage");

    //
    // We protect our input buffer by not allowing messages
    // greater than the currently documented particle cloud
    // argument size.
    //
    if (dweetLength > 63) {
        DBG_PRINT("DweetParticleChannel DispatchMessage to large");
        return -1;
    }

    // Reset the input buffer for a new message
    m_inputBufferIndex = 0;

    //
    // Handle short messages without $PDWT and *00
    //
    // Note: it's better to add it here than complicate
    // the NMEA 0183 library which must remain small for really
    // tiny embedded controllers. The Particle cloud class
    // microcontrollers have plenty of space to handle this here.
    //
    // NMEA short messages are a space saving way to deal with
    // the Particle clouds 63 character data parameter limit.
    //
    int prefixLength = strlen(m_prefix);

    if (!arg.startsWith(m_prefix)) {

        xDBG_PRINT("Dweet: short message on particle channel");

        // Short message, add the prefix and checksum trailer
        memcpy(&m_inputBuffer[m_inputBufferIndex], m_prefix, prefixLength);
        m_inputBufferIndex += prefixLength;

        m_inputBuffer[m_inputBufferIndex++] = ',';

        //
        // Note: This takes the size of the buffer, and automatically ensures the NULL
        // is there.
        //
        arg.getBytes((unsigned char*)&m_inputBuffer[m_inputBufferIndex], dweetLength + 1);

        // args.length does not include a NULL
        m_inputBufferIndex += dweetLength;

        // Add checksum symbol
        m_inputBuffer[m_inputBufferIndex++] = '*';

        m_inputBuffer[m_inputBufferIndex] = '\0';

        m_nmea.checksum(m_inputBuffer, &checksum);

        MenloUtility::UInt8ToHexBuffer(checksum, &checksumBuffer[0]);

        m_inputBuffer[m_inputBufferIndex++] = checksumBuffer[0];
        m_inputBuffer[m_inputBufferIndex++] = checksumBuffer[1];

        m_inputBuffer[m_inputBufferIndex++] = '\r';
        m_inputBuffer[m_inputBufferIndex++] = '\n';

        m_inputBuffer[m_inputBufferIndex] = '\0';
    }
    else {

        xDBG_PRINT("Dweet: full message on particle channel");

        // Full Dweet with prefix such as $PDWT and checksum trailer
        arg.getBytes((unsigned char*)&m_inputBuffer[m_inputBufferIndex], dweetLength);

        // args.length does not include a NULL
        m_inputBufferIndex += dweetLength;

        m_inputBuffer[m_inputBufferIndex] = '\0';
    }

    // MenloDweet.cpp
    retVal = DispatchMessage(&m_inputBuffer[0], m_inputBufferIndex);

    ResetWatchdog();

    xDBG_PRINT("Dweet Return DispatchMessage");

    if (retVal == 0) {
        // Error, no one recognized or processed the Dweet message
        xDBG_PRINT("Message not handled");
        return -1;
    }
    else {
        // Success, Dweet message was processed
        xDBG_PRINT("Message was processed");
        return 1;
    }
}

unsigned long
DweetParticleChannel::PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    unsigned long waitTime = MAX_POLL_TIME;

    //xDBG_PRINT("DweetParticleChannel PollEvent");

    //
    // sender is NULL on PollEvent's since the sender is the base
    // static MenloDispatchObject's poll/event dispatcher.
    //

    //
    // Unlike most MenloEvent's PollEvents are allowed to
    // raise new events from the PollEvent handler. This is because
    // it has the same top of stack status as the virtual method
    // Poll() of MenloDispatchObject.
    //

    // Keeps Particle data pump going
    Particle.process();

    return waitTime;
}
