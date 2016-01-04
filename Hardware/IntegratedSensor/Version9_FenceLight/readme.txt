
09/04/2015

 - Need to add a resistor divider network to allow battery/solar monitoring
   of higher voltage sources (12V).

 - This would allow the sensor to monitor solar battery charging systems
   that operate at 12V. They may, or may not supply the sensor (through
   any proper 12v -> 3.3V power conversion as required).

 - Running the sensor at 5V allows use of off the shelf auto lighter USB
   phone chargers as power supplies. Some low cost ones on Amazon are
   pretty efficient for $15. Cheaper ones can be purchased, but are not
   as efficient.

05/25/2015

 - Updated version 9

 - TMP36 instead of DS18B20. 
   - On analog input A4.

 - Verified reset circuit is in place.

 - Did not update for watch crystal as SparkFun watch schematic shows it without
   capacitors.

   - Can work in existing resonator holes if do not need capacitors.

     - Need to experiment before re-laying out board for bypass capacitors.

04/06/2015

Current version is Version8.

Not known if a set of boards were produced  at this version level.

Version 7 boards are in hand with correct hole spacing.

07/19/2014

Created Version 8.

Renamed diode from 1N4001 to 1N5817 20v 1A Schottky diode

06/30/2014

Version 7.

Updated from Version 6 to fix hole spacing.

06/15/2014

FenceLight holes and format/layout

Sent to oshpark.com 06/15/2014 4:00PM

MenloSensor with inline DIP AtMega 328p

This is really an experimentors board for hand soldering
through hole.

Provides the full gardenlight/plant monitor scenario, but
parts may be optionally populated, and the unused I/O's
and resistor slots used to configure other sensors.

