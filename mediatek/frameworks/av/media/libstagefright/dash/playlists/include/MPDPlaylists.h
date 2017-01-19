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
// DASH MPD Playlists creation
#ifndef __MPD_PLAYLISTS_H__
#define __MPD_PLAYLISTS_H__
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>

#include "MPDDebugLevel.h"


namespace android_dash
{  
    
    class MPDPlaylists
    {
         // notify message 
        public:
            enum MPDMessage
            {
                MPD_PERIOD_CHANGED,
                MPD_BANDWIDTH_CHANGED,
                MPD_STREAM_EOS,
                MPD_PLAYLISTS_EXIT,
                MPD_NEXT_UPDATED,
            };

            enum MPDDRMType
            {
                MPD_DRM_NONE,
                MPD_DRM_PR,
                MPD_DRM_WV,
                MPD_DRM_ML_BB,
                MPD_DRM_ML_MS3,
            };
            
            // for media type defined     
            enum MPDMediaType
            {
                MPD_VIDEO,
                MPD_AUDIO,
                MPD_SUBTITLE,
                MPD_THUBNAIL
            };

            enum MPDMediaNumberType
            {
                MPD_ADP,
                MPD_REP
            };
            
            // for container
            enum MPDMediaContainer
            {
                MPD_MP4,
                MPD_TS,
                MPD_UNKNOWN
            };
            
            // for subtitle type define
            enum MPDSubtitleType
            {
                MPD_SUB_TEXT,
                MPD_SUB_IMAGE
            };
            
            typedef void (*CallbackPtr)(MPDPlaylists::MPDMessage message);
            
        private:
            uint32_t mPlaybackStartTime; //s
            uint32_t mPlaybackEndTime;   //s
            uint64_t mDuration;          //us
            uint64_t mStartTime;         //us
            uint64_t mCurrentTime;       //ms

            uint64_t mCurrentPeriodStartTime; // ms
            MPDMediaContainer mContainer;
            MPDDRMType mDrmType;
            bool mHasDrm;
            bool mIsEmpty;
            bool mIsAvailableMPD;
            bool mIsLiveMPD;
            std::string mMPDUrl;
            CallbackPtr mCallback;
            

        public:
            MPDPlaylists(const char *MPDData, uint32_t size, const char *MPDUrl, CallbackPtr callback);
            ~MPDPlaylists();
            
        private:          
            bool initMPDPlayLists();
            bool updateMPDPlaylists();
            bool initMPDParserInst();
            bool destroyMPDParserInst();
            bool addDataToParserInst(std::string &MPDData);
            bool sendMessageToThread(MPDMessage message);
            bool createMPDPlaylists(std::string &MPDData, std::string &MPDUrl);
            void freeMPDPlaylist();
            


            
        public:

            
            // define media information struct
            typedef struct
            {
                std::string mCodecs;
                uint64_t mBandwidth;
                uint32_t mHeight;
                uint32_t mWidth;
                // may add some other video infor here
            }MPDVideoInfo;
            
            typedef struct
            {
                std::string mLanguage;
                std::string mCodecs;
                std::string mSampleRate;
            }MPDAudioInfo;
            
            typedef struct
            {
                std::string mLanguage;
                uint32_t mHeight;
                uint32_t mWidth;
                MPDSubtitleType mType;
            }MPDSubtitleInfo;
            
            
            typedef struct
            {
                MPDMediaType mType;
                std::vector<MPDVideoInfo> mVideoInfo;
                std::vector<MPDAudioInfo> mAudioInfo;
                std::vector<MPDSubtitleInfo> mSubtitleInfo;
            }MPDMediaInfo;
            
            
            typedef struct
            {
                MPDDRMType mType;
            }MPDDRMInfo;

            typedef struct
            {
                uint32_t mStartRange;
                uint32_t mEndRange;
                std::string mUrl;
            }MPDUrlType;
            
            typedef struct
            {
                public:
                    MPDMediaType mType;
                    uint64_t mStartTime; //us
                    uint64_t mDuration;   //us
                    uint64_t mBandwidth;   
                    bool mHasUrl;
                    MPDUrlType mMediaUrl;
                    MPDUrlType mInitUrl;
                    MPDUrlType mIndexUrl;
                    MPDUrlType mBitstreamUrl;
                    MPDDRMInfo mDrmInfo;
            }MPDURLInfo;
            
            // API defined 
            bool isEmptyMPD(){ return mIsEmpty; };
            bool isLiveMPD(){ return mIsLiveMPD; };
            bool isAvailableMPD(){ return mIsAvailableMPD; };
            bool isDRMIncluded(){ return mHasDrm; };
            MPDMediaContainer getMPDContainer(){ return mContainer; };
            MPDDRMType getDrmType(){ return mDrmType; };
            uint64_t getDuration(){ return mDuration; };
            
            
            // get live download information
            bool getNextMPDDlTime(uint64_t &time);
            bool getNextLiveMPDDlUrl(std::string &url);
                
            // get live stream availbale time
            bool getAvailableStartTime(uint64_t &time);
            bool getAvailableEndTime(uint64_t &time);
            
            // get mini buffer time
            bool getMiniBufferTime(uint64_t &time);
                    
            // get media information
            bool getMediaNumber(uint32_t &number, MPDMediaType type, MPDMediaNumberType nType);
            bool getMediaInformation(MPDMediaInfo &info, MPDMediaType type, MPDMediaNumberType nType);

            // get url by some information
            bool getUrl(MPDURLInfo &urlInfo);
                    
            // set some information to MPD
            bool setCurrentIndex(uint32_t index, MPDMediaType type,  MPDMediaNumberType nType);
            bool setCurrentBandwidth(uint64_t bandwidth, MPDMediaType type);
            bool setStartTime(uint64_t time);
            bool setEndTime(uint64_t time);
            bool setCurrentLanguage(std::string &language, MPDMediaType type);

            // add live mpd
            bool addLiveMPDToPlaylists(std::string &MPDData, std::string &MPDUrl);
                            
            // give static functin for playlist fecther
            
            bool initCheck(){return isAvailableMPD();};

            // maybe should update time to Playlists
            bool updateCurrentTime(uint64_t time);
            
#if MPD_PLAYLISTS_DEBUG
            void playlistsDebug();
#endif
    };
}

#endif
