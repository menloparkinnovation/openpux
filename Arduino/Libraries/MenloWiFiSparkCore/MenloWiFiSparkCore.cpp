
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
 *  Date: 07/08/2014
 *  File: MenloWiFiSparkCore.cpp
 */

//
// Menlo WiFi Handler for SparkCore
//

#include "MenloPlatform.h"
#include "MenloMemoryMonitor.h"
#include "MenloDebug.h"
#include "MenloUtility.h"
#include "MenloNMEA0183.h"
#include "MenloConfigStore.h"
#include "MenloDweet.h"
#include "DweetWiFi.h"

#include <MenloWiFiSparkCore.h>

#define DBG_PRINT_ENABLED 1

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (Serial.print(F(x)))
#define DBG_PRINT_INT(x) (Serial.print(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_INT(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 1

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)     (Serial.println(F(x)))
#define xDBG_PRINT_INT(x) (Serial.println(x))
#define xDBG_PRINT2(x, y) (Serial.print(F(x)) && Serial.println(y))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT2(x, y)
#endif

//
// SparkCore WiFi is a built in TI CC3000 and automatically configured
// by default.
//
// It has its own spark_wiring_wifi.h that has minimal function and
// defines "WiFiClass" and an instance "WiFi".
//
// The real interface to the WiFi is in
// core-firmware\src\spark_wiring_wifi.cpp
//
// spark_wiring_wifi.cpp calls into SPARK_WLAN_* which is implemented by:
//
//   core-firmware\inc\spark_wlan.h
//   core-firmware\src\spark_wlan.cpp
//
// Basic TCP access is through the Arduino compatible spark_wiring_client.h
// and class "Client".
//
// Class "Client" which is Arduino compatible is implemented by
//   core-firmware\inc\spark_wiring_tcpclient.h
//   core-firmware\src\spark_wiring_tcpclient.cpp
//
//   spark_wiring_tcpclient is a wrapper around the basic socket
//   interfaces supplied by either the ARM libraries or the TI
//   CC3000 WiFi chip. Offers another example of layering "Arduino Wiring"
//   on top of a sockets interface.
//
//

// Constructor
MenloWiFiSparkCore::MenloWiFiSparkCore() {
}

int
MenloWiFiSparkCore::InitializeClient(
    Client* client,
    int retryCount
    )
{
  m_client = client;

  //
  // Default for SparkCore is signed on since WiFi is typically
  // autoconfig on TI CC3000.
  //
  m_signedOn = true;

  return 0;
}

bool
MenloWiFiSparkCore::IsHardwarePresent()
{
  // SparkCore has WiFi built in
  return true;
}

//
// Attempt to connect to network with up to retryCount times.
//
int
MenloWiFiSparkCore::SignOn(char* ssid, char* passPhrase, int retryCount)
{
    if (m_signedOn) {
        return 0;
    }

    //
    // TODO: Add support for networks lists, override SparkCore
    // CC3000 auto config.
    //

    return (-1);
}

void
MenloWiFiSparkCore::Stop()
{
  ResetWatchdog();   // Tell watchdog we are still alive
  DBG_PRINT("MenloWiFi::Stop()");
  m_client->stop();
  ResetWatchdog();   // Tell watchdog we are still alive
}

bool
MenloWiFiSparkCore::IsConnected()
{
  if (m_client->connected() == 0) {
    return false;
  }
  else {
    DBG_PRINT("MenloWiFi::IsConnected() == true");
    return true;
  }
}

bool
MenloWiFiSparkCore::IsSignedOn()
{
  return m_signedOn;
}

int
MenloWiFiSparkCore::DataBytesAvailable()
{
  return m_client->available();
}

int
MenloWiFiSparkCore::Read()
{
  ResetWatchdog();   // Tell watchdog we are still alive
  return m_client->read();
}

int
MenloWiFiSparkCore::Read(uint8_t* buf, size_t size)
{
  ResetWatchdog();   // Tell watchdog we are still alive
  return m_client->read(buf, size);
}

size_t
MenloWiFiSparkCore::Write(uint8_t c)
{
  ResetWatchdog();   // Tell watchdog we are still alive
  return m_client->write(c);
}

size_t
MenloWiFiSparkCore::Write(const uint8_t* buf, size_t size)
{
  ResetWatchdog();   // Tell watchdog we are still alive
  return m_client->write(buf, size);
}

void
MenloWiFiSparkCore::Flush()
{
  ResetWatchdog();   // Tell watchdog we are still alive
  return m_client->flush();
}

void
MenloWiFiSparkCore::PrintDiagnostics(Stream* stream)
{
  //IPAddress ip;
  //long rssi;

  ResetWatchdog();   // Tell watchdog we are still alive

  //
  // SparkCore does not appear to export much information
  // from spark_wlan.h
  //

  stream->print(F("SSID: "));
  //stream->println(WiFi.SSID());
  stream->println("???");

  //  ip = WiFi.localIP();
  stream->print(F("IP Address: "));
  stream->println("???");
  //stream->println(ip);

  //  ip = WiFi.gatewayIP();
  stream->print(F("Gateway IP Address: "));
  stream->println("???");
  //stream->println(ip);

  ResetWatchdog();   // Tell watchdog we are still alive

  //  ip = WiFi.subnetMask();
  stream->print(F("SubNet Mask: "));
  //stream->println(ip);
  stream->println("???");

  //  rssi = WiFi.RSSI();
  stream->print(F("RSSI: "));
  stream->print("???");
  //stream->print(rssi);
  stream->println(F(" dBm"));

  ResetWatchdog();   // Tell watchdog we are still alive
}

//
// To save power the WiFi can be turned on and off
//

void
MenloWiFiSparkCore::PowerOn()
{
  //
  // WiFiClass is a static defined in the SparkCore environment by
  // spark_wiring_wifi.cpp
  //
  WiFi.on();
}

void
MenloWiFiSparkCore::PowerOff()
{
  //
  // WiFiClass is a static defined in the SparkCore environment by
  // spark_wiring_wifi.cpp
  //
  WiFi.off();
}

