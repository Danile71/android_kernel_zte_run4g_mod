#include "AudioALSASpeechLoopbackController.h"

#include "AudioALSAHardwareResourceManager.h"

#include "SpeechDriverInterface.h"
#include "SpeechDriverFactory.h"


#define LOG_TAG "AudioALSASpeechLoopbackController"

namespace android
{

AudioALSASpeechLoopbackController *AudioALSASpeechLoopbackController::mSpeechLoopbackController = NULL;
AudioALSASpeechLoopbackController *AudioALSASpeechLoopbackController::getInstance()
{
    static AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mSpeechLoopbackController == NULL)
    {
        mSpeechLoopbackController = new AudioALSASpeechLoopbackController();
    }
    ASSERT(mSpeechLoopbackController != NULL);
    return mSpeechLoopbackController;
}


AudioALSASpeechLoopbackController::AudioALSASpeechLoopbackController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mSpeechDriverFactory(SpeechDriverFactory::GetInstance()),
    mPcmUL(NULL),
    mPcmDL(NULL)
{

}


AudioALSASpeechLoopbackController::~AudioALSASpeechLoopbackController()
{

}

status_t AudioALSASpeechLoopbackController::open(const audio_devices_t output_devices, const audio_devices_t input_device)
{
    ALOGD("+%s(), output_devices = 0x%x, input_device = 0x%x", __FUNCTION__, output_devices, input_device);
    AudioAutoTimeoutLock _l(mLock);

    // get speech driver instance
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // check BT device // TODO(Harvey): BT Loopback?
    const uint32_t sample_rate = 16000;
    ALOGD("%s(), sample_rate = %d", __FUNCTION__, sample_rate);


    //--- here to test pcm interface platform driver_attach
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = 2;
    mConfig.rate = sample_rate;
    mConfig.period_size = 1024;
    mConfig.period_count = 2;
    mConfig.format = PCM_FORMAT_S16_LE;
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;

    ASSERT(mPcmUL == NULL && mPcmDL == NULL);
    mPcmUL = pcm_open(0, 2, PCM_IN, &mConfig);
    mPcmDL = pcm_open(0, 2, PCM_OUT, &mConfig);
    ASSERT(mPcmUL != NULL && mPcmDL != NULL);
    ALOGV("%s(), mPcmUL = %p, mPcmDL = %p", __FUNCTION__, mPcmUL, mPcmDL);

    pcm_start(mPcmUL);
    pcm_start(mPcmDL);


    // Set PMIC digital/analog part - uplink has pop, open first
    mHardwareResourceManager->startInputDevice(input_device);


    // Clean Side Tone Filter gain
    pSpeechDriver->SetSidetoneGain(0);

    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    pSpeechDriver->SetSpeechMode(input_device, output_devices);

    // Loopback on
    pSpeechDriver->SetAcousticLoopback(true);

    mHardwareResourceManager->startOutputDevice(output_devices, sample_rate);


    ALOGD("-%s(), output_devices = 0x%x, input_device = 0x%x", __FUNCTION__, output_devices, input_device);
    return NO_ERROR;
}


status_t AudioALSASpeechLoopbackController::close()
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("+%s()", __FUNCTION__);

    mHardwareResourceManager->stopOutputDevice();

    // Stop MODEM_PCM
    pcm_stop(mPcmDL);
    pcm_stop(mPcmUL);
    pcm_close(mPcmDL);
    pcm_close(mPcmUL);

    mPcmDL = NULL;
    mPcmUL = NULL;

    mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());

    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // Loopback off
    pSpeechDriver->SetAcousticLoopback(false);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

} // end of namespace android
