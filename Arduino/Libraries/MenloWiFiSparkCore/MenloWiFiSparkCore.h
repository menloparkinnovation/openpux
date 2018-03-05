
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
 *  File: MenloWiFiSparkCore.h
 */

#ifndef MenloWiFiSparkCore_h
#define MenloWiFiSparkCore_h

#include "MenloPlatform.h"

#include <MenloUtility.h>

// Implements MenloWiFi
#include <MenloWiFi.h>

//
// Handler for WiFI on SparkCore
//
// The SparkCore uses the TI CC3300 WiFi chip on an ARM32 processor.
//
// By default it has its own auto configuration that can be setup
// using an iPhone app, a utility, or even a serial interface.
//

class MenloWiFiSparkCore : public MenloWiFi {

public:
  
  MenloWiFiSparkCore();

  //
  // Initialization occurs outside of the constructor
  // to allow flexibility when instances are used.
  //
  // Returns 0 on success.
  //
  int InitializeClient(
      Client* client,
      int retryCount
      );

  //
  // Attempt to connect to a single network with up to retryCount times.
  //
  // Note: If autosignon is configured, this just returns.
  //
  virtual int SignOn(char* ssid, char* passPhrase, int retryCount);

  //
  // Returns 0 if hardware is present, error code if not
  //
  virtual bool IsHardwarePresent();

  virtual void Stop();

  //
  // Print network diagnostics to the debugger serial
  //
  virtual void PrintDiagnostics(Stream* stream);

  virtual bool IsConnected();

  virtual bool IsSignedOn();

  virtual int DataBytesAvailable();

  virtual int Read();

  virtual int Read(uint8_t* buf, size_t size);  

  virtual size_t Write(uint8_t c);

  virtual size_t Write(const uint8_t* buf, size_t size);

  virtual void Flush();

  virtual void PowerOn();

  virtual void PowerOff();

private:

  Client *m_client;

  bool m_signedOn;
};

#endif // MenloWiFiSparkCore_h
