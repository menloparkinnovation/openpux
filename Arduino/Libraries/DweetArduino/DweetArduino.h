
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
 *  Date: 04/26/2015
 *  File: DweetArduino.h
 *
 *  Handle Arduino hardware manipulation similar to Firmmata.
 */

#ifndef DweetArduino_h
#define DweetArduino_h

#include "MenloObject.h"

//
// This class takes 1966 bytes on an AtMega328
//
// TODO: Convert to table driven and cut down on space.
//
// TODO: Subclass DweetApp to automatically have a common
// registration hander for Dweets.
//
class DweetArduino : public MenloObject  {

public:

    DweetArduino() {
        m_dweet = NULL;
    }

    int Initialize(MenloDweet* dweet);

    //
    // Arduino commands processing.
    //
    // These commands allow Dweet access to the Arduino hardware ports.
    //
    // Returns 1 if the command is recognized.
    // Returns 0 if not.
    //
    int ProcessArduinoCommands(MenloDweet* dweet, char* name, char* value);

private:

    MenloDweet* m_dweet;

    //
    // DweetArduino is a client of MenloDweet for unhandled Dweet events
    //  

    // Event registration
    MenloDweetEventRegistration m_dweetEvent;

    // DweetEvent function
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // DweetArduino_h
