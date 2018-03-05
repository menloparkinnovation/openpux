
MenloSmartpux client that uses MenloCloudScheduler.

Original support for Photon, but should be portable to other platforms
with more memory than an Atmega328.

The parallel project MenloSmartPux is very limited by targeting
AtMega328's, so has a very limited command set due to lack of
buffer memory on the smallest platforms. The necessary state machine
is had to update for new commands and data, so this one replaces
it.

02/28/2016

