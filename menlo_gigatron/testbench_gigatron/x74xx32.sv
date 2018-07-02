
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
// 74xx32/SN74HCT32 quad 2 input OR gates in 14 pin DIP.
//
// http://www.ti.com/lit/ds/symlink/sn74hct32.pdf
//
// Note: Order of inputs and output signals follows the sequence of
// pins on the physical chip 1 - 14.
//
// Pin 7 is GND, Pin 14 is VCC which are not required here.
//
module x74xx32
(
    input  A1,    // 1A on datasheet (pin 1)
    input  B1,    // 1B on datasheet (pin 2)
    output Y1,    // 1Y on datasheet (pin 3)

    input  A2,    // 2A on datasheet (pin 4)
    input  B2,    // 2B on datasheet (pin 5)
    output Y2,    // 2Y on datasheet (pin 6)

    output Y3,    // 3Y on datasheet (pin 8)
    input  A3,    // 3A on datasheet (pin 9)
    input  B3,    // 3B on datasheet (pin 10)

    output Y4,    // 4Y on datasheet (pin 11)
    input  A4,    // 4A on datasheet (pin 12)
    input  B4     // 4B on datasheet (pin 13)
);

  assign Y1 = A1 | B1;
  assign Y2 = A2 | B2;
  assign Y3 = A3 | B3;
  assign Y4 = A4 | B4;

endmodule

`define tb_x74xx32_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

//
// Test bench
//
module tb_x74xx32();

  reg clock_50;

  reg input_A1;
  reg input_B1;

  reg input_A2;
  reg input_B2;

  reg input_A3;
  reg input_B3;

  reg input_A4;
  reg input_B4;

  wire output_Y1;
  wire output_Y2;
  wire output_Y3;
  wire output_Y4;

  // Device Under Test instance
  x74xx32 DUT(
      .A1(input_A1),
      .B1(input_B1),
      .Y1(output_Y1),

      .A2(input_A2),
      .B2(input_B2),
      .Y2(output_Y2),

      .A3(input_A3),
      .B3(input_B3),
      .Y3(output_Y3),

      .A4(input_A4),
      .B4(input_B4),
      .Y4(output_Y4)
  );

  // Set initial values
  initial begin
     clock_50 = 0;

     input_A1 = 0;
     input_B1 = 0;

     input_A2 = 0;
     input_B2 = 0;

     input_A3 = 0;
     input_B3 = 0;

     input_A4 = 0;
     input_B4 = 0;
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

     input_A1 = 1;
     input_B1 = 0;

     input_A2 = 1;
     input_B2 = 0;

     input_A3 = 1;
     input_B3 = 0;

     input_A4 = 1;
     input_B4 = 0;

     //
     // This gives the circuit time to calculate the result. Otherwise
     // it will make the evaluation at "zero time".
     //
     // This represents one clock period on a 50Mhz Cyclone V FPGA,
     // and would represent worse case you would want for any combinatorial
     // logic between stages.
     //
     @(posedge clock_50);
     assert_output_values(4'b1111);

     input_A1 = 0;
     input_B1 = 1;

     input_A2 = 0;
     input_B2 = 1;

     input_A3 = 0;
     input_B3 = 1;

     input_A4 = 0;
     input_B4 = 1;

     @(posedge clock_50);
     assert_output_values(4'b1111);

     input_A1 = 1;
     input_B1 = 1;

     input_A2 = 1;
     input_B2 = 1;

     input_A3 = 1;
     input_B3 = 1;

     input_A4 = 1;
     input_B4 = 1;


     @(posedge clock_50);
     assert_output_values(4'b1111);

     input_A1 = 0;
     input_B1 = 0;

     input_A2 = 0;
     input_B2 = 0;

     input_A3 = 0;
     input_B3 = 0;

     input_A4 = 0;
     input_B4 = 0;

     @(posedge clock_50);
     assert_output_values(4'b0000);

     // End the test bench will clean signal ends
     @(posedge clock_50);
     @(posedge clock_50);

     $stop;

  end

//
// Task to assert output values
//
task assert_output_values;
  input [3:0] values_to_assert;

  begin

    //
    // Note this task can see the signals (local variables) of the module
    // its included in.
    //

    `tb_x74xx32_assert(output_Y1, values_to_assert[0:0])
    `tb_x74xx32_assert(output_Y2, values_to_assert[1:1])
    `tb_x74xx32_assert(output_Y3, values_to_assert[2:2])
    `tb_x74xx32_assert(output_Y4, values_to_assert[3:3])

  end  
endtask

endmodule

