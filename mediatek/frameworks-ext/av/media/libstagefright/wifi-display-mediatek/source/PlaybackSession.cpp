/*
 * Copyright 2012, The Android Open Source Project
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
#define LOG_TAG "PlaybackSession"
#include <utils/Log.h>

#include "PlaybackSession.h"

#include "Converter.h"
#include "MediaPuller.h"
#include "RepeaterSource.h"
#include "include/avc_utils.h"
#include "WifiDisplaySource.h" 

#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <media/IHDCP.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/AudioSource.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/NuMediaExtractor.h>
#include <media/stagefright/SurfaceMediaSource.h>
#include <media/stagefright/Utils.h>

#include <OMX_IVCommon.h>

#ifndef ANDROID_DEFAULT_CODE
#include <cutils/properties.h>
#include <OMX_Video.h>
#include "DataPathTrace.h"
 
#ifdef MTB_SUPPORT
#define ATRACE_TAG ATRACE_TAG_MTK_WFD
#include <utils/Trace.h>
#endif

#ifdef USE_MTK_BWC
#include "bandwidth_control.h"
#endif

#define WFD_LOGI(fmt,arg...) ALOGI(fmt,##arg)

#else
#define WFD_LOGI(fmt,arg...)
#endif

namespace android {

#ifndef ANDROID_DEFAULT_CODE
void calcFrameRate(bool isVideo,sp<ABuffer> accessUnit){
    if(!isVideo){
        return;
    }
    int64_t nowUs;
    static int64_t mStartSysTime = 0;
    static int mCountFrames = 0;
    int mCountFramerate;

    //count framerate.
    if(((mCountFrames % 60) == 0) && (mCountFrames != 0))
    {
    	nowUs = ALooper::GetNowUs();
    	mCountFramerate = (mCountFrames * 1000 * 1000) / (nowUs - mStartSysTime);
    	WFD_LOGI("framerate = %d ", mCountFramerate);
    	mCountFrames = 0;
    	mStartSysTime = ALooper::GetNowUs();
    }
    int32_t dummy;
    if(accessUnit.get() != NULL && !accessUnit->meta()->findInt32("dummy-nal", &dummy)){
    	mCountFrames ++;
    }

}

#endif



struct WifiDisplaySource::PlaybackSession::Track : public AHandler {
    enum {
        kWhatStopped,
    };

    Track(const sp<AMessage> &notify,
          const sp<ALooper> &pullLooper,
          const sp<ALooper> &codecLooper,
          const sp<MediaPuller> &mediaPuller,
          const sp<Converter> &converter);

    Track(const sp<AMessage> &notify, const sp<AMessage> &format);

    void setRepeaterSource(const sp<RepeaterSource> &source);

    sp<AMessage> getFormat();
    bool isAudio() const;

    const sp<Converter> &converter() const;
    const sp<RepeaterSource> &repeaterSource() const;

    ssize_t mediaSenderTrackIndex() const;
    void setMediaSenderTrackIndex(size_t index);

    status_t start();
    void stopAsync();

    void pause();
    void resume();

    void queueAccessUnit(const sp<ABuffer> &accessUnit);
    sp<ABuffer> dequeueAccessUnit();

    bool hasOutputBuffer(int64_t *timeUs) const;
    void queueOutputBuffer(const sp<ABuffer> &accessUnit);
    sp<ABuffer> dequeueOutputBuffer();

#if SUSPEND_VIDEO_IF_IDLE
    bool isSuspended() const;
#endif

    size_t countQueuedOutputBuffers() const {
        return mQueuedOutputBuffers.size();
    }

    void requestIDRFrame();

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~Track();

private:
    enum {
        kWhatMediaPullerStopped,
    };

    sp<AMessage> mNotify;
    sp<ALooper> mPullLooper;
    sp<ALooper> mCodecLooper;
    sp<MediaPuller> mMediaPuller;
    sp<Converter> mConverter;
    sp<AMessage> mFormat;
    bool mStarted;
    ssize_t mMediaSenderTrackIndex;
    bool mIsAudio;
    List<sp<ABuffer> > mQueuedAccessUnits;
    sp<RepeaterSource> mRepeaterSource;
    List<sp<ABuffer> > mQueuedOutputBuffers;
    int64_t mLastOutputBufferQueuedTimeUs;

    static bool IsAudioFormat(const sp<AMessage> &format);

    DISALLOW_EVIL_CONSTRUCTORS(Track);

#ifndef ANDROID_DEFAULT_CODE
public:    
    bool isStopped() const{return !mStarted;}
#endif

    
};

WifiDisplaySource::PlaybackSession::Track::Track(
        const sp<AMessage> &notify,
        const sp<ALooper> &pullLooper,
        const sp<ALooper> &codecLooper,
        const sp<MediaPuller> &mediaPuller,
        const sp<Converter> &converter)
    : mNotify(notify),
      mPullLooper(pullLooper),
      mCodecLooper(codecLooper),
      mMediaPuller(mediaPuller),
      mConverter(converter),
      mStarted(false),
      mIsAudio(IsAudioFormat(mConverter->getOutputFormat())),
      mLastOutputBufferQueuedTimeUs(-1ll) {
}

WifiDisplaySource::PlaybackSession::Track::Track(
        const sp<AMessage> &notify, const sp<AMessage> &format)
    : mNotify(notify),
      mFormat(format),
      mStarted(false),
      mIsAudio(IsAudioFormat(format)),
      mLastOutputBufferQueuedTimeUs(-1ll) {
}

WifiDisplaySource::PlaybackSession::Track::~Track() {
 //   CHECK(!mStarted);
   WFD_LOGI("delete %s track while it is %s now",
        mIsAudio?"audio":"video",
        mStarted?"start":"stop");
}

// static
bool WifiDisplaySource::PlaybackSession::Track::IsAudioFormat(
        const sp<AMessage> &format) {
    AString mime;
    CHECK(format->findString("mime", &mime));

    return !strncasecmp(mime.c_str(), "audio/", 6);
}

sp<AMessage> WifiDisplaySource::PlaybackSession::Track::getFormat() {
    return mFormat != NULL ? mFormat : mConverter->getOutputFormat();
}

bool WifiDisplaySource::PlaybackSession::Track::isAudio() const {
    return mIsAudio;
}

const sp<Converter> &WifiDisplaySource::PlaybackSession::Track::converter() const {
    return mConverter;
}

const sp<RepeaterSource> &
WifiDisplaySource::PlaybackSession::Track::repeaterSource() const {
    return mRepeaterSource;
}

ssize_t WifiDisplaySource::PlaybackSession::Track::mediaSenderTrackIndex() const {
    CHECK_GE(mMediaSenderTrackIndex, 0);
    return mMediaSenderTrackIndex;
}

void WifiDisplaySource::PlaybackSession::Track::setMediaSenderTrackIndex(
        size_t index) {
    mMediaSenderTrackIndex = index;
}

status_t WifiDisplaySource::PlaybackSession::Track::start() {
    WFD_LOGI("Track::start isAudio=%d", mIsAudio);

    CHECK(!mStarted);

    status_t err = OK;

    if (mMediaPuller != NULL) {
        err = mMediaPuller->start();
    }

    if (err == OK) {
        mStarted = true;
    }

    return err;
}

void WifiDisplaySource::PlaybackSession::Track::stopAsync() {
    ALOGV("Track::stopAsync isAudio=%d", mIsAudio);

    if (mConverter != NULL) {
        mConverter->shutdownAsync();
    }

    sp<AMessage> msg = new AMessage(kWhatMediaPullerStopped, id());

    if (mStarted && mMediaPuller != NULL) {
        if (mRepeaterSource != NULL) {
            // Let's unblock MediaPuller's MediaSource::read().
            mRepeaterSource->wakeUp();
        }

        mMediaPuller->stopAsync(msg);
    } else {
        mStarted = false;
        msg->post();
    }
}

void WifiDisplaySource::PlaybackSession::Track::pause() {
    mMediaPuller->pause();
}

void WifiDisplaySource::PlaybackSession::Track::resume() {
    mMediaPuller->resume();
}

void WifiDisplaySource::PlaybackSession::Track::onMessageReceived(
        const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatMediaPullerStopped:
        {
            mConverter.clear();

            mStarted = false;

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatStopped);
            notify->post();

            ALOGI("kWhatStopped %s posted", mIsAudio ? "audio" : "video");
            break;
        }

        default:
            TRESPASS();
    }
}

void WifiDisplaySource::PlaybackSession::Track::queueAccessUnit(
        const sp<ABuffer> &accessUnit) {
    mQueuedAccessUnits.push_back(accessUnit);
}

sp<ABuffer> WifiDisplaySource::PlaybackSession::Track::dequeueAccessUnit() {
    if (mQueuedAccessUnits.empty()) {
        return NULL;
    }

    sp<ABuffer> accessUnit = *mQueuedAccessUnits.begin();
    CHECK(accessUnit != NULL);

    mQueuedAccessUnits.erase(mQueuedAccessUnits.begin());

    return accessUnit;
}

void WifiDisplaySource::PlaybackSession::Track::setRepeaterSource(
        const sp<RepeaterSource> &source) {
    mRepeaterSource = source;
}

void WifiDisplaySource::PlaybackSession::Track::requestIDRFrame() {

#ifndef ANDROID_DEFAULT_CODE
	 if( !mStarted)  {
	 	WFD_LOGI("error request in stop state");
	 	return; 
	 }
#endif


    if (mIsAudio) {
        return;
    }

    if (mRepeaterSource != NULL) {
        mRepeaterSource->wakeUp();
    }

    mConverter->requestIDRFrame();
}

bool WifiDisplaySource::PlaybackSession::Track::hasOutputBuffer(
        int64_t *timeUs) const {
    *timeUs = 0ll;

    if (mQueuedOutputBuffers.empty()) {
        return false;
    }

    const sp<ABuffer> &outputBuffer = *mQueuedOutputBuffers.begin();

    CHECK(outputBuffer->meta()->findInt64("timeUs", timeUs));

    return true;
}

void WifiDisplaySource::PlaybackSession::Track::queueOutputBuffer(
        const sp<ABuffer> &accessUnit) {
    mQueuedOutputBuffers.push_back(accessUnit);
    mLastOutputBufferQueuedTimeUs = ALooper::GetNowUs();
}

sp<ABuffer> WifiDisplaySource::PlaybackSession::Track::dequeueOutputBuffer() {
    CHECK(!mQueuedOutputBuffers.empty());

    sp<ABuffer> outputBuffer = *mQueuedOutputBuffers.begin();
    mQueuedOutputBuffers.erase(mQueuedOutputBuffers.begin());

    return outputBuffer;
}

#if SUSPEND_VIDEO_IF_IDLE
bool WifiDisplaySource::PlaybackSession::Track::isSuspended() const {
    if (!mQueuedOutputBuffers.empty()) {
        return false;
    }

    if (mLastOutputBufferQueuedTimeUs < 0ll) {
        // We've never seen an output buffer queued, but tracks start
        // out live, not suspended.
        return false;
    }

    // If we've not seen new output data for 60ms or more, we consider
    // this track suspended for the time being.
    return (ALooper::GetNowUs() - mLastOutputBufferQueuedTimeUs) > 60000ll;
}
#endif


////////////////////////////////////////////////////////////////////////////////
#ifndef ANDROID_DEFAULT_CODE
void WifiDisplaySource::PlaybackSession::init_pre(bool* enableAudio,bool* enableVideo){
       
	mTestFakeVideoPath= 0;
	mUseSliceMode  = 0;
    ALOGI("[config]mVideoResolutionType %d mVideoResolutionIndex %d",
                mVideoResolutionType,mVideoResolutionIndex);

	char value[PROPERTY_VALUE_MAX];
	//
	if(property_get("wfd.fake.path", value, NULL) ){
		mTestFakeVideoPath = atoi(value);
		ALOGI("fake video is enable=%d",mTestFakeVideoPath);
		if(mTestFakeVideoPath  == 1){  //test the sent ts file directly
			mMediaPath.setTo("/sdcard/test.ts");

			*enableAudio = true;
			*enableVideo = true;
		}
		if(mTestFakeVideoPath  == 2){//test the sent any file with h264 and aac  throught 2 sender
			//mMediaPath.setTo("/sdcard/test.ts");
			//enableAudio = true;
			//enableVideo = true;
		}
	}


#ifdef USE_MTK_BWC
   BWC bwc;
   bwc.Profile_Change(BWCPT_VIDEO_WIFI_DISPLAY, true);
   ALOGI("enter WFD BWCPT_VIDEO_WIFI_DISPLAY");
#endif 

}
void WifiDisplaySource::PlaybackSession::delete_pro(){

  ALOGI("delete_pro");
  bool shouldDeleteNow= true;
   for (size_t i = 0; i < mTracks.size(); ++i) {
        if (!mTracks.valueAt(i)->isStopped()) {
              shouldDeleteNow = false;
	      ALOGI("[error flow]TrackIndex % is not stopped now",i);
        }
  }
  if(shouldDeleteNow){
	 deleteWfdDebugInfo();
  }
#ifdef USE_MTK_BWC
	BWC bwc;
	bwc.Profile_Change(BWCPT_VIDEO_WIFI_DISPLAY, false);
    ALOGI("leave WFD BWCPT_VIDEO_WIFI_DISPLAY  !");
#endif  
}

void WifiDisplaySource::PlaybackSession::addSource_video_ext
             (sp<AMessage> format,int32_t profileIdc,int32_t levelIdc){

#ifdef MTK_VIDEO_HEVC_SUPPORT
	char value[PROPERTY_VALUE_MAX];
	 bool isHEVC = false;
	if(property_get("wfd.video.hevc", value, NULL) ){
		isHEVC = atoi(value);
	}

	bool choseHEVC=false;
	choseHEVC =  (mVideoResolutionType ==  VideoFormats::RESOLUTION_CEA ) 
			    && (mVideoResolutionIndex  == 17);

	
	if(isHEVC || choseHEVC){
		ALOGI("reconfig to hevc");
		format->setString("mime", MEDIA_MIMETYPE_VIDEO_HEVC);
	}
#endif


			 
     ALOGI("[config]format is '%s'", format->debugString(0).c_str()); 
        
     if( profileIdc ==  66 ){
	 	format->setInt32("profile", OMX_VIDEO_AVCProfileBaseline);
	 }else if(profileIdc == 100 ){
		 format->setInt32("profile", OMX_VIDEO_AVCProfileHigh);
	 }
	 if(levelIdc == 31){
		format->setInt32("level", OMX_VIDEO_AVCLevel31);
	 }else if(levelIdc == 32){
		format->setInt32("level", OMX_VIDEO_AVCLevel32);
	 }else if(levelIdc == 40){
		format->setInt32("level", OMX_VIDEO_AVCLevel4);
	 }else if(levelIdc == 41){
		format->setInt32("level", OMX_VIDEO_AVCLevel41);
	 }else if(levelIdc == 42){
		format->setInt32("level", OMX_VIDEO_AVCLevel42);
     }
    size_t width, height, framesPerSecond;
    bool interlaced;
    CHECK(VideoFormats::GetConfiguration(
                mVideoResolutionType,
                mVideoResolutionIndex,
                &width,
                &height,
                &framesPerSecond,
                &interlaced));

 	 format->setInt32("slice-mode",mUseSliceMode);
	 format->setInt32("frame-rate",framesPerSecond);
}


 status_t WifiDisplaySource::PlaybackSession::setWfdLevel(int32_t level){
 	if(mVideoTrackIndex>=0){
		 ssize_t index = mTracks.indexOfKey(mVideoTrackIndex);
	        CHECK_GE(index, 0);

                const sp<Converter> &converter =
                    mTracks.valueAt(index)->converter();
		   status_t err = converter->setWfdLevel(level);
		   return err;

	}else{
	       ALOGE("video track is not ready, set level is error now");
		return ERROR;

	}

 }
int  WifiDisplaySource::PlaybackSession::getWfdParam(int paramType){
	if(mVideoTrackIndex>=0){
		 ssize_t index = mTracks.indexOfKey(mVideoTrackIndex);
	        CHECK_GE(index, 0);

                const sp<Converter> &converter =
                    mTracks.valueAt(index)->converter();
		   int paramValue = converter->getWfdParam(paramType);
		   return paramValue;

	}else{
	       ALOGE("video track is not ready, set level is error now");
		return -1;

	}


}

void WifiDisplaySource::PlaybackSession::setSliceMode(int32_t useSliceMode){

	   mUseSliceMode = useSliceMode;
          ALOGI("mUseSliceMode =%d",mUseSliceMode);
}

status_t WifiDisplaySource::PlaybackSession::forceBlackScreen(bool blackNow) {

	   if(mVideoTrackIndex>=0){
		 ssize_t index = mTracks.indexOfKey(mVideoTrackIndex);
	        CHECK_GE(index, 0);

                const sp<Converter> &converter =
                    mTracks.valueAt(index)->converter();
		   converter->forceBlackScreen(blackNow);
		   return OK;

	}else{
	       ALOGE("video track is not ready,forceBlackScreen is error now");
		return ERROR;

	}

}

#endif









WifiDisplaySource::PlaybackSession::PlaybackSession(
        const sp<ANetworkSession> &netSession,
        const sp<AMessage> &notify,
        const in_addr &interfaceAddr,
        const sp<IHDCP> &hdcp,
        const char *path)
    : mNetSession(netSession),
      mNotify(notify),
      mInterfaceAddr(interfaceAddr),
      mHDCP(hdcp),
      mLocalRTPPort(-1),
      mWeAreDead(false),
      mPaused(false),
      mLastLifesignUs(),
      mVideoTrackIndex(-1),
      mPrevTimeUs(-1ll),
      mPullExtractorPending(false),
      mPullExtractorGeneration(0),
      mFirstSampleTimeRealUs(-1ll),
      mFirstSampleTimeUs(-1ll) {
    if (path != NULL) {
        mMediaPath.setTo(path);
    }
}

status_t WifiDisplaySource::PlaybackSession::init(
        const char *clientIP,
        int32_t clientRtp,
        RTPSender::TransportMode rtpMode,
        int32_t clientRtcp,
        RTPSender::TransportMode rtcpMode,
        bool enableAudio,
        bool usePCMAudio,
        bool enableVideo,
        VideoFormats::ResolutionType videoResolutionType,
        size_t videoResolutionIndex,
        VideoFormats::ProfileType videoProfileType,
        VideoFormats::LevelType videoLevelType) {

#ifndef ANDROID_DEFAULT_CODE
        mVideoResolutionType  =videoResolutionType ;
        mVideoResolutionIndex  = videoResolutionIndex;
        init_pre(&enableAudio,&enableVideo);
#endif

    sp<AMessage> notify = new AMessage(kWhatMediaSenderNotify, id());
    mMediaSender = new MediaSender(mNetSession, notify);
    looper()->registerHandler(mMediaSender);

    mMediaSender->setHDCP(mHDCP);

    status_t err = setupPacketizer(
            enableAudio,
            usePCMAudio,
            enableVideo,
            videoResolutionType,
            videoResolutionIndex,
            videoProfileType,
            videoLevelType);

    if (err == OK) {
// if mTestFakeVideoPath ==2, should setup a  rtpSender for each track,now this path can not be used"MODE_ELEMENTARY_STREAMS"
// mTestFakeVideoPath == 1, send ts directly is also use the MODE_TRANSPORT_STREAM
        err = mMediaSender->initAsync(
                -1 /* trackIndex */,
                clientIP,
                clientRtp,
                rtpMode,
                clientRtcp,
                rtcpMode,
                &mLocalRTPPort);
    }

    if (err != OK) {
        mLocalRTPPort = -1;

        looper()->unregisterHandler(mMediaSender->id());
        mMediaSender.clear();

        return err;
    }

    updateLiveness();
    WFD_LOGI("[profile]init done");
    return OK;
}

WifiDisplaySource::PlaybackSession::~PlaybackSession() {
#ifndef ANDROID_DEFAULT_CODE
	delete_pro();
#endif	
}

int32_t WifiDisplaySource::PlaybackSession::getRTPPort() const {
    return mLocalRTPPort;
}

int64_t WifiDisplaySource::PlaybackSession::getLastLifesignUs() const {
    return mLastLifesignUs;
}

void WifiDisplaySource::PlaybackSession::updateLiveness() {
    mLastLifesignUs = ALooper::GetNowUs();
}

status_t WifiDisplaySource::PlaybackSession::play() {
    WFD_LOGI("[profile]play +++");
    updateLiveness();

    (new AMessage(kWhatResume, id()))->post();

    return OK;
}

status_t WifiDisplaySource::PlaybackSession::onMediaSenderInitialized() {
    for (size_t i = 0; i < mTracks.size(); ++i) {
        CHECK_EQ((status_t)OK, mTracks.editValueAt(i)->start());
    }

    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatSessionEstablished);
    notify->post();

    return OK;
}

status_t WifiDisplaySource::PlaybackSession::pause() {
    WFD_LOGI("[profile]pause +++");
    updateLiveness();

    (new AMessage(kWhatPause, id()))->post();

    return OK;
}

void WifiDisplaySource::PlaybackSession::destroyAsync() {
    ALOGI("destroyAsync");

    for (size_t i = 0; i < mTracks.size(); ++i) {
        mTracks.valueAt(i)->stopAsync();
    }
}

void WifiDisplaySource::PlaybackSession::onMessageReceived(
        const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatConverterNotify:
        {
            if (mWeAreDead) {
                WFD_LOGI("dropping msg '%s' because we're dead",
                      msg->debugString().c_str());

                break;
            }

            int32_t what;
            CHECK(msg->findInt32("what", &what));

            size_t trackIndex;
            CHECK(msg->findSize("trackIndex", &trackIndex));

            if (what == Converter::kWhatAccessUnit) {
                sp<ABuffer> accessUnit;
                CHECK(msg->findBuffer("accessUnit", &accessUnit));

                const sp<Track> &track = mTracks.valueFor(trackIndex);

                status_t err = mMediaSender->queueAccessUnit(
                        track->mediaSenderTrackIndex(),
                        accessUnit);

                if (err != OK) {
		            WFD_LOGI("MediaSender queueAccessUnit signaled error %d ", err);
                    notifySessionDead();
                }
#ifndef ANDROID_DEFAULT_CODE				
		        calcFrameRate((mVideoTrackIndex == (ssize_t)trackIndex),accessUnit);
#endif			                
                break;
            } else if (what == Converter::kWhatEOS) {
                CHECK_EQ(what, Converter::kWhatEOS);

                ALOGI("output EOS on track %d", trackIndex);

                ssize_t index = mTracks.indexOfKey(trackIndex);
                CHECK_GE(index, 0);

                const sp<Converter> &converter =
                    mTracks.valueAt(index)->converter();
                looper()->unregisterHandler(converter->id());

                mTracks.removeItemsAt(index);

                if (mTracks.isEmpty()) {
                    ALOGI("Reached EOS");
                }
            } else if (what != Converter::kWhatShutdownCompleted) {
                CHECK_EQ(what, Converter::kWhatError);

                status_t err;
                CHECK(msg->findInt32("err", &err));

                ALOGE("converter signaled error %d", err);

                notifySessionDead();
            }
            break;
        }

        case kWhatMediaSenderNotify:
        {
            int32_t what;
            CHECK(msg->findInt32("what", &what));

            if (what == MediaSender::kWhatInitDone) {
                status_t err;
                CHECK(msg->findInt32("err", &err));

                if (err == OK) {
                    onMediaSenderInitialized();
                } else {
                    WFD_LOGI("MediaSender kWhatInitDone signaled error %d ", err);
                    notifySessionDead();
                }
            } else if (what == MediaSender::kWhatError) {
            	WFD_LOGI("MediaSender kWhatError " );
                notifySessionDead();
            } else if (what == MediaSender::kWhatNetworkStall) {
                size_t numBytesQueued;
                CHECK(msg->findSize("numBytesQueued", &numBytesQueued));

			 if (mVideoTrackIndex >= 0 && mTracks.indexOfKey(mVideoTrackIndex) >= 0) {
                    const sp<Track> &videoTrack =
                        mTracks.valueFor(mVideoTrackIndex);

                    sp<Converter> converter = videoTrack->converter();
                    if (converter != NULL) {
                        converter->dropAFrame();
                    }
                }
            } else if (what == MediaSender::kWhatInformSender) {
                onSinkFeedback(msg);
            } else {
                TRESPASS();
            }
            break;
        }

        case kWhatTrackNotify:
        {
            int32_t what;
            CHECK(msg->findInt32("what", &what));

            size_t trackIndex;
            CHECK(msg->findSize("trackIndex", &trackIndex));

            if (what == Track::kWhatStopped) {
                ALOGI("Track %d stopped", trackIndex);

                sp<Track> track = mTracks.valueFor(trackIndex);
                looper()->unregisterHandler(track->id());
                mTracks.removeItem(trackIndex);
                track.clear();

                if (!mTracks.isEmpty()) {
                    ALOGI("not all tracks are stopped yet");
                    break;
                }

                looper()->unregisterHandler(mMediaSender->id());
                mMediaSender.clear();

                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what", kWhatSessionDestroyed);
                notify->post();
            }
            break;
        }

        case kWhatPause:
        {
            if (mExtractor != NULL) {
                ++mPullExtractorGeneration;
                mFirstSampleTimeRealUs = -1ll;
                mFirstSampleTimeUs = -1ll;
            }

            if (mPaused) {
                break;
            }

            for (size_t i = 0; i < mTracks.size(); ++i) {
                mTracks.editValueAt(i)->pause();
            }

            mPaused = true;
            break;
        }

        case kWhatResume:
        {
            if (mExtractor != NULL) {
                schedulePullExtractor();
            }

            if (!mPaused) {
                break;
            }

            for (size_t i = 0; i < mTracks.size(); ++i) {
                mTracks.editValueAt(i)->resume();
            }

            mPaused = false;
            break;
        }

        case kWhatPullExtractorSample:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mPullExtractorGeneration) {
                break;
            }

            mPullExtractorPending = false;

            onPullExtractor();
            break;
        }

        default:
            TRESPASS();
    }
}

void WifiDisplaySource::PlaybackSession::onSinkFeedback(const sp<AMessage> &msg) {
    int64_t avgLatencyUs;
    CHECK(msg->findInt64("avgLatencyUs", &avgLatencyUs));

    int64_t maxLatencyUs;
    CHECK(msg->findInt64("maxLatencyUs", &maxLatencyUs));

    ALOGI("sink reports avg. latency of %lld ms (max %lld ms)",
          avgLatencyUs / 1000ll,
          maxLatencyUs / 1000ll);

    if (mVideoTrackIndex >= 0) {
        const sp<Track> &videoTrack = mTracks.valueFor(mVideoTrackIndex);
        sp<Converter> converter = videoTrack->converter();

        if (converter != NULL) {
            int32_t videoBitrate =
                Converter::GetInt32Property("media.wfd.video-bitrate", -1);

            char val[PROPERTY_VALUE_MAX];
            if (videoBitrate < 0
                    && property_get("media.wfd.video-bitrate", val, NULL)
                    && !strcasecmp("adaptive", val)) {
                videoBitrate = converter->getVideoBitrate();

                if (avgLatencyUs > 300000ll) {
                    videoBitrate *= 0.6;
                } else if (avgLatencyUs < 100000ll) {
                    videoBitrate *= 1.1;
                }
            }

            if (videoBitrate > 0) {
                if (videoBitrate < 500000) {
                    videoBitrate = 500000;
                } else if (videoBitrate > 10000000) {
                    videoBitrate = 10000000;
                }

                if (videoBitrate != converter->getVideoBitrate()) {
                    ALOGI("setting video bitrate to %d bps", videoBitrate);

                    converter->setVideoBitrate(videoBitrate);
                }
            }
        }

        sp<RepeaterSource> repeaterSource = videoTrack->repeaterSource();
        if (repeaterSource != NULL) {
            double rateHz =
                Converter::GetInt32Property(
                        "media.wfd.video-framerate", -1);

            char val[PROPERTY_VALUE_MAX];
            if (rateHz < 0.0
                    && property_get("media.wfd.video-framerate", val, NULL)
                    && !strcasecmp("adaptive", val)) {
                 rateHz = repeaterSource->getFrameRate();

                if (avgLatencyUs > 300000ll) {
                    rateHz *= 0.9;
                } else if (avgLatencyUs < 200000ll) {
                    rateHz *= 1.1;
                }
            }

            if (rateHz > 0) {
                if (rateHz < 5.0) {
                    rateHz = 5.0;
                } else if (rateHz > 30.0) {
                    rateHz = 30.0;
                }

                if (rateHz != repeaterSource->getFrameRate()) {
                    ALOGI("setting frame rate to %.2f Hz", rateHz);

                    repeaterSource->setFrameRate(rateHz);
                }
            }
        }
    }
}

status_t WifiDisplaySource::PlaybackSession::setupMediaPacketizer(
        bool enableAudio, bool enableVideo) {
    DataSource::RegisterDefaultSniffers();

    mExtractor = new NuMediaExtractor;

    status_t err = mExtractor->setDataSource(mMediaPath.c_str());

    if (err != OK) {
        return err;
    }

    size_t n = mExtractor->countTracks();
    bool haveAudio = false;
    bool haveVideo = false;
    for (size_t i = 0; i < n; ++i) {
        sp<AMessage> format;
        err = mExtractor->getTrackFormat(i, &format);

        if (err != OK) {
            continue;
        }

        AString mime;
        CHECK(format->findString("mime", &mime));

        bool isAudio = !strncasecmp(mime.c_str(), "audio/", 6);
        bool isVideo = !strncasecmp(mime.c_str(), "video/", 6);

        if (isAudio && enableAudio && !haveAudio) {
            haveAudio = true;
        } else if (isVideo && enableVideo && !haveVideo) {
            haveVideo = true;
        } else {
            continue;
        }

        err = mExtractor->selectTrack(i);

        size_t trackIndex = mTracks.size();

        sp<AMessage> notify = new AMessage(kWhatTrackNotify, id());
        notify->setSize("trackIndex", trackIndex);

        sp<Track> track = new Track(notify, format);
        looper()->registerHandler(track);

        mTracks.add(trackIndex, track);

        mExtractorTrackToInternalTrack.add(i, trackIndex);

        if (isVideo) {
            mVideoTrackIndex = trackIndex;
        }

        uint32_t flags = MediaSender::FLAG_MANUALLY_PREPEND_SPS_PPS;

        ssize_t mediaSenderTrackIndex =
            mMediaSender->addTrack(format, flags);
        CHECK_GE(mediaSenderTrackIndex, 0);

        track->setMediaSenderTrackIndex(mediaSenderTrackIndex);

        if ((haveAudio || !enableAudio) && (haveVideo || !enableVideo)) {
            break;
        }
    }

    return OK;
}

void WifiDisplaySource::PlaybackSession::schedulePullExtractor() {
    if (mPullExtractorPending) {
        return;
    }

    int64_t sampleTimeUs;
    status_t err = mExtractor->getSampleTime(&sampleTimeUs);

    int64_t nowUs = ALooper::GetNowUs();

    if (mFirstSampleTimeRealUs < 0ll) {
        mFirstSampleTimeRealUs = nowUs;
        mFirstSampleTimeUs = sampleTimeUs;
    }

    int64_t whenUs = sampleTimeUs - mFirstSampleTimeUs + mFirstSampleTimeRealUs;

    sp<AMessage> msg = new AMessage(kWhatPullExtractorSample, id());
    msg->setInt32("generation", mPullExtractorGeneration);
    msg->post(whenUs - nowUs);

    mPullExtractorPending = true;
}

void WifiDisplaySource::PlaybackSession::onPullExtractor() {
    sp<ABuffer> accessUnit = new ABuffer(1024 * 1024);
    status_t err = mExtractor->readSampleData(accessUnit);
    if (err != OK) {
        // EOS.
        return;
    }

    int64_t timeUs;
    CHECK_EQ((status_t)OK, mExtractor->getSampleTime(&timeUs));

    accessUnit->meta()->setInt64(
            "timeUs", mFirstSampleTimeRealUs + timeUs - mFirstSampleTimeUs);

    size_t trackIndex;
    CHECK_EQ((status_t)OK, mExtractor->getSampleTrackIndex(&trackIndex));

    sp<AMessage> msg = new AMessage(kWhatConverterNotify, id());

    msg->setSize(
            "trackIndex", mExtractorTrackToInternalTrack.valueFor(trackIndex));

    msg->setInt32("what", Converter::kWhatAccessUnit);
    msg->setBuffer("accessUnit", accessUnit);
    msg->post();

    mExtractor->advance();

    schedulePullExtractor();
}

status_t WifiDisplaySource::PlaybackSession::setupPacketizer(
        bool enableAudio,
        bool usePCMAudio,
        bool enableVideo,
        VideoFormats::ResolutionType videoResolutionType,
        size_t videoResolutionIndex,
        VideoFormats::ProfileType videoProfileType,
        VideoFormats::LevelType videoLevelType) {
    CHECK(enableAudio || enableVideo);

    if (!mMediaPath.empty()) {
        return setupMediaPacketizer(enableAudio, enableVideo);
    }

    if (enableVideo) {
        status_t err = addVideoSource(
                videoResolutionType, videoResolutionIndex, videoProfileType,
                videoLevelType);

        if (err != OK) {
            return err;
        }
    }

    if (!enableAudio) {
        return OK;
    }

    return addAudioSource(usePCMAudio);
}

status_t WifiDisplaySource::PlaybackSession::addSource(
        bool isVideo, const sp<MediaSource> &source, bool isRepeaterSource,
        bool usePCMAudio, unsigned profileIdc, unsigned levelIdc,
        unsigned constraintSet, size_t *numInputBuffers) {
    CHECK(!usePCMAudio || !isVideo);
    CHECK(!isRepeaterSource || isVideo);
    CHECK(!profileIdc || isVideo);
    CHECK(!levelIdc || isVideo);
    CHECK(!constraintSet || isVideo);

    sp<ALooper> pullLooper = new ALooper;
    pullLooper->setName("pull_looper");

    pullLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);

    sp<ALooper> codecLooper = new ALooper;
    codecLooper->setName("codec_looper");

    codecLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);

    size_t trackIndex;

    sp<AMessage> notify;

    trackIndex = mTracks.size();

    sp<AMessage> format;
    status_t err = convertMetaDataToMessage(source->getFormat(), &format);
    CHECK_EQ(err, (status_t)OK);

    if (isVideo) {
        format->setString("mime", MEDIA_MIMETYPE_VIDEO_AVC);
        format->setInt32("store-metadata-in-buffers", true);
        format->setInt32("store-metadata-in-buffers-output", (mHDCP != NULL)
                && (mHDCP->getCaps() & HDCPModule::HDCP_CAPS_ENCRYPT_NATIVE));
        format->setInt32(
                "color-format", OMX_COLOR_FormatAndroidOpaque);
        format->setInt32("profile-idc", profileIdc);
        format->setInt32("level-idc", levelIdc);
        format->setInt32("constraint-set", constraintSet);
#ifndef ANDROID_DEFAULT_CODE
        addSource_video_ext(format, profileIdc, levelIdc);
#endif 

    } else {
        format->setString(
                "mime",
                usePCMAudio
                    ? MEDIA_MIMETYPE_AUDIO_RAW : MEDIA_MIMETYPE_AUDIO_AAC);
    }

    notify = new AMessage(kWhatConverterNotify, id());
    notify->setSize("trackIndex", trackIndex);

    sp<Converter> converter = new Converter(notify, codecLooper, format);

    looper()->registerHandler(converter);

    err = converter->init();
    if (err != OK) {
        ALOGE("%s converter returned err %d", isVideo ? "video" : "audio", err);

        looper()->unregisterHandler(converter->id());
        return err;
    }

    notify = new AMessage(Converter::kWhatMediaPullerNotify, converter->id());
    notify->setSize("trackIndex", trackIndex);

    sp<MediaPuller> puller = new MediaPuller(source, notify);
    pullLooper->registerHandler(puller);

    if (numInputBuffers != NULL) {
        *numInputBuffers = converter->getInputBufferCount();
	     WFD_LOGI("encoder max buffer count=%d",*numInputBuffers);
    }

    notify = new AMessage(kWhatTrackNotify, id());
    notify->setSize("trackIndex", trackIndex);

    sp<Track> track = new Track(
            notify, pullLooper, codecLooper, puller, converter);

    if (isRepeaterSource) {
        track->setRepeaterSource(static_cast<RepeaterSource *>(source.get()));
    }

    looper()->registerHandler(track);

    mTracks.add(trackIndex, track);

    if (isVideo) {
        mVideoTrackIndex = trackIndex;
	 
    }
    WFD_LOGI("addSource: isVideo=%d  TrackIndex=%d",isVideo,trackIndex);  


    uint32_t flags = 0;
    if (converter->needToManuallyPrependSPSPPS()) {
        flags |= MediaSender::FLAG_MANUALLY_PREPEND_SPS_PPS;
    }

    ssize_t mediaSenderTrackIndex =
        mMediaSender->addTrack(converter->getOutputFormat(), flags);
    CHECK_GE(mediaSenderTrackIndex, 0);

    track->setMediaSenderTrackIndex(mediaSenderTrackIndex);

    return OK;
}


status_t WifiDisplaySource::PlaybackSession::addVideoSource(
        VideoFormats::ResolutionType videoResolutionType,
        size_t videoResolutionIndex,
        VideoFormats::ProfileType videoProfileType,
        VideoFormats::LevelType videoLevelType) {
    size_t width, height, framesPerSecond;
    bool interlaced;
    CHECK(VideoFormats::GetConfiguration(
                videoResolutionType,
                videoResolutionIndex,
                &width,
                &height,
                &framesPerSecond,
                &interlaced));

    unsigned profileIdc, levelIdc, constraintSet;
    CHECK(VideoFormats::GetProfileLevel(
                videoProfileType,
                videoLevelType,
                &profileIdc,
                &levelIdc,
                &constraintSet));

    sp<SurfaceMediaSource> source = new SurfaceMediaSource(width, height);

    source->setUseAbsoluteTimestamps();

    sp<RepeaterSource> videoSource =
        new RepeaterSource(source, framesPerSecond);

    size_t numInputBuffers;
    status_t err = addSource(
            true /* isVideo */, videoSource, true /* isRepeaterSource */,
            false /* usePCMAudio */, profileIdc, levelIdc, constraintSet,
            &numInputBuffers);

    if (err != OK) {
        return err;
    }

    err = source->setMaxAcquiredBufferCount(numInputBuffers);
    CHECK_EQ(err, (status_t)OK);

    mBufferQueue = source->getBufferQueue();

    return OK;
}

status_t WifiDisplaySource::PlaybackSession::addAudioSource(bool usePCMAudio) {
#ifndef ANDROID_DEFAULT_CODE	
    String8 param;
    sp<AudioSource> audioSource = new AudioSource(
            AUDIO_SOURCE_REMOTE_SUBMIX,
            48000 /* sampleRate */,
            param,
            2 /* channelCount */);
#else
    sp<AudioSource> audioSource = new AudioSource(
            AUDIO_SOURCE_REMOTE_SUBMIX,
            48000 /* sampleRate */,
            2 /* channelCount */);
#endif

    if (audioSource->initCheck() == OK) {
        return addSource(
                false /* isVideo */, audioSource, false /* isRepeaterSource */,
                usePCMAudio, 0 /* profileIdc */, 0 /* levelIdc */,
                0 /* constraintSet */, NULL /* numInputBuffers */);
    }

    ALOGW("Unable to instantiate audio source");

    return OK;
}

sp<IGraphicBufferProducer> WifiDisplaySource::PlaybackSession::getSurfaceTexture() {
    return mBufferQueue;
}


void WifiDisplaySource::PlaybackSession::requestIDRFrame() {

#ifndef ANDROID_DEFAULT_CODE
	if(mTestFakeVideoPath != 0) {
        return;
    }
#endif	
    for (size_t i = 0; i < mTracks.size(); ++i) {
        const sp<Track> &track = mTracks.valueAt(i);

        track->requestIDRFrame();
    }
}

void WifiDisplaySource::PlaybackSession::notifySessionDead() {
    // Inform WifiDisplaySource of our premature death (wish).
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatSessionDead);
    notify->post();

    mWeAreDead = true;
}

}  // namespace android

