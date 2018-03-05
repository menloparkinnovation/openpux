
/*
 * Copyright (C) 2015 Menlo Park Innovation LLC
 *
 * This is licensed software, all rights as to the software
 * is reserved by Menlo Park Innovation LLC.
 *
 * A license included with the distribution provides certain limited
 * rights to a given distribution of the work.
 *
 * This distribution includes a copy of the license agreement and must be
 * provided along with any further distribution or copy thereof.
 *
 * If this license is missing, or you wish to license under different
 * terms please contact:
 *
 * menloparkinnovation.com
 * menloparkinnovation@gmail.com
 */

#ifndef MenloMemoryMonitor_h
#define MenloMemoryMonitor_h

#include <inttypes.h>

// hardware\tools\avr\avr\include\stdlib.h for size_t
#include <stdlib.h>

//
// Memory Monitor Support for Arduino runtime.
//

//
// The memory monitor class allows tracking of dynamic
// heap and stack usage in order to determine if corruptions
// are occuring as a result of overflows.
//
// The Arduino environment by default does not track this
// and results in many tricky errors, esp. when normal
// print(), println() strings are added in an attempt
// to find out what is going on.
//

// Your application may do better with a different value
const uint8_t CanaryValue = 0x54;

class MenloMemoryMonitor {
 public:

  //
  // Init takes the callers proposed maximum expected
  // heap and stack sizes, and how much of a guard
  // region is desired.
  //
  // It validates it against the actual available memory
  // on the processor, and the programs static memory
  // consumption determined by the variables exported by
  // the linker for .data and .bss size. If there is
  // an overflow right at init, an error message
  // is reported and execution stops. This is better than
  // getting "random" failures and corruptions while
  // the applicaton is in service.
  //
  // If there is enough memory available to meet the
  // proposed limits, the stack and heap canary's
  // are initialized.
  //
  // If DetailedTracking is set, then the guard region,
  // the unused heap, and unused stack regions are
  // filled with canary values to allow runtime
  // determination of actual stack and heap usage for
  // program profiling.
  //
  static void Init(
          size_t MaxHeapSize,
          size_t MaxStackSize,
          size_t GuardRegionSize,
          bool   DetailedTracking
      );

  //
  // This is called to see if memory limits have
  // been exceeded.
  //
  // It reports an error and halts execution if so.
  //
  // The errorValue is reported, and allows an application
  // to provide unique values for different memory checks
  // such as after a suspect library function or operation.
  //
  // A useful way to call this is as:
  //
  // MenloMemoryMonitor::CheckMemory(__LINE__);
  //
  static void CheckMemory(int errorValue);

  //
  // Report current memory usage.
  //
  // Program state is a value that is included in the
  // report to aid in reporting where in the program a particular
  // memory usage report was invoked.
  //
  // MenloMemoryMonitor::ReportMemoryUsage(__LINE__);
  //
  static void ReportMemoryUsage(int programState);

  static void ReportAllMemoryValues();

private:

  static void ReportMemoryOverflow(int overflowType, int value);
};

#endif // MenloMemoryMonitor_h
