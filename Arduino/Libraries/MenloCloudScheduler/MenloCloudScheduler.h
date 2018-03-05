
#ifndef MenloCloudScheduler_h
#define MenloCloudScheduler_h

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

#include "MenloPlatform.h"
#include "MenloTimer.h"

// 30 seconds
#define DEFAULT_CLOUD_INTERVAL (1000L * 30L)

// 10 seconds
#define MINIMUM_CLOUD_INTERVAL (1000L * 10L)

//
// MenloCloudScheduler raises an Event when the cloud schedule occurs
//
class MenloCloudSchedulerEventArgs : public MenloEventArgs {
 public:

    // Currently configured period
    unsigned long period;
};

// Introduce the proper type name. Could be used for additional parameters.
class MenloCloudSchedulerEventRegistration : public MenloEventRegistration {
 public:
};

//
// Model:
//
// This class is intended to act as a super class for cloud
// providers.
//
// The cloud scheduler is initialized with an initial value
// and the application can control its activity with
// EnableTimer()/DisableTimer().
//
// It can change the interval at anytime without worrying about
// the current status of the timer. The timers enable/disable
// status will remain after the interval change.
//
// The timer may also be enabled/disabled without worrying about
// losing the current interval setting as well.
//
// This allows the cloud update rate to be controlled, paused,
// etc. without having to maintain to many application state variables.
//

class MenloCloudScheduler : public MenloDispatchObject {

 public:

    MenloCloudScheduler();

    //
    // Initialize sets the initial cloud period which can
    // be changed with setCloudPeriod().
    //
    // Timer must be enabled with EnableTimer().
    //
    int Initialize(unsigned long period);

    void EnableTimer();

    void DisableTimer();

    unsigned long IsTimerEnabled() {
        return m_enabled;
    }

    unsigned long getCloudPeriod() {
        return m_timerInterval;
    }

    //
    // Setting the period does not change the status
    // of the timer (enabled/disabled) unless its
    // set to 0 which always disables the timer, whether
    // running or not.
    //
    void setCloudPeriod(unsigned long period);

    //
    // Register event invoked when the cloud interval fires
    //
    void RegisterProcessEvent(MenloCloudSchedulerEventRegistration* callback);

    // Force Process() to run without waiting till the next interval.
    void SendNow();

    // Process function for subclass.
    virtual unsigned long Process();

    //
    // True if there is the potential to connect to the internet.
    //
    virtual bool IsConnected() = 0;

 private:

    // MenloCloudScheduler emits an event when the cloud schedule fires
    MenloEvent m_processList;

    //
    // Timers
    //
    bool m_enabled;

    //
    // This is in milliseconds.
    //
    unsigned long m_timerInterval;

    //
    // Timer and event registration
    //
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloCloudScheduler_h



