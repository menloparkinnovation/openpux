
#include "MirfSpiDriver.h"

#ifndef __MIRF_HARDWARE_SPI_DRIVER
#define __MIRF_HARDWARE_SPI_DRIVER 

#include <SPI.h>

class MirfHardwareSpiDriver : public MirfSpiDriver {

public: 
    virtual uint8_t transfer(uint8_t data);
    virtual void begin();
    virtual void end();
};

// MenloPark Innovation LLC 11/28/2013
// I don't like this, caller determines allocation
//extern MirfHardwareSpiDriver MirfHardwareSpi;
// MenloPark Innovation LLC 11/28/2013

#endif // __MIRF_HARDWARE_SPI_DRIVER 
