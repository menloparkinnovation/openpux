
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
 * MenloGateway Application class.
 */

#ifndef MenloGatewayApp_h
#define MenloGatewayApp_h

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

//
// Configuration store entries store power on values
// in the EEPROM. They are automatically read and
// applied to the application objects properties
// at power on/reset.
//
// The values are checked summed to prevent incorrect
// device operation on a missing or damaged configuration.
//

// TOP_INDEX is defined in MenloConfigStore.h
#define APP_STORAGE_BASE_INDEX TOP_INDEX

#define GATEWAY_INTERVAL_INDEX        APP_STORAGE_BASE_INDEX
#define GATEWAY_INTERVAL_SIZE         9 // 00000000'\0'

// Note: This must be last as the checksum range depends on it
#define GATEWAY_CHECKSUM   (GATEWAY_INTERVAL_INDEX + GATEWAY_INTERVAL_SIZE)
#define GATEWAY_CHECKSUM_SIZE 2

// The first saved value is the start of the INTERVAL checksum range
#define GATEWAY_CHECKSUM_BEGIN         GATEWAY_INTERVAL_INDEX
#define GATEWAY_CHECKSUM_SIZE          2

// End of range for BLINK checksum is the start of the checksum storage location
#define GATEWAY_CHECKSUM_END  GATEWAY_CHECKSUM

#define GATEWAY_MAX_SIZE GATEWAY_INTERVAL_SIZE

struct MenloGatewayConfiguration {
    int pinNumber;
    unsigned long interval;
};

//
// DweetApp allows standard signatures for event callbacks
// and ProcessAppCommands.
//
class MenloGatewayApp : public DweetApp  {

public:

    MenloGatewayApp() {
        m_lightToggle = false;
    }

    int Initialize(MenloGatewayConfiguration* config);

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // GET/SET property commands
    //

    // Set status light interval
    int Interval(char* buf, int size, bool isSet);

protected:

    //
    // Hardware functions
    //
    virtual void SetLightState(bool state) = 0;

    MenloGatewayConfiguration m_config;

private:

    bool m_lightToggle;

    //
    // A timer is used to provide a status light interval
    //

    // Timer and event registration
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloGatewayApp_h
