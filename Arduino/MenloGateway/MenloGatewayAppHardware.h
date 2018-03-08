
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
 * Date: 05/23/2015
 * File: MenloGatewayAppHardware.h
 *
 * MenloGateway Hardware class.
 *
 */

#ifndef MenloGatewayAppHardware_h
#define MenloGatewayAppHardware_h

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

#include <MenloGatewayApp.h>

//
// This is separate to allow MenloGatewayApp to operate on non-Arduino
// hardware as well.
//

class MenloGatewayAppHardware : public MenloGatewayApp  {

public:

protected:

    virtual void SetLightState(bool state);
};

#endif // MenloGatewayAppHardware_h
