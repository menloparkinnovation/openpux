#include "MirfSoftwareSpiDriver.h"
uint8_t MirfSoftwareSpiDriver::transfer(uint8_t data){
	return m_spi->transfer(data);
}

void MirfSoftwareSpiDriver::begin(){
	m_spi->begin();
	m_spi->setDataMode(SPI_MODE0);
#if MENLO_ATMEGA328
	m_spi->setClockDivider(SPI_2XCLOCK_MASK);
#endif
}

void MirfSoftwareSpiDriver::end(){
}

// I don't like having the library declare the instance/storage
//MirfSoftwareSpiDriver MirfSoftwareSpi;
