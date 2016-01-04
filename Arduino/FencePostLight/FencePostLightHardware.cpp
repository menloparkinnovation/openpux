
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
 *  Date: 02/10/2015
 *  File: FencePostLigthHardware.cpp
 *
 * Hardware handling for an application.
 *
 * Used for Smartpux DWEET's.
 *
 */

//
// Arduino libraries
//

//
// MenloFramework
//
// Note: All these includes are required together due
// to Arduino #include behavior.
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>

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
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define xDBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_HEX_STRING(x, l)
#define xDBG_PRINT_HEX_STRING_NNL(x, l)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

//
// Debug Strategy:
//
// This application is pretty full on an AtMega 328 (Arduino UNO and clones)
// taking up 32k. There is a few hundred bytes available for small DBG_PRINT()
// messages which are selectively enable with "x" such as xDBG_PRINT.
//
// When a code path has been verified, remove the "x" to leave the
// DBG_PRINT() as a marker for future debug tracing.
//

//
// Enable use of Dallas DS18S20 OneWire temperature sensor.
//
// Careful when you enable this. It can cause hardware lock ups,
// code size growth of 2114 bytes, and at least 2 seconds spinning
// around waiting for a reading and for the thing to settle at
// every sample interval.
// 
#if MENLO_ATMEGA
//
// TODO: Only works on AVR right now due to OneWire library using low level AVR I/O
// Dallas Digital 1 wire temperature sensor
//
//#include <OS_DS18S20.h>
//#define USE_DS18S20 1
#endif

#define USE_DS18S20 0

// Libraries/DweetLightHouse/LightHouseHardwareBase.h
#include <LightHouseHardwareBase.h>
#include "FencePostLightHardware.h"

FencePostLightHardware::FencePostLightHardware ()
{
        m_lightState = false;
        m_lightPolarity = true;
        m_rgbEnabled = false;

        m_whitePin = -1;
        m_redPin = -1;
        m_greenPin = -1;
        m_bluePin = -1;

        m_redIntensity = 0xFF;
        m_greenIntensity = 0xFF;
        m_blueIntensity = 0xFF;

        // m_sensors are initialized during Initialize()
}

int
FencePostLightHardware::Initialize(
    LightHouseLight* light,
    LightHouseSensors* sensors
    )
{
    m_whitePin = light->whitePin;

    m_redPin = light->redPin;
    m_greenPin = light->greenPin;
    m_bluePin = light->bluePin;

    // Setup our application hardware pins
    if (m_whitePin != (-1)) {
        pinMode(m_whitePin, OUTPUT);
    }

    if (m_redPin != (-1)) {
        pinMode(m_redPin, OUTPUT);

        // Set test short cut
        m_rgbEnabled = true;
    }

    if (m_greenPin != (-1)) {
        pinMode(m_greenPin, OUTPUT);
        m_rgbEnabled = true;
    }

    if (m_bluePin != (-1)) {
        pinMode(m_bluePin, OUTPUT);
        m_rgbEnabled = true;
    }

    //
    // Setup environmental sensors
    //
    InitializeSensors(sensors);

    //
    // NMEA 0183 streaming sensors must be deferred until
    // the MenloDweet* channel is registered by the application
    // later in initialization.
    //

    //
    // Note: The Dweet Application is responsible for getting
    // any configuration from the MenloConfigStore (EEPROM)
    // and setting states of the hardware.
    //

    return 0;
}

void
FencePostLightHardware::SetDweet(MenloDweet* dweet)
{
    m_dweet = dweet;
}

bool
FencePostLightHardware::InitializeSensors(LightHouseSensors* sensors)
{
    //
    // struct copy to local field as this is a stack
    // structure to the caller.
    //
    m_sensors = *sensors;

    if (m_sensors.sensorPower != (-1)) {
        pinMode(m_sensors.sensorPower, OUTPUT);
        digitalWrite(m_sensors.sensorPower, 0);
    }

#if USE_DS18S20
    if (m_sensors.temperature != (-1)) {
        m_temperatureSensor.Initialize(m_sensors.temperature);
    }
#endif

    //
    // These pins are analog input which by default need
    // no configuration.
    //
    // m_sensors.lightIntensity
    // m_sensors.battery
    // m_sensors.solar
    // m_sensors.moisture
    // m_sensors.temperature
    //

    return true;
}

//
// Read as many sensors as are configured
//
// One function saves code space on small micros such
// as the Atmega328.
//
bool
FencePostLightHardware::readSensors(LightHouseSensors* sensors)
{
     uint16_t tmp;

    // Power on if required
    if (m_sensors.sensorPower != (-1)) {
        digitalWrite(m_sensors.sensorPower, 1);
        delay(10); // wait for settling
    }

    // m_sensors contains the configured pin numbers.

    // sensors* is the output for the current readings.

    //
    // To test sensor hardware send the following Dweet:
    //
    //                        lite bat  slr  mois temp
    // GETSTATE_REPLY=SENSORS:0000.0000.0000.0000.0000
    // dweet GETSTATE=SENSORS
    //

    //
    // MenloSensor2 returns with 1.2v NiCD battery almost dead
    // and solar cell facing down/blocked.
    //
    // lite bat  slr  mois temp
    // 03B1.015B.016C.0120.0000
    //
    //

    if (m_sensors.lightIntensity != (-1)) {

        //
        //
        // MenloSensor1
        //
        // light:
        //
        // 10 bit A/D. Brighter light is a lower reading.
        //
        // Light is 0x27B (635) dark 0x01B (27) light
        //
        // So we subtract from the A/D range (0-1023) to
        // get a value that increases with light intensity.
        //
        tmp = analogRead(m_sensors.lightIntensity);

        sensors->lightIntensity = 1023 - tmp;
    }
    else {
        sensors->lightIntensity = 0;
    }

    if (m_sensors.battery != (-1)) {

        //
        // MenloSensor1
        //
        // battery:
        //
        // currently read 0x1B3 (435) on a fairly full Alkaline battery
        //
        // (435/1023) == 0.4252 ratio of a 3.3v reference == 1.403 volts
        //
        sensors->battery = analogRead(m_sensors.battery);
    }
    else {
        sensors->battery = 0;
    }

    if (m_sensors.solar != (-1)) {

        //
        // MenloSensor1
        //
        // solar:
        //
        // currently read 0x1B5 (437) on a disconnected solar cell and
        // a fairly full Alkaline battery installed.
        //
        // Even though there is a blocking diode we could be reading
        // back leakage since there is no load.
        //
        sensors->solar = analogRead(m_sensors.solar);
    }
    else {
        sensors->solar = 0;
    }

    if (m_sensors.moisture != (-1)) {

        //
        // MenloSensor1
        //
        // moisture:
        //
        // Currently read 0x015C () while disconnected (open ended)
        //
        sensors->moisture = analogRead(m_sensors.moisture);
    }
    else {
        sensors->moisture = 0;
    }

    if (m_sensors.temperature != (-1)) {

#if USE_DS18S20

        //
        // This sensor has the following issues:
        //
        // Cost: About $3.50
        // Slow: Must be awake for 1-2 seconds bouncing its signal
        //       and waiting to get a reading. Hurts battery life when
        //       could be sleeping.
        //
        // Weird signals: It's use has dropped the Nordic Radio/SPI offline
        // requiring re-initialization, and signal coupling has caused other
        // problems. It must always be on the power rail, though it goes into
        // it own low power sleep. Can't put it on sensor power pin.
        //
        // Code Size Cost: 2114 bytes for OS_DS18S20 library + OneWire support library.
        //            vs. a single analogRead() of a < $1.00 TMP36.
        //
        // 30,094 enabled
        // 27,980 disabled
        // -------
        //  2114 bytes
        //

        //
        // Currently a Dallas DS18B20 One Wire Temperature Sensor
        //
        sensors->temperature = m_temperatureSensor.GetTemperature();
#else
        //
        // A TMP36 is an analog sensor which is cheaper (< $1.00)
        // and does not burn xxx bytes of code space.
        //
        // sensors->temperature = analogRead(m_sensors.temperature);
        sensors->temperature = 0;
#endif
    }
    else {
        sensors->temperature = 0;
    }

    // Power off if required
    if (m_sensors.sensorPower != (-1)) {
        digitalWrite(m_sensors.sensorPower, 0);
    }

    //
    // Record any running meteorological sensor data that has
    // been received over NMEA 0183 from a co-resident weather station.
    //
    sensors->windspeed = m_windspeed;
    sensors->winddirection = m_winddirection;

    //
    // The WeatherStation temperature reading overrides any
    // base lighthouse one.
    //
    sensors->temperature = m_temperature;

    sensors->barometer = m_barometer;
    sensors->humidity = m_humidity;
    sensors->rainfall = m_rain;

    if (m_sensors.lightIntensity == (-1)) {

        //
        // No support for light intensity from the LightHouse
        // hardware so use the received NMEA 0183 value.
        //
        sensors->lightIntensity = m_nmeaLight;
    }
    else {

        //
        // The light sensor on the SparkFun Weathershield is underneath
        // the lantern house where it gets no light. So we can't use
        // the value from the NMEA 0183 stream from the WeatherStation.
        //
        // In this case the lighthouse hardware has a sensor into the
        // lantern house so we will use that value which was read previously.
        //
        // sensors->lightIntensity = m_nmeaLight;
    }

    sensors->battery = m_battery;
    sensors->solar = m_solar;

    return true;
}

bool
FencePostLightHardware::getLightState()
{
  return m_lightState;
}

void
FencePostLightHardware::setLightState(MenloLightHouseEventArgs* args)
{
  bool onState;

  m_lightState = args->lightState;

  //
  // args->rampUpPeriod
  // args->rampDownPeriod
  //

  //
  // Set the light state. Polarity determines whether
  // TRUE == ON (m_polarity == TRUE) or OFF (m_polarity == FALSE)
  //
  if (m_lightState) {
      // ON
      onState = m_lightPolarity;
  }
  else {
      // OFF
      onState = !m_lightPolarity;
  }

  if (m_whitePin != (-1)) {
      digitalWrite(m_whitePin, onState);
  }

  // Handle RGB. Polarity determines whether true == ON or OFF
  if (m_rgbEnabled) {

      m_redIntensity = args->redIntensity;
      m_greenIntensity = args->greenIntensity;
      m_blueIntensity = args->blueIntensity;

      setRGB(onState);
  }

  return;
}

void
FencePostLightHardware::setRGBIntensity(int red, int green, int blue)
{
  m_redIntensity = red;
  m_greenIntensity = green;
  m_blueIntensity = blue;
}

void
FencePostLightHardware::setRGB(bool state)
{
    // Pins are allowed to be individually set
    if (m_redPin != (-1)) {
      if (state) {
          analogWrite(m_redPin, m_redIntensity);
      }
      else {
          analogWrite(m_redPin, 0);
      }
    }

    if (m_bluePin != (-1)) {
      if (state) {
          analogWrite(m_bluePin, m_blueIntensity);
      }
      else {
          analogWrite(m_bluePin, 0);
      }
    }

    if (m_greenPin != (-1)) {
      if (state) {
          analogWrite(m_greenPin, m_greenIntensity);
      }
      else {
          analogWrite(m_greenPin, 0);
      }
    }
}
