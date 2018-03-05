
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
 *  Date: 07/05/2014
 *
 *  This class provides uniform WiFi interfaces.
 */

//
// MenloFramework
//
#include <MenloPlatform.h>
#include <MenloUtility.h>
#include <MenloDebug.h>
#include <MenloMemoryMonitor.h>
#include <MenloConfigStore.h>

// This libraries header
#include <MenloWiFi.h>

#define DBG_PRINT_ENABLED 1

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
#define XDBG_PRINT_ENABLED 1

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

MenloWiFi::MenloWiFi() {

    m_connectionState = WiFiConnectionStateInit;
    m_wifiEvent = WiFiEventNone;
    m_wifiEventSignaled = false;
    m_radioPowerState = false;
    m_radioActivity = false;
    m_sendRadioAttention = false;
    m_radioAttentionActive = false;

    m_attentionInterval = 0;
    m_sendAttentionInterval = 0;
    m_powerInterval = 0;
}

int
MenloWiFi::Initialize()
{
    // invoke base to initialize MenloDispatchObject
    MenloDispatchObject::Initialize();

    m_wifiEventSignaled = false;

    //
    // Setup the radio power timer
    //

    m_powerInterval = WIFI_DEFAULT_POWER_INTERVAL;

    m_attentionInterval = 0;

    // Standard args
    m_timerEvent.object = this;

    m_timerEvent.method = (MenloEventMethod)&MenloWiFi::TimerEvent;

    // Object type specific args
    m_timerEvent.m_interval = m_powerInterval;
    m_timerEvent.m_dueTime = 0L; // indicate not registered

    //
    // The wifi radio power timer is not started until PowerOn() or SetPowerTimer(time)
    // is called.
    //

    return 0;
}

void
MenloWiFi::SetActivity()
{
    m_radioActivity = 1;
}

void
MenloWiFi::SetSendAttention(unsigned long interval, uint8_t* address)
{
    m_sendAttentionAddress = address;
    m_sendAttentionInterval = interval;
    m_sendRadioAttention = true;
}

//
// Overridden from MenloDispatchObject
//
unsigned long
MenloWiFi::Poll()
{
  int retVal;
  MenloWiFiEventArgs eventArgs;
  unsigned long pollInterval = MAX_POLL_TIME;

  //
  // Process connection state machine
  //
  if (m_connectionState != WiFiConnectionStateConnected) {
       ProcessConnectionStateMachine();
  }

  //
  // If we have a registered receive event we poll the
  // radio for receive data.
  //
  if ((m_wifiEvent != WiFiEventNone) && (m_eventList.HasListeners())) {

       eventArgs.eventType = m_wifiEvent;
       m_wifiEvent = WiFiEventNone;

       m_radioActivity = true; // Indicate activity to the power timer
       m_wifiEventSignaled = true;
  }

  if (!m_wifiEventSignaled) {
      return pollInterval;
  }

  //
  // Raise event
  //

  m_wifiEventSignaled = false;

  eventArgs.ConnectionState = m_connectionState;
  eventArgs.eventData = NULL;
  eventArgs.eventDataLength = 0;

  // Send event to listeners
  pollInterval = m_eventList.DispatchEvents(this, &eventArgs);

  DBG_PRINT("MenloWiFi Event delivered");

  return pollInterval;
}

void
MenloWiFi::RegisterWiFiEvent(MenloWiFiEventRegistration* callback)
{
  // Add to event list
  m_eventList.Register(callback);
  return;
}

void
MenloWiFi::UnregisterWiFiEvent(MenloWiFiEventRegistration* callback)
{
  // Remove from event list
  m_eventList.Unregister(callback);
  return;
}

//
// Request to power on the radio.
//
void
MenloWiFi::PowerOn()
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
MenloWiFi::SetPowerTimer(unsigned long inactivityTimer)
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
MenloWiFi::TimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
    xDBG_PRINT("RadioPower TimerEvent");

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
// Received and attention indication
//
void
MenloWiFi::ProcessAttentionReceive(uint8_t* buf)
{
    unsigned long period;

    xDBG_PRINT("Radio attention received");

    //
    // Set radio attention to ensure the radio power
    // stays on.
    //
    // We know its currently on since we just received
    // a packet.
    //
    //m_attentionInterval = period;
    //m_radioAttentionActive = true;
}

void
MenloWiFi::ProcessAttentionSend()
{
    int retVal;

    if (!m_sendRadioAttention) return;

    //
    // Send this interval to the client to keep its radio listening
    //
    // m_sendAttentionInterval
    //

    m_sendRadioAttention = false;
}

//
// Set the currently active credentials.
//
bool
MenloWiFi::SetCredentials(char* ssid, char* password)
{
    if (ssid != NULL) {
        if (strlen(ssid) >= sizeof(m_ssid)) return false;
        strcpy(&m_ssid[0], ssid);        
    }
    else {
        m_ssid[0] = '\0';
    }

    if (password != NULL) {
        if (strlen(password) >= sizeof(m_password)) return false;
        strcpy(&m_password[0], password);        
    }
    else {
        m_password[0] = '\0';
    }

    return true;
}

void
MenloWiFi::ProcessConnectionStateMachine()
{
    //
    // See if the connecting wait timeout period has expired
    //
    if (m_connectionState == WiFiConnectionStateConnectingWait) {
        unsigned long current_time = GET_MILLISECONDS();

        // TODO: This has a 49.71 day wrap around, update to integer overflow math.
        if (current_time >= m_connectingWaitInterval) {
            // Transition back to the connecting state
            m_connectionState = WiFiConnectionStateConnecting;
        }
    }

    // Attempt to move to the connected state.
    if (m_connectionState == WiFiConnectionStateConnecting) {
        OnConnect();
    }

    // Attempt to move to the disconnected state.
    if (m_connectionState == WiFiConnectionStateDisconnecting) {
        OnDisconnect();
    }

    //
    // The following states are not processed by this function
    //
    // WiFiConnectionStateInit
    // WiFiConnectionStateNoHardware
    // WiFiConnectionStateIdle
    // WiFiConnectionStateConnected
    // WiFiConnectionStateDisconnected
    // WiFiConnectionStatePoweredDown
    // WiFiConnectionStateNoCredentials
    //

}

//
// Attempt to set a new connection state.
//
bool
MenloWiFi::SetConnectionTargetState(uint8_t targetState)
{
    uint8_t previousState = m_connectionState;

    //
    // Process the new state target
    //

    //
    // A force to no hardware state. No commands are valid since
    // we don't know the hardware state.
    //
    if (targetState == WiFiConnectionStateNoHardware) {
        m_connectionState = targetState;
        return true;
    }

    //
    // A WiFi driver can force transition from no hardware to init.
    //
    if (previousState == WiFiConnectionStateNoHardware) {
        if (targetState == WiFiConnectionStateInit) {
            m_connectionState = targetState;
            return true;
        }

        // Can't set other target states when no hardware.
        return false;
    }

    //
    // If connecting, a WiFi driver can transition to no credentials state.
    //
    if ((targetState == WiFiConnectionStateNoCredentials) &&
        (previousState == WiFiConnectionStateConnecting)) {

        m_connectionState = targetState;
        return true;
    }

    if (targetState == WiFiConnectionStateConnected) {
        if (previousState == WiFiConnectionStateConnected) return true;

        // A WiFi driver is reporting connected state
        if (previousState == WiFiConnectionStateConnecting) {

            m_wifiEvent = WiFiEventConnected;

            m_connectionState = targetState;
            return true;
        }

        if (previousState == WiFiConnectionStatePoweredDown) {

            // power up radio
            PowerOn();

            // Fallthrough
        }

        m_connectionState = WiFiConnectionStateConnecting;
        return true;
    }

    if (targetState == WiFiConnectionStateDisconnected) {
        if (previousState == WiFiConnectionStateDisconnected) return true;

        // A WiFi driver is reporting disconnected state
        if (previousState == WiFiConnectionStateDisconnecting) {

            m_wifiEvent = WiFiEventDisconnected;

            m_connectionState = targetState;
            return true;
        }

        m_connectionState = WiFiConnectionStateDisconnecting;
        return true;
    }

    // Target state can't be set
    return false;
}

void
MenloWiFi::SetConnectingWait(unsigned long interval)
{
    unsigned long current_time = GET_MILLISECONDS();

    m_connectingWaitInterval = current_time + interval;
    m_connectionState = WiFiConnectionStateConnectingWait;
}

bool
MenloWiFi::StartSignOn()
{
    return SetConnectionTargetState(WiFiConnectionStateConnected);
}

bool
MenloWiFi::StartSignOff()
{
    return SetConnectionTargetState(WiFiConnectionStateDisconnected);
}


