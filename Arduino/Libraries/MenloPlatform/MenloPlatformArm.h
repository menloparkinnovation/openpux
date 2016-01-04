
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
 *  Date: 05/09/2015
 *
 *  Platform support for ARM Processors.
 *
 */

#ifndef MenloPlatformArm_h
#define MenloPlatformArm_h

#if MENLO_ARM32

// Code space strings "F(x)" macro is missing in the Due port
#define F(string) string

//
// EEPROM emulation functions
//

uint8_t eeprom_read_byte(const uint8_t*);
void eeprom_write_byte(const uint8_t*, uint8_t);

//
// Redefines for _P version of string routines
//

#define strncmp_P strncmp

#define Print_P Print

#endif

#endif // MenloPlatformArm_h
