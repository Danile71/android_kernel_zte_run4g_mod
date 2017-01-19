#include "AudioALSACaptureHandlerFMRadio.h"

#include "AudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderFMRadio.h"



#define LOG_TAG "AudioALSACaptureHandlerFMRadio"

namespace android
{

AudioALSACaptureHandlerFMRadio::AudioALSACaptureHandlerFMRadio(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target)
{
    ALOGD("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerFMRadio::~AudioALSACaptureHandlerFMRadio()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerFMRadio::init()
{
    ALOGD("%s()", __FUNCTION__);


    return NO_ERROR;
}


status_t AudioALSACaptureHandlerFMRadio::open()
{
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source);

    // TODO(Harvey): check FM is already opened?

    ASSERT(mCaptureDataClient == NULL);
    mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderFMRadio::getInstance(), mStreamAttributeTarget);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerFMRadio::close()
{
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerFMRadio::routing(const audio_devices_t input_device)
{
    WARNING("Not support!!");
    return INVALID_OPERATION;
}


ssize_t AudioALSACaptureHandlerFMRadio::read(void *buffer, ssize_t bytes)
{
    ALOGV("%s()", __FUNCTION__);

    return mCaptureDataClient->read(buffer, bytes);
}

} // end of namespace android
