
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
 * Copyright (C) 2014 Menlo Park Innovation LLC
 *
 *  Date: 01/19/2014
 *  File: httpstate.h
 */

#ifndef httpstate_h
#define httpstate_h

//
// This is the decoded content from the sensor
// response document.
//
typedef struct _SensorReponseData {
  int documentOK;
  int command;
  int sleepTime;
  int targetMask0;
  int targetMask1;
  int targetMask2;
  int targetMask3;
  int targetMask4;
  int targetMask5;
  int targetMask6;
  int targetMask7;
  int targetMask8;
  int targetMask9;
} SensorResponseData;

#define HTTP_TOKEN_HTTP1_1_OK         0
#define HTTP_TOKEN_CONTENT_URLENCODED 1

// Must match HttpHeaderTokens[] entries
#define HTTP_TOKENS_NUMBER            2

//
// This contains the state for the HTTP response
// state machine.
//
typedef struct _HttpResponseState {
    char documentOK;
    char lastChar;
    char lastTokenNewline;
    char endOfHeaders;
    char candidateToken;
    char candidateTokenIndex;
    char TokensFound[HTTP_TOKENS_NUMBER];
} HttpResponseState;

//
// This contains the state for the Sensor response
// document state machine.
//
#define SENSOR_RESPONSE_MAX_DIGITS 8

typedef struct _SensorResponseState {
    char EndOfDocument;
    char NameChar;
    char NameChar2;
    char DigitsIndex;
    char Digits[SENSOR_RESPONSE_MAX_DIGITS + 1]; // +1 for null
} SensorResponseState;

//
// Http State machine functions and definitions
//
// State storage is handled by the caller to allow multiple
// connections to be processed at the same time on gateways
// with enough memory to do so.
//

void
HttpResponseStateMachineReset(
    HttpResponseState* httpState,
    SensorResponseState* sensorDocumentState,
    SensorResponseData* sensorDocumentData
    );

int
HttpResponseStateMachine(
    HttpResponseState* httpState,
    SensorResponseState* sensorDocumentState,
    SensorResponseData* sensorDocumentData,
    char *buf,
    int len
    );

#endif // httpstate_h
