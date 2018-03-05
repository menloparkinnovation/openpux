
// MenloPark Innovation LLC 11/28/2013
#include "../OS_SoftSPI/OS_SoftSPI.h"
#include "MirfSpiDriver.h"

#ifndef __MIRF_SOFTWARE_SPI_DRIVER
#define __MIRF_SOFTWARE_SPI_DRIVER 

#include <SPI.h>

class MirfSoftwareSpiDriver : public MirfSpiDriver {

	public: 
		virtual uint8_t transfer(uint8_t data);
		virtual void begin();
		virtual void end();

                void SetSPI(OS_SoftSPIClass* spi) {
		  m_spi = spi;
		}

         private:

                OS_SoftSPIClass* m_spi;
};

// I don't like having the library declare the instance/storage
//extern MirfSoftwareSpiDriver MirfSoftwareSpi;

#endif
// MenloPark Innovation LLC 11/28/2013
