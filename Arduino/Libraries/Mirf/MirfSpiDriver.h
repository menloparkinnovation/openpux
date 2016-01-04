
#ifndef __MIRF_SPI_DRIVER
#define __MIRF_SPI_DRIVER

extern "C" {
	#include <string.h>
	#include <inttypes.h>
}

//
// Menlo:
//
// 10/06/2014
//
// TODO: The Arduino Due now exposes an "automatic" transfer
// mode which is emulated by Intel Edison and others.
//
// Need to accomodate this here since it may be messing
// up the nRF24L01 driver for multi-byte transfers.
//
// This is because the default is to stop the SPI clock
// after each byte transfer unless told otherwise.
//
// This could be wrapped up in a multi-byte transfer
// abstraction.
//
// Lots of places in Mirf.c that depends on back to back
// byte transfers being on a consistent clock.
//

class MirfSpiDriver {
public:

    virtual uint8_t transfer(uint8_t data);

    virtual void begin();
    virtual void end();
};
#endif // __MIRF_SPI_DRIVER
