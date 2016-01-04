
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
 *  Date: 05/24/2015
 *  File: DweetRadioGatewayApp.cpp
 *
 *  Template application for RadioGateways
 */

//
// MenloPlatform support
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloPower.h>
#include <MenloFramework.h>

//
// MenloRadio support for small packet radios
//
#include <MenloRadio.h>

//
// Libraries used by the Nordic nRF24L01 radio
//
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

#include <OS_nRF24L01.h>

// MenloRadioSerial Support
#include <MenloRadioSerial.h>

// NMEA 0183 support
#include <MenloNMEA0183.h>

// Dweet Support
#include <MenloDweet.h>
#include <DweetSerialChannel.h>

// Dweet Radio
#include <DweetRadio.h>

#include <MenloDispatchObject.h>
#include <MenloTimer.h>

#include <DweetSerialApp.h>

#include "DweetRadioGatewayApp.h"

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

DweetRadioGatewayApp::DweetRadioGatewayApp()
{
}

int
DweetRadioGatewayApp::Initialize(DweetRadioGatewayAppConfiguration* config)
{
    char buf[GATEWAY_ENABLED_SIZE+1];

    ResetWatchdog();

    // Initialize the base serial channel configuration
    DweetSerialApp::Initialize(&config->serialConfig);

    //
    // Initialize the radio.
    //
    // The radio can load configuration from EEPROM configuration storage
    // or use the supplied default values.
    //
    // If the EEPROM configuration has not been loaded, or is corrupt
    // the supplied default settings are used.
    //
    // If forceDefaults is set, they are used regardless, which can
    // be used for initial configuration, recovery, etc.
    //
 
    // Initialize the Nordic nRF24L01 radio
    m_nordic.Initialize(
        config->forceDefaults,
        config->defaultChannel,
        config->defaultReceiveAddress,
        &m_MirfHardwareSpi,
        config->payLoadSize,  // payload size
        config->cePin,        // cePin
        config->csnPin        // csnPin
        );

    ResetWatchdog();

    //
    // Initialize Dweet Radio on the serial interface for configuration and
    // optional gateway usage.
    //
    m_dweetRadio.Initialize(&m_dweetSerialChannel, &m_nordic);

    ResetWatchdog();

    //
    // We check the memory monitor at the end of initialization
    // in case any initialization routines overflowed the stack or heap.
    //
    MenloMemoryMonitor::CheckMemory(LineNumberBaseAppFramework + __LINE__);

    return 0;
}
