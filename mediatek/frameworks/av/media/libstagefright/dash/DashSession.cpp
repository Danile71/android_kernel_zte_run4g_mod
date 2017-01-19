/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "DASHSession"
#include <utils/Log.h>

#include "DashSession.h"

#include "DashFileFetcher.h"

#include "include/HTTPBase.h"
#include "AnotherPacketSource.h"

#include <cutils/properties.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

namespace android {

DASHSession::DASHSession(
        const sp<AMessage> &notify, uint32_t flags, bool uidValid, uid_t uid)
    : mNotify(notify),
      mFlags(flags),
      mUIDValid(uidValid),
      mUID(uid),
      mInPreparationPhase(true),
      mHTTPDataSource(
              HTTPBase::Create(
                  (mFlags & kFlagIncognito)
                    ? HTTPBase::kFlagIncognito
                    : 0)),
      mStreamMask(0),
      mLastDequeuedTimeUs(0ll),
      mLastGetMPDTimeUs(-1ll),
      mNextMPDDlTimeUs(-1ll),
      mRefreshMPDGeneration(0),
      mRealTimeBaseUs(0ll), 
      mStreamSwitchGeneration(0),
      mReconfigurationInProgress(false),
      mPrevBandwidthIndex(-1),
      mDisconnectReplyID(0) {     	
    if (mUIDValid) {
        mHTTPDataSource->setUID(mUID);
    }
   
    mSeeking = false;
    mseekTimeUsAtCfg = -1;
    mAudioStreamState = STREAMSTATE_NONE;
    mVideoStreamState = STREAMSTATE_NONE;
    mIsLive = false;
    mConnected = false;
    mDuration = -1ll;
    mMPDMediaContainer = MPDPlaylists::MPD_UNKNOWN;
    mMPDDrmType = MPDPlaylists::MPD_DRM_NONE;
    
    mPacketSources.add(
            STREAMTYPE_AUDIO, new AnotherPacketSource(NULL /* meta */));

    mPacketSources.add(
            STREAMTYPE_VIDEO, new AnotherPacketSource(NULL /* meta */));

    mPacketSources.add(
            STREAMTYPE_SUBTITLES, new AnotherPacketSource(NULL /* meta */));
}

DASHSession::~DASHSession() {
    if(mMPDPlaylist)
        delete mMPDPlaylist;
}

status_t DASHSession::dequeueAccessUnit(
        StreamType stream, sp<ABuffer> *accessUnit) {
    if (!(mStreamMask & stream)) {
        return UNKNOWN_ERROR;
    }

    if(mSeeking || (stream == STREAMTYPE_AUDIO && mAudioStreamState == STREAMSTATE_DEQUEUE) || (stream == STREAMTYPE_VIDEO && mVideoStreamState == STREAMSTATE_DEQUEUE))
    {
        return -EAGAIN;
    }

    sp<AnotherPacketSource> packetSource = mPacketSources.valueFor(stream);

    status_t finalResult;
    if (!packetSource->hasBufferAvailable(&finalResult)) {
       return finalResult == OK ? -EAGAIN : finalResult;
    }
    
    status_t err = packetSource->dequeueAccessUnit(accessUnit);

    const char *streamStr;
    switch (stream) {
        case STREAMTYPE_AUDIO:
            streamStr = "audio";
            break;
        case STREAMTYPE_VIDEO:
            streamStr = "video";
            break;
        case STREAMTYPE_SUBTITLES:
            streamStr = "subs";
            break;
        default:
            TRESPASS();
    }

    if (err == INFO_DISCONTINUITY) {
        int32_t type;
        CHECK((*accessUnit)->meta()->findInt32("discontinuity", &type));

        sp<AMessage> extra;
        if (!(*accessUnit)->meta()->findMessage("extra", &extra)) {
            extra.clear();
        }

        ALOGI("[%s] read discontinuity of type %d, extra = %s",
              streamStr,
              type,
              extra == NULL ? "NULL" : extra->debugString().c_str());
        if(type == ATSParser::DISCONTINUITY_FORMATCHANGE)
        {
            if(stream == STREAMTYPE_VIDEO)
                mVideoStreamState = STREAMSTATE_DEQUEUE;
            else if(stream == STREAMTYPE_AUDIO)
                mAudioStreamState = STREAMSTATE_DEQUEUE;
        }
    } else if (err == OK) {
        if (stream == STREAMTYPE_AUDIO || stream == STREAMTYPE_VIDEO) {
            int64_t timeUs;
            CHECK((*accessUnit)->meta()->findInt64("timeUs",  &timeUs));
            ALOGD("[%s] read buffer at time %lld us", streamStr, timeUs);

            mLastDequeuedTimeUs = timeUs;
            mRealTimeBaseUs = ALooper::GetNowUs() - timeUs;
        } else if (stream == STREAMTYPE_SUBTITLES) {
            (*accessUnit)->meta()->setInt32("trackIndex", 0);//0 for temp
            (*accessUnit)->meta()->setInt64("baseUs", mRealTimeBaseUs);
        }
    } else {
        ALOGI("[%s] encountered error %d", streamStr, err);
    }

    return err;
}

status_t DASHSession::getStreamFormat(StreamType stream, sp<AMessage> *format) {
    if (!(mStreamMask & stream)) {
        return UNKNOWN_ERROR;
    }

    if(stream == STREAMTYPE_VIDEO && mVideoStreamState == STREAMSTATE_DEQUEUE)
        mVideoStreamState = STREAMSTATE_NONE; 
    if(stream == STREAMTYPE_AUDIO && mAudioStreamState == STREAMSTATE_DEQUEUE)
        mAudioStreamState = STREAMSTATE_NONE; 
       
    sp<AnotherPacketSource> packetSource = mPacketSources.valueFor(stream);

    sp<MetaData> meta = packetSource->getFormat();

    if (meta == NULL) {
        return -EAGAIN;
    }

    return convertMetaDataToMessage(meta, format);
}

void DASHSession::connectAsync(
        const char *url, const KeyedVector<String8, String8> *headers) {
    sp<AMessage> msg = new AMessage(kWhatConnect, id());
    msg->setString("url", url);

    if (headers != NULL) {
        msg->setPointer(
                "headers",
                new KeyedVector<String8, String8>(*headers));
    }

    msg->post();
}

status_t DASHSession::disconnect() {
    cancelStreamSwitching();
    sp<AMessage> msg = new AMessage(kWhatDisconnect, id());
    mHTTPDataSource->disconnect();

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

status_t DASHSession::seekTo(int64_t timeUs) {
    if(!mIsLive && mDuration >= 0 && timeUs >= mDuration)
    {
        ALOGD("timeUs:%lld >= duration:%lld", timeUs, mDuration);
        disconnect();
        return OK;
    }
    cancelStreamSwitching();
    mHTTPDataSource->disconnect();
    
    sp<AMessage> msg = new AMessage(kWhatSeek, id());
    msg->setInt64("timeUs", timeUs);
    mSeeking = true;

    ALOGD("seekTo:%lld",timeUs);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

void DASHSession::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatConnect:
        {
            onConnect(msg);
            break;
        }

        case kWhatDisconnect:
        {
            CHECK(msg->senderAwaitsResponse(&mDisconnectReplyID));

            if (mReconfigurationInProgress) {
                break;
            }

            finishDisconnect();
            break;
        }

        case kWhatSeek:
        {
            uint32_t replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            status_t err = onSeek(msg);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);

            response->postReply(replyID);
            break;
        }

        case kWhatFetcherNotify:
        {
            int32_t what;
            CHECK(msg->findInt32("what", &what));

            switch (what) {
                case DashFileFetcher::kWhatStarted:
                    break;

                case DashFileFetcher::kWhatError:
                {
                    status_t err;
                    CHECK(msg->findInt32("err", &err));

                    AString uri;
                    CHECK(msg->findString("dashMediaUri", &uri));
                    
                    if(mFetcherInfos.indexOfKey(uri) < 0)
                    {
                        ALOGD("skip stale kWhatError for %s,err:%d",uri.c_str(),err);
                        break;
                    }

                    ALOGD("fetch done for %s,error:%d",uri.c_str(),err);
                    FetcherInfo *info = &mFetcherInfos.editValueFor(uri);
                    
                    if(err == ERROR_END_OF_STREAM)//fetch done
                    {
                        ALOGD("mUrlInfo.mStartTime:%lld,mUrlInfo.mDuration:%lld,mDuration:%lld",info->mUrlInfo.mStartTime,info->mUrlInfo.mDuration,mDuration);
                        if((mDuration <=0 ) || (mDuration >0 && info->mUrlInfo.mStartTime + info->mUrlInfo.mDuration < mDuration))
                        {
                            uint32_t bandwidthIndex = getBandwidthIndex();
       
                            sp<AMessage> msg = new AMessage(kWhatStreamSwitching, id());
                            msg->setInt32("streamMask", info->mFetcherStreamMask);
                            msg->setInt32("segstarttimeus", info->mUrlInfo.mStartTime);
                            msg->setInt32("segmediatimeus", info->mUrlInfo.mDuration);
                            msg->setInt32("newFetcherBwIndex", bandwidthIndex);
                            msg->setString("staleFetcherUri", uri.c_str());

                            msg->setInt32("streamswitchgeneration", mStreamSwitchGeneration);
                            msg->post();
                            break;
                        }
                    }
                        
                    if (mInPreparationPhase && (info->mFetcherStreamMask != STREAMTYPE_SUBTITLES)) {
                        ALOGD("postPrepared err due to fetcher encount err:%d",err);
                        postPrepared(err);
                    }
                   
                    if(info->mFetcherStreamMask == STREAMTYPE_AUDIO)
                        mPacketSources.valueFor(STREAMTYPE_AUDIO)->signalEOS(err);
                    else if(info->mFetcherStreamMask == STREAMTYPE_VIDEO)
                        mPacketSources.valueFor(STREAMTYPE_VIDEO)->signalEOS(err);
                    else if(info->mFetcherStreamMask == STREAMTYPE_SUBTITLES)
                        mPacketSources.valueFor(STREAMTYPE_SUBTITLES)->signalEOS(err);
                        
                    if(info->mFetcherStreamMask != STREAMTYPE_SUBTITLES && err != ERROR_END_OF_STREAM)
                    {
                        ALOGD("post err due to fetcher encount err:%d",err);
                        finishDisconnect_l();
                        postError(err);
                    }
                    break;
                }

                case DashFileFetcher::kWhatTemporarilyDoneFetching:
                {
                    AString uri;
                    CHECK(msg->findString("dashMediaUri", &uri));

                    if(mFetcherInfos.indexOfKey(uri) < 0)
                    {
                        ALOGD("skip stale kWhatTemporarilyDoneFetching for %s",uri.c_str());
                        break;
                    }
                    
                    FetcherInfo *info = &mFetcherInfos.editValueFor(uri);
                    info->mIsPrepared = true;

                    if (mInPreparationPhase) {
                        bool allFetchersPrepared = true;
                        for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
                            if (!mFetcherInfos.valueAt(i).mIsPrepared) {
                                allFetchersPrepared = false;
                                break;
                            }
                        }

                        if (allFetchersPrepared) {
                            postPrepared(OK);
                        }
                    }
                    break;
                }

                default:
                    TRESPASS();
            }

            break;
        }

        case kWhatChangeConfiguration:
        {
            onChangeConfiguration(msg);
            break;
        }

        case kWhatChangeConfiguration2:
        {
            onChangeConfiguration2(msg);
            break;
        }

        case kWhatChangeConfiguration3:
        {
            onChangeConfiguration3(msg);
            break;
        }

        case kWhatFinishDisconnect2:
        {
            onFinishDisconnect2();
            break;
        }

        case kWhatStreamSwitching:
        {
            int32_t streamswitchgeneration;
            CHECK(msg->findInt32("streamswitchgeneration",&streamswitchgeneration));

            if(streamswitchgeneration != mStreamSwitchGeneration)
                break;//stale event
            if(mReconfigurationInProgress)
                break;
            else
            {
                int32_t streamMask;
                CHECK(msg->findInt32("streamMask", &streamMask));
                    
                if(((StreamType)streamMask == STREAMTYPE_AUDIO && mAudioStreamState != STREAMSTATE_NONE) || ((StreamType)streamMask == STREAMTYPE_VIDEO && mVideoStreamState != STREAMSTATE_NONE))
                {
                    ALOGD("repost kWhatStreamSwitching due to stream switch pending");
                    msg->post(1000000ll); // retry in 1 sec
                    break;
                }

                AString uri;
                CHECK(msg->findString("staleFetcherUri", &uri));
                
                int32_t segMediatimeUs,segStarttimeUs,bandwidthIndex;
                CHECK(msg->findInt32("segmediatimeus", &segMediatimeUs));
                CHECK(msg->findInt32("segstarttimeus", &segStarttimeUs));
                CHECK(msg->findInt32("newFetcherBwIndex", &bandwidthIndex));
                
                changeConfiguration_l(uri,segStarttimeUs,segMediatimeUs,bandwidthIndex,streamMask);
                break;
            }
        }
        
        case kWhatRefreshMPD:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mRefreshMPDGeneration) {
                // Stale event
                break;
            }

            onRefreshMPD();
            break;
        }

        default:
            TRESPASS();
            break;
    }
}

void DASHSession::onConnect(const sp<AMessage> &msg) {
    AString url;
    CHECK(msg->findString("url", &url));

    KeyedVector<String8, String8> *headers = NULL;
    if (!msg->findPointer("headers", (void **)&headers)) {
        mExtraHeaders.clear();
    } else {
        mExtraHeaders = *headers;
        delete headers;
        headers = NULL;
    }

    ALOGI("onConnect %s", url.c_str());

    mMPDURL = url;

    mMPDPlaylist = fetchPlaylist(url.c_str());

    if (mMPDPlaylist == NULL) {
        ALOGE("unable to fetch mpd playlist '%s'.", url.c_str());
        postPrepared(ERROR_CANNOT_CONNECT);
        return;
    }

    mIsLive = mMPDPlaylist->isLiveMPD();

    if(mIsLive && mMPDPlaylist->isEmptyMPD())
    {
        postRefreshMPD();
        return;
    }

    connect_l();
}

void DASHSession::connect_l(){
    mMPDMediaContainer = mMPDPlaylist->getMPDContainer();
    mMPDDrmType = mMPDPlaylist->getDrmType();

    mDuration = mMPDPlaylist->getDuration();

    MPDPlaylists::MPDMediaInfo mediaInfo;
    mediaInfo.mType = MPDPlaylists::MPD_VIDEO;
    MPDPlaylists::MPDMediaType mediaType = MPDPlaylists::MPD_VIDEO;
    MPDPlaylists::MPDMediaNumberType mediaNumberType = MPDPlaylists::MPD_REP;

    if(!mMPDPlaylist->getMediaInformation(mediaInfo,mediaType,mediaNumberType))
    {
        ALOGE("Can not get media information");
        postPrepared(UNKNOWN_ERROR);
        return;
    }

    mConnected = true;
    
    ALOGD("bandwidth info");
    //Update mBandwidthItems
    for (int i = 0; i < mediaInfo.mVideoInfo.size(); i ++)
    {
        BandwidthItem bwItem;
        bwItem.mIndex = i;
        bwItem.mBandwidth = mediaInfo.mVideoInfo[i].mBandwidth;
        ALOGD("entry[%d] bandwidth:%d",i,mediaInfo.mVideoInfo[i].mBandwidth);
        mBandwidthItems.add(bwItem);
    }

    changeConfiguration(0ll /* timeUs */, mBandwidthItems.itemAt(0).mIndex);
}

void DASHSession::finishDisconnect_l() {
    cancelRefreshMPD();
    for (size_t i = 0; i < mFetcherInfos.size();) {
        sp<DashFileFetcher> fetcher = mFetcherInfos.valueAt(i).mFetcher;
        mFetcherInfos.removeItemsAt(i);
        
        fetcher->stop();
        fetcher.clear();
    }

    //discard buffer and clear format | queue EOS
    mPacketSources.valueFor(STREAMTYPE_AUDIO)->clear();
    mPacketSources.valueFor(STREAMTYPE_AUDIO)->signalEOS(ERROR_END_OF_STREAM);
    mPacketSources.valueFor(STREAMTYPE_VIDEO)->clear();
    mPacketSources.valueFor(STREAMTYPE_VIDEO)->signalEOS(ERROR_END_OF_STREAM);
    mPacketSources.valueFor(STREAMTYPE_SUBTITLES)->clear();
    mPacketSources.valueFor(STREAMTYPE_SUBTITLES)->signalEOS(ERROR_END_OF_STREAM);
}

void DASHSession::finishDisconnect() {
    cancelRefreshMPD();
    for (size_t i = 0; i < mFetcherInfos.size();) {
        sp<DashFileFetcher> fetcher = mFetcherInfos.valueAt(i).mFetcher;
        mFetcherInfos.removeItemsAt(i);
        
        fetcher->stop();
        fetcher.clear();
    }

    sp<AMessage> msg = new AMessage(kWhatFinishDisconnect2, id());

    msg->post();
    
}

void DASHSession::onFinishDisconnect2() {
    //discard buffer and clear format | queue EOS
    mPacketSources.valueFor(STREAMTYPE_AUDIO)->clear();
    mPacketSources.valueFor(STREAMTYPE_AUDIO)->signalEOS(ERROR_END_OF_STREAM);
    mPacketSources.valueFor(STREAMTYPE_VIDEO)->clear();
    mPacketSources.valueFor(STREAMTYPE_VIDEO)->signalEOS(ERROR_END_OF_STREAM);
    mPacketSources.valueFor(STREAMTYPE_SUBTITLES)->clear();
    mPacketSources.valueFor(STREAMTYPE_SUBTITLES)->signalEOS(ERROR_END_OF_STREAM);

    sp<AMessage> response = new AMessage;
    response->setInt32("err", OK);

    response->postReply(mDisconnectReplyID);
    mDisconnectReplyID = 0;
}

bool DASHSession::timeToRefreshMPD(int64_t nowUs) const {
    return mLastGetMPDTimeUs < 0ll || mLastGetMPDTimeUs + mNextMPDDlTimeUs <= nowUs;
}

void DASHSession::postRefreshMPD(int64_t delayUs) {
    if(mIsLive)
    {
        sp<AMessage> msg = new AMessage(kWhatRefreshMPD, id());
        msg->setInt32("generation", mRefreshMPDGeneration);
        msg->post(delayUs);
    }
}

void DASHSession::cancelRefreshMPD() {
    if(mIsLive)
    {
        ++mRefreshMPDGeneration;
    }
}

void DASHSession::onRefreshMPD() {
    int64_t nowUs = ALooper::GetNowUs(); 
    if (timeToRefreshMPD(nowUs)) {
        std::string nextMPDUrl;
        uint64_t nextMPDDltime;
        mMPDPlaylist->getNextLiveMPDDlUrl(nextMPDUrl);
    
        sp<ABuffer> buffer;
        status_t err = fetchFile(nextMPDUrl.c_str(), &buffer);
        std::string mpdData((const char *)buffer->data());

        mMPDPlaylist->addLiveMPDToPlaylists(mpdData,nextMPDUrl);
        
        mMPDPlaylist->getNextMPDDlTime(nextMPDDltime);
 
        mNextMPDDlTimeUs = nextMPDDltime;
        mLastGetMPDTimeUs = ALooper::GetNowUs();
    }

    if(!mConnected && !mMPDPlaylist->isEmptyMPD())
        connect_l();
    
    postRefreshMPD(1000000ll);//refresh after 1s
}

sp<DashFileFetcher> DASHSession::addFetcher(std::string initUri,std::string mediaUri,uint32_t fetcherBwIndex,StreamType streamtype) {
    ssize_t index = mFetcherInfos.indexOfKey(mediaUri.c_str());

    if (index >= 0) {
        return NULL;
    }

    sp<AMessage> notify = new AMessage(kWhatFetcherNotify, id());
    notify->setString("dashMediaUri", mediaUri.c_str());
    notify->setInt32("file-Format", mMPDMediaContainer);
    notify->setInt32("Drm-type", mMPDDrmType);
   
    FetcherInfo info;
    info.mIsPrepared = false;
    info.mBwIndex = fetcherBwIndex;
    info.mFetcherStreamMask = streamtype;
    
    if(streamtype == STREAMTYPE_VIDEO)
    {
        if(initUri.length() != 0 && mPrevVideoInitURL != initUri)
            notify->setString("dashInitUri", initUri.c_str());
        else
            notify->setString("dashInitUri", "");
        mPrevVideoInitURL = initUri; 

       info.mUrlInfo = mVideoURIInfo;
       notify->setInt64("bandwidth", mVideoURIInfo.mBandwidth);
    }
    else if(streamtype == STREAMTYPE_AUDIO)
    {
        if(initUri.length() != 0 && mPrevAudioInitURL != initUri)
            notify->setString("dashInitUri", initUri.c_str());
        else
            notify->setString("dashInitUri", "");
        mPrevAudioInitURL = initUri; 
        
        info.mUrlInfo = mAudioURIInfo;
        notify->setInt64("bandwidth", mAudioURIInfo.mBandwidth);
    }
    else if(streamtype == STREAMTYPE_SUBTITLES)
    {
        if(initUri.length() != 0 && mPrevSubtitleInitURL != initUri)
            notify->setString("dashInitUri", initUri.c_str());
        else
            notify->setString("dashInitUri", "");
        mPrevSubtitleInitURL = initUri;
        
        info.mUrlInfo = mSubtitleURIInfo;
        notify->setInt64("bandwidth", mSubtitleURIInfo.mBandwidth);
    }

    info.mFetcher = new DashFileFetcher(notify, this, mMPDPlaylist,info.mUrlInfo.mStartTime,info.mUrlInfo.mDuration);
    
    looper()->registerHandler(info.mFetcher);

    mFetcherInfos.add(mediaUri.c_str(), info);

    return info.mFetcher;
}

status_t DASHSession::fetchFile(const char *url, sp<ABuffer> *out,int64_t range_offset, int64_t range_length) {
    *out = NULL;

    sp<DataSource> source;

    if (!strncasecmp(url, "file://", 7)) {
        source = new FileSource(url + 7);
    } else if (strncasecmp(url, "http://", 7)
            && strncasecmp(url, "https://", 8)) {
        ALOGE("unsupported file source %s", url);
        return ERROR_UNSUPPORTED;
    } else {
        KeyedVector<String8, String8> headers = mExtraHeaders;
        if (range_offset > 0 || range_length >= 0) {
            headers.add(
                    String8("Range"),
                    String8(
                        StringPrintf(
                            "bytes=%lld-%s",
                            range_offset,
                            range_length < 0
                                ? "" : StringPrintf("%lld", range_offset + range_length - 1).c_str()).c_str()));
        }
        status_t err = mHTTPDataSource->connect(url, &headers);

        if (err != OK) {
            ALOGE("connect fail");
            return err;
        }

        source = mHTTPDataSource;
    }

    off64_t size;
    status_t err = source->getSize(&size);

    if (err != OK) {
        size = 65536;
    }

    ALOGV("filefile-url:%s,range_offset:%lld,range_length:%lld,size:%lld",url,range_offset,range_length,size);

    sp<ABuffer> buffer = new ABuffer(size);
    buffer->setRange(0, 0);
    
    for (;;) {

        size_t bufferRemaining = buffer->capacity() - buffer->size();

        if (bufferRemaining == 0) {
            bufferRemaining = 32768;

            ALOGV("increasing download buffer to %d bytes",
                 buffer->size() + bufferRemaining);

            sp<ABuffer> copy = new ABuffer(buffer->size() + bufferRemaining);
            memcpy(copy->data(), buffer->data(), buffer->size());
            copy->setRange(0, buffer->size());

            buffer = copy;
        }

        size_t maxBytesToRead = bufferRemaining;
        if (range_length >= 0) {
            int64_t bytesLeftInRange = range_length - buffer->size();
            if (bytesLeftInRange < maxBytesToRead) {
                maxBytesToRead = bytesLeftInRange;

                if (bytesLeftInRange == 0) {
                    break;
                }
            }
        }

        ssize_t n = source->readAt(
                buffer->size(), buffer->data() + buffer->size(),
                maxBytesToRead);

        if (n < 0) {
            return n;
        }

        if (n == 0) {
            break;
        }

        buffer->setRange(0, buffer->size() + (size_t)n);
    }

    *out = buffer;

    return OK;
}

bool DASHSession::needToSwitchBandwidth(const sp<AMessage> &fetchNotify,int32_t segMediatimeUs)
{ 
    if(mVideoStreamState != STREAMSTATE_NONE)
        return false;

    AString uri;
    CHECK(fetchNotify->findString("dashMediaUri", &uri));
    FetcherInfo *info = &mFetcherInfos.editValueFor(uri);
            
    int32_t curBandwidthIndex = getBandwidthIndex();

    //1.dashfilefetcher is prepared;
    //2.dashfilefetcher bandwidth != getBandwidthIndex
    if (info->mIsPrepared && info->mBwIndex != curBandwidthIndex)
    {
       ALOGD("new dashfilefetcher pull in");
       //new dashfilefetcher pull in
       sp<AMessage> msg = new AMessage(kWhatStreamSwitching, id());
       msg->setInt32("streamMask", STREAMTYPE_VIDEO);
       msg->setInt32("segstarttimeus", 0);
       msg->setInt32("segmediatimeus", segMediatimeUs);
       msg->setInt32("newFetcherBwIndex", curBandwidthIndex);
       msg->setString("staleFetcherUri", uri.c_str());
       
       msg->setInt32("streamswitchgeneration", mStreamSwitchGeneration);

       msg->post();
       return true;
    }

    return false;
}
//static 
void DASHSession::MPDCallback(MPDPlaylists::MPDMessage message)
{

}

MPDPlaylists* DASHSession::fetchPlaylist(const char *url) {
    ALOGD("fetchPlaylist '%s'", url);

    sp<ABuffer> buffer;
    status_t err = fetchFile(url, &buffer);

    if (err != OK) {
        return NULL;
    }

    ALOGD("Playlist (size = %d),url:%s", buffer->size(),url);
    ALOGD("%s", (const char *)buffer->data());

    char DumpFileName[] = "/sdcard/dump_play.mpd";
    FILE *PlayListFile = fopen(DumpFileName, "wb");
    if (PlayListFile != NULL) {
        CHECK_EQ(fwrite(buffer->data(), 1, buffer->size(), PlayListFile),
                buffer->size());
        fclose(PlayListFile);
    
    } else {
        ALOGE("error to create dump playlist file %s", DumpFileName);
    }
    
    MPDPlaylists* playlist = new MPDPlaylists((const char *)buffer->data(),buffer->size(),url,MPDCallback);

    if (playlist != NULL && !playlist->initCheck()) {
        ALOGE("failed to parse .mpd playlist");
        delete playlist;
        return NULL;
    }

    return playlist;
}

void DASHSession::cancelStreamSwitching()
{
    ++mStreamSwitchGeneration;
}

int64_t DASHSession::getVideoBufferedDuration()
{
    status_t finalResult;
    int64_t bufferedDurationUs = mPacketSources.valueFor(STREAMTYPE_VIDEO)->getBufferedDurationUs_l(&finalResult);
    if(finalResult == OK)
        return bufferedDurationUs;
    else
        return 0;
}

size_t DASHSession::getBandwidthIndex() {
    if (mBandwidthItems.size() == 0) {
        return 0;
    }

    int32_t bandwidthBpsLongterm = 0,bandwidthBpsShortterm = 0;
    if (mHTTPDataSource != NULL
             && mHTTPDataSource->estimateBandwidth(&bandwidthBpsLongterm,&bandwidthBpsShortterm)) {
          ALOGV("bandwidth estimated at %.2f(longterm) VS %.2f(shortterm) kbps", bandwidthBpsLongterm / 1024.0f,bandwidthBpsShortterm / 1024.0f);
    } else {
            ALOGD("no bandwidth estimate.");
            return 0;  
    }

    if(mPrevBandwidthIndex == -1)
        return 0;

    int32_t bandwidthBps;
    int64_t bufferedDurationUs = getVideoBufferedDuration();

    if(bandwidthBpsShortterm >= bandwidthBpsLongterm)
    {
        if(bufferedDurationUs >= DashFileFetcher::kMinBufferedDurationUs * 2)
            bandwidthBps = bandwidthBpsShortterm;
        else
            bandwidthBps = bandwidthBpsLongterm;
    }
    else
    {
        if(bufferedDurationUs <= DashFileFetcher::kMinBufferedDurationUs)
            bandwidthBps = bandwidthBpsShortterm;
        else
            bandwidthBps = bandwidthBpsLongterm;
    }

    int32_t index;
    if(bandwidthBps >= mBandwidthItems.itemAt(mPrevBandwidthIndex).mBandwidth)//bandwidth increase
    {
        if(((mPrevBandwidthIndex+1) < mBandwidthItems.size()) && (mBandwidthItems.itemAt(mPrevBandwidthIndex+1).mBandwidth <= bandwidthBps))
            index = mPrevBandwidthIndex + 1;
        else
            index = mPrevBandwidthIndex;
    }
    else//bandwidth decrease
    {
        index = mPrevBandwidthIndex - 1;
        for (int i = index; i >= 0; i--) {
            if(mBandwidthItems.itemAt(i).mBandwidth <= (unsigned long)bandwidthBps) {
                index = i;
                break;
            }
        }

        if(index < 0)
            index = 0;
    }

    ALOGD("bandwidth estimated at %.2f(longterm) VS %.2f(shortterm) kbps,buffered duration:%lld,bandwidth index:%d,mPrevBandwidthIndex:%d", bandwidthBpsLongterm / 1024.0f,bandwidthBpsShortterm / 1024.0f,bufferedDurationUs,index,mPrevBandwidthIndex);
    return index;
}

status_t DASHSession::onSeek(const sp<AMessage> &msg) {
    int64_t timeUs;
    CHECK(msg->findInt64("timeUs", &timeUs));

    ALOGD("onSeek:%lld", timeUs);
    if (!mReconfigurationInProgress) {
        cancelRefreshMPD();
        changeConfiguration(timeUs, getBandwidthIndex());
    }
    else
        mseekTimeUsAtCfg = timeUs; 
    return OK;
}

status_t DASHSession::getDuration(int64_t *durationUs) const {
    if(mMPDPlaylist)
        *durationUs = mMPDPlaylist->getDuration();
    else
        *durationUs = 0;
    return OK;
}

bool DASHSession::isSeekable() const {
    int64_t durationUs;
    return getDuration(&durationUs) == OK && durationUs > 0;
}

void DASHSession::changeConfiguration_l(AString staleFetcherUrl,int32_t segStarttimeUs,int32_t segMediatimeUs, size_t newFetcherBwIndex,uint32_t fetcherStreamMask) {
    ALOGD("changeConfiguration_l staleFetcherUrl:%s,segStarttimeUs:%d,segMediatimeUs:%d,newFetcherBwIndex:%d,fetcherStreamMask:%d",staleFetcherUrl.c_str(),segStarttimeUs,segMediatimeUs,newFetcherBwIndex,fetcherStreamMask);
    mPrevBandwidthIndex = newFetcherBwIndex;
    //remove stale dashfilefetcher and queue BANDWIDTH_SWITCHING
    sp<DashFileFetcher> stalefetcher = mFetcherInfos.valueFor(staleFetcherUrl).mFetcher;
    mFetcherInfos.removeItem(staleFetcherUrl);
    
    stalefetcher->stop();
    stalefetcher.clear();

    CHECK_LT(newFetcherBwIndex, mBandwidthItems.size());
    unsigned long bandwidth = mBandwidthItems.itemAt(newFetcherBwIndex).mBandwidth;

    std::string mediaUri,initUri;
   
    if(fetcherStreamMask == STREAMTYPE_VIDEO)
    {
        MPDPlaylists::MPDURLInfo videoUrlInfo;

        videoUrlInfo.mType = MPDPlaylists::MPD_VIDEO;
        videoUrlInfo.mStartTime = segStarttimeUs + segMediatimeUs;
        videoUrlInfo.mBandwidth = bandwidth;
        
        if(mMPDPlaylist->getUrl(videoUrlInfo) && videoUrlInfo.mHasUrl) {
            ALOGD("VIDEO:%s,%s",videoUrlInfo.mMediaUrl.mUrl.c_str(),videoUrlInfo.mInitUrl.mUrl.c_str());
            mVideoURIInfo = videoUrlInfo;
            mediaUri = mVideoURIInfo.mMediaUrl.mUrl;
            initUri = mVideoURIInfo.mInitUrl.mUrl;
        }
        else
        {
            ALOGE("No video url info");
            finishDisconnect_l();
            postError(UNKNOWN_ERROR);
            return;
        }
    }
    else if(fetcherStreamMask == STREAMTYPE_AUDIO)
    {
        MPDPlaylists::MPDURLInfo audioUrlInfo;

        audioUrlInfo.mType = MPDPlaylists::MPD_AUDIO;
        audioUrlInfo.mStartTime = segStarttimeUs + segMediatimeUs;
        audioUrlInfo.mBandwidth = bandwidth;
        
        if(mMPDPlaylist->getUrl(audioUrlInfo) && audioUrlInfo.mHasUrl) {
            ALOGD("AUDIO:%s,%s",audioUrlInfo.mMediaUrl.mUrl.c_str(),audioUrlInfo.mInitUrl.mUrl.c_str());
            mAudioURIInfo = audioUrlInfo;
            mediaUri = mAudioURIInfo.mMediaUrl.mUrl;
            initUri = mAudioURIInfo.mInitUrl.mUrl;
        }
        else
            ALOGD("No audio url info");
    }
    else if(fetcherStreamMask == STREAMTYPE_SUBTITLES)
    {
        MPDPlaylists::MPDURLInfo subtitleUrlInfo;
        
        subtitleUrlInfo.mType = MPDPlaylists::MPD_SUBTITLE;
        subtitleUrlInfo.mStartTime = segStarttimeUs + segMediatimeUs;
        subtitleUrlInfo.mBandwidth = bandwidth;
        
        if(mMPDPlaylist->getUrl(subtitleUrlInfo) && subtitleUrlInfo.mHasUrl) {
            ALOGD("SUBTITLES:%s,%s",subtitleUrlInfo.mMediaUrl.mUrl.c_str(),subtitleUrlInfo.mInitUrl.mUrl.c_str());
            mSubtitleURIInfo = subtitleUrlInfo;
            mediaUri = mSubtitleURIInfo.mMediaUrl.mUrl;
            initUri = mSubtitleURIInfo.mInitUrl.mUrl;
        }
        else
            ALOGD("No subtitle url info");
    }
    
    if(fetcherStreamMask == STREAMTYPE_VIDEO)
    {
        ABuffer *discontinuity = new ABuffer(0);
        discontinuity->meta()->setInt32("discontinuity", ATSParser::DISCONTINUITY_FORMATCHANGE);
        discontinuity->meta()->setInt64("timeUs", -1);

        mPacketSources.valueFor(STREAMTYPE_VIDEO)->queueAccessUnit(discontinuity);

        mVideoStreamState = STREAMSTATE_QUEUE;
    }
    else if(fetcherStreamMask == STREAMTYPE_AUDIO)
    {
        ABuffer *discontinuity = new ABuffer(0);
        discontinuity->meta()->setInt32("discontinuity", ATSParser::DISCONTINUITY_FORMATCHANGE);
        discontinuity->meta()->setInt64("timeUs", -1);
        
        mPacketSources.valueFor(STREAMTYPE_AUDIO)->queueAccessUnit(discontinuity);
        mAudioStreamState = STREAMSTATE_QUEUE;
    }

    ALOGD("creating new fetchers for mask 0x%08x", fetcherStreamMask);
    
    sp<DashFileFetcher> fetcher = addFetcher(initUri,mediaUri,newFetcherBwIndex,(StreamType)fetcherStreamMask);
    
    CHECK(fetcher != NULL);
    
    sp<AnotherPacketSource> mediaPacketSource = mPacketSources.valueFor((StreamType)fetcherStreamMask);
    if(fetcherStreamMask == STREAMTYPE_AUDIO)
        fetcher->startAsync(mediaPacketSource,NULL,NULL,segMediatimeUs);
    else if(fetcherStreamMask == STREAMTYPE_VIDEO)
        fetcher->startAsync(NULL,mediaPacketSource,NULL,segMediatimeUs);
    else if(fetcherStreamMask == STREAMTYPE_SUBTITLES)
        fetcher->startAsync(NULL,NULL,mediaPacketSource,segMediatimeUs);

    ALOGD("XXX configuration_l change completed.");

    if (mDisconnectReplyID != 0) {
        finishDisconnect();
    }    
}

void DASHSession::changeConfiguration(int64_t timeUs, size_t bandwidthIndex) {
    CHECK(!mReconfigurationInProgress);
    mReconfigurationInProgress = true;
    mPrevBandwidthIndex = bandwidthIndex;
    
    ALOGD("timeUs:%lld, bandwidthIndex:%d,mseeking:%d",timeUs,bandwidthIndex,mSeeking);

    CHECK_LT(bandwidthIndex, mBandwidthItems.size());
    unsigned long bandwidth = mBandwidthItems.itemAt(bandwidthIndex).mBandwidth;

    uint32_t streamMask = 0;

    MPDPlaylists::MPDURLInfo videoUrlInfo,audioUrlInfo,subtitleUrlInfo;

    videoUrlInfo.mType = MPDPlaylists::MPD_VIDEO;
    videoUrlInfo.mStartTime = timeUs;
    videoUrlInfo.mBandwidth = bandwidth;

    if(mMPDPlaylist->getUrl(videoUrlInfo) && videoUrlInfo.mHasUrl) {
        streamMask |= STREAMTYPE_VIDEO;
        ALOGD("VIDEO:%s,%s",videoUrlInfo.mMediaUrl.mUrl.c_str(),videoUrlInfo.mInitUrl.mUrl.c_str());
    }
    else
    {
        ALOGE("No video url info");
        finishDisconnect_l();
        postError(UNKNOWN_ERROR);
        return;
    }
    
    audioUrlInfo.mType = MPDPlaylists::MPD_AUDIO;
    audioUrlInfo.mStartTime = timeUs;
    audioUrlInfo.mBandwidth = bandwidth;

    if(mMPDPlaylist->getUrl(audioUrlInfo) && audioUrlInfo.mHasUrl) {
        streamMask |= STREAMTYPE_AUDIO;
        ALOGD("AUDIO:%s,%s",audioUrlInfo.mMediaUrl.mUrl.c_str(),audioUrlInfo.mInitUrl.mUrl.c_str());
    }
    else
        ALOGD("No audio url info");

    subtitleUrlInfo.mType = MPDPlaylists::MPD_SUBTITLE;
    subtitleUrlInfo.mStartTime = timeUs;
    subtitleUrlInfo.mBandwidth = bandwidth;

    if(mMPDPlaylist->getUrl(subtitleUrlInfo) && subtitleUrlInfo.mHasUrl) {
        streamMask |= STREAMTYPE_SUBTITLES;
        ALOGD("SUBTITLES:%s,%s",subtitleUrlInfo.mMediaUrl.mUrl.c_str(),subtitleUrlInfo.mInitUrl.mUrl.c_str());
    }
    else
        ALOGD("No subtitle url info");

    // Step 1, stop and discard fetchers that are no longer needed.
    // Pause those that we'll reuse.
    uint32_t streamMaskToRemove = 0;//keep streams-fetcher needs to remove
    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        const AString &uri = mFetcherInfos.keyAt(i);

        bool discardFetcher = true;

        // If we're seeking all current fetchers are discarded.
        if (timeUs < 0ll) {
            if (((streamMask & STREAMTYPE_AUDIO) && !strcmp(audioUrlInfo.mMediaUrl.mUrl.c_str(),uri.c_str()))
                    || ((streamMask & STREAMTYPE_VIDEO) && !strcmp(videoUrlInfo.mMediaUrl.mUrl.c_str(),uri.c_str()))
                    || ((streamMask & STREAMTYPE_SUBTITLES) && !strcmp(subtitleUrlInfo.mMediaUrl.mUrl.c_str(),uri.c_str()))) {
                discardFetcher = false;
            }
        }
        
        if (discardFetcher) {
            mFetcherInfos.valueAt(i).mFetcher->stop();
            streamMaskToRemove |= mFetcherInfos.valueAt(i).mFetcherStreamMask;
            //discard buffer and clear format while seek
            mPacketSources.valueFor((StreamType)mFetcherInfos.valueAt(i).mFetcherStreamMask)->clear();
        } else {
            mFetcherInfos.valueAt(i).mFetcher->pause();
        }
    }

    for (size_t i = 0; i < mFetcherInfos.size(); ) {
        if(mFetcherInfos.valueAt(i).mFetcherStreamMask & streamMaskToRemove)
        {
            ALOGD("remove stream-fetcher:%d",mFetcherInfos.valueAt(i).mFetcherStreamMask);
            sp<DashFileFetcher> fetcher = mFetcherInfos.valueAt(i).mFetcher;
            mFetcherInfos.removeItemsAt(i);
        
            fetcher.clear();
        }
        else
            i++;
    }

    mAudioURIInfo = audioUrlInfo;
    mVideoURIInfo = videoUrlInfo;
    mSubtitleURIInfo = subtitleUrlInfo;
    
    sp<AMessage> msg = new AMessage(kWhatChangeConfiguration2, id());
    msg->setInt32("streamMask", streamMask);
    msg->setInt64("timeUs", timeUs);
    msg->setInt32("bandwidthindex", bandwidthIndex);

    if (streamMask & STREAMTYPE_AUDIO) {
        msg->setString("audioMediaURI", audioUrlInfo.mMediaUrl.mUrl.c_str());
        msg->setString("audioInitURI", audioUrlInfo.mInitUrl.mUrl.c_str());
    }
    if (streamMask & STREAMTYPE_VIDEO) {
        msg->setString("videoMediaURI", videoUrlInfo.mMediaUrl.mUrl.c_str());
        msg->setString("videoInitURI", videoUrlInfo.mInitUrl.mUrl.c_str());
    }
    if (streamMask & STREAMTYPE_SUBTITLES) {
        msg->setString("subtitleMediaURI", subtitleUrlInfo.mMediaUrl.mUrl.c_str());
        msg->setString("subtitleInitURI", subtitleUrlInfo.mInitUrl.mUrl.c_str());
    }

    msg->post();
   
}

void DASHSession::onChangeConfiguration(const sp<AMessage> &msg) {
    if (!mReconfigurationInProgress) {
        changeConfiguration(-1ll /* timeUs */, getBandwidthIndex());
    } else {
        msg->post(1000000ll); // retry in 1 sec
    }
}

void DASHSession::onChangeConfiguration2(const sp<AMessage> &msg) {
    // All fetchers are either suspended or have been removed now.
    if (mDisconnectReplyID != 0) {
        finishDisconnect();
        return;
    }

    if(mStreamMask == 0)
    {
        onChangeConfiguration3(msg);
        return;
    }    

    // Something changed, inform the player which will shutdown the
    // corresponding decoders and will post the reply once that's done.
    // Handling the reply will continue executing below in
    // onChangeConfiguration3.
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatStreamsChanged);//just conside all the real streams changed
    notify->setInt32("changedMask", mStreamMask);

    msg->setWhat(kWhatChangeConfiguration3);
    msg->setTarget(id());

    notify->setMessage("reply", msg);
    notify->post();
}

void DASHSession::onChangeConfiguration3(const sp<AMessage> &msg) {
    // All remaining fetchers are still suspended, the player has shutdown
    // any decoders that needed it.

    uint32_t streamMask;
    CHECK(msg->findInt32("streamMask", (int32_t *)&streamMask));

    AString audioMediaURI, videoMediaURI, subtitleMediaURI;
    AString audioInitURI, videoInitURI, subtitleInitURI;
    if (streamMask & STREAMTYPE_AUDIO) {
        CHECK(msg->findString("audioMediaURI", &audioMediaURI));
        CHECK(msg->findString("audioInitURI", &audioInitURI));
    }
    if (streamMask & STREAMTYPE_VIDEO) {
        CHECK(msg->findString("videoMediaURI", &videoMediaURI));
        CHECK(msg->findString("videoInitURI", &videoInitURI));
    }
    if (streamMask & STREAMTYPE_SUBTITLES) {
        CHECK(msg->findString("subtitleMediaURI", &subtitleMediaURI));
        CHECK(msg->findString("subtitleInitURI", &subtitleInitURI));
    }

    int64_t timeUs;
    CHECK(msg->findInt64("timeUs", &timeUs));

    if (timeUs < 0ll) {
        timeUs = mLastDequeuedTimeUs;
    }
    
    if (mseekTimeUsAtCfg >= 0)
        timeUs = mseekTimeUsAtCfg;

    mRealTimeBaseUs = ALooper::GetNowUs() - timeUs;
    mStreamMask = streamMask;

    // Resume all existing fetchers and assign them packet sources.
    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        const AString &uri = mFetcherInfos.keyAt(i);

        uint32_t resumeMask = 0;

        sp<AnotherPacketSource> audioSource;
        if ((streamMask & STREAMTYPE_AUDIO) && uri == audioMediaURI) {
            audioSource = mPacketSources.valueFor(STREAMTYPE_AUDIO);
            resumeMask |= STREAMTYPE_AUDIO;
        }

        sp<AnotherPacketSource> videoSource;
        if ((streamMask & STREAMTYPE_VIDEO) && uri == videoMediaURI) {
            videoSource = mPacketSources.valueFor(STREAMTYPE_VIDEO);
            resumeMask |= STREAMTYPE_VIDEO;
        }

        sp<AnotherPacketSource> subtitleSource;
        if ((streamMask & STREAMTYPE_SUBTITLES) && uri == subtitleMediaURI) {
            subtitleSource = mPacketSources.valueFor(STREAMTYPE_SUBTITLES);
            resumeMask |= STREAMTYPE_SUBTITLES;
        }

        CHECK_NE(resumeMask, 0u);

        ALOGD("resuming fetchers for mask 0x%08x", resumeMask);

        streamMask &= ~resumeMask;
        if(mseekTimeUsAtCfg >= 0)
        {
            mFetcherInfos.valueAt(i).mFetcher->startAsync(
                audioSource, videoSource, subtitleSource, timeUs);            
        }
        else
            mFetcherInfos.valueAt(i).mFetcher->startAsync(
                audioSource, videoSource, subtitleSource);
    }

    // streamMask now only contains the types that need a new fetcher created.

    if (streamMask != 0) {
        ALOGD("creating new fetchers for mask 0x%08x", streamMask);
    }

    while (streamMask != 0) {
        StreamType streamType = (StreamType)(streamMask & ~(streamMask - 1));

        AString mediaUri,initUri;
        switch (streamType) {
            case STREAMTYPE_AUDIO:
                mediaUri = audioMediaURI;
                initUri = audioInitURI;
                break;
            case STREAMTYPE_VIDEO:
                mediaUri = videoMediaURI;
                initUri = videoInitURI;
                break;
            case STREAMTYPE_SUBTITLES:
                mediaUri = subtitleMediaURI;
                initUri = subtitleInitURI;
                break;
            default:
                TRESPASS();
        }
    
        int32_t bandwidthIndex;
        CHECK(msg->findInt32("bandwidthindex", &bandwidthIndex));
        sp<DashFileFetcher> fetcher = addFetcher(initUri.c_str(),mediaUri.c_str(),bandwidthIndex,streamType);
        CHECK(fetcher != NULL);

        sp<AnotherPacketSource> videoSource;
        if ((streamMask & STREAMTYPE_VIDEO) && mediaUri == videoMediaURI) {
            videoSource = mPacketSources.valueFor(STREAMTYPE_VIDEO);
            videoSource->clear();

            streamMask &= ~STREAMTYPE_VIDEO;
        }

        sp<AnotherPacketSource> audioSource;
        if ((streamMask & STREAMTYPE_AUDIO) && mediaUri == audioMediaURI) {
            audioSource = mPacketSources.valueFor(STREAMTYPE_AUDIO);
            audioSource->clear();

            streamMask &= ~STREAMTYPE_AUDIO;
        }

        sp<AnotherPacketSource> subtitleSource;
        if ((streamMask & STREAMTYPE_SUBTITLES) && mediaUri == subtitleMediaURI) {
            subtitleSource = mPacketSources.valueFor(STREAMTYPE_SUBTITLES);
            subtitleSource->clear();

            streamMask &= ~STREAMTYPE_SUBTITLES;
        }

        fetcher->startAsync(audioSource, videoSource, subtitleSource, timeUs);
    }

    // All fetchers have now been started, the configuration change
    // has completed.

    ALOGD("XXX configuration change completed.");

    mReconfigurationInProgress = false;
    
    mSeeking = false;
    mseekTimeUsAtCfg = -1;
    
    mAudioStreamState = STREAMSTATE_NONE;
    mVideoStreamState = STREAMSTATE_NONE;
    
    if (mDisconnectReplyID != 0) {
        finishDisconnect();
    }
}

void DASHSession::postError(status_t err) {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();
}

void DASHSession::postPrepared(status_t err) {
    CHECK(mInPreparationPhase);

    sp<AMessage> notify = mNotify->dup();
    if (err == OK || err == ERROR_END_OF_STREAM) {
        notify->setInt32("what", kWhatPrepared);
    } else {
        notify->setInt32("what", kWhatPreparationFailed);
        notify->setInt32("err", err);
    }

    notify->post();

    mInPreparationPhase = false;
}

}  // namespace android
