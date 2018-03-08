
05/09/2017

MenloRadioDweetSensor:

Provides battery powered mostly sleeping remote sensor using nRF24L01.

Targeted for AtMega328 sized microcontrollers.

Allows application specific 32 byte radio binary packets.

Breaks out hardware support.

Supports configuration, power on settings, etc. through Dweet.

Ties into MenloFramework in the most generic way possible.

  - Tries to keep all work in the current directory.
  - But leverage libraries.

Dweet Commands Support:

  - Light Level
  - SENSORS command to return a string of readings.

  - Default MenloFramework Dweet support with power on config,
    serial number, etc.

  - Default Dweet radio settings.

  - Default Dweet power/sleep settings.

  - ?? Dweet gateway support between radio and USB.

