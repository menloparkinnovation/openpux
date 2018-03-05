
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
 *  Platform support for AVR (AtMega) processos
 *
 */

#include "MenloPlatform.h"

//
// The Arduino IDE is not very great at selecting
// per platform files so we just use a #ifdef on each
// platform specific file.
//
// They are separate rather than one since that is more
// maintainable.
//

//
// Experimental feature
//
// Note: Something in the Arduino environment is already claiming
// this vector.
//
#define WATCHDOG_INTERRUPT_ENABLED 0

#if MENLO_ATMEGA

#if WATCHDOG_INTERRUPT_ENABLED

// Program counter size differs from AtMega328 and "Mega" with large code space.
#if defined(__AVR_ATmega2560__)
#define PROGRAM_COUNTER_SIZE 3 // size in bytes
#else
#define PROGRAM_COUNTER_SIZE 2 // size in bytes
#endif

//
// Capture the PC when the watchdog interrupt goes off
//
// Note: byte order is reversed on AtMega's
//
unsigned long g_watchdogPC = 0;

//
// This is invoked by the watchdog interrupt which can be
// used as an early warning
//
// Symbol for linking is:
//
// __vector_6
//
ISR(WDT_vect, ISR_NAKED)
{
    register uint8_t* stackptr;

    // get stack pointer
    stackptr = (uint8_t*)SP;

    // AVR ISR's have the return address just above the current SP
    stackptr++;

    // Capture the previous PC that got interrupted
    memcpy(&g_watchdogPC, stackptr, PROGRAM_COUNTER_SIZE);

    //
    // TODO: Write diagnostics to the EEPROM to have the error PC value
    // survive the reset
    //
    // MenloConfigStore.h
    // EEPROM_DIAGNOSTICS_ERROR_CODE
    // EEPROM_DIAGNOSTICS_ERROR_PC
    // EEPROM_DIAGNOSTICS_ERROR_WATCHDOG
    //
}

#endif // WATCHDOG_INTERRUPT_ENABLED

void EnableWatchdog()
{
  wdt_enable(WDTO_8S);

#if WATCHDOG_INTERRUPT_ENABLED
  // Enable Watchdog ISR
  WDTCSR |= _BV(WDIE);
#endif
}

void ResetWatchdog()
{
  wdt_reset(); // Tell watchdog we are still alive
}

//
// Improve: Support "far" memory addressing for the
// "Mega" processor series.
//

char*
MenloPlatform::GetStringPointerFromStringArray(char** array, int index)
{
    PGM_P p;
    PGM_P array_p;

    array_p = (PGM_P)array;

    //
    // Calculate our effective address into the program
    // space table that is an array of string* also in program space.
    //
    p = array_p + (index * sizeof(char*));

    //
    // Improve: On AtMega's with larger than 16 bit addressing
    // this must be updated.
    //
    p = (PGM_P)pgm_read_word(p);

    return (char*)p;
}

unsigned long
MenloPlatform::GetMethodPointerFromMethodArray(char** array, int index)
{
    PGM_P p;
    PGM_P array_p;
    unsigned long methodptr;

    array_p = (PGM_P)array;

    //
    // Calculate our effective address into the program
    // space table that is an array of string* also in program space.
    //
    p = array_p + (index * sizeof(unsigned long));

    //
    // Note: C++ Method pointers declared as the following allocate
    // (2) pointer sized entries. The first is the 16 bit function
    // address, while the second is a compiler specific encoding for
    // the type of method. Since these are straightforward non-virtual
    // methods this information is all 0's for the current version of GCC
    // for AVR/Arduino.
    //
    // Here we load as a unsigned long 32 bit value of which represents
    // the function address. Most callers on AVR will truncate this
    // to a 16 bit target address ignoring the upper 0's.
    //

    methodptr = pgm_read_dword(p);

    return methodptr;
}

#endif // MENLO_ATMEGA
