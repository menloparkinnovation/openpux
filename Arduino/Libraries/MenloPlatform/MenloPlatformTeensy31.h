
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
 *  Date: 06/01/2015
 *
 *  Platform support for Teensy 3.1.
 *
 */

#ifndef MenloPlatformTeensy31_h
#define MenloPlatformTeensy31_h

//
// Teensy is weird. It's an ARM processor announcing its self as AVR
//

#if MENLO_BOARD_TEENSY31

// 32 bit ARM processor
#define MENLO_ARM32 1

//
// Teensy31 supports EEPROM emulation functions
//
#define IMPLEMENTS_EEPROM_ROUTINES 1

//
// Lots of build warnings with the following definitions.
//
// But if they are taken out there are build breaks on
// FlashStringHelper* definitions in Menlo Code. So sort
// this out.
//
// Though it currently builds with warnings at every string
// constant, the code still works.
//

// Code space strings "F(x)" macro is supported by Teensy31
#define F(string) string

//
// Redefines for _P version of string routines
//

#define strncmp_P strncmp

#define Print_P Print

#endif // MENLO_BOARD_TEENSY31

#endif // MenloPlatformTeensy31_h
