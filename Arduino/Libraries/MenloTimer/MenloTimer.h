
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
 *  Date: 02/15/2015
 *  File: MenloTimer.h
 *
 * MenloTimer
 */

#ifndef MenloTimer_h
#define MenloTimer_h

#include "MenloDispatchObject.h"

//
// Set to 60 seconds to allow sleeping where possible.
// If a timer interval is set for shorter, it is updated.
//
#define TIMER_DEFAULT_INTERVAL (60L * 1000L)

// m_currentTime is the time the event is invoked
class MenloTimerEventArgs : public MenloEventArgs {
 public:
  unsigned long m_currentTime;
};

class MenloTimerEventRegistration : public MenloEventRegistration {
 public:
  unsigned long m_dueTime;
  unsigned long m_interval;
};

//
// MenloTimer is a dispatch object tying into the poll()
// loop.
//
class MenloTimer : public MenloDispatchObject {
 public:

  MenloTimer();

  //
  // Allows an application to safely test if a timer
  // has already been queued.
  //
  bool
  IsTimerRegistered(MenloTimerEventRegistration* callback) {
      if (callback->m_dueTime == 0L) {
          return false;
      }
      else {
          return true;
      }
  }

  virtual int Initialize();

  virtual unsigned long Poll();

  //
  // Override from MenloDispatchObject
  //
  // It's a virtual to allow subclasses to override and determine
  // delivery. The default is the MenloDispatchObject event dispatcher.
  //
  virtual unsigned long DispatchSingleEvent(
      MenloDispatchObject* sender,
      MenloEventArgs* eventArgs,
      MenloEventRegistration* event
      );

  // Register a timer which is called at interval
  void RegisterIntervalTimer(MenloTimerEventRegistration* callback);

  // Unregister a the specified timer callback
  void UnregisterIntervalTimer(MenloTimerEventRegistration* callback);

 protected:

  unsigned long m_interval;

 private:

  MenloEvent m_timerList;
};

#endif // MenloTimer_h
