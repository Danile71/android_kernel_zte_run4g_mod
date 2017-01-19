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
// DASH MPD Representationset creation

#ifndef __MPD_REPRESENTATION_H__
#define __MPD_REPRESENTATION_H__

#include "MPDSubrepresentation.h"
#include "MPDSegment.h"
#include "MPDContentProtection.h"
#include "MPDBaseUrl.h"
#include "MPDDebugLevel.h"
#include "mpdschema_prefix.h"
#include "MPDSchema.h"
#include "XercesString.h"
#include "dash_static.h"

namespace android_dash
{
    class MPDRepresentation
    {
        public:
            typedef struct
            {
                uint32_t mStartRange;
                uint32_t mEndRange;
                std::string mUrl;
            }RepUrlType;
            
        private:
            uint32_t mTimeScale;
            uint32_t mWidth;
            uint32_t mHeight;
            double   mMaxPlayrate;
            uint64_t mBitrate;
            bool mSorted;
            MPDContentProtection::ProtectionType mDrmType;
            std::string mCodecs;
            

            std::vector<uint64_t> mDrmList;
            MPDSegment mInitList;
            std::vector<uint64_t> mIndexList;
            std::vector<uint64_t> mMediaList;
            std::vector<uint64_t> mBitstreamList;
            std::vector<uint64_t> mBaseUrlList;

            std::vector<uint64_t> mSubrepresentationList;
        public:
            uint32_t getTimeScale(){ return mTimeScale; };
            uint32_t getWidth(){ return mWidth; };
            uint32_t getHeight(){ return mHeight; };
            uint64_t getBitrate(){ return mBitrate; };
            std::string getCodecs(){ return mCodecs; };
            double getMaxPlayrate(){ return mMaxPlayrate; };
            bool setSorted(bool flag){mSorted = flag; return true;};
            bool getSorted(){ return mSorted; };
            uint64_t getBandwidth(){ return mBitrate; };
            MPDContentProtection::ProtectionType getDrmType(){ return mDrmType; };

        private:
            bool initSegmentUrl(MPDSchema::CPeriodType *period, MPDSchema::CPeriodType_ex *period_ex,
                MPDSchema::CAdaptationSetType *adp, MPDSchema::CAdaptationSetType_ex *adp_ex,
                MPDSchema::CRepresentationType *rep, MPDSchema::CRepresentationType_ex *rep_ex);
            
            bool initSegmentFromNone(double start, double duration);
            bool initSegmentFromBase(MPDSchema::CSegmentBaseType_ex *base_ex, double start, double duration);
            bool initSegmentFromList(MPDSchema::CSegmentListType_ex *list_ex, 
                MPDSchema::CSegmentListType* list,double start, double duration);
            
            bool initSegmentFromTemplate(MPDSchema::CSegmentTemplateType_ex *template_ex,
                MPDSchema::CSegmentTemplateType *template_ptr, double start, double duration, MPDSchema::CRepresentationType_ex *rep_ex);


            void freeMemory();
            

        public:
            MPDRepresentation();
            ~MPDRepresentation();


        public:
            bool parserRep(MPDSchema::CPeriodType *period, MPDSchema::CPeriodType_ex *period_ex,
                MPDSchema::CAdaptationSetType *adp, MPDSchema::CAdaptationSetType_ex *adp_ex,
                MPDSchema::CRepresentationType *rep, MPDSchema::CRepresentationType_ex *rep_ex);

            std::string getBaseUrl();
            bool findIndexUrlByTime(RepUrlType *url, uint64_t startTime);
            bool findBitstreamUrlByTime(RepUrlType *url, uint64_t startTime);
            bool findInitUrl(RepUrlType *url);
            bool findMediaUrlByTime(RepUrlType *url, uint64_t startTime, uint64_t &time, uint64_t &duration);

#if MPD_PLAYLISTS_DEBUG
            void repDebug();
#endif
    };
}
#endif
