
/*
 * Menlo IoT Button example
 *
 * 05/20/2016
 *
 * MenloPark Innovation LLC
 *
 * Menlo Contributions are MIT license.
 */

/*

Based on the example from SparkFun at:

https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/example-sketch-posting-to-phant

SparkFun code, firmware, and software is released under the MIT
License(http://opensource.org/licenses/MIT).

The MIT License (MIT)

Copyright (c) 2015 SparkFun Electronics

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2016 Menlo Park Innovation LLC
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

// Include the ESP8266 WiFi library. (Works a lot like the Arduino WiFi library.)
#include <ESP8266WiFi.h>

// Fingerprint of https://data.sparkfun.com for SSL
const char fingerPrint[] = "29 9F 93 FA B0 89 37 C4 BC 76 D7 EB DE 13 6E EC 82 A4 4D 68";

//////////////////////
// WiFi Definitions //
//////////////////////

//
// Note: These are case sensitive for both SSID and passphrase
//

const char WiFiSSID[] = "OceanHouse";
const char WiFiPSK[] = "thisisakey321";

//const char WiFiSSID[] = "mastergardener";
//const char WiFiPSK[] = "charlotte81098";

//
// Host
//
const char IotHost[] = "192.168.1.7";

//
// Host Port
//
const int IotHostPort = 8080;

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED
const int ANALOG_PIN = A0; // The only analog pin on the Thing
const int DIGITAL_PIN = 12; // Digital pin to be read

const int BUTTON1_PIN = 4;  // D4 is an interrupt pin, with pullup
const int BUTTON2_PIN = 12; // D12 is an interrupt pin, with pullup

//
// This limits how often posts occur.
//
//const unsigned long postRate = 30000;
const unsigned long postRate = 1000; // instant button feedback
unsigned long lastPost = 0;

void setup() 
{
  initHardware();
  connectWiFi();

  // Turn the LED on
  digitalWrite(LED_PIN, HIGH);
}

//
// Returns true if button1 is pressed.
//
// Note: Buttons are pulled up using input pullups and are LOW
// when pressed.
//
static int button1_state = HIGH;

bool
Button1Pressed()
{
    int state;
    int lastState;

    // https://www.arduino.cc/en/Reference/digitalRead

    //
    // Debounce the button
    //
    lastState = digitalRead(BUTTON1_PIN);
    delay(10);
    state = digitalRead(BUTTON1_PIN);
    
    if ((lastState == LOW) && (state == LOW)) {

        //
        // Button is pressed after debounce.
        //
        // If the last published state was low, we set it
        // high and return true.
        //
        if (button1_state != LOW) {
            button1_state = LOW;
            return true;
        }

        //
        // Button press has already been registered, so
        // return false.
        //

        return false;
    }
    else if ((lastState == HIGH) && (state == HIGH)) {

        //
        // We have a debounced low reading. Reset the state
        // so that we can re-arm for another button push
        // send.
        //

        button1_state = HIGH;

        return false;
    }
    else {
        // Do nothing, the button is bouncing.
        return false;
    }
}

//
// Returns true if button2 is pressed.
//
static int button2_state = HIGH;

bool
Button2Pressed()
{
    int state;
    int lastState;

    // https://www.arduino.cc/en/Reference/digitalRead

    //
    // Debounce the button
    //
    lastState = digitalRead(BUTTON2_PIN);
    delay(10);
    state = digitalRead(BUTTON2_PIN);
    
    if ((lastState == LOW) && (state == LOW)) {

        //
        // Button is pressed after debounce.
        //
        // If the last published state was low, we set it
        // high and return true.
        //
        if (button2_state != LOW) {
            button2_state = LOW;
            return true;
        }

        //
        // Button press has already been registered, so
        // return false.
        //

        return false;
    }
    else if ((lastState == HIGH) && (state == HIGH)) {

        //
        // We have a debounced low reading. Reset the state
        // so that we can re-arm for another button push
        // send.
        //

        button2_state = HIGH;

        return false;
    }
    else {
        // Do nothing, the button is bouncing.
        return false;
    }
}

//
// This allows retry attempts at send when a button
// is pressed.
//
static bool button1_pressed = false;
static bool button2_pressed = false;

void loop() 
{
    if (Button1Pressed()) {
        Serial.println("Button 1 Pressed!");
        button1_pressed = true;
    }

    if (Button2Pressed()) {
        Serial.println("Button 2 Pressed!");
        button2_pressed = true;
    }

    //
    // This limits the rate that message are sent
    //
    if (lastPost + postRate <= millis())
    {
        //
        // Process button 1 event
        //
        if (button1_pressed) {
            Serial.println("sending button 1 press event");

            if (postToButtonService(1)) {
                lastPost = millis();
                button1_pressed = false;
            }
            else {
                // Wait 10 seconds on failure
               delay(10000);
            }
        }

        //
        // Process button 2 event
        //
        if (button2_pressed) {
            Serial.println("sending button 2 press event");

            if (postToButtonService(2)) {
                lastPost = millis();
                button2_pressed = false;
            }
            else {
                // Wait 10 seconds on failure
               delay(10000);
            }
        }
    }
}

void connectWiFi()
{
  byte ledStatus = LOW;

  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  Serial.println("WiFi.begin");

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  Serial.println("Waiting for WiFi connection, blue LED will flash 100ms");

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }

  Serial.println("WiFi connection established");

}

void initHardware()
{
  Serial.begin(9600);

  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Configure Button pins
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  //
  // Been receiving phanton power on events, so clear out
  // any digital latches due to the above mode change.
  //
  delay(10);

  // Clear any latched status
  digitalRead(BUTTON1_PIN);
  digitalRead(BUTTON2_PIN);

  delay(10);

  // Turn the LED off
  digitalWrite(LED_PIN, LOW);

  // Don't need to set ANALOG_PIN as input, 
  // that's all it can be.
}

static const char HEADER_POST_URL1[] PROGMEM = "POST /iotbutton";
static const char HEADER_POST_URL2[] PROGMEM = " HTTP/1.1\n";
static const char HEADER_CONNECTION_CLOSE[] PROGMEM = "Connection: close\n";
static const char HEADER_CONTENT_TYPE[] PROGMEM = "Content-Type: application/json\n";
static const char HEADER_CONTENT_LENGTH[] PROGMEM = "Content-Length: ";

String generatePostContent(String host, String params)
{
  String result;

  //String result = "POST /iotbutton" HTTP/1.1\n";
  for (int i=0; i<strlen(HEADER_POST_URL1); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_POST_URL1 + i);
  }

  for (int i=0; i<strlen(HEADER_POST_URL2); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_POST_URL2 + i);
  }

  result += "Host: " + host + "\n";

  //result += "Connection: close\n";
  for (int i=0; i<strlen(HEADER_CONNECTION_CLOSE); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONNECTION_CLOSE + i);
  }

  //result += "Content-Type: application/json\n";
  for (int i=0; i<strlen(HEADER_CONTENT_TYPE); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONTENT_TYPE + i);
  }	

  //result += "Content-Length: " + String(params.length()) + "\n\n";
  for (int i=0; i<strlen(HEADER_CONTENT_LENGTH); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONTENT_LENGTH + i);
  }	

  result += String(params.length()) + "\n\n";
  result += params;

  return result;
}

int postToButtonService(int buttonNumber)
{
  int index;
  int c;

  // 10 seconds
  unsigned long timeout = 10L * 1000L;

  static char response[512];
  static int responseLength = 0;

  // LED turns on when we enter, it'll go off when we 
  // successfully post.
  digitalWrite(LED_PIN, HIGH);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String postedID = "IoTButton-" + macID;

  Serial.print("ThingID=");
  Serial.println(postedID);

  // Now connect to data.sparkfun.com, and post our data:
#if 1
  WiFiClient client;
  int httpPort = IotHostPort;

  if (!client.connect(IotHost, httpPort)) 
  {
    // If we fail to connect, return 0.
    Serial.println("Failed to connect on non-SSL");

    // ensures we don't leave a half connection hanging
    client.stop();

    return 0;
  }

#else

  //
  // Test SSL support
  //
  WiFiClientSecure client;
  int httpPort = IotHostPort;

  if (!client.connect(IotHost, httpPort)) 
  {
    // If we fail to connect, return 0.
    Serial.println("Failed to connect on SSL");

    // ensures we don't leave a half connection hanging
    client.stop();

    return 0;
  }

  if (client.verify(fingerPrint, IotHost)) {
      Serial.println("Site SSL fingerprint OK");
  }
  else {
      Serial.println("Site SSL fingerprint *NOT* OK");
  }

#endif

  String jsonDocument;

  if (buttonNumber == 1) {
      jsonDocument += "{\"device_id\": \"123\", \"button_state\": \"1 pressed\", \"battery_level\": \"90\"}";
  }
  else {
      jsonDocument += "{\"device_id\": \"123\", \"button_state\": \"2 pressed\", \"battery_level\": \"90\"}";
  }

  // If we successfully connected, print our post:
  client.print(generatePostContent(IotHost, jsonDocument));

  //
  // Now get any received document response
  //
  index = 0;

  while (client.connected() && (client.available() || (timeout-- > 0))) {

        c = client.read();
        if (c == (-1)) {
            // This ensures the timeout value represents a count of milliseconds
            delay(1);
            continue;
        }

        // Leave room for terminating NUL '\0'
        if (index < (sizeof(response) - 1)) {

            // add to response buffer
            response[index++] = c;
        }
        else {

            //
            // we continue to drain characters until timeout as to
            // not reset/hang the connection.
            //
        }
  }

  responseLength = index;
  response[index] = '\0'; // ensure NUL terminated

  //
  // We are done with the connection, close it now before any
  // serial output so we don't fire off any timeouts
  //
  client.stop();

  if (index == (sizeof(response) - 1)) {
      Serial.println("maximum document size received, truncated");
  }

  Serial.print(response);

  // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, LOW);

  Serial.println("HTTP success.");

  return 1; // Return success
}
