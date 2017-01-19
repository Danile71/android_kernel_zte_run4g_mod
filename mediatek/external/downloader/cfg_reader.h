/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2014
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
* Filename:
* ---------
*  cfg_reader.h
*
* Project:
* --------
*   FP Native Downloader
*
* Description:
* ------------
*   This module contains CFG reader header
*
* Author:
* -------
*  Roger Lo (mtk03685)
*
*==============================================================================*/

#ifndef _CFG_READER_H
#define _CFG_READER_H

#include "download_images.h"

#define MAX_ROM_FILE_NUM 16
#define MAX_FILE_PATH_LEN 256

// For BootRegion, ControlBlockRegion, MainRegion
typedef struct RegionSetting
{
    unsigned int    romNumber;  // rom number
    char            *romFiles[MAX_ROM_FILE_NUM]; // char pointer to rom name
} RegionSetting;

// For Linux Partion Region
typedef struct ParitionSetting
{
    unsigned int    romNumber;  // rom number
    char            *romFiles[MAX_ROM_FILE_NUM]; // char pointer to rom name
    unsigned int    address[MAX_ROM_FILE_NUM];
} ParitionSetting;


typedef struct CFG_Images_Name
{
    RegionSetting BootRegion;
    RegionSetting ControlBlockRegion;
    RegionSetting MainRegion;
    ParitionSetting LinuxRegion;
} CFG_Images_Name;


int ParseConfigFile(const char* p_cfgpath, CFG_Images_Name *cfgImagesName, ExternalMemorySetting *externalMemorySetting);
int release_cfg_images_name(CFG_Images_Name *cfgImagesName);
int DumpExternalMemorySetting(ExternalMemorySetting *externalMemorySetting);

#endif  // _CFG_READER_H

