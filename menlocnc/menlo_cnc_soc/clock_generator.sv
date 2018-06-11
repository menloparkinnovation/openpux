
// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 03/31/20118
//
// Clock Generator.
//
// Generates a 50% duty cycle clock based on a down counter.
//
// The period is (clock_in / 2) / initial_count.
//
// Frequency is 1 / ((clock_in / 2) / initial_count).
//
// Note that the output maximum frequency is clock_in / 2, due to the D-flop
// on output to generate the 50% duty cycle.
//
// Parameterized size, default 32.
//
// initial_count loaded when posedge write
//
// initial_count transfered to current_count when tc == 1
//  - reloads the counter to allow it to operate periodically as a timing generator.
//
// en == 0 stops the counter.
//
// reset == 1 resets
//   initial_count == 0
//   current_count == 0
//   clock_out == 0
//
// synchronous reset.
//
module clock_generator_xx
(
    clock_in,
    reset,
    en,
    write,
    initial_count,
    clock_out
);

  parameter width = 32;

  //
  // inputs
  //

  input               clock_in;
  input               reset;
  input               en;
  input               write;
  input [width-1:0]   initial_count;
  
  //
  // outputs
  //

  output reg clock_out;

  //
  // local signals, registers
  //

  // This holds the value loaded into the counter goes to 0.
  reg [width-1:0] initial_count_reg;

  // This holds the current count
  reg [width-1:0] current_count_reg;

  //
  // Stateful logic evaluated on the clock edge.
  //

  always @(posedge clock_in) begin

    // handle reset
    if (reset == 1'b1) begin 
      clock_out <= 0;
      initial_count_reg <= {width{1'h0}}; 
      current_count_reg <= {width{1'h0}};
    end
    else begin

	//
	// Not reset
	//

	if (write == 1'b1) begin
    
    	    //
	    // on write, load the initial count value.
	    //
            // It will be loaded into the current count register
            // when the current count goes to 0.
            //
	    initial_count_reg <= initial_count;
	end
        else  if (current_count_reg == 0) begin
    
    	    //
	    // Count == 0, reload the counter from the registered initial count value.
	    //
	    current_count_reg <= initial_count_reg;
    
            //
            // Toggle the output clock on each count to
            // create a 50% duty cycle.
            //
            if (clock_out == 0) begin
                clock_out <= 1'b1;
            end
            else begin
                clock_out <= 1'b0;
            end
    	end
        else if (en == 1'b1) begin
    
    	    //
	    // Count down when enabled, and not at 0.
	    //
    	    if (current_count_reg != 0) begin
		current_count_reg <= current_count_reg - 1;
	    end
	end

    end // not reset

  end // always
 
endmodule

`define d_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $finish; \
    end

module tb_clock_generator_xx();

  parameter width = 32;

  // declare local variables to connect to the device under test (DUT)
  // inputs to DUT
  reg clock_50;
  reg reset;
  reg en;
  reg write;
  reg [width-1:0] initial_count;

  // outputs from DUT
  wire clock_out;
   
  // Create an instance of device under test
  clock_generator_xx #(.width(width)) DUT(
      .clock_in(clock_50),
      .reset(reset),
      .en(en),
      .write(write),
      .initial_count(initial_count),
      .clock_out(clock_out)
  );
   
  // Setup initial values
  initial begin
     clock_50 = 0;

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

     // Assert clock_out == 0
    `d_assert(clock_out, 1'b0)
	  
     // Let it run
     #20 en = 1'b1;

     // Let it go through 4 cycles
     @(posedge clock_out);
     @(posedge clock_out);
     @(posedge clock_out);
     @(posedge clock_out);

     //
     // This is needed to ensure the last clocks are waited for an all output transitions
     // become visible in the simulation waveform bench.
     //

     @(posedge clock_50);
     @(posedge clock_50);

     #100 $stop;
   end

endmodule

module tb_assert(input clk, input test);
    always @(posedge clk)
    begin
        if (test !== 1)
        begin
            $display("ASSERTION FAILED in %m");
            $finish;
        end
    end
endmodule


