#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF_EXT_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF_EXT_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android
{

class AudioALSACaptureDataProviderEchoRefExt : public AudioALSACaptureDataProviderBase
{
    public:
        virtual ~AudioALSACaptureDataProviderEchoRefExt();

        static AudioALSACaptureDataProviderEchoRefExt *getInstance();

        /**
         * open/close pcm interface when 1st attach & the last detach
         */
        status_t open();
        status_t close();



    protected:
        AudioALSACaptureDataProviderEchoRefExt();



    private:
        /**
         * singleton pattern
         */
        static AudioALSACaptureDataProviderEchoRefExt *mAudioALSACaptureDataProviderEchoRefExt;


        /**
         * pcm read thread
         */
        static void *readThread(void *arg);
        pthread_t hReadThread;

        struct timespec mNewtime, mOldtime; //for calculate latency
        double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay

        status_t GetCaptureTimeStamp(time_info_struct_t *Time_Info, size_t read_size);
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF_EXT_H