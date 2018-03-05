
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
 *  Platform support for ARM32.
 *
 */

#include "MenloPlatform.h"

#if MENLO_ARM32

#if MENLO_BOARD_SPARKCORE
void EnableWatchdog()
{
}

void ResetWatchdog()
{
}
#else
void EnableWatchdog()
{
}

void ResetWatchdog()
{
}
#endif // MENLO_BOARD_SPARKCORE

char*
MenloPlatform::GetStringPointerFromStringArray(char** array, int index)
{
    // Uniform 32 bit address space, even if strings are in readonly memory
    return array[index];
}

unsigned long
MenloPlatform::GetMethodPointerFromMethodArray(char** array, int index)
{
    //
    // Uniform 32 bit address space, even if strings are in readonly memory
    // Function pointers == char*
    //

    //
    // Note: C++ Method pointers declared as the following allocate
    // (2) pointer sized entries. The first is the 32 bit function
    // address, while the second is a compiler specific encoding for
    // the type of method. Since these are straightforward non-virtual
    // methods this information is all 0's for the current version of GCC
    // for ARM/Arduino.
    //
    // As a result every other 32 bit entry must be skipped.
    //
    // typedef int (DweetRadio::*StateMethod)(char* buf, int size, bool isSet);
    //
    // PROGMEM const StateMethod radio_function_table[] =
    // {
    //     &DweetRadio::ChannelHandler,
    //     &DweetRadio::TxAddrHandler,
    //     &DweetRadio::RxAddrHandler,
    //     &DweetRadio::PowerHandler,
    //     &DweetRadio::OptionsHandler,
    //     &DweetRadio::GatewayHandler
    // };
    //
    // 00085300 <_ZL20radio_function_table>:
    //   85300:	000825a7 00000000 000825b3 00000000     .%.......%......
    //   85310:	000825bf 00000000 000825cb 00000000     .%.......%......
    //   85320:	000825d7 00000000 000825e3 00000000     .%.......%......
    //   85330:	000826b1 00000000                       .&......
    //

    // Multiply index by 2 to skip the NULL entries
    return (unsigned long)array[index*2];
}

#if !defined(IMPLEMENTS_EEPROM_ROUTINES)

//
// EEPROM emulation
//
// Note: This differs for each ARM processor
//

#if MENLO_BOARD_SPARKCORE

uint8_t
eeprom_read_byte(const uint8_t* index)
{
    return EEPROM.read((int)index);
}

void
eeprom_write_byte(const uint8_t* index, uint8_t value)
{
    //
    // update is used instead of write since it reads the location
    // and avoids the write if the value matches.
    //
    // Better for wear leveling.
    //
    EEPROM.update((int)index, value);
}

#else

//
// Basic implementation for runtime store/retrieval.
//
// ARM versions have plenty of RAM for the emulation array
//

// Note: This is zero init
static uint8_t eeprom_emulation[1024] = { 0 };

uint8_t
eeprom_read_byte(const uint8_t* index)
{
    return eeprom_emulation[(int)index];
}

void
eeprom_write_byte(const uint8_t* index, uint8_t value)
{
    eeprom_emulation[(int)index] = value;
}
#endif // EEPROM emulation

#endif // MENLO_BOARD_SPARKCORE


#if REQUIRES_PGM_ROUTINES

uint16_t
pgm_read_word(const int* ptr)
{
    const uint16_t* p = (const uint16_t*)ptr;

    return *p;
}

//
// Address is to a pointer word in memory
//
void*
pgm_read_ptr(const int* ptr)
{
    char** p = (char**)ptr;

    return *p;
}

#endif // PGM emulation

#endif // MENLO_ARM32
