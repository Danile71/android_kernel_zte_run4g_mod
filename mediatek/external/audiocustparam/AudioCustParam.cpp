/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioCustParam.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements customized parameter handling
 *
 * Author:
 * -------
 *   HP Cheng (mtk01752)
 *
 *------------------------------------------------------------------------------
 * $Revision: #2 $
 * $Modtime:$
 * $Log:$
 *
 * 06 05 2013 donglei.ji
 * [ALPS00683353] [Need Patch] [Volunteer Patch] DMNR3.0 and VOIP tuning check in
 * .
 *
 * 12 29 2012 donglei.ji
 * [ALPS00425279] [Need Patch] [Volunteer Patch] voice ui and password unlock feature check in
 * voice ui - NVRAM .
 *
 *
 *******************************************************************************/

/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "unistd.h"
#include "pthread.h"
#endif

#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <sys/types.h>

#include "CFG_AUDIO_File.h"
#include "CFG_file_lid.h"//AP_CFG_RESERVED_1 for AudEnh
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "CFG_Audio_Default.h"
#include <cutils/properties.h>
#include "AudioCustParam.h"
#include "nvram_agent_client.h"

static Mutex mLock;

//#define USE_DEFAULT_CUST_TABLE    //For BringUp usage, use default value, should disable when NVRAM ready

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioCustParam"

#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#else // no HAVE_AEE_FEATURE
#ifndef DB_OPT_DEFAULT
#define DB_OPT_DEFAULT (0)
#endif
static int aee_system_exception(const char *module, const char *path, unsigned int flags, const char *msg, ...)
{
    return 0;
}
static int aee_system_warning(const char *module, const char *path, unsigned int flags, const char *msg, ...)
{
    return 0;
}
#endif // end of HAVE_AEE_FEATURE


#define ASSERT(exp) \
    do { \
        if (!(exp)) { \
            ALOGE("ASSERT("#exp") fail: \""  __FILE__ "\", %uL", __LINE__); \
            aee_system_exception("[Audio]", NULL, DB_OPT_DEFAULT, " %s, %uL", strrchr(__FILE__, '/') + 1, __LINE__); \
        } \
    } while(0)

#define WARNING(string) \
    do { \
        ALOGW("WARNING("string") fail: \""  __FILE__ "\", %uL", __LINE__); \
        aee_system_warning("[Audio]", NULL, DB_OPT_DEFAULT, string); \
    } while(0)


namespace android
{

/*=============================================================================
 *                             Private Function
 *===========================================================================*/

static bool checkNvramReady(void)
{
    const uint32_t MAX_RETRY_COUNT = 30;
    uint32_t read_nvram_ready_retry = 0;

    bool ret = false;

    char nvram_init_val[PROPERTY_VALUE_MAX];
    while (read_nvram_ready_retry < MAX_RETRY_COUNT)
    {
        read_nvram_ready_retry++;
        property_get("nvram_init", nvram_init_val, NULL);
        if (strcmp(nvram_init_val, "Ready") == 0)
        {
            ret = true;
            break;
        }
        else
        {
            ALOGW("Get nvram restore ready retry cc=%d", read_nvram_ready_retry);
            usleep(500 * 1000);
        }
    }

    if (read_nvram_ready_retry >= MAX_RETRY_COUNT)
    {
        ALOGE("Get nvram restore ready faild !!!");
        ret = false;
    }

    return ret;
}

/*=============================================================================
 *                             Public Function
 *===========================================================================*/

static bool gUseBinderToAccessNVRam = true;

void setUseBinderToAccessNVRam(const bool use_binder_to_access_nvram)
{
    ALOGD("%s(), gUseBinderToAccessNVRam: %d => %d", __FUNCTION__, gUseBinderToAccessNVRam, use_binder_to_access_nvram);
    gUseBinderToAccessNVRam = use_binder_to_access_nvram;
}

int audioReadNVRamFile(int file_lid, void *pBuf)
{
    ALOGD("%s(), file_lid = %d, pBuf = %p, gUseBinderToAccessNVRam = %d", __FUNCTION__, file_lid, pBuf, gUseBinderToAccessNVRam);
    Mutex::Autolock _l(mLock);

    if (pBuf == NULL)
    {
        ALOGE("%s(), pBuf == NULL, return 0", __FUNCTION__);
        return 0;
    }

    if (checkNvramReady() == false)
    {
        ALOGE("%s(), checkNvramReady() == false, return 0", __FUNCTION__);
        return 0;
    }

    int result = 0;
    int read_size = 0;
    if (gUseBinderToAccessNVRam == true)
    {
        NvRAMAgentClient *NvRAMClient = NvRAMAgentClient::create();
        if (NvRAMClient == NULL)
        {
            ALOGE("%s(), NvRAMClient == NULL", __FUNCTION__);
            result = 0;
        }
        else
        {
            char *data = NvRAMClient->readFile(file_lid, read_size);
            if (data == NULL)
            {
                ALOGE("%s(), data == NULL", __FUNCTION__);
                result = 0;
            }
            else
            {
                ALOGD("%s(), data = %p, read_size = %d", __FUNCTION__, data, read_size);
                memcpy(pBuf, data, read_size);
                free(data);
                result = read_size;
            }
            delete NvRAMClient;
        }
    }
    else
    {
        int rec_size = 0;
        int rec_num = 0;

        F_ID audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        read_size = rec_size * rec_num;
        ALOGD("%s(), audio_nvram_fd.iFileDesc = %d, rec_size = %d, rec_num = %d, read_size = %d",
              __FUNCTION__, audio_nvram_fd.iFileDesc, rec_size, rec_num, read_size);

        memset(pBuf, 0, read_size);
        result = read(audio_nvram_fd.iFileDesc, pBuf, read_size);
        if (result != read_size)
        {
            ALOGE("%s(), result(%d) != read_size(%d), return 0", __FUNCTION__, result, read_size);
            result = 0;
        }

        NVM_CloseFileDesc(audio_nvram_fd);
    }

    return result;
}

int audioWriteNVRamFile(int file_lid, void *pBuf)
{
    ALOGD("%s(), file_lid = %d, pBuf = %p, gUseBinderToAccessNVRam = %d", __FUNCTION__, file_lid, pBuf, gUseBinderToAccessNVRam);
    Mutex::Autolock _l(mLock);

    if (pBuf == NULL)
    {
        ALOGE("%s(), pBuf == NULL, return 0", __FUNCTION__);
        return 0;
    }

    if (checkNvramReady() == false)
    {
        ALOGE("%s(), checkNvramReady() == false, return 0", __FUNCTION__);
        return 0;
    }

    int result = 0;
    int rec_size = 0;
    int rec_num = 0;
    int write_size = 0;
    if (gUseBinderToAccessNVRam == true)
    {
        NvRAMAgentClient *NvRAMClient = NvRAMAgentClient::create();
        if (NvRAMClient == NULL)
        {
            ALOGE("%s(), NvRAMClient == NULL", __FUNCTION__);
            result = 0;
        }
        else
        {
            result = NvRAMClient->getFileDesSize(file_lid, rec_size, rec_num);

            if (result == false)
            {
                ALOGE("%s(), getFileDesSize == false", __FUNCTION__);
                result = 0;
            }
            else
            {
                write_size = rec_size * rec_num;
                ALOGD("%s(), result = %d, rec_size = %d, rec_num = %d, write_size = %d",
                      __FUNCTION__, result, rec_size, rec_num, write_size);

                result = NvRAMClient->writeFile(file_lid, write_size, (char *)pBuf);
            }
            delete NvRAMClient;
        }
    }
    else
    {
        F_ID audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        write_size = rec_size * rec_num;
        ALOGD("%s(), audio_nvram_fd.iFileDesc = %d, rec_size = %d, rec_num = %d, write_size = %d",
              __FUNCTION__, audio_nvram_fd.iFileDesc, rec_size, rec_num, write_size);

        result = write(audio_nvram_fd.iFileDesc, pBuf, write_size);

        NVM_CloseFileDesc(audio_nvram_fd);
    }

    if (write_size != 0 && result != write_size)
    {
        ALOGE("%s(), result(%d) != read_size(%d), return 0", __FUNCTION__, result, write_size);
        result = 0;
    }

    return result;
}

uint32_t QueryFeatureSupportInfo()
{
    uint32_t RetInfo = 0;
    bool bForceEnable = false;
    bool bDUAL_MIC_SUPPORT = false;
    bool bVOIP_ENHANCEMENT_SUPPORT = false;
    bool bASR_SUPPORT = false;
    bool bVOIP_NORMAL_DMNR_SUPPORT = false;
    bool bVOIP_HANDSFREE_DMNR_SUPPORT = false;
    bool bINCALL_NORMAL_DMNR_SUPPORT = false;
    bool bINCALL_HANDSFREE_DMNR_SUPPORT = false;
    bool bNoReceiver = false;
    bool bNoSpeech = false;
    bool bWifiOnly = false;
    bool b3GDATAOnly = false;
    bool bWideBand = false;
    bool bHDRecord = false;
    bool bDMNR_3_0 = false;
    bool bDMNRTuningAtModem = false;
    bool bVoiceUnlock = false;
    bool bDMNR_COMPLEX_ARCH_SUPPORT = false;
    bool bGET_FO = false;
    bool bSpeakerMonitor = false;
	bool bSupportBesloudnessV5 = false;
    bool bMagiConference = false;
    bool bHAC = false;

#ifdef MTK_DUAL_MIC_SUPPORT
    bDUAL_MIC_SUPPORT = true;
#endif

#ifdef MTK_VOIP_ENHANCEMENT_SUPPORT
    bVOIP_ENHANCEMENT_SUPPORT = true;
#endif

#ifdef MTK_ASR_SUPPORT
    bASR_SUPPORT = true;
#endif

#ifdef MTK_VOIP_NORMAL_DMNR
    bVOIP_NORMAL_DMNR_SUPPORT = true;
#endif

#ifdef MTK_VOIP_HANDSFREE_DMNR
    bVOIP_HANDSFREE_DMNR_SUPPORT = true;
#endif

#ifdef MTK_INCALL_HANDSFREE_DMNR
    bINCALL_HANDSFREE_DMNR_SUPPORT = true;
#endif

#ifdef MTK_INCALL_NORMAL_DMNR
    bINCALL_NORMAL_DMNR_SUPPORT = true;
#endif

#ifdef MTK_DISABLE_EARPIECE  // DISABLE_EARPIECE
    bNoReceiver = true;
#endif

#ifdef MTK_WIFI_ONLY_SUPPORT
    bWifiOnly = true;
#endif

#ifdef MTK_3G_DATA_ONLY_SUPPORT
    b3GDATAOnly = true;
#endif

#ifdef MTK_WB_SPEECH_SUPPORT
    bWideBand = true;
#endif

#ifdef MTK_AUDIO_HD_REC_SUPPORT
    bHDRecord = true;
#endif

#ifdef MTK_HANDSFREE_DMNR_SUPPORT
    bDMNR_3_0 = true;
#endif

#ifdef DMNR_TUNNING_AT_MODEMSIDE
    bDMNRTuningAtModem = true;
#endif

#if defined(MTK_VOICE_UNLOCK_SUPPORT) || defined(MTK_VOICE_UI_SUPPORT)
    bVoiceUnlock = true;
#endif

#ifdef DMNR_COMPLEX_ARCH_SUPPORT
    bDMNR_COMPLEX_ARCH_SUPPORT = true;
#endif
#ifdef MTK_ACF_AUTO_GEN_SUPPORT
    bGET_FO = true;
#endif
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
    bSpeakerMonitor = true;
#endif

#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V5)
    bSupportBesloudnessV5 = true;
#endif
#if defined(MTK_MAGICONFERENCE_SUPPORT)
	bMagiConference = true;
#endif

#if defined(MTK_HAC_SUPPORT)
	bHAC = true;
#endif


    if (bWifiOnly || b3GDATAOnly)
    {
        bNoSpeech = true;
    }



    // SUPPORT_WB_SPEECH
    if (bWideBand)
    {
        RetInfo = RetInfo | SUPPORT_WB_SPEECH;
    }

    // SUPPORT_DUAL_MIC
    if (bDUAL_MIC_SUPPORT)
    {
        RetInfo = RetInfo | SUPPORT_DUAL_MIC;
    }

    // SUPPORT_HD_RECORD
    if (bHDRecord)
    {
        RetInfo = RetInfo | SUPPORT_HD_RECORD;
    }

    // SUPPORT_DMNR_3_0
    if (bDMNR_3_0)
    {
        RetInfo = RetInfo | SUPPORT_DMNR_3_0;
    }

    //SUPPORT_DMNR_AT_MODEM
    if (bDMNRTuningAtModem)
    {
        RetInfo = RetInfo | SUPPORT_DMNR_AT_MODEM;
    }

    //SUPPORT_VOIP_ENHANCE
    if (bVOIP_ENHANCEMENT_SUPPORT)
    {
        RetInfo = RetInfo | SUPPORT_VOIP_ENHANCE;
    }

    //SUPPORT_WIFI_ONLY
    if (bWifiOnly)
    {
        RetInfo = RetInfo | SUPPORT_WIFI_ONLY;
    }

    //SUPPORT_3G_DATA
    if (b3GDATAOnly)
    {
        RetInfo = RetInfo | SUPPORT_3G_DATA;
    }

    //SUPPORT_NO_RECEIVER
    if (bNoReceiver)
    {
        RetInfo = RetInfo | SUPPORT_NO_RECEIVER;
    }

    //SUPPORT_ASR
    if (bDUAL_MIC_SUPPORT && (bASR_SUPPORT || bForceEnable))
    {
        RetInfo = RetInfo | SUPPORT_ASR;
    }

    //SUPPORT_VOIP_NORMAL_DMNR
    if (!bNoReceiver)
    {
        if (bDUAL_MIC_SUPPORT && ((bVOIP_NORMAL_DMNR_SUPPORT && bVOIP_ENHANCEMENT_SUPPORT) || bForceEnable))
        {
            RetInfo = RetInfo | SUPPORT_VOIP_NORMAL_DMNR;
        }
    }

    //SUPPORT_VOIP_HANDSFREE_DMNR
    if (bDUAL_MIC_SUPPORT && ((bVOIP_HANDSFREE_DMNR_SUPPORT && bVOIP_ENHANCEMENT_SUPPORT) || bForceEnable))
    {
        RetInfo = RetInfo | SUPPORT_VOIP_HANDSFREE_DMNR;
    }

    //SUPPORT_INCALL_NORMAL_DMNR
    if (!bNoReceiver && !bNoSpeech)
    {
        if (bDUAL_MIC_SUPPORT && (bINCALL_NORMAL_DMNR_SUPPORT || bForceEnable))
        {
            RetInfo = RetInfo | SUPPORT_INCALL_NORMAL_DMNR;
        }
    }

    //SUPPORT_INCALL_HANDSFREE_DMNR
    if (!bNoSpeech)
    {
        if (bDUAL_MIC_SUPPORT && (bINCALL_HANDSFREE_DMNR_SUPPORT || bForceEnable))
        {
            RetInfo = RetInfo | SUPPORT_INCALL_HANDSFREE_DMNR;
        }
    }

    //SUPPORT_VOICE_UNLOCK
    if (bVoiceUnlock)
    {
        RetInfo = RetInfo | SUPPORT_VOICE_UNLOCK;
    }

    //DMNR_COMPLEX_ARCH_SUPPORT
    if (bDMNR_COMPLEX_ARCH_SUPPORT)
    {
        RetInfo = RetInfo | SUPPORT_DMNR_COMPLEX_ARCH;
    }

    if (bGET_FO)
    {
        RetInfo = RetInfo | SUPPORT_GET_FO_VALUE;
    }
    if (bSpeakerMonitor)
    {
        RetInfo = RetInfo | SUPPORT_SPEAKER_MONITOR;
    }

    if (bSupportBesloudnessV5)
    {
        RetInfo = RetInfo | SUPPORT_BESLOUDNESS_V5;
    }

    if (bMagiConference)
    {
        RetInfo = RetInfo | SUPPORT_MAGI_CONFERENCE;
    }
    if (bHAC)
    {
        RetInfo = RetInfo | SUPPORT_HAC;
    }
    ALOGD("%s(),feature support %x ", __FUNCTION__, RetInfo);
    return RetInfo;
}

int getDefaultSpeechParam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    // only for startup use
    ALOGW("Digi_DL_Speech = %u", speech_custom_default.Digi_DL_Speech);
    ALOGW("uMicbiasVolt = %u", speech_custom_default.uMicbiasVolt);
    ALOGW("sizeof AUDIO_CUSTOM_PARAM_STRUCT = %d", sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
    memcpy((void *)pSphParamNB, (void *)&speech_custom_default, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
    return sizeof(AUDIO_CUSTOM_PARAM_STRUCT);
}

int GetCustParamFromNV(AUDIO_CUSTOM_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultSpeechParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_LID, (void *)pPara);

    if (result != sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        result = getDefaultSpeechParam(pPara);
        ASSERT(0);
    }

#endif

    ALOGD("GetCustParamFromNV uMicbiasVolt = %d", pPara->uMicbiasVolt);

    return result;
}

int SetCustParamToNV(AUDIO_CUSTOM_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_LID, pPara);
#endif
    return result;
}

int getDefaultVer1VolumeParam(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *volume_param)
{
    // only for startup use
    ALOGD("getDefaultVer1VolumeParam");
    memcpy((void *)volume_param, (void *)(&audio_ver1_custom_default), sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT));
    return sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT);
}

// functions
int GetVolumeVer1ParamFromNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultVer1VolumeParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_VER1_VOLUME_CUSTOM_LID, (void *)pPara);

    if (result != sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT));
        result = getDefaultVer1VolumeParam(pPara);
        ASSERT(0);
    }
#endif
    return result;
}

int SetVolumeVer1ParamToNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_VER1_VOLUME_CUSTOM_LID, pPara);
#endif
    return result;
}


int GetNBSpeechParamFromNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultSpeechParam(pSphParamNB);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_LID, (void *)pSphParamNB);

    if (result != sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        result = getDefaultSpeechParam(pSphParamNB);
        ASSERT(0);
    }

#endif

    ALOGD("%s(), uMicbiasVolt = %d", __FUNCTION__, pSphParamNB->uMicbiasVolt);
    return result;
}

int SetNBSpeechParamToNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_LID, pSphParamNB);
#endif
    return result;
}

int getDefaultAudioGainTableParam(AUDIO_GAIN_TABLE_STRUCT *sphParam)
{
    ALOGW("sizeof AUDIO_GAIN_TABLE_STRUCT = %d", sizeof(AUDIO_GAIN_TABLE_STRUCT));
    memcpy((void *)sphParam, (void *) & (Gain_control_table_default), sizeof(AUDIO_GAIN_TABLE_STRUCT));
    return sizeof(AUDIO_GAIN_TABLE_STRUCT);
}

int GetAudioGainTableParamFromNV(AUDIO_GAIN_TABLE_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudioGainTableParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID, (void *)pPara);

    if (result != sizeof(AUDIO_GAIN_TABLE_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_GAIN_TABLE_STRUCT));
        result = getDefaultAudioGainTableParam(pPara);
        ASSERT(0);
    }

#endif
    return result;
}

int SetAudioGainTableParamToNV(AUDIO_GAIN_TABLE_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID, pPara);
#endif
    return result;
}


int getDefaultWBSpeechParam(AUDIO_CUSTOM_WB_PARAM_STRUCT *sphParam)
{
    ALOGW("sizeof AUDIO_CUSTOM_WB_PARAM_STRUCT = %d", sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
    memcpy((void *)sphParam, (void *) & (wb_speech_custom_default), sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
    return sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT);
}

int GetCustWBParamFromNV(AUDIO_CUSTOM_WB_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultWBSpeechParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID, pPara);

    if (result != sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        result = getDefaultWBSpeechParam(pPara);
        ASSERT(0);
    }
#endif
    return result;
}

int SetCustWBParamToNV(AUDIO_CUSTOM_WB_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID, pPara);
#endif
    return result;
}


int GetWBSpeechParamFromNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultWBSpeechParam(pSphParamWB);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID, (void *)pSphParamWB);

    if (result != sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        result = getDefaultWBSpeechParam(pSphParamWB);
        ASSERT(0);
    }

#endif
    return result;
}

int SetWBSpeechParamToNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID, pPara);
#endif
    return result;
}

int getDefaultMedParam(AUDIO_PARAM_MED_STRUCT *pPara)
{
    // only for startup use
    ALOGW("sizeof AUDIO_PARAM_MED_STRUCT = %d", sizeof(AUDIO_PARAM_MED_STRUCT));
    memcpy((void *)pPara, (void *) & (audio_param_med_default), sizeof(AUDIO_PARAM_MED_STRUCT));
    return sizeof(AUDIO_PARAM_MED_STRUCT);
}

int GetMedParamFromNV(AUDIO_PARAM_MED_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultMedParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID, (void *)pPara);

    if (result != sizeof(AUDIO_PARAM_MED_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_PARAM_MED_STRUCT));
        result = getDefaultMedParam(pPara);
        ASSERT(0);
    }
#endif
    return result;
}

int SetMedParamToNV(AUDIO_PARAM_MED_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID, pPara);
#endif
    return result;
}

int getDefaultAudioCustomParam(AUDIO_VOLUME_CUSTOM_STRUCT *volParam)
{
    // only for startup use
    ALOGW("sizeof AUDIO_VOLUME_CUSTOM_STRUCT = %d", sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
    memcpy((void *)volParam, (void *) & (audio_volume_custom_default), sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
    return sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
}

// get audio custom parameter from NVRAM
int GetAudioCustomParamFromNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudioCustomParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID, (void *)pPara);

    if (result != sizeof(AUDIO_VOLUME_CUSTOM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
        result = getDefaultAudioCustomParam(pPara);
        ASSERT(0);
    }

#endif
    return result;
}

int SetAudioCustomParamToNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID, pPara);
#endif
    return result;
}



//////////////////////////////////////////////
// Dual Mic Custom Parameter
//////////////////////////////////////////////

int getDefaultDualMicParam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *dualMicParam)
{
    ALOGD("sizeof AUDIO_CUSTOM_PARAM_STRUCT = %d", sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
    memcpy((void *)dualMicParam, (void *) & (dual_mic_custom_default), sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
    return sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT);
}

// Get Dual Mic Custom Parameter from NVRAM
int Read_DualMic_CustomParam_From_NVRAM(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultDualMicParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID, (void *)pPara);

    if (result != sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        result = getDefaultDualMicParam(pPara);
        ASSERT(0);
    }

#endif
    return result;
}

// Set Dual Mic Custom Parameter from NVRAM
int Write_DualMic_CustomParam_To_NVRAM(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID, pPara);
#endif
    return result;
}

// Get Dual Mic Custom Parameter from NVRAM
int GetDualMicSpeechParamFromNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultDualMicParam(pSphParamDualMic);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID, (void *)pSphParamDualMic);

    if (result != sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        result = getDefaultDualMicParam(pSphParamDualMic);
        ASSERT(0);
    }

#endif
    return result;
}

// Set Dual Mic Custom Parameter from NVRAM
int SetDualMicSpeechParamToNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID, pPara);
#endif
    return result;
}

//////////////////////////////////////////////
// HD Record Custom Parameter
//////////////////////////////////////////////
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
int getDefaultHdRecordParam(AUDIO_HD_RECORD_PARAM_STRUCT *pPara)
{
    ALOGD("sizeof AUDIO_HD_RECORD_PARAM_STRUCT = %d", sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
    memcpy((void *)pPara, (void *) & (Hd_Recrod_Par_default), sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
    return sizeof(AUDIO_HD_RECORD_PARAM_STRUCT);
}

/// Get HD record parameters from NVRAM
int GetHdRecordParamFromNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultHdRecordParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID, (void *)pPara);

    if (result != sizeof(AUDIO_HD_RECORD_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
        result = getDefaultHdRecordParam(pPara);
        ASSERT(0);
    }

#endif
    return result;
}

/// Set HD record parameters to NVRAM
int SetHdRecordParamToNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID, pPara);
#endif
    return result;
}

int getDefaultHdRecordSceneTable(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara)
{
    ALOGD("sizeof AUDIO_HD_RECORD_SCENE_TABLE_STRUCT = %d", sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
    memcpy((void *)pPara, (void *) & (Hd_Recrod_Scene_Table_default), sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
    return sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT);
}

/// Get HD record scene tables from NVRAM
int GetHdRecordSceneTableFromNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultHdRecordSceneTable(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID, (void *)pPara);

    if (result != sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
        result = getDefaultHdRecordSceneTable(pPara);
        ASSERT(0);
    }
#endif
    return result;
}

/// Set HD record scene tables to NVRAM
int SetHdRecordSceneTableToNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID, pPara);
#endif
    return result;
}

int getDefaultHdRecord48kParam(AUDIO_HD_RECORD_48K_PARAM_STRUCT *pPara)
{
    ALOGD("sizeof AUDIO_HD_RECORD_48K_PARAM_STRUCT = %d", sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT));
    memcpy((void *)pPara, (void *) & (Hd_Recrod_48k_Par_default), sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT));
    return sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT);
}


int GetHdRecord48kParamFromNV(AUDIO_HD_RECORD_48K_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultHdRecord48kParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HD_REC_48K_PAR_LID, (void *)pPara);

    if (result != sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT));
        result = getDefaultHdRecord48kParam(pPara);
        ASSERT(0);
    }
#endif
    return result;
}

int SetHdRecord48kParamToNV(AUDIO_HD_RECORD_48K_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HD_REC_48K_PAR_LID, pPara);
#endif
    return result;
}
#endif // end of MTK_AUDIO_HD_REC_SUPPORT


int getDefaultVoiceRecogCustParam(VOICE_RECOGNITION_PARAM_STRUCT *pPara)
{
    ALOGD("sizeof VOICE_RECOGNITION_PARAM_STRUCT = %d", sizeof(VOICE_RECOGNITION_PARAM_STRUCT));
    memcpy((void *)pPara, (void *) & (Voice_Recognize_Par_default), sizeof(VOICE_RECOGNITION_PARAM_STRUCT));
    return sizeof(VOICE_RECOGNITION_PARAM_STRUCT);
}

// Get voice revognition customization parameters
int GetVoiceRecogCustParamFromNV(VOICE_RECOGNITION_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultVoiceRecogCustParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_VOICE_RECOGNIZE_PARAM_LID, (void *)pPara);

    if (result != sizeof(VOICE_RECOGNITION_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(VOICE_RECOGNITION_PARAM_STRUCT));
        result = getDefaultVoiceRecogCustParam(pPara);
        ASSERT(0);
    }

#endif
    return result;
}

// Set voice revognition customization parameters
int SetVoiceRecogCustParamToNV(VOICE_RECOGNITION_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_VOICE_RECOGNIZE_PARAM_LID, pPara);
#endif
    return result;
}

// Get VOW customization paramtetrs
int GetVOWCustParamFromNV(int index)
{
    int vow_index = 5;
    int result    = 0;
    if ((index >= VOICE_RECOG_PARAM_NUM_MAX) || (index < 0))
    {
        ALOGE("%s wrong index %d", __FUNCTION__, index);
        result = -1;
    }
    else
    {
        VOICE_RECOGNITION_PARAM_STRUCT param_VR;
        GetVoiceRecogCustParamFromNV(&param_VR);
        result = param_VR.cust_param[vow_index][index];
    }
    return result;
}

// Set VOW customization paramtetrs
int SetVOWCustParamToNV(int index, int value)
{
    int vow_index = 5;
    int result    = 0;
    if ((index >= VOICE_RECOG_PARAM_NUM_MAX) || (index < 0) || (value > 127) || (value < -128))
    {
        ALOGE("%s wrong index/value(%d/%d)", __FUNCTION__, index, value);
        result = -1;
    }
    else
    {
        VOICE_RECOGNITION_PARAM_STRUCT param_VR;
        GetVoiceRecogCustParamFromNV(&param_VR);
        param_VR.cust_param[vow_index][index] = value;
        SetVoiceRecogCustParamToNV(&param_VR);
    }
    return result;
}

int getDefaultAudEnhControlOptionParam(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara)
{
    ALOGD("sizeof AUDIO_AUDENH_CONTROL_OPTION_STRUCT = %d", sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT));
    memcpy((void *)pPara, (void *) & (AUDENH_Control_Option_Par_default), sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT));
    return sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT);
}

int GetAudEnhControlOptionParamFromNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudEnhControlOptionParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, (void *)pPara);

    if (result != sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT));
        result = getDefaultAudEnhControlOptionParam(pPara);
        ASSERT(0);
    }

#endif

    if (pPara)
    {
        pPara->u32EnableFlg = pPara->u32EnableFlg & 0x01 ? 1 : 0;
    }

    return result;
}

int SetAudEnhControlOptionParamToNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT stFinalWriteData;

    audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, (void *)&stFinalWriteData);
    stFinalWriteData.u32EnableFlg &= (~0x01);
    stFinalWriteData.u32EnableFlg |= (pPara->u32EnableFlg & 0x01);
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, &stFinalWriteData);
#endif
    return result;
}

int GetBesLoudnessControlOptionParamFromNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudEnhControlOptionParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, (void *)pPara);

    if (result != sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT));
        result = getDefaultAudEnhControlOptionParam(pPara);
        ASSERT(0);
    }

#endif

    if (pPara)
    {
        pPara->u32EnableFlg = pPara->u32EnableFlg & 0x02 ? 1 : 0;
    }

    return result;
}

int SetBesLoudnessControlOptionParamToNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT stFinalWriteData;
    audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, (void *)&stFinalWriteData);
    stFinalWriteData.u32EnableFlg &= (~0x02);
    stFinalWriteData.u32EnableFlg |= ((pPara->u32EnableFlg & 0x01) << 0x01);
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, &stFinalWriteData);
#endif
    return result;
}

int GetHiFiDACControlOptionParamFromNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudEnhControlOptionParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, (void *)pPara);

    if (result != sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_AUDENH_CONTROL_OPTION_STRUCT));
        result = getDefaultAudEnhControlOptionParam(pPara);
        ASSERT(0);
    }

#endif

    if (pPara)
    {
        pPara->u32EnableFlg = pPara->u32EnableFlg & 0x04 ? 1 : 0;    //bit2
    }

    return result;
}

int SetHiFiDACControlOptionParamToNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT stFinalWriteData;
    audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, (void *)&stFinalWriteData);
    stFinalWriteData.u32EnableFlg &= (~0x04);
    stFinalWriteData.u32EnableFlg |= ((pPara->u32EnableFlg & 0x01) << 2); //bit2
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID, &stFinalWriteData);
#endif
    return result;
}

int getDefaultDcCalibrationParam(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *pPara)
{
    ALOGD("sizeof AUDIO_BUFFER_DC_CALIBRATION_STRUCT = %d", sizeof(AUDIO_BUFFER_DC_CALIBRATION_STRUCT));
    memcpy((void *)pPara, (void *) & (Audio_Buffer_DC_Calibration_Par_default), sizeof(AUDIO_BUFFER_DC_CALIBRATION_STRUCT));
    return sizeof(AUDIO_BUFFER_DC_CALIBRATION_STRUCT);
}

int GetDcCalibrationParamFromNV(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultDcCalibrationParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_BUFFER_DC_CALIBRATION_PAR_LID, (void *)pPara);

    if (result != sizeof(AUDIO_BUFFER_DC_CALIBRATION_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_BUFFER_DC_CALIBRATION_STRUCT));
        result = getDefaultDcCalibrationParam(pPara);
        ASSERT(0);
    }

#endif

    bool bSupportFlg = false;
    char stForFeatureUsage[PROPERTY_VALUE_MAX];
    const char PROPERTY_KEY_FORCE_DC_CALI_ON[PROPERTY_KEY_MAX] = "persist.af.feature.forcedccali";
    property_get(PROPERTY_KEY_FORCE_DC_CALI_ON, stForFeatureUsage, "0"); //"0": default off
    bSupportFlg = (stForFeatureUsage[0] == '0') ? false : true;
    if (bSupportFlg && pPara)
    {
        pPara->cali_flag = 0xFFFF;
    }

    return result;
}

int SetDcCalibrationParamToNV(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_BUFFER_DC_CALIBRATION_PAR_LID, pPara);
#endif
    return result;
}

int getDefaultAudioVoIPParam(AUDIO_VOIP_PARAM_STRUCT *pPara)
{
    ALOGD("sizeof AUDIO_VOIP_PARAM_STRUCT = %d", sizeof(AUDIO_VOIP_PARAM_STRUCT));
    memcpy((void *)pPara, (void *) & (Audio_VOIP_Par_default), sizeof(AUDIO_VOIP_PARAM_STRUCT));
    return sizeof(AUDIO_VOIP_PARAM_STRUCT);
}


/// Get VoIP parameters from NVRAM
int GetAudioVoIPParamFromNV(AUDIO_VOIP_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudioVoIPParam(pPara);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID, (void *)pPara);

    if (result != sizeof(AUDIO_VOIP_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_VOIP_PARAM_STRUCT));
        result = getDefaultAudioVoIPParam(pPara);
        ASSERT(0);
    }
#endif
    return result;
}

/// Set VoIP parameters to NVRAM
int SetAudioVoIPParamToNV(AUDIO_VOIP_PARAM_STRUCT *pPara)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID, pPara);
#endif
    return result;
}

#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
// Get ANC parameters from NVRAM
int GetANCSpeechParamFromNVRam(AUDIO_ANC_CUSTOM_PARAM_STRUCT *pSphParamAnc)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_ANC_LID, (void *)pSphParamAnc);
    if (result != sizeof(AUDIO_ANC_CUSTOM_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_ANC_CUSTOM_PARAM_STRUCT));
        ASSERT(0);
    }
#endif
    return result;
}

// Set ANC parameters to NVRAM
int SetANCSpeechParamToNVRam(AUDIO_ANC_CUSTOM_PARAM_STRUCT *pSphParamAnc)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_ANC_LID, pSphParamAnc);
#endif
    return result;
}
#endif // end of MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT

// Get Magic Conference Call parameters from NVRAM
int GetMagiConSpeechParamFromNVRam(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_MAGI_CONFERENCE_LID, (void *)pSphParamMagiCon);
    if (result != sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
        ASSERT(0);
    }
#endif
#else
    ALOGW("-%s(), MagiConference not support!", __FUNCTION__);
#endif
    return result;
}

// Set Magic Conference Call parameters to NVRAM
int SetMagiConSpeechParamToNVRam(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_MAGI_CONFERENCE_LID, pSphParamMagiCon);
#endif
#else
    ALOGW("-%s(), MagiConference not support!", __FUNCTION__);
#endif
    return result;
}

// Get HAC parameters from NVRAM
int GetHACSpeechParamFromNVRam(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(MTK_HAC_SUPPORT)
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HAC_PARAM_LID, (void *)pSphParamHAC);
    if (result != sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
        ASSERT(0);
    }
#endif
#else
    ALOGW("-%s(), HAC not support!", __FUNCTION__);
#endif
    return result;
}

// Set HAC parameters to NVRAM
int SetHACSpeechParamToNVRam(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(MTK_HAC_SUPPORT)
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_HAC_PARAM_LID, pSphParamHAC);
#endif
#else
    ALOGW("-%s(), HAC not support!", __FUNCTION__);
#endif
    return result;
}


#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
int getDefaultSpeakerMonitorParam(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT *pPara)
{
    ALOGD("sizeof AUDIO_SPEAKER_MONITOR_PARAM_STRUCT = %d", sizeof(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT));
    memcpy((void *)pPara, (void *) & (speaker_monitor_par_default), sizeof(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT));
    return sizeof(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT);
}

int GetSpeakerMonitorParamFromNVRam(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT *pParam)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultSpeakerMonitorParam(pParam);
#else
    result = audioReadNVRamFile(AP_CFG_RDCL_FILE_AUDIO_SPEAKER_MONITOR_LID, (void *)pParam);

    if (result != sizeof(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT))
    {
        ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT));
        result = getDefaultSpeakerMonitorParam(pParam);
        ASSERT(0);
    }

#endif
    return result;
}

// Set Speaker Monitor parameters to NVRAM
int SetSpeakerMonitorParamToNVRam(AUDIO_SPEAKER_MONITOR_PARAM_STRUCT *pParam)
{
    ALOGD("%s()", __FUNCTION__);
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    result = audioWriteNVRamFile(AP_CFG_RDCL_FILE_AUDIO_SPEAKER_MONITOR_LID, pParam);
#endif
    return result;
}
#endif
}; // namespace android
