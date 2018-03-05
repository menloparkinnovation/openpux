
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
 *  File: LightHouseApp.h
 *
 *  Lighthouse application state
 *
 *   Refactored from example app as a separate module to support
 *   multiple Dweet channels.
 */

#ifndef LightHouseApp_h
#define LightHouseApp_h

//
// Applications non-volatile storage allocations
//
#include "LightHouseConfig.h"
#include "LightHouseHardwareBase.h"
#include "MenloRadioSerial.h"

//
// A LightHouse is autonomous but can report its current conditions
// and accept management commands.
//
// If it resets, it will continue processing based on its last saved
// permanent configuration.
//

#define LIGHTHOUSE_SEQUENCE_BUFFER_SIZE 32

//
// Default environmental monitoring interval
//
#define LIGHTHOUSEAPP_DEFAULT_INTERVAL (30L * 1000L) // 30 seconds

//
// The LightHouseApp also can provide low overhead
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
struct LightHouseSensorData {

  //
  // This header follows SensorPacket in MenloRadio.h
  //

  // 2 bytes
  uint8_t  type;
  uint8_t  flags;       // High 4 bits application. Low 4 bits define state.

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

#define LIGHTHOUSE_SENSOR_DATA_SIZE sizeof(struct LightHouseSensorData)

//
// See MenloRadio.h for application type MENLO_RADIO_APPLICATION_LIGHT_HOUSE.
//

//
// The application provides Dweet handling contracts to be part
// of the main programs Dweet dispatch loop.
//
class LightHouseApp : MenloObject {

public:

    LightHouseApp() {
        m_initialized = false;
        m_lightLevel = 0; // default is darkness
        m_lightOnLevel = (-1); // default is always on.
        m_sendSensorUpdates = false;
    }

    int
    Initialize(
       LightHouseHardwareBase* hardware
       );

    void SetRadio(MenloRadio* menloRadio);

    //
    // Initialize the application/hardware state from the stored
    // configuration.
    //
    void InitializeStateFromStoredConfig();

    //
    // Light Sequence
    //

    bool GetLightSequence(char* buffer, int length, bool persistent);

    //
    // Set the application state from the string.
    //
    // If persisent is true, save the setting to the
    // EEPROM configuration store for loading at power on/reset.
    //

    bool SetLightSequence(char* str, bool persistent);

    //
    // Get/Set application state properties
    //
    // These mostly pass through to the worker class MenloLightHouse
    //

    void LightTick(unsigned long* tickPeriod, bool isSet) {
        m_lightHouse.LightTick(tickPeriod, isSet);
    }

    void LightPeriod(unsigned long* lightPeriod, bool isSet) {
        m_lightHouse.LightPeriod(lightPeriod, isSet);
    }

    void RampPeriod(uint16_t* rampUpPeriod, uint16_t* rampDownPeriod, bool isSet) {
        m_lightHouse.RampPeriod(rampUpPeriod, rampDownPeriod, isSet);
    }

    void LightColor(uint8_t* red, uint8_t* green, uint8_t* blue, bool isSet) {
        m_lightHouse.LightColor(red, green, blue, isSet);
    }

    void LightOnLevel(uint16_t* onLevel, bool isSet) {
        m_lightHouse.LightOnLevel(onLevel, isSet);
    }

    //
    // Get sensor readings
    //
    bool readSensors(LightHouseSensors* sensors) {
        return m_hardware->readSensors(sensors);
    }

    //
    // If set radio updates will occur at the periodic interval
    //
    // The value is in seconds.
    //
    void SensorUpdateRate(uint16_t* value, bool isSet);

protected:

    //
    // Set our internal values
    //
    void SetLightOnLevel(uint16_t onLevel) {
        m_lightOnLevel = onLevel;
    }

private:

    // Application hardware state
    LightHouseHardwareBase* m_hardware;

    // MenloRadio*, since every lighthouse needs a radio
    MenloRadio* m_radio;

    //
    // Level in which light turns on
    // Based on lightIntensity reading level. (analog 10 or 12 bit)
    //
    int m_lightOnLevel;

    //
    // This is the current light level.
    //
    int m_lightLevel;

    // MenloLightHouse provides light timing sequences
    MenloLightHouse  m_lightHouse;

    // Lighthouse Sequence buffer
    uint8_t m_lightSequence[LIGHTHOUSE_SEQUENCE_BUFFER_SIZE];

    // MenloLightHouse object emits events for light state changes
    MenloLightHouseEventRegistration m_lightStateEvent;

    // LightStateEvent function
    unsigned long LightStateEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

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

#endif // LightHouseApp_h
