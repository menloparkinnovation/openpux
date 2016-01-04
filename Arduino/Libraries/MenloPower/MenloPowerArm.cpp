
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
 *  Date: 05/09/2015
 *  File: MenloPowerArm.cpp
 */

//
// Include Menlo Debug library support
//
#include <MenloPlatform.h>
#include <MenloUtility.h>
#include <MenloDebug.h>
#include <MenloNMEA0183.h>
#include <MenloDweet.h>

// This libraries header
#include <MenloPower.h>

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

//
// The Arduino IDE is not very great at selecting
// per platform files so we just use a #ifdef on each
// platform specific file.
//
// They are separate rather than one since that is more
// maintainable.
//

#if MENLO_ARM32

//
// MenloPower is an important low level class
// that initializes to full speed defaults in
// the constructor.
//
// Once the system is initialized, it can be configured
// for more power efficient operating modes.
//

MenloPower Power;

// Constructor
MenloPower::MenloPower()
{
    m_watchdogEnabled = false;
    m_frequencyScaleFactor = 1;
    m_sleepTime = 0;
    m_awakeTime = 0;
    m_lastSleepTimeEnded = GET_MILLISECONDS();
    m_sleepMode = MENLOSLEEP_DISABLE;
}

void
MenloPower::SetWatchdog(bool enable)
{
    return;
}

int
MenloPower::SleepMode(char* buf, int size, bool isSet)
{
    return 0;
}

int
MenloPower::CpuSpeed(char* buf, int size, bool isSet)
{
    return 0;
}

//
// Return the amount of time spent awake
//
int
MenloPower::AwakeTime(char* buf, int size, bool isSet)
{
    if (isSet) return DWEET_ERROR_UNSUP;

    // 8 digits for value + '\0'
    if (size < 9) {
        xDBG_PRINT("SleepTime buf len is less than 9");
        return DWEET_INVALID_PARAMETER;
    }
    
    MenloUtility::UInt32ToHexBuffer(m_awakeTime, buf);
    buf[8] = '\0';

    return 0;
}

//
// Return the amount of time spent sleeping
//
int
MenloPower::SleepTime(char* buf, int size, bool isSet)
{
    if (isSet) return DWEET_ERROR_UNSUP;

    // 8 digits for value + '\0'
    if (size < 9) {
        xDBG_PRINT("SleepTime buf len is less than 9");
        return DWEET_INVALID_PARAMETER;
    }
    
    MenloUtility::UInt32ToHexBuffer(m_sleepTime, buf);
    buf[8] = '\0';

    return 0;
}

int
MenloPower::SetHardwareSpeed(uint8_t oldSpeed, uint8_t newSpeed)
{
    return 0;
}

//
// Sleep for up to sleepTime.
//
// Sleep may be shorter, due to the resolution of the sleep/watchdog
// timer, or interrupts that cause an early wakeup.
//
// Applications indicate in their Poll() loop return how long they
// can sleep for till the next Poll(). Well written ones maximize this
// time to save energy, while still providing the proper service rate
// the application scenario demands.
//
void
MenloPower::Sleep(unsigned long sleepTime)
{
    return;
}

//
// Set the sleep wakeup interrupt to occur at up to sleepTime.
//
void
MenloPower::SetSleepWakeupInterrupt(unsigned long sleepTime)
{
    return;
}

#endif // MENLO_ARM32





