
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
 *  Date: 11/20/2012
 *  File: MenloRadio.h
 *
 *  MenloRadio provides an abstract class interface for various
 *  embedded radios.
 *
 *  It works with a single common buffer for send and receive
 *  in order to minimize memory usage in small embedded systems.
 */

#ifndef MenloRadio_h
#define MenloRadio_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#include <inttypes.h>
#endif

#include "MenloObject.h"
#include "MenloDispatchObject.h"
#include "MenloTimer.h"

// 250 ms for now
#define RADIO_MINIMUM_POWER_TIMER (250)

// 0:00000000 + '\0'
#define RADIO_ATTENTION_SIZE 11

// Amount of time waiting for a send attention interval
#define SEND_ATTENTION_TIMEOUT (250)

//
// Radio Power Design:
//
// In the radio power design there are battery powered
// sensors who only perodically power on their radio,
// and gateways which are always listening since they
// have more power available. The power design is built
// around this "mostly sleeping" sensor model with
// "always on" gateway receivers.
//
// This is fairly common for an Internet of Things (IoT)
// scenario in which you have many low power, low cost
// battery and/or solar powered sensors and one or more
// gateways/hubs implemented with a RaspberryPi or similar
// computer that has main A/C power.
//
// The MenloRadio class can control a radio implementation
// drivers power for sensor nodes that are mostly sleeping.
//
// By default after initialization the radio remains always
// power on in receive mode so that ReceiveDataReady() will
// return if a packet is available.
//
// The radio driver will switch to transmit mode for write
// of packets, and revert back to receive mode when the
// write is done.
//
// If SetPowerTimer() is set to a value other than 0 the
// MenloRadio will monitor radio activity and if no
// activity occurs within the specified period the
// radio drivers OnPowerOff routine is invoked.
//
// Radio activity is defined as any receive data available, or
// radio transmissions started by either the application or
// the link level control. The radio driver can call
// SetActivity() where appropriate to indicate a reason
// to keep the radio powered on for another period.
//
// When the radio is in power down state ReceiveDataReady()
// will return no data until PowerOn() is called. Once
// PowerOn() is called, the radio will remain powered on
// for the last period specified in SetPowerTimer() if
// there is no activity, or for as long as the power down
// period since the last radio activity was present.
//
// Any request to transmit will automatically power on
// the radio until the transmit packet has completed.
//
// Once the transmit packet has completed the radio will
// revert to receive mode and remain powered on for
// the radio power down interval, unless further activity
// occurs. After a power down interval of no activity, the
// radio will power down again.
//
// The radio power control is intended to extend battery
// life for battery powered sensors. In a situation that
// the sensor sends periodic updates the power interval
// is fairly short to keep the duty cycle low and battery
// life extended.
//
// When a set of command and control messages is to be
// sent to the sensor the MenloRadio acting as a gateway
//

//
// MenloRadio provides a link level control structure
// that is used by applications and the radio support
// infrastructure itself.
//
// This link header is the first byte and determines
// the packet type and usage.
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

#define PACKET_TYPE 0xC0
#define PACKET_TYPE_MASK(t) (t & 0xC0)

// If PACKET_TYPE_MASK == PACKET_TYPE then its a subtype
#define PACKET_SUBTYPE_MASK(t) (t & 0xDF)

//
// This is a link control packet
//
// The data here is "out of band", and used to communicate
// link status, re-sync, etc.
//

#define MENLO_RADIO_LINKCONTROL 0xC1
#define MENLO_RADIO_LINKCONTROL_OVERHEAD 2

struct MenloRadioLinkControl {
  uint8_t type;
  uint8_t control;
  uint8_t data[30];
};

#define MENLO_RADIO_LINKCONTROL_SIZE sizeof(struct MenloRadioSerialLinkControl)

//
// Control bits
//

//
// Sync is a no-op packet used to re-sync a channel after
// loss of a packet in a multiple packet sequence.
//
#define RADIO_LINKCONTROL_SYNC 0x01

//
// Attention is a link control packet which indicates
// that the radio should remain powered on to receive
// further commands and data.
//
// The period is determined by the application and
// can be milliseconds, seconds, or intervals.
//
#define RADIO_LINKCONTROL_ATTENTION 0x02

struct MenloRadioLinkControlAttention {
  uint8_t type;
  uint8_t control;
  uint8_t period0;
  uint8_t period1;
  uint8_t period2;
  uint8_t period3;
  uint8_t data[26];
};

//
// Packet size used for small radios.
//
// The protocols for small sensors in MenloFramework as optimized
// around this size. Other transport types can use larger packet
// sizes as required.
//
#define MENLO_RADIO_PACKET_SIZE 32

//
// Default Radio Power interval
//
#define RADIO_DEFAULT_POWER_INTERVAL (0L) // 0 means always on

//
// MenloRadio raises an Event when a receive packet is available.
//
class MenloRadioEventArgs : public MenloEventArgs {
 public:
  uint8_t* data;
  uint8_t  dataLength;
  // FUTURE: Pointer to receive address structure for multiple client support
};

// Introduce the proper type name. Could be used for additional parameters.
class MenloRadioEventRegistration : public MenloEventRegistration {
 public:
};

class MenloRadio : public MenloDispatchObject {

public:
  
    MenloRadio();

    virtual int Initialize();

    //
    // The following methods have a base implementation in
    // MenloRadio.
    //
    virtual unsigned long Poll();

    void RegisterReceiveEvent(MenloRadioEventRegistration* callback);

    void UnregisterReceiveEvent(MenloRadioEventRegistration* callback);

    //
    // Read data into buffer, wait for up to timeout.
    //
    // A caller can call ReceiveDataReady() to poll for data available
    // before calling Read() to allow for zero wait reads with timeout == 0.
    //
    // The received data is made available in the buffer provided at
    // initialization time.
    //
    // Note: Even if ReceiveDataReady() returns TRUE for data available
    // a Read() can still return 0 since the packet could have been
    // overwritten, corrupted, or even grabbed by the link level
    // implementation as a non-data channel packet.
    //
    int Read(unsigned long timeout);

    //
    // Write fixed sized data from the supplied buffer waiting for
    // up to timeout to send.
    //
    int Write(
        byte *targetAddress,
        uint8_t* transmitBuffer,
        uint8_t transmitBufferLength,
        unsigned long timeout
        );

    //
    // The following pure virtual contracts must be implemented
    // by a given radio implementation.
    //

    //
    // Note: ASCII character strings are used to allow
    // different radios to control their format. Could be
    // hex numbers, strings, or symbolic names.
    //
    // MenloRadio supplies a base implementation for
    // persistent settings which can be delegated to support
    // a common configuration store.
    //

    //
    // Radio Settings
    //

    virtual int Channel(char* buf, int size, bool isSet) = 0;

    virtual int RxAddr(char* buf, int size, bool isSet) = 0;

    virtual int TxAddr(char* buf, int size, bool isSet) = 0;

    virtual int Power(char* buf, int size, bool isSet) = 0;

    virtual int Attention(char* buf, int size, bool isSet) = 0;

    //
    // Radio specific catch all for packet size, CRC, data rate, etc.
    //
    // Radio specific format string.
    //
    // SETSTATE=OPTIONS:CRC.32
    //
    virtual int Options(char* buf, int size, bool isSet) = 0;

    //
    // Returns whether there is data ready in the receiver.
    //
    // Note: If radio power is not on this will always return
    // no data available.
    //
    virtual bool ReceiveDataReady() = 0;

    //
    // Indicates if the transmitter is busy sending a packet.
    //
    virtual bool TransmitBusy() = 0;

    //
    // Packet size of the radio
    //
    virtual uint8_t GetPacketSize() = 0;

    //
    // On board common buffer routines.
    //
    // These work with small radios with a fixed
    // size on board buffer. This is optimized
    // for embedded systems with very little
    // memory.
    //
    // The contract specifies the receive buffer
    // as the transmit buffer is supplied by the caller.
    //

    //
    // Return the buffer used for receive.
    //
    // It's size is the PacketSize of the radio implementation.
    //
    virtual uint8_t* GetReceiveBuffer() = 0;

    virtual int OnRead(unsigned long timeout) = 0;

    //
    // Write fixed sized data from the supplied buffer waiting for
    // up to timeout to send.
    //
    virtual int OnWrite(
        byte *targetAddress,
        uint8_t* transmitBuffer,
        uint8_t transmitBufferLength,
        unsigned long timeout
        ) = 0;

    //
    // Radio Power Control
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

protected:

    void ProcessAttentionReceive(uint8_t* buf);

    void ProcessAttentionSend();

    //
    // These are indications to the radio implementation subclass
    // to transition to a particular power state.
    //
    virtual void OnPowerOn() = 0;

    virtual void OnPowerOff() = 0;

private:

    bool m_receiveEventSignaled;

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
    // MenloRadio is an Event generator for radio packet data events
    //
    MenloEvent m_eventList;

    //
    // MenloRadio utilizes timer events for power scheduling.
    //    
    MenloTimer m_timer;
    MenloTimerEventRegistration m_timerEvent;

    // TimerEvent function
    unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloRadio_h
