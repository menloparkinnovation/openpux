
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
 *  File: OS_nRF24L01.cpp
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

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

//
// Include Menlo Debug library support
//
#include <MenloPlatform.h>
#include <MenloUtility.h>
#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloConfigStore.h>

#include "MenloNMEA0183.h"
#include <MenloDweet.h>  // For error codes

// This libraries header
#include <OS_nRF24L01.h>

//
// A lot of times radio status is required.
// This allows quick enabling of radio status.
//
#define RDBG_PRINT_ENABLED 0

#if RDBG_PRINT_ENABLED
#define RDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#else
#define RDBG_PRINT(x)
#endif

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define DBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_HEX_STRING(x, l)
#define DBG_PRINT_HEX_STRING_NNL(x, l)
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
#define xDBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define xDBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_HEX_STRING(x, l)
#define xDBG_PRINT_HEX_STRING_NNL(x, l)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

//
// Constructor
//

OS_nRF24L01::OS_nRF24L01()
{
  m_present = false;
  m_BufferLength = 0;
}

//
// MenloRadio Contracts 
//

int
OS_nRF24L01::Channel(char* buf, int size, bool isSet)
{
    uint8_t chan;

    if (isSet) {

        if (strlen(buf) < 2) {
            xDBG_PRINT("SetChannel channel is less than 2");
            return DWEET_INVALID_PARAMETER;
        }

        chan = MenloUtility::HexToByte(buf);

        //
        // Channel must be 84 or under for US licensed use.
        //
        if ((chan >= 1) && (chan <= 84)) {

            // Mirf is a global class instance defined by the nRF24L01 library
            Mirf.channel = chan;
      
            xDBG_PRINT_NNL("CH ");
            xDBG_PRINT_INT(chan);

            //
            // Note: Radio needs re-initilize after setting, so caller needs to take
            // this into account.
            //

            return 0;
        }
        else {
            xDBG_PRINT("SetChannel channel is out of range");
            return DWEET_INVALID_PARAMETER;
        }
    }
    else {
        chan = Mirf.channel;

        MenloUtility::UInt8ToHexBuffer(chan, buf);
        buf[2] = '\0';

        return 0;
    }
}

int
OS_nRF24L01::RxAddr(char* buf, int size, bool isSet)
{
  int length, index, addrIndex;
  uint8_t receiveAddress[5];

  if (isSet) {

      DBG_PRINT_NNL("OS_nRF24L01 SetRxAddr=");
      DBG_PRINT_STRING(buf);

      length = strlen(buf);
      if (length < 10) {
          DBG_PRINT("OS_nRF24L01 SetRxAddr to short");
          return DWEET_PARAMETER_TO_SHORT;
      }

      addrIndex = 0;
      for (index = 0; index < 10;) {

        receiveAddress[addrIndex] = MenloUtility::HexToByte(&buf[index]);

        // Index goes up by 2 since we convert to chars to a byte at a time
        index += 2;
        addrIndex++;
      }

      //
      // Configure receiver address.
      // 
      // Note: This configures Pipe 1 RX_ADDR_P1
      // 
      // RX_ADDR_P0 is used for auto-ack on transmit and must
      // be the transmitter address.
      // 
      xDBG_PRINT_NNL("RxAddr ");
      xDBG_PRINT_HEX_STRING(receiveAddress, 5);

      // We let Mirf decide what to do for power
      Mirf.setRADDR(receiveAddress);

      // Save it
      memcpy(&m_receiveAddress[0], &receiveAddress[0], 5);

      return 0;
    }
    else {
#if RADIO_FULL_FUNCTION
      // Platforms with more space
      for (addrIndex = 0; addrIndex < 5; addrIndex++) {

        MenloUtility::UInt8ToHexBuffer(m_receiveAddress[addrIndex], &buf[index]);

        index += 2;
      }

      buf[10] = '\0';

      return 0;
#else
      //
      // This is optional since the caller can determine the address from
      // persistent settings and the history of commands sent.
      //
      // AtMega 328's have tight space constraints and this allows the
      // application developer to manage this tradeoff.
      //
      return DWEET_ERROR_UNSUP;
#endif
    }
}

int
OS_nRF24L01::TxAddr(char* buf, int size, bool isSet)
{
    int length, index, addrIndex;
    uint8_t targetAddress[5];

    if (isSet) {

      DBG_PRINT_NNL("OS_nRF24L01 SetTxAddr=");
      DBG_PRINT_STRING(buf);

      length = strlen(buf);
      if (length < 10) {
          DBG_PRINT("OS_nRF24L01 SetTxAddr to short");
          return DWEET_PARAMETER_TO_SHORT;
      }

      addrIndex = 0;
      for (index = 0; index < 10;) {

        targetAddress[addrIndex] = MenloUtility::HexToByte(&buf[index]);

        // Index goes up by 2 since we convert to chars to a byte at a time
        index += 2;
        addrIndex++;
      }

      xDBG_PRINT_NNL("TxAddr ");
      xDBG_PRINT_HEX_STRING(targetAddress, 5);

      Mirf.setTADDR(targetAddress);

      // Save it
      memcpy(&m_transmitAddress[0], &targetAddress[0], 5);

      return 0;
    }
    else {
#if RADIO_FULL_FUNCTION
      // Platforms with more space
      for (addrIndex = 0; addrIndex < 5; addrIndex++) {

        MenloUtility::UInt8ToHexBuffer(m_transmitAddress[addrIndex], &buf[index]);

        index += 2;
      }

      buf[10] = '\0';

      return 0;
#else
      //
      // This is optional since the caller can determine the address from
      // persistent settings and the history of commands sent.
      //
      // AtMega 328's have tight space constraints and this allows the
      // application developer to manage this tradeoff.
      //
      return DWEET_ERROR_UNSUP;
#endif
    }
}

int
OS_nRF24L01::Power(char* buf, int size, bool isSet)
{
    bool error;
    unsigned long powerTime;

    if (isSet) {
        powerTime = MenloUtility::HexToULong(buf, &error);
        if (error) {
            return DWEET_INVALID_PARAMETER;
        }

        SetPowerTimer(powerTime);

        return 0;  
    }
    else {
       powerTime = GetPowerTimer();

       MenloUtility::UInt32ToHexBuffer(powerTime, buf);

       return 0;
    }
}

//
// This is set for a radio gateway to immediately respond
// with an attention packet when the next packet is received
// from the other end.
//
// This is to keep "mostly" sleeping sensors awake when the application
// has further commands for it.
//
int
OS_nRF24L01::Attention(char* buf, int size, bool isSet)
{
    bool error;
    unsigned long powerTime;

    if (isSet) {
        powerTime = MenloUtility::HexToULong(buf, &error);
        if (error) {
            return DWEET_INVALID_PARAMETER;
        }

        if (powerTime < RADIO_MINIMUM_POWER_TIMER) {
            powerTime = RADIO_MINIMUM_POWER_TIMER;
        }

        // Attention is to our peer address
        SetSendAttention(powerTime, &m_transmitAddress[0]);

        return 0;  
    }
    else {
        // Not implemented
        return DWEET_ERROR_UNSUP;
    }
}

//
// Options is used as a catch all "ioctl" for the radio.
//
// It's used for the optional radio attention support feature
// with support from MenloRadio.
//
int
OS_nRF24L01::Options(char* buf, int size, bool isSet)
{
  // notimplemented until needed.
  return DWEET_ERROR_UNSUP;
}

uint8_t
OS_nRF24L01::GetPacketSize()
{
  return m_BufferLength;
}

uint8_t* OS_nRF24L01::GetReceiveBuffer()
{
  return m_Buffer;
}

//
// Initialize the radio.
//
// payloadSize - Standard size for all packets, must pad if shorter.
//               Default 16, max 32
//
// cePin - Digital pin number for Chip Enable
//
// csnPin - Digital pin for slave select not
//
int
OS_nRF24L01::Initialize(
    bool  forceDefaults,
    char* defaultChannel,
    char* defaultReceiveAddress,
    MirfSpiDriver* spi,
    uint8_t payloadSize,
    uint8_t cePin,
    uint8_t csnPin
    )
{

  // Mirf is a global class instance defined by the nRF24L01 library

  //
  // Setup pins / SPI.
  //
   
  //
  // Like most Arduino libraries Mirf is a global static class
  // variable defined by the Mirf library. The pins are publics, but
  // should be in a configuration method.
  //
  Mirf.cePin = cePin;
  Mirf.csnPin = csnPin;

  //
  // Note: The MirfHardwareSpi can conflict with the Arduino
  // WiFi since it reprograms the SPI control registers and
  // clock rates without restoring them.
  //
  // There is also no interrupt synchronization, so if the
  // WiFi uses actual vectored interrupts it could be in
  // trouble as well.
  //
  // To work around this, Menlo has developed a software SPI
  // and integrated it with the Mirf open source nRF24L01
  // driver. The caller of this class can select at runtime
  // whether to use hardare or software SPI.
  //
  //Mirf.spi = &MirfHardwareSpi;

  // Let the caller pick the SPI to use
  Mirf.spi = spi;

  //
  // Init must be called first to setup the cePin, csnPin
  // modes, and get the SPI ready for transfer.
  //
  Mirf.init();
  
  // Test presence now that we have the hardware configured

  //
  // NOTE: set true for loop on not present for hardware debugging
  // with an SPI/logic analyzer
  //
  //if (!Mirf.IsPresent(true)) {

  if (!Mirf.IsPresent(false)) {
    // Not present
    return 1;
  }

  //
  // Set the payload length specified.
  //
  // Note that the payload size must be the same size
  // on the client and the server, and must conform
  // to the radio's limits.
  //
  Mirf.payload = payloadSize;
  m_BufferLength = payloadSize;

  //
  // Always set a safe default channel and configuration
  // receive address.
  //
  Channel(defaultChannel, RADIO_CHANNEL_SIZE, true);

  RxAddr(defaultReceiveAddress, RADIO_RXADDR_SIZE, true);

  //
  // Invoke base class initialize.
  //
  MenloRadio::Initialize();

  //
  // Now initilize the hardware with the configuration
  //
  // This powers on the receiver.
  //
  Mirf.config();

  m_present = true;

  return 0;
}

//
// ReceiveDataReady() is a contract to MenloRadio in which
// we indicate if any data is available in the buffered
// queue.
//
// The current nRF24L01 implementation just uses the single
// transfer buffer which is only valid immediately after a
// packet is transferred from the radio.
//
bool
OS_nRF24L01::ReceiveDataReady()
{
  if (!m_present) {
      xDBG_PRINT("OS_nRF24L01 RadioDataReady no device present");
      return false;
  }

  //
  // Don't attempt to switch to the receiver until
  // the transmitter is idle.
  //
  if (TransmitBusy()) {
      xDBG_PRINT("OS_nRF24L01 RadioDataReady tx sending");
      return false;
  }

      delay(20); // TODO: remove

  if (Mirf.dataReady()) {

      RDBG_PRINT("nRF24L01 RadioDataReady is true");

      // This is needed to keep the radio from not working!
      //delay(10);
      delay(20); // TODO: remove

      return true;
  }

  delay(20); // TODO: Remove!

  //DBG_PRINT("Radio no data ready");

  return false;
}

int
OS_nRF24L01::WaitForRadioDataReady(unsigned long timeout)
{
    if (!m_present) {
      xDBG_PRINT("OS_nRF24L01 WaitForRadioDataReady no device present");
      return 0;
    }

    unsigned long startTime = millis();

    while(!ReceiveDataReady()) {

      if (( millis() - startTime ) > timeout) {
          RDBG_PRINT("WaitForRadioDataReady: timeout");
          return 0;
      }

      if (timeout != 0) {
          DBG_PRINT("OS_nRF24L01 WaitForRadioDataReady Data Not Ready");
      }

      delay(10);

      ResetWatchdog(); // Tell watchdog we are still alive
    }

    xDBG_PRINT("OS_nRF24L01 WaitForRadioDataReady DataReady");

    return 1;
}

int
OS_nRF24L01::WaitForSendComplete(unsigned long timeout)
{
  if (!m_present) {
    return 0;
  }

  unsigned long startTime = millis();

  while(TransmitBusy()){

      if ( ( millis() - startTime ) > timeout ) {
	RDBG_PRINT("OS_nRF24L01 WaitForSendComplete timeout");
        return 0;
      }

      DBG_PRINT("OS_nRF24L01 WaitForSendComplete Waiting for send complete");

      delay(10);

      ResetWatchdog(); // Tell watchdog we are still alive
  }

  DBG_PRINT("OS_nRF24L01 WaitForSendComplete completed");

  MenloMemoryMonitor::CheckMemory(LineNumberBaseNRF24 + __LINE__);

  return 1;
}

//
// Read data state machine:
//
// The radio receives packets and buffers it into its internal buffer.
//
// Current nRF24L01+ radios have a single buffer, so data needs to
// be received quickly so that a future send does not overflow the
// buffer.
//
// At the current time radio receive interrupts are not used.
//
// ReceiveDataReady() which is implemented by this function returns
// if any receive data is available immediately for a zero wait Read().
//
// MenloRadio, of which this is a subclass of calls ReceiveDataReady()
// from its Poll() function to determine if any receive data is available.
//
// If ReceiveDataReady() returns true, then MenloRadio will issue
// a zero wait Read() to retrieve data from the radio into the common
// shared buffer. It will then raise a radio receive data event with
// this buffer pointer. The application module using MenloRadio will then
// process this buffer consuming the data.
//
// Since the current implementation only has one shared buffer
// for Read() and Write(), it must be processed immediately before
// any future calls to ReceiveDataReady(), Read(), or Write().
//
// This module tries to keep buffering to a minimum conserving memory
// for small embedded implementations.
//
int
OS_nRF24L01::OnRead(unsigned long timeout)
{
  int retVal;

  //
  // Callers can specify a timeout of 0 for zero wait reads.
  // This is common with callers which use the radio receive event
  // from MenloRadio and only read when data is available.
  //
  retVal = WaitForRadioDataReady(timeout);
  if (retVal == 0) {
      xDBG_PRINT("OS_nRF24L01 Read timeout on WaitForDatReady");
      return 0;
  } 

  //
  // FUTURE: Need to store the address received from to support
  // multiple clients.
  //

  // The transfer size is fixed
  Mirf.getData(m_Buffer);

  MenloMemoryMonitor::CheckMemory(LineNumberBaseNRF24 + __LINE__);

  RDBG_PRINT("Radio packet received");

  return m_BufferLength;
}

int
OS_nRF24L01::OnWrite(
    byte* targetAddress,
    uint8_t* transmitBuffer,
    uint8_t transmitBufferLength,
    unsigned long timeout
    )
{
  int retVal;

  if (transmitBufferLength < m_BufferLength) {
      DBG_PRINT("Write: transmit buffer is to small");
      return 0;
  }

  // This should not happen since we wait for send completes after write
  retVal = WaitForSendComplete(timeout);
  if (retVal == 0) {
      RDBG_PRINT("nRF24L01 write Previous send not complete!");
      return 0;
  }

  // Request power if not enabled
  PowerOn();

  //
  // If targetAddress == NULL we use the settings configured
  // by SetTxAddr().
  //
  if (targetAddress == NULL) {
      DBG_PRINT_NNL("Radio sending to configured address ");
      DBG_PRINT_HEX_STRING(&m_transmitAddress[0], 5);
      Mirf.setTADDR(&m_transmitAddress[0]);
  }
  else {
      DBG_PRINT("Radio sending to argument address");
      DBG_PRINT_HEX_STRING(targetAddress, 5);
      Mirf.setTADDR(targetAddress);
  }

  DBG_PRINT("Radio sending packet");

  //
  // The transfer size is fixed
  // This automatically powers on the transmitter
  //
  Mirf.send(transmitBuffer);

  //
  // Wait for this send to complete
  //
  // When send is done this will automatically power up
  // the receiver.
  //
  retVal = WaitForSendComplete(timeout);
  if (retVal == 0) {
      RDBG_PRINT("OS_nRF24L01::Write Timeout waiting for send complete");
      return 0;
  }

  RDBG_PRINT("Radio packet sent");

#if RDBG_PRINT_ENABLED
  Mirf.DumpRegisters();
#endif

  return m_BufferLength;
}

/*
05/11/2015 no one calls this
int
OS_nRF24L01::TestRegisters()
{
  // Read RF_SETUP
  byte rf_setup = 0;
  Mirf.readRegister( RF_SETUP, &rf_setup, sizeof(rf_setup) );
  return rf_setup;
}
*/

bool
OS_nRF24L01::TransmitBusy()
{
  //
  // Note: Mirf.isSending() has the side effect of powering up
  // the receiver once the packet is sent.
  //
  if (Mirf.isSending()) {
    SetActivity();
    return true;
  }
  else {
    return false;
  }
}

//
// Power support
//

//
// Request to power on the radio
//
void
OS_nRF24L01::OnPowerOn()
{
    //
    // A request to power on is the power on the receiver which
    // is the radios default mode of operation when powered on.
    //
    // If a transmit is requested the Mirf will transition the radio
    // to transmit power on.
    //
    RDBG_PRINT("OS_nRF24L01 radio powered up");    
    Mirf.powerUpRx();
}

//
// Request to power off the radio
//
void
OS_nRF24L01::OnPowerOff()
{
    RDBG_PRINT("OS_nRF24L01 radio powered down");    
    Mirf.powerDown();
}

//
// PowerOnRadio()
// PowerOffRadio()
//
// Mirf.cpp, Mirf.h
//
// Mirf.powerUpRx()
//    csnLow();
//    spi->transfer( FLUSH_RX );
//    csnHi();
//
// Mirf.powerUpTx()
//	PTX = 1;
//	configRegister(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (0<<PRIM_RX) ) );
//
// Mirf.powerDown()
//	ceLow();
//	configRegister(CONFIG, mirf_CONFIG );
//
//      ce pin low powers down radio - ceLow()
//      ce pin high powers up radio  - ceHi()
//
//        CE pin controls radio power. Its turned on and off
//        when configuration SPI transfers are occuring.
//
//      csn pin low selects chip for SPI transfer
//      csn pin high de-selects chip for SPI transfer
//      csnLow(), csnHi() are strictly used to bracket SPI transfers.
//
//      The chip can be powered down (ce pin low) and still allow
//      SPI transfers with csn-low/csn-high.
//
//      Actual packet transmission commences when ce pin is set high
//      to power up.
//
//      ceLow() left set on return by:
//       init()
//       powerDown()
//
//      ceHi() left set on return by:
//       setRADDR()
//       send()
//       powerUpRx()
//       config()
//         through powerUpRx()
//       isSending()
//         through powerUpRx() if no packet is being sent
//
