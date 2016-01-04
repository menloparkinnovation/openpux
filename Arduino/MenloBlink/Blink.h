
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
 * File: Blink.h
 *
 * Application class.
 *
 */

#ifndef Blink_h
#define Blink_h

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

struct BlinkConfiguration {
    int pinNumber;
    unsigned long interval;
};

//
// MenloObject allows standard signatures for event callbacks.
//
class Blink : public MenloObject  {

public:

    Blink() {
        m_lightToggle = false;
    }

    int Initialize(BlinkConfiguration* config);

private:

    BlinkConfiguration m_config;

    bool m_lightToggle;

    //
    // A timer is used to provide the blink interval
    //

    // Timer and event registration
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // A MenloFramework application can register to receive
    // PollEvent's to allow it to drive an application or
    // hardware state machine similar to the Arduino poll() routine.
    //
    // For classes whose primary purpose is not event generation, or its
    // inconvenient to inherit from MenloDispatchObject the PollEvent
    // allows it to participate in the core system and application
    // processing loop.
    //
    MenloEventRegistration m_pollEvent;

    // PollEvent function
    unsigned long PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // Blink_h
