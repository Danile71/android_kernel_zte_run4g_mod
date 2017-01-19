#include "AudioALSACaptureHandlerANC.h"

#include "AudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderANC.h"



#define LOG_TAG "AudioALSACaptureHandlerANC"

namespace android
{

AudioALSACaptureHandlerANC::AudioALSACaptureHandlerANC(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target)
{
    ALOGD("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerANC::~AudioALSACaptureHandlerANC()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerANC::init()
{
    ALOGD("%s()", __FUNCTION__);


    return NO_ERROR;
}


status_t AudioALSACaptureHandlerANC::open()
{
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source);

    // TODO: check ANC is already opened?

    ASSERT(mCaptureDataClient == NULL);
    mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderANC::getInstance(), mStreamAttributeTarget);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerANC::close()
{
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerANC::routing(const audio_devices_t input_device)
{
    WARNING("Not support!!");
    return INVALID_OPERATION;
}


ssize_t AudioALSACaptureHandlerANC::read(void *buffer, ssize_t bytes)
{
    ALOGV("%s()", __FUNCTION__);

    return mCaptureDataClient->read(buffer, bytes);
}

} // end of namespace android
