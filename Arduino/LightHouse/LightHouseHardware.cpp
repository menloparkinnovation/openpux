
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
 *  File: LightHouseHardware.cpp
 *
 * Hardware handling for an application.
 *
 * Used for Smartpux DWEET's.
 *
 * Note: The header file for this implementation is in
 * Libraries/DweetLightHouse/LightHouseHardware.h
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

//
// This saves 922 bytes and its useful to compile it out
// for debugging as this allows selective tracing to be used.
//
// 04/17/2016
//
// 30,978 without
//
// 31,900 with
//
#define NMEA_WEATHER_SUPPORT 1

// Libraries/DweetLightHouse/LightHouseHardware.h
#include <LightHouseHardwareBase.h>
#include "LightHouseHardware.h"

LightHouseHardware::LightHouseHardware ()
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
LightHouseHardware::Initialize(
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
LightHouseHardware::SetDweet(MenloDweet* dweet)
{
    m_dweet = dweet;

    InitializeNMEA0183Sensors();
}

//
// Register for NMEA 0183 data streams which is used for the
// environmental sensors.
//
void
LightHouseHardware::InitializeNMEA0183Sensors()
{
    //
    // Reset NMEA 0183 values to 0 on initialize
    //
    m_windspeed = 0;
    m_winddirection = 0;
    m_temperature = 0;
    m_barometer = 0;
    m_humidity = 0;
    m_rain = 0;
    m_battery = 0;
    m_solar = 0;
    m_nmeaLight = 0;

#if NMEA_WEATHER_SUPPORT
    //
    // Register for NMEA 0183 messages
    //
    m_nmeaEvent.object = this;
    m_nmeaEvent.method = (MenloEventMethod)&LightHouseHardware::NMEAEvent;

    MenloDweet::RegisterGlobalNMEAMessageEvent(&m_nmeaEvent);
#endif

    return;
}

bool
LightHouseHardware::InitializeSensors(LightHouseSensors* sensors)
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
LightHouseHardware::readSensors(LightHouseSensors* sensors)
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
LightHouseHardware::getLightState()
{
  return m_lightState;
}

void
LightHouseHardware::setLightState(MenloLightHouseEventArgs* args)
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
LightHouseHardware::setRGBIntensity(int red, int green, int blue)
{
  m_redIntensity = red;
  m_greenIntensity = green;
  m_blueIntensity = blue;
}

void
LightHouseHardware::setRGB(bool state)
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

#if NMEA_WEATHER_SUPPORT

//
// Support for receiving environmental data from streaming
// NMEA 0183 sensors.
//

//
// Move to token position
//
// This count represents the occurance of the token
// separator ',' '*', or '\0'.
//
// Position 0 always returns the begining.
//
//     270,T,3.5,M,A*00
//    |   | |   | | |
//    0   1 2   3 4 5
//
//     a,x.x,a,name*hh<CR><LF>
//    | |   | |    |
//    0 1   2 3    4
//
char*
NMEAMoveToToken(char* p, int count)
{
    int tokenCount = 0;

    if (count == 0) {
        // begining of string, no token
        return p;
    }

    // No tokens if the string starts at '\0'
    if (*p == '\0') {
        return NULL;
    }

    //
    //   P,101,K,BAROMETER*00
    //  | |   | |         |
    //  0 1   2 3         4  tokenCount
    //

    while (true) {

        // ',' , '*' or '\0' can end a token
        if ((*p == ',') || (*p == '*') || (*p == '\0')) {
            tokenCount++;
            if (tokenCount == count) {
                return p;
            }
            else if ((*p == '*') || (*p == '\0')) {

                // Did not find token index count
                return NULL;
            }
        }

        p++;
    }

    return NULL;
}

//
// Extract a NMEA 0183 word from a sentence at the given
// position.
//
// Positions are numbered from 1 to n, based on position in the
// NMEA sentence.
//
// The position separator is ",", except for the last entry
// in which its "*".
//
// $WIMWV,270,T,3.5,M,A*00
//         |  | |   | |
//         1  2 3   4 5
//
// The first position (1) does not start with "," as its pointed
// to after the prefix ($WIMWV,)
//
// The sentence is expected to end with '*'
//
// "270,T,3.5,M,A*"
//
// Returns the count placed into the buffer not including the '\0'
//
int
ExtractNMEAWord(char* str, int position, char* outputBuf, int outputBufSize)
{
    int length;
    char* start;
    char* end;
    int tokenCount = 0;

    if (str == NULL) {
        return 0;
    }

    if (*str == '\0') {
        return 0;
    }

    if (position == 0) {
        DBG_PRINT("E1");
        return 0;
    }

    // Find start token
    start = NMEAMoveToToken(str, position - 1);
    if (start == NULL) {
        DBG_PRINT("E2");
        return 0;
    }

    // Move past ',' if not start
    if (position != 1) {
        start++;
    }

    // Find next token
    end = NMEAMoveToToken(str, position);
    if (end == NULL) {
        DBG_PRINT("E3");
        return 0;
    }

    //
    // *start points to first char
    // *end points at ',' or '*'
    //

    length = end - start;
    if ((length + 1) > outputBufSize) {
        DBG_PRINT("E4");
        return 0;
    }

    memcpy(outputBuf, start, length);
    outputBuf[length] = '\0';

    DBG_PRINT_NNL("length ");
    DBG_PRINT_INT(length);

    return length;
}

//
// Handle fractional number such as 0.0 scaled by 100
// to an integer.
//
// str points to a '\0' terminated number
//
bool
HandleScaledBy100FractionalNumber(char* str, unsigned long* output)
{
    char *p;
    unsigned long valInteger;
    unsigned long valFraction = 0;
    char* endptr;
    int length;
    char buf[3];

    p = strchr(str, '.');
    if (p != NULL) {

        //
        // Number can be:
        //
        // .0
        // .5
        // .50
        // .05
        //
        // Digits over 2 are dropped rounding precision to 1/100
        //

        //
        // Set '.', to '\0' and skip over
        //
        // This is set to allow the whole number component to be
        // null terminated for belows conversion.
        //
        *p++ = '\0';

        // Load first digit
        buf[0] = *p++;

        //
        // If second digit is null, we pad with a '0'
        // since we scale by 100 and need the full value.
        //
        // Example: If scale by 100 .5 == 50 (1/2 of 100)
        //
        // .5   => 50
        // .50  => 50
        // .500 => 50
        //
        if (*p == '\0') {
            buf[1] = '0';
        }
        else {
            buf[1] = *p;
        }

        // Ensure null terminator
        buf[2] = '\0';

        // Get the fraction
        endptr = NULL;
        valFraction = strtoul(buf, &endptr, 10);
        if (endptr == str) {
            // conversion failure
            return false;
        }
    }

    //
    // Get the whole component
    //

    endptr = NULL;
    valInteger = strtoul(str, &endptr, 10);
    if (endptr == str) {
        // conversion failure
        return false;
    }

    //
    // Scale the integer portion by 100.
    //
    // The fractional component has already been handled
    // as a two digit 1/100's scale.
    //
    *output = (valInteger * 100) + valFraction;

    return true;
}

unsigned long
LightHouseHardware::NMEAEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    char buf[16];
    char name[16];
    int length;
    char sensorType;
    MenloNMEAMessageEventArgs* nmeaArgs = (MenloNMEAMessageEventArgs*)eventArgs;
    unsigned long val;

    xDBG_PRINT("NMEAEvent Sensors");

    //
    // nmeaArgs->dweet
    // nmeaArgs->prefix
    // nmeaArgs->cmds
    // nmeaArgs->buffer
    //

    //
    // First dispatch on the prefix type
    //
    if (strncmp_P(nmeaArgs->prefix, PSTR("$WIMWV"), 6) == 0)  {

        //
        // WIMWV - Weather Instruments windspeed and direction
        //

        xDBG_PRINT("NMEAEvent WIMWV");

        //
        // http://fort21.ru/download/NMEAdescription.pdf
        //
        // MWV Wind Speed and Angle
        //
        // Example: $WIMWV,270,T,3.5,M,A*00
        //                  |  |  |  | |
        //                  1  2  3  4 5
        //
        // $--MWV,x.x,a,x.x,a*hh
        // 1) Wind Angle, 0 to 360 degrees
        // 2) Reference, R = Relative, T = True
        // 3) Wind Speed
        // 4) Wind Speed Units, K/M/N
        // 5) Status, A = Data Valid
        // 6) Checksum
        //

        // nmeaArgs->cmds points to the start after the prefix $WIMWV,

        // WindAngle 0 - 360
        length = ExtractNMEAWord(nmeaArgs->cmds, 1, buf, sizeof(buf));
        if (length == 0) {
            DBG_PRINT("angle fail");
            goto Done;
        }

        if (!HandleScaledBy100FractionalNumber(buf, &val)) {
            DBG_PRINT("scale angle fail");
            goto Done;
        }

        m_winddirection = (int)val;

        xDBG_PRINT_NNL("windirection ");
        xDBG_PRINT_INT(m_winddirection);

        // WindSpeed 0.0
        length = ExtractNMEAWord(nmeaArgs->cmds, 3, buf, sizeof(buf));
        if (length == 0) {
            DBG_PRINT("speed fail");
            goto Done;
        }

        // Handle a fractional number as an integer scaled by 100
        if (!HandleScaledBy100FractionalNumber(buf, &val)) {
            DBG_PRINT("scale speed fail");
            goto Done;
        }
        
        m_windspeed = (int)val;

        xDBG_PRINT_NNL("windspeed ");
        xDBG_PRINT_INT(m_windspeed);
    }
    else if (strncmp_P(nmeaArgs->prefix, PSTR("$WIXDR"), 6) == 0)  {

        xDBG_PRINT("NMEAEvent WIXDR");

        //
        // General transducer message
        //
        // http://www.catb.org/gpsd/NMEA.html#_xdr_transducer_measurement
        //
        // XDR - Transducer Measurement
        //
        //  $WIXDR,a,x.x,a,c--c, ..... *hh<CR><LF>
        //         |  |  |   |       |
        //         1  2  3   4       n
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

        // 
        // The following are transducer values produced by the
        // Menlo WeatherStationApp as part of the MenloFramework.
        // 
        // 
        // Type     Name            Units
        // 
        //  "T"     "TEMPERATURE"    "F"
        //  "P"     "BAROMETER"      "K"
        //  "H"     "HUMIDITY"       "P"
        //  "R"     "RAIN"           "I"
        //  "B"     "BATTERY"        "V"
        //  "S"     "SOLAR"          "V"
        //  "L"     "LIGHT"          "R" (RAW)
        // 

        DBG_PRINT_NNL("XDR ");
        DBG_PRINT_STRING(nmeaArgs->cmds);

        // Transducer type
        sensorType = nmeaArgs->cmds[0];

        // Measurement data
        length = ExtractNMEAWord(nmeaArgs->cmds, 2, buf, sizeof(buf));
        if (length == 0 ) {
            DBG_PRINT("Extract data fail");
            goto Done;
        }

        // Name
        length = ExtractNMEAWord(nmeaArgs->cmds, 4, name, sizeof(name));
        if (length == 0 ) {
            DBG_PRINT("Extract name fail");
            goto Done;
        }

        xDBG_PRINT_NNL("buf ");
        xDBG_PRINT_STRING(buf);

        // Handle a fractional number as an integer scaled by 100
        if (!HandleScaledBy100FractionalNumber(buf, &val)) {
            DBG_PRINT("scale value fail");
            goto Done;
        }

        xDBG_PRINT_NNL("sensorType");
        xDBG_PRINT_INT(sensorType);

        xDBG_PRINT_NNL("value ");
        xDBG_PRINT_INT((int)val);

        xDBG_PRINT_NNL("Name ");
        xDBG_PRINT_STRING(name);

        //
        // We could validate sensor names then type, but performing
        // a series of strncmp_P()'s is tight on the code space.
        //
        switch(sensorType) {

        case 'T':
            m_temperature = (int)val;
            break;

        case 'P':
            m_barometer = (int)val;
            break;

        case 'H':
            m_humidity = (int)val;
            break;

        case 'R':
            m_rain = (int)val;
            break;

        case 'B':
            m_battery = (int)val;
            break;

        case 'S':
            m_solar = (int)val;
            break;

        case 'L':
            m_nmeaLight = (int)val;
            break;

        default: 
            // Unrecognized
            DBG_PRINT("WIXDR Unknown sensor type");
            goto Done;
        }
    }
    else {
        // Unknown NMEA 0183 sentence, we just ignore it
        xDBG_PRINT("NMEAEvent Unknown NMEA sentence");
    }

Done:

    return MAX_POLL_TIME;
}

#else  // NMEA_WEATHER_SUPPORT
unsigned long
LightHouseHardware::NMEAEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    return MAX_POLL_TIME;
}
#endif // NMEA_WEATHER_SUPPORT
