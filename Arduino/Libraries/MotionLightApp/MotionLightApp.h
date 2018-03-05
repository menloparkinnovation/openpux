
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
 * Date: 05/26/2015
 * File: MotionLightApp.h
 *
 * Application class.
 *
 */

#ifndef MotionLightApp_h
#define MotionLightApp_h

//
// MenloPlatform support
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloDispatchObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloTimer.h>
#include <MenloPower.h>

//
// MenloDweet Support
//
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetApp.h>
#include <DweetSerialChannel.h>

// RadioSerial
#include <MenloRadioSerial.h>

// These mainly keep a mis-configured timer from running to often
#define MOTIONLIGHT_MINIMUM_ON_TIME (1000*10)
#define MOTIONLIGHT_MINIMUM_OFF_TIME (1000*10)

// Default Sensor Update Interval
#define MOTIONLIGHT_DEFAULT_SENSOR_INTERVAL (30*1000)

//
// Light Modes
//

//
// Normal mode flashes light on, goes steady, then off.
//
// New triggers while the light is on restarts the flash sequence.
//
// This has the effect of flashing the light for current/new motion.
//
// This is a good mode for general security situations in which you
// want to draw attention to the reason for the light trigger
// and further movement. This can be used to track someone moving
// around a property.
//
const uint8_t LightModeNormal = 0x00;

//
// Light remains steady on new triggers. This is used when the
// light is in convenience mode and flashing on new trigger
// events would be distracting/annoying.
//
const uint8_t LightModeSteady = 0x00;

//
// Light States
//

// Idle and off
const uint8_t StateIdle                  = 0x01;

const uint8_t StateTransitionToLightOn   = 0x02;

const uint8_t StateLightOn               = 0x03;

const uint8_t StateTransitionToLightOff  = 0x04;

const uint8_t StateLightOff              = 0x05;

const uint8_t StateTransitionToTriggered = 0x06;

const uint8_t StateLightOnTriggered      = 0x07;

const uint8_t StateTransitionToBlinkTriggered = 0x08;

const uint8_t StateBlinkTriggered        = 0x09;

//
// Configuration store entries store power on values
// in the EEPROM. They are automatically read and
// applied to the application objects properties
// at power on/reset.
//
// The values are checked summed to prevent incorrect
// device operation on a missing or damaged configuration.
//

// TOP_INDEX is defined in MenloConfigStore.h
#define APP_STORAGE_BASE_INDEX TOP_INDEX

#define MOTIONLIGHT_ONTIME_INDEX        APP_STORAGE_BASE_INDEX
#define MOTIONLIGHT_ONTIME_SIZE         9 // 00000000'\0'

#define MOTIONLIGHT_LIGHTMODE_INDEX     (MOTIONLIGHT_ONTIME_INDEX + MOTIONLIGHT_ONTIME_SIZE)
#define MOTIONLIGHT_LIGHTMODE_SIZE      3 // 00'\0'

#define MOTIONLIGHT_BLINKRATE_INDEX     (MOTIONLIGHT_LIGHTMODE_INDEX + MOTIONLIGHT_LIGHTMODE_SIZE)
#define MOTIONLIGHT_BLINKRATE_SIZE      9 // 00000000'\0'

#define MOTIONLIGHT_BLINKINTERVAL_INDEX (MOTIONLIGHT_BLINKRATE_INDEX + MOTIONLIGHT_BLINKRATE_SIZE)
#define MOTIONLIGHT_BLINKINTERVAL_SIZE  9 // 00000000'\0'

// Note: This must be last as the checksum range depends on it
#define MOTIONLIGHT_CHECKSUM   (MOTIONLIGHT_BLINKINTERVAL_INDEX + MOTIONLIGHT_BLINKINTERVAL_SIZE)
#define MOTIONLIGHT_CHECKSUM_SIZE 2

//
// These don't have EEPROM entries, but have parameter string sizes
//
#define MOTIONLIGHT_LIGHTON_SIZE        9 // 00000000'\0'

#define MOTIONLIGHT_LIGHTOFF_SIZE       9 // 00000000'\0'

// The first saved value is the start of the LIGHT checksum range
#define MOTIONLIGHT_CHECKSUM_BEGIN         MOTIONLIGHT_ONTIME_INDEX
#define MOTIONLIGHT_CHECKSUM_SIZE          2

// End of range for MOTIONLIGHT checksum is the start of the checksum storage location
#define MOTIONLIGHT_CHECKSUM_END  MOTIONLIGHT_CHECKSUM

#define MOTIONLIGHT_MAX_SIZE MOTIONLIGHT_BLINKRATE_SIZE

struct MotionLightConfiguration {

    // Default interval
    unsigned long defaultOnTime;

    // PinNumber for trigger pin
    int triggerPinNumber;

    // LightPin Number
    int lightPinNumber;
};

//
// The MotionSensorApp also can provide low overhead
// 32 byte radio packet updates.
//
// The basic packet definition comes from MenloRadioSerial
// so the radio channel can be shared with general management
// commands (Dweets).
//

//
// This is an overlay for MenloRadioSerialSensorData which
// is a general use application level radio packet form
//
struct MotionLightSensorData {

  // 2 bytes
  uint8_t type;
  uint8_t subType; // 0xFF for MotionLightSensorData

  uint8_t lightTriggered; // != 0 light is triggered
  uint8_t lightState;

  uint8_t triggerState;
  uint8_t pad0;

  //
  // Multibyte types are stored LSB first as on the AtMega series.
  // Other big endian processors need to reverse the bytes.
  //
  // (These have more code space to accomodate)
  //

  // Optional battery state and solar output level
  uint16_t battery;
  uint16_t solar;

  uint16_t pad1;
  uint16_t pad2;
  uint16_t pad3;
  uint16_t pad4;
  uint16_t pad5;
  uint16_t pad6;
  uint16_t pad7;
  uint16_t pad8;
  uint16_t pad9;
  uint16_t pad10;
  uint16_t pad11;
};

#define MOTIONLIGHT_SENSOR_DATA_SIZE sizeof(struct MotionLightSensorData)

#define MOTIONLIGHT_SENSOR_DATA_PACKET   MENLO_RADIO_SERIAL_SENSORDATA
#define MOTIONLIGHT_SENSOR_DATA_OVEHEAD  MENLO_RADIO_SERIAL_SENSORDATA_OVERHEAD

//
// See MenloRadioSerial.h for subType MOTIONLIGHT_SENSOR_DATA_SUBTYPE
//

//
// DweetApp allows standard signatures for event callbacks
// and ProcessAppCommands.
//
class MotionLightApp : public DweetApp  {

public:

    MotionLightApp() {
        m_state = StateIdle;
        m_lightToggle = false;
        m_lightMode = LightModeNormal;
    }

    int Initialize(MotionLightConfiguration* config);

    void SetRadio(MenloRadio* menloRadio);

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // GET/SET property commands
    //

    int OnTime(char* buf, int size, bool isSet);
    int LightMode(char* buf, int size, bool isSet);
    int BlinkRate(char* buf, int size, bool isSet);
    int BlinkInterval(char* buf, int size, bool isSet);
    int LightOn(char* buf, int size, bool isSet);
    int LightOff(char* buf, int size, bool isSet);

    // Set if a light trigger event occurs
    void SetLightTriggerEvent();

protected:

    void SensorUpdateRate(uint16_t* value, bool isSet);

    void ProcessLightEvents();

    //
    // Hardware functions
    //

    // Set light state
    virtual void SetLightState(bool state) = 0;

    // Return trigger input state
    virtual bool GetTriggerState() = 0;

    MotionLightConfiguration m_config;

private:

    void StartBlink(unsigned long blinkInterval);

    void StopBlink();

    // MenloRadio* for packet level updates
    MenloRadio* m_radio;

    // State
    uint8_t m_state;

    // operating mode
    uint8_t m_lightMode;

    // Whether sensor updates are sent.
    bool m_sendSensorUpdates;

    // Toggle state for blink
    bool m_lightToggle;

    // Time between sensor updates
    unsigned long m_sensorUpdateTime;

    //
    // Time that the current mode ends
    //
    // This can represent:
    //
    // m_lighton == true, time left the light should be on
    //
    //    May then transition to blinking, or off
    //
    // m_lightblink == true, time left the light should be blinking
    //
    //    May then transition to steady on, or off
    //
    // m_forceontime != 0, time till forceontime is cancelled
    //
    //    Then transition to off, and continue operating according to m_mode
    //
    // m_forceofftime != 0, time till forceofftime is cancelled
    //
    //    Then transition back to current m_mode
    //
    unsigned long m_currentStateEndTime;

    // This is the time the light is on after a trigger
    unsigned long m_ontime;

    // This is the rate it blinks at (ms per period) for blink modes
    unsigned long m_blinkrate;

    // This is the time the light will blink during a blink interval
    unsigned long m_blinkinterval;

    // This is the force on time
    unsigned long m_lightOnInterval;

    // This is the force off time
    unsigned long m_lightOffInterval;

    //
    // A timer is used to run the main state machine
    //

    MenloTimer m_stateTimer;
    MenloTimerEventRegistration m_stateTimerEvent;

    // StateTimerEvent function
    unsigned long StateTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // A timer is used to provide the blink interval
    //

    MenloTimer m_blinkTimer;
    MenloTimerEventRegistration m_blinkTimerEvent;

    // BlinkTimerEvent function
    unsigned long BlinkTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // A Timer provides the radio packet update interval
    //

    MenloTimer m_sensorTimer;
    MenloTimerEventRegistration m_sensorTimerEvent;

    // SensorUpdateTimerEvent function
    unsigned long SensorTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Dweet channel support over serial.
    //
    DweetSerialChannel m_dweetSerialChannel;

    // Dweet Serial Channel Event registration
    MenloDweetEventRegistration m_serialDweetEvent;

    // DweetEvent function
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};


#endif // MotionLightApp_h
