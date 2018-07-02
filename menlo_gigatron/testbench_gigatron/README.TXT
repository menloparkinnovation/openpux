
06/29/2018

Test bench for gigatron components.

---

//
// The MIT License (MIT)
// Copyright (c) 2018 Menlo Park Innovation LLC
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//

Generic Test bench for System Verilog components.

Due to the complexity of the included SoC components and Avalon
fabric ModelSim appears to break down on simulation errors deep
inside Altera IP modules. In these complex final designs can
take many minutes or hours to compile.

The solution is to keep a simple test bench project in which components
to undergo test are copied over for bench top simulation as unit tests.

This is helpful for engineering changes and improvements to the
sub-modules based on their external functional contracts and
including into the larger project.

To add a new module to the test bench:

1) Create the module saving it as an .sv for System Verilog file.

2) Using the project navigator, seleect Files view, and right click
  "Files" and select add file/module and select your new module and
   close this window.

3) Double click this module and it will open in the design area as tab.

4) While in the project view tab for your file, go to the top level
   menu under Processing, and select "Analyze Current File". This
   will validate your Verilog code at a very basic syntax level.

   Fix the code until this is clean, as you can not generate a symbol
   file unless it passes this step.

5) Go to the top level menu "File", and select Create/Update, and
   then select Generate Symbol file for current file.

   This will generate a symbol file to add it to the schematic view.

6) You need to tell the compilation system about your file so that you
   can run the test bench in ModelSim, the built in circuit simulator
   for Quartus II.

   To avoid messing around with TCL based project settings, do the following:

   Go to the top level schematic work sheet, find an open area or expand its
   surface to create one and right click the open area. Then select
   Insert Symbol, navigation into your project, and if the symbol file
   generation worked in step #5 you should see your module. Select it
   and drop it onto the schematic.

7) Select Processing, Start Compilation. This will validate that
   RTL code can be generated from your System Verilog. You will be
   surprised at what passed the syntax check you did in step #4 and
   what happens when it goes to actually generate a representation
   of the circuit.

   Look at the warnings for your file, though note that there will be
   many for other built in IP logic files. This is important since
   what would be an error in a normal programming language is a warning
   here. Examples of "warnings" are miss named signals on input to
   a logic function/module which is treated as a "don't care", and
   don't get wired up. To make matters worse, it will perform logic
   reduction and eliminate any logic dependent on the mis-named signal,
   resulting in sometimes either a NULL, or very bizare circuit and
   not what you intended.

   Also note that you can go direct to ModelSim after edits to your
   module once you have made it known to the system. But its still
   a good idea to validate that valid RTL can be generated from
   your System Verilog between attempting simulation. ModelSim can
   get very confused and give misleading results if the circuit
   can not represent a real world implemnentation.

   Worse, in many cases ModelSim will simulate a circuit from the
   System Verilog and appear to work correctly, but the circuit
   will either not compile or work for real on the FPGA.

8) Now define a test bench.

   Select the top level menu Assignments, Settings, and select
   Simulation. This is the ModelSim setup. See the Altera documentation
   for setting up ModelSim in general if it did not happen as part
   of your installation.

   Down near the bottom is Test Benches, click that, and click
   Add Test Bench. Fill in the name of your System Verilog file,
   and the name of your test bench model such as tb_xxx.

   Click add, then OK or Apply and exit back to Simulation menu.

   Now in the drop down select your new test bench.

9) To run ModelSim select Tools, Run Simulation Tool, RTL Simulation
   to run the test bench. See the Altera documentation for ModelSim, etc.

Errors:

Lots can go wrong, and it will. It can be very confusing. Some tips:

Module Not found in ModelSim:

  This means it can't find a reference to a module in your project.
  There is the concept of a "top level entity" which starts the root
  of the compilation. The compiler is efficient and will only compile
  modules seen or reachable from this top level module/entity.

  To get your modules seen by the compiler one of the following:

  - Make your module the top level entity.

   - Though this can break your existing top level project, so see what
     its currently set at and record that so you can return.

   - Drag and drop an instance of your module on the schematic view, and
     ensure the schematic view is the default top level entity.

   - Define a top level master System Verilog module for your project
     and reference instances of your module from it.

   - There are other ways under the covers using the TCL syntax to
     edit the files that tell ModelSim what modules to look for, but
     be wary that these can be overwritten when Quartus II re-generates
     them from the various designer views it supports.

Things don't work:

No short cuts! The tool chain is no where near current state of the
art in programming error detection that you may be used to in the
software world. When in doubt, do the following and pay attention
to all warnings that apply to your module:

1) Processing, Analyze Current File

2) File, Add/Update File, Generate Symbols for Current File

3) Processing, Start Compilation

4) Run ModelSim on a test bench

If you pay attention to the warnings (in many cases they are circuit errors)
you should at least have correct execution in ModelSim once you have
debugged your design.

Then its on to SignalTap...




