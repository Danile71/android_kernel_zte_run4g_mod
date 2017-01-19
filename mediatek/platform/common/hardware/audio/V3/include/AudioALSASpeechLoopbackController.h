#ifndef ANDROID_AUDIO_ALSA_SPEECH_LOOPBACK_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_SPEECH_LOOPBACK_CONTROLLER_H

#include <tinyalsa/asoundlib.h> // TODO(Harvey): move it

#include "AudioType.h"
#include "SpeechType.h"

#include "AudioLock.h"


namespace android
{

class AudioALSAHardwareResourceManager;
class SpeechDriverFactory;

class AudioALSASpeechLoopbackController
{
    public:
        virtual ~AudioALSASpeechLoopbackController();

        static AudioALSASpeechLoopbackController *getInstance();

        virtual status_t        open(const audio_devices_t output_devices, const audio_devices_t input_device);
        virtual status_t        close();



    protected:
        AudioALSASpeechLoopbackController();

        AudioALSAHardwareResourceManager *mHardwareResourceManager;

        SpeechDriverFactory    *mSpeechDriverFactory;


        AudioLock               mLock;

        struct pcm_config mConfig;

        struct pcm *mPcmUL;
        struct pcm *mPcmDL;



    private:
        static AudioALSASpeechLoopbackController *mSpeechLoopbackController; // singleton

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_SPEECH_LOOPBACK_CONTROLLER_H
