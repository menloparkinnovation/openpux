
//
// 06/20/2016
//
// TODO: Use String.reserve() to pre-allocate memory
// where String() can't be avoided.
//   - Re-use the string buffer where required, such as float formatting.
//
// TODO: Make this stream based and stream out the TCP port
// so that large dynamic memory buffers are not required for
// the final output.
//
//   - Though this would require the two pass steam model in which
//     the first pass gets the document size, and the second
//     posts it. This is because content length is needed before
//     the body content, unless other HTTP options are used so
//     that content length can be skipped.
//

//
//   Openpux: The Operating System for IoT
//
//   MenloSmartpuxCloud.h
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2016 Menlo Park Innovation LLC
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

/*
 *  Date: 02/28/2016
 *  File: MenloSmartpuxCloud
 */

#include "MenloSmartpuxCloud.h"

#define DBG_PRINT_ENABLED 1

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
#define XDBG_PRINT_ENABLED 1

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

//
// Note:
//
// The setting sizes are set to allow full sensor configuration
// in 1024 bytes of EEPROM space.
//
// Sizes for settings come from MenloConfigStore.h and are
// the base settings useful for most smartpux deployments.
//
// Though Smartpux/Openpux are designed to fit within the length
// limits of the base EEPROM storage, some deployments may face
// long URL's for public cloud hosting, co-tenancy, etc.
//
// In addition this WiFi cloud provider is the reference for
// creating others using the MenloFramework, and those clouds
// may have requirements on long authentication tokens, etc.
//
// For microcontrollers with more than 1024 bytes of configuration
// storage there are _EXT_ versions of the cloud settings to allow
// for servers with long URL's, DNS paths, account identity, etc.
//
// Change the definitions here and in MenloSmartpuxCloud.h as required
// if these larger values are used.
//
// Note that Dweet length limits would involve setting the value
// in multiple steps. Example would be SPXURL0, SPXURL1, SPXURL2, etc.
// to transfer a large URL 53 characters at a time to fit within
// NEMA 0183 maximum sentence length limits which Dweet adheres to
// in order to manage the buffer space on small microcontrollers.
//
// Example:
//
// This is 27 characters
// $PDWT,SETCONFIG=SPXURL0:*00
//
// This is the maximum 80 characters
// $PDWT,SETCONFIG=SPXURL0:/long/url/to/application/path/usually/guid/embedded/*00
//

const char spx_module_name_string[] PROGMEM = "SpxCloud";

// SETCONFIG=SPXSERVER:www.smartpux.com
extern const char dweet_spx_server_string[] PROGMEM = "SPXSERVER";

// SETCONFIG=SPXURL:/data
extern const char dweet_spx_url_string[] PROGMEM = "SPXURL";

// SETCONFIG=SPXPORT:80
extern const char dweet_spx_port_string[] PROGMEM = "SPXPORT";

// SETCONFIG=SPXTOKEN:12345678
extern const char dweet_spx_token_string[] PROGMEM = "SPXTOKEN";

// SETCONFIG=SPXACCOUNT:12345678
extern const char dweet_spx_account_string[] PROGMEM = "SPXACCOUNT";

// SETCONFIG=SPXSENSOR:12345678
extern const char dweet_spx_sensor_string[] PROGMEM = "SPXSENSOR";

// SETCONFIG=SPXOPTIONS:00
extern const char dweet_spx_options_string[] PROGMEM = "SPXOPTIONS";

const char* const spx_string_table[] PROGMEM =
{
  dweet_spx_server_string,
  dweet_spx_url_string,
  dweet_spx_port_string,
  dweet_spx_token_string,
  dweet_spx_account_string,
  dweet_spx_sensor_string,
  dweet_spx_options_string
};

// Locally typed version of state dispatch function
typedef int (MenloSmartpuxCloud::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod spx_function_table[] =
{
    &MenloSmartpuxCloud::SpxServer,
    &MenloSmartpuxCloud::SpxUrl,
    &MenloSmartpuxCloud::SpxPort,
    &MenloSmartpuxCloud::SpxToken,
    &MenloSmartpuxCloud::SpxAccount,
    &MenloSmartpuxCloud::SpxSensor,
    &MenloSmartpuxCloud::SpxOptions
};

//
// These are defined in MenloConfigStore.h
//
PROGMEM const int spx_index_table[] =
{
    CLOUD_SERVER,
    CLOUD_URL,
    CLOUD_PORT,
    CLOUD_TOKEN,
    CLOUD_ACCOUNT,
    CLOUD_SENSOR,
    0               // WIFI_OPTIONS is a placeholder right now
};

//
// The size table contains the length of the strings
// both in the configuration store, and for the runtime
// set/get state property functions.
//
// These sizes include the '\0' on the end.
//
PROGMEM const int spx_size_table[] =
{
    CLOUD_SERVER_SIZE,
    CLOUD_URL_SIZE,
    CLOUD_PORT_SIZE,
    CLOUD_TOKEN_SIZE,
    CLOUD_ACCOUNT_SIZE,
    CLOUD_SENSOR_SIZE,
    0               // WIFI_OPTIONS is a placeholder right now
};

//
// SETCONFIG=SPXSERVER:www.smartpux.com
//
int
MenloSmartpuxCloud::SpxServer(char* buf, int size, bool isSet)
{
    int length, index;

    if (isSet) {

        xDBG_PRINT_NNL("SPX Set SERVER ");
        xDBG_PRINT_STRING(buf);

        // MenloConfigStore.h defines the sizes
        length = strlen(buf);
        if (length > (CLOUD_SERVER_SIZE - 1)) {
            DBG_PRINT("SPX SERVER to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            m_configServer[index] = buf[index];
        }

        // Ensure null terminated string
        m_configServer[index] = '\0';

        xDBG_PRINT_NNL("SERVER ");
        xDBG_PRINT_STRING(m_configServer);

        return 0;
    }
    else {

        length = strlen(m_configServer) + 1;
        if (length > size) {
            DBG_PRINT("SPX Server to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            buf[index] = m_configServer[index];
        }

        // Ensure null terminated string
        buf[index] = '\0';

        return 0;
    }
}

int
MenloSmartpuxCloud::SpxUrl(char* buf, int size, bool isSet)
{
    int length, index;

    if (isSet) {

        xDBG_PRINT_NNL("SPX Set URL ");
        xDBG_PRINT_STRING(buf);

        // MenloConfigStore.h defines the sizes
        length = strlen(buf);
        if (length > (CLOUD_URL_SIZE - 1)) {
            DBG_PRINT("SPX URL to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            m_configUrl[index] = buf[index];
        }

        // Ensure null terminated string
        m_configUrl[index] = '\0';

        xDBG_PRINT_NNL("URL ");
        xDBG_PRINT_STRING(m_configUrl);

        return 0;
    }
    else {

        length = strlen(m_configUrl) + 1;
        if (length > size) {
            DBG_PRINT("SPX URL to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            buf[index] = m_configUrl[index];
        }

        // Ensure null terminated string
        buf[index] = '\0';

        return 0;
    }
}

int
MenloSmartpuxCloud::SpxPort(char* buf, int size, bool isSet)
{
    //
    // Port is a 4 digit hex number
    //

    uint16_t port;

    if (isSet) {

        if (strlen(buf) < 4) {
            xDBG_PRINT("SPX Port less than 4 digits");
            return DWEET_INVALID_PARAMETER;
        }

        m_configPort = MenloUtility::HexToUShort(buf);

        return 0;
    }
    else {

        if (size < 5) {
            return DWEET_PARAMETER_TO_LONG;
        }

        port = m_configPort;

        MenloUtility::UInt16ToHexBuffer(port, buf);
        buf[4] = '\0';

        return 0;
    }
}

int
MenloSmartpuxCloud::SpxToken(char* buf, int size, bool isSet)
{
    int length, index;

    if (isSet) {

        xDBG_PRINT_NNL("SPX Set TOKEN ");
        xDBG_PRINT_STRING(buf);

        // MenloConfigStore.h defines the sizes
        length = strlen(buf);
        if (length > (CLOUD_TOKEN_SIZE - 1)) {
            DBG_PRINT("SPX TOKEN to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            m_configToken[index] = buf[index];
        }

        // Ensure null terminated string
        m_configToken[index] = '\0';

        xDBG_PRINT_NNL("TOKEN ");
        xDBG_PRINT_STRING(m_configToken);

        return 0;
    }
    else {

        length = strlen(m_configToken) + 1;
        if (length > size) {
            DBG_PRINT("SPX TOKEN to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            buf[index] = m_configToken[index];
        }

        // Ensure null terminated string
        buf[index] = '\0';

        return 0;
    }
}

int
MenloSmartpuxCloud::SpxAccount(char* buf, int size, bool isSet)
{
    int length, index;

    if (isSet) {

        xDBG_PRINT_NNL("SPX Set ACCOUNT ");
        xDBG_PRINT_STRING(buf);

        // MenloConfigStore.h defines the sizes
        length = strlen(buf);
        if (length > (CLOUD_ACCOUNT_SIZE - 1)) {
            DBG_PRINT("SPX ACCOUNT to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            m_configAccountId[index] = buf[index];
        }

        // Ensure null terminated string
        m_configAccountId[index] = '\0';

        xDBG_PRINT_NNL("ACCOUNT ");
        xDBG_PRINT_STRING(m_configAccountId);

        return 0;
    }
    else {

        length = strlen(m_configAccountId) + 1;
        if (length > size) {
            DBG_PRINT("SPX ACCOUNT to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            buf[index] = m_configAccountId[index];
        }

        // Ensure null terminated string
        buf[index] = '\0';

        return 0;
    }
}

int
MenloSmartpuxCloud::SpxSensor(char* buf, int size, bool isSet)
{
    int length, index;

    xDBG_PRINT_NNL("SPX SENSOR index ");
    xDBG_PRINT_INT(CLOUD_SENSOR);

    if (isSet) {

        DBG_PRINT_NNL("SPX Set SENSOR ");
        DBG_PRINT_STRING(buf);

        // MenloConfigStore.h defines the sizes
        length = strlen(buf);
        if (length > (CLOUD_SENSOR_SIZE - 1)) {
            DBG_PRINT("SPX SENSOR to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            m_configSensorId[index] = buf[index];
        }

        // Ensure null terminated string
        m_configSensorId[index] = '\0';

        xDBG_PRINT_NNL("SENSOR ");
        xDBG_PRINT_STRING(m_configSensorId);

        return 0;
    }
    else {

        length = strlen(m_configSensorId) + 1;
        if (length > size) {
            DBG_PRINT("SPX SENSOR to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            buf[index] = m_configSensorId[index];
        }

        // Ensure null terminated string
        buf[index] = '\0';

        return 0;
    }
}

int
MenloSmartpuxCloud::SpxOptions(char* buf, int size, bool isSet)
{
    return DWEET_ERROR_UNSUP;
}

MenloSmartpuxCloud::MenloSmartpuxCloud()
{
    m_configPort = 80;

    //
    // Set these to NULL in order to determine if the
    // settings are not configured from the EEPROM.
    //
    m_configServer[0] = '\0';
    m_configUrl[0] = '\0';
    m_configToken[0] = '\0';
    m_configAccountId[0] = '\0';
    m_configSensorId[0] = '\0';

    // Reset the response buffer.
    m_response[0] = '\0';

#if XDBG_PRINT_ENABLED
    m_debug = true;
#else
    m_debug = false;
#endif
}

MenloSmartpuxCloud::~MenloSmartpuxCloud()
{
}

bool
MenloSmartpuxCloud::IsConnected()
{
    if ((m_config.wifi == NULL) ||
        !m_config.wifi->IsConnected()) {
        return false;
    }
    else {
        return true;
    }
}

//
// This is exposed to allow runtime setting, such as through
// a Particle.function() call.
//
void
MenloSmartpuxCloud::SetDebug(bool value)
{
    m_debug = value;
}

int
MenloSmartpuxCloud::Initialize(
        MenloSmartpuxCloudConfig *config
        )
{
    int result;
    struct StateSettingsParameters parms;
    char workingBuffer[CLOUD_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(spx_string_table) / sizeof(char*);

    m_config = *config;

    if (m_config.defaultPort != 0) {
        m_configPort = m_config.defaultPort;
    }

    if (m_config.defaultHost != NULL) {
        strncpy(&m_configServer[0], m_config.defaultHost, sizeof(m_configServer));
        m_configServer[CLOUD_SERVER_SIZE - 1] = '\0';
    }

    if (m_config.defaultUrl != NULL) {
        strncpy(&m_configUrl[0], m_config.defaultUrl, sizeof(m_configUrl));
        m_configUrl[CLOUD_URL_SIZE - 1] = '\0';
    }

    if (m_config.defaultToken != NULL) {
        strncpy(&m_configToken[0], m_config.defaultToken, sizeof(m_configToken));
        m_configToken[CLOUD_TOKEN_SIZE - 1] = '\0';
    }

    if (m_config.defaultAccountId != NULL) {
        strncpy(&m_configAccountId[0], m_config.defaultAccountId, sizeof(m_configAccountId));
        m_configAccountId[CLOUD_ACCOUNT_SIZE - 1] = '\0';
    }

    if (m_config.defaultSensorId != NULL) {
        strncpy(&m_configSensorId[0], m_config.defaultSensorId, sizeof(m_configSensorId));
        m_configSensorId[CLOUD_SENSOR_SIZE - 1] = '\0';
    }

    // Reset to start initial parameters
    Reset();

    //
    // Register for unhandled Dweets arriving on any transport.
    //
    m_dweetEvent.object = this;
    m_dweetEvent.method = (MenloEventMethod)&MenloSmartpuxCloud::DweetEvent;

    MenloDweet::RegisterGlobalUnhandledDweetEvent(&m_dweetEvent);

    // Call base class initialization
    MenloCloudFormatter::Initialize(m_config.defaultUpdateRate);

    //
    // Load the configuration settings from EEPROM if valid
    //
    // Note: "this" is used to refer to this class (DweetWiFi) since
    // the handlers are on this class.
    //

    parms.ModuleName = (PGM_P)spx_module_name_string;
    parms.stringTable = (PGM_P)spx_string_table;
    parms.functionTable = (PGM_P)spx_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = spx_index_table;
    parms.sizeTable = spx_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;
    parms.checksumIndex = CLOUD_CHECKSUM;
    parms.checksumBlockStart = CLOUD_CHECKSUM_BEGIN;
    parms.checksumBlockSize = CLOUD_CHECKSUM_END - CLOUD_CHECKSUM_BEGIN;
    parms.name = NULL;
    parms.value = NULL;

    // DweetState.cpp
    result = MenloDweet::LoadConfigurationSettingsTable(&parms);
    if (result != 0) {
        if (result == DWEET_INVALID_CHECKSUM) {
            MenloDebug::Print(F("MenloSmartpuxCloud Stored settings checksum is invalid"));
        }
        else {
            MenloDebug::Print(F("MenloSmartpuxCloud Stored settings are invalid"));
            xDBG_PRINT_NNL("result is ");
            xDBG_PRINT_INT(result);
        }
    }
    else {
        MenloDebug::Print(F("MenloSmartpuxCloud Stored settings are valid"));
    }

    return 1;
}

void
MenloSmartpuxCloud::StartPreAmble()
{
    //
    // Start the pre-amble
    //

    //
    // Note abbreviated codes are accepted for really small
    // devices or SMS/text message transports, but here
    // we do not need to. The Smartpux server accepts
    // both forms.
    //
    if (IsShortForm()) {
        add("A", m_configAccountId);

        add("P", m_configToken);
        add("S", m_configSensorId);
    }
    else {
        add("AccountID", m_configAccountId);

        add("PassCode", m_configToken);
        add("SensorID", m_configSensorId);
    }
}

//
// This is invoked by MenloCloudFormatter::Format()
//
// timeout is in milliseconds()
//
int
MenloSmartpuxCloud::Post(
    unsigned long timeout
    )
{
    int c;
    int retVal;
    unsigned int index;

    xDBG_PRINT("MenloSmartpuxCloud::Post");

    //
    // If the WiFi is not set, or not connected, return error.
    //
    if ((m_config.wifi == NULL) ||
        !m_config.wifi->IsConnected()) {

            DBG_PRINT("MenloSmartpux Post No WiFi connection");

            if (m_debug) {
                Serial.print(F("MenloSmartpux Post No WiFi connection"));
            }

            return -1;
    }

    if (m_configServer[0] == '\0') {

        DBG_PRINT("MenloSmartpux Post no server configured");

        if (m_debug) {
            Serial.print(F("MenloSmartpux Post no server configured"));
        }

        return -1;
    }

    //
    // Note: Arduino WiFiClient is used directly here right now though
    // MenloWifiArduino handles configuration, setup, etc.
    //
#if MENLO_BOARD_SPARKCORE
    // Particle Photon follows the Arduino interface, but has a different class name.
    TCPClient client;
#else
    // Arduino style pattern and class name.
    WiFiClient client;
#endif

    //
    // Clear the response buffer as we are starting a new request exchange
    // with the server.
    //
    m_response[0] = '\0';
    m_responseLength = 0;

    // Generate the headers, URL, and content document
    String postBody = generatePostBody();

    // Attempt to connect to the host
    retVal = client.connect(m_configServer, m_configPort);

    if (retVal == 0) {
        // error connecting

        DBG_PRINT("MenloSmartpuxCloud::Post error connecting to server");

        if (m_debug) {
            Serial.print("MenloSmartpux: Could not connect to server ");
            Serial.print(m_configServer);
            Serial.print(" port ");
            Serial.println(m_configPort);
        }

        // ensures we don't leave a half connection hanging
        client.stop();
        return -1;
    }

    if (m_debug) {
        xDBG_PRINT("Connected to server");
        Serial.print("MenloSmartpux: Connected to server ");
            Serial.print(m_configServer);
        Serial.print(" port ");
        Serial.println(m_configPort);
    }

    if (m_debug) {
        Serial.println("posting document:");
        Serial.println(postBody);
    }

    xDBG_PRINT("posting document");

    // Send the POST
    client.print(postBody);

    if (m_debug) {
        Serial.println("MenloSmartpux: Done posting document body");
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

//
// TODO: Use String.reserve() to pre-allocate memory
// where String() can't be avoided.
//
// TODO: Make this stream based and stream out the TCP port
// so that large dynamic memory buffers are not required.
//
//   - Though this would require the two pass steam model in which
//     the first pass gets the document size, and the second
//     posts it. This is because content length is needed before
//     the body content, unless other HTTP options are used so
//     that content length can be skipped.
//
// This means changing the order of MenloCloudScheduler/MenloCloudFormatter
// to generate the marshalled stream after the connection is initiated
// to the remote server.
//
// MenloCloudScheduler::Post
//   MenloSmartpuxCloud->Connect();
//   MenloCloudFormatter->Format();
//
//     - This would be where the actual application level event would occur
//       so it can generate the data in real time for streaming out to the
//       cloud.
//
//   MenloSmartpuxCloud->Done();
//

String
MenloSmartpuxCloud::generatePostBody()
{
    String values = getDataBuffer();

    //
    // Outline from MenloFramework MenloSmartpux, httpstate.cpp
    //

    String body = "";

    //body += "POST /smartpuxdata/data HTTP/1.1\n";
    //body += "Host: data.smartpux.com\n";

    body += "POST ";
    body += m_configUrl;
    body += " HTTP/1.1\n";

    body += "Host: ";
    body += m_configServer;
    body += "\n";

    body += "Connection: close\n";
    body += "Content-Type: application/x-www-form-urlencoded\n";
    body += "Content-Length: " + String(values.length()) + "\n";

    // Terminate headers, begin content section
    body += "\n";

    // write the content
    body += values;

    // Terminate the content with two \n's
    body += "\n\n";

    return body;
}

char*
MenloSmartpuxCloud::getResponse()
{
    return m_response;
}

int
MenloSmartpuxCloud::getResponseLength()
{
    return m_responseLength;
}

int
MenloSmartpuxCloud::getResponseBufferSize()
{
    return sizeof(m_response);
}

unsigned long
MenloSmartpuxCloud::DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    MenloDweetEventArgs* dweetArgs = (MenloDweetEventArgs*)eventArgs;

    // Check memory
    MenloMemoryMonitor::CheckMemory(LineNumberBaseDweetWiFi + __LINE__);

    xDBG_PRINT("MenloSmartpuxCloud DweetEvent");

    if (ProcessDweetCommands(dweetArgs->dweet, dweetArgs->name, dweetArgs->value) == 0) {
        // Not handled
        xDBG_PRINT("MenloSmartpuxCloud DweetEvent NOT HANDLED");
        return MAX_POLL_TIME;
    }
    else {
        // handled
        xDBG_PRINT("MenloSmartpuxCloud DweetEvent WAS HANDLED");
        return 0;
    }
}

//
// Dweet commands processing.
//
// These commands allow Dweet access to channel, operating modes,
// power level, internal registers, etc.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
MenloSmartpuxCloud::ProcessDweetCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;
    char workingBuffer[CLOUD_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(spx_string_table) / sizeof(char*);

    xDBG_PRINT("MenloSmartpuxCloud calling table processor");

    //
    // GETSTATE/SETSTATE, GETCONFIG/SETCONFIG are handled
    // by a common table driven function.
    //

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetWiFi.
    //

    parms.ModuleName = (PGM_P)spx_module_name_string;
    parms.stringTable = (PGM_P)spx_string_table;
    parms.functionTable = (PGM_P)spx_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = spx_index_table;
    parms.sizeTable = spx_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;

    // SPX_CHECKSUM is defined in MenloConfigStore.h
    parms.checksumIndex = CLOUD_CHECKSUM;
    parms.checksumBlockStart = CLOUD_CHECKSUM_BEGIN;
    parms.checksumBlockSize = CLOUD_CHECKSUM_END - CLOUD_CHECKSUM_BEGIN;

    parms.name = name;
    parms.value = value;

    // DweetState.cpp
    return dweet->ProcessStateCommandsTable(&parms);
}
