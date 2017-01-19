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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaReceiver"
#include <utils/Log.h>

#include "MediaReceiver.h"

#include "AnotherPacketSource.h"
#include "rtp/RTPReceiver.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ANetworkSession.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

#ifdef MTK_WFD_SINK_SUPPORT
// mtk80902: dump raw data
#include <cutils/properties.h>
#endif
#include "DataPathTrace.h"

///Trace debug
#ifdef MTB_SUPPORT
#define ATRACE_TAG ATRACE_TAG_MTK_WFD
#include <utils/Trace.h>
#endif

namespace android {

MediaReceiver::MediaReceiver(
        const sp<ANetworkSession> &netSession,
        const sp<AMessage> &notify)
    : mNetSession(netSession),
      mNotify(notify),
      mMode(MODE_UNDEFINED),
      mGeneration(0),
      mInitStatus(OK),
      mInitDoneCount(0),
      mNextPrintTimeUs(-1),
      mLastAUTimeUs(-1){
   	  
#ifdef MTK_WFD_SINK_SUPPORT
	char val[PROPERTY_VALUE_MAX] = {0};

       mLatencyRule = BaseOnMbit;
	   
	if (property_get("media.wfd.latency", val, NULL)) 
	{		
		if ( !strcmp("0", val)){
           	    mLatencyRule = BaseOnMbit;
              }else if (!strcmp("1", val)){		
                  mLatencyRule = BaseOnRtpTimestamp;
		}else if(!strcmp("2", val)){
		    mLatencyRule = BaseOnRtpSeqNo;
		}
	}
#endif


}

MediaReceiver::~MediaReceiver() {
}

ssize_t MediaReceiver::addTrack(
        RTPReceiver::TransportMode rtpMode,
        RTPReceiver::TransportMode rtcpMode,
        int32_t *localRTPPort) {
    if (mMode != MODE_UNDEFINED) {
        return INVALID_OPERATION;
    }

    size_t trackIndex = mTrackInfos.size();

    TrackInfo info;

    sp<AMessage> notify = new AMessage(kWhatReceiverNotify, id());
    notify->setInt32("generation", mGeneration);
    notify->setSize("trackIndex", trackIndex);

    info.mReceiver = new RTPReceiver(mNetSession, notify);
    looper()->registerHandler(info.mReceiver);

    info.mReceiver->registerPacketType(
            33, RTPReceiver::PACKETIZATION_TRANSPORT_STREAM);

    info.mReceiver->registerPacketType(
            96, RTPReceiver::PACKETIZATION_AAC);

    info.mReceiver->registerPacketType(
            97, RTPReceiver::PACKETIZATION_H264);

    status_t err = info.mReceiver->initAsync(
            rtpMode,
            rtcpMode,
            localRTPPort);

    if (err != OK) {
        looper()->unregisterHandler(info.mReceiver->id());
        info.mReceiver.clear();

        return err;
    }

    mTrackInfos.push_back(info);

    return trackIndex;
}

status_t MediaReceiver::connectTrack(
        size_t trackIndex,
        const char *remoteHost,
        int32_t remoteRTPPort,
        int32_t remoteRTCPPort) {
    if (trackIndex >= mTrackInfos.size()) {
        return -ERANGE;
    }

    TrackInfo *info = &mTrackInfos.editItemAt(trackIndex);
    return info->mReceiver->connect(remoteHost, remoteRTPPort, remoteRTCPPort);
}

status_t MediaReceiver::initAsync(Mode mode) {
    if ((mode == MODE_TRANSPORT_STREAM || mode == MODE_TRANSPORT_STREAM_RAW)
            && mTrackInfos.size() > 1) {
        return INVALID_OPERATION;
    }

    sp<AMessage> msg = new AMessage(kWhatInit, id());
    msg->setInt32("mode", mode);
    msg->post();

    return OK;
}

void MediaReceiver::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatInit:
        {
            int32_t mode;
            CHECK(msg->findInt32("mode", &mode));

            CHECK_EQ(mMode, MODE_UNDEFINED);
            mMode = (Mode)mode;

            if (mInitStatus != OK || mInitDoneCount == mTrackInfos.size()) {
                notifyInitDone(mInitStatus);
            }

#ifdef MTK_WFD_SINK_SUPPORT
            mTSParser = new ATSParser(ATSParser::TS_TIMESTAMPS_ARE_ABSOLUTE);
#else
            mTSParser = new ATSParser(
                    ATSParser::ALIGNED_VIDEO_DATA
                        | ATSParser::TS_TIMESTAMPS_ARE_ABSOLUTE);
#endif

            mFormatKnownMask = 0;
#ifdef MTK_WFD_SINK_SUPPORT 
            // mtk80902: dump raw data
            mDumpFile = NULL;
            char value[PROPERTY_VALUE_MAX];
            if (property_get("wfd.dumpSinkTS", value, NULL)
                    && (!strcmp(value, "1") || !strcasecmp(value, "true"))) {
                ALOGI("open log raw data");
                mDumpFile = fopen("/sdcard/wfdsink_tsdump.ts", "wb");
            }
            ALOGD("MediaReceiver init! dump TS? %s", mDumpFile == NULL?"No":"Yes");
#endif
            break;
        }

        case kWhatReceiverNotify:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));
            if (generation != mGeneration) {
                break;
            }

            onReceiverNotify(msg);
            break;
        }

        default:
            TRESPASS();
    }
}

void MediaReceiver::onReceiverNotify(const sp<AMessage> &msg) {
    int32_t what;
    CHECK(msg->findInt32("what", &what));
	
	///Trace debug
	#ifdef MTB_SUPPORT	
	ATRACE_CALL();
	#endif

    switch (what) {
        case RTPReceiver::kWhatInitDone:
        {
            ++mInitDoneCount;

            int32_t err;
            CHECK(msg->findInt32("err", &err));

            if (err != OK) {
                mInitStatus = err;
                ++mGeneration;
            }

            if (mMode != MODE_UNDEFINED) {
                if (mInitStatus != OK || mInitDoneCount == mTrackInfos.size()) {
                    notifyInitDone(mInitStatus);
                }
            }
            break;
        }

        case RTPReceiver::kWhatError:
        {
            int32_t err;
            CHECK(msg->findInt32("err", &err));

            notifyError(err);
            break;
        }

        case RTPReceiver::kWhatAccessUnit:
        {
            size_t trackIndex;
            CHECK(msg->findSize("trackIndex", &trackIndex));

            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit", &accessUnit));
			///M: Latency improve issue, print log			
			//int32_t printTime;			
			//CHECK(accessUnit->meta()->findInt32("printTime",&printTime));
			int64_t timeUs;
			CHECK(accessUnit->meta()->findInt64("timeUs",&timeUs));	
			int32_t Mbit;
			CHECK(accessUnit->meta()->findInt32("Mbit",&Mbit));	
			int32_t seqNo;
			CHECK(accessUnit->meta()->findInt32("seqNo", &seqNo));

            int32_t followsDiscontinuity;
            if (!msg->findInt32(
                        "followsDiscontinuity", &followsDiscontinuity)) {
                followsDiscontinuity = 0;
            }

            if (mMode == MODE_TRANSPORT_STREAM) {
                if (followsDiscontinuity) {
                    mTSParser->signalDiscontinuity(
                            ATSParser::DISCONTINUITY_TIME, NULL /* extra */);
                }

#ifdef MTK_WFD_SINK_SUPPORT 
                // mtk80902: dump raw data 
                if (mDumpFile != NULL) { 
                    fwrite(accessUnit->data(), 1, accessUnit->size(), mDumpFile); 
                }
#endif


#ifdef MTK_WFD_SINK_SUPPORT

	sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
	int64_t now = ALooper::GetNowUs();
	   
    
      if(BaseOnMbit == mLatencyRule){
	  	
          if(1 == Mbit)
          {		
          	debugInfo->addTimeInfoByKey(true, timeUs, "atsparserin", now/1000);
          }
      }else if(BaseOnRtpTimestamp == mLatencyRule){
      
          static int64_t PretimeUs = -1; 
	   static int64_t PreNowTime = -1;
	   if((-1 != PretimeUs) && (PretimeUs != timeUs)){
	       debugInfo->addTimeInfoByKey(true, PretimeUs, "atsparserin", (now - PreNowTime)/1000);
	   }

	   PretimeUs = timeUs;
	   PreNowTime = now;
	   
      
      }else if(BaseOnRtpSeqNo == mLatencyRule){
      
           if(0 == seqNo % 10){
               debugInfo->addTimeInfoByKey(true, (int64_t)seqNo, "atsparserin", now/1000);
           }   
      }
#endif	
                for (size_t offset = 0;
                        offset < accessUnit->size(); offset += 188) {
                    status_t err = mTSParser->feedTSPacket(
                             accessUnit->data() + offset, 188);

                    if (err != OK) {
                        notifyError(err);
                        break;
                    }
                }
						
                drainPackets(0 /* trackIndex */, ATSParser::VIDEO);
                drainPackets(1 /* trackIndex */, ATSParser::AUDIO);
			#if 0	
				///M: Add Latency print log{
				if(Mbit == 1)
				{
					sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
					int64_t now = ALooper::GetNowUs();					
					debugInfo->addTimeInfoByKey(true, timeUs, "mediarecvout", now/1000);
				}
				///@}	
			#endif
				
            } else {
                postAccessUnit(trackIndex, accessUnit, NULL);
            }
            break;
        }

        case RTPReceiver::kWhatPacketLost:
        {
            notifyPacketLost();
            break;
        }

        default:
            TRESPASS();
    }
}

void MediaReceiver::drainPackets(
        size_t trackIndex, ATSParser::SourceType type) {
    sp<AnotherPacketSource> source =
        static_cast<AnotherPacketSource *>(
                mTSParser->getSource(type).get());

    if (source == NULL) {
        return;
    }
	///Trace debug
	#ifdef MTB_SUPPORT	
	ATRACE_CALL();
	#endif
    sp<AMessage> format;
    if (!(mFormatKnownMask & (1ul << trackIndex))) {
        sp<MetaData> meta = source->getFormat();
        CHECK(meta != NULL);

        CHECK_EQ((status_t)OK, convertMetaDataToMessage(meta, &format));

        mFormatKnownMask |= 1ul << trackIndex;
    }

    status_t finalResult;
    while (source->hasBufferAvailable(&finalResult)) {
        sp<ABuffer> accessUnit;
        status_t err = source->dequeueAccessUnit(&accessUnit);
        if (err == OK) {

            postAccessUnit(trackIndex, accessUnit, format);		

		
            format.clear();
        } else if (err != INFO_DISCONTINUITY) {
            notifyError(err);
        }
    }

    if (finalResult != OK) {
        notifyError(finalResult);
    }
}

void MediaReceiver::notifyInitDone(status_t err) {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatInitDone);
    notify->setInt32("err", err);
    notify->post();
}

void MediaReceiver::notifyError(status_t err) {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();
}

void MediaReceiver::notifyPacketLost() {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatPacketLost);
    notify->post();
}

void MediaReceiver::postAccessUnit(
        size_t trackIndex,
        const sp<ABuffer> &accessUnit,
        const sp<AMessage> &format) {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatAccessUnit);
    notify->setSize("trackIndex", trackIndex);
    notify->setBuffer("accessUnit", accessUnit);

    if (format != NULL) {
        notify->setMessage("format", format);
    }
	///M: Add Latency print log{
	int64_t timeUs;
	CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));			
	
	sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
	int64_t now = ALooper::GetNowUs();
	//ALOGI("apacksourceout timeUs is %lldms",timeUs/1000);
	debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "apacksourceout", now/1000);
	///@}

    notify->post();
}

status_t MediaReceiver::informSender(
        size_t trackIndex, const sp<AMessage> &params) {
    if (trackIndex >= mTrackInfos.size()) {
        return -ERANGE;
    }

    TrackInfo *info = &mTrackInfos.editItemAt(trackIndex);
    return info->mReceiver->informSender(params);
}
///M : Add for RTP data contol{
status_t MediaReceiver::mtkRTPPause(size_t trackIndex)
{
	if (trackIndex >= mTrackInfos.size()) {
        return -ERANGE;
    }

    TrackInfo *info = &mTrackInfos.editItemAt(trackIndex);
	return info->mReceiver->mtkRTPPause();
}
status_t MediaReceiver::mtkRTPResume(size_t trackIndex)
{
    if (trackIndex >= mTrackInfos.size()) {
        return -ERANGE;
    }

    TrackInfo *info = &mTrackInfos.editItemAt(trackIndex);
	return info->mReceiver->mtkRTPResume();

}

int64_t MediaReceiver::getRTPRecvNum(size_t trackIndex){
    if (trackIndex >= mTrackInfos.size()) {
        return -ERANGE;
    }

    TrackInfo *info = &mTrackInfos.editItemAt(trackIndex);
	return info->mReceiver->getRTPRecvNum();

}
status_t MediaReceiver::resetRTPRecvNum(size_t trackIndex){

	if (trackIndex >= mTrackInfos.size()) {
		return -ERANGE;
	}

	TrackInfo *info = &mTrackInfos.editItemAt(trackIndex);
	return info->mReceiver->resetRTPRecvNum();
}

///@}

}  // namespace android


