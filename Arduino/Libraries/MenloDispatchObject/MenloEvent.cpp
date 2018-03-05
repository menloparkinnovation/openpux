
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
// Header is in MenloDispatchObject.h
//
// 04/02/2014
//  05/03/2015 - separated out from MenloDispatchObject.cpp
//

#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloDispatchObject.h>
#include <MenloPower.h>

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
// MenloEvent support
//

//
// MenloDispatchObject.h contains the MenloEvent, MenloEventRegistration classes
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

    //
    // Debugging:
    //
    // The xDBG_PRINT debugging code prints out the core event dispatch
    // of a given list. It shows the address of the list head variable,
    // and shows each MenloEventRegistration pointer and its method.
    //

    // DispatchEvents Start for the given list
    xDBG_PRINT_NNL("DE S ");
    xDBG_PRINT_INT((int)&m_eventList);
    
    tmp = m_eventList;

    while(tmp != NULL) {

        xDBG_PRINT_NNL("p ");
        xDBG_PRINT_INT((int)tmp);

        xDBG_PRINT_NNL("m ");

        // Must use a trick to get the address of the C++ method pointer block
        int* p = (int*)(&tmp->method);
        int x = (int)*p;
        xDBG_PRINT_INT(x);

        //
        // Note: The handler can unregister the event, so be
        // prepared for that. So we do any updates caching
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

    // DispatchEvents End
    xDBG_PRINT_NNL("DE E ");
    xDBG_PRINT_INT((int)&m_eventList);

    return pollInterval;
}
