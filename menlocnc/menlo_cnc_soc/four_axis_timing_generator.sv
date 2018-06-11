
// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 04/20/2018
//
// Four axis timing generator.
//
// Handles (4) machine control axis with using fifo_timing_generator.
//

module four_axis_timing_generator
(
    //
    // Common input signals
    //
    // Each axis is operated together in the same clock
    // domain to ensure synchronization.
    //
    c_clock,
    c_reset,
    c_clear_buffer,
    c_enable,
    c_estop,

    //
    // Input (write command) side
    //
    c_insert_clock,
    c_insert_signal,

    //
    // FIFO state outputs
    //
    c_insert_buffer_full,
    c_remove_buffer_empty,

    //
    // Timing generator state outputs
    //
    c_timing_generator_error,
    c_timing_generator_busy,

    //
    // X Axis
    //

    // command input
    x_insert_instruction,
    x_insert_pulse_period,
    x_insert_pulse_count,
    x_insert_pulse_width,

    // timing generator outputs
    x_timing_generator_pulse,
    x_timing_generator_direction,

    //
    // Y Axis
    //

    // command input
    y_insert_instruction,
    y_insert_pulse_period,
    y_insert_pulse_count,
    y_insert_pulse_width,

    // timing generator outputs
    y_timing_generator_pulse,
    y_timing_generator_direction,

    //
    // Z Axis
    //

    // command input
    z_insert_instruction,
    z_insert_pulse_period,
    z_insert_pulse_count,
    z_insert_pulse_width,

    // timing generator outputs
    z_timing_generator_pulse,
    z_timing_generator_direction,

    //
    // A Axis
    //

    // command input
    a_insert_instruction,
    a_insert_pulse_period,
    a_insert_pulse_count,
    a_insert_pulse_width,

    // timing generator outputs
    a_timing_generator_pulse,
    a_timing_generator_direction
);

  parameter c_signal_width = 32;
  parameter c_instruction_width = 4;

  //
  // Shared input signals
  //

  input                        c_clock;
  input                        c_reset;
  input                        c_clear_buffer;
  input                        c_enable;
  input                        c_estop;
  input                        c_insert_clock;
  input                        c_insert_signal;

  //
  // Shared output signals
  //

  output                       c_insert_buffer_full;
  output                       c_remove_buffer_empty;
  output                       c_timing_generator_error;
  output                       c_timing_generator_busy;

  //
  // Per Axis signals
  //

  //
  // X Axis
  //
  input [c_instruction_width-1:0] x_insert_instruction;
  input [c_signal_width-1:0]   x_insert_pulse_period;
  input [c_signal_width-1:0]   x_insert_pulse_count;
  input [c_signal_width-1:0]   x_insert_pulse_width;

  // Timing generator outputs
  output                       x_timing_generator_pulse;
  output                       x_timing_generator_direction;

  //
  // Y Axis
  //
  input [c_instruction_width-1:0] y_insert_instruction;
  input [c_signal_width-1:0]   y_insert_pulse_period;
  input [c_signal_width-1:0]   y_insert_pulse_count;
  input [c_signal_width-1:0]   y_insert_pulse_width;

  // Timing generator outputs
  output                       y_timing_generator_pulse;
  output                       y_timing_generator_direction;

  //
  // Z Axis
  //
  input [c_instruction_width-1:0] z_insert_instruction;
  input [c_signal_width-1:0]   z_insert_pulse_period;
  input [c_signal_width-1:0]   z_insert_pulse_count;
  input [c_signal_width-1:0]   z_insert_pulse_width;

  // Timing generator outputs
  output                       z_timing_generator_pulse;
  output                       z_timing_generator_direction;

  //
  // A Axis
  //
  input [c_instruction_width-1:0] a_insert_instruction;
  input [c_signal_width-1:0]   a_insert_pulse_period;
  input [c_signal_width-1:0]   a_insert_pulse_count;
  input [c_signal_width-1:0]   a_insert_pulse_width;

  // Timing generator outputs
  output                       a_timing_generator_pulse;
  output                       a_timing_generator_direction;

  //
  // local signals, registers
  //

  wire                         x_insert_buffer_full;
  wire                         y_insert_buffer_full;
  wire                         z_insert_buffer_full;
  wire                         a_insert_buffer_full;

  wire                         x_remove_buffer_empty;
  wire                         y_remove_buffer_empty;
  wire                         z_remove_buffer_empty;
  wire                         a_remove_buffer_empty;

  wire                         x_timing_generator_error;
  wire                         y_timing_generator_error;
  wire                         z_timing_generator_error;
  wire                         a_timing_generator_error;

  wire                         x_timing_generator_busy;
  wire                         y_timing_generator_busy;
  wire                         z_timing_generator_busy;
  wire                         a_timing_generator_busy;

  //
  // Invoke sub-module instances
  //

  //
  // X axis
  //
  fifo_timing_generator x_fifo_timing_generator(

      .clock(c_clock),
      .reset(c_reset),
      .clear_buffer(c_clear_buffer),
      .enable(c_enable),
      .estop(c_estop),

      .insert_clock(c_clock),
      .insert_signal(c_insert_signal),
      .insert_instruction(x_insert_instruction),
      .insert_pulse_period(x_insert_pulse_period),
      .insert_pulse_count(x_insert_pulse_count),
      .insert_pulse_width(x_insert_pulse_width),

      .insert_buffer_full(x_insert_buffer_full),
      .remove_buffer_empty(x_remove_buffer_empty),

      .timing_generator_error(x_timing_generator_error),
      .timing_generator_busy(x_timing_generator_busy),

      .timing_generator_pulse(x_timing_generator_pulse),
      .timing_generator_direction(x_timing_generator_direction)
  );

  //
  // Y axis
  //
  fifo_timing_generator y_fifo_timing_generator(

      .clock(c_clock),
      .reset(c_reset),
      .clear_buffer(c_clear_buffer),
      .enable(c_enable),
      .estop(c_estop),

      .insert_clock(c_clock),
      .insert_signal(c_insert_signal),

      .insert_instruction(y_insert_instruction),
      .insert_pulse_period(y_insert_pulse_period),
      .insert_pulse_count(y_insert_pulse_count),
      .insert_pulse_width(y_insert_pulse_width),

      .insert_buffer_full(y_insert_buffer_full),
      .remove_buffer_empty(y_remove_buffer_empty),

      .timing_generator_error(y_timing_generator_error),
      .timing_generator_busy(y_timing_generator_busy),

      .timing_generator_pulse(y_timing_generator_pulse),
      .timing_generator_direction(y_timing_generator_direction)
  );

  //
  // Z axis
  //
  fifo_timing_generator z_fifo_timing_generator(

      .clock(c_clock),
      .reset(c_reset),
      .clear_buffer(c_clear_buffer),
      .enable(c_enable),
      .estop(c_estop),

      .insert_clock(c_clock),
      .insert_signal(c_insert_signal),

      .insert_instruction(z_insert_instruction),
      .insert_pulse_period(z_insert_pulse_period),
      .insert_pulse_count(z_insert_pulse_count),
      .insert_pulse_width(z_insert_pulse_width),

      .insert_buffer_full(z_insert_buffer_full),
      .remove_buffer_empty(z_remove_buffer_empty),

      .timing_generator_error(z_timing_generator_error),
      .timing_generator_busy(z_timing_generator_busy),

      .timing_generator_pulse(z_timing_generator_pulse),
      .timing_generator_direction(z_timing_generator_direction)
  );

  //
  // A axis
  //
  fifo_timing_generator a_fifo_timing_generator(

      .clock(c_clock),
      .reset(c_reset),
      .clear_buffer(c_clear_buffer),
      .enable(c_enable),
      .estop(c_estop),

      .insert_clock(c_clock),
      .insert_signal(c_insert_signal),

      .insert_instruction(a_insert_instruction),
      .insert_pulse_period(a_insert_pulse_period),
      .insert_pulse_count(a_insert_pulse_count),
      .insert_pulse_width(a_insert_pulse_width),

      .insert_buffer_full(a_insert_buffer_full),
      .remove_buffer_empty(a_remove_buffer_empty),

      .timing_generator_error(a_timing_generator_error),
      .timing_generator_busy(a_timing_generator_busy),

      .timing_generator_pulse(a_timing_generator_pulse),
      .timing_generator_direction(a_timing_generator_direction)
  );

  //
  // Continous assigns
  //
  // The output signals from each axis are OR'd together
  // to indicate overall operating state/machine condition
  // to the controller.
  //

  assign c_insert_buffer_full = 
      x_insert_buffer_full | 
      y_insert_buffer_full |
      z_insert_buffer_full |
      a_insert_buffer_full;

  assign c_remove_buffer_empty =
      x_remove_buffer_empty |
      y_remove_buffer_empty |
      z_remove_buffer_empty |
      a_remove_buffer_empty;

  assign c_timing_generator_error =
      x_timing_generator_error |
      y_timing_generator_error |
      z_timing_generator_error |
      a_timing_generator_error;

  assign c_timing_generator_busy =
      x_timing_generator_busy |
      y_timing_generator_busy |
      z_timing_generator_busy |
      a_timing_generator_busy;

   //
   // Processes (always blocks)
   //

endmodule // four_axis_timing_generator

`define d_assert_four_axis_timing_generator(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

module tb_four_axis_timing_generator();

  parameter c_signal_width = 32;
  parameter c_instruction_width = 4;

  //
  // Declare local variables to connect to the device under test (DUT)
  //
  reg clock_50;
  reg reset;
  reg clear_buffer;
  reg enable;
  reg estop;
  
  reg insert_signal;
  reg [c_instruction_width-1:0] insert_instruction;
  reg [c_signal_width-1:0] insert_pulse_period;
  reg [c_signal_width-1:0] insert_pulse_count;
  reg [c_signal_width-1:0] insert_pulse_width;

  wire insert_buffer_full;
  wire remove_buffer_empty;
  wire timing_generator_error;
  wire timing_generator_busy;
  wire timing_generator_pulse;
  wire timing_generator_direction;

  // debug probes for timing_generator.
  wire debug_pulse_period_generator1_out;
  wire debug_pulse_generator1_out;
  wire debug_pulse_generator1_tc;

  wire [1:0] debug_fifo_remove_state;
  wire debug_fifo_remove_signal;
  wire debug_timing_generator_write;

  //
  // local variables to track simulation state
  //
  reg [31:0] number_of_insert_instructions;
  reg [31:0] number_of_timing_generator_busy;

/*
  //
  // Create an instance of Device Under Test.
  //
  four_axis_timing_generator DUT(
      .clock(clock_50),
      .reset(reset),
      .clear_buffer(clear_buffer),
      .enable(enable),
      .estop(estop),
      .insert_clock(clock_50),
      .insert_signal(insert_signal),
      .insert_instruction(insert_instruction),
      .insert_pulse_period(insert_pulse_period),
      .insert_pulse_count(insert_pulse_count),
      .insert_pulse_width(insert_pulse_width),
      .insert_buffer_full(insert_buffer_full),
      .remove_buffer_empty(remove_buffer_empty),
      .timing_generator_error(timing_generator_error),
      .timing_generator_busy(timing_generator_busy),
      .timing_generator_pulse(timing_generator_pulse),
      .timing_generator_direction(timing_generator_direction),

      // debug probes
      .debug_pulse_period_generator1_out(debug_pulse_period_generator1_out),
      .debug_pulse_generator1_out(debug_pulse_generator1_out),
      .debug_pulse_generator1_tc(debug_pulse_generator1_tc),

      .debug_fifo_remove_state(debug_fifo_remove_state),
      .debug_fifo_remove_signal(debug_fifo_remove_signal),
      .debug_timing_generator_write(debug_timing_generator_write)
  );
   
  // Setup initial values
  initial begin
     clock_50 = 0;
     reset = 0;
     clear_buffer = 0;
     enable = 0;
     estop = 0;

     insert_signal = 0;
     insert_instruction = 0;
     insert_pulse_period = 0;
     insert_pulse_count = 0;
     insert_pulse_width = 0;

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

     reset = 1;

     @(negedge clock_50);

     reset = 0;

     @(negedge clock_50);

     //
     // Assert insert_buffer_full == 0
     //
     `d_assert_fifo_timing_generator(insert_buffer_full, 1'b0)

     // Enable the circuit
     enable = 1;
     #20

     //
     // Note: Update number of insert instructions above.
     //

     //
     // PULSE Clockwise 
     //
     // instruction [0:0] == 0 == clockwise
     // instruction [1:1] == 1 == PULSE out
     //
     //             instr period count width
     load_instruction(2,    4,     4,    1);

     //
     // PULSE Counter Clockwise 
     //
     // instruction [0:0] == 1 == counter-clockwise
     // instruction [1:1] == 1 == PULSE out
     //
     //             instr period count width
     load_instruction(3,    4,    16,    4);

     //
     // DWELL No Pulse for the pulse_period, pulse_count.
     //
     // instruction [0:0] == 0 == clockwise (don't care, no pulse)
     // instruction [1:1] == 0 == no PULSE out
     //
     //             instr period count width
     load_instruction(0,    4,    16,   4);

     //
     // instruction [0:0] == 0 == clockwise (don't care, no pulse)
     // instruction [1:1] == 1 == PULSE out
     //
     // Sherline:
     //
     // width == 1100 is 22us for Sherline drivers minimum pulse time.
     //
     // Maximum frequency for a Sherline with a 22us clock is about
     // 35khz (verify minimum idle time) which is a 28us period
     // represented by 357 in period.
     //
     // This gives a 6.5us low/idle time for recovery before the next pulse.
     //
     // A Sherline is 16,000 steps per inch with microstepping enabled
     // in the driver box. So this is just over 2 inches/second or
     // 120 inches/minute movement rate.
     //
     // Pulse period is based on clock_50 / 2 == 40ns
     //
     // 22us == 22000 ns / 40ns == 550.
     // But clock for width is scaled by 2X due to output
     // flip flop, so 1100 is the value for 22us pulse width
     // at 50Mhz clock.
     //
     // Measured 22us with 1100 for width.
     //
     // Period is a 4X scaling factor going through the period
     // counter flip flop fed into the pulse generator which also
     // has a flip flop on output.
     //
     // 35khz == 28.57 us == 28570 ns
     // 28570 / 80 ns == 357.125
     // 
     // 28.64us measured.
     //
     //             instr period count width
     load_instruction(2,   357,   16,  1100);

     @(posedge clock_50);

     //
     // Assert insert_buffer_full == 0
     //
     `d_assert_fifo_timing_generator(insert_buffer_full, 1'b0)

     @(posedge clock_50);

   end

   //
   // TODO: Add always block to test pulse_out based on
   // which phase its in to assert DWELL generates no pulse.
   //
   // Also good to look for glitches of pulse out when no pulses
   // should be expected.
   //

   always @(posedge clock_50) begin
   end

   //
   // This counts the number of instructions executed by the timing
   // generator by watching the falling edge of its busy status.
   //
   always @(negedge timing_generator_busy) begin
       number_of_timing_generator_busy <= number_of_timing_generator_busy + 1;

       // This needs to match the number of instructions inserted above.
       if (number_of_timing_generator_busy == number_of_insert_instructions) begin
           @(posedge clock_50);
           @(posedge clock_50);

           $stop;
       end

   end
*/

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
//     `d_assert_fifo_timing_generator(insert_buffer_full, 1'b0)

     // give it 10ns setup time
     #10 insert_signal = 1'b1;

     // must be asserted for at least 1 clock period
     #20 insert_signal = 1'b0;

    end

endtask

endmodule
