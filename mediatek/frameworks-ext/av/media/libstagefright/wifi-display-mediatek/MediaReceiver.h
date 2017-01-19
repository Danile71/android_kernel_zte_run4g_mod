/*
 * Copyright 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <media/stagefright/foundation/AHandler.h>

#include "ATSParser.h"
#include "rtp/RTPReceiver.h"

namespace android {

struct ABuffer;
struct ANetworkSession;
struct AMessage;
struct ATSParser;

// This class facilitates receiving of media data for one or more tracks
// over RTP. Either a 1:1 track to RTP channel mapping is used or a single
// RTP channel provides the data for a transport stream that is consequently
// demuxed and its track's data provided to the observer.
struct MediaReceiver : public AHandler {
    enum {
        kWhatInitDone,
        kWhatError,
        kWhatAccessUnit,
        kWhatPacketLost,
    };

    MediaReceiver(
            const sp<ANetworkSession> &netSession,
            const sp<AMessage> &notify);

    ssize_t addTrack(
            RTPReceiver::TransportMode rtpMode,
            RTPReceiver::TransportMode rtcpMode,
            int32_t *localRTPPort);

    status_t connectTrack(
            size_t trackIndex,
            const char *remoteHost,
            int32_t remoteRTPPort,
            int32_t remoteRTCPPort);

    enum Mode {
        MODE_UNDEFINED,
        MODE_TRANSPORT_STREAM,
        MODE_TRANSPORT_STREAM_RAW,
        MODE_ELEMENTARY_STREAMS,
    };
    status_t initAsync(Mode mode);

    status_t informSender(size_t trackIndex, const sp<AMessage> &params);
	///M : Add for RTP data contol{
	status_t mtkRTPPause(size_t trackIndex);
	status_t mtkRTPResume(size_t trackIndex);

	int64_t getRTPRecvNum(size_t trackIndex);
	status_t resetRTPRecvNum(size_t trackIndex); 
	///@}

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~MediaReceiver();

private:
    enum {
        kWhatInit,
        kWhatReceiverNotify,
    };

    struct TrackInfo {
        sp<RTPReceiver> mReceiver;
    };

    sp<ANetworkSession> mNetSession;
    sp<AMessage> mNotify;

    Mode mMode;
    int32_t mGeneration;

    Vector<TrackInfo> mTrackInfos;

    status_t mInitStatus;
    size_t mInitDoneCount;

    sp<ATSParser> mTSParser;
    uint32_t mFormatKnownMask;
#ifdef MTK_WFD_SINK_SUPPORT
	// mtk80902: dump raw data
	FILE* mDumpFile;
       int32_t mLatencyRule;
#endif
	///M : Add for Latency log@{
	static const int64_t kPrintLatenessEveryUs = 10000000ll; //10s
	int64_t mNextPrintTimeUs;
	int64_t mLastAUTimeUs;
	///}

    void onReceiverNotify(const sp<AMessage> &msg);

    void drainPackets(size_t trackIndex, ATSParser::SourceType type);

    void notifyInitDone(status_t err);
    void notifyError(status_t err);
    void notifyPacketLost();

    void postAccessUnit(
            size_t trackIndex,
            const sp<ABuffer> &accessUnit,
            const sp<AMessage> &format);


    DISALLOW_EVIL_CONSTRUCTORS(MediaReceiver);
};

}  // namespace android

