#include "AudioALSASampleRateController.h"

#include "AudioLock.h"


#define LOG_TAG "AudioALSASampleRateController"

namespace android
{

AudioALSASampleRateController *AudioALSASampleRateController::mAudioALSASampleRateController = NULL;
AudioALSASampleRateController *AudioALSASampleRateController::getInstance()
{
    AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSASampleRateController == NULL)
    {
        mAudioALSASampleRateController = new AudioALSASampleRateController();
    }
    ASSERT(mAudioALSASampleRateController != NULL);
    return mAudioALSASampleRateController;
}


AudioALSASampleRateController::AudioALSASampleRateController() :
    mPrimaryStreamOutSampleRate(44100),
    mPlaybackScenarioMask(0)
{
    ALOGD("%s()", __FUNCTION__);
}


AudioALSASampleRateController::~AudioALSASampleRateController()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSASampleRateController::setPrimaryStreamOutSampleRate(const uint32_t sample_rate)
{
    AudioAutoTimeoutLock _l(mLock);

    ALOGD("+%s(), mPrimaryStreamOutSampleRate: %u => %u", __FUNCTION__, mPrimaryStreamOutSampleRate, sample_rate);

    if (mPlaybackScenarioMask != 0)
    {
        ALOGW("-%s(), mPlaybackScenarioMask(0x%x) != 0, return", __FUNCTION__, mPlaybackScenarioMask);
        return INVALID_OPERATION;
    }
    else if (sample_rate == mPrimaryStreamOutSampleRate)
    {
        ALOGW("-%s(), sample_rate == mPrimaryStreamOutSampleRate, return", __FUNCTION__);
        return ALREADY_EXISTS;
    }


    mPrimaryStreamOutSampleRate = sample_rate;


    ALOGD("-%s(), mPrimaryStreamOutSampleRate: %u", __FUNCTION__, mPrimaryStreamOutSampleRate);
    return NO_ERROR;
}


uint32_t AudioALSASampleRateController::getPrimaryStreamOutSampleRate()
{
    AudioAutoTimeoutLock _l(mLock);
    return mPrimaryStreamOutSampleRate;
}


void AudioALSASampleRateController::setScenarioStatus(const playback_scenario_mask_t playback_scenario_mask)
{
    AudioAutoTimeoutLock _l(mLock);

    ASSERT(getScenarioStatus(playback_scenario_mask) == false);
    mPlaybackScenarioMask |= playback_scenario_mask;

    ALOGV("AudioALSASampleRateController mPlaybackScenarioMask = %x", mPlaybackScenarioMask);
}

void AudioALSASampleRateController::resetScenarioStatus(const playback_scenario_mask_t playback_scenario_mask)
{
    AudioAutoTimeoutLock _l(mLock);

    ALOGV("AudioALSASampleRateController mPlaybackScenarioMask = %x", mPlaybackScenarioMask);

    ASSERT(getScenarioStatus(playback_scenario_mask) == true);
    mPlaybackScenarioMask &= (~playback_scenario_mask);
}


} // end of namespace android
