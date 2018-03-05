
/*
 * Copyright (C) 2015,2016 Menlo Park Innovation LLC
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
 * Date: 01/31/2016
 * File: MenloMotionControlApp
 *
 * Application class for motion conrol (stepper, encoder).
 *
 */

#ifndef MenloMotionControlApp_h
#define MenloMotionControlApp_h

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

// Default Timer Intervals
#define MOTION_CONTROL_MINIMUM_STATE_TIME  (10)

#define MOTION_CONTROL_MINIMUM_MOTION_TIME (10)


#define DEFAULT_POWER_MODE  0x0 // Manual Power Mode
#define DEFAULT_MICROSTEP   0x0 // Full Step Mode

//
// Motion Modes
//


// The hardware configuration/application needs to be stable with no current
// flowing in the motor.
//

//
// Explicit power on, power off commands must be issued for motion.
//
const uint8_t MotionModeManualPower = 0x00;

//
// AutoPower mode will automatically power on when motion is to occur,
// and power off when motion is stopped.
//
const uint8_t MotionModeAutoPower   = 0x01;

//
// Note: No modes for open/closed loop since these can be issued
// by separate commands depending on encoder configuration
// and the application.
//


//
// Motion States
//

// Idle - no motion is occuring, power is off on Motor/FET's
const uint8_t StateIdlePowerOff                     = 0x01;

const uint8_t StateTransitionToIdlePowerOff         = 0x02;

//
// Idle - no motion is occuring, power is on Motor/FET's
// This allows the motor to "hold" torque
//
const uint8_t StateIdlePowerOn                      = 0x03;

const uint8_t StateTransitionToIdlePowerOn          = 0x04;

//
// Motion is in progress, open loop.
//
// Encoder is not being read and the programmed number of
// steps are being performed.
//
const uint8_t StateTransitionToSteppingOpenLoop     = 0x05;

const uint8_t StateSteppingOpenLoop                 = 0x06;

const uint8_t StateTransitionToSteppingClosedLoop   = 0x07;

//
// Motion is in progress, closed loop.
//
// Encoder is being read to determine if target is reached
// within the maximum steps configured.
//
const uint8_t StateSteppingClosedLoop               = 0x08;

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

#define MOTIONCONTROL_MICROSTEP_INDEX      APP_STORAGE_BASE_INDEX
#define MOTIONCONTROL_MICROSTEP_SIZE       3 // 00'\0'

#define MOTIONCONTROL_STEPRATE_INDEX       (MOTIONCONTROL_MICROSTEP_INDEX + MOTIONCONTROL_MICROSTEP_SIZE)
#define MOTIONCONTROL_STEPRATE_SIZE        9 // 00000000'\0'

#define MOTIONCONTROL_POWERMODE_INDEX      (MOTIONCONTROL_STEPRATE_INDEX + MOTIONCONTROL_STEPRATE_SIZE)
#define MOTIONCONTROL_POWERMODE_SIZE       9 // 00000000'\0'

// Note: This must be last as the checksum range depends on it
#define MOTIONCONTROL_CHECKSUM             (MOTIONCONTROL_POWERMODE_INDEX + MOTIONCONTROL_POWERMODE_SIZE)
#define MOTIONCONTROL_CHECKSUM_SIZE        2

//
// These don't have EEPROM entries, but have parameter string sizes
//
#define MOTIONCONTROL_STEP_OPENLOOP_SIZE   9 // 00000000'\0'

#define MOTIONCONTROL_STEP_CLOSEDLOOP_SIZE 9 // 00000000'\0'

// The first saved value is the start of the LIGHT checksum range
#define MOTIONCONTROL_CHECKSUM_BEGIN       APP_STORAGE_BASE_INDEX
#define MOTIONCONTROL_CHECKSUM_SIZE        2

// End of range for MOTIONCONTROL checksum is the start of the checksum storage location
#define MOTIONCONTROL_CHECKSUM_END         MOTIONCONTROL_CHECKSUM

#define MOTIONCONTROL_MAX_SIZE             MOTIONCONTROL_STEPRATE_SIZE

struct MotionControlConfiguration {

    uint8_t stepper_power;
    uint8_t stepper_direction;
    uint8_t stepper_step;
    uint8_t stepper_ms1;
    uint8_t stepper_ms2;
    uint8_t stepper_ms3;
    
    uint8_t encoder_A;
    uint8_t encoder_B;
};

//
// DweetApp allows standard signatures for event callbacks
// and ProcessAppCommands.
//
class MenloMotionControlApp : public DweetApp  {

public:

    MenloMotionControlApp() {
        m_state = StateIdlePowerOff;
        m_currentStateEndTime = 0;
        m_mode = MotionModeManualPower;
        m_steprate = 100; // 100ms per step
        m_stepdirection = true; // default to clockwise
        m_powermode = DEFAULT_POWER_MODE;
        m_microstep = DEFAULT_MICROSTEP;
        m_openLoopStepsTarget = 0;
        m_closedLoopStepsTarget = 0;
    }

    int Initialize(MotionControlConfiguration* config);

    //
    // Application command dispatcher. Invoked from DweetEvent
    // handler when Dweet's arrive for app to examine.
    //
    virtual int ProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // GET/SET property commands
    //

    int MicroStep(char* buf, int size, bool isSet);
    int StepRate(char* buf, int size, bool isSet);
    int PowerMode(char* buf, int size, bool isSet);
    int StepOpenLoop(char* buf, int size, bool isSet);
    int StepClosedLoop(char* buf, int size, bool isSet);

protected:

    // Worker routines
    // TODO: Update these to long*
    void UpdateMicroStep(uint16_t* value, bool isSet);
    void UpdateStepMode(uint16_t* value, bool isSet);
    void UpdatePowerMode(uint16_t* value, bool isSet);

    int PerformStepOpenLoop(char* buf, int size, bool isSet);
    int PerformClosedLoop(char* buf, int size, bool isSet);

    //
    // Hardware functions
    //

    virtual void HardwareInitialize() = 0;

    virtual void StepperPowerEnable(boolean on) = 0;

    virtual void StepperDirection(boolean clockwise) = 0;

    virtual void StepperReset() = 0;

    virtual long StepperStep(boolean clockwise, long steps, long delay_per_step) = 0;

    virtual long
    StepperClosedLoop(
        boolean clockwise,
        long steps,
        long delay_per_step,
        long max_steps_per_increment
        ) = 0;

    virtual long StepperGetPosition() = 0;

    virtual void EncoderReset() = 0;

    //
    // Note: Since this is a long, there could be word tear, so disable
    // interrupts for encoder read at the encoder hardware handler.
    //
    virtual long EncoderGetPosition() = 0;

    //
    // Called by encoder ISR to indicate current encoder position
    //
    // Note: Since its an ISR action may need to wait till the next
    // processing loop if there are any delays, or variable update issues.
    //
    //void EncoderUpdate(long position);

    MotionControlConfiguration m_config;

private:

    void ProcessStateEvents();

    void StartMotionTimer(unsigned long motionInterval);

    void StopMotionTimer();

    void IdleStepper();

    boolean m_stepdirection; // true is clockwise

    // State
    uint8_t m_state;

    // operating mode
    uint8_t m_mode;

    uint8_t m_powermode;

    uint8_t m_microstep;

    long m_steprate;

    long m_openLoopStepsTarget;

    long m_closedLoopStepsTarget;

    long m_currentStateEndTime;

    //
    // A timer is used to handle the state machine, motion timeouts, and watchdog
    //

    MenloTimer m_stateTimer;
    MenloTimerEventRegistration m_stateTimerEvent;

    // StateTimerEvent function
    unsigned long StateTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // A separate timer is used for active motion commands
    //

    MenloTimer m_motionTimer;
    MenloTimerEventRegistration m_motionTimerEvent;

    // MotionTimerEvent function
    unsigned long MotionTimerEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Dweet channel support over serial.
    //
    DweetSerialChannel m_dweetSerialChannel;

    // Dweet Serial Channel Event registration
    MenloDweetEventRegistration m_serialDweetEvent;

    // DweetEvent function
    unsigned long DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);
};

#endif // MenloMotionControlApp_h
