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
// From scratch Arduino template not started from any other
// included project.
//
// As such this is non-GNU.
//

//
// Include any headers needed here. Arduino headers are automatically included.
//

//
// Digital Pins are 0-13
//
// D0  - Serial RX
// D1  - Serial TX
// D2  - 
// D3  - (PWM)
// D4  -
// D5  - (PWM)
// D6  - (PWM)
// D7  -
// D8  -
// D9  - (PWM)
// D10 - SPI SS   (PWM)
// D11 - SPI MOSI (PWM)
// D12 - SPI MISO
// D13 - SPI SCK, on board LED on many boards
//

int g_DigitalPin = 13;
int g_Toggle = LOW;

// Analog pins are 0-5
int g_AnalogPin = 0;

// PWM Pins are D3, D5, D6, D9, D10, D11
int g_PwmPin = 3;

void
setup()
{
    Serial.begin(9600);

    //
    // Uncomment what is needed for various hardware setup
    //
   
    //analogReference(EXTERNAL);
    //pinMode(g_DigitalPin, INPUT);

    pinMode(g_DigitalPin, OUTPUT);
    digitalWrite(g_DigitalPin, LOW);

    pinMode(g_PwmPin, OUTPUT);

    // To write a PWM value to a (PWM) pin (0-255)
    analogWrite(g_PwmPin, 128);
}

void
loop()
{
    int analogValue;

    analogValue = analogRead(g_AnalogPin);

    Serial.print("Analog Input=");
    Serial.println(analogValue);

    // Toggle LED
    if (g_Toggle == LOW) {
        g_Toggle = HIGH;

        analogWrite(g_PwmPin, 255);
    }
    else {
        g_Toggle = LOW;

        analogWrite(g_PwmPin, 128);
    }

    digitalWrite(g_DigitalPin, g_Toggle);

    // delay is in ms
    delay(5000);
}
