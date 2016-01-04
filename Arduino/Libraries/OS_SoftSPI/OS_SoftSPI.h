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
 */

#ifndef _OS_SOFTSPI_H_INCLUDED
#define _OS_SOFTSPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>
#include <avr/pgmspace.h>

//
// This class is designed to not collide with SPI.h
//
// Common usage is to include both SPI.h and SoftSPI.h
// in a project, for accessing low speed devices in the
// presence of a WiFi shield which "parks" on the SPI
// bus.
//

//#define SPI_CLOCK_DIV4 0x00
//#define SPI_CLOCK_DIV16 0x01
//#define SPI_CLOCK_DIV64 0x02
//#define SPI_CLOCK_DIV128 0x03
//#define SPI_CLOCK_DIV2 0x04
//#define SPI_CLOCK_DIV8 0x05
//#define SPI_CLOCK_DIV32 0x06
//#define SPI_CLOCK_DIV64 0x07

//
// We use these values, but the project may not include SPI.h
// so we use compatible values to allow drop in replacement.
//

#ifndef SPI_MODE0
#define SPI_MODE0 0x00
#endif

#ifndef SPI_MODE1
#define SPI_MODE1 0x04
#endif

#ifndef SPI_MODE2
#define SPI_MODE2 0x08
#endif

#ifndef SPI_MODE3
#define SPI_MODE3 0x0C
#endif

//#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
//#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
//#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

//
// Note: Unlike Arduino SPI this class is an instance class
// which allows multiple usage, and programmer control
// of the storage type.
//

class OS_SoftSPIClass {
public:
  byte transfer(byte _data);

  //
  // SPI Configuration methods
  //

  //
  // These are undefined so a project can identify usage of
  // interrupt mode. (rare)
  //
  // void attachInterrupt();
  // void detachInterrupt(); // Default

  //
  // Configure is required before first transfer/begin()
  // to identify the pins you will use.
  //
  void configure(byte mosi, byte miso, byte sck);

  void begin(); // Default
  void end();

  void setBitOrder(uint8_t);

  //
  // These inform the software as to your intentions, but
  // there is no clock rate guaranteed.
  //
  void setDataMode(uint8_t);
  void setClockDivider(uint8_t);

  // Interface timings are in microseconds
  void setInterfaceTimings(unsigned int csnDelay,
			   unsigned int mosiDelay,
                           unsigned int misoDelay);

private:

  // Pin numbers
  uint8_t m_mosi;
  uint8_t m_miso;
  uint8_t m_sck;

  // Operational values changed by mode setup
  uint8_t m_sckIdle;
  uint8_t m_sckAssert;
  bool m_lsbFirst;

  // Clocking
  uint8_t m_csnDelay;
  uint8_t m_mosiDelay;
  uint8_t m_misoDelay;
};

//
// MenloNote:
//
// I really don't like this class design of a static global
// as I prefer to allow more flexibility by the project
// implementer. So this is commented out by default.
//
// If you are only supporting one (or the first) SoftSPI
// and you want to make minimal changes to your Arduino
// project, uncomment the line below, and the similar
// line in SoftSPI.cpp.
//

// Uncomment to enable global SoftSPI
//extern OS_SoftSPIClass SoftSPI;

//
// Adding SoftSPI to your existing Ardunio Project
//
// - Add OS_SoftSPIClass SoftSPI to your project.
// - Call SoftSPI.configure(mosi, miso, sclk) to set the pins you are using
// - Change occurances of SPI.func() to SoftSPI.func()
// - If you use SPI.attachInterrupt()/SPI.detachInterrupt() configure
//   your project for non-interrupt mode
//

#endif
