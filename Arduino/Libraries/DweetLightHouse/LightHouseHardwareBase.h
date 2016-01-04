
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
 *  Date: 05/12/2015
 *  File: LightHouseHardwareBase.h
 *
 * Base class for LightHouse hardware handling.
 *
 * The Lighthouse provides 3 color LED and environmental
 * and battery monitoring.
 *
 */

#ifndef LightHouseHardwareBase_h
#define LightHouseHardwareBase_h

#include "MenloLightHouse.h"

//
// Sensors are in a struct which is used throughout since
// each configuration may add or delete sensors.
//
// This minimizes updates to the various call sites when
// a new sensor type is added or deleted.
//
struct LightHouseSensors {
  int sensorPower;           // Power on sensors

  // LightStation monitoring
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
  int moisture;              // soil moisture/tide level

  // GPS readings
  int latitude;
  int longitude;
  int altitude;
};

//
// LightHouseHardware is a slave to LightHouseApp which handles
// the logic of when the light should be on, as well as its
// flash sequence.
//
// In addition LightHouseApp determines when environmental
// readings are done.
//
class LightHouseHardwareBase : public MenloObject {

public:

    virtual bool getLightState() = 0;

    virtual void setLightState(MenloLightHouseEventArgs* args) = 0;

    virtual void setRGB(bool state) = 0;

    virtual void setRGBIntensity(int red, int green, int blue) = 0;

    virtual void setPolarity(bool polarity) = 0;

    virtual bool readSensors(LightHouseSensors* sensors) = 0;

private:

};

#endif // LightHouseHardwareBase_h
