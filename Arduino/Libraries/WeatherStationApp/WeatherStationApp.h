
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
 * Date: 05/13/2015
 * File: WeatherStationApp.h
 *
 * WeatherStation Application Class
 *
 */

#ifndef WeatherStationApp_h
#define WeatherStationApp_h

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

#include "WeatherStationHardwareBase.h"

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
#define APP_STORAGE_BASE_INDEX WEATHER_MODULE_BASE_INDEX

#define WEATHER_INTERVAL_INDEX        APP_STORAGE_BASE_INDEX
#define WEATHER_INTERVAL_SIZE         9 // 00000000'\0'

#define WEATHER_SAMPLEINTERVAL_INDEX (WEATHER_INTERVAL_INDEX + WEATHER_INTERVAL_SIZE)
#define WEATHER_SAMPLEINTERVAL_SIZE   9 // 00000000'\0'

#define WEATHER_NMEA_INDEX           (WEATHER_SAMPLEINTERVAL_INDEX + WEATHER_SAMPLEINTERVAL_SIZE)
#define WEATHER_NMEA_SIZE             5 // 0000'\0'

#define WEATHER_GPSPOWER_INDEX       (WEATHER_NMEA_INDEX + WEATHER_NMEA_SIZE)
#define WEATHER_GPSPOWER_SIZE         9 // 00000000'\0'

#define WEATHER_LIGHT1_INDEX         (WEATHER_GPSPOWER_INDEX + WEATHER_GPSPOWER_SIZE)
#define WEATHER_LIGHT1_SIZE           5 // 0000'\0'

#define WEATHER_LIGHT2_INDEX         (WEATHER_LIGHT1_INDEX + WEATHER_LIGHT1_SIZE)
#define WEATHER_LIGHT2_SIZE           5 // 0000'\0'

// Note: This must be last as the checksum range depends on it
#define WEATHER_CHECKSUM   (WEATHER_LIGHT2_INDEX + WEATHER_LIGHT2_SIZE)
#define WEATHER_CHECKSUM_SIZE 2

// The first saved value is the start of the Weather checksum range
#define WEATHER_CHECKSUM_BEGIN         WEATHER_INTERVAL_INDEX
#define WEATHER_CHECKSUM_SIZE          2

// 46 bytes 03/06/2016

// End of range for Weather checksum is the start of the checksum storage location
#define WEATHER_CHECKSUM_END  WEATHER_CHECKSUM

//
// These values do not have EEPROM storage entries but require size
// for the parameters to the command.
//

#define WEATHER_WINDSPEED_SIZE        9
#define WEATHER_WINDDIR_SIZE          9
#define WEATHER_TEMPERATURE_SIZE      9
#define WEATHER_BAROMETER_SIZE        9
#define WEATHER_HUMIDITY_SIZE         9
#define WEATHER_RAINFALL_SIZE         9
#define WEATHER_GPS_SIZE              9
#define WEATHER_BATTERY_SIZE          9
#define WEATHER_SOLAR_SIZE            9
#define WEATHER_LIGHT_SIZE            9
#define WEATHER_SENDREADINGS_SIZE     2

//
// This must be the maximum size of all values above as its
// used for temporary command buffers on the stack.
//
#define WEATHER_MAX_SIZE WEATHER_INTERVAL_SIZE

struct WeatherStationConfiguration {
    unsigned long updateInterval;
    unsigned long sampleInterval;
};

struct WeatherStationReadings {
    unsigned long updateInterval;
    unsigned long sampleInterval;

    int GpsPower;
    int WindSpeed;     // 100's of MPH
    int WindDirection; // degrees
    int Temperature;   // 100's of a degree F
    int Barometer;
    int Humidity;      // 100's of a percent humidity
    int RainFall;      // 100's of an inch
    int Battery;       // 10 bit ADC value
    int Solar;         // 10 bit ADC value
    int Light;         // Light Intensity
};

//
// qualityIndicator
//  0 = no fix
//  1 = GPS fix
//  2 = DGPS fix
//  8 = Simulation mode
//
struct WeatherStationGpsData {

    int latitude;
    int longitude;
    int altitude;
    char sats;   // 00 - 12
    char qualityIndicator; // 0 == no fix
    bool isNorth; // True if latitude == N, otherwise == S
    bool isEast;  // True if longitude == E, otherwise == W

    // UTC time/date
    char year;
    char month;
    char day;
    char hour;
    char minute;
    char second;
    char hundreds;
};

//
// DweetApp allows standard signatures for event callbacks
// and ProcessAppCommands.
//
class WeatherStationApp : public DweetApp  {

public:

    WeatherStationApp() {
    }

    int Initialize(
        WeatherStationConfiguration* config,
        WeatherStationHardwareBase* hardware
        );

    void SetDweet(MenloDweet* dweet) {
        m_dweet = dweet;
    }

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // Read sensors
    //
    int ReadSensors(WeatherStationReadings* readings);

    //
    // Print sensors
    //
    int PrintSensors(WeatherStationReadings* readings);

    //
    // Send sensors as NMEA 0183 messages
    //
    int SendSensorsAsNMEA();

    int SendWindAsNMEA(WeatherStationReadings* readings);

    int SendMeteorologicalAsNMEA(WeatherStationReadings* readings);

    int SendGpsAsNMEA(WeatherStationReadings* readings);

    //
    // GET/SET property commands
    //

    // With EEPROM storage, power on state
    int UpdateInterval(char* buf, int size, bool isSet);
    int SampleInterval(char* buf, int size, bool isSet);
    int NmeaStream(char* buf, int size, bool isSet);
    int GpsPower(char* buf, int size, bool isSet);
    int Light1(char* buf, int size, bool isSet);
    int Light2(char* buf, int size, bool isSet);

    // Dynamic state at runtime
    int WindSpeed(char* buf, int size, bool isSet);
    int WindDirection(char* buf, int size, bool isSet);
    int Temperature(char* buf, int size, bool isSet);
    int Barometer(char* buf, int size, bool isSet);
    int Humidity(char* buf, int size, bool isSet);
    int Rain(char* buf, int size, bool isSet);
    int Gps(char* buf, int size, bool isSet);
    int Battery(char* buf, int size, bool isSet);
    int Solar(char* buf, int size, bool isSet);
    int Light(char* buf, int size, bool isSet);
    int SendReadings(char* buf, int size, bool isSet);

protected:

    // WeatherStation configuration
    WeatherStationConfiguration m_config;

private:

    //
    // Worker Routines
    //

    // Send an integer scaled by 100 as a decimal fraction such as x.x
    void SendDecimalFractionFromScaledInteger(MenloDweet* dweet, int value);

    void
    SendXDRValue(
        MenloDweet* dweet,
        PGM_P sensorType,
        PGM_P sensorName,
        PGM_P sensorUnits,
        int   sensorValue
        );

    // Application hardware state
    WeatherStationHardwareBase* m_hardware;

    //
    // Dweet channel if available for sending NMEA 0183 formatted
    // weather updates when configured.
    //
    MenloDweet* m_dweet;

    //
    // This is used to defer sensor readings when there
    // is a shared channel in place.
    //
    bool m_deferSensorReadings;

    //
    // A complex sensor such as WeatherStation registers for and receives
    // multiple events from the MenloFramework.
    // 
    // These events are:
    // 
    //  DweetEvent - Commands arriving over the NMEA 0183 channel
    // 
    //  PollEvent  - MenloFramework runtime event loop similar to
    //               the Arduino poll() function.
    //
    //               This is needed to drive sensor state machines such
    //               as receiving and processing NMEA 0183 data from
    //               the GPS.
    // 
    //  Update TimerEvent - A TimerEvent is used for sending updated sensor
    //               data on the communications channel.
    //
    //                Typical periods would be 30 seconds to a few minutes.
    //
    //  Sample TimerEvent - A TimerEvent is used for sensor sampling in which
    //               averages are kept.
    //
    //               Typical periods would be 1-30 seconds.
    // 

    // Dweet Serial Channel Event registration
    MenloDweetEventRegistration m_serialDweetEvent;

    // DweetEvent function
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // MenloFramework poll event
    //
    MenloEventRegistration m_pollEvent;

    // PollEvent function
    unsigned long PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Timer for sending sensor reading updates on the channel.
    //

    // Timer and event registration
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Timer for sampling sensors and calculating averages.
    //

    MenloTimer m_sampleTimer;
    MenloTimerEventRegistration m_sampleEvent;
    unsigned long SampleTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // WeatherStationApp_h
