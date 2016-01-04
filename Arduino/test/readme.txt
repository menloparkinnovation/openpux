
Testing mainly consists of using the Arduino IDE to build the various projects
ooking for code breaks.

The paths are specific to an Arduino_1.6.4 installation on a Mac installed in
/Applications/Arduino_1.6.4.app

Other platforms may be supported by the test scripts in the future.

Note: For building the Mega and Due versions it appears the last settings
in the IDE must be set to that board target, and the IDE exited in order
to save the default environment settings. Otherwise weird build errors
will occur due to a mixing of the definitions and library paths.

As of 12/30/2015 this is still being researched in how to force the IDE
to change preferences, or create separate preferences files.

https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

Running:

 Load the Arduino IDE, select the target board, then exit it.

From the openpux/Arduino root directory, run:

test/mac/macbuild_uno.cmd
test/mac/macbuild_mega.cmd
test/mac/macbuild_due.cmd

Note: It uses $PWD for current path to the project files.

