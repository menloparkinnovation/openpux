
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

char ssid[] = "OceanHouse"; //  your network SSID (name) 
char pass[] = "thisisakey321";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

char server[] = "192.168.1.11";
int serverport = 8080;

// 30 seconds
unsigned long SensorUpdateInterval = (30 * 1000);

char* openpuxUrl = "/smartpuxdata/data";

int status = WL_IDLE_STATUS;

//
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//
//IPAddress server(192,168,1,11);

// Initialize the WiFi client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

// To manage the connection state machine
boolean lastConnected = false;

void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true) {
        delay(5000);
    }
  } 

  String fv = WiFi.firmwareVersion();
  if( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");
  
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
  
  Serial.println("\nDone with setup()");
}

void loop() {

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

void printWifiStatus() {

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
// MenloParkInnovation LLC openpux code
//

//
// Note: On the Intel Galileo at least we are running inside
// a full Linux process and not an Atmel 328 with only
// 2K of RAM. So we can use standard content length buffers
// and not require a deep, complex state machine for HTTP
// receives.
//

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

      SendSensorUpdate();

      // reset starttime
      startTime = millis();
  }
  else {
      //Serial.println("SensorLoop waiting...");
  }
}

void
SendSensorUpdate()
{
  char* content;

  Serial.println("Sending Sensor Readings:");

  content = "A=1&P=12345678&S=1&D0=0&D1=1&D2=2&D3=3&M0=0&M1=1&M2=2&M3=3";

  SendHttpPost(server, serverport, openpuxUrl, content);
}

#define CONTENT_BUFFER_SIZE 512

int ContentBufferLength = 0;
int ContentBufferSize = CONTENT_BUFFER_SIZE;
char ContentBuffer[CONTENT_BUFFER_SIZE];

void
ContentBufferReset()
{
    ContentBufferLength = 0;
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
