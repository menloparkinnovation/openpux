
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
// 74xx273/SN74HCT273  Octal D type flip flop with reset TTL chip in 20 pin DIP.
//
// http://www.ti.com/lit/ds/symlink/cd74hc273.pdf
//
// Note: Order of inputs and output signals follows the sequence of
// pins on the physical chip 1 - 20.
//
// Pin 10 is GND, Pin 20 is VCC which are not required here.
//
module x74xx273
(
    input  MR,     // MR on datasheet (pin 1)

    output reg Q0, // Q0 on datasheet (pin 2)
    input  D0,     // D0 on datasheet (pin 3)

    input  D1,     // D1 on datasheet (pin 4)
    output reg Q1, // Q1 on datasheet (pin 5)
    
    output reg Q2, // Q2 on datasheet (pin 6)
    input  D2,     // D2 on datasheet (pin 7)

    input  D3,     // D3 on datasheet (pin 8)
    output reg Q3, // Q3 on datasheet (pin 9)

    input  CP,     // CP on datasheet (pin 11)

    output reg Q4, // Q4 on datasheet (pin 12)
    input  D4,     // D4 on datasheet (pin 13)

    input  D5,     // D5 on datasheet (pin 14)
    output reg Q5, // Q5 on datasheet (pin 15)

    output reg Q6, // Q6 on datasheet (pin 16)
    input  D6,     // D6 on datasheet (pin 17)

    input  D7,     // D7 on datasheet (pin 18)
    output reg Q7  // Q7 on datasheet (pin 19)
);

  //
  // The x74xx273 is a positive clock edge device.
  //
  // Note: Qx outputs are declared as reg so they will
  // implement a flip-flop to hold their value.
  //

  always @ (posedge CP) begin

      if (MR == 1'b0) begin

          //
          // Master Reset
          //

          Q0 <= 0;
          Q1 <= 0;
          Q2 <= 0;
          Q3 <= 0;
          Q4 <= 0;
          Q5 <= 0;
          Q6 <= 0;
          Q7 <= 0;
      end
      else begin

          //
          // Not Reset, latch the inputs to the outputs
          // till the next clock cycle.
          //

          Q0 <= D0;
          Q1 <= D1;
          Q2 <= D2;
          Q3 <= D3;
          Q4 <= D4;
          Q5 <= D5;
          Q6 <= D6;
          Q7 <= D7;

      end
  end

endmodule

`define tb_x74xx273_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

//
// Test bench
//
module tb_x74xx273();

  reg clock_50;

  reg input_MR;

  reg input_D0;
  reg input_D1;
  reg input_D2;
  reg input_D3;
  reg input_D4;
  reg input_D5;
  reg input_D6;
  reg input_D7;

  wire output_Q0;
  wire output_Q1;
  wire output_Q2;
  wire output_Q3;
  wire output_Q4;
  wire output_Q5;
  wire output_Q6;
  wire output_Q7;

  // Device Under Test instance
  x74xx273 DUT(
      .MR(input_MR),

      .Q0(output_Q0),
      .D0(input_D0),

      .D1(input_D1),
      .Q1(output_Q1),

      .Q2(output_Q2),
      .D2(input_D2),

      .D3(input_D3),
      .Q3(output_Q3),

      .CP(clock_50),

      .Q4(output_Q4),
      .D4(input_D4),

      .D5(input_D5),
      .Q5(output_Q5),

      .Q6(output_Q6),
      .D6(input_D6),

      .D7(input_D7),
      .Q7(output_Q7)
  );

  // Set initial values
  initial begin
     clock_50 = 0;

     input_MR = 1;

     input_D0 = 0;
     input_D1 = 0;
     input_D2 = 0;
     input_D3 = 0;
     input_D4 = 0;
     input_D5 = 0;
     input_D6 = 0;
     input_D7 = 0;
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
     // First reset it so that the registers are in a known good state
     // to ModelSim.
     //
     // Otherwise they will register as unknowns, even though FPGA registers
     // start at 0 by default.
     //

     input_MR = 0;
     @(posedge clock_50);
     input_MR = 1;
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
     // Test Master RESET (MR)
     //

     input_MR = 0;
     @(posedge clock_50);
     input_MR = 1;
     @(posedge clock_50);
     assert_output_values(8'b00000000);

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

    `tb_x74xx273_assert(output_Q0, values_to_assert[0:0])
    `tb_x74xx273_assert(output_Q1, values_to_assert[1:1])
    `tb_x74xx273_assert(output_Q2, values_to_assert[2:2])
    `tb_x74xx273_assert(output_Q3, values_to_assert[3:3])
    `tb_x74xx273_assert(output_Q4, values_to_assert[4:4])
    `tb_x74xx273_assert(output_Q5, values_to_assert[5:5])
    `tb_x74xx273_assert(output_Q6, values_to_assert[6:6])
    `tb_x74xx273_assert(output_Q7, values_to_assert[7:7])

  end  
endtask

task set_input_values;
  input [7:0] values_to_assert;

  begin

     input_D0 = values_to_assert[0:0];
     input_D1 = values_to_assert[1:1];
     input_D2 = values_to_assert[2:2];
     input_D3 = values_to_assert[3:3];
     input_D4 = values_to_assert[4:4];
     input_D5 = values_to_assert[5:5];
     input_D6 = values_to_assert[6:6];
     input_D7 = values_to_assert[7:7];

  end  
endtask

endmodule

