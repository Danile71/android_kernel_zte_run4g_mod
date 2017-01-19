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

// representation setup 
#include "MPDRepresentation.h"

using namespace android_dash;

MPDRepresentation::MPDRepresentation()
{
    mTimeScale = 1;
    mWidth = 0;
    mHeight = 0;
    mMaxPlayrate = 1.0;
    mBitrate = 0;
    mSorted = false;
    mDrmType = MPDContentProtection::CP_DRM_NONE;
    freeMemory();
}

MPDRepresentation::~MPDRepresentation()
{
    mTimeScale = 1;
    mWidth = 0;
    mHeight = 0;
    mMaxPlayrate = 1.0;
    mBitrate = 0;
    mSorted = false;

    freeMemory();
}


void MPDRepresentation::freeMemory()
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


    count = mDrmList.size();
    if(count > 0)
    {
        for(index = 0; index < count; ++index)
        {
            MPDContentProtection *ptr = (MPDContentProtection *)mDrmList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mDrmList.clear();
    }


    count = mIndexList.size();
    if(count > 0)
    {
        for(index = 0; index < count; ++ index)
        {
            MPDSegment *ptr = (MPDSegment *)mIndexList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mIndexList.clear();
    }

    count = mMediaList.size();
    if(count > 0)
    {
        for(index = 0; index < count; ++ index)
        {
            MPDSegment *ptr = (MPDSegment *)mMediaList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mMediaList.clear();
    }

    count = mBitstreamList.size();
    if(count > 0)
    {
        for(index = 0; index < count; ++ index)
        {
            MPDSegment *ptr = (MPDSegment *)mBitstreamList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mBitstreamList.clear();
    }

    count = mSubrepresentationList.size();

    if(count > 0)
    {
        for(index = 0; index < count; ++ index)
        {
            MPDSubrepresentation *ptr = (MPDSubrepresentation *)mSubrepresentationList[index];
            assert(ptr != NULL);
            delete ptr;
        }

        mSubrepresentationList.clear();
    }

}

bool MPDRepresentation::parserRep(MPDSchema::CPeriodType *period, MPDSchema::CPeriodType_ex *period_ex,
            MPDSchema::CAdaptationSetType *adp, MPDSchema::CAdaptationSetType_ex *adp_ex,
            MPDSchema::CRepresentationType *rep, MPDSchema::CRepresentationType_ex *rep_ex)
{

    assert(period != NULL);
    assert(period_ex != NULL);
    assert(adp != NULL);
    assert(adp_ex != NULL);
    assert(rep != NULL);
    assert(rep_ex != NULL);

    
    if(rep_ex->bandwidth_exists)
    {
        mBitrate = rep_ex->bandwidth_ex;
    }

    if(rep_ex->width_exists)
    {
        mWidth = rep_ex->width;
    }

    if(rep_ex->height_exists)
    {
        mHeight = rep_ex->height;
    }

    // base url setup  
    uint32_t count = rep->BaseURL.count();
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
            
            baseUrl->setBaseUrl((std::string)rep->BaseURL[index]);

            
            if(rep->BaseURL[index].byteRange.exists())
            {
                baseUrl->setByteRange((std::string)(rep->BaseURL[index].byteRange));
            }

            mBaseUrlList.push_back(*((uint64_t*)&baseUrl));
        }
    }


    // max playout rate
    if(rep->maxPlayoutRate.exists())
    {
        mMaxPlayrate = (double)rep->maxPlayoutRate;
    }
    else if(adp->maxPlayoutRate.exists())
    {
        mMaxPlayrate = (double)adp->maxPlayoutRate;
    }

    if(rep->codecs.exists())
    {
        mCodecs = (std::string)rep->codecs;
    }

    // drm information setup
    if(rep->ContentProtection.exists())
    {
        count = rep->ContentProtection.count();
        for(index = 0; index < count; ++ index)
        {
            MPDSchema::CContentProtectionType *drm = &rep->ContentProtection[index];
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

            mDrmList.push_back(*((uint64_t*)&protection));
        }
    }

    
    if(mDrmList.size() > 0)
    {
        count = mDrmList.size();
        for(index = 0; index < count; ++ index)
        {
            MPDContentProtection *protection = (MPDContentProtection *)mDrmList[index];
            assert(protection != NULL);
            if(protection->getType() != MPDContentProtection::CP_DRM_NONE)
            {
                mDrmType = protection->getType();
            }
        }
    }

    // subrepresentation setup

    if(rep->SubRepresentation.exists())
    {
        count = rep->SubRepresentation.count();
        for(index = 0; index < count; ++ index)
        {
            MPDSubrepresentation *subrep = new MPDSubrepresentation();
            if(subrep == NULL)
            {
                MPD_ERROR("no memory");
                return false;
            }
            if(!subrep->parserSubrep(&rep->SubRepresentation[index]))
            {
                MPD_ERROR("subrep create error in representation");
                delete subrep;
                return false;
            }

            mSubrepresentationList.push_back(*((uint64_t*)&subrep));
        }
    }


    // create url information
    if(!initSegmentUrl(period, period_ex, adp, adp_ex, rep, rep_ex))
    {
        MPD_ERROR("segment url create error");
        return false;
    }

    return true;
    
}


// no segment information in MPD
bool MPDRepresentation::initSegmentFromNone(double start, double duration)
{
    MPDSegment *segment = new MPDSegment();
    if(segment == NULL)
    {
        MPD_ERROR("no memory");
        return false;
    }
    segment->mStartTime = (uint64_t)start;
    segment->mDuration = (uint64_t)duration;
    mMediaList.push_back(*((uint64_t*)&segment));
    return true;
}


// 
bool MPDRepresentation::initSegmentFromBase(MPDSchema::CSegmentBaseType_ex *base_ex, double start, double duration)
{
    assert(base_ex != NULL);
    std::string url = std::string("");
    std::string range = std::string("");

    // initial url:

    if(base_ex->Initialisation_exists)
    {
        url = base_ex->Initialisation_ex;
        if(base_ex->Initialisation_byteRange_exists)
        {
            range = base_ex->Initialisation_byteRange_ex;
        }

        if(url.length() > 0 || range.length() > 0)
        {
            
            mInitList.mStartTime = start;
            mInitList.mDuration = duration;
            

            
            uint32_t len = url.length() + 1;

            if(len > 1)
            {
                mInitList.mUrl = url;
            }

            len = range.length() + 1;

            if(len > 1)
            {
                mInitList.mRangeFirst = dash_handle::handle_byte_range(range, mInitList.mRangeLast);
            }

            
            
        }
    }

    //index url
    url = std::string("");
    range = std::string("");
    
    if(base_ex->RepresentationIndex_exists)
    {
        url = base_ex->RepresentationIndex_ex;
        if(base_ex->RepresentationIndex_byteRange_exists)
        {
            range =  base_ex->RepresentationIndex_byteRange_ex;
        }

        if(url.length() > 0 || range.length() > 0)
        {
            MPDSegment *segment = new MPDSegment();
            if(segment == NULL)
            {
                MPD_ERROR("no memory");
                return false;
            }
            segment->mStartTime = start;
            segment->mDuration = duration;

            uint32_t len = url.length() + 1;

            if(len > 1)
            {
                segment->mUrl = url;
            }

            len  = range.length() + 1;

            if(len > 1)
            {
                segment->mRangeFirst = dash_handle::handle_byte_range(range, segment->mRangeLast);
            }
            
            mIndexList.push_back(*((uint64_t*)&segment));
        }
    }
    // no bitstream url

    
    // media struct
    {
        MPDSegment *segment = new MPDSegment();
        if(segment == NULL)
        {
            MPD_ERROR("no memory");
            return false;
        }
        
        segment->mDuration = duration;
        
        if(!base_ex->presentationTimeOffset_exists)
        {
            segment->mStartTime = start;
        }
        else
        {
            segment->mStartTime = base_ex->presentationTimeOffset_ex;
        }

        mMediaList.push_back(*((uint64_t*)&segment));
        
    }

    //set timescale
    if(base_ex->timescale_exists)
    {
        mTimeScale = base_ex->timescale_ex;
    }

    return true;

}


bool MPDRepresentation::initSegmentFromList(MPDSchema::CSegmentListType_ex *list_ex, 
    MPDSchema::CSegmentListType* list,double start, double duration)
{
    assert(list_ex != NULL);
    assert(list != NULL);

    std::string url = std::string("");
    std::string range = std::string("");
    uint64_t dur = 0;

    if(list_ex->timescale_exists)
    {
        mTimeScale = list_ex->timescale_ex;
    }

    uint64_t start_t = start * mTimeScale;
    uint64_t duration_t = duration * mTimeScale;

    //initial url
    if(list_ex->Initialisation_exists)
    {
        url = list_ex->Initialisation_ex;
        if(list_ex->Initialisation_byteRange_exists)
        {
            range = list_ex->Initialisation_byteRange_ex;
        }

        if(url.length() > 0 || range.length() > 0)
        {
            mInitList.mStartTime = start_t;
            mInitList.mDuration = duration_t;

            
            uint32_t len = url.length() + 1;

            if(len > 1)
            {
                mInitList.mUrl = url;
            }

            len  = range.length() + 1;

            if(len > 1)
            {
                mInitList.mRangeFirst = dash_handle::handle_byte_range(range, mInitList.mRangeLast);
            } 
        }
    }


    url = std::string("");
    range = std::string("");
    
    //bitstream url
    if(list_ex->BitstreamSwitching_exists)
    {
        url = list_ex->BitstreamSwitching_ex;
        if(list_ex->BitstreamSwitching_byteRange_exists)
        {
            range = list_ex->BitstreamSwitching_byteRange_ex;
        }

        if(url.length() > 0 || range.length() > 0)
        {
           MPDSegment *segment = new MPDSegment();
            if(segment == NULL)
            {
                MPD_ERROR("no memory");
                return false;
            }
            
            segment->mStartTime = start_t;
            segment->mDuration = duration_t;

            
            uint32_t len = url.length() + 1;

            if(len > 1)
            {
                segment->mUrl = url;
            }

            len  = range.length() + 1;

            if(len > 1)
            {
                segment->mRangeFirst = dash_handle::handle_byte_range(range, segment->mRangeLast);
            }

            mBitstreamList.push_back(*((uint64_t*)&segment));
        }
    }


    url = std::string("");
    range = std::string("");
    
    //index url
    if(list_ex->RepresentationIndex_exists)
    {
        url = list_ex->RepresentationIndex_ex;
        if(list_ex->RepresentationIndex_byteRange_exists)
        {
            range =  list_ex->RepresentationIndex_byteRange_ex;
        }

        if(url.length() > 0 || range.length() > 0)
        {
            MPDSegment *segment = new MPDSegment();
            if(segment == NULL)
            {
                MPD_ERROR("no memory");
                return false;
            }

            
            segment->mStartTime = start_t;
            segment->mDuration = duration_t;

            
            uint32_t len = url.length() + 1;

            if(len > 1)
            {
                segment->mUrl = url;
            }

            len  = range.length() + 1;

            if(len > 1)
            {
                segment->mRangeFirst = dash_handle::handle_byte_range(range, segment->mRangeLast);
            }

            
            mIndexList.push_back(*((uint64_t*)&segment));
        }
    }


    // :) to media url: in this case, maybe multi media url need to handle, and multi index url

    if(list->SegmentURL.exists())
    {
        uint32_t size = list->SegmentURL.count();
        uint32_t begin = 0;

        if(list_ex->duration_exists)
        {
            dur = list_ex->duration_ex;
        }
        else
        {
            // if no duration, can not handle media url list
            assert(0);
        }

        
        for(; begin < size; ++ begin)
        {
            MPDSchema::CSegmentURLType* segment_url = &list->SegmentURL[begin];

            // to handle index range
            if(segment_url->index.exists() || segment_url->indexRange.exists())
            {
                if(begin == 0 && mIndexList.size() != 0)
                {
                    // there is index for all media list, but there is one index url create before.
                    assert(0);
                }
                
                MPDSegment *segment = new MPDSegment();
                if(segment == NULL)
                {
                    MPD_ERROR("no memory");
                    return false;
                }

                url = std::string("");
                range = std::string("");
                
                if(segment_url->index.exists())
                {
                    url = (std::string)segment_url->index;
                }
                
                if(segment_url->indexRange.exists())
                {
                    range = (std::string)segment_url->indexRange;
                }

                segment->mStartTime = start_t + dur * begin;
                segment->mDuration = dur;
                
                if(url.length() > 0)
                {
                    segment->mUrl = url;
                }

                if(range.length() > 0)
                {
                    segment->mRangeFirst = dash_handle::handle_byte_range(range, segment->mRangeLast);
                }

                if(segment->mStartTime + segment->mDuration > start_t + duration_t)
                {
                    segment->mDuration = start_t + duration_t - segment->mStartTime;
                    if(segment->mDuration == 0)
                    {
                        segment->mDuration = 1 * mTimeScale;  // it maybe less than 1s (double)
                    }
                }

                mIndexList.push_back(*((uint64_t*)&segment));
            }

            url = std::string("");
            range = std::string("");
            if(segment_url->media.exists() ||segment_url->mediaRange.exists())
            {
                MPDSegment *segment = new MPDSegment();
                if(segment == NULL)
                {
                    MPD_ERROR("no memory");
                    return false;
                }
                
                segment->mStartTime = start_t + dur * begin;
                segment->mDuration = dur;

                
                if(segment_url->media.exists())
                {
                    url = (std::string)segment_url->media;
                }

                if(segment_url->mediaRange.exists())
                {
                    range = (std::string)segment_url->mediaRange;
                }

                if(range.length() > 0)
                {
                    segment->mRangeFirst = dash_handle::handle_byte_range(range, segment->mRangeLast);
                }

                if(url.length() > 0)
                {
                    segment->mUrl = url;
                }

                if(segment->mStartTime + segment->mDuration > start_t + duration_t)
                {
                    segment->mDuration = start_t + duration_t - segment->mStartTime;

                    if(segment->mDuration == 0)
                    {
                        segment->mDuration = 1 * mTimeScale;  // it maybe less than 1s (double)
                    }
                }

               
                mMediaList.push_back(*((uint64_t*)&segment));
            }
            
        }
    }
    else
    {
       // only one media url as segment base
       {
            MPDSegment *segment = new MPDSegment();
            if(segment == NULL)
            {
                MPD_ERROR("no memory");
                return false;
            }
            
            segment->mStartTime = start_t;
            segment->mDuration = duration_t;

            mMediaList.push_back(*((uint64_t*)&segment));
        }
    }


    assert(mMediaList.size() != 0);
    return true;
}

bool MPDRepresentation::initSegmentFromTemplate(MPDSchema::CSegmentTemplateType_ex *template_ex,
MPDSchema::CSegmentTemplateType *template_ptr, double start, double duration, MPDSchema::CRepresentationType_ex *rep_ex)
{
    std::string url = std::string("");
    std::string range = std::string("");
    long long bandwidth = -1;
    std::string rep_id = std::string("");


    assert(template_ex != NULL);
    assert(template_ptr != NULL);
    
    if(rep_ex->id_exists)
    {
        rep_id = rep_ex->id_ex;
    }

    if(rep_ex->bandwidth_exists)
    {
        bandwidth = rep_ex->bandwidth_ex;
    }
    
    // time scale
    if(template_ex->timescale_exists)
    {
        mTimeScale = template_ex->timescale_ex;
    }

    uint64_t start_t = start * mTimeScale;
    uint64_t duration_t = duration * mTimeScale;
    
    // initial url two choice
    if(template_ex->initialisation_exists)
    {
        url = template_ex->initialisation_ex;
        if(url.length() > 0)
        {
            try
            {
                url = template_ex->handle_initialisation_url(url, rep_id, bandwidth);
            }
            catch (all_exceptions &e)
            {
                MPD_ERROR("initialization url handle error %s", e.get_message().c_str());
                return false;
            }


            if(url.length())
            {
                mInitList.mStartTime = start_t;
                mInitList.mDuration = duration_t;   
                mInitList.mUrl = url;
            }
            else
            {
                assert(0);
            }
            
        }
        else
        {
            assert(0);
        }
        
    }
    else if(template_ex->Initialisation_exists)
    {
        url = template_ex->Initialisation_ex;
        if(template_ex->Initialisation_byteRange_exists)
        {
            range = template_ex->Initialisation_byteRange_ex;
        }

        if(url.length() > 0 || range.length() > 0)
        {
            mInitList.mStartTime = start_t;
            mInitList.mDuration = duration_t;
            
            if(url.length() > 0)
            {
                mInitList.mUrl = url;
            }


            if(range.length() > 0)
            {
                mInitList.mRangeFirst = dash_handle::handle_byte_range(range, mInitList.mRangeLast);
            }
        }
        else
        {
            assert(0);
        }
    }


     // bitstream url 

     if(template_ex->bistreamswitch_exists)
     {
         url = template_ex->bistreamswitch_ex;

         if(url.length())
         {
             try
             {
                 url = template_ex->handle_bitstream_url(url,rep_id,bandwidth);
             }
             catch(all_exceptions &e)
             {
                 MPD_ERROR("handle bitstream url error %s", e.get_message().c_str());
                 return false;
             }

             if(url.length() > 0)
             {
                 MPDSegment *segment = new MPDSegment();
                 if(segment == NULL)
                 {
                     MPD_ERROR("no memory");
                     return false;
                 }
                 
                 segment->mStartTime = start_t;
                 segment->mDuration = duration_t;    
                 segment->mUrl = url;

                 mBitstreamList.push_back(*((uint64_t*)&segment));
              }
             else
             {
                 assert(0);
             }
         }
         else
         {
             assert(0);
         }
     }


     //index has only one since index segment url has no time and number attribute
     if(template_ex->index_exists)
     {
         url = template_ex->index_ex;


         if(url.length() > 0)
         {
             try
             {
                 url = template_ex->handle_index_url(url, rep_id, bandwidth);
             }
             catch(all_exceptions &e)
             {
                 MPD_ERROR("handle index url error %s", e.get_message().c_str());
                 return false;
             }

             if(url.length() > 0)
             {
                 MPDSegment *segment = new MPDSegment();
                 if(segment == NULL)
                 {
                     MPD_ERROR("no memory");
                     return false;
                 }
                 
                 segment->mStartTime = start_t;
                 segment->mDuration = duration_t;    
                 segment->mUrl = url;

                 mIndexList.push_back(*((uint64_t*)&segment));

             }
             else
             {
                 assert(0);
             }
         }
         else
         {
             assert(0);
         }
     }
     else if(template_ex->RepresentationIndex_exists)
     {
         url = template_ex->RepresentationIndex_ex;
         range = std::string("");
         
         if(template_ex->RepresentationIndex_byteRange_exists)
         {
             range = template_ex->RepresentationIndex_byteRange_ex;
         }

         if(url.length() > 0 || range.length() > 0)
         {
             MPDSegment *segment = new MPDSegment();
             if(segment == NULL)
             {
                 MPD_ERROR("no memory");
                return false;
             }
                 
             segment->mStartTime = start_t;
             segment->mDuration = duration_t;  

             if(url.length() > 0)
             {
                 segment->mUrl = url;
             }

             if(range.length() > 0)
             {
                 segment->mRangeFirst = dash_handle::handle_byte_range(range, segment->mRangeLast);
             }

             mIndexList.push_back(*((uint64_t*)&segment));
         }
     }


    // media url setup
    if(template_ex->media_exists)
    {
        std::string media_url = template_ex->media_ex;
        assert(media_url.length() > 0);
        uint32_t num = 1;

        if(template_ex->startNumber_exists)
        {
            num = template_ex->startNumber_ex;
        }

        if(template_ptr->SegmentTimeline.exists())
        {
            //assert(template_ex->timeline_exists);
            uint32_t index = 0;
            uint32_t end = template_ptr->SegmentTimeline[0].S.count();
            assert(end == template_ex->segmenttimeline_ex.s_list.size());

            MPD_DEBUG("timeline exists index %d end %d\n", index, end);

            for(;index < end; ++ index)
            {
                uint32_t repeat = template_ex->segmenttimeline_ex.s_list[index].start_time_for_r.size();
                uint32_t d = 0;
                
                if(!template_ptr->SegmentTimeline[0].S[index].r.exists())
                {
                    assert(repeat == 1);
                }
                else
                {
                   assert((unsigned)template_ptr->SegmentTimeline[0].S[index].r + 1 == repeat);
                }


                while(repeat > 0)
                {
                   
                    uint32_t media_time = template_ex->segmenttimeline_ex.s_list[index].start_time_for_r[d];
                    try
                    {
                        url = template_ex->handle_media_url(media_url,rep_id,num,bandwidth,media_time);
                    }
                    catch(all_exceptions &e)
                    {
                        MPD_ERROR("handle media error %s\n", e.get_message().c_str());
                        return false;
                    }

                   //DASH_CTRL_DEBUG(("media time = %d url = %s\n", media_time, url.c_str()));

                    if(url.length() > 0)
                    {
                        MPDSegment *segment = new MPDSegment();
                        if(segment == NULL)
                        {
                            MPD_ERROR("no memory");
                            return false;
                        }
 
                        segment->mStartTime = media_time + start_t;
                     // d() should exist
 
                     // to check end time is equal to this time
                        segment->mDuration = template_ex->segmenttimeline_ex.s_list[index].d_ex;
                        segment->mUrl = url;
 
                     // final url, but duration != d, so duration should be get.
                        if(segment->mStartTime + segment->mDuration >
                            start_t + duration_t)
                        {
                            segment->mDuration = start_t + duration_t - segment->mStartTime;
                            if(segment->mDuration == 0)
                            {
                                segment->mDuration = 1 * mTimeScale;  // it maybe less than 1s (double)
                            }
                      
                        }

                        mMediaList.push_back(*((uint64_t*)&segment));
                    }
                    else
                    {
                        assert(0);

                    }

                    -- repeat;
                    ++ d;
                    ++ num;
                }
            }
        }
        else if(template_ptr->duration.exists())
        {
            assert(template_ex->duration_exists);
            uint32_t dur = (unsigned int)template_ex->duration_ex;
            assert(dur > 0);
            uint32_t t = start * mTimeScale;
            uint32_t end = t + duration * mTimeScale;

            //DASH_CTRL_DEBUG(("......duration exists exists dur = %d  t %d end %d\n", dur, t, end));
            //DASH_CTRL_DEBUG(("start = %lf  duration = %lf\n", start, duration));
            while(t < end)
            {
                try
                {
                    url = template_ex->handle_media_url(media_url, rep_id, num, bandwidth, t);
                }
                catch (all_exceptions &e)
                {
                    MPD_ERROR("handle media url error %s\n", e.get_message().c_str());
                    return false;
                }

                //DASH_CTRL_DEBUG(("media url %s\n", url.c_str()));
                if(url.length() > 0)
                {
                    
                    MPDSegment *segment = new MPDSegment();
                    if(segment == NULL)
                    {
                        MPD_ERROR("no memory");
                        return false;
                    }
                    segment->mStartTime = t;
                    segment->mDuration = dur;
                    
                    t += dur;
                    ++ num;
                    segment->mUrl = url;

                    if(segment->mStartTime + segment->mDuration > end)
                    {
                        segment->mDuration = end - segment->mStartTime;
                        if(segment->mDuration == 0)
                        {
                            segment->mDuration = 1 * mTimeScale;  // it maybe less than 1s (double)
                        }
                    }

					mMediaList.push_back(*((uint64_t*)&segment));
					
                }
                else
                {
                    assert(0);
                }
            }
            
        }
    }


    assert(mMediaList.size() != 0);
    return true;
}

bool MPDRepresentation::initSegmentUrl(MPDSchema::CPeriodType * period, MPDSchema::CPeriodType_ex * period_ex, 
    MPDSchema::CAdaptationSetType * adaptation, MPDSchema::CAdaptationSetType_ex * adaptation_ex, 
    MPDSchema::CRepresentationType * representation, MPDSchema::CRepresentationType_ex *representation_ex)
{

    assert(period != NULL);
    assert(period_ex != NULL);
    assert(adaptation != NULL);
    assert(adaptation_ex != NULL);
    assert(representation != NULL);
    assert(representation_ex != NULL);
    
    dash_enum::dash_segment_info_type segment_info = dash_enum::none_segment;
	MPD_DEBUG("start = %f, duration = %f", period_ex->start_time, period_ex->duration_time);
    
    if(representation_ex->segment_pos == dash_enum::pos_representation)
    {
        segment_info = dash_handle::get_segment_info_type(representation->SegmentBase.exists(),
            representation->SegmentList.exists(), representation->SegmentTemplate.exists());

        assert(segment_info == representation_ex->segment_info);

        
        switch(segment_info)
        {
            case dash_enum::none_segment:
            {
                if(!initSegmentFromNone(period_ex->start_time, period_ex->duration_time))
                {
                    MPD_ERROR("mpd_parser error\n");
                    return false;
                }
            }

            break;

            case dash_enum::segment_base:
            {
                assert(representation->SegmentBase.exists());
                if(!initSegmentFromBase(&representation_ex->segment_base, period_ex->start_time,
                    period_ex->duration_time) )
                {
                    MPD_ERROR("mpd_parser error\n");
                    return false;
                }
            }
            break;

            case dash_enum::segment_list:
            {
                assert(representation->SegmentList.exists());
                if(!initSegmentFromList(&representation_ex->segment_list,&representation->SegmentList[0],
                    period_ex->start_time, period_ex->duration_time))
                {
                    MPD_ERROR("mpd_parser error\n");
                    return false;
                }
            }
            break;
            
            // template create
            default:
            {
                assert(representation->SegmentTemplate.exists());
                if(!initSegmentFromTemplate(&representation_ex->segment_template, &representation->SegmentTemplate[0],
                    period_ex->start_time, period_ex->duration_time, representation_ex))
                {
                    MPD_ERROR("mpd_parser error\n");

                    return false;
                }
            }
            break;
            
        }

        return true;
    }
    else if(representation_ex->segment_pos == dash_enum::pos_adaptation)
    {
        switch(adaptation_ex->segment_info)
        {
            case dash_enum::segment_base:
            {
                // in this case, there is no segment info in representation
                 assert(adaptation->SegmentBase.exists());
                 assert(segment_info == dash_enum::none_segment);

                 if(!initSegmentFromBase(&adaptation_ex->segment_base, period_ex->start_time,
                    period_ex->duration_time) )
                 {
                     MPD_ERROR("mpd_parser error\n");
                     return false;
                 }
            }
            break;

            case dash_enum::segment_list:
            {
                assert(adaptation->SegmentList.exists());
                assert(segment_info < dash_enum::segment_list);

                if(!initSegmentFromList(&adaptation_ex->segment_list,&adaptation->SegmentList[0],
                    period_ex->start_time, period_ex->duration_time))
                {
                    MPD_ERROR("mpd_parser error\n");
                    return false;
                }
            }
            break;

            case dash_enum::segment_template:
            {
                //assert(segment_info < dash_enum::segment_list);
                assert(adaptation->SegmentTemplate.exists());

                if(!initSegmentFromTemplate(&adaptation_ex->segment_template, &adaptation->SegmentTemplate[0],
                    period_ex->start_time, period_ex->duration_time, representation_ex))
                {
                    MPD_ERROR("mpd_parser error\n");

                    return false;
                }
            }

            break;

            default:
            {
                assert(0);
            }
        }

        return true;
    }
    else 
    {
        switch(period_ex->segment_info)
        {
            case dash_enum::segment_base:
            {
                assert(segment_info == dash_enum::none_segment);
                assert(adaptation_ex->segment_info == dash_enum::none_segment);

                assert(period->SegmentBase.exists());

                if(!initSegmentFromBase(&period_ex->segment_base, period_ex->start_time,
                    period_ex->duration_time) )
                {
                    MPD_ERROR("mpd_parser error\n");
                    return false;
                }
            }

            break;

            case dash_enum::segment_list:
            {
                assert(segment_info < dash_enum::segment_list);
                assert(adaptation_ex->segment_info < dash_enum::segment_list);
                assert(period->SegmentList.exists());

                if(!initSegmentFromList(&period_ex->segment_list,&period->SegmentList[0],
                    period_ex->start_time, period_ex->duration_time))
                {
                    MPD_ERROR("mpd_parser error\n");
                    return false;
                }
            }
            break;

            case dash_enum::segment_template:
            {
                assert(period->SegmentTemplate.exists());

                if(!initSegmentFromTemplate(&period_ex->segment_template, &period->SegmentTemplate[0],
                    period_ex->start_time, period_ex->duration_time, representation_ex))
                {
                    MPD_ERROR("mpd_parser error\n");

                    return false;
                }
            }

            break;

            default:
            {
                assert(0);
            }
        }

        return true;
    }

}


std::string MPDRepresentation::getBaseUrl()
{
    if(mBaseUrlList.size() > 0)
    {
        MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[0];
        assert(base != NULL);
        return base->getBaseUrl();
    }

    return std::string("");
}

bool MPDRepresentation::findIndexUrlByTime(RepUrlType *url, uint64_t startTime)
{
    if(url == NULL)
    {
        MPD_ERROR("invalid parameter");
        return false;
    }

    url->mStartRange = 0;
    url->mEndRange = 0;
    url->mUrl = std::string("");

    uint32_t count = mIndexList.size();
    if(count > 0)
    {
        uint32_t index = 0;
        for(;index < count ; ++ index)
        {
            // find it by time
            MPDSegment *ptr = (MPDSegment *)mIndexList[index];
            assert(ptr != NULL);
            
            if(ptr->mStartTime <= startTime
                && ptr->mStartTime + ptr->mDuration > startTime)
            {
                url->mUrl = ptr->mUrl;
                url->mStartRange = ptr->mRangeFirst;
                url->mEndRange = ptr->mRangeLast;

                break;
                    
            }
        }

        if(index == count && index > 0)
        {
            MPD_ERROR("time is error, can't find time ");
            assert(0);
        }
    }

    return true;
}

bool MPDRepresentation::findInitUrl(RepUrlType *url)
{
    if(url == NULL)
    {
        assert(0);
    }

    url->mStartRange = 0;
    url->mEndRange = 0;
    url->mUrl = std::string("");


    if(mInitList.mUrl.length() > 0)
    {
        url->mStartRange = mInitList.mRangeFirst;
        url->mEndRange = mInitList.mRangeLast;
        url->mUrl = mInitList.mUrl;
    }

    return true;
}


bool MPDRepresentation::findBitstreamUrlByTime(RepUrlType *url, uint64_t startTime)
{
    if(url == NULL)
    {
        assert(0);
    }

    url->mStartRange = 0;
    url->mEndRange = 0;
    url->mUrl = std::string("");

    uint32_t count = mBitstreamList.size();
    if(count > 0)
    {
        uint32_t index = 0;
        for(;index < count ; ++ index)
        {
            // find it by time

            MPDSegment *ptr = (MPDSegment *)mBitstreamList[index];
            assert(ptr != NULL);
            
            if(ptr->mStartTime <= startTime
                && ptr->mStartTime + ptr->mDuration > startTime)
            {
                url->mUrl = ptr->mUrl;
                url->mStartRange = ptr->mRangeFirst;
                url->mEndRange = ptr->mRangeLast;

                break;
                    
            }
        }

        if(index == count && index > 0)
        {
            MPD_ERROR("time is error, can't find time ");
            assert(0);
        }
    }

    return true;
}

bool MPDRepresentation::findMediaUrlByTime(RepUrlType *url, uint64_t startTime, uint64_t &time, uint64_t &duration)
{
    if(url == NULL)
    {
        assert(0);
    }

    uint32_t count = mMediaList.size();
    if(count > 0)
    {
        uint32_t index = 0;
        for(;index < count ; ++ index)
        {
            // find it by time

            MPDSegment *ptr = (MPDSegment *)mMediaList[index];
            assert(ptr != NULL);
            
            if(ptr->mStartTime <= startTime
                && ptr->mStartTime + ptr->mDuration > startTime)
            {
                url->mUrl = ptr->mUrl;
                url->mStartRange = ptr->mRangeFirst;
                url->mEndRange = ptr->mRangeLast;
                time = ptr->mStartTime/mTimeScale * 1000;
                duration = ptr->mDuration/mTimeScale * 1000;
                break;
                    
            }
        }

        if(index == count && index > 0)
        {
            MPD_ERROR("time is error, can't find time ");
            assert(0);
        }
    }


    return true;
}


#if MPD_PLAYLISTS_DEBUG
void MPDRepresentation::repDebug()
{
    MPD_DEBUG("\n#################\n");
    MPD_DEBUG("time scale = %d, width = %d", mTimeScale, mWidth);
    MPD_DEBUG("height = %d, maxplayrate = %f", mHeight, mMaxPlayrate);
    MPD_DEBUG("bitrate = %lld, codec = %s", mBitrate, mCodecs.c_str());


    if(mInitList.mUrl.length() > 0)
    {
        mInitList.segmentDebug();
    }


    uint32_t count = mDrmList.size();
    uint32_t index = 0;

    for(; index < count; ++ index)
    {
        MPDContentProtection *ptr = (MPDContentProtection *)mDrmList[index];
        assert(ptr != NULL);
        ptr->cpDebug();
    }

    count = mIndexList.size();
    index = 0;

    for(;index < count; ++ index)
    {
        MPDSegment *ptr = (MPDSegment *)mIndexList[index];
        assert(ptr != NULL);
        ptr->segmentDebug();
    }

    count = mBaseUrlList.size();
    for(index = 0; index < count; ++ index)
    {
        MPDBaseUrl *base = (MPDBaseUrl *)mBaseUrlList[index];
        assert(base != NULL);
        base->baseUrlDebug();
    }

    count = mBitstreamList.size();
    for(index = 0; index < count; ++ index)
    {
        MPDSegment *ptr = (MPDSegment *)mBitstreamList[index];
        assert(ptr != NULL);
        ptr->segmentDebug();
    }

    count = mMediaList.size();
    index = 0;

    for(;index < count; ++ index)
    {
        MPDSegment *ptr = (MPDSegment *)mMediaList[index];
        assert(ptr != NULL);
        ptr->segmentDebug();
    }


    count = mSubrepresentationList.size();
    for(index = 0; index < count; ++ index)
    {
        MPDSubrepresentation *ptr = (MPDSubrepresentation *)mSubrepresentationList[index];
        assert(ptr != NULL);
        ptr->subrepDebug();
    }
}
#endif
