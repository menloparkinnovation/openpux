
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
 *  File: MenloTimer.cpp
 *
 * MenloTimer
 */

#include <MenloPlatform.h>
#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloTimer.h>

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

MenloTimer::MenloTimer()
{
  // Default timer interval
  m_interval = TIMER_DEFAULT_INTERVAL;
}

int
MenloTimer::Initialize()
{
  // invoke base to initialize MenloDispatchObject
  MenloDispatchObject::Initialize();

  // Default timer interval
  m_interval = TIMER_DEFAULT_INTERVAL;

  return 0;
}

// Overridden from MenloDispatchObject
unsigned long
MenloTimer::DispatchSingleEvent(
    MenloDispatchObject* sender,
    MenloEventArgs* eventArgs,
    MenloEventRegistration* event
    )
{
  xDBG_PRINT("MenloTimer::DispatchSingleEvent");

  // Invoke base event dispatcher
  return m_timerList.DispatchSingleEvent(sender, eventArgs, event);
}

// Overridden from MenloDispatchObject
unsigned long
MenloTimer::Poll()
{
  MenloTimerEventArgs eventArgs;
  MenloTimerEventRegistration* tmp;
  MenloTimerEventRegistration* next;
  unsigned long newPollTime;
  unsigned long difference;
  unsigned long pollInterval = MAX_POLL_TIME;
  bool eventFired = false;

  // read current time
  unsigned long current_time = GET_MILLISECONDS();

  // Get head of timer list
  tmp = (MenloTimerEventRegistration*)m_timerList.GetHead();

  while(tmp != NULL) {

      //
      // IMPROVE: Sort the list by interval/dueTime so we can early out
      // and not always walk the whole list.
      //

      //
      // Note: The handler can unregister the timer, so be
      // prepared for that. So we do any updates can caching
      // of next pointers to ensure no corruption if the
      // timer is unregistered, and registered for a different list entry.
      //
      next = (MenloTimerEventRegistration*)tmp->GetNext();

      //
      // C99 ensures that wrap around during subtraction of two unsigned values
      // is 2**n modulo. Cast to signed to get the difference.
      //
      // http://en.wikipedia.org/wiki/Integer_overflow
      //
      // http://playground.arduino.cc/Code/TimingRollover
      //

      // Compare current time to head of timer list
      if ((long)(current_time - tmp->m_dueTime) >= 0) {

  	  eventFired = false;

          // Provide the event handler the current time
          eventArgs.m_currentTime = current_time;

          //
          // Setup the next dueTime before callback in case
          // of registration. A non-zero value is used to indicate
          // registered/unregistered status and we don't want to
          // overwrite an unlinked (and possibly reassigned, freed)
          // entry.
          //

          // Update to the next internal using the entries interval
          tmp->m_dueTime += tmp->m_interval;

          //
          // Robustness: Over a long runtime there is a non-zero chance
          // that a timer rollover could result in a 0 due time. 0 is
          // used as a check value in RegisterEvent/UnRegisterEvent to
          // look for whether a timer is already linked on the list.
          // If this occurs, we bump 1ms forward to avoid this
          // ambiguity.
          //
          if (tmp->m_dueTime == 0) {
              tmp->m_dueTime = 1;
          }

          // If the timer has expired, execute the events
          newPollTime = DispatchSingleEvent(this, &eventArgs, tmp);

          // Check memory
          MenloMemoryMonitor::CheckMemory(LineNumberBaseTimer + __LINE__);

	  DISPATCH_PRINT2("MenloTimer::Poll DispatchSingleEvent: newPollTime=", newPollTime);

          if (newPollTime < pollInterval) {
              pollInterval = newPollTime;
          }
      }
      else {
	  // Ensure the pollInterval does not exceed the time till entry is due
          difference = tmp->m_dueTime - current_time;
          if (pollInterval > difference) {
	      pollInterval = difference;
          }
      }

      // next was fetched before callback in case of re-registration of entry
      tmp = next;
  }

  DISPATCH_PRINT2("MenloTimer::Poll m_interval=", m_interval);

  if (pollInterval > m_interval) {
    pollInterval = m_interval;
  }

  if (!eventFired) {
      DISPATCH_PRINT("MenloTimer::Poll return: no events fired ");
  }

  DISPATCH_PRINT2("MenloTimer::Poll return: pollInterval=", pollInterval);

  return pollInterval;
}

void
MenloTimer::RegisterIntervalTimer(MenloTimerEventRegistration* callback)
{
  //
  // If the callback has a shorter interval we update
  // the timer interval to supports its requirements.
  //

  // A zero dueTime means not registered on a list
  if (callback->m_dueTime != 0L) {
      MenloDebug::Print(F("MenloTimer attempt to reregister timer"));
      return;
  }

  // Register the shorter interval
  if (callback->m_interval < m_interval) {
    m_interval = callback->m_interval;
  }

  // Note: Can't return 0 or we will be stuck in a loop
  if (m_interval == 0) {
    m_interval = MAX_POLL_TIME;
  }

  callback->m_dueTime = GET_MILLISECONDS() + callback->m_interval;

  // Add to event list
  m_timerList.Register(callback);

  return;
}

void
MenloTimer::UnregisterIntervalTimer(MenloTimerEventRegistration* callback)
{
  MenloTimerEventRegistration* tmp;
  unsigned long shortestInterval = TIMER_DEFAULT_INTERVAL;

  if (callback->m_dueTime == 0) {
      MenloDebug::Print(F("MenloTimer attempt to unregister non registered timer"));
      return;
  }

  // Unregister from event list
  m_timerList.Unregister(callback);

  // A zero dueTime means not registered on a list
  callback->m_dueTime = 0L;

  //
  // The interval timer we just unregistered could
  // have been the shortest interval. It is more
  // power efficient to calculate the new minimum
  // interval which could be longer allowing for more
  // sleep time.
  //
  // The event list could be empty, and this case
  // the default timer interval is set.
  //

  // Get head of timer list
  tmp = (MenloTimerEventRegistration*)m_timerList.GetHead();

  while(tmp != NULL) {

      if (tmp->m_interval < shortestInterval) {
        shortestInterval = tmp->m_interval;
      }

      tmp = (MenloTimerEventRegistration*)tmp->GetNext();
  }

  m_interval = shortestInterval;
}
