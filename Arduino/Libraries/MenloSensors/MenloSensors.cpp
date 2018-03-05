
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
 *  File: MenloSensors.cpp
 */

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#include <MenloPlatform.h>

//
// Include Menlo Debug library support
//
#include <MenloDebug.h>

// This libraries header
#include <MenloSensors.h>

// Constructor without LED support
MenloSensors::MenloSensors(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin) : 

  m_sensorPowerEnable(sensorPowerEnablePin),
  m_temperaturePin(temperaturePin),
  m_moistureInput(moistureInputPin),
  m_lightInput(lightInputPin)
{
  // 0xFF means invalid pin, don't touch the pin
  m_redLedPin = 0xFF;
  m_greenLedPin = 0xFF;
  m_blueLedPin = 0xFF;

  // Interface classes are optional
  m_temperature = NULL;
}

// Constructor with LED support
MenloSensors::MenloSensors(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin,
        uint8_t redLedPin,
        uint8_t greenLedPin,
        uint8_t blueLedPin) : 
  m_sensorPowerEnable(sensorPowerEnablePin),
  m_temperaturePin(temperaturePin),
  m_moistureInput(moistureInputPin),
  m_lightInput(lightInputPin),
  m_redLedPin(redLedPin),
  m_greenLedPin(greenLedPin),
  m_blueLedPin(blueLedPin)
{
  // Interface classes are optional
  m_temperature = NULL;
}

int
MenloSensors::Initialize(
    MenloTemperature* temperatureClass
    )
{
    // Setup the sensor enable/power pin
    if (m_sensorPowerEnable != 0xFF) {
        pinMode(m_sensorPowerEnable, OUTPUT);  
    }

    // If the LED pins are valid, initialize them for output
    if (m_redLedPin != 0xFF) {
        pinMode(m_redLedPin, OUTPUT);  
    }

    if (m_greenLedPin != 0xFF) {
        pinMode(m_greenLedPin, OUTPUT);  
    }

    if (m_blueLedPin != 0xFF) {
        pinMode(m_blueLedPin, OUTPUT);  
    }

    m_temperature = temperatureClass;

    return 0;
}

int
MenloSensors::GetMoisture()
{
  // Read moisture
  if (m_moistureInput != 0xFF) {
    return  0;
  }
  else {
    return  analogRead(m_moistureInput);    
  }
}

int
MenloSensors::GetLight()
{
  if (m_lightInput != 0xFF) {
      return analogRead(m_lightInput);
  }
  else {
      return 0;
  }
}

float
MenloSensors::GetTemperature()
{
  float value;

  if (m_temperature != NULL) {
      value = m_temperature->GetTemperature();
  }
  else {
      value = 0;
  }

  return value;
}

int
MenloSensors::GetSensorReadings(
    int *temperature,
    int *moisture,
    int *light)
{
  // Turn on the sensors
  EnableSensorPower();

  // Read moisture
  *moisture = GetMoisture();

  // Read light
  *light = GetLight();

  *temperature = GetTemperature();

  // Turn sensors power off
  DisableSensorPower();

  return 0;
}

void
MenloSensors::EnableSensorPower()
{
  // Turn on the sensors
  if (m_sensorPowerEnable != 0xFF) {
      digitalWrite(m_sensorPowerEnable, HIGH);  
  }
}

void
MenloSensors::DisableSensorPower()
{
  if (m_sensorPowerEnable != 0xFF) {
      digitalWrite(m_sensorPowerEnable, LOW);  
  }
}

void
MenloSensors::SetLedState(
    uint8_t redValue,
    uint8_t greenValue,
    uint8_t blueValue
    )
{
    uint8_t state;

    // TODO: Support PWM!
    if (m_redLedPin != 0xFF) {
        if (redValue == 0) state = LOW;
        else state = HIGH;
        digitalWrite(m_redLedPin, state);
    }

    if (m_greenLedPin != 0xFF) {
        if (greenValue == 0) state = LOW;
        else state = HIGH;
        digitalWrite(m_greenLedPin, state);
    }

    if (m_blueLedPin != 0xFF) {
        if (blueValue == 0) state = LOW;
        else state = HIGH;
        digitalWrite(m_blueLedPin, state);
    }

    return;
}
