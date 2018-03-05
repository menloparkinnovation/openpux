
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
 *  Date: 11/25/2012
 *  File: OS_Cosm.cpp
 *
 * Code contained within comes from the following open, shared, community,
 * public domain, or other licenses:
 *
 * Arduino example sketch for Cosm/Pachube in the IDE.
 *
 * Updates:
 *  08/25/2013 - Implemented runtime debugging print outs.
 */

//------------------ Begin Public Domain Example Code ------------------------
/*
  Wifi Cosm sensor client
 
 This sketch connects an analog sensor to Cosm (http://www.cosm.com)
 using an Arduino Wifi shield.
 
 This example is written for a network using WPA encryption. For 
 WEP or WPA, change the Wifi.begin() call accordingly.
 
 This example has been updated to use version 2.0 of the Cosm.com API. 
 To make it work, create a feed with a datastream, and give it the ID
 sensor1. Or change the code below to match your feed.
 
 Circuit:
 * Analog sensor attached to analog in 0
 * Wifi shield attached to pins 10, 11, 12, 13
 
 created 13 Mar 2012
 modified 31 May 2012
 by Tom Igoe
 
 This code is in the public domain.
 
 */
//------------------ End Public Domain Example Code --------------------------

/*
 * Cosm Client class
 */

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

//
// Include Menlo Debug library support
//
#include <Debug.h>

// This class implements MenloCloud
#include <MenloCloud.h>

// This libraries header
#include <OS_Cosm.h>

// 12/01/2013 See:
//https://xively.com/dev/docs/api/communicating/
//

OS_Cosm::OS_Cosm() : 
  m_cosmServer(216,52,233,121) // COSM static IP address
  // m_cosmServer(173,203,98,29) // 12/01/2013 xively static IP address
  //m_cosmServer(216,52,233,120) // COSM static IP address ping api.xively.com
{

}

int
OS_Cosm::Initialize(
    WiFiClient* networkClient,
    char* userAgent,
    char* apiKey,
    int feedID,
    Stream* debugStream
    )
{
  m_debugStream = debugStream;
  m_client = networkClient;
  m_userAgent = userAgent;
  m_apiKey = apiKey;
  m_feedID = feedID;

  m_contentBuffer = NULL;

  // 30 seconds retry
  m_postingInterval = (1000 * 30);
  m_lastConnected = false;
  m_lastConnectionTime = millis();

  return 0;
}

//
// Starts an HTTP PUT request to the COSM server
// with contentLength set to the client supplied value.
//
// The client is expected to follow up with exactly
// contentLength bytes onto the output stream.
//
int
OS_Cosm::StartHttpPut(int contentLength)
{
  if (m_debugStream) {
      m_debugStream->print(F("OS_COSM: Attempting Server connection to "));
      m_debugStream->println(m_cosmServer);
  }

  // if there's a successful connection:
  if (m_client->connect(m_cosmServer, 80)) {

    m_lastConnected = true;

    if (m_debugStream) m_debugStream->println(F("OS_COSM: Connected to Server..."));

    // send the HTTP PUT request:
    m_client->print(F("PUT /v2/feeds/"));
    m_client->print(m_feedID);
    //m_client->println(F(".json HTTP/1.1")); // 12/01/2013 XivelyClient.cpp line 14
    m_client->println(F(".csv HTTP/1.1"));
    //m_client->println(F("Host: api.cosm.com"));
    m_client->println(F("Host: api.xively.com"));
    m_client->print(F("X-ApiKey: "));
    m_client->println(m_apiKey);
    m_client->print(F("User-Agent: "));
    m_client->println(m_userAgent);
    m_client->print(F("Content-Length: "));

    m_client->println(contentLength);

    // last pieces of the HTTP PUT request:
    m_client->println(F("Content-Type: text/csv"));
    m_client->println(F("Connection: close"));
    m_client->println();
  } 
  else {
    if (m_debugStream) {
        m_debugStream->println(F("OS_COSM: Server connection failed"));
        m_debugStream->println();
    }

    m_client->stop();
    m_lastConnected = false;
  }
   // note the time that the connection was made or attempted:
  m_lastConnectionTime = millis();
}

int
OS_Cosm::SendDataString(char* dataString)
{
    // Buffer is busy 
    if (m_contentBuffer != NULL) {
       return 1;
    }

    m_contentBuffer = dataString;
}

int
OS_Cosm::PerformHttpRequest(char* dataString)
{
  int retval;
  int contentLength;

  //
  // calculate the length of the sensor reading in bytes:
  // 8 bytes for "sensor1," + number of digits of the data:
  //
  contentLength = 8 + strlen(dataString);

  retval = StartHttpPut(contentLength);
  if (retval != 0) {
    return retval;
  }

  // here's the actual content of the PUT request:
  m_client->print("sensor1,");
  m_client->println(dataString);

  if (m_debugStream) m_debugStream->println(F("OS_COSM: Content sent"));

  return 0;
}

bool
OS_Cosm::Process()
{
  bool hadData = false;

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (m_client->available() > 0) {
    hadData = true;
    Serial.println(F("COSM: Data From Server:"));
    while (m_client->available() > 0) {
      char c = m_client->read();
      Serial.print(c);
    }
    Serial.println(F(""));
    Serial.println(F("COSM: End Data From Server:"));
  }

  //
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  //
  if (!m_client->connected() && m_lastConnected) {
    Serial.println();
    Serial.println(F("COSM: disconnecting."));
    m_client->stop();
    m_lastConnected = false;
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data if any.
  if(!m_client->connected() &&
     (m_contentBuffer != NULL) &&
     (millis() - m_lastConnectionTime > m_postingInterval)) {

        Serial.print(F("COSM: Sending Sensor Data :"));
        Serial.print(m_contentBuffer);
        Serial.println(F(":"));
        PerformHttpRequest(m_contentBuffer);
        Serial.println(F("COSM: Sensor Data Sent"));

        // Sensor Data has been sent to COSM
        // Note: This must point to a static buffer
        m_contentBuffer = NULL;
        hadData = true;
  }

  // store the state of the connection for next time through
  // the loop:
  m_lastConnected = m_client->connected();

  return hadData;
}

//
// This method calculates the number of digits in the
// sensor reading.  Since each digit of the ASCII decimal
// representation is a byte, the number of digits equals
// the number of bytes:
//
int
OS_Cosm::GetLength(int data)
{
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten, 
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = data /10;
  while (dividend > 0) {
    dividend = dividend /10;
    digits++;
  }
  // return the number of digits:
  return digits;
}
