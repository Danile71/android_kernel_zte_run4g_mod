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

#include <string.h>
#include <list>
#include <stdlib.h>

#include "MPDPlaylists.h"
#include "MPDPlaylist.h"
#include "MPDParser.h"
#include "MPDDebugLevel.h"
#include "mpdschema_prefix.h"
#include "MPDSchema.h"
#include "XercesString.h"
#include "dash_static.h"
#include "MPDPeriod.h"
#include "MPDAdaptationset.h"
#include "MPDRepresentation.h"
#include "MPDContentProtection.h"
#include "MPDBaseUrl.h"
#include "UriGlue.h"

using namespace android_dash;

// MPDPlaylist will be not released , only MPDPlaylists.h will be open
static std::vector<uint64_t> sPlaylists;
static pthread_mutex_t sPlaylistsLock = PTHREAD_MUTEX_INITIALIZER;
static MPDParser *sInst = NULL;

#define MPD_LOCK()                            \
    do                                        \
    {                                         \                                 
        MPD_DEBUG("lock");                    \
        pthread_mutex_lock(&sPlaylistsLock);  \
    }while(0);

#define MPD_UNLOCK()                          \
    do                                        \
    {                                         \
        MPD_DEBUG("unlock");                  \
        pthread_mutex_unlock(&sPlaylistsLock);\
    }while(0);


#define SANITY_CHECK(isLocked)                \
    if(sPlaylists.size() == 0)                \
    {                                         \
        MPD_ERROR("no playlist member,error");\
        if(isLocked)                          \
        {                                     \
            MPD_UNLOCK();                     \
        }                                     \
        if(mIsLiveMPD)                        \
        {                                     \
            MPD_ERROR("live MPD may be empty"); \
            return true;                      \
        }                                     \
        else                                  \
        {                                     \
            MPD_ERROR("static MPD error");    \
            return false;                     \
        }                                     \
    }                                      


#define FILE_PATH  "/system/lib/libmpdparser.so"

// only MPDParser inst memory should be released.
MPDPlaylists::MPDPlaylists(const char *MPDData, uint32_t size, const char *MPDUrl, CallbackPtr callback)
{

    assert(MPDData != NULL);
    assert(MPDUrl != NULL);
    assert(size > 0);
    
    mPlaybackEndTime = 0xFFFFFFFF;
    mCurrentPeriodStartTime = 0;
    mCurrentTime = 0;
    mPlaybackStartTime = 0;
    mContainer = MPD_UNKNOWN;
    mHasDrm = false;
    mDuration = UINT64_MAX;
    mMPDUrl = std::string("");
    mIsEmpty = false;
    mIsAvailableMPD = false;
    mCallback = callback;
    mDrmType = MPD_DRM_NONE;
    mIsLiveMPD = false;
    mStartTime = 0;

    // static varaible clear
    MPD_LOCK();
    freeMPDPlaylist();

    // parser instant create
    destroyMPDParserInst();
    char *buffer = (char *)malloc(size + 1);
    assert(buffer != NULL);
    strncpy(buffer, MPDData, size);
    buffer[size] = '\0';
    
    
    std::string data = std::string(buffer);
    std::string url = std::string(MPDUrl);
    
    createMPDPlaylists(data, url);
    
    if(mCallback == NULL)
    {
        mIsAvailableMPD = false;
        MPD_ERROR("callback should be set");
    }
    free(buffer);
    MPD_UNLOCK();
    MPD_DEBUG("Playlists Created");
}


// delete message, and exit thread
MPDPlaylists::~MPDPlaylists()
{
    mPlaybackEndTime = 0xFFFFFFFF;
    mPlaybackStartTime = 0;
    mContainer = MPD_UNKNOWN;
    mHasDrm = false;
    mDuration = UINT64_MAX;
    mMPDUrl = std::string("");
    mIsEmpty = false;
    mIsAvailableMPD = false;
    mIsLiveMPD = false;
    mDrmType = MPD_DRM_NONE;
    
    // static varaible clear
    MPD_LOCK();
    freeMPDPlaylist();
    destroyMPDParserInst();
  


    if(!sendMessageToThread(MPD_PLAYLISTS_EXIT))
    {
        MPD_ERROR("error, fatal error, can't exit thread");
    }


    MPD_UNLOCK();
    MPD_DEBUG("Playlists Deleted");
}



// send message to thread, and thread will notify to caller
bool MPDPlaylists::sendMessageToThread(MPDMessage message)
{
    if(mCallback != NULL)
    {
        mCallback(message);
        return true;
    }

    MPD_ERROR("callback is NULL");
    return false;
  
}



bool MPDPlaylists::initMPDPlayLists()
{
    // init members
    SANITY_CHECK(false);
    
    // get final element 
    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    mContainer = (MPDMediaContainer)playlist->getContainer();  
    mIsEmpty = (playlist->getDynamicType() == MPDPlaylist::DASH_TYPE_DYNAMIC_EMPTY);
    mIsLiveMPD = (playlist->getDynamicType() != MPDPlaylist::DASH_TYPE_STATIC);
    mDrmType = (MPDDRMType)playlist->getDrmType();
    mHasDrm = (mDrmType == MPD_DRM_NONE) ? false : true;
    mDuration = mIsLiveMPD ? 0 : (playlist->getDuration() * 1000 * 1000);
    mStartTime = playlist->getBeginTime() * 1000 * 1000;
    
    return true;
}


bool MPDPlaylists::updateMPDPlaylists()
{
    SANITY_CHECK(false);

    // check Live or Empty;
    uint32_t  it = sPlaylists.size(); 
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    
    bool isEmpty = (playlist->getDynamicType() == MPDPlaylist::DASH_TYPE_DYNAMIC_EMPTY);
    mIsLiveMPD = (playlist->getDynamicType() != MPDPlaylist::DASH_TYPE_STATIC);


    // if it is no empty mpd now,  send message to caller
    if(!isEmpty && mIsEmpty)
    {
        mIsEmpty = false;
        if(!sendMessageToThread(MPD_NEXT_UPDATED))
        {
            // error handle
            MPD_ERROR("send update message error");
            return false;
        }
    }
    
    return true;
}

bool MPDPlaylists::initMPDParserInst()
{
    if(NULL == sInst)
    {
        sInst = new MPDParser();

        if(sInst != NULL)
        {
            if(!sInst->init())
            {
                delete sInst;
                sInst = NULL;
                return false;
            }
        }
    }

    return true;
}


bool MPDPlaylists::destroyMPDParserInst()
{
    
    if(NULL != sInst)
    {
        if(!sInst->destory())
        {
            MPD_ERROR("delete mpd_parser error\n");
        }
        delete sInst;
        sInst = NULL;    

    }

    return true;
}


// add to parser it
bool MPDPlaylists::addDataToParserInst(std::string &MPDData)
{
    if(MPDData.length() == 0)
    {
        MPD_ERROR("no data\n");
        return false;
    }

    if(sInst != NULL)
    {
        mpd_parser_result ret = sInst->parser(MPDData, MPD_DATA_STRING);
        switch(ret)
        {
            case mpd_parser_empty:
            {
                mIsEmpty = true;
                mIsLiveMPD = true;
                return true; 
            }
            case mpd_parser_ok:
            {
                return true;
            }
            default:
            {
                return false;
            }
        }
    }

    return false;
}


// get live download information
bool MPDPlaylists::getNextMPDDlTime(uint64_t &time)
{
    MPD_LOCK();
    SANITY_CHECK(true);

    if(!mIsLiveMPD)
    {
        // not live;
        time = 0;
        MPD_UNLOCK();
        return false;
    }
   
   // get end one, and give the time
   uint32_t  it = sPlaylists.size();
   MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
   assert(playlist != NULL);
    
   time = playlist->getMiniUpdatePeriod() * 1000 * 1000;
   MPD_UNLOCK();
   return true;

}



bool MPDPlaylists::getNextLiveMPDDlUrl(std::string &url)
{
    MPD_LOCK();
    SANITY_CHECK(true);
    
    if(!mIsLiveMPD)
    {
        // not live;
        url = std::string("");
        MPD_UNLOCK();
        return false;
    }
    
    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    
    std::string localUrl = playlist->getUpdateLocation();
    if(localUrl.length() == 0)
    {
        url = mMPDUrl;
    }
    else
    {
        url = localUrl;
    }
    MPD_UNLOCK();

    return true;
}
    
// get live stream availbale time to playback
bool MPDPlaylists::getAvailableStartTime(uint64_t &time) // in us
{
    MPD_LOCK();
    SANITY_CHECK(true);

    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    
    time = playlist->getAvailableStartTime() * 1000000;

    MPD_UNLOCK();
    return true;
}

bool MPDPlaylists::getAvailableEndTime(uint64_t &time) // in us
{
    MPD_LOCK();
    SANITY_CHECK(true);

    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    
    time = playlist->getAvailableEndTime() * 1000000;

    MPD_UNLOCK();
    return true;
}

// get mini buffer time
bool MPDPlaylists::getMiniBufferTime(uint64_t &time)
{
    MPD_LOCK();
    SANITY_CHECK(true);

    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    
    time = playlist->getMiniBufferTime() * 1000000; // in Us

    MPD_UNLOCK();
    return true;
}
        
// get media information
bool MPDPlaylists::getMediaNumber(uint32_t &number, MPDMediaType type, MPDMediaNumberType nType)
{
    // this is difficult to decide, maybe should be related to current time
    if(type == MPD_VIDEO && nType != MPD_REP)
    {
        MPD_ERROR("video should use rep count");
        return false;
    }
    
    MPD_LOCK();
    SANITY_CHECK(true);

    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);

    if(!playlist->getMediaNumber(number, (MPDPlaylist::MediaType)type, (MPDPlaylist::MediaNumberType)nType))
    {
        MPD_ERROR("get media number error");
        MPD_UNLOCK();
        return false;
    }


    MPD_UNLOCK();
    return true;
}

bool MPDPlaylists::getMediaInformation(MPDMediaInfo &info, MPDMediaType type, MPDMediaNumberType nType)
{
    // it also should be decided by time
    if(type == MPD_VIDEO && nType != MPD_REP)
    {
        MPD_ERROR("video should use rep count");
        return false;
    }
    
    MPD_LOCK();
    SANITY_CHECK(true);

    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);

    info.mType = type;
    uint32_t count = 0;
    uint32_t index = 0;
    
    switch(type)
    {
        case MPD_VIDEO:
        {
            info.mVideoInfo.clear();
            if(!playlist->getMediaNumber(count, (MPDPlaylist::MediaType)type, 
                (MPDPlaylist::MediaNumberType)nType))
            {
                MPD_UNLOCK();
                MPD_ERROR("get video number error");
                return false;
            }

            if(count == 0)
            {
                MPD_UNLOCK();
                MPD_ERROR("error, video size is 0");
                return false;
            }

            MPD_DEBUG("video number count = %d", count);
            for(; index < count; ++index)
            {
                MPDPlaylist::VideoInfo media;
                media.mType = MPDPlaylist::DASH_MEDIA_VIDEO;
                if(!playlist->getMediaInfoByIndex(index, (MPDPlaylist::MediaType)type,
                    (MPDPlaylist::MediaNumberType)nType, &media))
                {
                    MPD_UNLOCK();
                    MPD_ERROR("get video infor error");

                    return false;
                }

                MPD_DEBUG("video bandwidth = %llu", media.mBandwidth);
                MPDVideoInfo mVideo;
                mVideo.mCodecs = media.mCodecs;
                mVideo.mBandwidth = media.mBandwidth;
                mVideo.mHeight = media.mHeight;
                mVideo.mWidth = media.mWidth;
                info.mVideoInfo.push_back(mVideo);
            }
            
        }
        break;

        case MPD_AUDIO:
        {
            info.mAudioInfo.clear();
            if(!playlist->getMediaNumber(count, (MPDPlaylist::MediaType)type, 
                (MPDPlaylist::MediaNumberType)nType))
            {
                MPD_UNLOCK();
                MPD_ERROR("get video number error");
                return false;
            }

            for(; index < count; ++index)
            {
                MPDPlaylist::AudioInfo media;
                media.mType = MPDPlaylist::DASH_MEDIA_AUDIO;
                if(!playlist->getMediaInfoByIndex(index, (MPDPlaylist::MediaType)type,
                    (MPDPlaylist::MediaNumberType)nType, &media))
                {
                    MPD_UNLOCK();
                    MPD_ERROR("get video infor error");

                    return false;
                }

                MPDAudioInfo mAudio;
                mAudio.mLanguage = media.mLanguage;
                mAudio.mCodecs = media.mCodecs;
                mAudio.mSampleRate = media.mSampleRate;
                info.mAudioInfo.push_back(mAudio);
            }
        }
        break;

        case MPD_SUBTITLE:
        {
            info.mSubtitleInfo.clear();
            if(!playlist->getMediaNumber(count, (MPDPlaylist::MediaType)type, 
                (MPDPlaylist::MediaNumberType)nType))
            {
                MPD_UNLOCK();
                MPD_ERROR("get video number error");
                return false;
            }

            for(; index < count; ++index)
            {
                MPDPlaylist::SubtitleInfo media;
                media.mType = MPDPlaylist::DASH_MEDIA_SUBTITLE;
                if(!playlist->getMediaInfoByIndex(index, (MPDPlaylist::MediaType)type,
                    (MPDPlaylist::MediaNumberType)nType, &media))
                {
                    MPD_UNLOCK();
                    MPD_ERROR("get video infor error");

                    return false;
                }

                MPDSubtitleInfo mSubtitle;
                mSubtitle.mLanguage = media.mLanguage;
                mSubtitle.mHeight = media.mHeight;
                mSubtitle.mWidth = media.mWidth;
                mSubtitle.mType = (MPDSubtitleType)media.mStype;
                info.mSubtitleInfo.push_back(mSubtitle);
            }
        }
        break;

        default:
            MPD_UNLOCK();
            MPD_ERROR("get media information error");
            return false;
    }

    MPD_UNLOCK();
    return true;
}

// get url by some information
bool MPDPlaylists::getUrl(MPDPlaylists::MPDURLInfo &urlInfo)
{

    urlInfo.mHasUrl = false;

    MPD_LOCK();
    SANITY_CHECK(true);
    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    
    MPDPeriod *period = NULL;
    MPDAdaptationset *adp = NULL;
    MPDRepresentation *rep = NULL;
    uint64_t startTimeMs = urlInfo.mStartTime / 1000;
    

    MPD_ERROR("time = %d, bandwidth = %lld, type = %d ", startTimeMs,
        urlInfo.mBandwidth, urlInfo.mType);
    
    if(!playlist->findPeriodByTime(startTimeMs, &period))
    {
        if(mIsLiveMPD)
        {
            MPD_UNLOCK();
            MPD_ERROR("no period found in live case, should wait");
            return true;
        }

        MPD_ERROR("no period found in static, impossible, start time should be ok");
        
        MPD_UNLOCK();
        return false;  
    }

    // free the playlist
    if(it > 1)
    {
        MPDPlaylist *temp = (MPDPlaylist *)sPlaylists[0];
        assert(temp != playlist);
        if(temp != NULL)
        {
            delete temp;
        }

        sPlaylists.erase(sPlaylists.begin());
    }

    assert(period != NULL);
    

    // record the url begin time, and according to update time to check period changed
    mCurrentPeriodStartTime = period->getStartTime() * 1000;
    uint64_t startTime = 0;
    std::string baseUrl;
    
    switch(urlInfo.mType)
    {
        case MPD_VIDEO:
        {
            if(!period->findRepForVideoByBitrate(&adp, &rep, urlInfo.mBandwidth))
            {
                MPD_ERROR("no adp for video found");
                assert(0);
            }

            assert(adp != NULL);
            assert(rep != NULL);

            startTime = rep->getTimeScale() * startTimeMs/1000; // from ms to timescale
            baseUrl = playlist->getBaseUrlByAdpAndRep(mMPDUrl, period, adp, rep);
            MPD_DEBUG("base url = %s", baseUrl.c_str());
            
            
        }
        break;

        case MPD_AUDIO:
        {
            if(!period->findRepForAudio(&adp, &rep))
            {
                MPD_ERROR("no adp for audio found");
                assert(0);
            }

            if(adp == NULL || rep == NULL)
            {
                MPD_ERROR("no audio in dash");
                MPD_UNLOCK();
                return true;
            }

            startTime = rep->getTimeScale() * startTimeMs/1000; // from ms to timescale
            baseUrl = playlist->getBaseUrlByAdpAndRep(mMPDUrl, period, adp, rep);
            MPD_DEBUG("base url = %s", baseUrl.c_str());
            
        }
        break;

        case MPD_SUBTITLE:
        {
            if(!period->findRepForSubtitle(&adp, &rep))
            {
                MPD_ERROR("no adp for subtitle found");
                assert(0);
            }

            if(adp == NULL || rep == NULL)
            {
                MPD_ERROR("no subtitle in dash");
                MPD_UNLOCK();
                return true;
            }

            startTime = rep->getTimeScale() * startTimeMs/1000; // from ms to timescale
            baseUrl = playlist->getBaseUrlByAdpAndRep(mMPDUrl, period, adp, rep);
            MPD_DEBUG("base url = %s", baseUrl.c_str());
        }
        break;

        default:
            assert(0);
        
    }


    ///TODO:: check available time ....
    ///TODO:: get drm information for url
            
    // find urls
    MPDRepresentation::RepUrlType tempUrl;
    if(!rep->findIndexUrlByTime(&tempUrl, startTime))
    {
        MPD_ERROR("impossible");
        assert(0);
    }

    if(tempUrl.mUrl.length() > 0)
    {
        urlInfo.mIndexUrl.mStartRange = tempUrl.mStartRange;
        urlInfo.mIndexUrl.mEndRange = tempUrl.mEndRange;
        urlInfo.mIndexUrl.mUrl = tempUrl.mUrl;

        // relative url
        if(tempUrl.mUrl.find("http") == std::string::npos)
        {
            MPD_DEBUG("index url = %s",tempUrl.mUrl.c_str());
            char buffer[1024] = {0};
            if(!URIParser_get_url(baseUrl.c_str(), tempUrl.mUrl.c_str(), buffer, 1024))
            {
                assert(0);
            }

            urlInfo.mIndexUrl.mUrl = std::string(buffer);
            MPD_DEBUG("final index url = %s",urlInfo.mIndexUrl.mUrl.c_str());
        }
    }

    if(!rep->findInitUrl(&tempUrl))
    {
        MPD_ERROR("impossible");
        assert(0);
    }

    if(tempUrl.mUrl.length() > 0)
    {
        urlInfo.mInitUrl.mStartRange = tempUrl.mStartRange;
        urlInfo.mInitUrl.mEndRange = tempUrl.mEndRange;
        urlInfo.mInitUrl.mUrl = tempUrl.mUrl;

        if(tempUrl.mUrl.find("http") == std::string::npos)
        {
            MPD_DEBUG("init url = %s",tempUrl.mUrl.c_str());
            char buffer[1024] = {0};
            if(!URIParser_get_url(baseUrl.c_str(), tempUrl.mUrl.c_str(), buffer, 1024))
            {
                assert(0);
            }

            urlInfo.mInitUrl.mUrl = std::string(buffer);
            MPD_DEBUG("final init url = %s",urlInfo.mInitUrl.mUrl.c_str());
        }
    }

    if(!rep->findBitstreamUrlByTime(&tempUrl, startTime))
    {
        MPD_ERROR("impossible");
        assert(0);
    }

    if(tempUrl.mUrl.length() > 0)
    {
        urlInfo.mBitstreamUrl.mStartRange = tempUrl.mStartRange;
        urlInfo.mBitstreamUrl.mEndRange = tempUrl.mEndRange;
        urlInfo.mBitstreamUrl.mUrl = tempUrl.mUrl;

        if(tempUrl.mUrl.find("http") == std::string::npos)
        {
            MPD_DEBUG("bitstream url = %s",tempUrl.mUrl.c_str());
            char buffer[1024] = {0};
            if(!URIParser_get_url(baseUrl.c_str(), tempUrl.mUrl.c_str(), buffer, 1024))
            {
                assert(0);
            }

            urlInfo.mBitstreamUrl.mUrl = std::string(buffer);
            MPD_DEBUG("final bitstream url = %s",urlInfo.mBitstreamUrl.mUrl.c_str());
        }
    }

    
    uint64_t durationMs = 0;
    
    if(!rep->findMediaUrlByTime(&tempUrl, startTime, startTimeMs,
        durationMs))
    {
        MPD_ERROR("impossible");
        assert(0);
    }

    urlInfo.mStartTime = startTimeMs * 1000;
    urlInfo.mDuration = durationMs * 1000;
        
    urlInfo.mBandwidth = rep->getBitrate();
    
    if(tempUrl.mUrl.length() > 0)
    {
        urlInfo.mMediaUrl.mStartRange = tempUrl.mStartRange;
        urlInfo.mMediaUrl.mEndRange = tempUrl.mEndRange;
        urlInfo.mMediaUrl.mUrl = tempUrl.mUrl;

        if(tempUrl.mUrl.find("http") == std::string::npos)
        {
            MPD_DEBUG("media url = %s",tempUrl.mUrl.c_str());
            char buffer[1024] = {0};
            if(!URIParser_get_url(baseUrl.c_str(), tempUrl.mUrl.c_str(), buffer, 1024))
            {
                assert(0);
            }

            urlInfo.mMediaUrl.mUrl = std::string(buffer);
            MPD_DEBUG("final media url = %s",urlInfo.mMediaUrl.mUrl.c_str());
        }
    }
    else
    {
        urlInfo.mMediaUrl.mUrl = baseUrl;
    }

    urlInfo.mHasUrl = true;
    MPD_UNLOCK();
    return true;
}
        
// set some information to MPD
bool MPDPlaylists::setCurrentIndex(uint32_t index, MPDMediaType type, MPDMediaNumberType nType)
{
    if((type == MPD_VIDEO && nType != MPD_REP)
        || (type != MPD_VIDEO && nType != MPD_ADP))
    {
        MPD_ERROR("some media support special index setting");
        return false;
    }
    
    MPD_LOCK();
    SANITY_CHECK(true);
    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);

    if(!playlist->setCurrentPlayIndex(index, (MPDPlaylist::MediaType)type, (MPDPlaylist::MediaNumberType)nType))
    {
        MPD_ERROR("set index error");
        MPD_UNLOCK();
        return false;
    }

    MPD_UNLOCK();
    return true;
}


bool MPDPlaylists::setCurrentBandwidth(uint64_t bandwidth, MPDMediaType type)
{
    if(type != MPD_VIDEO)
    {
        MPD_ERROR("only video supported bandwidth setting");
        return false;
    }

    MPD_LOCK();
    SANITY_CHECK(true);
    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);

    if(!playlist->setBandwidth(bandwidth))
    {
        MPD_ERROR("set bandwidth error");
        MPD_UNLOCK();
        return false;
    }

    MPD_UNLOCK();
    return true;
}



// should use this start time if user set it
// but app may use this few times to set start and end time
bool MPDPlaylists::setStartTime(uint64_t time)
{
    MPD_LOCK();
    SANITY_CHECK(true);
    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    uint32_t timeInMs = time / 1000;

    
    if(playlist->getBeginTime() * 1000 < timeInMs)
    {
        mPlaybackStartTime = timeInMs/1000;
    }
    else
    {
        mPlaybackStartTime = playlist->getBeginTime();
    }
    MPD_UNLOCK();
    return true;
}


bool MPDPlaylists::setEndTime(uint64_t time)
{
    MPD_LOCK();
    SANITY_CHECK(true);
    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    uint32_t timeInMs = time / 1000;
    
    if((playlist->getBeginTime() + playlist->getDuration()) * 1000 > timeInMs)
    {
        mPlaybackEndTime = timeInMs/1000;
    }
    else
    {
        mPlaybackEndTime = playlist->getBeginTime() + playlist->getDuration();
    }
    MPD_UNLOCK();
    return true;
}


bool MPDPlaylists::setCurrentLanguage(std::string &language, MPDMediaType type)
{
    if(language.length() == 0)
    {
        MPD_ERROR("language is null");
        return false;
    }

    MPD_LOCK();
    SANITY_CHECK(true);

    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);

    
    if(!playlist->setCurrentLanguage(language, (MPDPlaylist::MediaType)type))
    {
        MPD_ERROR("set language error");
        MPD_UNLOCK();
        return false;
    }


    MPD_LOCK();
    return true;
}


// add live mpd data, and upate data
bool MPDPlaylists::addLiveMPDToPlaylists(std::string &MPDData, std::string &MPDUrl)
{
    if(MPDData.length() == 0 || MPDUrl.length() == 0)
    {
        MPD_ERROR("invalid parameter");
        mIsAvailableMPD = false;
        return false;
    }

    MPD_LOCK();
    mMPDUrl = MPDUrl;
    if(!initMPDParserInst())
    {
        destroyMPDParserInst();
        MPD_UNLOCK();
        MPD_ERROR("MPD Parser init Error");
        return false;
    }

    if(!addDataToParserInst(MPDData))
    {
        MPD_UNLOCK();
        MPD_ERROR("MPD Parse Data Error");
        return false;
    }
    
    MPDPlaylist *playlist = new MPDPlaylist();
    if(playlist == NULL)
    {
        MPD_ERROR("no memory");
        return false;
    }

    if(playlist->parserMPD(sInst))
    {
        sPlaylists.push_back(*((uint64_t *)&playlist));
    }
    else
    {
        mIsAvailableMPD = false;
        delete playlist;
        MPD_UNLOCK();
        MPD_ERROR("MPD Parser Error");
        return false;
    }

    if(!updateMPDPlaylists())
    {
        mIsAvailableMPD = false;
        MPD_UNLOCK();
        MPD_ERROR("Playlists update error");
        return false;
    }

    mIsAvailableMPD = true;
    MPD_UNLOCK();
    return true;
}


// static API to add MPD data into Playlists
bool MPDPlaylists::createMPDPlaylists(std::string &MPDData, std::string &MPDUrl)
{
    if(MPDData.length() == 0 || MPDUrl.length() == 0)
    {
        MPD_ERROR("invalid parameter");
        return false;
    }

    // firstly, record the url for live url decide
    mMPDUrl = MPDUrl;

    // create MPDParser Inst, and init xerces library
    if(!initMPDParserInst())
    {
        destroyMPDParserInst();
        MPD_ERROR("MPD Parser init Error");
        return false;
    }

    // then add data into MPDParser, get result.
    if(!addDataToParserInst(MPDData))
    {
        MPD_ERROR("MPD Parse Data Error");
        return false;
    }

    MPDPlaylist *playlist = new MPDPlaylist();
    if(playlist == NULL)
    {
        MPD_ERROR("no memory");
        return false;
    }
    
    // parser Inst Data into Playlist
    if(playlist->parserMPD(sInst))
    {
        sPlaylists.push_back(*((uint64_t*)&playlist));
    }
    else
    {
        MPD_ERROR("playlist Parser Error");
        delete playlist;
        return false;
    }

    if(!initMPDPlayLists())
    {
        MPD_ERROR("Playlists Init error");
        return false;
    }


    mIsAvailableMPD = true;

    if(!isLiveMPD())
    {
        destroyMPDParserInst();
        MPD_DEBUG("static MPD, delete Parser Inst");
    }
    
    return true;
}



bool MPDPlaylists::updateCurrentTime(uint64_t time)
{
    MPD_LOCK();
    SANITY_CHECK(true);

    // check the period time, and notify the period_changed event


    return true;
}

void MPDPlaylists::freeMPDPlaylist()
{
    if(sPlaylists.size() > 0)
    {
        uint32_t j = 0;
        for(;j < sPlaylists.size(); ++ j)
        {
            uint64_t ptr = sPlaylists[j];
            if(ptr != 0)
            {
                delete ((MPDPlaylist *)ptr);
            }
        }
    }

    sPlaylists.clear();
}

#if MPD_PLAYLISTS_DEBUG
void MPDPlaylists::playlistsDebug()
{
    MPD_DEBUG("debug playlists");

    MPD_DEBUG("PB start time = %lu end time = %lu not care", mPlaybackStartTime, mPlaybackEndTime);
    MPD_DEBUG("duration = %llu, current time = %llu", mDuration, mCurrentTime);
    MPD_DEBUG("Current period time = %llu", mCurrentPeriodStartTime);
    MPD_DEBUG("container = %d drm type = %d", mContainer, mDrmType);
    MPD_DEBUG("has drm = %d, isempty = %d", mHasDrm, mIsEmpty);
    MPD_DEBUG("available = %d, is live = %d", mIsAvailableMPD, mIsLiveMPD);


    uint32_t  it = sPlaylists.size();
    MPDPlaylist *playlist = (MPDPlaylist *)sPlaylists[it - 1];;
    assert(playlist != NULL);
    
    MPD_DEBUG("point = %llu \n", playlist);
    playlist->playlistDebug();
}
#endif

