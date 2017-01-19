
//#define LOG_NDEBUG 0
#define LOG_TAG "ds1_utility"
#include <system/audio.h>
#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/String16.h>
#include <utils/RefBase.h>
#include <binder/BinderService.h>
#include <binder/Parcel.h>
using namespace android;

static int gHdmiSupportedChannel = 2;

#define DOLBY_SYSTEM_PROPERTY "dolby.audio.sink.info"

void ds1ConfigureHdmiSupportedChannel(int channelCount)
{
    ALOGV("%s channelCount = %d\n", __FUNCTION__, channelCount);
    gHdmiSupportedChannel = channelCount;
}

void ds1ConfigureRoutingDevice(unsigned int audioDevice)
{
    switch (audioDevice)
    {
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
            ALOGV("%s headset\n", __FUNCTION__);
            property_set(DOLBY_SYSTEM_PROPERTY, "headset");
            break;
        case AUDIO_DEVICE_OUT_AUX_DIGITAL:
            if (gHdmiSupportedChannel == 8) {
                ALOGV("%s hdmi8\n", __FUNCTION__);
                property_set(DOLBY_SYSTEM_PROPERTY, "hdmi8");
            } else if (gHdmiSupportedChannel == 6) {
                ALOGV("%s hdmi6\n", __FUNCTION__);
                property_set(DOLBY_SYSTEM_PROPERTY, "hdmi6");
            } else {
                ALOGV("%s hdmi2\n", __FUNCTION__);
                property_set(DOLBY_SYSTEM_PROPERTY, "hdmi2");
            }
            break;
        case AUDIO_DEVICE_OUT_SPEAKER:
            ALOGV("%s speaker\n", __FUNCTION__);
            property_set(DOLBY_SYSTEM_PROPERTY, "speaker");
            break;
       case AUDIO_DEVICE_OUT_REMOTE_SUBMIX:
            ALOGV("DOLBY_ENDPOINT HDMI2");
            property_set(DOLBY_SYSTEM_PROPERTY, "hdmi2");
            break;
        case AUDIO_DEVICE_OUT_DEFAULT:
        default:
            ALOGV("%s invalid\n", __FUNCTION__);
            property_set(DOLBY_SYSTEM_PROPERTY, "invalid");
            break;
    }
}// Send a broadcast event with value set in
// resultCode field, so that the receiver (DsService) gets this 
// intent, checks the action and reads the resultCode.
bool ds1SendBroadcastMessage(String16 action, int value)
{
    ALOGV("sendBroadcastMessage(): Action: %s, Value: %d ", action.string(), value);
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> am = sm->getService(String16("activity"));
    if (am != NULL) {
        int msg[] = {0, -1, 0, -1, -1, 0, 0, 0, 0, -1, -1 }; 
        unsigned int i;
        Parcel data, reply;

        data.writeInterfaceToken(String16("android.app.IActivityManager"));
        data.writeStrongBinder(NULL);
        data.writeString16(action);

        for (i = 0; i < sizeof(msg) / sizeof(msg[0]); i++) {
            data.writeInt32(msg[i]);
        }

        data.writeStrongBinder(NULL);
        data.writeInt32(value);
        data.writeInt32(-1);
        data.writeInt32(-1);
        data.writeInt32(-1);
        data.writeInt32(0);
        data.writeInt32(0);
        data.writeInt32(0);

        status_t ret = am->transact(IBinder::FIRST_CALL_TRANSACTION+13, data, &reply); // BROADCAST_INTENT_TRANSACTION
        
        if (ret == NO_ERROR) {
            int exceptionCode = reply.readExceptionCode();
            if (exceptionCode) {
                ALOGE("sendBroadcastMessage(%s) caught exception %d\n",
                        action.string(), exceptionCode);
                return false;
            }
        } else {
            return false;
        }
    } else {
        ALOGE("getService() couldn't find activity service!\n");
        return false;
    }
    return true;
}

bool checkFlagsToMoveDs(audio_output_flags_t flags)
{
    if ((flags & AUDIO_OUTPUT_FLAG_DIRECT) ||
        (flags & AUDIO_OUTPUT_FLAG_FAST)) {
        return false;
    }
	
    return true;    
}
