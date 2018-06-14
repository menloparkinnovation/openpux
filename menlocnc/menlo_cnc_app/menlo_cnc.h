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
// Binary command packet for each axis
//
typedef struct _MENLO_CNC_AXIS_OPCODE_BINARY {
  unsigned long instruction;
  unsigned long pulse_rate;
  unsigned long pulse_count;
  unsigned long pulse_width;
} MENLO_CNC_AXIS_OPCODE_BINARY, *PMENLO_CNC_AXIS_OPCODE_BINARY;

typedef struct _MENLO_CNC_OPCODE_BLOCK_FOUR_AXIS_BINARY {
  MENLO_CNC_AXIS_OPCODE_BINARY x;
  MENLO_CNC_AXIS_OPCODE_BINARY y;
  MENLO_CNC_AXIS_OPCODE_BINARY z;
  MENLO_CNC_AXIS_OPCODE_BINARY a;
} MENLO_CNC_OPCODE_BLOCK_FOUR_AXIS_BINARY, *PMENLO_CNC_OPCODE_BLOCK_FOUR_AXIS_BINARY;

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

typedef struct _MENLO_CNC_AXIS_COMMAND {
    unsigned long instruction;
    unsigned long pulse_rate;
    unsigned long pulse_count;
    unsigned long pulse_width;
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
menlo_cnc_load_four_axis(
    PMENLO_CNC_REGISTERS registers,
    unsigned long command,
    PMENLO_CNC_AXIS_OPCODE_BINARY x,
    PMENLO_CNC_AXIS_OPCODE_BINARY y,
    PMENLO_CNC_AXIS_OPCODE_BINARY z,
    PMENLO_CNC_AXIS_OPCODE_BINARY a
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

#define MENLO_CNC_REGISTERS_ERROR_MASK (MENLO_CNC_REGISTERS_STATUS_ERR | MENLO_CNC_REGISTERS_STATUS_EMS)

int menlo_cnc_registers_is_error(unsigned long status);
