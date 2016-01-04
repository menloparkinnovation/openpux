
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
 *  Date: 01/07/2015
 *  File: MenloDweet.h
 *
 * Dweet handling
 *
 * Used for Smartpux DWEET's.
 */

#include "MenloPlatform.h"
#include "MenloUtility.h"
#include "MenloDebug.h"
#include "MenloMemoryMonitor.h"
#include "MenloNMEA0183.h"
#include "DweetStrings.h"
#include "MenloDweet.h"

#define DBG_PRINT_ENABLED 0

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

MenloDweet::MenloDweet()
{
  m_ignoreChecksumErrors = false;
  m_traceLevel = 0;
  m_port = NULL;
  m_nmea = NULL;
}

int
MenloDweet::Initialize()
{
  // invoke base to initialize MenloDispatchObject
  MenloDispatchObject::Initialize();
  return 0;
}

int
MenloDweet::Initialize(MenloNMEA0183* nmea, Stream* port)
{
  Initialize();

  m_nmea = nmea;
  m_port = port;
  return 0;
}

//
// Invoked from ProcessUnrecognizedCommand()
//
// Note: Unrecognized Dweet's are dispatched right away
// rather than defering to poll(). This is because the
// shared input buffer can be re-used by a receiver
// class (serial or radio) for a newly arrived command.
//
// DweetHandlers have their own careful stack management
// restrictions to avoid recursion and running out of
// stack, though they re-use the MenloEvent infrastructure.
//
int
MenloDweet::EmitUnhandledDweetEvent(char* name, char* value)
{
  unsigned long pollInterval;
  MenloDweetEventArgs eventArgs;

  DBG_PRINT("MenloDweet: Emitting Unhandled Event");

  if (!m_eventList.HasListeners()) {
    DBG_PRINT("MenloDweet: No listeners");
    return 0; // no handlers registered
  }

  eventArgs.dweet = this;
  eventArgs.name = name;
  eventArgs.value = value;

  //
  // Send event to listeners
  //
  // Note: pollInterval is re-used here:
  //
  // The standard DispatchEvents() code returns the smallest value
  // with the default being MAX_POLL_TIME.
  //
  // MAX_POLL_TIME - Event not handled, just as a normal Poll()
  //                 indicates by default.
  //
  //  0            - Someone handled the event. In theory only
  //                 one should handle any event, but its not
  //                 an infrastructure failure if multiple do. They
  //                 are responsible for what it means for any common
  //                 I/O buffers used.
  //
  pollInterval = m_eventList.DispatchEvents(this, &eventArgs);
  if (pollInterval == 0) {
    DBG_PRINT("MenloDweet: identified handler");
    return 1; // One or more identified as a handler
  }
  else {
    DBG_PRINT("MenloDweet: no handler identified");
    return 0; // No one identified as a handler
  }
}

void
MenloDweet::RegisterUnhandledDweetEvent(MenloDweetEventRegistration* callback)
{
  // Add to event list
  m_eventList.Register(callback);
  return;
}

void
MenloDweet::RegisterNMEAMessageEvent(MenloNMEAMessageEventRegistration* callback)
{
  // Add to event list
  m_nmeaEventList.Register(callback);
  return;
}

//
// Process a Dweet input buffer.
//
// This is the raw set of characters ending with '\n' which indicates
// a possble NMEA 0183 message.
//
// Called from DweetSerialChannel.cpp, ProcessSerialInput()
//
int
MenloDweet::DispatchMessage(char* buffer, int length)
{
    int retVal = 0;
    unsigned char checksum = 0;
    int checksumOK = 0;
    char* cmds = NULL;
    char prefix[MenloNMEA0183::MaxPrefixSize + 1];

    DBG_PRINT("DispatchMessage entered");

    //
    // Echoing an unescaped NMEA 0183/Dweet sentence causes errors due
    // to embedded special characters.
    //
    //MenloDebug::Print("ReceivedBuffer ");
    //MenloDebug::PrintHex(length);
    //MenloDebug::Print(buffer);

    //
    // Try and parse it as a valid NMEA 0183 message.
    //
    // This returns a char* to the first command following the ","
    // character after the prefix.
    //
    cmds = m_nmea->parse(buffer, &prefix[0], &checksum, &checksumOK);
   
    //MenloDebug::PrintNoNewline("checksumOK ");
    //MenloDebug::PrintHex(checksumOK);

    if (checksumOK || m_ignoreChecksumErrors) {

        //
        // Validate Prefix is for Menlo Dweet $PDWT
        //
        if (strncmp_P(prefix, PSTR("$PDWT"), 5) == 0)  {

            // cmds should not be NULL
            if (cmds == NULL) {
                xDBG_PRINT("DispatchMessage cmds is NULL");
            }
            else {
                retVal = ProcessDweetCommands(&prefix[0], cmds);
            }
        }
        else {
            xDBG_PRINT("non Dweet NMEA 0183 message");
            retVal = EmitNMEAMessageEvent(&prefix[0], cmds, buffer);
        }
    }
    else {
        xDBG_PRINT("dweet bad checksum");
    }

    return retVal;
}

unsigned long
MenloDweet::EmitNMEAMessageEvent(char* prefix, char* cmds, char* buffer)
{
    unsigned long pollInterval;
    MenloNMEAMessageEventArgs eventArgs;

    if (!m_nmeaEventList.HasListeners()) {
        DBG_PRINT("MenloDweet: No listeners for NMEA Message Event");
        return 0; // no handlers registered
    }

    eventArgs.dweet = this;
    eventArgs.prefix = prefix;
    eventArgs.cmds = cmds;
    eventArgs.buffer = buffer;

    //
    // Send event to listeners
    //
    pollInterval = m_nmeaEventList.DispatchEvents(this, &eventArgs);

    return pollInterval;
}

int
MenloDweet::SendDweetCommand(char* command)
{
    char *str;
    int result;

    m_nmea->reset();

    //
    // addCommand returns result 0 if there is not enough
    // space for the command in the current sentence.
    //
    // since we are starting a new sentence we should not
    // get this error.
    //

    result = m_nmea->addCommand(command);
    if (result == 0) {
       return result;
    }

    str = m_nmea->generateSentence();
    if (str == NULL) {
       return 0;
    }

    result = strlen(str);

    // Send it out
    m_port->write(str, result);

    return 1;
}

int
MenloDweet::SendDweetCommand_P(PGM_P command)
{
    char *str;
    int result;

    m_nmea->reset();

    //
    // addCommand returns result 0 if there is not enough
    // space for the command in the current sentence.
    //
    // since we are starting a new sentence we should not
    // get this error.
    //

    result = m_nmea->addCommand_P(command);
    if (result == 0) {
       return result;
    }

    str = m_nmea->generateSentence();
    if (str == NULL) {
        return 0;
    }

    result = strlen(str);

    // Send it out
    m_port->write(str, result);

    return 1;
}

int
MenloDweet::SendDweetCommandStart()
{
    // Start a new NMEA 0183 sentence
    m_nmea->reset();

    // Start a new command
    m_nmea->addCommandStart();

    return 1;
}

int
MenloDweet::SendDweetCommandStart(char* prefix)
{
    // Start a new NMEA 0183 sentence
    m_nmea->reset(prefix);

    // Start a new command
    m_nmea->addCommandStart();

    return 1;
}

int
MenloDweet::SendDweetCommandSeparator()
{
    return m_nmea->addCommandStart();
}

int
MenloDweet::SendDweetCommandPart(char* command)
{
    return m_nmea->addCommandPart(command);
}

int
MenloDweet::SendDweetCommandPart_P(PGM_P command)
{
    return m_nmea->addCommandPart_P(command);
}

int
MenloDweet::SendDweetCommandComplete()
{
    char *str;
    int length;

    // Send the current NMEA 0183 sentence
    str = m_nmea->generateSentence();
    if (str == NULL) {
       DBG_PRINT("error generating NMEA0183 sentence");
       return 0;
    }

    length = strlen(str);

    // Send it out
    m_port->write(str, length);

    return 1;
}

//
// Process the Dweet commands that have been
// received from the valid NMEA 0183 sentence.
//
// each entry in cmds is a separate NMEA 0183 "word"
// which is a dweet such as GETCONFIG=NAME which are
// separated using the "," character.
//
int
MenloDweet::ProcessDweetCommands(char* prefix, char* cmds)
{

    char* ptr;
    char* cmd;
    int handled = 0;

    //
    // dispatch based on sentence prefix
    //
    //MenloDebug::PrintNoNewline("prefix: ");
    //MenloDebug::Print(prefix);

    //
    // dispatch based on specific command
    //
    //MenloDebug::PrintNoNewline("cmds: ");
    //MenloDebug::Print(cmds);

    // Start of command string
    cmd = cmds;

    while(1) {
        ptr = strchr(cmd, ',');
        if (ptr == NULL) {
            // no more ','
	    if (ProcessDweetCommand(cmd)) {
	      handled = 1;
	    }

            return handled;
        }

        // Set null terminator
        *ptr = '\0';
        if (ProcessDweetCommand(cmd)) {
	      handled = 1;
	}

        // Next command is after this pointer
        cmd = ptr + 1;
    }

    return handled;
}

//
// Process a single Dweet command
//
int
MenloDweet::ProcessDweetCommand(char* cmd) {
    int retVal = 0;
    char* ptr;
    char* name;
    char* value = NULL;

    //MenloDebug::PrintNoNewline("cmd: ");
    //MenloDebug::Print(cmd);

    ptr = strchr(cmd, '=');
    if (ptr == NULL) {
        // single value command with no '='
        name = cmd;

        //MenloDebug::PrintNoNewline("singleton command: name: ");
        //MenloDebug::Print(name);
    }
    else {

        name = cmd;
        *ptr = '\0';

        value = ptr+1;

        //MenloDebug::PrintNoNewline("nv command: name: ");
        //MenloDebug::PrintNoNewline(name);
        //MenloDebug::PrintNoNewline(" value: ");
        //MenloDebug::Print(value);

        // Find a handler for the command
        retVal = DispatchDweetCommand(name, value);
    }

    return retVal;
}

//
// Many Dweet commands contain an item:action for their
// value portion.
//
//    name       value
// "DWEET_CMD=item:action"
//
// Parameters:
//
//  str - value portion of Dweet command string
//
//  item - pointer to location to return NULL terminated action string
//
//  action - pointer to location to return NULL terminated action string
//
//  Note: this modifies the buffer in place by setting '\0' string
//  terminators.
//
int
MenloDweet::ProcessItemAction(char* str, char** item, char** action) {

    char* ptr;

    ptr = strchr(str, ':');
    if (ptr == NULL) {
	// no ':'

	// Not an item:action, give original string
        *item = str;
	return 0;
    }
    else {

	*item = str;
	*ptr = '\0';
	*action = ptr+1;
    }

    return 1;
}

//
// Process an address:value string.
//
// Format example:
//
//  addr value
//  0000:0000
//
int
MenloDweet::ProcessAddressValue(
    char* str,
    unsigned long* address,
    unsigned long* value
    )
{
    return ProcessAddressValueStrings(str, address, value, NULL, NULL);
}

int
MenloDweet::ProcessAddressValueStrings(
    char* str,
    unsigned long* address,
    unsigned long* value,
    char** addressString,
    char** valueString
    )
{
    char* item = NULL;
    char* action = NULL;
    char* endptr = NULL;
    unsigned long val = 0;

    if (address != NULL) {
        *address = 0;
    }

    if (value != NULL) {
        *value = 0;
    }

    if (ProcessItemAction(str, &item, &action) == 0) {

	// error, not item:action format
	return 0;
    }

    if (addressString != NULL) {
        *addressString = item;
    }

    if (valueString != NULL) {
        *valueString = action;
    }

    //
    // Note: strtoul cost from avr-libc:
    //
    // 10,762 bytes before strtoul
    // 11,402 bytes after strtoul
    // - 640 bytes
    //

    //
    // Convert address from ascii to long
    //

    if (address != NULL) {

        val  = strtoul(item, &endptr, 16);
        if (endptr == item) {
            // conversion failure
            return 0;
        }

        *address = val;
    }

    //
    // Convert data value ascii to long
    //

    if (value != NULL) {

        endptr = NULL;
        val  = strtoul(action, &endptr, 16);
        if (endptr == action) {
            // conversion failure
            return 0;
        }

        *value = val;
    }

    return 1;
}

//
// Process an address string.
//
// Format example:
//
//  addr
//  0000
//
int
MenloDweet::ProcessAddress(
    char* str,
    unsigned long* address
    )
{
    char* endptr = NULL;
    unsigned long val = 0;

    *address = 0;

    //
    // Convert address from ascii to long
    //

    val = strtoul(str, &endptr, 16);
    if (endptr == str) {
        // conversion failure
        return 0;
    }

    *address = val;

    return 1;
}

//
// Caller overrides.
//
int
MenloDweet::ProcessAppCommands(char* name, char* value)
{
  return 0;
}

//
// Find a handler for a Dweet command.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
// Note: This is a virtual function and the caller
// can override, handle local commands, and return.
//
// dispatchCommands
// KEYWORDS: dispatchCommands
int
MenloDweet::DispatchDweetCommand(char* name, char* value) {

    xDBG_PRINT("MenloDweet DispatchDweetCommand");

    if (ProcessAppCommands(name, value)) {
       xDBG_PRINT("App handled");
    }
    else if (ProcessBuiltInCommands(name, value)) {
       xDBG_PRINT("BuiltIn command handled");
    }
    else {
        xDBG_PRINT("unhandled");

        // No one recognized it.

        //
        // This allows virtual override by the caller.
        // It's default is to emit an UnrecognizedDweet
        // event, or return _UNSUP if no handlers.
        //
        return ProcessUnrecognizedCommand(name, value);
    }
    
    // if anyone handled it it falls through
    return 1;
}

//
// Process an unrecognized command.
//
// Callers can override this method as its virtual to
// filter behavior.
//
// It's default behavior is to try and find a handler through
// emitting the UnhandledDweet event, and if no handlers
// returns a Dweet _UNSUP reply.
//
// If no handler found, results in:
//
// $PDWT,COMMAND_UNSUP=VALUE*00\r\n
//
int
MenloDweet::ProcessUnrecognizedCommand(char* name, char* value)
{

     DBG_PRINT("MenloDweet: ProcessUnrecognizedCommand");

    // Attempt to find a handler by emmitting an event
    if (EmitUnhandledDweetEvent(name, value) != 0) {
      // Processed
      return 1;
    }

    //
    // No handlers send an _UNSUP reply
    //

    SendDweetCommandStart();
    SendDweetCommandPart(name);
    SendDweetCommandPart_P(dweet_unsup_string);
    SendDweetCommandPart(value);
    SendDweetCommandComplete();

    return 0;
}

//
// Marshal a reply that has an address, data value.
//
// This routine uses a combination of code space and
// data space buffers.
//
// GETCONFIG_REPLY=VERSION:0000
//

int
MenloDweet::SendDweetItemValueReply(
    PGM_P command,
    char* item,
    char* value
    )
{
    SendDweetCommandStart();
    SendDweetCommandPart_P(command);
    SendDweetCommandPart_P(dweet_reply_string);
    SendDweetCommandPart(item);
    SendDweetCommandPart_P(PSTR(":"));
    SendDweetCommandPart(value);
    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDweetItemValueReply_P(
    PGM_P command,
    PGM_P item,
    char* value
    )
{
    SendDweetCommandStart();
    SendDweetCommandPart_P(command);
    SendDweetCommandPart_P(dweet_reply_string);
    SendDweetCommandPart_P(item);
    SendDweetCommandPart_P(PSTR(":"));
    SendDweetCommandPart(value);
    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDweetItemValueReply_PP(
    PGM_P command,
    PGM_P item,
    PGM_P value
    )
{
    SendDweetCommandStart();
    SendDweetCommandPart_P(command);
    SendDweetCommandPart_P(dweet_reply_string);
    SendDweetCommandPart_P(item);
    SendDweetCommandPart_P(PSTR(":"));
    SendDweetCommandPart_P(value);
    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDweetItemReply(
    PGM_P command,
    char* item
    )
{
    SendDweetCommandStart();
    SendDweetCommandPart_P(command);
    SendDweetCommandPart_P(dweet_reply_string);
    SendDweetCommandPart(item);
    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDweetItemValueInt8Reply(
    PGM_P command,
    char* item,
    char  value
    )
{
    char buf[3];

    MenloUtility::UInt8ToHexBuffer(value, &buf[0]);
    buf[2] = '\0';

    return SendDweetItemValueReply(command, item, buf);
}

//
// Note: Buffer must have at least one character after bufferlength
// available for a NULL.
//
// Why? Trying to minimize buffer allocations and stack space
// on the AtMega328 with 2k total RAM. This contract is similar
// to str* functions of std C libraries which do not include the
// implied NULL in their length specifiers but expect the callers
// to do so in their calculations.
//
int
MenloDweet::SendDweetItemValueInt8StringReply(
    PGM_P command,
    char* item,
    uint8_t* buffer,
    int bufferlength
    )
{
    int index;
    char buf[3];

    SendDweetCommandStart();
    SendDweetCommandPart_P(command);
    SendDweetCommandPart_P(dweet_reply_string);
    SendDweetCommandPart(item);
    SendDweetCommandPart_P(PSTR(":"));

    //
    // The buffer of uint8_t's expands to two ascii hex characters
    // for each entry.
    //
    for (index = 0; index < bufferlength; index++) {

        MenloUtility::UInt8ToHexBuffer(buffer[index], &buf[0]);
        buf[2] = '\0';

        SendDweetCommandPart(buf);
    }

    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDweetItemValueInt16Reply(
    PGM_P command,
    char* item,
    int   value
    )
{
    char buf[5];

    MenloUtility::UInt16ToHexBuffer(value, &buf[0]);
    buf[4] = '\0';

    return SendDweetItemValueReply(command, item, buf);
}

int
MenloDweet::SendDweetItemValueReplyType(
    PGM_P command,
    PGM_P replyType,
    char* item,
    char* value
    )
{
    SendDweetCommandStart();
    SendDweetCommandPart_P(command);
    SendDweetCommandPart_P(replyType);
    SendDweetCommandPart(item);
    SendDweetCommandPart_P(PSTR(":"));
    SendDweetCommandPart(value);
    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDweetItemReplyType_P(
    PGM_P command,
    PGM_P replyType,
    char* item
    )
{
    SendDweetCommandStart();
    SendDweetCommandPart_P(command);
    SendDweetCommandPart_P(replyType);
    SendDweetCommandPart(item);
    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDweetItemReplyType(
    char* command,
    PGM_P replyType,
    char* item
    )
{
    SendDweetCommandStart();
    SendDweetCommandPart(command);
    SendDweetCommandPart_P(replyType);
    SendDweetCommandPart(item);
    SendDweetCommandComplete();

    return 1;
}

int
MenloDweet::SendDebugItemValueReplyType(
    PGM_P command,
    PGM_P replyType,
    char* item,
    char* value
    )
{
    // This will start a NMEA 0183 $PDBG sentence
    MenloDebug::PrintNoNewline(command);

    MenloDebug::PrintNoNewline(replyType);
    MenloDebug::PrintNoNewline((const char*)item);
    MenloDebug::PrintNoNewline(PSTR(":"));

    // This will complete the NMEA 0183 $PDBG sentence
    MenloDebug::Print((const char*)value);

    return 1;
}

int
MenloDweet::SendDebugItemReplyType(
    PGM_P command,
    PGM_P replyType,
    char* item
    )
{
    // This will start a NMEA 0183 $PDBG sentence
    MenloDebug::PrintNoNewline(command);

    MenloDebug::PrintNoNewline(replyType);

    // This will complete the NMEA 0183 $PDBG sentence
    MenloDebug::Print((const char*)item);

    return 1;
}

int
MenloDweet::CalculateMaximumValueReply(
    PGM_P command,
    PGM_P replyType,
    char* value
    )
{
    int size;
    int dweetOverhead;

    // Max NMEA 0183 user payload
    int maxPayload = MenloNMEA0183::MaximumUserPayload();

    // Calculate Dweet command overhead COMMAND_REPLY=
    dweetOverhead = strlen_P(command);
    dweetOverhead += strlen_P(replyType);

    // subtract Dweet overhead
    maxPayload -= dweetOverhead;

    size = strlen(value);
    if (size > maxPayload) {
        size = maxPayload;
    }

    // Maximum size Dweet value portion can be
    return size;
}

//
// Lookup the index of an entry in the string table.
//
// Returns the index of the name found.
//
// Returns -1 for not found.
//
// The length of the string in the table determines
// the comparison length. If the supplied compareString
// is larger and the prefix matches, the index is returned.
//
int
MenloDweet::LookupStringPrefixTableIndex(
    PGM_P stringTable,          // table of PGM character strings
    int tableEntries,           // number of entries in the table
    char* compareString
    )
{
    PGM_P p;
    int index;
    int length;

    // Look for the entry
    for (index = 0; index < tableEntries; index++) {

        // MenloPlatform.h
        p = (PGM_P)MenloPlatform::GetStringPointerFromStringArray((char**)stringTable, index);

        // Get the length of the prefix string
        length = strlen_P(p);

        xDBG_PRINT_NNL("prefixIndex compareString is ");
        xDBG_PRINT_STRING(compareString);

        xDBG_PRINT_NNL("length is ");
        xDBG_PRINT_INT(length);

        // If the prefix matches the input string, its a match
        if (strncmp_P(compareString, p, length) == 0)  {
            // found entry
            xDBG_PRINT_NNL("prefixIndex entry found index is ");
            xDBG_PRINT_INT(index);
            return index;
        }
    }

    xDBG_PRINT("prefixIndex no entry found");

    return -1;
}
