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
#define LOG_TAG "DirectRenderer"
#include <utils/Log.h>

#include "DirectRenderer.h"

#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <media/AudioTrack.h>
#include <media/ICrypto.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#ifdef MTK_WFD_SINK_SUPPORT
#include <utils/String8.h>
#include <binder/IPCThreadState.h>
#include "DataPathTrace.h"
#include <cutils/properties.h>
#endif

namespace android {

/*
   Drives the decoding process using a MediaCodec instance. Input buffers
   queued by calls to "queueInputBuffer" are fed to the decoder as soon
   as the decoder is ready for them, the client is notified about output
   buffers as the decoder spits them out.
*/
struct DirectRenderer::DecoderContext : public AHandler {
    enum {
        kWhatOutputBufferReady,
    };
    DecoderContext(const sp<AMessage> &notify);

    status_t init(
            const sp<AMessage> &format,
            const sp<IGraphicBufferProducer> &surfaceTex);

#ifdef MTK_WFD_SINK_SUPPORT
    void pause();
    void resume(const sp<IGraphicBufferProducer> &surfaceTex);
#endif
    void queueInputBuffer(const sp<ABuffer> &accessUnit);

    status_t renderOutputBufferAndRelease(size_t index);
    status_t releaseOutputBuffer(size_t index);

protected:
    virtual ~DecoderContext();

    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatDecoderNotify,
    };

    sp<AMessage> mNotify;
    sp<ALooper> mDecoderLooper;
    sp<MediaCodec> mDecoder;
    Vector<sp<ABuffer> > mDecoderInputBuffers;
    Vector<sp<ABuffer> > mDecoderOutputBuffers;
    List<size_t> mDecoderInputBuffersAvailable;
#ifdef MTK_WFD_SINK_SUPPORT
    sp<AMessage> mFormat;
#endif
    bool mDecoderNotificationPending;

    List<sp<ABuffer> > mAccessUnits;

    void onDecoderNotify();
    void scheduleDecoderNotification();
    void queueDecoderInputBuffers();

    void queueOutputBuffer(
            size_t index, int64_t timeUs, const sp<ABuffer> &buffer);

    DISALLOW_EVIL_CONSTRUCTORS(DecoderContext);
};

////////////////////////////////////////////////////////////////////////////////

/*
   A "push" audio renderer. The primary function of this renderer is to use
   an AudioTrack in push mode and making sure not to block the event loop
   be ensuring that calls to AudioTrack::write never block. This is done by
   estimating an upper bound of data that can be written to the AudioTrack
   buffer without delay.
*/
struct DirectRenderer::AudioRenderer : public AHandler {
    AudioRenderer(const sp<DecoderContext> &decoderContext);

    void queueInputBuffer(
            size_t index, int64_t timeUs, const sp<ABuffer> &buffer);

#ifdef MTK_WFD_SINK_SUPPORT
    void flush();
#endif
protected:
    virtual ~AudioRenderer();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatPushAudio,
#ifdef MTK_WFD_SINK_SUPPORT
        kWhatFlush,
#endif
    };

    struct BufferInfo {
        size_t mIndex;
        int64_t mTimeUs;
        sp<ABuffer> mBuffer;
    };

    sp<DecoderContext> mDecoderContext;
    sp<AudioTrack> mAudioTrack;

    List<BufferInfo> mInputBuffers;
    bool mPushPending;
    size_t mNumFramesWritten;
#ifdef MTK_WFD_SINK_SUPPORT
	size_t mNumInputBuffer;
	size_t mInputBufferSize;
	size_t mcurrentInfosize;
	bool mNeedTimeStretch;
#endif
    void schedulePushIfNecessary();
    void onPushAudio();

    ssize_t writeNonBlocking(const uint8_t *data, size_t size);

    DISALLOW_EVIL_CONSTRUCTORS(AudioRenderer);
};

////////////////////////////////////////////////////////////////////////////////

DirectRenderer::DecoderContext::DecoderContext(const sp<AMessage> &notify)
    : mNotify(notify),
#ifdef MTK_WFD_SINK_SUPPORT
      mFormat(NULL),
#endif
      mDecoderNotificationPending(false) {
}

DirectRenderer::DecoderContext::~DecoderContext() {
    if (mDecoder != NULL) {
        mDecoder->release();
        mDecoder.clear();

        mDecoderLooper->stop();
        mDecoderLooper.clear();
    }
}

status_t DirectRenderer::DecoderContext::init(
        const sp<AMessage> &format,
        const sp<IGraphicBufferProducer> &surfaceTex) {
    CHECK(mDecoder == NULL);

    AString mime;
    CHECK(format->findString("mime", &mime));

    mDecoderLooper = new ALooper;
#ifdef MTK_WFD_SINK_SUPPORT
    if (!strncasecmp("video/", mime.c_str(), 6)) 
    {
         mDecoderLooper->setName("video codec looper");
    }else{
         mDecoderLooper->setName("audio codec looper");
    }
#else
    mDecoderLooper->setName("video codec looper");
#endif

#ifdef MTK_WFD_SINK_SUPPORT
    mDecoderLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
#else
    mDecoderLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_DEFAULT);
#endif

    mDecoder = MediaCodec::CreateByType(
            mDecoderLooper, mime.c_str(), false /* encoder */);

    CHECK(mDecoder != NULL);

#ifdef MTK_WFD_SINK_SUPPORT
    if (!strncasecmp("video/", mime.c_str(), 6)) 
    {
        format->setInt32("vdec-no-record", 1);
	 format->setInt32("vdec-lowlatency", 1);
    }
#endif


    status_t err = mDecoder->configure(
            format,
            surfaceTex == NULL
                ? NULL : new Surface(surfaceTex),
            NULL /* crypto */,
            0 /* flags */);
    CHECK_EQ(err, (status_t)OK);

    err = mDecoder->start();
    CHECK_EQ(err, (status_t)OK);

    err = mDecoder->getInputBuffers(
            &mDecoderInputBuffers);
    CHECK_EQ(err, (status_t)OK);

    err = mDecoder->getOutputBuffers(
            &mDecoderOutputBuffers);
    CHECK_EQ(err, (status_t)OK);

    scheduleDecoderNotification();

#ifdef MTK_WFD_SINK_SUPPORT
    mFormat = format;
#endif
    return OK;
}

#ifdef MTK_WFD_SINK_SUPPORT
void DirectRenderer::DecoderContext::pause() {
    status_t err = mDecoder->flush();    
    CHECK_EQ(err, (status_t)OK);

    err = mDecoder->release();    
    CHECK_EQ(err, (status_t)OK);
}

void DirectRenderer::DecoderContext::resume(const sp<IGraphicBufferProducer> &surfaceTex) {
    CHECK(mFormat != NULL);

    status_t err = mDecoder->configure(
            mFormat,
            surfaceTex == NULL
            ? NULL : new Surface(surfaceTex),
            NULL /* crypto */,
            0 /* flags */);
    CHECK_EQ(err, (status_t)OK);

    err = mDecoder->start();
    CHECK_EQ(err, (status_t)OK);

    err = mDecoder->getInputBuffers(
            &mDecoderInputBuffers);
    CHECK_EQ(err, (status_t)OK);

    err = mDecoder->getOutputBuffers(
            &mDecoderOutputBuffers);
    CHECK_EQ(err, (status_t)OK);
}
#endif

void DirectRenderer::DecoderContext::queueInputBuffer(
        const sp<ABuffer> &accessUnit) {
    CHECK(mDecoder != NULL);

    mAccessUnits.push_back(accessUnit);
    queueDecoderInputBuffers();
}

status_t DirectRenderer::DecoderContext::renderOutputBufferAndRelease(
        size_t index) {
    return mDecoder->renderOutputBufferAndRelease(index);
}

status_t DirectRenderer::DecoderContext::releaseOutputBuffer(size_t index) {
    return mDecoder->releaseOutputBuffer(index);
}

void DirectRenderer::DecoderContext::queueDecoderInputBuffers() {
    if (mDecoder == NULL) {
        return;
    }

    bool submittedMore = false;

    while (!mAccessUnits.empty()
            && !mDecoderInputBuffersAvailable.empty()) {
        size_t index = *mDecoderInputBuffersAvailable.begin();

        mDecoderInputBuffersAvailable.erase(
                mDecoderInputBuffersAvailable.begin());

        sp<ABuffer> srcBuffer = *mAccessUnits.begin();
        mAccessUnits.erase(mAccessUnits.begin());

        const sp<ABuffer> &dstBuffer =
            mDecoderInputBuffers.itemAt(index);

        memcpy(dstBuffer->data(), srcBuffer->data(), srcBuffer->size());

        int64_t timeUs;
        CHECK(srcBuffer->meta()->findInt64("timeUs", &timeUs));

#ifdef MTK_WFD_SINK_SUPPORT
        sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
        int64_t now = ALooper::GetNowUs();
        size_t trackIndex;
        CHECK(mNotify->findSize("trackIndex", &trackIndex));
        debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "queueDecIn1", now/1000);
#endif
        status_t err = mDecoder->queueInputBuffer(
                index,
                0 /* offset */,
                srcBuffer->size(),
                timeUs,
                0 /* flags */);
        CHECK_EQ(err, (status_t)OK);
#ifdef MTK_WFD_SINK_SUPPORT
        //now = ALooper::GetNowUs();
        //debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "queueDecIn2", now/1000);
#endif

        submittedMore = true;
    }

    if (submittedMore) {
        scheduleDecoderNotification();
    }
}

void DirectRenderer::DecoderContext::onMessageReceived(
        const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatDecoderNotify:
        {
            onDecoderNotify();
            break;
        }

        default:
            TRESPASS();
    }
}

void DirectRenderer::DecoderContext::onDecoderNotify() {
    mDecoderNotificationPending = false;

    for (;;) {
        size_t index;
        status_t err = mDecoder->dequeueInputBuffer(&index);

        if (err == OK) {
            mDecoderInputBuffersAvailable.push_back(index);
        } else if (err == -EAGAIN) {
            break;
        } else {
            TRESPASS();
        }
    }

    queueDecoderInputBuffers();

    for (;;) {
        size_t index;
        size_t offset;
        size_t size;
        int64_t timeUs;
        uint32_t flags;
#ifdef MTK_WFD_SINK_SUPPORT
        int64_t now1 = ALooper::GetNowUs();
        size_t trackIndex;
        CHECK(mNotify->findSize("trackIndex", &trackIndex));
#endif
        status_t err = mDecoder->dequeueOutputBuffer(
                &index,
                &offset,
                &size,
                &timeUs,
                &flags);

        if (err == OK) {
#ifdef MTK_WFD_SINK_SUPPORT
            sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
            int64_t now2 = ALooper::GetNowUs();
            debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "dequeueDecOut1", now1/1000);
            //debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "dequeueDecOut2", now2/1000);
	    // ALOGD("[%s][%lld] dequeueDecOut1", ((trackIndex == 0) ? "video":"audio"), (timeUs / 1000));
#endif
            queueOutputBuffer(
                    index, timeUs, mDecoderOutputBuffers.itemAt(index));
        } else if (err == INFO_OUTPUT_BUFFERS_CHANGED) {
            err = mDecoder->getOutputBuffers(
                    &mDecoderOutputBuffers);
            CHECK_EQ(err, (status_t)OK);
        } else if (err == INFO_FORMAT_CHANGED) {
            // We don't care.
        } else if (err == -EAGAIN) {
            break;
        } else {
            TRESPASS();
        }
    }

    scheduleDecoderNotification();
}

void DirectRenderer::DecoderContext::scheduleDecoderNotification() {
    if (mDecoderNotificationPending) {
        return;
    }

    sp<AMessage> notify =
        new AMessage(kWhatDecoderNotify, id());

    mDecoder->requestActivityNotification(notify);
    mDecoderNotificationPending = true;
}

void DirectRenderer::DecoderContext::queueOutputBuffer(
        size_t index, int64_t timeUs, const sp<ABuffer> &buffer) {
    sp<AMessage> msg = mNotify->dup();
    msg->setInt32("what", kWhatOutputBufferReady);
    msg->setSize("index", index);
    msg->setInt64("timeUs", timeUs);
    msg->setBuffer("buffer", buffer);
    msg->post();
}

////////////////////////////////////////////////////////////////////////////////

DirectRenderer::AudioRenderer::AudioRenderer(
        const sp<DecoderContext> &decoderContext)
    : mDecoderContext(decoderContext),
      mPushPending(false),
#ifdef MTK_WFD_SINK_SUPPORT      
      mNumInputBuffer(0),
      mInputBufferSize(0),
      mcurrentInfosize(0),
      mNeedTimeStretch(false), 
#endif      
      mNumFramesWritten(0) {
#ifdef MTK_WFD_SINK_SUPPORT   
    size_t tempFrameCnt = 0;
    AudioTrack::getMinFrameCount( &tempFrameCnt, AUDIO_STREAM_DEFAULT, 48000.0f);
    ALOGE("AudioRenderer::AudioRenderer audiotrack framecount = %d", tempFrameCnt);
    mAudioTrack = new AudioTrack(
            AUDIO_STREAM_DEFAULT,
            48000.0f,
            AUDIO_FORMAT_PCM,
            AUDIO_CHANNEL_OUT_STEREO,
            tempFrameCnt);
#else
    mAudioTrack = new AudioTrack(
            AUDIO_STREAM_DEFAULT,
            48000.0f,
            AUDIO_FORMAT_PCM,
            AUDIO_CHANNEL_OUT_STEREO,
            (int)0 /* frameCount */);
#endif

    CHECK_EQ((status_t)OK, mAudioTrack->initCheck());

    mAudioTrack->start();
}

DirectRenderer::AudioRenderer::~AudioRenderer() {
}

void DirectRenderer::AudioRenderer::queueInputBuffer(
        size_t index, int64_t timeUs, const sp<ABuffer> &buffer) {
    BufferInfo info;
    info.mIndex = index;
    info.mTimeUs = timeUs;
    info.mBuffer = buffer;
#ifdef MTK_WFD_SINK_SUPPORT
    //ALOGD("queue input buffer: info index:%d, timeUs:%lld, bufq size:%d",
    //        index, timeUs, mInputBuffers.size());
    if(mInputBufferSize == 0){
		mInputBufferSize = info.mBuffer->size();
	}
#endif

    mInputBuffers.push_back(info);
    schedulePushIfNecessary();
}

#ifdef MTK_WFD_SINK_SUPPORT
void DirectRenderer::AudioRenderer::flush() {
    sp<AMessage> msg = new AMessage(kWhatFlush, id());
    msg->post();
}
#endif

void DirectRenderer::AudioRenderer::onMessageReceived(
        const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatPushAudio:
        {
            onPushAudio();
            break;
        }
#ifdef MTK_WFD_SINK_SUPPORT
        case kWhatFlush:
        {
            while (!mInputBuffers.empty()) {
                const BufferInfo &info = *mInputBuffers.begin();

                mDecoderContext->releaseOutputBuffer(info.mIndex);

                mInputBuffers.erase(mInputBuffers.begin());
            }
            break;
        }
#endif
        default:
            break;
    }
}

void DirectRenderer::AudioRenderer::schedulePushIfNecessary() {
    if (mPushPending || mInputBuffers.empty()) {
#ifdef MTK_WFD_SINK_SUPPORT 
        //ALOGD("push exit! mPushPending:%d, bufq size: %d",
        //        mPushPending, mInputBuffers.size());
#endif
        return;
    }

    mPushPending = true;

    uint32_t numFramesPlayed;
    CHECK_EQ(mAudioTrack->getPosition(&numFramesPlayed),
             (status_t)OK);

    uint32_t numFramesPendingPlayout = mNumFramesWritten - numFramesPlayed;

    // This is how long the audio sink will have data to
    // play back.
    const float msecsPerFrame = 1000.0f / mAudioTrack->getSampleRate();

    int64_t delayUs =
        msecsPerFrame * numFramesPendingPlayout * 1000ll;

    // Let's give it more data after about half that time
    // has elapsed.
 

#ifdef MTK_WFD_SINK_SUPPORT 

	if(!mInputBuffers.empty()){
		ALOGD("mInputBuffers.size() = %d",mInputBuffers.size());
		numFramesPendingPlayout = (mInputBufferSize*(mInputBuffers.size()-1)+mcurrentInfosize)/ mAudioTrack->frameSize()+numFramesPendingPlayout;
	
	}	
	int64_t	TotaldelayUs = msecsPerFrame * numFramesPendingPlayout * 1000ll;	
   ALOGD("numFramesPendingPlayout = %d,numFramesPlayed = %d,delayUs = %lld,TotaldelayUs = %lld",numFramesPendingPlayout,numFramesPlayed,delayUs,TotaldelayUs);
/*
   if (TotaldelayUs > 140000 & !mNeedTimeStretch){
	   mAudioTrack->setParameters(String8("time_stretch=80")); 
	   mNeedTimeStretch = true;
	   ALOGD(" timestretch set to 80",delayUs);
   }
   if(TotaldelayUs <60000 &mNeedTimeStretch)
   {
	   mAudioTrack->setParameters(String8("time_stretch=100"));
	   mNeedTimeStretch = false;
	   ALOGD("timestretch reset to 100",delayUs);
   }
*/
	 if (TotaldelayUs > 150000){
		mAudioTrack->setParameters(String8("time_stretch=80")); 
	}
	else if(TotaldelayUs < 100000){
		mAudioTrack->setParameters(String8("time_stretch=100"));	
	}


#endif
    (new AMessage(kWhatPushAudio, id()))->post(delayUs / 2);
}

void DirectRenderer::AudioRenderer::onPushAudio() {
    mPushPending = false;

    while (!mInputBuffers.empty()) {
        const BufferInfo &info = *mInputBuffers.begin();

        ssize_t n = writeNonBlocking(
                info.mBuffer->data(), info.mBuffer->size());

        if (n < (ssize_t)info.mBuffer->size()) {
            CHECK_GE(n, 0);

            info.mBuffer->setRange(
                    info.mBuffer->offset() + n, info.mBuffer->size() - n);
#ifdef MTK_WFD_SINK_SUPPORT
			mcurrentInfosize = info.mBuffer->size();
#endif
            break;
        }
#ifdef MTK_WFD_SINK_SUPPORT		
		else{
			mcurrentInfosize = 0;
		}		
#endif
#ifdef MTK_WFD_SINK_SUPPORT
        sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
        int64_t now = ALooper::GetNowUs();
        debugInfo->addTimeInfoByKey(false, info.mTimeUs, "queueRender1", now/1000);
#endif
        mDecoderContext->releaseOutputBuffer(info.mIndex);

        mInputBuffers.erase(mInputBuffers.begin());
    }

    schedulePushIfNecessary();
}

ssize_t DirectRenderer::AudioRenderer::writeNonBlocking(
        const uint8_t *data, size_t size) {
    uint32_t numFramesPlayed;
    status_t err = mAudioTrack->getPosition(&numFramesPlayed);
    if (err != OK) {
        return err;
    }

    ssize_t numFramesAvailableToWrite =
        mAudioTrack->frameCount() - (mNumFramesWritten - numFramesPlayed);

    size_t numBytesAvailableToWrite =
        numFramesAvailableToWrite * mAudioTrack->frameSize();

    if (size > numBytesAvailableToWrite) {
        size = numBytesAvailableToWrite;
    }

    CHECK_EQ(mAudioTrack->write(data, size), (ssize_t)size);

    size_t numFramesWritten = size / mAudioTrack->frameSize();
    mNumFramesWritten += numFramesWritten;

    return size;
}

////////////////////////////////////////////////////////////////////////////////

DirectRenderer::DirectRenderer(
        const sp<IGraphicBufferProducer> &bufferProducer)
    : mSurfaceTex(bufferProducer),
      mVideoRenderPending(false),
      mNumFramesLate(0),
      mNumFrames(0) {
}

DirectRenderer::~DirectRenderer() {
}

void DirectRenderer::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatDecoderNotify:
        {
            onDecoderNotify(msg);
            break;
        }

        case kWhatRenderVideo:
        {
            onRenderVideo();
            break;
        }

        case kWhatQueueAccessUnit:
            onQueueAccessUnit(msg);
            break;

        case kWhatSetFormat:
            onSetFormat(msg);
            break;

#ifdef MTK_WFD_SINK_SUPPORT
        case kWhatPause:
        {
            uint32_t replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            onPause();

            sp<AMessage> response = new AMessage;
            response->postReply(replyID);
            break;
        }
        case kWhatPlay:
        {
            uint32_t replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            onResume();
            sp<AMessage> response = new AMessage;
            response->postReply(replyID);
            break;
        }
#endif
        default:
            TRESPASS();
    }
}

void DirectRenderer::setFormat(size_t trackIndex, const sp<AMessage> &format) {
    sp<AMessage> msg = new AMessage(kWhatSetFormat, id());
    msg->setSize("trackIndex", trackIndex);
    msg->setMessage("format", format);
    msg->post();
}

void DirectRenderer::onSetFormat(const sp<AMessage> &msg) {
    size_t trackIndex;
    CHECK(msg->findSize("trackIndex", &trackIndex));

    sp<AMessage> format;
    CHECK(msg->findMessage("format", &format));

    internalSetFormat(trackIndex, format);
}

void DirectRenderer::internalSetFormat(
        size_t trackIndex, const sp<AMessage> &format) {
    CHECK_LT(trackIndex, 2u);

    CHECK(mDecoderContext[trackIndex] == NULL);

    sp<AMessage> notify = new AMessage(kWhatDecoderNotify, id());
    notify->setSize("trackIndex", trackIndex);

    mDecoderContext[trackIndex] = new DecoderContext(notify);
    looper()->registerHandler(mDecoderContext[trackIndex]);

    CHECK_EQ((status_t)OK,
             mDecoderContext[trackIndex]->init(
                 format, trackIndex == 0 ? mSurfaceTex : NULL));

    if (trackIndex == 1) {
        // Audio
        mAudioRenderer = new AudioRenderer(mDecoderContext[1]);
        looper()->registerHandler(mAudioRenderer);
    }
}

void DirectRenderer::queueAccessUnit(
        size_t trackIndex, const sp<ABuffer> &accessUnit) {
#ifdef MTK_WFD_SINK_SUPPORT
    sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
    int64_t now = ALooper::GetNowUs();
    int64_t timeUs;
    CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));
    debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "queueAccu", now/1000);
#endif
    sp<AMessage> msg = new AMessage(kWhatQueueAccessUnit, id());
    msg->setSize("trackIndex", trackIndex);
    msg->setBuffer("accessUnit", accessUnit);
    msg->post();
}

void DirectRenderer::onQueueAccessUnit(const sp<AMessage> &msg) {
    size_t trackIndex;
    CHECK(msg->findSize("trackIndex", &trackIndex));

    sp<ABuffer> accessUnit;
    CHECK(msg->findBuffer("accessUnit", &accessUnit));

    CHECK_LT(trackIndex, 2u);
    CHECK(mDecoderContext[trackIndex] != NULL);

    mDecoderContext[trackIndex]->queueInputBuffer(accessUnit);
}

void DirectRenderer::onDecoderNotify(const sp<AMessage> &msg) {
    size_t trackIndex;
    CHECK(msg->findSize("trackIndex", &trackIndex));

    int32_t what;
    CHECK(msg->findInt32("what", &what));

    switch (what) {
        case DecoderContext::kWhatOutputBufferReady:
        {
            size_t index;
            CHECK(msg->findSize("index", &index));

            int64_t timeUs;
            CHECK(msg->findInt64("timeUs", &timeUs));

            sp<ABuffer> buffer;
            CHECK(msg->findBuffer("buffer", &buffer));

            queueOutputBuffer(trackIndex, index, timeUs, buffer);
            break;
        }

        default:
            TRESPASS();
    }
}

void DirectRenderer::queueOutputBuffer(
        size_t trackIndex,
        size_t index, int64_t timeUs, const sp<ABuffer> &buffer) {
#ifdef MTK_WFD_SINK_SUPPORT
    sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
    int64_t now = ALooper::GetNowUs();
    debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "queueOutput", now/1000);
#endif
    if (trackIndex == 1) {
        // Audio
        mAudioRenderer->queueInputBuffer(index, timeUs, buffer);
        return;
    }

    OutputInfo info;
    info.mIndex = index;
    info.mTimeUs = timeUs;
    info.mBuffer = buffer;
    mVideoOutputBuffers.push_back(info);

    scheduleVideoRenderIfNecessary();
}

void DirectRenderer::scheduleVideoRenderIfNecessary() {
    if (mVideoRenderPending || mVideoOutputBuffers.empty()) {
        return;
    }

    mVideoRenderPending = true;

    int64_t timeUs = (*mVideoOutputBuffers.begin()).mTimeUs;
    int64_t nowUs = ALooper::GetNowUs();

    int64_t delayUs = timeUs - nowUs;

     //video frame display as soon as possible.
     delayUs = 0;

    (new AMessage(kWhatRenderVideo, id()))->post(delayUs);
}

void DirectRenderer::onRenderVideo() {
    mVideoRenderPending = false;

    int64_t nowUs = ALooper::GetNowUs();

    while (!mVideoOutputBuffers.empty()) {
        const OutputInfo &info = *mVideoOutputBuffers.begin();

        //video frame display as soon as possible.
        /*if (info.mTimeUs > nowUs) {
            break;
        }*/

        if (info.mTimeUs + 15000ll < nowUs) {
            ++mNumFramesLate;
        }
        ++mNumFrames;

#ifdef MTK_WFD_SINK_SUPPORT
        sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
        int64_t now = ALooper::GetNowUs();
	 int64_t timeUs = info.mTimeUs;
        debugInfo->addTimeInfoByKey(true, info.mTimeUs, "queueRender1", now/1000);
#endif
        status_t err =
            mDecoderContext[0]->renderOutputBufferAndRelease(info.mIndex);
        CHECK_EQ(err, (status_t)OK);

        mVideoOutputBuffers.erase(mVideoOutputBuffers.begin());
#ifdef MTK_WFD_SINK_SUPPORT
        debugLatency(true, timeUs);    
#endif
    }

    scheduleVideoRenderIfNecessary();
}

#ifdef MTK_WFD_SINK_SUPPORT
    void DirectRenderer::debugLatency(bool mediaType, int64_t timestamp){
    //mediatype: true:video,false:audio;
    
        sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
	 int64_t offset = debugInfo->getTimeInfoByKey(true, timestamp, "Offset");
	 int64_t now = ALooper::GetNowUs();
	 
	 int64_t oldtimeUs = timestamp + offset;
        int64_t t1 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "rtprecvin");	
        int64_t t2 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "atsparserin");
       // int64_t t3 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "mediarecvout");
        int64_t t4 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "apacksourceout");
        //int64_t t5 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "sinkin");
        //int64_t t6 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "sinkout");
        
        
        
        int64_t q1 = debugInfo->getTimeInfoByKey(true, timestamp, "queueAccu");   
        int64_t q2 = debugInfo->getTimeInfoByKey(true, timestamp, "queueDecIn1");
       // int64_t q3 = debugInfo->getTimeInfoByKey(true, timestamp, "queueDecIn2");
        int64_t q4 = debugInfo->getTimeInfoByKey(true, timestamp, "dequeueDecOut1");
        //int64_t q5 = debugInfo->getTimeInfoByKey(true, timestamp, "dequeueDecOut2");
        int64_t q6 = debugInfo->getTimeInfoByKey(true, timestamp, "queueOutput");   
        int64_t q7 = debugInfo->getTimeInfoByKey(true, timestamp, "queueRender1");   
        

        if ( offset != -1&& t1 != -1&& t2 != -1&& t4!= -1 && q1 != -1 && q2 != -1 && q4 != -1 && q6 != -1 && q7 != -1) {
			
                                                                  ALOGD("[%s][%lld]  rv->parsin %lld,parsein->out %lld ,out->dr %lld, dr->mc %lld, dec %lld, dec->queOut %lld,  dr->rend %lld, rend %lld,     total %lld",
         (mediaType? "video":"audio"),timestamp / 1000 ,              t2-t1 ,                     t4-t2,           q1-t4,             q2 - q1,      q4 - q2,      q6 -q4,                   q7 - q6,     now/1000 - q7, q7 - t1);
        }else if (q1 != -1 && q2 != -1 && q4 != -1 && q6 != -1 && q7 != -1){
                                                   ALOGD("[%s][%lld]Partial   dr->mc %lld, dec %lld, dec->queOut %lld,  dr->rend %lld, rend %lld,     total %lld",
         (mediaType? "video":"audio"),timestamp / 1000 ,             q2 - q1,      q4 - q2,      q6 -q4,                   q7 - q6,     now/1000 - q7, q7 - q1);
        }
		
}
    void DirectRenderer::pause() {
        ALOGD("pause caller pid: %d, our pid: %d",
                IPCThreadState::self()->getCallingPid(), getpid());
        if (getpid() == IPCThreadState::self()->getCallingPid()) {
            onPause();
        } else {
            sp<AMessage> msg = new AMessage(kWhatPause, id());
            sp<AMessage> response;
            msg->postAndAwaitResponse(&response);
        }
    }

    void DirectRenderer::onPause() {
        ALOGD("kWhatPause >>");
        // flush video renderer
        while (!mVideoOutputBuffers.empty()) {
            const OutputInfo &info = *mVideoOutputBuffers.begin();

            status_t err =
                mDecoderContext[0]->releaseOutputBuffer(info.mIndex);
            CHECK_EQ(err, (status_t)OK);

            mVideoOutputBuffers.erase(mVideoOutputBuffers.begin());
        }
        // flush audio renderer
        ALOGD("calling audio render %p's flush.", mAudioRenderer.get());
        if (mAudioRenderer != NULL)
            mAudioRenderer->flush();

        // flush decoder
        for (size_t trackIndex = 0; trackIndex < 2; trackIndex ++) 
            if (mDecoderContext[trackIndex] != NULL)
                mDecoderContext[trackIndex]->pause();
        
        ALOGD("<< kWhatPause");
    }

    void DirectRenderer::resume(const sp<IGraphicBufferProducer> &bufferProducer) {
        mSurfaceTex = bufferProducer;
        ALOGD("resume caller pid: %d, our pid: %d",
                IPCThreadState::self()->getCallingPid(), getpid());
        if (getpid() == IPCThreadState::self()->getCallingPid()) {
            onResume();
        } else {
            sp<AMessage> msg = new AMessage(kWhatPlay, id());
            sp<AMessage> response;
            msg->postAndAwaitResponse(&response);
        }
    }

    void DirectRenderer::onResume() {
        ALOGD("kWhatPlay >>");
        for (size_t trackIndex = 0; trackIndex < 2; trackIndex ++)
            if (mDecoderContext[trackIndex] != NULL)
                mDecoderContext[trackIndex]->resume(trackIndex == 0 ? mSurfaceTex : NULL);
        ALOGD("<< kWhatPlay");
    }
#endif
}  // namespace android

