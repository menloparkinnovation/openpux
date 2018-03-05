
/*
 * Modifications, additions, subtractions are
 * Copyright (C) 2012 Menlo Park Innovation LLC
 *
 * This work incorporates one or more "open" or "shared" source
 * licenses. As such, its use and licensing conforms to the licenses
 * of the code incorporated.
 *
 * Menlo Park Innovation LLC does not provide any warranty, support,
 * or restrictions on its use and may be distributed under the
 * terms of the contained license(s).
 *
 *  Date: 11/20/2012
 *  File: OS_DS18S20.h
 *
 * Code contained within comes from the following open, shared, community,
 * public domain, or other licenses:
 *
 * Example code for DS18S20 1 wire temperature sensor.
 *
 * http://bildr.org/2011/07/ds18b20-arduino/ *
 *
 * Code also uses an open source Arduino library "OneWire"
 */

#ifndef OS_DS18S20_h
#define OS_DS18S20_h

//
// Any inclusion of standard libraries and headers is "Library Use"
// licensing.
//
#include <Arduino.h>
#include <inttypes.h>

// Include open source 1 wire library
#include <OneWire.h>

class OS_DS18S20 {

public:
  
  OS_DS18S20();

  //
  // initialize the sensor.
  //
  // pinNumber is the data line to use for the 1 wire protocol
  //
  // If the pinNumber == 0xFF the pin is left alone.
  //
  int Initialize(uint8_t pinNumber);

  // Get temperature reading
  unsigned short GetTemperature();

#if CALCULATE_TEMPERATURE
  // Get a calculated temperature as a float
  float GetCalculatedTemperature();
#endif

private:

  unsigned short GetReading();

  OneWire m_ds;

  uint8_t m_pinNumber;
};

#endif // OS_DS18S20_h
