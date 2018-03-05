
/**
 * Phant.cpp
 *
 *             .-.._
 *       __  /`     '.
 *    .-'  `/   (   a \
 *   /      (    \,_   \
 *  /|       '---` |\ =|
 * ` \    /__.-/  /  | |
 *    |  / / \ \  \   \_\  jgs
 *    |__|_|  |_|__\
 *    never   forget.
 *
 * Original Author: Todd Treece <todd@sparkfun.com>
 * Edited for Particle by: Jim Lindblom <jim@sparkfun.com>
 *
 * Copyright (c) 2014 SparkFun Electronics.
 * Licensed under the GPL v3 license.
 *
 */

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
//   Same license as the code from SparkFun above so as not to conflict
//   with the original licensing of the code.
//
//     >> Licensed under the GPL v3 license.
//

#include "SparkFunPhant.h"
#include <stdlib.h>

// MenloFramework
#include "MenloPlatform.h"
#include "MenloDebug.h"
#include "MenloTimer.h"
#include "MenloCloudScheduler.h"
#include "MenloCloudFormatter.h"

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
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

Phant::Phant()
{
}

int
Phant::Initialize(PhantConfig* config)
{

  m_config = *config;

  _host = config->host;
  _pub = config->publicKey;
  _prv = config->privateKey;

  _port = config->port;
  _responseLength = 0;
  _response[0] = '\0';

  _debug = false;

  // Reset to start initial parameters
  Reset();

  // Call base class initialization
  MenloCloudFormatter::Initialize(m_config.updateRate);

  return 1;
}

//
// This is exposed to allow runtime setting, such as through
// a Particle.function() call.
//
void
Phant::SetDebug(bool value)
{
    _debug = value;
}

bool
Phant::IsConnected()
{
    if ((m_config.wifi == NULL) ||
        !m_config.wifi->IsConnected()) {
        return false;
    }
    else {
        return true;
    }
}

void
Phant::StartPreAmble()
{
    // No preamble required for Phant
    return;
}

String Phant::url()
{
  String params = getDataBuffer();

  String result = "http://" + _host + "/input/" + _pub + ".txt";
  result += "?private_key=" + _prv + params;

  return result;
}

String Phant::get() {

  String result = "GET /output/" + _pub + ".csv HTTP/1.1\n";
  result += "Host: " + _host + "\n";
  result += "Connection: close\n";

  return result;
}

String Phant::getPostBody() {

    String buffer = getDataBuffer();

    String result = "POST /input/" + _pub + ".txt HTTP/1.1\n";
    result += "Host: " + _host + "\n";
    result += "Phant-Private-Key: " + _prv + "\n";
    result += "Connection: close\n";
    result += "Content-Type: application/x-www-form-urlencoded\n";

    result += "Content-Length: " + String(buffer.length()) + "\n\n";

    result += buffer;

    return result;
}

String Phant::ResetHttpConnection() {

  String result = "DELETE /input/" + _pub + ".txt HTTP/1.1\n";
  result += "Host: " + _host + "\n";
  result += "Phant-Private-Key: " + _prv + "\n";
  result += "Connection: close\n";

  return result;

}

//
// Menlo re-written post function.
//
// menloparkinnovation.com
// menloparkinnovation@gmail.com
//
// - Properly handles TCP segment framing and delays
//
// - Properly ensures NUL termination of response buffers
//   before searching them with strstr
//
// - Allow the proper HTTP response code for new item create 201
//
// - Support configurable timeout
//
// - Support configurable debug
//
int Phant::postToPhant(
    unsigned long timeout
    )
{
    int c;
    int retVal;
    unsigned int index;

// TODO: #ifdef is not the right strategy, but getting the platform working.
#if ESP8266
    // Arduino style pattern
    WiFiClient client;
#else
    // Particle Photon
    TCPClient client;
#endif

    //
    // If the WiFi is not set, or not connected, return error.
    //
    if ((m_config.wifi == NULL) ||
        !m_config.wifi->IsConnected()) {

            DBG_PRINT("SparkFunPhant Post No WiFi connection");

            if (_debug) {
                Serial.print(F("SparkFunPhant Post No WiFi connection"));
            }

            return -1;
    }

    xDBG_PRINT("Phant::postToPhant Posting");

    //
    // Clear the response buffer as we are starting a new request exchange
    // with the server.
    //
    _response[0] = '\0';
    _responseLength = 0;

    // Generate the headers, URL, and content document
    String postBody = this->getPostBody();

    // Attempt to connect to the host
    //retVal = client.connect(_host, _port);
    retVal = client.connect(_host.c_str(), _port);
    if (retVal == 0) {
        // error connecting

        DBG_PRINT("Phant::postToPhant error connecting to server");

        if (_debug) {
            Serial.print("Phant: Could not connect to server ");
            Serial.print(_host);
            Serial.print(_host);
            Serial.print(" port ");
            Serial.println(_port);
        }

        // ensures we don't leave a half connection hanging
        client.stop();
        return -1;
    }

    if (_debug) {
        Serial.print("Phant: Connected to server ");
        Serial.print(_host);
        Serial.print(" port ");
        Serial.println(_port);
    }

    if (_debug) {
        Serial.println("posting document:");
        Serial.println(postBody);
    }

    // Send the POST
    client.print(postBody);

    if (_debug) {
        Serial.println("Phant: Done posting document body");
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
        if (index < (sizeof(_response) - 1)) {

            // add to response buffer
            _response[index++] = c;
        }
        else {

            //
            // we continue to drain characters until timeout as to
            // not reset/hang the connection.
            //
        }
    }

    _responseLength = index;
    _response[index] = '\0'; // ensure NUL terminated

    //
    // We are done with the connection, close it now before any
    // serial output so we don't fire off any timeouts
    //
    if (_debug) {
        Serial.println("calling client.stop...");
    }

    client.stop();

    if (index == (sizeof(_response) - 1)) {

        //
        // Caller can test for overflow by
        // getResponseLength() == (getResponseBufferSize() - 1)
        //
        if (_debug) {
            Serial.println("Phant: maximum document size received, truncated");
        }
    }

    if (_debug) {
        Serial.println("Phant: response document before string search:");
        Serial.print("responseLength ");
        Serial.print(_responseLength);
        Serial.println(" responseDocument:");
        Serial.println(_response);
    }

    //
    // search for the 200 or 201 OK responses
    //
    // Note: 201 is the correct response for a new item added, but
    // many servers return generic 200 OK
    //
    if (strstr(_response, "200 OK") || strstr(_response, "201 OK")) {
        retVal = 1;

        //
        // The _response buffer can be examined by the caller
        // to process the response contents.
        //

        if (_debug) {
            Serial.println("Phant: OK response");
            Serial.print("responseLength ");
            Serial.print(_responseLength);
            Serial.println(" responseDocument:");
            Serial.println(_response);
        }
    }
    else if (strstr(_response, "400 Bad Request")) {
        retVal = -1;

        if (_debug) {
            Serial.println("Phant: Bad Request");
            Serial.print("responseLength ");
            Serial.print(_responseLength);
            Serial.println(" responseDocument:");
            Serial.println(_response);
        }
    }
    else {
        // error of some sort
        retVal = -1;
    }

    return retVal;
}

//
// This is invoked by MenloCloudFormatter::Format()
//
// timeout is in milliseconds()
//
int
Phant::Post(
    unsigned long timeout
    )
{
    int retVal;

    retVal = postToPhant(timeout);

    // Reset the send buffer for a new set of values
    Reset();

    return retVal;
}
