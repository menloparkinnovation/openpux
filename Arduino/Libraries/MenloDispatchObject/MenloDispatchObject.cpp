
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
// MenloDispatchObject.cpp
// 04/02/2014
//

#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloDispatchObject.h>
#include <MenloPower.h>

//
// Set this when debugging tricky memory corruptions from classes
// invoked by dispatch.
//
#if ARDUINO_AVR_PROMICRO8
#define DISPATCH_MEMORY_CHECKS 1
#endif

MenloDispatchObject*
MenloDispatchObject::s_pollList = (MenloDispatchObject*)NULL;

//
// Declare the PollEvent list class instance.
//
MenloEvent MenloDispatchObject::s_pollEventList;

//
// Loop is a static invoked from the Arduino-style loop().
//
unsigned long
MenloDispatchObject::loop(unsigned long maxSleepTime)
{
    unsigned long waitTime = MAX_POLL_TIME;

#if DISPATCH_MEMORY_CHECKS
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDispatch + __LINE__);
#endif

    waitTime = MenloDispatchObject::LoopInternal();

#if DISPATCH_MEMORY_CHECKS
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDispatch + __LINE__);
#endif

    if (waitTime > maxSleepTime) {
        waitTime = maxSleepTime;
    }

    Power.Sleep(waitTime);

#if DISPATCH_MEMORY_CHECKS
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDispatch + __LINE__);
#endif

    ResetWatchdog();

    return waitTime;
}

unsigned long
MenloDispatchObject::LoopInternal()
{
    MenloDispatchObject* tmp;
    unsigned long newPollTime;
    unsigned long pollInterval;

#if DISPATCH_MEMORY_CHECKS
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDispatch + __LINE__);
#endif

    //
    // This first pass invokes all objects which are subclasses
    // of MenloDispatchObject() and their constructors have
    // added them to the s_pollList;
    //

    //
    // Invoke Poll() for all MenloObjects in the
    // registered s_pollList.
    //
    // If pollInterval == 0, we continue to process
    // until its != 0 in order to process events
    // and state changes in the objects.
    //
    // This is because objects are not allowed to raise
    // events from their own event handlers that may be
    // triggered by Poll(), and have to wait until their
    // Poll() occurs.
    //
    // Since there is no order dependency, we visit
    // all MenloObject's in the s_pollList() until all
    // processing events are drained by an indication
    // of a non zero pollInterval.
    //

    do {
        tmp = s_pollList;
        pollInterval = MAX_POLL_TIME;

        while (tmp != NULL) {

#if DISPATCH_MEMORY_CHECKS
            MenloMemoryMonitor::CheckMemory(LineNumberBaseDispatch + __LINE__);

            //
            // This will show the address of the class. Since most are statically
            // allocated you can consult the linker map to see which class caused
            // the memory corruption upon return.
            //

            //MenloDebug::PrintNoNewline(F("disp poll "));
            //MenloDebug::Print((int)tmp);
#endif

            newPollTime = tmp->Poll();

            // Check memory after each callback in case stack overflowed
            MenloMemoryMonitor::CheckMemory(LineNumberBaseDispatch + __LINE__);

            if (newPollTime < pollInterval) {
                pollInterval = newPollTime;
            }

            tmp = tmp->m_link;
        }

        //
        // Note: there is no watchdog reset here since the design is that
        // the processing chain either completes within the watchdog timeout,
        // or lengthy processing modules handle the watchdog keep alive
        // call themselves.
        //
        // A common bug is setting a MenloTimer with a period of 0 which
        // causes an endless loop, and we want the watchdog to catch it.
        //

    } while (pollInterval == 0);

    //
    // The second pass invokes any classes which have registered
    // for the PollEvent. This is used by classes/objects which are
    // not subclasses of MenloDispatchObject(), or may dynamically
    // register/unregister for PollEvent's based on application
    // and/or hardware state.
    //
    newPollTime = DispatchPollEvents();

    // Check memory after each callback in case stack overflowed
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDispatch + __LINE__);

    if (newPollTime < pollInterval) {
        pollInterval = newPollTime;
    }

    return pollInterval;
}

//
// MenloEvent based Poll() dispatching.
//

void
MenloDispatchObject::RegisterPollEvent(MenloEventRegistration* callback)
{
    // Add to event list
    s_pollEventList.Register(callback);
    return;
}

void
MenloDispatchObject::UnregisterPollEvent(MenloEventRegistration* callback)
{
    // Unregister from event list
    s_pollEventList.Unregister(callback);
    return;
}

//
// Dispatch registered PollEvents.
//
unsigned long
MenloDispatchObject::DispatchPollEvents()
{
    MenloEventArgs eventArgs;
    return s_pollEventList.DispatchEvents(NULL, &eventArgs);
}

//
// MenloDispatchObject instance class support routines
//

// Constructor
MenloDispatchObject::MenloDispatchObject()
{
  //
  // Each subclass runs this constructor and adds itself to the
  // global poll list.
  //
  this->m_link = s_pollList;
  s_pollList = this;
}

MenloDispatchObject::~MenloDispatchObject()
{
  MenloDispatchObject* tmp;
  MenloDispatchObject* prev;

  prev = NULL;

  tmp = s_pollList;

  while(tmp != NULL) {

    if (tmp == this) {

      if (prev == NULL) {
	s_pollList = tmp->m_link;
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

int
MenloDispatchObject::Initialize()
{
  return 0;
}

unsigned long
MenloDispatchObject::Poll()
{
  return  MAX_POLL_TIME;
}
