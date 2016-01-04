
11/29/2015 - last update

------
Smartpux/openpux/deploy/raspberrypi - work in progress
Smartpux/openpux/deploy/ec2 - work in progress
------

Setup information and scripts for RaspberryPi.

This configures the RaspberryPi Raspian distribution as a "headless"
IoT server node hosting openpux and any sensor gateways you enable.

It leaves the ability for you to plug into an HDMI monitor, keyboard,
and mouse and access the gui by manually running "startx" after logon.

This way the memory overhead, power, CPU and GPU core heating of the GUI
environment is not present when operating in IoT gateway/application/server
mode.

You can access and manage the openpux IoT gateway through either
the openpux management interfaces from a web browser or node.js
utilities, ssh, or the RaspberryPi's serial port connection with
the proper TTL serial to USB adapter available from AdaFruit, SparkFun,
etc. You can also configured advanced options such as VNC for remote
GUI access, but those instructions are not here but available on the
web if you search for RaspberryPi VNC.

// RaspberryPi setup:

 - setup RaspberryPi by booting with a NOOB SD card
   - "New Out Of Box" setup SD card allows you to select OS's. Choose Raspian.
   - requires initial DHCP based ethernet connection, HDMI monitor, USB keyboard, USB mouse

 - run raspi-config and set the following:

   - enable serial console shell/login

     // from remote computer:
     screen /dev/ttyUSB0 115200 // Linux
     screen /dev/tty.usbserial 115200 // Mac
     Control-A k // exit screen

   - enable ssh
   - don't boot into X or Scratch, stay in text mode
     - Can always get to GUI with "startx" from command line when on HDMI monitor.
   - setup host name, user password
   - export host name

 - install and configure WiFi if that is what you will be using

 - do the standard update/upgrade dance to get the latest OS, drivers, and libraries:
   sudo apt-get update
   sudo apt-get upgrade
   reboot

 - optional: sudo apt-get install emacs
   - If you prefer the emacs editor over vi, nano

 - install node.js

 - install forever (optional)
   npm install forever -g

 - install git:
   apt-get install git-core

 - enlist in git:

   mkdir Smartpux
   cd Smartpux
   git init
   git remote add Smartpux https://github.com/jrichardson1234/Smartpux
   git pull Smartpux master

 - cd Smartpux/openpux

 For local memory based setup

 - bin/memorystore_runit.sh

 For AWS setup

 - Install aws libraries

    - npm install aws-sdk

    - setup your amazon credentials in $HOME/.aws

    - bin/provision.sh

    - bin/awssimpledb_runit.sh

AWS with forever:

  - Setup AWS as above

  - sudo npm install forever -g

    - bin/awssimpledb_forever_run.sh

-----

Dweet Gateway Support

Dweet is in Smartpux/nodejs/dweet

Dweet provides a gateway service for low power IoT sensors over small packet
based radios. The target is for battery powered devices without the power,
memory, or other attributes to support WiFi and TCP/HTTP connectivity.

The current gateway operates using a USB connected small radio "USB Stick"
based on the Nordic nRF24L01+, AtMega328, and FTDI USB serial interface.

  - This gateway is intended to be accessible to hobbyists who use the
    Arduino IDE for the firmware and off the shelf components and simple
    open source hardware based on Eagle Schematics and Boards.

    Other implementations are available as well.

Installing Dweet:

- Install node serialport
  - npm install serialport -g

- Then install Dweet itself to be available as a nodejs implemented utility
  - cd Smartpux/nodejs
  - sudo npm install nodejs/dweet -g

-----
