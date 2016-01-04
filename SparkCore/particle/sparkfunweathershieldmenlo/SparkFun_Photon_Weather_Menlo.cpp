
//------------------ Cloud Configuration -------------------------------------

//
// These variables are up top to be easy to find for per project
// configuration. Note they are before the headers, so symbolic
// definitions are not yet available.
//

const long default_cloud_update_rate_in_seconds = 30;

// Smartpux/Openpux configuration
const char smartpux_host[] = "www.smartpux.com";
const int  smartpux_port = 80;
const char smartpux_token[] = "12345678";
const char smartpux_account[] = "1";
const char smartpux_sensor[] = "2";

// NOTE: Photon resets after 8 seconds in a routine.
const unsigned long smartpux_timeout = 3000;

// Phant configuration
const char server[] = "data.sparkfun.com";
const unsigned long phant_timeout = 3000;

// Whidbey Weather Phant Account
const char publicKey[] = "DJR7yANKy3slL4qmJ8q7";
//const char privateKey[] = "xxxxxxxxxxxxxxxxxxxx";
const char privateKey[] = "P41W7yKz7jsxR87DmP7N";

//------------------ Cloud Configuration -------------------------------------

//------------------ Headers/CopyRights --------------------------------------

/******************************************************************************
  SparkFun_Photon_Weather_Basic_Soil_Meters.ino
  SparkFun Photon Weather Shield basic example with soil moisture and temp
  and weather meter readings including wind speed, wind direction and rain.
  Joel Bartlett @ SparkFun Electronics
  Original Creation Date: May 18, 2015

  Based on the Wimp Weather Station sketch by: Nathan Seidle
  https://github.com/sparkfun/Wimp_Weather_Station

  This sketch prints the temperature, humidity, barrometric preassure, altitude,
  soil moisture, and soil temperature to the Seril port. This sketch also
  incorporates the Weather Meters avaialbe from SparkFun (SEN-08942), which allow
  you to measure Wind Speed, Wind Direction, and Rainfall. Upload this sketch
  after attaching a soil moisture and or soil temperature sensor and Wetaher
  Meters to test your connections.

  Hardware Connections:
	This sketch was written specifically for the Photon Weather Shield,
	which connects the HTU21D and MPL3115A2 to the I2C bus by default.
  If you have an HTU21D and/or an MPL3115A2 breakout,	use the following
  hardware setup:
      HTU21D ------------- Photon
      (-) ------------------- GND
      (+) ------------------- 3.3V (VCC)
       CL ------------------- D1/SCL
       DA ------------------- D0/SDA

    MPL3115A2 ------------- Photon
      GND ------------------- GND
      VCC ------------------- 3.3V (VCC)
      SCL ------------------ D1/SCL
      SDA ------------------ D0/SDA

    Soil Moisture Sensor ----- Photon
        GND ------------------- GND
        VCC ------------------- D5
        SIG ------------------- A1

    DS18B20 Temp Sensor ------ Photon
        VCC (Red) ------------- 3.3V (VCC)
        GND (Black) ----------- GND
        SIG (White) ----------- D4


  Development environment specifics:
  	IDE: Particle Dev
  	Hardware Platform: Particle Photon
                       Particle Core

  This code is beerware; if you see me (or any other SparkFun
  employee) at the local, and you've found our code helpful,
  please buy us a round!
  Distributed as-is; no warranty is given.
*******************************************************************************/

//
//   Copyright (C) 2014,2015 Menlo Park Innovation LLC
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
//   This specific snapshot is made available under the following license:
//
//   Same license as the code from SparkFun above so as not to conflict
//   with the original licensing of the code. Please include any Menlo
//   Park Innovation LLC employees in the beer rounds at the next
//   hacker fest!
//
//    >> This code is beerware; if you see me (or any other SparkFun
//    >> employee) at the local, and you've found our code helpful,
//    >> please buy us a round!
//    >> Distributed as-is; no warranty is given.
//

//------------------ Headers/CopyRights --------------------------------------

//------------------ Notes ---------------------------------------------------

//
// Code Style:
//
// Note on some conventions:
//
// Even though this an Arduino style "sketch" its organized in a semi
// component fashion. This is a way for staging the movement of routines
// into C++ library classes for re-use in other projects.
//
// As this is a staging project, it uses mock C++ naming convention
// of Component_Routine() though its still a mainly C project.
//
// Line lengths are tried to be limited to less than 80, as it enables
// use of emacs side by side a command window on a MacBook Air 13", the
// benchmark of hackerfest's.
//
// Also "literate" style is used to make the code more readable.
//
//    float temp = 0;
//    for(int i = 0 ; i < 120 ; i++)
//      temp += windspdavg[i];
//    temp /= 120.0;
//    windspdmph_avg2m = temp;
//
// Becomes:
//
//    float temp = 0;
//
//    for(int i = 0 ; i < 120 ; i++) {
//      temp += windspdavg[i];
//    }
//
//    temp /= 120.0;
//    windspdmph_avg2m = temp;
//

//------------------ Notes ---------------------------------------------------

//------------------ includes ------------------------------------------------

#include "SparkFun_Photon_Weather_Shield_Library.h"
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "SparkFunPhant.h"

// Particle library support
#include "application.h" 

// Smartpux.com/Openpux IoT cloud support
#include "MenloFramework/MenloSmartpuxPhoton.h"

//------------------ includes ------------------------------------------------

//------------------ global constants ----------------------------------------

//
// The Photon status LED can be remotely controlled
// by Particle cloud commands. It's currrently not hooked up
// to any of the weather shield hardware, but could be used
// by your installation to control a device throught the proper
// MOSFET/Relay driver.
//
const int STATUS_LED = D7; // LED D7

#define ONE_WIRE_BUS D4
#define TEMPERATURE_PRECISION 11

#define SOIL_MOIST A1
#define SOIL_MOIST_POWER D5

int WDIR = A0;
int RAIN = D2;
int WSPEED = D3;

//
// Run I2C Scanner to get address of DS18B20(s)
// (found in the Firmware folder in the Photon Weather Shield Repo)
//
/***********REPLACE THIS ADDRESS WITH YOUR ADDRESS*************/
DeviceAddress inSoilThermometer =
{0x28, 0x6F, 0xD1, 0x5E, 0x06, 0x00, 0x00, 0x76}; //Waterproof temp sensor address
/***********REPLACE THIS ADDRESS WITH YOUR ADDRESS*************/

//------------------ global constants ----------------------------------------

//------------------ global variables ----------------------------------------

//
// Timers_Variables
//
long lastSecond; // The millis counter to see when a second rolls by
byte seconds;    // Seconds counter.
byte minutes;    // Minutes counter.
long hours;      // hours, or uptime counter since last reset/power down

// Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
byte seconds_2m = 0;

// Keeps track of where we are in wind gust/dir over last 10 minutes array of data
byte minutes_10m = 0;

//
// Wind sensors
//

//
// We need to keep track of the following variables:
// Wind speed/dir each update (no storage)
// Wind gust/dir over the day (no storage)
// Wind speed/dir, avg over 2 minutes (store 1 per second)
// Wind gust/dir over last 10 minutes (store 1 per minute)
// Rain over the past hour (store 1 per minute)
// Total rain over date (store one per day)
//

byte windspdavg[120]; // 120 bytes to keep track of 2 minute average
int winddiravg[120]; // 120 ints to keep track of 2 minute average
float windgust_10m[10]; // 10 floats to keep track of 10 minute max
int windgustdirection_10m[10]; // 10 ints to keep track of 10 minute max

// These are all the weather values that wunderground expects:
int winddir = 0; // [0-360 instantaneous wind direction]
float windspeedmph = 0; // [mph instantaneous wind speed]
float windgustmph = 0; // [mph current wind gust, using software specific time period]
int windgustdir = 0; // [0-360 using software specific time period]
float windspdmph_avg2m = 0; // [mph 2 minute average wind speed mph]
int winddir_avg2m = 0; // [0-360 2 minute average wind direction]
float windgustmph_10m = 0; // [mph past 10 minutes wind gust mph ]
int windgustdir_10m = 0; // [0-360 past 10 minutes wind gust direction]
long lastWindCheck = 0;

//
// Rain sensor
//
volatile float rainHour[60]; // 60 floating numbers to keep track of 60 minutes of rain
volatile float dailyrainin = 0; // [rain inches so far today in local time]

// [rain inches over the past hour)] -- the accumulated rainfall in the past 60 min
float rainin = 0;

//
// Humidity
//
float humidity = 0;

//
// Temperature
//
float tempf = 0;
double InTempC = 0;//original temperature in C from DS18B20

//
// Pressure
//
float pascals = 0;
//float altf = 0; //uncomment this if using altitude mode instead
float baroTemp = 0;

//
// Remote Soil sensor
//
int soilMoisture = 0;
float soiltempf = 0;//converted temperature in F from DS18B20

//
// These are updated by remote Particle cloud commands to change
// operating modes.
//
bool g_debug = true;
bool publishToSmartpux = true;
bool publishToParticle = true;
bool publishToPhant = true;
bool publishToPrintInfo = true;

//
// These are cloud scheduler values
//
bool schedulerState = false;
bool scheduleSmartpux = false;
bool scheduleParticle = false;
bool schedulePhant = false;
bool schedulePrintInfo = false;
int cloudTimer = 0;
long cloud_update_rate_in_seconds = 30;

//------------------ global variables ----------------------------------------

//------------------ volatile/interrupt accessed global variables ------------

// volatiles are subject to modification by IRQs
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;
volatile unsigned long raintime, rainlast, raininterval, rain;

//------------------ volatile/interrupt accessed global variables ------------

//------------------ static class declarations -------------------------------

//
// These class declarations allocate global memory and run the
// classes constructor at program initialization before setup()
// is invoked. Keep this in mind.
//
// Note that static allocations are preferred when memory is tight
// such as on Arduino's. This avoids the slight overhead and
// potential fragmention of the small memory heap.
//
// Code Style: Constructors should just place the class into a sane
// state for a later Initialize() function to be invoked when the
// environment is setup, especially when there are dependencies on
// other classes. This is because constructor invoke order for
// global variables is not guaranteed or predictable as a project
// changes over time.
//

//
// This creates instances of HTU21D or SI7021 temp and humidity sensor
// and MPL3115A2 barrometric sensors.
//
Weather sensor;

//
// Remote Dallas temperature probe support
//
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

void update18B20Temp(DeviceAddress deviceAddress, double &tempC);

//
// Cloud services
//
MenloSmartpuxPhoton g_smartpux;

Phant phant(server, publicKey, privateKey);

//------------------ static class declarations -------------------------------

//------------------ forward references --------------------------------------

//
// Application routines
//
void Application_Initialize();
void getWeather();
void printInfo();
float get_wind_speed();
int get_wind_direction();

// Hardware Support
void Hardware_Initialize();
void Hardware_EnableInterrupts();

// Timers
void Timers_Initialize();
void Timers_Start();
void Timers_Poll();

// Scheduler
void hours_Tick();
void minutes_Tick();
void seconds_Tick();
void cloudScheduler();
int setCloudScheduler(long);

// Particle Cloud
void Particle_Initialize();
void Particle_Post();

// Smartpux.com Cloud
int Smartpux_Initialize();
int Smartpux_Post();

// Phant Cloud
int Phant_Initialize();
int Phant_Post();

//------------------ forward references --------------------------------------

//------------------ setup/loop ----------------------------------------------

//
// Setup the infrastructure, hardware, then application.
//
void setup()
{
    Timers_Initialize();

    Hardware_Initialize();

    // Start scheduler after hardware initialize
    Timers_Start();

    Application_Initialize();

    // Turn on interrupts once timers are initialized
    Hardware_EnableInterrupts();

    //
    // Initialize cloud providers
    //
    Particle_Initialize();

    Smartpux_Initialize();

    Phant_Initialize();
}

//
// The main processing loop is a timer scheduler.
//
// This is because you can't use delay() for scheduling since
// any delay greater than 8 seconds will cause a timeout/reset
// on your Photon, or on any Arduino that has enabled the watchdog
// timer. (Recommended for long running projects deployed in the field)
//
void loop()
{
    //
    // All processing is done under timers.
    //
    // This makes it easier to eventually convert the Photon
    // threaded mode.
    //
    Timers_Poll();
}
//------------------ setup/loop ----------------------------------------------

//------------------ Timers --------------------------------------------------
void Timers_Initialize()
{
    seconds = 0;
    minutes = 0;
    hours = 0;
    cloudTimer = 0;

    //
    // This will update the default rate according to
    // the number of configured cloud providers.
    //
    setCloudScheduler(default_cloud_update_rate_in_seconds);
}

void Timers_Start()
{
    // Set the scheduler baseline
    lastSecond = millis();
}

//
// This handles the various interval timers of
// 1 second, one minute, and one hour.
//
// Anything less than a second resolution uses interrupts.
//
// It would be reasonable to add a 100ms timer, but this
// application does not require it.
//
void Timers_Poll()
{
    //
    // The timer loop may get delayed by other processing, but it
    // "catches up" by incrementing the lastSecond count by
    // 1000 ms rather than reading from millis().
    //
    // This will result in a series of back-to-back tick invocations
    // until its caught up. This is by design so as not to lose
    // time.
    //

    //
    // Per second tick routine
    //

    if (!(millis() - lastSecond >= 1000))
    {
        //
        // Code Style: Early out is preferred rather than nested if's.
        // It's more clear what is happening after this point and
        // less state variables to keep track of when examining the code.
        //

        return;
    }

    lastSecond += 1000;

    // Advance to the next second
    seconds++;

    //
    // Process next minute tick
    //
    if (seconds > 59)
    {
        seconds = 0;
        minutes++;

        // Hours roll over
        if(minutes > 59) {
            minutes = 0;
            hours++;
            hours_Tick();
        }

        minutes_Tick();
    }

    //
    // Seconds tick.
    //
    seconds_Tick();
}
//------------------ Timers --------------------------------------------------

//------------------ Timer Handlers ------------------------------------------

//
// This is invoked for every seconds tick
//
void seconds_Tick()
{
    // Take a speed and direction reading every second for 2 minute average
    if(++seconds_2m > 119) seconds_2m = 0;

    // Calc the wind speed and direction every second for 120 second to get 2 minute average
    float currentSpeed = get_wind_speed();
    //float currentSpeed = random(5); //For testing

    int currentDirection = get_wind_direction();
    windspdavg[seconds_2m] = (int)currentSpeed;
    winddiravg[seconds_2m] = currentDirection;

    //if(seconds_2m % 10 == 0) displayArrays(); //For testing

    //Check to see if this is a gust for the minute
    if(currentSpeed > windgust_10m[minutes_10m])
    {
      windgust_10m[minutes_10m] = currentSpeed;
      windgustdirection_10m[minutes_10m] = currentDirection;
    }

    //Check to see if this is a gust for the day
    if(currentSpeed > windgustmph)
    {
      windgustmph = currentSpeed;
      windgustdir = currentDirection;
    }

    //
    // A cloudTimer is used to schedule cloud updates and data print out.
    //
    cloudTimer++;

    if (cloudTimer == cloud_update_rate_in_seconds)
    {
       //
       // Get readings from all sensors. This will update the global
       // weather variables.
       //
       getWeather();

       //
       // NOTE: Doing all cloud operations within one processing
       // loop will result in a timeout/reset of the Photon.
       //
       // So they are staged by the cloud scheduler.
       //
       cloudScheduler();

       // Reset the interval counter
       cloudTimer = 0;
    }
}

//
// This is invoked for every minutes tick
//
void minutes_Tick()
{
    // 10 minute counter used by averages/peak
    if (++minutes_10m > 9) {
        minutes_10m = 0;
    }

    rainHour[minutes] = 0; // Zero out this minute's rainfall amount
    windgust_10m[minutes_10m] = 0; // Zero out this minute's gust
}

//
// This is invoked for every hour tick
//
void hours_Tick()
{
    return;
}

//------------------ Timer Handlers ------------------------------------------

//------------------ Hardware Support ----------------------------------------

//
// Interrupt routines (these are called by the hardware interrupts, not by the main code)
//

//
// Count rain gauge bucket tips as they occur
// Activated by the magnet and reed switch in the rain gauge, attached to input D2
//
void rainIRQ()
{
  raintime = millis(); // grab current time
  raininterval = raintime - rainlast; // calculate interval between this and last event

  if (raininterval > 10) // ignore switch-bounce glitches less than 10mS after initial edge
  {
    dailyrainin += 0.011; // Each dump is 0.011" of water
    rainHour[minutes] += 0.011; // Increase this minute's amount of rain

    rainlast = raintime; // set up for next event
  }
}

//
// Activated by the magnet in the anemometer (2 ticks per rotation),
// attached to input D3
//
void wspeedIRQ()
{
  //
  // Ignore switch-bounce glitches less than 10ms
  // (142MPH max reading) after the reed switch closes
  //
  if (millis() - lastWindIRQ > 10)
  {
    lastWindIRQ = millis(); // Grab the current time
    windClicks++; // There is 1.492MPH for each click per second.
  }
}

void Hardware_Initialize()
{
    // DS18B20 initialization
    sensors.begin();
    sensors.setResolution(inSoilThermometer, TEMPERATURE_PRECISION);

    pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
    pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor

    pinMode(SOIL_MOIST_POWER, OUTPUT); // power control for soil moisture
    digitalWrite(SOIL_MOIST_POWER, LOW); // Leave off by defualt

    // Setup status LED
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    Serial.begin(9600);   // open serial over USB

    // Initialize the I2C sensors and ping them
    sensor.begin();

    //
    // You can only receive acurate barrometric readings or acurate altitiude
    // readings at a given time, not both at the same time. The following two lines
    // tell the sensor what mode to use. You could easily write a function that
    // takes a reading in one made and then switches to the other mode to grab that
    // reading, resulting in data that contains both acurate altitude and barrometric
    // readings. For this example, we will only be using the barometer mode. Be sure
    // to only uncomment one line at a time.
    //

    sensor.setModeBarometer(); // Set to Barometer Mode
    //baro.setModeAltimeter(); // Set to altimeter Mode

    // These are additional MPL3115A2 functions the MUST be called for the sensor to work.

    sensor.setOversampleRate(7); // Set Oversample rate

    //
    // Call with a rate from 0 to 7. See page 33 for table of ratios.
    // Sets the over sample rate. Datasheet calls for 128 but you can set it
    // from 1 to 128 samples. The higher the oversample rate the greater
    // the time between data samples.
    //

    sensor.enableEventFlags(); // Necessary register calls to enble temp, baro ansd alt

    return;
}

void Hardware_EnableInterrupts()
{
    // attach external interrupt pins to IRQ functions
    attachInterrupt(RAIN, rainIRQ, FALLING);
    attachInterrupt(WSPEED, wspeedIRQ, FALLING);

    // turn on interrupts
    interrupts();
}

//------------------ Hardware Support ----------------------------------------

//------------------ Application ---------------------------------------------

void Application_Initialize()
{
    // Place application initialization code here as required.
    return;
}

void printInfo()
{
      //
      // This function prints the weather data out to the default Serial Port
      //
      Serial.print("Wind_Dir:");
      switch (winddir)
      {
        case 0:
          Serial.print("North");
          break;
        case 1:
          Serial.print("NE");
          break;
        case 2:
          Serial.print("East");
          break;
        case 3:
          Serial.print("SE");
          break;
        case 4:
          Serial.print("South");
          break;
        case 5:
          Serial.print("SW");
          break;
        case 6:
          Serial.print("West");
          break;
        case 7:
          Serial.print("NW");
          break;
        default:
          Serial.print("No Wind");
          // if nothing else matches, do the
          // default (which is optional)
      }

      Serial.print(" Wind_Speed:");
      Serial.print(windspeedmph, 1);
      Serial.print("mph, ");

      Serial.print("Rain:");
      Serial.print(rainin, 2);
      Serial.print("in., ");

      Serial.print("Temp:");
      Serial.print(tempf);
      Serial.print("F, ");

      Serial.print("Humidity:");
      Serial.print(humidity);
      Serial.print("%, ");

      Serial.print("Baro_Temp:");
      Serial.print(baroTemp);
      Serial.print("F, ");

      Serial.print("Pressure:");
      Serial.print(pascals/100);
      Serial.print("hPa, ");

      //
      // The MPL3115A2 outputs the pressure in Pascals. However, most weather stations
      // report pressure in hectopascals or millibars. Divide by 100 to get a reading
      // more closely resembling what online weather reports may say in hPa or mb.
      // Another common unit for pressure is Inches of Mercury (in.Hg). To convert
      // from mb to in.Hg, use the following formula. P(inHg) = 0.0295300 * P(mb)
      //
      // More info on conversion can be found here:
      //
      // www.srh.noaa.gov/images/epz/wxcalc/pressureConversion.pdf
      //

      //
      // If in altitude mode, print with these lines
      //
      //Serial.print("Altitude:");
      //Serial.print(altf);
      //Serial.println("ft.");

      Serial.print("Soil_Temp:");
      Serial.print(soiltempf);
      Serial.print("F, ");

      Serial.print("Soil_Mositure:");

      //
      // Moisture Content is expressed as an analog
      // value, which can range from 0 (completely dry) to the value of the
      // materials' porosity at saturation. The sensor tends to max out between
      // 3000 and 3500.
      //
      Serial.println(soilMoisture);
}

//---------------------------------------------------------------

void getSoilTemp()
{
    // get temp from DS18B20
    sensors.requestTemperatures();
    update18B20Temp(inSoilThermometer, InTempC);

    // Every so often there is an error that throws a -127.00, this compensates
    if(InTempC < -100) {
      soiltempf = soiltempf; // push last value so data isn't out of scope
    }
    else {
      soiltempf = (InTempC * 9)/5 + 32; // else grab the newest, good data
    }
}

//---------------------------------------------------------------

void getSoilMositure()
{
    //
    // We found through testing that leaving the soil moisture sensor powered
    // all the time lead to corrosion of the probes. Thus, this port breaks out
    // Digital Pin D5 as the power pin for the sensor, allowing the Photon to
    // power the sensor, take a reading, and then disable power on the sensor,
    // giving the sensor a longer lifespan.
    //
    digitalWrite(SOIL_MOIST_POWER, HIGH);
    delay(200);
    soilMoisture = analogRead(SOIL_MOIST);
    delay(100);
    digitalWrite(SOIL_MOIST_POWER, LOW);
}

//---------------------------------------------------------------

void update18B20Temp(DeviceAddress deviceAddress, double &tempC)
{
  tempC = sensors.getTempC(deviceAddress);
}

//---------------------------------------------------------------

//
// Read the wind direction sensor, return heading in degrees
//
int get_wind_direction()
{
  unsigned int adc;

  adc = analogRead(WDIR); // get the current reading from the sensor

  //
  // The following table is ADC readings for the wind direction sensor output,
  // sorted from low to high.
  //
  // Each threshold is the midpoint between adjacent headings. The output is
  // degrees for that ADC reading.
  //
  // Note that these are not in compass degree order! See Weather Meters
  // datasheet for more information.
  //

  //
  // Wind Vains may vary in the values they return. To get exact wind direction,
  // it is recomended that you AnalogRead the Wind Vain to make sure the values
  // your wind vain output fall within the values listed below.
  //

  if(adc > 2270 && adc < 2290) return (0); // North
  if(adc > 3220 && adc < 3299) return (1); // NE
  if(adc > 3890 && adc < 3999) return (2); // East
  if(adc > 3780 && adc < 3850) return (3); // SE

  if(adc > 3570 && adc < 3650) return (4); // South
  if(adc > 2790 && adc < 2850) return (5); // SW
  if(adc > 1580 && adc < 1610) return (6); // West
  if(adc > 1930 && adc < 1950) return (7); // NW

  return (-1); // error, disconnected?
}

//---------------------------------------------------------------

//
// Returns the instataneous wind speed
//
float get_wind_speed()
{
  float deltaTime = millis() - lastWindCheck; // 750ms

  deltaTime /= 1000.0; // Convert to seconds

  float windSpeed = (float)windClicks / deltaTime; // 3 / 0.750s = 4

  windClicks = 0; // Reset and start watching for new wind
  lastWindCheck = millis();

  windSpeed *= 1.492; // 4 * 1.492 = 5.968MPH

  /* Serial.println();
   Serial.print("Windspeed:");
   Serial.println(windSpeed);*/

  return(windSpeed);
}

//---------------------------------------------------------------

void getWeather()
{
    // Measure Relative Humidity from the HTU21D or Si7021
    humidity = sensor.getRH();

    // Measure Temperature from the HTU21D or Si7021
    tempf = sensor.getTempF();

    //
    // Temperature is measured every time RH is requested.
    // It is faster, therefore, to read it from previous RH
    // measurement with getTemp() instead with readTemp()
    //

    // Measure the Barometer temperature in F from the MPL3115A2
    baroTemp = sensor.readBaroTempF();

    // Measure Pressure from the MPL3115A2
    pascals = sensor.readPressure();

    // If in altitude mode, you can get a reading in feet  with this line:
    //altf = sensor.readAltitudeFt();

    getSoilTemp(); // Read the DS18B20 waterproof temp sensor
    getSoilMositure(); // Read the soil moisture sensor

    // Calc winddir
    winddir = get_wind_direction();

    // Calc windspeed
    windspeedmph = get_wind_speed();

    // Calc windgustmph
    // Calc windgustdir
    // Report the largest windgust today
    windgustmph = 0;
    windgustdir = 0;

    // Calc windspdmph_avg2m
    float temp = 0;

    for(int i = 0 ; i < 120 ; i++) {
      temp += windspdavg[i];
    }

    temp /= 120.0;
    windspdmph_avg2m = temp;

    // Calc winddir_avg2m
    temp = 0; // Can't use winddir_avg2m because it's an int

    for(int i = 0 ; i < 120 ; i++) {
      temp += winddiravg[i];
    }

    temp /= 120;
    winddir_avg2m = temp;

    //
    // Calc windgustmph_10m
    // Calc windgustdir_10m
    // Find the largest windgust in the last 10 minutes
    //
    windgustmph_10m = 0;
    windgustdir_10m = 0;

    // Step through the 10 minutes
    for(int i = 0; i < 10 ; i++)
    {
      if(windgust_10m[i] > windgustmph_10m)
      {
        windgustmph_10m = windgust_10m[i];
        windgustdir_10m = windgustdirection_10m[i];
      }
    }

    //
    // Total rainfall for the day is calculated within the interrupt
    // Calculate amount of rainfall for the last 60 minutes
    //
    rainin = 0;

    for(int i = 0 ; i < 60 ; i++) {
      rainin += rainHour[i];
    }
}

//------------------ Application ---------------------------------------------

//------------------ Phant Cloud ---------------------------------------------

int Phant_Initialize()
{
    phant.setDebug(g_debug);
    return 0;
}

int Phant_Post()
{
    //
    // Add readings
    //

    phant.add("windspeed", windspeedmph);
    phant.add("winddirection", winddir);
    phant.add("barometer", pascals/100);
    phant.add("temperature", tempf);
    phant.add("rainfall", rainin);
    phant.add("humidity", humidity);
    phant.add("moisture", soilMoisture);
    phant.add("soiltemp", soiltempf);

    phant.add("windgust", windgustmph_10m);
    phant.add("windgustdir", windgustdir_10m);
    phant.add("light", 0);

    //phant.add("altf", altf);//add this line if using altitude instead
    //    phant.add("barotemp", baroTemp);
    //    phant.add("humidity", humidity);
    //phant.add("hectopascals", pascals/100);
    //phant.add("rainin", rainin);
    //phant.add("soiltempf", soiltempf);
    //phant.add("soilmoisture", soilMoisture);
    //phant.add("tempf", tempf);
    //phant.add("winddir", winddir);
    //phant.add("windspeedmph", windspeedmph);

    return phant.postToPhant(phant_timeout);
}

//------------------ Phant Cloud ---------------------------------------------

//------------------ Particle Cloud ------------------------------------------

//
// These routines process commands from the Particle cloud.
//

//
// Toggle LED
//
int setLed(String arg)
{
    if (arg == "on") {
        digitalWrite(STATUS_LED, HIGH);
        return 1;
    }
    else if (arg == "off") {
        digitalWrite(STATUS_LED, LOW);
        return 0;
    }
    else {
        // unknown state
        return -1;
    }
}

//
// Set the cloud update rate
//
int setUpdateRate(String arg)
{
    long rate = arg.toInt();
    if (rate == 0) {
        // error
        return -1;
    }

    return setCloudScheduler(rate);
}

//
// This allows operating modes to be configured remotely
// using the particle cloud.
//
int commandReceived(String arg)
{
    //
    // This enables/disables the various cloud services
    // we post to.
    //

    if (arg == "debug_on") {
        g_debug = true;
    }
    else if (arg == "debug_off") {
        g_debug = false;
    }
    else if (arg == "print_info_on") {
        publishToPrintInfo = true;
    }
    else if (arg == "print_info_off") {
        publishToPrintInfo = false;
    }
    else if (arg == "all_on") {
 
        // publish readings to all cloud providers
        publishToSmartpux = true;
        publishToParticle = true;
        publishToPhant = true;
    }
    else if (arg == "all_off") {
        // stop publishing readings to all cloud providers
        publishToSmartpux = false;
        publishToParticle = false;
        publishToPhant = false;
    }
    else if (arg == "smartpux_on") {
        // publish readings to Smartpux
        publishToSmartpux = true;
    }
    else if (arg == "smartpux_off") {
        // stop publishing readings to Smartpux
        publishToSmartpux = false;
    }
    else if (arg == "particle_on") {
        // publish variables to Particle
        publishToParticle = true;
    }
    else if (arg == "particle_off") {
        // stop publishing variables to Particle
        publishToParticle = false;
    }
    else if (arg == "phant_on") {
        // publish readings to data.sparkfun.com (Phant)
        publishToPhant = true;
    }
    else if (arg == "phant_off") {
        // stop publishing readings to data.sparkfun.com (Phant)
        publishToPhant = false;
    }
    else {
        return -1;
    }

    return -1;
}

//
// Receive a Dweet.
//
// A Dweet is a NMEA 0183 formatted command.
//
// Note: Since NMEA 0183 maximum length is 82 characters
// and we are limited to 63 by Particle functions, the
// prefix, checksum, are optional given the particle infrastructure
// has assured complete message delivery.
//
// Example:
//
// $PDWT,SETCONFIG=UPDATERATE:30*00 -> SETCONFIG=UPDATERATE:30
//
//
int dweetReceived(String arg)
{
    // Need to bring in the Menlo Framework
    return -1;
}

//
// This publishes a high wind event.
//
void PublishHighWindEvent()
{
    //
    // Note: TTL is always 60 right now.
    // Event data can be up to 255 bytes.
    //
    bool result = Particle.publish("highwind", "10+ MPH", 60, PUBLIC);
    if (!result) {
        //
        // failed to publish the event. Could be that the cloud
        // is disconnected. May want to add code that attempts
        // to republish an event on an interval until it succeeds
        // so it will not be lost.
        //
    }

    // If you want the event PRIVATE
    //Particle.publish("highwind", "10+ MPH", 60, PRIVATE);
}

void somethingEventHandler(const char *event, const char *data)
{
  Serial.print("event received! name=");
  Serial.print(event);
  Serial.print(", data: ");

  if (data) {
    Serial.println(data);
  }
  else {
    Serial.println("NULL");
  }
}

//
// This publishes essential variables to the Particle cloud
//
// Rather than re-write the code to use double instead
// of float we just convert using shadow varilables here.
//
double d_windspeedmph;
double d_windgustmph;
double d_tempf;
double d_pascals; // barometer
double d_rainin; 
double d_humidity;
double d_hours;

//
// This is called from setup() to prepare for publishing to the particle cloud.
//
void Particle_Initialize()
{
    //
    // Setup control functions
    //
    // Up to 4 functions of up to 12 characters names may be registered.
    //
    // Received argument for the handler function may be up to 63 characters.
    //
    Particle.function("dweet", dweetReceived);
    Particle.function("command", commandReceived);
    Particle.function("setrate", setUpdateRate);
    Particle.function("led", setLed);

    //
    // Setup events
    //
    // Event names may be up to 63 characters.
    //
    // Events may be published up to once per second.
    //
    // Event names may not start with "spark" (case insensitive)
    //
    // Events always live for 60 seconds, regardless of ttl parameter today.
    //
    // Event data may be up to 255 bytes
    //

    //
    // Nothing to setup to post events, Particle.publish() is called when the
    // event occurs. See PublishHighWindEvent().
    //

    //
    // Subscribe to receive events.
    //
    // Up to 4 event subscriptions may be done.
    //
    Particle.subscribe("eventprefix_something", somethingEventHandler);

    // Subscribe from a specific device id
    //Particle.subscribe("motion/front-door", motionHandler, "55ff70064989495339432587");

    //
    // Setup published data variables
    //
    // Note: Up to 10 variables of up 12 character names may be published
    // Types are limited to INT, DOUBLE, STRING
    //
    
    // Convert the values from float to double which Particle requires
    d_windspeedmph = windspeedmph;
    d_windgustmph = windgustmph;
    d_tempf = tempf;
    d_pascals = pascals / 100;
    d_rainin = rainin;
    d_humidity = humidity;
    d_hours = hours;

    // This is the older firmware way
    Particle.variable("windspeed", &d_windspeedmph, DOUBLE);
    Particle.variable("winddir", &winddir, INT);
    Particle.variable("windgust", &d_windgustmph, DOUBLE);
    Particle.variable("windgustdir", &windgustdir, INT);
    Particle.variable("temperature", &d_tempf, DOUBLE);
    Particle.variable("barometer", &d_pascals, DOUBLE);
    Particle.variable("rainfall", &d_rainin, DOUBLE);
    Particle.variable("humidity", &d_humidity, DOUBLE);
    Particle.variable("uptime", &d_hours, DOUBLE);
}

//
// This is invoked from the cloud scheduler publishing time is reached
//
void Particle_Post()
{
    // Update the double shadow variables
    d_windspeedmph = windspeedmph;
    d_windgustmph = windgustmph;
    d_tempf = tempf;
    d_pascals = pascals / 100;
    d_rainin = rainin;
    d_humidity = humidity;
    d_hours = hours;

    if (g_debug) Serial.println("Particle.publish variables");

    // The String(val) constructor converts from native to string format
    Particle.publish("windspeed", String(d_windspeedmph));
    Particle.publish("winddir", String(winddir));
    Particle.publish("windgust", String(d_windgustmph));
    Particle.publish("windgustdir", String(windgustdir));
    Particle.publish("temperature", String(d_tempf));
    Particle.publish("barometer", String(d_pascals));
    Particle.publish("rainfall", String(d_rainin));
    Particle.publish("humidity", String(d_humidity));
    Particle.publish("uptime", String(d_hours));

    if (g_debug) Serial.println("Particle.publish variables done");
}

//------------------ Particle Cloud ------------------------------------------

//------------------ Smartpux Cloud ------------------------------------------

int Smartpux_Initialize()
{
   int retVal;

   retVal = g_smartpux.Initialize(
        smartpux_host,
        smartpux_port,
        smartpux_token,
        smartpux_account,
        smartpux_sensor
        );

    g_smartpux.SetDebug(g_debug);

    return retVal;
}

int Smartpux_Post()
{
    g_smartpux.add("windspeed", windspeedmph);
    g_smartpux.add("winddirection", winddir);
    g_smartpux.add("barometer", pascals/100);
    g_smartpux.add("temperature", tempf);
    g_smartpux.add("rainfall", rainin);
    g_smartpux.add("humidity", humidity);
    g_smartpux.add("moisture", soilMoisture);
    g_smartpux.add("soiltemp", soiltempf);

    g_smartpux.add("windgust", windgustmph_10m);
    g_smartpux.add("windgustdir", windgustdir_10m);
    g_smartpux.add("light", 0);

    g_smartpux.Post(smartpux_timeout);

    return 1;
}

//------------------ Smartpux Cloud ------------------------------------------

//------------------ Cloud Scheduler -----------------------------------------

//
// Return the number of currently enabled clouds.
//
// This updates the schedule to accomodate them, at a staggered interval.
//
long countEnabledClouds() {

    long count = 0;

    if (publishToSmartpux) {
        count++;
    }

    if (publishToParticle) {
        count++;
    }


    if (publishToPhant) {
        count++;
    }

    if (publishToPrintInfo) {
        count++;
    }

    return count;
}

//
// Update the number of seconds for the cloud updates
//
int setCloudScheduler(long seconds)
{
    long newRate;

    if (seconds < 15) {
        return -1;
    }

    newRate = seconds / (countEnabledClouds() + 1);

    cloud_update_rate_in_seconds = newRate;

    // Updated
    return 1;
}

//
// Cloud Scheduler
//
// This gets called every cloud_update_rate_in_seconds interval.
//
// For every interval in which its called, it will update one particular
// cloud service in sequence if its enabled for publishing.
//
// Once all enabled cloud services are published in sequence, the
// scheduler toggle is flipped to re-process the sequence again.
//
// Frequency of cloud updates interaction with cloud_update_rate_in_seconds:
//
// The actual rate that any given cloud provider is updated is the
// interval rate divided by the number of enabled cloud providers + 1.
//
// So if one is enabled on a 30 second interval, it will be updated
// once per 60 seconds.
//
// If two are enabled, they are updated every 90 seconds.
//
// If three, its every 120 seconds.
//
// The cloud_update_rate_in_seconds interval should be adjusted
// based on frequency, and number of cloud providers are being
// updated. This can be done in the routine that receives update
// rate adjustment commands in the Particle function "setrate".
//
void cloudScheduler()
{
    //
    // This scheduler ensures we don't perform back to back transactions
    // to multiple clouds since the Photon can timeout.
    //

    //
    // The toggle indicates which schedule phase we are in. It's used
    // to ensure that each enabled cloud service is scheduled once
    // before starting over.
    //

    //
    // publishTo<cloud> determines if a given cloud service is enabled
    //

    //
    // schedule<cloud> tells the processing loop to perform the transaction
    // with the specific cloud.
    //
    bool toggle = schedulerState;

    if (publishToPrintInfo && (schedulePrintInfo != toggle)) {

        // This prints the information out the serial port monitoring line
        printInfo();
        schedulePrintInfo = toggle;
        return;
    }

    if (publishToSmartpux && (scheduleSmartpux != toggle)) {

        if (g_debug) Serial.println("Smartpux_Post: ");

        Smartpux_Post();

        if (g_debug) Serial.println("Smartpux_Post: returned");

        scheduleSmartpux = toggle;
        return;
    }

    if (publishToParticle && (scheduleParticle != toggle)) {

       if (g_debug) Serial.println("Particle_Post:");

       Particle_Post();

       if (g_debug) Serial.println("Particle_Post: returned");

        scheduleParticle = toggle;
        return;
    }

    if (publishToPhant && (schedulePhant != toggle)) {

       if (g_debug) Serial.println("Phant_Post:");

       Phant_Post();

       if (g_debug) Serial.println("Phant_Post: returned");

        schedulePhant = toggle;
        return;
    }

    //
    // If we got here we have scheduled all enabled cloud services
    // at the current scheduler state, so flip it.
    //
    // Note: this is where the +1 comes from in the cloud scheduling
    // calculation. We need one extra pass through the scheduler
    // after all services have been run for the current toggle
    // state in order to flip its state.
    //
    // If none are enabled this will just toggle every scheduler
    // time, but no cloud publishes will occur.
    //
    schedulerState = !schedulerState;
}

//------------------ Cloud Scheduler -----------------------------------------
