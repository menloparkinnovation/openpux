
//
//   Openpux: The Operating System for IoT
//
//   MenloSmartPuxPhoton.h
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2015,2016 Menlo Park Innovation LLC
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

#ifndef MenloSmartpuxPhoton_h
#define MenloSmartpuxPhoton_h

// Particle library routines
#include "application.h"

class MenloSmartpuxPhoton {

 public:

    MenloSmartpuxPhoton();

    ~MenloSmartpuxPhoton();

    int Initialize(
        String host,
        int port,
        String token,
        String accountId,
        String sensorId
    );

    //
    // Make it easy by using common patterns. Unlike the Arduino
    // version we don't have to stream, but can buffer.
    //
    void add(String name, int value);
    void add(String name, long value);
    void add(String name, unsigned long value);
    void add(String name, String value);
    void add(String name, float value, unsigned int precision = 4);
    void add(String name, double value, unsigned int precision = 4);

    int Post(unsigned long timeout);

    char* getResponse();

    int getResponseLength();

    int getResponseBufferSize();

    // Reset the buffer
    void Reset();

    void SetDebug(bool value);

 private:

    String generatePostBody();

    bool m_debug;

    int m_port;
    String m_host;
    String m_token;
    String m_accountId;
    String m_sensorId;

    String m_buffer;

    int m_responseLength;

    // Response buffer
    char m_response[512];
};

#endif // MenloSmartpuxPhoton_h
