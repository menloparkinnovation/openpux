
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
 *  Date: 10/12/2014
 *  File: MenloTemperature.h
 *
 *  MenloTemperature provides an abstract class interface for various
 *  temperature sensors.
 *
 */

//
// Include Menlo Debug library support
//
#include <MenloDebug.h>

// This libraries header
#include <MenloTemperature.h>

// Constructor
MenloTemperature::MenloTemperature() {
}

// Default implementation
float
MenloTemperature::GetTemperature()
{
      return 0;
}
