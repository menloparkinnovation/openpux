
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
 *  Date: 11/21/2012
 *  File: MenloSensorProtocol.cpp
 */

/*
 * This class handles protocol generation and formatting
 * for communicating between sensors and the gateway unit.
 */

/*
 * Design of the Menlo Sensor Protocol
 *
 * The Menlo Sensor Protocol supports the exchange of short
 * binary messages between a sensor and the base/gateway unit.
 *
 * It's built on framed, short binary messages.
 *
 * The reason for short binary messages are:
 *
 *  Short messages are less likely to get lost in a noisy environment.
 *
 *  Short messages work with small memories, esp. with forwarders.
 *
 *  Short messages can fit within on board radio buffers and sent
 *  as high speed bursts saving total energy.
 *
 * It's independent of the transport, but if a transport does
 * not natively support framing of messages, then it needs to
 * supply framing at the link interface layer software. For
 * example, the XBee over transparent serial does this.
 *
 * In addition, if its not a binary interface, or certain
 * characters are special, its up to the link layer software
 * to handle this. For example the XBee over transparent serial
 * uses AsciiHex coding.
 *
 * Any higher level application protocol may be developed
 * on top transparently. For example configuration, gateways,
 * Zigbee or Mote style forwarding networks, etc.
 *
 */

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

//
// Include Menlo Debug library support
//
#include <MenloPlatform.h>
#include <MenloDebug.h>

// This libraries header
#include <MenloSensorProtocol.h>

// Constructor
MenloSensorProtocol::MenloSensorProtocol() {
}

//
// In all cases the suppied buffer represents the radios
// transport buffer which is only valid during the call
// to marshall/unmarshall.
//

void
MenloSensorProtocol::Register(
      uint8_t* buffer,
      uint8_t* serialNumber,
      uint8_t* modelNumber,
      uint8_t options
      )
{
  struct MenloSensorProtocolRegister* p;

  p = (MenloSensorProtocolRegister*)buffer;

  p->Type =  MENLO_SENSOR_PROTOCOL_REGISTER;
  
  // UnitID is always 0 for the register packet
  p->UnitID = 0;

  p->SerialNumberByte0 = serialNumber[0];
  p->SerialNumberByte1 = serialNumber[1];
  p->SerialNumberByte2 = serialNumber[2];
  p->SerialNumberByte3 = serialNumber[3];
  p->SerialNumberByte4 = serialNumber[4];
  p->SerialNumberByte5 = serialNumber[5];
  p->SerialNumberByte6 = serialNumber[6];
  p->SerialNumberByte7 = serialNumber[7];

  p->ModelNumberByte0 = modelNumber[0];
  p->ModelNumberByte1 = modelNumber[1];
  p->ModelNumberByte2 = modelNumber[2];
  p->ModelNumberByte3 = modelNumber[3];

  return;
}

int
MenloSensorProtocol::RegisterUnmarshall(
      uint8_t* buffer,
      uint8_t* serialNumber,
      uint8_t* modelNumber,
      uint8_t* options
      )
{
  struct MenloSensorProtocolRegister* p;

  p = (MenloSensorProtocolRegister*)buffer;

  if (p->Type != MENLO_SENSOR_PROTOCOL_REGISTER) {
    return 0;
  }
  
  // Unit ID is always 0 for register.
  if (p->UnitID != 0) {
    return 0;
  }

  serialNumber[0] = p->SerialNumberByte0;
  serialNumber[1] = p->SerialNumberByte1;
  serialNumber[2] = p->SerialNumberByte2;
  serialNumber[3] = p->SerialNumberByte3;
  serialNumber[4] = p->SerialNumberByte4;
  serialNumber[5] = p->SerialNumberByte5;
  serialNumber[6] = p->SerialNumberByte6;
  serialNumber[7] = p->SerialNumberByte7;

  modelNumber[0] = p->ModelNumberByte0;
  modelNumber[1] = p->ModelNumberByte1;
  modelNumber[2] = p->ModelNumberByte2;
  modelNumber[3] = p->ModelNumberByte3;

  return MENLO_SENSOR_PROTOCOL_REGISTER_SIZE;
}

void
MenloSensorProtocol::RegisterResponse(
      uint8_t* buffer,
      uint8_t unitID,
      uint8_t* serialNumber,
      uint8_t* modelNumber,
      uint8_t options
      )
{
  struct MenloSensorProtocolRegisterResponse* p;

  p = (MenloSensorProtocolRegisterResponse*)buffer;

  p->Type = MENLO_SENSOR_PROTOCOL_REGISTER_RESPONSE;
  
  p->UnitID = unitID;

  p->SerialNumberByte0 = serialNumber[0];
  p->SerialNumberByte1 = serialNumber[1];
  p->SerialNumberByte2 = serialNumber[2];
  p->SerialNumberByte3 = serialNumber[3];
  p->SerialNumberByte4 = serialNumber[4];
  p->SerialNumberByte5 = serialNumber[5];
  p->SerialNumberByte6 = serialNumber[6];
  p->SerialNumberByte7 = serialNumber[7];

  p->ModelNumberByte0 = modelNumber[0];
  p->ModelNumberByte1 = modelNumber[1];
  p->ModelNumberByte2 = modelNumber[2];
  p->ModelNumberByte3 = modelNumber[3];

  return;
}

int
MenloSensorProtocol::RegisterResponseUnmarshall(
      uint8_t* buffer,
      uint8_t* unitID,
      uint8_t* expectedSerialNumber,
      uint8_t* expectedModelNumber,
      uint8_t expectedOptions
      )
{
  struct MenloSensorProtocolRegisterResponse* p;

  p = (MenloSensorProtocolRegisterResponse*)buffer;

  if (p->Type != MENLO_SENSOR_PROTOCOL_REGISTER_RESPONSE) {
    return 0;
  }
  
  //
  // TODO: It's better for the higher level logic to determine
  // matches for serial numbers, etc.
  //

  // Compare the serial number against expected. If not, its an error.
  if (expectedSerialNumber[0] != p->SerialNumberByte0) return 0;
  if (expectedSerialNumber[1] != p->SerialNumberByte1) return 0;
  if (expectedSerialNumber[2] != p->SerialNumberByte2) return 0;
  if (expectedSerialNumber[3] != p->SerialNumberByte3) return 0;
  if (expectedSerialNumber[4] != p->SerialNumberByte4) return 0;
  if (expectedSerialNumber[5] != p->SerialNumberByte5) return 0;
  if (expectedSerialNumber[6] != p->SerialNumberByte6) return 0;
  if (expectedSerialNumber[7] != p->SerialNumberByte7) return 0;

  // Compare the model number against expected, if not, its an error
  if (expectedModelNumber[0] != p->ModelNumberByte0) return 0;
  if (expectedModelNumber[1] != p->ModelNumberByte1) return 0;
  if (expectedModelNumber[2] != p->ModelNumberByte2) return 0;
  if (expectedModelNumber[3] != p->ModelNumberByte3) return 0;

  // Return the units assigned ID
  *unitID = p->UnitID;

  return MENLO_SENSOR_PROTOCOL_REGISTER_RESPONSE_SIZE;
}

void
MenloSensorProtocol::SensorToGateway(
    uint8_t* buffer,
    MenloSensorProtocolDataAppBuffer* bf
    )
{
  struct MenloSensorProtocolData* p;

  p = (MenloSensorProtocolData*)buffer;

  p->Type = MENLO_SENSOR_PROTOCOL_SENSOR_DATA;
  p->UnitID = bf->UnitID;

  p->LsbSensor0 = bf->Sensor0 & 0x00FF;
  p->MsbSensor0 = (bf->Sensor0 >> 8) & 0x00FF;

  p->LsbSensor1 = bf->Sensor1 & 0x00FF;
  p->MsbSensor1 = (bf->Sensor1 >> 8) & 0x00FF;

  p->LsbSensor2 = bf->Sensor2 & 0x00FF;
  p->MsbSensor2 = (bf->Sensor2 >> 8) & 0x00FF;

  p->LsbSensor3 = bf->Sensor3 & 0x00FF;
  p->MsbSensor3 = (bf->Sensor3 >> 8) & 0x00FF;

  p->LsbSensor4 = bf->Sensor4 & 0x00FF;
  p->MsbSensor4 = (bf->Sensor4 >> 8) & 0x00FF;

  p->LsbSensor5 = bf->Sensor5 & 0x00FF;
  p->MsbSensor5 = (bf->Sensor5 >> 8) & 0x00FF;

  p->LsbSensor6 = bf->Sensor6 & 0x00FF;
  p->MsbSensor6 = (bf->Sensor6 >> 8) & 0x00FF;

  p->LsbSensor7 = bf->Sensor7 & 0x00FF;
  p->MsbSensor7 = (bf->Sensor7 >> 8) & 0x00FF;

  p->LsbSensor8 = bf->Sensor8 & 0x00FF;
  p->MsbSensor8 = (bf->Sensor8 >> 8) & 0x00FF;

  p->LsbSensor9 = bf->Sensor9 & 0x00FF;
  p->MsbSensor9 = (bf->Sensor9 >> 8) & 0x00FF;

  p->LsbMask0 = bf->Mask0 & 0x00FF;
  p->MsbMask0 = (bf->Mask0 >> 8) & 0x00FF;

  p->LsbMask1 = bf->Mask1 & 0x00FF;
  p->MsbMask1 = (bf->Mask1 >> 8) & 0x00FF;

  p->LsbMask2 = bf->Mask2 & 0x00FF;
  p->MsbMask2 = (bf->Mask2 >> 8) & 0x00FF;

  p->LsbMask3 = bf->Mask3 & 0x00FF;
  p->MsbMask3 = (bf->Mask3 >> 8) & 0x00FF;

  return;
}

int
MenloSensorProtocol::SensorToGatewayUnmarshall(
    uint8_t* buffer,
    MenloSensorProtocolDataAppBuffer* output
    )
{
  unsigned short tmp;
  struct MenloSensorProtocolData* p;

  p = (MenloSensorProtocolData*)buffer;

  if (p->Type != MENLO_SENSOR_PROTOCOL_SENSOR_DATA) {
      return 0;
  }

  output->Type = p->Type;
  output->UnitID = p->UnitID;


  tmp = p->LsbSensor0 & 0x00FF;
  tmp |= (p->MsbSensor0 << 8) & 0xFF00;
  output->Sensor0 = tmp;
  
  tmp = p->LsbSensor1 & 0x00FF;
  tmp |= (p->MsbSensor1 << 8) & 0xFF00;
  output->Sensor1 = tmp;

  tmp = p->LsbSensor2 & 0x00FF;
  tmp |= (p->MsbSensor2 << 8) & 0xFF00;
  output->Sensor2 = tmp;

  tmp = p->LsbSensor3 & 0x00FF;
  tmp |= (p->MsbSensor3 << 8) & 0xFF00;
  output->Sensor3 = tmp;

  tmp = p->LsbMask0 & 0x00FF;
  tmp |= (p->MsbMask0 << 8) & 0xFF00;
  output->Mask0 = tmp;

  tmp = p->LsbMask1 & 0x00FF;
  tmp |= (p->MsbMask1 << 8) & 0xFF00;
  output->Mask1 = tmp;

  //
  // Extra data from sensor
  //

  tmp = p->LsbSensor4 & 0x00FF;
  tmp |= (p->MsbSensor4 << 8) & 0xFF00;
  output->Sensor4 = tmp;

  tmp = p->LsbSensor5 & 0x00FF;
  tmp |= (p->MsbSensor5 << 8) & 0xFF00;
  output->Sensor5 = tmp;

  tmp = p->LsbSensor6 & 0x00FF;
  tmp |= (p->MsbSensor6 << 8) & 0xFF00;
  output->Sensor6 = tmp;

  tmp = p->LsbSensor7 & 0x00FF;
  tmp |= (p->MsbSensor7 << 8) & 0xFF00;
  output->Sensor7 = tmp;

  tmp = p->LsbSensor8 & 0x00FF;
  tmp |= (p->MsbSensor8 << 8) & 0xFF00;
  output->Sensor8 = tmp;

  tmp = p->LsbSensor9 & 0x00FF;
  tmp |= (p->MsbSensor9 << 8) & 0xFF00;
  output->Sensor9 = tmp;

  tmp = p->LsbMask2 & 0x00FF;
  tmp |= (p->MsbMask2 << 8) & 0xFF00;
  output->Mask2 = tmp;

  tmp = p->LsbMask3 & 0x00FF;
  tmp |= (p->MsbMask3 << 8) & 0xFF00;
  output->Mask3 = tmp;

  return MENLO_SENSOR_PROTOCOL_SENSOR_DATA_SIZE;
}

void
MenloSensorProtocol::SensorDataResponse(
    uint8_t* buffer,
    MenloSensorProtocolDataAppResponseBuffer* data
    )
{
  struct MenloSensorProtocolDataResponse* p;

  p = (MenloSensorProtocolDataResponse*)buffer;

  p->Type = MENLO_SENSOR_PROTOCOL_DATA_RESPONSE;
  p->UnitID = data->UnitID;

  p->SleepTime0 = data->SleepTime &   0x000000FF;
  p->SleepTime1 = (data->SleepTime >> 8) &  0x000000FF;
  p->SleepTime2 = (data->SleepTime >> 16) & 0x000000FF;
  p->SleepTime3 = (data->SleepTime >> 24) & 0x000000FF;

  p->LsbTargetMask0 = data->TargetMask0 & 0x00FF;
  p->MsbTargetMask0 = (data->TargetMask0 >> 8) & 0x00FF;

  p->LsbTargetMask1 = data->TargetMask1 & 0x00FF;
  p->MsbTargetMask1 = (data->TargetMask1 >> 8) & 0x00FF;

  p->LsbTargetMask2 = data->TargetMask2 & 0x00FF;
  p->MsbTargetMask2 = (data->TargetMask2 >> 8) & 0x00FF;

  p->LsbTargetMask3 = data->TargetMask3 & 0x00FF;
  p->MsbTargetMask3 = (data->TargetMask3 >> 8) & 0x00FF;

  p->LsbTargetMask4 = data->TargetMask4 & 0x00FF;
  p->MsbTargetMask4 = (data->TargetMask4 >> 8) & 0x00FF;

  p->LsbTargetMask5 = data->TargetMask5 & 0x00FF;
  p->MsbTargetMask5 = (data->TargetMask5 >> 8) & 0x00FF;

  p->LsbTargetMask6 = data->TargetMask6 & 0x00FF;
  p->MsbTargetMask6 = (data->TargetMask6 >> 8) & 0x00FF;

  p->LsbTargetMask7 = data->TargetMask7 & 0x00FF;
  p->MsbTargetMask7 = (data->TargetMask7 >> 8) & 0x00FF;

  p->LsbTargetMask8 = data->TargetMask8 & 0x00FF;
  p->MsbTargetMask8 = (data->TargetMask8 >> 8) & 0x00FF;

  p->LsbTargetMask9 = data->TargetMask9 & 0x00FF;
  p->MsbTargetMask9 = (data->TargetMask9 >> 8) & 0x00FF;

  return;
}

int
MenloSensorProtocol::SensorDataResponseUnmarshall(
    uint8_t* buffer,
    uint8_t* unitID,
    unsigned long* sleepTime,
    unsigned short* targetMask0,
    unsigned short* targetMask1,
    unsigned short* targetMask2,
    unsigned short* targetMask3
    )
{
  unsigned long tmp;
  unsigned short tmp2;
  struct MenloSensorProtocolDataResponse* p;

  p = (MenloSensorProtocolDataResponse*)buffer;

  if (p->Type != MENLO_SENSOR_PROTOCOL_DATA_RESPONSE) {
      return 0;
  }

  *unitID = p->UnitID;

  tmp = 0;

  tmp |= p->SleepTime0 & (unsigned long)0x000000FF;
  tmp |= (p->SleepTime1 << 8) & (unsigned long)0x0000FF00;
  tmp |= (p->SleepTime2 << 16) & (unsigned long)0x00FF0000;
  tmp |= (p->SleepTime3 << 24) & (unsigned long)0xFF000000;

  *sleepTime = tmp;

  tmp2 = p->LsbTargetMask0 & 0x00FF;
  tmp2 |= (p->MsbTargetMask0 << 8) & 0xFF00;
  *targetMask0 = tmp2;

  tmp2 = p->LsbTargetMask1 & 0x00FF;
  tmp2 |= (p->MsbTargetMask1 << 8) & 0xFF00;
  *targetMask1 = tmp2;

  tmp2 = p->LsbTargetMask2 & 0x00FF;
  tmp2 |= (p->MsbTargetMask2 << 8) & 0xFF00;
  *targetMask2 = tmp2;

  tmp2 = p->LsbTargetMask3 & 0x00FF;
  tmp2 |= (p->MsbTargetMask3 << 8) & 0xFF00;
  *targetMask3 = tmp2;

  return MENLO_SENSOR_PROTOCOL_DATA_RESPONSE_SIZE;
}

void
MenloSensorProtocol::SensorWakeup(
    uint8_t* buffer,
    uint8_t unitID
    )
{
  struct MenloSensorProtocolWakeup* p;

  p = (MenloSensorProtocolWakeup*)buffer;

  p->Type = MENLO_SENSOR_PROTOCOL_WAKEUP;  
  p->UnitID = unitID;

  return;
}

int
MenloSensorProtocol::SensorWakeupUnmarshall(
    uint8_t* buffer,
    uint8_t* unitID
    )
{
  struct MenloSensorProtocolWakeup* p;

  p = (MenloSensorProtocolWakeup*)buffer;

  if (p->Type != MENLO_SENSOR_PROTOCOL_WAKEUP) {
    return 0;
  }

  *unitID = p->UnitID;

  return MENLO_SENSOR_PROTOCOL_WAKEUP_SIZE;
}

void
MenloSensorProtocol::SensorPoll(
    uint8_t* buffer,
    uint8_t unitID
    )
{
  struct MenloSensorProtocolSensorPoll* p;

  p = (MenloSensorProtocolSensorPoll*)buffer;

  p->Type = MENLO_SENSOR_PROTOCOL_SENSOR_POLL;  
  p->UnitID = unitID;

  return;
}

int
MenloSensorProtocol::SensorPollUnmarshall(
    uint8_t* buffer,
    uint8_t* unitID
    )
{
  struct MenloSensorProtocolSensorPoll* p;

  p = (MenloSensorProtocolSensorPoll*)buffer;

  if (p->Type != MENLO_SENSOR_PROTOCOL_SENSOR_POLL) {
    return 0;
  }

  *unitID = p->UnitID;

  return MENLO_SENSOR_PROTOCOL_SENSOR_POLL_SIZE;
}
