
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
 *  Date: 11/20/2012
 *  File: MenloSensors.h
 */

#ifndef MenloSensors_h
#define MenloSensors_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#include <MenloPlatform.h>

#include <MenloTemperature.h>

//
// MenloSensors includes a roll up of the sensors used
// for various embedded sensor projects.
//

class MenloSensors {

public:
  
    //
    // Note: Any sensor with a pin value == 0xFF is disabled
    // and left alone.
    //

    //
    // Initialize the sensors without LED support
    //
    MenloSensors(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin
        );

    //
    // Initialize the sensors with LED support
    //
    MenloSensors(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin,
        uint8_t RedLedPin,
        uint8_t GreenLedPin,
        uint8_t BlueLedPin
        );

    int Initialize(MenloTemperature* temperatureClass);

    int GetMoisture();

    int GetLight();

    float GetTemperature();

    int GetSensorReadings(
        int *temperature,
        int *Moisture,
        int *light
	);

    //
    // A LedValue of 0 means off
    // 1 or greater is on.
    //
    // If PWM is available and on, then 1-100 encodes
    // a duty cycle.
    //
    void SetLedState(
        uint8_t redValue,
        uint8_t greenValue,
        uint8_t blueValue
        );

    void EnableSensorPower();

    void DisableSensorPower();

private:

    uint8_t    m_temperaturePin;
    
    uint8_t    m_sensorPowerEnable;
    uint8_t    m_moistureInput;
    uint8_t    m_lightInput;

    // RGB LED support
    uint8_t    m_redLedPin;
    uint8_t    m_greenLedPin;
    uint8_t    m_blueLedPin;

    // Interfaces to sensor classes
    MenloTemperature* m_temperature;
};

#endif // MenloSensors_h
