
// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 04/07/2018
//
// Timing Generator for Machine Tool Control Axis.
//
// Provides control of a machine tool axis by taking a series
// of commands that specify the following:
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
//  4) Direction
//
//   0 for clockwise, 1 for counter clockwise.
//
//  5) Instruction
//
//   0 - DWELL
//
//      Executes pulse count at the pulse period but does not generate
//      a final output pulse. This is a programmed machine delay function.
//
//   1 - PULSE
//
//      Generates the output pulse for machine movement.
//
// It expects to be front ended by a FIFO and a state machine to allow
// a series of real time commands to be fed into it without pausing.
//
// But it may also be directly hooked up to a microcontroller interface
// as registers to allow commands to be issued one by one. It provides
// sufficient signals to provide for automatic FIFO, or microcontroller
// commands.
//
// input clk_in:  Clock that provides base pulse timing and circuit synchronization.
//
// input reset == 1 resets
//   pulse_instruction = 0
//   pulse_out == 0
//   pulse_count == 0
//   pulse_period == 0
//
// input estop - active high stops the pulse generator.
//
// input direction - direction value to the machine tool controller.
//
// input instruction - Whether output pulses are generated or not.
//
// input pulse_period - Value that controls the pulse period.
//
// input pulse_count -  Number of pulses still stop.
//
// input pulse_width - Value that controls the pulse width.
//
// input write - active high specifes a new command written in pulse_period,
// pulse_count, pulse width, instruction.
//
// output busy - active high when busy executing a command
//
// output error - active high when an error condition occurs
//
// output pulse_out - pulse_out is high when the pulse is active, low when not.
//
// synchronous reset.
//
module timing_generator
(
    clock_in,
    reset,
    enable,
    estop,
    write,
    instruction,
    pulse_period,
    pulse_count,
    pulse_width,
    busy,
    error,
    pulse_out,
    direction_out
	 
`ifdef DEBUG
    // debug outputs to probe internal signals
    ,debug_pulse_period_generator1_out,
    debug_pulse_generator1_out,
    debug_pulse_generator1_tc
`endif
);

  parameter width = 32;
  parameter c_instruction_width = 4;

  //
  // inputs
  //

  input               clock_in;
  input               reset;
  input               enable;
  input               estop;

  input               write;

  input [c_instruction_width-1:0] instruction;

  input [width-1:0]   pulse_period;
  input [width-1:0]   pulse_count;
  input [width-1:0]   pulse_width;

  //
  // outputs
  //

  output               busy;
  output               error;

  output               pulse_out;
  output               direction_out;
  
`ifdef DEBUG
  // Debug outputs
  output               debug_pulse_period_generator1_out;
  output               debug_pulse_generator1_out;
  output               debug_pulse_generator1_tc;
`endif

  //
  // These hold the state received on the write signal
  //

  reg [c_instruction_width-1:0] reg_instruction;
  reg [width-1:0]   reg_pulse_period;
  reg [width-1:0]   reg_pulse_count;
  reg [width-1:0]   reg_pulse_width;

  //
  // These are locally generated registered signals.
  //

  reg               reg_write;
  reg               reg_error;
  reg               reg_enable_pulse_out;
  reg               reg_direction;
  reg               reg_freerun;
  
  //
  // These signals connect the modules together
  // and with the outputs through assign statements.
  //

  // pulse_period_generator1
  wire pulse_period_generator1_out;

  // pulse_generator1
  wire pulse_generator1_out;
  wire pulse_generator1_tc;

  // pulse_shaper1
  wire pulse_shaper_out;

  //
  // --- Begin Instruction Decoder ---
  //

  // Opcode map:
  parameter[3:0]
      OPCODE_NOP   = 4'b0000,
      OPCODE_DWELL = 4'b0001,
      OPCODE_CW    = 4'b0010,
      OPCODE_CCW   = 4'b0011,
      OPCODE_PWM   = 4'b0100;

  //
  // Opcode map:
  //
  // 0000 - OPCODE_NOP
  // 0001 - OPCODE_DWELL
  // 0010 - OPCODE_CW
  // 0011 - OPCODE_CCW
  // 0100 - OPCODE_PWM
  // 0101 - RESERVED, generates error signal
  // 0110 -   ""
  // 0111 -   ""
  // 1000 -   ""
  // 1001 -   ""
  // 1010 -   ""
  // 1011 -   ""
  // 1100 -   ""
  // 1101 -   ""
  // 1110 -   ""
  // 1111 -   ""
  //

  //
  // Low level decodes of the above opcode map:
  //
  // TODO: fix the assigns for this
  //
  // [0:0] == direction, 0 == CW, 1 == CCW
  // [1:1] == enable pulse output, 0 == no pulse out, 1 == generate pulse out
  // [3:2] == 00 - Above definition
  // [3:2] == 01 - opcode1
  // [3:2] == 02 - opcode2
  // [3:2] == 03 - opcode3
  //


  //
  // --- End Instruction Decoder ---
  //
  
  //
  // Continuous assigns.
  //

  assign busy = !pulse_generator1_tc;
  assign error = reg_error;
  
  // pulse_out depends on reg_enable_pulse_out signal
  assign pulse_out = reg_enable_pulse_out & pulse_shaper_out;

  // direction output
  assign direction_out = reg_direction;

`ifdef DEBUG
  // debug continous assignments to monitor internal signals
  assign debug_pulse_period_generator1_out = pulse_period_generator1_out;
  assign debug_pulse_generator1_out = pulse_generator1_out;
  assign debug_pulse_generator1_tc = pulse_generator1_tc;
`endif
  
  //
  // Main instruction decoder
  //

  always @ (posedge clock_in) begin

    // handle reset
    if (reset == 1'b1) begin 

      reg_instruction <= 0;
      reg_pulse_period <= 0;
      reg_pulse_count <= 0;
      reg_pulse_width <= 0;

      reg_write <= 0;
      reg_error <= 0;
      reg_enable_pulse_out <= 0;
      reg_direction <= 0;
      reg_freerun <= 0;

    end
    if (write == 1'b1) begin

      //
      // The write signal declares a new instruction to
      // execute on the timing generator.
      //

      //
      // First capture parameter values
      //
      reg_instruction <= instruction;
      reg_pulse_period <= pulse_period;
      reg_pulse_count <= pulse_count;
      reg_pulse_width <= pulse_width;

      //
      // Clear any previous instruction error.
      //
      // This can be overridden if an error is decoded below.
      //
      reg_error <= 1'b0;

      //
      // Default not free running.
      //
      reg_freerun <= 0;

      //
      // Now decode the instruction setting the registered
      // control signals as required.
      //
      case (instruction)

        //
        // This is where OPCODE validation and unique signal generation
        // occurs. This sets registered output signals that controls
        // the timing generators operation.
        //

        OPCODE_NOP: begin

          //
          // OPCODE_NOP does not operate the timing engine, and completes
          // right away so that it does not block completion of the current
          // instruction block. The other axis in the current instruction
          // block will determine completion.
          //

          //
          // NOP is expressed as blocking pulse out and programming
          // the timing engine with a zero count at zero rate.
          //
          reg_enable_pulse_out <= 0;
          reg_direction <= 0;

          //
          // Note: These override the above default assignments, as the
          // Verilog always process non-blocking assignments sets the
          // last assignment.
          //

          reg_pulse_period <= 0;
          reg_pulse_count <= 0;
          reg_pulse_width <= 0;

          //
          // Note reg_write signal is not generated and leaves the
          // timing generator in its idle state.
          //

        end

        OPCODE_DWELL: begin
          reg_instruction <= instruction;

          //
          // This commands the timing engine to run, but its output is blocked.
          // So DWELL delays the terminal count output until the specified
          // pulse_rate, pulse_count have completed.
          //

          reg_enable_pulse_out <= 0;
          reg_direction <= 0;

          // Set write to the timing generator components
          reg_write <= 1'b1;
        end

        OPCODE_CW: begin
          reg_enable_pulse_out <= 1'b1;
          reg_direction <= 1'b0;

          // Set write to the timing generator components
          reg_write <= 1'b1;
        end

        OPCODE_CCW: begin
          reg_enable_pulse_out <= 1'b1;
          reg_direction <= 1'b1;

          // Set write to the timing generator components
          reg_write <= 1'b1;
        end

        OPCODE_PWM: begin

          //
          // Generate continuous output pulses at the specified
          // rate and frequency.
          //
          // The pulse_count value specifies how many PWM pulses
          // till tc is set, but will free run until a new command.
          //
          reg_enable_pulse_out <= 1;
          reg_direction <= 0;
          reg_freerun <= 1;

          // Set write to the timing generator components
          reg_write <= 1'b1;
        end

        default: begin

          // Unsupported opcodes set to instruction decode register to OPCODE_NOP for safety.
          reg_enable_pulse_out <= 0;
          reg_direction <= 0;
          reg_pulse_period <= 0;
          reg_pulse_count <= 0;
          reg_pulse_width <= 0;

          // Unsupported OPCODE, set error
          reg_error <= 1'b1;
        end

      endcase // instruction

    end // write == 1'b1
    else begin

      //
      // Not write
      //

      //
      // reg_write state machine
      //
      // N.B. If write is asserted for multiple clock cycles the above
      // logic will keep asserting the instruction and parameter values
      // until write is deasserted.
      //

      //
      // N.B. There is no setup delay from outputting the registered
      // parameter values and reg_write command to the timing generator
      // components since they all arrive together by the next clock
      // cycle when reg_write will be sampled along with the parameters
      // by the logic in all sub-components which are synchronous, not
      // async in regards to the their write command.
      //

      // clear write to the timing generator components
      reg_write <= 1'b0;
    end
  end

  //
  // Invoke sub-module instances with wiring to the inputs/outputs.
  //

  //
  // This generates the pulse rate clock
  //
  clock_generator_xx #(.width(32)) pulse_period_generator1(
      .clock_in(clock_in),
      .reset(reset),
      .en(enable),
      .write(reg_write),
      .initial_count(reg_pulse_period),
      .clock_out(pulse_period_generator1_out)
  );

  //
  // This is the pulse counter, which takes standard clock_inn
  // but gets its rate from the pulse_period_generator as pulse_clock_in.
  //
  pulse_generator_xx #(.width(32)) pulse_generator1(
      .clock_in(clock_in),
      .pulse_clock_in(pulse_period_generator1_out),
      .reset(reset),
      .en(enable),
      .write(reg_write),
      .initial_count(reg_pulse_count),
      .freerun(reg_freerun),
      .pulse_out(pulse_generator1_out),
      .tc(pulse_generator1_tc)
  );

  //
  // This is the pulse width generator.
  //
  // It operates from clock_in, but is triggered by pulse_generator1_out.
  //
  pulse_shaper_xx #(.width(32)) pulse_shaper1(
      .clock_in(clock_in),
      .reset(reset),
      .trigger(pulse_generator1_out),
      .write(reg_write),
      .initial_count(reg_pulse_width),
      .pulse_out(pulse_shaper_out)
  ); 

endmodule

`define d_assert2(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

module tb_timing_generator();

  parameter width = 32;
  parameter c_instruction_width = 4;

  // Opcode map:
  parameter[3:0]
      OPCODE_NOP   = 4'b0000,
      OPCODE_DWELL = 4'b0001,
      OPCODE_CW    = 4'b0010,
      OPCODE_CCW   = 4'b0011,
      OPCODE_PWM   = 4'b0100;

  // declare local variables to connect to the device under test (DUT)
  // inputs to DUT
  reg clock_50;
  reg reset;
  reg enable;
  reg estop;

  reg write;

  reg [c_instruction_width-1:0] instruction;
  reg [width-1:0] pulse_period;
  reg [width-1:0] pulse_count;
  reg [width-1:0] pulse_width;

  // outputs from DUT
  wire busy;
  wire error;
  wire pulse_out;
  wire direction_out;
  
`ifdef DEBUG
  // debug probe outputs
  wire debug_pulse_period_generator1_out;
  wire debug_pulse_generator1_out;
  wire debug_pulse_generator1_tc;
`endif
   
  // Create an instance of device under test
  timing_generator DUT(
      .clock_in(clock_50),
      .reset(reset),
      .enable(enable),
      .estop(estop),
      .write(write),
      .instruction(instruction),
      .pulse_period(pulse_period),
      .pulse_count(pulse_count),
      .pulse_width(pulse_width),
      .busy(busy),
      .error(error),
      .pulse_out(pulse_out),
      .direction_out(direction_out)
		
`ifdef DEBUG
      // debug probes
      ,.debug_pulse_period_generator1_out(debug_pulse_period_generator1_out),
      .debug_pulse_generator1_out(debug_pulse_generator1_out),
      .debug_pulse_generator1_tc(debug_pulse_generator1_tc)
`endif
  );
   
  // Setup initial values
  initial begin
     clock_50 = 0;

     // These values start disabled
     reset = 0;
     enable = 0;
     estop = 0;

     write = 0;
     instruction = 0;
    
     pulse_period = 0;
     pulse_count = 0;
     pulse_width = 0;
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

     // Toggle reset
     #20 reset = 1'b1;

     // Release reset after 20ns, wait for negative clock edge
     #20 reset = 1'b0;  
     @(negedge clock_50);

     #20 enable = 1'b1;

     // instruction, period, count, width
     load_instruction(OPCODE_NOP, 0, 0, 0);

     // Note: OPCODE_NOP will not have a not busy edge since it never goes busy.
     #100

     // instruction, period, count, width
     load_instruction(OPCODE_DWELL, 4, 4, 0);

     @(negedge busy);

     // instruction,  period, count, width
     load_instruction(OPCODE_CW, 4, 4, 1);

     // busy goes low when current requested set of pulses are done.
     @(negedge busy);

     // instruction, period, count, width
     load_instruction(OPCODE_CCW, 4, 16, 4);

     @(negedge busy);

     // instruction, period, count, width
     load_instruction(OPCODE_PWM, 4, 4, 8);

     @(negedge busy);
     
     #1000

     //
     // This is needed to ensure the last clocks are waited for an all output transitions
     // become visible in the simulation waveform bench.
     //

     @(posedge clock_50);
     @(posedge clock_50);

     #100 $stop;
   end

//
// Task to load values into the timing_generator.
//
task load_instruction;
    input [c_instruction_width-1:0] _instruction;
    input [width-1:0] _period;
    input [width-1:0] _count;
    input [width-1:0] _width;

    begin

     //
     // Note: The task can access the local variables of the module
     // its contained within. Be careful when defining its inputs
     // above to ensure unique names.
     //

     instruction = _instruction;

     pulse_period = _period;
     pulse_count = _count;
     pulse_width = _width;

     // give it 10ns setup time
     #10 write = 1'b1;

     // must be asserted for at least 1 clock period
     #20 write = 1'b0;

    end

endtask

endmodule



