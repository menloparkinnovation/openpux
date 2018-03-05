
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
// Http State Machine for Smartpux Cloud
//
// Provides a full example of SendSensorData and receive sensor target
// state over HTTP with code targeted for embedded controllers with
// minimal buffers.
//
// 12/02/2013
// 01/18/2014 ported to Arduino MenloSmartPux class.
//
// (C) 2013 Menlo Park Innovation LLC
// (C) 2014 Menlo Park Innovation LLC
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "MenloPlatform.h"
#include "MenloSmartpux.h"

//
// Why does this program hand code HTTP POST, REST, etc? and not
// use "mumble foo library, language, etc?"
//
// Because its intended as an example and test vehicle for
// code that can be implemented inside of embedded systems.
// So it tries to take the most minimum dependencies on sockets
// and basic string functions.
//
// It even goes as far as to use static buffers since dynamic
// memory allocation can be problematic on embedded systems, but
// can be easily converted.
//
// Since this is only focused on the most basic sensor transactions
// it does not represent a significant investment.
//
// Higher level application transactions will use any set of
// standard web technologies and languages such as Java, Jersey,
// Ruby, Python, Javascript, etc.
//


//
// HTTP response processing is designed to be supported on tiny
// microcontrollers with minimal buffer space. As a result
// it operates as a state machine with minimal history while
// processing a response.
//

//
// TCP can return data on any record boundary and this
// follows for the HTTP response received. Since minimal to
// no buffering is available inside tiny microcontroller
// implementations, the state machine directly processes the incoming
// byte stream handles HTTP state transitions, HTTP header processing,
// response document processing, and response document parsing into
// binary values.
//

//
// Systems with larger buffers can have a simpler state machine
// to process a line at a time, buffer an entire response
// document, array of HTTP headers, etc. We have no such
// luxury with embedded systems such as AtMega328's with
// 2kb of total RAM for heap, stack, and initialized data.
//

//
// HTTP receive state machine
//

// Set to 1 for detailed debugging dumps
#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (Serial.println(F(x)))
#define DBG_PRINT_INT(x) (Serial.println(x))
#define DBG_PRINT2(x, y) (Serial.print(F(x)) && Serial.println(y))
#define DBG_PRINT3(x, y, z) (Serial.print(F(x)) && Serial.print(y) && Serial.println(z))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT2(x, y)
#define DBG_PRINT3(x, y, z)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)     (Serial.println(F(x)))
#define xDBG_PRINT_INT(x) (Serial.println(x))
#define xDBG_PRINT2(x, y) (Serial.print(F(x)) && Serial.println(y))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT2(x, y)
#endif

//
// Tokens/response lines we look for
//

// TODO: Use pgm_read_byte() and F() to place in pgmspace
const char* HttpHeaderTokens[] = {
  "HTTP/1.1 200 OK",
  "Content-Type: application/x-www-form-urlencoded"
};

int HttpHeaderTokensSize = HTTP_TOKENS_NUMBER;

// Prepare the state machine for a new response
void
HttpResponseStateMachineReset(
    HttpResponseState* http,
    SensorResponseState* sensorState,
    SensorResponseData* sensor
    )
{
  int index;

  http->documentOK = 0;
  http->endOfHeaders = 0;
  http->candidateTokenIndex = 0;
  http->lastTokenNewline = 0;

  http->candidateToken = -1;

  // lastChar initialized to '\n' allows a new line to to be recognized at start
  http->lastChar = '\n';

  // Intitalize our HTTP receiver headers state machine
  for(index = 0; index < HTTP_TOKENS_NUMBER; index++) {
    http->TokensFound[index] = 0;
  }

  // Initialize our response document data
  sensorState->EndOfDocument = 0;
  sensorState->NameChar = 0;
  sensorState->NameChar2 = 0;
  sensorState->DigitsIndex = -1;

  // Initialize parsed result data
  sensor->documentOK = 0;
  sensor->command = 0;
  sensor->sleepTime = 0;
  sensor->targetMask0 = 0;
  sensor->targetMask1 = 0;
  sensor->targetMask2 = 0;
  sensor->targetMask3 = 0;
  sensor->targetMask4 = 0;
  sensor->targetMask5 = 0;
  sensor->targetMask6 = 0;
  sensor->targetMask7 = 0;
  sensor->targetMask8 = 0;
  sensor->targetMask9 = 0;
}

// Return != 0 when document processing is done
int
HttpProcessSensorDocument(
    HttpResponseState* http,
    SensorResponseState* sensorState,
    SensorResponseData* sensor,
    char* buf,
    int len
    )
{
  int value;


    while (len > 0) {

        DBG_PRINT("HttpProcessSensorDocument: processing ");

        if (sensorState->EndOfDocument) {
            // Swallow rest of buffer(s) at end of document
            xDBG_PRINT("HttpProcessSensorDocument: throwing away chars after EndOfDocument");
            return 1;
        }

        // C=0&S=3C&M0=0&M1=0&M2=0&M3=99

	if((sensorState->DigitsIndex != -1) &&
	     (((buf[0] >= '0') && (buf[0] <= '9')) ||
	     ((buf[0] >= 'a') && (buf[0] <= 'f')) ||
	     ((buf[0] >= 'A') && (buf[0] <= 'F')))) {

	    if(sensorState->DigitsIndex >= SENSOR_RESPONSE_MAX_DIGITS) {
		// Abandon current number, to long
		xDBG_PRINT("Number to Long\n");
		sensorState->NameChar = 0;
		sensorState->DigitsIndex = -1;
	    }

            DBG_PRINT2("processed digit ", buf[0]);
	    sensorState->Digits[(int)sensorState->DigitsIndex] = buf[0];
	    sensorState->DigitsIndex++;
	}
	else {

          DBG_PRINT2("HttpProcessSensorDocument: switch for c ", buf[0]);

	  switch(buf[0]) {

	  case '\0':
	    // a null in the HTTP response stream is skipped
	    break;

	  case '=':
	    if (sensorState->NameChar != 0) {
	       DBG_PRINT2("Found = for NameChar, switching to digits state ", sensorState->NameChar);
	       // Switch to recording numbers
	       sensorState->DigitsIndex = 0;
	    }
	    break;

	  case '\n':

	      // Two Newline's ("\r\n") represents end of document
	      if ((http->lastChar == '\r') && http->lastTokenNewline) {
		  xDBG_PRINT("EndOfDocument, documentOK\n");
		  sensorState->EndOfDocument = 1;
                  sensor->documentOK = 1;
		  return 1;
	      }

	  case '\r':
	      // Fall through since '\r' ends any current name=value item

	  case '&':
	    // End of current name=value

	    if (sensorState->DigitsIndex != -1) {

	      sensorState->Digits[(int)sensorState->DigitsIndex] = 0;

  	      value = strtol(sensorState->Digits, NULL, 16);
    
    	      DBG_PRINT2("Found & or \\r, ending current value ", value);

    	      if (sensorState->NameChar == 'C') {
                DBG_PRINT2("Setting value for C", value);
		sensor->command = value;
	      }
	      else if (sensorState->NameChar == 'S') {
                DBG_PRINT2("Setting value for S", value);
		sensor->sleepTime = value;
	      }
	      else if (sensorState->NameChar == 'M') {
		switch(sensorState->NameChar2) {
		case '0':
                DBG_PRINT2("Setting value for M0", value);
		sensor->targetMask0 = value;
		break;
		case '1':
                DBG_PRINT2("Setting value for M1", value);
		sensor->targetMask1 = value;
		break;
		case '2':
                DBG_PRINT2("Setting value for M2", value);
		sensor->targetMask2 = value;
		break;
		case '3':
                DBG_PRINT2("Setting value for M3", value);
		sensor->targetMask3 = value;
		break;
		case '4':
                DBG_PRINT2("Setting value for M4", value);
		sensor->targetMask4 = value;
		break;
		case '5':
                DBG_PRINT2("Setting value for M5", value);
		sensor->targetMask5 = value;
		break;
		case '6':
                DBG_PRINT2("Setting value for M6", value);
		sensor->targetMask6 = value;
		break;
		case '7':
                DBG_PRINT2("Setting value for M7", value);
		sensor->targetMask7 = value;
		break;
		case '8':
                DBG_PRINT2("Setting value for M8", value);
		sensor->targetMask8 = value;
		break;
		case '9':
                DBG_PRINT2("Setting value for M9", value);
		sensor->targetMask9 = value;
		break;
    		}
	      }

	      sensorState->NameChar = 0;
	      sensorState->DigitsIndex = -1;
    	    }
    
    	    break;

	  case 'C':
	  case 'S':
	  case 'M':
	  case 'E':
	    DBG_PRINT2("Found keyword ", buf[0]);
	    sensorState->NameChar = buf[0];
	    break;

	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    if (sensorState->NameChar == 'M') {
		DBG_PRINT2("Found Mask ", buf[0]);
		sensorState->NameChar2 = buf[0];
	    }
	    else {
		DBG_PRINT2("Number not part of a Mask ", buf[0]);
	    }
	    break;

	  default:

	    // Unrecognized character, stop current name=value processing
	    DBG_PRINT2("Unrecognized character ", buf[0]);
	    sensorState->NameChar = 0;
	    sensorState->DigitsIndex = -1;

	    break;
         }
     }

#if notdefined
	 //
	 // Newline is a two character sequence, so we handle here so as
	 // not to complicate the above switch()
	 //
	 if ((buf[0] == '\n') && (http->lastChar == '\r')) {
	     http->lastTokenNewline = 1;
	 }
	 else if (buf[0] == '\r') {
	     // do nothing, as we may be setting up for \r\n\r\n
	 }
	 else {
	     http->lastTokenNewline = 0;
	 }

	 http->lastChar = buf[0];
#endif
	 buf++;
         len--;
    }

  return 0;
}

//
// Find the token that starts with prefix and
// matches the newChar after the prefix.
//
// Skips the entry indexToSkip if != -1 to allow
// search for "all, but entry".
//
int
HttpFindCandidateToken(
    const char* prefix,
    char  newChar,
    int indexToSkip
    )
{
  int len;
  int index;

  for(index = 0; index < HttpHeaderTokensSize; index++) {

    len = strlen(HttpHeaderTokens[index]);

    if (strncmp(HttpHeaderTokens[index], prefix, len) == 0) {

      if ((index != indexToSkip) &&
          (HttpHeaderTokens[index][len] == newChar)) {

        return index;
      }
    }
  }

  return -1;
}

// Returns != 0 if done processing all documents
int
HttpResponseStateMachine(
    HttpResponseState* http,
    SensorResponseState* sensorState,
    SensorResponseData* sensor,
    char* buf,
    int len
    )
{
  int index = 0;
  int retval = 0;

  //
  // TCP streams bring in data in any chunk size, at any
  // boundary.
  //
  // This processes the HTTP response stream looking first
  // for a valid set of HTTP headers and the headers end
  // response ("\r\n\r\n"). It then switches to processing
  // the expected www-urlencoded document stream till end
  // of connection.
  //

  //
  // TODO: Currently does not obey content length, but uses
  // connection close as an indication of done.
  //

  //DBG_PRINT2("HttpResponseStateMachine: len ", len);

  while (len > 0) {

    //
    // At any time we can get end of headers and
    // have to switch to content processing.
    //
    if (http->endOfHeaders) {

      //
      // We only process the content response if the headers
      // are what we expected, and not an error response from
      // the cloud server.
      //
      if (http->documentOK) {

          // Processing HTTP response document/content
          retval = HttpProcessSensorDocument(http, sensorState, sensor, buf, len);
      }
    }
    else {

      // Processing HTTP response headers

      //
      // HTTP header tokens are matched from the begining of
      // a line, but may have trailing data.
      //

      switch(buf[0]) {

      case '\0':
        // a null in the HTTP response stream is skipped
        break;

      case '\n':

          // Two newlines (of "\r\n" each) represents end of HTTP response headers
  	  if ((http->lastChar == '\r') && http->lastTokenNewline) {

            xDBG_PRINT("End of Headers\n");
	    http->endOfHeaders = 1;

            //
            // See if its an OK response document that
	    // we recognize
            //
            if (http->TokensFound[HTTP_TOKEN_HTTP1_1_OK] &&
                http->TokensFound[HTTP_TOKEN_CONTENT_URLENCODED]) {
              xDBG_PRINT("Document type OK\n");
	      http->documentOK = 1;
	    }
            else {
              xDBG_PRINT("Http Response document not recognized\n");
            }

            break;
          }

        // fall through

      case '\r':
        // fall through

      default:

        // See if we have a token in progress
        if (http->candidateToken != (-1)) {
          
	  http->candidateTokenIndex++;

          // if reached end of candidateToken, complete it
 	  if ('\0' == HttpHeaderTokens[(int)http->candidateToken][(int)http->candidateTokenIndex]) {
            DBG_PRINT2("Completing token ", HttpHeaderTokens[http->candidateToken]);
	    http->TokensFound[(int)http->candidateToken] = 1;
            http->candidateToken = -1;
            http->candidateTokenIndex = 0;
          }
	  else if(buf[0] == '\r') {
	    // '\r' ends the search for the current header line and begins a new one
            http->candidateToken = -1;
            http->candidateTokenIndex = 0;
          }
	  else if (buf[0] != HttpHeaderTokens[(int)http->candidateToken][(int)http->candidateTokenIndex]) {

            //
            // Character does not match current candidate token, see if
	    // there is a token which does match
	    //

            // Used for DBG_PRINT3() below
            //int saveIndex = http->candidateToken;

            http->candidateToken = HttpFindCandidateToken(
		  HttpHeaderTokens[(int)http->candidateToken],
                  buf[0],
                  index);

            if (http->candidateToken == (-1)) {
	        // Cancel the current candidate, hit a no match character
                //DBG_PRINT3("Canceling token %s due to no match character %c\n",
	         //HttpHeaderTokens[saveIndex], buf[0]);
                http->candidateTokenIndex = 0;
            }
	  }
	}

        //
        // Search for a new candidate if this is the start of a new line
	//
        if ((http->lastChar == '\n') && http->candidateToken == (-1)) {
	  for(index = 0; index < HttpHeaderTokensSize; index++) {
            if (HttpHeaderTokens[index][0] == buf[0]) {
              DBG_PRINT2("Found new candidate token ", HttpHeaderTokens[index]);
	      http->candidateToken = index;
	    }
	  }
        }

        break;
      }
    }

    //
    // Newline is a two character sequence, so we handle here so as
    // not to complicate the above switch()
    //
    if ((buf[0] == '\n') && (http->lastChar == '\r')) {
      http->lastTokenNewline = 1;
    }
    else if (buf[0] == '\r') {
      // do nothing, as we may be setting up for \r\n\r\n
    }
    else {
      http->lastTokenNewline = 0;
    }

    http->lastChar = buf[0];

    buf++;
    len--;
  }

  return retval;
}
