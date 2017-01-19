#include "AudioSpeechEnhanceInfo.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include "AudioUtility.h"
#include <cutils/properties.h>
#include "AudioCustParam.h"
#include "AudioALSAStreamManager.h"

//#include "AudioLock.h"

#define LOG_TAG "AudioSpeechEnhanceInfo"
static const char PROPERTY_KEY_VOIP_SPH_ENH_MASKS[PROPERTY_KEY_MAX] = "persist.af.voip.sph_enh_mask";


namespace android
{
AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::mAudioSpeechEnhanceInfo = NULL;

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::getInstance()
{
    static AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioSpeechEnhanceInfo == NULL)
    {
        ALOGD("%s()", __FUNCTION__);
        mAudioSpeechEnhanceInfo = new AudioSpeechEnhanceInfo();
    }
    ASSERT(mAudioSpeechEnhanceInfo != NULL);
    return mAudioSpeechEnhanceInfo;
}

AudioSpeechEnhanceInfo::AudioSpeechEnhanceInfo()
{
    ALOGD("%s()", __FUNCTION__);
    mBesRecScene = -1;

    //for tuning purpose
    mBesRecTuningEnable = false;
    mAPDMNRTuningEnable = false;
    mAPTuningMode = TUNING_MODE_NONE;

    mForceMagiASR = false;
    mForceAECRec = false;

    // default value (all enhancement on)
    char property_default_value[PROPERTY_VALUE_MAX];
    sprintf(property_default_value, "0x%x", VOIP_SPH_ENH_DYNAMIC_MASK_ALL);

    // get voip sph_enh_mask_struct from property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VOIP_SPH_ENH_MASKS, property_value, property_default_value);

    // parse mask info from property_value
    sscanf(property_value, "0x%x", &mVoIPSpeechEnhancementMask);

    PreLoadBesRecordParams();
}

AudioSpeechEnhanceInfo::~AudioSpeechEnhanceInfo()
{
    ALOGD("%s()", __FUNCTION__);
}

//----------------for HD Record Preprocess +++ -------------------------
void AudioSpeechEnhanceInfo::SetBesRecScene(int32_t BesRecScene)
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("%s() %d", __FUNCTION__, BesRecScene);
    mBesRecScene = BesRecScene;
}

int32_t AudioSpeechEnhanceInfo::GetBesRecScene()
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("%s() %d", __FUNCTION__, mBesRecScene);
    return mBesRecScene;
}

void AudioSpeechEnhanceInfo::ResetBesRecScene()
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("%s()", __FUNCTION__);
    mBesRecScene = -1;
}

//----------------for HD Record Preprocess --- -----------------------------

//----------------Get MMI info for AP Speech Enhancement --------------------------------
void AudioSpeechEnhanceInfo::UpdateDynamicSpeechEnhancementMask(const voip_sph_enh_mask_struct_t &mask)
{
    uint32_t feature_support = QueryFeatureSupportInfo();

    ALOGD("%s(), mask = %x, feature_support=%x, %x", __FUNCTION__, mask, feature_support, (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)));

    if (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE))
    {

        char property_value[PROPERTY_VALUE_MAX];
        sprintf(property_value, "0x%x", mask.dynamic_func);
        property_set(PROPERTY_KEY_VOIP_SPH_ENH_MASKS, property_value);

        mVoIPSpeechEnhancementMask = mask;
        AudioALSAStreamManager::getInstance()->UpdateDynamicFunctionMask();
    }
    else
    {
        ALOGD("%s(), not support", __FUNCTION__);
    }

}

status_t AudioSpeechEnhanceInfo::SetDynamicVoIPSpeechEnhancementMask(const voip_sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on)
{
    //Mutex::Autolock lock(mHDRInfoLock);
    uint32_t feature_support = QueryFeatureSupportInfo();

    ALOGD("%s(), feature_support=%x, %x", __FUNCTION__, feature_support, (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)));

    if (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE))
    {
        voip_sph_enh_mask_struct_t mask = GetDynamicVoIPSpeechEnhancementMask();

        ALOGW("%s(), dynamic_mask_type(%x), %x",
              __FUNCTION__, dynamic_mask_type, mask.dynamic_func);
        const bool current_flag_on = ((mask.dynamic_func & dynamic_mask_type) > 0);
        if (new_flag_on == current_flag_on)
        {
            ALOGW("%s(), dynamic_mask_type(%x), new_flag_on(%d) == current_flag_on(%d), return",
                  __FUNCTION__, dynamic_mask_type, new_flag_on, current_flag_on);
            return NO_ERROR;
        }

        if (new_flag_on == false)
        {
            mask.dynamic_func &= (~dynamic_mask_type);
        }
        else
        {
            mask.dynamic_func |= dynamic_mask_type;
        }

        UpdateDynamicSpeechEnhancementMask(mask);
    }
    else
    {
        ALOGW("%s(), not support", __FUNCTION__);
    }

    return NO_ERROR;
}

//----------------Audio tunning +++ --------------------------------
//----------------for BesRec tunning --------------------------------
void AudioSpeechEnhanceInfo::SetBesRecTuningEnable(bool bEnable)
{
    ALOGD("%s()+", __FUNCTION__);
    AudioAutoTimeoutLock _l(mLock);
    mBesRecTuningEnable = bEnable;
    ALOGD("%s()- %d", __FUNCTION__, bEnable);
}

bool AudioSpeechEnhanceInfo::IsBesRecTuningEnable(void)
{
    ALOGD("%s()+", __FUNCTION__);
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("%s()- %d", __FUNCTION__, mBesRecTuningEnable);
    return mBesRecTuningEnable;
}

status_t AudioSpeechEnhanceInfo::SetBesRecVMFileName(const char *fileName)
{
    ALOGD("%s()+", __FUNCTION__);
    AudioAutoTimeoutLock _l(mLock);
    if (fileName != NULL && strlen(fileName) < 128 - 1)
    {
        ALOGD("%s(), file name:%s", __FUNCTION__, fileName);
        memset(mVMFileName, 0, 128);
        strcpy(mVMFileName, fileName);
    }
    else
    {
        ALOGD("%s(), input file name NULL or too long!", __FUNCTION__);
        return BAD_VALUE;
    }
    return NO_ERROR;
}
void AudioSpeechEnhanceInfo::GetBesRecVMFileName(char *VMFileName)
{
    ALOGD("%s()+", __FUNCTION__);
    AudioAutoTimeoutLock _l(mLock);
    memset(VMFileName, 0, VM_FILE_NAME_LEN_MAX);
    strcpy(VMFileName, mVMFileName);
    ALOGD("%s(), mVMFileName=%s, VMFileName=%s", __FUNCTION__, mVMFileName, VMFileName);
}

//----------------for AP DMNR tunning +++ --------------------------------
void AudioSpeechEnhanceInfo::SetAPDMNRTuningEnable(bool bEnable)
{
    if ((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0)
    {
        AudioAutoTimeoutLock _l(mLock);
        ALOGD("%s(), %d", __FUNCTION__, bEnable);
        mAPDMNRTuningEnable = bEnable;
    }
    else
    {
        ALOGD("%s(), no Dual MIC, not set", __FUNCTION__);
    }
}

bool AudioSpeechEnhanceInfo::IsAPDMNRTuningEnable(void)
{
    if ((QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0)
    {
        ALOGD("%s()+", __FUNCTION__);
        AudioAutoTimeoutLock _l(mLock);
        //ALOGD("IsAPDMNRTuningEnable=%d",mAPDMNRTuningEnable);
        ALOGD("%s(), %d", __FUNCTION__, mAPDMNRTuningEnable);
        return mAPDMNRTuningEnable;
    }
    else
    {
        return false;
    }
}

bool AudioSpeechEnhanceInfo::SetAPTuningMode(const TOOL_TUNING_MODE mode)
{
    bool bRet = false;
    //for different MIC gain
    ALOGD("%s(), SetAPTuningMode mAPDMNRTuningEnable=%d, mode=%d", __FUNCTION__, mAPDMNRTuningEnable, mode);
    if (mAPDMNRTuningEnable)
    {
        mAPTuningMode = mode;
        bRet = true;
    }
    return bRet;
}

int AudioSpeechEnhanceInfo::GetAPTuningMode()
{
    ALOGD("%s(), mAPTuningMode=%d", __FUNCTION__, mAPTuningMode);

    return mAPTuningMode;
}

//----------------Audio tunning --- --------------------------------

//Engineer mode enable MagiASR+++
bool AudioSpeechEnhanceInfo::SetForceMagiASR(bool enable)
{
    ALOGD("%s, %d", __FUNCTION__, enable);
    mForceMagiASR = enable;
    return true;
}

status_t AudioSpeechEnhanceInfo::GetForceMagiASRState()
{
    status_t ret = 0;
    uint32_t feature_support = QueryFeatureSupportInfo();

    ALOGD("%s(), feature_support=%x, %x, mForceMagiASR=%d", __FUNCTION__, feature_support, (feature_support & SUPPORT_ASR), mForceMagiASR);

    if (feature_support & SUPPORT_ASR)
    {
        if (mForceMagiASR)
        {
            ret = 1;
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = 0;
    }

    return ret;
}

//Engineer mode MagiASR---

//Engineer mode enable AECRecord+
bool AudioSpeechEnhanceInfo::SetForceAECRec(bool enable)
{
    ALOGD("%s, %d", __FUNCTION__, enable);
    mForceAECRec = enable;
    return true;
}

bool AudioSpeechEnhanceInfo::GetForceAECRecState()
{
    status_t ret = false;

    ALOGD("%s(), mForceAECRec=%d", __FUNCTION__, mForceAECRec);

    if (mForceAECRec)
    {
        ret = true;
    }
    return ret;
}
//Engineer mode enable AECRecord-

//BesRecord+++
//preload BesRecord parames to avoid record loading fail when storage full
void AudioSpeechEnhanceInfo::PreLoadBesRecordParams(void)
{
    ALOGD("%s+", __FUNCTION__);
    //for NVRAM create file first to reserve the memory
    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT DummyhdRecordSceneTable;
    AUDIO_HD_RECORD_PARAM_STRUCT DummyhdRecordParam;
    AUDIO_VOIP_PARAM_STRUCT DummyVoIPParam;
    AUDIO_CUSTOM_EXTRA_PARAM_STRUCT DummyDMNRParam;
    GetHdRecordSceneTableFromNV(&DummyhdRecordSceneTable);
    GetHdRecordParamFromNV(&DummyhdRecordParam);
    GetAudioVoIPParamFromNV(&DummyVoIPParam);
    GetDualMicSpeechParamFromNVRam(&DummyDMNRParam);
    ALOGD("%s-", __FUNCTION__);
}
//BesRecord---

}

