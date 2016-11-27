
Deployment for C.H.I.P. Computer.

11/27/2016

Note: As of 11/27/2016 CHIP deploys using $HOME/openpux/openpux.

This CHIP computer is an ArmV7 architecture running a Debian Linux.

It uses the R8 AllWinner chipset and is pretty much code compatible with
the RaspberryPi2 as a 32 bit Arm7, at least for user mode executables.

The board has 512MB of RAM, 4GB of flash, WiFi, Bluetooth, and a 1Ghz
single core processor.

As its running Linux with reasonable resources, NodeJS is used as its
main embedded target, and its fully capable of running Openpux with
not porting or translation.

At $9 retail pricing, the board is a more compelling platform for
actual deployments than a RaspberryPi2.

Provisioning a CHIP Computer:

Plug into USB port and start screen (or other terminal emulator)

     screen /dev/tty.usbmodem1423 115200

     Username: chip
     Password: chip

Provision WiFi

     nmcli device wifi list

     nmcli device wifi connect 'networkname' password 'xxxx' ifname wlan0

     nmcli device status

     // Google's DNS server to test
     ping 8.8.8.8

Install basic software:

     apt-get update
     apt-get -y install emacs
     apt-get -y install nodejs
     apt-get -y install npm
     apt-get -y install git

Install Openpux:

     cd $HOME

     git clone https://github.com/menloparkinnovation/openpux.git

     cd openpux/openpux
     npm install

     // On Debian Linux's node is "nodejs"
     ln -s /usr/bin/nodejs /user/bin/node

     npm start

     // get local ipaddr
     ifconfig

     use chrome to navigate to http://<ipaddr>:8080

     // When done
     sudo shutdown now

     // Needed when screen aborts (at least on Mac)
     stty sane

TODO: Install your specific credentials for Openpux services.
TODO: Install rc.d script(s) to automatically start openpux at boot.

      deploy/chip/openpux/setup.txt
