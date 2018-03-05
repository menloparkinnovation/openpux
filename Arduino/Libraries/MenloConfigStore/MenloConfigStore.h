
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
 *  File: MenloConfigStore.h
 *
 *  Configuration Store Support.
 */

#ifndef MenloConfigStore_h
#define MenloConfigStore_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
//#include <Arduino.h>
//#include <inttypes.h>

// Platforms with more space, or applications that make the tradeoffs
#define CONFIG_PIN_SUPPORT 1

//
// Each configuration store entry is an ASCII string terminated
// by a '\0' terminator. Non-ASCII characters are not allowed.
//
// The size of each entry *includes* this '\0' as it is stored
// in the firmware store.
//

//
// This is supported by AtMega328, Particle Photon, and emulators.
//
// Future: This could be updated to take advantage of larger configuration
// stores, which include updating the size shift used for allocation
// blocks.
//
// In the interests of code portablility across platforms the configuration
// store will manage 1024 bytes of storage, and leave any extra on a given
// platform for local application usage.
//
#define MAX_EEPROM_SIZE 1023

//
// 0 is used to indicate failure since no valid address returned can
// represent it as its the allocation table header.
//

#define CONFIG_STORE_ERROR              0

//
// These are extra error indications returned if the optional
// error indication pointer is supplied.
//

//
// This is returned if the config table is corrupt.
//
#define CONFIG_STORE_INVALID_TABLE       1

//
// This is returned if a given block is not found for the allocation id.
//
#define CONFIG_STORE_BLOCK_NOT_FOUND     2

// This is returned if a block was allocated on a get
#define CONFIG_STORE_BLOCK_ALLOCATED     3

#define CONFIG_STORE_TABLE_BAD_CHECKSUM  4

#define CONFIG_STORE_BLOCK_BAD_CHECKSUM  5

#define CONFIG_STORE_BLOCK_SIZE_MISMATCH 6

#define CONFIG_STORE_FULL                7

#define CONFIG_STORE_ALLREADY_ALLOCATED  8

#define CONFIG_STORE_INVALID_SIZE        9

// **************** New Model ****************

// Set this to 1 to enable the dynamic allocation model
#define MENLOCONFIGSTORE_DYNAMIC_MODEL  0

//
// Layout:
//
// An allocation table exists at the front of the memory space
// while storage is allocated from the top down based on the
// maximum storage size for a given platform.
//
// Application modules or subsystems allocate a block of data
// using a byte sized allocationid which represents the application
// or module identifier. Small embedded systems are unlikely to have
// more than 255 separate application modules.
//
// The allocation table, and each modules data block is protected and
// validated by two byte checksum at the begining of each structure.
//
// The allocation table has a header which contains the number of
// entries, a flags/identifier byte, the allocation pointer, and a
// checksum for the table including its entries.
//

// Little endian layout
struct ConfigStoreAllocationTableHeader {
    uint8_t  checksum;
    uint8_t  size;
    uint16_t allocation_address;
};

#define ALLOCATION_TABLE_CHECKSUM           0
#define ALLOCATION_TABLE_SIZE               1
#define ALLOCATION_TABLE_ADDRESS_LOW_BYTE   2
#define ALLOCATION_TABLE_ADDRESS_HIGH_BYTE  3

// The checksum starts at the first byte after the checksum itself
#define ALLOCATION_TABLE_CHECKSUM_BEGIN ALLOCATION_TABLE_SIZE

// Size of table not including the checksum
#define ALLOCATION_TABLE_CHECKSUM_BYTES     3

#define ALLOCATION_TABLE_HEADER_SIZE        4

//
// An allocation table entry identifies a block allocation of
// configuration memory for a specific subsystem/module identified
// by index.
//
// The entry contains the address of the entry, and its size.
//
// The size is shifted by a value depending on the size of the
// configuration store in order to represent a larger block in
// a single byte. This represents the minimum block allocation
// size.
//
// The allocation table entries are protected by the allocation
// table checksum.
//
// The address is not shifted, so this model supports up to 64k
// of configuration storage.
//
// Future: The address could be shifted as well, easily doubling
// the storage or more. Though the platforms targeted have
// modest amounts of configuration memory storage.
//

struct ConfigStoreAllocationTableEntry {
    uint16_t address;        // address
    uint8_t  allocationsize; // size of allocated block
    uint8_t  allocationid;   // subsystem identifier
};

#define ALLOCATION_TABLE_ENTRY_SIZE 4

#define ALLOCATION_TABLE_ENTRY_ADDRESS_LOW       0
#define ALLOCATION_TABLE_ENTRY_ADDRESS_HIGH      1
#define ALLOCATION_TABLE_ENTRY_ALLOCATION_SIZE   2
#define ALLOCATION_TABLE_ENTRY_ALLOCATION_ID     3

//
// This is multiply by 4, which means 4 bytes is the minimum
// allocation size (1 byte for checksum, 3 bytes for user data).
//
// This allows an individual block to represent up to 1024
// bytes of storage.
//
#define ALLOCATION_TABLE_ENTRY_SIZE_SHIFT         2

#define ALLOCATION_TABLE_ENTRY_MINIMUM_SIZE       (1 << ALLOCATION_TABLE_ENTRY_SIZE_SHIFT)

//
// A particular allocation block starts with its checksum.
//
// It's size and address is determined by its allocation table entry.
//
struct ConfigStoreAllocationBlock {
    uint8_t checksum;
};

#define ALLOCATION_BLOCK_CHECKSUM    0
#define ALLOCATION_BLOCK_DATA_START  1

// **************** New Model ****************

//
// For each application block two checksum digits are allocated.
// A checksum for a block consists of a single byte represented
// as two hex ascii characters.
//
// The checksum is calculated with xor and the total size of
// the block is added to ensure all 0's are detected.
//

//
// The first 64 bytes contain model, version, serial
// number information.
//
// 0 can't be used since it indicates no config entry
// in the table driven routines.
//

// begining of BUILTIN block that is checksumed
#define MODEL_INDEX            16
#define MODEL_SIZE             16

#define NAME_INDEX             (MODEL_INDEX + MODEL_SIZE)
#define NAME_SIZE              16

#define SERIAL_INDEX           (NAME_INDEX + NAME_SIZE)
#define SERIAL_SIZE            16

#define VERSION_INDEX          (SERIAL_INDEX + SERIAL_SIZE)
#define VERSION_SIZE            8  // 4 bytes as ASCII

#define FIRMWARE_VERSION_INDEX (VERSION_INDEX + VERSION_SIZE)
#define FIRMWARE_VERSION_SIZE   8 // 4 bytes as ASCII

#define SLEEPMODE_INDEX        (FIRMWARE_VERSION_INDEX + FIRMWARE_VERSION_SIZE)
#define SLEEPMODE_SIZE          4

#define CPUSPEED_INDEX         (SLEEPMODE_INDEX + SLEEPMODE_SIZE)
#define CPUSPEED_SIZE           4
// end of BUILTIN block that is checksumed

#define BUILTIN_CHECKSUM       (CPUSPEED_INDEX + CPUSPEED_SIZE)
#define BUILTIN_CHECKSUM_SIZE   2

#define BUILTIN_CHECKSUM_BEGIN MODEL_INDEX
#define BUILTIN_CHECKSUM_END   BUILTIN_CHECKSUM

// This is set to the largest of built in values
#define BUILTIN_MAX_SIZE       MODEL_SIZE

// begining of PIN block that is checksumed
#define PIN_INDEX              (BUILTIN_CHECKSUM + BUILTIN_CHECKSUM_SIZE)
#define PIN_SIZE               16
// end of PIN block that is checksumed

#define PIN_CHECKSUM           (PIN_INDEX + PIN_SIZE)
#define PIN_CHECKSUM_SIZE       2

#define PIN_CHECKSUM_BEGIN     PIN_INDEX
#define PIN_CHECKSUM_END       PIN_CHECKSUM

#define PIN_MAX_SIZE           PIN_SIZE

//
// Tracing Support
//

#define MENLOTRACE_SETMASK_INDEX        (PIN_CHECKSUM + PIN_CHECKSUM_SIZE)
#define MENLOTRACE_SETMASK_SIZE         9 // 00000000'\0'

// Note: This must be last as the checksum range depends on it
#define MENLOTRACE_CHECKSUM   (MENLOTRACE_SETMASK_INDEX + MENLOTRACE_SETMASK_SIZE)
#define MENLOTRACE_CHECKSUM_SIZE 2

// The first saved value is the start of the SETMASK checksum range
#define MENLOTRACE_CHECKSUM_BEGIN         MENLOTRACE_SETMASK_INDEX

// End of range for MENLOTRACE checksum is the start of the checksum storage location
#define MENLOTRACE_CHECKSUM_END  MENLOTRACE_CHECKSUM

#define MENLOTRACE_MAX_SIZE MENLOTRACE_SETMASK_SIZE

// begining of radio block that is checksumed
#define GATEWAY_ENABLED        (MENLOTRACE_CHECKSUM + MENLOTRACE_CHECKSUM_SIZE)
#define GATEWAY_ENABLED_SIZE    4

#define RADIO_CHANNEL          (GATEWAY_ENABLED + GATEWAY_ENABLED_SIZE)
#define RADIO_CHANNEL_SIZE      4

#define RADIO_RXADDR           (RADIO_CHANNEL + RADIO_CHANNEL_SIZE)
#define RADIO_RXADDR_SIZE      16

#define RADIO_TXADDR           (RADIO_RXADDR + RADIO_RXADDR_SIZE)
#define RADIO_TXADDR_SIZE      16

#define RADIO_POWER_TIMER      (RADIO_TXADDR + RADIO_TXADDR_SIZE)
#define RADIO_POWER_TIMER_SIZE  8

#define RADIO_OPTIONS          (RADIO_POWER_TIMER + RADIO_POWER_TIMER_SIZE)
#define RADIO_OPTIONS_SIZE     32
// End of radio block that is checksumed

#define RADIO_CHECKSUM         (RADIO_OPTIONS + RADIO_OPTIONS_SIZE)
#define RADIO_CHECKSUM_SIZE     2

#define RADIO_CHECKSUM_BEGIN   (GATEWAY_ENABLED)
#define RADIO_CHECKSUM_END     (RADIO_CHECKSUM)

// Maximum size of any individual item, Used by temporary buffers 
#define RADIO_MAX_SIZE         32

//
// WiFi Support
//
// 123 bytes of EEPROM storage
//

#define WIFI_SSID_INDEX        (RADIO_CHECKSUM + RADIO_CHECKSUM_SIZE)
#define WIFI_SSID_SIZE         33

#define WIFI_PASSWORD_INDEX    (WIFI_SSID_INDEX + WIFI_SSID_SIZE)
#define WIFI_PASSWORD_SIZE     65

#define WIFI_CHANNEL_INDEX     (WIFI_PASSWORD_INDEX + WIFI_PASSWORD_SIZE)
#define WIFI_CHANNEL_SIZE      5

#define WIFI_POWER_TIMER_INDEX (WIFI_CHANNEL_INDEX + WIFI_CHANNEL_SIZE)
#define WIFI_POWER_TIMER_SIZE  9

#define WIFI_OPTIONS_INDEX     (WIFI_POWER_TIMER_INDEX + WIFI_POWER_TIMER_SIZE)
#define WIFI_OPTIONS_SIZE      9

#define WIFI_CHECKSUM         (WIFI_OPTIONS_INDEX + WIFI_OPTIONS_SIZE)
#define WIFI_CHECKSUM_SIZE     2

// The first saved value is the start of the SETMASK checksum range
#define WIFI_CHECKSUM_BEGIN   WIFI_SSID_INDEX

// End of range for MENLOTRACE checksum is the start of the checksum storage location
#define WIFI_CHECKSUM_END     WIFI_CHECKSUM

// Attention does not have storage in EEPROM
#define WIFI_ATTENTION_SIZE    9

#define WIFI_MAX_SIZE          WIFI_PASSWORD_SIZE

//
// Area for CLOUD configuration storage.
//
// The sizes here work for most cloud providers such as
// www.smartpux.com, etc. and are configured for a 1024 byte
// configuration store available on most internet capable
// microcontrollers.
//
// The server, protocol, port, and URL are broken out
// to make handling easier without  having to write URL parse
// routines for these small embedded controllers.
//
// If a server or URL name exceeds these lengths the application
// can use direct IP addresses, DNS registrations, server application
// URL configuration, and URL shortening services to fit within
// these limits.
//
// For microcontrollers with greater than 1024 bytes of EEPROM
// storage, extended cloud configuration data is available
// and can be enabled on those controllers.
//
// Note:
//
// By default the MenloSmartpuxCloud application uses these
// locations in the core EEPROM storage as this is the primary
// cloud target for the MenloFramework.
//
// (hosted locally, or publically at www.smartpux.com).
//
// If the Smartpux Cloud is not used in a given application these
// locations may be used for other cloud providers. Though the
// recommendation is that each cloud provider provide for alternative
// storage locations if a given application uses multiple cloud providers.
//

//
// 212 bytes for cloud configuration including checksum.
//
// Note: Most WiFi capable microcontrollers have more than 1024
// EEPROM locations, so an extended set of cloud configuration
// values can be used to reach clouds with long URL's.
//

// begining of CLOUD block that is checksumed

//
// Single char + '\0' to represent protocol.
// H -> HTTP, S -> HTTPS, F -> FTP
//
#define CLOUD_PROTOCOL         (WIFI_CHECKSUM + WIFI_CHECKSUM_SIZE)
#define CLOUD_PROTOCOL_SIZE    2

#define CLOUD_PORT             (CLOUD_PROTOCOL + CLOUD_PROTOCOL_SIZE)
#define CLOUD_PORT_SIZE        4

//
// www.server.com
// data.server.com
// 192.168.1.1
//
// Note: low cost cloud accounts have long URL's, may need
// additional application specific configuration for those.
//

//
// 50 chars + '\0'
//
#define CLOUD_SERVER           (CLOUD_PROTOCOL + CLOUD_PROTOCOL_SIZE)
#define CLOUD_SERVER_SIZE      51

//
// 50 chars + '\0'
//
#define CLOUD_URL              (CLOUD_SERVER + CLOUD_SERVER_SIZE)
#define CLOUD_URL_SIZE         51

//
// 50 chars + '\0'
//
#define CLOUD_TOKEN            (CLOUD_URL + CLOUD_URL_SIZE)
#define CLOUD_TOKEN_SIZE       51

//
// 8 chars + '\0'
//
#define CLOUD_SENSOR            (CLOUD_TOKEN + CLOUD_TOKEN_SIZE)
#define CLOUD_SENSOR_SIZE       9

#define CLOUD_ACCOUNT           (CLOUD_SENSOR + CLOUD_SENSOR_SIZE)
#define CLOUD_ACCOUNT_SIZE      9

// end of CLOUD block that is checksumed

#define CLOUD_CHECKSUM          (CLOUD_ACCOUNT + CLOUD_ACCOUNT_SIZE)
#define CLOUD_CHECKSUM_SIZE     2

#define CLOUD_CHECKSUM_BEGIN    (CLOUD_PROTOCOL)
#define CLOUD_CHECKSUM_END      (CLOUD_CHECKSUM)

#define CLOUD_MAX_SIZE          CLOUD_URL_SIZE

// End of system configuration EEPROM area
#define SYSTEM_CONFIGURATION_END (CLOUD_CHECKSUM + CLOUD_CHECKSUM_SIZE)

//
// Compile assert required to ensure that this is not exceeded by the
// previous definitions.
//
// Currently at 512 bytes at 212 bytes cloud configuration size.
//

//
// EEPROM locations at this value and above are for the
// applications configuration settings.
//

#define TOP_INDEX 512

// This represents the begining of per application storage
#define APP_BASE_INDEX TOP_INDEX

//
// The rest of the area up to EEPROM size reserved for the application
//

//
// 128 bytes allocated here for common applications
//
#define APPLICATION_CONFIG_BASE_INDEX TOP_INDEX
#define APPLICATION_CONFIG_BASE_SIZE  128

//
// Common application components have definitions here to prevent
// collisions.
//
// A given application can customize these.
//
//

// byte offset 640 (0x280)
#define LIGHTHOUSE_MODULE_BASE_INDEX (APPLICATION_CONFIG_BASE_INDEX + APPLICATION_CONFIG_BASE_SIZE)
#define LIGHTHOUSE_MODULE_BASE_SIZE  128 // actual is 118 bytes 03/06/2016

// byte offset 768
#define WEATHER_MODULE_BASE_INDEX (LIGHTHOUSE_MODULE_BASE_INDEX + LIGHTHOUSE_MODULE_BASE_SIZE)
#define WEATHER_MODULE_BASE_SIZE      64 // actual is 46 bytes 03/06/2016

// byte offset 832
#define MODULE3_BASE_INDEX (WEATHER_MODULE_BASE_INDEX + WEATHER_MODULE_BASE_SIZE)
#define MODULE3_BASE_SIZE             64

// byte offset 896
#define MODULE4_BASE_INDEX (MODULE3_BASE_INDEX + MODULE3_BASE_SIZE)
#define MODULE4_BASE_SIZE             64

// byte offset 960
#define MODULE5_BASE_INDEX (MODULE4_BASE_INDEX + MODULE4_BASE_SIZE)
#define MODULE5_BASE_SIZE             39

//
// Define a diagnostics save area of 24 bytes
// from 1000 - 1023.
//
// This is used to log critical errors, watchdog resets, etc.
// to memory that is not cleared by the reset.
//
#define EEPROM_DIAGNOSTICS_AREA      1000
#define EEPROM_DIAGNOTICS_AREA_SIZE  24

// Use 32 bit types, same layout for AVR, AVR-MEGA, ARM
#define EEPROM_DIAGNOSTICS_ERROR_CODE     1000
#define EEPROM_DIAGNOSTICS_ERROR_PC       1004
#define EEPROM_DIAGNOSTICS_ERROR_WATCHDOG 1008
#define EEPROM_DIAGNOSTICS_ERROR_X0       1012
#define EEPROM_DIAGNOSTICS_ERROR_X1       1016
#define EEPROM_DIAGNOSTICS_ERROR_X2       1020

//
// Some controllers provide for more than 1024 bytes of
// EEPROM addressing such as the Photon, ESP8366, and
// other processors. This extended EEPROM space is used
// for cloud connected applications with long URL's
// and DNS server paths, as well as other more extended
// application usage.
//
// Note: These also result in larger ram blocks for their
// storage, and stack buffers for the table driven configuration
// store routines. But these larger microcontrollers have
// the space. (80k RAM vs. 2k RAM).
//
// Note: Application configuration Dweet's have to write the
// extended configuration in blocks since Dweet's have a maximum
// 80 character full message limit in order to limit buffer sizes
// and conform to the NMEA 0183 standard protocol specification.
//

#define EXTENDED_EEPROM_BASE 1024

// begining of CLOUD_EXT block that is checksumed

//
// CLOUD_EXT storage size 392 bytes
//
// Single char + '\0' to represent protocol.
// H -> HTTP, S -> HTTPS, F -> FTP
//
#define CLOUD_EXT_PROTOCOL         (EXTENDED_EEPROM_BASE)
#define CLOUD_EXT_PROTOCOL_SIZE    2

#define CLOUD_EXT_PORT             (CLOUD_EXT_PROTOCOL + CLOUD_EXT_PROTOCOL_SIZE)
#define CLOUD_EXT_PORT_SIZE        4

//
// Note: low cost cloud accounts have long URL's, may need
// additional application specific configuration for those.
//
// 127 chars + '\0'
//
#define CLOUD_EXT_SERVER           (CLOUD_EXT_PROTOCOL + CLOUD_EXT_PROTOCOL_SIZE)
#define CLOUD_EXT_SERVER_SIZE      128

//
// 127 chars + '\0'
//
#define CLOUD_EXT_URL              (CLOUD_EXT_SERVER + CLOUD_EXT_SERVER_SIZE)
#define CLOUD_EXT_URL_SIZE         128

//
// 127 chars + '\0'
//
#define CLOUD_EXT_TOKEN            (CLOUD_EXT_URL + CLOUD_EXT_URL_SIZE)
#define CLOUD_EXT_TOKEN_SIZE       128

//
// 127 chars + '\0'
//
#define CLOUD_EXT_SENSOR           (CLOUD_EXT_TOKEN + CLOUD_EXT_TOKEN_SIZE)
#define CLOUD_EXT_SENSOR_SIZE      128

#define CLOUD_EXT_ACCOUNT          (CLOUD_EXT_SENSOR + CLOUD_EXT_SENSOR_SIZE)
#define CLOUD_EXT_ACCOUNT_SIZE     129

// end of CLOUD block that is checksumed

#define CLOUD_EXT_CHECKSUM         (CLOUD_EXT_TOKEN + CLOUD_EXT_TOKEN_SIZE)
#define CLOUD_EXT_CHECKSUM_SIZE    2

#define CLOUD_EXT_CHECKSUM_BEGIN   (CLOUD_EXT_PROTOCOL)
#define CLOUD_EXT_CHECKSUM_END     (CLOUD_EXT_CHECKSUM)

#define CLOUD_EXT_MAX_SIZE         CLOUD_EXT_URL_SIZE

// End of system configuration EEPROM area
#define CLOUD_CONFIGURATION_END (CLOUD_EXT_CHECKSUM + CLOUD_EXT_CHECKSUM_SIZE)

class MenloConfigStore {

public:
  
    MenloConfigStore();

    int ReadConfig(int configIndex, uint8_t* buffer, uint8_t length);

    int WriteConfig(int configIndex, uint8_t* buffer, uint8_t length);

    //
    // These routines allow just basic ASCII characters in the
    // config store. They are optional, but useful when using human
    // readable configuration values such as with Dweet's.
    //
    bool IsValidConfigChar(char c);

    void ProcessConfigBufferForValidChars(char* buf, int len);

    bool ConfigBufferHasInvalidChars(char* buf, int len);

    bool
    CalculateAndStoreCheckSumRange(
        int checkSumStoreIndex,
        int configIndex,
        int length
	);

    bool
    CalculateAndValidateCheckSumRange(
        int checkSumStoreIndex,
        int configIndex,
        int length
	);

    // Calculate checksum for a range of config store
    uint8_t CalculateCheckSumRange(int configIndex, int length);

    //
    // PIN support.
    //
    // If PIN support is enabled, then WriteConfig() is not
    // allowed unless the unlock pin has been provided.
    //
    bool IsPinLocked();

    // Unlock with the pin to enable WriteConfig
    bool UnlockPin(char* pin);

    // Set a new pin.
    bool SetNewPin(char* pin);

#if MENLOCONFIGSTORE_DYNAMIC_MODEL

    //
    // Lookup the configuration entry for allocationid.
    //
    // If an existing entry is found, its size is compared
    // to blockSize, and if it does not match CONFIG_STORE_BLOCK_SIZE_MISMATCH
    // is returned.
    //
    // If an existing entry is not found a new one will be allocated and initialized
    // to zero with an invalid block checksum, and CONFIG_STORE_BLOCK_ALLOCATED is
    // returned in the error output, but the routine routine succeeds and
    // returns the address of the block.
    //
    // If the initialValuesTable pointer is not null it will be used to provide the
    // initial data values. In this case the block checksum will be valid. The return
    // CONFIG_STORE_BLOCK_ALLOCATED is an indication that default values are now
    // present.
    //
    // On success the returned pointer is to the first byte checksum area.
    //
    uint16_t GetConfigurationEntry(
        uint8_t allocationid,
        uint16_t blockSize,
        PGM_P initialValuesTable,
        uint8_t* error
        );

    // Validate table header
    bool ValidateTableHeader(uint8_t* error);
    
    // (re) initialize the table.
    bool InitializeTable(uint8_t* error);

    uint16_t GetBlockEntry(uint8_t allocationid, uint16_t* blockSize, uint8_t* error);

    uint16_t AllocateBlockEntry(uint8_t allocationid, uint16_t size, uint8_t* error);

    bool ValidateBlockEntryChecksum(uint8_t allocationid, uint8_t* error);

    bool UpdateBlockEntryChecksum(uint8_t allocationid, uint8_t* error);

#endif

private:

#if MENLOCONFIGSTORE_DYNAMIC_MODEL

    // These values save reading from EEPROM constantly
    bool m_headerCacheValid;
    bool m_blockCacheValid;

    uint8_t m_cachedNumberOfEntries;
    uint16_t m_cachedAllocationPointer;

    //
    // cache to last block entry
    //
    // An entry is not placed into the cache unless its found and
    // has a valid checksum.
    //
    uint8_t  m_cachedAllocationid; // the allocationid in the cache
    uint8_t  m_cachedIndex;        // it's actual index
    uint16_t m_cachedBlockAddress; // cached data block address
    uint16_t m_cachedBlockSize;    // cached data block size
#endif

#if CONFIG_PIN_SUPPORT
    void ReadAndValidatePin();

    bool m_pinLocked;
#endif
};

//
// I generally don't like an automatic global but there
// is only one instance.
//
extern MenloConfigStore ConfigStore;

#endif // MenloConfigStore_h
