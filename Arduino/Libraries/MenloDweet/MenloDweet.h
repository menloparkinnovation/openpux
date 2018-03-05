
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

#ifndef MenloDweet_h
#define MenloDweet_h

#include <MenloPlatform.h>
#include "MenloObject.h"
#include "MenloDispatchObject.h"
#include "DweetStrings.h"

//
// Setting this support to on overflows Uno for LightHouse by 38 bytes
// and Photon by 8 bytes.
//
// MenloWatchDog:
//
// 19,800 with
// 19,604 without
//    196 bytes
//
//#define DWEET_STATE_ENABLE_DEFAULT_TABLE_SUPPORT 0
#define DWEET_STATE_ENABLE_DEFAULT_TABLE_SUPPORT 1

// Built in string sizes not in MenloConfigStore.h
// Includes '\0'
#define NOCHECKSUM_SIZE 2
#define TRACELEVEL_SIZE 3
#define SLEEPTIME_SIZE  9 // 8 digits + '\0'
#define AWAKETIME_SIZE  9 // 8 digits + '\0'

#define DWEET_ERROR                    0xFFFF
#define DWEET_INVALID_PARAMETER        0xFFFE
#define DWEET_INVALID_PARAMETER_LENGTH 0xFFFD
#define DWEET_PARAMETER_TO_SHORT       0xFFFC
#define DWEET_PARAMETER_TO_LONG        0xFFFB
#define DWEET_ERROR_UNSUP              0xFFFA // No object name handler
#define DWEET_NO_FUNCTION              0xFFF9 // No function in table
#define DWEET_INVALID_CHECKSUM         0xFFF8
#define DWEET_ENTRY_NOT_FOUND          0xFFF7
#define DWEET_PIN_REQUIRED             0xFFF6
#define DWEET_APP_FAILURE              0xFFF5
#define DWEET_SET_NOTSUPPORTED         0xFFF4
#define DWEET_NO_POWER                 0xFFF3
#define DWEET_GET_NOTSUPPORTED         0xFFF2
#define DWEET_NO_RESOURCE              0xFFF1

#define DWEET_EVENT_NOT_PROCESSED      0

//
// For documentation of all parameters to the table driven
// routines see model.txt
//

//
//
// Default values table entry.
//
// Entries without default values will not invoke the set function
// when there is a checksum error on the EEPROM configuration block.
//
// A valid EEPROM checksum, but rejected value will attempt to provide
// a default entry if its available in this table.
//
// Size comes from the size_table[] for the given index_table[configIndex]
//
struct StateSettingsDefaultValue {
    int configIndex; // EEPROM config index value identifing value
    PGM_P address;   // Address of variable with default configuration data
};

//
// Arguments structure used to pass the applications configured table pointers.
//
struct StateSettingsParameters {
    PGM_P stringTable;         // table of PGM character strings
    PGM_P functionTable;       // table of PGM function pointers
    PGM_P ModuleName;          // Dweet Module providing the parameters
    const int* indexTable;     // table of PGM int's for configIndex
    const int* sizeTable;      // table of PGM int's for configSize

    // table of PGM default values when checksum is invalid
    const struct StateSettingsDefaultValue* defaultsTable;

    MenloObject* object;       // object "this" to invoke on
    int tableEntries;          // number of entries in the above tables
    char* workingBuffer;       // caller ensures is maximum size for commands
    int checksumIndex;         // index of checksum (always 2 ASCII chars)
    int checksumBlockStart;    // start block that is checksummed
    int checksumBlockSize;     // size of block that is checksummed
    char* name;                // Dweet left hand side request GETCONFIG of "GETCONFIG=ITEM"
    char* value;               // Dweet right hand side request ITEM of "GETCONFIG=ITEM"
};

//
// MenloDweet raises an Event when an unrecognized Dweet
// is received to allow Dweet component handlers. This
// allows the construction of composable applications.
//
class MenloDweet;

class MenloDweetEventArgs : public MenloEventArgs {
 public:
  MenloDweet* dweet;
  char* name;
  char* value;
};

// Introduce the proper type name. Could be used for additional parameters.
class MenloDweetEventRegistration : public MenloEventRegistration {
 public:
};

//
// This class emits a NMEA Event message when non-Dweet
// NMEA sentences are received.
//
class MenloNMEAMessageEventArgs : public MenloEventArgs {
 public:
  MenloDweet* dweet;
  char* prefix;
  char* cmds;
  char* buffer;
};

// Introduce the proper type name. Could be used for additional parameters.
class MenloNMEAMessageEventRegistration : public MenloEventRegistration {
 public:
};

//
// An instance of MenloDweet is created per transport type in order
// to handle the internal state of a Dweet transport independently.
//
// Multiple Applications can register for Dweet events and the
// dispatch logic allows an application to "claim" the event.
//
// This allows for composible applications in which subsystems handle
// specific Dweets.
//
// Example: LightHouse application package, Debug package, Arduino
// low level base platform package, etc.
//
// The listener event list is global across all Dweet transports
// which allow applications to receive dweets regardless of transport.
//
// MenloDweetEventArgs.dweet points to the "this" instance of
// the MenloDweet transport instance raising the event and is what should
// be used to parse/format a reply as it will be for the specific
// transport the Dweet came in on.
//

class MenloDweet : public MenloDispatchObject {

 private:

  //
  // These are static so application modules can register for
  // Dweets from all transports.
  //

  //
  // MenloDweet is an Event generator
  //  
  static MenloEvent s_eventList;

  //
  // Registrations for NMEA 0183 message received notifications
  //
  static MenloEvent s_nmeaEventList;

protected:

  int EmitUnhandledDweetEvent(char* name, char* value);

  //
  // Process a NMEA 0183 message that has arrived that is not
  // the Menlo Dweet $PDWT prefix by emitting an event for listeners
  // to handle.
  //
  unsigned long EmitNMEAMessageEvent(char* prefix, char* cmds, char* buffer);

 public:

  static void RegisterGlobalUnhandledDweetEvent(MenloDweetEventRegistration* callback);

  static void RegisterGlobalNMEAMessageEvent(MenloNMEAMessageEventRegistration* callback);

  MenloDweet();

  virtual int Initialize();

  //
  // MenloDweet() uses the default Poll() implmementation of the base class
  // as its events are delivered synchronously due to shared buffer usage
  // in the I/O contract for small memory embedded systems.
  //

  int Initialize(MenloNMEA0183* nmea, Stream* port);

  //
  // This virtual allows a caller to override the port write
  // function for transports that don't model an Arduino
  // Stream* style port.
  //
  // In this case m_port would be NULL at initialize.
  //
  virtual size_t WritePort(const uint8_t *buffer, size_t size);

  //
  // Process a Dweet input buffer.
  //
  // A return value of 0 means the Dweet message was not recognized
  // and processed by any of the application listeners or subsystems.
  //
  int DispatchMessage(char* buffer, int length);

  //
  // Send a Dweet Command
  //
  int SendDweetCommand(char* command);

  int SendDweetCommand_P(PGM_P command);

  //
  // This allows a command to be started and then sent
  // as a series of data or program space buffers and then
  // sent when complete.
  //

  //
  // Start a new NMEA 0183/dweet command by reseting the
  // buffer and pre-loading the prefix.
  //
  int SendDweetCommandStart();
  int SendDweetCommandStart(char* prefix);

  // Add ',' command separator
  int SendDweetCommandSeparator();

  int SendDweetCommandPart(char* command);

  int SendDweetCommandPart_P(PGM_P command);

  int SendDweetCommandComplete();

  //
  // Dispatch commands
  //
  // These are virtual to allow selective override
  //
  // Each dispatch handler returns a value to indicate
  // whether a command was handled.
  //
  // Returns 1 if the command is handled and the search for
  // a handler is stopped.
  //
  // Returns 0 if the command is not handled and the search
  // for a handler should continue.
  //

  //
  // Main dispatcher. This invokes the built in dispatchers
  // and the stub for the AppDispatcher.
  //
  virtual int DispatchDweetCommand(char* name, char* value);

  //
  // Process an unrecognized command.
  //
  virtual int ProcessUnrecognizedCommand(char* name, char* value);

  //
  // Process application specific commands.
  //
  virtual int ProcessAppCommands(char* name, char* value);

  //
  // Common table driven worker routines.
  //

  // DweetState.cpp
  static
  int
  LoadConfigurationSettingsTable(
      struct StateSettingsParameters* parms
      );

  static
  int
  LoadDefaultSettingFromTable(
      struct StateSettingsParameters* parms,
      int index
      );

  // MenloDweet.cpp
  int
  LookupStringPrefixTableIndex(
      PGM_P stringTable,          // table of PGM character strings
      int tableEntries,           // number of entries in the table
      char* compareString
      );

  int
  ProcessGetConfigCommandsTable(
      PGM_P configTable,         // table of PGM character strings
      const int* indexTable,     // table of PGM int's
      const int* sizeTable,      // table of PGM int's
      int tableEntries,          // number of entries in the above tables
      char* workingBuffer,
      char* name,
      char* value
      );

  // DweetConfig.cpp
  int
  ProcessSetConfigCommandsTable(
      PGM_P configTable,         // table of PGM character strings
      const int* indexTable,     // table of PGM int's
      const int* sizeTable,      // table of PGM int's
      int tableEntries,          // number of entries in the above tables
      int checksumIndex,         // index of checksum (always 2 ASCII chars)
      int checksumBlockStart,    // start block that is checksummed
      int checksumBlockSize,     // size of block that is checksummed
      char* name,                // SETCONFIG
      char* item,                // item name such as RADIOCHANNEL
      char* action               // action/value such as 01
      );

  //
  // Support for calling SETSTATE/GETSTATE functions driven by
  // a function table.
  //

  int
  ProcessStateCommandsTable(
      struct StateSettingsParameters* parms
      );

  //
  // isSet == FALSE is GETSTATE=
  // isSet == TRUE is SETSTATE=
  //
  typedef int (MenloObject::*StateMethod)(char* buf, int size, bool isSet);

  // DweetState.cpp
  static int
  DispatchFunctionFromTable(
      PGM_P functionTable,       // table of PGM function pointers
      MenloObject* object,       // object "this" to invoke on
      int index,
      char* buf,
      int size_arg,
      bool bool_arg
      );

  //
  // BuiltIn commands are separate to allow applications to override them.
  //
  virtual int ProcessBuiltInCommands(char* name, char* value);

  // Set NoChecksum
  int NoChecksumHandler(char* buf, int size, bool isSet);

  // Built in trace level handler
  int TraceLevelHandler(char* buf, int size, bool isSet);

  // Built in CPU sleep control
  int SleepModeHandler(char* buf, int size, bool isSet);

  // Built in CPU speed control
  int CpuSpeedHandler(char* buf, int size, bool isSet);

  // Return the amount of time spent awake
  int AwakeTimeHandler(char* buf, int size, bool isSet);

  // Return the amount of time spent sleeping
  int SleepTimeHandler(char* buf, int size, bool isSet);

  //
  // Process the Dweet commands that have been
  // received from the valid NMEA 0183 sentence.
  //
  int ProcessDweetCommands(char* prefix, char* cmds);

  //
  // Process a single Dweet command
  //
  int ProcessDweetCommand(char* cmd);

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
  //  Note: this modifies the buffer in place by setting NULL string
  //  terminators.
  //
  int ProcessItemAction(char* str, char** item, char** action);

  //
  // Process an address:value string.
  //
  // Format example:
  //
  //  addr value
  //  0000:0000
  //
  int ProcessAddressValue(
    char* str,
    unsigned long* address,
    unsigned long* value
    );

  //
  // Process an address:value string.
  //
  // Returns the component strings.
  //
  // Format example:
  //
  //  addr value
  //  0000:0000
  //
  //  addressString = "0000"
  //  valueString = "0000"
  //
  // Parameters are optional and if NULL the particular
  // converion and/or value is not returned.
  //
  int ProcessAddressValueStrings(
    char* str,
    unsigned long* address,
    unsigned long* value,
    char** addressString,
    char** valueString
    );

  //
  // Process an address string.
  //
  // Format example:
  //
  //  addr
  //  0000
  //
  int ProcessAddress(
    char* str,
    unsigned long* address
    );

  //
  // Workers for command reply formats to save and buffers
  // in a common space.
  //
  // COMMAND_REPLY=ITEM:VALUE
  //

  int
  SendDweetItemValueReply(
    PGM_P command,
    char* item,
    char* value
    );

  int
  SendDweetItemValueReply_P(
    PGM_P command,
    PGM_P item,
    char* value
    );

  int
  SendDweetItemValueReply_PP(
    PGM_P command,
    PGM_P item,
    PGM_P value
    );

  //
  // COMMAND_REPLY=ITEM
  //
  int
  SendDweetItemReply(
    PGM_P command,
    char* item
    );

  int
  SendDweetItemValueInt8Reply(
    PGM_P command,
    char* item,
    char  value
    );

  //
  // Send a Int8 buffer as ASCII.
  //
  // Note: Buffer must have at least one character after bufferlength
  // available for a NULL. This is similar behavior to strlen()/strcat() which
  // returns valid string characters length, but does not include
  // the NULL in the length.
  //
  int
  SendDweetItemValueInt8StringReply(
    PGM_P command,
    char* item,
    uint8_t* buffer,
    int bufferlength
    );

  int
  SendDweetItemValueInt16Reply(
    PGM_P command,
    char* item,
    int   value
    );

  //
  // General reply routines which allow the
  // caller to specify whether _REPLY, _ERROR, _UNSUP, etc.
  //
  // TODO: Phase out previous ones to save code space.
  //

  //
  // COMMAND_REPLY=item:value
  // COMMAND_ERROR=item:value
  // COMMAND_UNSUP=item:value
  //
  int
  SendDweetItemValueReplyType(
    PGM_P command,
    PGM_P replyType,
    char* item,
    char* value
    );

  //
  // COMMAND_REPLY=value
  // COMMAND_ERROR=value
  // COMMAND_UNSUP=value
  //
  int
  SendDweetItemReplyType_P(
    PGM_P command,
    PGM_P replyType,
    char* item
    );

  int
  SendDweetItemReplyType(
    char* command,
    PGM_P replyType,
    char* item
    );

  //
  // These place the error into the optional
  // debug stream $PDBG and are not replied
  // on the main stream. Their use is for more
  // advanced development time and/or field diagnosis
  // debugging.
  // 
  int
  SendDebugItemValueReplyType(
    PGM_P command,
    PGM_P replyType,
    char* item,
    char* value
    );

  int
  SendDebugItemReplyType(
    PGM_P command,
    PGM_P replyType,
    char* item
    );

  byte GetTraceLevel() {
    return m_traceLevel;
  }

  void SetTraceLevel(byte level) {
    m_traceLevel = level;
  }

  int
  CalculateMaximumValueReply(
      PGM_P command,
      PGM_P replyType,
      char* value
      );

  //
  // Radio support
  //
  int
  ProcessRadioTransmit(
     char* name,
     char* value
     );

  bool
  ProcessToRadioBuffer(
    char* str,
    uint8_t* channel,
    uint8_t* radioBuffer,
    uint8_t radioBufferLength
    );

  void
  SendRadioReceiveDweet(
    uint8_t channel,
    uint8_t* radioBuffer,
    uint8_t radioBufferLength
    );

 protected:

 private:

  //  void SendDweetCommandPreamble_P(PGM_P command);

  // Used for parsing and formatting output
  MenloNMEA0183* m_nmea;

  // Used for sending complete sentences out
  Stream* m_port;

  byte m_traceLevel;

  bool m_ignoreChecksumErrors;

}; // 4 bytes

#endif // MenloDweet_h
