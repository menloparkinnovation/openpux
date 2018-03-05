
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
 *  File: MenloSensorApp.h
 */

/*
 * Main Menlo Park Innovation Sensor Application.
 */

#ifndef MenloSensorApp_h
#define MenloSensorApp_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

// Include Menlo components
#include <MenloPlatform.h>
#include <MenloRadio.h>
#include <MenloSensorProtocol.h>
#include <MenloSensors.h>

//
// Example Sensor Application: Range Test
//
// Uses:
//
// TargetMask3 bit 15 - If on, range test mode enabled.
//
// TargetMask2 all bits - Count of commands sent from the Cloud
// as a roll over.
//
// Sensor8 - good packets to gateway count
//
// Sensor9 - bad packets to gateway count (timeout, not replied to)
//
// The sensor is placed into range test mode when the range
// test command bit is set in bit 15 of Sensor targetMask3.
// 
// In range test mode, the LED is turned Red before an update
// is sent to the gateway. If a reply is received from the
// gateway, the LED is turned Blue, and the good packets
// count is incremented. If no reply from the gateway is
// received, the bad packets count is incremented.
// 
// Sensor targetMask2 contains a count from the Cloud server that
// indicates the number of packets sent. If this value differs
// from a previous reading, the LED is set to Green to indicate
// commands are being received from the Cloud server through
// the gateway.
//
// The sensor continues to perform its other normal functions
// while in range test mode, except that the LED is overridden.
//
#define SENSOR_MASK3_RANGE_TEST 0x8000

//
// TODO: Implement an example of a "latch state" in a sensor
// and a specific command required from the Cloud to clear it.
//
// This can simulate an example of a burgler alarm in which
// a sensor is triggered, and only positive command action
// from the Cloud server will clear the condition.
//
// It takes into account that end to end data can get
// dropped.
//
// May have to use latch state to "push" packets beyond
// queue policy.
//
// Pattern:
//
//  The sensor latches an event and this is indicated in
//  a targetMask or Sensorx reading.
//
//  A targetMask "clear latch state bit" command is sent
//  to specifically clear it.
//
// The Cloud application can look at updated targetMask/Sensorx
// state to see when the state actually clears, and retries
// until it has.
//
// Organize targetMasks:
//
// targetMask0 - Command and control
//
// TODO: Move range test mode to targetMask1
// targetMask1 - "latched states, Sensor states, Program states"
//
// targetMask2 - available for programs
//
// targetMask3 - available for programs
//

class MenloSensorApp {

public:
  
  MenloSensorApp(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin
        );

  MenloSensorApp(
        uint8_t sensorPowerEnablePin,
        uint8_t temperaturePin,
        uint8_t moistureInputPin,
        uint8_t lightInputPin,
        uint8_t redLedPin,
        uint8_t greenLedPin,
        uint8_t blueLedPin
        );

  int Initialize(
      MenloRadio *radio
      );

  int Initialize(
      MenloRadio *radio,
      MenloTemperature* temperature
      );

  // Register with the gateway and receive a unitID
  void RegisterWithGateway(
        byte* gatewayAddress,
        uint8_t* serialNumber,
	uint8_t* modelNumber,
	uint8_t options,
        uint8_t* unitID	
	);

  // This is called from the main dispatch event loop
  int AppProcess();

    //
    // A LedValue of 0 means off
    // 1 or greater is on.
    //
    // If PWM is available and on, then 1-100 encodes
    // a duty cycle.
    //
    void SetLedState(
        uint8_t redValue,
        uint8_t greenValue,
        uint8_t blueValue
        );

private:

  int TryRegisterWithGateway(
        unsigned long timeout,
        uint8_t* serialNumber,
	uint8_t* modelNumber,
	uint8_t options,
        uint8_t* unitID	
	);

  int
  SendSensorReadings();

  int
  TrySendSensorReadings();

  void LowPowerSleep();

  int ProcessSensorResponse(uint8_t* buffer);

  int CheckForAndProcessMoreData();

  int PollGatewayAndExecuteCommands();

  int TryPollGatewayAndExecuteCommands();

  int ReadSensorPollResponseAndExecute(uint8_t* buffer);

  int ReadSensorResponse(uint8_t* buffer);

  void RandomWait(long maxWait);

  MenloRadio* m_radio;
  MenloSensorProtocol m_protocol;
  MenloSensors m_sensors;

  //
  // These variables change according to updates
  // from the gateway when the sensor is in contact
  // with it after registration.
  //

  // Our gateway assigned unit ID
  uint8_t m_unitID;

  //
  // Our current sleep time returned from the gateway.
  // This configures the sensors energy/response profile.
  //
  unsigned long m_sleepTime;

  //
  // If this is true, the gateway has given us an
  // indication of more data/commands available, so
  // we try and get these commands until moreData
  // returns 0. Since we may receive errors, this
  // is cached here and impacts recovery/sleep
  // timers.
  //
  uint8_t m_moreData;

  // Options from the gateway
  uint8_t m_options;

  uint8_t m_spare0;

  //
  // These are the target masks we last received
  // from the gateway.
  //
  uint16_t m_targetMask0;
  uint16_t m_targetMask1;
  uint16_t m_targetMask2;

  // Target Mask3 enables range test, and range test toggle
  uint16_t m_targetMask3;

  // This the main sensor data we will send to the gateway
  MenloSensorProtocolDataAppBuffer m_sensorData;

  //
  // These member variables are controlled by
  // per sensor configuration.
  //

  // Our gateway address
  // Note: This buffer is not copied when passed from the main application.
  byte* m_gatewayAddress;

  //
  // This is the amount of time we wait for the radio
  // to send/receive data before we timeout.
  //
  // TODO: Make this updateable from the gateway.
  //
  unsigned long m_radioTimeoutTime;

  //
  // Max random wait time for communication errors
  // to avoid flooding the radio channel.
  //
  // TODO: Make this updateable from the gateway.
  //
  unsigned long m_maxRandomWait;

  //
  // This is the maximum number retries when there
  // is interferance on the radio channel.
  //
  // TODO: Make this updateable from the gateway.
  //
  uint8_t m_maxRetries;

  //
  // Range test mode/application
  //  
  unsigned short m_goodGatewayCount;
  unsigned short m_goodCloudCount;
  unsigned short m_badUpdateCount;
};

//
// Default Sleep Time
//
#define DEFAULT_SLEEP_TIME (15 * 1000) // 15 seconds

//
// Minimum Sleep time so a sensor does not go into a tight
// loop
//

#define MINIMUM_SLEEP_TIME (10 * 1000) // 10 seconds

//
// Timeout when conducting radio transactions
//
// Low end gateways can be stuck doing a slow internet
// exchange over WiFi, Cell modem, etc., and are
// single tasking.
//
// TODO: Make this settable based on gateway capabilities.
//
#define DEFAULT_RADIO_TIMEOUT_TIME (10 * 1000) // 15 seconds

//
// Maximum wait time for random wait when there is
// radio channel interferance.
//
#define MAXIMUM_RANDOM_WAIT (10 * 1000) // 10 seconds

//
// Maximum number of retries when attempting to
// contact gateway before going to sleep again.
//
#define MAXIMUM_SEND_RETRIES 5

//
// Panic Codes
//
// These represent lower numbers of two digit packed BCD codes to make it
// easy to flash on a status led.
//
// See MenloDebug.h for more details.
//
// PANIC_CODE_SENSOR_APPLICATION
//
// Gateway communication failures
#define GATEWAY_FAILURE_REGISTER_NO_RESPONSE  0x01
#define GATEWAY_FAILURE_REGISTER_BAD_RESPONSE 0x02
#define GATEWAY_FAILURE_REGISTER_SEND_TIMEOUT 0x03
#define GATEWAY_FAILURE_BAD_RESPONSE          0x04
#define GATEWAY_FAILURE_SEND_TIMEOUT          0x05
#define GATEWAY_FAILURE_READ_TIMEOUT          0x06
#define GATEWAY_FAILURE_NOT_MY_ID             0x07

#endif // MenloSensorApp_h
