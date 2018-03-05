
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

//
// Memory Monitor Support for Arduino runtime.
//
// 08/27/2013
//

#if defined(ARDUINO) && ARDUINO >= 100
// Main Arduino C++ include header for all projects
//#include "Arduino.h"
#if defined(MENLO_ATMEGA)
#include <MemoryFree.h>
#endif
#endif

#include "MenloMemoryMonitor.h"

#include <MenloDebug.h>

//
// Detailed memory tracing for debugging.
// Consumes more code space than minimal tracing, reporting.
//

// Show a memory range report for debugging a new platform
#if ARDUINO_AVR_PROMICRO8
#define MEMORY_RANGE_REPORT 0
//#define MEMORY_RANGE_REPORT 1
#endif

#if ARDUINO_AVR_PROMICRO8
// Report details on a canary smash failure
#define MEMORY_CANARY_REPORT 0
//#define MEMORY_CANARY_REPORT 1
#endif

// 03/01/2015
// Defining this uses 1832 bytes of code space
//#define MEMORY_MONITOR_FULL 1

#if MEMORY_MONITOR_FULL
#define DBG_PRINT_ENABLED 1
#endif

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define DBG_PRINT_NNL(x)  (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x) (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#define DBG_PRINT2(x, y) (MenloDebug::PrintNoNewline(F(x)) && MenloDebug::PrintHex((int)y))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#define DBG_PRINT2(x, y)
#endif

//
// By default this is always enabled to give
// diagnostics of a memory failure.
//
// This can be turned off for production to minimize
// code space, while the checking code is still in
// place and operating and will stop processing if a runtime
// overflow occurs.
//
// It is expected that a watchdog timer will eventually
// reset the system on an overflow.
//
// Currently enabling consumes 500 bytes of code space
// on the AtMega 328 with Arduino 1.0.1 compilers.
//

#if MEMORY_MONITOR_FULL
#define xDBG_PRINT_ENABLED 1
#endif

//
// Additional reporting can be turned on when debugging
// a memory overwrite situation after an overflow error
// has occurred.
//
// Normally they are off to save code space.
//

#if MEMORY_RANGE_REPORT
#define xDBG_PRINT_ENABLED 1

void MemoryRangeReport();

#endif

//
// Enabled detailed canary reporting
//
#if MEMORY_CANARY_REPORT
void CanarySmashReport(unsigned char* ptr);
#endif

#if xDBG_PRINT_ENABLED
#define xDBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define xDBG_PRINT_NNL(x) (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x) (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#define xDBG_PRINT2(x, y) (MenloDebug::PrintNoNewline(F(x)) && MenloDebug::PrintHex((int)y))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT2(x, y)
#endif

#if defined(MENLO_ATMEGA)

//
// This information is from avr-lib online.
//
// It's helpful to use avr-objdump -h -S -l project.elf
// to generate an asm listing with memory layout for your
// project. See online for the locations of avr-objdump
// in your Arduino distribution, and where it places
// your project.elf file after a successful compilation.
//

//
// Data memory address layout on AtMega328p:
//
// ----------------
// | Registers    | 0x0 - 0x1f
// ----------------
// |   Ports      | 0x20 - 0x5f
// ----------------
// | Ext Ports    | 0x60 - 0xff
// ----------------
// |   .data      | 0x100 - .data_end
// ----------------
// |   .bss       | .data_end - .bss_end
// ----------------
// |   heap       | .bss_end - ??
// |    |         |
// |    \/        | dynamic heap area grows up
// ----------------
// |              |
// | free space   | free space shared by heap + stack
// |              |
// ----------------
// |     ^        | dynamic stack area grows down
// |     |        |
// |   stack      | 0x8ff (2048 bytes + 0x100 for register and ports)
// ----------------
//
// This is the address layout implemented by this
// class which enforces sizes on heap and stack
// at runtime.
//
// This class uses "canary's" which are data values
// placed into memory and if overwritten indicates
// an overflow has occurred. Not perfect, but with
// good canary value selection it can be pretty
// reliable.
//
// In this model, the application declares the maximum amount
// of stack and heap it will use at initialization and
// this is validated against available memory from the
// linker defined variables for initial .data and .bss.
//
// Canary's are written at these boundaries to allow
// runtime detection of overflow.
//
// The guard region is configured by the application
// to assist in detecting deep overflows when calling
// into library functions, servicing interrupts, etc.
// It's design is to allow determination of actual
// heap and stack usage from runtime testing of the
// embedded application. This space can be zero, in
// which case detection of overflow may be compromised
// by corruption of execution variables.
//
// ----------------
// | Registers    | 0x0 - 0x1f
// ----------------
// |   Ports      | 0x20 - 0x5f
// ----------------
// | Ext Ports    | 0x60 - 0xff
// ----------------
// |   .data      | 0x100 - .data_end
// ----------------
// |   .bss       | .data_end - .bss_end
// ----------------
// |   heap       | .bss_end - ??
// |    |         |
// |    \/        | dynamic heap area grows up
// ----------------
// | heap canary  | heap growth canary
// ----------------
// |              |
// | guard region | configured guard region
// |              |
// ----------------
// | stack canary | stack growth canary
// ----------------
// |     ^        | dynamic stack area grows down
// |     |        |
// |   stack      | 0x8ff (2048 bytes + 0x100 for register and ports)
// ----------------
//

//
// These are external symbols produced by the compiler/assembler/linker
// in the AVR environment. They are setup by the early Crt1* file
// specific to the selected processor chip.
//

extern unsigned int __data_start; // start of initialized data
extern unsigned int __data_end;   // end of initialized data
extern unsigned int __bss_start;
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern unsigned int __heap_end;

//
// These are runtime library support variables for
// the C style memory allocator.
//

extern void *__brkval;
extern char *__malloc_heap_start;
extern char *__malloc_heap_end;
extern size_t __malloc_margin;

//
// SP is a built in access to the memory mapped location
// for the stack pointer in common.h
//

//
// RAMEND is defined in the Arduino/AVR environment for
// the selected processor.
//
// For the 328 its 0x8ff
//

//
// These are the calculated limits supported by this class
//

// static allocation 5 bytes.
unsigned char* _stack_limit;
unsigned char* _heap_limit;
bool _detailed_tracking;

void MenloMemoryMonitor::Init(
    size_t MaxHeapSize,
    size_t MaxStackSize,
    size_t GuardRegionSize,
    bool   DetailedTracking
    )
{
  int16_t size;
  int16_t ramSize;
  int16_t dataSize;
  int16_t bssSize;
  int16_t totalDataSize;
  int16_t currentHeapSize;
  int16_t heapTestSize;
  int16_t heapAllocationOverhead;
  char* ptr;

  // Set a heap allocation overhead for two pointers
  heapAllocationOverhead = sizeof(char*) * 2;

  // Dynamically size ram based on our current processor
  ramSize = (int)RAMEND - (int)&__data_start;

  // Total data is data + bss
  dataSize = (int)&__data_end - (int)&__data_start;
  bssSize = (int)&__bss_end - (int)&__bss_start;

  totalDataSize = dataSize + bssSize;

  // Catch any build time/link time error right away
  if (totalDataSize > ramSize) {
    ReportMemoryOverflow(OverflowTypeConfiguration, __LINE__);
    return;
  }

  //
  // We force a guard region of 2 bytes always for a canary
  // just at the end of the heap, and just below the stack
  //
  size = 2;

  // Calculate all of our proposed memory commitments
  size += totalDataSize;
  size += MaxHeapSize;
  size += MaxStackSize;

  //
  // GuardRegionSize is the application supplied extra guard
  // region in addition to the minimum two canaries we always
  // allocate.
  //
  size += GuardRegionSize;

  //
  // Static configuration error, the program requires more
  // memory than the current processor allows.
  //
  if (size > ramSize) {
    ReportMemoryOverflow(OverflowTypeConfiguration, size);
    return;
  }

  // Calculate the stack and heap limits and place the canary's
  _stack_limit = (unsigned char*)(RAMEND - MaxStackSize);
  _heap_limit = (unsigned char*)((int)&__data_start + totalDataSize + MaxHeapSize);
  _detailed_tracking = DetailedTracking;

  // Offset to the canary's which are just beyond in the guard region
  *(_stack_limit - 1) = CanaryValue;
  *(_heap_limit + 1) = CanaryValue;

  //
  // This initializes the heap variables if the heap has
  // not been used until entry. The AVR LIB C heap does not set
  // some of its variables until the first allocation.
  //
  ptr = (char*)malloc(10);
  if (ptr != NULL) {
      free(ptr);
  }

  //
  // Show an initial memory report for development
  //
  // Detailed memory tracking places canaries in the entire
  // unused guard region in addition to the working stack
  // space.
  //
  // The canaries in the working stack space allow dynamic
  // determination of the maximum stack depth encountered
  // in the program, and is useful for debugging stack
  // overflows when calling into system libraries, dealing
  // with interrupts, etc.
  //
  if (DetailedTracking) {
  
      //
      // If the DBG_PRINT, xDBG_PRINT has been turned off to save space
      // we rely on a terse summary.
      //
      MenloDebug::PrintNoNewline(F("MEM datasize "));
      MenloDebug::PrintHexNoNewline(dataSize);
      MenloDebug::PrintNoNewline(F(" bss_size "));
      MenloDebug::PrintHex(bssSize);

#if MEMORY_RANGE_REPORT

      //
      // Report memory ranges
      //
      MemoryRangeReport();
#endif

      // these tend to get lost in the serial monitor, so highlight
      // and repeat.
      DBG_PRINT("+++++");

      DBG_PRINT_NNL(".data+.bss size: 0x");
      DBG_PRINT_INT(totalDataSize);

      DBG_PRINT_NNL(".data size: 0x");
      DBG_PRINT_INT(dataSize);

      DBG_PRINT_NNL(".bss size: 0x");
      DBG_PRINT_INT(bssSize);
      DBG_PRINT("+++++");

      DBG_PRINT("+++++");

      DBG_PRINT_NNL(".data+.bss size: 0x");
      DBG_PRINT_INT(totalDataSize);

      DBG_PRINT_NNL(".data size: 0x");
      DBG_PRINT_INT(dataSize);

      DBG_PRINT_NNL(".bss size: 0x");
      DBG_PRINT_INT(bssSize);
      DBG_PRINT("+++++");

      DBG_PRINT_NNL("Total Memory Commitment 0x");

      DBG_PRINT_INT_NNL(size);

      DBG_PRINT_NNL(" out of 0x");
      DBG_PRINT_INT(ramSize);

      // Report how much is left over
      dataSize = ramSize - size;
      DBG_PRINT_NNL("Unused Memory 0x");
      DBG_PRINT_INT(dataSize);
      DBG_PRINT("");

      //MenloDebug::PrintNoNewline(F("Unused Memory 0x"));
      //MenloDebug::PrintHex(dataSize);

      // Place canary's throughout the free stack space
      size = ((int)SP - (int)_stack_limit) - 1;
      memset(_stack_limit, CanaryValue, size);

      // Place canary's on the free heap space
      if (__brkval != 0) {
	ptr = (char*)__brkval;
      }
      else {
        // This is what __brkval is set to in 1.6.4 avr-libc
        // We could do a malloc()/free to set __brkval
	ptr = __malloc_heap_start;
      }

      size = (int)_heap_limit - (int)ptr;

      memset(ptr, CanaryValue, size);

      // Place canary's in the guard region
      ptr = (char*)(_heap_limit + 1);
      size = (int)_stack_limit - (int)ptr;
      
      memset(ptr, CanaryValue, size);

      //MenloDebug::Print(F("Canaries set"));

      ReportAllMemoryValues();
  }

  MenloMemoryMonitor::CheckMemory(LineNumberBaseMemoryMonitor + __LINE__);

  //
  // Allocate dynamic memory specified by the caller
  // and release it. This is to test mis-configuration
  // that would overflow upfront.
  //
  // Note: The callers supplied HeapSize is the total
  // heap size of the application plus system support
  // libraries. Since we have no way to know their
  // overhead we read the current heap size and subtract
  // from the test allocation below. Otherwise we hit
  // our own canaries right away.
  //
  currentHeapSize = (int)__brkval - (int)&__heap_start;

  heapTestSize = MaxHeapSize - currentHeapSize;

  // Account for heap allocation overhead
  heapTestSize -= heapAllocationOverhead;

  ptr = (char*)malloc(heapTestSize);
  if (ptr == NULL) {
      ReportMemoryOverflow(OverflowTypeConfiguration, __LINE__);
  }

  memset(ptr, 0, heapTestSize);

  MenloMemoryMonitor::CheckMemory(LineNumberBaseMemoryMonitor + __LINE__);

  free(ptr);

  MenloMemoryMonitor::CheckMemory(LineNumberBaseMemoryMonitor + __LINE__);
}

void MenloMemoryMonitor::CheckMemory(int errorValue)
{
  unsigned char* ptr;

  // If a canary has been hit, we have real problems...
  if (*(_stack_limit - 1) != CanaryValue) {
    ReportMemoryOverflow(OverflowTypeStackCanary, errorValue);
  }

  if (*(_heap_limit + 1) != CanaryValue) {
    ReportMemoryOverflow(OverflowTypeHeapCanary, errorValue);
  }

  //
  // Check runtime consumption against configured limits
  //
  if ((int)SP < (int)_stack_limit) {
    ReportMemoryOverflow(OverflowTypeStackOverflow, errorValue);
  }

  if (__brkval != 0) {
    if (__brkval > _heap_limit) {
        ReportMemoryOverflow(OverflowTypeHeapOverflow, errorValue);
    }
  }

  if (_detailed_tracking) {

    //  Check the guard region's canary's
    for (ptr = (unsigned char*)(_heap_limit + 1); (int)ptr < (int)(_stack_limit-1); ptr++) {
      if (*ptr != CanaryValue) {

#if ARDUINO_AVR_PROMICRO8
          //
          // Total hack:
          // Something asynchronously writes a zero at 0x800, 0x0801 sometime
          // after startup on the Arduino Pro Micro. Need to debug its USB driver.
          //
          if ((ptr == (unsigned char*)0x800) || (ptr == (unsigned char*)0x801)) {
              continue;
          }
#endif

#if MEMORY_CANARY_REPORT
        CanarySmashReport(ptr);
#endif
        // Guard region canary indicated overflow
        ReportMemoryOverflow(OverflowTypeGuardRegion, errorValue);
      }
    }
  }
}

void MenloMemoryMonitor::ReportMemoryUsage(int programState)
{
  int16_t heapSize;
  int16_t stackSize;
  unsigned char* ptr;

  //
  // This reports the current heap and stack usage and useful
  // if called within a method that could be deeply nested.
  //
  stackSize = (int)RAMEND - (int)SP;

  if (__brkval == 0) {
      heapSize = 0;
  }
  else {
      heapSize = (int)__brkval - (int)&__heap_start;
  }

  xDBG_PRINT2("Current HeapSize ", heapSize);

  xDBG_PRINT2("Current StackSize ", stackSize);

  if (_detailed_tracking) {

    //
    // Check for the maximum depth the stack has
    // had by looking from the stack limit up until
    // the canary value is no longer present.
    //
    // This will show the furthest point the stack
    // has ever grown.
    //
    for (ptr = _stack_limit; (int)ptr < (int)SP; ptr++) {
      if (*ptr != CanaryValue) {

	stackSize = (int)SP - (int)ptr;
        xDBG_PRINT2("Maximum Stack Used ", stackSize);
	break;
      }
    }

    //
    // We don't have to search the canary's for the heap
    // since __brkval sets the maximum heap and is never
    // shrunk with the current (1.6.4) version of malloc
    // in avr-libc.
    //
  }
}

void MenloMemoryMonitor::ReportAllMemoryValues()
{
#if DBG_PRINT_ENABLED

  //
  // This is a useful debugging/profiling tool.
  //
  // It's rather large, so only enabled for detailed tracing.
  //

  int16_t size;
  int16_t ramSize;
  int16_t dataSize;

  int16_t heapSize;
  int16_t stackSize;

  stackSize = (int)RAMEND - (int)SP;

  if (__brkval == 0) {
      heapSize = 0;
  }
  else {
      heapSize = (int)__brkval - (int)&__heap_start;
  }

  // Dynamically size ram based on our current processor
  ramSize = (int)RAMEND - (int)&__data_start;

  MenloDebug::PrintNoNewline(F("Ram range: &__data_start 0x"));
  MenloDebug::PrintNoNewline((int)&__data_start);
  MenloDebug::PrintNoNewline(F(" RAMEND: 0x"));
  MenloDebug::PrintNoNewline(RAMEND);
  MenloDebug::PrintNoNewline(F(" ramSize 0x"));
  MenloDebug::Print(ramSize);

  // Print out our limits/canary addresses
  MenloDebug::PrintNoNewline(F("(stack upper limit)RAMEND "));
  MenloDebug::PrintNoNewline((int)RAMEND);
  MenloDebug::PrintNoNewline(F(" (lower stack limit)_stack_limit "));
  MenloDebug::Print((int)_stack_limit);

  MenloDebug::PrintNoNewline(F("current_SP: 0x"));
  MenloDebug::PrintNoNewline(SP);
  MenloDebug::PrintNoNewline(F(" stack allocated: stackSize 0x"));
  MenloDebug::Print(stackSize);

  MenloDebug::PrintNoNewline(F("&__heap_start 0x"));
  MenloDebug::PrintNoNewline((int)&__heap_start);
  MenloDebug::PrintNoNewline(F(" _heap_limit 0x"));
  MenloDebug::Print((int)_heap_limit);

  MenloDebug::PrintNoNewline(F("current heap_end: __brkval 0x"));
  MenloDebug::PrintNoNewline((int)__brkval);
  MenloDebug::PrintNoNewline(F(" Current HeapSize: 0x"));
  MenloDebug::Print(heapSize);

  // .data
  size   = (int)&__data_end - (int)&__data_start;
  MenloDebug::PrintNoNewline(F("(.data)&__data_start 0x"));
  MenloDebug::PrintNoNewline((int)&__data_start);
  MenloDebug::PrintNoNewline(F(" &__data_end 0x"));
  MenloDebug::PrintNoNewline((int)&__data_end);
  MenloDebug::PrintNoNewline(F(" dataSize 0x"));
  MenloDebug::Print(size);

  // .bss
  size = (int)&__bss_end - (int)&__bss_start;
  MenloDebug::PrintNoNewline(F("(.bss)&__bss_start 0x"));
  MenloDebug::PrintNoNewline((int)&__bss_start);
  MenloDebug::PrintNoNewline(F(" &__bss_end 0x"));
  MenloDebug::PrintNoNewline((int)&__bss_end);
  MenloDebug::PrintNoNewline(F(" bssSize 0x"));
  MenloDebug::Print(size);

  // Total data is data + bss
  dataSize = (int)&__data_end - (int)&__data_start;
  size = (int)&__bss_end - (int)&__bss_start;
  dataSize += size;

  MenloDebug::PrintNoNewline(F("(.data+.bss)totalDataSize 0x"));
  MenloDebug::Print(dataSize);

  // __brkval is 0 when no malloc()'s have occurred yet
  MenloDebug::PrintNoNewline(F("__brkval 0x"));
  MenloDebug::Print((int)__brkval);

  MenloDebug::PrintNoNewline(F("&__heap_start 0x"));
  MenloDebug::PrintNoNewline((int)(&__heap_start));
  MenloDebug::PrintNoNewline(F(" &__heap_end 0x"));
  MenloDebug::Print((int)(&__heap_end));

  MenloDebug::PrintNoNewline(F("__heap_end 0x"));
  MenloDebug::PrintNoNewline((int)(__heap_end));
  MenloDebug::PrintNoNewline(F(" __malloc_heap_start 0x"));
  MenloDebug::Print((int)(__malloc_heap_start));

  MenloDebug::PrintNoNewline(F("__malloc_margin 0x"));
  MenloDebug::Print((int)(__malloc_margin));
#endif
}

#if MEMORY_RANGE_REPORT
void
MemoryRangeReport()
{
    char tmp;

    //
    // Report memory ranges
    //

    // Data range allocated by the program is from __data_start to __bss_end
    MenloDebug::PrintNoNewline(F("__data_start "));
    MenloDebug::PrintHex((int)&__data_start);

    MenloDebug::PrintNoNewline(F("__data_end "));
    MenloDebug::PrintHex((int)&__data_end);

    MenloDebug::PrintNoNewline(F("__bss_start "));
    MenloDebug::PrintHex((int)&__bss_start);

    MenloDebug::PrintNoNewline(F("__bss_end "));
    MenloDebug::PrintHex((int)&__bss_end);

    MenloDebug::PrintNoNewline(F("__heap_end "));
    MenloDebug::PrintHex((int)&__heap_end);

    MenloDebug::PrintNoNewline(F("__brkval "));
    MenloDebug::PrintHex((int)__brkval);

    // RAMEND is the environments definition for the maximum RAM address
    MenloDebug::PrintNoNewline(F("RAMEND "));
    MenloDebug::PrintHex(RAMEND);

    //
    // Show current stack pointer
    // Default on AVR's is to place top of stack at RAMEND - 2
    //
    MenloDebug::PrintNoNewline(F("stackptr "));
    MenloDebug::PrintHex((int)&tmp); // address of any local stack variable
}
#endif

#if MEMORY_CANARY_REPORT

void
CanarySmashDisplay(unsigned char* ptr)
{

    MenloDebug::PrintNoNewline(F("guard canary smashed ptr "));
    MenloDebug::PrintHexNoNewline((int)ptr);
    MenloDebug::PrintNoNewline(F(" IS  "));
    MenloDebug::PrintHexNoNewline(*ptr);
    MenloDebug::PrintNoNewline(F(" SB  "));
    MenloDebug::Print(CanaryValue);

    MenloDebug::PrintNoNewline(F("_heap_limit "));
    MenloDebug::Print((int)_heap_limit);

    MenloDebug::PrintNoNewline(F("_stack_limit "));
    MenloDebug::Print((int)_stack_limit);
}

void
CanarySmashReport(unsigned char* ptr)
{
    CanarySmashDisplay(ptr);

    MenloDebug::Print(F("RANGE"));

    for (; (int)ptr < (int)(_stack_limit-1); ptr++) {

        if (*ptr != CanaryValue) {
            CanarySmashDisplay(ptr);
        }
    }

    MenloDebug::Print(F("RANGE DONE"));
}
#endif

// MENLO_ATMEGA
#else
#if defined(ESP8266)

//
// ESP8266 Support
//

//
// http://www.esp8266.com/wiki/doku.php?id=esp8266_memory_map
//

// https://github.com/esp8266/Arduino/issues/81
extern "C" {
// hardware/esp8266/2.2.0/tools/sdk/include/user_interface.h
// void system_print_meminfo(void)
// uint32 system_get_free_heap_size(void)
#include "user_interface.h"
}

//
// The ESP8266 does not use the standard C allocator, but its
// own custom one umm_malloc.
//
// umm_malloc has its own internal support for heap canary's
// to detect stack overflow if the POISON option is set.
//
//   - Need to dig into it, may just be for overrun, re-use
//     after free errors.
//
// Routine to check memory blocks:
//
// UMM_HEAP_INFO ummHeapInfo;
// umm_info(void *ptr, int force);
//

#define ESP8266_RAM_START 0x3FFE8000
#define ESP8266_RAM_SIZE  0x14000
#define ESP8266_RAM_END   (ESP8266_RAM_START + ESP8266_RAM_SIZE)

//
// Stack range is from a stackdump after an exception.
//
// It is below the beginning of the heap.
//
// Confirm with the linker map file.
//
#define ESP8266_STACK_START 0x3FFF1900
#define ESP8266_STACK_END   0x3FFF1CD0

//
// Standard linker variable names are not available on the esp8266.
//
//extern unsigned int __data_start; // start of initialized data
//extern unsigned int __data_end;   // end of initialized data
//extern unsigned int __bss_start;
//extern unsigned int __bss_end;
//extern unsigned int __heap_start;
//extern unsigned int __heap_end;

//
// Standard C heap variables are not available on esp8266 as it
// uses a custom umm_malloc library.
//
//extern void *__brkval;
//extern char *__malloc_heap_start;
//extern char *__malloc_heap_end;
//extern size_t __malloc_margin;

//
// Variables from umm_malloc.c
//

extern void* umm_heap; // actually umm_block*

extern char _heap_start;
extern short int umm_numblocks;

// From cores/esp8266/umm_malloc/umm_malloc_cfg.h
size_t heap_end = (size_t)(0x3fffc000);

//
// These are the calculated limits supported by this class
//

// A value of 0 means unlimited (no checking)
size_t maxStackSize = 0;
size_t maxHeapSize = 0;
size_t guardRegionSize = 0;

//
// The ESP8266 heap has its own region which will fail
// an allocation when exceeded. So guard region between
// it and the stack is not required.
//
// GuardRegion is used as a minimum memory free region
// for both heap and stack on memory checks during runtime.
//

void MenloMemoryMonitor::Init(
    size_t MaxHeapSize,
    size_t MaxStackSize,
    size_t GuardRegionSize,
    bool   DetailedTracking
    )
{
    void* stackPtr;
    void* heapPtr = NULL;

    //
    // Save the configuration parameters.
    //
    maxStackSize = MaxStackSize;
    maxHeapSize = MaxHeapSize;
    guardRegionSize = GuardRegionSize;

    //
    // Allocate dynamic memory specified by the caller
    // and release it. This is to test mis-configuration
    // that would overflow upfront.
    //
    if (maxHeapSize != 0) {
        heapPtr = (char*)malloc(maxHeapSize);
        if (heapPtr == NULL) {
            ReportMemoryOverflow(OverflowTypeConfiguration, __LINE__);
        }
    }

    if (maxStackSize != 0) {
        stackPtr = (char*)alloca(maxStackSize);
        if (stackPtr == NULL) {
            ReportMemoryOverflow(OverflowTypeConfiguration, __LINE__);
        }
    }

    //
    // Run the memory check to verify the configuration limits
    // can be met.
    //

    CheckMemory(__LINE__);

    if (heapPtr != NULL) {
        free(heapPtr);
    }

    // stackPtr does not need free since its alloca()

    return;
}

void MenloMemoryMonitor::CheckMemory(int errorValue)
{
    // Trick to get current SP
    int currentStackSize = 0;
    int SP = (int)&currentStackSize;

    currentStackSize = ESP8266_STACK_END - SP;

    //
    // Check for stack overflow
    //
    if ((SP - ESP8266_STACK_START) <= guardRegionSize) {
        ReportMemoryOverflow(OverflowTypeStackOverflow, errorValue);
    }

    uint32_t freeMemory = system_get_free_heap_size();

    //
    // Check for heap overflow
    //
    if (freeMemory <= guardRegionSize) {
        ReportMemoryOverflow(OverflowTypeHeapOverflow, errorValue);
    }

    return;
}

void MenloMemoryMonitor::ReportMemoryUsage(int programState)
{
    ReportAllMemoryValues();
}

void MenloMemoryMonitor::ReportAllMemoryValues()
{
    int heapStart = (int)&_heap_start;
    int ummHeap = (int)umm_heap;
    int ummNumBlocks = (int)umm_numblocks;

    uint32_t freeMemory = system_get_free_heap_size();

    MenloDebug::PrintNoNewline(F("Heap Free 0x"));

    // Shows 0x9800 (38,912 bytes)
    MenloDebug::PrintHex32(freeMemory);

    system_print_meminfo();

    // Trick to get current SP
    int currentStackSize = 0;
    int SP = (int)&currentStackSize;

    MenloDebug::PrintNoNewline(F("SP 0x"));

    //
    // Shows 0x3FFF1A90
    // The stack is 760 bytes below the bottom of the heap
    //
    // On exception says:
    //
    // sp: 3fff1900 end: 3fff1cd0 offset: 01a0
    //   0x3d0 range (976 bytes)
    //
    // stack dump is from:
    //
    // 3fff1aa0
    // 3fff1cc0
    //
    MenloDebug::PrintHexPtr((void*)SP);

    currentStackSize = (int)ESP8266_RAM_END - SP;

    MenloDebug::PrintNoNewline(F("Current Stack Size 0x"));
    MenloDebug::PrintHex32(currentStackSize);

    MenloDebug::PrintNoNewline(F("umm_numblocks 0x"));

    // Shows 0x144F
    MenloDebug::PrintHex32(ummNumBlocks);

    MenloDebug::PrintNoNewline(F("umm_heap 0x"));

    // Shows 0x3FFF1D88
    MenloDebug::PrintHexPtr((void*)ummHeap);

    MenloDebug::PrintNoNewline(F("_heap_start 0x"));

    // Shows 0x3FFF1D88
    MenloDebug::PrintHexPtr((void*)heapStart);

    MenloDebug::PrintNoNewline(F("_heap_end 0x"));

    // Shows 0x3FFFC000
    MenloDebug::PrintHexPtr((void*)heap_end);
}

// ESP8266
#else

//
// NON-AVR Platforms
//

//
// MenloMemoryMonitor Stub
//
// John Richardson 07/03/2014
// MenloParkInnovation LLC
// Copyright (C) Menlo Park Innovation LLC
//

//
// This stub implements a correct action "NULL" memory monitor
// and is useful for porting to new platforms quickly.
//
// The memory monitor routines can be filled in for a specific
// platform as needs arise. On some platforms such as RaspberryPI
// and Intel Galileo you would not even bother as these are
// running "real" operating systems with MMU's and full
// process and debugger support which can be leveraged to
// debug stack and/or heap overflows, if they even occur.
//

void MenloMemoryMonitor::Init(
    size_t MaxHeapSize,
    size_t MaxStackSize,
    size_t GuardRegionSize,
    bool   DetailedTracking
    )
{
}

void MenloMemoryMonitor::CheckMemory(int errorValue)
{
}

void MenloMemoryMonitor::ReportMemoryUsage(int programState)
{
}

void MenloMemoryMonitor::ReportAllMemoryValues()
{
}
#endif
#endif

//
// These routines are common and not implementation specific.
//

//
// overflowType == OverflowTypexxx
// value == __LINE__
//
void MenloMemoryMonitor::ReportMemoryOverflow(int overflowType, int value)
{
  MenloDebug::PrintNoNewline(F("+MO+ Type "));
  MenloDebug::PrintHexNoNewline(overflowType);
  MenloDebug::PrintNoNewline(F("L "));
  MenloDebug::PrintHex(value);

#if MEMORY_RANGE_REPORT
  MemoryRangeReport();
#endif

  xDBG_PRINT("Details: OverflowType ");

  switch (overflowType) {

  case OverflowTypeConfiguration:
    xDBG_PRINT("Configuration");
    break;

  case OverflowTypeStackCanary:
    xDBG_PRINT("StackCanary");
    break;

  case OverflowTypeStackOverflow:
    xDBG_PRINT("StackOverflow");
    break;

  case OverflowTypeHeapOverflow:
    xDBG_PRINT("HeapOverflow");
    break;

  case OverflowTypeGuardRegion:
    xDBG_PRINT("GuardRegion");
    break;

  default:
    xDBG_PRINT2("+++!!!MemoryOverflow Type ", overflowType);
  break;
  }

  xDBG_PRINT2(" DebugValue(Program Line Number) ", value);

  // Attempt to get a detailed usage report out
  ReportMemoryUsage(__LINE__);
  ReportAllMemoryValues();

#if MEMORY_RANGE_REPORT
  for (;;) {
      MemoryRangeReport();
      delay(1000);
  }
#endif

  //
  // Wait for watchdog reset as our state is too seriously corrupt
  // to be able to attempt recovery.
  //
  MenloDebug::Panic(overflowType);
}
