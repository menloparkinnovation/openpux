
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
// MenloRadio defines the basic packet type for applications
// that use the radio in a way that allows them to be used
// on the same radio channels at the same time.
//
// The overall packet category is defined here, while the
// subtype and format is defined by the particular radio
// application.
//
// This is represented in the link level control structure
// that is used by applications and the radio support
// infrastructure itself.
//
// Currently its optimized for efficient encoding of
// serial over packet radio emulation "radio serial", but
// have 32 possible sub-application encodings for sensor
// data, commands, and configuration.
//
// This link header is the first byte and determines
// the packet type and usage.
//
// The upper 2 bits (7 + 6) define the packet type.
//
//  00 0x00 -> (upper bits) unassigned, can be used for extended types.
//  01 0x40 -> RadioSerial Data 31 bytes
//  10 0x80 -> RadioSerial Data + Line status 30 bytes data
//  11 0xC0 -> Extended packet type, type is in lower 5 bits
//
// Bit 5 is a sequence number in all cases
//
// The 5 lower bits 4 - 0 indicate packet size unless its 0xC0
// (Extended packet type 0) which indicates the second packet
// byte is an packet subtype giving a 256 address application
// space name.
//

//
// Packet size used for small radios.
//
// The protocols for small sensors in MenloFramework are optimized
// around this size. Other transport types can use larger packet
// sizes as required.
//
// A transport with a smaller size must isolate the multiple
// packet assembly required. This is a core design decision
// taken around the popular low power nRF24L01+ series.
//
// Note: The 16 byte packet size of the nRF24L01+ is not used,
// but application specific scenarios may choose to fall within
// this restriction for very low bandwidth communication in noisy
// environments, which is the only reason to consider this
// packet size.
//
// Note: MenloRadio will ensure it can operate within efficient
// packet sizes for LORA radios which may have smaller packets
// due to operating very close to the noise floor. Currently
// its application specific as how many bytes after the first
// 2-3 to use.
//
#define MENLO_RADIO_PACKET_SIZE 32

//
// The type mask macro's must be used to ensure the encoding
// of the sequence bit (5) is not included in the comparison.
//
#define PACKET_TYPE 0xC0
#define PACKET_TYPE_MASK(t) (t & 0xC0)

struct MenloRadioPacket {
  uint8_t type;
  uint8_t data[31];
};

//
// If PACKET_TYPE_MASK == PACKET_TYPE then its a subtype
// with byte 1 representing 32 possible subtypes.
//
// This means 0x1F types are available encoded as:
// 0xC0 - 0xDF.
//

// This is helpful to make the code clear.
#define PACKET_TYPE_EXTENDED 0xC0

//
// This is for a full comparision such as packet[0] == MENLO_RADIO_LINK_CONTROL
// and can prevent nested #if's in packet handers saving code space.
//
#define PACKET_SUBTYPE_MASK(t) (t & 0xDF)

//
// extended packet type 0x00 (encoded as 0xC0) is used
// to represent an additional application specific subtype
// in a generic way.

#define PACKET_TYPE_SUBTYPE 0xC0

// overhead not available to application
#define PACKET_TYPE_SUBTYPE_OVERHEAD 2

struct MenloRadioPacketSubType {
  uint8_t type;      // 0xC0
  uint8_t subType;   // 0 - 0xFF
  uint8_t data[30];  // Application defined
};

//
// Core link control packets are defined here for use
// across all radio applications.
//

//
// 0xC1 - Radio Link Control.
//
// This is an "out of band" link control and status
// packet for line status, re-sync, etc.
//

#define MENLO_RADIO_LINKCONTROL 0xC1

// overhead not available to application
#define MENLO_RADIO_LINKCONTROL_OVERHEAD 2

struct MenloRadioLinkControl {
  uint8_t type;
  uint8_t control;
  uint8_t data[30];
};

#define MENLO_RADIO_LINKCONTROL_SIZE sizeof(struct MenloRadioSerialLinkControl)

//
// Control bits for link control.
//
// These go into the control field (byte 1) of the packet.
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
// Some common application scenarios define their extended packet
// types here to avoid conflicts.
//
// In general, extended packet types from 0x10 - 0x1F are available
// without conflict (except with an other application).
//
// The lower range is used by the framework to allow multiple
// applications to be deployed on a given sensor on a shared
// radio channel.
//

//
// General sensor data, configuration, and state.
//
// These are designed to be shared by applications, but allow
// for specific customizations.
//

//
// Note: This is a guideline for "registered" applications to avoid
// conflict.
//
// These values go into the "application" field.
//

//
// Generic sensor
//
//  - What ever you come up with.
//
#define MENLO_RADIO_APPLICATION_SENSOR          0x00

//
// Menlo Satellite Sensor - Weather Station
//
//  - Wind speed, direction, temperature, humidity, rain, light, barometer.
//  - Data you can send to Weather Underground (TM) or similar.
//
#define MENLO_RADIO_APPLICATION_WEATHER_STATION 0x10

//
// Menlo Satellite Sensor - Light House
//
//  - Light control, day/night, period, duration, color.
//  - Solar and battery status.
//  - Optional NEMA 0183 weather data link.
//  - Optional NEMA 0183 GPS data link for accurate
//    time, providing a form of differential GPS, etc.
//    - If the lighthouse moves, see Sea Buoy.
//
#define MENLO_RADIO_APPLICATION_LIGHT_HOUSE     0x20

//
// Menlo Satellite Sensor - Boat Monitor
//
//  - also can monitor an RV, remote cabin, etc.
//
#define MENLO_RADIO_APPLICATION_BOAT_MONITOR    0x30

//
// Menlo Satellite Sensor - Garden Monitor
//
//  - Plants, lawn, etc.
//   - Plant soil/moisture, temperature, light, humidity
//   - Could integrate with water valve control for self watering,
//     drip irrigation.
//   - Can be use by vinyards, farmers, etc.
//
#define MENLO_RADIO_APPLICATION_GARDEN_MONITOR  0x40

//
// Menlo Satellite Sensor - Motion Light
//
//  - Solar or A/C powered
//  - Control on/off time, flash, be notified when triggered
//
#define MENLO_RADIO_APPLICATION_MOTION_LIGHT    0x50

//
// Menlo Satellite Sensor - House Monitor
//
//  - Temperature, wet/dry, humidity, presence, noise, motion
//
#define MENLO_RADIO_APPLICATION_HOUSE_MONITOR   0x60

//
// Menlo Satellite Sensor - Water Valve
//
//  - Main house valve, accessory valve, sprinklers, etc.
//
#define MENLO_RADIO_APPLICATION_WATER_VALVE     0x70

//
// Menlo Satellite Sensor - Humidor
//
// - Also can monitor a refrigerator, etc.
//
#define MENLO_RADIO_APPLICATION_HUMIDOR         0x80

//
// Menlo Satellite Sensor - Hot Tub
//
// - also can monitor a pool, fountain, koi pond, collection basin
//
#define MENLO_RADIO_APPLICATION_HOT_TUB         0x90

//
// Menlo Satellite Sensor - Solar Battery
//
//  - Monitoring of solar charged batteries.
//   - Could be used for other intermittent energy sources such as wind, etc.
//
#define MENLO_RADIO_APPLICATION_SOLAR_BATTERY   0xA0

//
// Menlo Satellite Sensor - Ham Radio Antenna
//
//  - Remote control of a ham radio antenna
//    - StepIR(TM), Rotor, Tuner, etc.
//
#define MENLO_RADIO_APPLICATION_HAM_ANTENNA     0xB0

//
// Menlo Satellite Sensor - Ham Radio
//
//  - Remote control of a ham radio, DF unit, repeater, etc.
//
#define MENLO_RADIO_APPLICATION_HAM_RADIO       0xC0

//
// Menlo Satellite Sensor - Sea Buoy
//
//  - A Buoy at Sea reporting environmental conditions
//   - Can be used to monitor if off station/adrift with GPS
//   - Wave action, sea temp, solar radiation, boat traffic counting, etc.
//
#define MENLO_RADIO_APPLICATION_SEA_BUOY        0xD0

//
// User defined use above this range.
//
#define MENLO_RADIO_APPLICATION_USER_DEFINED_E 0xE0
#define MENLO_RADIO_APPLICATION_USER_DEFINED_F 0xF0

//
// 0xC2 - Sensor data
//
// Sensor data represents what a sensor reports about its
// current status, readings, environment, and any trigger/alarm
// statuses.
//
#define MENLO_RADIO_SENSOR_DATA 0xC2

//
// This is a generic packet definition that allows
// decoding and setting the common header fields.
//
struct SensorPacket {

  uint8_t  type;        // Based on packet type
  uint8_t  flags;       // High 4 bits application. Low 4 bits define state.

  uint8_t data[30];
};

#define SENSOR_PACKET_SIZE sizeof(struct SensorPacket)

// The upper 4 bits define the application.
#define SENSOR_PACKET_APPLICATION_MASK(a) (a & 0xF0)

// If any data in the packet represents an alarm condition this bit is set.
#define SENSOR_PACKET_ALARM        0x01

// If a gateway has more traffic for a sensor, this bit is set.
// This is also called a receiver alert to listen for more traffic.
#define SENSOR_PACKET_MORE_TRAFFIC 0x02

// This represents a reply to a base packet
#define SENSOR_PACKET_REPLY        0x04

// This flag is used by each packet type for their own options.
#define SENSOR_PACKET_OPTION       0x08

//
// 0xC3 - Sensor get state
//
// Get the current runtime operating state for a sensor.
//
#define MENLO_RADIO_SENSOR_GET_STATE 0xC3

//
// 0xC4 - Sensor set state.
//
// Update the operating state, cancel alarms, etc.
//
// This does not update the power on default state which
// is done with Set Configuration messages.
//
#define MENLO_RADIO_SENSOR_SET_STATE 0xC4

//
// Note: The same packet is used to request state
// (0xC3) and for its reply with the application specific
// state.
//
// How the data[] is handled depends on whether the device
// is the sender, or receiver.
//
// For the sender its a request for state, and
// data[] may have optional command options such as which
// state values to get.
//
// For the receiver it represents state that was
// sent, with data[] representing the state data
// and options.
//
// Each application (byte 1) defines the actual format of
// the packet. So applications should ignore packets for
// applications that don't match their number.
//

struct MenloRadioSensorState {

  // 0xC3 == Get state request
  // 0xC4 == Set state request
  uint8_t type;

  uint8_t application;
  uint8_t data[28];
};

//
// Many simple sensors can operate by a set of simple virtual
// registers that are similar to hardware chip control
// and status registers.
//
// These optional packet encoding for the Get/Set sensor
// state document this pattern. It's not a requirement to use
// it if a sensor operates at a higher logical level, but
// many simple sensors/scenarios can be represented in this
// manner with minimal handling code or the requirement to
// define their own application specific data packet formats.
//
// To represent the use of the status/control virtual register
// model a common IoT application pattern for operation on a lossy
// channel is as follows:
//
// House Burgler Alarm:
//
//  The alarm provides the following:
//
//  operating state such as the open/closed status of door and window sensors.
//
//  control state such as whether the alarm is armed or not.
//
//  configuration state which determines which door and window sensors
//  are present, or overidden.
//
// A Get State request would return alarm arming status, and virtual
// registers with bits to define the open/close status of the door/window
// sensors. All get state requests never change state, since a get state
// request or reply could be lost, requiring application re-transmission.
//
// A set state request would specific with register bits to clear or set,
// such as arming and disarming the alarm. In addition, once an alarm has
// triggered, it can only move the operating state out of current alarm
// status by a specific clear current alarm command. This is to ensure
// any alarm message/status is not lost.
//
// Set state requests must be idempotent, similar to programming over
// TCP UDP. This means if requests or reply's are lost a resend does
// not change the overall state. For example sending multiple alarm
// arm, or disarm requests does not change the ultimate status goal
// which is a specific arming target state. It would be a mistake
// to keep an "arm/disarm" counter and have that influence operating
// state since there is no way to ensure reliable end to end transfer.
//
// Note: Set state commands have not expected reply. The new state
// can only be determined by the application by the following:
//
//  - Sending a get state command to query current virtual register status
//
//  - In a future normal sensor data/status message which indicates its
//    current operational state.
//
// To help better understand a cloud connected IoT device think of
// the "cloud" (or application on your phone for local loop) as having
// a "goal state" for your sensor. It will continue to try and contact
// the sensor with set state and get state commands to ensure it has
// received a proper set state message, moved to the goal state, and now
// reporting back that its actually achieved the goal state by the get
// state message.
//
// Follow this pattern and your applications will be reliable in
// the cloud. Even over HTTP/TCP networks to a cloud service provider,
// who says that a crash of a worker server in the front end did not
// lose the request/reply? All scaleable cloud architectures use pools
// of worker servers to receive and process requests, with just
// "routing around" failing servers without worry as to lost messages.
//
// This also extends to these low power packet radios deployed in
// your configuration which is typically a noisy environment, radios
// are mostly sleeping, etc.
//
struct MenloRadioSensorStatusRegisters {

  // 0xC3 == Get state request
  // 0xC4 == Set state request
  uint8_t type;

  uint8_t application;

  //
  // The layout is flexible in that an application can choose
  // a 8, 16, or 32 bit status register or combination
  // as required.
  //

  //  bits 0 - 7 represent Status0 - Status7 registers present.
  uint8_t validRegisters;

  uint8_t status0;

  uint8_t status1;
  uint8_t status2;

  uint16_t status3;
  uint16_t status4;

  uint32_t status5;
  uint32_t status6;
  uint32_t status7;

  uint8_t data[10];
};


//
// 0xC5 - Sensor get configuration
//
// Get configuration for a sensor in EEPROM, nvram, or flash.
//
#define MENLO_RADIO_SENSOR_GET_CONFIGURATION 0xC5

//
// 0xC6 - Sensor set configuration
//
// Set configuration for a sensor in EEPROM, nvram, or flash.
//

#define MENLO_RADIO_SENSOR_SET_CONFIGURATION 0xC6

//
// Note: The same packet is used to request configuration
// (0xC5) and for its reply with the application specific
// configuration.
//
// How the data[] is handled depends on whether the device
// is the sender, or receiver.
//
// For the sender its a request for configuration, and
// data[] may have optional command options such as which
// configuration values to get.
//
// For the receiver it represents configuration that was
// sent, with data[] representing the configuration data
// and options.
//
// Each application (byte 1) defines the actual format of
// the packet. So applications should ignore packets for
// applications that don't match their number.
//

struct MenloRadioSensorConfiguration {

  // 0xC5 == Get configuration request
  // 0xC6 == Set configuration request
  uint8_t type;

  uint8_t application;
  uint8_t data[28];
};

//
// An alternate suggested encoding for MenloRadioSensorConfiguration
// that specifies which fields are present, and breaks them out for
// a general purpose model.
//
// An application free to define its own form provided the first
// two bytes represent the configuration and application unique packet ids.
//

struct MenloRadioSensorConfigurationGeneric {

  // 0xC5 == Get configuration request
  // 0xC6 == Set configuration request
  uint8_t type;
  uint8_t application;

  uint8_t  commandBits; // bit to define which fields are valid to be set.
  uint8_t  configForm;  // SENSOR_CONFIG_FORM_GENERIC

  //
  // User defined format for custom paramters.
  // Including commandBits available above.
  //
  uint8_t data[28];
};

//
// 0xC7 - Dweet
//
// Binary encoding of Dweet commands in 32 byte radio packet.
//
// Dweets normally operate over the serial/USB port for
// configuration and represent general purpose application
// and sensor configuration.
//
// By default they are available through the MenloRadioSerial
// protocol which emulates a NMEA 0183 serial port/channel
// over the 32 byte packet radios.
//
// But NMEA 0183 encoded commands in ASCII text require many
// packets to be sent/received, and re-tranmitted if lost.
//
// This can be problematic in noisy radio environments, or with
// a distant sensor at the fringe of reception. Allowing these
// commands to occur in a single self contained 32 byte radio
// packet enhances reliabilty of communication, and a better
// end to end application result and user experience.
//
// In addition for very small mostly battery powered sensors
// on the AtMega328 code and data space is extremely tight,
// and the 4k code, few hundred bytes of RAM is needed to
// fit fairly detailed application scenario into 32K code,
// 2K ram.
//

// Encoding them to operate over the 32 byte

#define MENLO_RADIO_DWEET 0xC7

//
// Binary Dweets over 32 byte packet radio channels.
//
// $PDWT,GETCONFIG=RADIOPOWER
// $PDWT,SETCONFIG=RADIOPOWER:00
//
struct MenloRadioDweet {

  uint8_t  type;           // 0xC7
  uint8_t  dweetSubsystem; // target MenloRadio, MenloPower, etc.
  uint8_t  dweetCode;      // SETCONFIG, GETCONFIG, GETSTATE, SETSTATE
  uint8_t  pad0;

  //
  // 28 bytes
  //
  // Interpreted by the specific dweet command
  //
  uint32_t parameter0;
  uint32_t parameter1;
  uint32_t parameter2;
  uint32_t parameter3;
  uint32_t parameter4;
  uint32_t parameter5;
  uint32_t parameter6;
};

//
// 0xC8 - Dweet Reply
//

#define MENLO_RADIO_DWEET_REPLY 0xC8

struct MenloRadioDweetReply {

  uint8_t  type;           // 0xC8
  uint8_t  dweetSubsystem; // target MenloRadio, MenloPower, etc.
  uint8_t  dweetCode;      // SETCONFIG, GETCONFIG, GETSTATE, SETSTATE
  uint8_t  pad0;

  //
  // 28 bytes
  //
  // Interpreted by the specific dweet command
  //
  uint32_t parameter0;
  uint32_t parameter1;
  uint32_t parameter2;
  uint32_t parameter3;
  uint32_t parameter4;
  uint32_t parameter5;
  uint32_t parameter6;
};

//
// Default Radio Power interval
//
#define RADIO_DEFAULT_POWER_INTERVAL (0L) // 0 means always on

//
// MenloRadio raises an Event when a receive packet is available.
//
// Values for AtMega328 on Arduino 1.6.8:
//
//       Code before adding radio event handler:
//         24,748 code 755 ram
//
//       Code after adding radio event handler:
//
//         24,808 code 764 ram
//
//       Total: 60 bytes code, 9 bytes ram
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
    virtual int Write(
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
