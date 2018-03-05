
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

//
// Generate Morse code from characters and strings
//
// 11/27/2010
//
// Updated 11/09/2012 for Arduino-1.0.1
//

// Main Arduino C++ include header for all projects
#include "Arduino.h"

//
// Remove this before production ship. Always
// include Debug.h as it contains NULL versions
// of the ASSERT definitions and runtime debug
// message/print functions.
//
#define DEBUG 1

//
// Define detailed tracing
//
//#define TRACE 1

//
// Include Debug library support
//
#include "Debug.h"

// Include Morse library header
#include "MorseSync.h"

// Define our MORSE_TIMER_BASE
//#define MORSE_TIMER_BASE 100 // ~= 5.5 WPM
#define MORSE_TIMER_BASE 200 // ~= 2.75 WPM, lighthouse rate

//
// Morse Letters 'A' - 'Z'
//
// TODO: Put this in PROGRAM space
//
uint8_t MorseLetters[] = {
    ENCODE_MORSE_LETTER(M_DIT, M_DAH, M_END, M_END), // A
    ENCODE_MORSE_LETTER(M_DAH, M_DIT, M_DIT, M_DIT), // B
    ENCODE_MORSE_LETTER(M_DAH, M_DIT, M_DAH, M_DIT), // C
    ENCODE_MORSE_LETTER(M_DAH, M_DIT, M_DIT, M_END), // D
    ENCODE_MORSE_LETTER(M_DIT, M_END, M_END, M_END), // E
    ENCODE_MORSE_LETTER(M_DIT, M_DIT, M_DAH, M_DIT), // F
    ENCODE_MORSE_LETTER(M_DAH, M_DAH, M_DIT, M_END), // G
    ENCODE_MORSE_LETTER(M_DIT, M_DIT, M_DIT, M_DIT), // H
    ENCODE_MORSE_LETTER(M_DIT, M_DIT, M_END, M_END), // I
    ENCODE_MORSE_LETTER(M_DIT, M_DAH, M_DAH, M_DAH), // J
    ENCODE_MORSE_LETTER(M_DAH, M_DIT, M_DAH, M_END), // K
    ENCODE_MORSE_LETTER(M_DIT, M_DAH, M_DIT, M_DIT), // L
    ENCODE_MORSE_LETTER(M_DAH, M_DAH, M_END, M_END), // M
    ENCODE_MORSE_LETTER(M_DAH, M_DIT, M_END, M_END), // N
    ENCODE_MORSE_LETTER(M_DAH, M_DAH, M_DAH, M_END), // O
    ENCODE_MORSE_LETTER(M_DIT, M_DAH, M_DAH, M_DIT), // P
    ENCODE_MORSE_LETTER(M_DAH, M_DAH, M_DIT, M_DAH), // Q
    ENCODE_MORSE_LETTER(M_DIT, M_DAH, M_DIT, M_END), // R
    ENCODE_MORSE_LETTER(M_DIT, M_DIT, M_DIT, M_END), // S
    ENCODE_MORSE_LETTER(M_DAH, M_END, M_END, M_END), // T
    ENCODE_MORSE_LETTER(M_DIT, M_DIT, M_DAH, M_END), // U
    ENCODE_MORSE_LETTER(M_DIT, M_DIT, M_DIT, M_DAH), // V
    ENCODE_MORSE_LETTER(M_DIT, M_DAH, M_DAH, M_END), // W
    ENCODE_MORSE_LETTER(M_DAH, M_DIT, M_DIT, M_DAH), // X
    ENCODE_MORSE_LETTER(M_DAH, M_DAH, M_DIT, M_DAH), // Y
    ENCODE_MORSE_LETTER(M_DAH, M_DAH, M_DIT, M_DIT)  // Z
};

// Constructor
MorseSync::MorseSync() {
}

//
// Send the string now, not returning to the caller
// until done.
//
void MorseSync::SendString(char* string) {

  while (*string) {

    char c = *string;

    switch (c) {
    case ' ':
      delay(INTERWORD_SPACING * MORSE_TIMER_BASE);
      break;

    default:
      SendLetterInternal(c);
      delay(INTERLETTER_SPACING * MORSE_TIMER_BASE);
    }

    string++;
  }
}

void MorseSync::SendLetterInternal(uint8_t letter) {

  TRACE("SendLetterInternal:\n");

  uint8_t index = toupper(letter) - 'A';
  uint8_t sequence = MorseLetters[index];

  uint8_t atomIndex = 0;
  uint8_t atom = sequence & 0x03;

  while (atomIndex < 4) {
    atom = (sequence >> (atomIndex * 2)) & 0x03;

    switch (atom) {

    case M_DIT:
      FlashLed(DIT_LENGTH * MORSE_TIMER_BASE);
      delay(INTRALETTER_SPACING * MORSE_TIMER_BASE);
      break;

    case M_DAH:
      FlashLed(DAH_LENGTH * MORSE_TIMER_BASE);
      delay(INTRALETTER_SPACING * MORSE_TIMER_BASE);
      break;

    case M_END:
      break;
    }

    atomIndex++;
  }
}

//
// Flash the led on for the specified interval
//
// Assumes state is off on entry, and is off on exit
//
void MorseSync::FlashLed(int interval) {
 
  digitalWrite(13, HIGH);   // set the LED on

  delay(interval);

  digitalWrite(13, LOW);    // set the LED off
}

