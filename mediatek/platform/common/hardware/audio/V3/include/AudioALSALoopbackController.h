#ifndef ANDROID_AUDIO_ALSA_LOOPBACK_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_LOOPBACK_CONTROLLER_H

#include <tinyalsa/asoundlib.h> // TODO(Harvey): move it

#include "AudioType.h"
#include "AudioLock.h"


namespace android
{

class AudioALSAHardwareResourceManager;

class AudioALSALoopbackController
{
    public:
        virtual ~AudioALSALoopbackController();

        static AudioALSALoopbackController *getInstance();

        virtual status_t        open(const audio_devices_t output_devices, const audio_devices_t input_device);
        virtual status_t        close();



    protected:
        AudioALSALoopbackController();

        AudioALSAHardwareResourceManager *mHardwareResourceManager;

        AudioLock mLock;

        struct pcm_config mConfig;

        struct pcm *mPcmDL;
        struct pcm *mPcmUL;



    private:
        /**
         * singleton pattern
         */
        static AudioALSALoopbackController *mAudioALSALoopbackController;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_LOOPBACK_CONTROLLER_H
