
For information on installing libraries, see: http://arduino.cc/en/Guide/Libraries

Nordic nRF24L01 radio support:

MenloNRF24L01 - non-open source MenloFramework private implementation
                of nRF24L01+. 

              - incomplete.

----
Used by:

Essentially used by everyone as of 11/27/2015

MenloGateway
LightHouse
FencePostLight

These two work together:

// open source driver
Mirf - original older driver. Need to move away from it.

// MenloRadio implementation wrapper
OS_nRF24L01 - MenloRadio implementation of Mirf version of the driver.

----

ForkNRF24 - downloaded source code to the newer RF24 driver.
             Must update to 11/27/2015 downloaded driver.

OS_RF24 - MenloRadio implementation of SparkCore version of RF24 from 07/03/2015
        - Need to update to 11/27/2015 version.

OS_ForkRF24 - Newer driver from 10/07/2015
            - Look into using this one for 11/27/2015?

----
