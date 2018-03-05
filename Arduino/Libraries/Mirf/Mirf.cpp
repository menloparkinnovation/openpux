/**
 * Mirf
 *
 * Additional bug fixes and improvements
 *  11/03/2011:
 *   Switched spi library.
 *  07/13/2010:
 *   Added example to read a register
 *  11/12/2009:
 *   Fix dataReady() to work correctly
 *   Renamed keywords to keywords.txt ( for IDE ) and updated keyword list
 *   Fixed client example code to timeout after one second and try again
 *    when no response received from server
 * By: Nathan Isburgh <nathan@mrroot.net>
 * $Id: mirf.cpp 67 2010-07-13 13:25:53Z nisburgh $
 *
 *
 * An Ardunio port of:
 * http://www.tinkerer.eu/AVRLib/nRF24L01
 *
 * Significant changes to remove depencence on interupts and auto ack support.
 *
 * Aaron Shrimpton <aaronds@gmail.com>
 *
 */

/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id: mirf.cpp 67 2010-07-13 13:25:53Z nisburgh $
*/

// MenloPark Innovation LLC 10/02/2014
#include <MenloPlatform.h>
#include <MenloDebug.h>

// MenloPark Innovation LLC 10/02/2014
#define MENLO_TEST_DETAILS 1

//
// This is useful for debugging new hardware configurations
// in which timing problems may be an issue.
//
// #define MENLO_ADD_DELAYS 1

#include "Mirf.h"
// Defines for setting the MiRF registers for transmitting or receiving mode

Nrf24l Mirf = Nrf24l();

Nrf24l::Nrf24l(){
	cePin = 8;
	csnPin = 7;
	channel = 1;
	payload = 16;
	spi = NULL;
}

void Nrf24l::transferSync(uint8_t *dataout,uint8_t *datain,uint8_t len){
	uint8_t i;
	for(i = 0;i < len;i++){
		datain[i] = spi->transfer(dataout[i]);
	}
}


void Nrf24l::transmitSync(uint8_t *dataout,uint8_t len){
	uint8_t i;
	for(i = 0;i < len;i++){
		spi->transfer(dataout[i]);
	}
}


void Nrf24l::init() 
// Initializes pins to communicate with the MiRF module
// Should be called in the early initializing phase at startup.
{   
    pinMode(cePin,OUTPUT);
    pinMode(csnPin,OUTPUT);

    ceLow();
    csnHi();

    // Initialize spi module
    spi->begin();
}

//
// Sets the important registers in the MiRF module and powers the module
// in receiving mode
// NB: channel and payload must be set now.
//
void Nrf24l::config() 
{
    // Set RF channel
        configRegister(RF_CH,channel);

    // Set length of incoming payload 
	configRegister(RX_PW_P0, payload);
	configRegister(RX_PW_P1, payload);

    // Start receiver 
    powerUpRx();
    flushRx();
}

//
// This sets Receive address P1 which is the primary receiver address.
//
// P0 is the auto-ack receive address.
//
// Sets the receiving address
void Nrf24l::setRADDR(uint8_t * adr) 
{
        //
        // Menlo note: This sets P1 to the receive address which is 5
        // digits. According to the Nordic data sheet the rest of
        // the receivers from P2 - P5 *must* have the same (4) upper
        // bytes of the address.
        //
        // Question: Is the low address byte sent first, or upper? Look
        // at data sheet for order of SPI byte shift in of address string.
        //

	ceLow();
	writeRegister(RX_ADDR_P1, adr, mirf_ADDR_LEN);
	ceHi();
}

//
// Sets the transmitting address
//
void Nrf24l::setTADDR(uint8_t * adr)
{
	/*
	 * RX_ADDR_P0 must be set to the sending addr for auto ack to work.
	 */

	writeRegister(RX_ADDR_P0,adr,mirf_ADDR_LEN);

	writeRegister(TX_ADDR,adr,mirf_ADDR_LEN);
}

//
// Checks if data is available for reading
//
extern bool Nrf24l::dataReady() 
{
    // See note in getData() function - just checking RX_DR isn't good enough
    uint8_t status = getStatus();

    // We can short circuit on RX_DR, but if it's not set, we still need
    // to check the FIFO for any pending packets
    if ( status & (1 << RX_DR) ) {
        return 1;
    }

    return !rxFifoEmpty();
}

extern bool Nrf24l::rxFifoEmpty(){
	uint8_t fifoStatus;

	readRegister(FIFO_STATUS,&fifoStatus,sizeof(fifoStatus));
	return (fifoStatus & (1 << RX_EMPTY));
}

// Reads payload bytes into data array
extern void Nrf24l::getData(uint8_t * data) 
{
    csnLow();                               // Pull down chip select
    spi->transfer( R_RX_PAYLOAD );            // Send cmd to read rx payload
    transferSync(data,data,payload); // Read payload
    csnHi();                               // Pull up chip select
    // NVI: per product spec, p 67, note c:
    //  "The RX_DR IRQ is asserted by a new packet arrival event. The procedure
    //  for handling this interrupt should be: 1) read payload through SPI,
    //  2) clear RX_DR IRQ, 3) read FIFO_STATUS to check if there are more 
    //  payloads available in RX FIFO, 4) if there are more data in RX FIFO,
    //  repeat from step 1)."
    // So if we're going to clear RX_DR here, we need to check the RX FIFO
    // in the dataReady() function
    configRegister(STATUS,(1<<RX_DR));   // Reset status register
}

//
// configRegister() is a generic one byte write to a register
//
// Clocks only one byte into the given MiRF register
//
void Nrf24l::configRegister(uint8_t reg, uint8_t value)
{
    csnLow();
    spi->transfer(W_REGISTER | (REGISTER_MASK & reg));
    spi->transfer(value);
    csnHi();
}

void Nrf24l::readRegister(uint8_t reg, uint8_t * value, uint8_t len)
// Reads an array of bytes from the given start position in the MiRF registers.
{
    csnLow();
    spi->transfer(R_REGISTER | (REGISTER_MASK & reg));
    transferSync(value,value,len);
    csnHi();
}

void Nrf24l::writeRegister(uint8_t reg, uint8_t * value, uint8_t len) 
// Writes an array of bytes into inte the MiRF registers.
{
    csnLow();
    spi->transfer(W_REGISTER | (REGISTER_MASK & reg));
    transmitSync(value,len);
    csnHi();
}


void Nrf24l::send(uint8_t * value) 
// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
{
    uint8_t status;
    status = getStatus();

    while (PTX) {
	    status = getStatus();

	    if((status & ((1 << TX_DS)  | (1 << MAX_RT)))){
		    PTX = 0;
		    break;
	    }
    }                  // Wait until last paket is send

    ceLow();
    
    powerUpTx();       // Set to transmitter mode , Power up
    
    csnLow();                    // Pull down chip select
    spi->transfer( FLUSH_TX );     // Write cmd to flush tx fifo
    csnHi();                    // Pull up chip select
    
    csnLow();                    // Pull down chip select
    spi->transfer( W_TX_PAYLOAD ); // Write cmd to write payload
    transmitSync(value,payload);   // Write payload
    csnHi();                    // Pull up chip select

    ceHi();                     // Start transmission
}

/**
 * isSending.
 *
 * Test if chip is still sending.
 * When sending has finished return chip to listening.
 *
 */

bool Nrf24l::isSending(){
	uint8_t status;
	if(PTX){
		status = getStatus();
	    	
		/*
		 *  if sending successful (TX_DS) or max retries exceded (MAX_RT).
		 */

		if((status & ((1 << TX_DS)  | (1 << MAX_RT)))){
			powerUpRx();
			return false; 
		}

		return true;
	}
	return false;
}

uint8_t Nrf24l::getStatus(){
	uint8_t rv;
	readRegister(STATUS,&rv,1);
	return rv;
}

void Nrf24l::powerUpRx(){
	PTX = 0;
	ceLow();
	configRegister(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (1<<PRIM_RX) ) );
	ceHi();
	configRegister(STATUS,(1 << TX_DS) | (1 << MAX_RT)); 
#if MENLO_ADD_DELAYS
        // MENLO: Todo: Remove if needed!
        delay(10);
#endif
}

void Nrf24l::flushRx(){
    csnLow();
    spi->transfer( FLUSH_RX );
    csnHi();
}

void Nrf24l::powerUpTx(){
	PTX = 1;
	configRegister(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (0<<PRIM_RX) ) );
}

void Nrf24l::ceHi(){
	digitalWrite(cePin,HIGH);
#if MENLO_ADD_DELAYS
        delay(10);
#endif
}

void Nrf24l::ceLow(){
	digitalWrite(cePin,LOW);
#if MENLO_ADD_DELAYS
        delay(10);
#endif
}

void Nrf24l::csnHi(){
	digitalWrite(csnPin,HIGH);
#if MENLO_ADD_DELAYS
        delay(10);
#endif
}

void Nrf24l::csnLow(){
	digitalWrite(csnPin,LOW);
#if MENLO_ADD_DELAYS
        delay(10);
#endif
}

void Nrf24l::powerDown(){
	ceLow();
	configRegister(CONFIG, mirf_CONFIG );
}

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
 *
 * License:
 *
 * The following code is copyright Menlo Park Innovation LLC
 * and is a "code snapshot". The code text as follows here is incorporated
 * into this work (Mirf.cpp by Nathan Isburgh, Aaron Shrimpton) and is
 * a contribution by Menlo Park Innovation LLC to be used following the
 * existing license terms of this source code module Mirf.cpp.
 *
 * The following source code text "snapshot" is also included in
 * other works by Menlo Park Innovation LLC under other licenses and
 * inclusion here provides no rights to those licenses in any way.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 */

//
// Menlo:
//
// MenloPark Innovation LLC 11/28/2013
//
// Detect presence of an nRF24L01 on the configured
// SPI interface.
//
// This serves as a useful wiring validation
//
// If loop == TRUE, it will stay in the detection/test loop
// on failure, which is useful for hooking up a logic
// analyzer, scope, etc, to the SPI lines.
//
bool
Nrf24l::IsPresent(bool loop)
{
  bool result;
  uint8_t index;
  uint8_t buffer[mirf_ADDR_LEN];

  //
  // NOP loop for testing with a logic analyzer.
  //
  // Set trigger on CSN falling edge
  //
  //  while(1) {
  // data contains the status register
  //
  // Read the status register with the NOP instruction
  // Most basic test loop
  //    csnLow();
  //    data = spi->transfer(NOP);
  //    csnHi();
  //}

  //
  // This is here for hardware debugging to watch
  // on a logic analyzer.
  //
  // After reset the values should be 0xE7E7E7E7E7
  // according to the data sheet on pdf page 56.
  //
  // This will issue on the SPI bus:
  //
  // MOSI 0xA  R_REGISTER (0x0) | (reg) 
  // MOSI 0x0 MISO 0xE7
  // MOSI 0x1 MISO 0xE7
  //    ...
  //
  // readRegister(RX_ADDR_P0, buffer, mirf_ADDR_LEN);

TestLoop:

  result = true;

  //
  // Note: Since the MCU/microcontroller can't directly
  // reset the nRF24L01 chip on most boards, we can not
  // rely on cold reset initialization values in the chip
  // since an MCU watchdog restart, or program download
  // reset does not reset the nRF24L01.
  //

  //
  // Set our own values in two patterns with opposite bits,
  // so its very unlikely to be a stuck bit.
  // In addition rapid transitions would test clocking
  // and settling times on longer/higher capacitance lines.
  //

  for (index = 0; index < mirf_ADDR_LEN; index++) {
    buffer[index] = 0xAA;
  }

  writeRegister(RX_ADDR_P0, buffer, mirf_ADDR_LEN);

  for (index = 0; index < mirf_ADDR_LEN; index++) {
    buffer[index] = 0;
  }

  readRegister(RX_ADDR_P0, buffer, mirf_ADDR_LEN);

  for (index = 0; index < mirf_ADDR_LEN; index++) {
    if (buffer[index] != 0xAA){
#if MENLO_TEST_DETAILS
        MenloDebug::PrintNoNewline(F("rtst RXPO SB 0xAA fail index="));
        MenloDebug::PrintHex(index);
        MenloDebug::PrintNoNewline(F(" data="));
        MenloDebug::PrintHex(buffer[index]);
        MenloDebug::Print("\n");
#endif
        result = false;
        goto Cleanup;
    }
  }

  for (index = 0; index < mirf_ADDR_LEN; index++) {
    buffer[index] = 0x55;
  }

  writeRegister(RX_ADDR_P0, buffer, mirf_ADDR_LEN);

  for (index = 0; index < mirf_ADDR_LEN; index++) {
    buffer[index] = 0;
  }

  readRegister(RX_ADDR_P0, buffer, mirf_ADDR_LEN);

  for (index = 0; index < mirf_ADDR_LEN; index++) {
    if (buffer[index] != 0x55){
#if MENLO_TEST_DETAILS
        MenloDebug::PrintNoNewline(F("MIRF: Read Test RX_ADDR_PO SB 0x55 failure index="));
        MenloDebug::PrintHex(index);
        MenloDebug::PrintNoNewline(F(" data="));
        MenloDebug::PrintHex(buffer[index]);
        MenloDebug::Print("\n");
#endif
        result = false;
        goto Cleanup;
    }
  }

  // Set back the reset address value
  for (index = 0; index < mirf_ADDR_LEN; index++) {
    buffer[index] = 0xE7;
  }

  writeRegister(RX_ADDR_P0, buffer, mirf_ADDR_LEN);

 Cleanup:

  if (!result) {
      MenloDebug::Print(F("nRF24L01 --- NOT PRESENT ---"));
  }
  else {
      MenloDebug::Print(F("nRF24L01 --- PRESENT ---"));
  }
  
  //
  // This allows the caller to specify that it stays in the test
  // loop to trigger a logic/SPI analyzer on the signals to validate
  // what is occuring.
  //
  if (!result && loop) goto TestLoop;

  return result;
}

//
// Dump registers for debugging
//
void
Nrf24l::DumpRegisters()
{
#if DUMP_REGISTERS_ENABLED
    uint8_t reg;
    uint8_t index;
    uint8_t buffer[mirf_ADDR_LEN];

    //
    // Note: Debug values are from the LightHouse.ino in Gateway mode.
    //
    // They are the dumped values for working on UNO
    //  

    MenloDebug::Print(F("nRF24L01 Registers"));

    // readRegister(reg, value*, len);

    //
    // STATUS
    //
    // 0x0E -> 0000 1110
    //   TX not full
    //   RX_P_1 - RX_P_2, RX_P3 == 1 (rx fifo empty)
    //
    // #define RX_DR       6
    // #define TX_DS       5
    // #define MAX_RT      4
    // #define RX_P_NO     1
    // #define TX_FULL     0
    //
    readRegister(STATUS, &reg, 1);
    MenloDebug::PrintNoNewline(F("STATUS "));
    MenloDebug::PrintHex(reg);

    //
    // FIFO_STATUS
    //
    // 0x01 -> 0000 0001
    // RX_EMPTY
    //
    readRegister(FIFO_STATUS, &reg, 1);
    MenloDebug::PrintNoNewline(F("FIFO_STATUS "));
    MenloDebug::PrintHex(reg);

    //
    // EN_AA - Enable auto ack virtual register 0x01
    //
    // 0x3F
    //
    // 0011 1111
    //
    // bit 5:0 - enable auto ack for pipe 0 - 5
    // bit 6 - reserved
    // bit 7 - reserved
    //
    readRegister(EN_AA, &reg, 1);
    MenloDebug::PrintNoNewline(F("EN_AA "));
    MenloDebug::PrintHex(reg);

    readRegister(EN_RXADDR, &reg, 1);
    MenloDebug::PrintNoNewline(F("EN_RXADDR "));
    MenloDebug::PrintHex(reg);

    //
    // Channel
    // 0x01
    //
    readRegister(RF_CH, &reg, 1);
    MenloDebug::PrintNoNewline(F("RF_CH "));
    MenloDebug::PrintHex(reg);

    readRegister(RF_SETUP, &reg, 1);
    MenloDebug::PrintNoNewline(F("RF_SETUP "));
    MenloDebug::PrintHex(reg);

    //
    // CONFIG register contains configuration bits:
    //
    // configRegister(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (1<<PRIM_RX) ) );
    // EN_CRC CRC0 PWR_UP PRIM_RX
    //
    // 0x0B ->  0000 1011
    // EN_CRC | PWR_UP | PRIM_RX
    //
    readRegister(CONFIG, &reg, 1);
    MenloDebug::PrintNoNewline(F("CONFIG "));
    MenloDebug::PrintHex(reg);

    // 0x00 no dynamic payloads enabled
    readRegister(DYNPD, &reg, 1);
    MenloDebug::PrintNoNewline(F("DYNPD "));
    MenloDebug::PrintHex(reg);

    // Packet Width (Length)
    // 0x20 == 32 bytes for P0
    readRegister(RX_PW_P0, &reg, 1);
    MenloDebug::PrintNoNewline(F("RX_PW_P0 "));
    MenloDebug::PrintHex(reg);

    // Packet Width (Length)
    // 0x20 == 32 bytes for P1
    readRegister(RX_PW_P1, &reg, 1);
    MenloDebug::PrintNoNewline(F("RX_PW_P1 "));
    MenloDebug::PrintHex(reg);

    //
    // Get and dump receive address for P0
    //
    // byte 0: 73 's'
    // byte 1: 65 'e'
    // byte 3: 6E 'n'
    // byte 4: 73 's'
    // byte 5: 30 '0'
    //
    readRegister(RX_ADDR_P0, buffer, mirf_ADDR_LEN);

    MenloDebug::Print(F("RX_ADDR_P0 "));
    for (index = 0; index < mirf_ADDR_LEN; index++) {
        MenloDebug::PrintHex(buffer[index]);
    }  

    //
    // Get and dump receive address for P1
    //
    // byte 0: 67 'g'
    // byte 1: 61 'a'
    // byte 3: 74 't'
    // byte 4: 65 'e'
    // byte 5: 30 '0'
    //
    readRegister(RX_ADDR_P1, buffer, mirf_ADDR_LEN);

    MenloDebug::Print(F("RX_ADDR_P1 "));
    for (index = 0; index < mirf_ADDR_LEN; index++) {
        MenloDebug::PrintHex(buffer[index]);
    }  

    //
    // Get and dump transmit address
    //
    // byte 0: 73 's'
    // byte 1: 65 'e'
    // byte 3: 6E 'n'
    // byte 4: 73 's'
    // byte 5: 30 '0'
    //
    readRegister(TX_ADDR, buffer, mirf_ADDR_LEN);

    MenloDebug::Print(F("TXADDR "));
    for (index = 0; index < mirf_ADDR_LEN; index++) {
        MenloDebug::PrintHex(buffer[index]);
    }  

    //
    // Dump additional registers that are defaulted
    //

    //
    // EN_RXADDR - Virtual register 0x02
    //
    // page 54 of nRF24L01Plus datasheet
    //
    // 0x03
    // ERX_P1 | ERX_P0 - Data pipe 1 and data pipe 0 enabled
    //
    readRegister(EN_RXADDR, &reg, 1);
    MenloDebug::PrintNoNewline(F("EN_RXADDR "));
    MenloDebug::PrintHex(reg);

    //
    // SETUP_AW - Virtual register 0x03
    //
    // page 55 of nRF24L01Plus datasheet
    //
    // 0x03 -> 5 byte address length
    //
    readRegister(SETUP_AW, &reg, 1);
    MenloDebug::PrintNoNewline(F("SETUP_AW "));
    MenloDebug::PrintHex(reg);

    //
    // RF_SETUP - Virtual register 0x06
    //
    // page 55 of nRF24L01Plus datasheet
    //
    // 0x0F -> 0000 1111
    //
    // bit 0   - obsolete, don't care
    // bit 2:1 - Tx power, 11 == 0 dbm, or full power
    // bit 3   - RF_DR_HIGH 2mbps since bit 5 == 0
    // bit 4   - == 0, PLL_LOCK (test mode) is off
    // bit 5   - RF_DR_LOW == 0, use bit 3 to indicate rate
    // bit 6   - == 0, reserved
    // bit 7   - == 0, CONT_WAVE carrier is off
    //
    readRegister(RF_SETUP, &reg, 1);
    MenloDebug::PrintNoNewline(F("RF_SETUP "));
    MenloDebug::PrintHex(reg);

    //
    // SETUP_RETR - Virtual register 0x04
    //
    //
    // 0x03
    //
    // 3 auto retransmits
    //
    // bits 7:4 - auto retransmit delay 250us
    //
    readRegister(SETUP_RETR, &reg, 1);
    MenloDebug::PrintNoNewline(F("SETUP_RETR "));
    MenloDebug::PrintHex(reg);

    // 0x13
    //
    // 0001 0011
    //
    // 3:0 - count of retransmit packets == 3
    //
    // 7:4 - count of lost packets == 1
    //
    readRegister(OBSERVE_TX, &reg, 1);
    MenloDebug::PrintNoNewline(F("OBSERVE_TX "));
    MenloDebug::PrintHex(reg);

    // 0x00
    readRegister(CD, &reg, 1);
    MenloDebug::PrintNoNewline(F("CD "));
    MenloDebug::PrintHex(reg);

    // 0x00
    readRegister(RPD, &reg, 1);
    MenloDebug::PrintNoNewline(F("RPD "));
    MenloDebug::PrintHex(reg);

    //
    // 0x00
    //
    // bit 0 == 0 - EN_DYN_ACK, W_TX_PAYLOAD_NOACK command not enabled
    // bit 1 == 0 - EN_ACK_PAY is disabled (enables payload with ack)
    // bit 2 == 0 - dynamic payload disabled
    // bit 7:3 - Reserved
    //
    readRegister(FEATURE, &reg, 1);
    MenloDebug::PrintNoNewline(F("FEATURE "));
    MenloDebug::PrintHex(reg);

#endif
}

/*
Settings from Arduino Uno as Lighthouse sensor settings:

 DBG: Radio packet sent
 DBG: nRF24L01 Registers
 DBG: STATUS 000E *same*
 DBG: FIFO_STATUS 0001 *same*
 DBG: CONFIG 000B *same*
 DBG: RF_CH 0001 *same*
 DBG: RX_PW_P0 0020 *same*
 DBG: RX_PW_P1 0020 *same*
 DBG: RX_ADDR_P0 0067 expected to be transmitter (correct)
 DBG: 0061
 DBG: 0074
 DBG: 0065
 DBG: 0030
 DBG: RX_ADDR_P1 0073 (correct as expected)
 DBG: 0065
 DBG: 006E
 DBG: 0073
 DBG: 0030
 DBG: TXADDR 0067 (correct as expected)
 DBG: 0061
 DBG: 0074
 DBG: 0065
 DBG: 0030
 DBG: EN_RXADDR 0003 (*same*)
 DBG: SETUP_AW 0003 (*same*)
 DBG: RF_SETUP 000F (*same)
 DBG: EN_AA 003F (*same*)
 DBG: SETUP_RETR 0003 (*same*)
 DBG: OBSERVE_TX 0013 (*same*)
 DBG: CD 0000 (*same*)
 DBG: RPD 0000 (*same)
 DBG: FEATURE 0000 (*same*)
 DBG: DYNPD 0000 (*same*)

*/


//
// Power control
//
// ceHi()  -> power on, transmit
// ceLow() -> power off
//
// uint8_t PTX - TX power is on
//
// powerUpTx()
//   PTX = 1
//   configRegister(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (0<<PRIM_RX) ) );
//
// powerUpRx()
//   PTX = 0
//
//
// send()
//
//   if PTX is on, packet is being sent. Waits for it.
//


// MenloPark Innovation LLC 04/09/2015

