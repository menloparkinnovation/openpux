
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
 *  Date: 05/03/2015
 *  File: DweetRadioSerialApp.h
 *
 * Template for MenloDweet Application on RadioSerial and serial connections.
 *
 */

#ifndef DweetRadioSerialApp_h
#define DweetRadioSerialApp_h

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
#include <DweetApp.h>

#include <DweetSerialChannel.h>

// Dweet Radio
#include <DweetRadio.h>

// Dispatch + Timers
#include <MenloDispatchObject.h>
#include <MenloTimer.h>

#include <DweetSerialApp.h>

struct DweetRadioSerialAppConfiguration {

    // We share the configuration for DweetSerialApp
    struct DweetSerialAppConfiguration serialConfig;

    //
    // Radio configuration. SPI uses the defaults.
    //
    uint8_t csnPin;
    uint8_t cePin;
    uint8_t irqPin;
    uint8_t payLoadSize;
    bool forceDefaults;
    char* defaultChannel;
    char* defaultReceiveAddress;
    unsigned long sendTimeout;
   
    //
    // RadioSerial
    //
    char* radioSerialTransmitAddress;
};

class DweetRadioSerialApp : public DweetSerialApp  {

public:

    DweetRadioSerialApp();

    int Initialize(DweetRadioSerialAppConfiguration* config);

    //
    // Return the radio class for application usage that can
    // use the radio directly for link level packet transfers.
    //
    MenloRadio* GetRadio() {
        return &m_nordic;
    }

private:

    bool m_gatewayEnabled;

    //
    // Use the same input buffer size as HardwareSerial
    // This allows two full radio packets to be received
    //
    static const uint8_t RadioSerialInputBufferSize = 64;
    uint8_t m_radioSerialInputBuffer[RadioSerialInputBufferSize];

    // nRF24L01 radio instance
    OS_nRF24L01 m_nordic;
    MirfHardwareSpiDriver m_MirfHardwareSpi;

    //
    // Radio Dweet support
    //
    // This supports configuring and controlling the radio.
    //
    DweetRadio m_dweetRadio;

    //
    // Radio Serial support.
    //
    // Provides virtual serial port support over small packet
    // radios.
    //
    MenloRadioSerial m_radioSerial;

    //
    // Dweet can support multiple channels active at one time
    // to allow an application to be reachable by HTTP, Serial port,
    // RadioSerial, COAP, etc.
    //

    //
    // Dweet channel support over radio serial.
    //
    DweetSerialChannel m_dweetRadioSerialChannel;

    //
    // Event registration for transports.
    //
    // For each transport an unhandled Dweet event handler
    // is registered to receive application commands from
    // the transport.
    //
    // A common event handler function is used since each
    // events arguments include the Dweet channel it came
    // in on.
    //

    // RadioSerial Event registration
    MenloDweetEventRegistration m_radioSerialDweetEvent;
};

#endif // DweetRadioSerialApp_h
