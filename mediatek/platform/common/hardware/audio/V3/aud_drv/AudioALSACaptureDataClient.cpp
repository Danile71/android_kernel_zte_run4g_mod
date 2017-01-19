#include "AudioALSACaptureDataClient.h"

#include "AudioUtility.h"

#include "AudioType.h"
#include "AudioLock.h"

#include "AudioALSACaptureDataProviderBase.h"

//BesRecord+++
#include "AudioCustParam.h"
#include "CFG_Audio_Default.h"
//BesRecord---

#define LOG_TAG "AudioALSACaptureDataClient"

extern "C" {
#include "MtkAudioSrc.h"
}

namespace android
{
#define MTK_STREAMIN_VOLUEM_MAX (0x1000)
#define MTK_STREAMIN_VOLUME_VALID_BIT (12)

//BesRecord+++
#define VOICE_RECOGNITION_RECORD_SAMPLE_RATE (16000)
#define HD_RECORD_SAMPLE_RATE (48000)
#define NORMAL_RECORDING_DEFAULT_MODE    (1)
#define VOICE_REC_RECORDING_DEFAULT_MODE (0)
#define VOICE_UnLock_RECORDING_DEFAULT_MODE (6)
//BesRecord---

/*==============================================================================
 *                     Constant
 *============================================================================*/

static const uint32_t kClientBufferSize = 0x8000; // 32k


/*==============================================================================
 *                     Implementation
 *============================================================================*/
static short clamp16(int sample)
{
    if ((sample >> 15) ^ (sample >> 31))
    {
        sample = 0x7FFF ^ (sample >> 31);
    }
    return sample;
}

AudioALSACaptureDataClient::AudioALSACaptureDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target) :
    mCaptureDataProvider(pCaptureDataProvider),
    mIdentity(0xFFFFFFFF),
    mStreamAttributeSource(mCaptureDataProvider->getStreamAttributeSource()),
    mStreamAttributeTarget(stream_attribute_target),
    mMicMute(false),
    mMuteTransition(false),
    mIsIdentitySet(false),
    mBliSrc(NULL),
    mSPELayer(NULL),
    mAudioALSAVolumeController(AudioALSAVolumeController::getInstance()),
    //echoref+++
    mCaptureDataProviderEchoRef(NULL),
    mStreamAttributeSourceEchoRef(NULL),
    mStreamAttributeTargetEchoRef(NULL),
    mBliSrcEchoRef(NULL),
    mBliSrcEchoRefBesRecord(NULL)
    //echoref---
{
    ALOGD("%s()", __FUNCTION__);

    // raw data
    memset((void *)&mRawDataBuf, 0, sizeof(mRawDataBuf));
    mRawDataBuf.pBufBase = new char[kClientBufferSize];
    mRawDataBuf.bufLen   = kClientBufferSize;
    mRawDataBuf.pRead    = mRawDataBuf.pBufBase;
    mRawDataBuf.pWrite   = mRawDataBuf.pBufBase;
    ASSERT(mRawDataBuf.pBufBase != NULL);

    // src data
    memset((void *)&mSrcDataBuf, 0, sizeof(mSrcDataBuf));
    mSrcDataBuf.pBufBase = new char[kClientBufferSize];
    mSrcDataBuf.bufLen   = kClientBufferSize;
    mSrcDataBuf.pRead    = mSrcDataBuf.pBufBase;
    mSrcDataBuf.pWrite   = mSrcDataBuf.pBufBase;
    ASSERT(mSrcDataBuf.pBufBase != NULL);

    // processed data
    memset((void *)&mProcessedDataBuf, 0, sizeof(mProcessedDataBuf));
    mProcessedDataBuf.pBufBase = new char[kClientBufferSize];
    mProcessedDataBuf.bufLen   = kClientBufferSize;
    mProcessedDataBuf.pRead    = mProcessedDataBuf.pBufBase;
    mProcessedDataBuf.pWrite   = mProcessedDataBuf.pBufBase;
    ASSERT(mProcessedDataBuf.pBufBase != NULL);

    //TODO: Sam, move here for temp
    //BesRecord+++
    mBesRecordModeIndex = -1;
    mBesRecordSceneIndex = -1;
    mBesRecordStereoMode = false;
    mBypassBesRecord = false;
    mNeedBesRecordSRC = false;
    mBliSrcHandler1 = NULL;
    mBliSrcHandler2 = NULL;
    mBesRecSRCSizeFactor = 1;

    mSpeechProcessMode = SPE_MODE_REC;
    mVoIPSpeechEnhancementMask = mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask;

    //BesRecord Config
    mSPELayer = new SPELayer();
    if (!mSPELayer)
    {
        ALOGE("new SPELayer() FAIL");
        ASSERT(mSPELayer != NULL);
    }

    SetCaptureGain();
    ALOGD("%s(), besrecord_enable=%d, besrecord_scene=%d", __FUNCTION__, mStreamAttributeTarget->BesRecord_Info.besrecord_enable,
          mStreamAttributeTarget->BesRecord_Info.besrecord_scene);
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_enable)
    {
        LoadBesRecordParams();

        mSPELayer->SetVMDumpEnable(mStreamAttributeTarget->BesRecord_Info.besrecord_tuningEnable || mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable);
        mSPELayer->SetVMDumpFileName(mStreamAttributeTarget->BesRecord_Info.besrecord_VMFileName);

        CheckBesRecordBypass();
        CheckNeedBesRecordSRC();
        CheckBesRecordMode();
        ConfigBesRecordParams();
        StartBesRecord();
    }
    //BesRecord---
    //Android Native Preprocess effect +++
    mAudioPreProcessEffect = NULL;
    mAudioPreProcessEffect = new AudioPreProcess(mStreamAttributeTarget);
    if (!mAudioPreProcessEffect)
    {
        ALOGE("new mAudioPreProcessEffect() FAIL");
        ASSERT(mAudioPreProcessEffect != NULL);
    }
    CheckNativeEffect();
    //Android Native Preprocess effect ---

    // attach client to capture data provider
    ALOGV("mCaptureDataProvider=%p", mCaptureDataProvider);
    mCaptureDataProvider->attach(this); // mStreamAttributeSource will be updated when first client attached

    //assume starts after PCM open
    mSPELayer->SetUPLinkDropTime(CAPTURE_DROP_MS);
    mSPELayer->SetUPLinkIntrStartTime(GetSystemTime(false));

    // init SRC
    if (mStreamAttributeSource->sample_rate  != mStreamAttributeTarget->sample_rate  ||
        mStreamAttributeSource->num_channels != mStreamAttributeTarget->num_channels ||
        mStreamAttributeSource->audio_format != mStreamAttributeTarget->audio_format)
    {
        ALOGD("sample_rate: %d => %d, num_channels: %d => %d, audio_format: 0x%x => 0x%x",
              mStreamAttributeSource->sample_rate, mStreamAttributeTarget->sample_rate,
              mStreamAttributeSource->num_channels, mStreamAttributeTarget->num_channels,
              mStreamAttributeSource->audio_format, mStreamAttributeTarget->audio_format);

        mBliSrc = new MtkAudioSrc(
            mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels,
            mStreamAttributeTarget->sample_rate, mStreamAttributeTarget->num_channels,
            SRC_IN_Q1P15_OUT_Q1P15); // TODO(Harvey, Ship): 24bit
        mBliSrc->Open();
    }
}

AudioALSACaptureDataClient::~AudioALSACaptureDataClient()
{
    ALOGD("%s()", __FUNCTION__);

    //EchoRef+++
    if (mCaptureDataProviderEchoRef != NULL)
    {
        ALOGD("%s(), remove EchoRef data provider,mCaptureDataProviderEchoRef=%p", __FUNCTION__, mCaptureDataProviderEchoRef);
        mSPELayer->SetOutputStreamRunning(false, true);
        mCaptureDataProviderEchoRef->detach(this);
        if (mEchoRefRawDataBuf.pBufBase != NULL) { delete[] mEchoRefRawDataBuf.pBufBase; }

        if (mEchoRefSrcDataBuf.pBufBase != NULL) { delete[] mEchoRefSrcDataBuf.pBufBase; }
    }

    if (mBliSrcEchoRef != NULL)
    {
        mBliSrcEchoRef->Close();
        delete mBliSrcEchoRef;
        mBliSrcEchoRef = NULL;
    }

    if (mBliSrcEchoRefBesRecord != NULL)
    {
        mBliSrcEchoRefBesRecord->Close();
        delete mBliSrcEchoRefBesRecord;
        mBliSrcEchoRefBesRecord = NULL;
    }
    //EchoRef---

    mCaptureDataProvider->detach(this);

    if (mRawDataBuf.pBufBase != NULL) { delete[] mRawDataBuf.pBufBase; }

    if (mSrcDataBuf.pBufBase != NULL) { delete[] mSrcDataBuf.pBufBase; }

    if (mProcessedDataBuf.pBufBase != NULL) { delete[] mProcessedDataBuf.pBufBase; }

    if (mBliSrc != NULL)
    {
        mBliSrc->Close();
        delete mBliSrc;
        mBliSrc = NULL;
    }

    //TODO: Sam, add here for temp
    //BesRecord+++
    StopBesRecord();
    if (mBliSrcHandler1)
    {
        mBliSrcHandler1->Close();
        delete mBliSrcHandler1;
        mBliSrcHandler1 = NULL;
    }

    if (mBliSrcHandler2)
    {
        mBliSrcHandler2->Close();
        delete mBliSrcHandler2;
        mBliSrcHandler2 = NULL;
    }
    if (mSPELayer != NULL) { delete mSPELayer; }
    //BesRecord---

    //Android Native Preprocess effect +++
    if (mAudioPreProcessEffect != NULL) { delete mAudioPreProcessEffect; }
    //Android Native Preprocess effect ---

    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataClient::setIdentity(const uint32_t identity)
{
    ALOGD("%s(), mIsIdentitySet=%d, identity=%d", __FUNCTION__, mIsIdentitySet, identity);
    if (!mIsIdentitySet)
    {
        mIdentity = identity;
        mIsIdentitySet = true;
    }
}

uint32_t AudioALSACaptureDataClient::copyCaptureDataToClient(RingBuf pcm_read_buf)
{
    ALOGV("+%s()", __FUNCTION__);

    mLock.lock();

    uint32_t freeSpace = RingBuf_getFreeSpace(&mRawDataBuf);
    uint32_t dataSize = RingBuf_getDataCount(&pcm_read_buf);
    if (freeSpace < dataSize)
    {
        ALOGE("%s(), mRawDataBuf <= pcm_read_buf, freeSpace(%u) < dataSize(%u), buffer overflow!!", __FUNCTION__, freeSpace, dataSize);
        RingBuf_copyFromRingBuf(&mRawDataBuf, &pcm_read_buf, freeSpace);
    }
    else
    {
        RingBuf_copyFromRingBuf(&mRawDataBuf, &pcm_read_buf, dataSize);
    }

    // SRC
    const uint32_t kNumRawData = RingBuf_getDataCount(&mRawDataBuf);
    uint32_t num_free_space = RingBuf_getFreeSpace(&mSrcDataBuf);

    if (mBliSrc == NULL) // No need SRC
    {
        //ASSERT(num_free_space >= kNumRawData);
        if (num_free_space < kNumRawData)
        {
            ALOGW("%s(), num_free_space(%u) < kNumRawData(%u)", __FUNCTION__, num_free_space, kNumRawData);
            RingBuf_copyFromRingBuf(&mSrcDataBuf, &mRawDataBuf, num_free_space);
        }
        else
        {
            RingBuf_copyFromRingBuf(&mSrcDataBuf, &mRawDataBuf, kNumRawData);
        }
    }
    else // Need SRC
    {
        char *pRawDataLinearBuf = new char[kNumRawData];
        RingBuf_copyToLinear(pRawDataLinearBuf, &mRawDataBuf, kNumRawData);

        char *pSrcDataLinearBuf = new char[num_free_space];

        char *p_read = pRawDataLinearBuf;
        uint32_t num_raw_data_left = kNumRawData;
        uint32_t num_converted_data = num_free_space; // max convert num_free_space

        uint32_t consumed = num_raw_data_left;
        mBliSrc->Process((int16_t *)p_read, &num_raw_data_left,
                         (int16_t *)pSrcDataLinearBuf, &num_converted_data);
        consumed -= num_raw_data_left;

        p_read += consumed;
        ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);

        //ASSERT(num_raw_data_left == 0);
        if (num_raw_data_left > 0)
        {
            ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
        }

        RingBuf_copyFromLinear(&mSrcDataBuf, pSrcDataLinearBuf, num_converted_data);
        ALOGV("%s(), pRead:%u, pWrite:%u, dataCount:%u", __FUNCTION__,
              mSrcDataBuf.pRead - mSrcDataBuf.pBufBase,
              mSrcDataBuf.pWrite - mSrcDataBuf.pBufBase,
              RingBuf_getDataCount(&mSrcDataBuf));

        delete[] pRawDataLinearBuf;
        delete[] pSrcDataLinearBuf;
    }

    // TODO(Harvey, Sam): Do AP side enhancement here
    freeSpace = RingBuf_getFreeSpace(&mProcessedDataBuf);
    dataSize = RingBuf_getDataCount(&mSrcDataBuf);
    uint32_t ProcesseddataSize = dataSize;
    //PreProcess effect, BesRecord + native effect
    if (((mStreamAttributeTarget->BesRecord_Info.besrecord_enable) && !mBypassBesRecord) ||
        (mAudioPreProcessEffect->num_preprocessors > 0))
    {
        char *pSrcDataLinearBuf = new char[dataSize];
        uint32_t native_processed_byte = 0;
        RingBuf_copyToLinear(pSrcDataLinearBuf, &mSrcDataBuf, dataSize);
        //BesRecord+++
        if ((mStreamAttributeTarget->BesRecord_Info.besrecord_enable) && !mBypassBesRecord) //need to do BesRecord process
        {
            ProcesseddataSize = BesRecordPreprocess(pSrcDataLinearBuf, dataSize);
        }
        //BesRecord---

        //Native Preprocess effect+++
        if ((mAudioPreProcessEffect->num_preprocessors > 0) && (IsVoIPEnable() == false))
        {
            native_processed_byte = NativePreprocess(pSrcDataLinearBuf, ProcesseddataSize);

            if (freeSpace < native_processed_byte)
            {
                ALOGE("%s(), NativeProcess mProcessedDataBuf <= mSrcDataBuf, freeSpace(%u) < native_processed size(%u), buffer overflow!!", __FUNCTION__, native_processed_byte, dataSize);
                RingBuf_copyFromLinear(&mProcessedDataBuf, pSrcDataLinearBuf, freeSpace);
            }
            else
            {
                RingBuf_copyFromLinear(&mProcessedDataBuf, pSrcDataLinearBuf, native_processed_byte);
            }

        }
        //Native Preprocess effect---
        else
        {
            if (freeSpace < ProcesseddataSize)
            {
                ALOGE("%s(), BesRecord mProcessedDataBuf <= mSrcDataBuf, freeSpace(%u) < ProcesseddataSize(%u), buffer overflow!!", __FUNCTION__, freeSpace, ProcesseddataSize);
                RingBuf_copyFromLinear(&mProcessedDataBuf, pSrcDataLinearBuf, freeSpace);
            }
            else
            {
                RingBuf_copyFromLinear(&mProcessedDataBuf, pSrcDataLinearBuf, ProcesseddataSize);
            }
        }

        delete[] pSrcDataLinearBuf;
    }
    else
    {
        if (freeSpace < dataSize)
        {
            ALOGE("%s(), mProcessedDataBuf <= mSrcDataBuf, freeSpace(%u) < dataSize(%u), buffer overflow!!", __FUNCTION__, freeSpace, dataSize);
            RingBuf_copyFromRingBuf(&mProcessedDataBuf, &mSrcDataBuf, freeSpace);
        }
        else
        {
            RingBuf_copyFromRingBuf(&mProcessedDataBuf, &mSrcDataBuf, dataSize);
        }
    }

    mWaitWorkCV.signal();
    mLock.unlock();

    ALOGV("-%s()", __FUNCTION__);
    return 0;
}

ssize_t AudioALSACaptureDataClient::read(void *buffer, ssize_t bytes)
{
    ALOGV("+%s()", __FUNCTION__);

    char *pWrite = (char *)buffer;
    char *pStart = (char *)buffer;
    uint32_t RingBufferSize = 0;
    uint32_t ReadDataBytes = bytes;

    int TryCount = 20;

    do
    {
        mLock.lock();

        CheckNativeEffect();    //add here for alsaStreamIn lock holding
        CheckDynamicSpeechMask();

        RingBufferSize = RingBuf_getDataCount(&mProcessedDataBuf);
        if (RingBufferSize >= ReadDataBytes) // ring buffer is enough, copy & exit
        {
            RingBuf_copyToLinear((char *)pWrite, &mProcessedDataBuf, ReadDataBytes);
            ReadDataBytes = 0;
            mLock.unlock();
            break;
        }
        else // ring buffer is not enough, copy all data
        {
            RingBuf_copyToLinear((char *)pWrite, &mProcessedDataBuf, RingBufferSize);
            ReadDataBytes -= RingBufferSize;
            pWrite += RingBufferSize;
        }

        // wait for new data
        if (mWaitWorkCV.waitRelative(mLock, milliseconds(300)) != NO_ERROR)
        {
            ALOGW("%s(), waitRelative fail", __FUNCTION__);
            mLock.unlock();
            break;
        }

        mLock.unlock();
        TryCount--;
    }
    while (ReadDataBytes > 0 && TryCount);

    ApplyVolume(buffer, bytes);
    CheckNeedDataConvert((short *)buffer, bytes);

    ALOGV("-%s()", __FUNCTION__);
    return bytes - ReadDataBytes;
}

status_t AudioALSACaptureDataClient::ApplyVolume(void *Buffer , uint32_t BufferSize)
{
    // check if need apply mute
    if (mMicMute != mStreamAttributeTarget->micmute)
    {
        mMicMute =  mStreamAttributeTarget->micmute ;
        mMuteTransition = false;
    }

    if (mMicMute == true)
    {
        // do ramp down
        if (mMuteTransition == false)
        {
            uint32_t count = BufferSize >> 1;
            float Volume_inverse = (float)(MTK_STREAMIN_VOLUEM_MAX / count) * -1;
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count)
            {
                value = *pPcm * (MTK_STREAMIN_VOLUEM_MAX + (Volume_inverse * ConsumeSample));
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
        else
        {
            memset(Buffer, 0, BufferSize);
        }
    }
    else if (mMicMute == false)
    {
        // do ramp up
        if (mMuteTransition == false)
        {
            uint32_t count = BufferSize >> 1;
            float Volume_inverse = (float)(MTK_STREAMIN_VOLUEM_MAX / count);
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count)
            {
                value = *pPcm * (Volume_inverse * ConsumeSample);
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
    }
    return NO_ERROR;
}

void AudioALSACaptureDataClient::CheckNeedDataConvert(short *buffer, ssize_t bytes)
{
    //Stereo/mono convert
    const uint8_t num_channel = mStreamAttributeTarget->num_channels;

    //only need convert when BesRecord is enabled
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_enable == true)
    {
        //need return stereo data
        if (num_channel == 2)
        {
            if (!mBesRecordStereoMode) //speech enhancement output data is mono, need to convert to stereo
            {
                short left;
                int copysize = bytes >> 2;

                while (copysize)    //only left channel data is processed
                {
                    left = *(buffer);
                    *(buffer) = left;
                    *(buffer + 1) = left;
                    buffer += 2;
                    copysize--;
                }
            }
        }
    }
}

//TODO: Move here for temp solution, need to move to DataProcess
//BesRecord+++

void AudioALSACaptureDataClient::LoadBesRecordParams(void)
{
    uint8_t total_num_scenes = MAX_HD_REC_SCENES;
    ALOGD("+%s()", __FUNCTION__);
    // get scene table
    if (GetHdRecordSceneTableFromNV(&mBesRecordSceneTable) == 0)
    {
        ALOGD("GetHdRecordSceneTableFromNV fail, use default value");
        memcpy(&mBesRecordSceneTable, &Hd_Recrod_Scene_Table_default, sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
    }

    // get hd rec param
    if (GetHdRecordParamFromNV(&mBesRecordParam) == 0)
    {
        ALOGD("GetHdRecordParamFromNV fail, use default value");
        memcpy(&mBesRecordParam, &Hd_Recrod_Par_default, sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
    }

    //get VoIP param
    if (GetAudioVoIPParamFromNV(&mVOIPParam) == 0)
    {
        ALOGD("GetAudioVoIPParamFromNV fail, use default value");
        memcpy(&mVOIPParam, &Audio_VOIP_Par_default, sizeof(AUDIO_VOIP_PARAM_STRUCT));
    }

    //get DMNR param
    if ((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0)
    {
        if (GetDualMicSpeechParamFromNVRam(&mDMNRParam) == 0)
        {
            ALOGD("GetDualMicSpeechParamFromNVRam fail, use default value");
            memcpy(&mDMNRParam, &dual_mic_custom_default, sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        }
    }

#if 1 // Debug print
    for (int i = 0; i < total_num_scenes; i++)
        for (int j = 0; j < NUM_HD_REC_DEVICE_SOURCE; j++)
        {
            ALOGD("scene_table[%d][%d] = %d", i, j, mBesRecordSceneTable.scene_table[i][j]);
        }
#endif
    ALOGD("-%s()", __FUNCTION__);
}

int AudioALSACaptureDataClient::SetCaptureGain(void)
{

    if (mAudioALSAVolumeController != NULL)
    {
        mAudioALSAVolumeController->SetCaptureGain(mStreamAttributeTarget->audio_mode, mStreamAttributeTarget->input_source,
                                                   mStreamAttributeTarget->input_device, mStreamAttributeTarget->output_devices);
    }
    return 0;
}

int AudioALSACaptureDataClient::CheckBesRecordMode(void)
{
    //check the BesRecord mode and scene
    ALOGD("+%s()", __FUNCTION__);
    uint8_t modeIndex = 0;
    int32_t u4SceneIdx = 0;
    bool RecordModeGet = false;

    if (IsVoIPEnable() == true)
    {
        mSpeechProcessMode = SPE_MODE_VOIP;
    }
    else if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)  //MagiASR need AEC
             || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2))   //Normal Record + AEC
    {
        mSpeechProcessMode = SPE_MODE_AECREC;
    }
    else
    {
        mSpeechProcessMode = SPE_MODE_REC;
    }

    //u4SceneIdx = mAudioSpeechEnhanceInfoInstance->GetBesRecScene();
    u4SceneIdx = mStreamAttributeTarget->BesRecord_Info.besrecord_scene;

    //TODO:Sam
    //mAudioSpeechEnhanceInfoInstance->ResetBesRecScene();
    mBesRecordStereoMode = false;


    //special input source case
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) || mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K)
    {
        ALOGD("voice recognition case");
        u4SceneIdx = VOICE_REC_RECORDING_DEFAULT_MODE;
    }
    else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_UNLOCK)
    {
        ALOGD("voice unlock case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE;
    }
    else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
    {
        ALOGD("CUSTOMIZATION1 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 1;
    }
    else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
    {
        ALOGD("CUSTOMIZATION2 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 2;
    }
    else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION3)
    {
        ALOGD("CUSTOMIZATION3 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 3;
    }

    //for BT record case, use specific params, whether what input source it is.
    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        ALOGD("%s, is BT device", __FUNCTION__);
        if (mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_BT_EARPHONE] != 0xFF)
        {
            modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_BT_EARPHONE];
            ALOGD("%s, get specific BT modeIndex = %d", __FUNCTION__, modeIndex);
        }
        else
        {
            modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
        }
        mBesRecordSceneIndex = NORMAL_RECORDING_DEFAULT_MODE;
        RecordModeGet = true;
    }
    //Get the BesRecord Mode from the Record Scene
    else if ((u4SceneIdx >= 0) && (u4SceneIdx < MAX_HD_REC_SCENES))
    {
        // get mode index
        if ((mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET] != 0xFF)
            && (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET))
        {
            modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
            ALOGD("%s, is HEADSET device, u4SceneIdx=%d, modeIndex=%d", __FUNCTION__, u4SceneIdx, modeIndex);
        }
        // Handset Mic
        else if (mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET] != 0xFF)
        {
            modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
            ALOGD("%s, is HANDSET device, u4SceneIdx=%d, modeIndex=%d", __FUNCTION__, u4SceneIdx, modeIndex);
            //#if defined(MTK_DUAL_MIC_SUPPORT)
            /* only stereo flag is true, the stereo record preprocess is enabled */
            if ((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0)
            {
                if (mBesRecordParam.hd_rec_map_to_stereo_flag[modeIndex] != 0)
                {
                    mBesRecordStereoMode = true;
                }
            }
            //#endif
        }
        else
        {
            ALOGD("%s, Handset mode index shoule not be -1, u4SceneIdx=%d, modeIndex=%d", __FUNCTION__, u4SceneIdx, modeIndex);
        }

        // Debug print
        ALOGD("GetBesRecordModeInfo: map_fir_ch1=%d, map_fir_ch2=%d",
              mBesRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex],
              mBesRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex]);

        mBesRecordSceneIndex = u4SceneIdx;
        RecordModeGet = true;
    }

    //No correct mode get, use default mode parameters
    if (RecordModeGet == false)
    {
        //ALOGD("%s, use default mode mdevices=%x, mAttribute.mPredevices=%x, mHDRecordSceneIndex = %d ", __FUNCTION__,
        //    mAttribute.mdevices, mAttribute.mPredevices, mHDRecordSceneIndex);
        ALOGD("%s, use default mode input_device=%x, mBesRecordSceneIndex = %d ", __FUNCTION__, mStreamAttributeTarget->input_device, mBesRecordSceneIndex);

        //can not get match HD record mode, use the default one
        // check if 3rd party camcorder
        if (mStreamAttributeTarget->input_source != AUDIO_SOURCE_CAMCORDER) //not camcorder
        {
#if 0   //TODO:Sam, if Capture handler is reopen when routing, no needed.
            if (mAttribute.mdevices != mAttribute.mPredevices)  //device changed, use previous scene (since scene not changed), (headset plug in/out during recording case)
            {
                if (mHDRecordSceneIndex == -1)
                {
                    mHDRecordSceneIndex = NORMAL_RECORDING_DEFAULT_MODE;
                }
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
            else
#endif
            {
                if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else    //default use internal one
                {
                    modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
                }
                mBesRecordSceneIndex  = NORMAL_RECORDING_DEFAULT_MODE;
            }
        }
        else  //camcoder
        {
            u4SceneIdx = mBesRecordSceneTable.num_voice_rec_scenes + NORMAL_RECORDING_DEFAULT_MODE;//1:cts verifier offset
#if 0 //TODO:Sam, if Capture handler is reopen when routing, no needed.
            if (mAttribute.mdevices != mAttribute.mPredevices)  //device changed, use previous scene
            {
                if (mHDRecordSceneIndex == -1)
                {
                    mHDRecordSceneIndex = u4SceneIdx;
                }
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
            else
#endif
            {
                if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else    //default use internal one
                {
                    modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
                }
                mBesRecordSceneIndex  = u4SceneIdx;
            }
        }

        //#if defined(MTK_DUAL_MIC_SUPPORT)
        //also need to configure the channel when use default mode
        /* only stereo flag is true, the stereo record is enabled */
        if ((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0)
        {
            if (mStreamAttributeTarget->input_device ==  AUDIO_DEVICE_IN_BUILTIN_MIC) //handset
            {
                if (mBesRecordParam.hd_rec_map_to_stereo_flag[modeIndex] != 0)
                {
                    mBesRecordStereoMode = true;
                }
            }
        }
        //#endif
    }

    ALOGD("-%s(), mBesRecordSceneIndex=%d, modeIndex=%d", __FUNCTION__, mBesRecordSceneIndex, modeIndex);
    mBesRecordModeIndex = modeIndex;
    return modeIndex;
}


void AudioALSACaptureDataClient::ConfigBesRecordParams(void)
{
    ALOGD("+%s()", __FUNCTION__);

    unsigned long BesRecordEnhanceParas[EnhanceParasNum] = {0};
    short BesRecordCompenFilter[CompenFilterNum] = {0};
    short BesRecordDMNRParam[DMNRCalDataNum] = {0};

    bool bVoIPEnable = IsVoIPEnable();
    int RoutePath = GetBesRecordRoutePath();
    SPE_MODE mode = mSpeechProcessMode;

    ALOGD("%s(),mBesRecordStereoMode=%d, input_source= %d, input_devices=%x,mBesRecordModeIndex=%d, bVoIPEnable=%d, mode=%d, bypassDualProcess=%d", __FUNCTION__, mBesRecordStereoMode, mStreamAttributeTarget->input_source,
          mStreamAttributeTarget->input_device, mBesRecordModeIndex, bVoIPEnable, mode,mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess);

    //set speech parameters+++
    for (int i = 0; i < EnhanceParasNum; i++) //EnhanceParasNum = 16+12(common parameters)
    {
        if (i < SPEECH_PARA_NUM)
        {
            if (mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable == true)
            {
                //specific parameters
                BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_NORMAL][i];     //use loud speaker mode speech params
            }
            else if (mode == SPE_MODE_VOIP)
            {
                //specific parameters
                if (RoutePath == ROUTE_BT)
                {
                    BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_BT][i];
                }
                else if (RoutePath == ROUTE_HEADSET)
                {
                    BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_HEADSET][i];
                }
                else if (RoutePath == ROUTE_SPEAKER)
                {
                    BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_SPEAKER][i];
                }
                else    //normal receiver case
                {
                    BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_NORMAL][i];
                }
            }
            else
            {
                BesRecordEnhanceParas[i] = mBesRecordParam.hd_rec_speech_mode_para[mBesRecordModeIndex][i];
            }
        }
        else
        {
            //common parameters also use VoIP's
            BesRecordEnhanceParas[i] = mVOIPParam.speech_common_para[i - SPEECH_PARA_NUM];
        }
        ALOGV("BesRecordEnhanceParas[%d]=%ld", i, BesRecordEnhanceParas[i]);
    }

    mSPELayer->SetEnhPara(mode, BesRecordEnhanceParas);
    //speech parameters---

    //FIR parameters+++
    for (int i = 0; i < WB_FIR_NUM; i++)
    {
        if (mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable == true)
        {
            BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];
            //ALOGD("BesRecordCompenFilter[%d]=%d", i, BesRecordCompenFilter[i]);
            BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];
        }
        else if (mode == SPE_MODE_VOIP)
        {
            if (RoutePath == ROUTE_BT)
            {
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_BT][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_BT][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_BT][i]; //DL params
            }
            else if (RoutePath == ROUTE_HEADSET)
            {
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_HEADSET][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_HEADSET][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_HEADSET][i]; //DL params
            }
            else if (RoutePath == ROUTE_SPEAKER)
            {
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_SPEAKER][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_SPEAKER][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_SPEAKER][i]; //DL params
            }
            else    //normal receiver case
            {
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_NORMAL][i]; //DL params
            }
        }
        else
        {
            BesRecordCompenFilter[i] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch1[mBesRecordModeIndex]][i];
            //ALOGD("BesRecordCompenFilter[%d]=%d", i, BesRecordCompenFilter[i]);
            if (mBesRecordStereoMode) //stereo, UL2 use different FIR filter
            {
                BesRecordCompenFilter[i + WB_FIR_NUM] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch2[mBesRecordModeIndex]][i];
            }
            else    //mono, UL2 use the same FIR filter
            {
                BesRecordCompenFilter[i + WB_FIR_NUM] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch1[mBesRecordModeIndex]][i];
            }
        }
    }

    mSPELayer->SetCompFilter(mode, BesRecordCompenFilter);
    //FIR parameters---

    //DMNR parameters+++
    if (((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) && (mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess == false))
    {
        //DMNR parameters
        //google default input source AUDIO_SOURCE_VOICE_RECOGNITION not using DMNR (on/off by parameters)
        if (((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
             || mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K || mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable)
            && ((QueryFeatureSupportInfo()& SUPPORT_ASR) > 0))
        {
            for (int i = 0; i < NUM_ABFWB_PARAM; i++)
            {
                BesRecordDMNRParam[i] = mDMNRParam.ABF_para_VR[i];
            }
        }
        else if (mode == SPE_MODE_VOIP) //VoIP case
        {
            //receiver path
            if ((RoutePath == ROUTE_NORMAL) && ((QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0)
                && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR))
            {
                //enable corresponding DMNR flag
                for (int i = 0; i < NUM_ABFWB_PARAM; i++)
                {
                    BesRecordDMNRParam[i] = mDMNRParam.ABF_para_VOIP[i];
                }
                SetDMNREnable(DMNR_NORMAL, true);
            }
            //speaker path
            else if ((RoutePath == ROUTE_SPEAKER) && ((QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)
                     && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR))
            {
                for (int i = 0; i < NUM_ABFWB_PARAM; i++)
                {
                    BesRecordDMNRParam[i] = mDMNRParam.ABF_para_VOIP_LoudSPK[i];
                }
                SetDMNREnable(DMNR_HANDSFREE, true);
            }
            else
            {
                memset(BesRecordDMNRParam, 0, sizeof(BesRecordDMNRParam));
                SetDMNREnable(DMNR_DISABLE, false);
            }
        }
        else
        {
            memset(BesRecordDMNRParam, 0, sizeof(BesRecordDMNRParam));
        }

        mSPELayer->SetDMNRPara(mode, BesRecordDMNRParam);
    }
    else
    {
        memset(BesRecordDMNRParam, 0, sizeof(BesRecordDMNRParam));
        mSPELayer->SetDMNRPara(mode, BesRecordDMNRParam);
        SetDMNREnable(DMNR_DISABLE, false);
    }
    //DMNR parameters---

    //need to config as 16k sample rate for voice recognition or VoIP or REC+AEC
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
        || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
        || (mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K == true) || (IsVoIPEnable() == true))
    {
        if (mode == SPE_MODE_VOIP) //VoIP case
        {
            mSPELayer->SetSampleRate(mode, VOICE_RECOGNITION_RECORD_SAMPLE_RATE);
            mSPELayer->SetAPPTable(mode, WB_VOIP);
        }
        else    //voice recognition case
        {
            mSPELayer->SetSampleRate(mode, VOICE_RECOGNITION_RECORD_SAMPLE_RATE);
            if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
            {
                mSPELayer->SetAPPTable(mode, MONO_AEC_RECORD);   //set library do AEC Record
            }
            else
            {
                mSPELayer->SetAPPTable(mode, SPEECH_RECOGNITION);   //set library do voice recognition process or MagiASR
            }
        }
    }
    else    //normal record  use 48k
    {
        mSPELayer->SetSampleRate(mode, HD_RECORD_SAMPLE_RATE);
        if (mBesRecordStereoMode)
        {
            mSPELayer->SetAPPTable(mode, STEREO_RECORD);    //set library do stereo process
        }
        else
        {
            mSPELayer->SetAPPTable(mode, MONO_RECORD);    //set library do mono process
        }
    }

    mSPELayer->SetRoute((SPE_ROUTE)RoutePath);

    //set MIC digital gain to library
    long gain = mAudioALSAVolumeController->GetSWMICGain();
    uint8_t TotalGain = mAudioALSAVolumeController->GetULTotalGain();
    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        gain = 0;
        TotalGain = 0;
        ALOGD("BT path set Digital MIC gain = 0");
    }
    mSPELayer->SetMICDigitalGain(mode, gain);
    mSPELayer->SetUpLinkTotalGain(mode, TotalGain);

    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataClient::StartBesRecord(void)
{
    ALOGD("+%s()", __FUNCTION__);
    mSPELayer->Start(mSpeechProcessMode);
    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataClient::StopBesRecord(void)
{
    ALOGD("+%s()", __FUNCTION__);
    mSPELayer->Stop();
    ALOGD("-%s()", __FUNCTION__);
}

uint32_t AudioALSACaptureDataClient::BesRecordPreprocess(void *buffer , uint32_t bytes)
{
    struct InBufferInfo InBufinfo = {0};
    uint32_t retSize = bytes;

    //ALOGD("+%s()", __FUNCTION__);
    if (mBypassBesRecord)
    {
        //not do the process if need bypass
    }
    else
    {
        if (mNeedBesRecordSRC && mBliSrcHandler1 != 0 && mBliSrcHandler2 != 0)
        {
            //ALOGD("BesRecordPreprocess buffer=%p, bytes=%d", buffer, bytes);
            uint32_t consume = 0;
            size_t inputLength = bytes;
            size_t outputLength = bytes * mBesRecSRCSizeFactor;
            size_t processedLength = bytes;
            char *pPreProcessDataLinearBuf = new char[outputLength];
            //convert to BesRecord preprocess needed sample rate
            consume = inputLength;
            //ALOGD("%s, before inputLength=%d,outputLength=%d,consume=%d", __FUNCTION__,inputLength,outputLength,consume);
            mBliSrcHandler1->Process((int16_t *)buffer, &inputLength, (int16_t *)pPreProcessDataLinearBuf, &outputLength);
            consume -= inputLength;

            //ALOGD("%s, inputLength=%d,outputLength=%d,consume=%d", __FUNCTION__,inputLength,outputLength,consume);

            InBufinfo.pBufBase = (short *)pPreProcessDataLinearBuf;
            InBufinfo.BufLen = outputLength;
            InBufinfo.time_stamp_queued = GetSystemTime(false);
            InBufinfo.bHasRemainInfo = true;
            InBufinfo.time_stamp_predict = GetCaptureTimeStamp();

            processedLength = mSPELayer->Process(&InBufinfo);

            //convert back to streamin needed sample rate
            //ALOGD("%s, processedLength=%d", __FUNCTION__,processedLength);
            consume = processedLength;
            mBliSrcHandler2->Process((int16_t *)pPreProcessDataLinearBuf, &processedLength, (int16_t *)buffer, &retSize);
            consume -= processedLength;
            delete[] pPreProcessDataLinearBuf;
        }
        else
        {
            InBufinfo.pBufBase = (short *)buffer;
            InBufinfo.BufLen = bytes;
            InBufinfo.time_stamp_queued = GetSystemTime(false);
            //InBufinfo.bHasRemainInfo = false;
            InBufinfo.bHasRemainInfo = true;
            InBufinfo.time_stamp_predict = GetCaptureTimeStamp();

            retSize = mSPELayer->Process(&InBufinfo);
        }
    }
    //ALOGD("-%s(), bytes=%d, retSize=%d", __FUNCTION__, bytes, retSize);

    return retSize;
}

int AudioALSACaptureDataClient::GetBesRecordRoutePath(void)
{
    int RoutePath;
    ALOGD("+%s(), %x", __FUNCTION__, mStreamAttributeTarget->output_devices);

    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        RoutePath = ROUTE_BT;
    }
    else if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET)
    {
        RoutePath = ROUTE_HEADSET;
    }
    else if (mStreamAttributeTarget->output_devices & AUDIO_DEVICE_OUT_SPEAKER)  //speaker path
    {
        RoutePath = ROUTE_SPEAKER;
    }
    else
    {
        RoutePath = ROUTE_NORMAL;
    }
    return RoutePath;
}


bool AudioALSACaptureDataClient::CheckBesRecordBypass()
{
#if 0   //these input sources will not enable BesRecord while capture handle create (BesRecord_Info.besrecord_enable), keep this function for other purpose in the future
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_UNLOCK) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_FM) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_MATV) ||
        (mStreamAttributeTarget->input_source == AUDIO_SOURCE_ANC))
    {
        mBypassBesRecord = true;
    }
    else
    {
        mBypassBesRecord = false;
    }
#endif
    ALOGD("%s() %d", __FUNCTION__, mBypassBesRecord);
    return mBypassBesRecord;
}

bool AudioALSACaptureDataClient::CheckNeedBesRecordSRC()
{
    uint32_t BesRecord_usingsamplerate = HD_RECORD_SAMPLE_RATE;
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_enable == true)
    {
        //BesRecord need 16K sample rate data
        if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
            || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
            || (mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K == true) || (IsVoIPEnable() == true))
        {
            //need src if the stream target sample rate are not the same with BesRecord needed
            if ((mStreamAttributeTarget->sample_rate  != VOICE_RECOGNITION_RECORD_SAMPLE_RATE) || (mStreamAttributeTarget->num_channels != 2))
            {
                mNeedBesRecordSRC = true;
                BesRecord_usingsamplerate = VOICE_RECOGNITION_RECORD_SAMPLE_RATE;
            }
            else
            {
                mNeedBesRecordSRC = false;
            }
        }
        else    //BesRecord need 48K sample rate data
        {
            //need src if the stream target sample rate are not the same with BesRecord needed
            if ((mStreamAttributeTarget->sample_rate  != HD_RECORD_SAMPLE_RATE)  || (mStreamAttributeTarget->num_channels != 2))
            {
                mNeedBesRecordSRC = true;
                BesRecord_usingsamplerate = HD_RECORD_SAMPLE_RATE;
            }
            else
            {
                mNeedBesRecordSRC = false;
            }
        }

        //if need to do BesRecord SRC
        if (mNeedBesRecordSRC)
        {
            // Need SRC from stream target to BesRecord needed
            mBliSrcHandler1 = new MtkAudioSrc(mStreamAttributeTarget->sample_rate, mStreamAttributeTarget->num_channels,
                                              BesRecord_usingsamplerate, 2, SRC_IN_Q1P15_OUT_Q1P15);
            mBliSrcHandler1->Open();

            mBesRecSRCSizeFactor = ((BesRecord_usingsamplerate * 2) / (mStreamAttributeTarget->sample_rate * mStreamAttributeTarget->num_channels)) + 1;

            // Need SRC from BesRecord to stream target needed
            mBliSrcHandler2 = new MtkAudioSrc(BesRecord_usingsamplerate, 2,
                                              mStreamAttributeTarget->sample_rate, mStreamAttributeTarget->num_channels, SRC_IN_Q1P15_OUT_Q1P15);
            mBliSrcHandler2->Open();
        }
    }
    else
    {
        mNeedBesRecordSRC = false;
    }
    ALOGD("%s(), %d, %d, mBesRecSRCSizeFactor=%d", __FUNCTION__, mNeedBesRecordSRC, BesRecord_usingsamplerate, mBesRecSRCSizeFactor);
    return mNeedBesRecordSRC;
}

bool AudioALSACaptureDataClient::IsVoIPEnable(void)
{
    //ALOGV("%s() %d", __FUNCTION__, mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable);
    return mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable;
}

status_t AudioALSACaptureDataClient::UpdateBesRecParam()
{
    ALOGD("+%s() besrecord_voip_enable %d, besrecord_enable=%d", __FUNCTION__,
          mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable, mStreamAttributeTarget->BesRecord_Info.besrecord_enable);
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable && mStreamAttributeTarget->BesRecord_Info.besrecord_enable)
    {
        if (mSPELayer->IsSPERunning())
        {
            StopBesRecord();
            ConfigBesRecordParams();
            mSPELayer->Standby();   //for doing the time resync
            StartBesRecord();
        }
        else
        {
            ConfigBesRecordParams();
        }
    }
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void AudioALSACaptureDataClient::CheckDynamicSpeechMask(void)
{
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask.dynamic_func != mVoIPSpeechEnhancementMask.dynamic_func)    //need update dynamic mask
    {
        UpdateDynamicFunction();
        mVoIPSpeechEnhancementMask = mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask;
    }
}

void AudioALSACaptureDataClient::UpdateDynamicFunction(void)
{
    ALOGD("+%s()", __FUNCTION__);
    int RoutePath = GetBesRecordRoutePath();
    SPE_MODE mode = mSpeechProcessMode;
    short DMNRParam[DMNRCalDataNum] = {0};
    ALOGD("%s(), RoutePath %d, mode %d", __FUNCTION__, RoutePath, mode);
    //DMNR function update
    if ((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0)
    {
        if ((QueryFeatureSupportInfo()& SUPPORT_DMNR_3_0) > 0)
        {

            if (mode == SPE_MODE_VOIP)
            {
                //receiver path & receiver DMNR is enabled
                if ((RoutePath == ROUTE_NORMAL) && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) &&
                    ((QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0))
                {
                    ALOGD("enable normal mode DMNR");
                    //enable corresponding DMNR flag
                    for (int i = 0; i < NUM_ABFWB_PARAM; i++)
                    {
                        DMNRParam[i] = mDMNRParam.ABF_para_VOIP[i];
                    }
                    SetDMNREnable(DMNR_NORMAL, true);
                }
                else if ((RoutePath == ROUTE_SPEAKER) && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) &&
                         ((QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)) //speaker path
                {
                    ALOGD("enable loudspeaker mode DMNR");
                    for (int i = 0; i < NUM_ABFWB_PARAM; i++)
                    {
                        DMNRParam[i] = mDMNRParam.ABF_para_VOIP_LoudSPK[i];
                    }
                    SetDMNREnable(DMNR_HANDSFREE, true);
                }
                else
                {
                    ALOGD("disable DMNR");
                    memset(DMNRParam, 0, sizeof(DMNRParam));
                    SetDMNREnable(DMNR_DISABLE, false);
                }
                mSPELayer->SetDMNRPara(mode, DMNRParam);
            }
        }
        else
        {
            ALOGD("%s(),disable DMNR due to not support", __FUNCTION__);
            memset(DMNRParam, 0, sizeof(DMNRParam));
            SetDMNREnable(DMNR_DISABLE, false);
            mSPELayer->SetDMNRPara(mode, DMNRParam);
        }
    }
    ALOGD("-%s()", __FUNCTION__);
}

bool AudioALSACaptureDataClient::CheckDynamicSpeechEnhancementMaskOnOff(const voip_sph_enh_dynamic_mask_t dynamic_mask_type)
{
    ALOGV("%s() %x, %x", __FUNCTION__, mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask.dynamic_func, dynamic_mask_type);
    bool bret = false;

    if ((mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask.dynamic_func & dynamic_mask_type) > 0)
    {
        bret = true;
    }

    return bret;
}

//0: disable DMNR
//1: normal mode DMNR
//2: handsfree mode DMNR
void AudioALSACaptureDataClient::SetDMNREnable(DMNR_TYPE type, bool enable)
{
    ALOGD("%s(), type=%d, bypassDMNR=%d", __FUNCTION__, type,mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess);

    if (((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) && (mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess == false))
    {
        if ((QueryFeatureSupportInfo()& SUPPORT_DMNR_3_0) > 0)
        {
            switch (type)
            {
                case DMNR_DISABLE :
                    mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
                    mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
                    break;
                case DMNR_NORMAL :
                    mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, enable);
                    break;
                case DMNR_HANDSFREE :
                    mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, enable);
                    break;
                default:
                    mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
                    mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
                    break;
            }
        }
        else
        {

            ALOGD("%s(), turn off due to not support", __FUNCTION__);
            mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
            mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
        }
    }
}

timespec AudioALSACaptureDataClient::GetCaptureTimeStamp(void)
{
    struct timespec capturetime;

    long ret_ns;
    capturetime.tv_sec  = 0;
    capturetime.tv_nsec = 0;

    if ((mStreamAttributeSource->Time_Info.timestamp_get.tv_sec == 0) && (mStreamAttributeSource->Time_Info.timestamp_get.tv_nsec == 0))
    {
        ALOGE("%s fail", __FUNCTION__);
    }
    else
    {
        capturetime = mStreamAttributeSource->Time_Info.timestamp_get;
        ret_ns = mStreamAttributeSource->Time_Info.kernelbuffer_ns;
        if ((capturetime.tv_nsec - ret_ns) >= 0)
        {
            capturetime.tv_nsec -= ret_ns;
        }
        else
        {
            capturetime.tv_sec -= 1;
            capturetime.tv_nsec = 1000000000 + capturetime.tv_nsec - ret_ns;
        }
        ALOGV("%s, sec= %ld, nsec=%ld, ret_ns = %ld\n", __FUNCTION__, capturetime.tv_sec, capturetime.tv_nsec, ret_ns);
    }

    return capturetime;
}

timespec AudioALSACaptureDataClient::GetEchoRefTimeStamp(void)
{
    struct timespec echoreftime;

    long ret_ns;
    echoreftime.tv_sec  = 0;
    echoreftime.tv_nsec = 0;

    if ((mStreamAttributeSourceEchoRef->Time_Info.timestamp_get.tv_sec == 0) && (mStreamAttributeSourceEchoRef->Time_Info.timestamp_get.tv_nsec == 0))
    {
        ALOGE("%s fail", __FUNCTION__);
    }
    else
    {
        echoreftime = mStreamAttributeSourceEchoRef->Time_Info.timestamp_get;
        ret_ns = mStreamAttributeSourceEchoRef->Time_Info.kernelbuffer_ns;
        if ((echoreftime.tv_nsec - ret_ns) >= 0)
        {
            echoreftime.tv_nsec -= ret_ns;
        }
        else
        {
            echoreftime.tv_sec -= 1;
            echoreftime.tv_nsec = 1000000000 + echoreftime.tv_nsec - ret_ns;
        }
        ALOGV("%s, sec= %ld, nsec=%ld, ret_ns = %ld\n", __FUNCTION__, echoreftime.tv_sec, echoreftime.tv_nsec, ret_ns);
    }

    return echoreftime;
}

//BesRecord---
//#endif    //MTK_AUDIO_HD_REC_SUPPORT

//Android Native Preprocess effect +++
void AudioALSACaptureDataClient::CheckNativeEffect(void)
{

    if (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Update == true)
    {
        ALOGD("+%s() %d", __FUNCTION__, mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Count);

        if (mAudioPreProcessEffect != NULL)
        {
#if 0
            for (int i = 0; i < mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Count; i++)
            {
                //mAudioPreProcessEffect->removeAudioEffect(mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Record[i]);
                //mAudioPreProcessEffect->addAudioEffect(mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Record[i]);
            }
#endif
            mAudioPreProcessEffect->CheckNativeEffect();
        }

        mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Update = false;
        ALOGD("-%s()", __FUNCTION__);
    }
}

uint32_t AudioALSACaptureDataClient::NativePreprocess(void *buffer , uint32_t bytes)
{
    uint32_t retsize = bytes;
    retsize = mAudioPreProcessEffect->NativePreprocess(buffer, bytes, &mStreamAttributeSource->Time_Info);
    return retsize;
}

//Android Native Preprocess effect ---

//EchoRef+++
void AudioALSACaptureDataClient::AddEchoRefDataProvider(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target)
{
    ALOGD("+%s()", __FUNCTION__);
    mStreamAttributeTargetEchoRef = stream_attribute_target;
    mCaptureDataProviderEchoRef = pCaptureDataProvider;//AudioALSACaptureDataProviderEchoRef::getInstance();
    mStreamAttributeSourceEchoRef = mCaptureDataProviderEchoRef->getStreamAttributeSource();

    //check SRC needed and created
    // raw data
    memset((void *)&mEchoRefRawDataBuf, 0, sizeof(mEchoRefRawDataBuf));
    mEchoRefRawDataBuf.pBufBase = new char[kClientBufferSize];
    mEchoRefRawDataBuf.bufLen   = kClientBufferSize;
    mEchoRefRawDataBuf.pRead    = mEchoRefRawDataBuf.pBufBase;
    mEchoRefRawDataBuf.pWrite   = mEchoRefRawDataBuf.pBufBase;
    ASSERT(mEchoRefRawDataBuf.pBufBase != NULL);

    // src data
    memset((void *)&mEchoRefSrcDataBuf, 0, sizeof(mEchoRefSrcDataBuf));
    mEchoRefSrcDataBuf.pBufBase = new char[kClientBufferSize];
    mEchoRefSrcDataBuf.bufLen   = kClientBufferSize;
    mEchoRefSrcDataBuf.pRead    = mEchoRefSrcDataBuf.pBufBase;
    mEchoRefSrcDataBuf.pWrite   = mEchoRefSrcDataBuf.pBufBase;
    ASSERT(mEchoRefSrcDataBuf.pBufBase != NULL);

    // attach client to capture EchoRef data provider
    ALOGD("%s(), mCaptureDataProviderEchoRef=%p", __FUNCTION__, mCaptureDataProviderEchoRef);
    mCaptureDataProviderEchoRef->attach(this); // mStreamAttributeSource will be updated when first client attached


    ALOGD("%s(), Source sample_rate=%d, num_channels=%d, audio_format=%d", __FUNCTION__
          , mStreamAttributeSourceEchoRef->sample_rate, mStreamAttributeSourceEchoRef->num_channels, mStreamAttributeSourceEchoRef->audio_format);
    ALOGD("%s(), Target sample_rate=%d, num_channels=%d, audio_format=%d", __FUNCTION__
          , mStreamAttributeTargetEchoRef->sample_rate, mStreamAttributeTargetEchoRef->num_channels, mStreamAttributeTargetEchoRef->audio_format);

    //assume starts after PCM open
    mSPELayer->SetOutputStreamRunning(true, true);
    mSPELayer->SetEchoRefStartTime(GetSystemTime(false));
    mSPELayer->SetDownLinkLatencyTime(mStreamAttributeSourceEchoRef->latency);

    // init SRC, this SRC is for Android Native.
    if (mStreamAttributeSourceEchoRef->sample_rate  != mStreamAttributeTargetEchoRef->sample_rate  ||
        mStreamAttributeSourceEchoRef->num_channels != mStreamAttributeTargetEchoRef->num_channels ||
        mStreamAttributeSourceEchoRef->audio_format != mStreamAttributeTargetEchoRef->audio_format)
    {
        mBliSrcEchoRef = new MtkAudioSrc(
            mStreamAttributeSourceEchoRef->sample_rate, mStreamAttributeSourceEchoRef->num_channels,
            mStreamAttributeTargetEchoRef->sample_rate, mStreamAttributeTargetEchoRef->num_channels,
            SRC_IN_Q1P15_OUT_Q1P15); // TODO(Harvey, Ship): 24bit
        mBliSrcEchoRef->Open();
    }

    // init SRC, this SRC is for MTK VoIP
    if ((mStreamAttributeTargetEchoRef->sample_rate != 16000) || (mStreamAttributeTargetEchoRef->num_channels != 1))
    {
        mBliSrcEchoRefBesRecord = new MtkAudioSrc(
            mStreamAttributeTargetEchoRef->sample_rate, mStreamAttributeTargetEchoRef->num_channels,
            16000, 1,
            SRC_IN_Q1P15_OUT_Q1P15);
        mBliSrcEchoRefBesRecord->Open();
    }

    ALOGD("-%s()", __FUNCTION__);
}


//EchoRef data no need to provide to Capture handler
uint32_t AudioALSACaptureDataClient::copyEchoRefCaptureDataToClient(RingBuf pcm_read_buf)
{
    ALOGV("+%s()", __FUNCTION__);

    uint32_t freeSpace = RingBuf_getFreeSpace(&mEchoRefRawDataBuf);
    uint32_t dataSize = RingBuf_getDataCount(&pcm_read_buf);
    if (freeSpace < dataSize)
    {
        ALOGE("%s(), mRawDataBuf <= pcm_read_buf, freeSpace(%u) < dataSize(%u), buffer overflow!!", __FUNCTION__, freeSpace, dataSize);
        RingBuf_copyFromRingBuf(&mEchoRefRawDataBuf, &pcm_read_buf, freeSpace);
    }
    else
    {
        RingBuf_copyFromRingBuf(&mEchoRefRawDataBuf, &pcm_read_buf, dataSize);
    }

    // SRC to 48K stereo (as streamin format since AWB data might be the same as DL1 before), to Native AEC need format
    const uint32_t kNumRawData = RingBuf_getDataCount(&mEchoRefRawDataBuf);
    uint32_t num_free_space = RingBuf_getFreeSpace(&mEchoRefSrcDataBuf);

    if (mBliSrcEchoRef == NULL) // No need SRC
    {
        //ASSERT(num_free_space >= kNumRawData);
        if (num_free_space < kNumRawData)
        {
            ALOGW("%s(), num_free_space(%u) < kNumRawData(%u)", __FUNCTION__, num_free_space, kNumRawData);
            RingBuf_copyFromRingBuf(&mEchoRefSrcDataBuf, &mEchoRefRawDataBuf, num_free_space);
        }
        else
        {
            RingBuf_copyFromRingBuf(&mEchoRefSrcDataBuf, &mEchoRefRawDataBuf, kNumRawData);
        }
    }
    else // Need SRC
    {
        char *pEchoRefRawDataLinearBuf = new char[kNumRawData];
        RingBuf_copyToLinear(pEchoRefRawDataLinearBuf, &mEchoRefRawDataBuf, kNumRawData);

        char *pEchoRefSrcDataLinearBuf = new char[num_free_space];

        char *p_read = pEchoRefRawDataLinearBuf;
        uint32_t num_raw_data_left = kNumRawData;
        uint32_t num_converted_data = num_free_space; // max convert num_free_space

        uint32_t consumed = num_raw_data_left;
        mBliSrcEchoRef->Process((int16_t *)p_read, &num_raw_data_left,
                                (int16_t *)pEchoRefSrcDataLinearBuf, &num_converted_data);
        consumed -= num_raw_data_left;

        p_read += consumed;
        ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);

        //ASSERT(num_raw_data_left == 0);
        if (num_raw_data_left > 0)
        {
            ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
        }

        RingBuf_copyFromLinear(&mEchoRefSrcDataBuf, pEchoRefSrcDataLinearBuf, num_converted_data);
        ALOGV("%s(), pRead:%u, pWrite:%u, dataCount:%u", __FUNCTION__,
              mEchoRefSrcDataBuf.pRead - mEchoRefSrcDataBuf.pBufBase,
              mEchoRefSrcDataBuf.pWrite - mEchoRefSrcDataBuf.pBufBase,
              RingBuf_getDataCount(&mEchoRefSrcDataBuf));

        delete[] pEchoRefRawDataLinearBuf;
        delete[] pEchoRefSrcDataLinearBuf;
    }


    //for Preprocess
    const uint32_t kNumEchoRefSrcData = RingBuf_getDataCount(&mEchoRefSrcDataBuf);
    char *pEchoRefProcessDataLinearBuf = new char[kNumEchoRefSrcData];
    RingBuf_copyToLinear(pEchoRefProcessDataLinearBuf, &mEchoRefSrcDataBuf, kNumEchoRefSrcData);

    //here to queue the EchoRef data to Native effect, since it doesn't need to SRC here
    if ((mAudioPreProcessEffect->num_preprocessors > 0))    //&& echoref is enabled
    {
        //copy pEchoRefProcessDataLinearBuf to native preprocess for echo ref
        mAudioPreProcessEffect->WriteEchoRefData(pEchoRefProcessDataLinearBuf, kNumEchoRefSrcData, &mStreamAttributeSourceEchoRef->Time_Info);
    }

    //If need MTK VoIP process
    if ((mStreamAttributeTarget->BesRecord_Info.besrecord_enable) && !mBypassBesRecord)
    {
        struct InBufferInfo BufInfo;
        //for MTK native SRC
        if (mBliSrcEchoRefBesRecord == NULL) // No need SRC
        {
            //TODO(Sam),copy pEchoRefProcessDataLinearBuf to MTK echoref data directly

            BufInfo.pBufBase = (short *)pEchoRefProcessDataLinearBuf;
            BufInfo.BufLen = kNumEchoRefSrcData;
            BufInfo.time_stamp_queued = GetSystemTime(false);
            BufInfo.bHasRemainInfo = true;
            BufInfo.time_stamp_predict = GetEchoRefTimeStamp();

            mSPELayer->WriteReferenceBuffer(&BufInfo);

        }
        else // Need SRC
        {
            char *pEchoRefProcessSRCDataLinearBuf = new char[kNumEchoRefSrcData];

            char *p_read = pEchoRefProcessDataLinearBuf;
            uint32_t num_raw_data_left = kNumEchoRefSrcData;
            uint32_t num_converted_data = kNumEchoRefSrcData; // max convert num_free_space
            uint32_t consumed = num_raw_data_left;

            mBliSrcEchoRefBesRecord->Process((int16_t *)p_read, &num_raw_data_left,
                                             (int16_t *)pEchoRefProcessSRCDataLinearBuf, &num_converted_data);
            consumed -= num_raw_data_left;

            p_read += consumed;
            ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
                  __FUNCTION__, num_raw_data_left, num_converted_data);

            //ASSERT(num_raw_data_left == 0);
            if (num_raw_data_left > 0)
            {
                ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
            }

            //TODO: (Sam )copy pEchoRefSrcDataLinearBuf to MTK VoIP write echo ref data
            BufInfo.pBufBase = (short *)pEchoRefProcessSRCDataLinearBuf;
            BufInfo.BufLen = num_converted_data;
            BufInfo.time_stamp_queued = GetSystemTime(false);
            BufInfo.bHasRemainInfo = true;
            BufInfo.time_stamp_predict = GetEchoRefTimeStamp();

            mSPELayer->WriteReferenceBuffer(&BufInfo);

            delete[] pEchoRefProcessSRCDataLinearBuf;
        }
    }

    delete[] pEchoRefProcessDataLinearBuf;
    return 0;
}

//EchoRef---

} // end of namespace android

