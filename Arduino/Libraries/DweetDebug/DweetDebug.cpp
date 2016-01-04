
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
 *  Date: 04/26/2015
 *  File: DweetDebug.h
 *
 *  Handle memory/register/EEPROM/flash  manipulation to support debuggers.
 *
 * Used for Smartpux DWEET's.
 */

#include "MenloPlatform.h"
#include "MenloDebug.h"
#include "MenloMemoryMonitor.h"
#include "MenloNMEA0183.h"
#include "MenloDweet.h"

#include "DweetDebug.h"

#define MAX_READ_LENGTH 16
#define MAX_WRITE_LENGTH 16

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

int
DweetDebug::Initialize(MenloDweet* dweet)
{
    m_dweet = dweet;

    //
    // Register for unhandled Dweets
    //
    m_dweetEvent.object = this;
    m_dweetEvent.method = (MenloEventMethod)&DweetDebug::DweetEvent;
    m_dweet->RegisterUnhandledDweetEvent(&m_dweetEvent);

    return 1;
}

unsigned long
DweetDebug::DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    MenloDweetEventArgs* dweetArgs = (MenloDweetEventArgs*)eventArgs;

    // Check memory
    MenloMemoryMonitor::CheckMemory(__LINE__);

    DBG_PRINT("DweetDebug ");

    if (ProcessDebugCommands(dweetArgs->dweet, dweetArgs->name, dweetArgs->value) == 0) {
        // Not handled
        DBG_PRINT("DweetDebug DweetEvent NOT HANDLED");
        return MAX_POLL_TIME;
    }
    else {
        // handled
        DBG_PRINT("DweetDebug DweetEvent WAS HANDLED");
        return 0;
    }
}

//
// Command summary
//
// TODO: Transitioning to table driven GETSTATE/SETSTATE
//
// This is implemented in DweetConfig.cpp now
// SETSTATE=TRACELEVEL:00
//
// GETSTATE=MEMSTATS
//
//                  addr length
// GETSTATE=MEMREAD:0000.01
//
// see MEMWRITE for details
// SETSTATE=MEMWRITE:0000.0000
//


//
// TODO: Abstract flash, eeprom in MenloPlatform for general
// purpose debugging, or break out into AVR debug functions.
//

//
// Debug commands processing.
//
// These commands allow Dweet access to memory, registers, etc.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetDebug::ProcessDebugCommands(MenloDweet* dweet, char* name, char* value) {

    int index;
    int a;
    int length;
    volatile char* ptr;
    char* endptr;
    unsigned long address;
    unsigned long data;
    uint8_t buf2[MAX_READ_LENGTH + 1];

    char* item = NULL;
    char* action = NULL;

    // SETTRACELEVEL=00
    if (strncmp_P(name, PSTR("SETTRACELEVEL"), 13) == 0)  {

        endptr = NULL;
        data  = strtoul((const char*)value, &endptr, 16);
        if (endptr == value) {
            // conversion failure
            dweet->SendDweetItemReplyType_P(
                PSTR("SETTRACELEVEL"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // value is tracelevel from 0-255
        MenloDebug::SetTraceLevel((uint8_t)data);

        dweet->SendDweetItemReply(
            PSTR("SETTRACELEVEL"),
            value
            );
    }
    else if (strncmp_P(name, PSTR("MEMSTATS"), 8) == 0)  {

        // value is not looked at
        MenloMemoryMonitor::ReportMemoryUsage(__LINE__);

        dweet->SendDweetItemReply(
            PSTR("MEMSTATS"),
            value
            );
    }
    else if (strncmp_P(name, PSTR("MEMREAD"), 7) == 0)  {

        // MEMREAD=0000:01
        // MEMREAD=addr:length
        if (dweet->ProcessAddressValueStrings(value, &address, &data, &item, &action) == 0) {

            // format error
            dweet->SendDweetItemReplyType_P(
                PSTR("MEMREAD"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // item string is the ASCII address
        // action string is the ASCII data value

        // Data is the read length
        length = (int)data;

        // 16 bytes max
        if (length > MAX_READ_LENGTH) {
            dweet->SendDweetItemValueReplyType(
                PSTR("MEMREAD"),
                dweet_error_string,
                item,
                action
                );

	    return 1;
        }

        //
        // Note: On hardware with word alignment requirements this
        // would unfold the byte transfers to word/quadword transfers
        // and impose the required restrictions on address alignments
        // and lengths.
        //
        // Hopefully those architectures can afford the extra code space
        // for the more complex transfer routine. Many do offer slower
        // byte access routines/macro's/instructions which are fine for
        // a debugging oriented protocol.
        //
        // MEMREAD/MEMWRITE is not for hardware registers which do
        // have specific alignment and transfer size requirements.
        // Those are served by REGREAD/REGWRITE.
        //
        // Note the return string is in the order of bytes read,
        // not in any "big endian, little endian" order. The user
        // agent is responsible for apply any semantics.
        //
        // Example: four bytes from address 0 - 3 whose values
        // are 0x01, 0x02, 0x03, 0x04 would be sent as
        // MEMREAD_REPLY=0:01020304
	//

        ptr = (char*)address;

        for (index = 0; index < length; index++) {
            buf2[index] = *ptr;
            ptr++;
        }

        // Buffer has a null on the end
        buf2[index] = '\0';

        // MEMREAD_REPLY=0000:00
        dweet->SendDweetItemValueInt8StringReply(
            PSTR("MEMREAD"),
            item,
            &buf2[0], // buffer has length + 1 for NULL
            length    // length does not include the storage for NULL
            );
    }
    else if (strncmp_P(name, PSTR("MEMWRITE"), 8) == 0)  {

        //
        // MEMWRITE=0000:0000
        // Value is a byte string with the left most characters the lower address.
        // It is not a formatted multi-byte type.
        //
        if (dweet->ProcessAddressValueStrings(value, &address, NULL, &item, &action) == 0) {

            // format error
            dweet->SendDweetItemReplyType_P(
                PSTR("MEMWRITE"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // item string is the ASCII address
        // action string is the ASCII data value

        //
	// Count the length of the data string to get number of
        // bytes to transfer.
        //
        length = strlen(action);

        // 16 bytes max, and can't be an odd count
        if ((length % 2) || (length > MAX_WRITE_LENGTH)) {
            dweet->SendDweetItemValueReplyType(
                PSTR("MEMWRITE"),
                dweet_error_string,
                item,
                action
                );

	    return 1;
        }

        // address == address for reference
        // action == string of ASCII hex characters representing data to store
        ptr = (char*)address;

        for (index = 0; index < length; index += 2) {

            buf2[0] = action[index];
            buf2[1] = action[index+1];
            buf2[2] = '\0';
            
            endptr = NULL;
            data  = strtoul((const char*)&buf2[0], &endptr, 16);
            if (endptr == (char*)&buf2[0]) {
                // conversion failure
                dweet->SendDweetItemValueReplyType(
                    PSTR("MEMWRITE"),
                    dweet_error_string,
                    item,
                    action
                    );

                return 1;
            }

            *ptr = (char)data;

            ptr++;
        }

        dweet->SendDweetItemValueReplyType(
            PSTR("MEMWRITE"),
            dweet_reply_string,
            item,
            action
            );
    }
    else if (strncmp_P(name, PSTR("EEPROMREAD"), 10) == 0)  {

        // EEPROMREAD=0000
        if (dweet->ProcessAddressValueStrings(value, &address, &data, &item, &action) == 0) {

            // format error
            dweet->SendDweetItemReplyType_P(
                PSTR("EEPROMREAD"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // Data is the read length
        length = (int)data;

        // 16 bytes max
        if (length > MAX_READ_LENGTH) {
            dweet->SendDweetItemValueReplyType(
                PSTR("EEPROMREAD"),
                dweet_error_string,
                item,
                action
                );

	    return 1;
        }

        ptr = (char*)address;

        for (index = 0; index < length; index++) {
            buf2[index] = eeprom_read_byte((unsigned char*)ptr);
            ptr++;
        }

        // Buffer has a null on the end
        buf2[index] = '\0';

        // EEPROMREAD_REPLY=0000:00
        dweet->SendDweetItemValueInt8StringReply(
            PSTR("EEPROMREAD"),
            item,
            &buf2[0], // buffer has length + 1 for NULL
            length    // length does not include the storage for NULL
            );
    }
    else if (strncmp_P(name, PSTR("EEPROMWRITE"), 11) == 0)  {

        // EEPROMWRITE=0000:0000
        if (dweet->ProcessAddressValueStrings(value, &address, NULL, &item, &action) == 0) {

            // format error
            dweet->SendDweetItemReplyType_P(
                PSTR("EEPROMWRITE"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // item string is the ASCII address
        // action string is the ASCII data value

        //
	// Count the length of the data string to get number of
        // bytes to transfer.
        //
        length = strlen(action);

        // 16 bytes max, and can't be an odd count
        if ((length % 2) || (length > MAX_WRITE_LENGTH)) {
            dweet->SendDweetItemValueReplyType(
                PSTR("EEPROMWRITE"),
                dweet_error_string,
                item,
                action
                );

	    return 1;
        }

        // address == address for reference
        // action == string of ASCII hex characters representing data to store
        ptr = (char*)address;

        for (index = 0; index < length; index += 2) {

            buf2[0] = action[index];
            buf2[1] = action[index+1];
            buf2[2] = '\0';
            
            endptr = NULL;
            data  = strtoul((const char*)&buf2[0], &endptr, 16);
            if (endptr == (char*)&buf2[0]) {
                // conversion failure
                dweet->SendDweetItemValueReplyType(
                    PSTR("EEPROMWRITE"),
                    dweet_error_string,
                    item,
                    action
                    );

                return 1;
            }

            eeprom_write_byte((unsigned char*)ptr, (uint8_t)data);

            ptr++;
        }

        dweet->SendDweetItemValueReplyType(
            PSTR("EEPROMWRITE"),
            dweet_reply_string,
            item,
            action
            );
    }
    else if (strncmp_P(name, PSTR("REGREAD"), 7) == 0)  {

        //
        // the 32 byte wide registers are mapped into the first
        // 32 bytes of the address space (0x0000 - 0x001f)
        //
        // The ports occupy the next 64 bytes of the address space.
        //
        // The first 64 ports are the AVR control registers.
        //
        // They are accessed with I/O Instructions (IN, OUT)
        // from 0x0 - 0x40, and memory instructions (LD, ST)
        // from 0x20 - 0x60.
        //

        //
        // All RAM locations start at 0x60 since the first
        // part of the RAM address space is the registers
        // and ports.
        //

        // REGREAD=0000
        if (dweet->ProcessAddressValueStrings(value, &address, &data, &item, &action) == 0) {

            // format error
            dweet->SendDweetItemReplyType_P(
                PSTR("REGREAD"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // Data is the read length
        length = (int)data;

        // 16 bytes max
        if (length > MAX_READ_LENGTH) {
            dweet->SendDweetItemValueReplyType(
                PSTR("REGREAD"),
                dweet_error_string,
                item,
                action
                );

	    return 1;
        }

        //
        // AVR's have memory mapped registers, so memory mapped
        // accesses are used.
        //
        // NOTE: The registers are byte addressed on AVR's so byte accesses
        // are used.
        //
        // Architectures with word oriented registers would perform proper
        // validation/decoding of the requests to perform the proper
        // register alignments.
        //

	/*
	    //
	    // hardware/tools/avr/avr/include/avr/sft_defs.h
	    //
	    // This is the memory address map (lds, sts instructions)
	    0x00 - 0x1F - 32 8 bit general purpose registers
	    0x20 - 0x60 - special function registers, "I/O ports"
	    0x61 - 0x7FFF - on board RAM

	    Note: I/O Access instructions (in, out) access the range from
	    0x40 - 0x60. So address 0 to an I/O instruction is memory address
	    0x20. I/O instructions are limited to a maximum address of 0x40,
	    which maps to memory address 0x60.

	    Depending on whether _SFR_ASM_COMPAT is set or not the GCC
	    compiler will generate low memory access I/O instructions automatically
	    when I/O range addresses are indicated from constants. This is because
	    they are a short form of instruction.
	*/

	//
        // Given the above, we just rely on the compiler.
	//
        for (index = 0; index < length; index++) {

            a = (unsigned int)address;

            if ((a >= 0x0) && (a <= 0x60)) {
   	        // In this range is a memory read instruction
                ptr = (char*)address;
                buf2[index] = *ptr;
            }
            else {
	        // we don't understand the range, do nothing
	        buf2[index] = 0;
            }

            address++;
        }

        // Buffer has a null on the end
        buf2[index] = '\0';

        // REGREAD_REPLY=0000:00
        dweet->SendDweetItemValueInt8StringReply(
            PSTR("REGREAD"),
            item,
            &buf2[0], // buffer has length + 1 for NULL
            length  // length does not include the storage for NULL
            );
    }
    else if (strncmp_P(name, PSTR("REGWRITE"), 8) == 0)  {

        // REGWRITE=0000:0000
        if (dweet->ProcessAddressValueStrings(value, &address, &data, &item, &action) == 0) {

            // format error
            dweet->SendDweetItemReplyType_P(
                PSTR("REGWRITE"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // We only allow single byte register writes
        length = strlen(action);
        if (length != 2) {
            dweet->SendDweetItemValueReplyType(
                PSTR("REGWRITE"),
                dweet_error_string,
                item,
                action
                );
        }

        a = (unsigned int)address;
        if ((a >= 0x0) && (a <= 0x60)) {
	    // In this range is a memory read instruction
            ptr = (char*)address;
            *ptr = (uint8_t)data;
        }
        else {
	    // we don't understand the range, do nothing
        }

        dweet->SendDweetItemValueReplyType(
            PSTR("REGWRITE"),
            dweet_reply_string,
            item,
            action
            );
    }
    else if (strncmp_P(name, PSTR("FLASHREAD"), 9) == 0)  {

        // FLASHREAD=0000
        if (dweet->ProcessAddressValueStrings(value, &address, &data, &item, &action) == 0) {

            // format error
            dweet->SendDweetItemReplyType_P(
                PSTR("FLASHREAD"),
                dweet_error_string,
                value
                );

            return 1;
        }

        // Data is the read length
        length = (int)data;

        // 16 bytes max
        if (length > MAX_READ_LENGTH) {
            dweet->SendDweetItemValueReplyType(
                PSTR("FLASHREAD"),
                dweet_error_string,
                item,
                action
                );

	    return 1;
        }

        for (index = 0; index < length; index++) {

            // Program space is the flash on Atmega's
            buf2[index] = pgm_read_byte_near((uint16_t)address);

            address++;
        }

        // Buffer has a null on the end
        buf2[index] = '\0';

        // FLASHREAD_REPLY=0000:00
        dweet->SendDweetItemValueInt8StringReply(
            PSTR("FLASHREAD"),
            item,
            &buf2[0], // buffer has length + 1 for NULL
            length    // length does not include the storage for NULL
            );
    }
    else if (strncmp_P(name, PSTR("FLASHWRITE"), 10) == 0)  {

      //
      // FLASHWRITE is a more involved process that must
      // be done in blocks from a buffer. We don't do this here.
      //

      //
      // Continue looking for a handler. If none it will
      // automatically respond with _UNSUP.
      //
      return 0;
    }
    else {

        // Continue looking for a handler
        return 0;
    }

    return 1;
}
