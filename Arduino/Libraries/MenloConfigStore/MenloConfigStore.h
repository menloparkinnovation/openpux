
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
#include <Arduino.h>
#include <inttypes.h>

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
// Defines for information layout
//
// A static layout for core information allows
// easier management, debugging.
//

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

// begining of radio block that is checksumed
#define GATEWAY_ENABLED        (PIN_CHECKSUM + PIN_CHECKSUM_SIZE)
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

#define RADIO_MAX_SIZE         32

//
// Area for URL storage
//
// 64 characters for a base config URL.
//
// An application can assume "HTTP", "HTTPS", "WWW",
// protocol and port if required to squeeze more space.
// 
// But most base connections can use the 64 characters.
// 
// Indexes are defined for protocol, port which consume
// 3 of the 64 characters if used.
// 
// http://www.smartpux.com/rest/my_application_action
//
// For example, the following is 64 characters and did not
// have to the compact format.
//
// https://www.smartpux.com:8080/rest/routing/my_application_action
//
// Compact format squeezed 14 more characters by using 3 bytes
// for protocol, port and assuming/building in the need for
// http:, https:, www, etc.
//
// smartpux.com/rest/routing/my_application_action/fourteen_more_
//
// Many application just need to contact a root of the application
// such as:
//
// https://www.smartpux.com:8080/device
//

// begining of URL block that is checksumed
#define URL_STORAGE            (RADIO_CHECKSUM + RADIO_CHECKSUM_SIZE)
#define URL_STORAGE_SIZE       64

#define URL_PROTOCOL           (URL_STORAGE + URL_STORAGE_SIZE)
#define URL_PROTOCOL_SIZE       2

#define URL_PORT               (URL_PROTOCOL + URL_PROTOCOL_SIZE)
#define URL_PORT_SIZE           4
// end of URL block that is checksumed

#define URL_CHECKSUM           (URL_PORT + URL_PORT_SIZE)
#define URL_CHECKSUM_SIZE       2

#define URL_CHECKSUM_BEGIN     URL_STORAGE
#define URL_CHECKSUM_END       URL_CHECKSUM

#define URL_MAX_SIZE           URL_STORAGE_SIZE

//
// Compile assert required to ensure that this is not exceeded by the
// previous definitions.
//
// Currently at 260 bytes
//

//
// EEPROM locations at this value and above are for the
// applications configuration settings.
//

#define TOP_INDEX 512

//
// The rest of the area up to EEPROM size reserved for the application
//

#define MAX_EEPROM_SIZE 1023

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

private:

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
