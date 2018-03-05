
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

#include <Arduino.h>
#include "MenloPlatform.h"

#if defined(ESP8266)

#include "EEPROM.h"

void EnableWatchdog()
{
}

void ResetWatchdog()
{
    // Let esp8266 tasks run
    yield();
}

char*
MenloPlatform::GetStringPointerFromStringArray(char** array, int index)
{
    //
    // Uniform 32 bit address space, even if strings are in readonly memory
    //
    // Note: They must be 4 byte reads on 4 byte boundaries, so use the PGM
    // copy functions which takes care of the alignment restrictions.
    //
    // Otherwise exception (3) or exception (9) will occur.
    //
    return array[index];
}

unsigned long
MenloPlatform::GetMethodPointerFromMethodArray(char** array, int index)
{
    //
    // Uniform 32 bit address space, even if strings are in readonly memory
    //
    // Function pointers == char*
    //
    // Note: They must be 4 byte reads on 4 byte boundaries, so use the PGM
    // copy functions which takes care of the alignment restrictions.
    //
    // Otherwise exception (3) or exception (9) will occur.
    //

    //
    // ESP8266/Tensilica L106 works as of 06/30/2016 using this layout
    // with GCC.
    //
    // Note: C++ Method pointers declared as the following allocate
    // (2) pointer sized entries. The first is the 32 bit function
    // address, while the second is a compiler specific encoding for
    // the type of method. Since these are straightforward non-virtual
    // methods this information is all 0's for the current version of GCC
    // for ESP8266/Tensilica L106.
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

//
// EEPROM emulation
//
// https://github.com/esp8266/Arduino/tree/master/libraries/EEPROM/examples
//

//
// ESP8266 EEPROM requires first time initilization on boot up
//
bool g_eepromInitialized = false;

void
eeprom_initialize()
{
    if (g_eepromInitialized) {
        return;
    }

    g_eepromInitialized = true;

    // Emulate 1024 byte EEPROM
    EEPROM.begin(1024);

    // Note: EEPROM.end() makes it inaccessible.
}

uint8_t
eeprom_read_byte(const uint8_t* index)
{
    eeprom_initialize();

    return EEPROM.read((int)index);
}

//
// Note: Block writes are required on newer eeprom emulations
// to keep from hammering the block addressed flash used to emulate
// single byte EEPROM entries.
//
void
eeprom_write_byte(const uint8_t* index, uint8_t value)
{
    eeprom_initialize();

    EEPROM.write((int)index, value);

    //
    // Need to commit. This is bad for where as a more block oriented
    // update is desired.
    //
    EEPROM.commit();
    delay(100);
}

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

#endif // MENLO_ESP8266
