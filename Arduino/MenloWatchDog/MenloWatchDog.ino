
/*
 * Copyright (C) 2018 Menlo Park Innovation LLC
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
 * Date: 02/28/2018
 * File: MenloWatchDog.ino
 *
 * Watchdog for IoT projects.
 *
 */

//
// MenloWatchDog
//
// Designed to work with low cost Arduino Pro Mini's which are
// now about $2.49 on Ebay (As of 03/04/2018 for clones).
//
// The Arduino can directly drive the RESET line of the device
// being monitored, in addition to controlling a power switch
// to the device.
//
// The Arduino listens on an input port for a complete high-low-high
// transition to see if the device is still functioning. If this
// transition does not occur within the configured watchdog interval,
// the software goes through a device reset, then power cycle
// of the monitored device by controlling its output pins which
// are reconfigurable for your project. (in code).
//
// In addition a serially connected device such as a RaspberryPi
// over the USB serial port, or TTL serial can send messages
// to the Ardunio instead of the hardware handshake.
//
// An easy, no soldering solution to a USB powered device can be
// utilized with the SwitchDocLabs MiniProLP which is an Arduino Pro Mini
// with Grove connectors along with their USB Power control and allows
// a single Grove cable to connect the control function, and another
// Grove breakout cable to connect to your monitored devices
// RESET line and heartbeat line.
//
// Power must be supplied both to the Arduino watchdog and the
// input to the USB power switch. The easiest way to supply power
// to both without soldering is with a USB Y power splitter that goes to
// both devices. There are even a small cube on Amazon that
// does this. This is because the USB power switch, and the Arduino
// must receive power directly since the Arduino will always be running,
// but control the device being monitored whose power is fed through
// the input to the USB power switcher.
//
// Note: The SwitchDocLabs MiniProLP is actually $0.95 cheaper than
// the 555 timer based hardware watchdog which does not work
// solderless/plug and play out of the box with the USB power switcher
// using Grove connectors. It also just uses a single pulse, not full
// transition cycle. So if your device gets stuck with the watchdog pin low,
// it will never reset. With the Arduino Pro Mini and this code you are ensured
// of a reset if a full cycle(s) are not seen.
//
// The software method on the Arduino based on the AtMega328 chip
// with built in hardware watchdog is a lot more flexible to adapt
// to your project. With many of the output port pins unused, you
// can sequence multiple devices and outboard I/O with the watchdog
// ensuring water pumps, heaters, etc. get shutdown on software failure
// in the main control loop.
//
// An additional feature that can be added is detecting a potentiometer
// plugged into one of the Grove Ax inputs and allow it to set the
// watchdog timeout for field deployments where its easier to use
// a screw driver.
//  - software can validate minimum/maximum range against the config.
//  - presence detect would need to probe whether its set fully
//    high or low on the potentiometer.
//
// Note: Clone Arduino Pro Mini's are now about $2.49 on Ebay (03/04/2018)
// from China. So order ahead and get cheap watch dog units, though
// you must do a little soldering yourself, along with an inexpensive
// MOSFET switcher.
//
// Project Notes:
//
// 21,134 bytes code
// 955 bytes RAM.
//
// on Mac with SwitchDocLabs provided FTDI:
// /dev/cu.usbserial-AK059ABD
//
// Note: If EEPROM configuration is messed up, use the Arduino sketch
// clear EEPROM which is part of the standard IDE.
//
//  File->Examples->EEPROM->eeprom_clear
//
//  Build and upload it and it will clear the EEPROM.
//
//  Then reload the MenloWatchDog project.
//
// Note: This could be improved by holding an input pin
// low on power on/reset to clear configuration.
//

//
// MenloFramework
//

//
// Note: Arduino requires all these includes even though
// MenloFramework.h already includes them.
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>
#include <MenloConfigStore.h>
#include <MenloPower.h>

#include <MenloFramework.h>

#include <MenloDispatchObject.h>
#include <MenloTimer.h>

//
// MenloDweet Support
//
#include <MenloNMEA0183.h>
#include <MenloDweet.h>
#include <DweetApp.h>
#include <DweetSerialChannel.h>

//
// Note: This example introduces the Application Framework
// pattern used by MenloFramework to promote code-reuse
// and component based building of applications.
//
// Two Application Framework modules are used here:
//
// 1) DweetSerialApp - This provides support for Menlo Dweet over
//    standard serial ports.
//
// 2) MenloWatchDogApp - This is the application broken down into
//    its component parts since it now may be re-used for:
//
//    Different transports/configuration such as Serial, RadioSerial, Http, etc.
//
//    Different hardware platforms that do not start with an Arduino Sketch/Project
//
//    As a component in further frameworks.
//
// All Application Framework components are in the Libraries directory.
//

//
// Application Framework support
//
#include <DweetSerialApp.h>

//
// Hardware specialization class
//
#include "MenloWatchDogApp.h"

//
// Main Application class
//
MenloWatchDogApp g_App;

//
// Application Framework class for MenloDweet of a serial port
//
DweetSerialApp g_AppFramework;

// Arduino 1.6.8 now requires forward declarations like a proper C/C++ compiler.
void HardwareSetup();
void ApplicationSetup();
void MenloFrameworkSetup();

void WatchDogSetResetPin(int value);

void WatchDogSetPowerPin(int value);

int WatchDogGetAndResetWatchDogCount();

// interrupt routine
void WatchDogPinChangeISR();

//
// Hardware Map
// Hardware Configuration
//
// This configuration enables use of the SwitchDocLabs MiniProLP
// with Grove connectors connected to the SwitchDocLabs USB power conntrol.
//
// J1 - D3/D4
//
//     D3 - watchdog input to D7 output on Particle Photon/WeatherShield.
//        - AtMega328 interrupt pin.
//        - Normally flashes as part of LightHouse function.
//        - Ensures MenloFramework event loop is running.
//        - Other projects connect this to an output pin that
//          will make a complete high-low-high transition within
//          the timeout period.
//
//     D4 - Active low RESET to RST on Particle Photon/WeatherShield
//        - active low, pulled up by 1K to VCC on particle.
//        - program for hi-z/input when not active.
//        - Sink it low for RESET.
//        - Arduinos are similar.
//

// interrupt input pin
int g_watchDogPin = 3;

// Active LOW for reset.
int g_resetOutputPin = 4;

//
// SwitchDocLabs USB Power Switch Control
//
// Connect to J4 D8/D9 on MiniProLP to J3 on SwitchDocLabs USB Power Control.
//
//  - pin 4 is GND
//  - pin 3 is VCC
//  - pin 2 is ENABLE  (Sig1) - When High, disable LIPO_IN (JP1) signal.
//  - pin 1 is CONTROL (Sig0) - When High, power on, when low, power off.
//

int g_usbPowerControl_Control = 8;
int g_usbPowerControl_Enable = 9;

//
// Arduinos connect the  on board LED to D13.
// Used to signal when watchdog reset/power cycle is occuring.
// Used by MenloFramework to flash a crash code on firmware failure.
//
int g_panicPin = 13;

//
// Pin cycles are high-low-high transitions.
//
// Note: accessed by both interrupt handler WatchDogPinChangeISR() and
// WatchDogGetAndResetWatchDogCount().
//
volatile int g_watchDogPinCycles = 0;

//
// Only accessed by WatchDogPinChangeISR()
//
volatile int g_watchDogLastPinState = 0;

//
// Setup
//
void setup()
{
    //
    // Setup the hardware
    //
    HardwareSetup();

    //
    // Setup the MenloFramework
    //
    MenloFrameworkSetup();

    //
    // Setup application framework
    //
    ApplicationSetup();
}

//
// Loop
//
void loop()
{
    //
    // We don't have any other processing to do so we set the maximum
    // polltime.
    //
    // Sleep can be shorter due to waking up due to application set timers
    // such as the watchdog interval.
    //
    MenloFramework::loop(MAX_POLL_TIME);
}

//
// Setup application framework.
//
void
ApplicationSetup()
{
  //
  // Application class Configuration
  //
  WatchdogConfiguration appConfig;

  //
  // Application Framework Configuration
  //
  MenloDebug::Print(F("MenloWatchDog Version 1.0 03/10/2018"));

  //
  // Initialize the application class
  //
  appConfig.timeout_time = WATCHDOG_TIMEOUT_DEFAULT;
  appConfig.reset_time = WATCHDOG_RESET_DEFAULT;
  appConfig.power_time = WATCHDOG_POWER_DEFAULT;
  appConfig.number_of_resets = WATCHDOG_RESETS_DEFAULT;
  appConfig.indicator = WATCHDOG_IND_DEFAULT;

  g_App.Initialize(&appConfig);

  ResetWatchdog();

  //
  // Initialize the Application Framework
  //

  // No settings right now. Constructor initializes defaults.
  DweetSerialAppConfiguration config;

  g_AppFramework.Initialize(&config);

  //
  // We check the memory monitor at the end of initialization
  // in case any initialization routines overflowed the stack or heap.
  //
  MenloMemoryMonitor::CheckMemory(__LINE__);

  return;
}

//
// Setup the core MenloFramework
//
void
MenloFrameworkSetup()
{

  //
  // MenloFramework configuration
  //
  MenloFrameworkConfiguration frameworkConfig;

  //
  // The initial framework is configured first
  // to allow use of framework facilities during
  // application hardware and state configuration.
  //

  // Set this to true for hard to debug hangs, resets, and reboots
  //frameworkConfig.synchronousDebug = true;
  frameworkConfig.synchronousDebug = false;

  //
  // Set watchdog which will reset the board after 8 seconds
  // if the application becomes unresponsive.
  //
  frameworkConfig.enableWatchdog = true;

  //
  // Setup memory monitor configuration
  //
  // These values work for MemoyRadioSensorApp, but may have
  // to be adjusted based on actual application heap and stack usage.
  //
  frameworkConfig.heapSize = 128;
  frameworkConfig.stackSize = 320;
  frameworkConfig.guardRegionSize = 16;
  frameworkConfig.enableUsageProfiling = true;

  // Serial port configuration. Defaults to Serial0
  frameworkConfig.baudRate = 9600;

  // Panic Pin
  frameworkConfig.panicPin = g_panicPin;

  MenloFramework::setup(&frameworkConfig);
}

void
HardwareSetup()
{
    // set pinmode as we are using it for flashing the light in our main loop
    if (g_panicPin != (-1)) {
        pinMode(g_panicPin, OUTPUT);     
        digitalWrite(g_panicPin, LOW);        
    }

    // Setup watchdog input/interrupt pin with with pull up
    pinMode(g_watchDogPin, INPUT);
    digitalWrite(g_watchDogPin, HIGH); // enable pull-up resistor

    //
    // setup reset pin as not asserted with high-z since it
    // connects to wire OR reset logic with pullups.
    //
    pinMode(g_resetOutputPin, INPUT);

    //
    // setup USB Power control pins as power on
    //

    // This places the USB power control under control of the Control pin state.
    pinMode(g_usbPowerControl_Enable, OUTPUT);
    digitalWrite(g_usbPowerControl_Enable, HIGH);

    // The control pin state is the power state HIGH == POWER ON.
    pinMode(g_usbPowerControl_Control, OUTPUT);
    digitalWrite(g_usbPowerControl_Control, HIGH);

    // Setup interrupt handler
    attachInterrupt(digitalPinToInterrupt(g_watchDogPin), WatchDogPinChangeISR, CHANGE);
}

//
// Set light state
//
// D13 is the standard Arduino LED pin.
//
// It's used to flash the panic code in the framework due to stack
// overflow, heap overflow, or other exception.
//
// During non-panic cases its ON when the watchdog has activated the
// RESET, power cycle of the device being monitored.
//
void
WatchDogSetLightState(bool state)
{
    digitalWrite(g_panicPin, state);
}

//
// true - assert RESET pin
// false - de-assert RESET pin
//
void
WatchDogSetResetPin(int value)
{
    if (value != 0) {

        //
        // Assert RESET ON
        //
        // This is a wire "OR", so set for output and sink it
        //
        pinMode(g_resetOutputPin, OUTPUT);
        digitalWrite(g_resetOutputPin, LOW);
    }
    else {

        //
        // Assert RESET OFF
        //
        // This is a wire "OR", so set it for input so its hi-z
        // float with RST line pullups on the microcontroller.
        //
        pinMode(g_resetOutputPin, INPUT);
    }

    return;
}

//
// true - assert POWER pin
// false - de-assert POWER pin
//
void
WatchDogSetPowerPin(int value)
{
    if (value != 0) {

        //
        // Assert POWER ON
        //
        digitalWrite(g_usbPowerControl_Control, HIGH);
    }
    else {

        //
        // Assert POWER OFF
        //
        digitalWrite(g_usbPowerControl_Control, LOW);
    }

    return;
}

//
// WatchDog Pin Change ISR routine        
//
void
WatchDogPinChangeISR()
{
  int previous = g_watchDogLastPinState;

  g_watchDogLastPinState = digitalRead(g_watchDogPin);

  if (g_watchDogLastPinState != previous) {

      if (g_watchDogPinCycles < 32000) {
          g_watchDogPinCycles++;
      }
      else {
          //
          // Ensure rollover is never zero.
          //
          // Note: This must be 2 to represent a complete transition.
          // See WatchDogGetAndResetWatchDogCount().
          //
          g_watchDogPinCycles = 2;
      }
  }
}

//
// Get number of transitions and reset the count.
//
// Done as an atomic operation since the counter value is shared
// with the interrupt handler.
//
int
WatchDogGetAndResetWatchDogCount()
{
    int value;
    char oldSREG;

    // Enter ISR critical section
    oldSREG = SREG;
    cli();

    value = g_watchDogPinCycles;
    g_watchDogPinCycles = 0;

    // Leave ISR critical section
    SREG = oldSREG;

    //
    // Note: Since we only count full transitions from high-low-high there must
    // be at least two transitions seen to be non-zero.
    //
    // The ISR will only record transitions as cycles.
    //
    if (value < 2) {
        // No complete transitions have occured.
        return 0;
    }

    //
    // Return value is the number of edges seen. Divided by 2 is the
    // number of complete cycles.
    //
    return value;
}

//
// This is invoked from a DWEET or other means of indicating keep alive
// count than the digital input pin transistions.
//
void
WatchDogIncrementCount()
{
    int value;
    char oldSREG;

    // Enter ISR critical section
    oldSREG = SREG;
    cli();

    if (g_watchDogPinCycles < 32000) {
        g_watchDogPinCycles++;
    }
    else {
          //
          // Ensure rollover is never zero.
          //
          // Note: This must be 2 to represent a complete transition.
          // See WatchDogGetAndResetWatchDogCount().
          //
          g_watchDogPinCycles = 2;
    }

    // Leave ISR critical section
    SREG = oldSREG;
}
