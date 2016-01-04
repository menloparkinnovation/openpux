/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

/*
 * 11/24/2013
 *
 * Software SPI created from Arduino SPI library by
 * MenloPark Innovation LLC
 *
 *
 * Software SPI allows the use of low speed devices on
 * alternate pins than the hardware SPI.
 *
 * In many cases a WiFi or Ethernet shield will take over
 * the SPI bus with their own clock frequency, mode, etc,
 * and setup an interrupt handler. Without careful re-design
 * of these (usually vendor supplied) classes, sharing of the
 * SPI bus is not possible.
 *
 * The SoftSPI allows use of alternate pins for SPI.
 *
 * The Arduino SPI contract is described at:
 * http://arduino.cc/en/Reference/SPI
 */

#include <Arduino.h>
#include "pins_arduino.h"
#include "OS_SoftSPI.h"

//
// SPI Basics from
//
// http://arduino.cc/en/Reference/SPI//
// http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus//
//
// SS - Slave Select
//   - When LOW, the device is enabled
//   - Caller of this class is responsible since its device dependent
//
// SCK - Serial Clock
//
// Clock Polarity value is the value when the clock is *IDLE*
//
// Clock Phase describes SCK assertion edge or going idle edge
//
// SPI.setDataMode(uint8_t)
//
//              Clock Polarity (CPOL)    Clock Phase (CPHA)
//  SPI_MODE0          0                         0
//  SPI_MODE1          0                         1
//  SPI_MODE2          1                         0
//  SPI_MODE3          1                         1
//
// Data can be MSD or LSB
//  SPI.setBitOrder(uint8_t)
//
// SPI Speed
//  SPI.setClockDivider(uint8_t)
//
//    - Based on 16Mhz input clock
//
//    - Ranges from SPI_CLOCK_DIV2 (8Mhz) to SPI_CLOCK_DIV128 (125Khz)
//
//    - Default is 4Mhz on Arduino by setting SPI_CLOCK_DIV4
//
// Arduino Uno SPI pins
//
// MOSI - 11 or ICSP 4
// MISO - 12 or ICSP 1
// SCK  - 13 or ICSP 3
// SS   - 10
//
// Note: The SS pin 10 can be an input or output on the AVR processors.
//
// If its an OUTPUT the board is always the master and can't
// be put into slave mode by an external device. Once an OUTPUT
// the pin us usable to select one of possibly many SPI devices
// so its not wasted.
//
// If this pin is allowed to be an input, the Arduino can be
// placed into slave mode. This may be useful for some projects
// but is not supported by the Arduino SPI library.
//

// See notes in SoftSPI.h on this global
// Uncomment to enable global SoftSPI
//OS_SoftSPIClass SoftSPI;

void OS_SoftSPIClass::configure(byte mosi, byte miso, byte sck) {
  m_mosi = mosi;
  m_miso = miso;
  m_sck = sck;

  //
  // http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus//
  // states that the default is MSB first.
  //

  m_lsbFirst = false;

  // Default to SPI_MODE0
  setDataMode(SPI_MODE0);

  //
  // Set our data setup and hold timings to a 1 us, which
  // should work for most devices. For higher speed interfaces,
  // the caller can override with zero. For slower devices, or
  // devices on longer/higher capacitance wires, later
  // call can increase these values.
  //

  setInterfaceTimings(1, 1, 1);
}

void OS_SoftSPIClass::setInterfaceTimings(unsigned int csnDelay,
    unsigned int mosiDelay,
     unsigned int misoDelay) {

  m_csnDelay = csnDelay;
  m_mosiDelay = mosiDelay;
  m_misoDelay = misoDelay;
}

void OS_SoftSPIClass::begin() {

  //
  // Set direction to OUTPUT for SCK and MOSI pins
  //
  // Set direction to INPUT for the MISO pin
  //

  pinMode(m_miso, INPUT);
  pinMode(m_mosi, OUTPUT);
  pinMode(m_sck, OUTPUT);
  
  //
  // Set the idle state
  //

  digitalWrite(m_sck, m_sckIdle);
  digitalWrite(m_mosi, LOW);
}

void OS_SoftSPIClass::end() {
}

void OS_SoftSPIClass::setBitOrder(uint8_t bitOrder)
{
  if(bitOrder == LSBFIRST) {
    m_lsbFirst = true;
  } else {
    m_lsbFirst = false;
  }
}

void OS_SoftSPIClass::setDataMode(uint8_t mode)
{
  //SPCR = (SPCR & ~SPI_MODE_MASK) | mode;

  //
  // Clock Polarity describes the clocks *Idle* state.
  //
  // CPOL=0 - Idle low
  // CPOL=1 = Idle high
  //
  // Clock Phase describes when the data is sampled and
  // must be stable on the line.
  //
  // CPHA=0 is when the clock asserts
  // CPHA=1 is when the clock de-asserts or idles
  //

  //
  // Note: The code ensures that data is always stable
  // while the clock is asserted, and the slave line is
  // sampled. This makes master operation work independent
  // of clock phase.
  //

  switch (mode) {
  case SPI_MODE0:

    // CPOL=0
    m_sckIdle = LOW;
    m_sckAssert = HIGH;

    // CPHA=0
    break;

  case SPI_MODE1:

    // CPOL=0
    m_sckIdle = LOW;
    m_sckAssert = HIGH;

    // CPHA=1
    break;

  case SPI_MODE2:

    // CPOL=1
    m_sckIdle = HIGH;
    m_sckAssert = LOW;

    // CPHA=0
    break;

  case SPI_MODE3:

    // CPOL=1
    m_sckIdle = HIGH;
    m_sckAssert = LOW;

    // CPHA=1
    break;

  default:
    // Unknown mode, need an assert function
    break;
  }
}

void OS_SoftSPIClass::setClockDivider(uint8_t rate)
{
  //
  // Noop for software.
  //
  // Note: Could insert a delay here for really slow rates
  // for devices on long wires that are tolerant of slow clocks.
  //
  // If you are on a faster micro, such as an ARM, a delay may
  // be required for chips which require higher divider values
  // (lower frequencies) since the processor may overclock
  // the device if its fast enough. Not likely with an ATMEGA 328.
  //
  // (Could put a logic analyzer on it and measure and tune this using
  //  constants for different processors)
  //

  //SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
  //SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
}

uint8_t OS_SoftSPIClass::transfer(uint8_t _data) {
  uint8_t index;
  uint8_t shiftIn;
  uint8_t shiftOut;
  int reg;

  //
  // SPI implements a virtual ring that exchanges a byte
  // (or other sized word) from the master to the slave a
  // single bit at a time.
  //
  // Note: some chips may exchange a word of different
  // bit size. For example some A/D chips will transfer
  // 12 bits for their samples. See the chip programming
  // data sheet. This class may need to be updated to exchange
  // larger bit sizes as it currently uses the default 8.
  //
  // If you do this, create a transfer12() or other function
  // since its likely the chip will require a mixture of
  // byte commands and 12 bit readings depending on the
  // chip.
  //
  // A Chip Select (CS) line is set to LOW to enable a chip
  // before communication is started. By default HIGH deselects
  // the chip. Only one chip may be selected at a time
  // to allow for multi-drop support.
  //
  // The Chip Select (CS) is the responsibility of the caller/user
  // of this class, including waiting any chip specific wait
  // times. For example a humidity sensor requires some number
  // of milliseconds once its CS is asserted before data
  // may be transfered using this function.
  //
  // The SCK line clocks the data in and out. Its IDLE value
  // depends on the Clock Polarity (CPOL) value.
  // CPOL=0 clock is IDLE low
  // CPOL=1 clock is IDLE high
  //
  // A bit is sampled at either the assertion, or de-assertion
  // of the SCK signal depending on the configured Clock Phase
  //(CPHA). In either case, the data must be stable at the time
  // it is sampled.
  //
  // Both devices expect a whole, word to be exchanged per transfer,
  // so 8 SCLK transitions are expected for a byte transfer sequence.
  //
  // Since this is a data exchange between the master and the slave,
  // the send/write data is shifted out of the MOSI (Master Out Slave In)
  // port at the same time the receive/read data is shifted in from the
  // MISO (Master In Slave Out) port.
  //
  // Whether the Most Significant Bit (MSB) or the Least Significant Bit
  // (LSB) is transferred first depends on the configured bit order
  // in setBitOrder().
  //
  // There is always a received word from the slave device when a word is
  // sent as the serial ring protocol demands it. This function will always
  // return that value, but the caller may not use it depending on the device
  // and its protocol. In many device sequences there a series of commands
  // which act in one direction, and responses, reads which return meanful
  // data.
  //
  // For example, many devices internally implement a "virtual register file"
  // similar to a set of I/O ports. Commands typically look like:
  //
  // Register Read:
  //
  // transfer(READ_REGISTER_COMMAND);
  // regValue = transfer(REGISTER_NUMBER);
  //
  // Register Write:
  //
  // transfer(WRITE_REGISTER_COMMAND);
  // transfer(REGISTER_NUMBER);
  // transfer(data_value);
  //

  // The caller controls CSN select, so issue its delay here if set
  delayMicroseconds(m_csnDelay);

  shiftIn = 0;
  shiftOut = _data;

  if (m_lsbFirst) {

    for(index = 0; index < 8; index++) {

      shiftIn >>= 1;

      // Send the bit
      if (shiftOut & 0x01) {
	reg = HIGH;
      }
      else {
        reg = LOW;
      }

      //
      // Write the output bit to the slave
      //
      // Note: We hold the data stable for the
      // entire SCK transition from ASSERT to
      // IDLE. This allows either clock phase
      // configured to work since the data is stable
      // for both rising edge and falling edge.
      //
      // In addition we read the slave value while the
      // SCK is ASSERTed which works for either
      // polarity.
      //
      digitalWrite(m_mosi, reg);

      // Delay our MOSI setup time for the slave
      delayMicroseconds(m_mosiDelay);

      //clock it
      digitalWrite(m_sck, m_sckAssert);

      // Delay our MISO valid time for the slave to respond
      // before we sample it
      delayMicroseconds(m_misoDelay);

      // Read the input bit from the slave
      reg = digitalRead(m_miso);

      //
      // We clock as fast as possible.
      // Could insert a delay before the next
      // digitalWrite to slow down the clock if
      // required.
      //
      digitalWrite(m_sck, m_sckIdle);

      if (reg != LOW) {
	 shiftIn |= 0x80;
      }
      else {
	 shiftIn &= 0xFE;
      }

      shiftOut >>= 1;
    }
  }
  else {

    // MSB First
    for(index = 0; index < 8; index++) {

      shiftIn <<= 1;

      // Send the bit
      if (shiftOut & 0x80) {
	reg = HIGH;
      }
      else {
        reg = LOW;
      }

      // Write the output bit to the slave
      digitalWrite(m_mosi, reg);

      // Delay our MOSI setup time for the slave
      delayMicroseconds(m_mosiDelay);

      //clock it
      digitalWrite(m_sck, m_sckAssert);

      // Delay our MISO valid time for the slave to respond
      // before we sample it
      delayMicroseconds(m_misoDelay);

      // Read the input bit from the slave
      reg = digitalRead(m_miso);

      digitalWrite(m_sck, m_sckIdle);

      if (reg != LOW) {
	 shiftIn |= 0x01;
      }
      else {
	 shiftIn &= 0xFE;
      }

      shiftOut <<= 1;
    }
  }

  return shiftIn;
}
