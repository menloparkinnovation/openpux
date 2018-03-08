
/*
 * Copyright (C) 2018 Menlo Park Innovation LLC
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
 * Date: 02/28/2018
 * File: MenloWatchDogApp.h
 *
 * MenloWatchDog Application class.
 *
 */

#ifndef MenloWatchDogApp_h
#define MenloWatchDogApp_h

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
// 5 minutes in seconds Note: This is an int so max is ~500 minutes for default
// though the Dweet command can specify a longer interval.
//
#define WATCHDOG_TIMEOUT_DEFAULT (5 * 60)

//
// Time that the RESET output is transitioned on watchdog timeout
// in milliseconds.
//
#define WATCHDOG_RESET_DEFAULT   (200)

//
// Time that the POWER output is transistioned on watchdog timeout
// in milliseconds.
//
#define WATCHDOG_POWER_DEFAULT   (5000)

//
// Watchdog resets are stored in EEPROM, so nothing to initialize.
//
#define WATCHDOG_RESETS_DEFAULT   (0)

//
// Default indicator state (on)
//
#define WATCHDOG_IND_DEFAULT   (1)

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

#define WATCHDOG_TIMEOUT_INDEX        APP_STORAGE_BASE_INDEX
#define WATCHDOG_TIMEOUT_SIZE         9 // 00000000'\0'

#define WATCHDOG_RESET_INDEX          (WATCHDOG_TIMEOUT_INDEX + WATCHDOG_TIMEOUT_SIZE)
#define WATCHDOG_RESET_SIZE           9 // 00000000'\0'

#define WATCHDOG_POWER_INDEX          (WATCHDOG_RESET_INDEX + WATCHDOG_RESET_SIZE)
#define WATCHDOG_POWER_SIZE           9 // 00000000'\0'

#define WATCHDOG_RESETS_INDEX         (WATCHDOG_POWER_INDEX + WATCHDOG_POWER_SIZE)
#define WATCHDOG_RESETS_SIZE          9 // 00000000'\0'

#define WATCHDOG_IND_INDEX            (WATCHDOG_RESETS_INDEX + WATCHDOG_RESETS_SIZE)
#define WATCHDOG_IND_SIZE              9 // 00000000'\0'

// Note: This must be last as the checksum range depends on it
#define WATCHDOG_CHECKSUM   (WATCHDOG_IND_INDEX + WATCHDOG_IND_SIZE)
#define WATCHDOG_CHECKSUM_SIZE 2

// The first saved value is the start of the LIGHT checksum range
#define WATCHDOG_CHECKSUM_BEGIN         WATCHDOG_TIMEOUT_INDEX
#define WATCHDOG_CHECKSUM_SIZE          2

// End of range for BLINK checksum is the start of the checksum storage location
#define WATCHDOG_CHECKSUM_END  WATCHDOG_CHECKSUM

// Maximum size of any entry
#define WATCHDOG_MAX_SIZE WATCHDOG_TIMEOUT_SIZE

struct WatchdogConfiguration {
    unsigned long timeout_time;
    unsigned long reset_time;
    unsigned long power_time;
    unsigned long number_of_resets;
    int indicator;
};

//
// DweetApp allows standard signatures for event callbacks
// and ProcessAppCommands.
//
class MenloWatchDogApp : public DweetApp  {

public:

    MenloWatchDogApp() {
    }

    int Initialize(WatchdogConfiguration* config);

    //
    // Invoked to reset the watchdog timer.
    //

    void KeepAlive();

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // GET/SET property commands
    //

    int Timeout(char* buf, int size, bool isSet);
    int Reset(char* buf, int size, bool isSet);
    int Power(char* buf, int size, bool isSet);
    int Indicator(char* buf, int size, bool isSet);
    int Resets(char* buf, int size, bool isSet);
    int Poke(char* buf, int size, bool isSet);

    //
    // Update EEPROM with resets
    //

    int WriteResetsToEEPROM();

protected:

    //
    // Hardware functions
    //
    virtual void SetLightState(bool state);

    WatchdogConfiguration m_config;

private:

    bool m_performingReset;
    bool m_performingPowerCycle;

    //
    // A timer is used to provide the watchdog interval
    //

    // Timer and event registration
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Reset timer used when performing reset/power cycle function.
    //
    MenloTimer m_resetTimer;
    MenloTimerEventRegistration m_resetTimerEvent;

    // ResetTimerEvent function
    unsigned long ResetTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Dweet channel support over serial.
    //
    DweetSerialChannel m_dweetSerialChannel;

    // DweetEvent function
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloWatchDogApp_h
