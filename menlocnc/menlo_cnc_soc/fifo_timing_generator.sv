
// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 04/14/2018
//
// FIFO front end for timing_generator.
//
// Buffers commands and automatically sends them to the timing generator
// to conduct machine movements without pauses.
//
// It outputs the signals and status of the timing generator, which
// operates independently on the commands placed into the FIFO
// until the FIFO is empty.
//
// Uses command_fifo.sv which contains the FPGA vendor optimized
// FIFO implementations for buffering timing generator commands.
//
// See command_fifo.sv, timing_generator.sv sub-modules for full
// documentation on the inputs and outputs.
//

`define FIFO_INSERT_DELAY_CLOCKS 1 // FIFO insert delay in clocks
`define FIFO_REMOVE_DELAY_CLOCKS 1 // FIFO insert delay in clocks

module fifo_timing_generator
(
    //
    // Shared input signals
    //
    clock,
    reset,
    clear_buffer,
    enable,
    estop,

    //
    // Input (write command to FIFO) side
    //
    insert_clock,
    insert_signal,
    insert_instruction,
    insert_pulse_period,
    insert_pulse_count,
    insert_pulse_width,

    //
    // FIFO state outputs
    //
    insert_buffer_full,
    remove_buffer_empty,

    //
    // timing generator outputs
    //
    timing_generator_error,
    timing_generator_busy,
    timing_generator_pulse,

    timing_generator_direction

`ifdef DEBUG
    // Debug outputs
    ,debug_pulse_period_generator1_out,
    debug_pulse_generator1_out,
    debug_pulse_generator1_tc,

    debug_fifo_remove_state,
    debug_fifo_remove_signal,
    debug_timing_generator_write
`endif
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

  input                        clock;
  input                        reset;
  input                        clear_buffer;
  input                        enable;
  input                        estop;

  //
  // Input Write FIFO side. These signals represent input
  // commands from the microprocessor.
  //

  input                        insert_clock;
  input                        insert_signal;
  input [c_instruction_width-1:0] insert_instruction;
  input [c_signal_width-1:0]   insert_pulse_period;
  input [c_signal_width-1:0]   insert_pulse_count;
  input [c_signal_width-1:0]   insert_pulse_width;

  //
  // FIFO outputs
  //
  output                       insert_buffer_full;
  output                       remove_buffer_empty;

  //
  // Timing generator outputs
  //
  output                       timing_generator_error;
  output                       timing_generator_busy;
  output                       timing_generator_pulse;
  output                       timing_generator_direction;

`ifdef DEBUG
  //
  // debug
  //
  output                       debug_pulse_period_generator1_out;
  output                       debug_pulse_generator1_out;
  output                       debug_pulse_generator1_tc;

  output                       debug_fifo_remove_signal;
  output [2:0]                 debug_fifo_remove_state;
  output                       debug_timing_generator_write;
`endif

  //
  // local signals, registers
  //

  //
  // FIFO insert side
  //
  reg [1:0]                      fifo_insert_state;
  reg [1:0]                      fifo_insert_clock_delay;

  //
  // FIFO remove side
  //
  wire [c_instruction_width-1:0] remove_instruction;
  wire [c_signal_width-1:0]      remove_pulse_period;
  wire [c_signal_width-1:0]      remove_pulse_count;
  wire [c_signal_width-1:0]      remove_pulse_width;

  reg                            fifo_remove_signal;
  reg [2:0]                      fifo_remove_state;

  //
  // timing generator signals
  //
  wire                           timing_generator_instruction_in;
  wire                           timing_generator_direction_in;
  reg                            timing_generator_write;

  // FIFO insert side states
  parameter[1:0]
      FIFO_INSERT_IDLE    = 2'b00,
      FIFO_INSERT_DELAY   = 2'b01,
      FIFO_INSERT_ACTION  = 2'b10,
      FIFO_INSERT_INVALID = 2'b11;

  //
  // The output 'q' of the DCFIFO is not valid until
  // 1 rdclk delay after rdreq.
  //

  // FIFO remove side states
  parameter[2:0]
      FIFO_REMOVE_IDLE      = 3'b000,
      FIFO_REMOVE_Q_DELAY_0 = 3'b001, // Waiting for FIFO data valid
      FIFO_REMOVE_Q_DELAY_1 = 3'b010, // Waiting for FIFO data valid
      FIFO_REMOVE_TG_WRITE  = 3'b011, // Timing Generator Write
      FIFO_REMOVE_TG_WRITE_COMPLETE = 3'b100;


  //
  // Invoke sub-module instances
  //

  //
  // Command FIFO for buffering stream of instructions
  //
  command_fifo command_fifo1(

      .clear_buffer(clear_buffer),

      .insert_clock(clock),
      .insert_signal(insert_signal),
      .insert_instruction(insert_instruction),
      .insert_pulse_period(insert_pulse_period),
      .insert_pulse_count(insert_pulse_count),
      .insert_pulse_width(insert_pulse_width),
      .insert_buffer_full(insert_buffer_full),

      .remove_clock(clock),
      .remove_signal(fifo_remove_signal),
      .remove_instruction(remove_instruction),
      .remove_pulse_period(remove_pulse_period),
      .remove_pulse_count(remove_pulse_count),
      .remove_pulse_width(remove_pulse_width),
      .remove_buffer_empty(remove_buffer_empty)
  );

  //
  // The timing_generator instance driven by the output
  // of the command buffer FIFO.
  //
  timing_generator timing_generator1(
      .clock_in(clock),
      .reset(reset),
      .enable(enable),
      .estop(estop),
      .write(timing_generator_write),
      .instruction(timing_generator_instruction_in),
      .direction(timing_generator_direction_in),
      .pulse_period(remove_pulse_period),
      .pulse_count(remove_pulse_count),
      .pulse_width(remove_pulse_width),
      .busy(timing_generator_busy),
      .error(timing_generator_error),
      .pulse_out(timing_generator_pulse),
      .direction_out(timing_generator_direction)

`ifdef DEBUG
      // debug probes
      ,.debug_pulse_period_generator1_out(debug_pulse_period_generator1_out),
      .debug_pulse_generator1_out(debug_pulse_generator1_out),
      .debug_pulse_generator1_tc(debug_pulse_generator1_tc)
`endif
  );

  //
  // Continous assigns for decoding the instruction from the FIFO
  // to the specific timing generator inputs.
  //
  assign timing_generator_direction_in = remove_instruction[0:0];
  assign timing_generator_instruction_in = remove_instruction[1:1];

`ifdef DEBUG
  //
  // Debug continous assignments act as a signal tap.
  //
  assign debug_fifo_remove_state = fifo_remove_state;
  assign debug_fifo_remove_signal = fifo_remove_signal;
  assign debug_timing_generator_write = timing_generator_write;
`endif

  //
  // This always block removes items from the command FIFO and feeds the
  // timing generator with new commands until empty.
  //
  always_ff @(posedge clock) begin

      if (reset) begin
          fifo_remove_signal <= 0;
          timing_generator_write <= 0;
          fifo_remove_state <= FIFO_REMOVE_IDLE;
      end
      else begin

          //
          // If the timing generator is not executing a command
          // and the FIFO has a new command remove it and apply
          // it to the timing generator using the state machine.
          //
	  case (fifo_remove_state)

	      FIFO_REMOVE_IDLE: begin

		  //
		  // IDLE state
		  //

		  if ((timing_generator_busy == 1'b0) &&
		      (remove_buffer_empty == 1'b0)) begin

		      //
		      // timing generator idle, and there are new
		      // requests in the FIFO so issue the next one.
		      //

		      // Set the FIFO remove signal
		      fifo_remove_signal <= 1'b1;

		      // Update state
		      fifo_remove_state <= FIFO_REMOVE_Q_DELAY_0;

		  end

	      end

	      FIFO_REMOVE_Q_DELAY_0: begin

                  //
		  // Deassert FIFO remove signal
                  // FIFO Must have its remove signal deasserted
                  // at the next clock it will retrieve multiple
                  // entries, one per clock cycle.
                  //
		  fifo_remove_signal <= 1'b0;

                  // Wait one more clock
	          fifo_remove_state <= FIFO_REMOVE_Q_DELAY_1;
              end

	      FIFO_REMOVE_Q_DELAY_1: begin

		  //
		  // Waiting for FIFO read data (Q) valid.
		  //

		  // Start timing generator write
		  timing_generator_write <= 1'b1;

		  fifo_remove_state <= FIFO_REMOVE_TG_WRITE;

	      end

	      FIFO_REMOVE_TG_WRITE: begin

		  //
		  // Timing generater write, deassert.
		  //
		  timing_generator_write <= 1'b0;

		  //
		  // The current command has been placed into the
		  // timing generator.
		  //
		  // There is a one clock cycle delay before allowing
		  // the next command to be issued if the timing generator
		  // completes the command right away.
		  //

		  fifo_remove_state <= FIFO_REMOVE_TG_WRITE_COMPLETE;

	      end

	      FIFO_REMOVE_TG_WRITE_COMPLETE: begin

		  //
		  // Timing generator write 1 clock idle has completed.
		  //

		  // Back to idle
		  fifo_remove_state <= FIFO_REMOVE_IDLE;
	      end

	      default: begin
	      end

	  endcase

      end // end else not reset

  end // end always posedge clock

endmodule

`define d_assert_fifo_timing_generator(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

module tb_fifo_timing_generator();

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

`ifdef DEBUG
  // debug probes for timing_generator.
  wire debug_pulse_period_generator1_out;
  wire debug_pulse_generator1_out;
  wire debug_pulse_generator1_tc;

  wire [1:0] debug_fifo_remove_state;
  wire debug_fifo_remove_signal;
  wire debug_timing_generator_write;
`endif

  //
  // local variables to track simulation state
  //
  reg [31:0] number_of_insert_instructions;
  reg [31:0] number_of_timing_generator_busy;

  //
  // Create an instance of Device Under Test.
  //
  fifo_timing_generator DUT(
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
      .timing_generator_direction(timing_generator_direction)

`ifdef DEBUG
      // debug probes
      , .debug_pulse_period_generator1_out(debug_pulse_period_generator1_out),
      .debug_pulse_generator1_out(debug_pulse_generator1_out),
      .debug_pulse_generator1_tc(debug_pulse_generator1_tc),

      .debug_fifo_remove_state(debug_fifo_remove_state),
      .debug_fifo_remove_signal(debug_fifo_remove_signal),
      .debug_timing_generator_write(debug_timing_generator_write)
`endif
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
     `d_assert_fifo_timing_generator(insert_buffer_full, 1'b0)

     // give it 10ns setup time
     #10 insert_signal = 1'b1;

     // must be asserted for at least 1 clock period
     #20 insert_signal = 1'b0;

    end

endtask

endmodule
