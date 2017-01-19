/*----------------------------------------------------------------------------*
* Copyright Statement:                                                       *
*                                                                            *
*   This software/firmware and related documentation ("MediaTek Software")   *
* are protected under international and related jurisdictions'copyright laws *
* as unpublished works. The information contained herein is confidential and *
* proprietary to MediaTek Inc. Without the prior written permission of       *
* MediaTek Inc., any reproduction, modification, use or disclosure of        *
* MediaTek Software, and information contained herein, in whole or in part,  *
* shall be strictly prohibited.                                              *
* MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
*                                                                            *
*   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
* AGREES TO THE FOLLOWING:                                                   *
*                                                                            *
*   1)Any and all intellectual property rights (including without            *
* limitation, patent, copyright, and trade secrets) in and to this           *
* Software/firmware and related documentation ("MediaTek Software") shall    *
* remain the exclusive property of MediaTek Inc. Any and all intellectual    *
* property rights (including without limitation, patent, copyright, and      *
* trade secrets) in and to any modifications and derivatives to MediaTek     *
* Software, whoever made, shall also remain the exclusive property of        *
* MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
* title to any intellectual property right in MediaTek Software to Receiver. *
*                                                                            *
*   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
* representatives is provided to Receiver on an "AS IS" basis only.          *
* MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
* including but not limited to any implied warranties of merchantability,    *
* non-infringement and fitness for a particular purpose and any warranties   *
* arising out of course of performance, course of dealing or usage of trade. *
* MediaTek Inc. does not provide any warranty whatsoever with respect to the *
* software of any third party which may be used by, incorporated in, or      *
* supplied with the MediaTek Software, and Receiver agrees to look only to   *
* such third parties for any warranty claim relating thereto.  Receiver      *
* expressly acknowledges that it is Receiver's sole responsibility to obtain *
* from any third party all proper licenses contained in or delivered with    *
* MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
* releases made to Receiver's specifications or to conform to a particular   *
* standard or open forum.                                                    *
*                                                                            *
*   3)Receiver further acknowledge that Receiver may, either presently       *
* and/or in the future, instruct MediaTek Inc. to assist it in the           *
* development and the implementation, in accordance with Receiver's designs, *
* of certain softwares relating to Receiver's product(s) (the "Services").   *
* Except as may be otherwise agreed to in writing, no warranties of any      *
* kind, whether express or implied, are given by MediaTek Inc. with respect  *
* to the Services provided, and the Services are provided on an "AS IS"      *
* basis. Receiver further acknowledges that the Services may contain errors  *
* that testing is important and it is solely responsible for fully testing   *
* the Services and/or derivatives thereof before they are used, sublicensed  *
* or distributed. Should there be any third party action brought against     *
* MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
* to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
* mutually agree to enter into or continue a business relationship or other  *
* arrangement, the terms and conditions set forth herein shall remain        *
* effective and, unless explicitly stated otherwise, shall prevail in the    *
* event of a conflict in the terms in any agreements entered into between    *
* the parties.                                                               *
*                                                                            *
*   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
* cumulative liability with respect to MediaTek Software released hereunder  *
* will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
* MediaTek Software at issue.                                                *
*                                                                            *
*   5)The transaction contemplated hereunder shall be construed in           *
* accordance with the laws of Singapore, excluding its conflict of laws      *
* principles.  Any disputes, controversies or claims arising thereof and     *
* related thereto shall be settled via arbitration in Singapore, under the   *
* then current rules of the International Chamber of Commerce (ICC).  The    *
* arbitration shall be conducted in English. The awards of the arbitration   *
* shall be final and binding upon both parties and shall be entered and      *
* enforceable in any court of competent jurisdiction.                        *
*---------------------------------------------------------------------------*/
// DASH MPD Period creation

#ifndef __MPD_PERIOD_H__
#define __MPD_PERIOD_H__

#include "MPDAdaptationset.h"
#include "MPDBaseUrl.h"
#include "MPDDebugLevel.h"
#include "mpdschema_prefix.h"
#include "MPDSchema.h"
#include "XercesString.h"
#include "dash_static.h"

namespace android_dash
{
    class MPDPeriod 
    {
        public:
            enum PeriodSubtitleType
            {
                PERIOD_SUBTITLE_TEXT,
                PERIOD_SUBTITLE_IMAGE
            };

            enum PeriodDRMType
            {
                PERIOD_DRM_NONE,
                PERIOD_DRM_PR,
                PERIOD_DRM_WV,
                PERIOD_DRM_ML_BB,
                PERIOD_DRM_ML_MS3,
            };

            typedef struct
            {
                uint32_t mStartRange;
                uint32_t mEndRange;
                std::string mUrl;
            }PeriodUrlType;
            
        private:
            uint32_t mVideoIndex;
            uint32_t mAudioIndex;
            uint32_t mSubtitleIndex;
            uint32_t mThumbnailIndex;
            uint32_t mStartTime;
            uint32_t mDuration;
            
            std::vector<uint64_t> mVideoAdapList;
            std::vector<uint64_t> mAudioAdapList;
            std::vector<uint64_t> mSubtitleAdapList;
            std::vector<uint64_t> mThumbnailAdapList;
            std::vector<uint64_t> mBaseUrlList;

        public:
            uint32_t getStartTime(){ return mStartTime; };
            uint32_t getEndTime(){ return mStartTime + mDuration; };

        public:
            MPDPeriod();
            ~MPDPeriod();

        private:
            bool initAdaptionSet(MPDSchema::CPeriodType *period, MPDSchema::CPeriodType_ex *period_ex);
            void freeMemory();

        public:
            enum PeriodMediaType
            {
                PERIOD_MEDIA_VIDEO,
                PERIOD_MEDIA_AUDIO,
                PERIOD_MEDIA_SUBTITLE,
                PERIOD_MEDIA_THUMBNAIL           
            };

            enum PeriodNumberType
            {
                PERIOD_ADP,
                PERIOD_REP
            };

            bool getMediaNumber(uint32_t &num, PeriodMediaType type, PeriodNumberType mType);


            // parser it
            bool parserPeriod(MPDSchema::CMPDtype* mpd, MPDSchema::CMPDtype_ex* mpd_ex, uint32_t index);
            bool setCurrentLanguage(std::string language, PeriodMediaType type);
            bool setBandwidth(uint64_t bandwidth);
            bool getAudioInfo(uint32_t index, std::string &codecs, std::string &lang, 
                std::string &samplerate, PeriodNumberType mType);
            bool getVideoInfo(uint32_t index, uint64_t &bandwidth, std::string &codecs, uint32_t &height, 
                uint32_t &width, PeriodNumberType mType);
            bool getSubtitleInfo(uint32_t index, std::string &language, uint32_t &height, 
                uint32_t &width, PeriodSubtitleType &type, PeriodNumberType mType);

            bool setAudioPlayIndex(uint32_t index, PeriodNumberType mType);
            bool setSubtitleIndex(uint32_t index, PeriodNumberType mType);
            bool setVidoPlayIndex(uint32_t index, PeriodNumberType mType);

            PeriodDRMType getDrmType();
            bool findRepForVideoByBitrate(MPDAdaptationset **adp, MPDRepresentation **rep, uint64_t bitrate);

            std::string getBaseUrl();
            bool findRepForAudio(MPDAdaptationset **adp, MPDRepresentation **rep);
            bool findRepForSubtitle(MPDAdaptationset **adp, MPDRepresentation **rep);
#if MPD_PLAYLISTS_DEBUG
            void periodDebug();
#endif
    };
}
#endif