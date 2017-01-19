:: Start loop testing
@SET testRunner=android.test.InstrumentationTestRunner

@CALL:log "Start MoMS TestCase"

:: Test source protection (JPE & Proguard)
@CALL:log "Test source protection"
adb install -r MoMS_Source_Protection_TestCase.apk
@SET testPackage=com.mediatek.mom.test.protection.source
@adb shell am instrument -w %testPackage%/%testRunner%

:: Test API protection
@CALL:log "Test API protection"
adb install -r MoMS_API_Protection_TestCase.apk
@SET testPackage=com.mediatek.mom.test.protection.api
@adb shell am instrument -w %testPackage%/%testRunner%

@CALL:log "Setup function test"
@call mom_function_testacse_steup

:: Test receiver controller functions
@CALL:log "Test receiver controller"
@SET testPackage=com.mediatek.mom.test.function.rec
@adb shell am instrument -w %testPackage%/%testRunner%

:: Test permission controller functions
@CALL:log "Test permission controller"
@SET testPackage=com.mediatek.mom.test.function.pmc
@adb shell am instrument -w %testPackage%/%testRunner%

@CALL:log "Finish MoMS TestCase"

@GOTO:EOF

:Log
@ECHO //--------------------------------
@ECHO // [ %1 ]
@ECHO //--------------------------------
@GOTO:EOF