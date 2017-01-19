#define LOG_TAG "AALTOOL_JNI"

#include <jni.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <linux/sensors_io.h>
#include "AALClient.h"

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jint JNICALL Java_com_mediatek_aaltool_AALALSCalibration_nGetALSRawData(JNIEnv * env, jobject jobj)
{
    int err = 0;
    int als = 0;
    int fd = open("/dev/als_ps", O_RDONLY);
    
    if (fd < 0) 
    {
        ALOGE("Fail to open alsps device (error: %s)\n", strerror(errno));
        return -1;
    }
    
    if ((err = ioctl(fd, ALSPS_GET_ALS_RAW_DATA, &als)))
    {
        ALOGE("ioctl ALSPS_GET_ALS_RAW_DATA error: %d\n", err);
        close(fd);
        return -1;
    }
    
    ALOGD("als = %d\n", als);
    close(fd);
    return als;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetBrightnessLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Brightness level = %d", level);
    if (AALClient::getInstance().setBrightnessLevel(level) != 0)
    {
        ALOGE("fail to set Brightness level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetDarkeningSpeedLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("DarkeningSpeed level = %d", level);
    if (AALClient::getInstance().setDarkeningSpeedLevel(level) != 0)
    {
        ALOGE("fail to set Darkening Speed level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetBrighteningSpeedLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("BrighteningSpeed level = %d", level);
    if (AALClient::getInstance().setBrighteningSpeedLevel(level) != 0)
    {
        ALOGE("fail to set Brightening Speed level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetSmartBacklightStrength(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("SmartBacklight level = %d", level);
    if (AALClient::getInstance().setSmartBacklightStrength(level) != 0)
    {
        ALOGE("fail to set SmartBacklight strength");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetSmartBacklightRange(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("SmartBacklight level = %d", level);
    if (AALClient::getInstance().setSmartBacklightRange(level) != 0)
    {
        ALOGE("fail to set SmartBacklight range");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetReadabilityLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Readability level = %d", level);
    if (AALClient::getInstance().setReadabilityLevel(level) != 0)
    {
        ALOGE("fail to set Readability level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetBacklight(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Backlight level = %d", level);
    if (AALClient::getInstance().setBacklightBrightness(level) != 0)
    {
        ALOGE("fail to set backlight level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


class JavaObjectRef {
private:
    JNIEnv *mJEnv;
    jclass mClass;
    jobject mObj;
    
public:
    JavaObjectRef(JNIEnv *env, const char *className, jobject obj) {
        mJEnv = env;
        mClass = mJEnv->FindClass(className);
        mObj = obj;

        if (mClass == NULL)
            ALOGE("ERROR: class %s not found", className);
    }

    inline void set(const char *fieldName, jint value) {
        if (mClass != NULL) {
            jfieldID field = mJEnv->GetFieldID(mClass, fieldName, "I");
            if (field != NULL)
                mJEnv->SetIntField(mObj, field, value);
            else
                ALOGE("ERROR: field %s not found", fieldName);
        }
    }
};


JNIEXPORT void JNICALL Java_com_mediatek_aaltool_AALTuning_nGetParameters(JNIEnv *env, jobject jobj, jobject jparam)
{
    AALParameters param;

    if (AALClient::getInstance().getParameters(&param) == NO_ERROR) {
        JavaObjectRef jParamRef(env, "com/mediatek/aaltool/AALTuning$AALParameters", jparam);
        
        if (param.brightnessLevel >= 0)
            jParamRef.set("brightnessLevel", param.brightnessLevel);
        if (param.darkeningSpeedLevel >= 0)
            jParamRef.set("darkeningSpeedLevel", param.darkeningSpeedLevel);
        if (param.brighteningSpeedLevel >= 0)
            jParamRef.set("brighteningSpeedLevel", param.brighteningSpeedLevel);
        if (param.readabilityLevel >= 0)
            jParamRef.set("readabilityLevel", param.readabilityLevel);
        if (param.smartBacklightStrength >= 0)
            jParamRef.set("smartBacklightStrength", param.smartBacklightStrength);
        if (param.smartBacklightRange >= 0)
            jParamRef.set("smartBacklightRange", param.smartBacklightRange);

    } else {
        ALOGE("fail to get AAL parameters");
    }
}


#ifdef __cplusplus
}
#endif
