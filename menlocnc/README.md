
MenloCNC High Performance FGPA based Machine Controller
=======================================================

Copyright (C) 2018 Menlo Park Innovation LLC

   menloparkinnovation.com
   menloparkinnovation@gmail.com

> Last update 06/11/2018

# Single system CNC machine tool controller

  > Runs real time at high end machine tool rates.

  > 12.5 Mhz tested for simple step/dir on 4 axis with
    20 ns clock synchronization between each axis.

  > Outperforms all software based solutions.

  > No PC, laptop, RaspberyPi, or Arduino required.

  > Linux and the real time engine run on a single chip

  > Runs ARM Linux

  > Supports desktop display, keyboard, mouse, jogging controls

  > Supports ethernet and WiFi with USB WiFi.

# Based on Terasic DE10-Standard

  > Altera Cyclone V FPGA with (2) 925 Mhz ARM SoC cores.

  > Also support for Terasic DE10-Nano ($130) with Arduino headers.

# System Architecture  

# FPGA based 4 axis signal timing generator in Verilog

  > Wide word microcode

  > Updatable

  > Additional axis can be added to limit of FPGA pins/logic elments.

# Nios II embedded real time processor

  > Option for more complex procesing.

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

# Arduino UNO/Arduino MEGA Compatible breakout board

  > Developed in free version of Eagle CAD software

  > Boards produced by oshpark.com for $39 for 3.

  > Can be used for just about any Arduino shield for motor
    controller breakouts.

  > Includes input pins for limit switches, etc.

# Support for Standard Motor controllers

  > GRBL Shield for Arduino UNO

  > Mendel Shield for Arduino MEGA

# Expandable project

  > This is a baseline, its intended to add new features

  > New microinstructions

  > New control types such as encoder feedback

  > Optional controls for spindles, coolant, etc.

  > Multi-axis signal generation

# Example Design/Template for a Hardware/Software Co-Design

  > Serves as a template for building a real time application using modern FPGA's with SoC's

  > Clearly separates software, middle, and hardware concerns

  > The right tool is applied at the proper point in the problem domain

# Notes

  > This is a work in progress.

  > Prototype is running and generating 12.5 Mhz signals

  > Additional files, modifications uploaded as development progresses

  > Electrical specifications undergoing testing and validation

# Languages

  > Verilog, System Verilog for RTL on the FPGA

  > C for core utilities that need to run embedded, on a soft-processor, etc.

  > C++ where needed to interface with existing solutions

# Environments

  > Altera Quartus II for Cyclone V SoC FPGA running on Windows 10

  > Ubuntu 16.04 LTS Linux running natively or on Windows 10 LinuxSS

