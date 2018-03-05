
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

#ifndef OS_nRF51422_h
#define OS_nRF51422_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#include <Arduino.h>
#include <inttypes.h>

#include <RFduinoBLE.h>

// This class implements MenloRadio
#include <MenloRadio.h>

//
// This is the maximum packet size for the radio
// and allows both small (16 byte) and large (32) byte
// sensor -> gateway update packets.
//
// See MenloSensorProtocol for details.
//
#define OS_PACKET_SIZE 32

class OS_nRF51422 : public MenloRadio {

public:
  
  OS_nRF51422();

  int Initialize(
      uint8_t payloadSize,
      char* deviceName,
      char* advertisementData,
      int advertisementInterval,
      int txPowerLevel
    );

  virtual bool DataReady();

  virtual void ResetReceiveBuffer();

  //
  // MenloRadio general buffer implementation
  //
   virtual int Read(
       byte *receivedFromAddress,
       uint8_t* buffer,
       int length,
       unsigned long timeout
   );

   virtual int Write(
       byte *targetAddress,
       uint8_t* buffer,
       int length,
       unsigned long timeout
       );

  //
  // MenloRadio fixed buffer implementation
  //
  virtual uint8_t* GetBuffer();

  virtual int GetBufferLength();

  virtual int Read(unsigned long timeout);

  virtual int Write(
      byte* targetAddress,
      unsigned long timeout
      );

  int TestRegisters();

  int RunRangeTestServer(byte* targetAddress);

  //
  // C callable so public.
  //
  void onReceive(char *data, int len);

private:

  // Set this to 1 and it compiles in tracing.
  // Set to 0 and the code should be left out.
  static const int trace = 1;
  static const int trace2 = 1;

  int WaitForDataReady(unsigned long timeout);

  int WaitForSendComplete(unsigned long timeout);

  //
  // Buffers
  //
  // If BufferLength == 0, the buffer
  // is available.
  //
  uint8_t m_receiveBuffer[OS_PACKET_SIZE];
  uint8_t m_receiveBufferLength;

  uint8_t m_sendBuffer[OS_PACKET_SIZE];
  uint8_t m_sendBufferLength;
};

#endif // OS_nRF51422_h
