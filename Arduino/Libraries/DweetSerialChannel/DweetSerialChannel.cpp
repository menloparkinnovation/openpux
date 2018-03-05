
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
 *  File: DweetSerialChannel.cpp
 *
 * Handle a Dweet channel instance
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

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>
#include <DweetChannel.h>

//
// This must be "", not <>
// if the file is local to the .ino file and not in the libraries.
//
#include "DweetSerialChannel.h"

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
// called by application.
//
int
DweetSerialChannel::Initialize(
    Stream* port,
    char* prefix
    )
{
    int result;

    m_port = port;

    m_inputBufferIndex = 0;
    m_inputBufferMaxIndex = sizeof(m_inputBuffer) - 1;

    result = m_nmea.Initialize(
        prefix,
        (char*)m_outputBuffer,
        sizeof(m_outputBuffer)
        );

    // Initialize base class.
    MenloDweet::Initialize(
        &m_nmea,
        m_port
    );

    //
    // Register for PollEvent to pull data from the serial channel.
    //
    m_pollEvent.object = this;
    m_pollEvent.method = (MenloEventMethod)&DweetSerialChannel::PollEvent;

    MenloDispatchObject::RegisterPollEvent(&m_pollEvent);

    return result;
}

unsigned long
DweetSerialChannel::PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    unsigned long waitTime = MAX_POLL_TIME;

    xDBG_PRINT("DweetSerialChannel PollEvent");

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

    ProcessSerialInput();

    return waitTime;
}

//
// Handle serial input
//
// This receives data until a whole line is received delimited
// by '\n'.
//
void
DweetSerialChannel::ProcessSerialInput()
{
    char c;

    while(m_port->available()) {

        c = m_port->read();
        m_inputBuffer[m_inputBufferIndex++] = c;

        //
        // receiving '\n' terminates a line
        //
        if (c == '\n') {

	    //
	    // If there is no character in the buffer this
            // is a single '\n' message to perform a channel sync
            //
  	    if (m_inputBufferIndex == 1) {
                xDBG_PRINT("Dweet got sync packet");
                m_inputBufferIndex = 0;
                return;
            }

            m_inputBuffer[m_inputBufferIndex] = '\0';

            xDBG_PRINT("Dweet DispatchMessage");

            // MenloDweet.cpp
            DispatchMessage((char*)&m_inputBuffer[0], m_inputBufferIndex);

            ResetWatchdog();

            xDBG_PRINT("Dweet Return DispatchMessage");

            // Reset the buffer
            m_inputBufferIndex = 0;
            return;
        }

        //
        // Getting to the end of the buffer without a '\n' means
        // an overflow or lost data. Drop all data and let the next
        // received '\n' re-sync the buffer.
        //
        // Buffer re-sync is tricky since we don't know if we have
        // an almost complete message. But the following rules apply
        // here:
        //
        //  All '\n''s terminate the current buffer and start at index 0
	//    - So there are no '\n''s seen since the last line termination.
        //
        //  Lines can not exceed the buffer length. If they do its either
        //
        //  1) An incorrect line, drop it.
        //
        //  2) A '\n' was dropped and we have two (or more) messages now
        //     scrambled inside the buffer with no markers.
        //
        // So re-sync is basically to drop all future input data until
        // a '\n' is seen again resyncing on a good message boundary.
        //
	// By just reseting the buffer here, re-sync is accomplished by
        // the above logic by:
        //
        // 1) Filling the buffer till the next '\n'
        //
        // 2) Attempting to deliver it to DispatchMessage() above
        //    which will validate the checksum, which is incorrect.
	//    
	//    The buffer is now positioned for the next new valid message.
        //
        if (m_inputBufferIndex >= m_inputBufferMaxIndex) {

	    xDBG_PRINT("DweetSerialChannel input buffer overflow resyncing");

            // Reset the buffer
            m_inputBufferIndex = 0;
            return;
        }
    }

    return;
}
