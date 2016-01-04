
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

/*
 *  Date: 05/09/2015
 *
 *  Platform support for x86 32 bit Processors.
 *
 */

#ifndef MenloPlatformX86_h
#define MenloPlatformX86_h

#if MENLO_X86

// Code space strings "F(x)" macro is missing in the x86 port
#define F(string) string

#endif

#endif // MenloPlatformX86_h
