
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
 *  File: LightHouseApp.cpp
 *
 *  Lighthouse application state
 *
 *   Refactored from example app as a separate module to support
 *   multiple Dweet channels.
 */

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//
#include <MenloPlatform.h>
#include <MenloUtility.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloPower.h>
#include <MenloTimer.h>

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>

// Lighthouse support
#include <MenloLightHouse.h>

#include "LightHouseHardwareBase.h"
#include "LightHouseApp.h"

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

void
LightHouseApp::SetRadio(
    MenloRadio* radio
    )
{
    // This could be NULL, which means no radio
    m_radio = radio;
}

int
LightHouseApp::Initialize(
    LightHouseHardwareBase* hardware
    )
{
    int result = 1;

    //
    // Initialize our application state
    //
    m_hardware = hardware;

    m_radio = NULL;

    //
    // Initialize lighthouse worker which is the event generator
    // for the sequence
    //
    m_lightHouse.Initialize();

    // Setup to receive LightHouse events
    m_lightStateEvent.object = this;

    m_lightStateEvent.method = (MenloEventMethod)&LightHouseApp::LightStateEvent;

    //
    // Setup the environmental monitoring timer
    //

    //
    // It's critical that this is not zero or you will have a timer loop
    // which will cause a watchdog reset
    //
    m_monitorInterval = LIGHTHOUSEAPP_DEFAULT_INTERVAL;

    // Standard args
    m_timerEvent.object = this;

    m_timerEvent.method = (MenloEventMethod)&LightHouseApp::TimerEvent;

    // Object type specific args
    m_timerEvent.m_interval = m_monitorInterval;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    //
    // Read the EEPROM for power on/reset state
    // settings.
    //
    InitializeStateFromStoredConfig();

    //
    // Now register our events now we are configured and ready
    // to receive them.
    //

    // MenloLightHouse fires events when the light state changes
    m_lightHouse.RegisterLightStateEvent(&m_lightStateEvent);

    // Timer
    if (m_timerEvent.m_interval != 0) {
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }

    // Indicate we are initialized and can perform async processing
    m_initialized = true;

    return result;
}

//
// Initialize the application/hardware state from the stored
// configuration.
//
void
LightHouseApp::InitializeStateFromStoredConfig()
{
    int bufferLength;

    // This is the largest of all buffers required
    char buffer[LIGHT_MAX_SIZE + 1];

    bufferLength = LIGHT_MAX_SIZE + 1;

    //
    // Set power on light sequence from the configuration store
    //

    if (GetLightSequence(buffer, bufferLength, true)) {
        SetLightSequence(buffer, false);
    }

    //
    // The rest of the settings are handled by DweetLightHouse.cpp
    // from the table driven configuration.
    //

    return;
}

bool
LightHouseApp::GetLightSequence(char* buffer, int length, bool persistent)
{
    int bufferLength;

    if (length < LIGHT_SEQUENCE_SIZE + 1) {
        DBG_PRINT("GetLightSequence buffer to small");
        return false;
    }

    buffer[0] = '\0';

    bufferLength = LIGHT_SEQUENCE_SIZE;

    if (persistent) {
        ConfigStore.ReadConfig(LIGHT_SEQUENCE, (uint8_t*)&buffer[0], bufferLength);
        buffer[bufferLength] = '\0';
    }
    else {

        //
        // TODO: if(!persistent) should return the runtime state
        //
        // These are being deferred for code space on AtMega328 as its
        // optional to a working application.
        //
    }

    return true;
}

//
// Set the light sequence buffer from the character string
// of hex digits.
//
bool
LightHouseApp::SetLightSequence(char* strArg, bool persistent)
{
    unsigned int index;
    uint8_t bitCount;
    char* str = strArg;
    int size = strlen(str);

    // Must be at least "00:0"
    if (size < 4) return false;

    bitCount = MenloUtility::HexToByte(str);
    
    // If bitCount is zero its a conversion error or an invalid length
    if (bitCount == 0) {
        DBG_PRINT("SetLightSequence 0 bitcount");
        return false;
    }

    str += 2; // consumes two characters

    if (*str != ':') {
        DBG_PRINT("SetLightSequence no :");
        return false;
    }
    
    // skip separator ':'
    str++;

    // Initialize light sequence buffer
    for (index = 0; index < sizeof(m_lightSequence); index++) {
        m_lightSequence[index] = 0;
    }

    // Decode ASCII hex into binary bytes
    index = 0;

    while (*str != '\0') {

        m_lightSequence[index] = MenloUtility::HexToByte(str);

        index++;
        if (index > sizeof(m_lightSequence)) {
	  break;
        }

        // Consumes two chars
        str++;
        if (*str == '\0') break; // actually an error, should be even # of chars
        str++;
    }

    //
    // Set the sequence on the MenloLightHouse which is the event
    // generator to create the proper sequence.
    //
    m_lightHouse.SetSequenceBuffer(&m_lightSequence[0], bitCount);

    DBG_PRINT("SetLightSequence NP");

    if (persistent) {

        DBG_PRINT("SetLightSequence P");

        // We store the string's '\0' terminator in the config store
        if ((size + 1) > LIGHT_SEQUENCE_SIZE) {
            size = LIGHT_SEQUENCE_SIZE - 1;
            // Ensure NULL
            strArg[size - 1] = '\0';
        }

        //
        // LightHouseConfig.h
        //
        ConfigStore.WriteConfig(LIGHT_SEQUENCE, (uint8_t*)strArg, size + 1);

        ConfigStore.CalculateAndStoreCheckSumRange(
            LIGHT_CHECKSUM,
            LIGHT_CHECKSUM_BEGIN,
            LIGHT_CHECKSUM_END - LIGHT_CHECKSUM_BEGIN
            );
    }

    return true;
}

unsigned long
LightHouseApp::LightStateEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
  unsigned long pollInterval = MAX_POLL_TIME;
  MenloLightHouseEventArgs* lightArgs = (MenloLightHouseEventArgs*)eventArgs;

  DISPATCH_PRINT2("LightHouseApp::LightStateEvent lightState=", lightArgs->lightState);

  //
  // If the light on level is greater than current ambient light
  // we do not enable the light
  //
  // Values 0x0 and (-1) are always enabled due to either no
  // sensor, or not being initialized in the EEPROM.
  //
  if ((m_lightOnLevel == (-1)) ||
      (m_lightOnLevel == 0) ||
      (m_lightLevel < m_lightOnLevel)) {

      // Invoke the hardware to change the light state
      m_hardware->setLightState(lightArgs);
  }
  else {
      // This ensures the light does not stay stuck on
      MenloLightHouseEventArgs tmp;
      tmp.lightState = false;
      m_hardware->setLightState(&tmp);
  }

  return pollInterval;
}

unsigned long
LightHouseApp::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    LightHouseSensors sensors;
    LightHouseSensorData data;

    xDBG_PRINT("LightHouseApp TimerEvent");

    memset(&sensors, 0, sizeof(sensors));
    memset(&data, 0, sizeof(data));

    // Check the environmental monitor hardware
    if (m_hardware->readSensors(&sensors)) {

        //
        // We expect "sane" readings from the hardware support
        // even if something is disconnected or optional.
        //

        // valid reading(s)
        m_lightLevel = sensors.lightIntensity;

        if (m_sendSensorUpdates) {

	    xDBG_PRINT("Sending radio sensor update");

    	    //
	    // Queue a radio sensor update packet if configured
	    //

	    data.type = MENLO_RADIO_SENSOR_DATA;
            data.flags = MENLO_RADIO_APPLICATION_LIGHT_HOUSE;

	    data.light = sensors.lightIntensity;
	    data.battery = sensors.battery;
	    data.solar = sensors.solar;
	    data.moisture = sensors.moisture;
	    data.temperature = sensors.temperature;

            //
            // Additional WeatherStation data
            //
            data.windspeed = sensors.windspeed;
            data.winddirection = sensors.winddirection;
            data.barometer = sensors.barometer;
            data.humidity = sensors.humidity;
            data.rainfall = sensors.rainfall;

            if (m_radio != NULL) {

                xDBG_PRINT("Writing to radio");

	        m_radio->Write(
   		    NULL, // Use the configured address
                    (uint8_t*)&data,
                    (uint8_t)sizeof(data),
                    250  // 250 ms
                    );
            }
        }

        //
        // Queue A Dweet update if configured:
        //
        // TODO:
        //
        // NOTE: This is on both radio serial or serial interfaces.
        // The incoming request works from either transport.
        // But we need configuration of where to send the periodic
        // sensor updates.
        //
        // DweetLightHouse::ProcessGetSensors(NULL, "SENSORS")
        //
        // Could make this radio packet only for updates, but
        // that would preclude USB connected sensor gateway
        // applications.
        //
    }

    return MAX_POLL_TIME;
}

void
LightHouseApp::SensorUpdateRate(uint16_t* value, bool isSet)
{
    if (!isSet) {
        *value = m_monitorInterval / 1000L;
        return;
    }

    //
    // If a sensor update rate is specified other
    // than zero sensor updates are scheduled at
    // the specified rate in seconds.
    //
    // The sensor update handler also performs local
    // timer based autonomous functions such as
    // day/night handling of the light sequence.
    //
    // If an update rate value of 0 is specified, no
    // sensor updates are sent.
    //
    // The timer event is re-registered to the default
    // rate so that local autonomous processing may
    // continue at the default.
    //
    // Improve: Could allow the default rate to be stored
    // in the config store and not locked into 30 seconds.
    //
    // Improve: The side effect of setting longer environmental
    // update times increase the response time to day/night
    // configuration.
    //
    if (*value != 0) {
        m_monitorInterval = (*value * 1000L);
        m_sendSensorUpdates = true;
    }
    else {

        //
        // set m_monitorInterval back to default as its
        // used for autonomous light on, light off handing.
        //
        m_monitorInterval = LIGHTHOUSEAPP_DEFAULT_INTERVAL;
        m_sendSensorUpdates = false;
    }

    //
    // If not initialized timer is not set and we are being
    // set based on EEPROM configuraiton.
    //
    // So update our interval, but don't attempt to touch
    // the timer. When Initialize() finally starts the timer
    // it will have the correct updated interval.
    //

    if (m_initialized) {

        // Unregister our current timer
        xDBG_PRINT("SENSORRATE Unregistering timer");

        if (m_timer.IsTimerRegistered(&m_timerEvent)) {
            m_timer.UnregisterIntervalTimer(&m_timerEvent);
        }
    }

    // Update interval
    m_timerEvent.m_interval = m_monitorInterval;

    if (m_initialized && (m_timerEvent.m_interval != 0)) {
        // Re-register timer
        xDBG_PRINT("SENSORRATE Re-registering timer");
        m_timer.RegisterIntervalTimer(&m_timerEvent);
    }
}
