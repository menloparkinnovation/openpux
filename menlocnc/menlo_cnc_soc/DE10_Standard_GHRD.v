// ============================================================================
// Copyright (c) 2016 by Terasic Technologies Inc.
// ============================================================================
//
// Permission:
//
//   Terasic grants permission to use and modify this code for use
//   in synthesis for all Terasic Development Boards and Altera Development 
//   Kits made by Terasic.  Other use of this code, including the selling 
//   ,duplication, or modification of any portion is strictly prohibited.
//
// Disclaimer:
//
//   This VHDL/Verilog or C/C++ source code is intended as a design reference
//   which illustrates how these types of functions can be implemented.
//   It is the user's responsibility to verify their design for
//   consistency and functionality through the use of formal
//   verification methods.  Terasic provides no warranty regarding the use 
//   or functionality of this code.
//
// ============================================================================
//           
//  Terasic Technologies Inc
//  9F., No.176, Sec.2, Gongdao 5th Rd, East Dist, Hsinchu City, 30070. Taiwan
//  
//  
//                     web: http://www.terasic.com/  
//                     email: support@terasic.com
//
// ============================================================================
//Date:  Tue Sep 27 10:46:00 2016
// ============================================================================

`define ENABLE_HPS
//`define ENABLE_HSMC

// Disable for now due to conflicts on GPIO.
//`define ENABLE_RAMPS_SIGNALS

module DE10_Standard_GHRD(


      ///////// CLOCK /////////
      input              CLOCK2_50,
      input              CLOCK3_50,
      input              CLOCK4_50,
      input              CLOCK_50,

      ///////// KEY /////////
      input    [ 3: 0]   KEY,

      ///////// SW /////////
      input    [ 9: 0]   SW,

      ///////// LED /////////
      output   [ 9: 0]   LEDR,

      ///////// Seg7 /////////
      output   [ 6: 0]   HEX0,
      output   [ 6: 0]   HEX1,
      output   [ 6: 0]   HEX2,
      output   [ 6: 0]   HEX3,
      output   [ 6: 0]   HEX4,
      output   [ 6: 0]   HEX5,

      ///////// SDRAM /////////
      output             DRAM_CLK,
      output             DRAM_CKE,
      output   [12: 0]   DRAM_ADDR,
      output   [ 1: 0]   DRAM_BA,
      inout    [15: 0]   DRAM_DQ,
      output             DRAM_LDQM,
      output             DRAM_UDQM,
      output             DRAM_CS_N,
      output             DRAM_WE_N,
      output             DRAM_CAS_N,
      output             DRAM_RAS_N,

      ///////// Video-In /////////
      input              TD_CLK27,
      input              TD_HS,
      input              TD_VS,
      input    [ 7: 0]   TD_DATA,
      output             TD_RESET_N,

      ///////// VGA /////////
      output             VGA_CLK,
      output             VGA_HS,
      output             VGA_VS,
      output   [ 7: 0]   VGA_R,
      output   [ 7: 0]   VGA_G,
      output   [ 7: 0]   VGA_B,
      output             VGA_BLANK_N,
      output             VGA_SYNC_N,

      ///////// Audio /////////
      inout              AUD_BCLK,
      output             AUD_XCK,
      inout              AUD_ADCLRCK,
      input              AUD_ADCDAT,
      inout              AUD_DACLRCK,
      output             AUD_DACDAT,

      ///////// PS2 /////////
      inout              PS2_CLK,
      inout              PS2_CLK2,
      inout              PS2_DAT,
      inout              PS2_DAT2,

      ///////// ADC /////////
      output             ADC_SCLK,
      input              ADC_DOUT,
      output             ADC_DIN,
      output             ADC_CONVST,

      ///////// I2C for Audio and Video-In /////////
      output             FPGA_I2C_SCLK,
      inout              FPGA_I2C_SDAT,

      ///////// GPIO /////////
      inout    [35: 0]   GPIO,

`ifdef ENABLE_HSMC
      ///////// HSMC /////////
      input              HSMC_CLKIN_P1,
      input              HSMC_CLKIN_N1,
      input              HSMC_CLKIN_P2,
      input              HSMC_CLKIN_N2,
      output             HSMC_CLKOUT_P1,
      output             HSMC_CLKOUT_N1,
      output             HSMC_CLKOUT_P2,
      output             HSMC_CLKOUT_N2,
      inout    [16: 0]   HSMC_TX_D_P,
      inout    [16: 0]   HSMC_TX_D_N,
      inout    [16: 0]   HSMC_RX_D_P,
      inout    [16: 0]   HSMC_RX_D_N,
      input              HSMC_CLKIN0,
      output             HSMC_CLKOUT0,
      inout    [ 3: 0]   HSMC_D,
      output             HSMC_SCL,
      inout              HSMC_SDA,
`endif /*ENABLE_HSMC*/

`ifdef ENABLE_HPS
      ///////// HPS /////////
      inout              HPS_CONV_USB_N,
      output      [14:0] HPS_DDR3_ADDR,
      output      [2:0]  HPS_DDR3_BA,
      output             HPS_DDR3_CAS_N,
      output             HPS_DDR3_CKE,
      output             HPS_DDR3_CK_N,
      output             HPS_DDR3_CK_P,
      output             HPS_DDR3_CS_N,
      output      [3:0]  HPS_DDR3_DM,
      inout       [31:0] HPS_DDR3_DQ,
      inout       [3:0]  HPS_DDR3_DQS_N,
      inout       [3:0]  HPS_DDR3_DQS_P,
      output             HPS_DDR3_ODT,
      output             HPS_DDR3_RAS_N,
      output             HPS_DDR3_RESET_N,
      input              HPS_DDR3_RZQ,
      output             HPS_DDR3_WE_N,
      output             HPS_ENET_GTX_CLK,
      inout              HPS_ENET_INT_N,
      output             HPS_ENET_MDC,
      inout              HPS_ENET_MDIO,
      input              HPS_ENET_RX_CLK,
      input       [3:0]  HPS_ENET_RX_DATA,
      input              HPS_ENET_RX_DV,
      output      [3:0]  HPS_ENET_TX_DATA,
      output             HPS_ENET_TX_EN,
      inout       [3:0]  HPS_FLASH_DATA,
      output             HPS_FLASH_DCLK,
      output             HPS_FLASH_NCSO,
      inout              HPS_GSENSOR_INT,
      inout              HPS_I2C1_SCLK,
      inout              HPS_I2C1_SDAT,
      inout              HPS_I2C2_SCLK,
      inout              HPS_I2C2_SDAT,
      inout              HPS_I2C_CONTROL,
      inout              HPS_KEY,
      inout              HPS_LCM_BK,
      inout              HPS_LCM_D_C,
      inout              HPS_LCM_RST_N,
      output             HPS_LCM_SPIM_CLK,
      output             HPS_LCM_SPIM_MOSI,
      output             HPS_LCM_SPIM_SS,
		input 				 HPS_LCM_SPIM_MISO,
      inout              HPS_LED,
      inout              HPS_LTC_GPIO,
      output             HPS_SD_CLK,
      inout              HPS_SD_CMD,
      inout       [3:0]  HPS_SD_DATA,
      output             HPS_SPIM_CLK,
      input              HPS_SPIM_MISO,
      output             HPS_SPIM_MOSI,
      inout              HPS_SPIM_SS,
      input              HPS_UART_RX,
      output             HPS_UART_TX,
      input              HPS_USB_CLKOUT,
      inout       [7:0]  HPS_USB_DATA,
      input              HPS_USB_DIR,
      input              HPS_USB_NXT,
      output             HPS_USB_STP,
`endif /*ENABLE_HPS*/
      ///////// IR /////////
      output             IRDA_TXD,
      input              IRDA_RXD
);


//=======================================================
//  REG/WIRE declarations
//=======================================================
//=======================================================
//  REG/WIRE declarations
//=======================================================
  wire  hps_fpga_reset_n;
  wire [3:0] fpga_debounced_buttons;
  wire [8:0]  led_pio_external_connection_export;
  wire [2:0]  hps_reset_req;
  wire        hps_cold_reset;
  wire        hps_warm_reset;
  wire        hps_debug_reset;
  wire [27:0] stm_hw_events;
  wire        fpga_clk_50;
  wire        fpga_clk_25;

//=======================================================
//  REG/WIRE declarations for timing_generator_registers
//=======================================================

   // Signals from soc_system generated by QSYS
   wire [7:0]  menlo_slave_template_0_user_interface_address;
   wire        menlo_slave_template_0_user_interface_read;
   wire        menlo_slave_template_0_user_interface_write;
   wire [31:0] menlo_slave_template_0_user_interface_readdata;
   wire [31:0] menlo_slave_template_0_user_interface_writedata;
   wire [3:0]  menlo_slave_template_0_user_interface_byteenable;
   
   // Hardware ESTOP button input to timing generator.
   wire        estop_in;

   wire        cnc_board_estop_in;
   wire        cnc_board_reset_out;

   // Outputs from timing generator
   wire        x_timing_generator_pulse;
   wire        x_timing_generator_direction;
   wire        y_timing_generator_pulse;
   wire        y_timing_generator_direction;
   wire        z_timing_generator_pulse;
   wire        z_timing_generator_direction;
   wire        a_timing_generator_pulse;
   wire        a_timing_generator_direction;

   // GRBL signals
   wire        grbl_enable;

   // RAMPS signals
   wire        ramps_x_enable;
   wire        ramps_y_enable;
   wire        ramps_z_enable;

//=======================================================
//  Assigns for menlo_cnc_register signals
//=======================================================

  assign GPIO[0:0] = cnc_board_reset_out; // D0 output
  assign cnc_board_estop_in = GPIO[1:1];  // D1 input
  assign GPIO[2:2] = x_timing_generator_pulse;
  assign GPIO[3:3] = y_timing_generator_pulse;
  assign GPIO[4:4] = z_timing_generator_pulse;
  assign GPIO[5:5] = x_timing_generator_direction;
  assign GPIO[6:6] = y_timing_generator_direction;
  assign GPIO[7:7] = z_timing_generator_direction;

  // Enable line for GRBL shield
  assign GPIO[8:8] = grbl_enable;

  assign GPIO[9:9] = a_timing_generator_pulse;
  assign GPIO[10:10] = a_timing_generator_direction;
  // GPIO11 D11  // D11-OUTPUT-A DB25 OUTPUT-A
  // GPIO12 D12  // D12-OUTPUT-B DB25 OUTPUT-B
  // GPIO13 D13  // D13-OUTPUT-C DB25 OUTPUT-C

`ifdef DEBUG_AVALON_MM_SLAVE

  // Ramps shield signals

  assign GPIO[14:14] = x_timing_generator_pulse;
  assign GPIO[15:15] = x_timing_generator_direction;
  assign GPIO[16:16] = ramps_y_enable;
   
  // E1 step needs extra axis   
  // GPIO17 A3  // A3-INPUT-C  DB25 INPUT-C,  RAMPS E1-STEP
  // GPIO18 A4  // A4-INPUT-D  DB25 INPUT-D,  RAMPS E1-DIR
  // GPIO19 A5  // A5-INPUT-E  DB25 INPUT-E,  RAMPS E1-EN
   
  assign GPIO[20:20] = y_timing_generator_pulse;
  assign GPIO[21:21] = y_timing_generator_direction;
  assign GPIO[22:22] = ramps_z_enable; // RAMPS Z-EN
   
  // 
  // GPIO23 D14 // RAMPS Y-MIN
  // GPIO24 D15 // RAMPS Y-MAX
  // GPIO25 D18 // RAMPS Z-MIN
  // GPIO26 D19 // RMAPS Z-MAX
  // GPIO27 D24 // RAMPS E0-EN
  // GPIO28 D26 // RAMPS E0-STEP
  // GPIO29 D28 // RAMPS E0-DIR
  // GPIO30 D30 // RAMPS E1-EN
  // GPIO31 D34 // RAMPS E1-DIR
  // GPIO32 D36 // RAMPS E1-STEP
  assign GPIO[33:33] = ramps_x_enable; // RAMPS X-EN

  assign GPIO[34:34] = z_timing_generator_pulse; // RAMPS Z-STEP
  assign GPIO[35:35] = z_timing_generator_direction; // RAMPS Z-DIR
`endif

  //
  // Assign Axis pulse signals to the LED's
  //

  assign LEDR[9:9] = a_timing_generator_direction;
  assign LEDR[8:8] = a_timing_generator_pulse;
  assign LEDR[7:7] = z_timing_generator_direction;
  assign LEDR[6:6] = z_timing_generator_pulse;
  assign LEDR[5:5] = y_timing_generator_direction;
  assign LEDR[4:4] = y_timing_generator_pulse;
  assign LEDR[3:3] = x_timing_generator_direction;
  assign LEDR[2:2] = x_timing_generator_pulse;
  //assign LEDR[1:1] = timing_generator_busy; // TODO: Need to export this signal
  // Note: LEDR[0:0] is current heartbeat

  //
  // Menlo CNC Signal defaults
  //
  assign estop_in = 1'b0;

  assign cnc_board_reset_out = 1'b0; // Default CNC_BOARD_RESET == FALSE

  //
  // GRBL signals
  //
  assign grbl_enable = 1'b1; // Default Enable == TRUE

  //
  // RAMPS signals
  //
  assign ramps_x_en = 1'b1; // RAMPS X-EN
  assign ramps_y_en = 1'b1; // RAMPS Y-EN
  assign ramps_z_en = 1'b1; // RAMPS Z-EN

  // connection of internal logics

  // Menlo: use these are outputs for CNC pulse signals
  //assign LEDR[9:1] = led_pio_external_connection_export;

  assign stm_hw_events    = {{4{1'b0}}, SW, led_pio_external_connection_export, fpga_debounced_buttons};
  assign fpga_clk_50=CLOCK_50;

  //
  // 25Mhz clock support to allow SignalTap to show circuit clock signal edge
  // signal transitions.
  //
  // If the FPGA logic is running on the same 50Mhz FPGA clock as SignalTap,
  // transitions during @always() blocks are not seen on the clock input, so
  // run the FPGA circuits being tested under a lower rate clock while SignTap
  // runs higher, as you would do with an external logic analyzer using timebase
  // sampling.
  //
   
  // Note: Strictly toggle is not needed unless more than 1
  reg 	       fpga_clk_25_toggle;
  reg 	       fpga_clk_25_counter;

  assign fpga_clk_25 = fpga_clk_25_toggle;

  always @(posedge fpga_clk_50 or negedge hps_fpga_reset_n) begin

      if(~hps_fpga_reset_n) begin
         fpga_clk_25_counter <= 0;
         fpga_clk_25_toggle <= 0;
      end
      else if (fpga_clk_25_counter == 1) begin
         fpga_clk_25_counter <= 0;

         if (fpga_clk_25_toggle == 1'b1) begin
             fpga_clk_25_toggle <= 0;
         end
         else begin
             fpga_clk_25_toggle <= 1;
         end
      end
      else begin
         fpga_clk_25_counter <= fpga_clk_25_counter + 1'b1;
      end
  end

//=======================================================
//  Structural coding
//=======================================================
soc_system u0 (      
		  .clk_clk                               (CLOCK_50),                             //                clk.clk
		  .reset_reset_n                         (1'b1),                                 //                reset.reset_n
		  //HPS ddr3
		  .memory_mem_a                          ( HPS_DDR3_ADDR),                       //                memory.mem_a
        .memory_mem_ba                         ( HPS_DDR3_BA),                         //                .mem_ba
        .memory_mem_ck                         ( HPS_DDR3_CK_P),                       //                .mem_ck
        .memory_mem_ck_n                       ( HPS_DDR3_CK_N),                       //                .mem_ck_n
        .memory_mem_cke                        ( HPS_DDR3_CKE),                        //                .mem_cke
        .memory_mem_cs_n                       ( HPS_DDR3_CS_N),                       //                .mem_cs_n
        .memory_mem_ras_n                      ( HPS_DDR3_RAS_N),                      //                .mem_ras_n
        .memory_mem_cas_n                      ( HPS_DDR3_CAS_N),                      //                .mem_cas_n
        .memory_mem_we_n                       ( HPS_DDR3_WE_N),                       //                .mem_we_n
        .memory_mem_reset_n                    ( HPS_DDR3_RESET_N),                    //                .mem_reset_n
        .memory_mem_dq                         ( HPS_DDR3_DQ),                         //                .mem_dq
        .memory_mem_dqs                        ( HPS_DDR3_DQS_P),                      //                .mem_dqs
        .memory_mem_dqs_n                      ( HPS_DDR3_DQS_N),                      //                .mem_dqs_n
        .memory_mem_odt                        ( HPS_DDR3_ODT),                        //                .mem_odt
        .memory_mem_dm                         ( HPS_DDR3_DM),                         //                .mem_dm
        .memory_oct_rzqin                      ( HPS_DDR3_RZQ),                        //                .oct_rzqin
       //HPS ethernet		
	     .hps_0_hps_io_hps_io_emac1_inst_TX_CLK ( HPS_ENET_GTX_CLK),       //                             hps_0_hps_io.hps_io_emac1_inst_TX_CLK
        .hps_0_hps_io_hps_io_emac1_inst_TXD0   ( HPS_ENET_TX_DATA[0] ),   //                             .hps_io_emac1_inst_TXD0
        .hps_0_hps_io_hps_io_emac1_inst_TXD1   ( HPS_ENET_TX_DATA[1] ),   //                             .hps_io_emac1_inst_TXD1
        .hps_0_hps_io_hps_io_emac1_inst_TXD2   ( HPS_ENET_TX_DATA[2] ),   //                             .hps_io_emac1_inst_TXD2
        .hps_0_hps_io_hps_io_emac1_inst_TXD3   ( HPS_ENET_TX_DATA[3] ),   //                             .hps_io_emac1_inst_TXD3
        .hps_0_hps_io_hps_io_emac1_inst_RXD0   ( HPS_ENET_RX_DATA[0] ),   //                             .hps_io_emac1_inst_RXD0
        .hps_0_hps_io_hps_io_emac1_inst_MDIO   ( HPS_ENET_MDIO ),         //                             .hps_io_emac1_inst_MDIO
        .hps_0_hps_io_hps_io_emac1_inst_MDC    ( HPS_ENET_MDC  ),         //                             .hps_io_emac1_inst_MDC
        .hps_0_hps_io_hps_io_emac1_inst_RX_CTL ( HPS_ENET_RX_DV),         //                             .hps_io_emac1_inst_RX_CTL
        .hps_0_hps_io_hps_io_emac1_inst_TX_CTL ( HPS_ENET_TX_EN),         //                             .hps_io_emac1_inst_TX_CTL
        .hps_0_hps_io_hps_io_emac1_inst_RX_CLK ( HPS_ENET_RX_CLK),        //                             .hps_io_emac1_inst_RX_CLK
        .hps_0_hps_io_hps_io_emac1_inst_RXD1   ( HPS_ENET_RX_DATA[1] ),   //                             .hps_io_emac1_inst_RXD1
        .hps_0_hps_io_hps_io_emac1_inst_RXD2   ( HPS_ENET_RX_DATA[2] ),   //                             .hps_io_emac1_inst_RXD2
        .hps_0_hps_io_hps_io_emac1_inst_RXD3   ( HPS_ENET_RX_DATA[3] ),   //                             .hps_io_emac1_inst_RXD3
       //HPS QSPI  
		  .hps_0_hps_io_hps_io_qspi_inst_IO0     ( HPS_FLASH_DATA[0]    ),     //                               .hps_io_qspi_inst_IO0
        .hps_0_hps_io_hps_io_qspi_inst_IO1     ( HPS_FLASH_DATA[1]    ),     //                               .hps_io_qspi_inst_IO1
        .hps_0_hps_io_hps_io_qspi_inst_IO2     ( HPS_FLASH_DATA[2]    ),     //                               .hps_io_qspi_inst_IO2
        .hps_0_hps_io_hps_io_qspi_inst_IO3     ( HPS_FLASH_DATA[3]    ),     //                               .hps_io_qspi_inst_IO3
        .hps_0_hps_io_hps_io_qspi_inst_SS0     ( HPS_FLASH_NCSO    ),        //                               .hps_io_qspi_inst_SS0
        .hps_0_hps_io_hps_io_qspi_inst_CLK     ( HPS_FLASH_DCLK    ),        //                               .hps_io_qspi_inst_CLK
       //HPS SD card 
		  .hps_0_hps_io_hps_io_sdio_inst_CMD     ( HPS_SD_CMD    ),           //                               .hps_io_sdio_inst_CMD
        .hps_0_hps_io_hps_io_sdio_inst_D0      ( HPS_SD_DATA[0]     ),      //                               .hps_io_sdio_inst_D0
        .hps_0_hps_io_hps_io_sdio_inst_D1      ( HPS_SD_DATA[1]     ),      //                               .hps_io_sdio_inst_D1
        .hps_0_hps_io_hps_io_sdio_inst_CLK     ( HPS_SD_CLK   ),            //                               .hps_io_sdio_inst_CLK
        .hps_0_hps_io_hps_io_sdio_inst_D2      ( HPS_SD_DATA[2]     ),      //                               .hps_io_sdio_inst_D2
        .hps_0_hps_io_hps_io_sdio_inst_D3      ( HPS_SD_DATA[3]     ),      //                               .hps_io_sdio_inst_D3
       //HPS USB 		  
		  .hps_0_hps_io_hps_io_usb1_inst_D0      ( HPS_USB_DATA[0]    ),      //                               .hps_io_usb1_inst_D0
        .hps_0_hps_io_hps_io_usb1_inst_D1      ( HPS_USB_DATA[1]    ),      //                               .hps_io_usb1_inst_D1
        .hps_0_hps_io_hps_io_usb1_inst_D2      ( HPS_USB_DATA[2]    ),      //                               .hps_io_usb1_inst_D2
        .hps_0_hps_io_hps_io_usb1_inst_D3      ( HPS_USB_DATA[3]    ),      //                               .hps_io_usb1_inst_D3
        .hps_0_hps_io_hps_io_usb1_inst_D4      ( HPS_USB_DATA[4]    ),      //                               .hps_io_usb1_inst_D4
        .hps_0_hps_io_hps_io_usb1_inst_D5      ( HPS_USB_DATA[5]    ),      //                               .hps_io_usb1_inst_D5
        .hps_0_hps_io_hps_io_usb1_inst_D6      ( HPS_USB_DATA[6]    ),      //                               .hps_io_usb1_inst_D6
        .hps_0_hps_io_hps_io_usb1_inst_D7      ( HPS_USB_DATA[7]    ),      //                               .hps_io_usb1_inst_D7
        .hps_0_hps_io_hps_io_usb1_inst_CLK     ( HPS_USB_CLKOUT    ),       //                               .hps_io_usb1_inst_CLK
        .hps_0_hps_io_hps_io_usb1_inst_STP     ( HPS_USB_STP    ),          //                               .hps_io_usb1_inst_STP
        .hps_0_hps_io_hps_io_usb1_inst_DIR     ( HPS_USB_DIR    ),          //                               .hps_io_usb1_inst_DIR
        .hps_0_hps_io_hps_io_usb1_inst_NXT     ( HPS_USB_NXT    ),          //                               .hps_io_usb1_inst_NXT
		  
		  //HPS SPI0->LCDM 	
        .hps_0_hps_io_hps_io_spim0_inst_CLK    ( HPS_LCM_SPIM_CLK),    //                               .hps_io_spim0_inst_CLK
        .hps_0_hps_io_hps_io_spim0_inst_MOSI   ( HPS_LCM_SPIM_MOSI),   //                               .hps_io_spim0_inst_MOSI
        .hps_0_hps_io_hps_io_spim0_inst_MISO   ( HPS_LCM_SPIM_MISO),   //                               .hps_io_spim0_inst_MISO
        .hps_0_hps_io_hps_io_spim0_inst_SS0    ( HPS_LCM_SPIM_SS),    //                               .hps_io_spim0_inst_SS0
       //HPS SPI1 		  
		  .hps_0_hps_io_hps_io_spim1_inst_CLK    ( HPS_SPIM_CLK  ),           //                               .hps_io_spim1_inst_CLK
        .hps_0_hps_io_hps_io_spim1_inst_MOSI   ( HPS_SPIM_MOSI ),           //                               .hps_io_spim1_inst_MOSI
        .hps_0_hps_io_hps_io_spim1_inst_MISO   ( HPS_SPIM_MISO ),           //                               .hps_io_spim1_inst_MISO
        .hps_0_hps_io_hps_io_spim1_inst_SS0    ( HPS_SPIM_SS ),             //                               .hps_io_spim1_inst_SS0
      //HPS UART		
		  .hps_0_hps_io_hps_io_uart0_inst_RX     ( HPS_UART_RX    ),          //                               .hps_io_uart0_inst_RX
        .hps_0_hps_io_hps_io_uart0_inst_TX     ( HPS_UART_TX    ),          //                               .hps_io_uart0_inst_TX
		//HPS I2C1
		  .hps_0_hps_io_hps_io_i2c0_inst_SDA     ( HPS_I2C1_SDAT    ),        //                               .hps_io_i2c0_inst_SDA
        .hps_0_hps_io_hps_io_i2c0_inst_SCL     ( HPS_I2C1_SCLK    ),        //                               .hps_io_i2c0_inst_SCL
		//HPS I2C2
		  .hps_0_hps_io_hps_io_i2c1_inst_SDA     ( HPS_I2C2_SDAT    ),        //                               .hps_io_i2c1_inst_SDA
        .hps_0_hps_io_hps_io_i2c1_inst_SCL     ( HPS_I2C2_SCLK    ),        //                               .hps_io_i2c1_inst_SCL
      //HPS GPIO  
		  .hps_0_hps_io_hps_io_gpio_inst_GPIO09  ( HPS_CONV_USB_N),           //                               .hps_io_gpio_inst_GPIO09
        .hps_0_hps_io_hps_io_gpio_inst_GPIO35  ( HPS_ENET_INT_N),           //                               .hps_io_gpio_inst_GPIO35
        .hps_0_hps_io_hps_io_gpio_inst_GPIO37  ( HPS_LCM_BK ),  //                               .hps_io_gpio_inst_GPIO37
		  .hps_0_hps_io_hps_io_gpio_inst_GPIO40  ( HPS_LTC_GPIO ),              //                               .hps_io_gpio_inst_GPIO40
        .hps_0_hps_io_hps_io_gpio_inst_GPIO41  ( HPS_LCM_D_C ),              //                               .hps_io_gpio_inst_GPIO41
        .hps_0_hps_io_hps_io_gpio_inst_GPIO44  ( HPS_LCM_RST_N  ),  //                               .hps_io_gpio_inst_GPIO44
		  .hps_0_hps_io_hps_io_gpio_inst_GPIO48  ( HPS_I2C_CONTROL),          //                               .hps_io_gpio_inst_GPIO48
        .hps_0_hps_io_hps_io_gpio_inst_GPIO53  ( HPS_LED),                  //                               .hps_io_gpio_inst_GPIO53
        .hps_0_hps_io_hps_io_gpio_inst_GPIO54  ( HPS_KEY),                  //                               .hps_io_gpio_inst_GPIO54
    	  .hps_0_hps_io_hps_io_gpio_inst_GPIO61  ( HPS_GSENSOR_INT),  //                               .hps_io_gpio_inst_GPIO61

			
		  .led_pio_external_connection_export    ( led_pio_external_connection_export ),               //                               led_pio_external_connection.export                     
        .dipsw_pio_external_connection_export  ( SW ),                 //                               dipsw_pio_external_connection.export
        .button_pio_external_connection_export ( fpga_debounced_buttons ),              //                               button_pio_external_connection.export 
		  .hps_0_h2f_reset_reset_n               ( hps_fpga_reset_n ),                //                hps_0_h2f_reset.reset_n
		  .hps_0_f2h_cold_reset_req_reset_n      (~hps_cold_reset ),      //       hps_0_f2h_cold_reset_req.reset_n
		  .hps_0_f2h_debug_reset_req_reset_n     (~hps_debug_reset ),     //      hps_0_f2h_debug_reset_req.reset_n
		  .hps_0_f2h_stm_hw_events_stm_hwevents  (stm_hw_events ),  //        hps_0_f2h_stm_hw_events.stm_hwevents
		  .hps_0_f2h_warm_reset_req_reset_n      (~hps_warm_reset ),      //       hps_0_f2h_warm_reset_req.reset_n
  
                  // conduit from menlo_slave_template
                  .menlo_slave_template_0_user_interface_address(menlo_slave_template_0_user_interface_address),
                  .menlo_slave_template_0_user_interface_read(menlo_slave_template_0_user_interface_read),
                  .menlo_slave_template_0_user_interface_write(menlo_slave_template_0_user_interface_write),
                  .menlo_slave_template_0_user_interface_readdata(menlo_slave_template_0_user_interface_readdata),
                  .menlo_slave_template_0_user_interface_writedata(menlo_slave_template_0_user_interface_writedata),
                  .menlo_slave_template_0_user_interface_byteenable(menlo_slave_template_0_user_interface_byteenable)
    );

	 
	 // Debounce logic to clean out glitches within 1ms
debounce debounce_inst (
  .clk                                  (fpga_clk_50),
  .reset_n                              (hps_fpga_reset_n),  
  .data_in                              (KEY),
  .data_out                             (fpga_debounced_buttons)
);
  defparam debounce_inst.WIDTH = 4;
  defparam debounce_inst.POLARITY = "LOW";
  defparam debounce_inst.TIMEOUT = 50000;               // at 50Mhz this is a debounce time of 1ms
  defparam debounce_inst.TIMEOUT_WIDTH = 16;            // ceil(log2(TIMEOUT))
  
// Source/Probe megawizard instance
hps_reset hps_reset_inst (
  .source_clk (fpga_clk_50),
  .source     (hps_reset_req)
);

altera_edge_detector pulse_cold_reset (
  .clk       (fpga_clk_50),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[0]),
  .pulse_out (hps_cold_reset)
);
  defparam pulse_cold_reset.PULSE_EXT = 6;
  defparam pulse_cold_reset.EDGE_TYPE = 1;
  defparam pulse_cold_reset.IGNORE_RST_WHILE_BUSY = 1;

altera_edge_detector pulse_warm_reset (
  .clk       (fpga_clk_50),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[1]),
  .pulse_out (hps_warm_reset)
);
  defparam pulse_warm_reset.PULSE_EXT = 2;
  defparam pulse_warm_reset.EDGE_TYPE = 1;
  defparam pulse_warm_reset.IGNORE_RST_WHILE_BUSY = 1;
  
altera_edge_detector pulse_debug_reset (
  .clk       (fpga_clk_50),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[2]),
  .pulse_out (hps_debug_reset)
);
  defparam pulse_debug_reset.PULSE_EXT = 32;
  defparam pulse_debug_reset.EDGE_TYPE = 1;
  defparam pulse_debug_reset.IGNORE_RST_WHILE_BUSY = 1;
  

//
// Menlo CNC Registers instance with signals passed through soc_system().
//
timing_generator_registers timing_generator_registers0(
    .clock(fpga_clk_50),
    .reset_n(hps_fpga_reset_n),
    .slave_address(menlo_slave_template_0_user_interface_address),
    .slave_read(menlo_slave_template_0_user_interface_read),
    .slave_write(menlo_slave_template_0_user_interface_write),
    .slave_readdata(menlo_slave_template_0_user_interface_readdata),
    .slave_writedata(menlo_slave_template_0_user_interface_writedata),
    .slave_byteenable(menlo_slave_template_0_user_interface_byteenable),
    .estop_in(estop_in),
    .x_timing_generator_pulse(x_timing_generator_pulse),
    .x_timing_generator_direction(x_timing_generator_direction),
    .y_timing_generator_pulse(y_timing_generator_pulse),
    .y_timing_generator_direction(y_timing_generator_direction),
    .z_timing_generator_pulse(z_timing_generator_pulse),
    .z_timing_generator_direction(z_timing_generator_direction),
    .a_timing_generator_pulse(a_timing_generator_pulse),
    .a_timing_generator_direction(a_timing_generator_direction)
    );

reg [25:0] counter; 
reg  led_level;
always @(posedge fpga_clk_50 or negedge hps_fpga_reset_n)
begin
if(~hps_fpga_reset_n)
begin
                counter<=0;
                led_level<=0;
end

else if(counter==24999999)
        begin
                counter<=0;
                led_level<=~led_level;
        end
else
                counter<=counter+1'b1;
end

assign LEDR[0]=led_level;
endmodule

  