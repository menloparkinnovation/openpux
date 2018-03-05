
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

#ifndef MorseSync_h
#define MorseSync_h

#include <inttypes.h>

//
// C++ file that implements a simple synchronous Morse
// code sending.
//
// 11/27/2010
//
// Updated 11/09/2012 for Arduino-1.0.1
//

class MorseSync {

public:

  MorseSync();

  void SendString(char* string);

private:

  void SendStringInternal(char* string);

  void SendLetterInternal(uint8_t letter);

  void FlashLed(int interval);
};

// 
// Morse Spacing
//
// TODO: Make these class constants/variables
//

// Basic unit is the dit or .
#define DIT_LENGTH 1

// A dah is three dit's in length
#define DAH_LENGTH (3*DIT_LENGTH) // 3

//
// Letters are composed of 1 - 4 dit's or dah's in combination
//

// Spacing between a dit or a dah in a letter is a dit length
#define INTRALETTER_SPACING DIT_LENGTH

// Spacing between letters is two dahs in length (6 dits)
#define INTERLETTER_SPACING (2*DAH_LENGTH) // (2*3) = 6

// Spacing between words is three dahs in length (9 dits)
#define INTERWORD_SPACING (3*DAH_LENGTH) // (3*3) = 9

//
// A letter has max interval of 4 dah's (4 * DAH_LENGTH) + (3 * INTRALETTER_SPACING)
//                                       4 * 3           + (3 * 1) = 15 dit length
//
#define LETTER_LENGTH ((4*DAH_LENGTH) + (3*INTRALETTER_SPACING)) // 15

//
// A word is (5 * LETTER_LENGTH) +  (4 * INTERLETTER_SPACING)
//           (5 * 15)            +  (4 * 6) = 99
#define WORD_LENGTH ((5*LETTER_LENGTH) + (4 * INTERLETTER_SPACING)) // 99

//
//
// 5 WPM = 25 characters = 100 dit's or dah's plus spaces
//
// (5 * WORD_LENGTH) + (5 * INTERWORD_SPACING)
// (5 * 99)          + (5 * 9) = 540
//
// This means there are 540 dit intervals per minute at 5 WPM, a basic morse speed
//
// (540 / 60) = .111 seconds, or 111 ms
//
// If a 100ms base is used, this will be 54 seconds for a 5 word string, and
// represent 5.55 words/minute
//

//
// Each Morse letter between A - Z has a maximum of four components
// of dit's and dah's
//
// Each letter is encoded in a byte using two bits to describe
// a dit or dah, or end of the sequence.
//
// 7 6  5 4  3 2  1 0  bit #
//  3    2    1    0   sequence index
//
// For example, Morse 'A' . - is encoded as:
//
// 7 6  5 4  3 2  1 0  bit #
// 0 0  0 0  1 0  0 1  = 0x05
//

#define M_END 0x0
#define M_DIT 0x1
#define M_DAH 0x2

//
// Encode the Morse letter. Each position x1, x2, x3, x4
// represents the sequence in the morse letter.
//
// For example, 'A' . - is:
//
// ENCODE_MORSE_LETTER(M_DIT, M_DAH, M_END, M_END)
//

#define ENCODE_MORSE_LETTER(x1, x2, x3, x4) ((x1) | (x2 << 2) | (x3 << 4) | (x4 << 6))

#endif // MorseSync_h

