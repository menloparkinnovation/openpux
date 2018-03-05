
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
 *  Date: 04/28/2015
 *  File: DweetSensorApp.h
 *
 *  Sensor application framework.
 */

#ifndef DweetSensorApp_h
#define DweetSensorApp_h

#include "MenloObject.h"
#include <DweetApp.h>
#include "DweetRadio.h"

//
// Default environmental monitoring interval
//
#define SENSOR_DEFAULT_INTERVAL (30L * 1000L) // 30 seconds

//
// Configuration Store
//

//
// TOP_INDEX is in MenloConfigStore.h and is above the pre-configured
// platform items.
//
#define SENSORAPP_STORAGE_BASE_INDEX TOP_INDEX

// begining of sensor block that is checksumed
#define SENSORAPP_SENSORRATE         SENSORAPP_STORAGE_BASE_INDEX
#define SENSORAPP_SENSORRATE_SIZE    5
// End of sensor block that is checksumed

// Note: This must be last as the checksum range depends on it
#define SENSORAPP_CHECKSUM   (SENSORAPP_SENSORRATE + SENSORAPP_SENSORRATE_SIZE)

#define SENSORAPP_CHECKSUM_SIZE 2

// The first saved value is the start of configuration range
#define SENSORAPP_CHECKSUM_BEGIN SENSORAPP_SENSORRATE

// End of range for configuration block is the start of the checksum storage location
#define SENSORAPP_CHECKSUM_END  SENSORAPP_CHECKSUM

#define SENSORAPP_MAX_SIZE SENSORAPP_SENSORRATE_SIZE

// Sub-applications of DweetSensorApp have storage after the base class values
#define SENSORAPP_SUBAPP_STORAGE_BASE_INDEX (SENSORAPP_CHECKSUM + SENSORAPP_CHECKSUM_SIZE)

//
// The application provides Dweet handling contracts to be part
// of the main programs Dweet dispatch loop.
//
class DweetSensorApp : public DweetApp  {

public:

    DweetSensorApp();

    int Initialize();

    void SetDweet(MenloDweet* dweet) {
        m_dweet = dweet;
    }

    MenloDweet* GetDweet() {
        return m_dweet;
    }

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    // This is handled by the parent class DweetApp which invokes
    // this virtual when the registered event occurs.
    //
    // Arguments:
    //
    // dweet - Channel the command came in on, and should be used
    //         if there are any replies.
    //
    // name  - Name of the Dweet being request such as GETCONFIG, SETCONFIG, etc.
    //
    // value - Item being operated on such as NAME, SERIALNUMBER, etc.
    //
    //         Note: Value may have an additional separator such as ":"
    //         for commands such as SETCONFIG=SERIALNUMBER:00000000
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // Applications based on DweetSensorApp have the option of using
    // sub-classing and virtual methods rather than their own independent
    // registration for received Dweets and Timer events.
    //
    // They are free to do so, for simple extensions of a basic application
    // subclassing is more straightforward than event registrations which
    // shines as a larger component integration framework.
    //
    virtual int SubClassProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // This is invoked from the sensor timer to allow sensors
    // to process.
    //
    virtual unsigned long SensorTimerEvent();

    //
    // GET/SET property commands
    //

    int SensorUpdateRate(char* buf, int size, bool isSet);

protected:

    // If set sensor updates are sent
    bool m_sendSensorUpdates;

    MenloDweet* m_dweet;

private:

    // Load settings from EEPROM
    void InitializeStateFromStoredConfig();

    // Reset the update timer to the new value
    void SetSensorUpdateRate(uint16_t value);

    //
    // MenloTimer is used to schedule periodic sensor
    // readings.
    //
    // This is also used to send sensor updates if configured.
    //
    unsigned long m_sensorTimerInterval;

    // Timer and event registration
    MenloTimer m_sensorTimer;
    MenloTimerEventRegistration m_sensorTimerEvent;

    // Sensor TimerEvent function
    unsigned long LocalSensorTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    // Indicates when we can start async processing
    bool m_initialized;
};

#endif // DweetSensorApp_h
