
/*
 * Copyright (C) 2016 Menlo Park Innovation LLC
 *
 * This is licensed software, all rights as to the software
 * is reserved by Menlo Park Innovation LLC.
 *
 * A license included with the distribution provides certain limited
 * rights to a given distribution of the work.
 *
 * This distribution includes a copy of the license agreement and must be
 * provided along with any further distribution or copy thereof.
 *
 * If this license is missing, or you wish to license under different
 * terms please contact:
 *
 * menloparkinnovation.com
 * menloparkinnovation@gmail.com
 */

/*
 *  Date: 07/06/2014 - 06/01/2016 rewrote for new WiFi model.
 *  File: MenloWiFiArduino.h
 */

#ifndef MenloWiFiArduino_h
#define MenloWiFiArduino_h

#include "MenloPlatform.h"

#if defined(ESP8266)
// ESP8266 has Arduino WiFi interface, but different header for definitions.
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <MenloUtility.h>

// Implements MenloWiFi
#include <MenloWiFi.h>

//
// Handler for WiFI on Arduino WiFi Shield (stock, or 100% compatible clones)
//
// This is specific to the Arduino.cc official WiFi Shield. Designed
// to be readily ported to other WiFi solutions, but keep the main
// orchestration outside the the main "sketch".
//
//

//
// Number of times to retry an access point entry which returns
// WL_IDLE_STATUS
//
#define MAX_IDLE_RETRIES 4

class MenloWiFiArduino : public MenloWiFi {

public:
  
    MenloWiFiArduino();

    //
    // Initialization occurs outside of the constructor
    // to allow flexibility when instances are used.
    //
    // Returns 0 on success.
    //
    int Initialize(WiFiClient* client);

    WiFiClient* getWiFi() {
        return m_wifi;
    }

    //
    // These are contracts that must be implemented by a WiFi Provider.
    //

    //
    // These are direct set/get routines
    //
    virtual int WiFiSSID(char* buf, int size, bool isSet);
    virtual int WiFiPassword(char* buf, int size, bool isSet);
    virtual int WiFiChannel(char* buf, int size, bool isSet);
    virtual int WiFiPower(char* buf, int size, bool isSet);
    virtual int WiFiAttention(char* buf, int size, bool isSet);
    virtual int WiFiOptions(char* buf, int size, bool isSet);

    //
    // Returns 0 if hardware is present, error code if not
    //
    virtual bool IsHardwarePresent();

    virtual void Stop();

    //
    // Print network diagnostics to the debugger serial
    //
    virtual void PrintMacAddress();

#if 0
    virtual void PrintDiagnostics(Stream* stream);
#endif

    virtual bool IsConnected();

    virtual bool IsSignedOn();

    virtual int DataBytesAvailable();

    virtual int Read();

    virtual int Read(uint8_t* buf, size_t size);  

    virtual size_t Write(uint8_t c);

    virtual size_t Write(const uint8_t* buf, size_t size);

    virtual void Flush();

protected:

    //
    // Connection State Machine
    //
    virtual void OnConnect();

    virtual void OnDisconnect();

    //
    // These are indications to the radio implementation subclass
    // to transition to a particular power state.
    //
    virtual void OnPowerOn();

    virtual void OnPowerOff();

private:

  WiFiClient *m_wifi;

  bool m_signedOn;
};

#endif // MenloWiFiArduino_h
