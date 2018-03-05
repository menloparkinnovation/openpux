
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
 *  Date: 02/22/2015
 *  File: DweetLightHouse.h
 *
 *  LightHouse application Dweet handling.
 *
 *   Refactored from example app as a separate module to support
 *   multiple Dweet channels.
 */

#ifndef DweetLightHouse_h
#define DweetLightHouse_h

#include "MenloObject.h"
#include <DweetApp.h>
#include "DweetRadio.h"

//
// Applications non-volatile storage allocations
//
#include "LightHouseConfig.h"

//
// Strings used in DweetLightHouse
//
extern const char dweet_light_string[];
extern const char dweet_lightperiod_string[];
extern const char dweet_lighttick_string[];
extern const char dweet_lightcolor_string[];
extern const char dweet_lightramp_string[];
extern const char dweet_lightsq_string[];
extern const char dweet_lightsp_string[];
extern const char dweet_lightonlevel_string[];
extern const char dweet_sensorrate_string[];

// Sensor/environmental support
extern const char dweet_sensors_string[];

//
// The application provides Dweet handling contracts to be part
// of the main programs Dweet dispatch loop.
//
class DweetLightHouse : public DweetApp  {

public:

    DweetLightHouse() {
    }

    int
    Initialize(
       LightHouseApp* lightHouseApp
       );

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    // Returns 0 to mean the command was not recognized or processed
    // and to continue searching for a handler.
    //
    // Return 1 if the command was processed an it should not continue
    // looking for a handler.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // GET/SET property commands
    //

    int LightPeriod(char* buf, int size, bool isSet);

    int LightTick(char* buf, int size, bool isSet);

    int LightColor(char* buf, int size, bool isSet);

    int LightRamp(char* buf, int size, bool isSet);

    int LightOnLevel(char* buf, int size, bool isSet);

    int SensorUpdateRate(char* buf, int size, bool isSet);

    int ProcessSensors(char* buf, int size, bool isSet);

    //
    // Common strings
    //
    static char* onState;
    static char* offState;

 protected:

    //
    // These are internal to the class, or sub-classes
    //

    int ProcessGetLightSequence(MenloDweet* dweet, char* name, char* value, bool persistent);

    int ProcessSetLightSequence(MenloDweet* dweet, char* name, char* value, bool persistent);

private:

    // LightHouse Application state
    LightHouseApp* m_lightHouseApp;
};

#endif // DweetLightHouse_h
