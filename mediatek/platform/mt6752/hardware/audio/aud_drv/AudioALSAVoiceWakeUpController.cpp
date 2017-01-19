#include "AudioALSAVoiceWakeUpController.h"

#include "AudioLock.h"
#include "AudioALSADriverUtility.h"

#include "AudioMTKHeadsetMessager.h"

#include <AudioCustParam.h>
#include "AudioUtility.h"
#include "AudioALSACaptureDataProviderVOW.h"


#define LOG_TAG "AudioALSAVoiceWakeUpController"
#define VOW_POWER_ON_SLEEP_MS 50
namespace android
{

AudioALSAVoiceWakeUpController *AudioALSAVoiceWakeUpController::mAudioALSAVoiceWakeUpController = NULL;
AudioALSAVoiceWakeUpController *AudioALSAVoiceWakeUpController::getInstance()
{
    AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSAVoiceWakeUpController == NULL)
    {
        mAudioALSAVoiceWakeUpController = new AudioALSAVoiceWakeUpController();
    }
    ASSERT(mAudioALSAVoiceWakeUpController != NULL);
    return mAudioALSAVoiceWakeUpController;
}

AudioALSAVoiceWakeUpController::AudioALSAVoiceWakeUpController() :
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mEnable(false),
    mIsUseHeadsetMic(false),
    mIsNeedToUpdateParamToKernel(true),
    mDebug_Enable(false)
{
    ALOGD("%s()", __FUNCTION__);

    mHandsetMicMode = GetMicDeviceMode(0);
    mHeadsetMicMode = GetMicDeviceMode(1);

    mIsUseHeadsetMic = AudioMTKHeadSetMessager::getInstance()->isHeadsetPlugged();

    mVOWCaptureDataProvider = AudioALSACaptureDataProviderVOW::getInstance();

    GetVoiceRecogCustParamFromNV(&mVRParam);
    updateParamToKernel();
}

AudioALSAVoiceWakeUpController::~AudioALSAVoiceWakeUpController()
{
    ALOGD("%s()", __FUNCTION__);
}


bool AudioALSAVoiceWakeUpController::getVoiceWakeUpEnable()
{
    AudioAutoTimeoutLock _l(mLock);
    return mEnable;
}


status_t AudioALSAVoiceWakeUpController::setVoiceWakeUpEnable(const bool enable)
{
    ALOGD("+%s(), mEnable: %d => %d, mIsUseHeadsetMic = %d", __FUNCTION__, mEnable, enable, mIsUseHeadsetMic);
    AudioAutoTimeoutLock _l(mLock);

    if (mEnable == enable)
    {
        ALOGW("-%s(), enable(%d) == mEnable(%d), return", __FUNCTION__, enable, mEnable);
        return INVALID_OPERATION;
    }

    if (enable == true)
    {
        setVoiceWakeUpDebugDumpEnable(true);

        updateParamToKernel();

        //set input MIC type
        if (mIsUseHeadsetMic)
        {
            //use headset mic
            if (mHeadsetMicMode == AUDIO_MIC_MODE_ACC)
            {
                if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HeadsetMIC"))
                {
                    ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HeadsetMIC");
                }
            }
            else if (mHeadsetMicMode == AUDIO_MIC_MODE_DCC)
            {
                if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HeadsetMIC_DCC"))
                {
                    ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HeadsetMIC_DCC");
                }
            }
            else if (mHeadsetMicMode == AUDIO_MIC_MODE_DCCECMDIFF)
            {
                if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HeadsetMIC_DCCECM"))
                {
                    ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value AUDIO_MIC_MODE_DCCECMDIFF");
                }
            }
            else if (mHeadsetMicMode == AUDIO_MIC_MODE_DCCECMSINGLE)
            {
                if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HeadsetMIC_DCCECM"))
                {
                    ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value AUDIO_MIC_MODE_DCCECMSINGLE");
                }
            }
            else
            {
                if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HeadsetMIC"))
                {
                    ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HeadsetMIC");
                }
            }
        }
        else
        {
            //DMIC device
            if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
            {
                if (mHandsetMicMode == AUDIO_MIC_MODE_DMIC_LP)
                {
                    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HandsetDMIC_800K"))
                    {
                        ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HandsetDMIC_800K");
                    }
                }
                else    //normal DMIC
                {
                    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HandsetDMIC"))
                    {
                        ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HandsetDMIC");
                    }
                }
            }
            else    //analog MIC device
            {
                if (mHandsetMicMode == AUDIO_MIC_MODE_ACC)
                {
                    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HandsetAMIC"))
                    {
                        ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HandsetAMIC");
                    }
                }
                else if (mHandsetMicMode == AUDIO_MIC_MODE_DCC) //DCC mems mic
                {
                    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HandsetAMIC_DCC"))
                    {
                        ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HandsetAMIC_DCC");
                    }
                }
                else if (mHandsetMicMode == AUDIO_MIC_MODE_DCCECMDIFF) //DCC ecm mic
                {
                    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HandsetAMIC_DCCECM"))
                    {
                        ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value AUDIO_MIC_MODE_DCCECMDIFF");
                    }
                }
                else if (mHandsetMicMode == AUDIO_MIC_MODE_DCCECMSINGLE) //DCC ecm mic
                {
                    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HandsetAMIC_DCCECM"))
                    {
                        ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value AUDIO_MIC_MODE_DCCECMSINGLE");
                    }
                }
                else
                {
                    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_MIC_Type_Select"), "HandsetAMIC"))
                    {
                        ALOGE("Error: Audio_Vow_MIC_Type_Select invalid value HandsetAMIC");
                    }
                }
            }
        }


        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_ADC_Func_Switch"), "On"))
        {
            ALOGE("Error: Audio_Vow_ADC_Func_Switch invalid value");
        }

        usleep(VOW_POWER_ON_SLEEP_MS * 1000);

        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_Digital_Func_Switch"), "On"))
        {
            ALOGE("Error: Audio_Vow_Digital_Func_Switch invalid value");
        }
    }
    else
    {
        setVoiceWakeUpDebugDumpEnable(false);
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_Digital_Func_Switch"), "Off"))
        {
            ALOGE("Error: Audio_Vow_Digital_Func_Switch invalid value");
        }


        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Vow_ADC_Func_Switch"), "Off"))
        {
            ALOGE("Error: Audio_Vow_ADC_Func_Switch invalid value");
        }
    }

    mEnable = enable;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAVoiceWakeUpController::updateDeviceInfoForVoiceWakeUp()
{
    ALOGD("+%s(), mIsUseHeadsetMic = %d", __FUNCTION__, mIsUseHeadsetMic);

    bool bIsUseHeadsetMic = AudioMTKHeadSetMessager::getInstance()->isHeadsetPlugged();

    if (bIsUseHeadsetMic != mIsUseHeadsetMic)
    {
        if (mEnable == false)
        {
            mIsUseHeadsetMic = bIsUseHeadsetMic;
        }
        else
        {
            setVoiceWakeUpEnable(false);
            mIsUseHeadsetMic = bIsUseHeadsetMic;
            setVoiceWakeUpEnable(true);
        }
    }

    ALOGD("-%s(), mIsUseHeadsetMic = %d", __FUNCTION__, mIsUseHeadsetMic);
    return NO_ERROR;
}


status_t AudioALSAVoiceWakeUpController::SetVOWCustParam(int index, int value)
{
    AudioAutoTimeoutLock _l(mLock);
    int vow_index = 5;
    mIsNeedToUpdateParamToKernel = true;

    if (SetVOWCustParamToNV(index, value) == 0)
    {
        mVRParam.cust_param[vow_index][index] = value;
    }
    return NO_ERROR;
}


status_t AudioALSAVoiceWakeUpController::updateParamToKernel()
{
    if (mIsNeedToUpdateParamToKernel == true)
    {
        mIsNeedToUpdateParamToKernel = false;

        int vow_index = 5;
        int mVOW_CFG2 = 0;
        int mVOW_CFG3 = 0;
        int mVOW_CFG4 = 0;

        struct mixer_ctl *ctl;
        ctl = mixer_get_ctl_by_name(mMixer, "Audio VOWCFG4 Data");
        mVOW_CFG4 = mixer_ctl_get_value(ctl, 0);
        ALOGD("%s(), mVOW_CFG4 load = 0x%x", __FUNCTION__, mVOW_CFG4);

        mVOW_CFG2 = ((mVRParam.cust_param[vow_index][5] & 0x0007) << 12) |
                    ((mVRParam.cust_param[vow_index][6] & 0x0007) << 8)  |
                    ((mVRParam.cust_param[vow_index][7] & 0x0007) << 4)  |
                    ((mVRParam.cust_param[vow_index][8] & 0x0007));
        mVOW_CFG3 = ((mVRParam.cust_param[vow_index][0] & 0x000f) << 12) |
                    ((mVRParam.cust_param[vow_index][1] & 0x000f) << 8)  |
                    ((mVRParam.cust_param[vow_index][2] & 0x000f) << 4)  |
                    ((mVRParam.cust_param[vow_index][3] & 0x000f));
        mVOW_CFG4 &= 0xFFF0;
        mVOW_CFG4 |= mVRParam.cust_param[vow_index][4];

        ALOGD("%s(), mVOW_CFG4 set = 0x%x, mVOW_CFG2=0x%x, mVOW_CFG3=0x%x", __FUNCTION__, mVOW_CFG4, mVOW_CFG2, mVOW_CFG3);
        //reenable when NVRAM's initial value is correct
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Audio VOWCFG2 Data"), 0, mVOW_CFG2))
        {
            ALOGE("Error: Audio VOWCFG2 Data invalid value");
        }

        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Audio VOWCFG3 Data"), 0, mVOW_CFG3))
        {
            ALOGE("Error: Audio VOWCFG3 Data invalid value");
        }

        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Audio VOWCFG4 Data"), 0, mVOW_CFG4))
        {
            ALOGE("Error: Audio VOWCFG4 Data invalid value");
        }

    }
    return NO_ERROR;
}

status_t AudioALSAVoiceWakeUpController::setVoiceWakeUpDebugDumpEnable(const bool enable)
{
    ALOGD("+%s(), mDebug_Enable: %d => %d", __FUNCTION__, mDebug_Enable, enable);

    if (mDebug_Enable == enable)
    {
        ALOGW("-%s(), enable(%d) == mDebug_Enable(%d), return", __FUNCTION__, enable, mDebug_Enable);
        return INVALID_OPERATION;
    }

    char value[PROPERTY_VALUE_MAX];
    property_get(streamin_propty, value, "0");
    int bflag = atoi(value);

    if (bflag && enable)
    {
        if (!mDebug_Enable)
        {
            //enable VOW debug dump
            mDebug_Enable = true;
            mVOWCaptureDataProvider->open();
        }
    }
    else
    {
        if (mDebug_Enable)
        {
            //disable VOW debug dump
            mVOWCaptureDataProvider->close();
            mDebug_Enable = false;
        }
    }
    ALOGD("-%s(), mDebug_Enable: %d", __FUNCTION__, mDebug_Enable);
    return NO_ERROR;
}

bool AudioALSAVoiceWakeUpController::getVoiceWakeUpStateFromKernel()
{
    ALOGD("%s()+", __FUNCTION__);
    bool bRet = false;
    struct mixer_ctl *ctl;
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_VOW_State");
    bRet = mixer_ctl_get_value(ctl, 0);
    ALOGD("%s(), state = 0x%x", __FUNCTION__, bRet);
        
    return bRet;
}

} // end of namespace android
