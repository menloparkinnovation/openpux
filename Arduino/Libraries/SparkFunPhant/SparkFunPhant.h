
/**
 * Phant.h
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

#ifndef Phant_h
#define Phant_h

// MenloFramework
#include "MenloPlatform.h"
#include "MenloTimer.h"
#include "MenloCloudScheduler.h"
#include "MenloCloudFormatter.h"
#include "MenloConfigStore.h"
#include "MenloWiFi.h"

struct PhantConfig {
    MenloWiFi* wifi;
    unsigned long updateRate;
    unsigned long timeout;
    String host;
    int port;
    String publicKey;
    String privateKey;

    PhantConfig() {
        wifi = NULL;
        updateRate = 0L;
        timeout = 30;
        port = 80;
        // String() values automatically initialize to NULL/empty string.
        // host = NULL;
        // publicKey = NULL;
        // privateKey = NULL;
    }
};

#if ESP8266
#include <ESP8266WiFi.h>
#else
// Particle library routines
#include "application.h"
#endif

class Phant : public MenloCloudFormatter {

  public:

    Phant();

    int Initialize(PhantConfig* config);

    //
    // True if there is the potential to connect to the internet.
    //
    virtual bool IsConnected();

    virtual int Post(unsigned long timeout);
    virtual void StartPreAmble();

    String url();
    String get();
    String getPostBody();
    String ResetHttpConnection();
	
    int postToPhant(unsigned long timeout);

    void SetDebug(bool value);

  private:

    PhantConfig m_config;

    String _pub;
    String _prv;
    String _host;
    int    _port;
    bool   _debug;

    int _responseLength;

    // Response buffer
    char _response[512];
};

#endif
