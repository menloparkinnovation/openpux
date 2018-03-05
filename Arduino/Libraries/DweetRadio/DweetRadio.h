
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
 *  Date: 02/24/2015
 *  File: DweetRadio.h
 *
 *  Radio Dweet handling.
 */

#ifndef DweetRadio_h
#define DweetRadio_h

#include "MenloObject.h"
#include "DweetStrings.h"
#include "MenloRadio.h"

class DweetRadio : public MenloObject {

 public:

  DweetRadio() {
    m_dweet = NULL;
    m_radio = NULL;
  }

  int Initialize(MenloDweet* dweet, MenloRadio* radio);

  //
  // By default received radio packets are not forwarded
  // as unsolicited Dweet's. This enables the Dweet stream
  // for received radio packets.
  //
  void EnableReceiveDweetStream(bool value);

  // Handlers for each DWEET message
  int ChannelHandler(char* buf, int size, bool isSet);
  int TxAddrHandler(char* buf, int size, bool isSet);
  int RxAddrHandler(char* buf, int size, bool isSet);
  int PowerHandler(char* buf, int size, bool isSet);
  int AttentionHandler(char* buf, int size, bool isSet);
  int OptionsHandler(char* buf, int size, bool isSet);
  int GatewayHandler(char* buf, int size, bool isSet);

 protected:

  //
  // Radio commands processing.
  //
  // These commands allow Dweet access to packet radio configuration.
  //
  // Returns 1 if the command is recognized.
  // Returns 0 if not.
  //
  int ProcessRadioCommands(MenloDweet* dweet, char* name, char* value);

  bool
  ProcessToRadioBuffer(
      char* str,
      uint8_t* channel,
      uint8_t* radioBuffer,
      uint8_t radioBufferLength
      );

  int ProcessRadioTransmit(MenloDweet* dweet, char* name, char* value);

  void
  SendRadioReceiveDweet(
      uint8_t channel,
      uint8_t* radioBuffer,
      uint8_t radioBufferLength
      );

 private:

  MenloRadio* m_radio;

  MenloDweet* m_dweet;

  bool m_receiveDweetStreamEnabled;

  //
  // DweetRadio is a client of MenloDweet for unhandled Dweet events
  // to support radio configuration Dweet messages.
  //
  // These messages may arrive on any transport.
  //

  // Event registration
  MenloDweetEventRegistration m_dweetEvent;

  // DweetEvent function
  unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

  //
  // DweetRadio is a client of MenloRadio for receive packets
  //

  // Event registration
  MenloRadioEventRegistration m_radioEvent;

  // RadioEvent function
  unsigned long RadioEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // DweetRadio_h
