
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
 *  Date: 02/10/2015
 *  File: LightHouseHardware.h
 *
 * Hardware handling for Lighthouse.
 *
 * The Lighthouse provides 3 color LED and environmental
 * and battery monitoring.
 *
 */

#ifndef LightHouseHardware_h
#define LightHouseHardware_h

#include <LightHouseHardwareBase.h>
#include <MenloLightHouse.h>

struct LightHouseLight {
    int whitePin;
    int redPin;
    int greenPin;
    int bluePin;
};

//
// LightHouseHardware is a slave to LightHouseApp which handles
// the logic of when the light should be on, as well as its
// flash sequence.
//
// In addition LightHouseApp determines when environmental
// readings are done.
//
class LightHouseHardware : public LightHouseHardwareBase {

public:

    LightHouseHardware();

    // Initialize with sensors
    int
    Initialize(
        LightHouseLight* light,
        LightHouseSensors* sensors
       );

    void SetDweet(MenloDweet* dweet);

    virtual bool getLightState();
    virtual void setLightState(MenloLightHouseEventArgs* args);

    virtual void setRGB(bool state);

    virtual void setRGBIntensity(int red, int green, int blue);

    virtual void setPolarity(bool polarity) {
        m_lightPolarity = polarity;
    }

    //
    // Get sensor readings
    //
    virtual bool readSensors(LightHouseSensors* sensors);

private:

    // For NMEA 0183 streaming sensor support
    MenloDweet* m_dweet;

    bool InitializeSensors(LightHouseSensors* sensors);

    void InitializeNMEA0183Sensors();

    // Hardware state
    bool m_lightState;

    // True  -> 1 == ON
    // FALSE -> 0 == ON
    bool m_lightPolarity;

    bool m_rgbEnabled;

    // Hardware config
    int m_whitePin;

    // RGB pins for PWM
    int m_redPin;
    int m_greenPin;
    int m_bluePin;

    // Default RGB intensity values
    int m_redIntensity;
    int m_greenIntensity;
    int m_blueIntensity;

    //
    // Pins for environmental monitoring
    // These pins are analog inputs
    //
    LightHouseSensors m_sensors;

    //
    // Support for receiving NMEA 0183 data streams for meteorological data.
    //

    // Event registration
    MenloNMEAMessageEventRegistration m_nmeaEvent;

    // NMEAEvent function
    unsigned long NMEAEvent(MenloDispatchObject* sender, MenloEventArgs* eventArgs);

    //
    // Values received from environmental sensors as NMEA 0183 messages
    //
    int m_windspeed;
    int m_winddirection;
    int m_temperature;
    int m_barometer;
    int m_humidity;
    int m_rain;
    int m_battery;
    int m_solar;
    int m_nmeaLight;
};

#endif // LightHouseHardware_h
