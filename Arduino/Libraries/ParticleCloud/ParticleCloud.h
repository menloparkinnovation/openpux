
#ifndef ParticleCloud_h
#define ParticleCloud_h

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

#include "MenloPlatform.h"
#include "MenloTimer.h"
#include "MenloCloudScheduler.h"
#include "MenloCloudFormatter.h"

// Particle library routines
#include "application.h"

struct ParticleCloudConfig {
    unsigned long updateRate;
    unsigned long timeout;
    String host;

    String token;
    String accountId;
    String sensorId;
};

class ParticleCloud : public MenloCloudFormatter {

 public:

    ParticleCloud();

    ~ParticleCloud();

    int Initialize(
        ParticleCloudConfig *config
    );

    //
    // True if there is the potential to connect to the internet.
    //
    virtual bool IsConnected();

    virtual int Post(unsigned long timeout);
    virtual void StartPreAmble();

    char* getResponse();

    int getResponseLength();

    int getResponseBufferSize();

    void SetDebug(bool value);

 private:

    ParticleCloudConfig m_config;

    bool m_debug;

    String m_host;
    String m_token;
    String m_accountId;
    String m_sensorId;

    String m_buffer;

    int m_responseLength;

    // Response buffer
    char m_response[512];
};

#endif // ParticleCloud_h
