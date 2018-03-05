
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
 *  Date: 05/30/2016
 *  File: DweetWiFi.h
 *
 *  WiFi Dweet handling.
 */

#ifndef DweetWiFi_h
#define DweetWiFi_h

#include "MenloObject.h"
#include "DweetStrings.h"
#include "MenloWiFi.h"

class DweetWiFi : public MenloObject {

 public:

  DweetWiFi() {
    m_dweet = NULL;
    m_wifi = NULL;
  }

  int Initialize(MenloDweet* dweet, MenloWiFi* wifi);

  // Handlers for each DWEET message
  int WiFiSSID(char* buf, int size, bool isSet);
  int WiFiPassword(char* buf, int size, bool isSet);
  int WiFiChannel(char* buf, int size, bool isSet);
  int WiFiPower(char* buf, int size, bool isSet);
  int WiFiAttention(char* buf, int size, bool isSet);
  int WiFiOptions(char* buf, int size, bool isSet);

 protected:

  //
  // Allows customization of WiFi Dweets.
  //
  // Returns 1 if the command is recognized.
  // Returns 0 if not.
  //
  int ProcessWiFiCommands(MenloDweet* dweet, char* name, char* value);

 private:

  MenloWiFi* m_wifi;

  MenloDweet* m_dweet;

  //
  // DweetRadio is a client of MenloDweet for unhandled Dweet events
  // to support WiFi configuration Dweet messages.
  //
  // These messages may arrive on any transport.
  //

  // Event registration
  MenloDweetEventRegistration m_dweetEvent;

  // DweetEvent function
  unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // DweetWiFi_h
