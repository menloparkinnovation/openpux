
/*
 * Copyright (C) 2015 Menlo Park Innovation LLC
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
 *  Date: 05/31/2016 rewrote.
 *
 *  This class provides a uniform interface to WiFi support.
 */

#ifndef MenloWiFi_h
#define MenloWiFi_h

#include "MenloPlatform.h"
#include "MenloObject.h"
#include "MenloDispatchObject.h"
#include "MenloTimer.h"

#include <MenloUtility.h>

//
// This class provides uniform WiFi interfaces.
//

//
// Connection State
//
const uint8_t WiFiConnectionStateInit           = 0x0;
const uint8_t WiFiConnectionStateNoHardware     = 0x1;
const uint8_t WiFiConnectionStateIdle           = 0x2;
const uint8_t WiFiConnectionStateConnecting     = 0x3;
const uint8_t WiFiConnectionStateConnected      = 0x4;
const uint8_t WiFiConnectionStateDisconnecting  = 0x5;
const uint8_t WiFiConnectionStateDisconnected   = 0x6;
const uint8_t WiFiConnectionStatePoweredDown    = 0x7;
const uint8_t WiFiConnectionStateNoCredentials  = 0x8;
const uint8_t WiFiConnectionStateConnectingWait = 0x9;

// Maximum access point connection retries when WL_IDLE_STATUS is returnd.
#define WIFI_MAX_IDLE_RETRIES 2

// Note: If passphrase[0] == '\0' there is no password
struct WiFiNetworkConfig {
  char ssid[32];
  char passphrase[32];
};

//
// MenloWiFi can raise events for certain conditions such as
// an access point available.
//
class MenloWiFiEventArgs : public MenloEventArgs {
public:
    uint8_t  eventType;
    uint8_t  ConnectionState;
    uint8_t  eventDataLength;
    uint8_t* eventData;
};

//
// Events
//

const uint8_t WiFiEventNone         = 0x0;
const uint8_t WiFiEventConnected    = 0x01;
const uint8_t WiFiEventDisconnected = 0x02;

// Introduce the proper type name. Could be used for additional parameters.
class MenloWiFiEventRegistration : public MenloEventRegistration {
 public:
};

//
// Default WiFi Radio Power interval
//
#define WIFI_DEFAULT_POWER_INTERVAL (0L) // 0 means always on

// 250 ms for attention.
#define WIFI_MINIMUM_POWER_TIMER (250)

class MenloWiFi : public MenloDispatchObject {

public:
  
    //
    // The following methods are base library methods
    // implemented by MenloWiFi.cpp
    //

    MenloWiFi();

    virtual int Initialize();

    //
    // Most callers just want to know if connections are possible.
    //
    bool isConnected() {
        if (m_connectionState == WiFiConnectionStateConnected) {
            return true;
        }
        else {
            return false;
        }
    }

    uint8_t GetConnectionState() {
        return m_connectionState;
    }

    //
    // Start the WiFi sign on process.
    //
    // An event will be raised if it achieves connection status.
    //
    // Progress may be queried with GetConnectionState().
    //
    
    bool StartSignOn();

    bool StartSignOff();

    //
    // The following methods have a base implementation in
    // MenloWiFi.
    //
    virtual unsigned long Poll();

    void RegisterWiFiEvent(MenloWiFiEventRegistration* callback);

    void UnregisterWiFiEvent(MenloWiFiEventRegistration* callback);

    //
    // The following are virtual methods implemented by a specific
    // WiFi host implementation.
    //

    //
    // These are direct set/get routines
    //
    virtual int WiFiSSID(char* buf, int size, bool isSet) = 0;
    virtual int WiFiPassword(char* buf, int size, bool isSet) = 0;
    virtual int WiFiChannel(char* buf, int size, bool isSet) = 0;
    virtual int WiFiPower(char* buf, int size, bool isSet) = 0;
    virtual int WiFiAttention(char* buf, int size, bool isSet) = 0;
    virtual int WiFiOptions(char* buf, int size, bool isSet) = 0;

    //
    // WiFi Radio Power Control
    //

    //
    // This allows the radio activity indicator to be set
    // by the subclass or application so that the power timer
    // does not power down the radio.
    //
    void SetActivity();

    //
    // This will send an attention packet with the following
    // interval immediately after next packet reception.
    //
    // This is to keep a mostly sleeping sensor awake for
    // further commands during its small receive window
    // after transmit before it goes to sleep again and shuts
    // off its radio to save power.
    //
    void SetSendAttention(unsigned long interval, uint8_t* address);

    //
    // Request that the radio be powered on.
    //
    void PowerOn();

    //
    // Set a timer to power off the radio after a
    // period of inactivity.
    //
    // The inactivityTimer is the amount of time the radio
    // should remain powered on when there is no more
    // activity.
    //
    // A value of 0 means the radio is always powered
    // on. Otherwise its the time in milliseconds before
    // the radio will power down due to inactivity.
    //
    // When the radio is powered off ReceiveDataReady()
    // will return a no data indication since the receiver
    // if off.
    //
    void SetPowerTimer(unsigned long inactivityTimer);

    //
    // Return how long the radio will remain powered on
    //
    // 0 means don't power down
    //
    unsigned long
    GetPowerTimer() {
        return m_powerInterval;
    }

    //
    // Operations API's
    //

    //
    // Returns 0 if hardware is present, error code if not
    //
    virtual bool IsHardwarePresent() = 0;

    virtual void Stop() = 0;

    virtual bool IsConnected() = 0;

    virtual bool IsSignedOn() = 0;

    virtual int DataBytesAvailable() = 0;

    virtual int Read() = 0;

    virtual int Read(uint8_t* buf, size_t size) = 0;

    virtual size_t Write(uint8_t c) = 0;

    virtual size_t Write(const uint8_t* buf, size_t size) = 0;

    virtual void Flush() = 0;

    //
    // Print network diagnostics to the debugger serial
    //
#if 0
    virtual void PrintDiagnostics(Stream* stream) = 0;
#endif

    bool SetCredentials(char* ssid, char* password);

    char* getSSID() {
        return &m_ssid[0];
    }

    char* getPassword() {
        return &m_password[0];
    }

protected:

    //
    // Set the target state for the WiFi connection.
    //
    bool SetConnectionTargetState(uint8_t newState);

    void SetConnectingWait(unsigned long interval);

    //
    // Connection State Machine
    //
    virtual void OnConnect() = 0;

    virtual void OnDisconnect() = 0;

    //
    // These are indications to the radio implementation subclass
    // to transition to a particular power state.
    //
    virtual void OnPowerOn() = 0;

    virtual void OnPowerOff() = 0;

    // MenloConfigStore.h defines the sizes
    char m_ssid[WIFI_SSID_SIZE];
    char m_password[WIFI_PASSWORD_SIZE];

private:

    void ProcessConnectionStateMachine();

    void ProcessAttentionReceive(uint8_t* buf);

    void ProcessAttentionSend();

    uint8_t m_connectionState;

    uint8_t m_wifiEvent;

    bool m_wifiEventSignaled;

    bool m_radioPowerState;

    bool m_radioActivity;

    //
    // sendRadioAttention is set to immediately reply to a remote
    // radio that is mostly sleeping.
    //
    bool m_sendRadioAttention;
    
    //
    // This is set when radio attention is active on the current radio
    // and delays its sleeping for m_attentionInterval.
    //
    bool m_radioAttentionActive;

    //
    // Interval used by the WiFiConnectStateConnectingWait state.
    //
    unsigned long m_connectingWaitInterval;

    // This is the interval that the current radio remains power up receiving.
    unsigned long m_attentionInterval;

    //
    // This is the interval that is sent to a radio if m_sendRadioAttention
    // is true.
    //
    unsigned long m_sendAttentionInterval;

    // Interval that the radio remains powered on when there is no activity
    unsigned long m_powerInterval;

    //
    // This is the address the send attention goes to
    //
    uint8_t* m_sendAttentionAddress;

    //
    // MenloWiFi is an Event generator for WiFi signon, sign off
    // and other events.
    //
    MenloEvent m_eventList;

    //
    // MenloWiFi utilizes timer events for power scheduling.
    //    
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloWiFi_h
