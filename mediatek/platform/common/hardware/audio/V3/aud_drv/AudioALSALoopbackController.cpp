#include "AudioALSALoopbackController.h"

#include "AudioALSAHardwareResourceManager.h"


#define LOG_TAG "AudioALSALoopbackController"

namespace android
{

AudioALSALoopbackController *AudioALSALoopbackController::mAudioALSALoopbackController = NULL;
AudioALSALoopbackController *AudioALSALoopbackController::getInstance()
{
    AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSALoopbackController == NULL)
    {
        mAudioALSALoopbackController = new AudioALSALoopbackController();
    }
    ASSERT(mAudioALSALoopbackController != NULL);
    return mAudioALSALoopbackController;
}


AudioALSALoopbackController::AudioALSALoopbackController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mPcmDL(NULL),
    mPcmUL(NULL)
{

}


AudioALSALoopbackController::~AudioALSALoopbackController()
{

}


status_t AudioALSALoopbackController::open(const audio_devices_t output_devices, const audio_devices_t input_device)
{
    ALOGD("+%s(), output_devices = 0x%x, input_device = 0x%x", __FUNCTION__, output_devices, input_device);
    AudioAutoTimeoutLock _l(mLock);


    // DL loopback setting
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = 2;
    mConfig.rate = 48000;
    mConfig.period_size = 1024;
    mConfig.period_count = 2;
    mConfig.format = PCM_FORMAT_S16_LE;
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    ALOGD("+%s(), mConfig.rate=%d", __FUNCTION__,mConfig.rate);

    ASSERT(mPcmUL == NULL && mPcmDL == NULL);
    mPcmUL = pcm_open(0, 4, PCM_IN, &mConfig);
    mPcmDL = pcm_open(0, 4, PCM_OUT, &mConfig);
    ASSERT(mPcmUL != NULL && mPcmDL != NULL);
    ALOGV("%s(), mPcmUL = %p, mPcmDL = %p", __FUNCTION__, mPcmUL, mPcmDL);

    pcm_start(mPcmUL);
    pcm_start(mPcmDL);


    mHardwareResourceManager->startInputDevice(input_device);
    mHardwareResourceManager->startOutputDevice(output_devices, mConfig.rate);


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSALoopbackController::close()
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("+%s()", __FUNCTION__);

    mHardwareResourceManager->stopOutputDevice();

    pcm_stop(mPcmDL);
    pcm_stop(mPcmUL);
    pcm_close(mPcmDL);
    pcm_close(mPcmUL);

    mPcmDL = NULL;
    mPcmUL = NULL;

    mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


} // end of namespace android
