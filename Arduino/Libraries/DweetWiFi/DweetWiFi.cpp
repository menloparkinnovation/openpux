
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
 *  Date: 05/30/2016
 *  File: DweetWiFi.h
 *
 *  WiFi Dweet handling.
 */

#include "MenloPlatform.h"
#include "MenloMemoryMonitor.h"
#include "MenloDebug.h"
#include "MenloUtility.h"
#include "MenloNMEA0183.h"
#include "MenloConfigStore.h"
#include "MenloDweet.h"
#include "DweetWiFi.h"

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)         (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
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
#define xDBG_PRINT_NNL(x)     (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x)     (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

const char wifi_module_name_string[] PROGMEM = "DweetWiFi";

extern const char dweet_wifi_ssid_string[] PROGMEM = "WIFISSID";

extern const char dweet_wifi_password_string[] PROGMEM = "WIFIPASSWORD";

extern const char dweet_wifi_channel_string[] PROGMEM = "WIFICHANNEL";

extern const char dweet_wifi_power_timer_string[] PROGMEM = "WIFIPOWERTIMER";

extern const char dweet_wifi_attention_string[] PROGMEM = "WIFIATT";

extern const char dweet_wifi_options_string[] PROGMEM = "WIFIOPTIONS";

//
// DweetWiFi provides an example of a common pattern used
// through MenloFramework for embedded devices using the
// Dweet protocol.
//
// The pattern for adding new Dweet commands for
// GETCONFIG/SETCONFIG, GETSTATE/SETSTATE is to create
// a unique string definition for each command in program memory.
// This provides one instance of the string stored efficiently on
// architectures with separate I + D space, or in constant/read-only
// memory on architectures with a uniform addressing model.
//
// A table with a pointer to each command string is created to
// allow commands to be looked up by a common worker routine
// when they arrive. Their index in the table defines its
// entry for other data required to support the request.
//
// GETCONFIG/SETCONFIG stored their setting in the MenloConfigStore,
// which is usually backed by EEPROM or an emulation. Parallel
// tables are created that contain the index into the configuration
// store, and the size of the entry to allow use of a common worker
// routine for all GETCONFIG/SETCONFIG handling. The data itself
// is stored as simple ASCII strings minimizing the translation
// of the data.
//
// GETSTATE/SETSTATE functions invoke action routines to retrieve
// or update the state of some object. As such a parallel table
// of function pointers is configured that is used by a common
// GETSTATE/SETSTATE worker to lookup their entry and invoke
// them, handling any errors and message replies.
//
// Note: Separate tables are used rather than a table of structs
// to keep it simple for compilers which use special attributes to
// place initialized data in code space. It also improves byte packing
// of the data values without customizing compiler pack() settings.
//

const char* const wifi_string_table[] PROGMEM =
{
  dweet_wifi_ssid_string,
  dweet_wifi_password_string,
  dweet_wifi_channel_string,
  dweet_wifi_power_timer_string,
  dweet_wifi_attention_string,
  dweet_wifi_options_string
};

// Locally typed version of state dispatch function
typedef int (DweetWiFi::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod wifi_function_table[] =
{
    &DweetWiFi::WiFiSSID,
    &DweetWiFi::WiFiPassword,
    &DweetWiFi::WiFiChannel,
    &DweetWiFi::WiFiPower,
    &DweetWiFi::WiFiAttention,
    &DweetWiFi::WiFiOptions
};


//
// These are defined in MenloConfigStore.h
//
PROGMEM const int wifi_index_table[] =
{
    WIFI_SSID_INDEX,
    WIFI_PASSWORD_INDEX,
    WIFI_CHANNEL_INDEX,
    WIFI_POWER_TIMER_INDEX,
    0,              // WIFI_ATTENTION does not support config operations
    0               // WIFI_OPTIONS is a placeholder right now
};

//
// The size table contains the length of the strings
// both in the configuration store, and for the runtime
// set/get state property functions.
//
// These sizes include the '\0' on the end.
//
PROGMEM const int wifi_size_table[] =
{
    WIFI_SSID_SIZE,
    WIFI_PASSWORD_SIZE,
    WIFI_CHANNEL_SIZE,
    WIFI_POWER_TIMER_SIZE,
    WIFI_ATTENTION_SIZE,
    WIFI_OPTIONS_SIZE
};

//
// Improve: Change the contracts to a single function per entry
// passing isSet to the wifi object itself.
//
// Then we can have the m_wifi object invoked directly and
// no longer need these stubs.
//

//
// Each Set/Get state entry has a dispatch funcion
//
// For isSet == false the buffer is written to with the returned
// value.
//
// For isSet == true, the buffer supplies the new value.
//

int
DweetWiFi::WiFiSSID(char* buf, int size, bool isSet)
{
    return m_wifi->WiFiSSID(buf, size, isSet);
}

int
DweetWiFi::WiFiPassword(char* buf, int size, bool isSet)
{
    return m_wifi->WiFiPassword(buf, size, isSet);
}

int
DweetWiFi::WiFiChannel(char* buf, int size, bool isSet)
{
    return m_wifi->WiFiChannel(buf, size, isSet);
}

int
DweetWiFi::WiFiPower(char* buf, int size, bool isSet)
{
    return m_wifi->WiFiPower(buf, size, isSet);
}

int
DweetWiFi::WiFiAttention(char* buf, int size, bool isSet)
{
    return m_wifi->WiFiAttention(buf, size, isSet);
}

int
DweetWiFi::WiFiOptions(char* buf, int size, bool isSet)
{
    return m_wifi->WiFiOptions(buf, size, isSet);
}

//
// WiFi commands processing.
//
// These commands allow Dweet access to channel, operating modes,
// power level, internal registers, etc.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetWiFi::ProcessWiFiCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;
    char workingBuffer[WIFI_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(wifi_string_table) / sizeof(char*);

    DBG_PRINT("DweetWiFi calling table processor");

    //
    // GETSTATE/SETSTATE, GETCONFIG/SETCONFIG are handled
    // by a common table driven function.
    //

    //
    // We dispatch on "this" because the method is part of the current
    // class instance as this function performs the specialization
    // required for DweetWiFi.
    //

    parms.ModuleName = (PGM_P)wifi_module_name_string;
    parms.stringTable = (PGM_P)wifi_string_table;
    parms.functionTable = (PGM_P)wifi_function_table;
    parms.defaultsTable = NULL;
    parms.object =  this;
    parms.indexTable = wifi_index_table;
    parms.sizeTable = wifi_size_table;
    parms.tableEntries = tableEntries;
    parms.workingBuffer = workingBuffer;

    // WIFI_CHECKSUM is defined in MenloConfigStore.h
    parms.checksumIndex = WIFI_CHECKSUM;
    parms.checksumBlockStart = WIFI_CHECKSUM_BEGIN;
    parms.checksumBlockSize = WIFI_CHECKSUM_END - WIFI_CHECKSUM_BEGIN;

    parms.name = name;
    parms.value = value;

    // DweetState.cpp
    return dweet->ProcessStateCommandsTable(&parms);
}

//
// Initializing DweetWiFi registers to receive unhandled
// Dweets so they may be examined to see if they are WiFi
// Dweet requests.
//
// If so the supplied Menlo WiFi argument is used to
// perform operations on the interface.
//
int
DweetWiFi::Initialize(MenloDweet* dweet, MenloWiFi* wifi)
{
  int result;
  struct StateSettingsParameters parms;
  char workingBuffer[WIFI_MAX_SIZE+1]; // Must be larger than any config values we fetch

  int tableEntries = sizeof(wifi_string_table) / sizeof(char*);

  m_dweet = dweet;
  m_wifi = wifi;

  //
  // Register for unhandled Dweets arriving on any transport.
  //
  m_dweetEvent.object = this;
  m_dweetEvent.method = (MenloEventMethod)&DweetWiFi::DweetEvent;

  MenloDweet::RegisterGlobalUnhandledDweetEvent(&m_dweetEvent);

  //
  // Load the configuration settings from EEPROM if valid
  //
  // Note: "this" is used to refer to this class (DweetWiFi) since
  // the handlers are on this class.
  //

  parms.ModuleName = (PGM_P)wifi_module_name_string;
  parms.stringTable = (PGM_P)wifi_string_table;
  parms.functionTable = (PGM_P)wifi_function_table;
  parms.defaultsTable = NULL;
  parms.object =  this;
  parms.indexTable = wifi_index_table;
  parms.sizeTable = wifi_size_table;
  parms.tableEntries = tableEntries;
  parms.workingBuffer = workingBuffer;
  parms.checksumIndex = WIFI_CHECKSUM;
  parms.checksumBlockStart = WIFI_CHECKSUM_BEGIN;
  parms.checksumBlockSize = WIFI_CHECKSUM_END - WIFI_CHECKSUM_BEGIN;
  parms.name = NULL;
  parms.value = NULL;

  // DweetState.cpp
  result = MenloDweet::LoadConfigurationSettingsTable(&parms);
  if (result != 0) {
      if (result == DWEET_INVALID_CHECKSUM) {
          MenloDebug::Print(F("DweetWiFi Stored settings checksum is invalid"));
      }
      else {
          MenloDebug::Print(F("DweetWiFi Stored settings are invalid"));
          xDBG_PRINT_NNL("result is ");
          xDBG_PRINT_INT(result);
      }
  }
  else {
      MenloDebug::Print(F("DweetWiFi Stored settings are valid"));
  }

  return 1;
}

//
// This event is a listener for Dweets to see if any are targeted
// to this module.
//
// This allows composition of Dweet handlers through registered events
// vrs. class relationships.
//
// Sketch.ino
//   DweetSerialChannel::ProcessSerialInput() // this owns the input buffer
//     MenloDweet::DispatchMessage(buffer, length)
//       MenloDweet::ProcessDweetCommands()
//         MenloDweet::ProcessDweetCommand()
//           MenloDweet::DispatchDweetCommand()
//             MenloDweet::ProcessUnrecognizedCommand()
//               MenloDweet::EmitUnhandledDweetEvent()
//                 DweetWiFi::DweetEvent()
//                   DweetWiFi::ProcessWiFiCommands()
//
unsigned long
DweetWiFi::DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
  MenloDweetEventArgs* dweetArgs = (MenloDweetEventArgs*)eventArgs;

  // Check memory
  MenloMemoryMonitor::CheckMemory(LineNumberBaseDweetWiFi + __LINE__);

  DBG_PRINT("DweetWiFi DweetEvent");

  if (ProcessWiFiCommands(dweetArgs->dweet, dweetArgs->name, dweetArgs->value) == 0) {
    // Not handled
    DBG_PRINT("DweetWiFi DweetEvent NOT HANDLED");
    return MAX_POLL_TIME;
  }
  else {
    // handled
    DBG_PRINT("DweetWiFi DweetEvent WAS HANDLED");
    return 0;
  }
}
