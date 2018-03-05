
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
 * Modifications, additions, subtractions are
 * Copyright (C) 2012 Menlo Park Innovation LLC
 *
 * This work incorporates one or more "open" or "shared" source
 * licenses. As such, its use and licensing conforms to the licenses
 * of the code incorporated.
 *
 * Menlo Park Innovation LLC does not provide any warranty, support,
 * or restrictions on its use and may be distributed under the
 * terms of the contained license(s).
 *
 *  Date: 11/20/2012
 *  File: OS_nRF24L01.h
 *
 * Code contained within comes from the following open, shared, community,
 * public domain, or other licenses:
 *
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#ifndef OS_nRF24L01_h
#define OS_nRF24L01_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#include <Arduino.h>
#include <inttypes.h>

// Libraries used for nRF24L01
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

// This class implements MenloRadio
#include <MenloRadio.h>

//
// Platforms such as AtMega 328's can be tight on space.
//
// Depending on the application it may want to trade off
// basic vrs. full radio functionality.
//
// This is controlled here based on space considerations.
//
#define RADIO_FULL_FUNCTION 1

// Allow optin/out for code size/testing
#define RADIO_POWER_CONTROL 1

//
// Note: A dynamic receive mode can be implemented in which the
// sensor first contacts the gateway using a known fixed address
// such as sens0, and then gets assigned its unitId. It can then
// reconfigure its receiver to sens<unitID> and have a unique
// address.
//
// The gateway would add the <unitID> in any responses/commands
// to the unit.
//
// This would avoid receiving packets not addressed to the
// sensor, even though a shared channel is available.
//
// This is currently not used, as the upper level protocol
// handles shared channel, and a sensor unit is mostly
// sleeping and ignoring other traffic. (no wake on carrier
// from low power mode on the Nordic nRF24L01 radio).
//

class OS_nRF24L01 : public MenloRadio {

public:
  
    OS_nRF24L01();

    //
    // defaultChannel and defaultReceiveAddress
    // are used if the configuration data is invalid.
    //
    // The caller can choose to override these settings by supplying
    // forceDefaults == true. In that case the caller/derived class is
    // responsible for all settings.
    //
    int Initialize(
        bool  forceDefaults,
        char* defaultChannel,
        char* defaultReceiveAddress,
        MirfSpiDriver* spi,
        uint8_t payloadSize,
        uint8_t cePin,
        uint8_t csnPin
        );

    //
    // MenloRadio required methods
    //

    //
    // Returns whether there is data ready in the receiver.
    //
    // Note: If radio power is not on this will always return
    // no data available.
    //
    virtual bool ReceiveDataReady();

    //
    // Indicates if the transmitter is busy sending a packet.
    //
    virtual bool TransmitBusy();

    virtual uint8_t GetPacketSize();

    // See comments in implementation file for OnRead() state machine
    virtual int OnRead(unsigned long timeout);

    virtual int OnWrite(
        byte *targetAddress,
        uint8_t* transmitBuffer,
        uint8_t transmitBufferLength,
        unsigned long timeout
        );

    virtual uint8_t* GetReceiveBuffer();

    //
    // Note: ASCII character strings are used to allow
    // different radios to control their format. Could be
    // hex numbers, strings, or symbolic names.
    //
    // MenloRadio supplies a base implementation for
    // persistent settings which can be delegated to support
    // a common configuration store.
    //

    //
    // Radio Settings
    //

    virtual int Channel(char* buf, int size, bool isSet);

    virtual int RxAddr(char* buf, int size, bool isSet);

    virtual int TxAddr(char* buf, int size, bool isSet);

    virtual int Power(char* buf, int size, bool isSet);

    virtual int Attention(char* buf, int size, bool isSet);

    //
    // Radio specific catch all for packet size, CRC, data rate, etc.
    //
    // Radio specific format string.
    //
    virtual int Options(char* buf, int size, bool isSet);

    //
    // Additional methods
    //

    //int TestRegisters();

    void DumpRegisters();

    int RunRangeTestServer(byte* targetAddress);

protected:

    //
    // Radio Power Control
    //
    virtual void OnPowerOn();

    virtual void OnPowerOff();

private:

    int WaitForRadioDataReady(unsigned long timeout);

    int WaitForSendComplete(unsigned long timeout);

    //
    // We place the buffer in this class since space
    // is tight. This buffer is used for receive.
    //
    uint8_t m_Buffer[MENLO_RADIO_PACKET_SIZE];

    uint8_t m_BufferLength;

    bool m_present;

    uint8_t m_receiveAddress[5];
    uint8_t m_transmitAddress[5];
};

#endif // OS_nRF24L01_h
