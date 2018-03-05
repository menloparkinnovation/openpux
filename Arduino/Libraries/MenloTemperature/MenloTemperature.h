
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

#ifndef MenloTemperature_h
#define MenloTemperature_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#include <inttypes.h>
#endif

class MenloTemperature {

public:
  
    MenloTemperature();

    virtual float GetTemperature();

private:
};

#endif // MenloTemperature_h
