
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
// SensorTest
//
// 05/19/2014
//
// Created from ReadAnalogVoltage example
//
//

float VoltageReference = 3.3;

int LightPin = A0;
int TemperaturePin = A1;
int MoisturePin = A2;

int SensorReading0 = 0; // Light
int SensorReading1 = 0; // Temperature
int SensorReading2 = 0; // Moisture

void setup() {
  Serial.begin(9600);

  // If you want to set the aref to something other than 5v
  //analogReference(EXTERNAL);

  //pinMode(redLEDpin, OUTPUT);
  //pinMode(greenLEDpin, OUTPUT);
  //pinMode(blueLEDpin, OUTPUT);

  //digitalWrite(greenLEDpin, HIGH);
  //digitalWrite(greenLEDpin, LOW);
}

void loop() {

  PerformSensorReadings();

  Serial.print("SensorReading0(Light)=");
  Serial.println(SensorReading0);

  Serial.print("SensorReading1(Temperature)=");
  Serial.println(SensorReading1);

  Serial.print("SensorReading2(Moisture)=");
  Serial.println(SensorReading2);

  delay(5000);
}

void PerformSensorReadings()
{
    SensorReading0 = ReadVoltage(LightPin);
    SensorReading1 = ReadTemperature(TemperaturePin);
    SensorReading2 = ReadVoltage(MoisturePin);
}

// Returns voltage in millivolts so it can be an int
int ReadVoltage(int pinNumber)
{
  int sensorValue = analogRead(pinNumber);

  Serial.print("Raw Reading Pin=");
  Serial.print(pinNumber);
  Serial.print(" Value=");
  Serial.println(sensorValue);

  // Convert the analog reading (which goes from 0 - 1023)
  // to a voltage (0 - VoltageReference):
  float voltage = sensorValue * (VoltageReference / 1023.0);

  Serial.print("voltage=");
  Serial.println(voltage);

  return (int)(voltage * 1000);
}

int ReadTemperature(int tempPin)
{
    int tempReading = analogRead(tempPin);

    // converting that reading to voltage, for 3.3v arduino use 3.3
    float voltage = tempReading * VoltageReference / 1024;
    float temperatureC = (voltage - 0.5) * 100 ;
    float temperatureF = (temperatureC * 9 / 5) + 32;

    return (int)temperatureF;
}
