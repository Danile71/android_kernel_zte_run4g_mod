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
#ifdef MTK_SUBTITLE_SUPPORT

#define LOG_TAG "NuPlayerTextPlayer"
#include <utils/Log.h>

#include <limits.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/timedtext/TimedTextDriver.h>
#include <media/stagefright/MediaErrors.h>
#include <media/MediaPlayerInterface.h>

#include "NuPlayerTextPlayer.h"


// use xlog to debug
#if !defined(ANDROID_DEFAULT_CODE) && defined(MTK_STAGEFRIGHT_USE_XLOG)
#include <cutils/xlog.h>
#endif

namespace android {

// Event should be fired a bit earlier considering the processing time till
// application actually gets the notification message.
static const int64_t kAdjustmentProcessingTimeUs = 100000ll;
static const int64_t kMaxDelayUs = 5000000ll;
static const int64_t kWaitTimeUsToRetryRead = 100000ll;
static const int64_t kInvalidTimeUs = INT_MIN;

NuPlayerTextPlayer::NuPlayerTextPlayer(const sp<AMessage> &notify,
        const wp<MediaPlayerBase> &listener)
    : mLooper(new ALooper),
      mInited(false),
      mShow(true),
      mNotify(notify),
      mListener(listener),
      mProxy(NULL),
      mPendingSeekTimeUs(kInvalidTimeUs),
      mPaused(false),
      mSendSubtitleGeneration(0) {
    ALOGV("[start]  NuPlayerTextPlayer::NuPlayerTextPlayer");
    mLooper->setName("NuPlayerTextPlayer");
    ALOGV("[end]  NuPlayerTextPlayer::NuPlayerTextPlayer");
}

NuPlayerTextPlayer::~NuPlayerTextPlayer() {
    ALOGV("[start]  NuPlayerTextPlayer::~NuPlayerTextPlayer");
    if (mProxy != NULL) {
        mProxy->stop();
        mProxy.clear();
        mProxy = NULL;
    }    

    mLooper->unregisterHandler(id());
    mLooper->stop();
    mInited = false;
    ALOGV("[end]  NuPlayerTextPlayer::~NuPlayerTextPlayer");
}

void NuPlayerTextPlayer::onFirstRef()
{
    AHandler::onFirstRef();
    mLooper->registerHandler(this);
    mLooper->start();
}

void NuPlayerTextPlayer::show() {
    ALOGE("[PANDA]  NuPlayerTextPlayer::show");
    (new AMessage(kWhatShow, id()))->post();
}

void NuPlayerTextPlayer::hide() {
    ALOGE("[PANDA]  NuPlayerTextPlayer::hide");
    (new AMessage(kWhatHide, id()))->post();
}


void NuPlayerTextPlayer::start() {
    ALOGE("[PANDA]  NuPlayerTextPlayer::start");
    (new AMessage(kWhatStart, id()))->post();
}

void NuPlayerTextPlayer::stop() {
    ALOGE("[PANDA]  NuPlayerTextPlayer::stop");
}

void NuPlayerTextPlayer::pause() {
    ALOGE("[PANDA]  NuPlayerTextPlayer::pause");
    (new AMessage(kWhatPause, id()))->post();
}

void NuPlayerTextPlayer::resume() {
    ALOGE("[PANDA]  NuPlayerTextPlayer::resume");
    (new AMessage(kWhatResume, id()))->post();
}

void NuPlayerTextPlayer::seekToAsync(int64_t timeUs) {
    ALOGE("[PANDA]  NuPlayerTextPlayer::seekToAsync");
    sp<AMessage> msg = new AMessage(kWhatSeek, id());
    msg->setInt64("seekTimeUs", timeUs);
    msg->post();
}

void NuPlayerTextPlayer::setTextProxy(sp<NuPlayerTextProxy> source) {
    ALOGE("[PANDA]  NuPlayerTextPlayer::setTextProxy");
    sp<AMessage> msg = new AMessage(kWhatSetProxy, id());
    msg->setObject("proxy", source);
    msg->post();
}

void NuPlayerTextPlayer::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatPause: {
            ALOGD("kWhatPause");
            mPaused = true;
            break;
        }
        case kWhatResume: {
            ALOGD("kWhatResume");
            mPaused = false;
            if (mPendingSeekTimeUs != kInvalidTimeUs) {
                seekToAsync(mPendingSeekTimeUs);
                mPendingSeekTimeUs = kInvalidTimeUs;
            } else {
#ifdef MTK_SUBTITLE_SUPPORT

#else
                doRead();
#endif

            }
            break;
        }
        case kWhatStart: {
            ALOGD("kWhatStart");
            sp<MediaPlayerBase> listener = mListener.promote();
            if (listener == NULL) {
                ALOGE("Listener is NULL when kWhatStart is received.");
                break;
            }

            if (mProxy == NULL) {
                ALOGE("mProxy is NULL when kWhatStart is received.");
                break;
            }
            
            mPaused = false;
            mPendingSeekTimeUs = kInvalidTimeUs;
#ifndef ANDROID_DEFAULT_CODE
            notifyListener();
#endif

#ifndef ANDROID_DEFAULT_CODE
            ALOGI("kWhatStart ");
#endif

            mSendSubtitleGeneration++;
            doRead();
            break;
        }
        case kWhatRead: {
            ALOGD("kWhatRead");
            int32_t generation = -1;
            sp<ABuffer> buffer;
            int32_t err = OK;
            int64_t startTimeUs = 0;
            int64_t endTimeUs = 0;

            
            CHECK(msg->findInt32("generation", &generation));
            if (generation != mSendSubtitleGeneration) {
                // Drop obsolete msg.
                break;
            }

            if (mProxy == NULL) {
                ALOGE("mProxy is NULL when kWhatStart is received.");
                break;
            }
            
            if (!msg->findBuffer("buffer", &buffer)) {
                CHECK(msg->findInt32("err", &err));

                ALOGE("reply error %d instead of an input buffer",err);

                buffer.clear();
                break;
            }

            if (false == mShow)
            {
                ALOGE("mShow is flase when kWhatStart is received.");
                break;
            }
            
            sp<ParcelEvent> parcelEvent = new ParcelEvent();
            CHECK(buffer->meta()->findInt64("timeUs", &startTimeUs));
            ALOGD("got %d byte text,timeUs:%d\n",buffer->size(),startTimeUs/1000);
            status_t parseResult = mProxy->parse(
                            buffer->data(),
                            buffer->size(),
                            startTimeUs,
                            -1,     /*fix me,this is a output parameter*/
                            &(parcelEvent->parcel));
            if (ERROR_BUFFER_TOO_SMALL == parseResult)
            {
                ALOGE("reply  %d, more data is needed,read more text data\n",parseResult);
                parcelEvent.clear();
                doRead();/* read next */
                break;
            }
            else if (OK != parseResult)
            {
                ALOGE("reply error %d instead of an valid parcel",parseResult);
                buffer.clear();
                parcelEvent.clear();
                doRead();/* read next */
                break;
            }
            postTextEvent(parcelEvent, startTimeUs);
            break;
        }

        case kWhatReadNext: {
                ALOGD("kWhatReadNext");
                int32_t generation = -1;
                CHECK(msg->findInt32("generation", &generation));
                if (generation != mSendSubtitleGeneration) {
                    // Drop obsolete msg.
                    break;
                }

                doRead();
                break;
            }
        case kWhatSeek: {
            ALOGD("kWhatSeek");
            int64_t seekTimeUs = kInvalidTimeUs;
            // Clear a displayed timed text before seeking.
            notifyListener();
            msg->findInt64("seekTimeUs", &seekTimeUs);
            if (seekTimeUs == kInvalidTimeUs) {
                sp<MediaPlayerBase> listener = mListener.promote();
                if (listener != NULL) {
                    int32_t positionMs = 0;
                    listener->getCurrentPosition(&positionMs);
                    seekTimeUs = positionMs * 1000ll;
                }
            }
            if (mPaused) {
                mPendingSeekTimeUs = seekTimeUs;
                break;
            }
            mSendSubtitleGeneration++;
            //doSeekAndRead(seekTimeUs);
            break;
        }
        case kWhatSendSubtitle: {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));
            if (generation != mSendSubtitleGeneration) {
                // Drop obsolete msg.
                break;
            }
            // If current time doesn't reach to the fire time,
            // re-post the message with the adjusted delay time.
            int64_t fireTimeUs = kInvalidTimeUs;
            if (msg->findInt64("fireTimeUs", &fireTimeUs)) {
                // TODO: check if fireTimeUs is not kInvalidTimeUs.
                int64_t delayUs = delayUsFromCurrentTime(fireTimeUs);
#ifndef ANDROID_DEFAULT_CODE
		ALOGV("%s() delayUs:%lld,line:%d, fireTimeUs:%lld", __FUNCTION__, delayUs, __LINE__, fireTimeUs);
#endif
                if (delayUs > 0) {
                    msg->post(delayUs);
                    break;
                }
            }
            sp<RefBase> obj;
            if (msg->findObject("subtitle", &obj)) {
                sp<ParcelEvent> parcelEvent;
                parcelEvent = static_cast<ParcelEvent*>(obj.get());
                notifyListener(&(parcelEvent->parcel));
                sp<AMessage> msg = new AMessage(kWhatReadNext, id());
                msg->setInt32("generation", mSendSubtitleGeneration);
                msg->post(kAdjustmentProcessingTimeUs*2);
                
            } else {
                notifyListener();
            }
            break;
        }
        case kWhatSetProxy: {
            mSendSubtitleGeneration++;
            sp<RefBase> obj;
            msg->findObject("proxy", &obj);
            if (mProxy != NULL) {
                ALOGE("[PANDA]  onMessageReceived::kWhatSetSource   mSource->stop");
                mProxy->stop();
                mProxy.clear();
                mProxy = NULL;
            }
            // null source means deselect track.
            if (obj == NULL) {
                mPendingSeekTimeUs = kInvalidTimeUs;
                mPaused = false;
                notifyListener();
                break;
            }
            mProxy = static_cast<NuPlayerTextProxy*>(obj.get());
            status_t err = mProxy->start();
            if (err != OK) {
                notifyError(err);
                break;
            }
            Parcel parcel;
            err = mProxy->extractGlobalDescriptions(&parcel);
            if (err != OK) {
                notifyError(err);
                break;
            }
            notifyListener(&parcel);
            break;
        }

        case kWhatShow: {
            mShow = true;
            break;
        }

        case kWhatHide: {
            mShow = false;
    
            /*clear screen*/
            notifyListener();
            break;
        }
    }
}

void NuPlayerTextPlayer::doRead() {    
    sp<AMessage> notify = mNotify->dup();
    sp<AMessage> reply = new AMessage(kWhatRead, id());
    reply->setInt32("generation", mSendSubtitleGeneration);
    
    notify->setMessage("reply", reply);    
    notify->post();
}

void NuPlayerTextPlayer::postTextEvent(const sp<ParcelEvent>& parcel, int64_t timeUs) {
    int64_t delayUs = delayUsFromCurrentTime(timeUs);
#ifndef ANDROID_DEFAULT_CODE
    ALOGV("%s() delayUs:%lld,line:%d, timeUs:%lld", __FUNCTION__, delayUs, __LINE__, timeUs);
#endif
    sp<AMessage> msg = new AMessage(kWhatSendSubtitle, id());
    msg->setInt32("generation", mSendSubtitleGeneration);
    if (parcel != NULL) {
        msg->setObject("subtitle", parcel);
    }
    msg->setInt64("fireTimeUs", timeUs);
    msg->post(delayUs);
}

int64_t NuPlayerTextPlayer::delayUsFromCurrentTime(int64_t fireTimeUs) {
    sp<MediaPlayerBase> listener = mListener.promote();
    if (listener == NULL) {
        // TODO: it may be better to return kInvalidTimeUs
        ALOGE("%s: Listener is NULL. (fireTimeUs = %lld)",
              __FUNCTION__, fireTimeUs);
        return 0;
    }
    int32_t positionMs = 0;
    listener->getCurrentPosition(&positionMs);
    int64_t positionUs = positionMs * 1000ll;

    if (fireTimeUs <= positionUs + kAdjustmentProcessingTimeUs) {
#ifndef ANDROID_DEFAULT_CODE
	ALOGI("fireTimeUs:%lld < positionUs:%lld +100ms ", fireTimeUs, positionUs);
#endif
	return 0;
    } else {
        int64_t delayUs = fireTimeUs - positionUs - kAdjustmentProcessingTimeUs;
	if (delayUs > kMaxDelayUs) {
#ifndef ANDROID_DEFAULT_CODE
	    ALOGI("delayUs > kMaxDelayUs,fireTimeUs:%lld, positionUs:%lld", fireTimeUs, positionUs);
#endif
	    return kMaxDelayUs;
	}
        return delayUs;
    }
}

void NuPlayerTextPlayer::notifyError(int error) {
    sp<MediaPlayerBase> listener = mListener.promote();
    if (listener == NULL) {
        ALOGE("%s(error=%d): Listener is NULL.", __FUNCTION__, error);
        return;
    }
    listener->sendEvent(MEDIA_INFO, MEDIA_INFO_TIMED_TEXT_ERROR, error);
}

void NuPlayerTextPlayer::notifyListener(const Parcel *parcel) {
    sp<MediaPlayerBase> listener = mListener.promote();
    if (listener == NULL) {
        ALOGE("%s: Listener is NULL.", __FUNCTION__);
        return;
    }
    int32_t positionMs = 0;
    listener->getCurrentPosition(&positionMs);
    
    if (parcel != NULL && (parcel->dataSize() > 0)) {
#ifndef ANDROID_DEFAULT_CODE
#ifdef MTK_STAGEFRIGHT_USE_XLOG
    // debug for check send string content, include properties and timedtext .etc.
	{
	    int num = parcel->dataSize();
            const uint8_t *tmp = (uint8_t *)parcel->data();
            XLOGV("^^^^^^^^^^^^^^");
	    for (int i=0; i<num; i++) {
                XLOGV("zxy:0x%x", *(tmp+i));
	    }
            XLOGV("$$$$$$$$$$$$$");
	}
#endif
#endif
        ALOGI("NuPlayerTextPlayer::notifyListener:notify MEDIA_TIMED_TEXT at %d\n",positionMs);
        listener->sendEvent(MEDIA_TIMED_TEXT, 0, 0, parcel);
    } else {  // send an empty timed text to clear the screen
        ALOGI("NuPlayerTextPlayer::notifyListener:clear screen at %d\n",positionMs);
        listener->sendEvent(MEDIA_TIMED_TEXT);
    }
}

}  // namespace android
#endif

