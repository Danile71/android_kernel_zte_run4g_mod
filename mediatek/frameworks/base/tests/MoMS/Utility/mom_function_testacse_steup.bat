:: MoMS Function Test Pre-condition
@SET TencentApk=Tencent_Permission_Manager_CTA.apk
:: Current is version 3.0
@SET CTAApk=MTK_CtaTestAPK_KK.apk

@adb remount
@adb push %TencentApk% /sdcard/MoMS/%TencentApk%
@adb push %CTAApk% /sdcard/MoMS/%CTAApk%
@adb uninstall com.tencent.tcuser
@adb shell rm -f /system/app/%TencentApk%
@adb shell rm -f /system/app/PermissionControl.apk

@adb install -r MoMS_Function_PMC_TestCase.apk
@adb install -r MoMS_Function_REC_TestCase.apk
@adb install -r ManagerEmulator.apk