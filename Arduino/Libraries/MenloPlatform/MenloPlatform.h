
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
 *  Date: 03/19/2014
 *
 *  Platform support for Arduino.
 *
 *  Allows differences between the AVR and ARM platforms
 *  to be dealt with.
 */

#ifndef MenloPlatform_h
#define MenloPlatform_h

#if defined(ARDUINO) && ARDUINO >= 100

//
// Arduino IDE environments, or close emulations.
//
// Arduino (AtMega328) and family
// Arduino Due (ARM SAM) and family
// RFDuino 32 bit ARM and BLE
// Edison 32 Bit x86 and WiFi/BLE on Linux
//

// Main Arduino C++ include header for all projects
#include "Arduino.h"

// Milliseconds timer function
#define GET_MILLISECONDS() millis()

//
// Simulate roll over after 10 seconds.
//
// Get the bugs out early rather than waiting for 50 days to
// find your app locked up due to timer rollover.
//
//#define GET_MILLISECONDS() (millis() + (((unsigned long)0xFFFFFFFFL) - (unsigned long)0x2710L))

//
// Processor/Board specific header/platform selection
//

#if defined(TEENSYDUINO) && TEENSYDUINO == 123

#define MENLO_BOARD_TEENSY31 1

#include "MenloPlatformTeensy31.h"

#else
#ifdef ARDUINO_ARCH_SAM

// 32 bit ARM processor
#define MENLO_ARM32 1

// Also ARDUINO_SAM_DUE defined by IDE
// Arduino Due
#define MENLO_BOARD_DUE  1

#include "MenloPlatformArm.h"

#else // ARDUINO_ARCH_SAM (Due)

#ifdef NRF51_H // ARM  on Nordic Bluetooth chip

//
// RFduino leverages the Arduino Due's ARM build
// environment so "mostly" looks like an Arduino with an
// ARM processor.
//

// 32 bit ARM processor
#define MENLO_ARM32 1

// RFDuino
#define MENLO_BOARD_RFDUINO  1

#include "MenloPlatformArm.h"

#elif defined(__ARDUINO_X86__) // X86 on Intel Edison

//
// Intel Edison provides an Arduino library that is
// hosted on an x86 32 bit Linux.
//

//
// Intel Edison provides an Arduino library that is
// hosted on an x86 32 bit Linux.
//

// 32 bit X86 processor
#define MENLO_X86 1

// Edison processor
#define MENLO_BOARD_EDISON 1

#include "MenloPlatformX86.h"

#else // AtMega is the default after all others

//
// AtMega
//
// AtMega's are the traditional Arduino's, such as the Uno,
// Arduino Pro Mini, etc.
//

//
// Definitions for low level AtMega support for
// AtMega based Arduinos such as the Arduino UNO.
//

// ATMEGA family processor
#define MENLO_ATMEGA 1

#define MENLO_BOARD_UNO 1

// Watchdog
#include <avr/wdt.h>

// EEPROM
#include <avr/eeprom.h>

// Improve: Use a macro to ensure the inline is used to save code space
// ATMEGA328
//#define ResetWatchdog() wdt_reset()
void ResetWatchdog();

//
// End Arduino IDE environmnets
//

#endif // else Atmega
#endif // ARDUINO_ARCH_SAM
#endif // defined(TEENSYDUINO) && TEENSYDUINO == 123

#elif defined(SPARK) // Non-Arduino platform

// ARM with WiFi, no Arduino (has compat lib)

//
// SparkCore is an ARM 32 bit processor that does not use
// the Arduino build environment, but instead uses a GCC
// based ARM tool chain.
//
// It does provide its own "Arduino Wiring" compatibility
// library that assists in code porting.
//

// 32 bit ARM processor
#define MENLO_ARM32 1

#define MENLO_BOARD_SPARKCORE 1

#include "MenloPlatformSparkCore.h"

#endif // SPARK

//
// The following is compiled for all processors
//

// Watchdog support
void EnableWatchdog();

void ResetWatchdog();

//
// Set this for environments that do not provide an
// "Arduino compatible" library so that MenloPlatform
// can subsitute missing low level routines.
//
// Note: This is an incomplete work in progress since I have found
// "Arduino style" libraries for most embedded projects I have
// encountered now.
//
//#define ARDUINO_COMPAT_REQUIRED 1

#ifdef ARDUINO_COMPAT_REQUIRED

#include "MenloPlatformArduinoCompatRequired.h"

#endif // ARDUINO_COMPAT_REQUIRED

//
// MenloPlatform contains worker routines that provide
// a uniform interface regardless of platform.
//
// The various definitions above drive which implementions
// get compiled for a given target definition.
//
class MenloPlatform {

public:

    //
    // Support for array of pointers to strings
    //
    // This allows routines that place strings in Atmega flash
    // memory (I space) to be used in a portable fashion.
    //
    // On regular architectures (ARM, x86) this resolves to the
    // C index to the array.
    //
    static char* GetStringPointerFromStringArray(char** array, int index);

    //
    // Return a Method Pointer from a Method Array
    //
    static unsigned long GetMethodPointerFromMethodArray(char** array, int index);
};

#endif // MenloPlatform_h
