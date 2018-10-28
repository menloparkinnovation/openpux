
//
// Template Application Shell
//
// 10/26/2018
//

//
// See MENLO_COPYRIGHT.TXT for the copyright notice for this file.
//
// Use of this file is not allowed without the copyright notice present
// and re-distributed along with these files.
//

// set timescale for 1ns with 100ps precision.
`timescale 1ns / 100ps

module Application_Shell (

    // FPGA 50Mhz clock
    input       fpga_clock,

    // VGA 25Mhz clock
    input       vga_clock,

    // 6.25 Mhz Application clock
    // Note: Adjust project PLL(s) to suit your application.
    input       application_clock,

    // Reset when == 1
    input reset,

    //
    // Write output to external framebuffer
    //
    // Allows application to display on the screen with simple
    // framebuffer memory writes.
    //
    output        framebuffer_write_clock,
    output        framebuffer_write_signal,
    output [18:0] framebuffer_write_address,
    output  [7:0] framebuffer_write_data,

    // 16 bit LPCM audio output from the Gigatron.
    output [15:0] audio_dac,

    // Digital volume control with range 0 - 11.
    input [3:0] digital_volume_control,

    // Signals from push button/switch user interface to control application.
    input application_go,      // true when user pushes application go button
    output application_active, // true when application is active

    // Switches
    input [3:0] switches,

    // LED's
    output [3:0]  leds,

    //
    // I/O Port signals
    //
    // Arduino I/O is a popular setup.
    //
    // Some boards have an Arduino UNO header which allows
    // shields to be directly connected.
    //
    // Boards with standard GPIO pins assign 16 pins to
    // Arduino I/O compatible signals and use a breakout
    // board to connect to an Arduino compatible header.
    //
    // Even without an Arduino shield, its a popular wiring
    // convention since you can move a project between an Arduino
    // and the FPGA for testing.
    //
    inout [15:0] arduino_io,
    inout arduino_reset_n
  );

  // Default output assignments. Replace with application logic.

  assign framebuffer_write_clock = 1'b0;
  assign framebuffer_write_signal = 1'b0;
  assign framebuffer_write_address = 19'd0;
  assign framebuffer_write_data = 8'd0;

  assign audio_dac = 16'd0;

  //
  // Note: to use arduino_io signals as inputs you must assign its
  // output to hi-z.
  //
  // This is required to input on the port without conflict since
  // the FPGA Arduino ports have been configured for bi-directional
  // operation, similar to an Arduino's native ports and the FPGA
  // way to dynamically configure for input it to assign hi-z to
  // its output driver.
  //
  // Arduino I/O 2 (usually an interrupt input pin) is assigned
  // to hi-z here as an example to allow its use as an input
  // such as responding to the Arduino INTR signal that comes from
  // many shields. It can be "polled" using an always@(posedge fpga_clock)
  // to operate as a true interrupt input at FPGA clock speed.
  //

  //
  // Assign arduino_io[2:2] output to hiZ as its only an input.
  //
  assign arduino_io[2:2] = 1'bz;

  //
  // Invoke application module here.
  //
  // Note: Create it as a module in a separate file so it
  // can be moved from board to board by adapting the board
  // specific application_shell and configuration for its requirements.
  //

  example_application app(
    .clock(fpga_clock),
    .reset(reset),
    .application_go(application_go),
    .application_active(application_active),
    .leds(leds)
  );

endmodule

//
// Example application module.
//
module example_application (
    input  clock,
    input  reset,
    input  application_go,     // true when user pushes application go button
    output application_active, // true when application is active
    output [3:0] leds
  );

  reg reg_application_active;
  assign application_active = reg_application_active;

  reg [3:0] reg_leds;
  assign leds = reg_leds;

  always@(posedge clock) begin
    if (reset == 1'b1) begin

      //
      // Arduino setup()
      //

      reg_application_active <= 1'b0;
      reg_leds <= 4'b0000;
    end
    else begin

      //
      // Arduino loop()
      //

      if (application_go == 1'b1) begin

        // This will cause the UI program active LED to flash
        reg_application_active <= 1'b1;

       // Light the LED's
       reg_leds <= 4'b1111;
      end
    end
  end
endmodule


