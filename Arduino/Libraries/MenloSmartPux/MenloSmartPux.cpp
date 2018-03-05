
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
 *  File: MenloSmartPux.cpp
 */

#include "MenloPlatform.h"

#include <MenloDebug.h>

// This class implements MenloCloud
#include <MenloCloud.h>

#include <MenloSmartPux.h>

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (Serial.println(F(x)))
#define DBG_PRINT_INT(x) (Serial.println(x))
#define DBG_PRINT2(x, y) (Serial.print(F(x)) && Serial.println(y))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT2(x, y)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)     (Serial.println(F(x)))
#define xDBG_PRINT_INT(x) (Serial.println(x))
#define xDBG_PRINT2(x, y) (Serial.print(F(x)) && Serial.println(y))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT2(x, y)
#endif

MenloSmartPux::MenloSmartPux()
{
}

bool
MenloSmartPux::IsBufferBusy()
{
  if (m_contentBuffer != NULL) {
    return true;
  }
  else {
    return false;
  }
}

bool
MenloSmartPux::CancelBuffer()
{
  if (m_contentBuffer != NULL) {
    m_contentBuffer = NULL;
    return true;
  }
  else {
    return false;
  }
}

// This is data from the sensor to the cloud
bool
MenloSmartPux::QueueSensorReadings(MenloSensorProtocolDataAppBuffer* data)
{
    // Buffer is busy 
    if (m_contentBuffer != NULL) {

        if (m_debugStream) {
            m_debugStream->println(F("Smartpux: Buffer busy"));
        }

        return false;
    }

    m_contentBuffer = (char*)data;
    m_contentBufferLength = sizeof(MenloSensorProtocolDataAppBuffer);

    if (m_debugStream) {
        m_debugStream->println(F("Smartpux: Sensor Readings Queued to Cloud"));
    }

    return true;
}

int
MenloSmartPux::Initialize(
    Client* client,
    IPAddress cloudServerIP,
    uint16_t cloudServerPort,
    const char* account,
    const char* passcode,
    Stream* debugStream
    )
{
  m_client = client;
  m_cloudServerIP = cloudServerIP;
  m_cloudServerPort = cloudServerPort;

  //
  // Note: These must be stable buffers passed in from the
  // application as we do not make an extra copy here.
  //
  m_account = account;
  m_passcode = passcode;

  // Cache this
  m_headersLength = DetermineHeadersLength();

  m_debugStream = debugStream;

  m_contentBuffer = NULL;
  m_contentBufferLength = 0;

  // 60 seconds update internal to cloud server
  m_lastConnected = false;
  m_lastConnectionTime = millis();

  return 0;
}

//
// Performs an HTTP POST request to the Cloud server
//
int
MenloSmartPux::PerformHttpPost(MenloSensorProtocolDataAppBuffer* data)
{
  int contentLength;

  if (m_debugStream) {
      m_debugStream->print(F("MenloSmartpux: Attempting Server connection to "));
      m_debugStream->println(m_cloudServerIP);
  }

  //
  // TODO: Since there is no buffering, TCP flow control
  // can delay us longer than the watchdog timer.
  //
  // Need to fix the WiFi class to keep the watchdog reset
  // while waiting for SPI responses for internet transactions.
  //

  ResetWatchdog();   // Tell watchdog we are still alive

  if (m_client->connect(m_cloudServerIP, m_cloudServerPort)) {

    ResetWatchdog();   // Tell watchdog we are still alive

    m_lastConnected = true;

    if (m_debugStream) m_debugStream->println(F("Connected to Cloud Server..."));

    // Prepare for receiving the HTTP response document
    HttpResponseStateMachineReset(
        &m_httpState,
        &m_sensorDocumentState,
        &m_sensorDocumentData
        );

    //
    // Send the HTTP POST request with content type
    // application/x-www-form-urlencoded in the format
    // required for the www.smartpux.com sensor POST interface.
    //

    m_client->println(F("POST /smartpuxdata/data HTTP/1.1"));
    m_client->println(F("Host: data.smartpux.com"));
    m_client->println(F("Connection: close"));
    m_client->println(F("Content-Type: application/x-www-form-urlencoded"));

    // We must calculate our content length based on valid sensor readings
    contentLength = DetermineContentLength(data);

    m_client->print(F("Content-Length: "));
    m_client->println(contentLength);

    // Terminate headers, begin content section
    m_client->print("\n");

    ResetWatchdog();   // Tell watchdog we are still alive

    // write the content
    OutputHttpContentBody(data);

    // Terminate the content with two \n's
    m_client->print("\n\n");

    ResetWatchdog();   // Tell watchdog we are still alive

    // The main processing loop will process the host response

    if (m_debugStream) {
        m_debugStream->println(F("MenloSmartpux: Data send to Cloud"));
    }
  } 
  else {
    if (m_debugStream) {
        m_debugStream->println(F("MenloSmartpux: Server connection failed"));
    }

    ResetWatchdog();   // Tell watchdog we are still alive
    m_client->stop();
    ResetWatchdog();   // Tell watchdog we are still alive

    m_lastConnected = false;
  }

  // note the time that the connection was made or attempted:
  m_lastConnectionTime = millis();

  return 0;
}

//
// This returns 0 if data buffer does not have valid response data
// != 0 means data buffer has been filled in with valid cloud data.
//
int
MenloSmartPux::HttpProcessResponseData(
    MenloSensorProtocolDataAppResponseBuffer* data,
    char* buf,
    int len
    )
{
  int retval;

  //
  // the state variables are updated as the HTTP
  // response comes in either a byte at a time or
  // in chunks depending on the TCP implementation, etc.
  //
  // Until we are done, the internal state is changing,
  // so we can't use it to represent current cloud
  // status back to the sensors.
  //
  retval = HttpResponseStateMachine(
        &m_httpState,
        &m_sensorDocumentState,
        &m_sensorDocumentData,
        buf,
        len
        );

  if (retval == 0) {
    // Document(s) still being processed
    return 0;
  }

  // Done processing, check document
  if(!m_sensorDocumentData.documentOK) {
    // Invalid document, return now
    xDBG_PRINT("Invalid response document, done processing");
    return 0;
  }

  xDBG_PRINT("Valid response document, using values");

  //
  // We have a complete document in which the state
  // machine has declared valid. We can now copy any
  // updates from the Cloud server into our sensors
  // target states that will be used in future replies
  // to that sensor.
  //

  //
  // Process new readings
  //

  xDBG_PRINT2("command ", m_sensorDocumentData.command);
  data->Command = m_sensorDocumentData.command;

  xDBG_PRINT2("sleeptime ", m_sensorDocumentData.sleepTime);
  data->SleepTime = m_sensorDocumentData.sleepTime;

  xDBG_PRINT2("targetmask0 ", m_sensorDocumentData.targetMask0);
  data->TargetMask0 = m_sensorDocumentData.targetMask0;

  xDBG_PRINT2("targetmask1 ", m_sensorDocumentData.targetMask1);
  data->TargetMask1 = m_sensorDocumentData.targetMask1;

  xDBG_PRINT2("targetmask2 ", m_sensorDocumentData.targetMask2);
  data->TargetMask2 = m_sensorDocumentData.targetMask2;

  xDBG_PRINT2("targetmask3 ", m_sensorDocumentData.targetMask3);
  data->TargetMask3 = m_sensorDocumentData.targetMask3;

  data->TargetMask4 = m_sensorDocumentData.targetMask4;

  data->TargetMask5 = m_sensorDocumentData.targetMask5;

  data->TargetMask6 = m_sensorDocumentData.targetMask6;

  data->TargetMask7 = m_sensorDocumentData.targetMask7;

  data->TargetMask8 = m_sensorDocumentData.targetMask8;

  data->TargetMask9 = m_sensorDocumentData.targetMask9;

  return 1;
}

int
MenloSmartPux::OutputHttpContentBody(MenloSensorProtocolDataAppBuffer* data)
{
  /*
  if( m_debugStream) {
      m_debugStream->print(F("A="));
      m_debugStream->print(m_account);

      m_debugStream->print(F("&P="));
      m_debugStream->print(m_passcode);

      m_debugStream->print(F("&S=0001"));
  }
  */

  m_client->print(F("A="));
  m_client->print(m_account);

  m_client->print(F("&P="));
  m_client->print(m_passcode);

  // Just Sensor 1 right now until we support multiple sensors
  m_client->print(F("&S=0001"));

  // Output any valid data readings

  if (data->Sensor0 != 0xFFFF) {
      m_client->print(F("&D0="));
      //      if (m_debugStream) m_debugStream->print(F("&D0="));
      PrintHexUShort(data->Sensor0);  
  }

  if (data->Sensor1 != 0xFFFF) {
      m_client->print(F("&D1="));
      //      if (m_debugStream) m_debugStream->print(F("&D1="));
      PrintHexUShort(data->Sensor1);  
  }

  if (data->Sensor2 != 0xFFFF) {
      m_client->print(F("&D2="));
      //      if (m_debugStream) m_debugStream->print(F("&D2="));
      PrintHexUShort(data->Sensor2);  
  }

  if (data->Sensor3 != 0xFFFF) {
      m_client->print(F("&D3="));
      //      if (m_debugStream) m_debugStream->print(F("&D3="));
      PrintHexUShort(data->Sensor3);  
  }

  if (data->Sensor4 != 0xFFFF) {
      m_client->print(F("&D4="));
      //      if (m_debugStream) m_debugStream->print(F("&D4="));
      PrintHexUShort(data->Sensor4);  
  }

  if (data->Sensor5 != 0xFFFF) {
      m_client->print(F("&D5="));
      //      if (m_debugStream) m_debugStream->print(F("&D5="));
      PrintHexUShort(data->Sensor5);  
  }

  if (data->Sensor6 != 0xFFFF) {
      m_client->print(F("&D6="));
      //      if (m_debugStream) m_debugStream->print(F("&D6="));
      PrintHexUShort(data->Sensor6);  
  }

  if (data->Sensor7 != 0xFFFF) {
      m_client->print(F("&D7="));
      //      if (m_debugStream) m_debugStream->print(F("&D7="));
      PrintHexUShort(data->Sensor7);  
  }

  if (data->Sensor8 != 0xFFFF) {
      m_client->print(F("&D8="));
      //      if (m_debugStream) m_debugStream->print(F("&D8="));
      PrintHexUShort(data->Sensor8);  
  }

  if (data->Sensor9 != 0xFFFF) {
      m_client->print(F("&D9="));
      //      if (m_debugStream) m_debugStream->print(F("&D9="));
      PrintHexUShort(data->Sensor9);  
  }

  // Output the masks
  m_client->print(F("&M0="));
  //  if (m_debugStream) m_debugStream->print(F("&M0="));
  PrintHexUShort(data->Mask0);  

  m_client->print(F("&M1="));
  //  if (m_debugStream) m_debugStream->print(F("&M1="));
  PrintHexUShort(data->Mask1);  

  m_client->print(F("&M2="));
  //  if (m_debugStream) m_debugStream->print(F("&M2="));
  PrintHexUShort(data->Mask2);  

  m_client->print(F("&M3="));
  //  if (m_debugStream) m_debugStream->print(F("&M3="));
  PrintHexUShort(data->Mask3);  

  if (data->Mask4 != 0) {
      m_client->print(F("&M4="));
      PrintHexUShort(data->Mask4);  
  }

  if (data->Mask5 != 0) {
      m_client->print(F("&M5="));
      PrintHexUShort(data->Mask5);  
  }

  if (data->Mask6 != 0) {
      m_client->print(F("&M6="));
      PrintHexUShort(data->Mask6);  
  }

  if (data->Mask7 != 0) {
      m_client->print(F("&M7="));
      PrintHexUShort(data->Mask7);  
  }

  if (data->Mask8 != 0) {
      m_client->print(F("&M8="));
      PrintHexUShort(data->Mask8);  
  }

  if (data->Mask9 != 0) {
      m_client->print(F("&M9="));
      PrintHexUShort(data->Mask9);  
  }

  return 0;
}

int
MenloSmartPux::DetermineHeadersLength()
{
  int length;

  length = strlen(m_account) + strlen(m_passcode);
  length += 5; // A=&P=

  // Sensor &S=0000
  length += 7;

  return length;
}

int
MenloSmartPux::DetermineContentLength(MenloSensorProtocolDataAppBuffer* data)
{
  int length;
  int readingsCount;

  // Headers are pre-calculated for the account at initialize
  length = m_headersLength;

  //
  // All 16 bit uint16_t values are sent as four hex
  // digits padded out.
  //

  readingsCount = DetermineValidReadingsCount(data);
  
  // Each reading has &Dx= + 4 digits from padded 16 bit hex data.
  length += 8 * readingsCount;

  // We always send the 4 masks, format &Mx=
  // &Mx=xxxx (8 * 4)
  length += 32;

  return length;
}

int
MenloSmartPux::DetermineValidReadingsCount(MenloSensorProtocolDataAppBuffer* data)
{
  int count = 0;  

  //
  // This application uses 0xFFFF as "no reading" for sensors.
  // Masks are always valid and sent.
  //
  if (data->Sensor0 != 0xFFFF) count++;
  if (data->Sensor1 != 0xFFFF) count++;
  if (data->Sensor2 != 0xFFFF) count++;
  if (data->Sensor3 != 0xFFFF) count++;
  if (data->Sensor4 != 0xFFFF) count++;
  if (data->Sensor5 != 0xFFFF) count++;
  if (data->Sensor6 != 0xFFFF) count++;
  if (data->Sensor7 != 0xFFFF) count++;
  if (data->Sensor8 != 0xFFFF) count++;
  if (data->Sensor9 != 0xFFFF) count++;

  return count;
}

//
// Print hex padded out.
//
// This allows control of the HTTP length without
// having to buffer the whole HTTP content body, or manipulate
// a print stream into a set of temporary buffers.
//

// TODO: Figure out how to use PROG_MEM for initialized constants
const char* HexChars = "0123456789ABCDEF";

void
MenloSmartPux::PrintHexUShort(uint16_t value)
{
    uint8_t index;

    // TODO: Looks like this needs to be buffered on the raw TCP connection

    index = ((value >> 12) & 0x000F);
    m_client->print(HexChars[index]);
    //    if (m_debugStream) m_debugStream->print(HexChars[index]);
  
    index = ((value >> 8) & 0x000F);
    m_client->print(HexChars[index]);
    //    if (m_debugStream) m_debugStream->print(HexChars[index]);

    index = ((value >> 4) & 0x000F);
    m_client->print(HexChars[index]);
    //    if (m_debugStream) m_debugStream->print(HexChars[index]);

    index = (value & 0x000F);
    m_client->print(HexChars[index]);
    //    if (m_debugStream) m_debugStream->print(HexChars[index]);
}

//
// This outputs data received from the cloud
//
bool
MenloSmartPux::Process(
    MenloSensorProtocolDataAppResponseBuffer* data,
    int* valid
    )
{
  bool hadData = false;

  *valid = 0;

  //
  // For debugging dump and response from the Smartpux
  // Cloud which can contain error and/or debug trace
  // messages.
  //
  if (m_client->available() > 0) {

    hadData = true;

    xDBG_PRINT("Process: Cloud Data ready From Server, getting TCP chunks");

    while (m_client->available() > 0) {
      ResetWatchdog();   // Tell watchdog we are still alive
      char c = m_client->read();

#if DBG_PRINT_ENABLED_X
      if (c == '\r') {
         DBG_PRINT_INT("\\r");
      }
      else if (c == '\n') {
         DBG_PRINT_INT("\\n");
      }
      else {
         DBG_PRINT_INT(c);
      }
#endif

      // Process it in the response state machine
      if (HttpProcessResponseData(data, &c, 1)) {
	*valid = 1;
      }
    }

    Serial.println(F(""));

    xDBG_PRINT("Process: no more TCP chunks from server");
  }

  //
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  //
  if (!m_client->connected() && m_lastConnected) {

    Serial.println();
    xDBG_PRINT("Process: Disconnecting from server");

    m_client->stop();

    m_lastConnected = false;
  }

  if(m_contentBuffer != NULL) {

        if (m_debugStream) {
            m_debugStream->println(F("Smartpux: Sending Sensor Data"));
        }

        PerformHttpPost((MenloSensorProtocolDataAppBuffer*)m_contentBuffer);

        if (m_debugStream) {
            m_debugStream->println(F("Smartpux: Finished sending sensor data"));
        }

        // Sensor data has been sent to the Smartpux Cloud server

        //
        // Note: This must point to a static buffer or a memory leak will result
        //
        // To increase reliability, the main application uses a fixed content
	// buffer for sensor send data strings.
        //
        m_contentBuffer = NULL;
        hadData = true;
  }

  // Store the current connection state
  m_lastConnected = m_client->connected();

  return hadData;
}
