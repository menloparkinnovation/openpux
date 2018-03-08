
Top level reference of particle projects.
07/12/2017

electron_BoatMonitor:
  - Most active project as of 07/12/2017
  - Boat monitor based on electron.
  - Most complete project.
  - Fills out configuration, debug, tracing.
  - Currently does not use MenloFramework.
    - Work out how to add it in for Dweet, logging, etc.

MenloParticleWeather:
  - Current photon (WiFi) based firmware for WeatherStation2.
  - Currently deployed project as of 07/12/2017
  - Uses MenloFramework.

electron_MenloParticleWeather:
  - builds for electron target, no WiFi support.
  - Not tested yet. Intended for cell based remote weatherstation/buoy.
  - Uses MenloFramework.

electron_RemoteMonitor:
  - Basic remote monitoring application.
  - "one page" app model with #include
  - No MenloFramework
  - "hack-a-thon" starter.

MenloFramework
  - Particle specific MenloFramework objects/classes

archived_projects:

Don't bother with these. Archived in case some code goes missing.

electron_beam:
  - example app from particle for testing electron communication.
  - Uses single page model.
  - No MenloFramework
  - Replaced by electron_RemoteMonitor above.

Details:

---

MenloParticleWeather:
  - Current photon (WiFi) based firmware for WeatherStation2.

---

electron_MenloParticleWeather:
  - builds for electron target, no WiFi support.
  - Not tested yet. Relies 100% on Particle cloud.

---
electron_BoatMonitor:

Electron based boat monitor application.

Particle Electron based so as not to have to rely on
WiFi.

Intended to be a complete consumer deployed project with
associated hardware.

Supports a Nordic nrf24L01+ radio to network with local battery
operated sensors for engine room, bilge pump, etc.

A future Photon version is planned, and the designed hardware
supports Photon in the socket with connections to the sensors.

---
electron_RemoteMonitor is the reference for remote cellular based
IoT application.

It's structured to be a quick + easy "one page app" that handles
the basics of remote monitoring using a cellular based IoT
device with backup battery.

This example does the following:

 - Power on configuration read from EEPROM
 - Cloud updates to both in memory, and EEPROM configuration
 - Notification of USB power loss/gain
 - Notification of low battery
 - Status query for battery voltage, charging status,
   cellular signal strength, etc.
 - Support for signaling led to flash for configuration, status, etc.
---

--------------------------------------------

---

electron_blink: This was the starting point for electron_RemoteMonitor
and is considered frozen.

*WeatherStation*
These are Photon WiFi based weatherstations for SparkFun WeatherShield.

It has been setup to build for Electron, but has not yet integrated
the remote battery management, etc. of the Electron.

--------------------------------------------



-----
readme.txt

02/23/2016

This directory builds in place from the particle build environment
in $HOME/particle/firmware

see make_menlo.sh

------------------------------------------------------------------------------
// Current Projects:

cd MenloParticleWeather

$HOME/Dropbox/code/macgitrepos/Smartpux/SparkCore/particle/MenloParticleWeather

------------------------------------------------------------------------------
------------------------------------------------------------------------------
------------------------------------------------------------------------------

Sources for local particle/sparkcore builds.

Applications go into
 particle/firmware/user/applications/*

build from:
 particle/firmware/main

make PLATFORM=photon APP=sparkfunweathershieldmenlo

// show build product
ls -l particle/firmware/build/target/user-part/platform-6-m/sparkfunweathershieldmenlo.bin

// Upload through the cloud
particle flash WeatherStation2 ../build/target/user-part/platform-6-m/sparkfunweathershieldmenlo.bin

Summary:

MenloFramework - Photon specific MenloFramework files.

  MenloSmartpuxPhoton - Stand-alone Photon client to Smartpux.com.

                        Does not require the full MenloFramework, scheduler, Dweet, etc.

                        Can be dropped into any Photon project.
-------------


Photon Projects
01/01/2016

General Notes:

The particle developent environment I am using (local build) does not
appear to take *.ino files, but ignores them. It generates firmware
that "hangs" the core, but "safe mode" reset works to allow re-loading
from the cloud.

Fixes made are to rename *.ino files to *.cpp, place things in the
proper directories according to the project version, declare forward
declarations, and use the required version of the Particle publish
API's.

sparkfunweathershieldmenlo

  - *** WORK IN PROGRESS ***

  - "All Up" Menlo Park Innovation LLC version.

  - Supports the following:

    - Communicates with Smartpux/Openpux protocol with both local
      versions and www.smartpux.com

       - Openpux is a Node.js based IoT web site ideal for hosting
         on RaspberryPi's.

    - Leverages the Particle cloud for station management, monitoring,
      settings, firmware updates, etc.

    - Continues to support SparkFun Phant by the following:

      - Communicates with SparkFun Phant at data.sparkfun.com

      - Communicates with Openpux server using Phant protocol and
        Phant add-in module for openpux.
        - ??? did I write this yet 01/24/2016 ???

  - Uses my phant test account.

  - Changes marked by "Menlo Additions" and new files.

sparkfunweathershieldphant

  - Communicates with data.sparkfun.com (Phant) but with Menlo
    customized values, windgust, etc.

  - Uses my phant test account.

  - Changes marked by "Menlo Additions"

sparkfunweathershieldcloud

  - Basic test for Particle cloud connections.

  - Publishes limited values to Particle Cloud.

  - No wind, rain, etc.

  - Pretty much the stock example from SparkFun except
    for fixes that had to be done based on Particle dev environment
    versions.

  - Changes marked by "Menlo Additions"

sparkfunweathershield

  - Basic weathershield without wind, streams data to serial only.

  - Pretty much the stock example from SparkFun except
    for fixes that had to be done based on Particle dev environment
    versions.

  - Changes marked by "Menlo Additions"

------------------------------------------------------------------------------
04/20/2017 projects log

// archived 04/20/2017, replaced by MenloParticleWeather
01/24/2016
cd sparkfunweathershieldmenlo

$HOME/Dropbox/code/macgitrepos/Smartpux/SparkCore/particle/sparkfunweathershieldmenlo
  - Main project for WhidbeyWeather2
  - Integrates openpux cloud, particle cloud, and phant.

// Stock example from SparkFun with fixes
// Status: Demo, idled.
// 04/20/2017 moved to archived_projects
$HOME/Dropbox/code/macgitrepos/Smartpux/SparkCore/particle/sparkfunweathershield

  - Basic weathershield without wind, streams data to serial only.

  - Pretty much the stock example from SparkFun except
    for fixes that had to be done based on Particle dev environment
    versions.

  - Changes marked by "Menlo Additions"

// Fixes to stock example and particle cloud additions
// Status: Demo, idled.
// 04/20/2017 moved to archived_projects
$HOME/Dropbox/code/macgitrepos/Smartpux/SparkCore/particle/sparkfunweathershieldcloud

  - Basic test for Particle cloud connections.

  - Publishes limited values to Particle Cloud.

  - No wind, rain, etc.

  - Pretty much the stock example from SparkFun except
    for fixes that had to be done based on Particle dev environment
    versions.

  - Changes marked by "Menlo Additions"

// Updated example that integrates the phant example into a working project
// Status: Demo, idled.
// 04/20/2017 moved to archived_projects
$HOME/Dropbox/code/macgitrepos/Smartpux/SparkCore/particle/sparkfunweathershieldphant

  - Communicates with data.sparkfun.com (Phant) but with Menlo
    customized values, windgust, etc.

  - Uses my phant test account.

  - Changes marked by "Menlo Additions"

------------------------------------------------------------------------------
------------------------------------------------------------------------------
------------------------------------------------------------------------------

