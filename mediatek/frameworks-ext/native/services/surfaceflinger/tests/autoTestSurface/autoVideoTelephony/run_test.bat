adb remount
rem adb push ..\..\..\..\..\..\..\..\out\target\product\mt6595_phone_v1\system\bin\autoVideoTelephonyUnitTest /system/bin
adb push ..\bin\autoVideoTelephonyUnitTest /system/bin
adb shell chmod 777 /system/bin/autoVideoTelephonyUnitTest
adb shell autoVideoTelephonyUnitTest 2 2 60
