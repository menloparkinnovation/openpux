
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
 *  File: MenloRadio.cpp
 */

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#include <alloca.h>

//
// Include Menlo Debug library support
//
#include <MenloPlatform.h>
#include <MenloUtility.h>
#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloConfigStore.h>

// This libraries header
#include <MenloRadio.h>

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define DBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_HEX_STRING(x, l)
#define DBG_PRINT_HEX_STRING_NNL(x, l)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define xDBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_HEX_STRING(x, l)
#define xDBG_PRINT_HEX_STRING_NNL(x, l)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

#define NEW_TIMER 1

// Constructor
MenloRadio::MenloRadio() {

    m_receiveEventSignaled = false;
    m_radioPowerState = false;
    m_radioActivity = false;
    m_sendRadioAttention = false;
    m_radioAttentionActive = false;

    m_attentionInterval = 0;
    m_sendAttentionInterval = 0;
    m_powerInterval = 0;
}

int
MenloRadio::Initialize()
{
    // invoke base to initialize MenloDispatchObject
    MenloDispatchObject::Initialize();

    m_receiveEventSignaled = false;

    //
    // Setup the radio power timer
    //

    m_powerInterval = RADIO_DEFAULT_POWER_INTERVAL;

    m_attentionInterval = 0;

    // Standard args
    m_timerEvent.object = this;

    m_timerEvent.method = (MenloEventMethod)&MenloRadio::TimerEvent;

    // Object type specific args
    m_timerEvent.m_interval = m_powerInterval;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    //
    // The radio power timer is not started until PowerOn() or SetPowerTimer(time)
    // is called.
    //

    return 0;
}

void
MenloRadio::SetActivity()
{
    m_radioActivity = 1;
}

void
MenloRadio::SetSendAttention(unsigned long interval, uint8_t* address)
{
    m_sendAttentionAddress = address;
    m_sendAttentionInterval = interval;
    m_sendRadioAttention = true;
}

// Overridden from MenloDispatchObject
unsigned long
MenloRadio::Poll()
{
  int retVal;
  MenloRadioEventArgs eventArgs;
  unsigned long pollInterval = MAX_POLL_TIME;

  //
  // If we have a registered receive event we poll the
  // radio for receive data.
  //
  if (m_eventList.HasListeners() && ReceiveDataReady()) {
       m_radioActivity = true; // Indicate activity to the power time
       m_receiveEventSignaled = true;
  }

  // If radio receive data raise the receiveEvent
  if (!m_receiveEventSignaled) {
      return pollInterval;
  }

  m_receiveEventSignaled = false;

  // 0 timeout since we only read when data is ready
  retVal = Read(0);
  if (retVal > 0) {
      eventArgs.data = GetReceiveBuffer();
      eventArgs.dataLength = GetPacketSize();

      // Send event to listeners
      pollInterval = m_eventList.DispatchEvents(this, &eventArgs);
  }
  else {
      DBG_PRINT("MenloRadio No data on read!");
  }

  return pollInterval;
}

void
MenloRadio::RegisterReceiveEvent(MenloRadioEventRegistration* callback)
{
  // Add to event list
  m_eventList.Register(callback);
  return;
}

void
MenloRadio::UnregisterReceiveEvent(MenloRadioEventRegistration* callback)
{
  // Remove from event list
  m_eventList.Unregister(callback);
  return;
}

//
// Request to power on the radio.
//
void
MenloRadio::PowerOn()
{
  // Power the radio on
  if (!m_radioPowerState) {

      xDBG_PRINT("Radio PowerOn request");

      m_radioPowerState = true;

      //
      // Start the power timer
      //
      // m_timerEvent.m_interval has already been set
      //
#if NEW_TIMER
      if (m_powerInterval != 0) {
          xDBG_PRINT("PowerOn registering power timer");
          m_timer.RegisterIntervalTimer(&m_timerEvent);
      }
#endif

      // We treat a request to power on the radio as activity
      m_radioActivity = true;

      // Inform the driver subclass of power on condition
      OnPowerOn();
  }  
}

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
void
MenloRadio::SetPowerTimer(unsigned long inactivityTimer)
{
    //
    // m_powerInterval is not zero if a timer has been set.
    //

    xDBG_PRINT_NNL("SetPowerTimer inactivityTimer low ");
    xDBG_PRINT_INT((uint16_t)inactivityTimer);

    xDBG_PRINT_NNL("SetPowerTimer inactivityTimer high ");
    xDBG_PRINT_INT((uint16_t)(inactivityTimer >> 16));

    // See if there has been a change in the activity timer
    if (inactivityTimer != m_powerInterval) {

#if NEW_TIMER
        if ((m_powerInterval != 0) && m_radioPowerState) {
#else
        if (m_powerInterval != 0) {
#endif
            xDBG_PRINT("SetPowerTimer unregistered previous timer");
            m_timer.UnregisterIntervalTimer(&m_timerEvent);
        }

        m_powerInterval = inactivityTimer;

        // Update interval
        m_timerEvent.m_interval = m_powerInterval;

        //
        // m_powerInterval == 0 means radio always powered on
        // so we don't start the inactivityTimer.
        //
        if (m_powerInterval != 0) {

#if NEW_TIMER
           if ((m_powerInterval != 0) && m_radioPowerState) {
                xDBG_PRINT("SetPowerTimer registering powerTimer");
                m_timer.RegisterIntervalTimer(&m_timerEvent);
           }
#else
            xDBG_PRINT("Old code SetPowerTimer registering powerTimer");
            m_timer.RegisterIntervalTimer(&m_timerEvent);
#endif
        }
        else {
            xDBG_PRINT("Radio inactivityTimer disabled");
        }
    }
}

//
// TimerEvent runs every m_powerInterval when enabled
// and the radio is powered on.
//
unsigned long
MenloRadio::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    xDBG_PRINT("RadioPower TimerEvent");

    //
    // Indications of radioActivity
    //
    // ReceiveDataReady() - power on and receive data available
    //
    // WaitForSendComplete() - If packet is transmitting power
    // needs to remain on.
    //
    // Write - packet transmit - Need to see when its done
    //
    // channel and settings management: Let the radio itself manage
    // power internally if required for settings.
    //

    //
    // Application pattern is sleep 30 seconds, wake up and send an
    // update packet, then shutdown the radio and sleep when transmit
    // is done. Though in many cases we also want to listen for a little
    // while in case the gateway/edge unit has a message for us.
    //

    // Check to see if we should power down the radio
    if (m_radioPowerState) {

        if (m_radioActivity) {

            //
            // There has been radio activity in the last period
            // so reset it and return and check again in the next
	    // period.
            //
	    m_radioActivity = false;
            xDBG_PRINT("RadioPower cleared activity bit power still on");
        }
        else {

            //
	    // No radioActivity for at least one radio power interval
            //
            // See if attention is active
            //
            if (m_radioAttentionActive) {

                xDBG_PRINT("Radio Attention is active keeping power on");

                DBG_PRINT_NNL("m_attentionInterval low ");
                DBG_PRINT_INT((uint16_t)m_attentionInterval);

                DBG_PRINT_NNL("m_attentionInterval high ");
                DBG_PRINT_INT((uint16_t)(m_attentionInterval >> 16));

                DBG_PRINT_NNL("m_powerInterval low ");
                DBG_PRINT_INT((uint16_t)m_powerInterval);

                DBG_PRINT_NNL("m_powerInterval high ");
                DBG_PRINT_INT((uint16_t)(m_powerInterval >> 16));

                if (m_powerInterval > m_attentionInterval) {
                    // Don't under wrap
                    m_attentionInterval = 0;
                }
                else {
                    m_attentionInterval -= m_powerInterval;
                }

                if (m_attentionInterval == 0) {

                    // Attention interval has gone to zero, clear attention
                    m_radioAttentionActive = false;

                    xDBG_PRINT("Radio Attention interval ended");

                    //
                    // Radio will be powered down on the next power interval
                    // if there is no activity.
                    //
                }

                DBG_PRINT_NNL("new m_attentionInterval low ");
                DBG_PRINT_INT((uint16_t)m_attentionInterval);

                DBG_PRINT_NNL("new m_attentionInterval high ");
                DBG_PRINT_INT((uint16_t)(m_attentionInterval >> 16));
            }
            else {

                //
                // Attention is not active and the radio has had
                // no activity for the power interval, so power down the radio.
                //
                DBG_PRINT("RadioPower no activity powering off radio");
                m_radioPowerState = false;
#if NEW_TIMER
                xDBG_PRINT("RadioTimerEvent powering off radio and uregistering timer");
                m_timer.UnregisterIntervalTimer(&m_timerEvent);
#endif
                OnPowerOff();
            }
        }
    }

    if (m_radioPowerState && (m_powerInterval != 0)) {

        //
        // Inform the scheduler we have an outstanding timer
        //
        // TODO: Shouldn't the MenloTimer do this on our behalf?
        // - validate this is true and remove this to be MAX_POLL_TIME
        //
        return m_powerInterval;
    }
    else {
        return MAX_POLL_TIME;
    }
}

//
// Received and attention packet over the radio
//
void
MenloRadio::ProcessAttentionReceive(uint8_t* buf)
{
    MenloRadioLinkControlAttention* att;
    unsigned long period;

    xDBG_PRINT("Radio attention received");

    att = (MenloRadioLinkControlAttention*)buf;

    period = 0;
    period |= (att->period0 & 0x000000FF);
    period |= ((att->period1 << 8)  & 0x0000FF00);
    period |= ((att->period2 << 16) & 0x00FF0000);
    period |= ((att->period3 << 24) & 0xFF000000);

    DBG_PRINT_NNL("Radio Attention received low ");
    DBG_PRINT_INT((uint16_t)period);

    DBG_PRINT_NNL("Radio Attention received high ");
    DBG_PRINT_INT((uint16_t)(period >> 16));

    //
    // Set radio attention to ensure the radio power
    // stays on.
    //
    // We know its currently on since we just received
    // a packet.
    //
    m_attentionInterval = period;
    m_radioAttentionActive = true;
}

void
MenloRadio::ProcessAttentionSend()
{
    int retVal;
    MenloRadioLinkControlAttention* att;
    uint8_t* buf = (uint8_t*)alloca(GetPacketSize());

    // Verify we did not overflow the stack here with alloca()
    MenloMemoryMonitor::CheckMemory(LineNumberBaseMenloRadio + __LINE__);

    if (!m_sendRadioAttention) return;

    att = (MenloRadioLinkControlAttention*)buf;

    att->type = MENLO_RADIO_LINKCONTROL;
    att->control = RADIO_LINKCONTROL_ATTENTION;

    att->period0 = m_sendAttentionInterval & 0x000000FF;
    att->period1 = (m_sendAttentionInterval >> 8)  & 0x000000FF;
    att->period2 = (m_sendAttentionInterval >> 16) & 0x000000FF;
    att->period3 = (m_sendAttentionInterval >> 24) & 0x000000FF;

    retVal = Write(
        m_sendAttentionAddress,
        buf,
        GetPacketSize(),
        SEND_ATTENTION_TIMEOUT
        );

    //
    // Send attention did not have an error, so clear
    // the setting.
    //
    if (retVal != 0) {
        DBG_PRINT("Sent attention packet");
        m_sendRadioAttention = false;
    }
    else {
        DBG_PRINT("Attention packet send failed");
    }
}

int
MenloRadio::Read(unsigned long timeout)
{
    uint8_t* buffer;

    int result = OnRead(timeout);

    if (result != 0) {
        buffer = GetReceiveBuffer();

        //
        // See if its an attention packet targeted at us
        // to extend our power down sleep interval due to
        // the application indicating there will be future traffice.
        //
        if ((PACKET_SUBTYPE_MASK(buffer[0] == MENLO_RADIO_LINKCONTROL) &&
            (buffer[1] & RADIO_LINKCONTROL_ATTENTION))) {

            // Set sleep interval to attention time period
            ProcessAttentionReceive(buffer);

            // We grab the packet from underneath the caller
            return 0;
        }

        //
        // We may have m_sendRadioAttention set in which we must
        // send an attention packet to the other side after
        // any packet is received.
        //
        // Note: This function can not change or damage the buffer
        // inGetReceiveBuffer() as its valid results are returned
        // to the caller.
        //
        if (m_sendRadioAttention) {
            ProcessAttentionSend();
        }
    }

    return result;
}

int
MenloRadio::Write(
    byte* targetAddress,
    uint8_t* transmitBuffer,
    uint8_t transmitBufferLength,
    unsigned long timeout
    )
{
    return OnWrite(
        targetAddress,
        transmitBuffer,
        transmitBufferLength,
        timeout
        );
}
