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

    $Id$
*/

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
 * Modifications, additions, subtractions are
 * Copyright (C) 2015 Menlo Park Innovation LLC
 *
 * This work incorporates one or more "open" or "shared" source
 * licenses. As such, its use and licensing conforms to the licenses
 * of the code incorporated.
 *
 * Menlo Park Innovation LLC does not provide any warranty, support,
 * or restrictions on its use and may be distributed under the
 * terms of the contained license(s).
 *
 *  Date: 05/11/2015
 *  File: nRF24L01.h
 *
 * Re-organized definitions with references to the datasheet.
 *
 */

/* Memory Map */

//
// MenloPark Innnovation LLC:
//
// Organize this to each virtual register
//
// Comment out what is not used in the driver to understand
// basic configuation.
//
// Document data sheet references.
//

//
// MenloPark Innnovation LLC:
//
// General SPI:
//
// SPI "rotates" data into and out of the chip as a clocked
// serial ring. So each transaction is modeled as an exchange
// of a byte for a byte, etc.
//
// This means to "read" data from the chip you must shift
// an equal number of bits in.
//
// Many "read" commands will shift 0's in as a default and most "read"
// commands on chips ignore the data, or expect 0's.
//
// Commands that "write" get data back. In some cases this data
// is meaninful, such as a status register value. In other cases
// its ignored.
//

//
// MenloPark Innnovation LLC:
//
// SPI command transactions.
//
// These are commands sent as SPI transactions which instruct
// the chip what to do.
//
// Start on page 48 of nRF24L01Plus datasheet
//
#define R_REGISTER    0x00 // virtual register read, low 5 bits is reg#
#define W_REGISTER    0x20 // virtual register write, low 5 bits is reg#
#define REGISTER_MASK 0x1F // mask for the above commands
#define R_RX_PAYLOAD  0x61 // read receiver payload, byte 0 first
#define W_TX_PAYLOAD  0xA0 // write transmitter payload, byte 0 first
#define FLUSH_TX      0xE1 // flush tx
#define FLUSH_RX      0xE2 // flush rx
//#define REUSE_TX_PL   0xE3
//#define NOP           0xFF // Nop, will read status register (return of SPI byte rotate)

//
// MenloPark Innnovation LLC:
//
// Virtual Registers
//
// They are virtual since they are not directly accessed, but accessed
// by the R_REGISTER, W_REGISTER commands over the SPI.
//
// Start on page 54 of nRF24L01Plus datasheet
//

#define CONFIG      0x00   // Main configuration
#define EN_AA       0x01   // Enable Auto Ack (just read by debug dump)
#define EN_RXADDR   0x02   // Enable Receive addresses (just read by debug dump)
#define SETUP_AW    0x03   // Setup address width (just read by debug dump)
#define SETUP_RETR  0x04 // Setup automatic retransmission (just read by debug dump)
#define RF_CH       0x05   // Channel
#define RF_SETUP    0x06   // RF parameters (just read by debug dump)
#define STATUS      0x07
#define OBSERVE_TX  0x08 // Lost packet count, TX retry count (just read by debug dump)
#define CD          0x09 // nRF24L01 Carrier Detect (just read by debug dump)
#define RPD         0x09 // nRF24L01+ Receive Power Detect (just read by debug dump)
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
//#define RX_ADDR_P2  0x0C
//#define RX_ADDR_P3  0x0D
//#define RX_ADDR_P4  0x0E
//#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11 // Number of bytes in data pipe 0 RX
#define RX_PW_P1    0x12 // Number of bytes in data pipe 1 RX
//#define RX_PW_P2    0x13
//#define RX_PW_P3    0x14
//#define RX_PW_P4    0x15
//#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17 // FIFO status
#define DYNPD     0x1C   // Dynamic Payload (just read by debug dump)
#define FEATURE   0x1D   // Feature register (just read by debug dump)

//
// MenloPark Innnovation LLC:
//
// CONFIG - virtual register 00
//
// page 54 of nRF24L01Plus datasheet
//
// CONFIG == 0x0B
//         0000 1011
//         EN_CRC | PWR_UP | PRIM_RX
//
#define PRIM_RX     0 // 1 == PRX, 0 == PTX (toggle between RX/TX power)
#define PWR_UP      1 // 1 == power up
#define CRCO        2 // 0 == 1 byte CRC, 1 == 2 byte CRC
#define EN_CRC      3 // 1 == enable CRC. Forced 1 if EN_AA is 1
//#define MASK_MAX_RT 4 // 0 == enable max retransmit interrupt
//#define MASK_TX_DS  5 // 0 == enable transmit interrupt
//#define MASK_RX_DR  6 // 0 == enable receive interrupt
//#define CONFIG_RESERVED 7

//
// MenloPark Innnovation LLC:
//
// STATUS - virtual register 07
//
// page 56 of nRF24L01Plus datasheet
//
// STATUS 0x0E ==
//    0000 1110
//   TX not full
//   RX_P_1 - RX_P_2, RX_P3 == 1 (rx fifo empty)
//

//#define STATUS_TX_FULL 0 // Transmitter FIFO is full
//#define RX_P_NO_1   1 // Which RX Pipe has data
//#define RX_P_NO_2   2 // 000 - 101 is pipe number, 110 is not used
//#define RX_P_NO_3   3 // 111 == RX Fifo empty
#define MAX_RT      4   // Max retransmit interrupt. If set must be cleared for further communication
#define TX_DS       5   // TX FIFO interrupt
#define RX_DR       6   // Data ready FIFO interrupt
//#define STATUS_RESERVED 7

//
// MenloPark Innnovation LLC:
//
// FIFO_STATUS
//
// page 58 of nRF24L01Plus datasheet
//
#define RX_EMPTY      0
//#define RX_FULL     1
//#define FIFO_STATUS_RESERVED_2 2
//#define FIFO_STATUS_RESERVED_3 3
//#define TX_EMPTY    4
//#define FIFO_STATUS_TX_FULL 5
//#define TX_REUSE    6
//#define FIFO_STATUS_RESERVED7 7

/*
  MenloPark Innnovation LLC:

  Just define is actually used.

#define ENAA_P5     5
#define ENAA_P4     4
#define ENAA_P3     3
#define ENAA_P2     2
#define ENAA_P1     1
#define ENAA_P0     0
#define ERX_P5      5
#define ERX_P4      4
#define ERX_P3      3
#define ERX_P2      2
#define ERX_P1      1
#define ERX_P0      0
#define AW          0
#define ARD         4
#define ARC         0
#define PLL_LOCK    4
#define RF_DR       3
#define RF_PWR      1
#define LNA_HCURR   0        
#define PLOS_CNT    4
#define ARC_CNT     0
#define FIFO_FULL   5
*/
