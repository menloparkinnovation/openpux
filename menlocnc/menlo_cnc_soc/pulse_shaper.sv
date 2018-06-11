
// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 04/06/2018
//
// Pulse Shaper for CNC machine tool control.
//
// The pulse shaper generates a programmable pulse length
// that is generated each time the rising clock edge of
// the trigger input.
//
// The trigger input comes from a pulse generator which generates
// a series of pulses are a particular rate for machine movement.
//
// Different machine tools from small desktop machine tools to higher
// end machine tools require different pulse widths, so this circuit
// allows a programmable way to set the minimum/maximum pulse width
// that a particular machine requires.
//
// The maximum pulse rate for a particular machine is controlled
// by its minimum pulse width, and minimum dwell time between
// pulses.
//
// For a desktop machine the Sherline control box is specified for
// a 22us pulse width minimum, with the pulse duration increasing
// proportionally with pulse frequency increase. Maximum pulse rate
// to the machine is limited by the minimum period required to reset
// the logic between pulses. Consult data sheet for your stepper driver.
//
// If the Sherline is setup for microstepping, there are 16,000 pulses
// per inch of machine movement on the axis. This approximates to about
// 2 inches/per second or 120/inches per minute, a movement rate at
// the limits of the machines unloaded jog rate.
// 
// A higher end professional machine tool requires pulse rates up to
// 125Khz or high in some cases. In these cases their stepper drivers
// accept smaller pulse widths. The input clock defines the lower
// limit of the pulse width.
//
// For the standard 32 bit configuration, a 5 Mhz input clock is used,
// and this generates pulse width range from 200ns - 400 seconds
// in 200ns increments. Running a higher input clock such as 50Mhz
// would increase resolution to 20ns, with a range from 20ns - 40
// seconds.
//  
// Note: When the pulse_shaper is incorporated into a circuit
// its initial_count input can be a subset of the 32 bits. For
// example a 16 bit counter range value could be input to
// initial_count[7:22] which would give a range 26us - 1.6 seconds.
//
// For most configurations its expected that the pulse width is fixed
// for a series of commands and the full 32 bit register can
// be readily exposed to the controlling microprocessor. But for
// designs which program the pulse width from command to command
// limiting the word width to the desired range saves FIFO space,
// allowing available FPGA block ram resources to be used for deeper
// FIFO's.
//
// clk_in:  Clock that provides base pulse timing and circuit synchronization.
//
// reset == 1 resets
//   initial_count == 0
//   current_count == 0
//   pulse_out == 0
//
// trigger - Rising edge generates a pulse.
//
//           Must be in the same clock domain as clk_in, and related
//           in timing edge as extra synchronization is not applied.
//
// write - when high writes the initial_count into the internal initial
//         count register.
//
// initial_count - Value that controls the pulse length.
//
// pulse_out - pulse_out is high when the pulse is active, low when not.
//
// synchronous reset.
//
module pulse_shaper_xx
(
    clock_in,
    reset,
    trigger,
    write,
    initial_count,
    pulse_out
);

  parameter width = 32;

  //
  // inputs
  //

  input               clock_in;
  input               reset;
  input               trigger;
  input               write;
  input [width-1:0]   initial_count;
  
  //
  // outputs
  //

  output pulse_out;

  //
  // local signals, registers
  //
  
  //
  // This reloads the count for each pulse trigger.
  //
  reg [width-1:0] initial_count_reg;

  //
  // This holds the current count
  //
  reg [width-1:0] current_count_reg;

  reg reg_hold_trigger;

  // pulse_out == 1 when current_count_reg != 0.
  assign pulse_out = (current_count_reg != {width{1'h0}}) ? 1'b1 : 1'b0;
  
  //
  // Stateful logic evaluated at the clock edge.
  //

  //
  // Trigger is in the same clock domain, and related to clk_in.
  //

  always_ff @ (posedge clock_in) begin
  
    //
    // handle reset
    //

    if (reset == 1'b1) begin 
      current_count_reg <= {width{1'h0}};
      initial_count_reg <= {width{1'h0}};
      reg_hold_trigger <= 1'b0;
    end
    else  begin

      //
      // Not reset
      //

      //
      // On write, the initial_count_reg is loaded with the
      // value used to define the pulse length.
      //
      if (write == 1'b1) begin
        initial_count_reg <= initial_count;
        reg_hold_trigger <= 1'b0;
      end
      else begin

	  if (trigger == 1'b1) begin

	    //
	    // See if its a new trigger
	    //
	    if (reg_hold_trigger == 1'b0) begin
	      // On new trigger period load the count register and begin the pulse
	      current_count_reg <= initial_count_reg;
	      reg_hold_trigger <= 1'b1;
	    end

	  end // end trigger == 1'b1
          else begin
	      // trigger has been seen low, clear hold_trigger
	      reg_hold_trigger <= 1'b0;
          end
    
	  //
	  // Count down when not at terminal count.
	  //
	  if (current_count_reg != {width{1'h0}}) begin
	    current_count_reg <= current_count_reg - 1;
	  end
	 
      end // not write

    end // end else !reset

  end // always

endmodule

`define d_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $finish; \
    end

module tb_pulse_shaper_xx();

  // pulse_shaper.v is tested against a width of 32.
  parameter width = 32;

  // declare local variables to connect to the device under test (DUT)
  // inputs to DUT
  reg clock_50;
  reg reset;
  reg trigger;
  reg write;
  reg [width-1:0] initial_count;

  // outputs from DUT
  wire pulse_out;
   
  // Create an instance of device under test
  pulse_shaper_xx #(.width(width)) DUT(
      .clock_in(clock_50),
      .reset(reset),
      .trigger(trigger),
      .write(write),
      .initial_count(initial_count),
      .pulse_out(pulse_out)
  );
   
  // Setup initial values
  initial begin
     clock_50 = 0;

     // These values start disabled
     reset = 0;
     trigger = 0;
     write = 0;

     // default initial count
     initial_count = 0;
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
	  
     // Write initial_count into counter
     // Note: write must be held for at least 1 clk or will be missed since its synchronous
     #20 initial_count = 4;

     // give it 10ns setup time
     #10 write = 1'b1;

     // must be asserted for at least 1 clock period
     #20 write = 1'b0;

     #20 // this is to see if tc goes to zero. It fails the next assert without this.

     // Assert pulse_out == 0
    `d_assert(pulse_out, 1'b0)
	  
     // trigger it
     #20 trigger = 1'b1;

	  // We want to hold trigger in the simulation to ensure correct behavior
	  // of the output pulses.
     //#20 trigger = 1'b0;

     // Let it go through cycles
     @(negedge pulse_out);

	  //
     // trigger it. In this case we don't hold trigger to validate the
	  // one shot case works as well.
	  //
     #20 trigger = 1'b0;
     #20 trigger = 1'b1;
	  #20 trigger = 1'b0;

     // Let it go through cycles
     @(negedge pulse_out);

     #20 initial_count = 8;

     // give it 10ns setup time
     #10 write = 1'b1;

     // must be asserted for at least 1 clock period
     #20 write = 1'b0;
	  
     // trigger it
     #20 trigger = 1'b1;
     #20 trigger = 1'b0;

     @(negedge pulse_out);

     //
     // This is needed to ensure the last clocks are waited for an all output transitions
     // become visible in the simulation waveform bench.
     //

     @(posedge clock_50);
     @(posedge clock_50);

     #100 $stop;
   end

endmodule
