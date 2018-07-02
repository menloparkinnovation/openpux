
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
// 74xx283/SN74HCT283 4 bit binary adder with carry in 16 pin DIP.
//
// http://www.ti.com/lit/ds/symlink/cd74hct283.pdf
//
// Fast ripple carry adder.
//
// Note: Order of inputs and output signals follows the sequence of
// pins on the physical chip 1 - 16.
//
// Pin 8 is GND, Pin 16 is VCC which are not required here.
//
// 4 bit adder with fast carry.
// 
// No resets or presets or clock. Simple inputs => outputs.
//
// CIN   - Carry in input to adder
// A1-A3 - A inputs to adder
// B1-B3 - B inputs to adder
// 
// S0-S3 - Outputs from adder
// COUT  - Carry out from adder
// 
// 

module x74xx283
(
    output S1,    // S1 on datasheet (pin 1)
    input  B1,    // B1 on datasheet (pin 2)
    input  A1,    // A1 on datasheet (pin 3)

    output S0,    // S0 on datasheet (pin 4)
    input  A0,    // A0 on datasheet (pin 5)
    input  B0,    // B0 on datasheet (pin 6)

    input  CIN,   // CIN on datasheet  (pin 7)
    output COUT,  // COUT on datasheet (pin 8)

    output S3,    // S1 on datasheet (pin 10)
    input  B3,    // B1 on datasheet (pin 11)
    input  A3,    // A1 on datasheet (pin 12)

    output S2,    // S1 on datasheet (pin 13)
    input  A2,    // A1 on datasheet (pin 14)
    input  B2     // B1 on datasheet (pin 151)
);

  //
  // Full adder circuit described in:
  //
  // Fundamentals of Digital Logic with Verilog Design
  // Stephen Brown, Zvonko Vranesic
  // Third edition
  // page 154 
  // McGraw Hill 2014
  //
  // www.eecg.toronto.edu/~brown/Verilog_3e
  //
  // Note: The above web site was not used, only the general
  // idea of a full adder consisting of XOR logic for its
  // sum, and AND logic for its carry computation as a well
  // known practice.
  //
  // This implements a ripple adder which should work fine on a
  // 50Mhz FPGA in a 6.25 Mhz circuit such as the Gigatron.
  //
  // A text book fast adder could be implemented, or use
  // Verilog Synthesis add statements to use FPGA based adders.
  //
  // Note HDL compilers tend to recognize "text book" Verilog
  // patterns such as this and optimize to an internal fast
  // adder, so this ripple carry adder may actually get reduced
  // to a fast adder block that will operate at the FPGA's
  // design speed.
  //

  wire carry1;
  wire carry2;
  wire carry3;
  
  fulladder stage0(CIN,    A0, B0, S0, carry1);
  fulladder stage1(carry1, A1, B1, S1, carry2);
  fulladder stage2(carry2, A2, B2, S2, carry3);
  fulladder stage3(carry3, A3, B3, S3, COUT);

endmodule

//
// Full Adder using industry text book description with
// XOR sum function and AND carry computation.
//
module fulladder(
    input CIN,
    input A,
    input B,
    output S,
    output COUT
    );

  //
  // Sum function is the XOR A, B and carry in.
  //
  // Note: This is not a 3 input XOR, but the result the XOR
  //       of the XOR *output* of (A ^ B) with CIN.
  //
  // See page 129 for gate level circuit diagram.
  //
  assign S = A ^ B ^ CIN;

  // Compute carry output as sum of products.
  assign COUT = ((A & B) | (A & CIN) | (B & CIN));

endmodule

`define tb_x74xx283_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

//
// Test bench
//
module tb_x74xx283();

  reg clock_50;

  reg input_CIN;
  reg input_A0;
  reg input_A1;
  reg input_A2;
  reg input_A3;

  reg input_B0;
  reg input_B1;
  reg input_B2;
  reg input_B3;

  wire output_S0;
  wire output_S1;
  wire output_S2;
  wire output_S3;
  wire output_COUT;

  // Device Under Test instance
  x74xx283 DUT(
      .S1(output_S1),
      .B1(input_B1),
      .A1(input_A1),

      .S0(output_S0),
      .A0(input_A0),
      .B0(input_B0),

      .CIN(input_CIN),
      .COUT(output_COUT),

      .S3(output_S3),
      .B3(input_B3),
      .A3(input_A3),

      .S2(output_S2),
      .A2(input_A2),
      .B2(input_B2)
  );

  // Set initial values
  initial begin
     clock_50 = 0;

     input_CIN = 0;

     input_A0 = 0;
     input_B0 = 0;

     input_A1 = 0;
     input_B1 = 0;

     input_A2 = 0;
     input_B2 = 0;

     input_A3 = 0;
     input_B3 = 0;
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
     // Note: A wait for posedge clock after setting the values gives the
     // circuit time to calculate the result. Otherwise it will make the
     // evaluation at "zero time".
     //
     // This represents one clock period on a 50Mhz Cyclone V FPGA,
     // and would represent worse case you would want for any combinatorial
     // logic between stages.
     //

     //
     // First test Carry IN == 0
     //

     add_values(1'b0, 4'b1111, 4'b0000);
     @(posedge clock_50);
     assert_output_values(1'b0, 4'b1111);

     add_values(1'b0, 4'b0000, 4'b1111);
     @(posedge clock_50);
     assert_output_values(1'b0, 4'b1111);

     //
     // This results in: cout= 1'b1, Sx = 4'b1110
     //
     //  1111 == 15
     // 11110 == 30 (1111 + 1111)
     //
     add_values(1'b0, 4'b1111, 4'b1111);
     @(posedge clock_50);
     assert_output_values(1'b1, 4'b1110);

     add_values(1'b0, 4'b0000, 4'b0000);
     @(posedge clock_50);
     assert_output_values(1'b0, 4'b0000);

     //
     // First test Carry IN == 1
     //

     add_values(1'b1, 4'b1111, 4'b0000);
     @(posedge clock_50);
     assert_output_values(1'b1, 4'b0000);

     add_values(1'b1, 4'b0000, 4'b1111);
     @(posedge clock_50);
     assert_output_values(1'b1, 4'b0000);

     add_values(1'b1, 4'b1111, 4'b1111);
     @(posedge clock_50);
     assert_output_values(1'b1, 4'b1111);

     add_values(1'b1, 4'b0000, 4'b0000);
     @(posedge clock_50);
     assert_output_values(1'b0, 4'b0001);

     // End the test bench will clean signal ends
     @(posedge clock_50);
     @(posedge clock_50);

     $stop;

  end

//
// Task to assert output values
//
task assert_output_values;
  input _carry_out_value_to_assert;
  input [3:0] _sum_out_value_to_assert;

  begin

    //
    // Note this task can see the signals (local variables) of the module
    // its included in.
    //

    `tb_x74xx283_assert(output_COUT, _carry_out_value_to_assert)

    `tb_x74xx283_assert(output_S0, _sum_out_value_to_assert[0:0])
    `tb_x74xx283_assert(output_S1, _sum_out_value_to_assert[1:1])
    `tb_x74xx283_assert(output_S2, _sum_out_value_to_assert[2:2])
    `tb_x74xx283_assert(output_S3, _sum_out_value_to_assert[3:3])

  end  
endtask

task add_values;
  input       _input_CIN;
  input [3:0] _input_A;
  input [3:0] _input_B;

  begin

     input_CIN = _input_CIN;

     input_A0 = _input_A[0:0];
     input_B0 = _input_B[0:0];

     input_A1 = _input_A[1:1];
     input_B1 = _input_B[1:1];

     input_A2 = _input_A[2:2];
     input_B2 = _input_B[2:2];

     input_A3 = _input_A[3:3];
     input_B3 = _input_B[3:3];

  end  
endtask

endmodule

