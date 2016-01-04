
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
 *
 * This incorparates code from the SparkFun Weathershield example
 * at https://github.com/sparkfun/Weather_Shield/ and its copyright
 * header follows:
 */

/* 
 Weather Shield Example
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 16th, 2013
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Much of this is based on Mike Grusin's USB Weather Board code: https://www.sparkfun.com/products/10586
 
 This code reads all the various sensors (wind speed, direction, rain gauge, humidty, pressure, light, batt_lvl)
 and reports it over the serial comm port. This can be easily routed to an datalogger (such as OpenLog) or
 a wireless transmitter (such as Electric Imp).
 
 Measurements are reported once a second but windspeed and rain gauge are tied to interrupts that are
 calcualted at each report.
 
 This example code assumes the GP-635T GPS module is attached.
 
 */

/*
 *  Date: 05/13/2015
 *  File: WeatherShieldHardware.cpp
 *
 * Hardware handling for WeatherStation
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

// Libraries/WeatherStationApp/WeatherStationHardwareBase.h
#include <WeatherStationHardwareBase.h>

// This implementations header
#include "WeatherShieldHardware.h"

//
// RAM on AtMega328's is pretty tight at 2K. Sensors
// and their libraries can consume significant RAM.
//
// In addition RAM is needed to support the MenloFramework
// runtime, Dweet serial port handling, and standard runtime
// stack and (very little) heap.
//
// The following are the calculations for this application:
//
// Note: Totals are impacted by which library classes are used
// in the MenloFramework. It's the delta's here we are concerned about.
//
// Before Sensor includes:
//
// 16,146 bytes flash, 895 bytes ram
//
// After Sensor includes:
//
// 17,402 bytes flash, 1,170 ram
//  cost: 1256 bytes flash,  275 ram
//
// With class instances:
//
// 18,532 flash, 1392 ram (655 free)
//  cost: 1130 flash, 222 ram
//
//  Total Cost for sensors:
//   2386 flash, 497 ram
//
// Current runtime memory configuration:
//
// 128 heap
// 320 stack
//  16 guard
// ------------
//  464 bytes
//
//  This is 191 bytes left over
//
#include <Wire.h> //I2C needed for sensors       (207 bytes RAM)
#include "SparkFunMPL3115A2.h" //Pressure sensor (0 bytes RAM)
#include "HTU21D.h" //Humidity sensor            (0 bytes RAM)
#include <SoftwareSerial.h> //Needed for GPS     (68 bytes RAM)
#include <TinyGPS++.h> //GPS parsing             (0 bytes RAM)

//
// Generally its best to place sub-classes for sensors and
// utilities into the main hardware class.
//
// But for quickly porting existing Arduino implementations
// which have their own existing logic it makes sense to still
// use globals since only one instance is available.
//
// The top level WeatherShieldHardware class can be concerned
// with communication with the rest of the framework and provides
// the isolated interface to the hardware driver implementation
// details.
//
// The same is true for coniguration. Where hardware pin
// configuration can be configured in the WeatherShieldHardwareConfig
// it is, but some require static configuration at class construction
// time. In this case Software Serial (instance ss()) is an example.
//

//
// These sensors are on the I2C (Two Wire) bus
//
MPL3115A2 g_Pressure; // Create an instance of the pressure sensor
HTU21D g_Humidity   ; // Create an instance of the humidity sensor

//
// GPS
//
// GPS is hooked up to pin 5 for RX and pin 4 for TX
//
// GPS power is controlled through pin 6
//
static const int gpsTxPin = 4;
static const int gpsRxPin = 5;

SoftwareSerial g_ss(gpsRxPin, gpsTxPin); 
TinyGPSPlus g_gps;

//
// Rain support
//
// Note: volatiles are subject to modification by IRQs
//

// Number of rain cycles since last reset.
volatile unsigned int g_rainCycles = 0; // each cycle is 0.011" of water

//
//
// These implement switch debounce by ignoring interrupts
// that occur within 10ms of each other.
//
volatile unsigned long g_raintime;
volatile unsigned long g_rainlast;
volatile unsigned long g_raininterval;

void rainIRQ()
{
    //
    // Count rain gauge bucket tips as they occur
    // Activated by the magnet and reed switch in the rain gauge, attached to input D2
    //

    g_raintime = millis(); // grab current time
    g_raininterval = g_raintime - g_rainlast; // calculate interval between this and last event

    // ignore switch-bounce glitches less than 10mS after initial edge
    if (g_raininterval > 10) {
        g_rainCycles++;
    
        //dailyrainin += 0.011; //Each dump is 0.011" of water
        //rainHour[minutes] += 0.011; //Increase this minute's amount of rain
        g_rainlast = g_raintime; // set up for next event
    }
}

//
// Wind speed Support
//

long g_lastWindCheck = 0;
volatile long g_lastWindIRQ = 0;
volatile byte g_windClicks = 0; // 1.492MPH for each click per second

void wspeedIRQ()
{
    //
    // Activated by the magnet in the anemometer (2 ticks per rotation), attached to input D3
    //

    //
    // Ignore switch-bounce glitches less than 10ms (142MPH max reading)
    // after the reed switch closes
    //
    if (millis() - g_lastWindIRQ > 10)
    {
        g_lastWindIRQ = millis(); //Grab the current time
        g_windClicks++; //There is 1.492MPH for each click per second.
    }
}

WeatherShieldHardware::WeatherShieldHardware ()
{
        m_gpsPower = false;
        m_gpsPassThrough = false;
        m_gpsInSentence = false;

        m_stat1State = false;
        m_stat2State = false;

        m_config.stat1Pin = -1;
        m_config.stat2Pin = -1;
}

int
WeatherShieldHardware::Initialize(
    WeatherShieldHardwareConfig* config
    )
{
    m_config = *config;

    //
    // Setup our application hardware pins
    //

    if (m_config.stat1Pin != (-1)) {
        pinMode(m_config.stat1Pin, OUTPUT);
    }

    if (m_config.stat2Pin != (-1)) {
        pinMode(m_config.stat2Pin, OUTPUT);
    }

    if (m_config.gpsPowerPin != (-1)) {

        bool gpsInitialPowerState = true;

        pinMode(m_config.gpsPowerPin, OUTPUT);

        // Turn on the GPS
        GpsPower(&gpsInitialPowerState, true);
    }

    if (m_config.windSpeedPin != (-1)) {
        pinMode(m_config.windSpeedPin, INPUT_PULLUP);
    }

    if (m_config.rainPin != (-1)) {
        pinMode(m_config.rainPin, INPUT_PULLUP);
    }

    //
    // Setup analog pins
    //
    if (m_config.referencePin != (-1)) {
        pinMode(m_config.referencePin, INPUT);
    }

    if (m_config.lightPin != (-1)) {
        pinMode(m_config.lightPin, INPUT);
    }

    //
    // attach external interrupt pins to IRQ functions
    //
    attachInterrupt(0, rainIRQ, FALLING);
    attachInterrupt(1, wspeedIRQ, FALLING);

    // turn on interrupts
    //interrupts(); // Not needed unless noInterrupts() is called

    //
    // Setup environmental sensors
    //
    InitializeSensors(config);

    //
    // Note: The Dweet Application is responsible for getting
    // any configuration from the MenloConfigStore (EEPROM)
    // and setting states of the hardware.
    //

    return 0;
}

bool
WeatherShieldHardware::InitializeSensors(WeatherShieldHardwareConfig* config)
{

    // Start GPS serial input listener
    g_ss.begin(9600);

    //
    //Configure the pressure sensor
    //

    g_Pressure.begin(); // Get sensor online
    g_Pressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
    g_Pressure.setOversampleRate(7); // Set Oversample to the recommended 128
    g_Pressure.enableEventFlags(); // Enable all three pressure and temp event flags 

    //Configure the humidity sensor
    g_Humidity.begin();

    return true;
}

void
WeatherShieldHardware::Stat1LightState(int* value, bool isSet)
{
  if (isSet) {
      m_stat1State = *value;

      if (m_stat1State != 0) {
          digitalWrite(m_config.stat1Pin, HIGH);
      }
      else {
          digitalWrite(m_config.stat1Pin, LOW);
      }
  }
  else {
      *value = m_stat1State;
  }
}

void
WeatherShieldHardware::Stat2LightState(int* value, bool isSet)
{
  if (isSet) {
      m_stat2State = *value;

      if (m_stat2State != 0) {
          digitalWrite(m_config.stat2Pin, HIGH);
      }
      else {
          digitalWrite(m_config.stat2Pin, LOW);
      }
  }
  else {
      *value = m_stat2State;
  }
}

//
// The GPS unit is GP-635T from AHD Technology Inc.
//
// http://cdn.sparkfun.com/datasheets/Sensors/GPS/GP-635T-121130.pdf
//
// https://www.sparkfun.com/products/11571
//
// The GPS unit consumes 56ma when powered on.
//
// Note: Cycling GPS power on the WeatherShield can require
// a lengthy re-acquisition time unless the auxiliary GPS power
// battery terminal is hooked up.
//
// When GPS is in passthrough streaming mode it will by default send
// GPS NMEA 0183 messages once per second regardless of the sensor update
// interval settings. Any NMEA 0183 weather station data will be
// interleaved by the logic in WeatherStationApp and sent on
// the configured sensor update interval.
//
void
WeatherShieldHardware::GpsPower(bool* value, bool isSet)
{
    if (isSet) {
        m_gpsPower = *value;

        if (m_gpsPower) {

            // enable Gps passthrough NMEA streaming
            m_gpsPassThrough = true;

            digitalWrite(m_config.gpsPowerPin, HIGH);
        }
        else {

            // disable Gps passthrough NMEA streaming
            m_gpsPassThrough = false;
            digitalWrite(m_config.gpsPowerPin, LOW);
        }

        return;
    }
    else {
      *value = m_gpsPower;
    }
}

//
// Returns the instantaneous wind speed
//
float
get_wind_speed()
{
  float deltaTime = millis() - g_lastWindCheck; //750ms

  deltaTime /= 1000.0; //Covert to seconds

  float windSpeed = (float)g_windClicks / deltaTime; //3 / 0.750s = 4

  g_windClicks = 0; //Reset and start watching for new wind
  g_lastWindCheck = millis();

  windSpeed *= 1.492; //4 * 1.492 = 5.968MPH

  return(windSpeed);
}

//
// Read the wind speed sensor.
//
// Value is in 100's.
//
// 35.12 MPH == 3512.
//
int
WeatherShieldHardware::GetWindSpeed() 
{
    int windspeed;
    float reading;

    // This accesses the interrupt updated variables
    reading = get_wind_speed();

    //
    // Scale it to an integer as shipping float's across
    // the communication links is not very portable.
    //
    // Windspeed is scaled by 100
    //
    // 32,767 == 327MPH
    //
    // 2.37  MPH == 237
    // 85.51 MPH == 8551
    //
    windspeed = reading * 100;

    return windspeed;
}

//
// Read the wind direction sensor, return heading in degrees
//
// 0 - 360
//
int
WeatherShieldHardware::GetWindDirection() 
{
  unsigned int adc;

  // get the current reading from the sensor
  adc = analogRead(m_config.windDirPin);

  //
  // The following table is ADC readings for the wind direction sensor output,
  // sorted from low to high.
  //
  // Each threshold is the midpoint between adjacent headings. The output is
  // degrees for that ADC reading.
  //
  // Note that these are not in compass degree order! See Weather Meters
  // datasheet for more information.
  //

  if (adc < 380) return (113);
  if (adc < 393) return (68);
  if (adc < 414) return (90);
  if (adc < 456) return (158);
  if (adc < 508) return (135);
  if (adc < 551) return (203);
  if (adc < 615) return (180);
  if (adc < 680) return (23);
  if (adc < 746) return (45);
  if (adc < 801) return (248);
  if (adc < 833) return (225);
  if (adc < 878) return (338);
  if (adc < 913) return (0);
  if (adc < 940) return (293);
  if (adc < 967) return (315);
  if (adc < 990) return (270);

  //
  // error, disconnected?
  //
  // 0 is a more reasonable value for no disconnect.
  //

  return 0; 
}

//
// Read humidity.
//
// Scaled by 100
//
// 50% == 5000
//
//
int
WeatherShieldHardware::GetHumidity()
{
    int humidity;
    float reading;

    reading = g_Humidity.readHumidity();

    humidity = reading * 100;

    return humidity;
}

//
// Read Temperature
//
// Scaled by 100
//
// 85.34 degrees F == 8534
//
int
WeatherShieldHardware::GetTemperatureF()
{
  float reading;
  int tempf;

  // Measure temperature in F
  reading = g_Pressure.readTempF();

  tempf = reading * 100;

  return tempf;
}

int
WeatherShieldHardware::GetPressure()
{
  int pressure;
  float reading;

  // Measure pressure in Pascals from 20 to 110 kPa
  reading = g_Pressure.readPressure();

  //
  // Pressure reads high readings such as 83351.25, so scale down
  // This will then get divided by a 100 when displayed and
  // represent Kilopascals
  //
  pressure = reading / 10;

  return pressure;
}

//
// Get the current value of rain cycles
//
int
WeatherShieldHardware::GetRain()
{
    int returnValue;
    int rainCycles;
    float inches;
    float inchesPerCycle;

    inchesPerCycle = 0.011; //Each dump is 0.011" of water

    // No interrupt tear of 16 bit word on AtMega328
    rainCycles = g_rainCycles;

    inches = (float)rainCycles * inchesPerCycle;

    returnValue = (int)(inches * 100);

    return returnValue;
}

int
WeatherShieldHardware::GetSolar()
{
    int value = 0;

    if (m_config.solarPin != (-1)) {
        value = analogRead(m_config.solarPin);
    }    

    return 0;
}

//
// Get the current value of rain cycles.
//
// Subtract the supplied previousValue from the reading
// "claiming" the cycles.
//
// This is done this way since interrupts could occur generating
// new cycles between the time of the callers read, and this
// functions reset.
//
// It uses an interrupt disable()/enable() guard to ensure
// no rain events are lost.
//
int
WeatherShieldHardware::GetAndResetRain(unsigned int previousValue)
{
    unsigned int currentCycles;

    // Disable interrupts due to access by ISR
    noInterrupts();

    currentCycles = g_rainCycles;

    if (currentCycles == 0) {
        // re-enable interrupts
        interrupts();
        return 0;
    }

    //
    // The higher value is because the caller wants to ensure
    // the counter is cleared.
    //    
    if (previousValue > g_rainCycles) {
        previousValue = g_rainCycles;
    }

    // "claim" the cycles
    g_rainCycles -= previousValue;

    // re-enable interrupts
    interrupts();
   
    // Actual value claimed
    return previousValue;
}

//
// Returns the voltage of the light sensor based on the 3.3V rail
// This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
//
int
WeatherShieldHardware::GetLight()
{
  int lightSensor;

  float operatingVoltage = analogRead(m_config.referencePin);

  float reading = analogRead(m_config.lightPin);
  
  operatingVoltage = 3.3 / operatingVoltage; // The reference voltage is 3.3V
  
  reading = operatingVoltage * reading;
  
  lightSensor = reading * 100;

  return(lightSensor);
}

//
// Returns the voltage of the raw pin based on the 3.3V rail
// This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
// Battery level is connected to the RAW pin on Arduino and is fed through two 5% resistors:
// 3.9K on the high side (R1), and 1K on the low side (R2)
//
int
WeatherShieldHardware::GetBattery()
{
  int valueToReturn;
  float operatingVoltage = analogRead(m_config.referencePin);

  float rawVoltage = analogRead(m_config.batPin);
  
  operatingVoltage = 3.30 / operatingVoltage; //The reference voltage is 3.3V
  
  // Convert the 0 to 1023 int to actual voltage on BATT pin
  rawVoltage = operatingVoltage * rawVoltage;
  
  //
  // (3.9k+1k)/1k - multiple BATT voltage by the voltage divider
  // to get actual system voltage
  //
  rawVoltage *= 4.90;
  
  valueToReturn = rawVoltage * 100;

  return(valueToReturn);
}

//
// Poll() routine required to gather GPS data from the software serial
// buffer before it overruns.
//
unsigned long
WeatherShieldHardware::PollSensors()
{
    char c;
    bool sentenceEnd = false;

    //    
    // If the GPS is powered off, return
    //
    if (!m_gpsPower)  {
        return MAX_POLL_TIME;
    }

    //
    // Pull characters from software serial and potentially
    // process the GPS NMEA stream.
    //
    while (g_ss.available() && !sentenceEnd) {

      c = g_ss.read();

      if (m_gpsPassThrough) {

          //
          // If the end of a complete sentence is seen exit early
          // till the next Poll() interval in order to allow multiplexing
          // of other NMEA 0183 traffic by the main application framework.
          //
          sentenceEnd = ProcessGpsPassThrough(c);
      }

      //
      // Supply the characters to the GPS handler which will
      // update its internal state as required.
      //
      // This class tracks the condition and status of
      // the GPS signals and keeps a running update of
      // position, altitude, etc.
      //
      g_gps.encode(c);
    }

    return MAX_POLL_TIME;
}

//
// Sample sensors and provide any averaging.
//
unsigned long 
WeatherShieldHardware::SampleSensors()
{
    return MAX_POLL_TIME;
}

int
WeatherShieldHardware::GetGps(WeatherStationGpsData* data)
{
    if (!m_gpsPower) {
        return DWEET_NO_POWER;
    }

#if GPS_NMEA_GENERATION

    //
    // Generating detailed NMEA 0183 GPS data is optional
    // for some small microcontrollers such as the AtMega 328
    // do not have the available code space.
    //
    // GPS streaming allows the NMEA 0183 messages from the GPS
    // to pass through to an application when GPS power is on.
    //

    //
    // Fill in the readings from the g_gps class
    //

    if (g_gps.location.isValid()) {
        // double g_gps.location.lat;
        // double g_gps.location.lng;
        // uint32_t g_gps.location.age; // in millis
    }

    return 0;
#else
    return DWEET_ERROR_UNSUP;
#endif
}

//
// Sensor busy/available status
//
bool
WeatherShieldHardware::SensorBusy()
{
    //
    // We return sensor busy when in the middle of a GPS
    // sentence in order to not mix NMEA 0183 messages.
    //
    return m_gpsInSentence;
}

//
// Process GPS passthrough when enabled.
//
// Returns true if its the end of a sentence.
//
bool
WeatherShieldHardware::ProcessGpsPassThrough(char c)
{
    bool sentenceEnd = false;

    //
    // If Gps passthrough is not on, return.
    //
    if (!m_gpsPassThrough) {
        return sentenceEnd;
    }

    //
    // Gps Passthrough allows the NMEA 0183 formatted messages
    // from the GPS to be output along with the WeatherStations
    // meteorological NMEA 0183 data.
    //
    // A state machine ensures that the messages do not
    // get mixed.
    //

    if (c == '$') {
        m_gpsInSentence = true;
    }
    else if (c == '\n') {
        sentenceEnd = true;
        m_gpsInSentence = false;
    }

    //
    // This is called at Poll() time from the main application
    // framework when the main application is not in the middle of processing
    // a NMEA 0183 message. So the GPS has the channel until it declares
    // sentence end.
    //

    //
    // This should be a better interface in which the
    // port* is passed.
    //
    Serial.write(c);

    return sentenceEnd;
}
