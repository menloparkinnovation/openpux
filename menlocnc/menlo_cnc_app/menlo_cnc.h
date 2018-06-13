
//
// 06/04/2018
//
// TODO:
//
// Provide registers that return basic configuration such
// as frequency of clock, scale factors, etc.
//
// Add registers that indicate installed options such as encoders,
// multi-axis control, soft processors, DMA, direct TCP/UDP, etc.
//
// Add pulse position accumulator registers that can operate even
// in open loop controllers to allow software to track machine
// position. These are added/subtracted as pulse and direction
// commands are executed against the machine. They can be reset
// by software when the machine is homed.
//
// When encoders are supported and available two sets of registers
// would represent estimated/commanded position and actual position
// from the encoders.
//

//
// Hardware Interface for Menlo CNC Controller.
//
// Copyright (c) 2018 Menlo Park Innovation LLC
//
// Designed to compile as pure C in various embedded software environments
// from embedded Linux, real time Linux, RTAI co-kernels, or soft processors
// such as Nios II on an FPGA.
//
// 05/17/2018
//

// 
// The MIT License (MIT)
// 
// Copyright (c) 2018 Menlo Park Innovation LLC
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 

//
// This is taken from LinuxCNC and applies here:
//
// https://github.com/LinuxCNC/linuxcnc/blob/master/README.md
//
// THE AUTHORS OF THIS SOFTWARE ACCEPT ABSOLUTELY NO LIABILITY FOR ANY HARM OR LOSS RESULTING FROM ITS USE.
// 
// IT IS EXTREMELY UNWISE TO RELY ON SOFTWARE ALONE FOR SAFETY.
// 
// Any machinery capable of harming persons must have provisions for completely
// removing power from all motors, etc, before persons enter any danger area.
// 
// All machinery must be designed to comply with local and national safety
// codes, and the authors of this software can not, and do not, take any
// responsibility for such compliance.
// 

//
// Microprocessor register interface for timing generator.
//
// This is mapped into the memory mapped I/O address space
// of either the ARM SoC's or the Nios II processor and
// presents a register file for loading commands into
// the timing generator FIFO's.
//
// The interface is organized in 16 byte blocks of four
// 32 bit words, so it may be transferred over a communications
// link such as an on-chip streaming DMA, DMA over a host PCI
// bus, or over a network using a block oriented wrapper
// protocol.
//
// The organization also allows ready storage in sequential
// memory addresses making code to feed the FIFO's during
// real time machining operations simple so that relatively
// slow "soft" processors such as Nios II may be used.
//
// Some configurations may use streaming DMA from sequential
// memory addresses to load the FIFO's and this general
// layout is compatible with such implementations.
//

//
// Microcontroller Programing Interface for timing generator:
//
// Provides a memory mapped 32 bit register file.
//
// Registers are little endian.
//
// Non-32 bit operations are not supported. They will read
// as 0, and writes are ignored.
//
// Reserved addresses read 0, writes ignored.
//
// Register map:
//
//  Higher Address
//
//   Maximum address 128 for 4 axis with reservation
//   for growth to 7 axis.
//    7 bit address decode.
//
// Implementation Note: 7 bit address decode is used
// to reserve the register file space for axis growth,
// and possible addition of registers. 8 bits is not used
// in order to not overly grab decode space.
//  
//
// Overall software model:
//
// Microcontroller writes values to the four axis control registers
// using 32 bit memory writes. Any axis not part of the command needs
// to be set to the DWELL command so as not to initiate motion.
//
// If the FIFO status is not full, the software sets the commmand register
// CMD bit and it transfers all axis commands into the axis command
// FIFO's in the same clock cycle. This is important to ensure no axis
// starts early, and would be out of synchronization with the others.
//
// Software may continue this sequence of writing commands to
// the FIFO's until its full, thus buffering commands in
// a pipeline to ensure there not a pause in machine motion.
//
// Software is expected to operate in "real time" to
// ensure the FIFO never becomes empty when commands are
// stored in the command buffer SDRAM/DRAM. Depending on FPGA
// space, additional FIFO depth may be configured in the Verilog
// portions of the project in command_fifo.sv.
//
// A dedicated Nios II real time processor is a recommended option,
// with storage of commands in the 64MB SDRAM directly connected to
// the FPGA on the DE10-Standard. This is a planned future option.
//
// It's possible to operate the timing generator writing from
// the ARM SoC cores running Linux using the 512 byte depth
// of the FIFO's. But Linux and ARM SoC cores are subject
// to uncontrolled interrupts with no worse case time
// specifications. This may be done for "jogging" style
// commands that are not for time sensitive part cutting
// motions in which an unexpected pause may be "seen" on
// the part due to non-smooth motion.
//
// Using Linux real-time extensions on the SoC is an option,
// provided the uncontrolled state changes of the ARM SoC's
// are not longer than the FIFO depth, the length of
// the real time Linux service time, and the time required
// to get commands into the FIFO.
//
// It is a planned option to provide Linux real time extensions
// on the SoC, with one SoC core dedicated to a high priority
// Linux real time process. This may be needed when running LinuxCNC
// as the motion planner in real time.
//
// The preferred model is to use the ARM SoC's to fill
// the 64MB SDRAM command buffer, and let a Nios II
// real time processor keep the command FIFO's full.
// This provides a very large buffer for any timing pauses that
// may be caused by higher level software, or SoC function.
// Once commands are loaded to the FPGA FIFO's, or the SDRAM,
// the FPGA itself will have no pauses, other than for machine/position
// errors or ESTOP.
//
// A future version may offer a streaming DMA interface
// from the SDRAM to the FIFO buffers to not have any
// processor in the path. This DMA engine would be
// controlled by additional registers to be added later,
// likely at the top of the register address space, or
// with an 8 bit decode.
//
// Additional command register bits control enable, software
// initiated emergency stop, reset, and command FIFO clear.
//
// The status register gives real time indication from the
// timing generator and the state of the command FIFO's in
// addition to the emergency stop status, including the
// external hardware ESTOP.

//                              128
// 31                         0
//  +-------------------------+
//  |       Reserved          | 124
//  +-------------------------+
//             ...
//  +-------------------------+
//  |       Reserved          | 80
//  +-------------------------+
//  |  A axis instruction     | 76
//  +-------------------------+
//  |  A axis pulse width     | 72
//  +-------------------------+
//  |  A asis pulse count     | 68
//  +-------------------------+
//  |  A axis pulse rate      | 64
//  +-------------------------+
//  |  Z axis instruction     | 60
//  +-------------------------+
//  |  Z axis pulse width     | 56
//  +-------------------------+
//  |  Z asis pulse count     | 52
//  +-------------------------+
//  |  Z axis pulse rate      | 48
//  +-------------------------+
//  |  Y axis instruction     | 44
//  +-------------------------+
//  |  Y axis pulse width     | 40
//  +-------------------------+
//  |  Y asis pulse count     | 36
//  +-------------------------+
//  |  Y axis pulse rate      | 32
//  +-------------------------+
//  |  X axis instruction     | 28
//  +-------------------------+
//  |  X axis pulse width     | 24
//  +-------------------------+
//  |  X asis pulse count     | 20
//  +-------------------------+
//  |  X axis pulse rate      | 16
//  +-------------------------+
//  |  Reserved1. Reads 0.    | 12
//  +-------------------------+
//  |  Reserved0. Reads 0.    |  8
//  +-------------------------+
//  |  Write Command Register |  4 (See details below)
//  +-------------------------+
//  |  Read Status Register   |  0 (See details below)
//  +-------------------------+
// 31                         0
//
//  Lower Address, Relative base 0
//
//  Details of the Instruction Register for each Axis
//
// 31     ...   6 5 4 3 2 1 0
//  +-------------------------+
//  | R                 R I D |
//  | S   ...           S N I |
//  | V                 V S R |
//  +-------------------------+
// 31                         0
//
// DIR - Direction. Clockwise == 0, Counter Clockwise == 1
//
// INS - Instruction. Generate Pulse (movement) == 1
//       IDLE, DWELL == 0
//
//       With IDLE/DWELL the command still takes the programmed
//       amount of time as if == 1, but does not generate any
//       movement pulses.
//
//       This is used to idle an axis while the others are moving.
//
//  Details of the Status and Command Registers
//
//  Read Status Register:
//
//  Status register is read only and ignores writes.
//
//  Status bits are real time, and not impacted by reads.
//  No "sticky" bits.
//
// 31     ...   6 5 4 3 2 1 0
//  +-------------------------+
//  | R         R E I B E F F |
//  | S   ...   S M D S R B B |
//  | V         V S L Y R E F |
//  +-------------------------+
// 31                         0
//
// FBF - FIFO Buffer Full when == 1
//   Can't write more commands till clear.
//
// FBE - FIFO Buffer Empty when == 1
//   Indicates no more commands in FIFO, but timing
//   generator may still be operating on the last command
//   util BSY == 0.
//
// ERR - Error indication from timing generator when == 1.
//   Real time status indicator of the error signal from
//   the timing generator.
//  
// BSY - Timing generator busy when == 1
//   Real time status indicator of the busy signal from
//   the timing generator.
//
//   This is 1 when the timing generator is performing an
//   operation.
//
//   True idle state is when (FBE == 1) && (BSY == 0) which
//   means all commands are completed.
//
//   Note that BSY can transition in real time as new commands
//   are fed from the FIFO. There is a two clock delay as the
//   last command is read from the FIFO and the timing generator
//   raises the busy signal. Use the IDL signal to indicate
//   truly that all commands have completed without any
//   race conditions.
//
// IDL - Timing generator idle when == 1
//   Gated signal that is set when (FBE == 1) && (BSY == 0)
//   and all internal clock delays have cleared.
//
//   This is the hardware timing reliable signal for software
//   to determine all commands have completed execution, and
//   the timing generator has truly idled the machine with no
//   more motion commands in flight.
//
// EMS - Emergency Stop when == 1
//
//  Emergency Stop is an OR of the external ESTOP signal
//  and the software driven EMS signal in the command
//  register.
//
//  This is set if either ESTOP source is active.
//
// RSV - Reserved. Reads 0, writes ignored.
//
//  Write Command Register:
//
//  Write Command register can be written and read.
//
// 31       ...   5 4 3 2 1 0
//  +-------------------------+
//  | R           R C E E C R |
//  | S     ...   S M M A B S |
//  | V           V D S N F T |
//  +-------------------------+
// 31                         0
//
//
// RST - Reset when == 1
//  Resets the timing engine and FIFO's.
//  TODO: Ensure FIFO's are reset.
//  reg_clear_buffer is existing register signal set to 0.
//
//  The value read is the last set RST value.
//
// CFB - Clear buffers when == 1
//  Clears the FIFO buffers when set to 1.
//
//  TODO: Ensure FIFO's are reset.
//  TODO: Not currently implemented.
//
//  This register reads as 0.
//
// EAN - Enable when == 1
//  Enables the timing generator command execution.
//
//  May be used to pause command execution by setting to 0,
//  and resume by setting back to 1.
//
//  This is a real time signal to the timing engine.
//
//  The value read is the last set EAN value.
//
// EMS - Emergency Stop when == 1
//
//  Software driven emergency stop command.
//
//  OR'd with external ESTOP signal. A software, or external
//  ESTOP signal generates ESTOP.
//
//  Read the EMS Status register to ensure external ESTOP
//  switch is not set when this register is clear.
//
//  This is a real time signal to the timing engine.
//
//  The value read is the last software set EMS value. To see
//  the total machine EMS state read the status register
//  EMS bit.
//
// CMD - Command write register when == 1.
// CMD == 1
//
// When set to 1 the values in the Axis registers
// are transferred to the timing generator FIFO's.
//
// Command execution begins immediately if EAN == 1
// and (EMS == 0).
//
// If paused, command execution begins when the above
// conditions allow command execution.
//
// This register always reads as 0, and does not
// hold the CMD setting.
//
// This is an important protocol control bit as it allows multiple
// axis to be updated by register writes to their settings, but the
// command is not transferred to the FIFO buffers till this bit is
// written. This ensures all axis receive their command within the
// same clock cycle, and can operate truly synchronized regardless
// of the speed of the microcontroller updates to the individual
// axis registers.
//
// RSV - Reserved. Reads 0, writes ignored.
//

//
// As menlo_cnc is designed to model and implement the motion
// commands for LinuxCNC the axis names come from the LinuxCNC project,
// specifically the file:
//
// https://github.com/LinuxCNC/linuxcnc/blob/master/docs/src/code/code-notes.txt
//

//
// It's expected that unsigned long is 32 bit on
// all architectures, 16 bit, 32 bit, 64 bit.
//
// *** WARNING ***
//
// Note that this is just a general interface register file and that
// a given FPGA bitstream/program, its pin/port assignments, and external
// wiring determine final behavior. Understand this fully before
// initiating any commands, even basic tests such as setting all
// registers to 0.
//
// Note as a design rule all 0's should be harmless with no machine motion
// (any axis or spindle(s)) or options such as coolant control on. But this is
// up to the implementor of the final FPGA bitstream/program, its pin
// assignments, and external wiring.
//
// *** WARNING ***
//
// *** WARNING ***
//
// Even though there is provision here for an ESTOP signal, it
// should be connected to an operator button "PAUSE", with an
// actual ESTOP implemented as a full relay power cut to *ALL*
// motors or supporting power supplies to ensure
// *NO MACHINE MOTION OF ANY KIND* can occur due to a full power
// cut off, regardless of the state of the software, FPGA, wiring, etc.
//
// Please be aware that the EMI (electromagnetic interference)
// environment of machine tools is very high, and as a result
// electronic controls may be forced into a bad, even dangerous
// state by such a static/power signal "glitch", or other EMI
// event.
//
// Even simple things such as an axis position encoder connector
// and/or wiring getting loose can cause a machine to go out of
// control since it thinks its not seeing any motion inputs, so
// it keeps repeating motion commands at potentially full
// axis speed. Note: This is why the position error limit value
// and setup is critical for machines with a "closed loop"
// positioning system.
//
// *** WARNING ***
//
// Why PAUSE vs. EMERGENCY STOP buttons?
//
// PAUSE is an attempt to stop machine motion when the operator
// observes a critical condition and wants to take corrective action,
// but attempts to preserve current machine/software/controller state
// so that the operation may be resumed after corrective action is taken.
//
// EMERGENCY STOP cuts *ALL POWER* to the machine, spindles, axis
// power supplies, coolant controls, pumps, etc. due to a harmful
// condition to either humans or the machine. In this case the current
// cutting operation is lost, but people/machines are uninjured so
// the operation may resume after resetting up, calibrating, finding zero,
// etc.
//
// It goes without saying 
//
// ***YOU SHOULD NEVER RELY ON ELECTRONICS FOR EMERGENCY STOP ***
//
// At best, a hardened electro-mechanical relay should be used
// 100% outside the control loop of the CNC control electronics.
//

typedef struct _MENLO_CNC_REGISTERS {

#if INTERFACE_VERSION_2

    //
    // Initialization validates register mapping by
    // looking for these values.
    //
    // Also the caller supplies the interface version number
    // they are looking for.
    //
    // This way alternate/compatible register files maybe
    // returned.
    //

    // Always reads as 0's
    unsigned long zero;

    // Always reads as 1's
    unsigned long one;

    // Identifies an FPGA <=> Software interface version
    unsigned long interface_version;

    // Indentifies a given revision of the interface, implementation.
    unsigned long interface_serial_number;

#endif // INTERFACE_VERSION_2

    // 4 * sizeof(unsigned long) == 16 bytes

    //
    // Note: (0xxx) represents word address
    //

    // Control and Status // 0x0
    unsigned long status;
    unsigned long command;
    unsigned long reserved0;
    unsigned long reserved1;

    //
    // Linear orthagonal axis
    //
    unsigned long x_pulse_rate; // 0x10 (4)
    unsigned long x_pulse_count; // 0x14 (5)
    unsigned long x_pulse_width; // 0x18 (6)
    unsigned long x_pulse_instruction; // 0x1C (7)

    unsigned long y_pulse_rate;
    unsigned long y_pulse_count;
    unsigned long y_pulse_width;
    unsigned long y_pulse_instruction;

    unsigned long z_pulse_rate;
    unsigned long z_pulse_count;
    unsigned long z_pulse_width;
    unsigned long z_pulse_instruction;

    //
    // Angular axis
    //
    unsigned long a_pulse_rate;
    unsigned long a_pulse_count;
    unsigned long a_pulse_width;
    unsigned long a_pulse_instruction;

    unsigned long b_pulse_rate;
    unsigned long b_pulse_count;
    unsigned long b_pulse_width;
    unsigned long b_pulse_instruction;

    unsigned long c_pulse_rate;
    unsigned long c_pulse_count;
    unsigned long c_pulse_width;
    unsigned long c_pulse_instruction;

    //
    // Relative linear orthagonal coordinates
    //

    unsigned long u_pulse_rate;
    unsigned long u_pulse_count;
    unsigned long u_pulse_width;
    unsigned long u_pulse_instruction;

    unsigned long v_pulse_rate;
    unsigned long v_pulse_count;
    unsigned long v_pulse_width;
    unsigned long v_pulse_instruction;

    unsigned long w_pulse_rate;
    unsigned long w_pulse_count;
    unsigned long w_pulse_width;
    unsigned long w_pulse_instruction;

    //
    // 44 * sizeof(unsigned long) == xxx bytes
    //

    //
    // Spindle controls. Depending on instruction could
    // be controlling a PWM or target RPM/voltage motor.
    //

    unsigned long spindle_pulse_rate;
    unsigned long spindle_pulse_count;
    unsigned long spindle_pulse_width;
    unsigned long spindle_pulse_instruction;

    //
    // General input control signals, 128 bits total.
    //
    // For position encoders, limit switches, edge finders,
    // motor speed, tool indicator, operator switches, etc.
    //
    // Note: FPGA program and external wiring determine
    // what they do for a given configuration.
    //
    // They may be individual status bits, or grouped
    // value words.
    //

    unsigned long gpio_input0;
    unsigned long gpio_input1;
    unsigned long gpio_input2;
    unsigned long gpio_input3;

    //
    // General output control signals, 128 bits total.
    //
    // For spindle options, tool changers, coolant, operator lights, etc.
    //
    // Note: FPGA program and external wiring determine
    // what they do for a given configuration.
    //
    // They may be individual control bits, or grouped
    // command words.
    //

    unsigned long gpio_output0;
    unsigned long gpio_output1;
    unsigned long gpio_output3;
    unsigned long gpio_output4;

    //
    // 56 * sizeof(unsigned long) == xxx bytes
    //

    //
    // Block for future expansion
    //

    unsigned long expansion_input0;
    unsigned long expansion_input1;
    unsigned long expansion_input2;
    unsigned long expansion_input3;
    
    unsigned long expansion_output0;
    unsigned long expansion_output1;
    unsigned long expansion_output2;
    unsigned long expansion_output3;

    //
    // 64 * sizeof(unsigned long) == 256 bytes
    //

} *PMENLO_CNC_REGISTERS, MENLO_CNC_REGISTERS;

#define MENLO_CNC_REGISTERS_INTERFACE_VERSION 0x00000002

#define MENLO_CNC_REGISTERS_SERIAL_NUMBER     0x12345678

//
// Instruction register is currently only 4 bits width.
//
#define MENLO_CNC_REGISTERS_INSTRUCTION_MASK 0x0000000F

//
// Status Register
//
#define MENLO_CNC_REGISTERS_STATUS_FBF 0x01
#define MENLO_CNC_REGISTERS_STATUS_FBE 0x02
#define MENLO_CNC_REGISTERS_STATUS_ERR 0x04
#define MENLO_CNC_REGISTERS_STATUS_BSY 0x08
#define MENLO_CNC_REGISTERS_STATUS_IDL 0x10
#define MENLO_CNC_REGISTERS_STATUS_EMS 0x20

//
// Command register
//
#define MENLO_CNC_REGISTERS_COMMAND_RST 0x01
#define MENLO_CNC_REGISTERS_COMMAND_CBF 0x02
#define MENLO_CNC_REGISTERS_COMMAND_EAN 0x04
#define MENLO_CNC_REGISTERS_COMMAND_EMS 0x08
#define MENLO_CNC_REGISTERS_COMMAND_CMD 0x10

//
// Axis Instruction Register
//
// 0 == Clockwise, 1 == counter clockwise
#define MENLO_CNC_REGISTERS_INSTRUCTION_DIR 0x01

// 0 == DWELL, 1 == generate movement pulses
#define MENLO_CNC_REGISTERS_INSTRUCTION_INS 0x02

//
// Published routines from menlo_cnc.c
//

void
menlo_cnc_registers_noop(
    PMENLO_CNC_REGISTERS registers
    );

unsigned long
menlo_cnc_registers_initialize(
    PMENLO_CNC_REGISTERS registers
    );

//
// Reset the FPGA timing engine.
//
// Clears all current commands.
//
// Clears the FIFO.
//
unsigned long
menlo_cnc_reset_timing_engine(
    PMENLO_CNC_REGISTERS registers
    );

//
// Read and return the status register.
//
// This is a non-volatile operation with no self-resetting
// "sticky bits".
//
unsigned long
menlo_cnc_read_status(
    PMENLO_CNC_REGISTERS registers
    );

//
// Spinwait until the FIFO can accept a new instruction block.
//
// Returns:
//
// Status value which may contain an error code or ESTOP.
//
unsigned long
menlo_cnc_wait_for_fifo_ready(
    PMENLO_CNC_REGISTERS registers
    );

//
// Load the command register.
//
// If CMD == 1 all of the axis registers are read and
// loaded into the FIFO.
//
// If EN == 1 then machine motion will immediately commence.
//
unsigned long
menlo_cnc_load_command(
    PMENLO_CNC_REGISTERS registers,
    unsigned long command
    );

//
// Test the registers.
//
// Returns:
//   0 - success
//
//  !0 - failure. The test value written and value read
//       from the register is returned.
//
//       The register number returned start from 0 at
//       the base of the register file (status register)
//       and each index represents a 32 bit register in the file.
//
//       register_in_error == (-1) means a general initialize failure.
//
int
menlo_cnc_registers_test(
    PMENLO_CNC_REGISTERS registers,
    unsigned long* test_value,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
    );

//
// Load new instruction/command to each axis
// register.
//
// The values in the axis registers are not transferred
// to the motion controller FIFO's until the
// CMD bit is set in the command register.
//

unsigned long
menlo_cnc_load_axis_x(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    );

unsigned long
menlo_cnc_load_axis_y(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    );

unsigned long
menlo_cnc_load_axis_z(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    );

unsigned long
menlo_cnc_load_axis_a(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    );

//
// Command packet for each axis
//
typedef struct _MENLO_CNC_AXIS_COMMAND {
    unsigned long pulse_rate;
    unsigned long pulse_count;
    unsigned long pulse_width;
    unsigned long pulse_instruction;
} *PMENLO_CNC_AXIS_COMMAND, MENLO_CNC_AXIS_COMMAND;

//
// Load commands for all 4 axis.
//
// If the command specifies MENLO_CNC_REGISTERS_COMMAND_CMD
// then the instruction block is loaded into the FIFO
// which can immediately state machine motion if enable.
//
// If the FIFO is full the command spins waiting until the
// FIFO can accept the instruction block.
//
// Returns:
//
// Value of Status register on error or success.
//
unsigned long
menlo_cnc_load_4_axis(
    PMENLO_CNC_REGISTERS registers,
    unsigned long command,
    PMENLO_CNC_AXIS_COMMAND x,
    PMENLO_CNC_AXIS_COMMAND y,
    PMENLO_CNC_AXIS_COMMAND z,
    PMENLO_CNC_AXIS_COMMAND a
    );

//
// Note: Calculation routines try and stay with 32 bit unsigned
// integers so they may be handled by a weak "soft" FPGA processor
// or simpler FPGA logic.
//
// Top edge routines use floating point to allow C programms running
// on the SoC to use higher level values.
//
// It's expected that any translation from higher level commands
// to a program to execute on the FPGA machine controller to do
// any lowering required based on the implemented capabilities.
//
// There is flexibility here to allow tradeoffs in designs as
// to whether to handle floating point, integer, and size of
// integers on either the SoC, soft/embedded processor, or FPGA
// fabric itself.
//
// Note: registers are passed in order to support versions of the
// timing engine which return their base clock, and other parameters
// programatically.
//

//
// Parameters for the timing generator.
//
// These are for the Cyclone V FPGA running at 50Mhz
// on the DE10-Standard.
//
// Other hosts can maintain these values if sourced with a
// 50Mhz clock and utilize the same scale factor.
//
// This can generate a peak 25Mhz axis clock, well beyond the
// frequency needs of current machine tools.
//

// 50Mhz
#define TIMING_GENERATOR_BASE_CLOCK_RATE 50000000

//
// This must track above. It's here to avoid having to do
// floating point calculations for this basic constant.
//
// 50Mhz
#define TIMING_GENERATOR_BASE_CLOCK_PERIOD_IN_NANOSECONDS 20

// Scale factor is base clock rate divided by stages scale factor.
#define TIMING_GENERATOR_PULSE_RATE_SCALE_FACTOR    4

// Pulse width scale factor is base clock rate divided by stages scale factor.
#define TIMING_GENERATOR_PULSE_WIDTH_SCALE_FACTOR   1

//
// Returns the value for the pulse_rate register that will generate
// the specified frequency in HZ.
//
// Frequency can be fractional or or whole to represent periods
// from 40ns to ~80 seconds based on 32 bit internal registers clocked
// at a base rate of 50Mhz.
//
unsigned long
menlo_cnc_registers_calculate_pulse_rate_by_hz(
    PMENLO_CNC_REGISTERS registers,
    double frequency_in_hz
    );

//
// Returns the value for the pulse_width register that
// will generate the specified pulse_width in nanoseconds.
//
unsigned long
menlo_cnc_registers_calculate_pulse_width(
    PMENLO_CNC_REGISTERS registers,
    unsigned long pulse_width_in_nanoseconds
    );

//
// Calculates the number of clock periods for the given
// frequency in HZ.
//
// Frequency can be fractional or or whole to represent periods
// from 40ns to ~80 seconds based on 32 bit internal registers clocked
// at a base rate of 50Mhz, which is a clock period of 20ns.
//
unsigned long
menlo_cnc_registers_calculate_clock_periods_by_hz(
    PMENLO_CNC_REGISTERS registers,
    double frequency_in_hz
    );
