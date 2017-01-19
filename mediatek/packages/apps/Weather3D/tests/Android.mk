# Add appropriate copyright banner here

ifeq ($(MTK_AUTO_TEST) , yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# We only want this apk build for tests.
LOCAL_MODULE_TAGS := tests

# Include all test java files.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

# Notice that we don't have to include the src files of ApiDemos because, by
# running the tests using an instrumentation targeting ApiDemos, we
# automatically get all of its classes loaded into our environment.

LOCAL_PACKAGE_NAME := Weather3DWidgetTests

LOCAL_JAVA_LIBRARIES := android.test.runner

LOCAL_STATIC_JAVA_LIBRARIES := libjunitreport-for-weather3d-tests hamcrest-core-for-weather3d-tests hamcrest-library-for-weather3d-tests robotium-solo-for-weather3d-tests libperfhelper-weather3d

LOCAL_INSTRUMENTATION_FOR := Weather3DWidget

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

##################################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libjunitreport-for-weather3d-tests:libs/android-junit-report-dev.jar hamcrest-core-for-weather3d-tests:libs/hamcrest-core-SNAPSHOT.jar hamcrest-library-for-weather3d-tests:libs/hamcrest-library-SNAPSHOT.jar robotium-solo-for-weather3d-tests:libs/robotium-solo-4.3.1.jar libperfhelper-weather3d:libs/perfhelper.jar
include $(BUILD_MULTI_PREBUILT)

endif
