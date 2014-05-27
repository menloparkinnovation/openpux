
//
//   Copyright (C) 2014 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   MenloParkInnovation LLC additions and changes are licensed under the
//   Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//

//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Snapshot License
//
//   This license is for a specific snapshot of a base work of
//   Menlo Park Innovation LLC on a non-exclusive basis with no warranty
//   or obligation for future updates. This work, any portion, or derivative
//   of it may be made available under other license terms by
//   Menlo Park Innovation LLC without notice or obligation to this license.
//
//   There is no warranty, statement of fitness, statement of
//   fitness for any purpose, and no statements as to infringements
//   on any patents.
//
//   Menlo Park Innovation has no obligation to offer support, updates,
//   future revisions and improvements, source code, source code downloads,
//   media, etc.
//
//   This specific snapshot is made available under the terms of
//   the Apache License Version 2.0 of January 2004
//   http://www.apache.org/licenses/
//

//
// DrinkCooler sensor test
//

#include <SPI.h>
#include <WiFi.h>

//
// User configuration
//

// WiFi
char ssid[] = "OceanHouse";
char pass[] = "thisisakey321";
int keyIndex = 0;            // network key Index number (needed only for WEP)

// Cloud Server
char server[] = "192.168.1.11";
int serverport = 8080;

char* openpuxUrl = "/smartpuxdata/data";

// Cloud Server Update Interval
unsigned long SensorUpdateInterval = (30 * 1000);

//
// Settings we send to the Cloud.
//
// Updated by the various sensor loops due to
// interrupts, timers, or polling.
//
// These are mapped to the application
//
int SensorReading0 = 0; // current Temperature in Fahrenheit
int SensorReading1 = 0; // current Temperature in Celsius
int SensorReading2 = 0; // reference/ambient temperature
int SensorReading3 = 0; // cooling
int SensorReading4 = 0; // heating
int SensorReading5 = 0; // amps
int SensorReading6 = 0;
int SensorReading7 = 0;
int SensorReading8 = 0;
int SensorReading9 = 0;

//
// Settings received from Cloud
//
// These are mapped to the application
//
int Command = 0;
int SleepTime = 0;
int TargetMask0 = 0; // auto mode
int TargetMask1 = 0; // cooling mode
int TargetMask2 = 0; // heating mode
int TargetMask3 = 0; // cooling target temp in F
int TargetMask4 = 0; // heating target temp in F
int TargetMask5 = 0;
int TargetMask6 = 0;
int TargetMask7 = 0;
int TargetMask8 = 0;
int TargetMask9 = 0;

//
// Application Sensor Control Loop
//

void
UpdateSensorReadings()
{
    //
    // Perform sensor readings
    //
    // These will automatically be sent to the Cloud on the
    // next update interval
    //

    SensorReading0 = ThermoCouple_GetExternalTemp_Fahrenheit();

    Serial.print("ThermoCouple F:");
    Serial.println(SensorReading0);

    SensorReading1 = ThermoCouple_GetExternalTemp_Celsius();
    SensorReading2 = ThermoCouple_GetInternalTemp_Celsius();

    SensorReading3 = Relays_ReadCoolingState();
    SensorReading4 = Relays_ReadHeatingState();

    SensorReading5 = CurrentSensor_Read();

    // Process control loop according to the new readings
    PerformControlLoop();
}

void
OperateOnSensorUpdate()
{
    // Operational masks are in TargetMask0 - TargetMask9

    // Process control loop according to the new settings
    PerformControlLoop();
}

//
// This is the main control loop
//

void
PerformControlLoop()
{
    // See if heating setpoint reached
    if (SensorReading0 > TargetMask4) {
        SetIdleMode();
        return;
    }

    // See i cooling setpoint reached
    if (SensorReading0 < TargetMask3) {
        SetIdleMode();
        return;
    }

    //
    // Determine if we are heating or cooling
    //

    if (TargetMask0 != 0) {

        //
        // auto cool/heat mode
        //
        // Drink less than room temperature should be cool
        //
        if (SensorReading0 < 75) {
            SetCoolingMode();
        }
        else if (SensorReading0 > 90) {
            // 90 or better triggers a warm drink
            SetHeatingMode();
        }
        else {
            SetIdleMode();
        }
    }
    else {
        // Auto mode overrides manual modes

        if (TargetMask1 != 0) {
           // cooling mode
           SetCoolingMode();
        }
        else {
             // Cooling overides heating if both are set
             if (TargetMask2 != 0) {
                 // heating mode
                 SetHeatingMode();
            }
        }
    }
}

void
SetIdleMode()
{
    Relays_SetHeating(LOW);
    Relays_SetCooling(LOW);
}

void
SetCoolingMode()
{
    Relays_SetHeating(LOW);
    Relays_SetCooling(HIGH);
}

void
SetHeatingMode()
{
    Relays_SetCooling(LOW);
    Relays_SetHeating(HIGH);
}

//
// Standard Arduino template
//

// Initialize the WiFi client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

// To manage the connection state machine
boolean lastConnected = false;

int status = WL_IDLE_STATUS;

void
setup()
{
    //Initialize serial and wait for port to open:
    Serial.begin(9600); 
    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }
  
    // Setup the hardware
    HardwareSetup();

    Serial.println("\nDone with setup()");
}

void
loop()
{
    UpdateSensorReadings();

    Serial.print("ThermoCouple F:");
    Serial.println(SensorReading0);

    Serial.print("ThermoCouple C:");
    Serial.println(SensorReading1);

    Serial.print("ThermoCouple InternalTemp C:");
    Serial.println(SensorReading2);

    Serial.print("CurrentSensor:");
    Serial.println(SensorReading5);

    Relays_SetCooling(HIGH);
    delay(500);
    Relays_SetCooling(LOW);
    delay(500);

    Relays_SetHeating(HIGH);
    delay(500);
    Relays_SetHeating(LOW);
    delay(500);
}

//
// MenloParkInnovation LLC Hardware specific code
//

//
// Hardware setup
//

//
// Digital Pins are 0-13
//
// D0  - Serial RX
// D1  - Serial TX
// D2  -  High speed, reserve
// D3  - (PWM) High speed capable, reserve
// D4  - Thermocouple CS
// D5  - (PWM) Thermocouple MISO
// D6  - (PWM) Thermocouple SCK
// D7  - Cooling Relay
// D8  - Heating Relay
// D9  - (PWM)
// D10 - SPI SS   (PWM)
// D11 - SPI MOSI (PWM)
// D12 - SPI MISO
// D13 - SPI SCK, on board LED on many boards
//
// A0  - CurrentSensor input
//

void
HardwareSetup()
{
    ThermoCouple_Setup();

    CurrentSensor_Setup();

    Relays_Setup();
}

//
// GHI Thermocouple Support
//

// Thermocouple is an SPI receive device
int g_ThermoCoupleCSNPin = 4;  // LOW is selected
int g_ThermoCoupleMISOPin = 5;
int g_ThermoCoupleSCKPin = 6;

// MAX31855K chipset
const int ERROR_NOCONECT = 0x01;
const int ERROR_SHORTGND = 0x02;
const int ERROR_SHORTVCC = 0x04;

void
ThermoCouple_Setup()
{
    // Thermocouple
    pinMode(g_ThermoCoupleCSNPin, OUTPUT);
    digitalWrite(g_ThermoCoupleCSNPin, HIGH);

    pinMode(g_ThermoCoupleSCKPin, OUTPUT);
    digitalWrite(g_ThermoCoupleSCKPin, LOW);

    pinMode(g_ThermoCoupleMISOPin, INPUT);
}

unsigned long
ThermoCouple_Read()
{
    int bitCount;
    unsigned long data = 0;

    digitalWrite(g_ThermoCoupleCSNPin, LOW);
    //_cs.Write(false);

    for (bitCount = 31; bitCount >= 0; bitCount--) {

        digitalWrite(g_ThermoCoupleSCKPin, HIGH);
        //_clk.Write(true);

        if (digitalRead(g_ThermoCoupleMISOPin) != 0) {
        //if (_miso.Read()) {
            data |= (uint)(1 << bitCount);
        }

        digitalWrite(g_ThermoCoupleSCKPin, LOW);
        //_clk.Write(false);
    }

    digitalWrite(g_ThermoCoupleCSNPin, HIGH);
    //_cs.Write(true);

    return data;
}

short
ThermoCouple_GetExternalTemp_Celsius()
{
    uint value = ThermoCouple_Read();
    short temperature = 0;
    unsigned char data[4];

    data[0] = (unsigned char)(value >> 24);
    data[1] = (unsigned char)(value >> 16);
    data[2] = (unsigned char)(value >> 8);
    data[3] = (unsigned char)(value >> 0);

    temperature = (short)(((data[1]) | (data[0] << 8)));
    temperature = (short)(temperature >> 4);
    return temperature;
}

short
ThermoCouple_GetExternalTemp_Fahrenheit()
{
    return (short)((ThermoCouple_GetExternalTemp_Celsius() * 1.8) + 32);
}

unsigned char
ThermoCouple_GetInternalTemp_Celsius()
{
    uint value = ThermoCouple_Read();
    return (unsigned char)((value >> 8) & 0xFF); // get byte 2
}

//
// GHI CurrentSensor Support
//

// CurrentSensor is an analog input
int g_CurrentSensorPin = 0;

void
CurrentSensor_Setup()
{
    //analogReference(EXTERNAL);

    return;
}

int
CurrentSensor_Read()
{
    int analogValue;

    analogValue = analogRead(g_CurrentSensorPin);

    return analogValue;
}

//
// Seeed Relays Support
//

// Heating/Cooling relays
int g_CoolingRelayPin = 7;
int g_HeatingRelayPin = 8;

void
Relays_Setup()
{
    // Relay control
    pinMode(g_CoolingRelayPin, OUTPUT);
    digitalWrite(g_CoolingRelayPin, LOW);

    pinMode(g_HeatingRelayPin, OUTPUT);
    digitalWrite(g_HeatingRelayPin, LOW);
}

void
Relays_SetCooling(int state)
{
    if (state) {
        digitalWrite(g_CoolingRelayPin, HIGH);
    }
    else {
        digitalWrite(g_CoolingRelayPin, LOW);
    }
}

void
Relays_SetHeating(int state)
{
    if (state) {
        digitalWrite(g_HeatingRelayPin, HIGH);
    }
    else {
        digitalWrite(g_HeatingRelayPin, LOW);
    }
}

int
Relays_ReadCoolingState()
{
    int value;

    value = digitalRead(g_CoolingRelayPin);

    return value;
}

int
Relays_ReadHeatingState()
{
    int value;

    value = digitalRead(g_HeatingRelayPin);

    return value;
}
