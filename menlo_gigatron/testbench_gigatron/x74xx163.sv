
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
// 74xx163/SN74HCT163 4 bit settable counter TTL chip in 16 pin DIP.
//
// http://www.ti.com/lit/ds/symlink/cd74hc273.pdf
//
// Note: Order of inputs and output signals follows the sequence of
// pins on the physical chip 1 - 20.
//
// Pin 8 is GND, Pin 16 is VCC which are not required here.
//
module x74xx163
(
    input  MR_N,    // MR on datasheet (pin 1)
    input  CP,      // CP on datasheet (pin 2)

    input  P0,      // P0 on datasheet (pin 3)
    input  P1,      // P1 on datasheet (pin 4)
    input  P2,      // P2 on datasheet (pin 5)
    input  P3,      // P3 on datasheet (pin 6)

    input  PE,      // PE on datasheet (pin 7)
    input  SPE_N,   // SPE on datasheet (pin 9)
    input  TE,      // TE  on datasheet (pin 10)

    output Q3,      // Q3 on datasheet (pin 11)
    output Q2,      // Q3 on datasheet (pin 12)
    output Q1,      // Q3 on datasheet (pin 13)
    output Q0,      // Q3 on datasheet (pin 14)

    output TC       // TC on datasheet (pin 15)
);

  //
  // The x74xx163 is a positive clock edge device.
  //
  // Presetable 4 bit binary counter.
  // 
  // An initial value may loaded into the counter through P0-P3.
  // 
  // !MR - master reset.
  // 
  // CP  - positive edge clock.
  // 
  // P0-P3 - Input to counter when !SPE is low.
  // 
  // Q0-Q3 - outputs
  // 
  // !SPE - Synchronous Parallel Enable Input.
  //        Loads data from P0-P3 into the counter.
  // 
  // PE  - Count Enable xxx
  //       Both PE & TE must be high to count.
  //       Used for cascading
  // 
  // TE  - Count Enable xxx
  //       Both PE & TE must be high to count.
  //       Used for cascading
  // 
  // TC  - high on terminal count of 1'b1111
  // 
  // From the data sheet page 1:
  //
  // The look-ahead carry feature simplifies serial cascading of
  // the counters. Both count enable inputs (PE and TE) must
  // be high to count. The TE input is gated with the Q outputs
  // of all four stages so that at the maximum count the terminal
  // count (TC) output goes high for one clock period. This TC
  // pulse is used to enable the next cascaded stage  // 
  // 

  reg [3:0] reg_counter;
  reg       reg_TC;

  wire      gated_TE;

  assign Q0 = reg_counter[0:0];
  assign Q1 = reg_counter[1:1];
  assign Q2 = reg_counter[2:2];
  assign Q3 = reg_counter[3:3];

  assign TC = reg_TC;

  // TE input is gated by TC according to datasheet
  assign gated_TE = TE & ~TC;  

  always @ (posedge CP) begin

    //
    // Note: To understand this logic, a review of the rules
    // of a clocked always @ block:
    //
    // On clock rising edge (posedge CP), all registered variables
    // will have the values set at the *last* clock pulse.
    //
    // All assignments to the registered variables will not take
    // effect till the end of all assignment statements in the
    // current always block, and will occur at the end of the current
    // clock cycle.
    //
    // This means that if a registered variable is assigned to, its
    // value read remains the same during the entire always block regardless
    // of any assignments within the current clock cycle. It will *not*
    // reflect any new values till the *next* clock cycle.
    //
    // If there are multiple assignments to the same registered variable
    // in the same clock cycle only the last value remains. The other
    // intermediate values are *lost*. If you want to present them such
    // as driving an output, a state machine is required.
    //
    // Always use non-blocking assignments "<=" and not "=". If you forget
    // and make a mistake weird results are guaranteed when the circuit
    // is synthesized.
    //
    // There are two types of signals:
    //
    //  Clocked signals: These are stable signals declared as register (reg)
    //  and updated using the above clocked logic rules. These are safe
    //  to evaluate and rely upon in the always block. These operate by
    //  holding stable state in flip-flops (1 bit memory cells) that
    //  are only changed between clock edges (posedge clock) within
    //  a clocked always @ block.
    //
    //  Real time signals: These are the result of combinatorial logic
    //  such as assign statements and represent a non-clocked computed
    //  output of a set of input signals. These are essentially logic
    //  gates with inputs and outputs that bounce around depending on
    //  timing and stability of their input signals. If all of these
    //  input signals are stable registers then the output is stable
    //  during the always block execution. If one or more of the inputs
    //  are unregistered, such as an external signal, then the timing
    //  of the output is controlled by the unregistered signal and any
    //  gate delays within the combinatorial logic generated. As such
    //  they typically require special handling to gate these signals
    //  within the setup and hold times required.
    //  For this application of emulating a x74xx163.sv TTL chip the
    //  data sheet specifies these setup and hold times, so the wider
    //  range of the chip being modeled should fit within the setup
    //  and hold times required by the FPGA logic, which is a faster
    //  part using internal wiring. But this is not guaranteed and
    //  attention should be paid to final circuit designs utilizing
    //  these emulations.
    //
    
    if (MR_N == 1'b0) begin
        reg_counter <= 0;
        reg_TC <= 0;
    end
    else begin

      //
      // Not Reset
      //

      //
      // If TC has been set, its cleared after one clock interval.
      //
      // Counting will not occur for this clock cycle since the
      // value of reg_TC will remain one until the end, thus blocking
      // the counter through gated_TE.
      //
      // Note according to reading the data sheet the one clock
      // hold then reset for TC is regardless of the value of
      // the PE and TE inputs, but obey master reset above.
      //
      if (reg_TC == 1'b1) begin
        reg_TC <= 0;
      end

      if (SPE_N == 1'b0) begin

        // Load counter. No count in this clock period.
        reg_counter[0:0] <= P0;
        reg_counter[1:1] <= P1;
        reg_counter[2:2] <= P2;
        reg_counter[3:3] <= P3;

        // Obey TC logic if loaded with terminal count value
        if ((P0 == 1'b1) && 
            (P1 == 1'b1) &&
            (P2 == 1'b1) &&
            (P3 == 1'b1)) begin

            reg_TC <= 1'b1;
        end
      end
      else begin

        if ((PE == 1'b1) && gated_TE) begin

          //
          // Count is enabled
          //
          // Both count enable inputs (PE and TE) must
          // be high to count. The TE input is gated with the Q outputs
          // of all four stages so that at the maximum count the terminal
          // count (TC) output goes high for one clock period. This TC
          // pulse is used to enable the next cascaded stage.
          // 

          if (reg_counter == 4'b1110) begin

            //
            // At the end of this clock cycle the counter will
            // be at the terminal count so TC needs to be set.
            //
            // TC will gate the TE input for the cycle its set
            // to allow TC to be a carry out for one clock cycle.
            //

            reg_TC <= 1'b1;
          end

          // Counter performs a 4 bit binary wrap around
          reg_counter <= reg_counter + 4'b0001;

        end // PE is enabled
      end // not SPE_N
    end // not reset
  end // always

endmodule

`define tb_x74xx163_assert(signal, value) \
    if (signal !== value) begin \
	     $display("ASSERTION FAILED in %m: signal != value"); \
		  $stop; \
    end

//
// Test bench
//
module tb_x74xx163();

  reg clock_50;

  reg input_MR_N;
  reg input_SPE_N;
  reg input_PE;
  reg input_TE;

  reg input_P0;
  reg input_P1;
  reg input_P2;
  reg input_P3;

  wire output_Q0;
  wire output_Q1;
  wire output_Q2;
  wire output_Q3;
  wire output_TC;

  // Device Under Test instance
  x74xx163 DUT(

      .MR_N(input_MR_N),
      .CP(clock_50),

      .P0(input_P0),
      .P1(input_P1),
      .P2(input_P2),
      .P3(input_P3),

      .PE(input_PE),
      .SPE_N(input_SPE_N),
      .TE(input_TE),

      .Q3(output_Q3),
      .Q2(output_Q2),
      .Q1(output_Q1),
      .Q0(output_Q0),

      .TC(output_TC)      
  );

  // Set initial values
  initial begin
     clock_50 = 0;

     input_MR_N = 1;
     input_SPE_N = 1;
     input_PE = 0;
     input_TE = 0;

     input_P0 = 0;
     input_P1 = 0;
     input_P2 = 0;
     input_P3 = 0;
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

     // Reset the logic
     input_MR_N = 0;
     @(posedge clock_50);
     input_MR_N = 1;
     @(posedge clock_50);

     //
     // Gate input values to the counter, verify its output.
     //
     // Note: count enables are disabled here.
     //

     // Disable count enable so we can read a stable output
     input_PE = 0;
     input_TE = 0;

     set_input_values(4'b0000);
     @(posedge clock_50);
     assert_output_values(4'b0000);
    `tb_x74xx163_assert(output_TC, 1'b0)

     set_input_values(4'b0101);
     @(posedge clock_50);
     assert_output_values(4'b0101);
    `tb_x74xx163_assert(output_TC, 1'b0)

     set_input_values(4'b1010);
     @(posedge clock_50);
     assert_output_values(4'b1010);
    `tb_x74xx163_assert(output_TC, 1'b0)

     set_input_values(4'b1111);
     @(posedge clock_50);
     assert_output_values(4'b1111);

     //
     // TC should be 1 since we loaded the terminal count
     //
     // Note: datasheet is not clear on TC getting set for one
     // clock after counting to the terminal count, vs. getting
     // loaded with the terminal count directly, but with counting
     // disabled.
     //

     // TC should be 1 for one clock after.
    `tb_x74xx163_assert(output_TC, 1'b1)
     @(posedge clock_50);

     // TC should be 0 after one clock.
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count should remain stable since PE and TE are disabled
     assert_output_values(4'b1111);

     @(posedge clock_50);

     //
     // Now test the counter values
     //

     //
     // Enable the counter inputs. We should expect it to
     // count from 4'b1110 => 4'b1111 with a pulse on
     // TC.
     //

     set_input_values(4'b1110);
     @(posedge clock_50);
     assert_output_values(4'b1110);
    `tb_x74xx163_assert(output_TC, 1'b0)

     //
     // Everything tests good to here.
     //

     input_PE = 1;
     input_TE = 1;

     // Allow a clock to recognize enable
     @(posedge clock_50);

     // Allow a clock to count from 4'b1110 => 4'b1111
     @(posedge clock_50);

     // TC should be 1 for one clock after the count to 4'b1111
    `tb_x74xx163_assert(output_TC, 1'b1)

     assert_output_values(4'b1111);

     // TC == 1 inhibits the counter for one cycle
     @(posedge clock_50);

     // count from 4'b1111 => 4'b0000
     @(posedge clock_50);

     // Count should have wrapped back to 0
     assert_output_values(4'b0000);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 1
     @(posedge clock_50);
     assert_output_values(4'b0001);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 2
     @(posedge clock_50);
     assert_output_values(4'b0010);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 3
     @(posedge clock_50);
     assert_output_values(4'b0011);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 4
     @(posedge clock_50);
     assert_output_values(4'b0100);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 5
     @(posedge clock_50);
     assert_output_values(4'b0101);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 6
     @(posedge clock_50);
     assert_output_values(4'b0110);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 7
     @(posedge clock_50);
     assert_output_values(4'b0111);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 8
     @(posedge clock_50);
     assert_output_values(4'b1000);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 9
     @(posedge clock_50);
     assert_output_values(4'b1001);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 10
     @(posedge clock_50);
     assert_output_values(4'b1010);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 11
     @(posedge clock_50);
     assert_output_values(4'b1011);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 12
     @(posedge clock_50);
     assert_output_values(4'b1100);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 13
     @(posedge clock_50);
     assert_output_values(4'b1101);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 14
     @(posedge clock_50);
     assert_output_values(4'b1110);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count to 15
     @(posedge clock_50);
     assert_output_values(4'b1111);

     // TC goes to 1 at 4'b1111 and inhibits the count for one cycle
    `tb_x74xx163_assert(output_TC, 1'b1)

     @(posedge clock_50);
    `tb_x74xx163_assert(output_TC, 1'b0)

     // Count should not change for 1 cycle after TC
     assert_output_values(4'b1111);

     // One more clock to wrap around counter
     @(posedge clock_50);

     // Count should wrap around to 0 and TC should be 0
     assert_output_values(4'b0000);
    `tb_x74xx163_assert(output_TC, 1'b0)

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

    `tb_x74xx163_assert(output_Q0, values_to_assert[0:0])
    `tb_x74xx163_assert(output_Q1, values_to_assert[1:1])
    `tb_x74xx163_assert(output_Q2, values_to_assert[2:2])
    `tb_x74xx163_assert(output_Q3, values_to_assert[3:3])

  end  
endtask

task set_input_values;
  input [3:0] values_to_assert;

  begin

     input_P0 = values_to_assert[0:0];
     input_P1 = values_to_assert[1:1];
     input_P2 = values_to_assert[2:2];
     input_P3 = values_to_assert[3:3];

     // Pulse SPE_N to gate the inputs to the outputs
     input_SPE_N = 0;
     @(posedge clock_50);
     input_SPE_N = 1;

  end  
endtask

endmodule
