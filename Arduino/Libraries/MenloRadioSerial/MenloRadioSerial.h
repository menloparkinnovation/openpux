
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

#ifndef MenloRadioSerial_h
#define MenloRadioSerial_h

#include "MenloPlatform.h"
#include "MenloRadio.h"

#define MENLO_RADIO_SERIAL_OUTPUT_PACE (0) // 0

//
// MenloRadioSerial sends unmodified 8 bit serial streams in the
// data buffer.
//
// Since its serial we do not have to worry about boundaries.
//
// MenloRadioSerial is tuned to the Nordic nRF24L01+ radio.
//
// It relies upon the following features:
//
//  - hardware point to point addressing
//  - hardware ECC/CRC
//  - hardware retransmissions on packet loss
//  - fixed packet size with known record boundaries
//  - single packet queue/buffer
//
//  As such the protocol can be vastly simplified.
//
// A single bit is used for sequence since there is a single
// buffer in queue on the short range radio network.
//
// Even with hardware re-transmissions, duplicate packets can
// occur at the edge of a retry/timeout window.
// 
// More complex protocols can be developed for radios that need
// it.
//
// type_and_size byte
//
// The upper 2 bits (7 + 6) define the packet type.
//
//  00 0x00 -> (upper bits) unassigned
//  01 0x40 -> RadioSerial Data 31 bytes
//  10 0x80 -> RadioSerial Data + Line status 30 bytes data
//  11 0xC0 -> Extended packet type, type is in lower 5 bits
//
// Bit 5 is a sequence number in all cases
//
// The 5 lower bits 4 - 0 indicate packet size unless its 0xC0
// in which they represent the sub packet type.
//
// The upper 2 bits set (0xC0 - 0xDF) are reserved as packet types
//
// This encoding allows 32 packet types with size in lower 5 bits
//

//
// These are unique to RadioSerials packet types
//
#define PACKET_SEQUENCE 0x20
#define PACKET_SIZE 0x1F

#define PACKET_SEQUENCE_MASK(t) (t & 0x20)
#define PACKET_SIZE_MASK(t) (t & 0x1F)

// The size represents the actual data payload size
#define DATA_PACKET_MAX_SIZE 31
#define DATA_PACKET_LINE_STATUS_MAX_SIZE 30

#define MENLO_RADIO_SERIAL_DATA          0x40
#define MENLO_RADIO_SERIAL_DATA_OVERHEAD    1

struct MenloRadioSerialData {
  uint8_t type_and_size;
  uint8_t data[31];
};

#define MENLO_RADIO_SERIAL_DATA_SIZE sizeof(struct MenloRadioSerialData)

//
// When line status transitions need to be communicated
// a data byte is reduced to include it.
//

#define MENLO_RADIO_SERIAL_DATA_LINE_STATUS 0x80
#define MENLO_RADIO_SERIAL_DATA_LINE_STATUS_OVERHEAD 2

struct MenloRadioSerialDataLineStatus {
  uint8_t type_and_size;
  uint8_t line_status;
  uint8_t data[30];
};

#define MENLO_RADIO_SERIAL_DATA_LINE_STATUS_SIZE \
  sizeof(struct MenloRadioSerialDataLineStatus)

//
// line_status
// DTR transitions indicate programming start
//
#define RADIO_SERIAL_DTR 0x01
#define RADIO_SERIAL_CTS 0x02
#define RADIO_SERIAL_RTS 0x04

//
// This is a sensor defined packet
//
// It is not queued as serial input, but processed
// as an out of band entry.
//
// The caller submits these directly to the radio.
//
// They are defined here since the radio serial data processing
// knows to skip these. By keeping the definition for the
// basic packet header and type confusion is avoided.
//
// The specific format after the type byte is defined by
// the application program/sensor data handler.
//

#define MENLO_RADIO_SERIAL_SENSORDATA 0xC2
#define MENLO_RADIO_SERIAL_SENSORDATA_OVERHEAD 1

//
// Note: By convention applications can use data[0]
// (second byte) as a sub-type code for display of the format.
//

//
// These subtype defines assists in decoding packets
//

// Libraries/LightHouseApp.h
#define LIGHTHOUSE_SENSOR_DATA_SUBTYPE  0xFF

// Libraries/MotionLightApp.h
#define MOTIONLIGHT_SENSOR_DATA_SUBTYPE 0xFE

struct MenloRadioSerialSensorData {
  uint8_t type;
  uint8_t data[31];
};

#define MENLO_RADIO_SERIAL_SENSORDATA_SIZE sizeof(struct MenloRadioSerialSensorData)

//
// MenloRadioSerial implements the same contracts
// as Arduino HardwareSerial.
//
class MenloRadioSerial : public Stream {

public:

  MenloRadioSerial();

  //
  // Stream.h and Print.h contract virtuals
  //
  virtual void flush() {
    m_inputBufferHead = m_inputBufferTail = 0;
  }

  virtual int peek() {
    if (m_inputBufferHead == m_inputBufferTail) return -1;
    return m_inputBuffer[m_inputBufferTail];
  }

  virtual int available();

  virtual int read();

  virtual size_t write(uint8_t c);

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
  virtual size_t write(const uint8_t *buffer, size_t size);

  //
  // MenloRadioSerial public interfaces
  //

  int Initialize(
      MenloRadio* radio,
      byte* targetAddress,
      uint8_t* inputBuffer,
      uint8_t intputBufferSize,
      uint8_t maxRadioBufferSize,
      unsigned long sendTimeout
      );

  //
  // Data is not made available until receive from the radio
  // is enabled.
  //
  // This allows sharing of a radio for multiple usage.
  //
  void EnableReceive(bool value);

  uint8_t GetLineStatus() {
    return m_lineStatus;
  }

  bool GetOverflowStatus() {
    return m_inputBufferOverflow;
  }

  void ResetOverflowStatus() {
      m_inputBufferOverflow = false;
  }

 protected:

  //
  // Receive data indication with radio data packet
  //
  // Input: Received Radio packet
  //
  void ReceiveDataIndication(uint8_t* buffer, uint8_t length);

  //
  // Put char into the buffer
  //
  void CopyCharToInputBuffer(char c);
  void CopyToInputBuffer(uint8_t* buffer, uint8_t length);

  //
  // Push queued output data
  //
  // Warning: If the radio send fails data remains in the buffer
  // for future send attempts.
  //
  // FlushOutput() can be used to abandon the data buffer.
  //
  int PushOutput();

  //
  // Destroy the data in the send buffer
  //
  void FlushOutput() {
    m_transmitSize = 0;
  }

  //
  // Pace output write packets.
  //
  // Low power (Arduino) gateways can drop back to back
  // packets from RadioSerial.
  //
  void PaceOutput();

 private:

  // Process output packets. Invoked from the PollEvent.
  unsigned long ProcessOutput();

  MenloRadio* m_radio;

  //
  // Each instance of MenloRadioSerial is a virtual COM port connection
  // using the radios transport level addressing.
  //
  byte* m_targetAddress;
  unsigned long m_sendTimeout;

  //
  // Packet radios have a fix size for transmit + receive
  // Protocol packet type + length determines actual useful payload.
  //
  // Radios may use short packets, with the rest of the data
  // in the buffer being undefined. Use of these radios understand
  // these guidelines.
  //
  uint8_t m_radioBufferSize;

  //
  // This buffer allows async buffering of small serial writes
  // for output on the radio.
  //
  uint8_t m_transmitBuffer[MENLO_RADIO_PACKET_SIZE];
  uint8_t m_transmitSize;

  // Supplied by caller for serial input from radio packets received
  uint8_t* m_inputBuffer;
  uint8_t m_inputBufferSize;

  // This implements a circular buffer for Stream/Print contract
  uint8_t m_inputBufferHead;
  uint8_t m_inputBufferTail;

  // virtual line status
  uint8_t m_lineStatus;
  bool m_inputBufferOverflow;

  //
  // The inputSequenceToggle is the next packet expected.
  //
  bool m_inputSequenceToggle;

  //
  // The outputSequenceToggle is the value for the next
  // packet to send.
  //
  bool m_outputSequenceToggle;

  //
  // MenloRadioSerial is a client of MenloRadio for receive packets
  //

  bool m_receiveStreamEnabled;

  // Event registration
  MenloRadioEventRegistration m_radioEvent;

  // RadioEvent function
  unsigned long RadioEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

  //
  // MenloRadioSerial uses the PollEvent to push queued transmit buffers out
  //
  MenloEventRegistration m_pollEvent;

  // PollEvent function
  unsigned long PollEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloRadioSerial_h
