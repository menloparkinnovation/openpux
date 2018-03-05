
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
 *  Date: 05/30/2016
 *  File: DweetRadioSerialTransport.h
 *
 *  Generic transport for packet radios.
 *
 */

#ifndef DweetRadioSerialTransport_h
#define DweetRadioSerialTransport_h

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

struct DweetRadioSerialTransportConfiguration {

    // We share the configuration for DweetSerialApp
    struct DweetSerialAppConfiguration serialConfig;

    //
    // Radio configuration.
    //
    uint8_t payLoadSize;
    unsigned long sendTimeout;
   
    //
    // MenloRadio provides the packet driver interface to a radio.
    //
    MenloRadio* radio;

    //
    // DweetRadio provides the control interface for a radio.
    //
    DweetRadio* dweetRadio;

    //
    // RadioSerial
    //
    char* radioSerialTransmitAddress;
};

//
// This class uses 604 bytes of data on the Atmega328.
//
// It's a complete application within a class, and contains:
//
//  - Full duplex Dweet buffers for serial channel support
//  - 2X Radio buffers for RadioSerial
//  - Full duplex Dweet buffers for radio serial channel support
//  - Application state variables, event registrations, etc.
//  - 2X base NMEA state class for serial channel, radio serial channel
//
//  218 bytes in the base class, the rest here:
//
//  64 m_radioSerialInputBuffer
//
//  25 m_dweetRadio
//     21 calculated, 25 actual from sizeof()
//
//  82 m_radioSerial - radio buffer + registration
//     68 calculated, 82 from sizeof()
//
// 205 m_dweetRadioSerialChannel - 168 in buffers
//     182 calculated, 205 from sizeof()
//     84 bytes each for input and output buffers
//
//   8 m_radioSerialDweetEvent
//
//  9 MenloEventRegistration
//    8 calculated, 9 from sizeof()
//
// Base Class DweetSerialApp: 218 bytes from sizeof()
//
// MenloDebug::PrintHex(sizeof(DweetRadioSerialApp)); // 706 bytes
// MenloDebug::PrintHex(sizeof(OS_nRF24L01)); // 100 bytes
// MenloDebug::PrintHex(sizeof(DweetRadio));  //  25 bytes
// MenloDebug::PrintHex(sizeof(DweetSerialChannel)); // 205 bytes
// MenloDebug::PrintHex(sizeof(MenloRadioSerial));   //  82 bytes
// MenloDebug::PrintHex(sizeof(MirfHardwareSpiDriver)); // 2 bytes
// MenloDebug::PrintHex(sizeof(MenloEventRegistration)); // 9 bytes
// MenloDebug::PrintHex(sizeof(DweetSerialApp)); // 218 bytes
//
class DweetRadioSerialTransport : public DweetSerialApp  {

public:

    DweetRadioSerialTransport();

    int Initialize(DweetRadioSerialTransportConfiguration* config);

    //
    // Return the radio class for application usage that can
    // use the radio directly for link level packet transfers.
    //
    MenloRadio* GetRadio() {
        return m_radio;
    }

private:

    bool m_gatewayEnabled;

    //
    // Use the same input buffer size as HardwareSerial
    // This allows two full radio packets to be received
    //
    static const uint8_t RadioSerialInputBufferSize = 64;
    uint8_t m_radioSerialInputBuffer[RadioSerialInputBufferSize];

    //
    // Menlo radio instance
    //
    MenloRadio* m_radio;

    //
    // Radio Dweet support
    //
    // This supports configuring and controlling the radio.
    //
    DweetRadio* m_dweetRadio;

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
};

#endif // DweetRadioSerialTransport_h
