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

//
// MenloEvent.cpp
//
// 04/02/2014
//  05/03/2015 - separated out from MenloDispatchObject.cpp
//

#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloDispatchObject.h>
#include <MenloPower.h>

//
// MenloEvent support
//

MenloEventRegistration*
MenloEventRegistration::GetNext()
{
  return m_link;
}

//
// Events have the same contract as Poll(), they return the time
// they require till the next Poll() to process any state.
//
// Events are not allowed to raise their own events to their
// listeners. They must record any internal state that requires
// the event to be raised, and return a value 0 zero for
// which will have the Poll() scheduler re-run the chain in order
// to allow the object's Poll() to be re-invoked so that it may
// deliver the event to its listeners.
//
// This is important to minimize stack space consumed, which
// can be tricky to debug in situations of multiple events
// being delivered simultaneously which may not occur during
// testing.
//

MenloEvent::MenloEvent()
{
  m_eventList = NULL;
}

void
MenloEvent::Register(MenloEventRegistration* registration)
{
#if MENLOEVENT_DEBUG
  if (registration->m_isLinked) {
      MenloDebug::Panic(DispatchEventAlreadyLinked);
  }
  else {
      registration->m_isLinked = true;
  }
#endif
  registration->m_link = m_eventList;
  m_eventList = registration;
}

void
MenloEvent::Unregister(MenloEventRegistration* registration)
{
  MenloEventRegistration* tmp;
  MenloEventRegistration* prev;

#if MENLOEVENT_DEBUG
  if (!registration->m_isLinked) {
      MenloDebug::Panic(DispatchEventNotLinked);
  }
  else {
      registration->m_isLinked = false;
  }
#endif

  prev = NULL;

  tmp = m_eventList;

  while(tmp != NULL) {

    if (tmp == registration) {

      if (prev == NULL) {
	m_eventList = tmp->m_link;
        return;
      }
      else {
        prev->m_link = tmp->m_link;
        return;
      }
    }

    prev = tmp;
    tmp = tmp->m_link;
  }
}

MenloEventRegistration*
MenloEvent::GetHead()
{
  MenloEventRegistration* tmp;

  tmp = m_eventList;

  return tmp;
}

//
// Dispatch single event
//
unsigned long
MenloEvent::DispatchSingleEvent(
    MenloDispatchObject* sender,
    MenloEventArgs* eventArgs,
    MenloEventRegistration* event
    )
{
  unsigned long pollInterval;

  pollInterval = ((event->object)->*(event->method))(sender, eventArgs);

  // Check memory
  MenloMemoryMonitor::CheckMemory(LineNumberBaseEvent + __LINE__);

  return pollInterval;
}

//
// DispatchEvents
//
unsigned long
MenloEvent::DispatchEvents(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    MenloEventRegistration* tmp;
    MenloEventRegistration* next;
    unsigned long newPollTime;
    unsigned long pollInterval = MAX_POLL_TIME;

    //
    // The event dispatcher returns the shortest of all the
    // poll intervals returned.
    //

    //
    // Object event handlers are not allowed to call Dispatch
    // events themselves to raise any of their own local events.
    //
    // They must arrange their local state to do so at the next
    // Poll(), and return 0 for newPollTime from the their event
    // handler to tell the scheduler to do another schedule
    // round.
    //

    tmp = m_eventList;

    while(tmp != NULL) {

        //
        // Note: The handler can unregister the event, so be
        // prepared for that. So we do any updates can caching
        // of next pointers to ensure no corruption if the
        // event is unregistered, and re-registered for a different
        // position on the list.
        //
        next = tmp->m_link;

        newPollTime = ((tmp->object)->*(tmp->method))(sender, eventArgs);

        // Check memory, esp. stack for overflows
        MenloMemoryMonitor::CheckMemory(LineNumberBaseEvent + __LINE__);

        if (newPollTime < pollInterval) {
            pollInterval = newPollTime;
        }

        // next was fetched before callback in case of re-registration of entry
        tmp = next;
    }

    return pollInterval;
}
