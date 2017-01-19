#ifndef ANDROID_AUDIO_ALSA_DRIVER_UTILITY_H
#define ANDROID_AUDIO_ALSA_DRIVER_UTILITY_H

#include <tinyalsa/asoundlib.h>

#include "AudioType.h"


namespace android
{

class AudioALSADriverUtility
{
    public:
        virtual ~AudioALSADriverUtility();

        static AudioALSADriverUtility *getInstance();

        struct mixer *getMixer() const { return mMixer; }

        int GetPropertyValue(const char* ProPerty_Key);


    private:
        AudioALSADriverUtility();


        /**
         * singleton pattern
         */
        static AudioALSADriverUtility *mAudioALSADriverUtility;


        /**
         * singleton pattern
         */
        struct mixer *mMixer;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_DRIVER_UTILITY_H
