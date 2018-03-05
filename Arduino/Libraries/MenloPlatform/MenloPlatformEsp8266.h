
/*
 * Copyright (C) 2016 Menlo Park Innovation LLC
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
 *  Date: 05/26/2016
 *
 *  Platform support for ESP8266 Processors.
 *
 */

#ifndef MenloPlatformEsp8266_h
#define MenloPlatformEsp8266_h

//
// ESP8266 Arduino plug in support.
//
// 32 bit architecture.
// Non-ARM, non-AVR.
// Uses AVR program space for read-only data/strings in flash.
// Moderate memory sizes. Similar or better to Particle Photon.
//
// Import compiler options/defines
//
// -DARDUINO_ARCH_ESP8266
// -DESP8266
// - ARDUINO_ESP8266_THING_DEV // SparkFun specific
//

#if MENLO_ESP8266

// A larger memory processor (not an AtMega 328 with 2k RAM/32K flash)
#define BIG_MEM 1

// Code space strings "F(x)" macro is missing
#define F(string) string

//
// EEPROM emulation functions
//
// Note: On the ESP8266 startup/shutdown style transactions
// are required to commit EEPROM space.
//
// This support must be added to the platform eeprom routines.
//

uint8_t eeprom_read_byte(const uint8_t*);
void eeprom_write_byte(const uint8_t*, uint8_t);

#endif // MENLO_ESP8266

#endif // MenloPlatformEsp8266_h
