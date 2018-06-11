
//`define DEBUG_AVALON_MM_SLAVE 1

// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 04/21/2018
//
// Microprocessor register interface for timing generator.
//
// This is mapped into the memory mapped I/O address space
// of either the ARM SoC's or the Nios II processor and
// presents a register file for loading commands into
// the timing generator FIFO's.
//
// It conforms to the Altera Avalon MM Slave interface
// for the Cyclone V. This interface is fairly generic
// and can be adapted to other slave output logic. This
// is a basic read-write register file, not a streaming
// interface.
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
// If the FIFO status is not full, the commmand register CMD bit is
// then set and it transfers all axis commands into the axis command
// FIFO's in the same clock cycle. This is important to ensure no axis
// starts early, and would be out of synchronization with the others.
//
// Software may continue this sequence of writing commands to
// the FIFO's until its full, thus buffering commands in
// a pipeline to ensure there not a pause in machine motion.
//
// Software is expected to operate in "real time" status to
// ensure the FIFO never becomes empty when commands are
// stored in the command buffer SDRAM/DRAM. A dedicated
// Nios II real time processor is recommended, with storage
// of commands in the 64MB SDRAM on the DE10-Standard.
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
// The preferred model is to use the ARM SoC's to fill
// the 64MB SDRAM command buffer, and let a Nios II
// real time processor keep the command FIFO's full.
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
//
//
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

module timing_generator_registers
(
    clock,
    reset_n,

    // signals passed our from soc_system and QSYS.
    slave_address,
    slave_read,
    slave_write,
    slave_readdata,
    slave_writedata,
    slave_byteenable,

    //
    // Hardware signals not connected to the microprocessor.
    //

    estop_in,

    //
    // X Axis timing generator outputs
    //
    x_timing_generator_pulse,
    x_timing_generator_direction,

    //
    // Y Axis
    //
    y_timing_generator_pulse,
    y_timing_generator_direction,

    //
    // Z Axis
    //
    z_timing_generator_pulse,
    z_timing_generator_direction,

    //
    // A Axis
    //
    a_timing_generator_pulse,
    a_timing_generator_direction

`ifdef DEBUG_AVALON_MM_SLAVE
    //
    // debug outputs
    //

    ,last_write_register,
    last_write_address_register,
    nc_last_write_register,
    nc_last_write_address_register,

    last_read_register,
    last_read_address_register,
    nc_last_read_register,
    nc_last_read_address_register
`endif
);

  parameter c_address_width = 7; // 128 bytes
  parameter c_data_width = 32;

  parameter c_signal_width = 32;
  parameter c_instruction_width = 4;

  input                           clock;
  input                           reset_n;

  //
  // Avalon MM Slave Interface Signals passed through conduit
  // from soc_system.
  //

  input [c_address_width-1:0]     slave_address;
  input                           slave_read;
  input                           slave_write;
  output [c_data_width-1:0]       slave_readdata;
  input [c_data_width-1:0]        slave_writedata;
  input [(c_data_width/8)-1:0]    slave_byteenable;

  //
  // Hardware signals not connected to the microprocessor.
  //

  input                        estop_in;

  //
  // Per axis output signals
  //
  output                       x_timing_generator_pulse;
  output                       x_timing_generator_direction;

  output                       y_timing_generator_pulse;
  output                       y_timing_generator_direction;

  output                       z_timing_generator_pulse;
  output                       z_timing_generator_direction;

  output                       a_timing_generator_pulse;
  output                       a_timing_generator_direction;

  //
  // local signals, registers
  //

  // Holds the value of the data register.
  reg [c_data_width-1:0]     read_data_register;

  // Holds the values of the status and command registers
  reg [c_data_width-1:0]     status_register;
  reg [c_data_width-1:0]     command_register;

`ifdef DEBUG_AVALON_MM_SLAVE

  //
  // There is a timing ambiguity on slave address and slave data
  // when the slave write signal occurs.
  //
  // This allows SignalTap to examine what is going on.
  //

  //
  // An online comment said making signals an output ensures their synthesis
  // for SignalTap.
  //

  // At first clock edge
  (*preserve*) output reg [c_data_width-1:0]     last_write_register;
  (*preserve*) output reg [c_data_width-1:0]     last_read_register;
  (*preserve*) output reg [c_data_width-1:0]     last_write_address_register;
  (*preserve*) output reg [c_data_width-1:0]     last_read_address_register;

  // At next clock edge
  (*preserve*) output reg [c_data_width-1:0]     nc_last_write_register;
  (*preserve*) output reg [c_data_width-1:0]     nc_last_read_register;
  (*preserve*) output reg [c_data_width-1:0]     nc_last_write_address_register;
  (*preserve*) output reg [c_data_width-1:0]     nc_last_read_address_register;

  (*preserve*) reg                               nc_last_write_waitstate;
  (*preserve*) reg                               nc_last_read_waitstate;

  (*preserve*) reg [31:0]                        debug_always1_clock_counter;
  (*preserve*) reg [31:0]                        debug_always2_clock_counter;

`endif

`ifdef DEBUG_AVALON_MM_SLAVE_NOT_WORKING

  //
  // The signals are not showing up even with attributes.
  //

  //
  // There is a timing ambiguity on slave address and slave data
  // when the slave write signal occurs.
  //
  // This allows SignalTap to examine what is going on.
  //

  // At first clock edge
  reg [c_data_width-1:0]     last_write_register; /* synthesis noprune */
  reg [c_data_width-1:0]     last_read_register; /* synthesis noprune */
  reg [c_data_width-1:0]     last_write_address_register; /* synthesis noprune */
  reg [c_data_width-1:0]     last_read_address_register; /* synthesis noprune */

  // At next clock edge
  reg [c_data_width-1:0]     nc_last_write_register; /* synthesis noprune */
  reg [c_data_width-1:0]     nc_last_read_register; /* synthesis noprune */
  reg [c_data_width-1:0]     nc_last_write_address_register; /* synthesis noprune */
  reg [c_data_width-1:0]     nc_last_read_address_register; /* synthesis noprune */

  reg                        nc_last_write_waitstate; /* synthesis noprune */
  reg                        nc_last_read_waitstate; /* synthesis noprune */

`endif

  // Signals to control timing generator input FIFO's
  reg                        fifo_write_signal;
  reg                        reg_clear_buffer;

  //
  // Registers that hold the individual axis instructions
  // and parameters.
  //
  reg [c_instruction_width-1:0] x_insert_instruction;
  reg [c_signal_width-1:0]   x_insert_pulse_period;
  reg [c_signal_width-1:0]   x_insert_pulse_count;
  reg [c_signal_width-1:0]   x_insert_pulse_width;

  reg [c_instruction_width-1:0] y_insert_instruction;
  reg [c_signal_width-1:0]   y_insert_pulse_period;
  reg [c_signal_width-1:0]   y_insert_pulse_count;
  reg [c_signal_width-1:0]   y_insert_pulse_width;

  reg [c_instruction_width-1:0] z_insert_instruction;
  reg [c_signal_width-1:0]   z_insert_pulse_period;
  reg [c_signal_width-1:0]   z_insert_pulse_count;
  reg [c_signal_width-1:0]   z_insert_pulse_width;

  reg [c_instruction_width-1:0] a_insert_instruction;
  reg [c_signal_width-1:0]   a_insert_pulse_period;
  reg [c_signal_width-1:0]   a_insert_pulse_count;
  reg [c_signal_width-1:0]   a_insert_pulse_width;

  //
  // Signals to timing generator instance.
  //
  wire                         reset_signal;
  wire                         estop_signal;
  wire                         enable_signal;
  wire                         software_reset;
  wire                         software_enable;
  wire                         software_estop;

  //
  // Aggregate signals from the timing generator instance.
  //
  // These are set by continous assign logic that is an OR
  // of the individual axis signals.
  //
  wire                         c_insert_buffer_full;
  wire                         c_remove_buffer_empty;
  wire                         c_timing_generator_error;
  wire                         c_timing_generator_busy;
  wire                         c_timing_generator_idle;

  //
  // 7 bit addresses for register bank.
  //
  // Note: The address bus represents each 32 bit
  // word with the bytes selected by byte strobes.
  //
  parameter[6:0]
      STATUS_REGISTER     = 7'd00,
      COMMAND_REGISTER    = 7'd01,
      RESERVED0_REGISTER  = 7'd02,
      RESERVED1_REGISTER  = 7'd03,

      X_AXIS_PULSE_PERIOD = 7'd04,
      X_AXIS_PULSE_COUNT  = 7'd05,
      X_AXIS_PULSE_WIDTH  = 7'd06,
      X_AXIS_PULSE_INST   = 7'd07,

      Y_AXIS_PULSE_PERIOD = 7'd08,
      Y_AXIS_PULSE_COUNT  = 7'd09,
      Y_AXIS_PULSE_WIDTH  = 7'd10,
      Y_AXIS_PULSE_INST   = 7'd11,

      Z_AXIS_PULSE_PERIOD = 7'd12,
      Z_AXIS_PULSE_COUNT  = 7'd13,
      Z_AXIS_PULSE_WIDTH  = 7'd14,
      Z_AXIS_PULSE_INST   = 7'd15,

      A_AXIS_PULSE_PERIOD = 7'd16,
      A_AXIS_PULSE_COUNT  = 7'd17,
      A_AXIS_PULSE_WIDTH  = 7'd18,
      A_AXIS_PULSE_INST   = 7'd19;

  //
  // Command register bits
  //
  parameter[31:0]
      COMMAND_RST = 32'h01, // RESET
      COMMAND_CFB = 32'h02, // CLEAR FIFO BUFFERS
      COMMAND_EAN = 32'h04, // ENABLE
      COMMAND_EMS = 32'h08, // EMERGENCY STOP input. Set to 1 for software ESTOP.
      COMMAND_CMD = 32'h10; // COMMAND, set to 1 to issue command in all registers.

  //
  // Status register bits
  //
  parameter[31:0]
      STATUS_FBF = 32'h01, // FIFO Buffer Full
      STATUS_FBE = 32'h02, // FIFO Buffer Empty (last command may stil be executing till IDLE)
      STATUS_ERR = 32'h04, // ERROR
      STATUS_BSY = 32'h08, // BUSY
      STATUS_IDL = 32'h10, // IDLE (no commands, no outstanding movement)
      STATUS_EMS = 32'h20; // EMERGENCY STOP indicated by software, hardware, external.

  //
  // Invoke sub-module instances
  //

  four_axis_timing_generator four_axis_timing_generator1(
      .c_clock(clock),
      .c_reset(reset_signal),
      .c_clear_buffer(reg_clear_buffer),
      .c_enable(enable_signal),
      .c_estop(estop_signal),
      
      .c_insert_clock(clock),
      .c_insert_signal(fifo_write_signal),
      
      .c_insert_buffer_full(c_insert_buffer_full),
      .c_remove_buffer_empty(c_remove_buffer_empty),

      .c_timing_generator_error(c_timing_generator_error),
      .c_timing_generator_busy(c_timing_generator_busy),

      //
      // X axis
      //

      .x_insert_instruction(x_insert_instruction),
      .x_insert_pulse_period(x_insert_pulse_period),
      .x_insert_pulse_count(x_insert_pulse_count),
      .x_insert_pulse_width(x_insert_pulse_width),

       // timing generator outputs
      .x_timing_generator_pulse(x_timing_generator_pulse),
      .x_timing_generator_direction(x_timing_generator_direction),

      //
      // Y axis
      //

      .y_insert_instruction(y_insert_instruction),
      .y_insert_pulse_period(y_insert_pulse_period),
      .y_insert_pulse_count(y_insert_pulse_count),
      .y_insert_pulse_width(y_insert_pulse_width),

      // timing generator outputs
      .y_timing_generator_pulse(y_timing_generator_pulse),
      .y_timing_generator_direction(y_timing_generator_direction),

      //
      // Z axis
      //

      .z_insert_instruction(z_insert_instruction),
      .z_insert_pulse_period(z_insert_pulse_period),
      .z_insert_pulse_count(z_insert_pulse_count),
      .z_insert_pulse_width(z_insert_pulse_width),

      // timing generator outputs
      .z_timing_generator_pulse(z_timing_generator_pulse),
      .z_timing_generator_direction(z_timing_generator_direction),

      //
      // A axis
      //

      .a_insert_instruction(a_insert_instruction),
      .a_insert_pulse_period(a_insert_pulse_period),
      .a_insert_pulse_count(a_insert_pulse_count),
      .a_insert_pulse_width(a_insert_pulse_width),

      // timing generator outputs
      .a_timing_generator_pulse(a_timing_generator_pulse),
      .a_timing_generator_direction(a_timing_generator_direction)
  );

  //
  // Continous assigns
  //

  //
  // read_data_register holds the output from the last read data
  // cycle and its assigned to the slave_readdata output. This way
  // we are independent of the external slaves read data timing.
  //

  assign slave_readdata = read_data_register;

  //
  // The following command register bits are wired to
  // continous assigns and do not require direct state
  // machine action.
  //

  assign software_reset = command_register[0:0];  // COMMAND_RST
  assign software_enable = command_register[2:2]; // COMMAND_EAN
  assign software_estop = command_register[3:3];  // COMMAND_EMS

  assign reset_signal = software_reset | ~reset_n;
  assign enable_signal = software_enable;

  //
  // estop_in is or'd with EMS in command register.
  //
  assign estop_signal = software_estop | estop_in;

  assign c_timing_generator_idle = !c_timing_generator_busy && c_remove_buffer_empty;

  //
  // Processes (always blocks)
  //

  //
  // This keeps the status_register up to date with current signal status.
  //
  always_ff @(posedge clock) begin

      if (~reset_n) begin
          status_register <= 0;
`ifdef DEBUG_AVALON_MM_SLAVE
          debug_always1_clock_counter <= 0;
`endif
      end
      else begin
`ifdef DEBUG_AVALON_MM_SLAVE
          debug_always1_clock_counter += debug_always1_clock_counter;
`endif
          status_register <=
              c_insert_buffer_full |
              (c_remove_buffer_empty << 1) |
              (c_timing_generator_error << 2) |
              (c_timing_generator_busy << 3) |
              (c_timing_generator_idle << 4) |
              (estop_signal << 5);
      end

  end

  //
  // This always block processes the register address decode and assignments.
  //
  always_ff @(posedge clock) begin

      if (~reset_n) begin
          read_data_register <= 0;
          command_register <= 0;
          fifo_write_signal <= 0;
          reg_clear_buffer <= 0;

          x_insert_instruction <= 0; 
          x_insert_pulse_period <= 0; 
          x_insert_pulse_count <= 0; 
          x_insert_pulse_width <= 0; 

          y_insert_instruction <= 0; 
          y_insert_pulse_period <= 0; 
          y_insert_pulse_count <= 0; 
          y_insert_pulse_width <= 0; 

          z_insert_instruction <= 0; 
          z_insert_pulse_period <= 0; 
          z_insert_pulse_count <= 0; 
          z_insert_pulse_width <= 0; 

          a_insert_instruction <= 0; 
          a_insert_pulse_period <= 0; 
          a_insert_pulse_count <= 0; 
          a_insert_pulse_width <= 0; 

`ifdef DEBUG_AVALON_MM_SLAVE
          last_write_register <= 0;
          last_read_register <= 0;
          last_write_address_register <= 0;
          last_read_address_register <= 0;

          nc_last_write_register <= 0;
          nc_last_read_register <= 0;
          nc_last_write_address_register <= 0;
          nc_last_read_address_register <= 0;

          nc_last_write_waitstate <= 0;
          nc_last_read_waitstate <= 0;
          debug_always2_clock_counter <= 0;
`endif

`ifdef DEBUG_AVALON_MM_SLAVE
`endif


      end
      else begin

          //
          // not reset_n case
          //

`ifdef DEBUG_AVALON_MM_SLAVE

          debug_always2_clock_counter += debug_always2_clock_counter;

          // Process next clock read/write data.
          if (nc_last_write_waitstate == 1'b1) begin

              nc_last_write_register <= slave_writedata;
              nc_last_write_address_register <= slave_address;

              nc_last_write_waitstate <= 1'b0;
          end
`endif

          //
          // The command register COMMAND_CMD signal indicates
          // to write the axis registers into the FIFO to the
          // timing generators.
          //
          // This gets toggled to off to ensure it does not
          // remain stuck on.
          //

          if ((command_register & COMMAND_CMD) != 0) begin
              fifo_write_signal <= 1;
              command_register <= (command_register & ~COMMAND_CMD);
          end
          else if (fifo_write_signal == 1'b1) begin

              //
              // Finished processing a FIFO write command
              //
              fifo_write_signal <= 0;
          end

          //
          // Process memory slave operations
          //

	  if (slave_write == 1'b1) begin

	      //
	      // Write register operation
	      //

`ifdef DEBUG_AVALON_MM_SLAVE
              last_write_register <= slave_writedata;
              last_write_address_register <= slave_address;

              // Setup to capture again at the next clock
              nc_last_write_waitstate <= 1'b1;
`endif

	      case (slave_address)

		  STATUS_REGISTER: begin

		      //
		      // Status register writes are ignored as
		      // it only indicates status, and there are
		      // no "sticky" bits to reset.
		      //

		  end

		  COMMAND_REGISTER: begin

		      //
		      // Call task that will directly access state variables
		      // within this module.
		      //
		      //process_command_register_write();

                      command_register <= slave_writedata;
		  end

		  //
		  // Axis register writes are stored until a future
		  // command register CMD == 1 is written, and then
		  // the current values of all axis registers are
		  // transferred to the FIFO's in the same clock cycle
		  // together.
		  //
		  // They may be read back by the software at any time
		  // to see their current value.
		  //

		  X_AXIS_PULSE_PERIOD: begin
		      x_insert_pulse_period <= slave_writedata;
		  end

		  X_AXIS_PULSE_COUNT: begin
		      x_insert_pulse_count <= slave_writedata;
		  end

		  X_AXIS_PULSE_WIDTH: begin
		      x_insert_pulse_width <= slave_writedata;
		  end

		  X_AXIS_PULSE_INST: begin
		      x_insert_instruction <= slave_writedata[c_instruction_width-1:0];
		  end

		  Y_AXIS_PULSE_PERIOD: begin
		      y_insert_pulse_period <= slave_writedata;
		  end

		  Y_AXIS_PULSE_COUNT: begin
		      y_insert_pulse_count <= slave_writedata;
		  end

		  Y_AXIS_PULSE_WIDTH: begin
		      y_insert_pulse_width <= slave_writedata;
		  end

		  Y_AXIS_PULSE_INST: begin
		      y_insert_instruction <= slave_writedata[c_instruction_width-1:0];
		  end

		  Z_AXIS_PULSE_PERIOD: begin
		      z_insert_pulse_period <= slave_writedata;
		  end

		  Z_AXIS_PULSE_COUNT: begin
		      z_insert_pulse_count <= slave_writedata;
		  end

		  Z_AXIS_PULSE_WIDTH: begin
		      z_insert_pulse_width <= slave_writedata;
		  end

		  Z_AXIS_PULSE_INST: begin
		      z_insert_instruction <= slave_writedata[c_instruction_width-1:0];
		  end

		  A_AXIS_PULSE_PERIOD: begin
		      a_insert_pulse_period <= slave_writedata;
		  end

		  A_AXIS_PULSE_COUNT: begin
		      a_insert_pulse_count <= slave_writedata;
		  end

		  A_AXIS_PULSE_WIDTH: begin
		      a_insert_pulse_width <= slave_writedata;
		  end

		  A_AXIS_PULSE_INST: begin
		      a_insert_instruction <= slave_writedata[c_instruction_width-1:0];
		  end

		  //
		  // These are not defined so they are handled
		  // by the default action which reads 0, writes ignored.
		  //
		  // RESERVED0_REGISTER:
		  // RESERVED1_REGISTER:
		  // FALLTHROUGH
		  //

		  default: begin

		      //
		      // Reserved or unaligned address accessed.
		      //
		      // Writes are ignored.
		      //

		  end

	      endcase

	  end // write register operation

	  if (slave_read == 1'b1) begin

	      //
	      // Read register operation
	      //
	      // Note: The data output signals can't be written
	      // to directly since its a tri-state with a continous
	      // assign. So read_data_register is written here, which
	      // is part of the evaluation for the tri-state assign.
	      //

`ifdef DEBUG_AVALON_MM_SLAVE
              last_read_address_register <= slave_address;
`endif

	      case (slave_address)

		  STATUS_REGISTER: begin

		      //
		      // Status register read bit values are real time
		      // as there are no "sticky" bits to clear after read.
		      //
		      // There is a separate always_ff @ process that keeps
		      // status_register up to date with current signal
		      // and condition status.
		      //

		      read_data_register <= status_register;
`ifdef DEBUG_AVALON_MM_SLAVE
                      last_read_register <= status_register;
`endif
		  end

		  COMMAND_REGISTER: begin

		      //
		      // Command register read values are real time
		      // and reads doe not cause any bit changes by
                      // themselves.
                      //
                      // Note that command register bits that have been
                      // written with a command will read as clear once
                      // the command has been accepted by the command register
                      // state machine.
		      //

		      read_data_register <= command_register;

`ifdef DEBUG_AVALON_MM_SLAVE
                      last_read_register <= command_register;
`endif
		  end

		  //
		  // Axis registers are read back to allow the
		  // microprocessor to see/verify set values.
		  //
		  // They do not cause any actions to be triggered.
		  //

		  X_AXIS_PULSE_PERIOD: begin
		      read_data_register <= x_insert_pulse_period;
		  end

		  X_AXIS_PULSE_COUNT: begin
		      read_data_register <= x_insert_pulse_count;
		  end

		  X_AXIS_PULSE_WIDTH: begin
		      read_data_register <= x_insert_pulse_width;
		  end

		  X_AXIS_PULSE_INST: begin
		      read_data_register <= x_insert_instruction;
		  end

		  Y_AXIS_PULSE_PERIOD: begin
		      read_data_register <= y_insert_pulse_period;
		  end

		  Y_AXIS_PULSE_COUNT: begin
		      read_data_register <= y_insert_pulse_count;
		  end

		  Y_AXIS_PULSE_WIDTH: begin
		      read_data_register <= y_insert_pulse_width;
		  end

		  Y_AXIS_PULSE_INST: begin
		      read_data_register <= y_insert_instruction;
		  end

		  Z_AXIS_PULSE_PERIOD: begin
		      read_data_register <= z_insert_pulse_period;
		  end

		  Z_AXIS_PULSE_COUNT: begin
		      read_data_register <= z_insert_pulse_count;
		  end

		  Z_AXIS_PULSE_WIDTH: begin
		      read_data_register <= z_insert_pulse_width;
		  end

		  Z_AXIS_PULSE_INST: begin
		      read_data_register <= z_insert_instruction;
		  end

		  A_AXIS_PULSE_PERIOD: begin
		      read_data_register <= a_insert_pulse_period;
		  end

		  A_AXIS_PULSE_COUNT: begin
		      read_data_register <= a_insert_pulse_count;
		  end

		  A_AXIS_PULSE_WIDTH: begin
		      read_data_register <= a_insert_pulse_width;
		  end

		  A_AXIS_PULSE_INST: begin
		      read_data_register <= a_insert_instruction;
		  end

		  //
		  // These are not defined so they are handled
		  // by the default action which reads 0, writes ignored.
		  //
		  // RESERVED0_REGISTER:
		  // RESERVED1_REGISTER:
		  // FALLTHROUGH
		  //

		  default: begin

		      //
		      // Reserved or unaligned address accessed.
		      //
		      // Reads are 0.
		      //
		      read_data_register <= 0;

		  end

	      endcase

	  end // read register operation

      end // not reset_n

  end // end always posedge clock

`ifdef USE_TASK

task process_command_register_write;
  begin

      //
      // These variable are in the scope of the contained module for
      // this task.
      //

      command_register <= slave_writedata;

      //
      // CBF == 1, clear FIFO buffers.
      // Currently not implemented, use RST.
      //
      // The following command register bits are wired to
      // continous assigns and do not require direct state
      // machine action.
      //
      // RST:
      // EAN:
      // EMS:
      //

      //
      // CMD == 1, write axis registers to FIFO's.
      //
      //if ((command_register & COMMAND_CMD) != 0) begin
      // command_register does not update till next clock in this case

      if ((slave_writedata & COMMAND_CMD) != 0) begin
          fifo_write_signal <= 1;
      end

  end

endtask

`endif // USE_TASK

endmodule // timing_generator_registers

`define d_assert_timing_generator_registers(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

module tb_timing_generator_registers();

  parameter c_address_width = 7;
  parameter c_data_width = 32;
  parameter c_signal_width = 32;
  parameter c_instruction_width = 4;

  //
  // Declare local variables to connect to the device under test (DUT)
  //
  reg clock_50;
  reg reset_n;

  reg [c_address_width-1:0] slave_address;
  reg slave_read;
  reg slave_write;
  reg [c_data_width-1:0]    slave_readdata;
  reg [c_data_width-1:0]    slave_writedata;
  reg [3:0] slave_byteenable;

  reg estop_in;
  
  wire x_timing_generator_pulse;
  wire x_timing_generator_direction;

  wire y_timing_generator_pulse;
  wire y_timing_generator_direction;

  wire z_timing_generator_pulse;
  wire z_timing_generator_direction;

  wire a_timing_generator_pulse;
  wire a_timing_generator_direction;

  //
  // local variables to track simulation state
  //
  reg [31:0] number_of_insert_instructions;
  reg [31:0] number_of_timing_generator_busy;

  reg timing_generator_running;
  reg [31:0] status_register_tmp;
  reg [1:0] status_waiter_state;

  //
  // 7 bit addresses for register bank.
  //
  // Note: The address bus represents each 32 bit
  // word with the bytes selected by byte strobes.
  //
  parameter[6:0]
      STATUS_REGISTER     = 7'd00,
      COMMAND_REGISTER    = 7'd01,
      RESERVED0_REGISTER  = 7'd02,
      RESERVED1_REGISTER  = 7'd03,

      X_AXIS_PULSE_PERIOD = 7'd04,
      X_AXIS_PULSE_COUNT  = 7'd05,
      X_AXIS_PULSE_WIDTH  = 7'd06,
      X_AXIS_PULSE_INST   = 7'd07,

      Y_AXIS_PULSE_PERIOD = 7'd08,
      Y_AXIS_PULSE_COUNT  = 7'd09,
      Y_AXIS_PULSE_WIDTH  = 7'd10,
      Y_AXIS_PULSE_INST   = 7'd11,

      Z_AXIS_PULSE_PERIOD = 7'd12,
      Z_AXIS_PULSE_COUNT  = 7'd13,
      Z_AXIS_PULSE_WIDTH  = 7'd14,
      Z_AXIS_PULSE_INST   = 7'd15,

      A_AXIS_PULSE_PERIOD = 7'd16,
      A_AXIS_PULSE_COUNT  = 7'd17,
      A_AXIS_PULSE_WIDTH  = 7'd18,
      A_AXIS_PULSE_INST   = 7'd19;

  //
  // Command register bits
  //
  parameter[31:0]
      COMMAND_RST = 32'h01,
      COMMAND_CFB = 32'h02,
      COMMAND_EAN = 32'h04,
      COMMAND_EMS = 32'h08,
      COMMAND_CMD = 32'h10;

  //
  // Status register bits
  //
  parameter[31:0]
      STATUS_FBF = 32'h01,
      STATUS_FBE = 32'h02,
      STATUS_ERR = 32'h04,
      STATUS_BSY = 32'h08,
      STATUS_IDL = 32'h10,
      STATUS_EMS = 32'h20;

  parameter[4:0]
      DWELL                   = 4'b0000,
      PULSE_CLOCKWISE         = 4'b0010,
      PULSE_COUNTER_CLOCKWISE = 4'b0011;

  parameter[1:0]
      STATUS_WAITER_IDLE       = 2'b00,
      STATUS_WAITER_DATA_READY = 2'b01,
      STATUS_WAITER_DELAY_WAIT = 2'b10,
      STATUS_WAITER_INVALID    = 2'b11;

  //
  // Create an instance of Device Under Test.
  //
  timing_generator_registers DUT(
      .clock(clock_50),
      .reset_n(reset_n),

      .slave_address(slave_address),
      .slave_read(slave_read),
      .slave_write(slave_write),
      .slave_readdata(slave_readdata),
      .slave_writedata(slave_writedata),
      .slave_byteenable(slave_byteenable),

      .estop_in(estop_in),

      .x_timing_generator_pulse(x_timing_generator_pulse),
      .x_timing_generator_direction(x_timing_generator_direction),

      .y_timing_generator_pulse(y_timing_generator_pulse),
      .y_timing_generator_direction(y_timing_generator_direction),

      .z_timing_generator_pulse(z_timing_generator_pulse),
      .z_timing_generator_direction(z_timing_generator_direction),

      .a_timing_generator_pulse(a_timing_generator_pulse),
      .a_timing_generator_direction(a_timing_generator_direction)
  );
   
  // Setup initial values
  initial begin
     clock_50 = 0;
     reset_n = 1;

     slave_address = 0;
     slave_read = 0;
     slave_write = 0;
     slave_readdata = 0;
     slave_writedata = 0;
     slave_byteenable = 0;

     estop_in = 0;

     timing_generator_running = 0;
     status_register_tmp = 0;
     status_waiter_state = STATUS_WAITER_IDLE;

     number_of_insert_instructions = 4;
     number_of_timing_generator_busy = 0;
  end

  //
  // Setup 50Mhz clock at 1ns resolution.
  //
  // 50mhz == 20ns period.
  // #10 delay is 1/2 of the cycle.
  //
  always #10 clock_50 = ~clock_50;

  // Stimulus to step through values
  initial begin

     //
     // Each line is executed after the specified simulation cycles
     // Setting the local variable hooked up to the given input name of the
     // DUT asserts that value.
     //

     reset_n = 0;

     @(posedge clock_50);

     reset_n = 1;

     @(posedge clock_50);

     //           instr                    period count width
     write_x_axis(PULSE_CLOCKWISE,         357,   16,   1100);
     write_y_axis(PULSE_COUNTER_CLOCKWISE, 357,   16,   1100);
     write_z_axis(PULSE_COUNTER_CLOCKWISE, 357,    8,   1100);
     write_a_axis(DWELL,                   357,   16,   1100);
     write_command_sync();

     // Now enable it
     write_enable_sync();

     // Give it time
     #2000

     //
     // Indicate the timing generator is running to the
     // always block which will monitor the status register
     // until its complete.
     //
     // For the initial synchronous pass of the test the
     // this flag prevents the always @() block from interfering
     // with our register access similar to a spinlock/lock
     // in a multitasking system running on the ARM SoC's.
     //
     timing_generator_running = 1;

     @(posedge clock_50);
     @(posedge clock_50);

     @(posedge clock_50);
     @(posedge clock_50);

   end

`ifdef ASYNC_LOAD

   //
   // WORK IN PROGRESS 05/09/2019
   //
   // Still wiring up the arbiter, and must design/build
   // the async command loader.
   //
   // WORK IN PROGRESS 05/09/2019
   //
   // The async load case simulates operation of a circuit
   // in which two always() async processes are accessing
   // the register file.
   //
   // One set of accesses are writing new commands, while
   // the other set is reading the status waiting for completion
   // as to stop the test.
   //
   // The synchronous version of the test works by having the
   // synchronous initial block send commands, then set a flag
   // for the async always @() that polls the status register
   // for test completion. This way the status register polling
   // is held off until all commands are entered. This simulates
   // actual usage in the real ARM SoC system in which a lock
   // protects the register file from multiple operations from
   // the two cores, or a single core with interrupt driven
   // access/multitasking.
   //

   //
   // Arbiter for common register access.
   //
   // Written so that it can be re-used as synthesized circuit.
   //
   // Currently A has priority.
   //
   // Arbiter allows 50/50 access when there is conflict.
   //
   // TODO: This should be a module so it maybe re-used.
   //

   // input
   wire arbiter_request_a;
   wire arbiter_request_b;

   // output
   reg arbiter_grant_a;
   reg arbiter_grant_b;

   // state variables
   reg arbiter_lock;
   reg arbiter_last_grant; // 0 == A, 1 == B

   always @(posedge clock_50 or ~reset_n) begin

       if (~reset_n) begin
           arbiter_lock <= 0;
           arbiter_last_grant <= 0;
           arbiter_grant_a <= 0;
           arbiter_grant_b <= 0;
       end
       else begin

	   if (arbiter_lock == 1'b0) begin
    
               //
               // Arbiter is free. See if any requests are available.
               //
               // Depending on which requestor was last granted, check
               // the other first to enable round robin access when
               // there is a conflict.
               //
    	       if (arbiter_last_grant == 1'b0) begin
                   // A was last, check B first
		   if (arbiter_request_b == 1'b1) begin
		       arbiter_lock <= 1'b1;
		       arbiter_grant_b <= 1'b1;
		       arbiter_last_grant <= 1'b1; // B
		   end
		   else if (arbiter_request_a == 1'b1) begin
		       arbiter_lock <= 1'b1;
		       arbiter_grant_a <= 1'b1;
		       arbiter_last_grant <= 1'b0; // A
		   end
               end // if last_grant
               else begin
                   // B was last, check A first
		   if (arbiter_request_a == 1'b1) begin
		       arbiter_lock <= 1'b1;
		       arbiter_grant_a <= 1'b1;
		       arbiter_last_grant <= 1'b0; // A
		   end
		   else if (arbiter_request_b == 1'b1) begin
		       arbiter_lock <= 1'b1;
		       arbiter_grant_b <= 1'b1;
		       arbiter_last_grant <= 1'b1; // B
		   end
               end // else last_grant
	   end // end if arbiter_lock held
           else begin

               //
               // Arbiter is granted. See if any requests have deasserted.
               //
               if ((arbiter_grant_a == 1'b1) &&
                   (arbiter_request_a == 1'b0)) begin

                   // A is granted, but has released its request.
		   arbiter_grant_a <= 1'b0;
		   arbiter_lock <= 1'b0;
               end
               else begin
                   if ((arbiter_grant_b == 1'b1) &&
                       (arbiter_request_b == 1'b0)) begin

                       // B is granted, but has released its request.
		       arbiter_grant_b <= 1'b0;
		       arbiter_lock <= 1'b0;
                   end
               end

           end // end else arbiter_lock held

       end // end if reset_n
   end

   //
   // This asynchronously feeds requests using the arbiter
   //

   always @(posedge clock_50) begin

       if (timing_generator_running) begin

         //
         // TODO: This must be done with compound state machines.
    	 //
	 // Add another instruction while running the first one.
	 //
	 // Note: This appears to conflict, need to write
	 // an arbiter.
	 //

	 //           instr                    period count width
	 //write_x_axis(PULSE_CLOCKWISE,         357,   16,   1100);
	 //write_y_axis(PULSE_COUNTER_CLOCKWISE, 357,   16,   1100);
	 //write_z_axis(PULSE_COUNTER_CLOCKWISE, 357,    8,   1100);
	 //write_a_axis(DWELL,                   357,   16,   1100);
	 //write_command_sync();

       end

   end // end always @()

`endif // ASYNC_LOAD

   //
   // State machine for reading status register.
   //
   // Note: Current implementation polls hard, should
   // add a delay between polls.
   //
   // TODO: Need a lock/arbiter for access to the registers
   // since the low level signal manipulation here will be
   // in parallel with the initial inline code.
   //

   always @(posedge clock_50) begin

       if (timing_generator_running) begin

           //
           // Note: read_register_sync() can't be used here
           // since its only for initial blocks which are
           // synchronous.
           //

           //read_register_sync(STATUS_REGISTER, status_register_tmp);

           case (status_waiter_state)

               STATUS_WAITER_IDLE: begin
                   slave_address <= STATUS_REGISTER;
                   slave_byteenable <= 4'b1111;
                   slave_read <= 1'b1;
                   status_waiter_state <= STATUS_WAITER_DATA_READY;
               end

               STATUS_WAITER_DATA_READY: begin
                   status_register_tmp <= slave_readdata;
                   slave_read <= 1'b0;
                   slave_byteenable <= 4'b0000;

                   // Hard spinloop on status register
                   //status_waiter_state <= STATUS_WAITER_IDLE;

                   // 1 clock delay between tests
                   status_waiter_state <= STATUS_WAITER_DELAY_WAIT;

                   if (status_register_tmp & STATUS_IDL) begin
                       $stop;
                   end

               end

               STATUS_WAITER_DELAY_WAIT: begin
                   status_waiter_state <= STATUS_WAITER_IDLE;
               end

               STATUS_WAITER_INVALID: begin
               end

           endcase

       end // end timing_generator_running
   end // end always @()

//
// Task to write the command.
//
// This causes all the axis registers to be loaded into
// the FIFO's.
//
// This is synchronous and can only be called from
// initial blocks during simulation.
//
task write_command_sync;

    begin
        // Command load
        write_register_sync(COMMAND_REGISTER, COMMAND_CMD);
    end

endtask

//
// Task to set the Enable bit.
//
// This is synchronous and can only be called from
// initial blocks during simulation.
//
task write_enable_sync;

    begin
        // Enable (run)
        write_register_sync(COMMAND_REGISTER, COMMAND_EAN);
    end

endtask

//
// Task to write the x_axis
//
task write_x_axis;
    input [c_instruction_width-1:0] instruction_arg;
    input [c_data_width-1:0]    pulse_period_arg;
    input [c_data_width-1:0]    pulse_count_arg;
    input [c_data_width-1:0]    pulse_width_arg;

    begin
        write_register_sync(X_AXIS_PULSE_PERIOD, pulse_period_arg);
        write_register_sync(X_AXIS_PULSE_COUNT,  pulse_count_arg);
        write_register_sync(X_AXIS_PULSE_WIDTH,  pulse_width_arg);
        write_register_sync(X_AXIS_PULSE_INST,   instruction_arg);
    end

endtask

//
// Task to write the y_axis
//
task write_y_axis;
    input [c_instruction_width-1:0] instruction_arg;
    input [c_data_width-1:0]    pulse_period_arg;
    input [c_data_width-1:0]    pulse_count_arg;
    input [c_data_width-1:0]    pulse_width_arg;

    begin
        write_register_sync(Y_AXIS_PULSE_PERIOD, pulse_period_arg);
        write_register_sync(Y_AXIS_PULSE_COUNT,  pulse_count_arg);
        write_register_sync(Y_AXIS_PULSE_WIDTH,  pulse_width_arg);
        write_register_sync(Y_AXIS_PULSE_INST,   instruction_arg);
    end

endtask

//
// Task to write the z_axis
//
task write_z_axis;
    input [c_instruction_width-1:0] instruction_arg;
    input [c_data_width-1:0]    pulse_period_arg;
    input [c_data_width-1:0]    pulse_count_arg;
    input [c_data_width-1:0]    pulse_width_arg;

    begin
        write_register_sync(Z_AXIS_PULSE_PERIOD, pulse_period_arg);
        write_register_sync(Z_AXIS_PULSE_COUNT,  pulse_count_arg);
        write_register_sync(Z_AXIS_PULSE_WIDTH,  pulse_width_arg);
        write_register_sync(Z_AXIS_PULSE_INST,   instruction_arg);
    end

endtask

//
// Task to write the a_axis
//
task write_a_axis;
    input [c_instruction_width-1:0] instruction_arg;
    input [c_data_width-1:0]    pulse_period_arg;
    input [c_data_width-1:0]    pulse_count_arg;
    input [c_data_width-1:0]    pulse_width_arg;

    begin
        write_register_sync(A_AXIS_PULSE_PERIOD, pulse_period_arg);
        write_register_sync(A_AXIS_PULSE_COUNT,  pulse_count_arg);
        write_register_sync(A_AXIS_PULSE_WIDTH,  pulse_width_arg);
        write_register_sync(A_AXIS_PULSE_INST,   instruction_arg);
    end

endtask

//
// Task to write a register.
//
// This is synchronous and can only be called from
// initial blocks during simulation.
//
task write_register_sync;
    input [c_address_width-1:0] address_arg;
    input [c_data_width-1:0]    data_arg;

    begin

     //
     // Note: The task can access the local variables of the module
     // its contained within. Be careful when defining its inputs
     // above to ensure unique names.
     //

     slave_address = address_arg;
     slave_byteenable = 4'b1111;

     slave_writedata = data_arg;

     #20 slave_write = 1'b1;

     // de-assert write signal
     #20 slave_write = 1'b0;
     slave_byteenable = 4'b0000;

    end

endtask

//
// Task to read a register.
//
// This is synchronous and can only be called from
// initial blocks during simulation.
//
task read_register_sync;
    input [c_address_width-1:0] address_arg;
    output [c_data_width-1:0]    data_arg;

    begin

     //
     // Note: The task can access the local variables of the module
     // its contained within. Be careful when defining its inputs
     // above to ensure unique names.
     //

     slave_address = address_arg;
     slave_byteenable = 4'b1111;

     #20 slave_read = 1'b1;

     data_arg = slave_readdata;

     #20 slave_read = 1'b0;
     slave_byteenable = 4'b0000;

    end

endtask

endmodule
