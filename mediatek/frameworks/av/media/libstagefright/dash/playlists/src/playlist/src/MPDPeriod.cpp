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

// create Period for dash
#include "MPDPeriod.h"

using namespace android_dash;

MPDPeriod::MPDPeriod()
{
    mVideoIndex = 0;
    mAudioIndex = 0;
    mSubtitleIndex = 0;
    mThumbnailIndex = 0;
    mStartTime = 0;
    mDuration = 0;

}

MPDPeriod::~MPDPeriod()
{
    mVideoIndex = 0;
    mAudioIndex = 0;
    mSubtitleIndex = 0;
    mThumbnailIndex = 0;
    mStartTime = 0;
    mDuration = 0;

    freeMemory();
}


void MPDPeriod::freeMemory()
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

    count = mVideoAdapList.size();

    if(count > 0)
    {
        for(; index < count; ++ index)
        {
            MPDAdaptationset *ptr = (MPDAdaptationset *)mVideoAdapList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mVideoAdapList.clear();
    }


    count = mAudioAdapList.size();

    if(count > 0)
    {
        for(; index < count; ++ index)
        {
            MPDAdaptationset *ptr = (MPDAdaptationset *)mAudioAdapList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mAudioAdapList.clear();
    }

    count = mSubtitleAdapList.size();

    if(count > 0)
    {
        for(; index < count; ++ index)
        {
            MPDAdaptationset *ptr = (MPDAdaptationset *)mSubtitleAdapList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mSubtitleAdapList.clear();
    }

    count = mThumbnailAdapList.size();

    if(count > 0)
    {
        for(; index < count; ++ index)
        {
            MPDAdaptationset *ptr = (MPDAdaptationset *)mThumbnailAdapList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mThumbnailAdapList.clear();
    }

    
}

bool MPDPeriod::getMediaNumber(uint32_t &num, PeriodMediaType type, PeriodNumberType mType)
{
    uint32_t count = 0;
    uint32_t index = 0;
    num = 0;

    std::vector<uint64_t> *ptr = NULL;
    switch(type)
    {
        case PERIOD_MEDIA_VIDEO:
            ptr = &mVideoAdapList;
            break;
            
        case PERIOD_MEDIA_AUDIO:
            ptr = &mAudioAdapList;
            break;
        case PERIOD_MEDIA_SUBTITLE:
            ptr = &mSubtitleAdapList;
            break;
        case PERIOD_MEDIA_THUMBNAIL:
            ptr = &mThumbnailAdapList;
            break;
        default:
            MPD_ERROR("invalid media type for number got");
            return false;
    }

    if(mType == PERIOD_ADP)
    {
        num = ptr->size();
    }
    else if(mType == PERIOD_REP)
    {
        count = ptr->size();
        for(index = 0; index < count; ++ index)
        {
            num += ((MPDAdaptationset *)((*ptr)[index]))->getRepresentationSize();
        }
    }
    else
    {
        MPD_ERROR("error get number type");
        return false;
    }
    
    return true;
}


bool MPDPeriod::parserPeriod(MPDSchema::CMPDtype* mpd, MPDSchema::CMPDtype_ex* mpd_ex, uint32_t index)
{
    assert(mpd != NULL);
    assert(mpd_ex != NULL);
    assert(index < mpd->Period.count());
    assert(mpd->Period.count() == mpd_ex->period_list.size());

    MPDSchema::CPeriodType* period = &mpd->Period[index];
    mpd_ex->period_list[index].segment_info = dash_handle::get_segment_info_type(period->SegmentBase.exists(),
         period->SegmentList.exists(),period->SegmentTemplate.exists());


	assert(period->AdaptationSet.count() >= mpd_ex->period_list[index].adaptation_list.size());
	
    // get base url firstly
    uint32_t count = period->BaseURL.count();
    uint32_t num = 0;
    if(count > 0)
    {        
        for(; num < count; ++ num)
        {
            MPDBaseUrl *baseUrl = new MPDBaseUrl();
            if(baseUrl == NULL)
            {
                MPD_ERROR("no memory");
                return false;
            }
                
            baseUrl->setBaseUrl((std::string)period->BaseURL[num]);

            
            if(period->BaseURL[num].byteRange.exists())
            {
                baseUrl->setByteRange((std::string)(period->BaseURL[num].byteRange));
            }

            mBaseUrlList.push_back(*((uint64_t*)&baseUrl));
        }
    }



    // init the time
    mStartTime = (unsigned)mpd_ex->period_list[index].start_time;
    mDuration = (unsigned)mpd_ex->period_list[index].duration_time;

    if(!initAdaptionSet(period, &(mpd_ex->period_list[index])))
    {
        MPD_ERROR("init adaptionset in period error");
        return false;
    }

    return true;
}


bool MPDPeriod::initAdaptionSet(MPDSchema::CPeriodType *period, MPDSchema::CPeriodType_ex *period_ex)
{
    assert(period != NULL);
    assert(period_ex != NULL);

    uint32_t index = 0;
    MPDSchema::CAdaptationSetType *adp = NULL;
    MPDSchema::CAdaptationSetType_ex *adp_ex = NULL;
    
    for(; index < period_ex->adaptation_list.size(); ++ index)
    {
        adp = &period->AdaptationSet[index];
        adp_ex = &period_ex->adaptation_list[index];
        switch(period_ex->adaptation_list[index].content_type)
        {   
            case dash_enum::audio:
                {
                    MPDAdaptationset *adaptation = new MPDAdaptationset();
                    if(adaptation == NULL)
                    {
                        MPD_ERROR("no memory");
                        return false;
                    }
                    
                    if(!adaptation->parserAdp(period, period_ex, adp, adp_ex))
                    {
                        MPD_ERROR("");
                        delete adaptation;
                        return false;
                    }
                    mAudioAdapList.push_back(*((uint64_t*)&adaptation));
                }
            break;
           
                
            case dash_enum::video:
                {
                    MPDAdaptationset *adaptation = new MPDAdaptationset();
                    if(adaptation == NULL)
                    {
                        MPD_ERROR("no memory");
                        return false;
                    }
                    
                    if(!adaptation->parserAdp(period, period_ex, adp, adp_ex))
                    {
                        MPD_ERROR("");
                        delete adaptation;
                        return false;
                    }
                    mVideoAdapList.push_back(*((uint64_t*)&adaptation));
                }
            break;

            case dash_enum::subtitle:
                {
                    MPDAdaptationset *adaptation = new MPDAdaptationset();
                    if(adaptation == NULL)
                    {
                        MPD_ERROR("no memory");
                        return false;
                    }
                    
                    if(!adaptation->parserAdp(period, period_ex, adp, adp_ex))
                    {
                        MPD_ERROR("");
                        delete adaptation;
                        return false;
                    }
                    mSubtitleAdapList.push_back(*((uint64_t*)&adaptation));
                }
            break;

            case dash_enum::c_ts:
                {
                    MPDAdaptationset *adaptation = new MPDAdaptationset();
                    if(adaptation == NULL)
                    {
                        MPD_ERROR("no memory");
                        return false;
                    }
                    
                    if(!adaptation->parserAdp(period, period_ex, adp, adp_ex))
                    {
                        MPD_ERROR("");
                        delete adaptation;
                        return false;
                    }
                    mVideoAdapList.push_back(*((uint64_t*)&adaptation));
                }
            break;

            case dash_enum::thumbnail:
                {
                    MPDAdaptationset *adaptation = new MPDAdaptationset();
                    if(adaptation == NULL)
                    {
                        MPD_ERROR("no memory");
                        return false;
                    }
                    
                    if(!adaptation->parserAdp(period, period_ex, adp, adp_ex))
                    {
                        MPD_ERROR("");
                        delete adaptation;
                        return false;
                    }
                    mThumbnailAdapList.push_back(*((uint64_t*)&adaptation));
                }
            break;

            default:
                {
                    MPD_ERROR("unknow type");
                }
            break;
        }
    }


    if(mVideoAdapList.size() == 0)
    {
        MPD_ERROR("no video adaptation set");
        return false;
    }

    return true;
}



bool MPDPeriod::setCurrentLanguage(std::string language, PeriodMediaType type)
{
    if(type == PERIOD_MEDIA_VIDEO
        || type == PERIOD_MEDIA_THUMBNAIL)
    {
        // no langauge in video and thumbnail

        MPD_ERROR("type is error during set lanauge");
        return false;
    }

    std::vector<uint64_t> *ptr  = NULL;

    if(type == PERIOD_MEDIA_AUDIO)
    {
        ptr = &mAudioAdapList;
    }
    else
    {
        ptr = &mSubtitleAdapList;
    }

    uint32_t index = 0;
    for(; index < ptr->size(); ++ index)
    {
        MPDAdaptationset *adp = (MPDAdaptationset *)((*ptr)[index]);
        assert(adp != NULL);
         if(language.compare(adp->getLanguage()))
         {
             MPD_DEBUG("find language %s", language.c_str());
             break;
         }
    }


    // if can't find it in some period, reset it to be first one
    if(index == ptr->size())
    {
        MPD_ERROR("language can't be found");
        index = 0;
    }

    if(type == PERIOD_MEDIA_AUDIO)
    {
        mAudioIndex = index;
    }
    else
    {
        mSubtitleIndex = index;
    }

    return true;
}



bool MPDPeriod::setBandwidth(uint64_t bandwidth)
{    
    uint32_t count = mVideoAdapList.size();
    uint32_t index = 0;

    for(;index < count; ++ index)
    {
        MPDAdaptationset *adp = (MPDAdaptationset *)mVideoAdapList[index];
        assert(adp != NULL);
        adp->setBandwidth(bandwidth);
    }

    return true;
}


bool MPDPeriod::getAudioInfo(uint32_t index, std::string &codecs, std::string &lang, 
    std::string &samplerate, PeriodNumberType mType)
{    
    switch(mType)
    {
        case PERIOD_ADP:
            {
                assert(mAudioAdapList.size() > index);
                MPDAdaptationset *adp = (MPDAdaptationset *)mAudioAdapList[index];
                assert(adp != NULL);
                codecs = adp->getCodecs();
                lang = adp->getLanguage();
                samplerate = adp->getSamplerate();
                
            }
        return true;

        case PERIOD_REP:
            {
               // do nothing , it may be supported if necessary
            }
        return false;

        default:
            assert(0);
    }

	return false;
}



bool MPDPeriod::getVideoInfo(uint32_t index, uint64_t &bandwidth, std::string &codecs, uint32_t &height, 
    uint32_t &width, PeriodNumberType mType)
{
    uint32_t count = 0;
    
    switch(mType)
    {
        case PERIOD_ADP:
            {
                // for video, it may use by period_rep
                
            }
        return false;

        case PERIOD_REP:
            {
                count = mVideoAdapList.size();
                uint32_t adpInd = 0;

                for(; adpInd < count; ++ adpInd)
                {
                	MPDAdaptationset *adp = (MPDAdaptationset *)mVideoAdapList[adpInd];
                    assert(adp != NULL);
				
                    uint32_t repCount = adp->getRepresentationSize();
                    if(repCount <= index)
                    {
                        index -= repCount;
                    }
                    else
                    {
                        bandwidth = adp->getBandwidthByIndex(index);
                        codecs = adp->getCodecsByIndex(index);
                        height = adp->getHeightByIndex(index);
                        width = adp->getWidthByIndex(index);

                        return true;
                    }
                }
            }
        return false;

        default:
            assert(0);
    }

	return false;
}
bool MPDPeriod::getSubtitleInfo(uint32_t index, std::string &language, uint32_t &height, 
    uint32_t &width, PeriodSubtitleType &type, PeriodNumberType mType)
{
    switch(mType)
    {
        case PERIOD_ADP:
            {
                // for subtitle, it may use by period_adp
                
                assert(mAudioAdapList.size() > index);

				MPDAdaptationset *adp = (MPDAdaptationset *)mAudioAdapList[index];
                assert(adp != NULL);
                width = 0;
                language = adp->getLanguage();
                height = 0;
                type = (PeriodSubtitleType)adp->getSubtitleType();
            }
        return true;

        case PERIOD_REP:
            {
                // now rep can't be supported
            }
        return false;

        default:
            assert(0);
    }

	return false;
}


bool MPDPeriod::setAudioPlayIndex(uint32_t index, PeriodNumberType mType)
{
    if(mType == PERIOD_REP)
    {
        // audio may not support REP

        MPD_ERROR("audio index only supports ADP");
        return false;
    }

    assert(index < mAudioAdapList.size());
    mAudioIndex = index;
    return true;
}
bool MPDPeriod::setSubtitleIndex(uint32_t index, PeriodNumberType mType)
{
    if(mType == PERIOD_REP)
    {
        // audio may not support REP

        MPD_ERROR("subtitle index only supports ADP");
        return false;
    }
    assert(index < mSubtitleAdapList.size());
    mSubtitleIndex = index;
    return true;
}


bool MPDPeriod::setVidoPlayIndex(uint32_t index, PeriodNumberType mType)
{
    if(mType == PERIOD_ADP)
    {
        assert(index < mVideoAdapList.size());
        mVideoIndex = index;
        return true;
    }
    else
    {
        uint32_t count = mVideoAdapList.size();
        uint32_t adpInd = 0;
        mVideoIndex = 0;
        
        for(;adpInd < count; ++ adpInd)
        {
        	MPDAdaptationset *adp = (MPDAdaptationset *)mVideoAdapList[adpInd];
            assert(adp != NULL);
			
            if(index >= adp->getRepresentationSize())
            {
                index -= adp->getRepresentationSize();
            }
            else
            {
                mVideoIndex = adpInd;
                adp->setCurrentRepIndex((int)index);
                return true;
            }
        }
    }

    MPD_ERROR("index is error");
    return false;;
}


MPDPeriod::PeriodDRMType MPDPeriod::getDrmType()
{
    PeriodDRMType type = PERIOD_DRM_NONE;

    uint32_t count = mAudioAdapList.size();
    uint32_t index = 0;

    for(;index < count; ++ index)
    {
    	MPDAdaptationset *adp = (MPDAdaptationset *)mAudioAdapList[index];
        assert(adp != NULL);
		
        type = (PeriodDRMType)adp->getDrmType();
        if(type != PERIOD_DRM_NONE)
        {
            return type;
        }
    }

    count = mVideoAdapList.size();
    index = 0;

    for(;index < count; ++ index)
    {
    	MPDAdaptationset *adp = (MPDAdaptationset *)mVideoAdapList[index];
        assert(adp != NULL);
		
        type = (PeriodDRMType)adp->getDrmType();
        if(type != PERIOD_DRM_NONE)
        {
            return type;
        }
    }

    count = mSubtitleAdapList.size();
    index = 0;

    for(;index < count; ++ index)
    {
    	MPDAdaptationset *adp = (MPDAdaptationset *)mSubtitleAdapList[index];
        assert(adp != NULL);
		
        type = (PeriodDRMType)adp->getDrmType();
        if(type != PERIOD_DRM_NONE)
        {
            return type;
        }
    }

    return PERIOD_DRM_NONE;
}


bool MPDPeriod::findRepForVideoByBitrate(MPDAdaptationset **adp, MPDRepresentation **rep, uint64_t bitrate)
{
    if(adp == NULL || rep == NULL)
    {
        MPD_ERROR("impossible for get rep for video");
        return false;
    }
    
    if(mVideoIndex != 0 && mVideoIndex < mVideoAdapList.size())
    {
        *adp = (MPDAdaptationset *)mVideoAdapList[mVideoIndex];
		assert(*adp != NULL);

        if(!(*adp)->findRepByBandwidth(rep, bitrate))
        {
            MPD_ERROR("error for find rep in adp");

            return false;
        }

        return true;
    }
    else
    {
        mVideoIndex = 0;
        int64_t minBitrateGap = INT64_MAX;
        uint64_t adpIndex = 0;
        int64_t bandwitdhGap = 0;
        MPDRepresentation *temp = NULL;

        uint32_t count = mVideoAdapList.size();
        for(uint32_t index = 0; index < count; ++ index)
        {

			MPDAdaptationset *adpt = (MPDAdaptationset *)mVideoAdapList[index];
        	assert(adpt != NULL);
		
            if(!adpt->findRepByBandwidthWithGap(bitrate, bandwitdhGap, &temp))
            {
                MPD_ERROR("error for find rep in adp");

                return false;
            }

            if(bandwitdhGap == 0)
            {
                *rep = temp;
                *adp = adpt;
                return true;
            }
            // > 0
            else if(bandwitdhGap > 0)
            {
                if(minBitrateGap > bandwitdhGap
                    || minBitrateGap < 0)
                {
                    *rep = temp;
                    *adp = adpt;
                    minBitrateGap = bandwitdhGap;
                }
            }
            else
            {
                // <0, and 
                if(bandwitdhGap > minBitrateGap
                    || (minBitrateGap == INT64_MAX))
                {
                    *rep = temp;
                    *adp = adpt;
                    minBitrateGap = bandwitdhGap;
                }
            }
        }

        if(*rep == NULL || *adp == NULL)
        {
            MPD_ERROR("error , adp and rep can't be found");
            return false;
        }

        return true;
    }
}



std::string MPDPeriod::getBaseUrl()
{
    if(mBaseUrlList.size() > 0)
    {
    	MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[0];
		assert(base != NULL);
        return base->getBaseUrl();
    }

    return std::string("");
}

bool MPDPeriod::findRepForAudio(MPDAdaptationset **adp, MPDRepresentation **rep)
{
    if(adp == NULL || rep == NULL)
    {
        assert(0);
    }

    if(mAudioIndex > 0 && mAudioAdapList.size() > mAudioIndex)
    {
        *adp = (MPDAdaptationset *)mAudioAdapList[mAudioIndex];
		assert(*adp != NULL);
        if(!(*adp)->findRep(rep))
        {
            MPD_ERROR("no rep in ");
            assert(0);
        }

        return true;
    }

    mAudioIndex = 0;
    if(mAudioAdapList.size() > 0)
    {
        *adp = (MPDAdaptationset *)mAudioAdapList[0];
		assert(*adp != NULL);
        if(!(*adp)->findRep(rep))
        {
            MPD_ERROR("no rep in ");
            assert(0);
        }

        return true;
    }

    *adp = NULL;
    *rep = NULL;

    return true;
}

bool MPDPeriod::findRepForSubtitle(MPDAdaptationset **adp, MPDRepresentation **rep)
{
    if(adp == NULL || rep == NULL)
    {
        assert(0);
    }

    if(mSubtitleIndex > 0 && mSubtitleAdapList.size() > mSubtitleIndex)
    {
        *adp = (MPDAdaptationset *)mSubtitleAdapList[mSubtitleIndex];
		assert(*adp != NULL);
		
        if(!(*adp)->findRep(rep))
        {
            MPD_ERROR("no rep in ");
            assert(0);
        }

        return true;
    }

    mSubtitleIndex = 0;
    if(mSubtitleAdapList.size() > 0)
    {
        *adp = (MPDAdaptationset *)mSubtitleAdapList[0];
		assert(*adp != NULL);
        if(!(*adp)->findRep(rep))
        {
            MPD_ERROR("no rep in ");
            assert(0);
        }

        return true;
    }

    *adp = NULL;
    *rep = NULL;

    return true;
}


#if MPD_PLAYLISTS_DEBUG
void MPDPeriod::periodDebug()
{
    MPD_DEBUG("\n****************\n");

    MPD_DEBUG("video index = %d, audio index = %d", mVideoIndex, mAudioIndex);
    MPD_DEBUG("subtitle index= %d, thumb index = %d", mSubtitleIndex, mThumbnailIndex);
    MPD_DEBUG("start = %d, duration = %d", mStartTime, mDuration);


    uint32_t count = mBaseUrlList.size();
    uint32_t index = 0;

    for(;index < count; ++ index)
    {
    	MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[index];
		assert(base != NULL);
		
        base->baseUrlDebug();
    }

    count = mVideoAdapList.size();
    index = 0;

    MPD_DEBUG("video debug ::: \n");
    for(;index < count; ++ index)
    {
    	MPDAdaptationset *adp = (MPDAdaptationset *)mVideoAdapList[index];
		assert(adp != NULL);
		
        adp->adpDebug();
    }


    count = mAudioAdapList.size();
    index = 0;

    MPD_DEBUG("audio debug ::: \n");
    for(; index < count; ++ index)
    {
	    MPDAdaptationset *adp = (MPDAdaptationset *)mAudioAdapList[index];
		assert(adp != NULL);
	    adp->adpDebug();
    }

    count = mSubtitleAdapList.size();
    index = 0;
    MPD_DEBUG("subtitle debug ::: \n");
    for(; index < count; ++ index)
    {
    	 MPDAdaptationset *adp = (MPDAdaptationset *)mSubtitleAdapList[index];
		 assert(adp != NULL);
         adp->adpDebug();
    }

    count = mThumbnailAdapList.size();
    index = 0;

    MPD_DEBUG("thumb debug ::: \n");
    for(; index < count; ++ index)
    {
         MPDAdaptationset *adp = (MPDAdaptationset *)mThumbnailAdapList[index];
		 assert(adp != NULL);
         adp->adpDebug();
    }
    
}

#endif


