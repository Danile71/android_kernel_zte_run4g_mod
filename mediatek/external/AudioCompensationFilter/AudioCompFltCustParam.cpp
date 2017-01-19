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
 * AudioCompFltCustParam.cpp
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
 *   Tina Tsai (mtk01981)
 *
 *------------------------------------------------------------------------------
 * $Revision: #2 $
 * $Modtime:$
 * $Log:$
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

#include "CFG_AUDIO_File.h"
#include "Custom_NvRam_LID.h"
#include "AudioCompensationFilter.h"
#include "AudioCustParam.h"


//#define USE_DEFAULT_CUST_TABLE    //For BringUp usage



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioCompFltCustParam"

namespace android
{

#include "CFG_Audio_Default.h"
#include <cutils/properties.h>
#define MAX_RETRY_COUNT 20

/*=============================================================================
 *                             Public Function
 *===========================================================================*/


int getDefaultAudioCompFltParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam)
{
    int dDataSize;
    if (AUDIO_COMP_FLT_AUDIO == eFLTtype)
    {
        memcpy((void *)audioParam, (void *) & (audio_custom_default), sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dDataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    }
    else if (AUDIO_COMP_FLT_HEADPHONE == eFLTtype)
    {
        memcpy((void *)audioParam, (void *) & (audio_hcf_custom_default), sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dDataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    }
    else if (AUDIO_COMP_FLT_AUDENH == eFLTtype)
    {
        memset((void *)audioParam, 0x00, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dDataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    }
    else if (AUDIO_COMP_FLT_VIBSPK == eFLTtype)
    {
        memcpy((void *)audioParam, (void *) & (audio_vibspk_custom_default), sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dDataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    }
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)&& defined(MTK_STEREO_SPK_ACF_TUNING_SUPPORT) //means :92 above support
    else if (AUDIO_COMP_FLT_AUDIO_SUB == eFLTtype)
    {
        memcpy((void *)audioParam, (void *) & (audiosub_custom_default), sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dDataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    }
#endif
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V5)
    else if (AUDIO_COMP_FLT_DRC_FOR_MUSIC == eFLTtype)
    {
        memcpy((void *)audioParam, (void *) & (audio_musicdrc_custom_default), sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dDataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    }
    else if (AUDIO_COMP_FLT_DRC_FOR_RINGTONE == eFLTtype)
    {
        memcpy((void *)audioParam, (void *) & (audio_ringtonedrc_custom_default), sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dDataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    }
#endif
    else//Shouldn't happen
    {
        ;
        ASSERT(0);
    }

    return dDataSize;
}

int  GetAudioCompFltCustParamFromNV(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam)
{
    int result = 0;

#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudioCompFltParam(eFLTtype, audioParam);
#else
    int file_lid;
    int rec_sizem, rec_size, rec_num;
    if (AUDIO_COMP_FLT_AUDIO == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
    }
    else if (AUDIO_COMP_FLT_HEADPHONE == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID;
    }
    else if (AUDIO_COMP_FLT_VIBSPK == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_VIBSPK_COMPFLT_LID;
    }
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)&&defined(MTK_STEREO_SPK_ACF_TUNING_SUPPORT) //means :92 above support
    else if (AUDIO_COMP_FLT_AUDIO_SUB == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIOSUB_COMPFLT_LID;
    }
#endif
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V5) //means :95 above support
    else if (AUDIO_COMP_FLT_DRC_FOR_MUSIC == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_MUSIC_DRC_LID;
    }
    else if (AUDIO_COMP_FLT_DRC_FOR_RINGTONE == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_RINGTONE_DRC_LID;
    }
#endif
    else//Shouldn't happen
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
        ASSERT(0);
    }

    result = audioReadNVRamFile(file_lid, audioParam);
#endif
    return result;
}

int  SetAudioCompFltCustParamToNV(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam)
{
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
    // write to NV ram
    int file_lid;
    int rec_sizem, rec_size, rec_num;

    if (AUDIO_COMP_FLT_AUDIO == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
    }
    else if (AUDIO_COMP_FLT_HEADPHONE == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID;
    }
    else if (AUDIO_COMP_FLT_VIBSPK == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_VIBSPK_COMPFLT_LID;
    }
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)&& defined(MTK_STEREO_SPK_ACF_TUNING_SUPPORT)//means :92 above support

    else if (AUDIO_COMP_FLT_AUDIO_SUB == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIOSUB_COMPFLT_LID;
    }
#endif
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V5)//means :95 above support
    else if (AUDIO_COMP_FLT_DRC_FOR_MUSIC == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_MUSIC_DRC_LID;
    }
    else if (AUDIO_COMP_FLT_DRC_FOR_RINGTONE == eFLTtype)
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_RINGTONE_DRC_LID;
    }
#endif
    else//Shouldn't happen
    {
        file_lid = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
        ASSERT(0);
    }

    result = audioWriteNVRamFile(file_lid, audioParam);
#endif
    return result;
}


}; // namespace android
