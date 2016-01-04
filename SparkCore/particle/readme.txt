

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

