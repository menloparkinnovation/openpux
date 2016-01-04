
#include "MirfHardwareSpiDriver.h"

uint8_t
MirfHardwareSpiDriver::transfer(uint8_t data)
{
    return SPI.transfer(data);
}

void
MirfHardwareSpiDriver::begin()
{
    SPI.begin();

    //
    // SPI.h
    //
    // SPISettings() default constructor:
    //
    //                    clock   bitOrder  dataMode
    // init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
    //
    // SPIClass
    //
    // MSBFIRST
    // SPI.setBitOrder(MSBFIRST);
    //

    SPI.setDataMode(SPI_MODE0);

#if ARDUINO_AVR_MEGA2560

    //
    // Note: SPI.h says that beginTransaction() should be used
    // for all SPI settings now.
    //
    // http://www.arduino.cc/en/Reference/SPISetClockDivider
    //

    //SPI.setBitOrder(MSBFIRST);

    //SPI.setClockDivider(SPI_CLOCK_DIV4);
    //SPI.setClockDivider(SPI_CLOCK_DIV64); // Try slow until wiring is validated
    //SPI.setClockDivider(SPI_CLOCK_DIV128); // Try slow until wiring is validated
#endif
}

void
MirfHardwareSpiDriver::end()
{
}

// Menlo: Caller determines allocation
// MirfHardwareSpiDriver MirfHardwareSpi;
