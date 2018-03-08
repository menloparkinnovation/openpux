
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
 *  Date: 05/28/2016
 *  File: SensorHardware.h
 *
 *  Hardware handling for sensor projects.
 */


#ifndef SensorHardware_h
#define SensorHardware_h

#include <DweetSensorApp.h>

//
// This contains readings from the available sensors.
//
// This is customized based on what data is reported.
//
struct SensorHardwareData {
    int lightIntensity;
    int battery;
    int solar;
    int moisture;
    int temperature;
};

//
// This contains the hardware configuration
//
// This structure is input by the application to configure
// for a specific hardware environment, board.
//
struct SensorHardwareConfig {

    //
    // This assumes every device has a one color, or three color LED
    // available for status, etc.
    //
    int whitePin;
    int redPin;
    int greenPin;
    int bluePin;

    // True  -> 1 == ON
    // FALSE -> 0 == ON
    bool lightPolarity;

    int sensorPower;           // Power on sensors
    int lightIntensity;        // light intensity reading
    int battery;               // battery charge state
    int solar;                 // solar cell voltage
    int moisture;              // moisture sensor
    int temperature;           // temperature sensor

    bool rgbEnabled;

#if USE_DS18S20
    //
    // Dallas OneWire Temperature Sensor
    //
    OS_DS18S20 temperatureSensor;
#endif

    void Initialize() {

        // Setup our application hardware pins
        if (whitePin != (-1)) {
            pinMode(whitePin, OUTPUT);
        }

        if (redPin != (-1)) {
            pinMode(redPin, OUTPUT);
            rgbEnabled = true;
        }

        if (greenPin != (-1)) {
            pinMode(greenPin, OUTPUT);
            rgbEnabled = true;
        }

        if (bluePin != (-1)) {
            pinMode(bluePin, OUTPUT);
            rgbEnabled = true;
        }

        if (sensorPower != (-1)) {
            pinMode(sensorPower, OUTPUT);
            digitalWrite(sensorPower, 0);
        }

#if USE_DS18S20
        if (temperature != (-1)) {
            temperatureSensor.Initialize(temperature);
        }
#endif

        //
        // These pins are analog input which by default need
        // no configuration.
        //
        // m_sensors.lightIntensity
        // m_sensors.battery
        // m_sensors.solar
        // m_sensors.moisture
        // m_sensors.temperature
        //
    }
};

//
// This contains the runtime operating state
//
struct SensorHardwareState {

    // Light on or off
    bool lightState;

    //
    // Level in which light turns on
    // Based on lightIntensity reading level. (analog 10 or 12 bit)
    //
    int lightOnLevel;

    //
    // This is the current light level.
    //
    int lightLevel;

    // Default RGB intensity values
    int redIntensity;
    int greenIntensity;
    int blueIntensity;

    void Initialize() {
        lightState = false;
        lightOnLevel = (-1); // default is always on
        lightLevel = 0 ;     // default is darkness
        redIntensity = 0xFF;
        greenIntensity = 0xFF;
        blueIntensity = 0xFF;
    }
};

//
// The SensorApp also can provide low overhead
// 32 byte radio packet updates.
//
// The basic packet definition comes from MenloRadioSerial
// so the radio channel can be shared with general management
// commands (Dweets).
//

//
// This is an overlay for MenloRadioSerialSensorData which
// is a general use application level radio packet form
//
// Same as LightHouseSensorData in LightHouseApp.h
//
struct SensorDataPacket {

  // 2 bytes
  uint8_t  type;
  uint8_t  subType; // subtype for applications

  //
  // Multibyte types are stored LSB first as on the AtMega series.
  // Other big endian processors need to reverse the bytes.
  //
  // (These have more code space to accomodate)
  //

  // 16 bytes
  uint16_t light;
  uint16_t battery;
  uint16_t solar;
  uint16_t moisture;
  uint16_t temperature;
  uint16_t windspeed;
  uint16_t winddirection;
  uint16_t barometer;

  // 14 bytes
  uint16_t rainfall;
  uint16_t humidity;
  uint16_t fog;
  uint16_t seastate;
  uint16_t radiomonitor;
  uint16_t batteryCurrent;
  uint16_t solarCurrent;
};

#define SENSOR_DATA_LIGHT       0x01
#define SENSOR_DATA_BATTERY     0x02
#define SENSOR_DATA_SOLAR       0x04
#define SENSOR_DATA_MOISTURE    0x08
#define SENSOR_DATA_TEMPERATURE 0x10
#define SENSOR_DATA_WIND        0x20
#define SENSOR_DATA_BAROMETER   0x40
#define SENSOR_DATA_HUMIDITY    0x80

#define SENSOR_DATA_SIZE sizeof(struct SensorDataPacket)

// MenloRadioSerial.h 0xC2
// Same as LightHouseApp.h
//#define SENSOR_DATA_PACKET   MENLO_RADIO_SERIAL_SENSORDATA
#define SENSOR_DATA_PACKET   MENLO_RADIO_SENSOR_DATA

#define SENSOR_DATA_OVEHEAD  1

//
// DweetSensorApp provides:
//
// Callbacks for Dweets.
//
// Timer functions for scheduling sensor samples and
// their configuration Dweets, including EEPROM storage.
//
// Requests to provide out of interval data samples through
// Dweet.
//

class SensorHardware : public DweetSensorApp {

public:

    SensorHardware();

    int Initialize(SensorHardwareConfig* config);

    void SetRadio(MenloRadio* menloRadio) {
        // This could be NULL, which means no radio
        m_radio = menloRadio;
    }

    //
    // Applications based on DweetSensorApp have the option of using
    // sub-classing and virtual methods rather than their own independent
    // registration for received Dweets and Timer events.
    //
    // They are free to do so, for simple extensions of a basic application
    // subclassing is more straightforward than event registrations which
    // shines as a larger component integration framework.
    //
    virtual int SubClassProcessAppCommands(MenloDweet* dweet, char* name, char* value);

    //
    // This is invoked from the sensor timer to allow sensors
    // to process.
    //
    virtual unsigned long SensorTimerEvent();

    //
    // GET/SET property commands
    //

    int LightColor(char* buf, int size, bool isSet);

    int LightOnLevel(char* buf, int size, bool isSet);

    int Sensors(char* buf, int size, bool isSet);

protected:

private:

    // Generic Data
    SensorHardwareConfig m_config;

    SensorHardwareState m_state;

    // MenloRadio to send sensor updates to
    MenloRadio* m_radio;

    //
    // Generic Functions
    //

    // Periodically sample sensors, process the readings.
    void ProcessLocalSensors();

    // Process Sensor State Transitions as a result of new samples.
    void ProcessSensorStateTransitions();

    // Report sensor data, state as a result of new samples.
    void ReportSensorReadings(SensorHardwareData* data);

    // Report sensor data, state as a result of new samples on the radio.
    void ReportSensorReadingsOnRadio(SensorHardwareData* data);

    // Load settings from EEPROM
    void InitializeStateFromStoredConfig();

    //
    // Get sensor readings
    //
    bool readSensors(SensorHardwareData* sensors);

    //
    // Custom functions
    //

    // Control RGB intensity values
    void SetRGB(bool state);
};

//
// Configuration Store
//

//
// SENSOR_SUBAPP_STORAGE_BASE_INDEX is defined in DweetSensorApp.h
//

// begining of sensor block that is checksumed
#define SENSOR_LIGHT_COLOR       SENSORAPP_SUBAPP_STORAGE_BASE_INDEX
#define SENSOR_LIGHT_COLOR_SIZE    9 // 00.00.00

#define SENSOR_LIGHT_ONLEVEL       (SENSOR_LIGHT_COLOR + SENSOR_LIGHT_COLOR_SIZE)
#define SENSOR_LIGHT_ONLEVEL_SIZE  5 // 0000
// End of sensor block that is checksumed

// Note: This must be last as the checksum range depends on it
#define SENSOR_CHECKSUM   (SENSOR_LIGHT_ONLEVEL + SENSOR_LIGHT_ONLEVEL_SIZE)
#define SENSOR_CHECKSUM_SIZE 2

// The first saved value is the start of the LIGHT checksum range
#define SENSOR_CHECKSUM_BEGIN SENSOR_LIGHT_COLOR

// End of range for LIGHT checksum is the start of the checksum storage location
#define SENSOR_CHECKSUM_END  SENSOR_CHECKSUM

//
// This is the response to the SENSORS Dweet which returns
// a string of the sensor readings.
//
//                        lite bat  slr  mois temp
// GETSTATE_REPLY=SENSORS:0000.0000.0000.0000.0000
//
//
#define SENSORS_RESPONSE_SIZE 32

#define SENSOR_MAX_SIZE SENSORS_RESPONSE_SIZE

#define LIGHTHOUSE_SENSOR_DATA_SUBTYPE  0xFF

#endif // SensorHardware_h
