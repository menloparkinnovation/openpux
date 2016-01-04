
MenloRadioDweetBlink re-uses the DweetBlink application class from
MenloDweetBlink but adds support for DweetRadioSerialApp.

It demonstrates a simple remote control device/application.

The remotely controlled RadioBlink target requires a gateway
configured. The separate Dweet setup command script allows the gateway
to be setup.

This requires an Arduino Uno with a Nordic nRF2401+ radio connected.

The gateway is plugged into a computer running Node.js and a USB
serial driver.

To setup the target:

Determine serial port name (use the Arduino IDE)

dweet -script=ConfigRadioBlinkTarget.dweet portname

To setup the gateway:

Determine serial port name (use the Arduino IDE)

run:

dweet -script=ConfigRadioBlinkGateway.dweet portname

