
//
// 06/20/2016
//
// TODO: Work out if this is specific to ESP8266 or more general PROGMEM
//

/*

TODO:

03/18/2016

Built in String object leads to poor memory discipline.
  - Re-use a single one with String.reserve();

This object copies the data for every new instance. It relies heavily
on dynamic memory allocation, new, etc.

Not the right object for embedded systems. The format routines are
useful, but at large cost.

Used in MenloCloudFormatter, but should be ripped out.

Make all the dependent classes of MenloCloudFormatter not see
a String.

Currently used for Particle devices, which have more memory than
Arduinos and a better behaving heap.

*/

#ifndef MenloCloudFormatter_h
#define MenloCloudFormatter_h

/*
 * Copyright (C) 2016 Menlo Park Innovation LLC
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
 *  Date: 02/28/2016
 *  File: MenloCloudFormatter
 */

#include "MenloPlatform.h"
#include "MenloTimer.h"

//
// MenloCloudFormatter provides a base implmentation for a cloud
// provider to marshal data described by application provided
// tables.
//
// This avoids having to write a subclass for every cloud provider
// and every application.
//
// An application sets up its tables once similar to Dweet
// and gets supported by all clouds that the framework supports.
//

const char READING_TYPE_END    = 0;
const char READING_TYPE_INT8   = 1;
const char READING_TYPE_INT16  = 2;
const char READING_TYPE_INT32  = 3;
const char READING_TYPE_FLOAT  = 4;
const char READING_TYPE_DOUBLE = 5;
const char READING_TYPE_STRING = 6;

//
// Note:
//
// The const PROGMEM pattern is used so the code can be used on
// embedded micro's with tiny ram, but larger flash storage.
//
struct ReadingsDescription {

    //
    // TODO: Work out if this is specific to ESP8266 or more general PROGMEM
    //
#if ESP8266
    // These are in code/flash/program memory
    char* stringsTable; // const char* PROGMEM
    int* typesTable;    // PROGMEM
    int* offsetsTable;  // const int* PROGMEM
#else
    // These are in code/flash/program memory
    const char* const* PROGMEM stringsTable;
    const int* PROGMEM typesTable;
    const int* PROGMEM offsetsTable;
#endif

    ReadingsDescription () {
        stringsTable = NULL;
        typesTable = NULL;
        offsetsTable = NULL;
    }
};

class MenloCloudFormatter : public MenloCloudScheduler {

 public:

    MenloCloudFormatter();

    int Initialize(unsigned long period) {
        // Initialize base class
        return MenloCloudScheduler::Initialize(period);
    }

    int
    Format (
        ReadingsDescription* descr,
        char* buffer
        );

    //
    // Format and Post the buffer.
    //
    // Reset()'s the buffer when done.
    //
    int
    FormatAndPost (
        ReadingsDescription* descr,
        char* buffer,
        unsigned long post_timeout
        );

    //
    // Cloud generic Data format routines
    //
    void appendBuffer(String value);
    void add(String name, int value);
    void add(String name, long value);
    void add(String name, String value);
    void add(String name, float value, unsigned int precision = 4);
    void add(String name, double value, unsigned int precision = 4);
    void add(String name, unsigned long value);

    //
    // char* versions to avoid creating String() classes when not needed
    //
#if 0
    void add(char* name, int value);
#endif

    //
    // Post Reset()'s the buffer on success.
    //
    // It does not Reset() the buffer on failure so a caller
    // can retry.
    //
    // The caller is responsible for eventual buffer Reset()
    // when retry attempts are completed without success.
    //
    virtual int Post(unsigned long timeout) = 0;

    virtual void StartPreAmble() = 0;

    //
    // This returns the generated data buffer for sending out
    // on alternate transports.
    //
    String getDataBuffer();

    // Reset the buffer
    void Reset();

    //
    // Shortform allows small messages such as over
    // SMS text or other data size limited transports.
    //

    void SetShortForm(bool value);

    bool IsShortForm() {
        return m_shortForm;
    }

 private:

    bool m_shortForm;

    String m_buffer;
};

#endif // MenloCloudFormater_h
