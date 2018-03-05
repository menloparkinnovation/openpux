
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
 *  Date: 02/28/2016
 *  File: MenloCloudScheduler
 */

//
// MenloFramework
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloDispatchObject.h>
#include "MenloTimer.h"

#include "MenloCloudScheduler.h"

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
// This class acts as kind of an adapter class that allows an
// asynchronous timer period to be configured, started, stopped,
// changed, etc.
//
// It fires MenloEvent's which allows listeners to register without
// being a subclass.
//
// Subclasses may override the main event indication function
// to provide customized contracts.
//

MenloCloudScheduler::MenloCloudScheduler()
{
    m_enabled = false;
    m_timerInterval = DEFAULT_CLOUD_INTERVAL;
}

int
MenloCloudScheduler::Initialize(unsigned long period)
{
    // invoke base to initialize MenloDispatchObject
    MenloDispatchObject::Initialize();

    m_timer.Initialize();

    m_timerEvent.object = this;

    m_timerEvent.method = (MenloEventMethod)&MenloCloudScheduler::TimerEvent;

    //
    // MenloTimer setup
    //
    setCloudPeriod(period);

    return 1;
}

//
// TimeEvent
//
unsigned long
MenloCloudScheduler::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    xDBG_PRINT("MenloCloudScheduler::TimerEvent");

    return Process();
}

//
// Allows an application to request an out of schedule send.
//
void
MenloCloudScheduler::SendNow()
{
    Process();
}

void
MenloCloudScheduler::RegisterProcessEvent(MenloCloudSchedulerEventRegistration* callback)
{
    // Add to event list
    m_processList.Register(callback);
    return;
}

//
// Process a cloud scheduled event
//
// Note: This is a virtual which allows a sub-class to override
// processing, but most classes register an event which does not
// require being a subclass and more composable.
//
unsigned long
MenloCloudScheduler::Process()
{
    MenloCloudSchedulerEventArgs eventArgs;
    unsigned long pollInterval = MAX_POLL_TIME;

    xDBG_PRINT("MenloCloudScheduler::Process() Enter");

    // We indicate the current period when the event is fired
    eventArgs.period = m_timerInterval;

    // Send event to listeners
    pollInterval = m_processList.DispatchEvents(this, &eventArgs);

    xDBG_PRINT("MenloCloudScheduler::Process() Leave");

    return pollInterval;
}

void
MenloCloudScheduler::EnableTimer()
{
    // A period of 0 disables the timer
    if (m_timerInterval == 0) {

        xDBG_PRINT("MenloCloudScheduler EnableTimer timer is 0");

        if (m_enabled) {
            m_timer.UnregisterIntervalTimer(&m_timerEvent);
            m_enabled = false;
        }

        return;
    }

    if (m_enabled) {
        xDBG_PRINT("MenloCloudScheduler EnableTimer timer is already enabled");
        return;
    }

    m_enabled = true;

    xDBG_PRINT_NNL("MenloCloudScheduler EnableTimer enabling timer period ");
    xDBG_PRINT_INT_NNL((int)(m_timerInterval >> 16L));
    xDBG_PRINT_INT((int)m_timerInterval);

    // Object type specific args
    m_timerEvent.m_interval = m_timerInterval;

    // Register timer
    m_timer.RegisterIntervalTimer(&m_timerEvent);
}

void
MenloCloudScheduler::DisableTimer()
{
    if (m_enabled) {

        // Unregister our current timer
        m_timer.UnregisterIntervalTimer(&m_timerEvent);
        m_enabled = false;
    }
}

//
// Changes the cloud period, but not the enable/disabled
// state of the timer though it must temporarily stop the
// timer for the change.
//
// Note: if the period is set to 0 the timer is disabled
// and remains so until a non-zero period is established
// and EnableTimer() is called.
//
void
MenloCloudScheduler::setCloudPeriod(unsigned long period)
{
    bool running = m_enabled;

    DisableTimer();

    //
    // A negative number confuses the Arduino libraries to
    // constantly indicate as if the time is 0.
    //
    if ((long)period <= 0) {
        // This disables the timer
        MenloDebug::Print(F("MenloCloudScheduler negative or 0 period disabling timer"));
        m_timerInterval = 0;
    }
    else if (period < MINIMUM_CLOUD_INTERVAL) {
        MenloDebug::PrintNoNewline(F("MenloCloudScheduler invalid interval time "));
        MenloDebug::PrintHexNoNewline((int)(period >> 16L));
        MenloDebug::PrintHex((int)period);
        m_timerInterval = MINIMUM_CLOUD_INTERVAL;
    }
    else {
        // set interval
        m_timerInterval = period;
    }

    if (running) {
        EnableTimer();
    }
}
