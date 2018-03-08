
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
 *  File: MenloPowerAvr.cpp
 */

//
// Note: The Power settings are controlled in Libraries/MenloDweet/DweetConfig.cpp
//

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

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

//
// The Arduino IDE is not very great at selecting
// per platform files so we just use a #ifdef on each
// platform specific file.
//
// They are separate rather than one since that is more
// maintainable.
//

#if MENLO_ATMEGA

//
// AVR Libraries to support power + sleep
//

// hardware/tools/avr/avr/include/avr/sleep.h
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

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

//
// Table of Watchdog timer constants
//
// values from page 54 of the data sheet
//
// WDP0 - WDP3 are defined in iom328p.h
//
// NOTE: WDP3 bit is split and at bit position #5
//
// WDP0 bit 0
// WDP1 bit 1
// WDP2 bit 2
// WDP3 bit 5
//
//  Timing configuration values from page 55-56 of data sheet.
//  Page 50 shows the diagram for the prescaler which is a 10:1
//  mux which selects each output of the watchdog prescaler.
//
//                                         WDT osc cycles
// WDP3=0, WDP2=0, WDP1=0, WDP0=0  16ms      2048   (2k)
// WDP3=0, WDP2=0, WDP1=0, WDP0=1  32ms      4096   (4k)
// WDP3=0, WDP2=0, WDP1=1, WDP0=0  64ms      8192   (8k)
// WDP3=0, WDP2=0, WDP1=1, WDP0=1  0.125s   16384   (16k)
// WDP3=0, WDP2=1, WDP1=0, WDP0=0  0.25s    32768   (32k)
// WDP3=0, WDP2=1, WDP1=0, WDP0=1  0.5s     65536   (64k)
// WDP3=0, WDP2=1, WDP1=1, WDP0=0  1.0s    131072  (128k)
// WDP3=0, WDP2=1, WDP1=1, WDP0=1  2.0s    262144  (256k)
// WDP3=1, WDP2=0, WDP1=0, WDP0=0  4.0s    524288  (512k)
// WDP3=1, WDP2=0, WDP1=0, WDP0=1  8.0s   1048576 (1024k)
//
PROGMEM const uint8_t watchdog_timer_table[] =
{
    0,                         // 16ms
    (1 << WDP0),               // 32ms
    (1 << WDP1),               // 64ms
    (1 << WDP1) | (1 << WDP0), // 0.125s
    (1 << WDP2),               // 0.25s
    (1 << WDP2) | (1 << WDP0), // 0.5s
    (1 << WDP2) | (1 << WDP1), // 1.0s
    (1 << WDP2) | (1 << WDP1) | (1 << WDP0), // 2.0s
    (1 << WDP3),               // 4.0s
    (1 << WDP3) | (1 << WDP0)  // 8.0s
};

//
// <avr/wdt.h>
//
// Note: These are values specific to the macro's used
// by avr/wdt.h and don't represent real bit positions
// since bit 3 in the following values is separated out
// and used to define bit 5.
//
// WDTO_30MS  1 // actually 31.25 MS
// WDTO_60MS  2 // actually 62.5 MS
// WDTO_120MS 3 // actually 125 MS
// WDTO_250MS 4
// WDTO_500MS 5
// WDTO_1S    6
// WDTO_2S    7
// WDTO_4S    8
// WDTO_8S    9
//

//
// MenloPower is an important low level class
// that initializes to full speed defaults in
// the constructor.
//
// Once the system is initialized, it can be configured
// for more power efficient operating modes.
//

MenloPower Power;

//
// The configured Watchdog Timer's interrupt vector
//
ISR(WDT_vect)
{
    //
    // We do not need to do anything here since its
    // just used to bump the processor awake from sleep.
    //
}

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
    if (enable) {

        if (m_watchdogEnabled) {
            // No change of state
            return;
        }

        m_watchdogEnabled = true;
        wdt_enable(WDTO_8S);
    }
    else {

        if (!m_watchdogEnabled) {
            // No change of state
            return;
        }

        m_watchdogEnabled = false;
        wdt_disable();
    }
}

//
// This is invoked by the Dweet handler in Libaries/MenloDweet/DweetConfig.cpp
//
// MENLOSLEEP_MODE_DISABLE is set as SETCONFIG=SLEEPMODE:0
// MENLOSLEEP_MODE_IDLE is set as SETCONFIG=SLEEPMODE:1
//
int
MenloPower::SleepMode(char* buf, int size, bool isSet)
{
    uint8_t mode;

    if (isSet) {

        if (strlen(buf) < 2) {
            xDBG_PRINT("SleepMode mode len is less than 2");
            return DWEET_INVALID_PARAMETER;
        }

        mode = MenloUtility::HexToByte(buf);

        // Mode must be one of the supported ones for this CPU/application
        switch (mode) {
    
             case MENLOSLEEP_DISABLE:
             case MENLOSLEEP_MODE_IDLE:

                // Sleep mode will take effect at the next sleep
                m_sleepMode = mode;

                xDBG_PRINT_NNL("SleepMode ");
                xDBG_PRINT_INT(mode);

                return 0;
                break;

             default:
                xDBG_PRINT("SleepMode unsupported value");
                xDBG_PRINT_NNL("mode ");
                xDBG_PRINT_INT(mode);
                return DWEET_INVALID_PARAMETER;
        }
    }
    else {
        mode = m_sleepMode;

        if (strlen(buf) < 3) {
            xDBG_PRINT("SleepMode set len is less than 3");
            return DWEET_INVALID_PARAMETER;
        }

        MenloUtility::UInt8ToHexBuffer(mode, buf);
        buf[2] = '\0';

        return 0;
    }
}

int
MenloPower::CpuSpeed(char* buf, int size, bool isSet)
{
    int retVal;
    uint8_t speed;
    uint8_t oldSpeed;

    if (isSet) {

        if (strlen(buf) < 2) {
            xDBG_PRINT("CpuSpeed mode len is less than 2");
            return DWEET_INVALID_PARAMETER;
        }

        speed = MenloUtility::HexToByte(buf);

        //
        // Attempt to set the new frequency
        //
        xDBG_PRINT_NNL("CpuSpeed ");
        xDBG_PRINT_INT(speed);

        oldSpeed = m_frequencyScaleFactor;
        m_frequencyScaleFactor = speed;

        retVal = SetHardwareSpeed(oldSpeed, speed);
        if (retVal != 0) {

            //
            // On failure return the old setting.
            // SetHardwareSpeed should not have modified the hardware,
            // or reverted its changes.
            //
            m_frequencyScaleFactor = oldSpeed;

            return retVal;
        }

        return retVal;
    }
    else {
        speed = m_frequencyScaleFactor;

        if (strlen(buf) < 3) {
            xDBG_PRINT("CpuSpeed set len is less than 3");
            return DWEET_INVALID_PARAMETER;
        }

        MenloUtility::UInt8ToHexBuffer(speed, buf);
        buf[2] = '\0';

        return 0;
    }
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
    //
    // Note this is a virtual allowing a CPU specific
    // subclass to override.
    //

    //
    // speed must be within range
    //
    // See page 399 of AtMega328 data sheet
    //
    switch (newSpeed) {
         case 1:   // 8Mhz, 3.1ma at 3.3v
         case 2:   // 4Mhz
         case 4:   // 2Mhz
         case 8:   // 1Mhz  0.5ma at 3.3v
             break;

         default:
            xDBG_PRINT("CpuSpeed invalid scale value");
            xDBG_PRINT_NNL("speed ");
            xDBG_PRINT_INT(newSpeed);
            return DWEET_INVALID_PARAMETER;
    }

    //
    // TODO: Set CPU control register
    //       Set SPI, USART, TIMER0 registers
    //

    //
    // May need to implement clock scaling for
    // millis(), delay(), etc. if Timer 0 clock
    // is not updated. May still be problem with
    // library constants expecting certain rates
    // especially so for micros().
    //

    // Currently only allow setting (staying) at 1X
    if (newSpeed == 1) {
        return 0;
    }

    return DWEET_ERROR_UNSUP;
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
    unsigned long sleepStart;

    //
    // If less than minimum sleep time return to the main processing loop.
    //
    if (sleepTime < MENLO_MINIMUM_SLEEP_TIME) {
        return;
    }

    sleepStart = GET_MILLISECONDS();

    // Account for the awake time
    m_awakeTime += (sleepStart - m_lastSleepTimeEnded);

    //
    // AtMega328 data sheet page numbers are referenced where applicable.
    //
    // page 37 clock prescale register controls operating frequency
    //
    // page 39 Power Management and sleep modes
    //

    //
    // iom328p.h  Sleep Mode Control Register
    //            Page 44 of data sheet
    //
    // #define SMCR _SFR_IO8(0x33)
    // #define SE 0   // Sleep Enable
    // #define SM0 1  // Sleep mode select 0
    // #define SM1 2  //     ""            1
    // #define SM2 3  //     ""            2
    //

    //
    // iom328p.h  MCU Control Register
    //            Page 44 of data sheet
    //
    // #define MCUCR _SFR_IO8(0x35)
    // #define IVCE 0
    // #define IVSEL 1
    // #define PUD 4
    // #define BODSE 5  // Brown out Detector Sleep Enable (BODSE)
    // #define BODS 6   // Brown Out Detector Sleep (BODS)

    // 

    //
    // sleep.h defines the following sleep modes:
    //
    // IDLE, ADC, PWR_DOWN, PWR_SAVE, STANDBY, EXT_STANDBY
    //

    //
    // The following are the Menlo defined sleep modes and behavior
    // for AtMega328.
    //
    // Since the AtMega328 has a flexible array of power modes
    // and control over various subsystems, clock rates, etc.,
    // synthetic sleep modes are created that represent useful
    // combinations. These represent useful combinations in which
    // various MenloFramework provided mitigations are in place
    // such as when clocks stop, change frequency, etc.
    //
    // Applications of course can add additional modes. To prevent
    // conflict with external configuration utilities they should
    // start with MENLOSLEEP_APPLICATION_CUSTOM_BASE (128) defined
    // in MenloPower.h
    //

    if (m_sleepMode == MENLOSLEEP_DISABLE) {

        //
        // m_sleepMode 0 MENLOSLEEP_DISABLE:
        //    CPU always running at full configured speed.
        //    CPU does not sleep.
        //    All subsystems are fully powered on and operating based
        //    on application configuration.
        //

        // do nothing, return
        return;
    }
    else if (m_sleepMode == MENLOSLEEP_MODE_IDLE) {

        //
        // m_sleepMode 1 MENLOSLEEP_MODE_IDLE:
        //
        //   AtMega IDLE mode, page 40 of data sheet
        //   CPU stopped.
        //
        //   The following subsystems are still running:
        //
        //   Pins and ports:   interrupts are enabled
        //   Timer 0:          millis()/delay(), etc. work as normal
        //   PWM Timers 1 + 2: analogWrite()
        //   SPI running:      radio, wifi
        //   ADC running:      analogRead()
        //   TWI running:      Two wire devices, two wire detect/wakeup
        //
        // IDLE mode Uses 1/5 of normal operating power.
        //
        // Operating current at 8mhz, 3.3v is 3.1ma
        //
        // IDLE mode current is approx. 0.62ma
        //
        // Since Timer 0 is running it will interrupt in 2.048ms
        // giving short sleep times which will impact percentage time
        // sleeping.
        //
        // As a result of Timer 0 running the watchdog interrupt is
        // not required to awake from sleep mode.
        //
        // Since all subsystems are running, millis(), delay(), micros(),
        // all operate as normal and no adjustments are required when
        // transitioning between sleep and wakeup.
        //

        set_sleep_mode(SLEEP_MODE_IDLE);
    }
    else if (m_sleepMode == MENLOSLEEP_MODE_PWRDOWN) {

        //
        // m_sleepMode 2:
        //
        // AtMega POWER DOWN mode, page 41 of data sheet
        //
        // CPU stopped, timers stopped, ADC stopped, SPI stopped.
        //
        // Clock must be re-adjusted on wakeup.
        //

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    }
    else if (m_sleepMode == MENLOSLEEP_MODE_PWRSAVE) {

        //
        // m_sleepMode 2:
        //
        // AtMega POWER SAVE mode, page 41 of data sheet
        //
        // CPU stopped, timers stopped except for timer 2, ADC stopped.
        //
        // This is useful if Timer 2 is outputting PWM such as a LED/light.
        //

        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    }
    else if (m_sleepMode == MENLOSLEEP_MODE_STANDBY) {

        //
        // AtMega STANDBY mode, page 41 of data sheet
        //
        // Same as POWER DOWN, but the external oscilator is kept running
        //

        set_sleep_mode(SLEEP_MODE_STANDBY);
    }
    else if (m_sleepMode == MENLOSLEEP_MODE_EXTSTANDBY) {

        //
        // AtMega EXT-STANDBY mode, page 41 of data sheet
        //
        // Same as POWER SAVE, but the external oscilator is kept running
        //

        set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
    }
    else {

        //
        // Unsupported mode.
        //
        // Use the most conservative sleep mode that does not
        // require device or timer mitigations.
        //

        set_sleep_mode(SLEEP_MODE_IDLE);
    }

    //
    // ADC Noise Reduction Mode
    //
    // Page 40 of the data sheet.
    //
    // Special mode for high accuracy ADC samples.
    //
    // Entering the mode starts an ADC conversion when the main digital
    // clocks are stopped.
    //
    // It can be configured to generate an ADC conversion interrupt to
    // wakeup when an ADC conversion is complete.
    //
    // CPU stops, Timer Counters 0 + 1 stop. Timer Count 2 continues to run.
    //

    //set_sleep_mode(SLEEP_MODE_ADC);

    //
    // Power Reduction Register (PRR)
    //
    // Page 45 of the data sheet.
    //
    // Can selectively turn off Timer 0, Timer 1, Timer 2,
    // two wire interface (TWI), SPI, USART0, and ADC.
    //
    // Powering off TWI, SPI, or USART0 requires full re-initialization
    // on power on as all state is lost.
    //
    // power.h contains the following macro's for the Power Reduction Register:
    //
    // // iom328p.h
    // #define PRR _SFR_MEM8(0x64)
    //
    // #define PRADC 0
    // power_adc_disable();
    // power_adc_enable();
    //
    // #define PRUSART0 1
    // power_usart0_disable()
    // power_usart0_enable()
    //
    // #define PRSPI 2
    // power_spi_disable()
    // power_spi_enable()
    //
    // #define PRTIM1 3
    // power_timer1_disable()
    // power_timer1_enable()
    //
    // #define PRTIM0 5
    // power_timer0_disable()
    // power_timer0_enable()
    //
    // #define PRTIM2 6
    // power_timer2_disable()
    // power_timer2_enable()
    //
    // #define PRTWI 7
    // power_twi_disable()
    // power_twi_enable()
    //
    // power_all_disable();
    // power_all_enable();
    //

    //
    // Brown Out Disable (BOD) Page 40 of data sheet.
    //
    // Optional disable of brown out detector to save
    // even more power. This is really only useful for the deeper sleep
    // modes with lots of clocks and on chip resources disabled before
    // its small (microamps) current consumption become meaningful.
    //
    // To disable the brown out detector it must also be written
    // with the Brown Out Sleep Enable (BODSE) bit which is a timed
    // sequence required to prevent accidental setting of BODS when
    // a CPU is losing power or software runaway.
    //
    // Page 44 of the data sheet for MCUCR describes the timing sequence.
    //
    // // First must set both bits
    // MCUCR |= (1 << BODS) | (1 << BODSE);
    //
    // // Then must just set the targeted bit with BODSE == 0
    // MCUCR &= ~(1 << BODSE);
    //

    //
    // Enter sleep
    //
    // This executes the following sequence as a macro in sleep.h
    //
    // sleep_enable();
    //   SMCR |= (uint8_t)_BV(SE);
    //
    // sleep_cpu();
    //   __asm__ __volatile__ ( "sleep" "\n\t" :: );
    //
    // sleep_disable();
    //   SMCR &= (uint8_t)(~_BV(SE));
    //

    //
    // sleep.h
    //
    // #define sleep_mode() \
    // do {                 \
    //     sleep_enable();  \
    //     sleep_cpu();     \
    //     sleep_disable(); \
    // } while (0)
    //
    //
    sleep_mode();

    //
    // Now we have woken up as a result of an interrupt.
    //
    // The interrupt handler has executed and returned before we
    // get to the instruction after sleep_mode();
    //
    // Interrupt sources are:
    //
    //   application configured pins from devices, etc.
    //
    //   Timer 0 if enabled (MENLOSLEEP_MODE_IDLE)
    //
    //   Watchdog timer interrupt (WDT)
    //

    //
    // Disable sleep
    //

    // sleep.h
    //   SMCR &= (uint8_t)(~_BV(SE));
    sleep_disable();

    //
    // wdt.h
    //
    // #define wdt_reset() __asm__ __volatile__ ("wdr")
    //

    //
    // Reset the watchdog timer right away incase Watchdog reset is enabled
    // and we are on a short interval
    //
    wdt_reset();

    //
    // Sleep modes deeper than IDLE use the watchdog timer to trigger
    // the wakeup interrupt.
    //
    if (m_sleepMode > MENLOSLEEP_MODE_IDLE) {

        //
        // Watchdog Reset Timer, page 50 of the data sheet.
        //

        //
        // Must first reset the watchdog timer and re-enable it
        // for maximum time (8S) because we are using dual interrupt and
        // reset mode. We may have selected a short sleep interval and received
        // the first timer interval interrupt, but this has armed a watchdog
        // reset for the next timer interval if we don't re-program it right away.
        //

        //
        // wdt.h
        //
        // #define wdt_enable(value)   \
        // __asm__ __volatile__ (  \
        //     "in __tmp_reg__,__SREG__" "\n\t"    \
        //     "cli" "\n\t"    \
        //     "wdr" "\n\t"    \
        //     "sts %0,%1" "\n\t"  \
        //     "out __SREG__,__tmp_reg__" "\n\t"   \
        //     "sts %0,%2" "\n\t" \
        //     : /* no outputs */  \
        //     : "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
        //     "r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)), \
        //     "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) | \
        //         _BV(WDE) | (value & 0x07)) ) \
        //     : "r0"  \
        // )
        //

        // Set our maximum 8S watchdog counter
        if (m_watchdogEnabled) {
            wdt_enable(WDTO_8S);
        }

        //
        // Re-enable power to various subsystems if any are disabled based on power mode
        //
        // power.h
        //
        // #define power_all_enable() 
        // (PRR &= (uint8_t)
        // ~((1<<PRADC)|(1<<PRSPI)|(1<<PRUSART0)|(1<<PRTIM0)|(1<<PRTIM1)|(1<<PRTIM2)|(1<<PRTWI)));
        //
        power_all_enable();
    }

    // Get the time sleep ended
    m_lastSleepTimeEnded = GET_MILLISECONDS();

    // Account for the sleep time
    m_sleepTime += (m_lastSleepTimeEnded - sleepStart);

    //
    // Note: Timers could get stopped depending on the sleep mode.
    //
    // TODO:
    //
    // For sleep modes in which the timer is stopped the code needs
    // to "bump" the timer forward for the amount that was slept.
    //
    // This is even more complicated by the fact that sleep could be
    // woken early due to an interrupt.
    //
    // TODO: Can the watchdog countdown timer be read to indicate the
    // actual sleep time?
    //
    // MenloPlatform::UpdateClock(sleepTimeInterval);
    //

    //
    // On wakeup the main event processing loop will run
    // through the loop giving each component a chance to indicate
    // how long it can sleep.
    //

    return;
}

//
// Set the sleep wakeup interrupt to occur at up to sleepTime.
//
void
MenloPower::SetSleepWakeupInterrupt(unsigned long sleepTime)
{

    //
    // If m_sleepMode == MENLOSLEEP_MODE_IDLE the
    // Timer0 still runs interrupting every 2.02 ms for
    // the standard timer.
    //
    // In this case the watchdog timer is not used to
    // wake from sleep, but the standard watchdog reset
    // function still applies.
    //
    if (m_sleepMode <= MENLOSLEEP_MODE_IDLE) {
        return;
    }
 
    //
    // Deeper sleep modes require the watchdog timer to interrupt
    // the CPU to bring it out of sleep.
    //
    // The built in Watchdog timer is used for this interrupt.
    //

    //
    // Watchdog RESET vs. Sleep Wakeup interrupt
    //
    // Notes: page 54/55 of the data sheet describe wakeup interrupt and
    // reset modes of the watchdog.
    //
    // If both WDIE and WDE are enabled the first watchdog timeout
    // will invoke the interrupt vector and clear the WDIE bit
    // leaving the WDE set. If the watchdog timer is not reset by
    // the next interval a watchdog reset will occur.
    //
    // If the main application loop sets the WDIE again the next
    // watchdog timeout will cause the interrupt again. This is because
    // WDIE + WDE means clear the interrupt first, and if not set perform
    // reset since only WDE is set during watchdog timeout.
    // 
    // For the menlo loop the watchdog timer is reprogrammed for interrupt
    // mode for sleeping various intervals. Upon wakeup it will immediately
    // reset the watchdog timer to maximum 8 seconds and normal wdt_reset()
    // code in the application and framework while executing keeps the reset
    // from occuring. When sleep goes to occur again the interrupt
    // is re-enabled.
    // 

    //
    // Program the watchdog timer to generate an interrupt up to
    // the configured time interval.
    //
    // The watchdog timer is on page 50 of the Atmega328 data sheet.
    //
    // The watchdog timer has its own dedicated 128khz on chip oscillator.
    //
    // The watchdog pre-scaler divides this clock into a range from
    // clock/2k -> clock/1024k
    //
    // The selectable WDP0 - WDP3 tap the lower of these lines
    // with WDP3 == clock / 128k and WDP0 == clock / 1024k
    //
    // WDR - Watchdog Timer Reset instruction restarts the counter.
    //
    // There is an interrupt + system reset mode which can be used
    // to wakeup and perform watchdog functions.
    //
    // WDCE - Watchdog Change Enable Bit
    //        To ensure you mean to make the register change
    //
    // page 51 gives careful timing and bit sequences to change
    // this timer that are designed to minimize accidental changes by
    // a run away program.
    //

    // Disable watchdog reset which is used when not sleeping
    wdt_disable();

    //
    // Disable interrupts around the critical region when
    // manipulating watchdog and CPU control registers.
    //
    // Manipulating the watchdog control registers have careful
    // timing requirements and we must prevent state tearing of these
    // registers by an interrupt inbetween.
    //

    noInterrupts();

    //
    // MCU Status Register _SFR_IO8(0x34)
    //
    // page 54 of data sheet.
    //
    // Watchdog System Reset Flag WDRF bit 3
    //

    //
    // Clear WatchDog Reset flag (WDRF)
    //
    // This must be cleared first before clearing WDE as per page
    // 55 of the data sheet.
    //
    MCUSR &= ~(1 << WDRF);

    //
    // Watchdog Timer Control Register (WDTCSR)
    // _SFR_MEM8(0x60)
    //
    // page 54 of data sheet
    //

    //
    // Set Watchdog Change Enable (WDCE) and Watchdog Enable (WDE)
    // WDE is bit 3
    // WDCE is bit 4
    //
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    //
    // Calculate the sleep time based on the requested
    // time. The time is rounded down were required.
    //

    //
    // We will use 31.25ms (128,000 / 4096) as our lowest sleep time.
    //

    //
    // To calculate register setting for sleep time:
    //
    // divide sleepTime_in_millis / 32_in_millis
    //

    //
    // Find highest bit, use its position as an index into
    // a table of settings. This relies on the fact that
    // each setting is a power of 2.
    //

    if (sleepTime > 8000L) {
        // Set for maximum 8 seconds
        WDTCSR = (1 << WDP0) | (1 << WDP3);
    }
    else {
        unsigned long tmp;
        unsigned long mask;
        uint8_t bitIndex;
        uint8_t value;

        //
        // 8192 == bit position 13
        // remove lowest value (16ms) and bit 9 is highest position
        //
        tmp = sleepTime << 4;

        //
        // Find highest bit set in tmp
        //
        // No bits are greater than position 9 due to check above
        //
        mask = 0x00000200L;
        for (bitIndex = 9; bitIndex > 0; bitIndex--) {

            if ((tmp & mask) != 0) {
                break;
            }

            mask >> 1;
        }

        // bitIndex represents table entry index
        value = pgm_read_byte(&watchdog_timer_table[bitIndex]);

        WDTCSR = value;
    }

    // Set Watchdog Interrupt and Reset Enable
    WDTCSR |= ((1 << WDIE) | (1 << WDE)); // bit 6 + bit 3

    // Re-enable interrupts
    interrupts();

    //
    // Watchdog is now armed for an immediate Sleep() call.
    //
}

#endif // MENLO_ATMEGA
