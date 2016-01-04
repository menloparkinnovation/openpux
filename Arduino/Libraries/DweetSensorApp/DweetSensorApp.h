
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
// Outline:
//
// By default a Sensor has an optional indication LED which
// can be single, or multi-color.
//
// It is configurable over NMEA 0183 using the Dweet protocol.
//
// The Sensor is also capable of generating and responding to
// other NMEA 0183 commands.
//
// It has a configurable update interval used to send updates
// over the radio, or serial streams.
//

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
#define APP_STORAGE_BASE_INDEX TOP_INDEX

// begining of sensor block that is checksumed
#define SENSOR_LIGHT_COLOR        APP_STORAGE_BASE_INDEX
#define SENSOR_LIGHT_COLOR_SIZE    9 // 00.00.00

#define SENSOR_LIGHT_ONLEVEL       (SENSOR_LIGHT_COLOR + SENSOR_LIGHT_COLOR_SIZE)
#define SENSOR_LIGHT_ONLEVEL_SIZE  5 // 0000

#define SENSOR_SENSORRATE (SENSOR_LIGHT_ONLEVEL + SENSOR_LIGHT_ONLEVEL_SIZE)
#define SENSOR_SENSORRATE_SIZE  5
// End of sensor block that is checksumed

// Note: This must be last as the checksum range depends on it
#define SENSOR_CHECKSUM   (SENSOR_SENSORRATE + SENSOR_SENSORRATE_SIZE)
#define SENSOR_CHECKSUM_SIZE 2

// The first saved value is the start of the LIGHT checksum range
#define SENSOR_CHECKSUM_BEGIN SENSOR_LIGHT_COLOR

// End of range for LIGHT checksum is the start of the checksum storage location
#define SENSOR_CHECKSUM_END  SENSOR_CHECKSUM

#define SENSOR_MAX_SIZE SENSOR_LIGHT_COLOR_SIZE

struct SensorsData {
    int lightIntensity;
    int battery;
    int solar;
    int moisture;
    int temperature;
};

struct SensorsConfiguration {
  int sensorPower;           // Power on sensors
  int lightIntensity;        // light intensity reading
  int battery;               // battery charge state
  int solar;                 // solar cell voltage
  int moisture;              // moisture sensor
  int temperature;           // temperature sensor

  int lightPin;
  int redPin;
  int greenPin;
  int bluePin;
};

//
// The SensorApp also can provide low overhead
// 32 byte radio packet updates.
//
// The basic packet definition comes from MenloRadioSerial
// so the radio channel can be shared with general management
// commands (Dweets).
//

//
// This is an overlay for MenloRadioSerialSensorData which
// is a general use application level radio packet form
//
struct SensorDataPacket {

  // 2 bytes
  uint8_t  type;
  uint8_t  flags; // a bit for each value present

  //
  // Multibyte types are stored LSB first as on the AtMega series.
  // Other big endian processors need to reverse the bytes.
  //
  // (These have more code space to accomodate)
  //

  // 16 bytes
  uint16_t light;
  uint16_t battery;
  uint16_t solar;
  uint16_t moisture;
  uint16_t temperature;
  uint16_t windspeed;
  uint16_t winddirection;
  uint16_t barometer;

  // 14 bytes
  uint16_t rainfall;
  uint16_t humidity;
  uint16_t fog;
  uint16_t seastate;
  uint16_t radiomonitor;
  uint16_t batteryCurrent;
  uint16_t solarCurrent;
};

#define SENSOR_DATA_LIGHT       0x01
#define SENSOR_DATA_BATTERY     0x02
#define SENSOR_DATA_SOLAR       0x04
#define SENSOR_DATA_MOISTURE    0x08
#define SENSOR_DATA_TEMPERATURE 0x10
#define SENSOR_DATA_WIND        0x20
#define SENSOR_DATA_BAROMETER   0x40
#define SENSOR_DATA_HUMIDITY    0x80

#define SENSOR_DATA_SIZE sizeof(struct SensorDataPacket)

#define SENSOR_DATA_PACKET   MENLO_RADIO_SERIAL_SENSORDATA
#define SENSOR_DATA_OVEHEAD  MENLO_RADIO_SERIAL_SENSORDATA_OVERHEAD

//
// The application provides Dweet handling contracts to be part
// of the main programs Dweet dispatch loop.
//
class DweetSensorApp : public DweetApp  {

public:

    DweetSensorApp();

    int Initialize(SensorsConfiguration* sensors);

    void SetRadio(MenloRadio* menloRadio);

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // GET/SET property commands
    //

    int LightColor(char* buf, int size, bool isSet);

    int LightOnLevel(char* buf, int size, bool isSet);

    int SensorUpdateRate(char* buf, int size, bool isSet);

    int ProcessSensors(char* buf, int size, bool isSet);

    //
    // Get sensor readings
    //
    bool readSensors(SensorsData* sensors);

protected:

private:

    // Load settings from EEPROM
    void InitializeStateFromStoredConfig();

    // Set light on state in the hardware
    void SetLightOnState();

    // Control RGB intensity values
    void SetRGB(bool state);

    // Reset the update timer to the new value
    void SetSensorUpdateRate(uint16_t value);

    // MenloRadio to send sensor updates to
    MenloRadio* m_radio;

    // Light on or off
    bool m_lightState;

    //
    // Level in which light turns on
    // Based on lightIntensity reading level. (analog 10 or 12 bit)
    //
    int m_lightOnLevel;

    //
    // This is the current light level.
    //
    int m_lightLevel;

    //
    // Hardware Configuration
    //

    // True  -> 1 == ON
    // FALSE -> 0 == ON
    bool m_lightPolarity;

    bool m_rgbEnabled;

    // Default RGB intensity values
    int m_redIntensity;
    int m_greenIntensity;
    int m_blueIntensity;

    //
    // Pins for environmental monitoring
    // These pins are analog inputs
    //
    SensorsConfiguration m_sensors;

#if USE_DS18S20
    //
    // Dallas OneWire Temperature Sensor
    //
    OS_DS18S20 m_temperatureSensor;
#endif

    //
    // MenloTimer is used to schedule periodic environmental
    // readings.
    //
    // This is also used to send sensor updates if configured.
    //
    unsigned long m_monitorInterval;

    // Timer and event registration
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    // If set sensor updates are sent
    bool m_sendSensorUpdates;

    // Indicates when we can start async processing
    bool m_initialized;
};

#endif // DweetSensorApp_h
