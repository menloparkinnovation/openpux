
mkdir \tmp\GerbersDir
xcopy /y IntegratedSensor.TXT \tmp\GerbersDir
xcopy /y IntegratedSensor.GBL \tmp\GerbersDir
xcopy /y IntegratedSensor.GTL \tmp\GerbersDir
xcopy /y IntegratedSensor.GBO \tmp\GerbersDir
xcopy /y IntegratedSensor.GTO \tmp\GerbersDir
xcopy /y IntegratedSensor.GBS \tmp\GerbersDir
xcopy /y IntegratedSensor.GTS \tmp\GerbersDir
xcopy /y IntegratedSensor.GTP \tmp\GerbersDir

rem These confuse the BatchPCB order submitter
rem xcopy /y IntegratedSensor.dri \tmp\GerbersDir
rem xcopy /y IntegratedSensor.gpi \tmp\GerbersDir
rem xcopy /y IntegratedSensor.pro \tmp\GerbersDir
