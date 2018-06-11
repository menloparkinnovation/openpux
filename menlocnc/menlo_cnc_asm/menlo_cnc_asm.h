
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

} BLOCK_ARRAY, *PBLOCK_ARRAY;

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

// NOP, 0, 0 ,0
#define OPCODE_NOP_SYMBOL         "NOP"

// CW, pulse_rate, pulse_count, pulse_width
#define OPCODE_MOTION_CW_SYMBOL   "CW"

// CCW, pulse_rate, pulse_count, pulse_width
#define OPCODE_MOTION_CCW_SYMBOL  "CCW"

// DWELL, pulse_rate, pulse_count, 0
#define OPCODE_DWELL_SYMBOL       "DWELL"

// INFO, arg0, arg1, arg2
//#define OPCODE_INFO_SYMBOL         "INFO"

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

