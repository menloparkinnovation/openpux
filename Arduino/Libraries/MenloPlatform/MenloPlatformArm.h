
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

//
// EEPROM emulation functions
//

uint8_t eeprom_read_byte(const uint8_t*);
void eeprom_write_byte(const uint8_t*, uint8_t);

#if MENLO_BOARD_RFDUINO
// RFDuino has its own emulation for these routines
#define strncmp_P strncmp
#else

//
// Redefines for _P version of string routines
//
// Used by ARM DUE, etc.
//

#define strncmp_P strncmp

// TODO: Does not build for SparkCore/ARM
//#define Print_P Print
#endif

#if REQUIRES_PGM_ROUTINES

#define PGM_P const char *

uint16_t pgm_read_word(const int* ptr);

void* pgm_read_ptr(const int* ptr);

#endif // REQUIRES_EEPROM_ROUTINES

#endif // MENLO_ARM32

#endif // MenloPlatformArm_h
