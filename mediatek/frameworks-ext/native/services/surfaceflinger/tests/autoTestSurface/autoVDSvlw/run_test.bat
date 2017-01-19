adb remount
rem adb push ..\..\..\..\..\..\..\..\out\target\product\mt6595_phone_v1\system\bin\autoVDSvlwUnitTest /system/bin
adb push ..\bin\autoVDSvlwUnitTest /system/bin
adb shell chmod 777 /system/bin/autoVDSvlwUnitTest
adb shell autoVDSvlwUnitTest 1 1 120 0 0
