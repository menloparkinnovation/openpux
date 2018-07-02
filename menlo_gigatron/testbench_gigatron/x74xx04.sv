
//
// See MENLO_COPYRIGHT.TXT for the copyright notice for this file.
//
// Use of this file is not allowed without the copyright notice present
// and re-distributed along with these files.
//

//
// See menlo_7xxx_series_library_notes.txt for more details on the
// general philosophy of this module.
//

// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

//
// 06/29/2018
//
// 74xx04/SN74HCT04 hex inverter TTL chip in 14 pin DIP.
//
// http://www.ti.com/lit/ds/symlink/sn74hct04.pdf
//
// Note: Order of inputs and output signals follows the sequence of
// pins on the physical chip 1 - 14.
//
// Pin 7 is GND, Pin 14 is VCC which are not required here.
//
module x74xx04
(
    input  A1,     // 1A on datasheet (pin 1)
    output Y1,     // 1Y on datasheet (pin 2)
    input  A2,     // 2A on datasheet (pin 3)
    output Y2,     // 2Y on datasheet (pin 4)
    input  A3,     // 3A on datasheet (pin 5)
    output Y3,     // 3Y on datasheet (pin 6)
    output Y4,     // 4Y on datasheet (pin 8)
    input  A4,     // 4A on datasheet (pin 9)
    output Y5,     // 5Y on datasheet (pin 10)
    input  A5,     // 5A on datasheet (pin 11)
    output Y6,     // 6Y on datasheet (pin 12)
    input  A6      // 6A on datasheet (pin 13)
);

  //
  // Each output signal is the inverse of its input.
  //

  assign Y1 = ~A1;
  assign Y2 = ~A2;
  assign Y3 = ~A3;
  assign Y4 = ~A4;
  assign Y5 = ~A5;
  assign Y6 = ~A6;

endmodule

`define tb_x74xx04_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

//
// Test bench
//
module tb_x74xx04();

  reg clock_50;

  reg input_1A;
  reg input_2A;
  reg input_3A;
  reg input_4A;
  reg input_5A;
  reg input_6A;

  wire output_1Y;
  wire output_2Y;
  wire output_3Y;
  wire output_4Y;
  wire output_5Y;
  wire output_6Y;

  // Device Under Test instance
  x74xx04 DUT(
      .A1(input_1A),
      .Y1(output_1Y),

      .A2(input_2A),
      .Y2(output_2Y),

      .A3(input_3A),
      .Y3(output_3Y),

      .Y4(output_4Y),
      .A4(input_4A),

      .Y5(output_5Y),
      .A5(input_5A),

      .Y6(output_6Y),
      .A6(input_6A)
  );

  // Set initial values
  initial begin
     clock_50 = 0;

     input_1A = 0;
     input_2A = 0;
     input_3A = 0;
     input_4A = 0;
     input_5A = 0;
     input_6A = 0;
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

     input_1A = 1;
     input_2A = 1;
     input_3A = 1;
     input_4A = 1;
     input_5A = 1;
     input_6A = 1;

     //
     // This gives the circuit time to calculate the result. Otherwise
     // it will make the evaluation at "zero time".
     //
     // This represents one clock period on a 50Mhz Cyclone V FPGA,
     // and would represent worse case you would want for any combinatorial
     // logic between stages.
     //
     @(posedge clock_50);
     assert_output_values(6'b000000);

     input_1A = 1;
     input_2A = 0;
     input_3A = 1;
     input_4A = 0;
     input_5A = 1;
     input_6A = 0;

     @(posedge clock_50);
     assert_output_values(6'b101010);

     input_1A = 0;
     input_2A = 1;
     input_3A = 0;
     input_4A = 1;
     input_5A = 0;
     input_6A = 1;

     @(posedge clock_50);
     assert_output_values(6'b010101);

     input_1A = 0;
     input_2A = 0;
     input_3A = 0;
     input_4A = 0;
     input_5A = 0;
     input_6A = 0;

     @(posedge clock_50);
     assert_output_values(6'b111111);

     // End the test bench will clean signal ends
     @(posedge clock_50);
     @(posedge clock_50);

     $stop;

  end

//
// Task to assert output values
//
task assert_output_values;
  input [5:0] values_to_assert;

  begin

    //
    // Note this task can see the signals (local variables) of the module
    // its included in.
    //

    `tb_x74xx04_assert(output_1Y, values_to_assert[0:0])
    `tb_x74xx04_assert(output_2Y, values_to_assert[1:1])
    `tb_x74xx04_assert(output_3Y, values_to_assert[2:2])
    `tb_x74xx04_assert(output_4Y, values_to_assert[3:3])
    `tb_x74xx04_assert(output_5Y, values_to_assert[4:4])
    `tb_x74xx04_assert(output_6Y, values_to_assert[5:5])

  end  
endtask

endmodule

