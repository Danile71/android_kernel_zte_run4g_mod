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
#include "MPDAdaptationset.h"

using namespace android_dash;

MPDAdaptationset::MPDAdaptationset()
{
    mSubsegmentAlign = 0;
    mRole = ADP_ROLE_UNKNOWN;
    mKind = ADP_KIND_NONE;
    mAccess = ADP_ACCESS_NONE;
    mCurrentRepresentIndex = -1;

    mLanguage = std::string("");
    mMimeType = std::string("");
    mAudioChannel = std::string("");
    mCodecs = std::string("");
    mFramerate = std::string("");
    mSamplerate = std::string("");

    mDrmType = ADP_DRM_NONE;
	freeMemory();
}



MPDAdaptationset::~MPDAdaptationset()
{
    mSubsegmentAlign = 0;
    mRole = ADP_ROLE_UNKNOWN;
    mKind = ADP_KIND_NONE;
    mDrmType = ADP_DRM_NONE;
    mAccess = ADP_ACCESS_NONE;
    mCurrentRepresentIndex = -1;

    mLanguage = std::string("");
    mMimeType = std::string("");
    mAudioChannel = std::string("");
    mCodecs = std::string("");
    mFramerate = std::string("");
    mSamplerate = std::string("");

	
    freeMemory();
}

void MPDAdaptationset::freeMemory()
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


	count = mRepresentationList.size();
	if(count > 0)
	{
	    for(index = 0; index < count; ++index)
	    {
	        MPDRepresentation *ptr = (MPDRepresentation *)mRepresentationList[index];
			assert(ptr != NULL);
			delete ptr;
	    }

		mRepresentationList.clear();
	}


	count = mDrmDataList.size();
	if(count > 0)
	{
	    for(index = 0; index < count; ++index)
	    {
	        MPDContentProtection *ptr = (MPDContentProtection *)mDrmDataList[index];
			assert(ptr != NULL);
			delete ptr;
	    }

		mDrmDataList.clear();
	}
}


bool MPDAdaptationset::parserAdp(MPDSchema::CPeriodType *period, MPDSchema::CPeriodType_ex *period_ex,
    MPDSchema::CAdaptationSetType *adp, MPDSchema::CAdaptationSetType_ex *adp_ex)
{
    assert(adp != NULL);
    assert(adp_ex != NULL);


    // for base url create
    uint32_t count = adp->BaseURL.count();
    uint32_t index = 0;
    if(count > 0)
    {
        
        for(; index < count; ++ index)
        {
        	MPDBaseUrl *baseUrl = new MPDBaseUrl();
			if(baseUrl == NULL)
			{
			    MPD_ERROR("no memory");
				return false;
			}
            baseUrl->setBaseUrl((std::string)adp->BaseURL[index]);

            
            if(adp->BaseURL[index].byteRange.exists())
            {
                baseUrl->setByteRange((std::string)(adp->BaseURL[index].byteRange));
            }

            mBaseUrlList.push_back(*((uint64_t *)&baseUrl));
        }
    }


    // subsegment align value
    if(adp->subsegmentAlignment.exists())
    {
        std::string subsegment = (std::string)adp->subsegmentAlignment;
        if(subsegment.compare("true") == 0)
        {
            mSubsegmentAlign = 1;
        }
    }


    ///TODO:: role and kind saving


    // got mime string type
    if(adp_ex->mimeType_exists)
    {
        mMimeType = adp_ex->mime_str;
    }


    // get mime type
    switch(adp_ex->content_type)
    {
        case dash_enum::audio:
            mType = ADP_AUDIO;
            break;
        case dash_enum::subtitle:
            mType = ADP_SUBTITLE;
            break;
        case dash_enum::thumbnail:
            mType = ADP_THUMBNAIL;
            break;
        default:
            mType = ADP_VIDEO;
            break;
    }

    ///TODO:: get subtitle type by content type maybe 
    mSubtitleType = 0;

    // language setup
    if(adp->lang.exists())
    {
        mLanguage = (std::string)adp->lang;
    }

    // codecs setup
    if(adp_ex->codec_exists)
	{
	    mCodecs = adp_ex->codec_str;
	}

    // audio channel setup
    if(adp->AudioChannelConfiguration.exists())
    {
        if(adp->AudioChannelConfiguration[0].value2.exists())
        {
            mAudioChannel = (std::string)adp->AudioChannelConfiguration[0].value2;
        }
    }

    // frame rate setup
    if(adp_ex->frameRate_exists)
	{
	    mFramerate = adp_ex->frameRate_str;
	}

    // sample rate setup
    if(adp_ex->audioSampleRate_exists)
	{
	    mSamplerate = adp_ex->audioSampleRate_str;
	}


    
    count = adp->Representation.count();
    assert(count == adp_ex->representation_list.size());
    std::vector<uint64_t> repTemp;

	
    for(index = 0; index < count; ++ index)
    {
    	
        MPDRepresentation *representation =  new MPDRepresentation();
		if(representation == NULL)
		{
		    MPD_ERROR("no memory");
			return false;
		}
		
        MPDSchema::CRepresentationType *rep = &adp->Representation[index];
        MPDSchema::CRepresentationType_ex *rep_ex = &adp_ex->representation_list[index];
        if(!representation->parserRep(period, period_ex, adp, adp_ex,
            rep, rep_ex))
        {
            MPD_ERROR("representation parser error in adp");
			delete representation;
            return false;
        }

        // video for sorted
        if(mType == ADP_VIDEO)
        {
            repTemp.push_back(*((uint64_t*)&representation));
        }
        else
        {
            mRepresentationList.push_back(*((uint64_t*)&representation));
        }
    }


    // for DRM data collection

	// MPD_DEBUG("adp drm number = %d", adp->ContentProtection.count());
    if(adp->ContentProtection.exists())
    {
        count = adp->ContentProtection.count();
        for(index = 0; index < count; ++ index)
        {
            MPDSchema::CContentProtectionType *drm = &adp->ContentProtection[index];
            MPDContentProtection *protection = new MPDContentProtection();

			if(protection == NULL)
			{
			    MPD_ERROR("no memory");
				return false;
			}

            if(!protection->parserPc(drm))
            {
                MPD_ERROR("drm parser error");
				delete protection;
                return false;
            }

            mDrmDataList.push_back(*((uint64_t*)&protection));
        }
    }


    // sorted by bandwidth, and it will be easy in bandwidth switch
    
    if(mType == ADP_VIDEO)
    {
        count = repTemp.size();
        uint32_t sortIndex = 0;
		
        for(sortIndex = 0; sortIndex < count; ++ sortIndex)
        {
            uint32_t findIndex = 0;
            uint64_t minBandwidth = 0xFFFFFFFF;
            for(index = 0; index < count; ++ index)
            {
            	MPDRepresentation *rep = (MPDRepresentation *)repTemp[index];
				assert(rep != NULL);
				
                if(rep->getBitrate() < minBandwidth
                    && !rep->getSorted())
                    
                {
                    findIndex = index;
                    minBandwidth = rep->getBitrate();
                }
            }

			MPDRepresentation *rept = (MPDRepresentation *)repTemp[findIndex];
			assert(rept != NULL);			
            rept->setSorted(true);
            mRepresentationList.push_back(repTemp[findIndex]);	
        }

        assert(mRepresentationList.size() == count);
    }

    return true;
}


bool MPDAdaptationset::setBandwidth(uint64_t bandwidth)
{
    assert(mRepresentationList.size() != 0);

    uint32_t count = mRepresentationList.size();
    uint32_t index = 0;
    for(;index < count; ++ index)
    {
    	MPDRepresentation *rep = (MPDRepresentation *)mRepresentationList[index];
		assert(rep != NULL);
		
        if(rep->getBandwidth() > bandwidth)
        {
            if(index != 0)
            {
                mCurrentRepresentIndex = index - 1;
                
            }
            else
            {
                mCurrentRepresentIndex = 0;
            }

            return true;
        }
    }

    if(index == count)
    {
        mCurrentRepresentIndex = count - 1;
    }

    return true;
}


uint64_t MPDAdaptationset::getBandwidthByIndex(uint32_t index)
{
    assert(index < mRepresentationList.size());
	MPDRepresentation *rep = (MPDRepresentation *)mRepresentationList[index];
	assert(rep != NULL);
	
    return rep->getBitrate();
}
std::string MPDAdaptationset::getCodecsByIndex(uint32_t index)
{
    assert(index < mRepresentationList.size());
	MPDRepresentation *rep = (MPDRepresentation *)mRepresentationList[index];
	assert(rep != NULL);
	
    return rep->getCodecs();
}


uint32_t MPDAdaptationset::getHeightByIndex(uint32_t index)
{
    assert(index < mRepresentationList.size());
	MPDRepresentation *rep = (MPDRepresentation *)mRepresentationList[index];
	assert(rep != NULL);
	
    return rep->getHeight();
}


uint32_t MPDAdaptationset::getWidthByIndex(uint32_t index)
{
    assert(index < mRepresentationList.size());
	MPDRepresentation *rep = (MPDRepresentation *)mRepresentationList[index];
	assert(rep != NULL);
    return rep->getWidth();
}


MPDAdaptationset::AdaptDrmType MPDAdaptationset::getDrmType()
{
    uint32_t count = mDrmDataList.size();
    uint32_t index = 0;

    AdaptDrmType type = ADP_DRM_NONE;

    for(; index < count; ++ index)
    {
    	MPDContentProtection *ptr = (MPDContentProtection *)mDrmDataList[index];
		assert(ptr != NULL);
		
        type = (AdaptDrmType)ptr->getType();
        if(type != ADP_DRM_NONE)
        {
            return type;
        }
    }


    count = mRepresentationList.size();
    for(index = 0; index < count; ++ index)
    {
    	MPDRepresentation *rep = (MPDRepresentation *)mRepresentationList[index];
		assert(rep != NULL);
		
        type = (AdaptDrmType)rep->getDrmType();

        if(type != ADP_DRM_NONE)
        {
            return type;
        }
        
    }

    return ADP_DRM_NONE;
}

bool MPDAdaptationset::findRepByBandwidth(MPDRepresentation **rep, uint64_t bitrate)
{
    if(rep == NULL)
    {
        MPD_ERROR("error parameter");
        return false;
    }

    uint32_t count = mRepresentationList.size();
    uint32_t index = 0;

    *rep = NULL;

	if(mCurrentRepresentIndex >= 0 && mCurrentRepresentIndex < count)
	{
	    *rep = (MPDRepresentation *)mRepresentationList[mCurrentRepresentIndex];
		assert(*rep != NULL);
        return true;
	}

	// mCurrentRepresentIndex = 0;
    
    for(;index < count; ++ index)
    {
    	MPDRepresentation *rept = (MPDRepresentation *)mRepresentationList[index];
		assert(rept != NULL);
        if(rept->getBitrate() < bitrate)
        {
            continue;
        }
        // find it
        else if(rept->getBitrate() == bitrate)
        {
            *rep = (MPDRepresentation *)mRepresentationList[index];
			assert(*rep != NULL);
            return true;
        }
        else
        {
            // all bigger than bitrate
            if(index == 0)
            {
                *rep = (MPDRepresentation *)mRepresentationList[0];
				assert(*rep != NULL);
            }
            else
            {
                // find the biggest one less than bitrate
                *rep = (MPDRepresentation *)mRepresentationList[index - 1];
				assert(*rep != NULL);
            }

            return true;
        }
    }

    // final one if no found 
    if(index > 0)
    {
        *rep = (MPDRepresentation *)mRepresentationList[index - 1];
		assert(*rep != NULL);
    }
    else
    {
        MPD_ERROR("no rep in adp");
        return false;
    }


    return true;
}


bool MPDAdaptationset::findRepByBandwidthWithGap(uint64_t bitrate, int64_t &bandwitdhGap, MPDRepresentation **temp)
{
    if(temp == NULL)
    {
        MPD_ERROR("error parameter");
        return false;
    }

    uint32_t count = mRepresentationList.size();
    uint32_t index = 0;

    *temp = NULL;

	if(mCurrentRepresentIndex >= 0 && mCurrentRepresentIndex < count)
	{
	    *temp = (MPDRepresentation *)mRepresentationList[mCurrentRepresentIndex];
		assert(*temp != NULL);
		bandwitdhGap = 0;
        return true;
	}

	// mCurrentRepresentIndex = 0;
	
    for(;index < count; ++index)
    {
    	MPDRepresentation *rept = (MPDRepresentation *)mRepresentationList[index];
		assert(rept != NULL);
		
        if(rept->getBitrate() < bitrate)
        {
            continue;
        }
        // find it
        else if(rept->getBitrate() == bitrate)
        {
            *temp = (MPDRepresentation *)mRepresentationList[index];
			assert(*temp != NULL);
            bandwitdhGap = 0;
            return true;
        }
        else
        {
            // all bigger than bitrate
            if(index == 0)
            {
                *temp = (MPDRepresentation *)mRepresentationList[0];
				assert(*temp != NULL);
                
            }
            else
            {
                // find the biggest one less than bitrate
                *temp = (MPDRepresentation *)mRepresentationList[index - 1];
				assert(*temp != NULL);
            }
            
            bandwitdhGap = bitrate - (*temp)->getBitrate();
            return true;
        }
    }

    // final one if no found
    if(index > 0)
    {   
        *temp = (MPDRepresentation *)mRepresentationList[index - 1];
		assert(*temp != NULL);
        bandwitdhGap = bitrate - (*temp)->getBitrate();
    }
    else
    {
        MPD_ERROR("error adp has no rep, drop it");
        bandwitdhGap = INT64_MAX;
    }

    return true;

}

std::string MPDAdaptationset::getBaseUrl()
{
    if(mBaseUrlList.size() > 0)
    {
    	MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[0];
		assert(base != NULL);
        return base->getBaseUrl();
   	}

	return std::string("");
}

bool MPDAdaptationset::findRep(MPDRepresentation **temp)
{
    assert(temp != NULL);

	///TODO:: maybe according to ... find a rep
	if(mRepresentationList.size() > 0)
	{
		*temp = (MPDRepresentation *)mRepresentationList[0];
		assert(*temp != NULL);
		return true;
	}

	*temp = NULL;

	return true;
}


#if MPD_PLAYLISTS_DEBUG
void MPDAdaptationset::adpDebug()
{
    MPD_DEBUG("\n ------------------- \n");
	MPD_DEBUG("align = %d, role = %d, access = %d", mSubsegmentAlign, mRole, mAccess);

	MPD_DEBUG("kind = %d type = %d, drm type = %d", mKind, mType, mDrmType);
	MPD_DEBUG("rep index = %d, subtitle = %d", mCurrentRepresentIndex, mSubtitleType);

	MPD_DEBUG("lang = %s, mimetype = %s, channel = %s, codecs = %s",
		mLanguage.c_str(), mMimeType.c_str(), mAudioChannel.c_str(), mCodecs.c_str());

	MPD_DEBUG("frame = %s sample = %s", mFramerate.c_str(), mSamplerate.c_str());

	uint32_t count = mBaseUrlList.size();
	uint32_t index = 0;

	for(;index < count; ++ index)
	{
		MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[index];
		assert(base != NULL);
	    base->baseUrlDebug();
	}

	count = mRepresentationList.size();
	index = 0;

	MPD_DEBUG("rep debug");
	for(; index < count; ++ index)
	{
		MPDRepresentation *rept = (MPDRepresentation *)mRepresentationList[index];
		assert(rept != NULL);
	    rept->repDebug();
	}


	
	count = mDrmDataList.size();
	MPD_DEBUG("drm debug count = %d", count);
	for(index = 0;index < count; ++ index)
	{
		MPDContentProtection *ptr = (MPDContentProtection *)mDrmDataList[index];
		assert(ptr != NULL);
	    ptr->cpDebug();
	}
}
#endif



