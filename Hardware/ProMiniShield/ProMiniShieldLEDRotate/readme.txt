
D9 - RED LED
D5 - GREEN LED
D6 - BLUE LED

This is a shield for ArduinoProMini and SparkFun ProMicro.

This one rotates the nRF24L01 to give a better overall formfactor.

05/24/2015

It connects the Nordic nRF24L01+ radio (8 pin module).

It has a 3 color LED connected at:

See eagle.txt, KEYWORDS: ProMiniShieldDesign

DIO5_GREEN => PD5(T1)   90 ohm  == Arduino D5 (PWM)
DIO6_BLUE  => xxx(xxx ) 90 ohm  == Arduino D6 (PWM)
DIO9_RED   => PB1(OC1A) 152 ohm == Arduino D9 (PWM)


It also has a push button for RESET to GND.

It also has edge connectors for unconnected signals.


