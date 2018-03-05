
//
//   Openpux: The Operating System for IoT
//
//   ParticleCloud.h
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
 *  File: ParticleCloud
 */

#include "ParticleCloud.h"

ParticleCloud::ParticleCloud()
{
#if XDBG_PRINT_ENABLED
    m_debug = true;
#else
    m_debug = false;
#endif
}

ParticleCloud::~ParticleCloud()
{
}

//
// This is exposed to allow runtime setting, such as through
// a Particle.function() call.
//
void
ParticleCloud::SetDebug(bool value)
{
    m_debug = value;
}

int
ParticleCloud::Initialize(
        ParticleCloudConfig *config
        )
{
    m_config = *config;

    // MenloParticleWeather.cpp sets up the config in ApplicationSetup()
    m_host = m_config.host;
    m_token = m_config.token;
    m_accountId = m_config.accountId;
    m_sensorId = m_config.sensorId;

    // Reset to start initial parameters
    Reset();

    // Call base class initialization
    MenloCloudScheduler::Initialize(m_config.updateRate);

    return 0;
}

bool
ParticleCloud::IsConnected()
{
    if (m_host == NULL) {
        return false;
    }

    // TODO: Query hardware WiFi connection status.
    return true;
}

void
ParticleCloud::StartPreAmble()
{
    //
    // Start the pre-amble
    //

    //
    // All new buffers start with "&" which is removed by
    // MenloCloudFormatter::getDataBuffer()
    //
    appendBuffer("&");

    //
    // Note abbreviated codes are accepted for really small
    // devices or SMS/text message transports, but here
    // we do not need to. The Smartpux server accepts
    // both forms.
    //
    if (IsShortForm()) {

        if (m_accountId != NULL) {
            appendBuffer("A=" + m_accountId + "&");
        }

        if (m_token != NULL) {
            appendBuffer("P=" + m_token + "&");
        }

        if (m_sensorId != NULL) {
            appendBuffer("S=" + m_sensorId);
        }
    }
    else {

        if (m_accountId != NULL) {
            appendBuffer("AccountID=" + m_accountId + "&");
        }

        if (m_token != NULL) {
            appendBuffer("PassCode=" + m_token + "&");
        }

        if (m_sensorId != NULL) {
            appendBuffer("SensorID=" + m_sensorId);
        }
    }
}

//
// timeout is in milliseconds()
//
int
ParticleCloud::Post(
    unsigned long timeout
    )
{
    int retVal = 0;
    String buffer = getDataBuffer();

    if (buffer == "") {
        return retVal;
    }

    // Publish a particle event
    Particle.publish("readings", buffer, 60, PUBLIC);

    // Reset the send buffer for a new set of values
    Reset();

    return retVal;
}

char*
ParticleCloud::getResponse()
{
    return m_response;
}

int
ParticleCloud::getResponseLength()
{
    return m_responseLength;
}

int
ParticleCloud::getResponseBufferSize()
{
    return sizeof(m_response);
}

