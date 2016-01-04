
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

#include "MenloMemoryMonitor.h"

#include <MenloDebug.h>

//
// Detailed memory tracing for debugging.
// Consumes more code space than minimal tracing, reporting.
//
#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (Serial.println(F(x)))
#define DBG_PRINT_INT(x) (Serial.println(x))
#define DBG_PRINT2(x, y) (Serial.print(F(x)) && Serial.println(y))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_INT(x)
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
// reset the system on an overflow as the code just
// goes into an error reporting loop and remains.
//
// Typical implementations will flash the overflow code
// on an onboard LED till reset occurs.
//
#define xDBG_PRINT_ENABLED 1

#if xDBG_PRINT_ENABLED
#define xDBG_PRINT(x)     (Serial.println(F(x)))
#define xDBG_PRINT_INT(x) (Serial.println(x))
#define xDBG_PRINT2(x, y) (Serial.print(F(x)) && Serial.println(y))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT2(x, y)
#endif

//
// These are external symbols produced by the compiler/assembler/linker.
// They are setup by the early Crt1* file specific to the selected
// processor chip.
//

//
// Note: These are commented out for MemoryMonitorStub, but
// are actually pretty generic to the GCC environment used on
// just about every (at least open source) microcontroller
// platform.
//

//extern unsigned int __data_start;
//extern unsigned int __data_end;
//extern unsigned int __bss_start;
//extern unsigned int __bss_end;
//extern unsigned int __heap_start;
//extern unsigned int __heap_end;

//
// These are runtime library support variables for
// the C style memory allocator.
//

//extern void *__brkval;
//extern char *__malloc_heap_start;
//extern char *__malloc_heap_end;
//extern size_t __malloc_margin;

//
// These are the calculated limits supported by this class
//
//unsigned char* _stack_limit;
//unsigned char* _heap_limit;
//bool _detailed_tracking;

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

void MenloMemoryMonitor::ReportMemoryOverflow(int overflowType, int value)
{
}

void MenloMemoryMonitor::ReportAllMemoryValues()
{
}
