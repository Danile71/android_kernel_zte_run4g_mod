adb remount
rem adb push ..\..\..\..\..\..\..\..\out\target\product\mt6595_phone_v1\system\bin\autoIonFdLifeCycleUnitTest /system/bin
adb push ..\bin\autoIonFdLifeCycleUnitTest /system/bin
adb shell chmod 777 /system/bin/autoIonFdLifeCycleUnitTest
adb shell autoIonFdLifeCycleUnitTest 1 1000 0 15000
