
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
 *  File: MenloSensorApp.cpp
 */

/*
 * Main Menlo Park Innovation Sensor Application.
 */

//
// 07/07/2014
//
// See MenloSensor.ino for smartpux.com URL's
//
// Range Test:
//
// // enable range test
//   targetmask3 == 0x8000
//
// Light is Red while waiting for response during test
//
// Light is Blue if a response is received, but value in targetmask2
// is not what it expected.
//
//   - Blue for same targetmask2 from gateway as is stored locally in m_targetmask2
//
// Light is Green if correct response is received
//
// If not doing a range test the LED state is:
//  (from the bottom of this file:)
//
//    // Not doing a range test, so program the led state from the
//    // lower mask bits
//    SetLedState(
//        m_targetMask1 & 0xFF,        // red
//        (m_targetMask1 >> 8) & 0xFF, // green
//        m_targetMask2 & 0xFF         // blue
//        );
//
// MenloGatewayApp.h, line 69
// #define SENSOR_MASK3_SERIALIZE_WITH_CLOUD 0x4000
//

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#include "MenloPlatform.h"

//
// Include Menlo Debug library support
//
#include <MenloDebug.h>

#include <MenloUtility.h>

// This libraries header
#include <MenloSensorApp.h>

//
// Allows turning on/off deep tracing when debugging.
//
// Memory and code space is typically tight, so only enable tracing
// in modules being debugged.
//
// This is per module, to allow selective enabling/disabling
// and not in a global header.
//
// On Arduino, F() places the text in code space, a big savings
// over placing the strings into the 2k RAM space. Even when placed
// in ram, the string is still in code space and transferred during
// C runtime init due to the harvard architecture's separate I+D spaces.
//
// These macro's also allow porting readily to Linux, Mac, RaspberryPi, etc.
//
#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define DBG_PRINT_INT(x) (MenloDebug::PrintHex())
#define DBG_PRINTHEX2(x, y) (MenloDebug::Print(F(x), y))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINTHEX2(x, y)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define xDBG_PRINT_INT(x) (MenloDebug::PrintHex())
#define xDBG_PRINTHEX2(x, y) (MenloDebug::Print(F(x), y))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINTHEX2(x, y)
#endif

// Constructor
MenloSensorApp::MenloSensorApp(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin) : 

  // Initialize the sensors embedded class statically
  m_sensors(
    sensorPowerEnablePin,
    temperaturePin,
    moistureInputPin,
    lightInputPin)
{
  m_unitID = 0xFF;
}

MenloSensorApp::MenloSensorApp(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin,
        uint8_t redLedPin,
        uint8_t greenLedPin,
        uint8_t blueLedPin) : 

  // Initialize the sensors embedded class statically
  m_sensors(
    sensorPowerEnablePin,
    temperaturePin,
    moistureInputPin,
    lightInputPin,
    redLedPin,
    greenLedPin,
    blueLedPin)
{
  m_unitID = 0xFF;
}

int
MenloSensorApp::Initialize(
    MenloRadio *radio 
    )
{
    return Initialize(radio, NULL);
}

int
MenloSensorApp::Initialize(
    MenloRadio *radio,
    MenloTemperature* temperature
    )
{
   m_goodGatewayCount = 0;
   m_goodCloudCount = 0;
   m_badUpdateCount = 0;
   m_targetMask0 = 0;
   m_targetMask1 = 0;
   m_targetMask2 = 0;
   m_targetMask3 = 0;
   m_moreData = 0;
   m_gatewayAddress = NULL;

   // Set the default sleep time until its updated by the gateway
   m_sleepTime = DEFAULT_SLEEP_TIME;

   m_radioTimeoutTime = DEFAULT_RADIO_TIMEOUT_TIME;

   m_maxRandomWait = MAXIMUM_RANDOM_WAIT;

   m_maxRetries = MAXIMUM_SEND_RETRIES;

   m_radio = radio;

  // Initialize the sensors
  return m_sensors.Initialize(temperature);
}

//
// Main state machine:
//
// The main state machine is for the sensor to send updates of
// its various sensor readings to the gateway, and then sleep
// for the amount of time in its response. This allows the
// gateway to control the update interval, and sleeping allows
// low average power operation and saving of the batteries.
//
// This is controlled by m_sleepTime.
//
// In any exchange with the gateway, the moreData flag can
// be returned in which the gateway indicates that the
// sensor should contact the gateway immediately for additional
// updates/actions before sleeping again. It should send
// SensorPoll requests to the gateway and operate on the return
// results until one returns with moreData no longer set. It
// can then continue with the last configured sleep interval
// returned from the gateway.
//
// The moreData state is cached in the sensor in case communication
// errors occur in attempting to get updated information from
// the gateway. This state will not clear until a successful
// exchange with the gateway returns moreData == 0. When moreData
// state is active, the main processing loop will continue to attempt
// to get updates from the gateway.
//
// When in communication recovery, the sleep time can be expected
// to be shorted in order to get the sensor back in communication
// with the gateway. This is actually handled by the gateway/cloud
// application service in returning an appropriately shorter
// sleepTime to the sensor when moreData is set. This prevents
// having to implement this additional logic in the sensor, which
// not only costs time and code space, but prevents "baking in"
// a hard coded recovery time.
//
// In effect the state machine makes the sensor a slave to the
// gateway allowing the gateway to control its main processing loop
// in regards to the sensors sleep time and responsiveness. On
// sensors which have no ability to listen on the radio
// in a low power state, the sleep time determines the tradeoff
// between responsiveness and battery consumption. This is configured
// by the gateway/cloud online applications according to the deployment
// of the sensors.
//
// Sensor models which have a lower power radio listen state would
// have modified sleep times as appropriate, and there is a provision
// for a wakeup packet from the gateway to the sensor to indicate
// moreData state handling in the normal fashion.
//

//
// This is called regularly from the embedded sensors main
// processing dispatch/poll() loop.
//

int
MenloSensorApp::AppProcess()
{
  int temperatureValue;
  int moistureValue;
  int lightValue;

  int retVal;

  //
  // If we have moreData set from a previous interval, process
  // this first as it has priorty.
  //
  // Typically this would only be set on entry to AppProcess
  // due to communication errors on the channel in which our
  // sleep interval was used before retrying communication.
  //
  // This avoids wasting energy and channel flooding.
  //

  // Note: This could fail, but will will still attempt to send
  // updated sensor readings anyway. If communication has been
  // re-established, the next check for moreData will continue
  // this processing.
  CheckForAndProcessMoreData();

  // Get the sensor readings to send to the gateway.
  m_sensors.GetSensorReadings(&temperatureValue, &moistureValue, &lightValue);

  //
  // Print the values for debugging
  //
  ResetWatchdog();

  xDBG_PRINT("Attempting to send sensor readings:");

  xDBG_PRINTHEX2("Sensor0: lightValue 0x", lightValue);
  xDBG_PRINTHEX2("Sensor0: lightValue 0x", lightValue);
  xDBG_PRINTHEX2("Sensor1: temperature 0x", temperatureValue);
  xDBG_PRINTHEX2("Sensor2: moistureValue 0x", moistureValue);
  xDBG_PRINTHEX2("Sensor7: goodCloudCount ", m_goodCloudCount);
  xDBG_PRINTHEX2("Sensor8: goodGatewayCount ", m_goodGatewayCount);

  ResetWatchdog();

  xDBG_PRINTHEX2("Sensor9: badUpdateCount ", m_badUpdateCount);
  xDBG_PRINTHEX2("m_targetMask0:  0x", m_targetMask0);
  xDBG_PRINTHEX2("m_targetMask1:  0x", m_targetMask1);
  xDBG_PRINTHEX2("m_targetMask2:  0x", m_targetMask2);
  xDBG_PRINTHEX2("m_targetMask3:  0x", m_targetMask3);

  ResetWatchdog();

  //
  // If the range test is on, we indicate the status of communication
  // with the gateway by turning on and changing the LED's color.
  //
  if (m_targetMask3 & SENSOR_MASK3_RANGE_TEST) {

    //
    // Set the LED to red when we attempt to contact the gateway
    // unit with updated sensor readings.
    //
    SetLedState(1, 0, 0); // Red
  }

  //
  // Format sensor readings
  //
  m_sensorData.UnitID = m_unitID;

  m_sensorData.Sensor0 = (unsigned short)lightValue;
  m_sensorData.Sensor1 = (unsigned short)temperatureValue;
  m_sensorData.Sensor2 = (unsigned short)moistureValue;
  m_sensorData.Sensor3 = (unsigned short)0; // humidity

  // This is currently un-assigned
  m_sensorData.Sensor4 = 0xFFFF;

  m_sensorData.Sensor5 = (unsigned short)0; // solar intensity
  m_sensorData.Sensor6 = (unsigned short)0; // battery

  m_sensorData.Sensor7 = (unsigned short)m_goodCloudCount;
  m_sensorData.Sensor8 = (unsigned short)m_goodGatewayCount;
  m_sensorData.Sensor9 = (unsigned short)m_badUpdateCount;

  // Send our current operating mask state
  m_sensorData.Mask0 = m_targetMask0;
  m_sensorData.Mask1 = m_targetMask1;
  m_sensorData.Mask2 = m_targetMask2;
  m_sensorData.Mask3 = m_targetMask3;

  //
  // This will retry if there is interference on the radio channel.
  //
  retVal = SendSensorReadings();
  if (retVal != 0) {
      m_badUpdateCount++;
  }

  //
  // The response to the SendSensorReadings could have set
  // moreData indicating the gateway has updates/actions for
  // the sensor.
  //
  CheckForAndProcessMoreData();

  // Perform our low power sleep based on the updated m_sleepTime.
  LowPowerSleep();

  //
  // We return to the caller to schedule other work
  // before we are returned to in order to process
  // any events that arrived during sleep.
  //

  return retVal;
}

//
// Sleep in a low power state for our configured interval.
//
void
MenloSensorApp::LowPowerSleep()
{
  //
  // m_sleepTime gets updated from the base unit and defines how
  // long we sleep until checking in again.
  //

  //
  // TODO: Make this a low power sleep!
  // - Turn off radio, ensure all radio routines power on before use.
  //   - Add this state to radio class OS_nRF24L01
  // - Execute AtMega328 low power sleep instruction based on our timer.
  // - Measure with multi-meter.
  //
  while(m_sleepTime > 0) {
      ResetWatchdog();
     
      delay(1000);

      if (m_sleepTime < 1000) {
	m_sleepTime = 0;
      }
      else {
	m_sleepTime -= 1000;
      }
  }
}

//
// This handles the moreData state.
//
int
MenloSensorApp::CheckForAndProcessMoreData()
{
  int retVal;

  //
  // The moreData state is set from a valid response from the
  // gateway in packet returned/sent to the sensor.
  //
  // This means the sensor needs to send a SensorPoll() command
  // to the gateway and operate on the the response until no more
  // responses include the moreData setting.
  //
  // This is the primary mechanism for the gateway to send
  // update information and commands to the sensor, as the
  // sensor operates in a poll/response relationship to
  // the gateway to save energy.
  //  
  // If there are communication errors in attempting to
  // send and receive a response to the SensorPoll command
  // due to radio channel interferance or otherwise, the
  // attempt is retried for a configurable number of times.
  //
  // If not successful, the m_moreData state is maintained
  // and a shorter sleep interval is selected (sleep recovery).
  //
  // Think of this as the ELF signal to a submarine saying
  // it needs to surface a communications buoy in order
  // to get important updates, such as new targeting
  // package. (Crimson Tide, 1995)
  //
#if notdefined
  while (m_moreData != 0) {
    retVal = PollGatewayAndExecuteCommands();

    if (retVal != 0) {

        //
        // We received an error attempting to contact the gateway.
        // We return with the m_moreData state still set. The
        // configuration variables control how long the sensor
        // will sleep before attempting the next retry in the
        // main application processing loop.
        //
        return retVal;
    }
  }
#endif
  return 0;
}

void
MenloSensorApp::SetLedState(
        uint8_t redValue,
        uint8_t greenValue,
        uint8_t blueValue
        )
{
  m_sensors.SetLedState(redValue, greenValue, blueValue);
}

void
MenloSensorApp::RegisterWithGateway(
    byte* gatewayAddress,
    uint8_t* serialNumber,
    uint8_t* modelNumber,
    uint8_t options,
    uint8_t* unitID	
    )
{
  int retVal;

  m_gatewayAddress = gatewayAddress;

  while(1) {

    //
    // We use the default sleep time as our timeout for
    // attempting to register the gateway.
    //
    retVal = TryRegisterWithGateway(
        m_radioTimeoutTime,
        serialNumber,
        modelNumber,
        options,
        unitID
        );

    if (retVal != 0) {

        // Display the error code
        MenloDebug::DisplayErrorCode(
            DEBUG_MAKE_ERROR_CODE(PANIC_CODE_SENSOR_APPLICATION, retVal));

        // Wait sleep time, try again
	MenloUtility::DelaySecondsWithWatchdog(m_sleepTime / 1000);

        //
        // We add a random element to the delay since
        // we could be in conflict with other sensor(s) trying
        // to register on a shared radio channel.
        //
        RandomWait(MAXIMUM_RANDOM_WAIT);
    }
    else {
        return;
    }
  }
  /* NOT REACHED */
}

//
// Random wait used when the radio channel is busy.
//
void
MenloSensorApp::RandomWait(long maxWait)
{
    long waitTime;

    randomSeed(millis());

    waitTime = random(maxWait);
    
    //
    // TODO: We could do this will a radio off and
    // micro controller sleep to save even more energy.
    //
    MenloUtility::DelaySecondsWithWatchdog(waitTime / 1000);
}

//
// Returns 0 on success
//
int
MenloSensorApp::TryRegisterWithGateway(
    unsigned long radioTimeout,
    uint8_t* serialNumber,
    uint8_t* modelNumber,
    uint8_t options,
    uint8_t* unitID	
    )
{
  uint8_t* buffer;
  int retVal;

  xDBG_PRINT("TryRegisterWithGateway...");

  buffer = m_radio->GetBuffer();

  *unitID = 0xFF;

  // Generate register command packet data
  m_protocol.Register(
       buffer,
       serialNumber,
       modelNumber,
       options
       );

  // Send it on the radio
  retVal = m_radio->Write(m_gatewayAddress, radioTimeout);
  if (retVal == 0) {
      xDBG_PRINTHEX2("Register: Timeout on radio send! timeout=", radioTimeout);
      return GATEWAY_FAILURE_REGISTER_SEND_TIMEOUT;
  }

  //
  // Now try and read the response
  //
  xDBG_PRINT("trying to read radio...");
  retVal = m_radio->Read(radioTimeout);
  if (retVal == 0) {
      xDBG_PRINTHEX2("Error reading register response packet timeout=", radioTimeout);
      return GATEWAY_FAILURE_REGISTER_NO_RESPONSE;
  }

  //
  // Attempt to unmarshal it.
  //
  // RegisterResponseUnmarshall validates that the received
  // packet matches our serialNumber, modelNumber, and options
  // bytes since we could be on a shared channel and received
  // a configuration response and unitID assignment intended
  // for another sensor unit.
  //
  // In this case we drop the packet and the caller will
  // retry.
  //
  retVal = m_protocol.RegisterResponseUnmarshall(
      buffer,
      unitID,
      serialNumber,
      modelNumber,
      options
      );

  if (retVal == 0) {
    xDBG_PRINTHEX2("Bad register response packet 0x", buffer[0]);
    return GATEWAY_FAILURE_REGISTER_BAD_RESPONSE;
  }

  m_unitID = *unitID;

  xDBG_PRINTHEX2("Registered with Gateway! Assigned UnitID=", m_unitID);

  return 0;
}

int
MenloSensorApp::SendSensorReadings()
{
    int retries;
    int retVal;

    for (retries = 0; retries < m_maxRetries; retries++) {
        retVal = TrySendSensorReadings();

        if (retVal == 0) return retVal;

        //
        // If range test is on, we don't execute the retry
        // loop except if its a bad response due to channel
        // interferance. This is so that we don't hide transient
        // errors on the channel when testing.
        //
        if ((m_targetMask3 & SENSOR_MASK3_RANGE_TEST) &&
            (retVal != GATEWAY_FAILURE_BAD_RESPONSE)) {
	    xDBG_PRINT("Send to sensor failure, range test, not retrying");
            return retVal;
        }

        // Wait a random amount of time in case of channel interference
        RandomWait(m_maxRandomWait);
    }

    return retVal;
}

//
// Returns 0 on success
//
int
MenloSensorApp::TrySendSensorReadings()
{
  uint8_t outputMask;
  unsigned long sleepTime;
  uint8_t* buffer;
  int retVal;

  buffer = m_radio->GetBuffer();

  // Generate SensorToGateway command packet data
  m_protocol.SensorToGateway(
       buffer,
       &m_sensorData
       );

  // Send it on the radio
  retVal = m_radio->Write(m_gatewayAddress, m_radioTimeoutTime);
  if (retVal == 0) {
      xDBG_PRINT("Sensor radio write data failure");
      return GATEWAY_FAILURE_SEND_TIMEOUT;
  }

  //
  // Now try and read the response
  //
  // On a shared channel radio we could get responses for other radios
  // which return as an unmarshall error. ReadSensorResponse will attempt
  // to continue receiving until the radio's timeout and drop the packets
  // not addressed to this sensor unit.
  //
  // On success, it will handle any processing required from the response
  // such as updating our sleep time and sensors output states.
  //
  retVal = ReadSensorResponse(buffer);

  return retVal;
}

//
// Read the sensor response packet for this sensors
// unitID. Retries until timeout.
//
// This routine expects and handles just a sensor response
// packet and is used after a SensorToGateway packet is sent.
//
int
MenloSensorApp::ReadSensorResponse(
    uint8_t* buffer
    )
{
  int retVal;

  //
  // We could be on a shared channel and receive packets addressed
  // for other sensors. So until our radio timeout time has passed
  // we will re-issue the read in attempting to get our packet.
  //
  // If this fails, the top level retry logic will re-send as
  // our packet may have been either overwritten on the air, or
  // dropped by the receiver at the gateway.
  //
  unsigned long startTime = millis();

  while (1) {

      ResetWatchdog();

      //
      // We may get multiple incorrect responses from the radio->Read() within
      // the total radio timeout time.
      //
      if ( ( millis() - startTime ) > m_radioTimeoutTime ) {
   	  xDBG_PRINT("ReadSensorResponse: Timeout waiting response from gateway");
          return GATEWAY_FAILURE_READ_TIMEOUT;
      }

      retVal = m_radio->Read(m_radioTimeoutTime);
      if (retVal == 0) {

	  //
          // If the radio read failed when given the maximum
          // timeout time without receiving even an incorrect
          // packet return right away.
          //
	  xDBG_PRINT("ReadSensorResponse: Timeout waiting for receive packet");
          return GATEWAY_FAILURE_READ_TIMEOUT;
      }

      // This will update m_moreData, m_sleepTime, m_targetMask's
      // on success.
      retVal = ProcessSensorResponse(buffer);
      if (retVal != 0) {
          // This is not a sensor response packet, or one matching our unitID,
          // so retry within the timeout interval.
	  xDBG_PRINT("ReadSensorResponse: Bad radio packet received, retrying");
          continue;
      }

      //
      // We received a good response addressed to this sensor,
      // sensor state variables have been updated.
      //

      return 0;
  }
  /* NOT REACHED */
}

//
// Send a SensorPoll command to the gateway, and dispatch
// and execute what comes back.
//
// Perform this action until no more responses from the gateway
// have moreData set.
//
// This is the main mechanism for the sensor to execute commands
// sent from the gateway to the sensor. It's usually called
// when moreData is returned in a previous packet exchange
// with the gateway.
//
int
MenloSensorApp::PollGatewayAndExecuteCommands()
{
    int retries;
    int retVal;

    for (retries = 0; retries < m_maxRetries; retries++) {
        retVal = TryPollGatewayAndExecuteCommands();

        if (retVal == 0) return retVal;

        // Wait a random amount of time in case of channel interferance
        RandomWait(m_maxRandomWait);
    }

    // This will return the last error code seen if failure.
    return retVal;
}

int
MenloSensorApp::TryPollGatewayAndExecuteCommands()
{
  uint8_t* buffer;
  int retVal;

  ResetWatchdog();

  buffer = m_radio->GetBuffer();

  // Generate SensorPoll command packet data
  m_protocol.SensorPoll(
       buffer,
       m_unitID
       );

  // Send it on the radio
  retVal = m_radio->Write(m_gatewayAddress, m_radioTimeoutTime);
  if (retVal == 0) {
      xDBG_PRINT("PollSensors: Error sending on sensor radio");
      return GATEWAY_FAILURE_SEND_TIMEOUT;
  }

  // Attempt to read the response, and perform any actions specified.
  retVal = ReadSensorPollResponseAndExecute(buffer);

  return retVal;
}

//
// Operate on the response packet from the gateway after
// a SensorPoll() command.
//
// There is expected to be one packet per SensorPoll()
// command to operate on.
//
// If additional data/commands are available at the
// gateway for this sensor, then the moreData bit
// will be set.
//
int
MenloSensorApp::ReadSensorPollResponseAndExecute(
    uint8_t* buffer
    )
{
  int retVal;
  uint8_t outputMask;
  uint8_t moreData;
  unsigned long sleepTime;

  //
  // We could be on a shared channel and receive packets addressed
  // for other sensors. So until our radio timeout time has passed
  // we will re-issue the read in attempting to get our packet.
  //
  // If this fails, the top level retry logic will re-send as
  // our packet may have been either overwritten on the air, or
  // dropped by the receiver at the gateway.
  //
  unsigned long startTime = millis();

  while (1) {

      ResetWatchdog();

      if ( ( millis() - startTime ) > m_radioTimeoutTime ) {
	  xDBG_PRINT("SensorPollReadResponse: waiting for sensor radio idle time");
          return GATEWAY_FAILURE_READ_TIMEOUT;
      }

      retVal = m_radio->Read(m_radioTimeoutTime);
      if (retVal == 0) {
	  xDBG_PRINT("SensorPollReadResponse: read sensor radio response timeout");
          // Return read failures right away
          return GATEWAY_FAILURE_READ_TIMEOUT;
      }

      moreData = 0;

      //
      // Look at the packet responses and determine what to do
      //
      // On bad packet responses we continue attempting to read
      // packets until the timeout since they may have been
      // addressed to other sensors on a shared channel.
      //
      switch(buffer[0]) {

      // These are the various commands.
      case MENLO_SENSOR_PROTOCOL_DATA_RESPONSE:

          //
          // Gateway will at least answer with this packet
          // to a SensorPoll() even if moreData is no
          // longer set.
          //
          retVal = ProcessSensorResponse(buffer);
          if (retVal != 0) continue;
          break;

      default:

          // Could be a response for another sensor
	  xDBG_PRINT("SensorPollReadResponse: unhandled response packet");
          continue;
      }

      //
      // If we get here, we have processed a valid
      // response from the gateway.
      // The returned packets handler function has updated
      // our sensors state such as m_moreData, m_sleepTime, etc.
      //
      // We only process one packet at a time from the gateway so
      // we return on the first successful one processed.
      //

      return 0;
  }
  /* NOT REACHED */
}

int
MenloSensorApp::ProcessSensorResponse(uint8_t* buffer)
{
  int retVal;
  uint8_t sensorID;
  unsigned long sleepTime;
  unsigned short targetMask0;
  unsigned short targetMask1;
  unsigned short targetMask2;
  unsigned short targetMask3;

  //
  // Attempt to unmarshal it
  //
  // TODO: Make this a pointer to struct.
  //
  // Less stack space passing it around, and can extend
  // the size easily.
  //

  retVal = m_protocol.SensorDataResponseUnmarshall(
      buffer,
      &sensorID,
      &sleepTime,
      &targetMask0,
      &targetMask1,
      &targetMask2,
      &targetMask3
      );

  // This is not a MENLO_SENSOR_PROTOCOL_DATA_RESPONSE packet
  if (retVal == 0) {
      // NOTE: a response of 0x2 is a duplicate of a GatwayRegister response packet.
      if (buffer[0] == MENLO_SENSOR_PROTOCOL_REGISTER_RESPONSE) {
          xDBG_PRINTHEX2("received MENLO_SENSOR_PROTOCOL_RESPONSE for unitID=", buffer[1]);
      }
      else {
          xDBG_PRINTHEX2("ProcessSensorResponse: wrong packet type SB 0x05, is 0x", buffer[0]);
      }
      return GATEWAY_FAILURE_BAD_RESPONSE;
  }

  if (sensorID != m_unitID) {
      MenloDebug::PrintNoNewline(F("ProcessSensorResponse: wrong unitID SB "));
      MenloDebug::PrintHex(m_unitID);
      MenloDebug::PrintNoNewline(F(" is "));
      MenloDebug::PrintHex(sensorID);
      MenloDebug::Print(F(""));

      MenloDebug::Print(F("TODO: FIX GATEWAY!"));

      // Help diagnose transport, addressing issues
      // TODO: Uncomment this when gateway is fixed!
      //return GATEWAY_FAILURE_NOT_MY_ID;
  }

  //
  // We received a good response addressed to this sensor
  //

  //
  // TODO: Application dispatcher should detect changes in new
  // target masks and sleep time and perform operations/callbacks.
  //
  // That way we don't glitch outputs processing the mask everytime
  // received.
  //
  // TODO: Include code to validate values and not respond to
  // out of range ones.
  //

  // Sleeptime is in seconds from gateway. We must
  // translate to millis.
  sleepTime = sleepTime * 1000L;

  if (sleepTime >= MINIMUM_SLEEP_TIME) {

      MenloDebug::PrintNoNewline(F("ProcessSensorResponse: new sleepTime "));
      MenloDebug::PrintHex(sleepTime);
      MenloDebug::Print(F(""));

      // Update our sleep time
      m_sleepTime = sleepTime;
  }
  else {
      MenloDebug::PrintNoNewline(F("ProcessSensorResponse: sleepTime does not meet minimum "));
      MenloDebug::PrintHex(sleepTime);
      MenloDebug::Print(F(""));
  }

  m_targetMask0 = targetMask0;
  m_targetMask1 = targetMask1;

  m_targetMask3 = targetMask3;

  m_goodGatewayCount++;

  //
  // Process any application state(s)
  //
  if (m_targetMask3 & SENSOR_MASK3_RANGE_TEST) {

      SetLedState(0, 1, 0); // Green

      m_targetMask2 = targetMask2;
  }
  else {
    m_targetMask2 = targetMask2;

    ResetWatchdog();

    xDBG_PRINTHEX2("Setting LED targetMask1 to ", m_targetMask1);
    xDBG_PRINTHEX2("Setting LED targetMask2 to ", m_targetMask2);

    // Not doing a range test, so program the led state from the
    // lower mask bits
    SetLedState(
        m_targetMask1 & 0xFF,        // red
        (m_targetMask1 >> 8) & 0xFF, // green
        m_targetMask2 & 0xFF         // blue
        );
  }

  return 0;
}
