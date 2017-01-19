#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_CLIENT_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_CLIENT_H

#include "AudioType.h"
#include "AudioLock.h"
#include "AudioUtility.h"

//BesRecord process +++
#include "AudioSpeechEnhLayer.h"
#include "AudioALSAVolumeController.h"
//BesRecord process ---

//Android Native Preprocess effect +++
#include "AudioPreProcess.h"
//Android Native Preprocess effect ---

namespace android
{

class AudioALSACaptureDataProviderBase;
class MtkAudioSrc;
class AudioALSACaptureDataProviderEchoRef;

/// Observer pattern: Observer
class AudioALSACaptureDataClient
{
    public:
        AudioALSACaptureDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target);
        virtual ~AudioALSACaptureDataClient();


        /**
         * set client index
         */
        void         setIdentity(const uint32_t identity);
        inline uint32_t     getIdentity() const { return mIdentity; }


        /**
         * get / process / offer data
         */
        virtual uint32_t    copyCaptureDataToClient(RingBuf pcm_read_buf); // called by capture data provider


        /**
         * read data from audio hardware
         */
        virtual ssize_t     read(void *buffer, ssize_t bytes); // called by capture handler

        //EchoRef+++
        /**
         * get / process / offer data
         */
        virtual uint32_t    copyEchoRefCaptureDataToClient(RingBuf pcm_read_buf); // called by capture data provider

        void AddEchoRefDataProvider(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target);
        //EchoRef---

        /**
         * Update BesRecord Parameters
         */
        status_t UpdateBesRecParam();

    private:
        AudioALSACaptureDataClient() {}
        AudioALSACaptureDataProviderBase *mCaptureDataProvider;

        status_t ApplyVolume(void *Buffer , uint32_t BufferSize);

        uint32_t mIdentity; // key for mCaptureDataClientVector
        bool mIsIdentitySet;

        AudioLock mLock;
        AudioCondition mWaitWorkCV;


        /**
         * attribute
         */
        const stream_attribute_t *mStreamAttributeSource; // from audio hw
        stream_attribute_t *mStreamAttributeTarget; // to stream in


        /**
         * local ring buffer
         */
        RingBuf             mRawDataBuf;
        RingBuf             mSrcDataBuf;
        RingBuf             mProcessedDataBuf;


        /**
         * Bli SRC
         */
        MtkAudioSrc *mBliSrc;

        bool mMicMute;
        bool mMuteTransition;

        //BesRecord Move to here for temp
        SPELayer    *mSPELayer;

        //#ifdef MTK_AUDIO_HD_REC_SUPPORT
        //BesRecord process +++
        //Load BesRecord parameters from NVRam
        void LoadBesRecordParams(void);
        int CheckBesRecordMode(void);
        void ConfigBesRecordParams(void);
        void StartBesRecord(void);
        void StopBesRecord(void);
        uint32_t BesRecordPreprocess(void *buffer , uint32_t bytes);
        int GetBesRecordRoutePath(void);
        int SetCaptureGain(void);
        bool CheckBesRecordBypass(void);
        bool CheckNeedBesRecordSRC(void);
        bool IsVoIPEnable(void);
        void CheckNeedDataConvert(short *buffer, ssize_t bytes);

        timespec GetCaptureTimeStamp(void);
        timespec GetEchoRefTimeStamp(void);

        bool CheckDynamicSpeechEnhancementMaskOnOff(const voip_sph_enh_dynamic_mask_t dynamic_mask_type);
        void SetDMNREnable(DMNR_TYPE type, bool enable);
        void CheckDynamicSpeechMask(void);
        void UpdateDynamicFunction(void);


        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT mBesRecordSceneTable;
        AUDIO_HD_RECORD_PARAM_STRUCT mBesRecordParam;
        AUDIO_VOIP_PARAM_STRUCT mVOIPParam;
        AUDIO_CUSTOM_EXTRA_PARAM_STRUCT mDMNRParam;

        AudioALSAVolumeController *mAudioALSAVolumeController;
        int mBesRecordModeIndex;
        int mBesRecordSceneIndex;
        bool mBesRecordStereoMode;

        bool mBypassBesRecord;
        bool mNeedBesRecordSRC;

        SPE_MODE mSpeechProcessMode;
        voip_sph_enh_mask_struct_t mVoIPSpeechEnhancementMask;
        //Audio tuning tool
        bool mBesRecTuningEnable;
        char m_strTuningFileName[VM_FILE_NAME_LEN_MAX];
        /**
         * Bli SRC
         */
        MtkAudioSrc *mBliSrcHandler1;
        MtkAudioSrc *mBliSrcHandler2;
        int mBesRecSRCSizeFactor;
        //BesRecord process ---
        //#endif

        //Android Native Preprocess effect +++
        AudioPreProcess *mAudioPreProcessEffect;
        uint32_t NativePreprocess(void *buffer , uint32_t bytes);
        void CheckNativeEffect(void);
        //Android Native Preprocess effect ---

        //EchoRef+++
        AudioALSACaptureDataProviderBase *mCaptureDataProviderEchoRef;

        /**
                 * attribute
             */
        const stream_attribute_t *mStreamAttributeSourceEchoRef; // from audio hw, need the same as DL1 stream out used
        stream_attribute_t *mStreamAttributeTargetEchoRef; // to stream in

        /**
         * local ring buffer
         */
        RingBuf             mEchoRefRawDataBuf;
        RingBuf             mEchoRefSrcDataBuf;

        /**
         * Bli SRC
         */
        MtkAudioSrc *mBliSrcEchoRef;
        MtkAudioSrc *mBliSrcEchoRefBesRecord;
        //EchoRef---
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_CLIENT_H
