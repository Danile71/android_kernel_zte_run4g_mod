/*
 * Copyright (C) 2012 The Android Open Source Project
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
#define LOG_TAG "DashFileFetcher"
#include <utils/Log.h>

#include "DashFileFetcher.h"

#include "AnotherPacketSource.h"

#include <media/IStreamSource.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

namespace android {

// static
const int64_t DashFileFetcher::kMinBufferedDurationUs = 10000000ll;

DashFileFetcher::DashFileFetcher(const sp<AMessage> &notify,const sp<DASHSession> &session,MPDPlaylists * mpdPlaylists,int64_t basetimeUs,int64_t duration)
    : mNotify(notify),
      mSession(session),
      mStreamTypeMask(0),
      mStartTimeUs(-1ll),
      mBaseTimeUs(basetimeUs),
      mSeqNumber(-1),
      mMonitorQueueGeneration(0),
      mDataProcessor(NULL),
      mOffset(0ll),
      mNotifyPrepared(false),
      mGotSidxForFMP4(false),
      mInitUriDownloaded(false),
      mMPDPlaylist(mpdPlaylists) {
      CHECK(mNotify->findString("dashMediaUri", &mMediaUri));
      CHECK(mNotify->findString("dashInitUri", &mInitUri));
      CHECK(mNotify->findInt32("file-Format", &mFileFormat));
      CHECK(mNotify->findInt32("Drm-type", &mDrmType));
      CHECK(mNotify->findInt64("bandwidth", &mBandwidth));
      mMediaDuration = duration;
}

DashFileFetcher::~DashFileFetcher() {
    if(mDataProcessor.get())
        mDataProcessor.clear();
}

int64_t DashFileFetcher::getDurationForSeqNumber(int32_t seqnumber) const {
   if(mStreamTypeMask == DASHSession::STREAMTYPE_VIDEO)
   {
       if(seqnumber < mVideoSidx.size())
          return mVideoSidx[seqnumber].mSegmentStartUs;
   }
   else if(mStreamTypeMask == DASHSession::STREAMTYPE_AUDIO)
   {
       if(seqnumber < mAudioSidx.size())
          return mAudioSidx[seqnumber].mSegmentStartUs;
   }

   return 0;
}

int32_t DashFileFetcher::getSeqNumberForTime(int64_t timeUs) const {
   if(mStreamTypeMask == DASHSession::STREAMTYPE_VIDEO)
   {
       for(int i = 0 ; i < mVideoSidx.size() ; i++)
       {
           if((mVideoSidx[i].mSegmentStartUs <= timeUs) && (mVideoSidx[i].mSegmentStartUs + mVideoSidx[i].mDurationUs > timeUs))
               return i;
       }
   }
   else if(mStreamTypeMask == DASHSession::STREAMTYPE_AUDIO)
   {
       for(int i = 0 ; i < mAudioSidx.size() ; i++)
       {
           if((mAudioSidx[i].mSegmentStartUs <= timeUs) && (mAudioSidx[i].mSegmentStartUs + mAudioSidx[i].mDurationUs > timeUs))
               return i;
       }
   }

   return 0;
}

int32_t DashFileFetcher::getSeqNumberForOffset(int64_t offset) const {
   if(mStreamTypeMask == DASHSession::STREAMTYPE_VIDEO)
   {
       for(int i = 0 ; i < mVideoSidx.size()-1 ; i++)
       {
           if((mVideoSidx[i].mOffset <= offset) && (mVideoSidx[i+1].mOffset > offset))
               return i;
       }
       return mVideoSidx.size()-1;
   }
   else if(mStreamTypeMask == DASHSession::STREAMTYPE_AUDIO)
   {
       for(int i = 0 ; i < mAudioSidx.size()-1 ; i++)
       {
           if((mAudioSidx[i].mOffset <= offset) && (mAudioSidx[i+1].mOffset > offset))
               return i;
       }
       return mAudioSidx.size()-1;
   }

   return 0;
}

void DashFileFetcher::postMonitorQueue(int64_t delayUs) {
    if(mStreamTypeMask == DASHSession::STREAMTYPE_VIDEO && mGotSidxForFMP4 && mSession->needToSwitchBandwidth(mNotify->dup(),getDurationForSeqNumber(mSeqNumber+1)))
    {
        ALOGD("mSeqNumber+1:%d,starttime:%lld",mSeqNumber+1,getDurationForSeqNumber(mSeqNumber+1));
        cancelMonitorQueue();
        return;
    }
    sp<AMessage> msg = new AMessage(kWhatMonitorQueue, id());
    msg->setInt32("generation", mMonitorQueueGeneration);
    msg->post(delayUs);
}

void DashFileFetcher::cancelMonitorQueue() {
    ++mMonitorQueueGeneration;
}

void DashFileFetcher::startAsync(
        const sp<AnotherPacketSource> &audioSource,
        const sp<AnotherPacketSource> &videoSource,
        const sp<AnotherPacketSource> &subtitleSource,
        int64_t startTimeUs) {
    sp<AMessage> msg = new AMessage(kWhatStart, id());

    uint32_t streamTypeMask = 0ul;

    if (audioSource != NULL) {
        msg->setPointer("audioSource", audioSource.get());
        streamTypeMask |= DASHSession::STREAMTYPE_AUDIO;
    }

    if (videoSource != NULL) {
        msg->setPointer("videoSource", videoSource.get());
        streamTypeMask |= DASHSession::STREAMTYPE_VIDEO;
    }

    if (subtitleSource != NULL) {
        msg->setPointer("subtitleSource", subtitleSource.get());
        streamTypeMask |= DASHSession::STREAMTYPE_SUBTITLES;
    }

    msg->setInt32("streamTypeMask", streamTypeMask);
    msg->setInt64("startTimeUs", startTimeUs);
    
    ALOGD("startTimeUs[%d]:%lld",streamTypeMask,startTimeUs);
    msg->post();
}

void DashFileFetcher::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatStart:
        {
            status_t err = onStart(msg);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatStarted);
            notify->setInt32("err", err);
            notify->post();
            break;
        }

        case kWhatMonitorQueue:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mMonitorQueueGeneration) {
                // Stale event
                break;
            }

            onMonitorQueue();
            break;
        }

        case kWhatDDPNotify://notify from DASHDataProcessor
        {
            int32_t what;
            CHECK(msg->findInt32("what",&what));
            if(what == DashDataProcessor::kWhatSidx)
            {
                //sidx info for fMP4
                Vector<DashFmp4Parser::SidxEntry> * ptVideoSidx = NULL;
                Vector<DashFmp4Parser::SidxEntry> * ptAudioSidx = NULL;
                if(msg->findPointer("video-sidx-vector", (void **)&ptVideoSidx) && ptVideoSidx != NULL)
                {
                    mVideoSidx = *ptVideoSidx;
                    ALOGD("Got Video sidx info");
                    for(int i = 0;i < mVideoSidx.size();i++)
                        ALOGD("entry[%d]-offset:%lld,size:%d,starttime:%lld,duration:%d",i,mVideoSidx[i].mOffset,mVideoSidx[i].mSize,mVideoSidx[i].mSegmentStartUs,mVideoSidx[i].mDurationUs);
                }
                if(msg->findPointer("audio-sidx-vector", (void **)&ptAudioSidx) && ptAudioSidx != NULL)
                {
                    mAudioSidx = *ptAudioSidx;
                    ALOGD("Got Audio sidx info");
                    for(int i = 0;i < mAudioSidx.size();i++)
                        ALOGD("entry[%d]-offset:%lld,size:%d,starttime:%lld,duration:%d",i,mAudioSidx[i].mOffset,mAudioSidx[i].mSize,mAudioSidx[i].mSegmentStartUs,mAudioSidx[i].mDurationUs);

                }
                mGotSidxForFMP4 = true;
            }
           
            break;
        }

        default:
            TRESPASS();
    }
}

status_t DashFileFetcher::onStart(const sp<AMessage> &msg) {
    mPacketSources.clear();

    uint32_t streamTypeMask;
    CHECK(msg->findInt32("streamTypeMask", (int32_t *)&streamTypeMask));

    int64_t startTimeUs;
    CHECK(msg->findInt64("startTimeUs", &startTimeUs));

    if (streamTypeMask & DASHSession::STREAMTYPE_AUDIO) {
        void *ptr;
        CHECK(msg->findPointer("audioSource", &ptr));

        mPacketSources.add(
                DASHSession::STREAMTYPE_AUDIO,
                static_cast<AnotherPacketSource *>(ptr));
    }

    if (streamTypeMask & DASHSession::STREAMTYPE_VIDEO) {
        void *ptr;
        CHECK(msg->findPointer("videoSource", &ptr));

        mPacketSources.add(
                DASHSession::STREAMTYPE_VIDEO,
                static_cast<AnotherPacketSource *>(ptr));
    }

    if (streamTypeMask & DASHSession::STREAMTYPE_SUBTITLES) {
        void *ptr;
        CHECK(msg->findPointer("subtitleSource", &ptr));

        mPacketSources.add(
                DASHSession::STREAMTYPE_SUBTITLES,
                static_cast<AnotherPacketSource *>(ptr));
    }

    mStreamTypeMask = streamTypeMask;
    mStartTimeUs = startTimeUs;

    if (mStartTimeUs >= 0ll) {
        mSeqNumber = -1;
    }
    
    postMonitorQueue();

    return OK;
}

void DashFileFetcher::pause() {
    cancelMonitorQueue();

    mPacketSources.clear();
    mStreamTypeMask = 0;
}

void DashFileFetcher::stop() {
    pause();
    if(mDataProcessor.get())
    {
        mDataProcessor->stop();
    }
}

void DashFileFetcher::notifyError(status_t err) {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();
}

void DashFileFetcher::onMonitorQueue() {
    bool downloadMore = false;

    status_t finalResult;
    if (mStreamTypeMask == DASHSession::STREAMTYPE_SUBTITLES) {
        sp<AnotherPacketSource> packetSource =
            mPacketSources.valueFor(DASHSession::STREAMTYPE_SUBTITLES);

        int64_t bufferedDurationUs =
                packetSource->getBufferedDurationUs(&finalResult);

        downloadMore = (bufferedDurationUs < kMinBufferedDurationUs);
        finalResult = OK;
    } else {
        bool first = true;
        int64_t minBufferedDurationUs = 0ll;

        for (size_t i = 0; i < mPacketSources.size(); ++i) {
            if ((mStreamTypeMask & mPacketSources.keyAt(i)) == 0) {
                continue;
            }
            int64_t bufferedDurationUs =
                mPacketSources.valueAt(i)->getBufferedDurationUs_l(&finalResult);

            if (first || bufferedDurationUs < minBufferedDurationUs) {
                minBufferedDurationUs = bufferedDurationUs;
                first = false;
            }
        }
        if(mNotifyPrepared)
            downloadMore = !first && (minBufferedDurationUs < kMinBufferedDurationUs * 4);
        else
            downloadMore = !first && (minBufferedDurationUs < kMinBufferedDurationUs / 2);
    }

    if (finalResult == OK && downloadMore) {
        onDownloadNext();
    } else {
        // Nothing to do yet, try again in a second.

        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatTemporarilyDoneFetching);
        msg->post();

        mNotifyPrepared = true;

        postMonitorQueue(1000000ll);
    }
}

int64_t DashFileFetcher::getDownloadLength() {
    int64_t rangeLength = mBandwidth / 8 * 1;//about 1s media data

    if(mMediaDuration > 0 && mMediaDuration < 5000000ll)
        rangeLength = -1;

    if((mFileFormat == MPDPlaylists::MPD_MP4) && mGotSidxForFMP4)
    {
        if(mSeqNumber < 0)
        {
            CHECK_GE(mStartTimeUs,0ll);
        
            mSeqNumber = getSeqNumberForTime(mStartTimeUs);
            sp<AMessage> flushMsg = new AMessage;
            if(mSeqNumber > 0 && mDataProcessor.get())
            {
                if(mStreamTypeMask == DASHSession::STREAMTYPE_VIDEO){
                    mOffset = mVideoSidx[mSeqNumber].mOffset;
                    flushMsg->setInt32("videoStartSegmentIndex",mSeqNumber);

                }
                if(mStreamTypeMask == DASHSession::STREAMTYPE_AUDIO){
                    mOffset = mAudioSidx[mSeqNumber].mOffset;
                    flushMsg->setInt32("audioStartSegmentIndex",mSeqNumber);

                }
            
                ALOGD("mSeqNumber:%d and flush ddp[%d]",mSeqNumber,mStreamTypeMask);
                mDataProcessor->flush(flushMsg);
            }
            else if(mSeqNumber == 0){
                mSeqNumber = getSeqNumberForOffset(mOffset);
                ALOGD("1st got the mSeqNumber %d after got sidx from ddp[%d]",mSeqNumber,mStreamTypeMask);
            }

            mStartTimeUs = -1ll;
        }

        if(mStreamTypeMask == DASHSession::STREAMTYPE_VIDEO)
        {
            int32_t seqNumber = mSeqNumber;

            for(; seqNumber < mVideoSidx.size() ; seqNumber++)
            {
                if((mVideoSidx[seqNumber].mSegmentStartUs + mVideoSidx[seqNumber].mDurationUs) - (mVideoSidx[mSeqNumber].mSegmentStartUs + mVideoSidx[mSeqNumber].mDurationUs) >= 5000000ll)//5s
                {
                    rangeLength = mVideoSidx[seqNumber].mOffset + mVideoSidx[seqNumber].mSize - mOffset;                  
                    break;
                }
            }
            if((seqNumber == mVideoSidx.size()) || (seqNumber == mVideoSidx.size()-1))
            {
                mSeqNumber = seqNumber - 1;
                rangeLength = -1;//read till the lastest moof
            }
            else
                mSeqNumber = seqNumber;
        }

        else if(mStreamTypeMask == DASHSession::STREAMTYPE_AUDIO)
        {
            int32_t seqNumber = mSeqNumber;

            for(; seqNumber < mAudioSidx.size() ; seqNumber++)
            {
                if((mAudioSidx[seqNumber].mSegmentStartUs + mAudioSidx[seqNumber].mDurationUs) - (mAudioSidx[mSeqNumber].mSegmentStartUs + mAudioSidx[mSeqNumber].mDurationUs) >= 5000000ll)//5s
                {
                    rangeLength = mAudioSidx[seqNumber].mOffset + mAudioSidx[seqNumber].mSize - mOffset;
                    break;
                }
            }
            if((seqNumber == mAudioSidx.size()) || (seqNumber == mAudioSidx.size()-1))
            {
                mSeqNumber = seqNumber - 1;
                rangeLength = -1;//read till the lastest moof
            }
            else
                mSeqNumber = seqNumber;
        }
    }

    if(rangeLength <= 0)
        rangeLength = -1;

    return rangeLength;
}

void DashFileFetcher::onDownloadNext() {
    if(!strcmp(mInitUri.c_str(),""))
        mInitUriDownloaded = true;
    if(!mInitUriDownloaded)
    {
        sp<ABuffer> initBuffer;
    
        ALOGD("fetching[%d] initUri '%s',", mStreamTypeMask, mInitUri.c_str());
        int64_t fetchStarttimeUs = ALooper::GetNowUs();
        status_t err = mSession->fetchFile(mInitUri.c_str(),&initBuffer);
        int64_t fetchEndtimeUs = ALooper::GetNowUs();
        ALOGD("fetching[%d] initUri '%s',spend:%.2f", mStreamTypeMask, mInitUri.c_str(),(fetchEndtimeUs - fetchStarttimeUs)/1E6);
        if(err != OK)
        {
            notifyError(err);
            return;
        }
        err = feedMoreBuffers(initBuffer);
        if(err != OK)
        {
            notifyError(err);
            return;
        }
        mInitUriDownloaded = true;
    }

    sp<ABuffer> buffer;
    int64_t rangelength = getDownloadLength();
        
    ALOGD("fetching[%d] '%s',offset:%lld,rangelength:%lld", mStreamTypeMask, mMediaUri.c_str(),mOffset,rangelength);
    int64_t fetchStarttimeUs = ALooper::GetNowUs();
    status_t err = mSession->fetchFile(mMediaUri.c_str(),&buffer,mOffset,rangelength);
    int64_t fetchEndtimeUs = ALooper::GetNowUs();
    ALOGD("fetching[%d] '%s',offset:%lld,rangelength:%lld,spend:%.2f", mStreamTypeMask, mMediaUri.c_str(),mOffset,rangelength,(fetchEndtimeUs - fetchStarttimeUs)/1E6);
    if(err != OK)
    {
        ALOGE("failed to fetch url '%s',mOffset:%lld,rangelength:%lld,err:%d",mMediaUri.c_str(),mOffset,rangelength,err);
        if(err == -ENOTCONN)
        {
            ALOGD("cancel read due to seek/disconnect");
            return;
        }
        notifyError(err);
        return;
    }

    mOffset += buffer->size();

    err = feedMoreBuffers(buffer,rangelength == -1);
    if(err != OK)
    {
        notifyError(err);
        return;
    }

    if(rangelength == -1)
    {
        if(!mNotifyPrepared)
        {
            sp<AMessage> msg = mNotify->dup();
            msg->setInt32("what", kWhatTemporarilyDoneFetching);
            msg->post();
            
            mNotifyPrepared = true;
        }

        cancelMonitorQueue();
        notifyError(ERROR_END_OF_STREAM);
       
        return;
    }

    postMonitorQueue();
}

status_t DashFileFetcher::feedMoreBuffers(sp<ABuffer> &buffer,bool startDequeueFlag) {
    status_t err = OK;
    if(mDataProcessor.get() == NULL)
    {
        sp<AMessage> notify = new AMessage(kWhatDDPNotify, id());
        sp<AMessage> dashDataInfo = new AMessage(kWhatDashDataInfo, id());

        sp<AnotherPacketSource> audioPacketSource=NULL,videoPacketSource=NULL,subtitlePacketSource=NULL;
        if(mStreamTypeMask == DASHSession::STREAMTYPE_AUDIO)
            audioPacketSource = mPacketSources.valueFor(DASHSession::STREAMTYPE_AUDIO);
        if(mStreamTypeMask == DASHSession::STREAMTYPE_VIDEO)
            videoPacketSource = mPacketSources.valueFor(DASHSession::STREAMTYPE_VIDEO);
        if(mStreamTypeMask == DASHSession::STREAMTYPE_SUBTITLES)
            subtitlePacketSource = mPacketSources.valueFor(DASHSession::STREAMTYPE_SUBTITLES);

        dashDataInfo->setPointer("audioSource", audioPacketSource == NULL ? NULL : audioPacketSource.get());
        dashDataInfo->setPointer("videoSource", videoPacketSource == NULL ? NULL : videoPacketSource.get());
        dashDataInfo->setPointer("subtitleSource", subtitlePacketSource == NULL ? NULL : subtitlePacketSource.get());
       
        dashDataInfo->setInt32("file-Format", mFileFormat);
        dashDataInfo->setInt32("Drm-type", mDrmType);
        dashDataInfo->setPointer("Drm-Handle", NULL);//revise to drm-handle and drm-client
        dashDataInfo->setPointer("Drm-Client", NULL);
        
        mDataProcessor = new DashDataProcessor(notify, NULL,mMPDPlaylist,dashDataInfo,mBaseTimeUs);
        if(mDataProcessor.get())
        {
            ALOGD("start DashDataProcessor[%d]", mStreamTypeMask);
            mDataProcessor->start();
        }
        else
        {
            ALOGE("create DashDataProcessor[%d] failed", mStreamTypeMask);
            return UNKNOWN_ERROR;
        }
    }

    if((mFileFormat == MPDPlaylists::MPD_MP4) && (mGotSidxForFMP4 || startDequeueFlag))
        buffer->setInt32Data(DashDataProcessor::kBufferFlag_StartDequeueAU);
    
    err = mDataProcessor->feedMoreBuffer(buffer);
    
    return err;
}

}  // namespace android
