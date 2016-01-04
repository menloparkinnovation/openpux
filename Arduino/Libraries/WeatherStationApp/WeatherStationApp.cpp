
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

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//
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
#include <MenloPower.h>
#include <MenloTimer.h>

#include "WeatherStationApp.h"

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define DBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_HEX_STRING(x, l)
#define DBG_PRINT_HEX_STRING_NNL(x, l)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#endif

//
// Using Menlo Dweet Node.js application
//
// node dweet.js <portname>
//
// Note: Values are in hex. 0x7530 == 30000 milliseconds, or 30 seconds.
//                          0x1288 ==  5000 milliseconds, or  5 seconds.
//
// Set the power on update interval in EEPROM
// This will also set a valid checksum for the configuration
// This also updates the runtime interval as if a reset had occured.
//
//   dweet SETCONFIG=UPDATEINTERVAL:00007530
//
// Get the power on interval in EEPROM
//   dweet GETCONFIG=UPDATEINTERVAL
//
// Set the temporary interval in memory. This setting
// is lost on power on/reset or watchdog timeout.
//
//   dweet SETSTATE=UPDATEINTERVAL:00007530
//
// Get the interval that is active regardless of from
// EEPROM or a previous SETSTATE=UPDATEINTERVAL command.
//
//   dweet GETSTATE=UPDATEINTERVAL
//

//
// Weather Station Control Commands
//
// These have persistent power on values stored in EEPROM
//

// Configure update send interval
const char dweet_updateinterval_string[] PROGMEM = "UPDATEINTERVAL";

// Configure sample interval
const char dweet_sampleinterval_string[] PROGMEM = "SAMPLEINTERVAL";

// Configure NMEA 0183 streaming
const char dweet_nmeastream_string[] PROGMEM = "NMEASTREAM";

// Configure GPS Power
const char dweet_gpspower_string[] PROGMEM = "GPSPOWER";

// Configure Status Light 1 and Status Light 2
const char dweet_light1_string[] PROGMEM = "LIGHT1";
const char dweet_light2_string[] PROGMEM = "LIGHT2";

//
// Sensor values that can be read individually
//
// These are dynamic values which don't have settings in EEPROM
//
const char dweet_windspeed_string[] PROGMEM = "WINDSPEED";
const char dweet_winddir_string[] PROGMEM = "WINDDIR";
const char dweet_temperature_string[] PROGMEM = "TEMPERATURE";
const char dweet_barometer_string[] PROGMEM = "BAROMETER";
const char dweet_humidity_string[] PROGMEM = "HUMIDITY";
const char dweet_rainfall_string[] PROGMEM = "RAINFALL";
const char dweet_gps_string[] PROGMEM = "GPS";
const char dweet_battery_string[] PROGMEM = "BATTERY";
const char dweet_solar_string[] PROGMEM = "SOLAR";
const char dweet_light_string[] PROGMEM = "LIGHT";
const char dweet_sendreadings_string[] PROGMEM = "SENDREADINGS";

const char* const weather_string_table[] PROGMEM =
{
    dweet_updateinterval_string,
    dweet_sampleinterval_string,
    dweet_nmeastream_string,
    dweet_gpspower_string,
    dweet_light1_string,
    dweet_light2_string,
    dweet_windspeed_string,
    dweet_winddir_string,
    dweet_temperature_string,
    dweet_barometer_string,
    dweet_humidity_string,
    dweet_rainfall_string,
    dweet_gps_string,
    dweet_battery_string,
    dweet_solar_string,
    dweet_light_string,
    dweet_sendreadings_string
};

// Locally typed version of state dispatch function
typedef int (WeatherStationApp::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod weather_function_table[] =
{
    // EEPROM stored, power on configurable values
    &WeatherStationApp::UpdateInterval,
    &WeatherStationApp::SampleInterval,
    &WeatherStationApp::NmeaStream,
    &WeatherStationApp::GpsPower,
    &WeatherStationApp::Light1,
    &WeatherStationApp::Light2,

    // Dynamic values
    &WeatherStationApp::WindSpeed,
    &WeatherStationApp::WindDirection,
    &WeatherStationApp::Temperature,
    &WeatherStationApp::Barometer,
    &WeatherStationApp::Humidity,
    &WeatherStationApp::Rain,
    &WeatherStationApp::Gps,
    &WeatherStationApp::Battery,
    &WeatherStationApp::Solar,
    &WeatherStationApp::Light,
    &WeatherStationApp::SendReadings
};

//
// 0 entries have no persistent EEPROM storage requirement.
//
PROGMEM const int weather_index_table[] =
{
  WEATHER_INTERVAL_INDEX,
  WEATHER_SAMPLEINTERVAL_INDEX,
  WEATHER_NMEA_INDEX,
  WEATHER_GPSPOWER_INDEX,
  WEATHER_LIGHT1_INDEX,
  WEATHER_LIGHT2_INDEX,
  0, // windspeed
  0, // winddir
  0, // temperature
  0, // barometer
  0, // humidity
  0, // rainfall
  0, // gps
  0, // battery
  0, // solar
  0, // light
  0  // send sensors
};

//
// All entries require a size to determine string length required
// for command.
//
PROGMEM const int weather_size_table[] =
{
  WEATHER_INTERVAL_SIZE,
  WEATHER_SAMPLEINTERVAL_SIZE,
  WEATHER_NMEA_SIZE,
  WEATHER_GPSPOWER_SIZE,
  WEATHER_LIGHT1_SIZE,
  WEATHER_LIGHT2_SIZE,
  WEATHER_WINDSPEED_SIZE,     // windspeed
  WEATHER_WINDDIR_SIZE,       // winddir
  WEATHER_TEMPERATURE_SIZE,   // temperature
  WEATHER_BAROMETER_SIZE,     // barometer
  WEATHER_HUMIDITY_SIZE,      // humidity
  WEATHER_RAINFALL_SIZE,      // rainfall
  WEATHER_GPS_SIZE,           // gps
  WEATHER_BATTERY_SIZE,       // battery
  WEATHER_SOLAR_SIZE,         // solar
  WEATHER_LIGHT_SIZE,         // light intensity
  WEATHER_SENDREADINGS_SIZE   // send sensor readings
};

int
WeatherStationApp::Initialize(
    WeatherStationConfiguration* config,
    WeatherStationHardwareBase* hardware
    )
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[WEATHER_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(weather_string_table) / sizeof(char*);

    m_config = *config;

    m_hardware = hardware;

    m_deferSensorReadings = false;

    // Setup a TimerEvent for sending updates
    m_timerEvent.object = this;
    m_timerEvent.method = (MenloEventMethod)&WeatherStationApp::TimerEvent;

    m_timerEvent.m_interval = m_config.updateInterval;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    // Setup a TimerEvent for sensor sampling and averaging
    m_sampleEvent.object = this;
    m_sampleEvent.method = (MenloEventMethod)&WeatherStationApp::SampleTimerEvent;

    m_sampleEvent.m_interval = m_config.sampleInterval;
    m_sampleEvent.m_dueTime = 0L; // indicate not registered

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (WeatherStationApp) since
    // the handlers are on this class.
    //
    parms.stringTable = (PGM_P)weather_string_table;
    parms.functionTable = (PGM_P)weather_function_table;
    parms.object =  this;
    parms.indexTable = weather_index_table;
    parms.sizeTable = weather_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = WEATHER_CHECKSUM;
    parms.checksumBlockStart = WEATHER_CHECKSUM_BEGIN;
    parms.checksumBlockSize = WEATHER_CHECKSUM_END - WEATHER_CHECKSUM_BEGIN;
    parms.name = NULL;
    parms.value = NULL;

    // DweetState.cpp
    result = MenloDweet::LoadConfigurationSettingsTable(&parms);
    if (result != 0) {
        if (result == DWEET_INVALID_CHECKSUM) {

            //
            // This error occurs if SETCONFIG has not been used to set
            // any valid settings since the device has been reflashed so
            // that the EEPROM is in an indeterminate state.
            //
            // It can also be the result of corruption of previously
            // valid EEPROM settings.
            //

            MenloDebug::Print(F("WeatherStationApp Stored settings checksum is invalid"));
        }
        else {

            //
            // This error occurs if one of the property handlers such as
            // Interval() rejects the value due to misconfigured or otherwise
            // corrupted value.
            //

            MenloDebug::Print(F("WeatherStationApp Stored settings are invalid"));
        }
    }
    else {

        //
        // Configuration is loaded and enabled if:
        //
        // 1) The stored checksum for the applications configuration
        //    EEPROM block is valid.
        //
        // 2) The application property methods such as Interval() have
        //    accepted and activated the value on the application.
        //

        MenloDebug::Print(F("WeatherStationApp Stored settings are valid"));
    }

    //
    // Register for PollEvent to drive streaming sensors
    //
    m_pollEvent.object = this;
    m_pollEvent.method = (MenloEventMethod)&WeatherStationApp::PollEvent;

    MenloDispatchObject::RegisterPollEvent(&m_pollEvent);

    //
    // Start the timer running if it was not already started
    // by Interval() being invoked by the EEPROM configuration
    // routines.
    //
    if (!m_timer.IsTimerRegistered(&m_timerEvent)) {
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }

    if (!m_sampleTimer.IsTimerRegistered(&m_sampleEvent)) {
        m_sampleTimer.RegisterIntervalTimer(&m_sampleEvent);
    }

    return 0;
}

unsigned long
WeatherStationApp::PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    unsigned long waitTime = MAX_POLL_TIME;

    //DBG_PRINT("WeatherStationApp PollEvent");

    waitTime = m_hardware->PollSensors();

    //
    // If sensor readings were deferred, check to see
    // if the channel is still busy.
    //
    if (m_deferSensorReadings) {

        if (!m_hardware->SensorBusy()) {

            // Reset deferral status
            m_deferSensorReadings = false;

            //
            // Send the sensor update now that was required
            // by the TimerEvent.
            //
            SendSensorsAsNMEA();
        }
    }

    return waitTime;
}

//
// This is the event in which the current sensor readings and
// averages are sent on the communications channel in the configured
// format.
//
unsigned long
WeatherStationApp::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    DBG_PRINT("WeatherStationApp TimerEvent");

    if (m_deferSensorReadings) {

        //
        // Recovery: If we get two timer periods with
        // sensor readings deferred recover by clearing
        // and continuing on with NMEA 0183 sensor data
        // processing.
        //
        // This can happen if a GPS message is corrupted,
        // the GPS stops sending, or it was powered off by
        // the application in the middle of a pass through
        // GPS NMEA 0183 sentence.
        //
        m_deferSensorReadings = false;
    }
    else {

        //
        // See if the sensor channel is busy. If so we will defer
        // sending the NMEA 0183 sensor updates until Poll()
        // time.
        //
        // This allows the NMEA 0183 channel to be shared by
        // multiple sentence generators.
        //
        if (m_hardware->SensorBusy()) {
            m_deferSensorReadings = true;

            //
            // We return 0 so have Poll() called right away.
            // The sensor Poll() routine will determined how
            // often the sensor needs to be polled to finish
            // processing and clear its busy status.
            //
            return 0;
        }
    }

    // Handle processing of NMEA 0183 weather station data updates
    SendSensorsAsNMEA();

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

//
// The application can configure what sensor sample time is desired
// and is used for calculating averages.
//
// This is separate from the time in which sensor updates are sent,
// which are typically less frequent.
//
unsigned long
WeatherStationApp::SampleTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    DBG_PRINT("WeatherStationApp SampleTimerEvent");

    m_hardware->SampleSensors();

    // The timer sets the poll time for us based on programmed interval
    return MAX_POLL_TIME;
}

//
// Process Dweet application commands.
//
// Note: Dweet application commands can arrive from multiple
// transports so the argument value dweet* should be used for
// any replies in order to ensure the reply is returned on the
// channel its received from.
//
// Member variable m_dweet points to a channel specifically configured
// for sending asynchronous NMEA 0183 messages when configured to do so.
//
int
WeatherStationApp::ProcessAppCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;

    // Must be larger than any config values we fetch
    char workingBuffer[WEATHER_MAX_SIZE+1];

    int tableEntries = sizeof(weather_string_table) / sizeof(char*);

    //
    // Sensor operationg modes and sensor commands
    // follow the GETSTATE/SETSTATE, GETCONFIG/SETCONFIG pattern
    // and use the table driven common code.
    //

    DBG_PRINT("WeatherStationApp calling table processor");

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetSensor
    //
    parms.stringTable = (PGM_P)weather_string_table;
    parms.functionTable = (PGM_P)weather_function_table;
    parms.object =  this;
    parms.indexTable = weather_index_table;
    parms.sizeTable = weather_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = WEATHER_CHECKSUM;
    parms.checksumBlockStart = WEATHER_CHECKSUM_BEGIN;
    parms.checksumBlockSize = WEATHER_CHECKSUM_END - WEATHER_CHECKSUM_BEGIN;
    parms.name = name;
    parms.value = value;

    return dweet->ProcessStateCommandsTable(&parms);
}

//
// Interval in which updates are sent to the host.
//
// Decode the interval which is up to 8 ASCII hex characters
// representing the 32 bit interval.
//
int
WeatherStationApp::UpdateInterval(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {

        // Buffer must be supplied with at a '\0'
        if (size < 1) {
            return DWEET_INVALID_PARAMETER;
        }

        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            return DWEET_INVALID_PARAMETER;
        }

        // Update our application state
        m_config.updateInterval = interval;

        // Reset timer interval to new value
        if (m_timer.IsTimerRegistered(&m_timerEvent)) {
            m_timer.UnregisterIntervalTimer(&m_timerEvent);
        }

        // Update interval
        m_timerEvent.m_interval = m_config.updateInterval;

        // Restart timer on new interval
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }
    else {

        //
        // get current value
        //

        //
        // Size is validated by the common dispatcher to the size
        // set in the configuration table.
        //

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.updateInterval, buf);
        buf[8] = '\0';
    }

    return 0;
}

//
// Update the rate at which the sensors are monitored.
//
// Local values are updated and re-calculated as required
// and used to return values on Dweet commands or the next
// scheduled update interval.
//
// Typically this is a more frequent cycle to retrieve
// data from non-interrupt driven sensors, or to pull data
// from interrupt driven sensors which may overrun, such
// as the GPS when powered on.
//
int
WeatherStationApp::SampleInterval(char* buf, int size, bool isSet)
{
    bool error;
    int length;
    unsigned long interval;

    if (isSet) {

        // Buffer must be supplied with at a '\0'
        if (size < 1) {
            return DWEET_INVALID_PARAMETER;
        }

        interval = MenloUtility::HexToULong(buf, &error);
        if (error) {

            // Error codes in Libraries/MenloDweet/MenloDweet.h
            return DWEET_INVALID_PARAMETER;
        }

        // Update our application state
        m_config.sampleInterval = interval;

        // Reset timer interval to new value
        if (m_sampleTimer.IsTimerRegistered(&m_sampleEvent)) {
            m_sampleTimer.UnregisterIntervalTimer(&m_sampleEvent);
        }

        // Update interval
        m_sampleEvent.m_interval = m_config.sampleInterval;

        // Restart timer on new interval
        m_sampleTimer.RegisterIntervalTimer(&m_sampleEvent);
    }
    else {

        //
        // get current value
        //

        //
        // Size is validated by the common dispatcher to the size
        // set in the configuration table.
        //

        // 8 hex chars without '\0'
        MenloUtility::UInt32ToHexBuffer(m_config.sampleInterval, buf);
        buf[8] = '\0';
    }

    return 0;
}

int
WeatherStationApp::NmeaStream(char* buf, int size, bool isSet)
{
    // Allows SETCONFIG/GETCONFIG to work with EEPROM
    return 0;
}

//
// Set a GPS power on time.
//
//   0 - Gps power is always off
//   1 - Gps power is always on
// > 1 - Time in milliseconds to leave the power on, then turn
//       off automatically.
int
WeatherStationApp::GpsPower(char* buf, int size, bool isSet)
{
    bool value;
    bool error;
    unsigned long powerTime;

    if (isSet) {

        // Buffer must be supplied with at a '\0'
        if (size < 1) {
            return DWEET_INVALID_PARAMETER;
        }

        powerTime = MenloUtility::HexToULong(buf, &error);
        if (error) {
            return DWEET_INVALID_PARAMETER;
        }

        if (powerTime != 0L) {
            value = true;
            m_hardware->GpsPower(&value, isSet);
        }
        else {
            value = false;
            m_hardware->GpsPower(&value, isSet);
        }

        return 0;  
    }
    else {
        m_hardware->GpsPower(&value, false);

        if (value) {
            powerTime = 0x1L;
        }
        else {
            powerTime = 0x0L;
        }

        MenloUtility::UInt32ToHexBuffer(powerTime, buf);

        return 0;
    }
}

int
WeatherStationApp::Light1(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) {

        // Parameter is supplied by the caller, could be shorter than required
        if (size < 4) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        value = MenloUtility::HexToUShort(buf);

        m_hardware->Stat1LightState(&value, isSet);
    }
    else {

        m_hardware->Stat1LightState(&value, false);

        // 4 hex chars without '\0'
        MenloUtility::UInt16ToHexBuffer(value, buf);
        buf[4] = '\0';
    }

    return 0;
}

int
WeatherStationApp::Light2(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) {

        // Parameter is supplied by the caller, could be shorter than required
        if (size < 4) {
            return DWEET_PARAMETER_TO_SHORT;
        }

        value = MenloUtility::HexToUShort(buf);

        m_hardware->Stat2LightState(&value, isSet);
    }
    else {

        m_hardware->Stat2LightState(&value, false);

        // 4 hex chars without '\0'
        MenloUtility::UInt16ToHexBuffer(value, buf);
        buf[4] = '\0';
    }

    return 0;
}

int
WeatherStationApp::WindSpeed(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetWindSpeed();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::WindDirection(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetWindDirection();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Temperature(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetTemperatureF();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Barometer(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetPressure();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Humidity(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetHumidity();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Rain(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetRain();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Gps(char* buf, int size, bool isSet)
{
    int value;
    WeatherStationGpsData data;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetGps(&data);

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Battery(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetBattery();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Solar(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetSolar();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::Light(char* buf, int size, bool isSet)
{
    int value;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    //
    // Size is validated by the common dispatcher to the size
    // set in the configuration table.
    //
    value = m_hardware->GetLight();

    // 4 hex chars without '\0'
    MenloUtility::UInt16ToHexBuffer(value, buf);
    buf[4] = '\0';

    return 0;
}

int
WeatherStationApp::SendReadings(char* buf, int size, bool isSet)
{
    WeatherStationReadings readings;

    if (isSet) return DWEET_SET_NOTSUPPORTED;

    SendSensorsAsNMEA();

    //ReadSensors(&readings);

    //PrintSensors(&readings);

    return 0;
}

int
WeatherStationApp::ReadSensors(WeatherStationReadings* readings)
{
    bool boolValue;

    m_hardware->GpsPower(&boolValue, false);
    readings->GpsPower = boolValue;

    readings->WindSpeed = m_hardware->GetWindSpeed();
    readings->WindDirection = m_hardware->GetWindDirection();
    readings->Temperature = m_hardware->GetTemperatureF();
    readings->Barometer = m_hardware->GetPressure();
    readings->Humidity = m_hardware->GetHumidity();
    readings->RainFall = m_hardware->GetRain();
    readings->Battery = m_hardware->GetBattery();
    readings->Solar = m_hardware->GetSolar();
    readings->Light = m_hardware->GetLight();

    return 0;
}

int
WeatherStationApp::PrintSensors(WeatherStationReadings* readings)
{
#if DBG_PRINT_ENABLED

    //
    // These are sent as NMEA 0183 format by default.
    //

    MenloDebug::PrintNoNewline(F("WindSpeed "));
    MenloDebug::PrintHex(readings->WindSpeed);

    MenloDebug::PrintNoNewline(F("WindDir "));
    MenloDebug::PrintHex(readings->WindDirection);

    MenloDebug::PrintNoNewline(F("Temp "));
    MenloDebug::PrintHex(readings->Temperature);

    MenloDebug::PrintNoNewline(F("Barometer "));
    MenloDebug::PrintHex(readings->Barometer);

    MenloDebug::PrintNoNewline(F("Humidity "));
    MenloDebug::PrintHex(readings->Humidity);

    MenloDebug::PrintNoNewline(F("RainFall "));
    MenloDebug::PrintHex(readings->RainFall);

    MenloDebug::PrintNoNewline(F("Battery "));
    MenloDebug::PrintHex(readings->Battery);

    MenloDebug::PrintNoNewline(F("Solar "));
    MenloDebug::PrintHex(readings->Solar);

    MenloDebug::PrintNoNewline(F("Light "));
    MenloDebug::PrintHex(readings->Light);

    MenloDebug::PrintNoNewline(F("GpsPower "));
    MenloDebug::PrintHex(readings->GpsPower);
#endif

    return 0;
}

int
WeatherStationApp::SendSensorsAsNMEA()
{
    WeatherStationReadings readings;

    //
    // Send current sensor readings out the channel
    //
    ReadSensors(&readings);

    //PrintSensors(readings);
    
    SendWindAsNMEA(&readings);

    SendMeteorologicalAsNMEA(&readings);

    // Handle GPS which may be optionally powered on
    SendGpsAsNMEA(&readings);

    return 0;
}

//
// value is an integer scaled by 100.
//
// Convert it to x.xx, xx.xx and add it to the current NMEA 0183 sentence.
//
void
WeatherStationApp::SendDecimalFractionFromScaledInteger(MenloDweet* dweet, int value)
{
    char* p;
    char buf[32];
    unsigned int digits;
    unsigned int fraction;
    PGM_P decimalPoint = PSTR(".");

    digits = (unsigned int)value / 100;
    fraction = (unsigned int)value % 100;

    //sprintf(buf, "%d", digits);
    p = MenloUtility::UInt16ToDecimalBuffer(digits, buf);

    dweet->SendDweetCommandPart(p);

    dweet->SendDweetCommandPart_P(decimalPoint);

    //sprintf(buf, "%d", fraction);
    p = MenloUtility::UInt16ToDecimalBuffer(fraction, buf);

    dweet->SendDweetCommandPart(p);
}

//
// Send $WIMWV Wind Speed and Angle.
//
int
WeatherStationApp::SendWindAsNMEA(WeatherStationReadings* readings)
{
    char buf[32];
    char* p;

    if (m_dweet == NULL) {
        // Nothing to do
        return 0;
    }

    //
    // http://fort21.ru/download/NMEAdescription.pdf
    //
    // Windspeed and Angle
    // Example: $WIWMV,270,T,3.5,M,A*00
    //
    // MWV Wind Speed and Angle
    //          1 2 3 4 5
    //          | | | | |
    // $--MWV,x.x,a,x.x,a*hh
    // 1) Wind Angle, 0 to 360 degrees
    // 2) Reference, R = Relative, T = True
    // 3) Wind Speed
    // 4) Wind Speed Units, K/M/N
    // 5) Status, A = Data Valid
    // 6) Checksum
    //

    // This adds the ',' after the prefix to start the command
    m_dweet->SendDweetCommandStart("$WIMWV");
    
    //
    // sprintf costs 1536 bytes of flash, 3 bytes of RAM
    //
    // Before 29,020 flash, 1598 RAM
    // After  30,566 flash  1602 RAM
    //
    // MenloUtility::UInt16ToDecimalBuffer
    //
    // Before 32,156 flash, 1610 RAM
    // After  30,780 flash, 1608 RAM
    //    
    // Savings 1376 bytes of flash, 2 bytes of RAM
    //    

    //
    // Add WindAngle
    //

    // WindDirection is an integer from 0 - 360
    //sprintf(buf, "%d", readings->WindDirection);
    p = MenloUtility::UInt16ToDecimalBuffer(readings->WindDirection, buf);

    m_dweet->SendDweetCommandPart(p);

    //
    // It appears most instruments send the .0 always even if
    // there is not fraction. Since the x.x format is specified in the
    // NMEA 0183 documentation most listeners are likely to expect it.
    //
    m_dweet->SendDweetCommandPart_P(PSTR(".0"));

    // end value, set T for true wind direction, and start new value
    m_dweet->SendDweetCommandPart_P(PSTR(",T,"));

    //
    // Add WindSpeed
    //

    SendDecimalFractionFromScaledInteger(m_dweet, readings->WindSpeed);

    // end value, set M for units in MPH and set data valid status 'A'
    m_dweet->SendDweetCommandPart_P(PSTR(",M,A"));

    // Send the NMEA 0183 sentence which will calculate the checksum
    m_dweet->SendDweetCommandComplete();

    return 0;
}

//
// Send NMEA 0183 XDR Transducer information sentence.
//
void
WeatherStationApp::SendXDRValue(
    MenloDweet* dweet,
    PGM_P sensorType,
    PGM_P sensorName,
    PGM_P sensorUnits,
    int   sensorValue
    )
{
    //
    // http://www.catb.org/gpsd/NMEA.html#_xdr_transducer_measurement
    //
    // XDR - Transducer Measurement
    //
    //         1 2   3 4            n
    //         | |   | |            |
    //  $WIXDR,a,x.x,a,c--c, ..... *hh<CR><LF>
    //
    // Field Number:
    // 
    // 1 Transducer Type
    // 
    // 2 Measurement Data
    // 
    // 3 Units of measurement
    // 
    // 4 Name of transducer
    // 
    // n Checksum
    // 
    // May be any number of the sequence here up to line length
    // 

    // This adds the ',' after the prefix to start the command
    dweet->SendDweetCommandStart("$WIXDR");

    dweet->SendDweetCommandPart_P(sensorType);
    dweet->SendDweetCommandSeparator();

    SendDecimalFractionFromScaledInteger(dweet, sensorValue);
    dweet->SendDweetCommandSeparator();

    dweet->SendDweetCommandPart_P(sensorUnits);
    dweet->SendDweetCommandSeparator();

    dweet->SendDweetCommandPart_P(sensorName);

    // Send the NMEA 0183 sentence which will calculate the checksum
    m_dweet->SendDweetCommandComplete();
}

//
// Values to send as XDR transducer messages
//
// Temperature in F
// Barometer
// Humidity
// RainFall
//
// Other:
//   Battery
//   Solar
//   Light
//
//   GPS
//     Gps Data
//     GpsPower
//
int
WeatherStationApp::SendMeteorologicalAsNMEA(WeatherStationReadings* readings)
{
    if (m_dweet == NULL) {
        // Nothing to do
        return 0;
    }

    //
    // Note: MDA - Meteorological Composite is now considered obsolete
    // and replaced by $WIMWV for wind and $--XDR for general transducers.
    //
    // http://www.nmea.org/Assets/100108_nmea_0183_sentences_not_recommended_for_new_designs.pdf
    //

    //
    // Documentation for some unit values are at:
    //
    // https://github.com/ktuukkan/marine-api/blob/master/src/main/java/
    // net/sf/marineapi/nmea/util/Units.java
    //
    // TODO:
    //
    // Conduct further research into the proper codes for
    // units and sensor types.
    //

    //
    // TemperatureF
    //
    SendXDRValue(
        m_dweet,
        PSTR("T"),           // Temperature
        PSTR("TEMPERATURE"), // Name
        PSTR("F"),           // F
        readings->Temperature
        );

    //
    // Barometer
    //
    SendXDRValue(
        m_dweet,
        PSTR("P"),          // Pressure
        PSTR("BAROMETER"),  // Name
        PSTR("K"),          // KiloPascals
        readings->Barometer
        );

    //
    // Humidity
    //
    SendXDRValue(
        m_dweet,
        PSTR("H"),         // Humidity
        PSTR("HUMIDITY"),  // Name
        PSTR("P"),         // Percent
        readings->Humidity
        );

    //
    // RainFall
    //
    SendXDRValue(
        m_dweet,
        PSTR("R"),         // Rain
        PSTR("RAIN"),      // Name
        PSTR("I"),         // Inches
        readings->RainFall
        );

    //
    // Battery
    //
    SendXDRValue(
        m_dweet,
        PSTR("B"),       // Battery
        PSTR("BATTERY"), // Name
        PSTR("V"),       // Voltage
        readings->Battery
        );

    //
    // Solar
    //
    SendXDRValue(
        m_dweet,
        PSTR("S"),       // Solar
        PSTR("SOLAR"),   // Name
        PSTR("V"),       // Voltage
        readings->Solar
        );

    //
    // Light
    //
    SendXDRValue(
        m_dweet,
        PSTR("L"),       // Light
        PSTR("LIGHT"),   // Name
        PSTR("R"),       // Raw
        readings->Light
        );

    return 0;
}

int
WeatherStationApp::SendGpsAsNMEA(WeatherStationReadings* readings)
{
#if GPS_NMEA_GENERATION
    bool value;
    WeatherStationGpsData data;

    if (m_dweet == NULL) {
        // Nothing to do
        return 0;
    }

    //
    // See if the GPS power is on. If no GPS power,
    // don't send anything.
    //
    if (!readings->GpsPower) {
        // Nothing to do
        return 0;
    }

    //
    // On AtMega 328's only GPS passthrough mode is currently
    // supported to save code space.
    //
    // It can be enabled for other processors with more memory
    // available (ARM, ArduinoMega, etc.).
    //
    // It may also be enabled on AtMega 328's for applications
    // where there is space, such as disabling other options.
    //


    //m_hardware->GetGps(&data);
#endif

    return 0;
}
