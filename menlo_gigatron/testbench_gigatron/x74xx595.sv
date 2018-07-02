
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
// 07/01/2018
//
// 74xx595/SN74HCT595 8  bit shift register with 3 state output TTL chip in 16 pin DIP.
//
// http://www.ti.com/lit/ds/symlink/sn74hc595.pdf
//
// Note: Order of inputs and output signals follows the sequence of
// pins on the physical chip 1 - 16.
//
// Pin 8 is GND, Pin 16 is VCC which are not required here.
//
module x74xx595
(
    output QB,       // QB on datasheet (pin 1)
    output QC,       // QC on datasheet (pin 2)
    output QD,       // QD on datasheet (pin 3)
    output QE,       // QE on datasheet (pin 4)
    output QF,       // QF on datasheet (pin 5)
    output QG,       // QG on datasheet (pin 6)
    output QH,       // QH on datasheet (pin 7)
    output QH_L,     // QH' on datasheet (pin 9) (QH, bit 7 look ahead)
    input  SRCLR_N,  // SRCLR_N on datasheet (pin 10)
    input  SRCLK,    // SRCLK on datasheet (pin 11)
    input  RCLK,     // RCLK on datasheet (pin 12)
    input  OE_N,     // OE_N on datasheet (pin 13)
    input  SER,      // SER on datasheet (pin 14)
    output QA        // QA on datasheet (pin 15)
);

  //
  // The x74xx595 is a positive clock edge device.
  //
  // Serial in, 8 bit out serial to parallel shift register.
  // 
  // As expected a serial input pin (SER) is sampled on a serial
  // clock input (SRCLK).
  // 
  // Each serial clock moves the 8 bit shift register one position
  // over with bit 0 being the current state of the serial input
  // pin.
  // 
  // There is an output register with its own clock/signal (RCLK) and when
  // this signal is high what ever is stable in the shift register
  // is latched into the output register flip flops and becomes a stable
  // parallel output signal regardless of additional serial clock inputs.
  // (QA - QH)
  // 
  // QH' (QH_L) is a look ahead presenting the value of bit 7 of the
  // shift register before being latched into the output register
  // by RCLK. If both SCLK && RCLK are tied together it is a one
  // clock look ahead for the value about to enter bit 7 of the output
  // register.
  //
  // If both serial clock input and output register clock are together,
  // the output register follows the serial shift register state by
  // one bit.
  // 
  // There is an output enable for the parallel outputs that tri-states
  // the outputs when high, and enables the outputs when low. !OE
  // 
  // There is a serial register clear input to start at 0 when it
  // is asserted low (!SRCLR).
  // 
  // SER   - Serial data in
  // 
  // SRCLK - Serial clock for serial data in. Data is sampled on positive edge.
  // 
  // QA-QH - 8 bit output from output latch register
  // 
  // QH_L - Direct output from bit 7 of the shift register.
//          Early carry out indication.
  //
  // RCLK - Write value from serial shift register into output latch register on
  //        positive edge. This holds the value till the next RCLK regardless
  //        of new serial input from SRCLK and changes to the serial shift
  //        register..
  // 
  // !OE - Enable output when low. Tristate when high.
  //
  // !SRCLR - Clear serial shift register to zero.
  // 

  reg [7:0] reg_output;
  reg [7:0] reg_shift;

  //
  // Implement tristate when OE_N == 1
  //
  assign QA = (OE_N == 1'b0) ? reg_output[0:0] : 1'hZ;
  assign QB = (OE_N == 1'b0) ? reg_output[1:1] : 1'hZ;
  assign QC = (OE_N == 1'b0) ? reg_output[2:2] : 1'hZ;
  assign QD = (OE_N == 1'b0) ? reg_output[3:3] : 1'hZ;
  assign QE = (OE_N == 1'b0) ? reg_output[4:4] : 1'hZ;
  assign QF = (OE_N == 1'b0) ? reg_output[5:5] : 1'hZ;
  assign QG = (OE_N == 1'b0) ? reg_output[6:6] : 1'hZ;
  assign QH = (OE_N == 1'b0) ? reg_output[7:7] : 1'hZ;

  //
  // The schematic on the datasheet shows this directly wired
  // to shift_register bit 7 output.
  //
  assign QH_L = (OE_N == 1'b0) ? reg_shift[7:7] : 1'hZ;

  //
  // Serial input clock
  //
  always @ (posedge SRCLK or negedge SRCLR_N) begin
      if (SRCLR_N == 1'b0) begin
          // clear, reset
          reg_shift <= 0;
      end
      else begin
          reg_shift <= reg_shift << 1'b1;

          //
          // This second assignment only sets the lower bit leaving
          // the upper bits to be set by the above shift assignment.
          //
          reg_shift[0:0] <= SER;
      end
  end

  //
  // Output register clock
  //
  always @ (posedge RCLK) begin
      reg_output <= reg_shift;
  end

endmodule

`define tb_x74xx595_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

//
// Test bench
//
module tb_x74xx595();

  reg [7:0] tmp;

  reg clock_50;

  reg input_SRCLR_N;
  reg input_SRCLK;
  reg input_RCLK;
  reg input_OE_N;
  reg input_SER;

  wire output_QA;
  wire output_QB;
  wire output_QC;
  wire output_QD;
  wire output_QE;
  wire output_QF;
  wire output_QG;
  wire output_QH;
  wire output_QH_L;

  // Device Under Test instance
  x74xx595 DUT(

      .QB(output_QB),
      .QC(output_QC),
      .QD(output_QD),
      .QE(output_QE),
      .QF(output_QF),
      .QG(output_QG),
      .QH(output_QH),
      .QH_L(output_QH_L),

      .SRCLR_N(input_SRCLR_N),
      .SRCLK(input_SRCLK),
      .RCLK(input_RCLK),
      .OE_N(input_OE_N),
      .SER(input_SER),
      .QA(output_QA)
  );

  // Set initial values
  initial begin
     clock_50 = 0;

     input_SRCLR_N = 1;
     input_SRCLK = 0;
     input_RCLK = 0;
     input_OE_N = 1;
     input_SER = 0;
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

     // Clear serial shift register
     @(posedge clock_50);
     input_SRCLR_N = 0;
     @(posedge clock_50);
     input_SRCLR_N = 1;
     @(posedge clock_50);

     //
     // Clear output register by reading the now
     // cleared serial shift register into it.
     //

     input_RCLK = 1;
     @(posedge clock_50);
     input_RCLK = 0;
     @(posedge clock_50);

     // output enable
     input_OE_N = 0;
     @(posedge clock_50);

     assert_output_values(8'b00000000);

     set_serial_input_value(8'b01010101);
     @(posedge clock_50);

     // Transfer it to the output register
     input_RCLK = 1;
     @(posedge clock_50);
     input_RCLK = 0;
     @(posedge clock_50);

     assert_output_values(8'b01010101);

     @(posedge clock_50);

     set_serial_input_value(8'b11111111);
     @(posedge clock_50);

     // Transfer it to the output register
     input_RCLK = 1;
     @(posedge clock_50);
     input_RCLK = 0;
     @(posedge clock_50);

     assert_output_values(8'b11111111);
     @(posedge clock_50);

     set_serial_input_value(8'b10101010);
     @(posedge clock_50);

     // Transfer it to the output register
     input_RCLK = 1;
     @(posedge clock_50);
     input_RCLK = 0;
     @(posedge clock_50);

     assert_output_values(8'b10101010);
     @(posedge clock_50);

     set_serial_input_value(8'b00000000);
     @(posedge clock_50);

     // Transfer it to the output register
     input_RCLK = 1;
     @(posedge clock_50);
     input_RCLK = 0;
     @(posedge clock_50);

     assert_output_values(8'b00000000);
     @(posedge clock_50);

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

    `tb_x74xx595_assert(output_QA, values_to_assert[0:0])
    `tb_x74xx595_assert(output_QB, values_to_assert[1:1])
    `tb_x74xx595_assert(output_QC, values_to_assert[2:2])
    `tb_x74xx595_assert(output_QD, values_to_assert[3:3])
    `tb_x74xx595_assert(output_QE, values_to_assert[4:4])
    `tb_x74xx595_assert(output_QF, values_to_assert[5:5])
    `tb_x74xx595_assert(output_QG, values_to_assert[6:6])
    `tb_x74xx595_assert(output_QH, values_to_assert[7:7])

  end  
endtask

task set_serial_input_value;
  input [7:0] value_to_insert;

  begin

     // Shift the value into the serial input
     tmp = value_to_insert;

     // Bit 7
     // Transmit MSB first
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

     // Bit 6
     tmp = tmp << 1;
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

     // Bit 5
     tmp = tmp << 1;
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

     // Bit 4
     tmp = tmp << 1;
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

     // Bit 3
     tmp = tmp << 1;
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

     // Bit 2
     tmp = tmp << 1;
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

     // Bit 1
     tmp = tmp << 1;
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

     // Bit 0
     tmp = tmp << 1;
     input_SER = tmp[7:7];

     input_SRCLK = 1;
     @(posedge clock_50);
     input_SRCLK = 0;
     @(posedge clock_50);

  end  
endtask

endmodule
