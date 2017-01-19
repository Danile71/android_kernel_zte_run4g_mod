adb remount
rem adb push ..\..\..\..\..\..\..\..\out\target\product\mt6595_phone_v1\system\bin\autoVDSaddVideoUnitTest /system/bin
adb push ..\bin\autoVDSaddVideoUnitTest /system/bin
adb shell chmod 777 /system/bin/autoVDSaddVideoUnitTest
adb shell autoVDSaddVideoUnitTest 1 1 120 1 0
