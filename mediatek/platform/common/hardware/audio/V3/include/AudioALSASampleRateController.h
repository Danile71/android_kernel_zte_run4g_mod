#ifndef ANDROID_AUDIO_ALSA_SAMPLE_RATE_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_SAMPLE_RATE_CONTROLLER_H

#include "AudioType.h"
#include "AudioAssert.h"
#include "AudioLock.h"

namespace android
{

enum playback_scenario_mask_t
{
    PLAYBACK_SCENARIO_STREAM_OUT    = (1 << 0),
    PLAYBACK_SCENARIO_FM            = (1 << 1),
    PLAYBACK_SCENARIO_ECHO_REF      = (1 << 2),
    PLAYBACK_SCENARIO_ECHO_REF_EXT  = (1 << 3),
};


class AudioALSASampleRateController
{
    public:
        virtual ~AudioALSASampleRateController();

        static AudioALSASampleRateController *getInstance();

        virtual status_t setPrimaryStreamOutSampleRate(const uint32_t sample_rate);
        uint32_t         getPrimaryStreamOutSampleRate();


        void             setScenarioStatus(const playback_scenario_mask_t playback_scenario_mask);
        void             resetScenarioStatus(const playback_scenario_mask_t playback_scenario_mask);




    protected:
        AudioALSASampleRateController();


        inline bool      getScenarioStatus(const playback_scenario_mask_t playback_scenario_mask) const
        {
            return ((mPlaybackScenarioMask & playback_scenario_mask) > 0);
        }



    private:
        /**
         * singleton pattern
         */
        static AudioALSASampleRateController *mAudioALSASampleRateController;

        uint32_t mPrimaryStreamOutSampleRate;

        uint32_t mPlaybackScenarioMask;

        AudioLock mLock;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_SAMPLE_RATE_CONTROLLER_H
