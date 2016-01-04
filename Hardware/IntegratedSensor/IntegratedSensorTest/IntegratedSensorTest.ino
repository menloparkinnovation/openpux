
// 
//  07/06/2014
//
//  Test Menlo Integrated Sensor
// 


// 
// 07/06/2014
//
// Menlo - From Arduino Blink
//
//
//  - Configure serial and place output
//
//

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// Version5
int red = 9;
int green = 5;
int blue = 3;

// the setup routine runs once when you press reset:
void setup() {                

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
 
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     

  pinMode(red, OUTPUT);     
  pinMode(green, OUTPUT);     
  pinMode(blue, OUTPUT);     
}

void flashcolors(unsigned long delaytime)
{
  digitalWrite(red, HIGH);
  delay(delaytime);
  digitalWrite(red, LOW);

  digitalWrite(green, HIGH);
  delay(delaytime);
  digitalWrite(green, LOW);

  digitalWrite(blue, HIGH);
  delay(delaytime);
  digitalWrite(blue, LOW);
}

// the loop routine runs over and over again forever:
void loop() {

  flashcolors(500);

//  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);               // wait for a second

//  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);               // wait for a second

  // Test serial output
  Serial.println("loop...");

  // Test serial input
  if (Serial.available()) {

    Serial.print("You typed character(s): ");

    while (Serial.available()) {
        char c = Serial.read();
        Serial.print(c);
    }

    Serial.println(":");

    // Blink the LED rapidly 5 times
    for (int index = 0; index < 10; index++) {
        flashcolors(50);
    }
  }
}
