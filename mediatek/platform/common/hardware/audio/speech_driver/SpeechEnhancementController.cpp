#include "SpeechEnhancementController.h"
#include "SpeechDriverFactory.h"
#include "SpeechDriverInterface.h"
#include "SpeechType.h"

#include <cutils/properties.h>

#include <utils/threads.h>

#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"

#define LOG_TAG "SpeechEnhancementController"

static const char PROPERTY_KEY_SPH_ENH_MASKS[PROPERTY_KEY_MAX] = "persist.af.modem.sph_enh_mask";
static const char PROPERTY_KEY_MAGIC_CON_CALL_ON[PROPERTY_KEY_MAX] = "persist.af.magic_con_call_on";
static const char PROPERTY_KEY_BT_HEADSET_NREC_ON[PROPERTY_KEY_MAX] = "persist.af.bt_headset_nrec_on";
static const char PROPERTY_KEY_HAC_ON[PROPERTY_KEY_MAX] = "persist.af.hac_on";


namespace android
{

SpeechEnhancementController *SpeechEnhancementController::mSpeechEnhancementController = NULL;
SpeechEnhancementController *SpeechEnhancementController::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechEnhancementController == NULL)
    {
        mSpeechEnhancementController = new SpeechEnhancementController();
    }
    ASSERT(mSpeechEnhancementController != NULL);
    return mSpeechEnhancementController;
}


SpeechEnhancementController::SpeechEnhancementController()
{
    // default value (all enhancement on)
    char property_default_value[PROPERTY_VALUE_MAX];
    sprintf(property_default_value, "0x%x 0x%x", SPH_ENH_MAIN_MASK_ALL, (SPH_ENH_DYNAMIC_MASK_ALL & (~SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) & (~SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN)));

    // get sph_enh_mask_struct from property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPH_ENH_MASKS, property_value, property_default_value);

    // parse mask info from property_value
    sscanf(property_value, "0x%x 0x%x",
           &mSpeechEnhancementMask.main_func,
           &mSpeechEnhancementMask.dynamic_func);

    ALOGD("mSpeechEnhancementMask: main_func = 0x%x, sub_func = 0x%x",
          mSpeechEnhancementMask.main_func,
          mSpeechEnhancementMask.dynamic_func);

    // Magic conference call
    char magic_conference_call_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_MAGIC_CON_CALL_ON, magic_conference_call_on, "0"); //"0": default off
    mMagicConferenceCallOn = (magic_conference_call_on[0] == '0') ? false : true;

    // HAC
    char hac_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_HAC_ON, hac_on, "0"); //"0": default off
    mHACOn = (hac_on[0] == '0') ? false : true;

    // BT Headset NREC
    char bt_headset_nrec_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_BT_HEADSET_NREC_ON, bt_headset_nrec_on, "1"); //"1": default on
    mBtHeadsetNrecOn = (bt_headset_nrec_on[0] == '0') ? false : true;
    mSMNROn = false;
}

SpeechEnhancementController::~SpeechEnhancementController()
{

}

status_t SpeechEnhancementController::SetNBSpeechParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;
    AUDIO_CUSTOM_PARAM_STRUCT mSphParamNB;

    if (mSMNROn == true)
    {
        //forcely set single mic setting
        ALOGD("%s(), mSMNROn = %d, set single mic setting", __FUNCTION__, mSMNROn);
        memcpy(&mSphParamNB, pSphParamNB, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        for (int speech_mode_index = 0; speech_mode_index < 8; speech_mode_index++)
        {
            (mSphParamNB.speech_mode_para[speech_mode_index][13]) = 0;
            (mSphParamNB.speech_mode_para[speech_mode_index][14]) = 0;

        }
    }


    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            if (mSMNROn != true)
            {
                pSpeechDriver->SetNBSpeechParameters(pSphParamNB);
            }
            else
            {
                pSpeechDriver->SetNBSpeechParameters(&mSphParamNB);
            }
        }
    }

    return NO_ERROR;
}


#if defined(MTK_DUAL_MIC_SUPPORT)
status_t SpeechEnhancementController::SetDualMicSpeechParametersToAllModem(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetDualMicSpeechParameters(pSphParamDualMic);
        }
    }

    return NO_ERROR;
}
#endif

#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
status_t SpeechEnhancementController::SetMagiConSpeechParametersToAllModem(const AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetMagiConSpeechParameters(pSphParamMagiCon);
        }
    }

    return NO_ERROR;
}
#endif

#if defined(MTK_HAC_SUPPORT)
status_t SpeechEnhancementController::SetHACSpeechParametersToAllModem(const AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetHACSpeechParameters(pSphParamHAC);
        }
    }

    return NO_ERROR;
}
#endif


#if defined(MTK_WB_SPEECH_SUPPORT)
status_t SpeechEnhancementController::SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;
    AUDIO_CUSTOM_WB_PARAM_STRUCT mSphParamWB;

    if (mSMNROn == true)
    {
        //forcely set single mic setting
        ALOGD("%s(), mSMNROn = %d, set single mic setting", __FUNCTION__, mSMNROn);
        memcpy(&mSphParamWB, pSphParamWB, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));

        for (int speech_mode_index = 0; speech_mode_index < 8; speech_mode_index++)
        {
            (mSphParamWB.speech_mode_wb_para[speech_mode_index][13]) = 0;
            (mSphParamWB.speech_mode_wb_para[speech_mode_index][14]) = 0;
        }
    }

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            if (mSMNROn != true)
            {
                pSpeechDriver->SetWBSpeechParameters(pSphParamWB);
            }
            else
            {
                pSpeechDriver->SetWBSpeechParameters(&mSphParamWB);
            }
        }
    }

    return NO_ERROR;
}
#endif


status_t SpeechEnhancementController::SetSpeechEnhancementMaskToAllModem(const sph_enh_mask_struct_t &mask)
{
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "0x%x 0x%x", mask.main_func, mask.dynamic_func);
    property_set(PROPERTY_KEY_SPH_ENH_MASKS, property_value);

    mSpeechEnhancementMask = mask;

    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetSpeechEnhancementMask(mSpeechEnhancementMask);
        }
    }

    return NO_ERROR;
}


status_t SpeechEnhancementController::SetDynamicMaskOnToAllModem(const sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on)
{
    sph_enh_mask_struct_t mask = GetSpeechEnhancementMask();

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

    return SetSpeechEnhancementMaskToAllModem(mask);
}

void SpeechEnhancementController::SetMagicConferenceCallOn(const bool magic_conference_call_on)
{
    ALOGD("%s(), mMagicConferenceCallOn = %d, new magic_conference_call_on = %d",
          __FUNCTION__, mMagicConferenceCallOn, magic_conference_call_on);
    property_set(PROPERTY_KEY_MAGIC_CON_CALL_ON, (magic_conference_call_on == false) ? "0" : "1");

    //always set
    mMagicConferenceCallOn = magic_conference_call_on;

    SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, mMagicConferenceCallOn);

}

void SpeechEnhancementController::SetHACOn(const bool hac_on)
{
    ALOGD("%s(), hac_on = %d, new hac_on = %d",
          __FUNCTION__, mHACOn, hac_on);
    property_set(PROPERTY_KEY_HAC_ON, (hac_on == false) ? "0" : "1");

    mHACOn = hac_on;

}

status_t SpeechEnhancementController::SetBtHeadsetNrecOnMaskOn(sph_enh_mask_struct_t* ori_mask, const sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on)
{
    const bool current_flag_on = (((*ori_mask).dynamic_func & dynamic_mask_type) > 0);
    if (new_flag_on == current_flag_on)
    {
        ALOGD("%s(), dynamic_mask_type(%x), new_flag_on(%d) == current_flag_on(%d), return",
              __FUNCTION__, dynamic_mask_type, new_flag_on, current_flag_on);
        return NO_ERROR;
    }

    if (new_flag_on == false)
    {
        (*ori_mask).dynamic_func &= (~dynamic_mask_type);
    }
    else
    {
        (*ori_mask).dynamic_func |= dynamic_mask_type;
    }
    ALOGD("-%s(), dynamic_mask_type(%x), new_flag_on(%d) == current_flag_on(%d), return",
          __FUNCTION__, dynamic_mask_type, new_flag_on, current_flag_on);

return NO_ERROR;
}

sph_enh_mask_struct_t SpeechEnhancementController::GetNRECMask(const bool bNrecOn, bool* bNrecSwitchNeed) 
{ 
    sph_enh_mask_struct_t mask = GetSpeechEnhancementMask();

    SetBtHeadsetNrecOnMaskOn(&mask, SPH_ENH_DYNAMIC_MASK_DLNR, bNrecOn);
    SetBtHeadsetNrecOnMaskOn(&mask, SPH_ENH_DYNAMIC_MASK_ULNR, bNrecOn);
    SetBtHeadsetNrecOnMaskOn(&mask, SPH_ENH_DYNAMIC_MASK_AEC, bNrecOn);
    //update current mask 
    
    if(mSpeechEnhancementMask.dynamic_func != mask.dynamic_func)
    {
        mSpeechEnhancementMask = mask;
        *bNrecSwitchNeed = true;
        ALOGD("-%s(), bNrecOn(%d), bNrecSwitchNeed(%d) , update", __FUNCTION__, bNrecOn, *bNrecSwitchNeed);
    }
    else
    {
        *bNrecSwitchNeed = false;
        ALOGD("-%s(), bNrecOn(%d), bNrecSwitchNeed(%d) , skip", __FUNCTION__, bNrecOn, *bNrecSwitchNeed);
    }
    return mask; 
}

void SpeechEnhancementController::SetBtHeadsetNrecOnToAllModem(const bool bt_headset_nrec_on)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    property_set(PROPERTY_KEY_BT_HEADSET_NREC_ON, (bt_headset_nrec_on == false) ? "0" : "1");
    mBtHeadsetNrecOn = bt_headset_nrec_on;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetBtHeadsetNrecOn(mBtHeadsetNrecOn);
        }
    }
}

void SpeechEnhancementController::SetSMNROn(void)
{
    mSMNROn = true;
    ALOGD("%s(), mSMNROn = %d", __FUNCTION__, mSMNROn);
}



}
