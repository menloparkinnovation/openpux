
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
 *  File: OS_DS18S20.cpp
 *
 * Code contained within comes from the following open, shared, community,
 * public domain, or other licenses:
 *
 * Example code for DS18S20 1 wire temperature sensor.
 *
 * http://bildr.org/2011/07/ds18b20-arduino/ *
 *
 * Code also uses an open source Arduino library "OneWire" from
 * the same source as above.
 */

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

//
// Include Menlo Debug library support
//
#include <MenloDebug.h>

// Include open source 1 wire library
#include <OneWire.h> 

// This libraries header
#include <OS_DS18S20.h>

//
// This constructor takes the pin number of the 1 wire
// data line or the temperature sensor.
//
// It uses member initialzer syntax to construct the
// embedded OneWire class instance.
//
//OS_DS18S20::OS_DS18S20(uint8_t pinNumber) : m_ds(pinNumber)

// This requires Menlo updated OneWire library
OS_DS18S20::OS_DS18S20()
{
}

int
OS_DS18S20::Initialize(uint8_t pinNumber)
{
    m_pinNumber = pinNumber;
    m_ds.Initialize(pinNumber);
    return 0;
}

//
// Get raw temperature reading.
//
// The caller/cloud performs any required conversion or leveling.
//
unsigned short
OS_DS18S20::GetTemperature()
{
  unsigned short rawValue;

  //
  // Read temperature
  // Note: We have to read it twice after power up
  // since its first reading is always 85. This must have a 1 second
  // delay.
  //
  rawValue = GetReading();
  delay(1000);          
  rawValue = GetReading();

  // Wait 1 second before powering it down
  delay(1000);          

  return rawValue;
}

#if CALCULATE_TEMPERATURE

//
// Get temperature reading.
//
// returns the temperature from one DS18S20 in DEG Celsius
//
float
OS_DS18S20::GetCalculatedTemperature()
{
  unsigned short rawValue = GetTemperature();

  if (rawValue == 0) {
    return (float)0;
  }

  float tempRead = rawValue; //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
}
#endif

//
// Perform a raw temperature read from the sensor.
//
// It is assumed that the power has been on the sensor
// for a few seconds by the caller in order to allow it
// to stabilize.
//
// From open source example 11/20/2012
//
// Updated to use a class and debug library.
//
// Updated 01/06/2013 to work with raw values leaving any float
// conversions to the caller.
//
unsigned short
OS_DS18S20::GetReading()
{
  byte data[12];
  byte addr[8];

  //
  // If pin is unassigned, disabled, do nothing
  //
  if (m_pinNumber == 0xFF) {
      return 0;
  }

  if ( !m_ds.search(addr)) {
      //no more sensors on chain, reset search
      m_ds.reset_search();
      return 0;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return 0;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return 0;
  }

  m_ds.reset();
  m_ds.select(addr);
  m_ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = m_ds.reset();
  m_ds.select(addr);    
  m_ds.write(0xBE); // Read Scratchpad
  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = m_ds.read();
  }
  
  m_ds.reset_search();
  
  unsigned short tempValue = 0;
  tempValue |= data[0] & 0x00FF; // LSB
  tempValue |= ((data[1] << 8) & 0xFF00); // MSB

  return tempValue;
}
