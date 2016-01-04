
/*
 *  Date: 05/13/2015
 *  File: WeatherShieldHardware.h
 *
 * Hardware handling for WeatherStation
 */

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

#ifndef WeatherShieldHardware_h
#define WeatherShieldHardware_h

#include <WeatherStationHardwareBase.h>

//
// This configuration is specific to the SparkFun WeatherShield
// at: https://www.sparkfun.com/products/12081
//
// https://github.com/sparkfun/Weather_Shield/
//
// This has the following sensors:
//
// MPL3115A2 altitude/pressure
//
// HTU2D humidity/temperature
//
// xxx GPS using NMEA 0183 on Software Serial
//
// Light on analog input
//
// Wind direction on analog input
//
// Wind speed on digital interrupt input
//
// Rain bucket counter on digital interrupt input
//
// Blue status LED on 7
//
// Green status LED on 8
//


//
// Note: SDA/SDC pins are used for the Two Wire (Wire.h)
// communication to the temperature, humidity, and altitude sensors.
//
struct WeatherShieldHardwareConfig {

    // Analog pins
    int windDirPin;   // A0
    int lightPin;     // A1
    int batPin;       // A2
    int referencePin; // A3
    int solarPin;     // A4 solar cell output

    // Digital pins
    int rainPin;      // 2
    int windSpeedPin; // 3
    int gpsTxPin;     // 4
    int gpsRxPin;     // 5
    int gpsPowerPin;  // 6
    int stat1Pin;     // 7 blue LED
    int stat2Pin;     // 8 green LED
};

//
// WeatherShieldHardware is a slave to WeatherStationApp.
//
// In addition WeatherStationApp determines when environmental
// readings are done.
//
class WeatherShieldHardware : public WeatherStationHardwareBase {

public:

    WeatherShieldHardware();

    // Initialize with sensors
    int
    Initialize(
        WeatherShieldHardwareConfig* config
       );

    //
    // Drive the GPS I/O and processing loop when powered on
    //
    virtual unsigned long PollSensors();

    //
    // This is invoked to sample sensors and perform averaging
    // where required between updates.
    //
    virtual unsigned long SampleSensors();

    //
    // Status lights
    //

    // Blue
    virtual void Stat1LightState(int* value, bool isSet);

    // Green
    virtual void Stat2LightState(int* value, bool isSet);

    // GPS power
    virtual void GpsPower(bool* value, bool isSet);

    // Sensor busy/available status
    virtual bool SensorBusy();

    //
    // Sensor readings
    //
    // Sensor readings are 16 bit integers. Signed is used to
    // allow straightforward use with gateway languages such as
    // Javascript.
    //
    // Values are typically scaled by 100 to allow double digit
    // X.xx precision from float values used during low level sensor
    // reading calculations.
    //
    virtual int GetWindSpeed();
    virtual int GetWindDirection();

    virtual int GetHumidity();
    virtual int GetTemperatureF();
    virtual int GetPressure();

    virtual int GetRain();
    virtual int GetAndResetRain(unsigned int previousValue);

    virtual int GetGps(WeatherStationGpsData* data);

    virtual int GetLight();

    virtual int GetSolar();

    virtual int GetBattery();

private:

    bool InitializeSensors(WeatherShieldHardwareConfig* config);

    bool ProcessGpsPassThrough(char c);

    //
    // Hardware state
    //

    bool m_gpsPower;

    //
    // True if Gps NMEA 0183 pass through is active
    //
    bool m_gpsPassThrough;

    //
    // This is set if GPS passthrough is in operation
    // and is in the middle of a sentence.
    //
    bool m_gpsInSentence;

    //
    // Light state is a 16 bit value to allow PWM intensity
    // encoding where available.
    //
    int m_stat1State;
    int m_stat2State;

    //
    // Wind speed
    //
    // The wind speed sensor is a magnetic switch which is interrupted
    // by motion of the anemometer. Each interruption of the switch
    // generates an interrupt on the microcontroller. By timing when
    // these interrupts occur wind speed may be calculated.
    //
    long m_lastWindCheck;
    volatile long m_lastWindIrq;
    volatile byte m_windClicks;

    //
    // Rain
    //
    // The rain sensor is a "bucket" type which fills with a calibrated
    // amount of water and then its weight triggers its dumping. This
    // dumping occurance generates an interrupt and is counted as a cycle.
    //
    volatile int m_rainCycles; // rain interrupts (bucket full/empties)

    //
    // Pins for environmental monitoring
    // These pins are analog inputs
    //
    WeatherShieldHardwareConfig m_config;
};

#endif // WeatherShieldHardware_h
