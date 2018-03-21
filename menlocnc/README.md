
MenloCNC High Performance FGPA based Machine Controller
=======================================================

Copyright (C) 2018 Menlo Park Innovation LLC

   menloparkinnovation.com
   menloparkinnovation@gmail.com

> Last update 03/20/2018

# Based on Tera-asic NE10-Standard

  > Altera Cyclone V FPGA with (2) 925 Mhz ARM SoC cores.

# System Architecture  

# FPGA based 4 axis signal timing generater in Verilog

  > Wide word microcode

  > Updatable

# Nios II embedded real time processor

  > FPGA based soft processor.

  > No OS, dedicated real time.

  > No interrupts, ARM TrustZone, Power management sleep states, etc.

  > Manages the signal generator in real-time.

#  ARM32 based Linux for compiling G code to microcode plan

  > Accepts G code or microcode for the timing engine through
    USB, ethernet, USB flash key, etc.

  > Can run desktop Linux with VGA monitor, keyboard, mouse.

# Can be used as a timing generator/G-code engine

  > Use existing machine control software with G-code

  > Compile microcode from existing software with add-in driver

# Can be used standalone as a complete control system

  > Run stand alone LinuxCNC with keyboard, monitor, mouse

  > supports jog controls, joysticks, etc.

  > Can support a web interface, VNC, etc.
