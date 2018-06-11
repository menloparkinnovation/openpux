
// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 03/31/2018
//
// Pulse Generator.
//
// Generates a programmable series of output pulses and then
// stops.
//
// Each write enables a new series of pulses to be generated when en == 1
// until it stops again.
//
// Parameterized size, deault 4.
//
// clock_in - main system clock.
//
// pulse_clock_in - determines the rate of the pulses.
// Must be related to main clock_in.
//
// reset - synchronous reset.
//
// en == 1 allows the counter to count.
// en == 0 stops the counter.
//
// write - Writes a new initial_count.
//
// initial_count - Count value.
//
// tc == 1 when current_count is 0.
//
// count stops when tc == 1
//
// reset == 1 resets
//   initial_count == 0
//   current_count == 0
//   pulse_out == 0
//   tc == 1
//
// synchronous reset.
//
module pulse_generator_xx
(
    clock_in,
    pulse_clock_in,
    reset,
    en,
    write,
    initial_count,
    pulse_out,
    tc
);

  //
  // The width parameter is the number of bits the caller wants
  // to represent their maximum count.
  //
  // In the implementation + 1 is used due to the D-Flop on output
  // to generate the 50% duty cycle pulses which halves the total count.
  //
  parameter width = 4;

  //
  // inputs
  //

  input               clock_in;
  input               pulse_clock_in;
  input               reset;
  input               en;
  input               write;
  input [width-1:0]   initial_count;
  
  //
  // outputs
  //

  output reg          pulse_out;
  output wire         tc;

  //
  // local signals, registers
  //
  
  //
  // This holds the current count
  // Note: Normally would be width-1, but we need +1 bit for X2 count.
  //
  reg [width:0] current_count_reg;
  
  //
  // Need to evaluate all signals in the clock_in always block.
  //
  // But pulse_clock_in rising edge should trigger at the initial_count
  // series of pulses, and when they are done there must be a new pulse_clock_in
  // rising edge.
  //
  // This is an edge (pulse_clock_in) within an edge (clock_in).
  //
  // If this can't be done, level of pulse_clock_in could be used if the first
  // pulse_clock_in == 1'b1 does the trigger, but another trigger will not occur
  // on clk_in until we have seen a pulse_clock_in == 1'b0 ensuring we don't
  // have multiple triggers due to the fast clock_in and the much slower pulse_clock.
  //
  
  //reg trigger;
  reg hold_trigger;

  // tc == 1 when current_count_reg == 0.
  assign tc = (current_count_reg == {width+1{1'h0}}) ? 1'b1 : 1'b0;
  
  //
  // Stateful logic evaluated at the clock edge.
  //
  
  always @ (posedge clock_in) begin

    //
    // handle reset
    //
    if (reset == 1'b1) begin
        current_count_reg <= {width+1{1'h0}};
        pulse_out <= 0;
        //trigger <= 0;
        hold_trigger <= 0;
    end
    else begin
  
        //
        // Not Reset.
        //

        //
        // On write, load the current_count value. This begins
        // the count as long as en == 1.
        //
        if (write == 1'b1) begin

            // The assignment shifts the input by 1 bit, multiplying X2.
            current_count_reg[width:1] <= initial_count[width-1:0];
            //trigger <= 0;
            hold_trigger <= 0;
        end
        else begin
    
	    //
	    // When pulse_clock_in occurs a pulse is generated.
	    //	 
	    // pulse_clock_in is slower than the main clock_in, so its value
	    // may be sampled multiple times by this always block during a
	    // pulse_clock_in cycle. The hold_trigger logic ensures we trigger
	    // once per pulse_clock_in cycle.
	    //
	    if ((en == 1'b1) && (pulse_clock_in == 1'b1)) begin

		 if (hold_trigger == 1'b0) begin

		     // First time we have seen this pulse_clock_in period.
		     //trigger <= 1'b1;

		     // Hold new triggers until pulse_clock_in low is seen.
		     hold_trigger <= 1'b1;

                     if (current_count_reg != {width+1{1'h0}}) begin
                         current_count_reg <= current_count_reg - 1;

			 //
			 // Pulse out is a D flip flop toggle of tc, making a 50% duty cycle
			 // with a period twice that of the loaded count.
			 //
			 if (pulse_out == 0) begin
			     pulse_out <= 1'b1;
			 end
			 else begin
			     pulse_out <= 1'b0;
			 end
		   end
		   else begin

		        // Count has gone to zero, restart the sequence.
		        pulse_out <= 0;
		        //trigger <= 0;
		        hold_trigger <= 0;
		   end
	
		 end // end hold_trigger == 1'b0
	    end
	    else begin

		//
		// Saw pulse_clock_in low, so cancel hold_trigger as we
		// are now expecting a new pulse_clock_in period.
		//
		hold_trigger <= 1'b0;
		//trigger <= 1'b0;
	    end

        end // end not write == 1

    end // not reset

  end // always

endmodule

`define d_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $finish; \
    end

module tb_pulse_generator_xx();

  parameter width = 4;

  // declare local variables to connect to the device under test (DUT)
  // inputs to DUT
  reg clock_50;
  reg pulse_clock;
  reg reset;
  reg en;
  reg write;
  reg [width-1:0] initial_count;

  // outputs from DUT
  wire pulse_out;
  wire tc;
   
  // Create an instance of device under test
  pulse_generator_xx #(.width(width)) DUT(
      .clock_in(clock_50),
		.pulse_clock_in(pulse_clock),
      .reset(reset),
      .en(en),
      .write(write),
      .initial_count(initial_count),
      .pulse_out(pulse_out),
		.tc(tc)
  );
   
  // Setup initial values
  initial begin
     clock_50 = 0;
	  pulse_clock = 0;

     // These values start disabled
     reset = 0;
     en = 0;
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

  //
  // Generate a 5Mhz pulse clock
  //
  always #100 pulse_clock = ~pulse_clock;

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
	  
     // Let it run
     #20 en = 1'b1;

     // Let it go through cycles
     @(posedge tc);
	  
	  #20 initial_count = 8;

     // give it 10ns setup time
     #10 write = 1'b1;

     // must be asserted for at least 1 clock period
     #20 write = 1'b0;
	  
	  @(posedge tc);

     //
     // This is needed to ensure the last clocks are waited for an all output transitions
     // become visible in the simulation waveform bench.
     //

     @(posedge clock_50);
     @(posedge clock_50);

     #100 $stop;
   end

endmodule
