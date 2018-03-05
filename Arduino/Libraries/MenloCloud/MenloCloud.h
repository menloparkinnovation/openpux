
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
 *  Date: 11/30/2013
 *  File: MenloCloud.h
 *
 *  MenloCloud provides an abstract class interface for various
 *  cloud/internet servers.
 */

#ifndef MenloCloud_h
#define MenloCloud_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#include "MenloPlatform.h"
#include "MenloSensorProtocol.h"

class MenloCloud {

public:
  
    MenloCloud();

    virtual bool Process(MenloSensorProtocolDataAppResponseBuffer* data, int* valid) = 0;

    virtual bool QueueSensorReadings(MenloSensorProtocolDataAppBuffer* data) = 0;

    virtual bool IsBufferBusy() = 0;

    virtual bool CancelBuffer() = 0;

private:
};

#endif // MenloCloud_h
