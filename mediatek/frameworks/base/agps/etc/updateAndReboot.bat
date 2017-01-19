adb remount
adb push agps_profiles_conf2.xml /etc/agps_profiles_conf2.xml
adb shell rm /data/agps_supl/agps_profiles_conf2.xml
adb shell sync
adb shell reboot
pause