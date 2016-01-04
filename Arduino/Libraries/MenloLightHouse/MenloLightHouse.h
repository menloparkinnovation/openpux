
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
 *  Date: 02/10/2015
 *  File: MenloLightHouse.h
 */

#ifndef MenloLightHouse_h
#define MenloLightHouse_h

// Include this if constants are used
#include "MenloPlatform.h"
#include "MenloTimer.h"

// Default interval is 250 ms. 30 byte string represents 60 seconds period.
 #define LIGHTHOUSE_DEFAULT_INTERVAL 250

//
// Lighthouse raises an Event when a change in light state occurs.
//
// The current configured state is made available in the event
// args for the event handler to update the hardware light state.
//
class MenloLightHouseEventArgs : public MenloEventArgs {
 public:
  bool lightState; // TRUE == ON, FALSE == OFF.

  uint16_t rampUpPeriod;
  uint16_t rampDownPeriod;

  uint8_t redIntensity;
  uint8_t greenIntensity;
  uint8_t blueIntensity;
};

// Introduce the proper type name. Could be used for additional parameters.
class MenloLightHouseEventRegistration : public MenloEventRegistration {
 public:
};

class MenloLightHouse : public MenloDispatchObject {
 public:

  MenloLightHouse();

  virtual int Initialize();

  virtual unsigned long Poll();

  //
  // Register event invoked when the light changes state.
  //
  void RegisterLightStateEvent(MenloLightHouseEventRegistration* callback);

  //
  // Set the sequence buffer
  //
  void SetSequenceBuffer(uint8_t* seq, uint8_t seqSizeInBits);

  void GetSequenceBuffer(uint8_t** seq, uint8_t* seqSizeInBits);

  //
  // Reset the sequence to the begining.
  //
  void ResetSequence();

  //
  // Light ticks are the smallest interval clock which represents
  // each entry in the sequence.
  //
  // All other periods need to be an integer multiple of this value
  // to prevent rounding of sequences.
  //
  void LightTick(unsigned long* tickPeriod, bool isSet);

  //
  // Light period is the total time for the sequence.
  //
  // If the sequence ends before the period is up the light is
  // in the last state of the sequence until the sequence restart.
  //
  void LightPeriod(unsigned long* lightPeriod, bool isSet);

  //
  // Ramp period is how long to go from off to on, and on to off.
  //
  // It's used to simulate a traditional filament bulb, or the side
  // light from a rotating fresnal lens and/or shutter.
  //
  void RampPeriod(uint16_t* rampUpPeriod, uint16_t* rampDownPeriod, bool isSet);

  //
  // Light color allows the setting of RGB values used for multi-color
  // lights.
  //
  // They are 8 bit values used for intensity to provide multiplle colors
  // with 3 color LEDs.
  //
  void LightColor(uint8_t* red, uint8_t* green, uint8_t* blue, bool isSet);

  //
  // Set the light on level which controls the transition from
  // daylight to darkness.
  //
  // If the reading of the light sensor falls below the set
  // value the light comes on.
  //
  // Setting the value to maximum of 0xFFFF means the light
  // is always on, day or night.
  //
  // The model assumes the light sensor readings are proportional
  // to the amount of daylight/ambient light from 0 -> 0xFFFF
  // with higher values being brighter light conditions.
  //
  // This value allows a user to customize the level required
  // to trigger the lighthouse function.
  //
  void LightOnLevel(uint16_t* onLevel, bool isSet);

 protected:

  // Perform the next tick in the sequence
  unsigned long SequenceTick();

  // Sequence consists of the bits for the sequence as an array of bytes.
  uint8_t* m_sequence;

  // This size in bits
  uint8_t  m_sequenceSizeInBits;

  //
  // Interval is the primary MenloTimer interval. It is the shorted
  // resolution time available for a given sequence spacing.
  //
  unsigned long m_interval;

  //
  // Period is for the total cycle
  //
  unsigned long m_period;

  uint16_t m_rampUpPeriod;

  uint16_t m_rampDownPeriod;

  // Light On level
  uint16_t m_lightOnLevel;

  //
  // Light color values
  //
  uint8_t m_red;
  uint8_t m_green;
  uint8_t m_blue;

  //
  // This represents the event state machine that indicates
  // when an event should be sent.
  //
  // An event can not be sent until the dispatch loop runs and
  // invokes Poll(), so this stores that state.
  //
  bool m_eventSignaled;
  bool m_eventLightState;

 private:

  //
  // The Lighthouse operates as a state machine to allow
  // event driven/interrupt driven operation.
  //
  // This allows it to multi-task and save battery life by sleeping
  // between sequence pauses.
  //

  // Current sequence index
  uint8_t m_sequenceIndex;

  //
  // MenloLightHouse is a client of MenloTimer.
  //  

  // Timer and event registration
  MenloTimer m_timer;
  MenloTimerEventRegistration m_timerEvent;

  // TimerEvent function
  unsigned long TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

  // MenloLightHouse is an Event generator for signaling light state
  MenloEvent m_lightList;
};

//
// State Machine Model:
//
// The array of bytes in m_sequence is treated as a series of bits
// in a bitstream. This bitstream's size is indicated by m_sequenceSize.
//
// The m_interval clocks the state machine which acts as a shift register
// through the bits in sequence. A 1 bit in a given position indicates
// light on state, while a zero bit indicates light off state.
//
// When m_sequenceSize is reached the shift register stops with
// the light value remaining the last bit value seen.
//
// The m_period specifies the begining of a cycle when the bitstream
// is clocked through the (virtual) shift register. It specifies
// the total period which encompasses the time spent clocking the
// sequence, and not the wait time at the end till the next start.
// This makes it easier to model typical periodic sequences
// of Lighthouses.
//
// Light polarity can be controlled separately so 1 is always on,
// and 0 is always off for the core state machine.
//
// When the light is on or off, its values are retrieved from the
// m_red, m_green, m_blue values to allow a multi-color LED to
// be used with PWM to allow customization.
//
// Since its just a clocked bit stream, just about any sequence
// of lights can be encoded. This includes occulating, pulsing,
// flashing, in addition to standard morse sequences common
// in many North American (at least US) Lighthouses.
//
// The applications design also accomodates modeling "buoy's"
// which are navigational markers which typically flash a
// red, green, or amber light as channel markers.
//

#endif // MenloLightHouse_h
