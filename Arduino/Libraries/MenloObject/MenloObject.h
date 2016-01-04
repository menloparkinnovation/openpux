
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
// MenloObject.h
// 03/26/2014
//

#ifndef MenloObject_h
#define MenloObject_h

#include "MenloPlatform.h"

class MenloObject;

//
// This provides the base class for MenloFramework.
//
class MenloObject {

public:
  
    MenloObject();
    ~MenloObject();

    virtual int Initialize();

 private:

};

#endif // MenloObject_h
