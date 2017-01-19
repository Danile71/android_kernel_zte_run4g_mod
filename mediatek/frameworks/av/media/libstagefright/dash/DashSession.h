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

#ifndef DASH_SESSION_H_

#define DASH_SESSION_H_

#include <media/stagefright/foundation/AHandler.h>

#include <utils/String8.h>

#include "MPDPlaylists.h"

using namespace android_dash;

namespace android {

struct ABuffer;
struct AnotherPacketSource;
struct HTTPBase;
struct DashFileFetcher;

struct DASHSession : public AHandler {
    enum Flags {
        // Don't log any URLs.
        kFlagIncognito = 1,
    };   
    DASHSession(
            const sp<AMessage> &notify,
            uint32_t flags = 0, bool uidValid = false, uid_t uid = 0);

    enum StreamType {
        STREAMTYPE_VIDEO        = 1,
        STREAMTYPE_AUDIO        = 2,
        STREAMTYPE_SUBTITLES    = 4,
    };

    enum StreamState {
        STREAMSTATE_NONE        = 1,
        STREAMSTATE_QUEUE        = 2,
        STREAMSTATE_DEQUEUE      = 4,
    };

    status_t dequeueAccessUnit(StreamType stream, sp<ABuffer> *accessUnit);

    status_t getStreamFormat(StreamType stream, sp<AMessage> *format);

    void connectAsync(
            const char *url,
            const KeyedVector<String8, String8> *headers = NULL);

    status_t disconnect();

    // Blocks until seek is complete.
    status_t seekTo(int64_t timeUs);

    status_t getDuration(int64_t *durationUs) const;

    static void MPDCallback(MPDPlaylists::MPDMessage message);

    bool isSeekable() const;

    enum {
        kWhatStreamsChanged,
        kWhatError,
        kWhatPrepared,
        kWhatPreparationFailed,
    };

protected:
    virtual ~DASHSession();

    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    friend struct DashFileFetcher;

    enum {
        kWhatConnect                    = 'conn',
        kWhatDisconnect                 = 'disc',
        kWhatSeek                       = 'seek',
        kWhatFetcherNotify              = 'notf',
        kWhatChangeConfiguration        = 'chC0',
        kWhatChangeConfiguration2       = 'chC2',
        kWhatChangeConfiguration3       = 'chC3',
        kWhatFinishDisconnect2          = 'fin2',
        kWhatStreamSwitching            = 'stsw',
        kWhatRefreshMPD                 = 'rmpd',
    };

    struct BandwidthItem {
        uint32_t mIndex;
        unsigned long mBandwidth;
    };

    struct FetcherInfo {
        bool mIsPrepared;
        uint32_t mBwIndex;
        uint32_t mFetcherStreamMask;
        MPDPlaylists::MPDURLInfo mUrlInfo;
        sp<DashFileFetcher> mFetcher;
    };

    sp<AMessage> mNotify;
    uint32_t mFlags;
    bool mUIDValid;
    uid_t mUID;

    bool mInPreparationPhase;

    sp<HTTPBase> mHTTPDataSource;
    KeyedVector<String8, String8> mExtraHeaders;

    AString mMPDURL;

    Vector<BandwidthItem> mBandwidthItems;

    MPDPlaylists* mMPDPlaylist;

    KeyedVector<AString, FetcherInfo> mFetcherInfos;
    MPDPlaylists::MPDURLInfo mAudioURIInfo, mVideoURIInfo, mSubtitleURIInfo;
    uint32_t mStreamMask;

    KeyedVector<StreamType, sp<AnotherPacketSource> > mPacketSources;

    int64_t mLastDequeuedTimeUs;
    int64_t mRealTimeBaseUs;
    bool mReconfigurationInProgress;
    uint32_t mDisconnectReplyID;   
    int32_t mStreamSwitchGeneration;
    int32_t mPrevBandwidthIndex;
    
    bool mSeeking;
    int64_t mseekTimeUsAtCfg;
    StreamState mAudioStreamState,mVideoStreamState;

    bool mIsLive;
    MPDPlaylists::MPDMediaContainer mMPDMediaContainer; 
    MPDPlaylists::MPDDRMType mMPDDrmType;
    int64_t mDuration;

    //-->for live stream
    int64_t mLastGetMPDTimeUs;
    int64_t mNextMPDDlTimeUs;
    int32_t mRefreshMPDGeneration;
    bool mConnected;
    std::string mPrevAudioInitURL;
    std::string mPrevVideoInitURL;
    std::string mPrevSubtitleInitURL;
    void postRefreshMPD(int64_t delayUs = 0);
    void cancelRefreshMPD();
    bool timeToRefreshMPD(int64_t nowUs) const;
    void onRefreshMPD();
    //<--for live stream
    
    sp<DashFileFetcher> addFetcher(std::string initUri,std::string mediaUri,uint32_t fetcherBwIndex,StreamType streamtype);
    void onConnect(const sp<AMessage> &msg);
    status_t onSeek(const sp<AMessage> &msg);
    void onFinishDisconnect2();

    MPDPlaylists* fetchPlaylist(const char *url);
    status_t fetchFile(
            const char *url, sp<ABuffer> *out,
            int64_t range_offset = 0, int64_t range_length = -1);
    
    bool needToSwitchBandwidth(const sp<AMessage> &fetchNotify,int32_t segMediatimeUs/*moof seq No. OR ts seq No. timeUs*/);
    void changeConfiguration_l(AString url, int32_t segStarttimeUs, int32_t segMediatimeUs/*moof seq No. OR ts seq No. timeUs*/, size_t bandwidthIndex,uint32_t fetcherStreamMask);
    void cancelStreamSwitching();
    int64_t getVideoBufferedDuration();
    uint32_t getBandwidthIndex();

    void changeConfiguration(int64_t timeUs, size_t bandwidthIndex);

    void onChangeConfiguration(const sp<AMessage> &msg);
    void onChangeConfiguration2(const sp<AMessage> &msg);
    void onChangeConfiguration3(const sp<AMessage> &msg);

    void finishDisconnect();
    void finishDisconnect_l();
    void connect_l();

    void postError(status_t err);
    void postPrepared(status_t err);

    DISALLOW_EVIL_CONSTRUCTORS(DASHSession);
};

}  // namespace android

#endif  // DASH_SESSION_H_
