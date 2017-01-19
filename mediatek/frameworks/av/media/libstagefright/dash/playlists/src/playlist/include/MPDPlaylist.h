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
// DASH MPD Playlist creation
#ifndef __MPD_PLAYLIST_H__
#define __MPD_PLAYLIST_H__

#include "MPDPeriod.h"
#include "MPDParser.h"
#include "MPDBaseUrl.h"
#include "MPDDebugLevel.h"
#include "mpdschema_prefix.h"
#include "MPDSchema.h"
#include "XercesString.h"
#include "dash_static.h"

namespace android_dash
{
    class MPDPlaylist
    {
        public:
            enum MediaContainer
            {
                DASH_MP4,
                DASH_TS,
                DASH_UNKNOWN
            };

            enum SubtitleType
            {
                DASH_SUB_TEXT,
                DASH_SUB_IMAGE
            };

            enum ProfileType
            {
                DASH_PROFILE_NONE,
                DASH_ISO_MAIN,
                DASH_ISO_DEMAND,
                DASH_ISO_LIVE,
                DASH_TS_MAIN,
                DASH_TS_SIMPLE,
                DASH_HBBTV_LIVE
            };

            enum DynamicType
            {
                DASH_TYPE_STATIC,
                DASH_TYPE_DYNAMIC_EMPTY,
                DASH_TYPE_DYNAMIC_NORMAL
            };

            enum DRMType
            {
                DASH_DRM_NONE,
                DASH_DRM_PR,
                DASH_DRM_WV,
                DASH_DRM_ML_BB,
                DASH_DRM_ML_MS3,
            };


            enum MediaType
            {
                DASH_MEDIA_VIDEO,
                DASH_MEDIA_AUDIO,
                DASH_MEDIA_SUBTITLE,
                DASH_MEDIA_THUMBNAIL           
            };

            enum MediaNumberType
            {
                DASH_ADP,
                DASH_REP
            };

        private:
            uint32_t mBeginTime;
            uint32_t mDuration;
            uint32_t mAvailableStartTime;
            uint32_t mAvailableEndTime;
            uint32_t mMiniBufferTime;
            uint32_t mMiniUpdatePeriod;
            uint32_t mCurrentPeriod;
            uint32_t mTimeShiftBufferDepth;
            uint32_t mSuggestionDelay;

            
            DynamicType mDynamicType;
            MediaContainer mContainer;
            ProfileType mProfileType;
            DRMType mDrmType;
            std::vector<uint64_t> mPeriodList;
            std::vector<uint64_t> mBaseUrlList;

            std::string mUpdateLocation;

        public:
            MediaContainer getContainer(){ return mContainer; };
            uint32_t getDuration(){ return mDuration; };
            DynamicType getDynamicType(){ return mDynamicType; };
            DRMType getDrmType(){ return mDrmType; };
            uint32_t getMiniUpdatePeriod(){ return mMiniUpdatePeriod; };
            std::string getUpdateLocation(){ return mUpdateLocation; };
            uint32_t getAvailableStartTime(){ return mAvailableStartTime; };
            uint32_t getAvailableEndTime(){ return mAvailableEndTime; };
            uint32_t getMiniBufferTime(){ return mMiniBufferTime; };
            uint32_t getBeginTime(){ return mBeginTime; };
            bool setBandwidth(uint64_t bandwidth);
            bool findPeriodByTime(uint64_t start, MPDPeriod **period);
            std::string getBaseUrlByAdpAndRep(std::string MPDUrl, MPDPeriod *period, MPDAdaptationset *adp, MPDRepresentation *rep);
            
        public:
            MPDPlaylist();
            ~MPDPlaylist();

        private:
            bool getCurrentPeriod(MPDPeriod **period);
            void freeMemory();


        public:
            
            // define media information struct
            class MediaInfo
            {
                public:
                    MediaType mType;
            };
            
            class VideoInfo: public MediaInfo
            {
                public:
                    std::string mCodecs;
                    uint64_t mBandwidth;
                    uint32_t mHeight;
                    uint32_t mWidth;
                // may add some other video infor here
            };

            class AudioInfo: public MediaInfo
            {
                public:
                    std::string mLanguage;
                    std::string mCodecs;
                    std::string mSampleRate;
            };
            
            class SubtitleInfo: public MediaInfo
            {
                public:
                    std::string mLanguage;
                    uint32_t mHeight;
                    uint32_t mWidth;
                    SubtitleType mStype;
            };

            
            // parser data
            bool parserMPD(MPDParser *data);
            bool getMediaNumber(uint32_t &num, MediaType type, MediaNumberType mType);
            bool setCurrentLanguage(std::string language, MPDPlaylist::MediaType type);
            bool getMediaInfoByIndex(uint32_t index, MediaType type, MediaNumberType mType, MediaInfo *info);
            bool setCurrentPlayIndex(uint32_t index, MediaType type, MediaNumberType nType);
#if MPD_PLAYLISTS_DEBUG
            void playlistDebug();
#endif
    };
}
#endif

