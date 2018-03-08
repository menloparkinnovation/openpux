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
 * Date: 05/07/2015
 * File: DweetBlinkAppArduino
 *
 * Arduino Hardware class.
 *
 */

#ifndef DweetBlinkAppArduino_h
#define DweetBlinkAppArduino_h

//
// MenloPlatform support
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloDispatchObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloTimer.h>
#include <MenloPower.h>

//
// MenloDweet Support
//
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetApp.h>
#include <DweetSerialChannel.h>

#include <DweetBlinkApp.h>

//
// This is separate to allow DweetBlinkApp to operate on non-Arduino
// hardware as well.
//
// MenloObject.h
//   DweetApp.h
//     - Registers for GlobalUnhandledDweetEvent
//     - Invokes ProcessAppCommands() in the inheritance hierarchy (virtual)
//     DweetBlinkApp.h
//       - contains DweetSerialChannel m_dweetSerialChannel
//       - registers to receive application specific dweet events
//         - set light timer period
//       - registers for MenloTimer, application period timer
//       DweetBlinkAppArduino (DweetBlinkAppArduino.h, this file)
//       - Set the light state based on timer.
//

class DweetBlinkAppArduino : public DweetBlinkApp  {

public:

protected:

    virtual void SetLightState(bool state);
};

#endif // DweetBlinkAppArduino_h
