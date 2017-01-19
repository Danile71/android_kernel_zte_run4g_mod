adb push ./busybox-armv7l /system/bin
adb shell "chmod 777 /system/bin/busybox-armv7l"
echo After stresstool started, you can enter CTRL + C to quit
adb shell "busybox-armv7l nohup stresstool -n3 -mfree --minsize=12"
