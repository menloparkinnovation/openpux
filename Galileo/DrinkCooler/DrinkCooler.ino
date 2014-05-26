
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

/*
  Wifi Openpux Web client

  DrinkCooler application.

 Based on Arduino/Galileo WiFiWebClient example.

 This example is written for a network using WPA encryption. For 
 WEP or WPA, change the Wifi.begin() call accordingly.
 
 Circuit:
 * WiFi shield attached
 
 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */

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

    // Setup WiFi, sign on
    WiFi_Setup();

    Serial.println("\nDone with setup()");
}

void
loop()
{

    if (client.available()) {
        ProcessDataAvailable();

        // This appears to be needed to reset the client
        // state machine, otherwise client send errors occur.
        client.stop();
    }

    // if there's no net connection, but there was one last time
    // through the loop, then stop the client:
    if (!client.connected() && lastConnected) {
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
    }

    SensorLoop();

    // store the state of the connection for next time through
    // the loop:
    lastConnected = client.connected();
}

//
// WiFi Support
//

void
WiFi_Setup()
{
    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present"); 
       // don't continue:
       while(true) {
          delay(5000);
       }
    } 

    String fv = WiFi.firmwareVersion();
    if( fv != "1.1.0" ) {
        Serial.println("Please upgrade the firmware");
    }
  
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) { 

        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);

        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
        status = WiFi.begin(ssid, pass);
  
        // wait 10 seconds for connection:
        delay(10000);
    } 

    Serial.println("Connected to wifi");
    printWifiStatus();
}

void
printWifiStatus()
{

    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
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

//
// MenloParkInnovation LLC openpux Cloud Connection Code
//

//
// Note: On the Intel Galileo at least we are running inside
// a full Linux process and not an Atmel 328 with only
// 2K of RAM. So we can use standard content length buffers
// and not require a deep, complex state machine for HTTP
// receives.
//

void
GenerateTestData()
{
    SensorReading0 = 0x30;
    SensorReading1 = 0x31;
    SensorReading2 = 0x32;
    SensorReading3 = 0x33;
    SensorReading4 = 0x34;
    SensorReading5 = 0x35;
    SensorReading6 = 0x36;
    SensorReading7 = 0x37;
    SensorReading8 = 0x38;
    SensorReading9 = 0x39;

    TargetMask0 = 0x40;
    TargetMask1 = 0x41;
    TargetMask2 = 0x42;
    TargetMask3 = 0x43;
    TargetMask4 = 0x44;
    TargetMask5 = 0x45;
    TargetMask6 = 0x46;
    TargetMask7 = 0x47;
    TargetMask8 = 0x48;
    TargetMask9 = 0x49;
}

void
GenerateTestDataSequential()
{
static int testdata = 0x0;

    // Generate test data
    SensorReading0 = testdata++;
    SensorReading1 = testdata++;
    SensorReading2 = testdata++;
    SensorReading3 = testdata++;
    SensorReading4 = testdata++;
    SensorReading5 = testdata++;
    SensorReading6 = testdata++;
    SensorReading7 = testdata++;
    SensorReading8 = testdata++;
    SensorReading9 = testdata++;

    TargetMask0 = testdata++;
    TargetMask1 = testdata++;
    TargetMask2 = testdata++;
    TargetMask3 = testdata++;
    TargetMask4 = testdata++;
    TargetMask5 = testdata++;
    TargetMask6 = testdata++;
    TargetMask7 = testdata++;
    TargetMask8 = testdata++;
    TargetMask9 = testdata++;
}

void
SensorLoop()
{
  char* content;
  static unsigned long startTime = 0;

  unsigned long timeout = SensorUpdateInterval;

  //
  // The loop() can't be blocked since HTTP sends and receives
  // are async and the loop() is required to process the state
  // machine.
  //
  if ( ( millis() - startTime ) > timeout ) {

      // Get lastest readings values
      UpdateSensorReadings();

      SendSensorUpdate();

      // reset starttime
      startTime = millis();
  }
  else {
      //Serial.println("SensorLoop waiting...");
  }
}

#define CONTENT_BUFFER_SIZE 512

int ContentBufferLength = 0;
int ContentBufferSize = CONTENT_BUFFER_SIZE;
char ContentBuffer[CONTENT_BUFFER_SIZE];

void
SendSensorUpdate()
{
  char* content;

  Serial.println("Sending Sensor Readings:");

  content = GenerateSensorContent();

  SendHttpPost(server, serverport, openpuxUrl, content);
}

char*
GenerateSensorContent()
{
  char* content;

  ContentBufferReset();

  GenerateNameValuePair("A", 1, true);
  GenerateNameValuePair("P", 12345678, false);
  GenerateNameValuePair("S", 1, false);

  //
  // First four TargetMasks are always sent
  // even if 0 since they can represent outstanding
  // commands.
  //
  GenerateNameValuePair("M0", TargetMask0, false);

  GenerateNameValuePair("M1", TargetMask1, false);

  GenerateNameValuePair("M2", TargetMask2, false);

  GenerateNameValuePair("M3", TargetMask3, false);

  //
  // Optional TargetMasks 4-9 are sent if set.
  //
  if (TargetMask4 != 0) {
      GenerateNameValuePair("M4", TargetMask4, false);
  }

  if (TargetMask5 != 0) {
      GenerateNameValuePair("M5", TargetMask5, false);
  }

  if (TargetMask6 != 0) {
      GenerateNameValuePair("M6", TargetMask6, false);
  }

  if (TargetMask7 != 0) {
      GenerateNameValuePair("M7", TargetMask7, false);
  }

  if (TargetMask8 != 0) {
      GenerateNameValuePair("M8", TargetMask8, false);
  }

  if (TargetMask9 != 0) {
      GenerateNameValuePair("M9", TargetMask9, false);
  }

  //
  // First four SensorReadings are always sent
  // even if 0 since they can represent outstanding
  // commands.
  //

  GenerateNameValuePair("D0", SensorReading0, false);
  GenerateNameValuePair("D1", SensorReading1, false);
  GenerateNameValuePair("D2", SensorReading2, false);
  GenerateNameValuePair("D3", SensorReading3, false);

  if (SensorReading4 != 0) {
      GenerateNameValuePair("D4", SensorReading4, false);
  }

  if (SensorReading5 != 0) {
      GenerateNameValuePair("D5", SensorReading5, false);
  }

  if (SensorReading6 != 0) {
      GenerateNameValuePair("D6", SensorReading6, false);
  }

  if (SensorReading7 != 0) {
      GenerateNameValuePair("D7", SensorReading7, false);
  }

  if (SensorReading8 != 0) {
      GenerateNameValuePair("D8", SensorReading8, false);
  }

  if (SensorReading9 != 0) {
      GenerateNameValuePair("D9", SensorReading9, false);
  }

  // NULL terminator
  ContentBufferAdd(0);

  content = ContentBuffer;

  return content;
}

void
GenerateNameValuePair(char* name, int value, int begin)
{
    char buffer[17];

    if (!begin) {
        ContentBufferAddString("&");
    }

    ContentBufferAddString(name);
    ContentBufferAddString("=");

    snprintf(buffer, sizeof(buffer), "%x", value);
    ContentBufferAddString(buffer);
}

void
ContentBufferReset()
{ 
   ContentBufferLength = 0;
}

void
ContentBufferAddString(char* string)
{
    ContentBufferAdd(string, strlen(string));
}

void
ContentBufferAdd(char* buf, int len)
{
    if ((ContentBufferLength + len) > ContentBufferSize) {
        Serial.println("Overflow on content buffer");
        return;
    }

    memcpy(&ContentBuffer[ContentBufferLength], buf, len);
    ContentBufferLength += len;
}

void
ContentBufferAdd(char c)
{
    if (ContentBufferLength > ContentBufferSize) {
        Serial.println("Overflow on content buffer receive");
        return;
    }

    ContentBuffer[ContentBufferLength++] = c;
}

void
ProcessDataAvailable()
{
  Serial.println("ProcessDataAvailable...");

  //
  // if there are incoming bytes available 
  // from the server, read them and print them:
  //
  while (client.available()) {
    char c = client.read();

    ContentBufferAdd(c);

    Serial.write(c);
  }

  ProcessResponse();

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
  }
}

void
ProcessResponse()
{
    if (ContentBufferLength == 0) {
        return;
    }

    // Parse and process response...
    Serial.println("ContentBuffer Begin:");

    Serial.print(ContentBuffer);

    Serial.println("ContentBuffer End:");

    HttpResponseData(ContentBuffer, ContentBufferLength);
}

void
SendHttpGet(
    char* server,
    int port,
    char* url
    )
{
  // if you get a connection, report back via serial:
  ContentBufferReset();

  HttpResponseReady();

  if (client.connect(server, port)) {

    Serial.println("connected to server");

    // Make a HTTP request:
    client.print("GET ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.println("Host: www.openpux.com");
    client.println("Connection: close");
    client.println();
  }
}

void
SendHttpPost(
    char* server,
    int port,
    char* url,
    char* content
    )
{
  int contentLength;

  ContentBufferReset();

  HttpResponseReady();

  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {

    Serial.print("connected to server ");
    Serial.print(server);
    Serial.print(" port ");
    Serial.println(port);

    Serial.print("url=");
    Serial.println(url);

    client.print("POST ");
    client.print(url);
    client.println(" HTTP/1.1");

    client.println("Host: www.openpux.com");

    contentLength = strlen(content);

    Serial.print("sending ContentLength=");
    Serial.println(contentLength);

    client.print("Content-Length: ");
    client.println(contentLength);

    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");

    // Terminate headers, begin content section
    client.print("\n");

    Serial.print("content=");
    Serial.println(content);

    // output content
    client.println(content);

    client.print("\n");

    // Terminate the content with two \n's
    //client.print("\n\n");
  }
  else {
      Serial.println("SendHttpPost: error connecting to server");
  }
}

//
// HTTP response processing is designed to be supported on tiny
// microcontrollers with minimal buffer space. As a result
// it operates as a state machine with minimal history while
// processing a response.
//

//
// TCP can return data on any record boundary and this
// follows for the HTTP response received. Since minimal to
// no buffering is available inside tiny microcontroller
// implementations, the state machine directly processes the incoming
// byte stream handles HTTP state transitions, HTTP header processing,
// response document processing, and response document parsing into
// binary values.
//

//
// Systems with larger buffers can have a simpler state machine
// to process a line at a time, buffer an entire response
// document, array of HTTP headers, etc. We have no such
// luxury with embedded systems such as AtMega328's with
// 2kb of total RAM for heap, stack, and initialized data.
//

//
// This is the decoded content from the sensor
// response document.
//
typedef struct _SensorResponseData {
  int command;
  int sleepTime;
  int targetMask0;
  int targetMask1;
  int targetMask2;
  int targetMask3;
  int targetMask4;
  int targetMask5;
  int targetMask6;
  int targetMask7;
  int targetMask8;
  int targetMask9;
} SensorResponseData;

//
// This processes the final decoded result
// document by the sensor.
//
void
ProcessSensorResponseDocument(
    //SensorResponseData* sensor
    struct _SensorResponseData* sensor
    )
{
    Serial.println("*** Cloud Response to Sensor ***");

    Serial.print(" command ");
    Serial.println(sensor->command, HEX);

    Serial.print(" sleepTime ");
    Serial.println(sensor->sleepTime, HEX);

    Serial.print(" targetMask0 ");
    Serial.println(sensor->targetMask0, HEX);

    Serial.print(" targetMask1 ");
    Serial.println(sensor->targetMask1, HEX);

    Serial.print(" targetMask2 ");
    Serial.println(sensor->targetMask2, HEX);

    Serial.print(" targetMask3 ");
    Serial.println(sensor->targetMask3, HEX);

    Serial.println("*** End Cloud Response to Sensor ***");

    // Update operational settings from the Cloud response
    Command = sensor->command;
    SleepTime = sensor->sleepTime;
    TargetMask0 = sensor->targetMask0;
    TargetMask1 = sensor->targetMask1;
    TargetMask2 = sensor->targetMask2;
    TargetMask3 = sensor->targetMask3;
    TargetMask4 = sensor->targetMask4;
    TargetMask5 = sensor->targetMask5;
    TargetMask6 = sensor->targetMask6;
    TargetMask7 = sensor->targetMask7;
    TargetMask8 = sensor->targetMask8;
    TargetMask9 = sensor->targetMask9;

    // Allow the application to process the new targetmasks
    OperateOnSensorUpdate();
}

//
// HTTP receive state machine
//

#if DEBUG_TRACE_ON
#define DEBUG_TRACE(x) Serial.println(x)
#define DEBUG_TRACE2(x, y) Serial.println(x, y)
#define DEBUG_TRACE3(x, y, z) Serial.println(x, y, z)
#else
#define DEBUG_TRACE(x)
#define DEBUG_TRACE2(x, y)
#define DEBUG_TRACE3(x, y, z)
#endif

#define HTTP_TOKEN_HTTP1_1_OK         0
#define HTTP_TOKEN_CONTENT_URLENCODED 1

#define HTTP_TOKENS_NUMBER            2

//
// Tokens/response lines we look for
//

// Document if any order is required, optimal
// such as:
//
// stringfulllength
// string
//
char* HttpHeaderTokens[] = {
  "HTTP/1.1 200 OK",
  "Content-Type: application/x-www-form-urlencoded"
};

int HttpHeaderTokensSize = 2;

//
// This contains the state for the HTTP response
// state machine.
//
typedef struct _HttpResponseState {
    char documentOK;
    char lastChar;
    char lastTokenNewline;
    char endOfHeaders;
    char candidateToken;
    char candidateTokenIndex;
    char TokensFound[HTTP_TOKENS_NUMBER];
} HttpResponseState;

//
// This contains the state for the Sensor response
// document state machine.
//
#define SENSOR_RESPONSE_MAX_DIGITS 8

typedef struct _SensorResponseState {
    char EndOfDocument;
    char NameChar;
    char NameChar2;
    char DigitsIndex;
    char Digits[SENSOR_RESPONSE_MAX_DIGITS + 1]; // +1 for null
} SensorResponseState;

struct _HttpResponseState g_httpData;
struct _SensorResponseState g_sensorStateData;
struct _SensorResponseData g_sensorData;

// Prepare the state machine for a new response
void
HttpResponseReset(
    struct _HttpResponseState* http,
    struct _SensorResponseState* sensorState,
    struct _SensorResponseData* sensor
    )
{
  int index;

  http->documentOK = 0;
  http->endOfHeaders = 0;
  http->candidateTokenIndex = 0;
  http->lastTokenNewline = 0;

  http->candidateToken = -1;

  // lastChar initialized to '\n' allows a new line to to be recognized at start
  http->lastChar = '\n';

  // Intitalize our HTTP receiver headers state machine
  for(index = 0; index < HTTP_TOKENS_NUMBER; index++) {
    http->TokensFound[index] = 0;
  }

  // Initialize our response document data
  sensorState->EndOfDocument = 0;
  sensorState->NameChar = 0;
  sensorState->NameChar2 = 0;
  sensorState->DigitsIndex = -1;

  // Initialize parsed result data
  sensor->command = 0;
  sensor->sleepTime = 0;
  sensor->targetMask0 = 0;
  sensor->targetMask1 = 0;
  sensor->targetMask2 = 0;
  sensor->targetMask3 = 0;
  sensor->targetMask4 = 0;
  sensor->targetMask5 = 0;
  sensor->targetMask6 = 0;
  sensor->targetMask7 = 0;
  sensor->targetMask8 = 0;
  sensor->targetMask9 = 0;
}

void
HttpProcessSensorDocument(
//    HttpResponseState* http,
//    SensorResponseState* sensorState,
//    SensorResponseData* sensor,
    struct _HttpResponseState* http,
    struct _SensorResponseState* sensorState,
    struct _SensorResponseData* sensor,
    char* buf,
    int len
    )
{
  int value;

  DEBUG_TRACE2("HttpProcessSensorDocument: len %d\n", len);

    while (len > 0) {

        if (sensorState->EndOfDocument) {
            // Swallow rest of buffer(s) at end of document
            return;
        }

        // C=0&S=3C&M0=0&M1=0&M2=0&M3=99

	if((sensorState->DigitsIndex != -1) &&
	     (((buf[0] >= '0') && (buf[0] <= '9')) ||
	     ((buf[0] >= 'a') && (buf[0] <= 'f')) ||
	     ((buf[0] >= 'A') && (buf[0] <= 'F')))) {

	    if(sensorState->DigitsIndex >= SENSOR_RESPONSE_MAX_DIGITS) {
		// Abandon current number, to long
		DEBUG_TRACE("Number to Long\n");
		sensorState->NameChar = 0;
		sensorState->DigitsIndex = -1;
	    }

	    sensorState->Digits[sensorState->DigitsIndex] = buf[0];
	    sensorState->DigitsIndex++;
	}
	else {

	  switch(buf[0]) {

	  case '\0':
	    // a null in the HTTP response stream is skipped
	    break;

	  case '=':
	    if (sensorState->NameChar != 0) {
	       DEBUG_TRACE2("Found = for %c, switching to digits state\n", sensorState->NameChar);
	       // Switch to recording numbers
	       sensorState->DigitsIndex = 0;
	    }
	    break;

	  case '\n':

	      // Two Newline's ("\r\n") represents end of document
	      if ((http->lastChar == '\r') && http->lastTokenNewline) {
		  DEBUG_TRACE("EndOfDocument\n");
		  sensorState->EndOfDocument = 1;
		  ProcessSensorResponseDocument(sensor);
		  return;
	      }

	  case '\r':
	      // Fall through since '\r' ends any current name=value item

	  case '&':
	    // End of current name=value

	    if (sensorState->DigitsIndex != -1) {
	      sensorState->Digits[sensorState->DigitsIndex] = 0;

  	      value = strtol(sensorState->Digits, NULL, 16);
    
    	      DEBUG_TRACE2("Found & or \\r, ending current value %x\n", value);

    	      if (sensorState->NameChar == 'C') {
                DEBUG_TRACE2("Setting value %x for C\n", value);
		sensor->command = value;
	      }
	      else if (sensorState->NameChar == 'S') {
                DEBUG_TRACE2("Setting value %x for S\n", value);
		sensor->sleepTime = value;
	      }
	      else if (sensorState->NameChar == 'M') {
		switch(sensorState->NameChar2) {
		case '0':
                DEBUG_TRACE2("Setting value %x for M0\n", value);
		sensor->targetMask0 = value;
		break;
		case '1':
                DEBUG_TRACE2("Setting value %x for M1\n", value);
		sensor->targetMask1 = value;
		break;
		case '2':
                DEBUG_TRACE2("Setting value %x for M2\n", value);
		sensor->targetMask2 = value;
		break;
		case '3':
                DEBUG_TRACE2("Setting value %x for M3\n", value);
		sensor->targetMask3 = value;
		break;
		case '4':
                DEBUG_TRACE2("Setting value %x for M4\n", value);
		sensor->targetMask4 = value;
		break;
		case '5':
                DEBUG_TRACE2("Setting value %x for M5\n", value);
		sensor->targetMask5 = value;
		break;
		case '6':
                DEBUG_TRACE2("Setting value %x for M6\n", value);
		sensor->targetMask6 = value;
		break;
		case '7':
                DEBUG_TRACE2("Setting value %x for M7\n", value);
		sensor->targetMask7 = value;
		break;
		case '8':
                DEBUG_TRACE2("Setting value %x for M8\n", value);
		sensor->targetMask8 = value;
		break;
		case '9':
                DEBUG_TRACE2("Setting value %x for M9\n", value);
		sensor->targetMask9 = value;
		break;
    		}
	      }

	      sensorState->NameChar = 0;
	      sensorState->DigitsIndex = -1;
    	    }
    
    	    break;

	  case 'C':
	  case 'S':
	  case 'M':
	  case 'E':
	    DEBUG_TRACE2("Found keyword %c\n", buf[0]);
	    sensorState->NameChar = buf[0];
	    break;

	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    if (sensorState->NameChar == 'M') {
		DEBUG_TRACE2("Found Mask %c\n", buf[0]);
		sensorState->NameChar2 = buf[0];
	    }
	    else {
		DEBUG_TRACE2("Number not part of a Mask %c\n", buf[0]);
	    }
	    break;

	  default:

	    // Unrecognized character, stop current name=value processing
	    DEBUG_TRACE2("Unrecognized character %c\n", buf[0]);
	    sensorState->NameChar = 0;
	    sensorState->DigitsIndex = -1;

	    break;
         }
     }

	 //
	 // Newline is a two character sequence, so we handle here so as
	 // not to complicate the above switch()
	 //
	 if ((buf[0] == '\n') && (http->lastChar == '\r')) {
	     http->lastTokenNewline = 1;
	 }
	 else if (buf[0] == '\r') {
	     // do nothing, as we may be setting up for \r\n\r\n
	 }
	 else {
	     http->lastTokenNewline = 0;
	 }

	 http->lastChar = buf[0];

	 buf++;
         len--;
    }

  return;
}

//
// Find the token that starts with prefix and
// matches the newChar after the prefix.
//
// Skips the entry indexToSkip if != -1 to allow
// search for "all, but entry".
//
int
HttpFindCandidateToken(
    char* prefix,
    char  newChar,
    int indexToSkip
    )
{
  int len;
  int index;

  for(index = 0; index < HttpHeaderTokensSize; index++) {

    len = strlen(HttpHeaderTokens[index]);

    if (strncmp(HttpHeaderTokens[index], prefix, len) == 0) {

      if ((index != indexToSkip) &&
          (HttpHeaderTokens[index][len] == newChar)) {

        return index;
      }
    }
  }

  return -1;
}

void
HttpResponseProcessBuffer(
    struct _HttpResponseState* http,
    struct _SensorResponseState* sensorState,
    struct _SensorResponseData* sensor,
    char* buf,
    int len
    )
{
  int index;

  //
  // TCP streams bring in data in any chunk size, at any
  // boundary.
  //
  // This processes the HTTP response stream looking first
  // for a valid set of HTTP headers and the headers end
  // response ("\r\n\r\n"). It then switches to processing
  // the expected www-urlencoded document stream till end
  // of connection.
  //

  //
  // TODO: Currently does not obey content length, but uses
  // connection close as an indication of done.
  //

  DEBUG_TRACE2("HttpResponseProcessBuffer: len %d\n", len);

  while (len-- > 0) {

    //
    // At any time we can get end of headers and
    // have to switch to content processing.
    //
    if (http->endOfHeaders) {

      //
      // We only process the content response if the headers
      // are what we expected, and not an error response from
      // the cloud server.
      //
      if (http->documentOK) {

          // Processing HTTP response document/content
          HttpProcessSensorDocument(http, sensorState, sensor, buf, len);
      }
    }
    else {

      // Processing HTTP response headers

      //
      // HTTP header tokens are matched from the begining of
      // a line, but may have trailing data.
      //

      switch(buf[0]) {

      case '\0':
        // a null in the HTTP response stream is skipped
        break;

      case '\n':

          // Two newlines (of "\r\n" each) represents end of HTTP response headers
  	  if ((http->lastChar == '\r') && http->lastTokenNewline) {

            DEBUG_TRACE("End of Headers\n");
	    http->endOfHeaders = 1;

            //
            // See if its an OK response document that
	    // we recognize
            //
            if (http->TokensFound[HTTP_TOKEN_HTTP1_1_OK] &&
                http->TokensFound[HTTP_TOKEN_CONTENT_URLENCODED]) {
              DEBUG_TRACE("Document OK\n");
	      http->documentOK = 1;
	    }
            else {
              DEBUG_TRACE("Http Response document not recognized\n");
            }

            break;
          }

        // fall through

      case '\r':
        // fall through

      default:

        // See if we have a token in progress
        if (http->candidateToken != (-1)) {
          
	  http->candidateTokenIndex++;

          // if reached end of candidateToken, complete it
 	  if ('\0' == HttpHeaderTokens[http->candidateToken][http->candidateTokenIndex]) {
            DEBUG_TRACE2("Completing token %s\n", HttpHeaderTokens[http->candidateToken]);
	    http->TokensFound[http->candidateToken] = 1;
            http->candidateToken = -1;
            http->candidateTokenIndex = 0;
          }
	  else if(buf[0] == '\r') {
	    // '\r' ends the search for the current header line and begins a new one
            http->candidateToken = -1;
            http->candidateTokenIndex = 0;
          }
	  else if (buf[0] != HttpHeaderTokens[http->candidateToken][http->candidateTokenIndex]) {

            //
            // Character does not match current candidate token, see if
	    // there is a token which does match
	    //
            int saveIndex = http->candidateToken;

            http->candidateToken = HttpFindCandidateToken(
		  HttpHeaderTokens[http->candidateToken],
                  buf[0],
                  index);

            if (http->candidateToken == (-1)) {
	        // Cancel the current candidate, hit a no match character
                DEBUG_TRACE3("Canceling token %s due to no match character %c\n",
                    HttpHeaderTokens[saveIndex], buf[0]);
                http->candidateTokenIndex = 0;
            }
	  }
	}

        //
        // Search for a new candidate if this is the start of a new line
	//
        if ((http->lastChar == '\n') && http->candidateToken == (-1)) {
	  for(index = 0; index < HttpHeaderTokensSize; index++) {
            if (HttpHeaderTokens[index][0] == buf[0]) {
              DEBUG_TRACE2("Found new candidate token %s\n", HttpHeaderTokens[index]);
	      http->candidateToken = index;
	    }
	  }
        }

        break;
      }
    }

    //
    // Newline is a two character sequence, so we handle here so as
    // not to complicate the above switch()
    //
    if ((buf[0] == '\n') && (http->lastChar == '\r')) {
      http->lastTokenNewline = 1;
    }
    else if (buf[0] == '\r') {
      // do nothing, as we may be setting up for \r\n\r\n
    }
    else {
      http->lastTokenNewline = 0;
    }

    http->lastChar = buf[0];

    buf++;
  }

  return;
}

//
// This is to keep the definitions of the state machine
// structures out of the rest of the utility.
//
void
HttpResponseReady()
{
  HttpResponseReset(&g_httpData, &g_sensorStateData, &g_sensorData);
}

void
HttpResponseData(
    char *buf,
    int len
    )
{
  HttpResponseProcessBuffer(&g_httpData, &g_sensorStateData, &g_sensorData, buf, len);
}
