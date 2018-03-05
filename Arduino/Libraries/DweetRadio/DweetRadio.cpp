
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
 * Smartpux (TM) Dweet support.
 *
 *  Date: 02/22/2015
 *  File: DweetRadio.cpp
 *
 * Used for Smartpux DWEET's.
 */

#include "MenloPlatform.h"
#include "MenloMemoryMonitor.h"
#include "MenloDebug.h"
#include "MenloUtility.h"
#include "MenloNMEA0183.h"
#include "MenloConfigStore.h"
#include "MenloDweet.h"
#include "DweetRadio.h"

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

// 32 bytes radio packet is pretty standard with nRF24L01+
#define RADIO_PACKET_SIZE 32

#define RADIO_SEND_TIMEOUT 500

const char radio_module_name_string[] PROGMEM = "DweetRadio";

extern const char dweet_radio_channel_string[] PROGMEM = "RADIOCHANNEL";

extern const char dweet_radio_txaddr_string[] PROGMEM = "RADIOTXADDR";

extern const char dweet_radio_rxaddr_string[] PROGMEM = "RADIORXADDR";

extern const char dweet_radio_power_timer_string[] PROGMEM = "RADIOPOWERTIMER";

extern const char dweet_radio_attention_string[] PROGMEM = "RADIOATT";

extern const char dweet_radio_options_string[] PROGMEM = "RADIOOPTIONS";

extern const char dweet_radio_gateway_string[] PROGMEM = "RADIOGATEWAY";

//
// DweetRadio provides an example of a common pattern used
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

const char* const radio_string_table[] PROGMEM =
{
  dweet_radio_channel_string,
  dweet_radio_txaddr_string,
  dweet_radio_rxaddr_string,
  dweet_radio_power_timer_string,
  dweet_radio_attention_string,
  dweet_radio_options_string,
  dweet_radio_gateway_string
};

// Locally typed version of state dispatch function
typedef int (DweetRadio::*StateMethod)(char* buf, int size, bool isSet);

PROGMEM const StateMethod radio_function_table[] =
{
    &DweetRadio::ChannelHandler,
    &DweetRadio::TxAddrHandler,
    &DweetRadio::RxAddrHandler,
    &DweetRadio::PowerHandler,
    &DweetRadio::AttentionHandler,
    &DweetRadio::OptionsHandler,
    &DweetRadio::GatewayHandler
};


//
// These are defined in MenloConfigStore.h
//
PROGMEM const int radio_index_table[] =
{
    RADIO_CHANNEL,
    RADIO_TXADDR,
    RADIO_RXADDR,
    RADIO_POWER_TIMER,
    0,              // RADIO_ATTENTION does not support config operations
    0,              // RADIO_OPTIONS is a placeholder right now
    GATEWAY_ENABLED
};

//
// The size table contains the length of the strings
// both in the configuration store, and for the runtime
// set/get state property functions.
//
// These sizes include the '\0' on the end.
//
PROGMEM const int radio_size_table[] =
{
    RADIO_CHANNEL_SIZE,
    RADIO_TXADDR_SIZE,
    RADIO_RXADDR_SIZE,
    RADIO_POWER_TIMER_SIZE,
    RADIO_ATTENTION_SIZE,
    RADIO_OPTIONS_SIZE,
    GATEWAY_ENABLED_SIZE
};

//
// Improve: Change the contracts to a single function per entry
// passing isSet to the radio object itself.
//
// Then we can have the m_radio object invoked directly and
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
DweetRadio::ChannelHandler(char* buf, int size, bool isSet)
{
    return m_radio->Channel(buf, size, isSet);
}

int
DweetRadio::TxAddrHandler(char* buf, int size, bool isSet)
{
    return m_radio->TxAddr(buf, size, isSet);
}

int
DweetRadio::RxAddrHandler(char* buf, int size, bool isSet)
{
    return m_radio->RxAddr(buf, size, isSet);
}

int
DweetRadio::PowerHandler(char* buf, int size, bool isSet)
{
    return m_radio->Power(buf, size, isSet);
}

int
DweetRadio::AttentionHandler(char* buf, int size, bool isSet)
{
    return m_radio->Attention(buf, size, isSet);
}

int
DweetRadio::OptionsHandler(char* buf, int size, bool isSet)
{
    return m_radio->Options(buf, size, isSet);
}

int
DweetRadio::GatewayHandler(char* buf, int size, bool isSet)
{
    int status = 0;

    if (isSet) {
        if (strcmp_P(buf, dweet_on_string) == 0) {
            EnableReceiveDweetStream(true);
            MenloDebug::Print(F("RadioGateway is ON"));
        }
        else if (strcmp_P(buf, dweet_off_string) == 0) {
            EnableReceiveDweetStream(false);
            MenloDebug::Print(F("RadioGateway is OFF"));
        }
        else {
            status = DWEET_INVALID_PARAMETER;
        }
    }
    else {

        buf[0] = 'O';
        if (m_receiveDweetStreamEnabled) {
            buf[1] = 'N';
            buf[2] = '\0';
        }
        else {
	    buf[1] = 'F';
	    buf[2] = 'F';
            buf[3] = '\0';
        }
    }

    return status;
}

//
// Radio commands processing.
//
// These are commands for small packet radios.
//
// These commands allow Dweet access to channel, operating modes,
// power level, internal registers, etc.
//
// Returns 1 if the command is recognized.
// Returns 0 if not.
//
int
DweetRadio::ProcessRadioCommands(MenloDweet* dweet, char* name, char* value)
{
    struct StateSettingsParameters parms;
    char workingBuffer[RADIO_MAX_SIZE+1]; // Must be larger than any config values we fetch

    int tableEntries = sizeof(radio_string_table) / sizeof(char*);

    //
    // This is the function specialized to DweetRadio.
    //
    // It first handles any custom Dweet commands that do not
    // fit within the table driven worker function.
    //
    // For everything else it invokes the table driver worker
    // function with the locally defined table pointers.
    //

    if (strncmp_P(name, PSTR("T"), 1) == 0)  {

        DBG_PRINT("DweetRadio: T command received");

        //
        // T=c:000...
        //
        // where c is channel
        // 000... data to send up to 32 bytes as 64 chars
        //
        return ProcessRadioTransmit(dweet, name, value);
    }
    else if (strncmp_P(name, PSTR("R"), 1) == 0)  {

        DBG_PRINT("DweetRadio: R command received");

        //
        // R=c:000...
        //
        // where c is channel
        // 000... data received up to 32 bytes as 64 chars
        //

        //
        // Note: Currently this embedded firmware version generates
        // radio receive Dweet's but does not handle them itself.
        //

        // Continue looking for a handler so the app can handle it.
        return 0;
    }
    else {

        DBG_PRINT("DweetRadio calling table processor");

        //
        // GETSTATE/SETSTATE, GETCONFIG/SETCONFIG are handled
        // by a common table driven function.
        //

        //
        // We dispatch on "this" because the method is part of the current
        // class instance as this function performs the specialization
        // required for DweetRadio.
        //

        parms.ModuleName = (PGM_P)radio_module_name_string;
        parms.stringTable = (PGM_P)radio_string_table;
        parms.functionTable = (PGM_P)radio_function_table;
        parms.defaultsTable = NULL;
        parms.object =  this;
        parms.indexTable = radio_index_table;
        parms.sizeTable = radio_size_table;
        parms.tableEntries = tableEntries;
        parms.workingBuffer = workingBuffer;

        // RADIO_CHECKSUM is defined in MenloConfigStore.h
        parms.checksumIndex = RADIO_CHECKSUM;
        parms.checksumBlockStart = RADIO_CHECKSUM_BEGIN;
        parms.checksumBlockSize = RADIO_CHECKSUM_END - RADIO_CHECKSUM_BEGIN;

        parms.name = name;
        parms.value = value;

        // DweetState.cpp
        return dweet->ProcessStateCommandsTable(&parms);
    }
}

//
// Initializing DweetRadio registers to receive unhandled
// Dweets so they may be examined to see if they are Radio
// Dweet requests.
//
// If so the supplied Menlo Radio argument is used to
// perform operations on the radio.
//
int
DweetRadio::Initialize(MenloDweet* dweet, MenloRadio* radio)
{
  int result;
  struct StateSettingsParameters parms;
  char workingBuffer[RADIO_MAX_SIZE+1]; // Must be larger than any config values we fetch

  int tableEntries = sizeof(radio_string_table) / sizeof(char*);

  m_dweet = dweet;
  m_radio = radio;
  m_receiveDweetStreamEnabled = false;

  //
  // Register for unhandled Dweets arriving on any transport.
  //
  m_dweetEvent.object = this;
  m_dweetEvent.method = (MenloEventMethod)&DweetRadio::DweetEvent;

  MenloDweet::RegisterGlobalUnhandledDweetEvent(&m_dweetEvent);

  //
  // By default received radio packets are not forwarded
  // as unsolicited Dweet's unless EnableReceiveDweetStream() is called.
  //

  //
  // Load the configuration settings from EEPROM if valid
  //
  // Note: "this" is used to refer to this class (DweetRadio) since
  // the handlers are on this class.
  //
  // Improve: These stubs can be eliminated if Gateway is handled
  // by MenloRadio and then "this" can be m_radio and allow
  // direct invoke. But gateway is a function transparent to
  // MenloRadio itself as its implemented at the Dweet level.
  //

  parms.ModuleName = (PGM_P)radio_module_name_string;
  parms.stringTable = (PGM_P)radio_string_table;
  parms.functionTable = (PGM_P)radio_function_table;
  parms.defaultsTable = NULL;
  parms.object =  this;
  parms.indexTable = radio_index_table;
  parms.sizeTable = radio_size_table;
  parms.tableEntries = tableEntries;
  parms.workingBuffer = workingBuffer;
  parms.checksumIndex = RADIO_CHECKSUM;
  parms.checksumBlockStart = RADIO_CHECKSUM_BEGIN;
  parms.checksumBlockSize = RADIO_CHECKSUM_END - RADIO_CHECKSUM_BEGIN;
  parms.name = NULL;
  parms.value = NULL;

  // DweetState.cpp
  result = MenloDweet::LoadConfigurationSettingsTable(&parms);
  if (result != 0) {
      if (result == DWEET_INVALID_CHECKSUM) {
          MenloDebug::Print(F("DweetRadio Stored settings checksum is invalid"));
      }
      else {
          MenloDebug::Print(F("DweetRadio Stored settings are invalid"));
          xDBG_PRINT_NNL("result is ");
          xDBG_PRINT_INT(result);
      }
  }
  else {
      MenloDebug::Print(F("DweetRadio Stored settings are valid"));
  }

  return 1;
}

//
// This enables the Dweet stream for received radio packets.
//
void
DweetRadio::EnableReceiveDweetStream(bool value)
{
  if (value) {
      if (m_receiveDweetStreamEnabled) {
          return;
      }

      m_receiveDweetStreamEnabled = true;

      //
      // Register for radio receive packets
      //
      m_radioEvent.object = this;
      m_radioEvent.method = (MenloEventMethod)&DweetRadio::RadioEvent;
      m_radio->RegisterReceiveEvent(&m_radioEvent);

      DBG_PRINT("DweetRadio ReceiveDweetStream enabled");
  }
  else {
      if (!m_receiveDweetStreamEnabled) {
          return;
      }

      m_receiveDweetStreamEnabled = false;
      m_radio->UnregisterReceiveEvent(&m_radioEvent);

      DBG_PRINT("DweetRadio ReceiveDweetStream +not+ enabled");
  }
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
//                 DweetRadio::DweetEvent()
//                   DweetRadio::ProcessRadioCommands()
//
unsigned long
DweetRadio::DweetEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
  MenloDweetEventArgs* dweetArgs = (MenloDweetEventArgs*)eventArgs;

  // Check memory
  MenloMemoryMonitor::CheckMemory(LineNumberBaseDweetRadio + __LINE__);

  DBG_PRINT("DweetRadio DweetEvent");

  if (ProcessRadioCommands(dweetArgs->dweet, dweetArgs->name, dweetArgs->value) == 0) {
    // Not handled
    DBG_PRINT("DweetRadio DweetEvent NOT HANDLED");
    return MAX_POLL_TIME;
  }
  else {
    // handled
    DBG_PRINT("DweetRadio DweetEvent WAS HANDLED");
    return 0;
  }
}

//
// This event occurs when a radio packet is available
//
unsigned long
DweetRadio::RadioEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs)
{
  MenloRadioEventArgs* radioArgs = (MenloRadioEventArgs*)eventArgs;

  DBG_PRINT("DweetRadio: RadioEvent");

  // Check memory
  MenloMemoryMonitor::CheckMemory(LineNumberBaseDweetRadio + __LINE__);

  // Process incoming radio packet
  SendRadioReceiveDweet('0', radioArgs->data, radioArgs->dataLength);

  return MAX_POLL_TIME;
}

//
// Process a radio transmission Dweet.
//
// Up to 32 bytes represented as 64 ASCII hex characters.
//
// NMEA 0183 payload limit is 71 characters.
//
//  4 +  64 characters == 68 characters
//
// $PDWT,T=0:0000...*00\r\n
//
bool
DweetRadio::ProcessToRadioBuffer(
    char* str,
    uint8_t* channel,
    uint8_t* radioBuffer,
    uint8_t radioBufferLength
    )
{
    int index;
    int size = strlen(str);

    // ensure its zero'd
    memset(radioBuffer, 0, radioBufferLength);

    // T=0:0000...

    // Must be at least "0:00"
    if (size < 4) return false;

    *channel = str[0];
    
    str++; // consumes 1 character
    if (*str != ':') return false;
    
    // skip separator ':'
    str++;

    // Remaining string must be even size
    size = strlen(str);
    if ((size % 2) != 0) {
      return false;
    }

    // Can't exceed radio buffer size
    if ((size / 2) > radioBufferLength) return false;

    // Initialize light sequence buffer
    for (index = 0; index < radioBufferLength; index++) {
        radioBuffer[index] = 0;
    }

    // Decode ASCII hex into binary bytes
    index = 0;

    while (*str != '\0') {

        radioBuffer[index] = MenloUtility::HexToByte(str);
        index++;

        DEBUG_ASSERT((index <= radioBufferLength));

        // Consumes two chars
        str++;
        if (*str == '\0') return false; // actually an error, should be even # of chars
        str++;
    }

    return true;
}

int
DweetRadio::ProcessRadioTransmit(MenloDweet* dweet, char* name, char* value)
{
    int size;
    int status;
    bool result;
    uint8_t channel;
    uint8_t radioBuffer[RADIO_PACKET_SIZE];
    char buf[2];
    PGM_P command = PSTR("T");

    DBG_PRINT("ProcessRadioTransmit");

    //
    //  0: => radio number + separator
    //
    //  0000 => up to 32 byte data payload for the radio
    // T=0:0000...
    //
    result = ProcessToRadioBuffer(value, &channel, radioBuffer, sizeof(radioBuffer));
    if (!result) {

        DBG_PRINT("ProcessRadioTransmit error in Dweet");

        //
        // error in format
        //
        size = dweet->CalculateMaximumValueReply(command, dweet_error_string, value);
        if ((int)strlen(value) > size) {
            // Update in buffer
            value[size] = '\0';
        }

        dweet->SendDweetItemReplyType_P(
	    command,
            dweet_error_string,
            value
            );

        return 1;
    }

    //
    // channel allows multiple radio to be supported.
    //

    DBG_PRINT("Sending packet to radio");

    // Send to the radio
    status = m_radio->Write(
        NULL,
        radioBuffer,
        RADIO_PACKET_SIZE,
        RADIO_SEND_TIMEOUT
     );

    DBG_PRINT("DweetRadio T xxx packet sent");

    // We just reply with the channel
    buf[0] = channel;
    buf[1] = '\0';

    dweet->SendDweetItemReplyType_P(
        command,
        dweet_reply_string,
        buf
        );

    return 1;
}

//
// Send a Radio Receive packet Dweet
//
// We don't expect a reply as the firmware is setup to stream
// received radio packets to the host.
//
void
DweetRadio::SendRadioReceiveDweet(
    uint8_t channel,
    uint8_t* radioBuffer,
    uint8_t radioBufferLength
    )
{
    int index;
    int bufferIndex;
    char buf[67]; // 64 chars for 32 bytes, + 2 for channel, separator, + 1 for null

    DBG_PRINT("DweetRadio SendRadioReceiveDweet R=");

    if (radioBufferLength > RADIO_PACKET_SIZE) {
      DBG_PRINT("Buffer > packet size");
      DEBUG_ASSERT(false);
      return;
    }

    bufferIndex = 0;

    buf[bufferIndex++] = channel; // Channel is not converted, single direct ASCII character code
    buf[bufferIndex++] = ':';

    for (index = 0; index < radioBufferLength; index++) {
      MenloUtility::UInt8ToHexBuffer(radioBuffer[index], &buf[bufferIndex]);

      // Consumes two characters
      bufferIndex += 2;
    }

    buf[bufferIndex] = '\0';

    DBG_PRINT("DweetRadio R xxx packet received");

    // Format is: R=0:xxxx...
    m_dweet->SendDweetItemReplyType_P(
	PSTR("R"),
	PSTR("="), // Don't want _REPLY on it as its a first request
        buf
        );
}
