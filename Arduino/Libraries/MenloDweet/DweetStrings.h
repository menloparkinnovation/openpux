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
 *  Date: 02/22/2015
 *  File: DweetStrings.h
 *
 *  Common Dweet Strings.
 */

#ifndef DweetStrings_h
#define DweetStrings_h

#include <MenloPlatform.h>

//
// Common strings are declared once and referenced. Otherwise they end up
// being defined multiple times in the output image taking up valuable space
// on 32k AtMega 328's.
//

// Normal reply to a command
extern const char dweet_reply_string[];

// Error in command
extern const char dweet_error_string[];

// Unsupported command
extern const char dweet_unsup_string[];

// GETSTATE
extern const char dweet_getstate_string[];

// SETSTATE
extern const char dweet_setstate_string[];

// GETCONFIG
extern const char dweet_getconfig_string[];

// SETCONFIG
extern const char dweet_setconfig_string[];

//
// Generic values
//

// ON
extern const char dweet_on_string[];

// OFF
extern const char dweet_off_string[];

#endif // DweetStrings_h
