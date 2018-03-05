
/*
 * Copyright (C) 2014 Menlo Park Innovation LLC
 *
 * This is a trade secret, unpublished work of original
 * authorship. This copyright grants no license for any
 * purpose, express or implied.
 *
 *  Date: 09/25/2014
 *  File: NMEA0813.c
 */

//
// Debug Monitor
//

//
// Models DOS Debug command
//
// AtMega328 additions:
//
//   - Extra dump command "di" added for code space reads.
//
//   - Normal commands can operate on data (RAM) space.
//
//   - I/O commands operate on register space
//
// d [range]
// d 100 10f    // dump range from 0x100 - 0x10F
// d100 10f     // no space required. dump range from 0x100 - 0x10F
//
// d 100 1 20   // dump 20 single byte items
//
// Output Format:
//
// Addr       16 bytes in hex                               printable ascii
//  0000  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
//
// Code space read: (i == instruction space)
//
// di [range]
// di 100 10f    // dump code space range from 0x100 - 0x10F
// di100 10f     // no space required. dump code space range from 0x100 - 0x10F
//
// e [range] [optional_values]
//
// e 100<CR>     <-- displays current value at 0100 for entry
//   0100 00.    <-- cursor after dot allow entry of value
//
// e 100 00  // enter 0x00 into address 0x0100
//
// Note: No "e" command for code space since it must be written
//   with special flash block write commands.
//
// i <port>  // displays byte from port
//           // Can be used for special memory spaces on AtMega328
//
// o <port> <byte_value> // sends data to I/O port
//

