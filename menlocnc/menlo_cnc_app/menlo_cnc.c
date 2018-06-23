
//
// Hardware Interface for Menlo CNC Controller.
//
// Copyright (c) 2017 Menlo Park Innovation LLC
//
// Designed to compile as pure C in various embedded software environments
// from embedded Linux, real time Linux, RTAI co-kernels, or soft processors
// such as Nios II on an FPGA.
//
// 05/17/2018
//

// 
// The MIT License (MIT)
// 
// Copyright (c) 2017 Menlo Park Innovation LLC
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 

//
// This is taken from LinuxCNC and applies here:
//
// https://github.com/LinuxCNC/linuxcnc/blob/master/README.md
//
// THE AUTHORS OF THIS SOFTWARE ACCEPT ABSOLUTELY NO LIABILITY FOR ANY HARM OR LOSS RESULTING FROM ITS USE.
// 
// IT IS EXTREMELY UNWISE TO RELY ON SOFTWARE ALONE FOR SAFETY.
// 
// Any machinery capable of harming persons must have provisions for completely
// removing power from all motors, etc, before persons enter any danger area.
// 
// All machinery must be designed to comply with local and national safety
// codes, and the authors of this software can not, and do not, take any
// responsibility for such compliance.
// 

//
// Keep this independent of the build environment. Pure C
// so it can go into the Linux ARM SoC, NIOS II embedded, 
// RTAI real time co-kernel, etc.
//
// So don't include any runtime specific include such as
// stdio.h, though for specific debugging its ok to map it
// to a platform specific debug output.
//

#define DBG_TRACE 1
#define DBG_TRACE2 0

#if DBG_TRACE
// Embedded Linux user mode can use printf()
#include <stdio.h>
#endif

#include "menlo_cnc.h"

int
menlo_cnc_registers_test_pass(
    PMENLO_CNC_REGISTERS registers,
    unsigned long* test_value,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
   );

unsigned long
menlo_cnc_registers_set_value_to_all_axis(
    PMENLO_CNC_REGISTERS registers,
    unsigned long set_value
    );

unsigned long
menlo_cnc_registers_validate_value_from_all_axis(
    PMENLO_CNC_REGISTERS registers,
    unsigned long compare_value,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
    );

unsigned long
menlo_cnc_registers_set_number_to_each_register(
    PMENLO_CNC_REGISTERS registers
    );

unsigned long
menlo_cnc_registers_validate_number_from_each_register(
    PMENLO_CNC_REGISTERS registers,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
    );

void
menlo_cnc_registers_noop(
    PMENLO_CNC_REGISTERS registers
    )
{
    return;
}

//
// Note: Calculation routines try and stay with 32 bit unsigned
// integers so they may be handled by a weak "soft" FPGA processor
// or simpler FPGA logic.
//
// Top edge routines use floating point to allow C programms running
// on the SoC to use higher level values.
//
// It's expected that any translation from higher level commands
// to a program to execute on the FPGA machine controller to do
// any lowering required based on the implemented capabilities.
//
// There is flexibility here to allow tradeoffs in designs as
// to whether to handle floating point, integer, and size of
// integers on either the SoC, soft/embedded processor, or FPGA
// fabric itself.
//

//
// Calculates the number of clock periods for the given
// frequency in HZ.
//
// Registers is optional and my be NULL.
//
// This routines *does not* validate whether the calculated
// clock periods fits within the machine constraints. It is
// up to the caller to do so. This allows the caller to apply
// any pre-scale factors to the result before determining if
// it fits within the hardware register range.
//
int
menlo_cnc_registers_calculate_clock_periods_by_hz(
    PMENLO_CNC_REGISTERS registers,
    double frequency_in_hz,
    double* clock_periods
    )
{
    double clock_rate;
    double clock_period;
    double period;
    double result_clock_periods;

    //
    // Zero is always zero.
    //
    if (frequency_in_hz == 0) {
      *clock_periods = 0;
      return 0;
    }

    if (frequency_in_hz < 0) {
      // Negative frequency makes no sense.
      return -1;
    }

    //
    // Most calculations are unsigned longs, since the hardware can
    // only express whole integers, and this calculation could
    // run on weak embedded or "soft" processors, or may be calculated
    // by a hardware pipeline.
    //
    // But basic frequency to period conversion would be lossy, so
    // double precision floating point is used for the calculation
    // and conversion. This could be replaced by scaled integers, or
    // expressed in hardware floating point units. Just this portion of
    // the calculation is done with floating point so it can be
    // readily substituted. Note that its not needed if the arqument
    // is the period in nano seconds.
    //

    clock_rate = (double)TIMING_GENERATOR_BASE_CLOCK_RATE;
    clock_period = (double)1 / clock_rate;

#if DBG_TRACE2
    printf("clock_rate %g, clock_period %g\n", clock_rate, clock_period);
#endif

    period = (double)1 / frequency_in_hz;

#if DBG_TRACE2
    printf("frequency_in_hz %g, period %g\n", frequency_in_hz, period);
#endif

    //
    // Scale the result in clock periods in order to not lose
    // precision within the range of 32 bit unsigned longs.
    //
    // This is done to keep the calculations close to hardware
    // which can implement these calculations.
    //

    result_clock_periods = period / clock_period;

#if DBG_TRACE2
    printf("result_clock_periods %g\n", result_clock_periods);
#endif

    *clock_periods = result_clock_periods;

    return 0;
}

//
// Returns the value for the pulse_rate register that will generate
// the specified frequency in HZ.
//
// Registers is optional and my be NULL.
//
// This routine returns an error if the requested
// range can not be specified on the target machine.
//
int
menlo_cnc_registers_calculate_pulse_rate_by_hz(
    PMENLO_CNC_REGISTERS registers,
    double frequency_in_hz,
    unsigned long* binary_pulse_rate
    )
{
    int ret;
    double clock_periods;
    double clock_scale_factor;
    double pulse_rate;

    //
    // Zero is always zero.
    //
    if (frequency_in_hz == 0) {
      *binary_pulse_rate = 0;
      return 0;
    }

    if (frequency_in_hz < 0) {
      // Negative frequency makes no sense.
      return -1;
    }

    //
    // Clock period for 50Mhz is 20ns
    //
    // Since the clock through the circuits is scaled this
    // value can exceed the range of the register so its
    // a double. Total range is validated below after any
    // scaling factors are applied.
    //
    ret = menlo_cnc_registers_calculate_clock_periods_by_hz(
        registers,
        frequency_in_hz,
        &clock_periods
        );

    if (ret != 0) {
      // Value out of range
#if DBG_TRACE
      printf("pulse_rate_by_hz overflow in clock_periods_by_hz frequency %g\n", frequency_in_hz);
#endif
      return ret;
    }

    clock_scale_factor = (double)TIMING_GENERATOR_PULSE_RATE_SCALE_FACTOR;

    //
    // Value is divided by the scale factor.
    //
    // (lower values == higher frequency)
    //
    pulse_rate = clock_periods / clock_scale_factor;

    //
    // Ensure we don't go over the range of the 32 bit
    // period counter value.
    //
    if (pulse_rate >= (double)TIMING_GENERATOR_MAXIMUM_CLOCK_PERIOD_COUNT) {
#if DBG_TRACE
      printf("pulse_rate_by_hz overflow in pulse_rate %g\n", pulse_rate);
      printf("frequency %g\n", frequency_in_hz);
      printf("clock_periods %g, clock_scale_factor %g\n", clock_periods, clock_scale_factor);
#endif
      return -1;
    }

    //
    // We could underflow. If converted to unsigned long its zero, fail.
    //
    // Note that we checked for a request of zero on entry so in this
    // case its an underflow.
    //
    if ((unsigned long)pulse_rate == 0) {
#if DBG_TRACE
      printf("pulse_rate_by_hz underflow in pulse_rate %g\n", pulse_rate);
#endif
      return -1;
    }

    //
    // This converts to unsigned long.
    // The above code has ensured the double value is
    // within the range.
    //

    *binary_pulse_rate = (unsigned long)pulse_rate;

    return 0;
}

//
// Returns the binary_pulse_count for the register.
//
// Normally pulse counts and the register value
// are 1:1, but the range needs to be checked against
// the machines capabilities. So this function validates
// that value.
//
// It's possible that some machine may return a non-1:1
// value, and that is legal, such as using software to
// adjust for any internal scaling factors.
//
// This routine returns an error if the requested
// range can not be specified on the target machine.
//
int
menlo_cnc_registers_calculate_pulse_count(
    PMENLO_CNC_REGISTERS registers,
    double pulse_count,
    unsigned long* binary_pulse_count
    )
{

  //
  // Zero is always zero.
  //
  if (pulse_count == 0) {
    *binary_pulse_count = 0;
    return 0;
  }

  if (pulse_count > TIMING_GENERATOR_MAXIMUM_PULSE_COUNT) {
    return -1;
  }

  if (pulse_count < 0) {
    // Negative pulse count makes no sense.
    return -1;
  }

  //
  // N.B. This rounds to integer as only whole pulses can be specified.
  //

  *binary_pulse_count = (unsigned long)pulse_count;

  return 0;
}


//
// Returns the value for the pulse_width register that
// will generate the specified pulse_width in nanoseconds.
//
// This routine returns an error if the requested
// range can not be specified on the target machine.
//
int
menlo_cnc_registers_calculate_pulse_width(
    PMENLO_CNC_REGISTERS registers,
    unsigned long pulse_width_in_nanoseconds,
    unsigned long* binary_pulse_width
    )
{
    double clock_period;
    double clock_scale_factor;
    double periods;

    //
    // Zero is always zero.
    //
    if (pulse_width_in_nanoseconds == 0) {
      *binary_pulse_width = 0;
      return 0;
    }

    clock_period = (double)TIMING_GENERATOR_BASE_CLOCK_PERIOD_IN_NANOSECONDS;

    clock_scale_factor = (double)TIMING_GENERATOR_PULSE_WIDTH_SCALE_FACTOR;

    periods = (double)pulse_width_in_nanoseconds / clock_period;

    //
    // Apply any scale factor
    //
    periods = periods / clock_scale_factor;

    if (periods > TIMING_GENERATOR_MAXIMUM_PULSE_WIDTH_COUNT) {
      return -1;
    }

    //
    // We could underflow. If converted to unsigned long its zero, fail.
    //
    // Note that we checked for a request of zero on entry so in this
    // case its an underflow.
    //
    if ((unsigned long)periods == 0) {
#if DBG_TRACE
      printf("calculate_pulse_width underflow in pulse_width %g\n", periods);
#endif
      return -1;
    }

    *binary_pulse_width = (unsigned long)periods;

    return 0;
}

int
menlo_cnc_registers_test(
    PMENLO_CNC_REGISTERS registers,
    unsigned long* test_value,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
    )
{
    int ret;
    unsigned long status;


    //
    // First do the basic 0, F, 5, A's tests.
    //

    *test_value = 0x00000000;

    ret = menlo_cnc_registers_test_pass(
        registers,
        test_value,
        register_in_error,
        register_in_error_value
        );

    if (ret != 0) {
        // Failed pass
        return ret;
    }

    *test_value = 0xFFFFFFFF;

    ret = menlo_cnc_registers_test_pass(
        registers,
        test_value,
        register_in_error,
        register_in_error_value
        );

    if (ret != 0) {
        // Failed pass
        return ret;
    }

    *test_value = 0x55555555;

    ret = menlo_cnc_registers_test_pass(
        registers,
        test_value,
        register_in_error,
        register_in_error_value
        );

    if (ret != 0) {
        // Failed pass
        return ret;
    }

    *test_value = 0xAAAAAAAA;

    ret = menlo_cnc_registers_test_pass(
        registers,
        test_value,
        register_in_error,
        register_in_error_value
        );

    if (ret != 0) {
        // Failed pass
        return ret;
    }

    //
    // Now set the number of each register into itself
    // so address decoder can be validated.
    //

    status = menlo_cnc_registers_set_number_to_each_register(registers);

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        *register_in_error = 0xFFFFFFFF;
        *register_in_error_value = 0;
        return 1;
    }

    status = menlo_cnc_registers_validate_number_from_each_register(
        registers,
        register_in_error,
        register_in_error_value
        );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        return 1;
    }

    return ret;
}

int
menlo_cnc_registers_test_pass(
    PMENLO_CNC_REGISTERS registers,
    unsigned long* test_value,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
    )
{
    unsigned long status;

#if DBG_TRACE
    printf("menlo_cnc_registers_test_pass entered test_value=%lx\n", *test_value);
#endif

    status = menlo_cnc_registers_initialize(registers);

    *register_in_error = -1;

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
#if DBG_TRACE
        printf("menlo_cnc_registers_test_pass initialize failure status=0x%lx\n", status);
#endif
        // failed
        return 1;
    }

#if DBG_TRACE
    printf("initialize done, setting values\n");
#endif

    status = menlo_cnc_registers_set_value_to_all_axis(
        registers,
        *test_value
    );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
#if DBG_TRACE
        printf("menlo_cnc_registers_test_pass set_value_to_all_axis failed status=0x%lx\n", status);
#endif
        // failed
        return 1;
    }

#if DBG_TRACE
    printf("settings values done, validating values\n");
#endif

    status = menlo_cnc_registers_validate_value_from_all_axis(
        registers,
        *test_value,
        register_in_error,
        register_in_error_value
        );

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
#if DBG_TRACE
        printf("menlo_cnc_registers_test_pass validate_value_from__all_axis failed status=0x%lx\n", status);
#endif
        // failed
        return 1;
    }
    else {
        return 0;
    }
}

unsigned long
menlo_cnc_registers_initialize(
    PMENLO_CNC_REGISTERS registers
    )
{
    unsigned long status;
    unsigned long value;

#if DBG_TRACE2
    printf("menlo_cnc_registers_initialize entered registers=0x%lx\n", (unsigned long)registers);
#endif

    //
    // Validate its what we are expecting.
    //

#if DBG_TRACE2
    printf("menlo_cnc_registers_initialize &registers->hexafives 0x%lx\n",
	   (unsigned long)&registers->hexafives);

    printf("menlo_cnc_registers_initialize &registers->status 0x%lx\n", 
	   (unsigned long)&registers->status);

    printf("menlo_cnc_registers_initialize &registers->command 0x%lx\n", 
	   (unsigned long)&registers->command);
#endif

    value = registers->hexafives;

    if (value != 0xA5A5A5A5) {
#if DBG_TRACE2
      printf("hexafives register not 0xA5A5A5A5, is 0x%lx\n", value);
#endif
      return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    value = registers->hexfiveas;

    if (value != 0x5A5A5A5A) {
#if DBG_TRACE2
      printf("hexfiveas register not 0x5A5A5A5A, is 0x%lx\n", value);
#endif
      return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    value = registers->interface_version;

#if DBG_TRACE2
    printf("interface_version 0x%lx\n", value);
#endif    

    //
    // Don't initialize if we don't recognize the interface version.
    //
    if (value != MENLO_CNC_REGISTERS_INTERFACE_VERSION) {
#if DBG_TRACE2
      printf("unsupported interface version %ld\n", value);
#endif
      return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    value = registers->interface_serial_number;

#if DBG_TRACE2
    printf("interface_serial_number 0x%lx\n", value);
#endif

    registers->command = 0;

#if DBG_TRACE2
    printf("menlo_cnc_registers_initialize survived first store\n");
#endif

    registers->reserved1 = 0;
    registers->reserved2 = 0;
    registers->reserved3 = 0;

    registers->x_pulse_rate = 0;
    registers->x_pulse_count = 0;
    registers->x_pulse_width = 0;
    registers->x_pulse_instruction = 0;

    registers->y_pulse_rate = 0;
    registers->y_pulse_count = 0;
    registers->y_pulse_width = 0;
    registers->y_pulse_instruction = 0;

    registers->z_pulse_rate = 0;
    registers->z_pulse_count = 0;
    registers->z_pulse_width = 0;
    registers->z_pulse_instruction = 0;

    registers->a_pulse_rate = 0;
    registers->a_pulse_count = 0;
    registers->a_pulse_width = 0;
    registers->a_pulse_instruction = 0;

    // TODO: Initialize optional axis

    status = registers->status;

    return status;
  
}

//
// Reset the FPGA timing engine.
//
// Clears all current commands.
//
// Clears the FIFO.
//
unsigned long
menlo_cnc_reset_timing_engine(
    PMENLO_CNC_REGISTERS registers
    )
{
    unsigned long status;

    registers->command = MENLO_CNC_REGISTERS_COMMAND_RST;

    registers->command = 0;

    status = registers->status;

    return status;
}

//
// Read and return the status register.
//
// This is a non-volatile operation with no self-resetting
// "sticky bits".
//
unsigned long
menlo_cnc_read_status(
    PMENLO_CNC_REGISTERS registers
    )
{
    unsigned long status;

    status = registers->status;

    return status;
}

//
// Spinwait until the FIFO can accept a new instruction block.
//
// Returns:
//
// Status value which may contain an error code or ESTOP.
//
unsigned long
menlo_cnc_wait_for_fifo_ready(
    PMENLO_CNC_REGISTERS registers
    )
{
    unsigned long status;

    status = registers->status;

    while ((status & MENLO_CNC_REGISTERS_STATUS_FBF) != 0) {
        status = registers->status;

        if (status & MENLO_CNC_REGISTERS_STATUS_EMS) {
          // ESTOP
	  return status;
	}

        if (status & MENLO_CNC_REGISTERS_STATUS_ERR) {
          // Don't hang on error
	  return status;
	}
    }

    return status;
}

//
// Load the command register.
//
// If CMD == 1 all of the axis registers are read and
// loaded into the FIFO.
//
// If EN == 1 then machine motion will immediately commence.
//
unsigned long
menlo_cnc_load_command(
    PMENLO_CNC_REGISTERS registers,
    unsigned long command
    )
{
    unsigned long status;

    registers->command = command;

    status = registers->status;

    return status;
}

unsigned long
menlo_cnc_registers_set_value_to_all_axis(
    PMENLO_CNC_REGISTERS registers,
    unsigned long set_value
    )
{
    unsigned long status;

    // Writes should be ignored for these, with read back == 0
    //registers->reserved0 = 0;
    //registers->reserved1 = 0;
    // TODO: set new reserved

    registers->x_pulse_rate = set_value;
    registers->x_pulse_count = set_value;
    registers->x_pulse_width = set_value;
    registers->x_pulse_instruction = set_value;

    registers->y_pulse_rate = set_value;
    registers->y_pulse_count = set_value;
    registers->y_pulse_width = set_value;
    registers->y_pulse_instruction = set_value;

    registers->z_pulse_rate = set_value;
    registers->z_pulse_count = set_value;
    registers->z_pulse_width = set_value;
    registers->z_pulse_instruction = set_value;

    registers->a_pulse_rate = set_value;
    registers->a_pulse_count = set_value;
    registers->a_pulse_width = set_value;
    registers->a_pulse_instruction = set_value;

    // TODO: set optional axis

    status = registers->status;

    return status;
  
}

unsigned long
menlo_cnc_registers_validate_value_from_all_axis(
    PMENLO_CNC_REGISTERS registers,
    unsigned long compare_value,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
    )
{
    unsigned long status;
    unsigned long value;
    
    *register_in_error = -1;
    *register_in_error_value = 0;

    // Reserved registers should always read as 0
    if ((value = registers->reserved1) != 0) {
#if DBG_TRACE
        printf("validate: reserved register 11 error\n");
#endif
        *register_in_error = 13;
        *register_in_error_value = value;
	return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    //
    // Without writing to the command register, axis control
    // registers should read/write the values stored.
    //
    // How they are interpreted when the command register
    // MENLO_CNC_REGISTERS_COMMAND_CMD bit is set is dependent
    // on the FPGA bitstream/program, its pin assignments, and
    // any external wiring.
    //

    if ((value = registers->x_pulse_rate) != compare_value) {
        *register_in_error = 4;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->x_pulse_count) != compare_value) {
        *register_in_error = 5;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->x_pulse_width) != compare_value) {
        *register_in_error = 6;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    //
    // Note: Instruction register only implements lower bits. Upper bits must be zero.
    //

    if ((value = registers->x_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 7;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->y_pulse_rate) != compare_value) {
        *register_in_error = 8;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->y_pulse_count) != compare_value) {
        *register_in_error = 9;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->y_pulse_width) != compare_value) {
        *register_in_error = 10;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->y_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 11;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->z_pulse_rate) != compare_value) {
        *register_in_error = 12;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->z_pulse_count) != compare_value) {
        *register_in_error = 13;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->z_pulse_width) != compare_value) {
        *register_in_error = 14;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->z_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 15;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->a_pulse_rate) != compare_value) {
        *register_in_error = 16;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->a_pulse_count) != compare_value) {
        *register_in_error = 17;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->a_pulse_width) != compare_value) {
        *register_in_error = 18;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    if ((value = registers->a_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 19;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    //
    // Status is read last in case any values or validations
    // triggered an error condition.
    //

    status = registers->status;

    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        *register_in_error = 0;
        *register_in_error_value = status;
    }

    return status;
}

//
// This sets a number to each register to ensure
// the decoder writes to, and reads from the proper register.
//
unsigned long
menlo_cnc_registers_set_number_to_each_register(
    PMENLO_CNC_REGISTERS registers
    )
{
    unsigned long status;

    registers->x_pulse_rate = 4;
    registers->x_pulse_count = 5;
    registers->x_pulse_width = 6;
    registers->x_pulse_instruction = 7;

    registers->y_pulse_rate = 8;
    registers->y_pulse_count = 9;
    registers->y_pulse_width = 10;
    registers->y_pulse_instruction = 11;

    registers->z_pulse_rate = 12;
    registers->z_pulse_count = 13;
    registers->z_pulse_width = 14;
    registers->z_pulse_instruction = 15;

    registers->a_pulse_rate = 16;
    registers->a_pulse_count = 17;
    registers->a_pulse_width = 18;
    registers->a_pulse_instruction = 19;

    status = registers->status;

    return status;
}

unsigned long
menlo_cnc_registers_validate_number_from_each_register(
    PMENLO_CNC_REGISTERS registers,
    unsigned long* register_in_error,
    unsigned long* register_in_error_value
    )
{
    unsigned long status;
    unsigned long value;
    unsigned long compare_value;


    compare_value = 4;
    if ((value = registers->x_pulse_rate) != compare_value) {
        *register_in_error = 4;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 5;
    if ((value = registers->x_pulse_count) != compare_value) {
        *register_in_error = 5;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 6;
    if ((value = registers->x_pulse_width) != compare_value) {
        *register_in_error = 6;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 7;
    if ((value = registers->x_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 7;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 8;
    if ((value = registers->y_pulse_rate) != compare_value) {
        *register_in_error = 8;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 9;
    if ((value = registers->y_pulse_count) != compare_value) {
        *register_in_error = 9;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 10;
    if ((value = registers->y_pulse_width) != compare_value) {
        *register_in_error = 10;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 11;
    if ((value = registers->y_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 11;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 12;
    if ((value = registers->z_pulse_rate) != compare_value) {
        *register_in_error = 12;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 13;
    if ((value = registers->z_pulse_count) != compare_value) {
        *register_in_error = 13;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 14;
    if ((value = registers->z_pulse_width) != compare_value) {
        *register_in_error = 14;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 15;
    if ((value = registers->z_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 15;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 16;
    if ((value = registers->a_pulse_rate) != compare_value) {
        *register_in_error = 16;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 17;
    if ((value = registers->a_pulse_count) != compare_value) {
        *register_in_error = 17;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 18;
    if ((value = registers->a_pulse_width) != compare_value) {
        *register_in_error = 18;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    compare_value = 19;

    if ((value = registers->a_pulse_instruction) != (compare_value & MENLO_CNC_REGISTERS_INSTRUCTION_MASK)) {
        *register_in_error = 19;
        *register_in_error_value = value;
        return MENLO_CNC_REGISTERS_STATUS_ERR;
    }

    status = registers->status;

    return status;
}

//
// Load new instruction/command to each axis
// register.
//
// The values in the axis registers are not transferred
// to the motion controller FIFO's until the
// CMD bit is set in the command register.
//

unsigned long
menlo_cnc_load_axis_x(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    )
{
    unsigned long status;

    registers->x_pulse_rate = pulse_rate;
    registers->x_pulse_count = pulse_count;
    registers->x_pulse_width = pulse_width;
    registers->x_pulse_instruction = instruction;

    status = registers->status;

    return status;
}

unsigned long
menlo_cnc_load_axis_y(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    )
{
    unsigned long status;

    registers->y_pulse_rate = pulse_rate;
    registers->y_pulse_count = pulse_count;
    registers->y_pulse_width = pulse_width;
    registers->y_pulse_instruction = instruction;

    status = registers->status;

    return status;
}

unsigned long
menlo_cnc_load_axis_z(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    )
{
    unsigned long status;

    registers->z_pulse_rate = pulse_rate;
    registers->z_pulse_count = pulse_count;
    registers->z_pulse_width = pulse_width;
    registers->z_pulse_instruction = instruction;

    status = registers->status;

    return status;
}

unsigned long
menlo_cnc_load_axis_a(
    PMENLO_CNC_REGISTERS registers,
    unsigned long instruction,
    unsigned long pulse_rate,
    unsigned long pulse_count,
    unsigned long pulse_width
    )
{
    unsigned long status;

    registers->a_pulse_rate = pulse_rate;
    registers->a_pulse_count = pulse_count;
    registers->a_pulse_width = pulse_width;
    registers->a_pulse_instruction = instruction;

    status = registers->status;

    return status;
}

//
// Load commands for all 4 axis.
//
// If the command specifies MENLO_CNC_REGISTERS_COMMAND_CMD
// then the instruction block is loaded into the FIFO
// which can immediately state machine motion if enable.
//
// If the FIFO is full the command spins waiting until the
// FIFO can accept the instruction block.
//
// Returns:
//
// Value of Status register on error or success.
//
unsigned long
menlo_cnc_load_four_axis(
    PMENLO_CNC_REGISTERS registers,
    unsigned long command,
    PMENLO_CNC_AXIS_OPCODE_BINARY x,
    PMENLO_CNC_AXIS_OPCODE_BINARY y,
    PMENLO_CNC_AXIS_OPCODE_BINARY z,
    PMENLO_CNC_AXIS_OPCODE_BINARY a
    )
{
    unsigned long status;

    //
    // If an error is set after loading any of the
    // axis return the status and don't load the command.
    //

    registers->x_pulse_rate = x->pulse_rate;
    registers->x_pulse_count = x->pulse_count;
    registers->x_pulse_width = x->pulse_width;
    registers->x_pulse_instruction = x->instruction;

    status = registers->status;
    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        return status;
    }

    registers->y_pulse_rate = y->pulse_rate;
    registers->y_pulse_count = y->pulse_count;
    registers->y_pulse_width = y->pulse_width;
    registers->y_pulse_instruction = y->instruction;

    status = registers->status;
    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        return status;
    }

    registers->z_pulse_rate = z->pulse_rate;
    registers->z_pulse_count = z->pulse_count;
    registers->z_pulse_width = z->pulse_width;
    registers->z_pulse_instruction = z->instruction;

    status = registers->status;
    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        return status;
    }

    registers->a_pulse_rate = a->pulse_rate;
    registers->a_pulse_count = a->pulse_count;
    registers->a_pulse_width = a->pulse_width;
    registers->a_pulse_instruction = a->instruction;

    status = registers->status;
    if ((status & MENLO_CNC_REGISTERS_STATUS_ERR) != 0) {
        return status;
    }

    //
    // Wait for the FIFO to not be busy.
    //
    status = menlo_cnc_wait_for_fifo_ready(registers);

    if ((status & MENLO_CNC_REGISTERS_STATUS_FBF) != 0) {
      // Buffer still full, must have returned due to an error, or ESTOP.
      return status;
    }

    //
    // Now load the instruction block into the FIFO.
    //
    // This could start axis motion.
    //

    registers->command = command;

    // Return the status
    status = registers->status;

    return status;
}

//
// Reset Sticky FIFO Empty.
//
// To detect FIFO underruns, reset the Sticky FIfO empty
// with this command then start issuing a stream of
// commands. This bit will be set if at any time the
// FIFO underruns to zero during the command stream.
//
// Test the Sticky FIFO Empty status bit just before issuing
// the final command and if not set there were no gaps
// in the instruction stream due to FIFO starvation.
//
// The bit will be set when the last command leaves the
// FIFO, so should be reset at the start of each run.
//
int
menlo_cnc_registers_reset_sfe(
    PMENLO_CNC_REGISTERS registers
    )
{
  // Clear it through the status clear bits register
  registers->statusclearbits = MENLO_CNC_REGISTERS_STATUS_SFE;

  return 0;
}

unsigned long
menlo_cnc_registers_get_fifo_depth(
    PMENLO_CNC_REGISTERS registers
    )
{
  return registers->current_fifo_depth;
}

int
menlo_cnc_registers_is_error(unsigned long status)
{
  if ((status & MENLO_CNC_REGISTERS_ERROR_MASK) != 0) {
    return 1;
  }

  return 0;
};

int
menlo_cnc_registers_is_underrun(unsigned long status)
{

  //
  // Sticky Fifo Empty is set if the FIFO goes empty
  // since the last reset.
  //
  if ((status & MENLO_CNC_REGISTERS_STATUS_SFE) != 0) {
    return 1;
  }

  return 0;
};
