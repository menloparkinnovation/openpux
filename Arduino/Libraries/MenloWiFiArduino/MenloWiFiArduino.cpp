
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
 *  Date: 06/01/2016 - rewrote for new WiFi model.
 *  File: MenloWiFiArduino.cpp
 */

//
// Menlo WiFi Handler
//
// This handles the official Arduino WiFi shield.
//
// 11/13/2012
//
// MenloParkInnovation - JR
//
// Last Update:
//
// 11/30/2013
//
// Cleaned up error handling.
// Moved debug strings to program memory.
//
// 08/26/2013
//
// 08/25/2013
//  - Sign on to open networks with no password
//  - Implement debug stream support
//
// 06/01/2016
//
//  - Rewrote MenloWiFi.h to operate similar to MenloRadio.h with
//    radio power timers, etc.
//
//  - Add support for DweetWiFi to support configuration from EEPROM,
//    updates over dweet/serial connections, etc.
//
//  - AtMega328's are no longer a primary target for WiFi, but
//    ARM and ESP8266 processors are.
//

#include "MenloPlatform.h"
#include "MenloMemoryMonitor.h"
#include "MenloDebug.h"
#include "MenloUtility.h"
#include "MenloNMEA0183.h"
#include "MenloConfigStore.h"
#include "MenloDweet.h"
#include "DweetWiFi.h"

#include <MenloWiFiArduino.h>

//
// A lot of times radio status is required.
// This allows quick enabling of radio status.
//
#define RDBG_PRINT_ENABLED 1

#if RDBG_PRINT_ENABLED
#define RDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#else
#define RDBG_PRINT(x)
#endif

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
#define xDBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define xDBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_HEX_STRING(x, l)
#define xDBG_PRINT_HEX_STRING_NNL(x, l)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

  //
  // "WiFi" is a global exported by the WiFi driver class
  //
  // arduino-1.0.1\libraries\WiFi\WiFi.h
  //  extern WiFiClass WiFi;
  //
  // arduino-1.0.1\libraries\WiFi\WiFi.cpp
  //  WiFiClass WiFi;
  //
  // WL_* status codes are in:
  // arduino-1.0.1\libraries\WiFi\utility\wl_definitions.h, wl_status_t
  //
  // typedef enum {
  //	WL_NO_SHIELD = 255,     // 255
  //      WL_IDLE_STATUS = 0,   // 0
  //      WL_NO_SSID_AVAIL,     // 1
  //      WL_SCAN_COMPLETED,    // 2
  //      WL_CONNECTED,         // 3
  //      WL_CONNECT_FAILED,    // 4
  //      WL_CONNECTION_LOST,   // 5
  //      WL_DISCONNECTED       // 6
  //} wl_status_t;
  //
  // WiFi.getSocket();
  // WiFi.firmwareVersion();
  // WiFi.begin(char* ssid);
  // WiFi begin(char* ssid, const char *passphrase);
  // WiFi begin(char* ssid, uint8_t key_idx, const char* key);
  // WiFi.disconnect();
  // WiFi.macAddress();
  // WiFi.localIP();
  // WiFi.subnetMask();
  // WiFi.gatewayIP();
  // WiFi.SSID();
  // WiFi.BSSID();
  // WiFi.RSSI();
  // WiFi.encryptionType();
  // WiFi.scanNetworks();
  // WiFi.SSID(uint8_t networkItem);
  // WiFi.encryptionType(uint8_t networkItem);
  // WiFi.RSSI(uint8_t networkItem);
  // WiFi.status(); // returns entries in wl_status_t
  // WiFi.hostByName(const char* aHostname, IPAddress& aResult);
  // friend class WiFiClient;
  // friend class WiFiServer;
  //

MenloWiFiArduino::MenloWiFiArduino() {
    m_wifi = NULL;
    m_signedOn = false;
}

int
MenloWiFiArduino::Initialize(
    WiFiClient* wifi
    )
{
    m_wifi = wifi;

    // Initialize base class
    MenloWiFi::Initialize();

    return 0;

    //
    // The static "WiFiClass WiFi" done in the Arduino WiFi support
    // already statically initializes the SPI driver for the WiFi
    // hardware upon C++ static constructors initialization.
    //
    // There is currently in the default class to re-initialize
    // the SPI and/or the WiFi hardware if there is a problem
    // with the SPI bus.
    //
    // In our case we would have to allow the watchdog reset
    // perform the re-initialization of the whole gateway
    // on WiFi drop out/communications problems.
    //
    // WiFi\utility\wifi_drv.cpp
    // WiFiDrv::wifiDriverInit() {
    //   SpiDrv::begin();
    // }
    //
    // WiFi\utility\spi_drv.cpp
    // void SpiDrv::begin(){
    // }
    //
}

//
// Power support
//

//
// Request to power on the radio
//
void
MenloWiFiArduino::OnPowerOn()
{
    //
    // A request to power on is to power on the receiver which
    // is the radios default mode of operation when powered on.
    //
    RDBG_PRINT("ArduinoWiFi radio powered up");    
    // TODO: PowerUp();
}

//
// Request to power off the radio
//
void
MenloWiFiArduino::OnPowerOff()
{
    RDBG_PRINT("ArduinoWiFi radio powered down");    
    // TODO: PowerDown();
}

//
// Properties/control
//

//
// Invoked from DweetWiFi to set the SSID.
//
// SETSTATE=WIFISSID:accesspointname
// SETCONFIG=WIFISSID:accesspointname
//
int
MenloWiFiArduino::WiFiSSID(char* buf, int size, bool isSet)
{
    int length, index;

    if (isSet) {

        DBG_PRINT_NNL("WiFi Set SSID ");
        DBG_PRINT_STRING(buf);

        // MenloConfigStore.h defines the sizes
        length = strlen(buf);
        if (length > (WIFI_SSID_SIZE - 1)) {
            DBG_PRINT("WiFi SetSSID to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        //
        // Note: this writes directly into the ssid setting
        // to avoid duplicating a large stack buffer here.
        //

        for (index = 0; index < length; index++) {
            m_ssid[index] = buf[index];
        }

        // Ensure null terminated string
        m_ssid[index] = '\0';

        xDBG_PRINT_NNL("SSID ");
        xDBG_PRINT_STRING(m_ssid);

        return 0;
    }
    else {

        length = strlen(m_ssid) + 1;
        if (length > size) {
            DBG_PRINT("WiFi GetSSID to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            buf[index] = m_ssid[index];
        }

        // Ensure null terminated string
        buf[index] = '\0';

        return 0;
    }
}

//
// Invoked from DweetWiFi to set the password.
//
// SETSTATE=WIFIPASSWORD:password
// SETCONFIG=WIFIPASSWORD:password
//
int
MenloWiFiArduino::WiFiPassword(char* buf, int size, bool isSet)
{
    int length, index;

    if (isSet) {

        DBG_PRINT_NNL("WiFi Set Password ");
        DBG_PRINT_STRING(buf);

        // MenloConfigStore.h defines the sizes
        length = strlen(buf);
        if (length > (WIFI_PASSWORD_SIZE - 1)) {
            DBG_PRINT("WiFi SetPassword to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        //
        // Note: this writes directly into the password setting
        // to avoid duplicating a large stack buffer here.
        //

        for (index = 0; index < length; index++) {
            m_password[index] = buf[index];
        }

        // Ensure null terminated string
        m_password[index] = '\0';

        xDBG_PRINT_NNL("Password ");
        xDBG_PRINT_STRING(m_password);

        return 0;
    }
    else {

        length = strlen(m_password) + 1;
        if (length > size) {
            DBG_PRINT("WiFi GetPassword to long");
            return DWEET_PARAMETER_TO_LONG;
        }

        for (index = 0; index < length; index++) {
            buf[index] = m_password[index];
        }

        // Ensure null terminated string
        buf[index] = '\0';

        return 0;
    }
}

//
// Invoked from DweetWiFi to set the channel.
//
// SETSTATE=WIFICHANNEL:00
// SETCONFIG=WIFICHANNEL:00
//
int
MenloWiFiArduino::WiFiChannel(char* buf, int size, bool isSet)
{
    if (isSet) {
        // If this fails, then EEPROM settings will fail.
        return 0;
    }
    else {

        if (size < 3) {
            return DWEET_PARAMETER_TO_LONG;
        }

        buf[0] = '0';
        buf[1] = '0';
        buf[2] = '\0';

        return 0;
    }
}

//
// Invoked from DweetWiFi to set the power state.
//
int
MenloWiFiArduino::WiFiPower(char* buf, int size, bool isSet)
{
    bool error;
    unsigned long powerTime;

    if (isSet) {
        powerTime = MenloUtility::HexToULong(buf, &error);
        if (error) {
            return DWEET_INVALID_PARAMETER;
        }

        SetPowerTimer(powerTime);

        return 0;  
    }
    else {

        if (size < 9) {
            return DWEET_PARAMETER_TO_LONG;
        }

       powerTime = GetPowerTimer();

       MenloUtility::UInt32ToHexBuffer(powerTime, buf);

       return 0;
    }
}

//
// Invoked from DweetWiFi to set attention to keep the radio awake
// for receiving additional messages.
//
int
MenloWiFiArduino::WiFiAttention(char* buf, int size, bool isSet)
{
    bool error;
    unsigned long powerTime;

    if (isSet) {
        powerTime = MenloUtility::HexToULong(buf, &error);
        if (error) {
            return DWEET_INVALID_PARAMETER;
        }

        if (powerTime < WIFI_MINIMUM_POWER_TIMER) {
            powerTime = WIFI_MINIMUM_POWER_TIMER;
        }

        // Attention is to our peer address
        SetSendAttention(powerTime, (uint8_t*)&m_ssid[0]);

        return 0;  
    }
    else {
        // Not implemented
        return DWEET_ERROR_UNSUP;
    }
}

//
// Invoked from DweetWiFi to set WiFi radio specific options.
//
int
MenloWiFiArduino::WiFiOptions(char* buf, int size, bool isSet)
{
    if (isSet) {
        return DWEET_ERROR_UNSUP;
    }
    else {
        return DWEET_ERROR_UNSUP;
    }
}

bool
MenloWiFiArduino::IsHardwarePresent()
{
    if (m_wifi->status() == WL_NO_SHIELD) {
        xDBG_PRINT("No WiFi hardware");
        return false;
    }
    else {
        return true;
    }
}

void
MenloWiFiArduino::Stop()
{
    ResetWatchdog();   // Tell watchdog we are still alive
    DBG_PRINT("ArduinoWiFi Stop");
    m_wifi->stop();
    ResetWatchdog();   // Tell watchdog we are still alive
}

bool
MenloWiFiArduino::IsConnected()
{
    if (m_wifi->connected() == 0) {
      return false;
    }
    else {
      DBG_PRINT("ArduinoWiFi IsConnected is true");
      return true;
    }
}

bool
MenloWiFiArduino::IsSignedOn()
{
    return m_signedOn;
}

int
MenloWiFiArduino::DataBytesAvailable()
{
    return m_wifi->available();
}

int
MenloWiFiArduino::Read()
{
    ResetWatchdog();   // Tell watchdog we are still alive
    return m_wifi->read();
}

int
MenloWiFiArduino::Read(uint8_t* buf, size_t size)
{
    ResetWatchdog();   // Tell watchdog we are still alive
    return m_wifi->read(buf, size);
}

size_t
MenloWiFiArduino::Write(uint8_t c)
{
    ResetWatchdog();   // Tell watchdog we are still alive
    return m_wifi->write(c);
}

size_t
MenloWiFiArduino::Write(const uint8_t* buf, size_t size)
{
    ResetWatchdog();   // Tell watchdog we are still alive
    return m_wifi->write(buf, size);
}

void
MenloWiFiArduino::Flush()
{
    ResetWatchdog();   // Tell watchdog we are still alive
    return m_wifi->flush();
}

void
MenloWiFiArduino::PrintMacAddress()
{
    byte mac_address[6];

    // WiFi.macAddress returns 6 bytes
    WiFi.macAddress(mac_address);

    MenloDebug::PrintNoNewline(F("WiFi MacAddress "));
    MenloDebug::PrintHexByteNoNewline(mac_address[5]);

    MenloDebug::PrintNoNewline(F("."));
    MenloDebug::PrintHexByteNoNewline(mac_address[4]);

    MenloDebug::PrintNoNewline(F("."));
    MenloDebug::PrintHexByteNoNewline(mac_address[3]);

    MenloDebug::PrintNoNewline(F("."));
    MenloDebug::PrintHexByteNoNewline(mac_address[2]);

    MenloDebug::PrintNoNewline(F("."));
    MenloDebug::PrintHexByteNoNewline(mac_address[1]);

    MenloDebug::PrintNoNewline(F("."));
    MenloDebug::PrintHexByte(mac_address[0]);
}

#if 0
void
MenloWiFiArduino::PrintDiagnostics(Stream* stream)
{
    IPAddress ip;
    long rssi;

    //
    // WiFi is a global statically created class
    // when the WiFi library is included.
    //

    // uint8_t* WiFi.macAddress(uint8_t* mac);

    ResetWatchdog();   // Tell watchdog we are still alive

    stream->print(F("SSID: "));
    stream->println(WiFi.SSID());

    ip = WiFi.localIP();
    stream->print(F("IP Address: "));
    stream->println(ip);

    ip = WiFi.gatewayIP();
    stream->print(F("Gateway IP Address: "));
    stream->println(ip);

    ResetWatchdog();   // Tell watchdog we are still alive

    ip = WiFi.subnetMask();
    stream->print(F("SubNet Mask: "));
    stream->println(ip);

    rssi = WiFi.RSSI();
    stream->print(F("RSSI: "));
    stream->print(rssi);
    stream->println(F(" dBm"));

    ResetWatchdog();   // Tell watchdog we are still alive
}
#endif

//
// Attempt to connect
//
void
MenloWiFiArduino::OnConnect()
{
    int status;

    RDBG_PRINT("ArduinoWiFi OnConnect");

    if (m_ssid[0] == '\0') {
        SetConnectionTargetState(WiFiConnectionStateNoCredentials);
        return;
    }

    MenloDebug::PrintNoNewline(F("WiFi Attempt conn to "));
    MenloDebug::Print(m_ssid);

    PrintMacAddress();

    ResetWatchdog();

    // A NULL Password is ok
    if (m_password[0] == '\0') {
        MenloDebug::Print(F(" with no password"));
        status = WiFi.begin(m_ssid);
    }
    else {
        MenloDebug::PrintNoNewline(F(" with "));
        MenloDebug::Print(m_password);

        status = WiFi.begin(m_ssid, m_password);
    }

    ResetWatchdog();

    if (status == WL_CONNECTED) {

        MenloDebug::PrintNoNewline(F("WiFi Connected with "));
        MenloDebug::Print(m_ssid);

        // Connected
        SetConnectionTargetState(WiFiConnectionStateConnected);
        return;
    }

    if (status == WL_IDLE_STATUS) {

        xDBG_PRINT("WL_IDLE_STATUS");

        //
        // This gets signaled while the board is syncing with the
        // network. We need to attempt a retry again it before
        // moving onto any new SSID or password, otherwise it will
        // the sign on sequence.
        //

        // stay in the WiFiStatusConnecting state

        // fallthrough;
    }
    else {
        MenloDebug::PrintNoNewline(F("WiFi Connect Error Status  "));
        MenloDebug::PrintHex(status);

        // stay in the WiFiStatusConnecting state

        PrintMacAddress();

        // fallthrough;
    }

    //
    // All Arduino example code for WiFi has a 10 second delay
    // after each connection attempt.
    //
    // Note: This should be done as part of the state machine and
    // timer.
    //
    SetConnectingWait(10 * 1000);
}

//
// Disconnect
//
void
MenloWiFiArduino::OnDisconnect()
{
    RDBG_PRINT("ArduinoWiFi OnDisconnect");

    WiFi.disconnect();

    SetConnectionTargetState(WiFiConnectionStateDisconnected);
}
