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
 *  Date: 02/22/2015
 *  File: LightHouseConfig.h
 */

//
// Applications non-volatile storage allocations
//

//
// LIGHTHOUSE_MODULE_BASE_INDEX is in MenloConfigStore.h
//

//
// Note: Each entry has an extra byte allocated
// to all a '\0' to terminate the string entry
//

// begining of lighthouse block that is checksumed
#define LIGHT_PERIOD        LIGHTHOUSE_MODULE_BASE_INDEX
#define LIGHT_PERIOD_SIZE   9  // 32 bit value in HEX chars
#define LIGHT_TICK          (LIGHT_PERIOD + LIGHT_PERIOD_SIZE)
#define LIGHT_TICK_SIZE     9  // 32 bit value in HEX chars
#define LIGHT_COLOR         (LIGHT_TICK + LIGHT_TICK_SIZE)
#define LIGHT_COLOR_SIZE    9 // 00.00.00
#define LIGHT_RAMP          (LIGHT_COLOR + LIGHT_COLOR_SIZE)
#define LIGHT_RAMP_SIZE     10 // 0000.0000

#define LIGHT_ONLEVEL       (LIGHT_RAMP + LIGHT_RAMP_SIZE)
#define LIGHT_ONLEVEL_SIZE  5 // 0000

#define LIGHT_SEQUENCE      (LIGHT_ONLEVEL + LIGHT_ONLEVEL_SIZE)
#define LIGHT_SEQUENCE_SIZE 68 // 00: + 64 chars + '\0'

#define LIGHT_STORAGE_INDEX (LIGHT_SEQUENCE + LIGHT_SEQUENCE_SIZE)
#define LIGHT_STORAGE_SIZE  1

#define LIGHT_SENSORRATE (LIGHT_STORAGE_INDEX + LIGHT_STORAGE_SIZE)
#define LIGHT_SENSORRATE_SIZE  5 // 0000
// End of lighthouse block that is checksumed

// Note: This must be last as the checksum range depends on it
#define LIGHT_CHECKSUM   (LIGHT_SENSORRATE + LIGHT_SENSORRATE_SIZE)
#define LIGHT_CHECKSUM_SIZE 2

// 118 bytes 03/06/2016

// The first saved value is the start of the LIGHT checksum range
#define LIGHT_CHECKSUM_BEGIN (LIGHT_PERIOD)

// End of range for LIGHT checksum is the start of the checksum storage location
#define LIGHT_CHECKSUM_END   (LIGHT_CHECKSUM)

// 0000.0000.0000.0000.0000'\0'
#define SENSOR_DATA_SIZE 26

// This is the largest of the buffers
#define LIGHT_MAX_SIZE LIGHT_SEQUENCE_SIZE
