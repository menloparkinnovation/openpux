
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
 *  Date: 12/26/2013
 *  File: MenloConfigStore.cpp
 */

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//

#include "MenloPlatform.h"
#include "MenloUtility.h"
#include "MenloMemoryMonitor.h"
#include "MenloDebug.h"
#include "MenloConfigStore.h"

#define DBG_PRINT_ENABLED 0

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define DBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define DBG_PRINT_STRING_P(x)  (MenloDebug::Print_P(x))
#define DBG_PRINT_NNL(x)  (MenloDebug::PrintNoNewline(F(x)))
#define DBG_PRINT_INT(x) (MenloDebug::PrintHex(x))
#define DBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define DBG_PRINT(x)
#define DBG_PRINT_STRING(x)
#define DBG_PRINT_STRING_P(x)
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
#define xDBG_PRINT(x)     (MenloDebug::Print(F(x)))
#define xDBG_PRINT_STRING(x)  (MenloDebug::Print(x))
#define xDBG_PRINT_STRING_P(x)  (MenloDebug::Print_P(x))
#define xDBG_PRINT_NNL(x)  (MenloDebug::PrintNoNewline(F(x)))
#define xDBG_PRINT_INT(x) (MenloDebug::PrintHex(x))
#define xDBG_PRINT_INT_NNL(x) (MenloDebug::PrintHexNoNewline(x))
#else
#define xDBG_PRINT(x)
#define xDBG_PRINT_STRING(x)
#define xDBG_PRINT_STRING_P(x)
#define xDBG_PRINT_NNL(x)
#define xDBG_PRINT_INT(x)
#define xDBG_PRINT_INT_NNL(x)
#endif

#define CONFIG_PIN_SUPPORT 1

MenloConfigStore ConfigStore;

// Constructor
MenloConfigStore::MenloConfigStore()
{
  //
  // TOOD: Read pin status while initializing
  // Could use a pin of 0000 as no pin.
  //

  // If checksum is bad could unlock for initial configuration.
  // In production the EEPROM should be initialized to a known good
  // pin, even if its 0000 with a good checksum.
  //
  // There is a slight risk that one attack is to cause a checksum
  // error that causes the device unlock the pin. But this means
  // the external attacker already caused an error in the EEPROM
  // inside the PIN storage block. We are not a high security
  // device such as a smart card, but just keeping someones neighbor
  // from stealing their gadgets.
  //
#if CONFIG_PIN_SUPPORT
  // Note: This kills the sketch during initialization!
  // TODO: Fix!
  //ReadAndValidatePin();
#endif
}

bool
MenloConfigStore::IsPinLocked()
{
#if CONFIG_PIN_SUPPORT
   return m_pinLocked;
#else
   // Always unlocked
   return false;
#endif
}

#if CONFIG_PIN_SUPPORT
void
MenloConfigStore::ReadAndValidatePin()
{
  int i;
  int index;
  uint8_t buf;
  bool result;

  //
  // Validate PIN in EEPROM
  // If set, enforce it
  //
  result = CalculateAndValidateCheckSumRange(
      PIN_CHECKSUM,
      PIN_INDEX,
      PIN_SIZE
      );

  //
  // Bad pin checksum. We return no pin set to keep the device
  // from being locked out. Higher security devices may want to
  // enforce a reflash recovery in this case.
  //
  if (!result) {
     MenloDebug::Print(F("Bad PIN checksum, unlocking"));
     m_pinLocked = false;
     return;
  }

  //
  // If the pin is all zero's, then there is no pin set
  //
  // Since the checksum above was determined to be valid,
  // any non-zero values means that a pin has been set.
  //
  index = PIN_INDEX;
  for (i = 0; i < PIN_SIZE; i++) {
     buf = eeprom_read_byte((const uint8_t*)index);

     if (buf != '0') {
       MenloDebug::Print(F("PIN Set and valid Locking Device"));
       m_pinLocked = true;
       return;
     }
  }

  //
  // Valid checksums, but pin has been set to all 0's, so no pin lock.
  //
  MenloDebug::Print(F("PIN is all zeros, no PIN set"));
  m_pinLocked = false;
}
#endif

bool
MenloConfigStore::UnlockPin(char* pin)
{
#if CONFIG_PIN_SUPPORT
  int i;
  int index;
  uint8_t buf;
  bool result;

  // If already unlocked, just return true
  if (!m_pinLocked) return true;

  // Since m_pinLocked we have already validated the pin checksum in EEPROM
  index = PIN_INDEX;
  for (i = 0; i < PIN_SIZE; i++) {
     buf = eeprom_read_byte((const uint8_t*)index);

     if (buf != pin[i]) {
       // does not match
       return false;
     }

     if (pin[i] == 0) {

       //
       // We reached a NULL. But we have matched NULL in both
       // the supplied pin and the EEPROM. This means the pin is shorter
       // than max pin characters, but is valid since all characters matched
       // to this point.
       //
       // When short pins are saved, the NULL is included in the EEPROM.
       //
       m_pinLocked = false;
       return true;
     }
  }

  // All chars of a maximum length pin matched.
  m_pinLocked = false;
  
  return true;
#else
  // Always unlocked
  return true;
#endif
}

bool
MenloConfigStore::SetNewPin(char* pin)
{
#if CONFIG_PIN_SUPPORT
  bool result;
  int length;

  if (m_pinLocked) {
    // Must have opened with the old pin first
    return false;
  }

  length = strlen(pin);
  if (length >= (PIN_SIZE - 1)) {
    return false;
  }

  ConfigStore.WriteConfig(PIN_INDEX, (uint8_t*)pin, length + 1);

  result = CalculateAndStoreCheckSumRange(
      PIN_CHECKSUM,
      PIN_INDEX,
      PIN_SIZE
      );

  if (!result) {
     // hiccup, bad checksum in EEPROM pin area should prevent lockout.
     return false;
  }

  return true;
#else
  // Not supported
  return false;
#endif
}

//
// Calculates the checksum for a range, and store it at
// the specified location.
//
// Note: It's expected that the checksum store index
// is two bytes. It also does not overlap the range
// so its not included in the calculation.
//
bool
MenloConfigStore::CalculateAndStoreCheckSumRange(
    int checkSumStoreIndex,
    int configIndex,
    int length
    )
{
  char buf[2];
  uint8_t checksum;

  MenloMemoryMonitor::CheckMemory(LineNumberBaseConfigStore + __LINE__);

  if ((configIndex + length) > MAX_EEPROM_SIZE) {
      DBG_PRINT("config write chksum exceeds EEPROM size");
      return false;
  }

  if ((checkSumStoreIndex + 2) > MAX_EEPROM_SIZE) {
      DBG_PRINT("config write chksum exceeds EEPROM size");
      return false;
  }

  checksum = CalculateCheckSumRange(configIndex, length);

  // Does not NULL terminate the buffer
  MenloUtility::UInt8ToHexBuffer(checksum, &buf[0]);

  //DBG_PRINT_NNL("config write checksum [0] ");
  //DBG_PRINT_INT_NNL(buf[0]);

  //DBG_PRINT_NNL(" [1] ");
  //DBG_PRINT_INT(buf[1]);

  eeprom_write_byte((uint8_t*)checkSumStoreIndex, buf[0]);
  eeprom_write_byte((uint8_t*)checkSumStoreIndex + 1, buf[1]);

  return true;
}

//
// Calculates the checksum for a range, and validate that
// it matches the one at the specified location.
//
// Note: It's expected that the checksum store index
// is two bytes. It also does not overlap the range
// so its not included in the calculation.
//
bool
MenloConfigStore::CalculateAndValidateCheckSumRange(
    int checkSumStoreIndex,
    int configIndex,
    int length
    )
{
  char buf[2];
  uint8_t checksumBuf[2];
  uint8_t checksum;

  MenloMemoryMonitor::CheckMemory(LineNumberBaseConfigStore + __LINE__);

  if ((configIndex + length) > MAX_EEPROM_SIZE) {
      xDBG_PRINT("config read chksum exceeds EEPROM size");
      return false;
  }

  if ((checkSumStoreIndex + 2) > MAX_EEPROM_SIZE) {
      xDBG_PRINT("config read chksum exceeds EEPROM size");
      return false;
  }

  checksumBuf[0] = eeprom_read_byte((const uint8_t*)checkSumStoreIndex);
  checksumBuf[1] = eeprom_read_byte((const uint8_t*)checkSumStoreIndex + 1);

  //DBG_PRINT_NNL("config read checksum [0] ");
  //DBG_PRINT_INT_NNL(checksumBuf[0]);

  //DBG_PRINT_NNL(" [1] ");
  //DBG_PRINT_INT(checksumBuf[1]);

  checksum = CalculateCheckSumRange(configIndex, length);

  MenloUtility::UInt8ToHexBuffer(checksum, &buf[0]);

  //DBG_PRINT_NNL("config calc checksum [0] ");
  //DBG_PRINT_INT_NNL(buf[0]);

  //DBG_PRINT_NNL(" [1] ");
  //DBG_PRINT_INT(buf[1]);

  if (checksumBuf[0] != buf[0]) {
     xDBG_PRINT("config read chksum low is bad");
     return false;
  }

  if (checksumBuf[1] != buf[1]) {
     xDBG_PRINT("config read chksum high is bad");
     return false;
  }

  return true;
}

uint8_t
MenloConfigStore::CalculateCheckSumRange(int configIndex, int length)
{
    uint8_t buf;
    int index, configEnd;
    unsigned char chksum = length; // detect zero's

    configEnd = configIndex + length;

    // Return a poison checksum if it exceeds the range
    if (configEnd > MAX_EEPROM_SIZE) {
        DBG_PRINT("ConfigStore calc chksum range exceed EEPROM size");
        return 0xFF;
    }

    index = configIndex;
    for (; index < configEnd; index++) {

        buf = eeprom_read_byte((const uint8_t*)index);

        chksum = (chksum ^ buf);
    }

    return chksum;
}

bool
MenloConfigStore::IsValidConfigChar(char c)
{

    if ((c >= 'a') && (c <= 'z')) return true;
    if ((c >= 'A') && (c <= 'Z')) return true;
    if ((c >= '0') && (c <= '9')) return true;

    // We allow a few special characters.

    // Allow My_Name
    if (c == '_') return true;

    // Allow 1.0, etc.
    if (c == '.') return true;

    //
    // We don't want to allow any Dweet/NMEA 0183 formatting
    // characters, spaces, or anything else that can be
    // confusing for configuration strings, model names, etc.
    //
    // An application is free to map a name to anything
    // it wants in its own database and not force the
    // requirement onto the embedded device.
    //

    return false;
}

//
// This is called for GETCONFIG which could be reading
// corrupted or invalid (reset, factory fresh, etc.) data
// from the EEPROM.
//
// Invalid chars are replaced with ASCII zero '0'
//
void
MenloConfigStore::ProcessConfigBufferForValidChars(char* buf, int len)
{
    int index;

    for (index = 0; index < len; index++) {
        if (!IsValidConfigChar(buf[index])) {

            // null is allowed
            if (buf[index] == '\0') {
	        continue;
            }
            else {
                buf[index] = '0'; // Zero Ascii means not initialized
            }
        }
    }
}

//
// This determines if the config buffer has
// any invalid chars we don't accept into the
// configuration EEPROM.
//
// NULL is allowed as it represents string null
// terminators, or initial state.
//
bool
MenloConfigStore::ConfigBufferHasInvalidChars(char* buf, int len)
{
    int index;

    for (index = 0; index < len; index++) {
        if (!IsValidConfigChar(buf[index])) {

            // null is allowed
  	   if (buf[index] == '\0') {
	     continue;
           }
           else {
            return true;
	   }
        }
    }

    return false;
}

int
MenloConfigStore::ReadConfig(int configIndex, uint8_t* buffer, uint8_t length)
{
  int bufferIndex;
  int eepromIndex;
  uint8_t buf;

  MenloMemoryMonitor::CheckMemory(LineNumberBaseConfigStore + __LINE__);

  bufferIndex = 0;
  eepromIndex = configIndex;

  for (; bufferIndex < length;) {
        buf = eeprom_read_byte((const uint8_t*)eepromIndex);

        buffer[bufferIndex] = buf;

	bufferIndex++;
        eepromIndex++;

        if (eepromIndex > MAX_EEPROM_SIZE) break;
  }

  // amount read may be short if off the end of the EEPROM
  return bufferIndex;
}

int
MenloConfigStore::WriteConfig(int configIndex, uint8_t* buffer, uint8_t length)
{
  int bufferIndex;
  int eepromIndex;
  uint8_t buf;

  // If pin locked fail
  if (IsPinLocked()) {
    return 0;
  }

  bufferIndex = 0;
  eepromIndex = configIndex;

  for (; bufferIndex < length;) {

        buf = buffer[bufferIndex];

        eeprom_write_byte((uint8_t*)eepromIndex, buf);

	bufferIndex++;
        eepromIndex++;

        if (eepromIndex > MAX_EEPROM_SIZE) break;
  }

  // amount written may be short if off the end of the EEPROM
  return bufferIndex;
}
