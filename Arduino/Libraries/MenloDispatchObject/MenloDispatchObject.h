
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
// MenloDispatchObject.h
// 04/02/2014
//

#ifndef MenloDispatchObject_h
#define MenloDispatchObject_h

#include "MenloPlatform.h"
#include "MenloObject.h"

//
// Set this if there is list corruption due to double linked
// events by the application, etc.
//
#define MENLOEVENT_DEBUG 1

//
// A MenloDispatchObject is automatically
// registered for a Poll() dispatch/processing
// chain and can raise application level events.
//

//
// An object that is not a subclass of MenloDispatchObject
// can register for poll event throught the event dispatching model.
//

#define MAX_POLL_TIME 0xFFFFFFFF

class MenloObject;
class MenloDispatchObject;
class MenloEvent;

class MenloEventArgs {
};

//
// Any MenloObject can register to receive events.
//
// Only MenloDispatchObject may send events when their Poll() dispatch
// function is invoked by the scheduler.
//

typedef unsigned long (MenloObject::*MenloEventMethod)(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

//
// An event registration object is allocated by the caller
// and is typically an embedded class.
//
// This is 7 bytes on an AtMega328.
//
class MenloEventRegistration {
 public:

  MenloObject* object;
  MenloEventMethod method;

  MenloEventRegistration* GetNext();

 private:

  MenloEventRegistration* m_link;

  friend class MenloEvent;
#if MENLOEVENT_DEBUG
  bool m_isLinked;
public:
  MenloEventRegistration() {
      m_isLinked = false;
  }
#endif
};

//
// A MenloEvent is typically embedded in a class that
// provides events.
//
class MenloEvent {
 public:

  MenloEvent();

  void Register(MenloEventRegistration* registration);

  void Unregister(MenloEventRegistration* registration);

  bool HasListeners() {
    if (m_eventList == NULL) return false;
    return true;
  }

  virtual unsigned long DispatchEvents(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

  virtual unsigned long DispatchSingleEvent(
      MenloDispatchObject* sender,
      MenloEventArgs* eventArgs,
      MenloEventRegistration* event
      );

  MenloEventRegistration* GetHead();

 private:

  MenloEventRegistration* m_eventList;
};

//
// This provides the base class for MenloFramework.
//
//
// Note: This class provides both static and instance
// methods and fields.
//
// The static methods and fields supports the application/system
// wide event dispatching loop.
//
// The instance methods and fields support subclasses of
// MenloDispatchObject which utilizes the lower level
// event dispatching framework.
//
class MenloDispatchObject : public MenloObject {

public:
  
    //
    // static methods and fields used to implement the event dispatch loop.
    //

    //
    // loop uses power efficient sleep if enabled.
    //
    // This is called from the main processing loop of the
    // application to drive event dispatching.
    //
    static unsigned long loop(unsigned long maxSleepTime);

    //
    // Poll events allow Poll() to be delivered to classes which are not
    // subclasses of MenloDispatchObject.
    //
    // Note: The sender in the MenloEventMethod for these registrations
    //       is NULL since they are sent from the static MenloDispatchObject
    //       low level dispatch support.
    //
    // Note: See bottom of file about Poll() details.
    //
    static void RegisterPollEvent(MenloEventRegistration* callback);

    static void UnregisterPollEvent(MenloEventRegistration* callback);

private:

    //
    // DispatchPollEvents()
    //
    static unsigned long DispatchPollEvents();

    //
    // This operates on the Poll() list and is invoked by
    // the main scheduler loop.
    //
    static unsigned long LoopInternal();

    //
    // All MenloDispatchObjects are automatically inserted into the poll list.
    //
    // Note: This is prefixed by "s_" which denotes global static for the class
    // Each derived class share this copy of the list head.
    //
    static MenloDispatchObject* s_pollList;

    //
    // This is the list of events which are invoked at Poll().
    //
    static MenloEvent s_pollEventList;

public:

    //
    // Instance methods and fields used by subclasses to implmement
    // their event dispatching handlers.
    //

    MenloDispatchObject();
    ~MenloDispatchObject();

    virtual int Initialize();

    //
    // This version of Poll() is accessible by subclasses of
    // MenloDispatchObject.
    //
    // Note: See bottom of file about Poll() details.
    //
    virtual unsigned long Poll();

private:

    //
    // Link for per object m_pollList
    //
    // "m_" indicates per instance member. Each derived class has
    // their own copy of this link.
    //
    MenloDispatchObject* m_link;
};

//
// It can be tricky debugging the dispatch loops throughout
// the code and application.
//
// This define controls whether dispatch tracing is compiled in or not
// and can be used by application as well as the infrastructure.
//

#define DISPATCH_PRINT_ENABLED 0

#if DISPATCH_PRINT_ENABLED
#define DISPATCH_PRINT(x)     (MenloDebug::Print(F(x)))
#define DISPATCH_PRINT_INT(x) (MenloDebug::Print(x))
#define DISPATCH_PRINT2(x, y) (MenloDebug::PrintNoNewline(F(x)) && MenloDebug::Print(y))
#else
#define DISPATCH_PRINT(x)
#define DISPATCH_PRINT_INT(x)
#define DISPATCH_PRINT2(x, y)
#endif

//
// Poll() and the MenloEvent dispatching model.
//
// Poll() is periodically called by the MenloFramework to allow
// an object/sensor/component to perform work.
//
// There is no guarantee of any timing relationship between
// calls to Poll().
//
// The object returns a value in milliseconds in which it
// requests that Poll() be called again. This represents
// the interval that a given object requires to properly
// operate.
//
// A return value of 0 means that an object requests the entire
// Poll() chain be executed again. This is due to the object
// changing state in the application, raising events, etc,
// which could cause other objects to require updating.
// The object that returns 0 from Poll() will be called again
// in its polling sequence. This is done to prevent recursion
// on limited stacks and is similar to many event driven
// frameworks with async callback dispatchers.
//
// Objects should be careful to return the longest time
// till the next desired Poll() interval to allow the
// main application to maximize sensor sleep intervals
// to save power and battery life.
//
// Objects which are slave devices which only perform
// meaningful processing when a method or event is
// invoked should return the default Poll() time
// of MAX_POLL_TIME. This is provided automatically
// by the default implementation of Poll().
//
// An object that receives a Poll() should be careful
// to be conservative with stack space.
//
// Objects which raise events in their Poll() loop are
// required to return the shortest interval of any
// of the Event invokes return value, and their required
// Poll() interval. This is to ensure no object gets
// starved when an event causes a state change
// that requires further processing, including the
// delivery of other events. This is because its illegal
// to deliver events to listers from within the event
// handler to conserve stack space and limit recursion.
//
// See the Event definition and handling code for details.
//
// return value:
//
//  Time in milliseconds before the next Poll() is desired.
//

#endif // MenloDispatchObject_h


