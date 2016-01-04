
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
 *  Date: 04/21/2015
 *  File: MenloPower.h
 *
 *  MenloPower provides an abstract class interface for power management.
 *  embedded radios.
 *
 */

#ifndef MenloPower_h
#define MenloPower_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#include <inttypes.h>
#endif

#include "MenloObject.h"
#include "MenloDispatchObject.h"
#include "MenloTimer.h"

class MenloPower {

public:
  
    // Constructor performs initialization
    MenloPower();

    void Sleep(unsigned long sleepTime);

    //
    // Properties programmed by Dweet's and power on configration
    //
    int SleepMode(char* buf, int size, bool isSet);

    // Return the amount of time spent awake
    int AwakeTime(char* buf, int size, bool isSet);

    // Return the amount of time spent sleeping
    int SleepTime(char* buf, int size, bool isSet);

    void SetWatchdog(bool enable);

    //
    // CpuSpeed is a pre-scale factor from 1 - 255.
    //
    // Typical valid ranges for an Atmega 328 running at
    // 8Mhz would be 1, 2, 4, 8 to support 8Mhz, 4Mhz,
    // 2Mhz, and 1Mhz.
    //
    int CpuSpeed(char* buf, int size, bool isSet);

protected:

    // Invoked when a change in state occurs
    virtual int SetHardwareSpeed(uint8_t oldSpeed, uint8_t newSpeed);

private:

    bool m_watchdogEnabled;

    // Setup the sleep wakeup interrupt
    void SetSleepWakeupInterrupt(unsigned long sleepTime);

    // Sleep mode controls power reduction actions, etc.
    uint8_t m_sleepMode;

    //
    // If not 1 this is the factor in which the frequence is scaled down
    // Example: 8Mhz base startup clock, an 8 scale factor is 1Mhz operation.
    //
    uint8_t m_frequencyScaleFactor;

    // Total time with CPU sleeping
    unsigned long m_sleepTime;

    // Total time with CPU running
    unsigned long m_awakeTime;

    // Time since last slept
    unsigned long m_lastSleepTimeEnded;
};

// 31.25ms on Atmega328
#define MENLO_MINIMUM_SLEEP_TIME    (128000L / 4096L)

// No sleep occurs
#define MENLOSLEEP_DISABLE           0

// sleep with interrupts enabled, timer running, ADC enabled, etc.
#define MENLOSLEEP_MODE_IDLE         1

// interrupts enabled, timer stopped
#define MENLOSLEEP_MODE_PWRDOWN      2

// interrupts enabled, all timers by timer 2 stopped
#define MENLOSLEEP_MODE_PWRSAVE      3

#define MENLOSLEEP_MODE_STANDBY      4

#define MENLOSLEEP_MODE_EXTSTANDBY   5

// This does not actually sleep, but reduces CPU clock dynamically instead
#define MENLOSLEEP_LOW_CLOCK_RATE0   6

// Second clock rate selection if platform supports it
#define MENLOSLEEP_LOW_CLOCK_RATE1   7

#define MENLOSLEEP_MODE_MAX          7

#define MENLOSEEP_APPLICATION_CUSTOM_BASE 128

// Single global instance
extern MenloPower Power;

#endif // MenloPower_h
