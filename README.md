
Openpux Internet Of Things (IOT) Framework
==========================================

Copyright (C) 2014,2015 Menlo Park Innovation LLC

   menloparkinnovation.com
   menloparkinnovation@gmail.com

> Last update 12/30/2015

# Openpux Sub Projects

## openpux:

  Node.js based IoT server with in memory or Amazon SimpleDB data stores.

  > Future versions will support MongoDB for local persistent storage through
  > the add-in data storage providers interface.

## nodejs/dweet:

  Node.js utility providing both a command line embedded device configuration
          utility and IoT gateway on RaspberryPi, Mac, PC, or Linux.

## Arduino/Libraries:

  MenloFramework, a framework approach to creating embedded firmware for Arduino
  supporting AtMega328, AVR/Mega, and ARM32/SAM/Due. This has been validated
  against Arduino IDE 1.6.4 and test/regression scripts are in tests/mac.

  > The framework architecture also supports ports to RFDuino (ARM/BLE),
  > SparkCore, (Photon), and variants such as Galileo/Edison (x86/Linux).

  > Tested versions of these ports will be in future updates as re-validation
  > against the latest releases from these providers is in progress.

## Arduino/<project_name>:

  IoT projects using the MenloFramework and communicating with the openpux
  IoT server.

## Hardware:

  MenloPark Innovation LLC hardware projects for IoT scenarios.

  All projects use the Eagle design software, and generally fit within
  the free versions limits.

  Designs have been tested by generating boards from OshPark.
