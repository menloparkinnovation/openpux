
Testing mainly consists of using the Arduino IDE to build the various projects
looking for code breaks.

The paths are specific to an Arduino_1.6.8 installation on a Mac installed in
/Applications/Arduino_1_6_8.app

Other platforms may be supported by the test scripts in the future.

Running:

From the openpux/Arduino root directory, run:

test/mac/macbuild_uno.sh
test/mac/macbuild_mega.sh
test/mac/macbuild_due.sh

Note: It uses $PWD for current path to the project files.
