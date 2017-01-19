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

#include "MPDPlaylist.h"
#include "UriGlue.h"

using namespace android_dash;
MPDPlaylist::MPDPlaylist()
{
    mBeginTime = 0;
    mDuration = 0;
    mAvailableStartTime = 0;
    mAvailableEndTime = 0;
    mMiniBufferTime = 0;
    mCurrentPeriod = 0;
    mDynamicType = DASH_TYPE_STATIC;
    mContainer = DASH_UNKNOWN;
    mProfileType = DASH_PROFILE_NONE;
    mMiniUpdatePeriod = 0;
	mTimeShiftBufferDepth = 0;
    mUpdateLocation = std::string("");
	mSuggestionDelay = 0;
    freeMemory();

    MPD_DEBUG("MPDPlaylist Create");
}


MPDPlaylist::~MPDPlaylist()
{
    mBeginTime = 0;
    mDuration = 0;
    mAvailableStartTime = 0;
    mAvailableEndTime = 0;
    mMiniBufferTime = 0;
    mCurrentPeriod = 0;
    mDynamicType = DASH_TYPE_STATIC;
    mContainer = DASH_UNKNOWN;
    mProfileType = DASH_PROFILE_NONE;
    mMiniUpdatePeriod = 0;
	mTimeShiftBufferDepth = 0;
	mSuggestionDelay = 0;

    freeMemory();
    MPD_DEBUG("MPDPlaylist Deleted");
}


void MPDPlaylist::freeMemory()
{
    uint32_t count = mBaseUrlList.size();
    uint32_t index = 0;

    if(count > 0)
    {
        for(; index < count; ++ index)
        {
            MPDBaseUrl *ptr = (MPDBaseUrl *)mBaseUrlList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mBaseUrlList.clear();
    }

    count = mPeriodList.size();

    if(count > 0)
    {
        for(index = 0; index < count; ++ index)
        {
            MPDPeriod *ptr = (MPDPeriod *)mPeriodList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mPeriodList.clear();
    }
}




bool MPDPlaylist::parserMPD(MPDParser *data)
{
    assert(data != NULL);   
    MPDSchema::CMPDtype root = data->doc->MPD.first();
    MPDSchema::CMPDtype_ex* root_ex = data->mpd_ex;
    assert(root_ex != NULL);

    mProfileType = (ProfileType)root_ex->profile_type;


    if(mProfileType == DASH_PROFILE_NONE)
    {
        MPD_ERROR("unsupported profile %d", mProfileType);
        return false;
    }


    // filter out base url
    uint32_t count = root.BaseURL.count();
    uint32_t index = 0;

    if(count > 0)
    {
        
        MPDSchema::CMPDtype* mpd_ptr = &root;
        
        for(; index < count; ++ index)
        {
            MPDBaseUrl *baseUrl = new MPDBaseUrl();
            if(baseUrl == NULL)
            {
                MPD_ERROR("no memory");
                return false;
            }

            baseUrl->setBaseUrl(((std::string)mpd_ptr->BaseURL[index]));
            
            if(mpd_ptr->BaseURL[index].byteRange.exists())
            {
                baseUrl->setByteRange((std::string)(mpd_ptr->BaseURL[index].byteRange));
            }

            mBaseUrlList.push_back(*((uint64_t*)&baseUrl));
        }
    }



    // setup mpd other info
   if(root_ex->availableendtime_exists)
   {
       assert(root.availabilityEndTime.exists());
       mAvailableEndTime = root_ex->availableendtime_ex;
   }


   if(root_ex->availablestarttime_exists)
   {
       assert(root.availabilityStartTime.exists());
       mAvailableStartTime = root_ex->availablestarttime_ex;
   }

   switch(root_ex->mpd_container)
   {
       case dash_enum::DASH_TS:
        mContainer = DASH_TS;
        break;

       case dash_enum::DASH_MP4:
        mContainer = DASH_MP4;
        break;

       default:
	   	if(!root_ex->empty())
	   	{
	        mContainer = DASH_UNKNOWN;
	        MPD_ERROR("unsupported container");
	        return false;
	   	}
		break;
   }

   mDynamicType = (DynamicType)root_ex->dynamic_type;
   mDuration = (uint32_t)root_ex->mpd_total_time;

   if(root_ex->minbuffertime_exists)
   {
       mMiniBufferTime = (uint32_t)root_ex->minbuffertime_ex;
   }

   if(root_ex->miniupdatetime_exists)
   {
       mMiniUpdatePeriod = (uint32_t)root_ex->miniupdatetime_ex;
   }

   if(root_ex->timeshiftbufferdepth_exists)
   {
       mTimeShiftBufferDepth = (uint32_t)root_ex->timeshiftbufferdepth_ex;
   }

   if(root_ex->suggestedpresentationdelay_exists)
   {
       mSuggestionDelay = (uint32_t)root_ex->suggestedpresentationdelay_ex;
   }

   if(root_ex->location_exists)
   {
       mUpdateLocation = root_ex->location_ex;
   }


   // filter out period data 


   assert(root.Period.count() >= root_ex->period_list.size());

   
   count = root_ex->period_list.size();
   assert(count >= 0);
   
   for(index = 0; index < count; ++ index)
   {
       MPDPeriod *period = new MPDPeriod();
       if(period == NULL)
       {
           MPD_ERROR("no memory");
           return false;
       }

       if(!period->parserPeriod(&root, root_ex, index))
       {
           MPD_ERROR("period parser error");
		   delete period;
           return false;
       }

       mPeriodList.push_back(*((uint64_t*)&period));
   }

   if(mPeriodList.size() > 0)
   {
       MPDPeriod *period = (MPDPeriod *)mPeriodList[0];
       assert(period != 0);
       mBeginTime = period->getStartTime();
   }

   // check drm type
   count = mPeriodList.size();
   index = 0;

   for(;index < count; ++ index)
   {
       MPDPeriod *period = (MPDPeriod *)mPeriodList[index];
       assert(period != 0);
       
       mDrmType = (DRMType)period->getDrmType();
       if(mDrmType != DASH_DRM_NONE)
       {
           MPD_DEBUG("find drm type = %d", mDrmType);
           break;
       }
   }

   return true;
}


bool MPDPlaylist::getCurrentPeriod(MPDPeriod **period)
{
    *period = NULL;
    if(mCurrentPeriod < mPeriodList.size())
    {
        *period = (MPDPeriod *)mPeriodList[mCurrentPeriod];
        assert(period != 0);
        return true;
    }

    MPD_ERROR("mCurrentPeriod = %d size = %d", mCurrentPeriod, mPeriodList.size());
    return false;
}


// must decide current period by time of ... getting url
bool MPDPlaylist::getMediaNumber(uint32_t &num, MediaType type, MediaNumberType mType)
{
    num = 0;
    MPDPeriod *period;
    if(getCurrentPeriod(&period))
    {
        if(period->getMediaNumber(num, (MPDPeriod::PeriodMediaType)type, (MPDPeriod::PeriodNumberType)mType))
        {  
            return true;
        }
    }

    MPD_ERROR("get media number error");
    return false;
}



// reset all period lanuage, for play down
bool MPDPlaylist::setCurrentLanguage(std::string language, MPDPlaylist::MediaType type)
{
    uint32_t index = 0;
    for(; index < mPeriodList.size(); ++ index)
    {
        MPDPeriod *period = (MPDPeriod *)mPeriodList[index];
        assert(period != 0);
        
        if(!period->setCurrentLanguage(language, (MPDPeriod::PeriodMediaType)type))
        {
            MPD_ERROR("set current language error");
            return false;
        }
    }

    return true;
    
}

bool MPDPlaylist::setBandwidth(uint64_t bandwidth)
{
    uint32_t index = 0;
    for(; index < mPeriodList.size(); ++ index)
    {
        MPDPeriod *period = (MPDPeriod *)mPeriodList[index];
        assert(period != 0);
        
        if(!period->setBandwidth(bandwidth))
        {
            MPD_ERROR("set current language error");
            return false;
        }
    }

    return true;
}

bool MPDPlaylist::getMediaInfoByIndex(uint32_t index, MediaType type, MediaNumberType mType, MediaInfo *info)
{
    if(info == NULL)
    {
        MPD_ERROR("media info struct error");
        return false;
    }

    MPDPeriod *period;

    if(!getCurrentPeriod(&period))
    {
        MPD_ERROR("get current period error");
        return false;
    }

    MPDPeriod::PeriodNumberType nType = (MPDPeriod::PeriodNumberType)mType;

    switch(type)
    {
        case DASH_MEDIA_AUDIO:
            {
                AudioInfo *ptr = (AudioInfo *)info;
                if(!period->getAudioInfo(index, ptr->mCodecs, ptr->mLanguage,
                    ptr->mSampleRate, nType))
                {
                    MPD_ERROR("get audio info error");
                    return false;
                }
            }
        return true;

        case DASH_MEDIA_VIDEO:
            {
                VideoInfo *ptr = (VideoInfo *)info;
                if(!period->getVideoInfo(index, ptr->mBandwidth, ptr->mCodecs,
                    ptr->mHeight, ptr->mWidth, nType))
                {
                    MPD_ERROR("get audio info error");
                    return false;
                }
            }
        return true;

        case DASH_MEDIA_SUBTITLE:
            {
                SubtitleInfo *ptr = (SubtitleInfo *)info;
                MPDPeriod::PeriodSubtitleType type;
                
                if(!period->getSubtitleInfo(index, ptr->mLanguage, ptr->mHeight,
                     ptr->mWidth, type, nType))
                {
                    MPD_ERROR("get audio info error");
                    return false;
                }
                ptr->mStype = (SubtitleType)type;
            }
        return true;

        default:
            MPD_ERROR("media information type error");
            return false;    
    }
}

bool MPDPlaylist::setCurrentPlayIndex(uint32_t index, MediaType type, MediaNumberType nType)
{
    uint32_t periodInd = 0;
    uint32_t count = mPeriodList.size();

    for(; periodInd < count; ++ periodInd)
    {
        MPDPeriod *period = (MPDPeriod *)mPeriodList[periodInd];
        assert(period != 0);
        
        switch(type)
        {
            case DASH_MEDIA_VIDEO:
                {
                    if(!period->setVidoPlayIndex(index, (MPDPeriod::PeriodNumberType)nType))
                    {
                        MPD_ERROR("set video current index error");
                        return false;
                    }
                }
            break;

            case DASH_MEDIA_AUDIO:
                {
                    if(!period->setAudioPlayIndex(index, (MPDPeriod::PeriodNumberType)nType))
                    {
                        MPD_ERROR("set audio current index error");
                        return false;
                    }
                }
            break;

            case DASH_MEDIA_SUBTITLE:
                {
                    if(!period->setSubtitleIndex(index, (MPDPeriod::PeriodNumberType)nType))
                    {
                        MPD_ERROR("set subtitle current index error");
                        return false;
                    }
                }
            break;

            default:
                MPD_ERROR("other type is not supported");
                return false;
        }
    }

    return true;
}

bool MPDPlaylist::findPeriodByTime(uint64_t start, MPDPeriod **period)
{
    assert(period != NULL);

    uint32_t count = mPeriodList.size();
    uint32_t index = 0;
    *period = NULL;

    for(;index < count; ++ index)
    {
        MPDPeriod *ptr = (MPDPeriod *)mPeriodList[index];
        assert(ptr != 0);
        
        if(ptr->getStartTime() * 1000 <= start
            && ptr->getEndTime() * 1000 > start)
        {
            *period = (MPDPeriod *)mPeriodList[index];

            ///TODO:: maybe current index should be set by current playback time
            mCurrentPeriod = index;
            assert(*period != NULL);
            MPD_DEBUG("find period for start time = %d end time = %d",
                ptr->getStartTime(), ptr->getEndTime());

            return true;
        }
    }

    MPD_ERROR("can't find a period for start time = %lld", start);
    return false;
}


std::string MPDPlaylist::getBaseUrlByAdpAndRep(std::string MPDUrl, MPDPeriod *period, MPDAdaptationset *adp, MPDRepresentation *rep)
{
    if(period == NULL || adp == NULL || rep == NULL)
    {
        MPD_ERROR("invalid param");
        assert(0);
    }

    if(MPDUrl.length() == 0)
    {
        MPD_ERROR("MPD url is empty");
        assert(0);
    }

    MPD_DEBUG("MPD URL is %s", MPDUrl.c_str());
    // needs get API from uriparser
    std::string baseUrlInMPD = std::string("");
    std::string baseUrlInPeriod = period->getBaseUrl();
    std::string baseUrlInAdp = adp->getBaseUrl();
    std::string baseurlInRep = rep->getBaseUrl();

    // only support first one, maybe support others later
    if(mBaseUrlList.size() > 0)
    {
        MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[0];
        assert(base != NULL);
        baseUrlInMPD = base->getBaseUrl();
    }

    std::string baseUrl = std::string("");
    if(baseUrlInMPD.length() > 0)
    {
        // absolute address
        MPD_DEBUG("MPD base url = %s", baseUrlInMPD.c_str());
        if(baseUrlInMPD.find("http") == 0)
        {
            baseUrl = std::string(baseUrlInMPD);
        }
        else // relative address
        {
            char buffer[1024] = {0};
            if(!URIParser_get_url(MPDUrl.c_str(), baseUrlInMPD.c_str(), buffer, 1024))
            {
                MPD_ERROR("base url transfer error");
                assert(0);
            }
            
            // absolute address
            baseUrl = std::string(buffer);
        }
    }
    else
    {
        baseUrl = MPDUrl;
    }

    // after MPD, base url should be a absolute address
    if(baseUrlInPeriod.length() > 0)
    {
        MPD_DEBUG("Period base url = %s", baseUrlInPeriod.c_str());
        if(baseUrlInPeriod.find("http") == 0)
        {
            baseUrl = std::string(baseUrlInPeriod);
        }
        else
        {
            char buffer[1024] = {0};
            if(!URIParser_get_url(baseUrl.c_str(), baseUrlInPeriod.c_str(), buffer, 1024))
            {
                MPD_ERROR("base url transfer error");
                assert(0);
            }

            baseUrl = std::string(buffer);
        }
    }
    else
    {
        // do nothing, baseUrl is absolute address before
    }

    if(baseUrlInAdp.length() > 0)
    {
        MPD_DEBUG("Adp base url = %s", baseUrlInAdp.c_str());
        if(baseUrlInAdp.find("http") == 0)
        {
            baseUrl = std::string(baseUrlInAdp);
        }
        else
        {
            char buffer[1024] = {0};
            if(!URIParser_get_url(baseUrl.c_str(), baseUrlInAdp.c_str(), buffer, 1024))
            {
                MPD_ERROR("base url transfer error");
                assert(0);
            }

            baseUrl = std::string(buffer);
        }
    }
    else
    {
        // do nothing
    }

    if(baseurlInRep.length() > 0)
    {
        MPD_DEBUG("Rep base url = %s", baseurlInRep.c_str());
        if(baseurlInRep.find("http") == 0)
        {
            baseUrl = std::string(baseurlInRep);
        }
        else
        {
            char buffer[1024] = {0};
            if(!URIParser_get_url(baseUrl.c_str(), baseurlInRep.c_str(), buffer, 1024))
            {
                MPD_ERROR("base url transfer error");
                assert(0);
            }

            baseUrl = std::string(buffer);
        }
    }

    return baseUrl;
}


#if MPD_PLAYLISTS_DEBUG
void MPDPlaylist::playlistDebug()
{
    MPD_DEBUG("\n\n\n");
    MPD_DEBUG("begin = %lu duration = %lu", mBeginTime, mDuration);
    MPD_DEBUG("available start = %lu, available end = %lu", mAvailableStartTime,
        mAvailableEndTime);

    MPD_DEBUG("mini buffer = %lu, mini update = %lu", mMiniBufferTime, mMiniUpdatePeriod);
    MPD_DEBUG("current period = %lu, time depth = %lu", mCurrentPeriod, mTimeShiftBufferDepth);

    MPD_DEBUG("delay = %lu dynamic = %lu", mSuggestionDelay, mDynamicType);
    MPD_DEBUG("container = %d profile = %lu", mContainer, mProfileType);
    MPD_DEBUG("drm type = %lu location = %s", mDrmType, mUpdateLocation.c_str());


    uint32_t count = mBaseUrlList.size();
    uint32_t index = 0;

    for(; index < count; ++ index)
    {
        MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[index];
		assert(base != NULL);
        base->baseUrlDebug();
    }


    MPD_DEBUG("period debug");
    count = mPeriodList.size();
    for(index = 0; index < count; ++ index)
    {
    	MPDPeriod *period = (MPDPeriod *)mPeriodList[index];
        assert(period != 0);
        period->periodDebug();
    }
}
#endif