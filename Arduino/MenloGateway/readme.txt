
05/30/2016



04/09/2016

Space with Arduino 1.6.8 is 24,362 bytes program space, 988 bytes ram
  Max is 30,720 bytes program space, 2048 bytes ram for Arduino Pro Mini 8Mhz 3.3V

09/03/2015

MenloGateway - Operates as a gateway for Menlo Framework sensors.

Uses the Nordic nRF24L01+ radio and Dweet protocol.

Install dweet with:

npm install nodejs/dweet -g

sudo npm install nodejs/dweet -g

lighthouseapp.js - customization for Dweet processor for handling a
                   weatherstation/lighthouse combination.

                   See projects Arduino/LightHouse, Arduino/WeatherStation.

Build by opening Arduino 1.6.4 and selecting board and building.

 Mini-Gateway - Arduino Pro Mini 3.3V/8MHz

  -  Uses 24,540 bytes program, 958 bytes RAM.

  - cd scripts

  -  scripts/promini_configuregateway.sh
      - program configuration 

  -  scripts/promini_startradiogatway.sh
      - run gateway

 Gateway - Arduino Uno

  -  Uses 24,552 bytes program, 958 bytes RAM.
  
  - cd scripts

  -  scripts/configuregateway.sh

  -  scripts/startradiogatway.sh

 Gateway - Arduino Due

  - Testing in progress.

  - cd scripts

  -  scripts/due_configuregateway.sh??? missing ???

  -  scripts/due_startradiogatway.sh

  -  scripts/debug_due_startradiogatway.sh

 Teensy31/ARM Gateway

  - Work in progress. Must validate nRF24L01+ on Teensy31

  - cd scripts

  -  scripts/teensy31_configuregateway.sh

  -  scripts/teensy31_startradiogatway.sh

-----------

// First install serialport for dwwet to use in its local project
cd nodejs/dweet

npm install serialport

// Now install dweet globally to make it available system wide
npm install nodejs/dweet -g

sudo npm install nodejs/dweet -g

-----------
