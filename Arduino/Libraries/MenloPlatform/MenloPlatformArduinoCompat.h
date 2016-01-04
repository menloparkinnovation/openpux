
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
 *  Platform support for Arduino Compatability
 *
 */

#ifndef MenloPlatformArduinoCompat_h
#define MenloPlatformArduinoCompat_h

#ifdef ARDUINO_COMPAT_REQUIRED

//
// This header provides basic Arduino style function
// replacement for platforms that don't supply an Arduino
// style library.
//
// Note: If this gets large enough place it into its own
// MenloArduinoCompat.h
//

//
// Arduino.h includes headers we need for basic types
// Since we don't have it, we supply the includes ourselves.
//
#include <stdint.h>

// Code space strings
class __FlashStringHelper;

// Note: The new cast style is required for SparkCore GCC compiler
#define PSTR(x) (x)
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))
//#define F(string) string

extern void delay( uint32_t dwMs );

#endif

#endif // MenloPlatformArduinoCompat_h
