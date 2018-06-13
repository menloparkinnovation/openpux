
//
// 05/16/2018
//
// Menlo CNC app for access/testing registers of menlo_cnc_soc project.
//

/*
This program demonstrate how to use hps communicate with FPGA through light AXI Bridge.
uses should program the FPGA by GHRD project before executing the program
refer to user manual chapter 7 for details about the demo
*/


#include <stdio.h>
#include <string.h> // strcmp
#include <stdlib.h> // strtof
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#include "menlo_cnc.h"

// Include the assembler
#include "menlo_cnc_asm.h"

#define HW_REGS_BASE ( ALT_STM_OFST ) // 0xfc000000
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

void usage();

int
run_assembler_file(
    void* menlo_cnc_registers_base_address,
    char *fileName
    );

int
test_leds(
    void* ledpio_base_address
    );

int
test_menlo_cnc(
    void* menlo_cnc_registers_base_address
    );

int
test_menlo_cnc_pulse(
    void* menlo_cnc_registers_base_address,
    char *frequency
    );

int setup_hardware();

int close_hardware();

int memory_fd = -1;

// Base virtual address returned from mmap()
void* virtual_base  = NULL;

// Offset from Avalon MMIO mapping space to LW bridge.
void* lw_bridge_offset = NULL;

// Address of the Avalon LW Bridge
void* lw_bridge_base = NULL;

void* ledpio_base_address = NULL;

void* menlo_cnc_registers_base_address = NULL;

int
main(int ac, char**av)
{
        int retValue = 0;

        bool option_test_leds = false;
        bool option_test_cnc = false;
        bool option_test_cnc_pulse = false;
        bool option_run_assembler_file = false;

        // Default frequency
        char* frequency = "1";
        char* fileName = NULL;

        if (ac == 1) {
  	    usage();
            return 1;
        }

        if (strcmp("test_leds", av[1]) == 0) {
	  option_test_leds = true;
        } 
	else if (strcmp("test_cnc", av[1]) == 0) {
	  option_test_cnc = true;
	}
	else if (strcmp("test_cnc_pulse", av[1]) == 0) {
          if (ac >= 3) {
	      frequency = av[2];
          }
	  option_test_cnc_pulse = true;

          printf("test_cnc_pulse: using frequency=%s\n", frequency);
	}
	else if (strcmp("run_assembler_file", av[1]) == 0) {
          if (ac >= 3) {
	      fileName = av[2];
          }
	  option_run_assembler_file = true;

          printf("run_assembler_file %s\n", fileName);
	}
	else {
  	    printf("menlo_cnc_app [test_leds] | [test_cnc] [test_cnc_pulse] [frequency]\n");
            return 1;
	}

        if (option_test_leds) {
            retValue = test_leds(ledpio_base_address);
        }

        if (option_test_cnc) {
            retValue = test_menlo_cnc(menlo_cnc_registers_base_address);
        }

        if (option_test_cnc_pulse) {
	   retValue = test_menlo_cnc_pulse(menlo_cnc_registers_base_address, frequency);
        }

        if (option_run_assembler_file) {
	   retValue = run_assembler_file(menlo_cnc_registers_base_address, fileName);
        }

        retValue = close_hardware();

	return( retValue );
}

int
setup_hardware()
{

    //
    // Hardware specific setup is here.
    //
    // These addresses can vary based on board model, and the
    // design downloaded to the FPGA.
    //
    // Map the upper 64MB block of the physical address range into 
    // the process to access the AXI and Avalon memory mapped bridges.
    //
    // This is called the CSR span in Altera documentation/samples.
    //
    if( ( memory_fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
	    printf( "ERROR: could not open \"/dev/mem\"...\n" );
	    return( 1 );
    }

    //
    // Address mapping:
    //
    // Devices appear at the last 64MB of the 32 bit physical
    // address range which is 0xfc000000.
    //
    // The mmap() routine returns a virtual address that whose base
    // address references 0xfc000000.
    //
    // http://man7.org/linux/man-pages/man2/mmap.2.html
    //
    // #include <sys/mman.h>
    //
    // void *mmap(void *addr, size_t length, int prot, int flags,
    //           int fd, off_t offset);
    //
    // addr == virtual base address. NULL is system assigned.
    //
    // offset == offset in "file" to start mapping at.
    //
    virtual_base = 
	mmap(NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, memory_fd, HW_REGS_BASE);

    if( virtual_base == MAP_FAILED ) {
	    printf( "ERROR: mmap() failed...\n" );
	    close( memory_fd );
	    return( 1 );
    }

    printf("HW_REGS_BASE=0x%lx\n", (unsigned long)HW_REGS_BASE);
    printf("virtual_base=0x%lx\n", (unsigned long)virtual_base);

    //
    // Calculate the offset to the LW Avalon MM bridge which has the following define:
    //
    // c:\intelFPGA_pro\17.1\embedded\ip\altera\hps\altera_hps\hwlib
    // \include\soc_cv_av\socal\hps.h
    //
    // #define ALT_LWFPGASLVS_OFST        0xff200000
    //
    // This is where the lightweight AXI interface appears in the mapping space.
    //
    // Note: unsigned char* is preferred since it works for 32 + 64 bit platforms
    // but the compiler defaults forces pointers to be cast to unsigned longs.
    //
    lw_bridge_offset = (void*)
      ((unsigned long)ALT_LWFPGASLVS_OFST - (unsigned long)HW_REGS_BASE);

    printf("ALT_LWFPGASLVS_OFST=0x%lx\n", (unsigned long)ALT_LWFPGASLVS_OFST);
    printf("lw_bridge_offset=0x%lx\n", (unsigned long)lw_bridge_offset);

    //
    // Add the offset to the virtual base to get the LW bridges virtual address
    // in the mapping region.
    //
    lw_bridge_base = (void*)((unsigned long)virtual_base + (unsigned long)lw_bridge_offset);

    printf("lw_bridge_base=0x%lx\n", (unsigned long)lw_bridge_base);

    //
    // Your hardware design elements Avalon MM Slaves appear in the 64MB
    // hardware mapping region. These addresses are assigned by QSYS in the
    // designer tool, and hps_0.h is generated with the #define's
    // for your project with each named item connected to an Avalon MM Slave
    // having a base and size entry.
    //
    // Note that depending on how you connected your device, it may appear
    // as an offset to the lightweight Avalon bridge address (lw_bridge_base)
    // or it may appear as an offset to the base of the mapping region accessed
    // through virtual_base.
    //

    //
    // The following LED_PIO_BASE is generated for the projectfrom QSYS
    // assigned resource addresses so can change when the project
    // is reconfigured through QSYS such as adding, deleting, or re-generating
    // base addresses.
    //
    // Note that this is on the LW Avalon bridge.
    //
    // hps_0.h
    //
    // #define LED_PIO_BASE 0x200
    // #define LED_PIO_SPAN 16
    //
    //
    ledpio_base_address =
	(void*)((unsigned long)lw_bridge_base + (unsigned long)LED_PIO_BASE);

    printf("LED_PIO_BASE=0x%lx\n", (unsigned long)LED_PIO_BASE);

    printf("ledpio_base_address=0x%lx\n", (unsigned long)ledpio_base_address);

    menlo_cnc_registers_base_address =
      (void*)((unsigned long)lw_bridge_base + (unsigned long)MENLO_SLAVE_TEMPLATE_0_BASE);

    printf("MENLO_SLAVE_TEMPLATE_0_BASE=0x%lx\n", (unsigned long)MENLO_SLAVE_TEMPLATE_0_BASE);
    printf("menlo_cnc_registers_base_address=0x%lx\n", (unsigned long)menlo_cnc_registers_base_address);

    return 0;
}

int
close_hardware()
{

    //
    // clean up our memory mapping and exit
    //
    if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
	printf( "ERROR: munmap() failed...\n" );
	close( memory_fd );
	return( 1 );
    }

    close( memory_fd );

    return 0;
}

int
run_assembler_file(
    void* menlo_cnc_registers_base_address,
    char *fileName
    )
{
  int ret;
  PBLOCK_ARRAY binary = NULL;

  ret = assemble_file(fileName, &binary);

  if (ret != 0) {
    printf("assembler error %d %s, exiting\n", ret, strerror(ret));
    return ret;
  }

  printf("assembled %d opcode blocks\n", block_array_get_array_size(binary));
  printf("assembly success, exiting\n");

  //
  // context->compiled_binary is a pointer to the block array
  // that contains the compiled assembly instructions as machine
  // code that can be saved, fed to the registers, placed into memory
  // for DMA, etc.
  //

  //
  // Read instructions from block array.
  //
  // Consider batches.
  //
  // Validate that compiled instruction block machine
  // parameters and versions match the machine/version
  // we are talking to. Fail if no match.
  //
  // Have command line parameters, config file which specifies
  // which machine we are controlling so that wrong assembler
  // files are rejected.
  //
  // Support a binary file mode as well in which its read into
  // the block array without the source assembly path. This
  // can allow streaming from a large file on flash SD and minimizes
  // processing time.
  //
  // Implement instruction block packet/record boundary model
  // that can transport over UDP, TCP, etc. to allow streaming
  // to machines from a streaming file server.
  //

  return 0;
}

int
load_block(
    void* menlo_cnc_registers_base_address,
    void* block
    )
{
  unsigned long status;
  unsigned long command;
  MENLO_CNC_AXIS_COMMAND x;
  MENLO_CNC_AXIS_COMMAND y;
  MENLO_CNC_AXIS_COMMAND z;
  MENLO_CNC_AXIS_COMMAND a;

  bzero(&x, sizeof(x));
  bzero(&y, sizeof(y));
  bzero(&z, sizeof(z));
  bzero(&a, sizeof(a));

  // Load commands from block... TODO:

  command = 0;
  command |= (MENLO_CNC_REGISTERS_COMMAND_CMD |
              MENLO_CNC_REGISTERS_COMMAND_EAN);

  
  //
  // N.B. This spinwaits for the FIFO to not be full.
  //
  // It will return early if there is an error.
  //

  status = menlo_cnc_load_4_axis(
      menlo_cnc_registers_base_address,
      command,
      &x,
      &y,
      &z,
      &a
      );

  // TODO: general error status check should include EMS, etc.
  if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
    printf("error status %ld\n", status);
  }

  return 0;
}

int
test_leds(
    void* ledpio_base_address
    )
{
    void *h2p_lw_led_addr;
    int loop_count;
    int led_direction;
    int led_mask;

    h2p_lw_led_addr = ledpio_base_address;

    //
    // toggle the LEDs a bit
    //
    loop_count = 0;
    led_mask = 0x01;
    led_direction = 0; // 0: left to right direction

    while( loop_count < 60 ) {

	    // control led
	    *(uint32_t *)h2p_lw_led_addr = ~led_mask; 

	    // wait 100ms
	    usleep( 100*1000 );

	    // update led mask
	    if (led_direction == 0){
		    led_mask <<= 1;
		    if (led_mask == (0x01 << (LED_PIO_DATA_WIDTH-1)))
			     led_direction = 1;
	    }else{
		    led_mask >>= 1;
		    if (led_mask == 0x01){ 
			    led_direction = 0;
			    loop_count++;
		    }
	    }

    } // while

    return 0;
}

int
test_menlo_cnc(
    void* menlo_cnc_registers_base_address
    )
{
    PMENLO_CNC_REGISTERS registers;
    unsigned long test_value;
    unsigned long register_in_error;
    unsigned long register_in_error_value;
    int ret;

    printf("menlo_cnc_registers_base_address=%lx\n", (unsigned long)menlo_cnc_registers_base_address);

    registers = (PMENLO_CNC_REGISTERS)menlo_cnc_registers_base_address;

    menlo_cnc_registers_noop(registers);

    ret = menlo_cnc_registers_test(
              registers,
              &test_value,
              &register_in_error,
              &register_in_error_value
          );

    if (ret == 0) {
        printf("menlo_cnc_registers_test success\n");
    }
    else {
         printf("menlo_cnc_registers_test failure!\n");
         printf("register_in_error=%ld\n", register_in_error);
         printf("test_value=0x%lx\n", test_value);
         printf("register_in_error_value=0x%lx\n", register_in_error_value);
    }

    return ret;
}

int
test_menlo_cnc_pulse(
    void* menlo_cnc_registers_base_address,
    char *frequency_as_string
    )
{
    PMENLO_CNC_REGISTERS registers;
    unsigned long command;
    unsigned long instruction;
    unsigned long pulse_rate;
    unsigned long pulse_count;
    unsigned long pulse_width;
    unsigned long status;
    char* endptr;
    double frequency;
    double period;
    double test_width_in_nanoseconds_f;
    unsigned long test_width_in_nanoseconds;

    printf("test_menlo_cnc_pulse registers base %lx\n", (unsigned long)menlo_cnc_registers_base_address);

    registers = (PMENLO_CNC_REGISTERS)menlo_cnc_registers_base_address;

    endptr = NULL;
    frequency = strtof(frequency_as_string, &endptr);

    if (endptr == frequency_as_string) {
        printf("test_menlo_cnc_pulse: conversion error for frequency %s\n", frequency_as_string);
        return 1;
    }

    //
    // First reset the timing engine to start with a clean slate.
    //

    printf("resetting timing engine fabric...\n");

    status = menlo_cnc_reset_timing_engine(registers);

    printf("reset timing engine done\n");

    printf("status after reset 0x%lx\n", status);

    status = menlo_cnc_read_status(registers);

    printf("status on enttry 0x%lx\n", status);

    //
    // Set for generating pulses with clockwise direction.
    //
    // instruction [0:0] == 0 == clockwise (don't care, no pulse)
    // instruction [1:1] == 1 == PULSE out
    //

    //instruction = 0; // clockwise (output is 0)
    instruction = 1; // counter clockwise (output is 1)
    instruction |= MENLO_CNC_REGISTERS_INSTRUCTION_INS;

    //
    // Sherline:
    //
    // 22us minimum pulse width for Sherline drivers.
    //
    // Maximum frequency for a Sherline with a 22us clock is about
    // 35khz (verify minimum idle time) which is a 28us period.
    //
    // This gives a 6.5us low/idle time for recovery before the next pulse.
    //
    // A Sherline is 16,000 steps per inch with microstepping enabled
    // in the driver box. So this is just over 2 inches/second or
    // 120 inches/minute movement rate. This represents its maximum
    // jog rate if the physics support it.
    //

    pulse_rate = menlo_cnc_registers_calculate_pulse_rate_by_hz(registers, frequency);

    period = (double)1 / frequency;

    // Pulse width should be 1/2 period
    period = period / (double)2;

    test_width_in_nanoseconds_f = period * (double)1000000000;

    test_width_in_nanoseconds = (unsigned long)test_width_in_nanoseconds_f;

    pulse_width = menlo_cnc_registers_calculate_pulse_width(registers, test_width_in_nanoseconds);

    // pulse count for testing
    //pulse_count = 999999;  // 0xF423F

    pulse_count = 0xFFFFFFFF;

#if notdefined
    pulse_rate = 357;      // 0x165
    pulse_count = 999999;  // 0xF423F
    pulse_width = 1100;    // 0x44C

    pulse_rate = 3570000;
    pulse_count = 9999999;
    pulse_width = 1100000;
#endif

    printf("loading axis commands:\n");
    printf("pulse_rate %ld 0x%lx\n", pulse_rate, pulse_rate);
    printf("pulse_count %ld 0x%lx\n", pulse_count, pulse_count);
    printf("pulse_width %ld 0x%lx\n", pulse_width, pulse_width);
    printf("instruction 0x%lx\n", instruction);

    status = menlo_cnc_load_axis_x(
              registers,
              instruction,
              pulse_rate,
              pulse_count,
              pulse_width
              );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        printf("test_menlo_cnc_pulse: error writing x axis command\n");
    }

    status = menlo_cnc_load_axis_y(
              registers,
              instruction,
              pulse_rate,
              pulse_count,
              pulse_width
              );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        printf("test_menlo_cnc_pulse: error writing y axis command\n");
    }

    status = menlo_cnc_load_axis_z(
              registers,
              instruction,
              pulse_rate,
              pulse_count,
              pulse_width
              );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        printf("test_menlo_cnc_pulse: error writing z axis command\n");
    }

    status = menlo_cnc_load_axis_a(
              registers,
              instruction,
              pulse_rate,
              pulse_count,
              pulse_width
              );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        printf("test_menlo_cnc_pulse: error writing a axis command\n");
    }

    //
    // Now write the command register to insert it into the
    // FIFO and begin machine motion.
    //

    command = 0;
    command |= (MENLO_CNC_REGISTERS_COMMAND_CMD |
                MENLO_CNC_REGISTERS_COMMAND_EAN);

    status = menlo_cnc_load_command(
              registers,
              command
	      );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        printf("test_menlo_cnc_pulse: error writing command register\n");
    }

    command = 0;
    command |= MENLO_CNC_REGISTERS_COMMAND_EAN;

    //
    // Toggle CMD back to zero
    //

    status = menlo_cnc_load_command(
              registers,
              command
	      );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        printf("test_menlo_cnc_pulse: error writing command register\n");
    }

    printf("test_menlo_cnc_pulse: axis commands loaded successfully\n");

    status = menlo_cnc_read_status(registers);

    printf("status on exit 0x%lx\n", status);

    return 0;
}

void
usage()
{
  printf("menlo_cnc_app:\n");
  printf("    run_assembler_file file_name\n");
  printf("    test_cnc_pulse [frequency]\n");
  printf("    test_leds\n");
  printf("    test_cnc\n");
  exit(1);
}
