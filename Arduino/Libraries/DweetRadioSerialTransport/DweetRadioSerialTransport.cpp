
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
 *  Date: 05/30/2016
 *  File: DweetRadioSerialTransport.cpp
 *
 *  Generic transport for packet radios.
 *
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

#include "DweetRadioSerialTransport.h"

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

DweetRadioSerialTransport::DweetRadioSerialTransport()
{
}

int
DweetRadioSerialTransport::Initialize(DweetRadioSerialTransportConfiguration* config)
{
    char buf[GATEWAY_ENABLED_SIZE+1];

    m_radio = config->radio;
    m_dweetRadio = config->dweetRadio;

    ResetWatchdog();

    // Initialize the base serial channel configuration
    DweetSerialApp::Initialize(&config->serialConfig);

    //
    // Setup our event handlers for the radio serial transport
    //
    //m_radioSerialDweetEvent.object = this;

    // We use the common DweetEvent from our parent class DweetSerialApp
    //m_radioSerialDweetEvent.method = (MenloEventMethod)&DweetSerialApp::DweetEvent;

    ResetWatchdog();

    //
    // Initialize Radio Serial
    //
    m_radioSerial.Initialize(
        m_radio,
        (byte *)config->radioSerialTransmitAddress, // Address serial.write's go to
        m_radioSerialInputBuffer,
        RadioSerialInputBufferSize,
        config->payLoadSize,  // max payload size
        config->sendTimeout
    );

    ResetWatchdog();

    ConfigStore.ReadConfig(GATEWAY_ENABLED, (uint8_t*)&buf[0], GATEWAY_ENABLED_SIZE);
    ConfigStore.ProcessConfigBufferForValidChars(&buf[0], GATEWAY_ENABLED_SIZE);

    if ((buf[0] == 'O') && (buf[1] == 'N')) {
        m_gatewayEnabled = true;
    }
    else {
        m_gatewayEnabled = false;
    }

    if (m_gatewayEnabled) {
        MenloDebug::Print(F("Gateway Mode"));
    }
    else {
        MenloDebug::Print(F("Sensor Mode"));

        //
        // Initialize Dweet over nRF24L01+ radio emulating a serial port
        //
        // This allows the sensor to receive and respond to Dweet commands
        // over the radio in addition to the serial port used for setup/configuration.
        //

        // Initialize the Dweet channel over radio serial
        m_dweetRadioSerialChannel.Initialize(&m_radioSerial, m_dweetPrefix);

        //
        // Register our Dweet event handler on serial
        //
        //MenloDweet::RegisterGlobalUnhandledDweetEvent(&m_radioSerialDweetEvent);

        // Enable its radio receive events
        m_radioSerial.EnableReceive(true);
    }

    //MenloDebug::Print(F("DweetRadioSerialTransport Initialized"));
    //MenloDebug::PrintHex(sizeof(m_radioSerialInputBuffer));

    ResetWatchdog();

    //
    // We check the memory monitor at the end of initialization
    // in case any initialization routines overflowed the stack or heap.
    //
    MenloMemoryMonitor::CheckMemory(LineNumberBaseRadioSerialApp + __LINE__);

    return 0;
}
