
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

#ifndef Morse_h
#define Morse_h

//
// C++ file that uses an event driven environment to
// blink a LED with morse code.
//
// Provide an example of using  event driven async
// programming and encapsulation of state and callbacks
// within a class.
//
// 11/27/2010
//

//
// Define our states.
//
// This uses a C++ enum, rather than #defines
//
// Note: It is common C++ practice that each member is prefixed
//       with the enum's name, since they actually have global
//       scope as with #defines. This is an issue when this
//       header file is included with others in a larger project.
//       This one of the issues that C#/Java has cleaned up.
//
typedef enum _MorseState {
  MorseStateInvalid = 0,   // Allows detection of uninitialized fields
  MorseStateIdle,          // Nothing to do
  MorseStateDelay,         // Executing delay period
  MorseStateFlashLed,      // Executing FlashLed light on period
  MorseStateSendLetter,    // Executing SendLetter
  MorseStateSendString,    // Executing SendString
  MorseStateMax            // End of states
} MorseState;

//
// MorseStateSendString substates
//
typedef enum _SendStringSubState {
  SendStringSubStateInvalid = 0,
  SendStringSubStateSendingLetter,
  SendStringSubStateInterLetterDelay,
  SendStringSubStateMax
} SendStringSubState;

//
// MorseStateSendLetter substates
//
typedef enum _SendLetterSubState {
  SendLetterSubStateInvalid = 0,
  SendLetterSubStateSendAtom,
  SendLetterSubStateInterAtomDelay,
  SendLetterSubStateMax
} SendLetterSubState;

class Morse {

public:

  Morse();

  void Initialize(int pin, int polarity);

  void StartMorseTimer(Morse& morseInstance, int period);

  void SendStringAsync(char* string, int repeat);

private:

  // Invoked at the timer interval
  void TimerCallback();

  //
  // Could use a virtual function callback for better control
  //

  // Pin specification from Initialize
  int         m_pin;

  int         m_pinPolarity;

  // Current state of the state machine
  MorseState  m_state;

  //
  // Invoked for each callback to dispatch to the
  // proper state handler to drive the state machine
  //
  void StateDispatcher();

  //
  // State entry methods that start a given state
  //
  // Note: Since we do not have a callstack, each state entry
  //       function takes the next state to set, which in
  //       effect acts as the "return".
  //
  //       It is the responsibility of the state function
  //       to store this in a way that it does not overwrite
  //       a previous callers nextState variable. In effect
  //       we implement a distributed "stack".
  //

  //
  // Each virtual state "method" is composed of:
  //
  // Enter a given state, equivalent to a "method call"
  //   MethodAsync(..., MorseState nextState)
  //
  // Invoked by the callback handling state machine when the
  // given method is 'active'
  //   MethodAsyncHandler()
  //
  // Each state "method" has persistent local variables while
  // in its state, and implements its equivalent to a "stack frame"
  //   m_methodAsyncReturnState
  //   m_methodAsyncVariables
  //      ...
  //
  // This pattern is re-used regularly
  //
  // In addition, more complex state "methods" have their
  // own set of substates to manage their operations within
  // the outer "method" call.
  //

  // SendStringAsync "method"
  void SendStringInternalAsync(char* string, MorseState nextState);
  void SendStringAsyncHandler();
  SendStringSubState m_sendStringSubState;
  char* m_stringToSend;
  char* m_stringToRepeat;
  MorseState m_sendStringReturnState;

  // SendLetterAsync "method"
  void SendLetterAsync(uint8_t letter, MorseState nextState);
  void SendLetterAsyncHandler();
  SendLetterSubState m_sendLetterSubState;
  uint8_t m_morseLetterEncoded; // Encoded Morse Letter
  int m_atomIndex; // Which atom is being sent
  MorseState m_sendLetterReturnState;

  // FlashLedAsync "method"
  void FlashLedAsync(int interval, MorseState nextState);
  void FlashLedAsyncHandler();
  MorseState m_flashLedReturnState;

  // DelayAsync "method"
  void DelayAsync(int interval, MorseState nextState);
  void DelayAsyncHandler();
  int m_delayIntervals; // Count down for current delay state
  MorseState m_delayReturnState;
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

#endif // Morse_h

