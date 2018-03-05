
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
 * Smartpux (TM) Dweet support.
 *
 *  Date: 01/07/2015
 *  File: DweetArduino.cpp
 *
 *  Handle Arduino hardware manipulation similar to Firmmata.
 *
 * Used for Smartpux DWEET's.
 */

#include "MenloPlatform.h"
#include "MenloDebug.h"
#include "MenloMemoryMonitor.h"
#include "MenloNMEA0183.h"
#include "MenloDweet.h"

#include "DweetArduino.h"

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
DweetArduino::Initialize(MenloDweet* dweet)
{
    m_dweet = dweet;

    //
    // Register for unhandled Dweets
    //
    m_dweetEvent.object = this;
    m_dweetEvent.method = (MenloEventMethod)&DweetArduino::DweetEvent;

    MenloDweet::RegisterGlobalUnhandledDweetEvent(&m_dweetEvent);

    return 1;
}

unsigned long
DweetArduino::DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    MenloDweetEventArgs* dweetArgs = (MenloDweetEventArgs*)eventArgs;

    // Check memory
    MenloMemoryMonitor::CheckMemory(__LINE__);

    DBG_PRINT("DweetArduino ");

    if (ProcessArduinoCommands(dweetArgs->dweet, dweetArgs->name, dweetArgs->value) == 0) {
        // Not handled
        DBG_PRINT("DweetArduino DweetEvent NOT HANDLED");
        return MAX_POLL_TIME;
    }
    else {
        // handled
        DBG_PRINT("DweetArduino DweetEvent WAS HANDLED");
        return 0;
    }
}

//
// Summary of commands
//
// TODO: Moving to GETSTATE/SETSTATE to support
// table driven commands.
//
// Also load from EEPROM as well by re-using standard
// table driven handlers.
//
// Also less code size.
//
// SETSTATE=PINMODE:D0.INPUT
//
// PINMODE=D0:INPUT
//   D0-D13
//   A0-A7
//   INPUT/OUTPUT/INPUT_PULLUP
//
// GETSTATE=DREAD:D0
// DREAD=D0
//
// SETSTATE=DWRITE:D0
// DWRITE=D0:0
//
// SETSTATE=AREF:INTERNAL
// AREF=INTERNAL
//   DEFAULT|INTERNAL|EXTERNAL
//
// GETSTATE=AREAD:A0
// AREAD=A0
//
// SETSTATE=AWRITE:D2.00
// AWRITE=D2:00  // PWM pins only
//

//
// TODO: Support SETCONFIG to save port states to EEPROM
// for power on initialization to a known state for a specific
// project.
//
//  D0 - D13 -> INPUT, OUTPUT, INPUT_PULLUP
//
//  D0 - D13 -> digitalWrite() value
//
//  A0 - A5 -> default is analog input
//
//  AREF -> INTERNAL, EXTERNAL
//
//  PWM - analogWrite() value on pins where available and set
// 

//
// NOTE:
// Many platforms now use a mapping to the "Arduino" basic port
// functions as these have become the vocabulary for makers,
// experiementers, and students.
//
// This module is intended to support these basic mappings
// even on non-"Arduino" platforms through mapping libraries.
//
// Direct access to specific microcontroller functions, extended
// functions and extended ports are through a microcontroller
// platform specific module in addition to this one. This module
// ensures that "Arduino" style experiment continue to work on
// even newly supported platforms.
//
// The core "Arduino" library functions used are:
//
// pinMode(), digitalRead(), digitalWrite(), 
// analogReference(), analogRead(), analogWrite()
//
// Pins:
// D0 - D13 - Digitial input/output
// A0-A7 - Analog input
// PWM (analogWrite()) on pins D3, D5, D6, D9, D10, D11
//
// Larger Arduino's support additional pins and modes
// and they can be added in the future.
//

//
// TODO: Make this an application referenced optional
// module to allow opt into different platforms.
//
// Or make the symbol compiled based on MenloPlatform settings.
//

//
// TODO: Create a "string intern" table so that PSTR()'s
// only show up once. Maybe AVR GCC already does this.
// If not do it manually with a PROGMEM space table
// of the PROGMEM space strings. Squeezing every byte
// possible for "overhead" libraries makes sense no matter
// how useful they are.
//

//
// Note: Symbolic names are used here rather than
// the underlyings codes.
//
// The underlying codes would take up less space than
// the name strings, but from a Dweet client perspective
// could be problematic in maintaining the mapping from
// symbolic codes to low level numbers.
//
// This is especially problematic if you are communicating
// with "Arduino Like" platforms based on different chips
// and underlying libraries. Many Arduino symbolics are
// different if you choose different AVR/ARM chip options.
//
// By using the symbolic constants this module will be
// correct for the embedded device being targeted, and
// Dweet/Node.js programmers can use high level text names
// for ports which are more readable.
//
// Most experimenters know what Arduino port D0 or A0
// mean, but not that there codes as 0 or 14 on an AtMega328.
//
// Even finding these definitions buried deep into the Arduino
// and AVR headers is an arduous task.
//

int
ArduinoPinNameToNumber(char* name)
{
    int pin = -1;

    if (strcmp_P(name, PSTR("LED_BUILTIN")) == 0)  {
       pin = 13;
    }
    else if (strcmp_P(name, PSTR("D0")) == 0)  {
       pin = 0;
    }
    else if (strcmp_P(name, PSTR("D1")) == 0)  {
       pin = 1;
    }
    else if (strcmp_P(name, PSTR("D2")) == 0)  {
       pin = 2;
    }
    else if (strcmp_P(name, PSTR("D3")) == 0)  {
       pin = 3;
    }
    else if (strcmp_P(name, PSTR("D4")) == 0)  {
       pin = 4;
    }
    else if (strcmp_P(name, PSTR("D5")) == 0)  {
       pin = 5;
    }
    else if (strcmp_P(name, PSTR("D6")) == 0)  {
       pin = 6;
    }
    else if (strcmp_P(name, PSTR("D7")) == 0)  {
       pin = 7;
    }
    else if (strcmp_P(name, PSTR("D8")) == 0)  {
       pin = 8;
    }
    else if (strcmp_P(name, PSTR("D9")) == 0)  {
       pin = 9;
    }
    else if (strcmp_P(name, PSTR("D10")) == 0)  {
       pin = 10;
    }
    else if (strcmp_P(name, PSTR("D11")) == 0)  {
       pin = 11;
    }
    else if (strcmp_P(name, PSTR("D12")) == 0)  {
       pin = 12;
    }
    else if (strcmp_P(name, PSTR("D13")) == 0)  {
       pin = 13;
    }
#if MENLO_ESP8266
    //
    // Just a single analog input on ESP8266
    //
    else if (strcmp_P(name, PSTR("A0")) == 0)  {
       pin = A0;
    }
#else
#if MENLO_BOARD_RFDUINO
    // TODO: fill in analog values for RFDuino
    // Better: Platform abstraction for this low level debug module
#else
    else if (strcmp_P(name, PSTR("A0")) == 0)  {
       pin = A0;
    }
    else if (strcmp_P(name, PSTR("A1")) == 0)  {
       pin = A1;
    }
    else if (strcmp_P(name, PSTR("A2")) == 0)  {
       pin = A2;
    }
    else if (strcmp_P(name, PSTR("A3")) == 0)  {
       pin = A3;
    }
    else if (strcmp_P(name, PSTR("A4")) == 0)  {
       pin = A4;
    }
    else if (strcmp_P(name, PSTR("A5")) == 0)  {
       pin = A5;
    }
    else if (strcmp_P(name, PSTR("A6")) == 0)  {
       pin = A6;
    }
    else if (strcmp_P(name, PSTR("A7")) == 0)  {
       pin = A7;
    }
#endif // MENLO_BOARD_RFDUINO
#endif // MENLO_ESP8266

    return pin;
}

//
// Arduino commands processing.
//
// These are commands specific to the Arduino Uno/ATmega328P
//
// These commands allow Dweet access to ports, PWM, analog
// inputs, etc.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetArduino::ProcessArduinoCommands(MenloDweet* dweet, char* name, char* value)
{

    uint8_t pin;
    uint8_t b;
    uint16_t a;
    unsigned long address;
    unsigned long data;
    char* item = NULL;
    char* action = NULL;
    char* endptr = NULL;

    if (strncmp_P(name, PSTR("PINMODE"), 7) == 0)  {

        //
        // http://arduino.cc/en/Reference/PinMode
        // http://arduino.cc/en/Reference/Constants
        //

        // PINMODE=D0:INPUT set D0 for INPUT
        if (dweet->ProcessItemAction(value, &item, &action) == 0) {

            // error, not item:action format
            dweet->SendDweetItemReplyType_P(
                PSTR("PINMODE"),
                dweet_error_string,
                value
                );

             return 1;
        }

        pin = ArduinoPinNameToNumber(item);
        if (pin == (-1)) {

            dweet->SendDweetItemValueReplyType(
                PSTR("PINMODE"),
                dweet_error_string,
                item,
                action
                );

             return 1;
        }

        // Default safe value
        b = INPUT;

        if (strcmp_P(action, PSTR("INPUT")) == 0)  {
           b = INPUT;
        }
        else if (strcmp_P(action, PSTR("OUTPUT")) == 0)  {
           b = OUTPUT;
        }
        else if (strcmp_P(action, PSTR("INPUT_PULLUP")) == 0)  {
           b = INPUT_PULLUP;
        }
        else {

            dweet->SendDweetItemValueReplyType(
                PSTR("PINMODE"),
                dweet_error_string,
                item,
                action
                );

            return 1;
        }

        //
        // pins_arduino.h
        //
        // pin = 0 - 13  == D0 - D13
        // pin = 14 - 21 == A0 - A7
        //
        // mode = INPUT 0x0, OUTPUT 0x1, INPUT_PULLUP 0x2
        //
        pinMode(pin, b);

        dweet->SendDweetItemValueReply(
            PSTR("PINMODE"),
            item,
            action
            );
    }
    else if (strncmp_P(name, PSTR("DREAD"), 5) == 0)  {

        // DREAD=D0
        pin = ArduinoPinNameToNumber(value);
        if (pin == (-1)) {

            dweet->SendDweetItemReplyType_P(
                PSTR("DREAD"),
                dweet_error_string,
                value
                );

             return 1;
        }

        // returns = HIGH 0x1, LOW 0x0
        b = digitalRead(pin);

        // DREAD_REPLY=D0:00
        dweet->SendDweetItemValueInt8Reply(
            PSTR("DREAD"),
            value, // value is the string version of the address
            b
            );
    }
    else if (strncmp_P(name, PSTR("DWRITE"), 6) == 0)  {

        // DWRITE=D0:0       write data to port D0, value 0
        if (dweet->ProcessItemAction(value, &item, &action) == 0) {

            // error, not item:action format
            dweet->SendDweetItemValueReplyType(
                PSTR("DWRITE"),
                dweet_error_string,
                item,
                action
                );

             return 1;
        }

        pin = ArduinoPinNameToNumber(item);
        if (pin == (-1)) {

            dweet->SendDweetItemValueReplyType(
                PSTR("DWRITE"),
                dweet_error_string,
                item,
                action
                );

            return 1;
        }

        data  = strtoul(action, &endptr, 16);
        if (endptr == action) {

            // conversion failure
            dweet->SendDweetItemValueReplyType(
                PSTR("DWRITE"),
                dweet_error_string,
                item,
                action
                );

            return 1;
        }
        else {
            b = (uint8_t)data;

            // value = HIGH 0x1, LOW 0x0
            digitalWrite(pin, b);

            dweet->SendDweetItemValueReply(
                PSTR("DWRITE"),
                item,
                action
                );
        }
    }
    else if (strncmp_P(name, PSTR("AREF"), 4) == 0)  {

       // AREF=INTERNAL   set analog reference to INTERNAL

       // type = DEFAULT, INTERNAL, INTERNAL1V1, INTERNAL2V56, EXTERNAL
       if (strcmp_P(value, PSTR("DEFAULT")) == 0)  {
           b = DEFAULT;
       }
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
       else if (strcmp_P(value, PSTR("INTERNAL1V1")) == 0)  {
           b = INTERNAL1V1;
       }
       else if (strcmp_P(value, PSTR("INTERNAL2V56")) == 0)  {
           b = INTERNAL2V56;
       }
#else
#if ARDUINO_ARCH_AVR
       else if (strcmp_P(value, PSTR("INTERNAL")) == 0)  {
           b = INTERNAL;
       }
       else if (strcmp_P(value, PSTR("EXTERNAL")) == 0)  {
           b = EXTERNAL;
       }
#endif
#endif
       else {

            // Uknown
            dweet->SendDweetItemReplyType_P(
                PSTR("AREF"),
                dweet_error_string,
                value
                );

             return 1;
       }

        //
        // http://arduino.cc/en/Reference/AnalogReference
        //
#if ARDUINO_ARCH_SAM
        //
        // packages/arduino/hardware/sam/cores/arduino/wiring_analog.h
        //
        // typedef enum _eAnalogReference
        // {
        //   AR_DEFAULT,
        // } eAnalogReference ;
        //
        // extern void analogReference( eAnalogReference ulMode ) ;
        //
        analogReference(AR_DEFAULT);
#else
        //
        // packages/arduino/hardware/avr/cores/arduino/Arduino.h
        //
        // void analogReference(uint8_t mode);
        //
        analogReference(b);
#endif

        dweet->SendDweetItemReply(
            PSTR("AREF"),
            value
            );
    }
    else if (strncmp_P(name, PSTR("AREAD"), 5) == 0)  {

        // AREAD=A0
        pin = ArduinoPinNameToNumber(value);
        if (pin == (-1)) {

            dweet->SendDweetItemReplyType_P(
                PSTR("AREAD"),
                dweet_error_string,
                value
                );

             return 1;
        }

        // pin = 14 - 21 == A0 - A7
        // returns 0 - 1023
        a = analogRead(pin);

        // AREAD_REPLY=A0:0000
        dweet->SendDweetItemValueInt8Reply(
            PSTR("AREAD"),
            value, // value is the string version of the address
            a
            );
    }
    else if (strncmp_P(name, PSTR("AWRITE"), 6) == 0)  {

        //
        // http://arduino.cc/en/Reference/AnalogWrite
        //

        // AWRITE=D2:0  set PWM on pin D2, rate 0
        if (dweet->ProcessItemAction(value, &item, &action) == 0) {

            // error, not item:action format
            dweet->SendDweetItemValueReplyType(
                PSTR("AWRITE"),
                dweet_error_string,
                item,
                action
                );

             return 1;
        }

        pin = ArduinoPinNameToNumber(item);
        if (pin == (-1)) {

            dweet->SendDweetItemValueReplyType(
                PSTR("AWRITE"),
                dweet_error_string,
                item,
                action
                );

             return 1;
        }

        data  = strtoul(action, &endptr, 16);
        if (endptr == action) {

            // conversion failure
            dweet->SendDweetItemValueReplyType(
                PSTR("AWRITE"),
                dweet_error_string,
                item,
                action
                );

            return 1;
        }

        b = (uint8_t)data;

        // value = 0 - 255
        analogWrite(pin, b);

        dweet->SendDweetItemValueReply(
            PSTR("AWRITE"),
            item,
            action
            );
    }
    else {

        // Continue looking for a handler
        return 0;
    }

    return 1;
}
