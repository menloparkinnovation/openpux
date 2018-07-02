
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
// 06/30/2018
//
// 74xx377/SN74HCT377  Octal D type flip flop with clock enable TTL chip in 20 pin DIP.
//
// http://www.ti.com/lit/ds/symlink/cd74hc273.pdf
//
// Note: Order of inputs and output signals follows the sequence of
// pins on the physical chip 1 - 20.
//
// Pin 10 is GND, Pin 20 is VCC which are not required here.
//
module x74xx377
(
    input  CLKEN_N, // CLKEN on datasheet (pin 1)

    output reg Q1,  // 1Q on datasheet (pin 2)
    input  D1,      // 1D on datasheet (pin 3)

    input  D2,      // 2D on datasheet (pin 4)
    output reg Q2,  // 2Q on datasheet (pin 5)

    output reg Q3,  // 3Q on datasheet (pin 6)
    input  D3,      // 3D on datasheet (pin 7)

    input  D4,      // 4D on datasheet (pin 8)
    output reg Q4,  // 4Q on datasheet (pin 9)

    input CLK,      // CLK on datasheet (pin 11)

    output reg Q5,  // 5Q on datasheet (pin 12)
    input  D5,      // 5D on datasheet (pin 13)

    input  D6,      // 6D on datasheet (pin 14)
    output reg Q6,  // 6Q on datasheet (pin 15)

    output reg Q7,  // 7Q on datasheet (pin 16)
    input  D7,      // 7D on datasheet (pin 17)

    input  D8,      // 8D on datasheet (pin 18)
    output reg Q8   // 8Q on datasheet (pin 19)
);

  //
  // The x74xx377 is a positive clock edge device.
  //
  // Note: Qx outputs are declared as reg so they will
  // implement a flip-flop to hold their value.
  //

  always @ (posedge CLK) begin

      if (CLKEN_N == 1'b0) begin
          Q1 <= D1;
          Q2 <= D2;
          Q3 <= D3;
          Q4 <= D4;
          Q5 <= D5;
          Q6 <= D6;
          Q7 <= D7;
          Q8 <= D8;
      end
  end

endmodule

`define tb_x74xx377_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

//
// Test bench
//
module tb_x74xx377();

  reg clock_50;

  reg input_CLKEN_N;

  reg input_D1;
  reg input_D2;
  reg input_D3;
  reg input_D4;
  reg input_D5;
  reg input_D6;
  reg input_D7;
  reg input_D8;

  wire output_Q1;
  wire output_Q2;
  wire output_Q3;
  wire output_Q4;
  wire output_Q5;
  wire output_Q6;
  wire output_Q7;
  wire output_Q8;

  // Device Under Test instance
  x74xx377 DUT(

      .CLKEN_N(input_CLKEN_N),

      .Q1(output_Q1),
      .D1(input_D1),

      .D2(input_D2),
      .Q2(output_Q2),

      .Q3(output_Q3),
      .D3(input_D3),

      .D4(input_D4),
      .Q4(output_Q4),

      .CLK(clock_50),

      .Q5(output_Q5),
      .D5(input_D5),

      .D6(input_D6),
      .Q6(output_Q6),

      .Q7(output_Q7),
      .D7(input_D7),

      .D8(input_D8),
      .Q8(output_Q8)
  );

  // Set initial values
  initial begin
     clock_50 = 0;

     input_CLKEN_N = 1;

     input_D1 = 0;
     input_D2 = 0;
     input_D3 = 0;
     input_D4 = 0;
     input_D5 = 0;
     input_D6 = 0;
     input_D7 = 0;
     input_D8 = 0;
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
     // Enable clock in
     //
     input_CLKEN_N = 0;
     @(posedge clock_50);

     set_input_values(8'b00000000);
     @(posedge clock_50);
     @(posedge clock_50);
     assert_output_values(8'b00000000);

     set_input_values(8'b01010101);
     @(posedge clock_50);
     @(posedge clock_50);
     assert_output_values(8'b01010101);

     set_input_values(8'b10101010);
     @(posedge clock_50);
     @(posedge clock_50);
     assert_output_values(8'b10101010);

     set_input_values(8'b11111111);
     @(posedge clock_50);
     @(posedge clock_50);
     assert_output_values(8'b11111111);

     //
     // Test CLKEN_N
     //

     input_CLKEN_N = 1;
     @(posedge clock_50);

     //
     // Value should not change
     //

     set_input_values(8'b00000000);
     @(posedge clock_50);
     @(posedge clock_50);
     assert_output_values(8'b11111111);

     // End the test bench will clean signal ends
     @(posedge clock_50);
     @(posedge clock_50);

     $stop;

  end

//
// Task to assert output values
//
task assert_output_values;
  input [7:0] values_to_assert;

  begin

    //
    // Note this task can see the signals (local variables) of the module
    // its included in.
    //

    `tb_x74xx377_assert(output_Q1, values_to_assert[0:0])
    `tb_x74xx377_assert(output_Q2, values_to_assert[1:1])
    `tb_x74xx377_assert(output_Q3, values_to_assert[2:2])
    `tb_x74xx377_assert(output_Q4, values_to_assert[3:3])
    `tb_x74xx377_assert(output_Q5, values_to_assert[4:4])
    `tb_x74xx377_assert(output_Q6, values_to_assert[5:5])
    `tb_x74xx377_assert(output_Q7, values_to_assert[6:6])
    `tb_x74xx377_assert(output_Q8, values_to_assert[7:7])

  end  
endtask

task set_input_values;
  input [7:0] values_to_assert;

  begin

     input_D1 = values_to_assert[0:0];
     input_D2 = values_to_assert[1:1];
     input_D3 = values_to_assert[2:2];
     input_D4 = values_to_assert[3:3];
     input_D5 = values_to_assert[4:4];
     input_D6 = values_to_assert[5:5];
     input_D7 = values_to_assert[6:6];
     input_D8 = values_to_assert[7:7];

  end  
endtask

endmodule
