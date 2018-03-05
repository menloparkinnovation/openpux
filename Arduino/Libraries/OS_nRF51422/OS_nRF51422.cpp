
/*
 * Modifications, additions, subtractions are
 * Copyright (C) 2014 Menlo Park Innovation LLC
 *
 * This work incorporates one or more "open" or "shared" source
 * licenses. As such, its use and licensing conforms to the licenses
 * of the code incorporated.
 *
 * Menlo Park Innovation LLC does not provide any warranty, support,
 * or restrictions on its use and may be distributed under the
 * terms of the contained license(s).
 *
 *  Date: 03/19/2014
 *  File: OS_nRF51422.h
 *
 * Code contained within comes from the following open, shared, community,
 * public domain, or other licenses:
 *
*/

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

//
// Include Menlo Debug library support
//
#include <MenloDebug.h>
//#include <MenloMemoryMonitor.h>

// This libraries header
#include <OS_nRF51422.h>

#define DBG_PRINT_ENABLED 1

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (Serial.println(F(x)))
#define DBG_PRINT_INT(x) (Serial.println(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_INT(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 1

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)     (Serial.println(F(x)))
#define xDBG_PRINT_INT(x) (Serial.println(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_INT(x)
#endif

//
// We only have one radio, so this variable allows
// us to find our class for receive event indications
// from the on-chip radio.
//
OS_nRF51422* G_radio;

//
// External C bindings to receive radio indications
//
// These are defined in libRFduinoBLE.h
//
// RFduino_1.7.1\RFduino\variants\RFduino\libRFduinoBLE.h
//
void RFduinoBLE_onReceive(char *data, int len)
{
  if (G_radio != NULL) {
    G_radio->onReceive(data, len);
    return;
  }
  else {
    DBG_PRINT("OS_nRF51422::_onReceive: No handler class registered for receive. Packet dropped.");
  }

  return;
}

void RFduinoBLE_onConnect()
{
}

void RFduinoBLE_onDisconnect()
{
}

void RFduinoBLE_onRSSI(int rssi)
{
}

void RFduinoBLE_onAdvertisement(bool start)
{
}

OS_nRF51422::OS_nRF51422()
{
  //
  // We only have one radio on chip that sends
  // us indications, so we store its pointer in
  // a global variable so that the C based BLE library
  // routines can invoke the class when events occur.
  //
  DEBUG_ASSERT(G_radio == NULL);

  G_radio = this;

  m_sendBufferLength = 0;
  m_receiveBufferLength = 0;
}

//
// Invoked from an interrupt handler when receive data arrives
// from the Bluetooth radio.
//
void OS_nRF51422::onReceive(char *data, int len)
{
  if (m_receiveBufferLength != 0) {
    DBG_PRINT("OS_nRF51422::_onReceive: Buffer already busy! Packet dropped.");
    return;
  }

  if (len > OS_PACKET_SIZE) {
    // A partial message is not a message (Crimson Tide)
    DBG_PRINT("OS_nRF51422::_onReceive: Receive packet larger than buffer, dropped");
    return;
  }

  memcpy(m_receiveBuffer, data, len);
  m_receiveBufferLength = len;

  return;
}

//
// Initialize the radio.
//
// payloadSize - Standard size for all packets, must pad if shorter.
//               Default 16, max 32
//
int
OS_nRF51422::Initialize(
      uint8_t payloadSize,
      char* deviceName,
      char* advertisementData,
      int advertisementInterval,
      int txPowerLevel
    )
{
  int result;

  // default is "RFduino" needed for example iPhone App
  if (deviceName != NULL) {
      RFduinoBLE.deviceName = deviceName;
  }

  //
  // this is the data we want to appear in the advertisement
  // (the deviceName length plus the advertisement length must be <= 18 bytes
  //
  // default is "sketch"
  //
  if (advertisementData != NULL) {
      RFduinoBLE.advertisementData = advertisementData;
  }
  
  RFduinoBLE.txPowerLevel = txPowerLevel;

  if (advertisementInterval != (-1)) {
      RFduinoBLE.advertisementInterval = advertisementInterval;
  }

  //
  // start the BLE stack
  // 0 = success
  // 1 = device_name + advertisement_data invalid (> 18)
  // 2 = tx_power_level invalid (< -20 || > +4)
  // 3 = advertisement_interval invalid (< 20ms || > 10.24s)
  //
  result = RFduinoBLE.begin();

  if (result != 0) {
    DBG_PRINT("OS_nRF51422::Initialize error from RFduinoBLE.begin()");
  }

  // RFduinoBLE.end() stops the stack

  return result;
}

//
// This gets the transmit buffer for the caller.
//
uint8_t* OS_nRF51422::GetBuffer()
{
  return m_sendBuffer;
}

//
// This gets the transmit buffer length for the caller.
//
int OS_nRF51422::GetBufferLength()
{
  return m_sendBufferLength;
}

bool
OS_nRF51422::DataReady()
{
  if (m_receiveBufferLength != 0) {
    return true;
  }
  else {
    return false;
  }
}

int
OS_nRF51422::WaitForDataReady(unsigned long timeout)
{
    unsigned long startTime = millis();

    //
    // TODO: Actual receives are async, make sure
    // us blocking the thread here does not prevent
    // receives. We should use low power receive
    // anyway.
    //

    while(!DataReady()){

      if ( ( millis() - startTime ) > timeout ) {
          xDBG_PRINT("Timeout on response from server!");
          return 0;
      }

      //DBG_PRINT("OS_nRF51422::WaitForDataReady Data Not Ready!");

      delay(10);

      ResetWatchdog(); // Tell watchdog we are still alive
    }

    xDBG_PRINT("OS_nRF51422::WaitForDataReady DataReady!");

    return 1;
}

int
OS_nRF51422::WaitForSendComplete(unsigned long timeout)
{
  // No status to watch on this radio, it is queued
  return 1;
}

int
OS_nRF51422::Read(unsigned long timeout)
{
  int retVal;

  retVal = WaitForDataReady(timeout);
  if (retVal == 0) {
    return 0;
  } 

  return m_receiveBufferLength;
}

int
OS_nRF51422::Write(
    byte* targetAddress,
    unsigned long timeout
    )
{
  int retVal;
  bool result;

  retVal = WaitForSendComplete(timeout);
  if (retVal == 0) {
    xDBG_PRINT("Write: Previous send not complete!");
    return 0;
  }

  // The transfer size is fixed
  result = RFduinoBLE.send((char*)m_sendBuffer, m_sendBufferLength);

  //
  // Wait for this send to complete
  //
  retVal = WaitForSendComplete(timeout);
  if (retVal == 0) {
      xDBG_PRINT("OS_nRF51422::Write Timeout waiting for send complete");
      return 0;
  }

  return m_sendBufferLength;
}

//
// Reads are actually asynchronously received into the
// buffer in an interrupt driven fashion from the RFduino
// BLE stack.
//
// So we return any available data here.
//

//
// Reset the receive buffer to allow reception of a new packet
//
void
OS_nRF51422::ResetReceiveBuffer()
{
  m_receiveBufferLength = 0;
  return;
}

int
OS_nRF51422::Read(
    byte *receivedFromAddress,
    uint8_t* buffer,
    int length,
    unsigned long timeout
    )
{
  int retVal;
  int transferLength;

  retVal = WaitForDataReady(timeout);
  if (retVal == 0) {
    return 0;
  } 

  if (m_receiveBufferLength == 0) {
    return 0;
  }

  transferLength = length;
  if (transferLength > m_receiveBufferLength) {
    transferLength = m_receiveBufferLength;
  }

  memcpy(buffer, m_receiveBuffer, transferLength);

  return transferLength;
}

int
OS_nRF51422::Write(
    byte* targetAddress,
    uint8_t* buffer,
    int length,
    unsigned long timeout
    )
{
  int retVal;
  bool result;

  // targetAddress is ignored with Bluetooth pairings

  // Wait for an previous send to complete
  retVal = WaitForSendComplete(timeout);
  if (retVal == 0) {
    xDBG_PRINT("Write: Previous send not complete!");
    return 0;
  }

  result = RFduinoBLE.send((char*)buffer, length);

  //
  // Wait for this send to complete
  //
  retVal = WaitForSendComplete(timeout);
  if (retVal == 0) {
      xDBG_PRINT("OS_nRF51422::Write Timeout waiting for send complete");
      return 0;
  }

  return length;
}

int
OS_nRF51422::TestRegisters()
{
  return 1;
}
