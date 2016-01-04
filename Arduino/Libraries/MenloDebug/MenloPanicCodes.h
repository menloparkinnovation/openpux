
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

//
// MenloPanicCodes.h
//
// Panic Codes. 
//

#ifndef MenloPanicCodes_h
#define MenloPanicCodes_h

//
// Panic codes
//
// Panic codes should be defined as two BCD numbers packed into the single
// panic code byte.
//
// The upper, then lower BCD nibble will be flashed on a user configured
// LED when a panic occurs. This is to enable detection and reporting of
// errors when no other communication is possible.
//
// Lower numbers in each BCD digit should be chosen first so it will be
// easy for a consumer to count the light flashes on error report. Higher
// numbers will involve a longer period of focus on the flashes.
//
// For example, a "33" sent as two series of three flashes will be easier
// to read than "99" which will be two series of nine flashes.
//
// This debug module does not define many codes, as its up to the
// application classes/sketches to define what they mean for
// that application.
//
// Note: Don't choose 0 as any digit as it will be "sent" as a
// pause, and it will be hard for a user to distinguish.
//

//
// This is a summary of error codes from major modules and is here
// to assist in support.
//

//
// The upper nibble contains the subsytem.
//

// 0 - not used since it flashes as blank.
// 1 - Runtime. MenloDebug, MenloMemoryMonitor, MenloDispatchObject
//

//
// MenloDispatchObject
// MenloEvent
//

const uint8_t DispatchEventAlreadyLinked = 0x11;
const uint8_t DispatchEventNotLinked     = 0x12;

//
// MenloDebug.h
//
const uint8_t MenloDebugUserAssertionFailed = 0x13;

//
// MenloMemoryMonitor.h
//

// Memory Overflow Types
const uint8_t OverflowTypeConfiguration = 0x14;
const uint8_t OverflowTypeStackCanary   = 0x15;
const uint8_t OverflowTypeHeapCanary    = 0x16;
const uint8_t OverflowTypeStackOverflow = 0x17;
const uint8_t OverflowTypeHeapOverflow  = 0x18;
const uint8_t OverflowTypeGuardRegion   = 0x19;

//
// MenloGateway
//
// PANIC_CODE_EARLY_INIT
const uint8_t WiFiError = 0x81;
const uint8_t WiFiNoHardware = 0x82;
const uint8_t WiFiNoSignOn = 0x83;
const uint8_t nRF24L01InitializeError = 0x84;
const uint8_t CloudError = 0x85;
const uint8_t InternalError = 0x86;

//
// Line number ranges for debugging modes.
//
// MenloMemoryMonitor::CheckMemmory() reports which line was
// in error.
//
// Some modules add a constant base in order to diagnose
// and narrow down. These are recorded here.
//
// 

const uint16_t LineNumberBaseNoModule       =     0;

const uint16_t LineNumberBaseMemoryMonitor  =  1000;

const uint16_t LineNumberBaseFramework      =  2000;

const uint16_t LineNumberBaseAppFramework   =  3000;

const uint16_t LineNumberBaseDweet          =  4000;

const uint16_t LineNumberBaseDweetRadio     =  5000;

const uint16_t LineNumberBaseNRF24          =  6000;

const uint16_t LineNumberBaseRadioSerial    =  7000;

const uint16_t LineNumberBaseMenloRadio     =  8000;

const uint16_t LineNumberBaseDweetSerialApp = 10000;

const uint16_t LineNumberBaseRadioSerialApp = 11000;

const uint16_t LineNumberBaseApp            = 12000;

const uint16_t LineNumberBaseTimer          = 13000;

const uint16_t LineNumberBaseConfigStore    = 14000;

const uint16_t LineNumberBaseDispatch       = 15000;

const uint16_t LineNumberBaseEvent          = 16000;

const uint16_t LineNumberBaseMenloNRF24     = 17000;

const uint16_t LineNumberBaseGatewayApp     = 18000;

//
// Reserved for application components
//
const uint16_t LineNumberBaseAppComp0       = 60000;

const uint16_t LineNumberBaseAppComp1       = 61000;

const uint16_t LineNumberBaseAppComp2       = 62000;

const uint16_t LineNumberBaseAppComp3       = 63000;

const uint16_t LineNumberBaseAppComp4       = 64000;

const uint16_t LineNumberBaseAppComp5       = 65000;

#endif // MenloPanicCodes_h
