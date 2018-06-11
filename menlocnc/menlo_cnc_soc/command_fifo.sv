
// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 04/12/2018
//
// Command FIFO for timing_generator.
//
// Buffers commands to allow the timing generator to conduct
// machine movements without pauses.
//
// Provides an interface to the vendor optimized FIFO implementations.
//
// Information stored in the FIFO:
//
// Note: There is an insert_ (write) side, and a remove (read) side
// to the FIFO which operate independently.
//
//  1) Pulse period (frequency)
//
//   A 32 bit value controls the pulse period in relation to the input clock.
//
//  2) Pulse Count
//
//   A 32 bit value specifies the pulse count until termination.
//
//  3) Pulse width
//
//   A 32 bit value that controls the pulse width in relation to the
//   input clock.
//
//  4) Instruction
//
//    Bit 0: DIRECTION
//
//      0 == clockwise motor direction
//      1 == counter-clockwise motor direction
//
//    Bit 1: ACTION
//
//      0 == DWELL
//        Executes pulse count at the pulse period but does not generate
//        an output pulse. This is a programmed machine delay function.
//
//      1 == PULSE
//        Generates the output pulse for machine movement according to
//        the pulse_period, pulse_count, and pulse_width parameters.
//
//    Bit 2: Reserved, must be 0
//
//    Bit 3: Reserved, must be 0
//
module command_fifo
(
    clear_buffer,

    //
    // Input (write FIFO) side
    //
    insert_clock,
    insert_signal,
    insert_instruction,
    insert_pulse_period,
    insert_pulse_count,
    insert_pulse_width,
    insert_buffer_full,

    //
    // output (read FIFO) side
    //
    remove_clock,
    remove_signal,
    remove_instruction,
    remove_pulse_period,
    remove_pulse_count,
    remove_pulse_width,
    remove_buffer_empty
);

  //
  // Note: Ensure these match the tool configured DCFIFO instances.
  //

  // Depth is shared by all FIFO's and must be configured as such.
  parameter c_depth = 512;

  parameter c_signal_width = 32;
  parameter c_instruction_width = 4;

  //
  // Shared signals
  //
  // DCFIFO is configured to synchronize clear on either the
  // read clock or the write clock.
  //

  input                        clear_buffer;

  //
  // Input (write FIFO) side
  //

  input                        insert_clock;
  input                        insert_signal;
  input [c_instruction_width-1:0] insert_instruction;
  input [c_signal_width-1:0]   insert_pulse_period;
  input [c_signal_width-1:0]   insert_pulse_count;
  input [c_signal_width-1:0]   insert_pulse_width;

  output                       insert_buffer_full;

  //
  // output (read FIFO) side
  //

  // FIFO inputs
  input                        remove_clock;
  input                        remove_signal;

  // FIFO outputs
  output [c_instruction_width-1:0] remove_instruction;
  output [c_signal_width-1:0]   remove_pulse_period;
  output [c_signal_width-1:0]   remove_pulse_count;
  output [c_signal_width-1:0]   remove_pulse_width;

  output                       remove_buffer_empty;

  //
  // local signals, registers
  //
  // reset to 0 at startup by FPGA
  //
  reg instruction_reserved [1:0];

  //
  // These are OR's of multiple FIFO's to a common signal
  //

  wire instruction_fifo_insert_buffer_full;
  wire pulse_period_fifo_insert_buffer_full;
  wire pulse_count_fifo_insert_buffer_full;
  wire pulse_width_fifo_insert_buffer_full;

  wire instruction_fifo_remove_buffer_empty;
  wire pulse_period_fifo_remove_buffer_empty;
  wire pulse_count_fifo_remove_buffer_empty;
  wire pulse_width_fifo_remove_buffer_empty;

  assign insert_buffer_full = 
      instruction_fifo_insert_buffer_full  ||
      pulse_period_fifo_insert_buffer_full ||
      pulse_count_fifo_insert_buffer_full  ||
      pulse_width_fifo_insert_buffer_full;

  assign remove_buffer_empty =
      instruction_fifo_remove_buffer_empty  ||
      pulse_period_fifo_remove_buffer_empty ||
      pulse_count_fifo_remove_buffer_empty  ||
      pulse_width_fifo_remove_buffer_empty;

  //
  // Invoke sub-module instances with wiring to the inputs/outputs.
  //

  //
  // Invoke Altera MegaFunction DCFIFO
  //
  // Dual Clock FIFO
  //
  // Dual clock is used since the write side is an external microcontroller,
  // while the read side is in the timing domain of the timing generator.
  //

  //
  // This instance was created using the Quartus II Altera IP configuration tool
  // for the built in DCFIFO, which is a free license MegaFunction.
  //
  // It's used since its tightly integrated with the FIFO logic on the FPGA
  // including specialized circuits for integrating between the two clock domains.
  //

  dcfifo4 instruction_fifo(

     // Common signals
     .aclr(clear_buffer),

     // Insert/write side of FIFO
     .wrclk(insert_clock),
     .wrreq(insert_signal),
     .wrfull(instruction_fifo_insert_buffer_full),
     .data(insert_instruction),

     // Remove/read side of FIFO
     .rdclk(remove_clock),
     .rdreq(remove_signal),
     .rdempty(instruction_fifo_remove_buffer_empty),
     .q(remove_instruction)
     );

  dcfifo32 pulse_period_fifo(

     // Common signals
     .aclr(clear_buffer),

     // Insert/write side of FIFO
     .wrclk(insert_clock),
     .wrreq(insert_signal),
     .wrfull(pulse_period_fifo_insert_buffer_full),
     .data(insert_pulse_period),

     // Remove/read side of FIFO
     .rdclk(remove_clock),
     .rdreq(remove_signal),
     .rdempty(pulse_period_fifo_remove_buffer_empty),
     .q(remove_pulse_period)
     );

  dcfifo32 pulse_count_fifo(

     // Common signals
     .aclr(clear_buffer),

     // Insert/write side of FIFO
     .wrclk(insert_clock),
     .wrreq(insert_signal),
     .wrfull(pulse_count_fifo_insert_buffer_full),
     .data(insert_pulse_count),

     // Remove/read side of FIFO
     .rdclk(remove_clock),
     .rdreq(remove_signal),
     .rdempty(pulse_count_fifo_remove_buffer_empty),
     .q(remove_pulse_count)
     );

  dcfifo32 pulse_width_fifo(

     // Common signals
     .aclr(clear_buffer),

     // Insert/write side of FIFO
     .wrclk(insert_clock),
     .wrreq(insert_signal),
     .wrfull(pulse_width_fifo_insert_buffer_full),
     .data(insert_pulse_width),

     // Remove/read side of FIFO
     .rdclk(remove_clock),
     .rdreq(remove_signal),
     .rdempty(pulse_width_fifo_remove_buffer_empty),
     .q(remove_pulse_width)
     );

endmodule

`define d_assert3(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

module tb_command_fifo();

  parameter c_signal_width = 32;
  parameter c_instruction_width = 4;

  //
  // Declare local variables to connect to the device under test (DUT)
  //
  reg clock_50;

  reg clear_buffer;

  reg insert_signal;
  reg [c_instruction_width-1:0] insert_instruction;
  reg [c_signal_width-1:0] insert_pulse_period;
  reg [c_signal_width-1:0] insert_pulse_count;
  reg [c_signal_width-1:0] insert_pulse_width;
  wire insert_buffer_full;

  reg remove_signal;
  wire [c_instruction_width-1:0] remove_instruction;
  wire [c_signal_width-1:0] remove_pulse_period;
  wire [c_signal_width-1:0] remove_pulse_count;
  wire [c_signal_width-1:0] remove_pulse_width;
  wire remove_buffer_empty;

  //
  // This holds the value read from fetch_instruction
  // from the FIFO.
  //
  reg [c_instruction_width-1:0] reg_remove_instruction;
  reg [c_signal_width-1:0] reg_remove_pulse_period;
  reg [c_signal_width-1:0] reg_remove_pulse_count;
  reg [c_signal_width-1:0] reg_remove_pulse_width;

  //
  // Simulation control variables.
  //
  reg async_read;
  integer async_read_count;



  //
  // Create an instance of Device Under Test.
  //
  command_fifo DUT(

      .clear_buffer(clear_buffer),

      .insert_clock(clock_50),
      .insert_signal(insert_signal),
      .insert_instruction(insert_instruction),
      .insert_pulse_period(insert_pulse_period),
      .insert_pulse_count(insert_pulse_count),
      .insert_pulse_width(insert_pulse_width),
      .insert_buffer_full(insert_buffer_full),

      .remove_clock(clock_50),
      .remove_signal(remove_signal),
      .remove_instruction(remove_instruction),
      .remove_pulse_period(remove_pulse_period),
      .remove_pulse_count(remove_pulse_count),
      .remove_pulse_width(remove_pulse_width),
      .remove_buffer_empty(remove_buffer_empty)
  );
   
  // Setup initial values
  initial begin
     clock_50 = 0;

     // These values start disabled
     async_read = 0;
     async_read_count = 0;

     clear_buffer =0;
     insert_signal = 0;
     remove_signal = 0;

     insert_instruction = 0;
     insert_pulse_period = 0;
     insert_pulse_count = 0;
     insert_pulse_width = 0;

     reg_remove_instruction = 0;
     reg_remove_pulse_period = 0;
     reg_remove_pulse_count = 0;
     reg_remove_pulse_width = 0;
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

     @(negedge clock_50);
     #1000

     @(negedge clock_50);

     //
     // Assert insert_buffer_full == 0
     //
     `d_assert3(insert_buffer_full, 1'b0)

     //
     // First sequentially load commands, assert indication
     // signals, and sequentually fetch the commands.
     //

     //
     // PULSE Clockwise 
     //
     //             instr period count width
     load_instruction(2,    4,     4,    1);

     //
     // PULSE Counter Clockwise 
     //
     //             instr period count width
     load_instruction(3,    4,    16,    4);

     //
     // DWELL No Pulse for the pulse_period, pulse_count.
     //
     //             instr period count width
     load_instruction(0,    4,    16,   4);

     load_instruction(0,    4,    16,   4);

     @(posedge clock_50);

     //
     // Assert insert_buffer_full == 0
     //
     `d_assert3(insert_buffer_full, 1'b0)


     //
     // Assert remove_buffer_empty == 0
     //
     `d_assert3(remove_buffer_empty, 1'b0)

     @(posedge clock_50);

     //
     // fetch instruction loads the next instruction from the FIFO
     // into the local registers connected to the outputs of the
     // Device Under Test.
     //
     fetch_instruction();

     @(posedge clock_50);
     fetch_instruction();

     @(posedge clock_50);
     fetch_instruction();

     @(posedge clock_50);
     fetch_instruction();

     @(posedge clock_50);

     //
     // Now synchronously load commands, but asynchronously fetch them.
     //

     // Start async reader in always block.
     async_read = 1;

     //
     // Assert remove_buffer_empty == 1
     //
     `d_assert3(remove_buffer_empty, 1'b1)

     //
     // PULSE Clockwise 
     //
     //             instr period count width
     load_instruction(2,    4,     4,    1);

     //
     // PULSE Counter Clockwise 
     //
     //             instr period count width
     load_instruction(3,    4,    16,    4);

     //
     // DWELL No Pulse for the pulse_period, pulse_count.
     //
     //             instr period count width
     load_instruction(0,    4,    16,   4);

     load_instruction(0,    4,    16,   4);

     @(posedge clock_50);

     // Wait till all have been read
     @(posedge remove_buffer_empty);

     //
     // This is needed to ensure the last clocks are waited for an all output transitions
     // become visible in the simulation waveform bench.
     //

     @(posedge clock_50);
     @(posedge clock_50);

     #100 $stop;
   end

   //
   // Always must be outside of the sequential initial block.
   //

   always @(posedge clock_50) begin

       //
       // This always block executes during the entire
       // simulation, but only want it to perform async
       // reads during part of the simulation.
       //
       if (async_read == 1'b1) begin

           if (remove_buffer_empty == 1'b0) begin

               async_read_count = async_read_count + 1;

               if (async_read_count > 4) begin
                   $display("ASSERTION FAILED async_read_count > 4");
                   $stop;
               end

               // Fetch new instruction from FIFO.
               fetch_instruction();

           end
       end
   end

//
// Task to load values into the timing_generator.
//
task load_instruction;
    input [c_instruction_width-1:0] _instruction;
    input [c_signal_width-1:0] _period;
    input [c_signal_width-1:0] _count;
    input [c_signal_width-1:0] _width;

    begin

     //
     // Note: The task can access the local variables of the module
     // its contained within. Be careful when defining its inputs
     // above to ensure unique names.
     //

     insert_instruction = _instruction;
     insert_pulse_period = _period;
     insert_pulse_count = _count;
     insert_pulse_width = _width;

     //
     // Assert insert_buffer_full == 0
     //
     `d_assert3(insert_buffer_full, 1'b0)

     // give it 10ns setup time
     #10 insert_signal = 1'b1;

     // must be asserted for at least 1 clock period
     #20 insert_signal = 1'b0;

    end

endtask

task fetch_instruction;
    begin

     //
     // Note: The task can access the local variables of the module
     // its contained within. Be careful when defining its inputs
     // above to ensure unique names.
     //

     //
     // Assert remove_buffer_empty == 0
     //
     `d_assert3(remove_buffer_empty, 1'b0)

     // give it 10ns setup time
     #10 remove_signal = 1'b1;

     // must be asserted for at least 1 clock period
     #20 remove_signal = 1'b0;

     @(posedge clock_50);

     //
     // Read the wire outputs and load the test bench registers
     //
     reg_remove_instruction = remove_instruction;
     reg_remove_pulse_period = remove_pulse_period;
     reg_remove_pulse_count = remove_pulse_count;
     reg_remove_pulse_width = remove_pulse_width;

    end

endtask

endmodule
