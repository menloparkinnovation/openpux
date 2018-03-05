
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

//
// 12/14/2014
//
// See MenloRadioSerial for an updated design for radio serial.
// 

/*
 *  Date: 11/21/2012
 *  File: MenloSensorProtocol.h
 */

/*
 * This class handles protocol generation and formatting
 * for communicating between sensors and the gateway unit.
 */

#ifndef MenloSensorProtocol_h
#define MenloSensorProtocol_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#include "MenloPlatform.h"

//
// New message design
//
// 07/09/2014
//

#define MENLO_SENSOR_PROTOCOL_TYPE_BASE 0x80

//
// The header is used by both thelLink + application level
//

//
// The low order type bit indicates whether its a
// 16 byte or 32 byte message to avoid having a dedicated
// field for message size. By using a small, fixed set of
// defined message sizes a general lookup table to translate
// message type to size is not required in routers who are
// just passing messages along.
//

#define MESSAGE_TYPE_SMALL (type) ((type & 0x01) == 0)
#define MESSAGE_TYPE_LARGE (type) ((type & 0x01) != 0)

// Message type index
#define MESSAGE_TYPE_INDEX(msg) (msg & 0xFE)

// Does the network need to know from and to at the link level?
// Or is the sensorid# an application level concern?
// Nack's and other communications are at the peer level
struct MenloSensorProtocolHeader {
  uint8_t Type;
  uint8_t UnitID;
};

//
// Link level message.
//
// Internal format is defined by the link layer.
//
// TODO: Handle time sync/clocks on the link layer.
//
// ?? Link layer a separate header ??
// ?? Esp. for nRf24L01P's
//

// Total size is 32 bytes
struct MenloSensorProtocolLinkControl {
  MenloSensorProtocolHeader Header;

  // May only be 14 bytes if (MESSAGE_TYPE_SMALL(Header.Type))
  uint8_t data[30];
};

#define MENLO_SENSOR_PROTOCOL_LINK_CONTROL (MENLO_SENSOR_PROTOCOL_TYPE_BASE + 0x02)

#define MENLO_SENSOR_PROTOCOL_LINK_CONTROL_SIZE sizeof(struct MenloSensorProtocolLinkControl))

// Total size is 32 bytes
struct newMenloSensorProtocolData {
  MenloSensorProtocolHeader Header;

  // May only be 14 bytes if (MESSAGE_TYPE_SMALL(Header.Type))
  uint8_t data[30];
};

#define newMENLO_SENSOR_PROTOCOL_SENSOR_DATA (MENLO_SENSOR_PROTOCOL_TYPE_BASE + 0x04)

#define newMENLO_SENSOR_PROTOCOL_DATA_SIZE sizeof(struct newMenloSensorProtocolData)

//
// Accumulating data messages will overwrite any
// previous accumulating data messages from the SensorId
// in the buffer.
//
// This is used for information in which the latest value
// is the most important, such as a temperature reading.
//

struct MenloSensorProtocolDataAccumulating {
  MenloSensorProtocolHeader Header;

  // May only be 14 bytes if (MESSAGE_TYPE_SMALL(Header.Type))
  uint8_t data[30];
};

#define MENLO_SENSOR_PROTOCOL_SENSOR_DATA_ACCUMULATING (MENLO_SENSOR_PROTOCOL_TYPE_BASE + 0x06)

#define MENLO_SENSOR_PROTOCOL_DATA_ACCULATING_SIZE sizeof(struct newMenloSensorProtocolDataAccumulating)

//
// An ack indicates the Data message was received
// and queued for later forwarding on a best efforts
// basis. The node should consider it sent.
//
// It also specifies the next contact time schedule for the
// gateway to allow the sensor to save energy and not have
// to re-transmit when there is not likely to be buffer space
// available, or the gateway is listening.
//
struct MenloSensorProtocolDataAck {
  MenloSensorProtocolHeader Header;
  uint8_t NextContactTimeLsb;
  uint8_t NextContactTimeMsb;
};

#define MENLO_SENSOR_PROTOCOL_DATA_ACK (MENLO_SENSOR_PROTOCOL_TYPE_BASE + 0x08)

#define MENLO_SENSOR_PROTOCOL_DATA_ACK_SIZE sizeof(struct MenloSensorProtocolDataAck))

//
// An ACK may also have a message for the receiver.
//
// There is no room for updating the clock time, so time
// may drift until a retry occurs.
//
// TODO: Clock recovery? Link level ping?
//
// Do we just handle clocks at the link level?
//
// Re-adjust them when out of sync?
//
// Have their own messages?
//
// On Nacks we could still have the time since its
// an expectation of upper level buffering/forwarding,
// vrs. keeping the link level clocks synced together.
//
struct MenloSensorProtocolDataAckWithData {
  MenloSensorProtocolHeader Header;
  uint8_t data[30];
};

#define MENLO_SENSOR_PROTOCOL_DATA_ACK_WITH_DATA (MENLO_SENSOR_PROTOCOL_TYPE_BASE + 0x0A)

#define MENLO_SENSOR_PROTOCOL_DATA_ACK_WITH_DATA_SIZE sizeof(struct MenloSensorProtocolDataAckWithData))

//
// The message was not handled due to lack of
// buffer space.
//
// NextContactTime represents when to re-contact
// the gateway in 1/4 second (250ms) intervals.
//
struct MenloSensorProtocolDataNack {
  MenloSensorProtocolHeader Header;
  uint8_t NextContactTimeLsb;
  uint8_t NextContactTimeMsb;
};

#define MENLO_SENSOR_PROTOCOL_DATA_NACK (MENLO_SENSOR_PROTOCOL_TYPE_BASE + 0x0C)

#define MENLO_SENSOR_PROTOCOL_DATA_NACK_SIZE sizeof(struct MenloSensorProtocolDataNAck))

//
// Radio serial sends unmodifier serial streams in the data buffer.
//
// Since its serial we do not have to worry about boundaries.
//

struct MenloSensorProtocolRadioSerial {
  MenloSensorProtocolHeader Header;

  uint8_t size; // low 5 bits is size of data
  uint8_t port; // low 2 bits is port #, high 5 bits DTR, CTS, RTS

  // May only be 12 bytes if (MESSAGE_TYPE_SMALL(Header.Type))
  uint8_t data[28];
};

// DTR transitions indicate programming start
#define RADIO_SERIAL_DTR 0x80

#define RADIO_SERIAL_PORT(port) (port & 0x03)

#define MENLO_SENSOR_PROTOCOL_RADIO_SERIAL (MENLO_SENSOR_PROTOCOL_TYPE_BASE + 0x0E)

#define MENLO_SENSOR_PROTOCOL_RADIO_SERIAL_SIZE sizeof(struct MenloSensorProtocolRadioSerial)

//
// Simple sensors operate on a basic set of masks for commands
// in an indempotent manner.
//
// They return stateless data readings about their state at
// the time of transmission.
//
// This overlay allows these simple sensors to minimize code
// and complexity by overlaying the standard generic data packet.
//
// MenloSensorProtocolData dataBuffer;
//
// MenloSensorProtoclDataOverlay* p = 
//  (MenloSensorProtoclDataOverlay*)&dataBuffer.data[0];
//

// Total size is 28 bytes
struct MenloSensorProtocolDataOverlay {

  // 20 bytes sensor readings
  uint16_t Sensor0;
  uint16_t Sensor1;
  uint16_t Sensor2;
  uint16_t Sensor3;
  uint16_t Sensor4;
  uint16_t Sensor5;
  uint16_t Sensor6;
  uint16_t Sensor7;
  uint16_t Sensor8;
  uint16_t Sensor9;

  // 8 bytes sensor masks
  uint16_t Mask0;
  uint16_t Mask1;
  uint16_t Mask2;
  uint16_t Mask3;
};

struct newMenloSensorProtocolDataResponse {
  MenloSensorProtocolHeader Header;
  uint8_t Flags;
  uint8_t Page; // Page can be used by application level

  uint8_t data[28];
};

// Define packet types

// 0 is a reserved type since it appears in cleared buffers
#define MENLO_SENSOR_PROTOCOL_RESERVED  0x00

// 0xFF is a reserved type since it appears from hardware interface errors
#define MENLO_SENSOR_PROTOCOL_RESERVED2 0xFF

// Length mask
#define MENLO_SENSOR_PROTOCOL_LENGTH_MASK 0x1F

// Continue mask
#define MENLO_SENSOR_PROTOCOL_CONTINUE_MASK 0x10

// Calculate Length from packet
#define MENLO_SENSOR_PROTOCOL_LENGTH_FROM_PACKET(value) \
    ((value & MENLO_SENSOR_PROTOCOL_LENGTH_MASK) + 1)

// Calculate Length to packet
#define MENLO_SENSOR_PROTOCOL_LENGTH_TO_PACKET(length) (length - 1)

/*
 * Sensor Register packet (sensor unit to gateway)
 *
 * This allows a newly powered on client sensor unit to
 * send its unique 64 bit serial number, a 32 bit model number,
 * and two option bytes and receive an assigned unit ID.
 *
 * The UnitID for a sensor register packet is 0, as this is
 * the gateway unitID and not a valid sensor Id as the
 * sensors ID has not yet been assigned.
 *
 */

#define MENLO_SENSOR_PROTOCOL_REGISTER 0x01

struct MenloSensorProtocolRegister {

  uint8_t Type;
  uint8_t UnitID;

  uint8_t SerialNumberByte0;
  uint8_t SerialNumberByte1;
  uint8_t SerialNumberByte2;
  uint8_t SerialNumberByte3;
  uint8_t SerialNumberByte4;
  uint8_t SerialNumberByte5;
  uint8_t SerialNumberByte6;
  uint8_t SerialNumberByte7;

  uint8_t ModelNumberByte0;
  uint8_t ModelNumberByte1;
  uint8_t ModelNumberByte2;
  uint8_t ModelNumberByte3;
};

#define MENLO_SENSOR_PROTOCOL_REGISTER_SIZE sizeof(struct MenloSensorProtocolRegister)

/*
 * Sensor Register Response packet (gateway to sensor unit)
 *
 * This contains the assigned unit ID from the gateway unit.
 *
 * The returned serial number, model number, and options is
 * the ones from the client unit, and the client is obligated
 * to compare these values with its own in order to see if
 * in fact this response and assigned unit ID is for this
 * unit. This is because the response may have occurred
 * over a shared radio channel and could be for another
 * unit.
 *
 * UnitID - UnitID assigned to this sensor client from the gateway
 *
 * SerialNumber - Sensor units own serial number reflected back
 *
 * ModelNumber - Sensor units own model number reflected back
 *
 * Options - Sensor units own options reflected back
 *
 */

#define MENLO_SENSOR_PROTOCOL_REGISTER_RESPONSE 0x02

struct MenloSensorProtocolRegisterResponse {

  uint8_t Type;
  uint8_t UnitID;

  uint8_t SerialNumberByte0;
  uint8_t SerialNumberByte1;
  uint8_t SerialNumberByte2;
  uint8_t SerialNumberByte3;
  uint8_t SerialNumberByte4;
  uint8_t SerialNumberByte5;
  uint8_t SerialNumberByte6;
  uint8_t SerialNumberByte7;

  uint8_t ModelNumberByte0;
  uint8_t ModelNumberByte1;
  uint8_t ModelNumberByte2;
  uint8_t ModelNumberByte3;
};

#define MENLO_SENSOR_PROTOCOL_REGISTER_RESPONSE_SIZE sizeof(struct MenloSensorProtocolRegisterResponse)

/*
 * Sensor data readings (sensor unit client to gateway)
 *
 * The actual sensors connected to each sensor input value
 * is configurable based on the sensor unit type.
 *
 * Note: This takes the place of a separate
 * "poll gateway for new commands" packet as most radios
 * always send a minimum length packet, so we might as well
 * send useful information such as latest sensor readings
 * data. This assumes that the energy to gather the
 * readings is minimal. A specific application may use
 * known out of range values for the sensors to indicate
 * its not a reading, but just a poll.
 *
 */

//
// An nRF24L01 radio has a default 16 byte packet size.
// It can be configured for up to 32 bytes which is what we use.
//

//
// Sensors may place frequently updated readings in the first
// sensor assignments so small packets may be used for most
// communication with infrequent large packets where desired.
//

#define MENLO_SENSOR_PROTOCOL_SENSOR_DATA 0x03

// Total size is 32 bytes
struct MenloSensorProtocolData {

  uint8_t Type;
  uint8_t UnitID;

  uint8_t LsbSensor0;
  uint8_t MsbSensor0;
  uint8_t LsbSensor1;
  uint8_t MsbSensor1;
  uint8_t LsbSensor2;
  uint8_t MsbSensor2;
  uint8_t LsbSensor3;
  uint8_t MsbSensor3;
  uint8_t LsbSensor4;
  uint8_t MsbSensor4;
  uint8_t LsbSensor5;
  uint8_t MsbSensor5;

  uint8_t LsbSensor6;
  uint8_t MsbSensor6;
  uint8_t LsbSensor7;
  uint8_t MsbSensor7;
  uint8_t LsbSensor8;
  uint8_t MsbSensor8;
  uint8_t LsbSensor9;
  uint8_t MsbSensor9;

  // These are the sensors current mask values
  uint8_t LsbMask0;
  uint8_t MsbMask0;
  uint8_t LsbMask1;
  uint8_t MsbMask1;
  uint8_t LsbMask2;
  uint8_t MsbMask2;
  uint8_t LsbMask3;
  uint8_t MsbMask3;
};

#define MENLO_SENSOR_PROTOCOL_SENSOR_DATA_SIZE sizeof(struct MenloSensorProtocolData)

#define MENLO_SENSOR_PROTOCOL_SENSOR_DATA_MASKS 0x4

// Total size is 32 bytes
struct MenloSensorProtocolDataMasks {

  uint8_t Type;
  uint8_t UnitID;

  // These are the sensors current mask values
  uint8_t LsbMask0;
  uint8_t MsbMask0;
  uint8_t LsbMask1;
  uint8_t MsbMask1;
  uint8_t LsbMask2;
  uint8_t MsbMask2;
  uint8_t LsbMask3;
  uint8_t MsbMask3;
  uint8_t LsbMask4;
  uint8_t MsbMask4;
  uint8_t LsbMask5;
  uint8_t MsbMask5;
  uint8_t LsbMask6;
  uint8_t MsbMask6;
  uint8_t LsbMask7;
  uint8_t MsbMask7;
  uint8_t LsbMask8;
  uint8_t MsbMask8;
  uint8_t LsbMask9;
  uint8_t MsbMask9;
};

#define MENLO_SENSOR_PROTOCOL_SENSOR_DATA_MASKS_SIZE sizeof(struct MenloSensorProtocolDataMasks)

/*
 * Sensor Data Response packet (gateway to client sensor unit)
 *
 * Unit ID is the addressed client unit.
 *
 * SleepTime is the amount of time the sensor unit can
 * sleep without listening, checking back in with the
 * main gateway unit.
 *
 * Length is the entire packet length including packet
 * type, unit ID, length, and sleep time bytes. It is
 * used to allow variable length additional command
 * bytes to be returned to the sensor unit, such
 * as commands to change the state of lights, actuators,
 * etc, for the client sensor unit.
 *
 * TargetMask - Command states for outputs such as
 *  lights on, off, etc.
 *
 */

#define MENLO_SENSOR_PROTOCOL_DATA_RESPONSE 0x05

struct MenloSensorProtocolDataResponse {

  // Header
  uint8_t Type;
  uint8_t UnitID;

  uint8_t SleepTime0;
  uint8_t SleepTime1;
  uint8_t SleepTime2;
  uint8_t SleepTime3;

  //
  // The target masks are sent from the cloud through the
  // gateway to attempt to achieve a desired sensor state.
  //
  // Sensors provide their current mask state along with
  // sensor readings when they provide readings to the gateway
  // so that applications can test end-to-end sensor state.
  //
  // This is important since a command/message can get lost
  // at any step from the cloud to the gateway to a sensor.
  // 
  uint8_t LsbTargetMask0;
  uint8_t MsbTargetMask0;
  uint8_t LsbTargetMask1;
  uint8_t MsbTargetMask1;
  uint8_t LsbTargetMask2;
  uint8_t MsbTargetMask2;
  uint8_t LsbTargetMask3;
  uint8_t MsbTargetMask3;
  uint8_t LsbTargetMask4;
  uint8_t MsbTargetMask4;
  uint8_t LsbTargetMask5;
  uint8_t MsbTargetMask5;
  uint8_t LsbTargetMask6;
  uint8_t MsbTargetMask6;
  uint8_t LsbTargetMask7;
  uint8_t MsbTargetMask7;
  uint8_t LsbTargetMask8;
  uint8_t MsbTargetMask8;
  uint8_t LsbTargetMask9;
  uint8_t MsbTargetMask9;

  // 28 bytes till here
};

#define MENLO_SENSOR_PROTOCOL_DATA_RESPONSE_MORE_DATA_FLAG 0x01

#define MENLO_SENSOR_PROTOCOL_DATA_RESPONSE_SIZE sizeof(struct MenloSensorProtocolDataResponse)

/*
 * Wakeup client sensor unit (gateway to sensor client unit)
 */

#define MENLO_SENSOR_PROTOCOL_WAKEUP 0x06

struct MenloSensorProtocolWakeup {
  uint8_t Type;
  uint8_t UnitID;
};

#define MENLO_SENSOR_PROTOCOL_WAKEUP_SIZE sizeof(struct MenloSensorProtocolWakeup)

/*
 * Sensor Poll (sensor to gateway)
 *
 * A sensor issues this packet to the gateway to see if
 * there are any commands/updates for the sensor.
 *
 * Typically this is done because the gateway has set
 * the continue bit on a previous response to the sensor.
 *
 * The response to a Sensor poll is any valid gateway
 * to sensor packet which can contain new command and/or
 * configuration data.
 *
 * The continue bit in those packets determine when
 * the sensor should stop issuing sensor polls.
 *
 */

#define MENLO_SENSOR_PROTOCOL_SENSOR_POLL 0x07

struct MenloSensorProtocolSensorPoll {
  uint8_t Type;
  uint8_t UnitID;
};

#define MENLO_SENSOR_PROTOCOL_SENSOR_POLL_SIZE sizeof(struct MenloSensorProtocolSensorPoll)

//
// These structures allow application storage of common
// sensor exchange data.
//
// The previous structures are not suitable for applications
// as they represent the low level unmarshalled data which is
// contained in a transient radio link buffer.
//
// It's more efficient to pass a pointer to this structure in
// a higher level application buffer than copies of these variables
// (or their pointers) up and down the stack many times.
//
// This is especially important in such memory tight environments.
//

struct MenloSensorProtocolDataAppBuffer {

  uint8_t Type;
  uint8_t UnitID;

  uint16_t Sensor0;
  uint16_t Sensor1;
  uint16_t Sensor2;
  uint16_t Sensor3;
  uint16_t Sensor4;
  uint16_t Sensor5;
  uint16_t Sensor6;
  uint16_t Sensor7;
  uint16_t Sensor8;
  uint16_t Sensor9;

  uint16_t Mask0;
  uint16_t Mask1;
  uint16_t Mask2;
  uint16_t Mask3;
  uint16_t Mask4;
  uint16_t Mask5;
  uint16_t Mask6;
  uint16_t Mask7;
  uint16_t Mask8;
  uint16_t Mask9;
};

//
// This is the buffer for sending data from
// the application to the sensor.
//
struct MenloSensorProtocolDataAppResponseBuffer {
  uint8_t Type;
  uint8_t UnitID;

  uint16_t Command;

  uint16_t TargetMask0;
  uint16_t TargetMask1;
  uint16_t TargetMask2;
  uint16_t TargetMask3;
  uint16_t TargetMask4;
  uint16_t TargetMask5;
  uint16_t TargetMask6;
  uint16_t TargetMask7;
  uint16_t TargetMask8;
  uint16_t TargetMask9;

  uint32_t SleepTime;
};

class MenloSensorProtocol {

public:
  
  MenloSensorProtocol();

  //
  // General Contract:
  //
  // Caller is responsible to ensure buffer* is large enough
  // for the packet.
  //
  // Returns: Number of bytes placed into buffer
  //

  //
  // Register a sensor with the gateway
  //
  // Note: SerialNumber (64 bit) and ModelNumber (32 bit)
  // are passed as byte arrays since they typically are read from
  // ROM, and embedded C/C++ compilers generate large amounts of
  // code to translate two and from these larger format data
  // types. This pushes the marshalling of the 32 and 64 bit
  // types out to the client application which is specific
  // to a targeted microcontroller and its compiler.
  //
  void Register(
      uint8_t* buffer,
      uint8_t* serialNumber,
      uint8_t* modelNumber,
      uint8_t options
      );

  //
  // Unmarshall Register packet
  //
  // Returns 0 on error.
  // Returns the number of bytes used in the buffer on success.
  //
  int RegisterUnmarshall(
      uint8_t* buffer,
      uint8_t* serialNumber,
      uint8_t* modelNumber,
      uint8_t* options
      );

  //
  // Send register response from gateway to the sensor
  //
  void RegisterResponse(
      uint8_t* buffer,
      uint8_t unitID,
      uint8_t* serialNumber,
      uint8_t* modelNumber,
      uint8_t options
      );

  //
  // Unmarshall Register Response packet
  //
  // This takes the original expected serialNumber, ModelNumber,
  // and options bytes in order to determine if this packet
  // is for this sensor unit. This avoids allocating another set
  // of buffers for these values.
  //
  // Returns 0 on error.
  // Returns the number of bytes used in the buffer on success.
  //
  int RegisterResponseUnmarshall(
      uint8_t* buffer,
      uint8_t* unitID,
      uint8_t* expectedSerialNumber,
      uint8_t* expectedModelNumber,
      uint8_t expectedOptions
      );

  //
  // Large 32 byte packet with extended sensor
  // information.
  //
  void SensorToGateway(
      uint8_t* buffer,
      MenloSensorProtocolDataAppBuffer* output
      );

  //
  // Large 32 byte packet with extended sensor
  // information.
  //
  int
  SensorToGatewayUnmarshall(
      uint8_t* buffer,
      MenloSensorProtocolDataAppBuffer* output
      );

  //
  // Sleeptime is the amount of time the sensor unit will
  // sleep till its next check in.
  //
  // OutputMask defines software controlled output bits.
  //
  // MoreData if != 0 means the gateway has more data for the sensor
  // and it should send a SensorPoll() to the gateway to get the
  // next command/update from the gateway.
  //
  // Returns: Number of bytes placed into buffer
  //
  void SensorDataResponse(
      uint8_t* buffer,
      MenloSensorProtocolDataAppResponseBuffer* data
      );

  //
  // Unmarshall the sensor response packet.
  //
  // Returns 0 on error.
  // Returns the number of bytes used in the buffer on success.
  //
  int SensorDataResponseUnmarshall(
      uint8_t* buffer,
      uint8_t* unitID,
      unsigned long* sleeptime,
      unsigned short* targetMask0,
      unsigned short* targetMask1,
      unsigned short* targetMask2,
      unsigned short* targetMask3
      );

  //
  // Wakeup the sensor unit (gateway to sensor)
  //
  // Only works for client sensor units that have a low
  // power receive, or carrier detect wakeup.
  //
  // Sensor responds by issuing a SensorToGatway() update
  // packet.
  //
  // Note: This implies MoreData, and always sets the
  // continue bit in the packet.
  //
  // Returns: Number of bytes placed into buffer
  //
  void SensorWakeup(
      uint8_t* buffer,
      uint8_t unitID
      );

  //
  // Unmarshall the sensor wakeup packet.
  //
  // Returns 0 on error.
  // Returns the number of bytes used in the buffer on success.
  //
  // MoreData is implied when this packet is received.
  //
  int SensorWakeupUnmarshall(
      uint8_t* buffer,
      uint8_t* unitID
      );

  //
  // Request any commands/updates from the gateway by
  // the sensor.
  //
  // The response can be any valid gateway -> sensor
  // packet.
  //
  // This is usually sent in response to moreData being
  // set on a previous gateway response packet to the
  // sensor.
  //
  // A sensor should continue to issue sensor polls until
  // moreData is no longer set on return packets from the
  // gateway.
  //
  void SensorPoll(
      uint8_t* buffer,
      uint8_t unitID
      );

  int SensorPollUnmarshall(
      uint8_t* buffer,
      uint8_t* unitID
      );

private:
};

#endif // MenloSensorProtocol_h
