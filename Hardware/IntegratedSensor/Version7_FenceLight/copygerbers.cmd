mkdir \tmp\GerbersDir

rem Top Copper
xcopy /y IntegratedSensor.GTL \tmp\GerbersDir

rem Top Solder Mask
xcopy /y IntegratedSensor.GTS \tmp\GerbersDir

rem Top Silkscreen
xcopy /y IntegratedSensor.GTO \tmp\GerbersDir

rem Bottom Copper
xcopy /y IntegratedSensor.GBL \tmp\GerbersDir

rem Bottom Solder Mask
xcopy /y IntegratedSensor.GBS \tmp\GerbersDir

rem Bottom Silkscreen
xcopy /y IntegratedSensor.GBO \tmp\GerbersDir

rem Drill File
xcopy /y IntegratedSensor.TXT \tmp\GerbersDir

rem Top Paste File
xcopy /y IntegratedSensor.GTP \tmp\GerbersDir

rem
rem These confuse the BatchPCB order submitter
rem
rem But needed for Pentalogix
rem

rem Drill Tool File
xcopy /y IntegratedSensor.dri \tmp\GerbersDir

rem Aperture File
xcopy /y IntegratedSensor.gpi \tmp\GerbersDir

rem ???
xcopy /y IntegratedSensor.pro \tmp\GerbersDir
