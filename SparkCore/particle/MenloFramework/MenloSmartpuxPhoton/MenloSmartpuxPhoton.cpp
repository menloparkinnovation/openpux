
//
//   Openpux: The Operating System for IoT
//
//   MenloSmartPuxPhoton.cpp
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014,2015 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Snapshot License
//
//   This license is for a specific snapshot of a base work of
//   Menlo Park Innovation LLC on a non-exclusive basis with no warranty
//   or obligation for future updates. This work, any portion, or derivative
//   of it may be made available under other license terms by
//   Menlo Park Innovation LLC without notice or obligation to this license.
//
//   There is no warranty, statement of fitness, statement of
//   fitness for any purpose, and no statements as to infringements
//   on any patents.
//
//   Menlo Park Innovation has no obligation to offer support, updates,
//   future revisions and improvements, source code, source code downloads,
//   media, etc.
//
//   This specific snapshot is made available under the following license:
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

//
// This provides support for the Particle Photon.
//
// This is the simplest implementation without requiring
// bringing in the MenloFramwork and can be readily dropped
// into any Photon project.
//

#include "MenloSmartpuxPhoton.h"

MenloSmartpuxPhoton::MenloSmartpuxPhoton()
{
    m_buffer = "";
    m_debug = false;
}

MenloSmartpuxPhoton::~MenloSmartpuxPhoton()
{
}

//
// This is exposed to allow runtime setting, such as through
// a Particle.function() call.
//
void
MenloSmartpuxPhoton::SetDebug(bool value)
{
    m_debug = value;
}

int
MenloSmartpuxPhoton::Initialize(
        String host,
        int port,
        String token,
        String accountId,
        String sensorId
        )
{
    m_host = host;
    m_port = port;
    m_token = token;
    m_accountId = accountId;
    m_sensorId = sensorId;

    // Reset to start initial parameters
    Reset();

    return 0;
}

//
// timeout is in milliseconds()
//
int
MenloSmartpuxPhoton::Post(
    unsigned long timeout
    )
{
    int c;
    int retVal;
    unsigned int index;

    TCPClient client;

    //
    // Clear the response buffer as we are starting a new request exchange
    // with the server.
    //
    m_response[0] = '\0';
    m_responseLength = 0;

    // Generate the headers, URL, and content document
    String postBody = generatePostBody();

    // Attempt to connect to the host
    retVal = client.connect(m_host, m_port);
    if (retVal == 0) {
        // error connecting

        if (m_debug) {
            Serial.print("MenloSmartpux: Could not connect to server ");
            Serial.print(m_host);
            Serial.print(m_host);
            Serial.print(" port ");
            Serial.println(m_port);
        }

        // ensures we don't leave a half connection hanging
        client.stop();
        return -1;
    }

    if (m_debug) {
        Serial.print("MenloSmartpux: Connected to server ");
        Serial.print(m_host);
        Serial.print(" port ");
        Serial.println(m_port);
    }

    if (m_debug) {
        Serial.println("posting document:");
        Serial.println(postBody);
    }

    // Send the POST
    client.print(postBody);

    if (m_debug) {
        Serial.println("MenloSmartpux: Done posting document body");

        // Hack! remove!
        delay(1000);
    }

    //
    // Now get any received document response
    //
    index = 0;

    while (client.connected() && (client.available() || (timeout-- > 0))) {

        c = client.read();
        if (c == (-1)) {
            // This ensures the timeout value represents a count of milliseconds
            delay(1);
            continue;
        }

        // Leave room for terminating NUL '\0'
        if (index < (sizeof(m_response) - 1)) {

            // add to response buffer
            m_response[index++] = c;
        }
        else {

            //
            // we continue to drain characters until timeout as to
            // not reset/hang the connection.
            //
        }
    }

    m_responseLength = index;
    m_response[index] = '\0'; // ensure NUL terminated

    //
    // We are done with the connection, close it now before any
    // serial output so we don't fire off any timeouts
    //
    if (m_debug) {
        Serial.println("calling client.stop...");
    }

    client.stop();

    if (index == (sizeof(m_response) - 1)) {

        //
        // Caller can test for overflow by
        // getResponseLength() == (getResponseBufferSize() - 1)
        //
        if (m_debug) {
            Serial.println("MenloSmartpux: maximum document size received, truncated");
        }
    }

    if (m_debug) {
        Serial.println("MenloSmartpux: response document before string search:");
        Serial.print("responseLength ");
        Serial.print(m_responseLength);
        Serial.println(" responseDocument:");
        Serial.println(m_response);
    }

    // search for the 200 or 201 OK responses
    if (strstr(m_response, "200 OK") || strstr(m_response, "201 OK")) {
        retVal = 1;

        //
        // The m_response buffer can be examined by the caller
        // to process the response contents which are in
        // querystring form.
        //

        if (m_debug) {
            Serial.println("MenloSmartpux: OK response");
            Serial.print("responseLength ");
            Serial.print(m_responseLength);
            Serial.println(" responseDocument:");
            Serial.println(m_response);
        }
    }
    else if (strstr(m_response, "400 Bad Request")) {
        retVal = -1;

        if (m_debug) {
            Serial.println("MenloSmartpux: Bad Request");
            Serial.print("responseLength ");
            Serial.print(m_responseLength);
            Serial.println(" responseDocument:");
            Serial.println(m_response);
        }
    }
    else {
        // error of some sort
        retVal = -1;
    }

    // Reset the send buffer for a new set of values
    Reset();

    return retVal;
}

char*
MenloSmartpuxPhoton::getResponse()
{
    return m_response;
}

int
MenloSmartpuxPhoton::getResponseLength()
{
    return m_responseLength;
}

int
MenloSmartpuxPhoton::getResponseBufferSize()
{
    return sizeof(m_response);
}

void
MenloSmartpuxPhoton::Reset()
{
    // Destructor will free memory of the existing buffer
    m_buffer = "";

    //
    // Start the pre-amble
    //
    // Note abbreviated codes are accepted for really small
    // devices or SMS/text message transports, but here
    // we do not need to. The Smartpux server accepts
    // both forms.
    //
    m_buffer += "AccountID=" + m_accountId + "&";
    m_buffer += "PassCode=" + m_token + "&";
    m_buffer += "SensorID=" + m_sensorId;

    //
    // We don't clear the m_response buffer as it gets
    // cleared when Post() is called. This allows callers
    // to clear for new values, but still process the
    // previous response without having to use an additional
    // buffer.
    //

    return;
}

void
MenloSmartpuxPhoton::add(String name, String value)
{
    m_buffer += "&" + name + "=" + value;
}

void
MenloSmartpuxPhoton::add(String name, int value)
{
    // Use the String() constructor to perform any conversions required
    m_buffer += "&" + name + "=" + String(value);
}

void
MenloSmartpuxPhoton::add(String name, long value)
{
    m_buffer += "&" + name + "=" + String(value);
}

void
MenloSmartpuxPhoton::add(String name, unsigned long value)
{
    m_buffer += "&" + name + "=" + String(value);
}

void
MenloSmartpuxPhoton::add(String name, float value, unsigned int precision)
{
    String str(value, precision);

    m_buffer += "&" + name + "=" + str;
}

void
MenloSmartpuxPhoton::add(String name, double value, unsigned int precision)
{
    String str(value, precision);

    m_buffer += "&" + name + "=" + str;
}

String
MenloSmartpuxPhoton::generatePostBody()
{
    String values = m_buffer;

    //
    // Outline from MenloFramework MenloSmartpux, httpstate.cpp
    //

    String body = "";

    body += "POST /smartpuxdata/data HTTP/1.1\n";
    body += "Host: data.smartpux.com\n";
    body += "Connection: close\n";
    body += "Content-Type: application/x-www-form-urlencoded\n";
    body += "Content-Length: " + String(m_buffer.length()) + "\n";

    // Terminate headers, begin content section
    body += "\n";

    // write the content
    body += values;

    // Terminate the content with two \n's
    body += "\n\n";

    return body;
}
