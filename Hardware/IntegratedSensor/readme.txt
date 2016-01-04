
------------
TODO:

04/05/2015

Use lower cost TMP36 sensor. Also save code space on
not having to load the 1 wire library for the DS18B20.

Add reset capacitor/resistor. This is to make it easy for Arduino
IDE usage and not required the Olimex or other ICSP programmer.

Add support for low power watch crystal. Hopefully this is
that same sockets as the ceramic resonator and the different
is in the fuses.

------------
Sent to oshpark.com 06/30/2014 ???

Version 7 has been updated to correct the hole spacing.

------------
Update 07/19/2014

Hole spacing is wrong!

Took some board space off the end, did not re-align components
so now the gap between the edge of the board and the large
retention holes is to short so the board wobbles in place.

Version 7 above created to adjust this dimension.

---

Sent to oshpark.com 06/16/2014 7:45AM

Version 6 is custom designed to fit within the Harbor Freight Garden Light case.

1.950" wide by 1.70 long

PCB standoff holes are:
  1.80" on center
  1/8" diameter
  0.1" from side edge when mounted in case
  0.3" from top edge when mounted in case

PCB tabs are:
  0.170" diameter -> used 0.1875" since it can be larger

  Center of hole is 0.275" from PCB edge
    1.590 from edge to edge
    1.500" center to center

------------
Version5 sent to oshpark 06/15/2014 4:00PM
------------

This is the Menlo Integrated Sensor.

It's mainly to get cost down by using a 28 pin dip AtMega328p
in place of the Arduino Pro Mini.

Still through hole technology for experimenters and prototyping.

SMD devices are through rider boards using the Sparkfun 3.3v NCP1402
based boost converter and the Chinese made Nordic nRF24L01+ radio
modules.

John Richardson

01/22/2013
