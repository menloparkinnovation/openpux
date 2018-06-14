
//
// This gives an outline of a simple assember for
// a C strings parsing example.
//
// 06/06/2018
//

//
// menlo_cnc_asm.h - Assembler for menlo_cnc, FPGA based machine tool controller.
//
// Copyright (c) 2018 Menlo Park Innovation LLC
//
// https://github.com/menloparkinnovation/openpux/tree/master/menlocnc
//
// 06/06/2018

typedef struct _AXIS_OPCODE {
  char* axis;
  char* opcode;
  char* arg0;
  char* arg1;
  char* arg2;
} AXIS_OPCODE, *PAXIS_OPCODE;

typedef struct _OPCODE_BLOCK_FOUR_AXIS {
  AXIS_OPCODE info;
  AXIS_OPCODE x;
  AXIS_OPCODE y;
  AXIS_OPCODE z;
  AXIS_OPCODE a;
} OPCODE_BLOCK_FOUR_AXIS, *POPCODE_BLOCK_FOUR_AXIS;

typedef struct _OPCODE_BLOCK_FIVE_AXIS {
  AXIS_OPCODE info;
  AXIS_OPCODE x;
  AXIS_OPCODE y;
  AXIS_OPCODE z;
  AXIS_OPCODE a;
  AXIS_OPCODE b;
} OPCODE_BLOCK_FIVE_AXIS, *POPCODE_BLOCK_FIVE_AXIS;

typedef struct _OPCODE_BLOCK_SEVEN_AXIS {
  AXIS_OPCODE info;
  AXIS_OPCODE x;
  AXIS_OPCODE y;
  AXIS_OPCODE z;
  AXIS_OPCODE a;
  AXIS_OPCODE b;
  AXIS_OPCODE c;
  AXIS_OPCODE u;
} OPCODE_BLOCK_SEVEN_AXIS, *POPCODE_BLOCK_SEVEN_AXIS;

typedef struct _OPCODE_BLOCK_NINE_AXIS {
  AXIS_OPCODE info;
  AXIS_OPCODE x;
  AXIS_OPCODE y;
  AXIS_OPCODE z;
  AXIS_OPCODE a;
  AXIS_OPCODE b;
  AXIS_OPCODE c;
  AXIS_OPCODE u;
  AXIS_OPCODE v;
  AXIS_OPCODE w;
} OPCODE_BLOCK_NINE_AXIS, *POPCODE_BLOCK_NINE_AXIS;

//
// State enum for opcode assembly
//
// Note: In the assembler model "opcode" is any operation, and
// could represent multi-axis or other machine control commands
// such as spindle speed, coolant flow, etc.
//

enum OpCodeState {
  OpCodeStateUnknown     = 0,
  OpCodeStateIdle        = 1,
  OpCodeStateAxis        = 2,
  OpCodeStateInstruction = 3,
  OpCodeStateArg0        = 4,
  OpCodeStateArg1        = 5,
  OpCodeStateArg2        = 6,
  OpCodeStateComplete    = 7
};

//
// Which axis is being assembled
//
// Note: In the assembler model "axis" represents the object
// being operated on. Typically its a machine axis, but it could
// be a spindle, etc.
//

enum AxisState {
  AxisStateUnknown     = 0,
  AxisStateIdle        = 1,

  AxisStateX           = 2,
  AxisStateY           = 3,
  AxisStateZ           = 4,

  AxisStateA           = 5,
  AxisStateB           = 6,
  AxisStateC           = 7,

  AxisStateU           = 8,
  AxisStateV           = 9,
  AxisStateW           = 10,

  //
  // INFO is a virtual axis.
  //
  AxisStateInfo        = 11
};

//
// Block array
//
// Approximates the javascript array .push(obj) operation.
//
typedef struct _BLOCK_ARRAY {

  // Array of blocks
  void* blocks;

  // Size of each entry
  int entry_size;

  // Current valid entries.
  int array_size;

  // Current allocated capacity.
  int array_capacity;

  // Amount of new capacity added when growing.
  int array_increment_size;

  // Supports seek, get next interface
  int seek_index;

} BLOCK_ARRAY, *PBLOCK_ARRAY;

//
// Binary command packet for each axis
//
typedef struct _AXIS_OPCODE_BINARY {
  unsigned long instruction;
  unsigned long pulse_rate;
  unsigned long pulse_count;
  unsigned long pulse_width;
} AXIS_OPCODE_BINARY, *PAXIS_OPCODE_BINARY;

//
// An opcode block has an explicit begin and end
// marker in its binary file.
//
// The end marker contains a checksum of the block.
//

typedef struct _OPCODE_BLOCK_FOUR_AXIS_BINARY {
  AXIS_OPCODE_BINARY begin;
  AXIS_OPCODE_BINARY x;
  AXIS_OPCODE_BINARY y;
  AXIS_OPCODE_BINARY z;
  AXIS_OPCODE_BINARY a;
  AXIS_OPCODE_BINARY end;
} OPCODE_BLOCK_FOUR_AXIS_BINARY, *POPCODE_BLOCK_FOUR_AXIS_BINARY;

//
// An opcode block can be assembled from multiple
// lines, so this allows the state to be tracked.
//
typedef struct _ASSEMBLER_CONTEXT {

  int lineNumber;

  // Current opcode binary being assembled.
  AXIS_OPCODE_BINARY opcode_binary;

  // Current opcode block being assembled.
  OPCODE_BLOCK_FOUR_AXIS opcode_block;

  char opcode_block_valid;

  char saw_header;

  char saw_config;

  char saw_begin;

  char saw_end;

  char single_line_block;

  enum OpCodeState opcode_state;

  enum AxisState axis_state;

  PAXIS_OPCODE current_axis_opcode;

  char* current_axis_symbol;

  //
  // Dynamic block array for compiled binary opcode blocks.
  //
  PBLOCK_ARRAY compiled_binary;

  //
  // This ensures there are no duplicates on axis
  // in a block.
  //
  char x_set;
  char y_set;
  char z_set;

  char a_set;
  char b_set;
  char c_set;

  char u_set;
  char v_set;
  char w_set;

} ASSEMBLER_CONTEXT, *PASSEMBLER_CONTEXT;

//
// Model:
//
// Instructions are decribed as resource, instruction, arg0, arg1, arg2.
//
// Resource may be a valid Axis for the machine, or a logical
// unit such as spindle, coolant control, indicator lights,
// or general registers.
//
// Example for clockwise X axis motion at rate 1000, 10 steps,
// 10 clock pulse width.
//
// X, CW, 1000, 10, 10
//
// Instructions are issued in blocks across all axis/resources
// in parallel in the same clock cycle. This allows coordinated
// machine motion.
//
// The instruction block completes when all instructions within
// the block complete. When an instruction block completes, the
// next instruction block is loaded if available, otherwise the
// last instructions block executed defines the state of the
// machine.
//
// Example:
//
// begin
//   X,CW,0x100,16,4
//   Y,CCW,50,8,2
//   Z,DWELL,75,12,0
//   A,NOP,0,0,0
// end
//
// A series of instruction blocks are placed into a hardware FIFO
// on the FPGA. This FIFO is kept loaded in real time by the SoC
// and/or DMA to ensure that machine motion does not pause as long
// as available commands are issued.
//
// The default depth of this FIFO is 512 instruction block entries.
//
// Some configurations may have a secondary buffer before the
// FIFO such as the 64MB SDRAM directly connected to the FPGA
// on the DE10-Standard. This secondary buffer may be read
// by FPGA logic, or a soft processor such as a Nios II.
//
// Some configurations may use direct Avalon channel DMA from
// SoC DRAM which is a larger pool of up to 1GB, but shared
// with the SoC operating system and applications in addition
// to having to deal with physical address scatter/gather details.
//
// The depth of this FIFO, and the response time of the
// process feeding it determines the maximum instruction rate
// that can be utilized without pauses. Instructions that take
// longer time (high pulse count and/or low frequency) are
// easier to be kept up to date since each instruction block
// takes longer to execute providing more time to load the
// next series into the FIFO.
//
// Insruction Parameters
//
// Frequency values for pulse rate may be specified as clock
// counts, or frequencies in HZ, KHZ, MHZ.
//
// Pulse width timings may be specified in clocks, ms, us, ns,
// or percent of pulse period.
//
// pulse period is 1 / pulse frequency.
//
// Current encoding is in clocks, which is the base machine binary, but
// for higher level abstraction pulse rate (frequency) and pulse width
// (fixed time or percentage) is easier to manipulate.
//
// Examples:
//
// pulse_rate:
//
// 1000 - clocks, machine specific. For example 20ns per clock at 50Mhz.
//
// 1000HZ - 1000 Hertz
// 0.1HZ    - 1/10 Hertz, or one pulse in 10 seconds.
//
// 1KHZ     - 1 Kilo Hertz
// 1.200KHZ - 1.2 Kilo Hertz or 1200 Hertz
//
// 1MHZ     - 1 Mega Hertz
// 1.200MHZ - 1.2 Mega Hertz or 1200000 Hertz
//
// Pulse Width:
//
//  20      - clocks. For example 20ns per clock at 50Mhz.
//
//  20us    - 20 microseconds
//
//  200ns    - 200 nanoseconds
//
//  50%     - 50% of pulse period
//            pulse period is 1 / pulse rate.
//
//  55.5%   - 55.5% of clock period
//            pulse period is 1 / pulse rate.
//

//
// NOP, 0, 0 ,0
//
// Executes a NOP.
//
// All parameters must be 0.
//
// The NOP does not delay instruction block completion.
// The instruction block completes when the other instructions
// complete.
//
#define OPCODE_NOP_SYMBOL         "NOP"

//
// CW, pulse_rate, pulse_count, pulse_width
//
// Execute a clockwise step/dir motion command on an axis.
//
// step/dir motion control commands are common on many small
// to medium sized machine tools, 3D printers, laser cutter
// motion, carvers, etc.
//
// Generates the pulse_count number of steps at
// pulse frequency. The width of the generated pulses
// is defined by pulse_width.
//
// When pulse_count goes to zero pulses stop and the
// current instruction block may complete if the other
// instructions in the block are  completed.
//
// The DIR output is logical low, or 0 to indicate
// clockwise motion.
//
// Note: Output wiring/configuration for some machines
// may invert this signal, but the assembler/timing engine will
// use "0" to indicate clockwise direction.
//
#define OPCODE_MOTION_CW_SYMBOL   "CW"

//
// CCW, pulse_rate, pulse_count, pulse_width
//
// Execute a counter clockwise step/dir motion command on an axis.
//
// step/dir motion control commands are common on many small
// to medium sized machine tools, 3D printers, laser cutter
// motion, carvers, etc.
//
// Generates the pulse_count number of steps at
// pulse frequency. The width of the generated pulses
// is defined by pulse_width.
//
// When pulse_count goes to zero pulses stop and the
// current instruction block may complete if the other
// instructions in the block are  completed.
//
// The DIR output is logical high, or 1 to indicate
// counter-clockwise motion.
//
// Note: Output wiring/configuration for some machines
// may invert this signal, but the assembler/timing engine will
// use "1" to indicate counter-clockwise direction.
//
#define OPCODE_MOTION_CCW_SYMBOL  "CCW"

//
// DWELL, pulse_rate, pulse_count, 0
//
// Delay instruction block completion for a period of time
// defined by pulse_rate and pulse_count.
//
// 0 is required for the third parameter.
//
// This is essentially a timed NOP and useful for delaying
// execution between a series of instruction blocks.
// 
// It may be applied to any axis in the instruction block
// in parallel with non-DWELL instructions.
//
// The instruction block completes when all instructions in
// the block complete as normal.
//
#define OPCODE_DWELL_SYMBOL       "DWELL"

//
// PWM pulse_rate, pulse_count, pulse_width
//
// Generate a PWM control signal on an axis and/or resource.
//
// PWM is used to control R/C style servo motors used in robotics,
// CNC spindle speeds, laser cutter output beam, and larger servo
// style positioning and control systems.
//
// PWM generates a series of pulses of pulse_width at a pulse_rate
// base frequency for pulse_count duration.
//
// While pulse_count is greater than zero the PWM command will
// prevent the current instruction block from completing. This
// allows the instruction to control the duration of a PWM signal.
//
// When pulse_count goes to zero, PWM maintains its current output
// settings, but allows the current instruction block to complete.
// This allows PWM style motion control signals to sustain a position
// when "idle" by a non-varying train of pulses.
//
// Once the instruction block completes, new instruction blocks
// will be loaded from the FIFO is available similar to the
// CW, CCW, commands.
//
#define OPCODE_MOTION_PWM_SYMBOL   "PWM"

//
// OUTPUT output_bits, 0, 0
//
// Set port output bits.
//
// output_bits is a 32 bit value.
//
// The meaning of the 0 or 1 states is machine specific.
//
// The second and third parameters must be zero at this
// time. In the future they may represent programmable
// output port configuration such as TTL, tri-state, etc,
// as well as bit-flip commands (read-modify-write).
//
// The instruction executes immediately and does not
// delay completion of the current instruction block.
//
// The last value placed into the output port is held
// until the next OUTPUT command to the port executes.
//
// Note: There is no port input instruction since commands
// execute in a pipeline block. Port reads are real time
// registers when available.
//
#define OPCODE_OUTPUT_PORT_SYMBOL   "OUTPUT"

//
// header, machine_version, machine_revision, machine_options
//
// Must be begining of file, and followed by config.
//
// Takes 3 32 bit unsigned options.
//
#define OPCODE_HEADER_SYMBOL      "HEADER"

//
// config, number_of_axis, options0, options1
//
// config must follow header
//
// Takes 3 32 bit unsigned options.
//
#define OPCODE_CONFIG_SYMBOL      "CONFIG"

//
// begin
//
// This is a single 32 bit word.
//
#define OPCODE_BEGIN_SYMBOL       "begin"

//
// end, checksum
//
// This is a single 32 bit word.
//
// checksum is encoded in the binary
//
#define OPCODE_END_SYMBOL         "end"

//
// Axis symbols
//

#define X_AXIS_SYMBOL "X"
#define Y_AXIS_SYMBOL "Y"
#define Z_AXIS_SYMBOL "Z"

#define A_AXIS_SYMBOL "A"
#define B_AXIS_SYMBOL "B"
#define C_AXIS_SYMBOL "C"

#define U_AXIS_SYMBOL "U"
#define V_AXIS_SYMBOL "V"
#define W_AXIS_SYMBOL "W"

//
// INFO is a virtual axis
//
#define INFO_AXIS_SYMBOL "INFO"

//
// The following are resources treated as instruction targets
// similar to Axis.
//

//
// Machine tool spindle speed control.
//
// Typically used with PWM.
//
#define SPINDLE_RESOURCE_SYMBOL "SPINDLE"

//
// Laser cutter laser beam output modulation.
//
// Typically used with PWM.
//
#define LASER_RESOURCE_SYMBOL "LASER"

//
// General purpose PWM resources.
//
// Large/advanced setups have user defined PWM signal controls.
//
#define PWM_RESOURCE_0 "PWM0"
#define PWM_RESOURCE_1 "PWM1"
#define PWM_RESOURCE_2 "PWM2"
#define PWM_RESOURCE_3 "PWM3"

//
// Output ports
//
// Output ports can be set are part of the instruction
// stream and be co-ordinated with instruction blocks.
//
// These are opposed to fixed outputs which are registers
// which hold a constant value. These output registers
// change by commands in the instruction stream.
//

#define OUTPUT_RESOURCE_0 "OUTPUT0"
#define OUTPUT_RESOURCE_1 "OUTPUT1"
#define OUTPUT_RESOURCE_2 "OUTPUT2"
#define OUTPUT_RESOURCE_4 "OUTPUT3"

//
// Parameters are 32 bit unsigned, little endian byte order.
//
// Note: NOP and DWELL are the same opcode.
//
// Set parameters to 0 for NOP so it does not delay
// execution block completion.
//
// DWELL time is the execution rate of the opcodes
// parameters pulse_rate and pulse_count.
//

#define OPCODE_NOP         0x00

#define OPCODE_MOTION_CW   0x02

#define OPCODE_MOTION_CCW  0x03

#define OPCODE_DWELL       0x00

//
// These are pseudo instructions that mark a program block,
// machine controller engine version, number of axis, etc.
// This is an important safety check so that a binary machine
// program is not run on the wrong machine and/or configuration.
//
// These also mark the begining and end of block instruction
// boundary.
//
// This information is assembled into the binary, but depending
// on machine controller version may be handled by host software,
// an embedded processor, or logic in the engine itself.
//
// The begin/end markers are useful for validating blocks
// of instructions in memory, after DMA, etc. It is optional
// whether a particular machine controller handles them, or
// they are handled by host or embedded firmware processors.
//

#define OPCODE_HEADER        0xFF000000

#define OPCODE_CONFIG        0xFE000000

// Begining of a block of instructions.
#define OPCODE_BEGIN_BLOCK 0xFD000000

//
// End of a block of instructions.
//
// The lower 8 or 16 bits represents a checksum
// of the block.
//
#define OPCODE_END_BLOCK   0xFC000000

//
// Axis codes
//

#define AXIS_CODE_X        0x0
#define AXIS_CODE_Y        0x1
#define AXIS_CODE_Z        0x2

#define AXIS_CODE_A        0x3
#define AXIS_CODE_B        0x4
#define AXIS_CODE_C        0x5

#define AXIS_CODE_U        0x6
#define AXIS_CODE_V        0x7
#define AXIS_CODE_W        0x8

#define AXIS_CODE_INFO     0xFF000000

//
// Block array defaults
//
#define BLOCK_ARRAY_INITIAL_ALLOCATION 1024

#define BLOCK_ARRAY_INCREMENTAL_ALLOCATION BLOCK_ARRAY_INITIAL_ALLOCATION

//
// API Contracts
//

//
// Higher level service oriented
//

//
// Open and load the specified assembly file, assemble it
// and return the binary instructions in memory.
//
int assemble_file(char* fileName, PBLOCK_ARRAY* binary);

//
// Seek stream in block array
//
// Seeks to a specific index.
//
// Seeking to 0 is rewind.
//
int block_array_seek_entry(PBLOCK_ARRAY ba, int entry_index);

//
// Get the next entry.
//
// Returns NULL if not more entries.
//
// Intended for high speed streaming of binary commands to
// a hardware interface with low per entry processor cycle overheads.
//
void* block_array_get_next_entry(PBLOCK_ARRAY ba);

//
// Lower level
//

void initialize_assembler_context(PASSEMBLER_CONTEXT context);

int process_assembler_line(PASSEMBLER_CONTEXT context, char* s);

PBLOCK_ARRAY block_array_allocate(int entry_size, int array_size, int array_increment_size);

void block_array_free(PBLOCK_ARRAY ba);

void* block_array_get_entry(PBLOCK_ARRAY ba, int index);

int block_array_get_array_size(PBLOCK_ARRAY ba);

//
// entry_size is a check. If you try and push a size not specified
// when the block array is created it causes a -1 return.
//
// Returns the newly allocated entry index.
//
void* block_array_push_entry(PBLOCK_ARRAY ba, void* entry, int entry_size);

