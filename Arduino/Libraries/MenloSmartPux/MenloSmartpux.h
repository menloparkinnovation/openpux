
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
 * Copyright (C) 2013 Menlo Park Innovation LLC
 *
 *  Date: 12/01/2013
 *  File: MenloSmartPux.h
 *
 * MenloSmartpux is optimized around really small microcontrollers
 * such as the AtMega328. As such it uses minimal buffering, and
 * simple exchanges using compact message strings that are url
 * encoding style.
 *
 * MenloOpenPux is a more comprehensive JSON based cloud client
 * for larger memory microcontrollers such as Due, Photon,
 * or ArduinoMega.
 *
 */

#ifndef MenloSmartPux_h
#define MenloSmartPux_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#if defined(ARDUINO) && ARDUINO >= 100
// Arduino compatible TCP client interface
#include <Client.h>
#endif

#include "MenloPlatform.h"
#include "MenloWiFi.h"

// This class implements MenloCloud
#include <MenloCloud.h>

#include <MenloSensorProtocol.h>

// Http State Machine
#include "httpstate.h"

//
// Maximum buffer size sent to Cloud:
//
// Currently the AtMega version of the gateway and sensors
// use 16 bit sensor readings and masks. This is contributes
// to the maximum data sent at 4 hex digits per sensor reading.
//
// A=012345678&P=0123456789ABCDEF&S=12345678&  (41 chars)
// D0=1234&D1=1234&D2=1234&D3=1234&D4=1234&D5=1234&D6=1234&D7=1234&D8=1234&D9=1234& (80 chars)
// M0=1234&M1=1234&M2=1234&M3=1234 (31 chars)
//
// Total characters 152
//

#define DATA_STRING_SIZE 152

class MenloSmartPux : public MenloCloud {

 public:

  MenloSmartPux();

  int Initialize(
      Client* client,
      IPAddress cloudServerIP,
      uint16_t cloudServerPort,
      const char* account,
      const char* passcode,
      Stream* debugStream);

  //
  // MenloCloud implementations
  //
  virtual bool Process(MenloSensorProtocolDataAppResponseBuffer* data, int* valid);

  virtual bool QueueSensorReadings(MenloSensorProtocolDataAppBuffer* data);

  virtual bool IsBufferBusy();

  virtual bool CancelBuffer();

 private:

  int PerformHttpPost(MenloSensorProtocolDataAppBuffer* data);

  int HttpProcessResponseData(
      MenloSensorProtocolDataAppResponseBuffer* data,
      char* buf,
      int len
      );

  int OutputHttpContentBody(MenloSensorProtocolDataAppBuffer* data);

  void PrintHexUShort(uint16_t value);

  int DetermineHeadersLength();

  int DetermineContentLength(MenloSensorProtocolDataAppBuffer* data);

  int DetermineValidReadingsCount(MenloSensorProtocolDataAppBuffer* data);

  Client* m_client;
  IPAddress m_cloudServerIP;
  uint16_t m_cloudServerPort;
  const char* m_account;
  const char* m_passcode;
  Stream*  m_debugStream;
  int m_headersLength;

  bool m_lastConnected;
  long m_lastConnectionTime;

  char* m_contentBuffer;
  int   m_contentBufferLength;

  // Allocate extra byte for NULL terminator
  char m_dataBuffer[DATA_STRING_SIZE + 1];

  // TODO: Can reuse above buffer as a union
  HttpResponseState m_httpState;
  SensorResponseState m_sensorDocumentState;
  SensorResponseData m_sensorDocumentData;
};

#endif // MenloSmartPux_h
