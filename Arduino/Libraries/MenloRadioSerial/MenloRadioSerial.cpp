
//
// 05/21/2017
//
// Cost is 4K code, few hundred bytes RAM.
//
// See MenloRadio.h for tradeoffs.
//

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
 *  Date: 12/14/2014
 *  File: MenloRadioSerial.h
 */

#include "MenloPlatform.h"
#include "MenloObject.h"
#include "MenloMemoryMonitor.h"
#include "MenloDispatchObject.h"
#include "MenloDebug.h"
#include "MenloRadio.h"
#include "MenloRadioSerial.h"

//
// A lot of times radio status is required.
// This allows quick enabling of radio status.
//
#define RDBG_PRINT_ENABLED 0

#if RDBG_PRINT_ENABLED
#define RDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#else
#define RDBG_PRINT(x)
#endif

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

MenloRadioSerial::MenloRadioSerial()
{
  m_inputBuffer = NULL;
  m_inputBufferHead = 0;
  m_inputBufferTail = 0;
  m_inputBufferSize = 0;
}

int
MenloRadioSerial::Initialize(
    MenloRadio* radio,
    byte* targetAddress,
    uint8_t* inputBuffer,
    uint8_t inputBufferSize,
    uint8_t maxRadioBufferSize,
    unsigned long sendTimeout
    )
{
  // Radio configuration
  m_radio = radio;
  m_targetAddress = targetAddress;
  m_sendTimeout = sendTimeout;

  m_radioBufferSize = m_radio->GetPacketSize();

  m_transmitSize = 0;

  // Serial input buffer configuration
  m_inputBuffer = inputBuffer;
  m_inputBufferHead = 0;
  m_inputBufferTail = 0;
  m_inputBufferSize = inputBufferSize;

  // Serial/protocol status
  m_lineStatus = 0;
  m_inputBufferOverflow = false;
  m_inputSequenceToggle = false;
  m_outputSequenceToggle = false;

  //
  // We don't register for radio receive packets
  // until the receive stream is enabled.
  //
  m_receiveStreamEnabled = false;

  //
  // We register for PollEvents for processing
  // outgoing packets.
  //
  m_pollEvent.object = (MenloObject*)this;
  m_pollEvent.method = (MenloEventMethod)&MenloRadioSerial::PollEvent;

  MenloDispatchObject::RegisterPollEvent(&m_pollEvent);

  return 0;
}

unsigned long
MenloRadioSerial::PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    unsigned long waitTime;

    xDBG_PRINT("DweetRadioSerial PollEvent");

    //
    // sender is NULL on PollEvent's since the sender is the base
    // static MenloDispatchObject's poll/event dispatcher.
    //

    //
    // Unlike most MenloEvent's PollEvents are allowed to
    // raise new events from the PollEvent handler. This is because
    // it has the same top of stack status as the virtual method
    // Poll() of MenloDispatchObject.
    //

    waitTime = ProcessOutput();

    return waitTime;
}

//
// This enables the radio serial receive stream
//
void
MenloRadioSerial::EnableReceive(bool value)
{
  if (value) {
      if (m_receiveStreamEnabled) {
          return;
      }

      m_receiveStreamEnabled = true;

      //
      // Register for radio receive packets
      //

      //
      // This is kind of a hack since we don't subclass MenloObject
      // due to subclassing Arduino Stream to get Arduino serial port
      // semantics.
      //
      // But only the pointer value is used here.
      // No MenloObject fields or methods are referenced.
      //
      m_radioEvent.object = (MenloObject*)this;

      m_radioEvent.method = (MenloEventMethod)&MenloRadioSerial::RadioEvent;
      m_radio->RegisterReceiveEvent(&m_radioEvent);
  }
  else {
      if (!m_receiveStreamEnabled) {
          return;
      }

      m_receiveStreamEnabled = false;
      m_radio->UnregisterReceiveEvent(&m_radioEvent);
  }
}

unsigned long
MenloRadioSerial::RadioEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
  unsigned long retVal = MAX_POLL_TIME;
  MenloRadioEventArgs* radioArgs = (MenloRadioEventArgs*)eventArgs;

  // Check memory
  MenloMemoryMonitor::CheckMemory(LineNumberBaseRadioSerial + __LINE__);

  // Process incoming radio packet whose data is indicated in the event
  ReceiveDataIndication(radioArgs->data, radioArgs->dataLength);

  return retVal;
}

unsigned long
MenloRadioSerial::ProcessOutput()
{
    //
    // Poll() is used to push transmit buffers out.
    //
    // Receive data occurs as MenloRadio receive events from the
    // MenloRadio Poll() loop.
    //

    //
    // Check to see if any outgoing data is still queued
    // in the radio buffer. This must be sent first
    // before we can use the buffer for receive.
    //
    if (m_transmitSize != 0) {

      //
      // Output is from write() calls which are buffered. To minimize packets
      // sent output is buffered until a full packet is ready, or until the
      // next Poll() interval.
      //
      // A serial transaction which is writing response data will finish
      // writing its data before returning to the Poll() loop. So this is
      // an indication we can flush now.
      //
      if (PushOutput() == (-1)) {

        //
	// Flush so as not to lock up the channel
        // We could add retries here, but the packet radio already does
        // at the hardware link level.
        //
        FlushOutput();
      }
    }

    //
    // Application drives how often the radio is checked
    // This could be a different value if we performed radio
    // retries on transmit failures.
    //
    return  MAX_POLL_TIME;
}

//
// This is called to pace output for gateways
// that can lose packets.
//
void
MenloRadioSerial::PaceOutput()
{
  delay(MENLO_RADIO_SERIAL_OUTPUT_PACE);
}

//
// This is called PushOutput() since it tries to push
// the current radio buffer out on the channel.
//
// FlushOutput() destroys any queued data.
//
int
MenloRadioSerial::PushOutput()
{
  int retVal;

  if (m_transmitSize == 0) {
      return 0;
  }

  DBG_PRINT("MenloRadioSerial PushOutput packet=");

  // Skip radio serial packet header
  DBG_PRINT_STRING((const char*)&m_transmitBuffer[1]);

  DBG_PRINT("\n");

  //
  // Packet header has been formated and kept up to date
  // by queued writes, so its ready to send.
  //
  retVal = m_radio->Write(
      //m_targetAddress,
      NULL,
      m_transmitBuffer,
      m_radioBufferSize, // Buffer must be the full radio packet
      m_sendTimeout
      );

  if (retVal == 0) {
    // Nothing sent, return failure so caller can handle
    RDBG_PRINT("MenloRadioSerial: radio write failure");
    return -1;
  }

  // Only reset on successful send so caller can retry.
  retVal = m_transmitSize;

  m_transmitSize = 0;

  RDBG_PRINT("radio write success");

  // pace the output
  PaceOutput();

  return retVal;
}

int
MenloRadioSerial:: available()
{
    int length;
    length = (int)(m_inputBufferSize + m_inputBufferHead - m_inputBufferTail) % m_inputBufferSize;

    if (length != 0) {
        DBG_PRINT_NNL("RadioSerial available length=");
        DBG_PRINT_INT(length);
    }

    return length;
}

int
MenloRadioSerial::read()
{
    int c = peek();
    if (c == -1) return -1;

    // Advance the pointer to consume the character
    m_inputBufferTail = (unsigned int)(m_inputBufferTail + 1) % m_inputBufferSize;
    return c;
}

//
// Put char into the buffer
//
void
MenloRadioSerial::CopyCharToInputBuffer(char c)
{
    int index;

    // unsigned math wraps around correctly
    index = (unsigned int)(m_inputBufferHead + 1) % m_inputBufferSize;

    if (index == m_inputBufferTail) {
      m_inputBufferOverflow = true;

      // Must drop the character due to buffer overflow
      RDBG_PRINT("RadioSerial input buffer overflow");
      return;
    }

    m_inputBuffer[m_inputBufferHead] = c;
    m_inputBufferHead = index;
}

void
MenloRadioSerial::CopyToInputBuffer(uint8_t* buffer, uint8_t length)
{
    while (length-- > 0) {
      CopyCharToInputBuffer(buffer[0]);
      buffer++;
    }

    DBG_PRINT_NNL("packet ");
    DBG_PRINT_INT(length);
}

//
// Returns count transfered on success.
//
// If the transferred count is > 0, but less
// than requested length it was a partial send.
//
// The amount returned was sent, but the remainder
// was not due to an error reported from the radio level
// such as loss of signal, etc.
//
size_t
MenloRadioSerial::write(
      const uint8_t* appBuffer,
      size_t appLength
      )
{
  int retVal;
  uint8_t toGo = appLength;

  while (toGo != 0) {

      retVal = write(appBuffer[0]);
      if (retVal != 1) {

        //
	// error, return. May have sent partial, so indicate this.
        // Returning the precise amount sent allows the upper caller
        // to handle any retries to try and avoid losing a portion
        // of a longer message.
        //
        xDBG_PRINT("RS write inc");
        return appLength - toGo;
      }
      else {
        appBuffer++;
	toGo--;
      }
  }

  DBG_PRINT("RadioSerial write complete");

  return appLength;
}

//
// The Stream.h/Print.h present characters one at a time, but
// we want to pack as many as possible in a radio packet.
//
// This is done by queuing the output and sending when a whole
// radio packet is full. For any partial sends they are sent at
// Poll() time since applications will send a series of output
// characters and then return to the Poll() loop. So this indicates
// a given write sequence has completed and we should send the
// queued data right away, even if its a partial packet.
//
size_t
MenloRadioSerial::write(
      uint8_t c
      )
{
  int retVal;
  MenloRadioSerialData* pk = (MenloRadioSerialData*)m_transmitBuffer; 
  int dataCount;

  //
  // If a previous send did not go out for a full buffer
  // attempt to push again. A failure here we don't accept
  // the new character as the outgoing buffer is full.
  //
  // This allows callers to handle retries, etc.
  //
  if (m_transmitSize == m_radioBufferSize) {
    retVal = PushOutput();
    if (retVal == 0) {
      return -1;
    }
  }

  if (m_transmitSize == 0) {

    // starting a new radio packet
    m_transmitSize = MENLO_RADIO_SERIAL_DATA_OVERHEAD + 1;
    pk->type_and_size = MENLO_RADIO_SERIAL_DATA;
    pk->type_and_size |= PACKET_SIZE_MASK(1);

    if (m_outputSequenceToggle) {

      // Send current packet with sequence != 0
      pk->type_and_size |= PACKET_SEQUENCE;

      // Next packet will be sent as sequence == 0
      m_outputSequenceToggle = false;
    }
    else {
      // Send current packet with sequence == 0
      pk->type_and_size &= ~PACKET_SEQUENCE;

      // Next packet will be sent as sequence != 0
      m_outputSequenceToggle = true;
    }

    pk->data[0] = c;

    // We queue the data until either a full packet or the next Poll()
    return 1;
  }
  
  //
  // Existing radio packet already formatted. Add the character to it.
  //
  dataCount = m_transmitSize - MENLO_RADIO_SERIAL_DATA_OVERHEAD;

  pk->data[dataCount] = c;
  dataCount++;
  m_transmitSize++;

  // Update the count in the packet header
  pk->type_and_size &= ~PACKET_SIZE;
  pk->type_and_size |= PACKET_SIZE_MASK(dataCount);

  if (m_transmitSize == m_radioBufferSize) {

    //
    // Full buffer, attempt to push it out
    // We don't look at the return value since we accepted the
    // character in the queue anyway.
    //
    PushOutput();
  }

  return 1;
}

//
// Receive data indication with radio data packet
//
// Input: Received Radio packet
//
void
MenloRadioSerial::ReceiveDataIndication(uint8_t* buf, uint8_t bufSize)
{
  uint8_t size;

  if (bufSize == 0) {
      DBG_PRINT("RadioSerial bufSize == 0");
      return;
  }

  //
  // Upper bits == 0xC0 means an extended packet type
  // with the lower 5 bits defining it.
  //
  // The type mask macro's must be used to ensure the encoding
  // of the sequence bit (5) is not included in the comparison.
  //
  if (PACKET_TYPE_MASK(buf[0]) == PACKET_TYPE_EXTENDED) {

      //
      // Lower 5 subtypes bits == 0x01 is link control
      //
      if (PACKET_SUBTYPE_MASK(buf[0] == MENLO_RADIO_LINKCONTROL)) {

          //
          // Link control packet
          //

          if (buf[1] & RADIO_LINKCONTROL_SYNC) {
	      //
	      // Nothing to do right now. It's a NOP to re-sync comms.
              // Compiler has no overhead for this function. But allows the
              // definition to be claimed/validated.
              //
	      RDBG_PRINT("Radio sync received");
          }
      }
      else {
          RDBG_PRINT("RS not packet");
      }

      return;
  }

  // See if a serial input buffer has been setup
  if (m_inputBuffer == NULL) {
      xDBG_PRINT("RadioSerial no input buffer");
      return;
  }

  //
  // All RadioSerial packets have a common first byte format for
  // packet type, sequence number, data payload size.
  //

  //
  // Determine if the packet is the expected sequence, and not
  // a duplicate due to re-transmission with a lost ack, or
  // timing window.
  //
  if (m_inputSequenceToggle) {

      // Expect next packet to have sequence != 0
      if (PACKET_SEQUENCE_MASK(buf[0]) == 0) {
	// bad packet sequence, drop
        xDBG_PRINT("RadioSerial bad sequence 0");
        return;
      }

      // packet accepted, toggle input sequence
      m_inputSequenceToggle = false;
  }
  else {
      // Expect next packet to have sequence == 0
      if (PACKET_SEQUENCE_MASK(buf[0]) != 0) {
	// bad packet sequence, drop
        xDBG_PRINT("RadioSerial bad sequence 1");
        return;
      }

      // packet accepted, toggle input sequence
      m_inputSequenceToggle = true;
  }

  size = PACKET_SIZE_MASK(buf[0]);

  if (PACKET_TYPE_MASK(buf[0]) == MENLO_RADIO_SERIAL_DATA) {
    MenloRadioSerialData* pk = (MenloRadioSerialData*)buf; 

    //
    // 0 length packets are allowed.
    // They are useful for sequence re-sync after startup or
    // transmission errors.
    //
    if (size != 0) {
        // Will set m_inputBufferOverflow if it results in overflow
        CopyToInputBuffer(&pk->data[0], size);
        RDBG_PRINT("RS accepted input data");
    }

    return;
  }
  else if (PACKET_TYPE_MASK(buf[0]) == MENLO_RADIO_SERIAL_DATA_LINE_STATUS) {
    MenloRadioSerialDataLineStatus* st = (MenloRadioSerialDataLineStatus*)buf; 

    m_lineStatus = st->line_status;

    if (size != 0) {
        // Will set m_inputBufferOverflow if it results in overflow
        CopyToInputBuffer(&st->data[0], size);
        RDBG_PRINT("RS accepted input data");
    }

    return;
  }
  else {
    // Unknown packet, but in the RadioSerial category
    xDBG_PRINT("RS unknwn pkt");
    return;
  }
}
