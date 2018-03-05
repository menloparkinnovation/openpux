
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

#define CONFIG_PIN_SUPPORT 0

MenloConfigStore ConfigStore;

// Constructor
MenloConfigStore::MenloConfigStore()
{
#if MENLOCONFIGSTORE_DYNAMIC_MODEL
    m_headerCacheValid = false;
    m_blockCacheValid = false;
#endif

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

  DBG_PRINT_NNL("idx ");
  DBG_PRINT_INT_NNL(configIndex);
  DBG_PRINT_NNL(" l ");
  DBG_PRINT_INT(length);

  checksum = CalculateCheckSumRange(configIndex, length);

  // Does not NULL terminate the buffer
  MenloUtility::UInt8ToHexBuffer(checksum, &buf[0]);

  DBG_PRINT_NNL("config write checksum ");
  DBG_PRINT_INT_NNL(checksum);

  DBG_PRINT_NNL(" buf[0] ");
  DBG_PRINT_INT_NNL(buf[0]);

  DBG_PRINT_NNL(" buf[1] ");
  DBG_PRINT_INT(buf[1]);

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
     xDBG_PRINT_NNL("chksum low is bad sb ");
     xDBG_PRINT_INT_NNL(checksumBuf[0]);
     xDBG_PRINT_NNL(" is ");
     xDBG_PRINT_INT(buf[0]);
     return false;
  }

  if (checksumBuf[1] != buf[1]) {
     xDBG_PRINT_NNL("chksum high is bad sb ");
     xDBG_PRINT_INT_NNL(checksumBuf[1]);
     xDBG_PRINT_NNL(" is ");
     xDBG_PRINT_INT(buf[1]);
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

//
// Characters allowed are the characters that can be data
// values in a Dweet configuration setting.
//
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

    // Allow URL and path separators
    if (c == '/') return true;

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
// Characters allowed are the characters that can be data
// values in a Dweet configuration setting.
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
            xDBG_PRINT("invalid char");
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
    xDBG_PRINT("pin locked");
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

#if MENLOCONFIGSTORE_DYNAMIC_MODEL

//
// This is the new dynamic EEPROM storage allocation model.
//
// This makes it easier to compose application modules.
//

//
// Validate the table header.
//
// Caches important entries on success to speed future lookups.
//
bool
MenloConfigStore::ValidateTableHeader(uint8_t* error)
{
    uint8_t low;
    uint8_t high;
    uint8_t checksum;
    uint8_t checksumBytes;
    uint8_t calculated_checksum;

    if (m_headerCacheValid) {
        return true;
    }

    checksum = eeprom_read_byte((const uint8_t*)ALLOCATION_TABLE_CHECKSUM);

    m_cachedNumberOfEntries = eeprom_read_byte((const uint8_t*)ALLOCATION_TABLE_SIZE);

    // Calculate the proposed number of entries
    checksumBytes = m_cachedNumberOfEntries * ALLOCATION_TABLE_ENTRY_SIZE;
    checksumBytes += ALLOCATION_TABLE_CHECKSUM_BYTES;

    // Range check it for maximum eeprom size
    if (checksumBytes > MAX_EEPROM_SIZE) {
        DBG_PRINT("ValidataTableHeader: invalid table header, size out of EEPROM range");
        *error = CONFIG_STORE_INVALID_SIZE;
        return false;
    }

    // Checksum it. We start just after the checksum itself.
    calculated_checksum = CalculateCheckSumRange(ALLOCATION_TABLE_CHECKSUM_BEGIN, checksumBytes);

    if (calculated_checksum != checksum) {
        DBG_PRINT("ValidataTableHeader: invalid table header, bad checksum");
        *error = CONFIG_STORE_TABLE_BAD_CHECKSUM;
        return false;
    }

    //
    // Checksum is good. Load the cache and mark it as valid.
    //
    
    low = eeprom_read_byte((const uint8_t*)ALLOCATION_TABLE_ENTRY_ADDRESS_LOW);
    high = eeprom_read_byte((const uint8_t*)ALLOCATION_TABLE_ENTRY_ADDRESS_HIGH);

    m_cachedAllocationPointer = ((high << 8) | low);

    m_headerCacheValid = true;

    DBG_PRINT("ValidataTableHeader: valid header, cache loaded");

    return true;
}

//
// Initialize the table clearing all existing entries.
//
bool
MenloConfigStore::InitializeTable(uint8_t* error)
{
    uint8_t tmp;
    uint16_t allocationptr;

    DBG_PRINT("MenloConfigStore::InitializeTable: Initializating table");

    m_headerCacheValid = false;
    m_blockCacheValid = false;

    // mark 0 entries.
    eeprom_write_byte((uint8_t*)ALLOCATION_TABLE_SIZE, 0);

    // Allocation pointer is just below maximum EEPROM space
    allocationptr = MAX_EEPROM_SIZE - 1;

    tmp = allocationptr & 0x00FF;
    eeprom_write_byte((uint8_t*)ALLOCATION_TABLE_ENTRY_ADDRESS_LOW, tmp);

    tmp = (allocationptr >> 8) & 0x00FF;
    eeprom_write_byte((uint8_t*)ALLOCATION_TABLE_ENTRY_ADDRESS_HIGH, tmp);

    // Calculate the checksum and write it
    tmp = CalculateCheckSumRange(ALLOCATION_TABLE_CHECKSUM_BEGIN, ALLOCATION_TABLE_CHECKSUM_BYTES);

    eeprom_write_byte((uint8_t*)ALLOCATION_TABLE_CHECKSUM, tmp);

    // Now use the validation routine to load and cache it.
    return ValidateTableHeader(error);
}

//
// Lookup the address for the data block
//
uint16_t
MenloConfigStore::GetBlockEntry(
    uint8_t allocationid,
    uint16_t* returnedBlockSize,
    uint8_t* error
    )
{
    uint8_t low;
    uint8_t high;
    uint8_t tmp;
    uint16_t ptr;
    uint16_t blockPtr;
    uint16_t blockSize;
    int index;
    uint8_t checksum;
    uint8_t calculated_checksum;

    //
    // If the header cache is invalid, load the header.
    //
    if (!m_headerCacheValid) {

        xDBG_PRINT("GetBlockEntry: cache invalid, loading table header");

        if (!ValidateTableHeader(error)) {
            return CONFIG_STORE_ERROR;
        }
    }

    // See if its a cache hit
    if (m_blockCacheValid && (m_cachedAllocationid == allocationid)) {
        *returnedBlockSize = m_cachedBlockSize;
        return m_cachedBlockAddress;
    }

    //
    // Perform a search for the allocation id
    //

    // Point to the start of the allocation entries
    ptr = ALLOCATION_TABLE_HEADER_SIZE;

    for (index = 0; index < m_cachedNumberOfEntries; index++) {
        tmp = eeprom_read_byte((const uint8_t*)(ptr + ALLOCATION_TABLE_ENTRY_ALLOCATION_ID));

        if (tmp == allocationid) {

            //
            // Get the blocks address and size
            //

            tmp = eeprom_read_byte((const uint8_t*)(ptr + ALLOCATION_TABLE_ENTRY_ALLOCATION_SIZE));
            blockSize = (tmp >> ALLOCATION_TABLE_ENTRY_SIZE_SHIFT);

            low = eeprom_read_byte((const uint8_t*)(ptr + ALLOCATION_TABLE_ENTRY_ADDRESS_LOW));
            high = eeprom_read_byte((const uint8_t*)(ptr + ALLOCATION_TABLE_ENTRY_ADDRESS_HIGH));

            blockPtr = ((high << 8) | low);

            // Load cached entry and return
            m_cachedBlockAddress = blockPtr;
            m_cachedBlockSize = blockSize;
            m_cachedIndex = index;
            m_cachedAllocationid = allocationid;

            m_blockCacheValid = true;

            *returnedBlockSize = m_cachedBlockSize;

            // This points to the blocks checksum
            return m_cachedBlockAddress;
        }

        // Go to the next entry
        ptr += ALLOCATION_TABLE_ENTRY_SIZE;
    }

    *error = CONFIG_STORE_BLOCK_NOT_FOUND;
    return CONFIG_STORE_ERROR;
}

uint16_t
MenloConfigStore::AllocateBlockEntry(uint8_t allocationid, uint16_t size, uint8_t* error)
{
    uint8_t sizeShifted;
    uint8_t checksum;
    uint8_t resultError;
    uint16_t blockPtr;
    uint16_t blockSize;
    uint16_t index;
    uint16_t tableEntryPtr;
    uint16_t tableEndPtr;
    uint16_t newAllocationPointer;

    //
    // If the header cache is invalid, load the header.
    //
    if (!m_headerCacheValid) {

        xDBG_PRINT("AllocateBlockEntry: cache invalid, loading table header");

        if (!ValidateTableHeader(error)) {
            return CONFIG_STORE_ERROR;
        }
    }

    //
    // This saves a bunch of branch instructions in the if's.
    // It at least makes the code read cleaner.
    //
    uint8_t errorScratch;

    if (error == NULL) {
        error = &errorScratch;
    }

    if (size < 2) {
        *error = CONFIG_STORE_INVALID_SIZE;
        return CONFIG_STORE_ERROR;
    }

    // Round up size to our allocation chunk size
    size += (ALLOCATION_TABLE_ENTRY_MINIMUM_SIZE - 1);
    size = size & ~(ALLOCATION_TABLE_ENTRY_MINIMUM_SIZE - 1);

    // Ensure its not already allocated
    blockPtr = GetBlockEntry(allocationid, &blockSize, &resultError);
    if (blockPtr != CONFIG_STORE_ERROR) {

        //
        // We fail in case they asked for a larger size than already
        // present. Otherwise we would hide an overwrite bug in the
        // caller. So its best to fail them and they can figure out
        // what is going on.
        //
        *error = CONFIG_STORE_ALLREADY_ALLOCATED;
        return CONFIG_STORE_ERROR;
    }

    // Calculate the table end pointer
    tableEndPtr = ALLOCATION_TABLE_HEADER_SIZE;
    tableEndPtr += (m_cachedNumberOfEntries + 1) * ALLOCATION_TABLE_ENTRY_SIZE;

    // See if the new allocation request will fit
    newAllocationPointer = m_cachedAllocationPointer;
    newAllocationPointer -= size;

    //
    // The configuration store if full if the new allocations
    // of the table entry and data block overlap
    //
    if (newAllocationPointer <= tableEndPtr) {
        *error = CONFIG_STORE_FULL;
        return CONFIG_STORE_ERROR;
    }

    // Calculate the pointer to the new entry
    tableEntryPtr = ALLOCATION_TABLE_HEADER_SIZE;
    tableEntryPtr += (m_cachedNumberOfEntries) * ALLOCATION_TABLE_ENTRY_SIZE;

    sizeShifted = size >> ALLOCATION_TABLE_ENTRY_SIZE_SHIFT;

    eeprom_write_byte((uint8_t*)(tableEntryPtr + ALLOCATION_TABLE_ENTRY_ALLOCATION_ID), allocationid);
    eeprom_write_byte((uint8_t*)(tableEntryPtr + ALLOCATION_TABLE_ENTRY_ALLOCATION_SIZE), sizeShifted);

    eeprom_write_byte(
        (uint8_t*)(tableEndPtr + ALLOCATION_TABLE_ENTRY_ADDRESS_LOW),
        newAllocationPointer & 0xFF
        );

    eeprom_write_byte(
        (uint8_t*)(tableEndPtr + ALLOCATION_TABLE_ENTRY_ADDRESS_HIGH),
        (newAllocationPointer >> 8) & 0xFF
        );

    // Recalculate header checksum
    checksum = CalculateCheckSumRange(ALLOCATION_TABLE_CHECKSUM_BEGIN, tableEndPtr);
    eeprom_write_byte((uint8_t*)ALLOCATION_TABLE_CHECKSUM, checksum);

    // Invalidate the caches so they will be reloaded
    m_headerCacheValid = false;
    m_blockCacheValid = false;

    // Now clear the block area including the checksum
    eeprom_write_byte((uint8_t*)(newAllocationPointer + ALLOCATION_BLOCK_CHECKSUM), 0);

    // Note: This skips the checksum.
    for (index = ALLOCATION_BLOCK_DATA_START; index < size; index++) {
        eeprom_write_byte((uint8_t*)(newAllocationPointer + index), 0);
    }

    //
    // Note: The newly allocated data area is left with its checksum invalid
    // since it has no valid contents yet until set by the caller.
    //

    return newAllocationPointer;
}

bool
MenloConfigStore::ValidateBlockEntryChecksum(uint8_t allocationid, uint8_t* error)
{
    uint16_t blockPtr;
    uint16_t blockSize;
    uint8_t checksum;
    uint8_t calculated_checksum;

    blockPtr = GetBlockEntry(allocationid, &blockSize, error);
    if (blockPtr == CONFIG_STORE_ERROR) {
        return false;
    }

    calculated_checksum =
        CalculateCheckSumRange(blockPtr + ALLOCATION_BLOCK_DATA_START, blockSize - 1);

    checksum = eeprom_read_byte((const uint8_t*)(blockPtr + ALLOCATION_BLOCK_CHECKSUM));

    if (calculated_checksum != checksum) {
        *error = CONFIG_STORE_BLOCK_BAD_CHECKSUM;
        return false;
    }

    return true;
}

//
// Recalculate the checksum for an application id's block after it
// has made a series of updates.
//
// To prevent mistakes the entry is looked up in the table
//
bool
MenloConfigStore::UpdateBlockEntryChecksum(uint8_t allocationid, uint8_t* error)
{
    uint8_t checksum;
    uint16_t blockPtr;
    uint16_t blockSize;

    //
    // This does not impact any of the caches
    //

    blockPtr = GetBlockEntry(allocationid, &blockSize, error);
    if (blockPtr == CONFIG_STORE_ERROR) {
        return false;
    }

    checksum = CalculateCheckSumRange(blockPtr + ALLOCATION_BLOCK_DATA_START, blockSize - 1);

    eeprom_write_byte((uint8_t*)(blockPtr + ALLOCATION_BLOCK_CHECKSUM), checksum);

    return true;
}

#endif // MENLOCONFIGSTORE_DYNAMIC_MODEL
