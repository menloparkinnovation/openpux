
#ifndef MenloSmartpuxCloud_h
#define MenloSmartpuxCloud_h

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

#include "MenloPlatform.h"
#include "MenloMemoryMonitor.h"
#include "MenloDebug.h"
#include "MenloTimer.h"
#include "MenloCloudScheduler.h"
#include "MenloCloudFormatter.h"
#include "MenloNMEA0183.h"
#include "MenloConfigStore.h"
#include "MenloDweet.h"
#include "MenloWiFi.h"

#if ESP8266
#include <ESP8266WiFi.h>
#else
// Particle library routines
#include "application.h"
#endif

struct MenloSmartpuxCloudConfig {
    MenloWiFi* wifi;
    unsigned long defaultUpdateRate;
    unsigned long defaultTimeout;
    char* defaultHost;
    char* defaultUrl;
    int defaultPort;
    char* defaultToken;
    char* defaultAccountId;
    char* defaultSensorId;

    MenloSmartpuxCloudConfig() {
        wifi = NULL;
        defaultUpdateRate = 0L;
        defaultTimeout = 30;
        defaultHost = NULL;
        defaultUrl = NULL;
        defaultPort = NULL;
        defaultToken = NULL;
        defaultAccountId = NULL;
        defaultSensorId = NULL;
    }
};

class MenloSmartpuxCloud : public MenloCloudFormatter {

 public:

    MenloSmartpuxCloud();

    ~MenloSmartpuxCloud();

    int Initialize(
        MenloSmartpuxCloudConfig *config
    );

    //
    // True if there is the potential to connect to the internet.
    //
    virtual bool IsConnected();

    //
    // Since cloud updates are scheduled the data is updated
    // by the application asynchronously.
    //

    //
    // Make it easy by using common patterns. Unlike the Arduino
    // version we don't have to stream, but can buffer.
    //

    virtual int Post(unsigned long timeout);
    virtual void StartPreAmble();

    char* getResponse();

    int getResponseLength();

    int getResponseBufferSize();

    void SetDebug(bool value);

    //
    // Dweet configuration routines.
    //
    int SpxServer(char* buf, int size, bool isSet);
    int SpxUrl(char* buf, int size, bool isSet);
    int SpxPort(char* buf, int size, bool isSet);
    int SpxToken(char* buf, int size, bool isSet);
    int SpxAccount(char* buf, int size, bool isSet);
    int SpxSensor(char* buf, int size, bool isSet);
    int SpxOptions(char* buf, int size, bool isSet);

 private:

    //
    // Handles Dweets.
    //
    // Returns 1 if the command is recognized.
    // Returns 0 if not.
    //
    int ProcessDweetCommands(MenloDweet* dweet, char* name, char* value);

    String generatePostBody();

    MenloSmartpuxCloudConfig m_config;

    bool m_debug;

    int m_responseLength;

    //
    // Configuration from EEPROM storage
    //
    uint16_t m_configPort;
    char m_configServer[CLOUD_SERVER_SIZE];
    char m_configUrl[CLOUD_URL_SIZE];
    char m_configToken[CLOUD_TOKEN_SIZE];
    char m_configAccountId[CLOUD_ACCOUNT_SIZE];
    char m_configSensorId[CLOUD_SENSOR_SIZE];

    // Response buffer
    char m_response[512];

    //
    // MenloSmartpuxCloud is a client of MenloDweet for unhandled Dweet events
    // to support WiFi configuration Dweet messages.
    //
    // These messages may arrive on any transport.
    //

    // Event registration
    MenloDweetEventRegistration m_dweetEvent;

    // DweetEvent function
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloSmartpuxCloud_h
