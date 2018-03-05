
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

// Main Arduino C++ include header for all projects
#include "WProgram.h"

// Include the AikoEvents event driven programming environment
#include <AikoEvents.h>
#include <AikoCallback.h> // Callbacks
using namespace Aiko;

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
#include "Morse.h"

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
Morse::Morse() {

  m_stringToSend = (char*)0;
  m_state = MorseStateIdle;

  //
  // Default to PIN 13, HIGH == ON
  // This is the default LED, but shares with SPI CLK
  //
  m_pin = 13;
  m_pinPolarity = HIGH;
}

void Morse::Initialize(int pin, int polarity) {
  m_pin = pin;
  m_pinPolarity = polarity;
}

//
// To allow sending asynchronously, we must start a timer first
// by registering a callback with the timer event system.
//
void Morse::StartMorseTimer(Morse& morseInstance, int period)
{
  // Allocate an instance method callback for our timer handler
  Callback handler  = methodCallback(morseInstance, &Morse::TimerCallback);

  // Register our period
  Events.addHandler(handler, period);  // Every 100ms, ~= 5.55 WPM
}

//
// Send the string asynchronously from the timer
//
void Morse::SendStringAsync(char* string, int repeat) {

  // The string will be repeated until reset
  // TODO: This is a one shot, add Repeat state..

  if (repeat) {
    m_stringToRepeat = string;
  }
  else {
    m_stringToRepeat = (char*)0;
  }

  SendStringInternalAsync(string, MorseStateIdle);
  return;
}

//
// Invoked by Timer
//
// 100ms interval ~= 5.55 WPM
//
void Morse::TimerCallback() {

  // Process a timer event for the current interval
  StateDispatcher();
  return;
}

//
// This is invoked for each timer interval callback to
// process the current state and drive the state machine forward.
//
void Morse::StateDispatcher()
{
  switch(m_state) {

  case MorseStateIdle:
    // Nothing to do
    TRACE("MorseStateIdle\n");
    return;

  case MorseStateDelay:
    TRACE("MorseStateDelay\n");
    DelayAsyncHandler();
    return;

  case MorseStateFlashLed:
    TRACE("MorseStateFlashLed\n");
    FlashLedAsyncHandler();
    return;

  case MorseStateSendLetter:
    TRACE("MorseStateSendLetter\n");
    SendLetterAsyncHandler();
    return;

  case MorseStateSendString:
    TRACE("MorseStateSendString\n");
    SendStringAsyncHandler();
    return;

  case MorseStateInvalid:
  case MorseStateMax:
  default:
    // Bad state, recover
    TRACE("MorseStateInvalid\n");
    m_state = MorseStateIdle;
    DEBUG_SHIP_ASSERT(DEBUG_FALSE);
    return;
  }
}

void Morse::SendStringInternalAsync(char* string, MorseState nextState)
{
   if ((string == 0) || (*string == 0)) {
     m_state = nextState;
     return;
   }

   m_state = MorseStateSendString;
   m_sendStringReturnState = nextState;

   m_stringToSend = string;

   // Set our sub-state
   m_sendStringSubState = SendStringSubStateSendingLetter;

   uint8_t c = *m_stringToSend++;

   // Call SendLetterAsync, returning to our current (MorseStateSendString) state
   SendLetterAsync(c, m_state);

   return;
}

void Morse::SendStringAsyncHandler()
{
   uint8_t c;

   if (m_sendStringSubState == SendStringSubStateSendingLetter) {

     // Must wait for the inter-letter delay returning to our state
     m_sendStringSubState = SendStringSubStateInterLetterDelay;

     DelayAsync(INTERLETTER_SPACING, m_state);
     return;
   }

   // Fetch the next character
   if ((m_stringToSend == 0) || ((c = *m_stringToSend++) == 0)  ) {

     // No more characters to send, see if we have a repeat
     if (m_stringToRepeat != (char*)0) {
        m_stringToSend = m_stringToRepeat;
        c = *m_stringToSend++;
        // Fallthrough
     }
     else {
        // return to our callers desired state
        m_state = m_sendStringReturnState;
        return;
     }
   }

   // Set our sub-state
   m_sendStringSubState = SendStringSubStateSendingLetter;

   // Call SendLetterAsync, returning to our current (MorseStateSendString)
   SendLetterAsync(c, m_state);

   return;
}

void Morse::SendLetterAsync(uint8_t letter, MorseState nextState)
{
  //
  // As this is more complex, it implements its own sub-state
  // variables to encapsulate its complexity.
  //
  // This could be handled by increasing the number of the
  // master state variables, along with each "edge" method
  // for enter, handler, etc. But by implementing a state
  // machine nested within the higher level state, a clearer
  // overall design and pattern is accomplished.
  //
  // The technique is one of hierarchical, or nested state
  // machines, and essentially models the callstack, and
  // method call/return model of higher level language
  // programming.
  //
  // (Also can be called "stack ripping" when language support
  // is present to capture the state of the computation and
  // manage the states)
  //

  m_state = MorseStateSendLetter;
  m_sendLetterReturnState = nextState;

  //
  // If this is a space, we just implement a character length
  // delay and return to our callers state when done.
  //
  if (letter == ' ') {
    DelayAsync(INTERLETTER_SPACING, nextState);
    return;
  }

  // This has the handler beging sending the atom at atomIndex == 0
  m_sendLetterSubState = SendLetterSubStateInterAtomDelay;

  // Lookup the letter in the array
  uint8_t index = toupper(letter) - 'A';
  m_morseLetterEncoded = MorseLetters[index];
  m_atomIndex = 0;

  // Invoke our own handler to start the send and have common code
  SendLetterAsyncHandler();

  return;
}

void Morse::SendLetterAsyncHandler()
{
  // If end of the atom index, we are done.
  if (m_atomIndex >= 4) {

    //
    // Last state was SendLetterSubStateSendAtom
    // and no wait is needed at the end
    //
    DEBUG_ASSERT(0x01, (m_sendLetterSubState == SendLetterSubStateSendAtom));

    // Return to our callers desired state
    m_state = m_sendLetterReturnState;
    return;
  }

  // Now safe to get next atom, since we checked array bounds above
  uint8_t atom = (m_morseLetterEncoded >> (m_atomIndex * 2)) & 0x03;

  // Done sending atom, execute inter-atom delay before the next atom
  if (m_sendLetterSubState == SendLetterSubStateSendAtom) {

    //
    // If actually at the end of the sequence, we don't want the delay
    // and we are done
    //
    if (atom == M_END) {
      // Return to our callers desired state
      m_state = m_sendLetterReturnState;
      return;
    }

    // Execute the inter-atom delay
    m_sendLetterSubState = SendLetterSubStateInterAtomDelay;
    DelayAsync(INTRALETTER_SPACING, m_state);
    return;
  }

  //
  // If done  waiting for inter-atom delay, move to send atom state
  // and send the next atom
  //
  if (m_sendLetterSubState == SendLetterSubStateInterAtomDelay) {
    m_sendLetterSubState = SendLetterSubStateSendAtom;
    // Fallthrough
  }

  DEBUG_ASSERT(0x01, (m_sendLetterSubState == SendLetterSubStateSendAtom));

  switch (atom) {

    case M_DIT:
      m_sendLetterSubState = SendLetterSubStateSendAtom;
      m_atomIndex++;

      // Flash the light, return to our current state
      FlashLedAsync(DIT_LENGTH, m_state);
      return;

    case M_DAH:
      m_sendLetterSubState = SendLetterSubStateSendAtom;
      m_atomIndex++;

      // Flash the light, return to our current state
      FlashLedAsync(DAH_LENGTH, m_state);
      return;

    case M_END:
      // End of atom, done with sending letter, return to our callers state
      m_state = m_sendLetterReturnState;
      return;

    default:
      DEBUG_ASSERT(0x01, DEBUG_FALSE);
   }

  return;
}

void Morse::FlashLedAsync(int interval, MorseState nextState)
{
  m_state = MorseStateFlashLed;
  m_flashLedReturnState = nextState;

  digitalWrite(m_pin, m_pinPolarity);   // set the LED on

  DelayAsync(interval, MorseStateFlashLed);

  return;
}

// Invoked when m_state == MorseStateFlash
void Morse::FlashLedAsyncHandler()
{
  //
  // Delay call is done, turn off the light
  //
  digitalWrite(m_pin, !m_pinPolarity);    // set the LED off

  // Return to our callers desired state
  m_state = m_flashLedReturnState;

  return;
}

void Morse::DelayAsync(int interval, MorseState nextState)
{
  m_delayIntervals = interval;
  m_state = MorseStateDelay;
  m_delayReturnState = nextState;
  return;
}

// Invoked when m_state == MorseStateDelay
void Morse::DelayAsyncHandler()
{
  if (m_delayIntervals > 0) {
    m_delayIntervals--;
  }

  if (m_delayIntervals == 0) {

    // Next state
    m_state = m_delayReturnState;
  }

  // If m_delayIntervals > 0, we remain in MorseStateDelay

  return;
}
