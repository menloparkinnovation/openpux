
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
 *  Date: 04/27/2015
 *  File: DweetApp.h
 *
 *  Template class for Dweet applications.
 */

#ifndef DweetApp_h
#define DweetApp_h

//
// DweetApp provides a standard contract signature for Dweet applications.
//

class DweetApp : public MenloObject  {

public:

    DweetApp();

    int Initialize();

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for the app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value) = 0;

    //
    // DweetEvent function
    //
    // This allows subclasses to use a common Dweet event delivery function.
    //
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

private:

    //
    // Each DweetApp registers for the unhandled Dweet event hander
    // in which Dweets arrive from all transports.
    //
    MenloDweetEventRegistration m_dweetEvent;

};

#endif // DweetApp_h
