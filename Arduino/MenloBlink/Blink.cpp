
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
 * Date: 04/30/2015
 * File: Blink.cpp
 *
 * Application class.
 *
 */

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//
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
#include <MenloDispatchObject.h>
#include <MenloTimer.h>
#include <MenloNMEA0183.h>
#include <MenloDweet.h>

#include "Blink.h"

#define DBG_PRINT_ENABLED 1

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

int
Blink::Initialize(BlinkConfiguration* config)
{
    m_config = *config;

    // Setup a TimerEvent for the requested period
    m_timerEvent.object = this;
    m_timerEvent.method = (MenloEventMethod)&Blink::TimerEvent;

    m_timerEvent.m_interval = m_config.interval;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    if (m_config.pinNumber != (-1)) {
        pinMode(m_config.pinNumber, OUTPUT);
    }

    //
    // Register for PollEvent to pull data from the serial channel.
    //
    m_pollEvent.object = this;
    m_pollEvent.method = (MenloEventMethod)&Blink::PollEvent;

    MenloDispatchObject::RegisterPollEvent(&m_pollEvent);

    //
    // Start the timer running
    //
    m_timer.RegisterIntervalTimer(&m_timerEvent);

    return 0;
}

unsigned long
Blink::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{

    DBG_PRINT("Blink TimerEvent");

    if (m_lightToggle) {
        m_lightToggle = false;
    }
    else {
        m_lightToggle = true;
    }

    digitalWrite(m_config.pinNumber, m_lightToggle);

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

//
// This is here as a demonstration.
//
// Blink does not need the PollEvent since it uses MenloTimer to generate
// its timing sequences automatically.
//
// The PollEvent allows the application, or any class to participate
// in the low level processing loop of the system/application.
//
unsigned long
Blink::PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    unsigned long waitTime = MAX_POLL_TIME;

    //DBG_PRINT("Blink PollEvent");

    //
    // sender is NULL on PollEvent's since the sender is the base
    // static MenloDispatchObject's poll/event dispatcher.
    //

    //
    // An application returns a time value in milliseconds in which
    // it expects to be polled again. This represents an estimated
    // maximum time since the PollEvent may occur at much shorter
    // durations if there is activity present in the application or
    // system.
    //
    // If there is no other activity present in the application or
    // system the MenloPower module will attempt to place the processor
    // into a power saving sleep or idle mode. Maximizing the time
    // returned from this event handler allows energy to be saved
    // for battery powered applications by allowing for longer sleeps.
    //
    // An application is free to make the trade off between
    // energy saving and responsiveness that makes the most sense
    // for the given application. It's also free to change the
    // wait time returned based on application states, such as
    // when in the middle of an operation or response.
    //
    // The MenloRadio handler exhibits this behavior when there
    // has been recent radio activity, and after a period of
    // time goes back to its scheduled listen interval.
    //

    return waitTime;
}


