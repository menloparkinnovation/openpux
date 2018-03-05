
/*
 * Modifications, additions, subtractions are
 * Copyright (C) 2012 Menlo Park Innovation LLC
 *
 * This work incorporates one or more "open" or "shared" source
 * licenses. As such, its use and licensing conforms to the licenses
 * of the code incorporated.
 *
 * Menlo Park Innovation LLC does not provide any warranty, support,
 * or restrictions on its use and may be distributed under the
 * terms of the contained license(s).
 *
 *  Date: 11/25/2012
 *  File: OS_Cosm.h
 *
 * Code contained within comes from the following open, shared, community,
 * public domain, or other licenses:
 *
 * Arduino example sketch for Cosm/Pachube in the IDE.
 *
 */

/*
 * Cosm Client class
 */

#ifndef OS_Cosm_h
#define OS_Cosm_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#include <Arduino.h>
#include <inttypes.h>

// Internet support
#include <SpI.h>
#include <WiFi.h>

// This class implements MenloCloud
#include <MenloCloud.h>

class OS_Cosm : public MenloCloud {

public:
  
  OS_Cosm();

  int Initialize(
    WiFiClient* networkClient,
    char* userAgent,
    char* apiKey,
    int feedID,
    Stream* debugStream
    );

  int GetLength(int data);

  int StartHttpPut(int contentLength);

  int PerformHttpRequest(char* dataString);

  //
  // MenloCloud implementations
  //
  virtual bool Process();

  virtual int SendDataString(char* dataString);

private:

  Stream*  m_debugStream;

  //char server[] = "api.cosm.com";  // DNS address for cosm API
  IPAddress m_cosmServer; // numeric IP for api.cosm.com

  WiFiClient* m_client;

  char* m_userAgent;
  char* m_apiKey;
  int   m_feedID;

  // Used to prevent flooding the server and getting shutdown
  bool m_lastConnected;
  long m_lastConnectionTime;
  long m_postingInterval;  

  char* m_contentBuffer;
};

#endif // OS_Cosm_h
