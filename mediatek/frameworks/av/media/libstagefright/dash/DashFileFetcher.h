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

#ifndef DASHFILE_FETCHER_H_

#define DASHFILE_FETCHER_H_

#include <media/stagefright/foundation/AHandler.h>

#include "DashFmp4Parser.h"
#include "DashSession.h"
#include "DashDataProcessor.h"
#include "MPDPlaylists.h"

using namespace android_dash;

namespace android {

struct ABuffer;
struct AnotherPacketSource;
struct String8;
struct DashDataProcessor;

struct DashFileFetcher : public AHandler {
    enum {
        kWhatStarted,
        kWhatError,
        kWhatTemporarilyDoneFetching,
        kWhatPrepared,
        kWhatPreparationFailed,
        kWhatDDPNotify,
    };

    DashFileFetcher(const sp<AMessage> &notify,const sp<DASHSession> &session,MPDPlaylists * mpdPlaylists,int64_t basetimeUs,int64_t duration);

    void startAsync(
            const sp<AnotherPacketSource> &audioSource,
            const sp<AnotherPacketSource> &videoSource,
            const sp<AnotherPacketSource> &subtitleSource,
            int64_t startTimeUs = -1ll);
    
    void pause();//sync func

    void stop();

    static const int64_t kMinBufferedDurationUs;
protected:
    virtual ~DashFileFetcher();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatStart          = 'strt',
        kWhatMonitorQueue   = 'moni',
        kWhatDashDataInfo   = 'ddif',
    };

    sp<AMessage> mNotify;
    sp<DASHSession> mSession;
    AString mInitUri;
    AString mMediaUri;
    int64_t mOffset;
    bool mNotifyPrepared;
    bool mInitUriDownloaded;

    //for fMP4
    bool mGotSidxForFMP4;
    int64_t getDurationForSeqNumber(int32_t seqnumber) const;
    int32_t getSeqNumberForTime(int64_t timeUs) const;
    int32_t getSeqNumberForOffset(int64_t offset) const;
    Vector<DashFmp4Parser::SidxEntry> mAudioSidx,mVideoSidx;

    uint32_t mStreamTypeMask;
    int64_t mStartTimeUs;
    int64_t mBaseTimeUs;
    int64_t mMediaDuration;

    //pass to DDP
    int32_t mFileFormat;
    int32_t mDrmType;
    int64_t mBandwidth;
    MPDPlaylists* mMPDPlaylist;

    KeyedVector<DASHSession::StreamType, sp<AnotherPacketSource> > mPacketSources;

    int32_t mSeqNumber;/*moof seq No. for small/big fMP4 OR ts seq No.*/
    int32_t mMonitorQueueGeneration;

    sp<DashDataProcessor> mDataProcessor;

    void postMonitorQueue(int64_t delayUs = 0);
    void cancelMonitorQueue();

    status_t onStart(const sp<AMessage> &msg);
    void onMonitorQueue();
    void onDownloadNext();

    int64_t getDownloadLength();

    status_t feedMoreBuffers(sp<ABuffer> &buffer,bool startDequeueFlag = false);

    void notifyError(status_t err);

    DISALLOW_EVIL_CONSTRUCTORS(DashFileFetcher);
};

}  // namespace android

#endif  // DASHFILE_FETCHER_H_

