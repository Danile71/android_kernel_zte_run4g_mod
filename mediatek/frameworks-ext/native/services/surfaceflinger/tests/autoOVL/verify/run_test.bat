adb remount
rem adb push ..\..\..\..\..\..\..\..\out\target\product\mt6595_phone_v1\system\bin\autoOvl /system/bin
adb push ..\bin\autoOvl /system/bin
adb shell chmod 777 /system/bin/autoOvl
adb shell autoOvl -l 2 -d 1 -l 1

