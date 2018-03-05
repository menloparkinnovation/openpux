
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
 *  Date: 05/13/2015
 *  File: WeatherStationHardwareBase.h
 *
 * Base class for WeatherStation hardware handling.
 */

#ifndef WeatherStationHardwareBase_h
#define WeatherStationHardwareBase_h

//
// Sensor hardware configuration pins.
//
// Sensors are in a struct which is used throughout since
// each configuration may add or delete sensors.
//
// This minimizes updates to the various call sites when
// a new sensor type is added or deleted.
//
struct WeatherStationSensors {

  // WeatherStation monitoring
  int battery;               // battery charge state
  int batterycurrent;        // battery current
  int solar;                 // solar cell voltage

  // Environmental
  int lightIntensity;        // light intensity reading
  int temperature;           // temperature sensor
  int humidity;
  int barometer;
  int windspeed;
  int winddirection;
  int rainfall;
  int moisture;              // soil moisture/tide level/seastate

  // GPS
  int gpsPower;              // Power on GPS
  int latitude;
  int longitude;
  int altitude;
};

//
// This is an optional struct for architectures that
// return full precision float values.
//
// This supports the SparkFun Photon WeatherShield
//
struct WeatherStationSensors_Float {

  // Ints
  int winddirection;
  int windgustdirection;
  int soilmoisture;

  int freememory;

  // Floats
  float windspeed;
  float windgust;
  float rainfall;
  float humidity;
  float temperature;
  float barometer;
  float soiltemperature;
};

struct WeatherStationGpsData;

//
// WeatherStationHardware is a slave to WeatherStationApp.
//
// In addition WeatherStationApp determines when environmental
// readings are done.
//
class WeatherStationHardwareBase : public MenloObject {

public:

    //
    // These drive the sensor hardware state machine(s)
    //

    //
    // Main processing loop to process real time streaming data
    // that can be lost between samples.
    //
    // Example: streaming GPS data.
    //
    // Returns the number of milliseconds till next call,
    // or MAX_POLL_TIME if it does not need service due to
    // being interrupt driven, etc.
    //
    virtual unsigned long PollSensors() = 0;

    //
    // Application configured sensor read/averaging interval
    //
    // Example: Averaging WindSpeed over time
    //
    // Returns the number of milliseconds till next call,
    // or MAX_POLL_TIME if it does not need service due to
    // being interrupt driven, etc.
    //
    // This is called at a default 1 second interval.
    //
    virtual unsigned long SampleSensors() = 0;

    // Status lights
    virtual void Stat1LightState(int* value, bool isSet) = 0;
    virtual void Stat2LightState(int* value, bool isSet) = 0;

    // GPS power
    virtual void GpsPower(bool* value, bool isSet) = 0;

    // Sensor busy/available status
    virtual bool SensorBusy() = 0;

    //
    // Sensor readings
    //
    // Returns temporary structure whose values are
    // valid till the next call.
    //
    // This is done to minimize stack memory.
    //
    virtual WeatherStationSensors_Float*  GetReadings_Float() {
        return NULL;
    }

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

    // WindSpeed is in 100's of MPH (354 == 3.54 MPH)
    virtual int GetWindSpeed() = 0;

    // WindDirection is in degrees (270 == West)
    virtual int GetWindDirection() = 0;

    // Humidity as 100's of a percent (5000 == 50%)
    virtual int GetHumidity() = 0;

    // 100's of a degree F (7500 == 75 degrees F)
    virtual int GetTemperatureF() = 0;

    virtual int GetPressure() = 0;

    // Rain is in 100's of an inch (211 == .211")
    virtual int GetRain() = 0;

    //
    // Subtract previousValue from the current rain reading to "claim" them and
    // reset the counter.
    //
    virtual int GetAndResetRain(unsigned int previousValue) = 0;

    virtual int GetGps(WeatherStationGpsData* data) = 0;

    virtual int GetLight() = 0;

    virtual int GetSolar() = 0;

    virtual int GetBattery() = 0;

private:

};

#endif // WeatherStationHardwareBase_h
