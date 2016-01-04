
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

#include <MenloPlatform.h>
#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloLightHouse.h>

MenloLightHouse::MenloLightHouse()
{
  m_sequence = NULL;
  m_sequenceSizeInBits = 0;
  m_interval = 0;
  m_period = (60L * 1000L); // 60 seconds
  m_rampUpPeriod = 0;
  m_rampDownPeriod = 0;
  m_red = 0;
  m_green = 0;
  m_blue = 0;
  m_eventSignaled = false;
  m_eventLightState = false;
  m_sequenceIndex = 0;
}

int
MenloLightHouse::Initialize()
{
  // invoke base to initialize MenloDispatchObject
  MenloDispatchObject::Initialize();

  m_eventSignaled = false;
  m_eventLightState = false;

  m_interval = LIGHTHOUSE_DEFAULT_INTERVAL;

  m_timer.Initialize();

  // Standard args
  m_timerEvent.object = this;

  m_timerEvent.method = (MenloEventMethod)&MenloLightHouse::TimerEvent;

  // Object type specific args
  m_timerEvent.m_interval = m_interval;

  m_timer.RegisterIntervalTimer(&m_timerEvent);

  return 0;
}

// Overridden from MenloDispatchObject
unsigned long
MenloLightHouse::Poll()
{
  MenloLightHouseEventArgs eventArgs;
  unsigned long pollInterval = MAX_POLL_TIME;

  //  
  // An objects internal state can change async from the dispatcher
  // loop. These events can not be signaled until the Poll()
  // dispatcher callback is invoked. To handle this the event
  // "trigger" and updated state/status is recorded until the next
  // Poll() interval. It is reset once an event is set.
  //
  // This is a very common event driven programming pattern in how
  // async events such as interrupts are communicated to mainline
  // code which can not process the state transition until its
  // at a code synchronization/idle point.
  //
  // Note: In simple cases boolean is used, but could lose
  // state transitions. An event counter could be used by implementations
  // that require no event loss.
  //
  // For lighthouse the most up to date lightstate is sent when an event
  // is to be delivered, so even if events are lost the next event
  // sent "catches up" with the light state.
  //
  if (!m_eventSignaled) {
      DISPATCH_PRINT2("MenloLightHouse::Poll no events: pollInterval=", pollInterval);
      return pollInterval;
  }

  eventArgs.lightState = m_eventLightState;

  eventArgs.redIntensity = m_red;
  eventArgs.greenIntensity = m_green;
  eventArgs.blueIntensity = m_blue;

  eventArgs.rampUpPeriod = m_rampUpPeriod;
  eventArgs.rampDownPeriod = m_rampDownPeriod;

  m_eventSignaled = false;

  // Send event to listeners
  pollInterval = m_lightList.DispatchEvents(this, &eventArgs);

  DISPATCH_PRINT2("MenloLightHouse::Poll event signaled, return: pollInterval=", pollInterval);

  return pollInterval;
}

unsigned long
MenloLightHouse::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
  // Check memory
  MenloMemoryMonitor::CheckMemory(__LINE__);

  // Perform tick processing
  DISPATCH_PRINT2("MenloLighHouse::TimerEvent tick! millis=",GET_MILLISECONDS());

  // Process lighthouse state machine ticker
  return SequenceTick();
}

void
MenloLightHouse::RegisterLightStateEvent(MenloLightHouseEventRegistration* callback)
{
  // Add to event list
  m_lightList.Register(callback);
  return;
}

void
MenloLightHouse::SetSequenceBuffer(uint8_t* seq, uint8_t seqSizeInBits)
{
  m_sequence = seq;
  m_sequenceSizeInBits = seqSizeInBits;

  ResetSequence();
}

void
MenloLightHouse::GetSequenceBuffer(uint8_t** seq, uint8_t* seqSizeInBits)
{
  *seq = m_sequence;
  *seqSizeInBits = m_sequenceSizeInBits;
}

void
MenloLightHouse::ResetSequence()
{
  // Reset the operation counters
  m_sequenceIndex = 0;
}

unsigned long
MenloLightHouse::SequenceTick()
{
  uint8_t byteIndex;
  uint8_t bitIndex;
  uint8_t b;
  bool bitValue;

  //
  // If a sequence is not set we return.
  //
  // In this case the light state is controlled statically.
  //
  if (m_sequence == NULL) {
      return 0;
  }

  //
  // Advance through the bit sequence.
  //
  // If it results in a change of light status update
  // the signaled status so a LightStatus event is raised.
  //

  // Integer divide rounds down to whole bytes
  byteIndex = m_sequenceIndex / 8;

  // Remainder is the bits to shift
  bitIndex = m_sequenceIndex % 8;

  b = m_sequence[byteIndex];
  
  bitValue = (b >> bitIndex) & 0x01;
  
  m_sequenceIndex++;

  if (m_sequenceIndex > m_sequenceSizeInBits) {
      m_sequenceIndex = 0;
  }

  if (m_eventLightState != bitValue) {

      //
      // Indicate new light state and event which will be indicated at
      // the next Poll() interval.
      //

      m_eventLightState = bitValue;
      m_eventSignaled = true;

      //
      // We indicated a state change so we need to return a poll
      // time of 0 to immediately process the event loop.
      //
      return 0;
  }

  //
  // TODO: We could maximize sleep time by counting how many intervals ahead
  // that have the same light state and resetting the interval timer
  // and poll() time to the new value.
  //
  // Currently we do not have resetable interval support on
  // timer. (Maybe do with UnRegister/Register, but not tested yet)
  //

  return MAX_POLL_TIME;
}

void
MenloLightHouse::LightTick(unsigned long* tickPeriod, bool isSet)
{
  if (!isSet) {
      *tickPeriod = m_interval;
      return;
  }

  // Unregister our current timer
  m_timer.UnregisterIntervalTimer(&m_timerEvent);

  // reset interval
  m_interval = *tickPeriod;

  // Object type specific args
  m_timerEvent.m_interval = m_interval;

  // Register timer
  m_timer.RegisterIntervalTimer(&m_timerEvent);
}

void
MenloLightHouse::LightPeriod(unsigned long* lightPeriod, bool isSet)
{
  if (isSet) {
      m_period = *lightPeriod;
  }
  else {
      *lightPeriod = m_period;
  }
}

void
MenloLightHouse::RampPeriod(
    uint16_t* rampUpPeriod,
    uint16_t* rampDownPeriod,
    bool isSet
    )
{
  if (isSet) {
      m_rampUpPeriod = *rampUpPeriod;
      m_rampDownPeriod = *rampDownPeriod;
  }
  else {
      *rampUpPeriod = m_rampUpPeriod;
      *rampDownPeriod = m_rampDownPeriod;
  }
}

void
MenloLightHouse::LightColor(uint8_t* red, uint8_t* green, uint8_t* blue, bool isSet)
{
  if (isSet) {
      m_red = *red;
      m_blue = *blue;
      m_green = *green;
  }
  else {
      *red = m_red;
      *blue = m_blue;
      *green = m_green;
  }
}

void
MenloLightHouse::LightOnLevel(uint16_t* onLevel, bool isSet)
{
  if (isSet) {
      m_lightOnLevel = *onLevel;
  }
  else {
      *onLevel = m_lightOnLevel;
  }
}
