
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
// PlantMonitor
//
// 05/25/2014
//
// Demonstrates a plant monitor and watering solution.
//

//
// Hardware Setup
//

//
// The sensor is wired to a prototype breadboard
// to an RFduino based sensor.
//
// This is the mapping to the Arduino pins on the Breadboard
//
//GPIO 0 - Green LED digital output (90 ohm)      Arduino D3
//GPIO 1 - Red LED digital output (152 ohm)       Ardunio D5
//GPIO 2 - Blue LED digital output (90 ohm)       Arduino D6
//GPIO 3 - Moisture Sensor analog input           Arduino A1
//GPIO 4 - Sensor Power for CDS and 2n2222a       Arduino D2
//GPIO 5 - Light Sensor/CDS analog input          Arduino A0
//GPIO 6 - Bat analog in through 100 ohm resistor Arduino A5
//
//RFDuino     Arduino
//
//GPIO_0      D3       (PWM) Green LED
//GPIO_1      D5       (PWM) Red LED
//GPIO_2      D6       (PWM) Blue LED
//GPIO_3      A1        Moisture
//GPIO_4      D2        Sensor Power (Light, Moisture)
//GPIO_5      A0        Light
//GPIO_6      A5        Battery Level
//

// Digital (PWM) output ports
int g_RedLed = 5;
int g_GreenLed = 3;
int g_BlueLed = 6;

// Digital non-PWM output ports
int g_SensorEnable = 2;
int g_IrrigationEnable = 4;

// Analog input ports
int g_Light = 0;
int g_Moisture = 1;

// Sensor Input variables (readings);
int g_LightReading = 0;
int g_MoistureReading = 0;

// Sensor Output variables (commands)
int g_RedSetting = 0;
int g_GreenSetting = 0;
int g_BlueSetting = 0;
int g_IrrigationSetting = 0;

// Control Limits
int g_DarkLimit = 200; // > 200

int g_MoistureLimit = 1; // > 0 is moist

int g_IrrigationOn = 0;

int g_IrrigationOff = 1;

void
setup()
{
    Serial.begin(9600);

    //
    // Uncomment what is needed for various hardware setup
    //
   
    //analogReference(EXTERNAL);

    pinMode(g_RedLed, OUTPUT);
    pinMode(g_GreenLed, OUTPUT);
    pinMode(g_BlueLed, OUTPUT);

    pinMode(g_IrrigationEnable, OUTPUT);
    digitalWrite(g_IrrigationEnable, g_IrrigationOff);

    pinMode(g_SensorEnable, OUTPUT);
    digitalWrite(g_SensorEnable, LOW);

    SequenceLED();
    //TestLED();
}

void
loop()
{
    // Gather inputs, run rules, generate output values
    PerformSensorLoop();

    // delay is in ms
    delay(1000);
}

void
PerformSensorLoop()
{
    GetSensorReadings();

    Serial.print("Light Reading=");
    Serial.println(g_LightReading);

    Serial.print("Moisture Reading=");
    Serial.println(g_MoistureReading);

    // If any moisture turn on Green LED
    if (g_MoistureReading > 0) {
        g_GreenSetting = 255;
        g_RedSetting = 0;
    }
    else {
        // Needs Irrigation
        g_RedSetting = 255;
        g_GreenSetting = 0;
    }

    //
    // If dark, and no moisture, turn on irrigation
    //
    if ((g_LightReading > g_DarkLimit) &&
        (g_MoistureReading < g_MoistureLimit)) {

        Serial.println("Irrigation On");
        g_IrrigationSetting = g_IrrigationOn;
    }
    else {
        Serial.println("Irrigation Off");
        g_IrrigationSetting = g_IrrigationOff;
    }

    // Perform outputs
    SetLED(g_RedSetting, g_GreenSetting, g_BlueSetting);

    SetIrrigation();
}

//
// The LED values are 0-255 for PWM intensity
//
void
SetLED(
    int Red,
    int Green,
    int Blue
    )
{
    // Write using PWM outputs
    analogWrite(g_RedLed, Red);
    analogWrite(g_GreenLed, Green);
    analogWrite(g_BlueLed, Blue);
}

void
SequenceLED()
{
    int index;

    SetLED(0, 0, 0);
    for (index = 0; index < 255; index++) {
        analogWrite(g_RedLed, index);
        delay(10);
    }

    SetLED(0, 0, 0);
    for (index = 0; index < 255; index++) {
        analogWrite(g_GreenLed, index);
        delay(10);
    }

    SetLED(0, 0, 0);
    for (index = 0; index < 255; index++) {
        analogWrite(g_BlueLed, index);
        delay(10);
    }

    SetLED(0, 0, 0);
}

void
TestLED()
{

    Serial.println("Red...");
    digitalWrite(g_RedLed, HIGH);
    delay(1000);
    digitalWrite(g_RedLed, LOW);
    delay(1000);

    Serial.println("Green...");
    digitalWrite(g_GreenLed, HIGH);
    delay(1000);
    digitalWrite(g_GreenLed, LOW);
    delay(1000);

    Serial.println("Blue...");
    digitalWrite(g_BlueLed, HIGH);
    delay(1000);
    digitalWrite(g_BlueLed, LOW);
    delay(1000);
}

void
SetIrrigation()
{
    digitalWrite(g_IrrigationEnable, g_IrrigationSetting);
}

void
GetSensorReadings()
{
    // Enable sensor power and let it settle for 100ms
    digitalWrite(g_SensorEnable, HIGH);
    delay(100);

    g_LightReading = analogRead(g_Light);

    g_MoistureReading = analogRead(g_Moisture);

    // Disable sensor power
    digitalWrite(g_SensorEnable, LOW);
}
