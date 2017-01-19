#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_NORMAL_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_NORMAL_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android
{

class AudioALSACaptureDataProviderNormal : public AudioALSACaptureDataProviderBase
{
    public:
        virtual ~AudioALSACaptureDataProviderNormal();

        static AudioALSACaptureDataProviderNormal *getInstance();

        /**
         * open/close pcm interface when 1st attach & the last detach
         */
        status_t open();
        status_t close();



    protected:
        AudioALSACaptureDataProviderNormal();



    private:
        /**
         * singleton pattern
         */
        static AudioALSACaptureDataProviderNormal *mAudioALSACaptureDataProviderNormal;


        /**
         * pcm read thread
         */
        static void *readThread(void *arg);
        pthread_t hReadThread;
        status_t GetCaptureTimeStamp(time_info_struct_t *Time_Info, size_t read_size);

        uint32_t mCaptureDropSize;

        struct timespec mNewtime, mOldtime; //for calculate latency
        double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay

#ifdef MTK_VOW_SUPPORT
        /**
         * DC calculate thread
         */
        static void *DCCalThread(void *arg);
        pthread_t hDCCalThread;
        pthread_mutex_t mDCCal_Mutex;
        pthread_cond_t mDCCal_Cond;

        RingBuf mDCCalBuffer;
        bool mDCCalEnable;
        bool mDCCalBufferFull;
        AudioLock mDCCalEnableLock;

        void copyCaptureDataToDCCalBuffer(void *buffer, size_t size);
        size_t CalulateDC(short *buffer , size_t size);

        //DC calculate PCM dump
        String8 mDCCalDumpFileName;
        FILE *mDCCalDumpFile;
        void  OpenDCCalDump(void);
        void  CloseDCCalDump(void);
        void  WriteDCCalDumpData(void *buffer , size_t size);
#endif
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_NORMAL_H