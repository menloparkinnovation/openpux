
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

//
// MenloFramework
//
#include <MenloPlatform.h>
#include <MenloObject.h>
#include <MenloMemoryMonitor.h>
#include <MenloUtility.h>
#include <MenloNMEA0183Stream.h>
#include <MenloDebug.h>

#include "MenloCloudScheduler.h"
#include "MenloCloudFormatter.h"

#define DBG_PRINT_ENABLED 1

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define DBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_HEX_STRING(x, l)
#define DBG_PRINT_HEX_STRING_NNL(x, l)
#define DBG_PRINT_NNL(x)
#define DBG_PRINT_INT(x)
#define DBG_PRINT_INT_NNL(x)
#endif

//
// Allows selective print when debugging but just placing
// an "x" in front of what you want output.
//
#define XDBG_PRINT_ENABLED 0

#if XDBG_PRINT_ENABLED
#define xDBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_HEX_STRING(x, l)  (MenloDebug::PrintHexString(x, l))
#define xDBG_PRINT_HEX_STRING_NNL(x, l)  (MenloDebug::PrintHexStringNoNewline(x, l))
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_HEX_STRING(x, l)
#define xDBG_PRINT_HEX_STRING_NNL(x, l)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

MenloCloudFormatter::MenloCloudFormatter()
{
    m_buffer = "";
}

//
// Return the data buffer for sending out on alternate transports.
//
String
MenloCloudFormatter::getDataBuffer()
{
    // Begining & must be removed
    return m_buffer.substring(1);
}

void
MenloCloudFormatter::Reset()
{
    // Destructor will free memory of the existing buffer
    m_buffer = "";

    StartPreAmble();
}

//
// Shortform allows the use of small values.
//
void
MenloCloudFormatter::SetShortForm(bool value)
{
    m_shortForm = value;
}

int
MenloCloudFormatter::FormatAndPost (
    ReadingsDescription* descr,
    char* buffer,
    unsigned long post_timeout
    )
{
    int retVal;

    xDBG_PRINT("CloudFormatter invoking format");

    retVal = Format(descr, buffer);
    if (retVal == 0) {
        return retVal;
    }

    xDBG_PRINT("CloudFormatter invoking post");

    retVal = Post(post_timeout);

    // Ensure buffer is reset.
    Reset();

    return retVal;
}

int
MenloCloudFormatter::Format (
    ReadingsDescription* descr,
    char* buffer
    )
{
    PGM_P p;
    int dataType;
    int length;
    int index;
    int8_t i8_tmp;
    int16_t i16_tmp;
    int32_t i32_tmp;
    float float_tmp;
    double double_tmp;
    char* string_tmp;
    bool entry = false;
    int fieldOffset;
    char fieldName[80];

    //
    // Note: using the String based routines means that each call site
    // allocates String() objects to contain the value when calling
    // ::add(String name, String value). This temporary object is
    // then deleted when it leaves the scope by running its destructor.
    //
    // This model is very malloc() heavy, and not recommended for really
    // tiny memory controllers such as 2K RAM AtMega328's.
    //

    for (index = 0;; index++) {

        //
        // Note: We have to deal with PGM strings for architectures
        // that place constant tables in flash that are accessed using
        // different instructions or I/O routines.
        //
        // MenloFramework uses flash based data strings and tables
        // to minimize impact on runtime memory on architectures
        // that make the distinction.
        //

        xDBG_PRINT("Format loop top");

        MenloMemoryMonitor::CheckMemory(__LINE__);

        //
        // Note: the address math is for an array of int's, though
        // we are only accessing word (16 bit) data on 32 bit
        // architectures. This is because programs declare the
        // tables as int's in PGMSPACE, and its 16 bit on AVR's
        // which were the first platform supported.
        //
        // If the full width is needed, use pgm_read_int32(),
        // or pgm_read_int() for architecture specific sized.
        // (16 bit on AVR, 32 bit on ARM/ESP8266).
        //

        dataType = pgm_read_word(&descr->typesTable[index]);

        xDBG_PRINT_NNL("dataType ");
        xDBG_PRINT_INT(dataType);

        if (dataType == READING_TYPE_END) {

            xDBG_PRINT("Format done");

            // Done
            break;
        }

        fieldOffset = pgm_read_word(&descr->offsetsTable[index]);

        // MenloPlatform.cpp
        p = (PGM_P)MenloPlatform::GetStringPointerFromStringArray(
            (char**)descr->stringsTable, index);

        length = strlen_P(p);
        if (length > (sizeof(fieldName) - 1)) {
            DBG_PRINT("MenloCloudFormatter truncated field name");
            length = sizeof(fieldName) - 1;
        }
        
        strcpy_P(fieldName, p);
        fieldName[sizeof(fieldName) - 1] = '\0';

        xDBG_PRINT_NNL("FieldName ");
        xDBG_PRINT_STRING(fieldName);

        xDBG_PRINT_NNL("FieldOffset ");
        xDBG_PRINT_INT(fieldOffset);

        switch(dataType) {

        //
        // Note: the buffer is in regular memory space.
        //

        case READING_TYPE_INT8:
            xDBG_PRINT("INT8");
            i8_tmp = *((int8_t*)(buffer + fieldOffset));
            add(fieldName, i8_tmp);
            entry = true;
            break;

        case READING_TYPE_INT16:
            xDBG_PRINT("INT16");
            i16_tmp = *((int16_t*)(buffer + fieldOffset));
            add(fieldName, i16_tmp);
            entry = true;
            break;

        case READING_TYPE_INT32:

            xDBG_PRINT("INT32");

            i32_tmp = *((int32_t*)(buffer + fieldOffset));

            xDBG_PRINT_NNL("INT32 got value ");
            xDBG_PRINT_INT(i32_tmp);

            add(fieldName, i32_tmp);

            xDBG_PRINT("INT32 return from add");

            entry = true;
            break;

        case READING_TYPE_FLOAT:
            xDBG_PRINT("FLOAT");
            float_tmp = *((float*)(buffer + fieldOffset));
            add(fieldName, float_tmp);
            entry = true;
            break;

        case READING_TYPE_DOUBLE:
            xDBG_PRINT("DOUBLE");
            double_tmp = *((double*)(buffer + fieldOffset));
            add(fieldName, double_tmp);
            entry = true;
            break;

        case READING_TYPE_STRING:
            xDBG_PRINT("STRING");
            string_tmp = ((char*)(buffer + fieldOffset));
            add(fieldName, string_tmp);
            entry = true;
            break;

        default:
            // skip it
            xDBG_PRINT("Format unknown type skipping");
            break;
        }
    }

    if (!entry) {
        // No entries
        xDBG_PRINT("Format no entries");
        return 0;
    }

    xDBG_PRINT("Format done");

    return 1;
}

void
MenloCloudFormatter::appendBuffer(String value)
{
    m_buffer += value;
}

void
MenloCloudFormatter::add(String name, String value)
{
    m_buffer += "&" + name + "=" + value;
}

void
MenloCloudFormatter::add(String name, int value)
{
    // Use the String() constructor to perform any conversions required
    m_buffer += "&" + name + "=" + String(value);
}

void
MenloCloudFormatter::add(String name, long value)
{
    m_buffer += "&" + name + "=" + String(value);
}

void
MenloCloudFormatter::add(String name, unsigned long value)
{
    m_buffer += "&" + name + "=" + String(value);
}

void
MenloCloudFormatter::add(String name, float value, unsigned int precision)
{
    String str(value, precision);

    m_buffer += "&" + name + "=" + str;
}

void
MenloCloudFormatter::add(String name, double value, unsigned int precision)
{
    String str(value, precision);

    m_buffer += "&" + name + "=" + str;
}
