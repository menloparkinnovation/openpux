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
// bringing in the MenloFramwork.
//

#include "MenloSmartpuxPhoton.h"

MenloSmartpuxPhoton::MenloSmartpuxPhoton()
{
    m_buffer = "";
}

MenloSmartpuxPhoton::~MenloSmartpuxPhoton()
{
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
    char c;
    int retVal;
    int index = 0;

    TCPClient client;

    m_response[0] = '\0';

    // Generate the headers, URL, and content document
    String postBody = generatePostBody();

    // Attempt to connect to the host
    retVal = client.connect(m_host, m_port);
    if (retVal == 0) {
        // error connecting

        // ensures we don't leave a half connection hanging
        client.stop();
        return -1;
    }

    // Send the POST
    client.print(postBody);

    //
    // Now get any received document response
    //
    while (client.available() || (timeout-- > 0)) {

        c = client.read();

        // add to reponse buffer
        m_response[index++] = c;

        // This ensures the timeout value represents something other than a spinloop
        delay(1);
    }

    m_response[index++] = '\0'; // ensure NUL terminated

    // search for the 200 or 201 OK responses
    if (strstr(m_response, "200 OK") || strstr(m_response, "201 OK")) {
        retVal = 1;

        //
        // The m_response buffer can be examined by the caller
        // to process the response contents which are in
        // querystring form.
        //
    }
    else if (strstr(m_response, "400 Bad Request")) {
        retVal = -1;
    }
    else {
        // error of some sort
        retVal = -1;
    }

    client.stop();

    // Reset the buffer for a new set of values
    Reset();

    return retVal;
}


char*
MenloSmartpuxPhoton::getResponse()
{
    return m_response;
}

void
MenloSmartpuxPhoton::Reset()
{
    // Destructor will free memory of the existing buffer
    m_buffer = "";

    // Start the pre-amble
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
    // chop off the leading "&"
    String values = m_buffer;

    //
    // Outline from MenloFramework MenloSmartpux, httpstate.cpp
    //

    String body = "";

    body += "POST /smartpuxdata/data HTTP/1.1";
    body += "Host: data.smartpux.com";
    body += "Connection: close";
    body += "Content-Type: application/x-www-form-urlencoded";
    body += "Content-Length: " + String(m_buffer.length());

    // Terminate headers, begin content section
    body += "\n";

    // write the content
    body += values;

    // Terminate the content with two \n's
    body += "\n\n";

    return body;
}
