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
#define LOG_TAG "DASHSource"
#include <utils/Log.h>

#include "DashSource.h"

#include "AnotherPacketSource.h"
#include "DashSession.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>

namespace android {

NuPlayer::DASHSource::DASHSource(
        const sp<AMessage> &notify,
        const char *url,
        const KeyedVector<String8, String8> *headers,
        bool uidValid, uid_t uid)
    : Source(notify),
      mURL(url),
      mUIDValid(uidValid),
      mUID(uid),
      mFlags(0),
      mFetchSubtitleDataGeneration(0) {
    if (headers) {
        mExtraHeaders = *headers;

        ssize_t index =
            mExtraHeaders.indexOfKey(String8("x-hide-urls-from-log"));

        if (index >= 0) {
            mFlags |= kFlagIncognito;

            mExtraHeaders.removeItemsAt(index);
        }
    }
}

NuPlayer::DASHSource::~DASHSource() {
    if (mDASHSession != NULL) {
        mDASHSession->disconnect();
        mDASHSession.clear();

        mDASHLooper->stop();
        mDASHLooper.clear();
    }
}

void NuPlayer::DASHSource::prepareAsync() {
    mDASHLooper = new ALooper;
    mDASHLooper->setName("MPEG-DASH");
    mDASHLooper->start();

    sp<AMessage> notify = new AMessage(kWhatSessionNotify, id());

    mDASHSession = new DASHSession(
            notify,
            (mFlags & kFlagIncognito) ? DASHSession::kFlagIncognito : 0,
            mUIDValid,
            mUID);

    mDASHLooper->registerHandler(mDASHSession);

    mDASHSession->connectAsync(
            mURL.c_str(), mExtraHeaders.isEmpty() ? NULL : &mExtraHeaders);
}

void NuPlayer::DASHSource::start() {
}

sp<AMessage> NuPlayer::DASHSource::getFormat(bool audio) {
    sp<AMessage> format;
    status_t err = mDASHSession->getStreamFormat(
            audio ? DASHSession::STREAMTYPE_AUDIO
                  : DASHSession::STREAMTYPE_VIDEO,
            &format);

    if (err != OK) {
        return NULL;
    }

    return format;
}

status_t NuPlayer::DASHSource::feedMoreTSData() {
    return OK;
}

status_t NuPlayer::DASHSource::dequeueAccessUnit(
        bool audio, sp<ABuffer> *accessUnit) {
    return mDASHSession->dequeueAccessUnit(
            audio ? DASHSession::STREAMTYPE_AUDIO
                  : DASHSession::STREAMTYPE_VIDEO,
            accessUnit);
}

status_t NuPlayer::DASHSource::getDuration(int64_t *durationUs) {
    return mDASHSession->getDuration(durationUs);
}

status_t NuPlayer::DASHSource::seekTo(int64_t seekTimeUs) {
    return mDASHSession->seekTo(seekTimeUs);
}

void NuPlayer::DASHSource::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSessionNotify:
        {
            onSessionNotify(msg);
            break;
        }

        case kWhatFetchSubtitleData:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mFetchSubtitleDataGeneration) {
                // stale
                break;
            }

            sp<ABuffer> buffer;
            if (mDASHSession->dequeueAccessUnit(
                    DASHSession::STREAMTYPE_SUBTITLES, &buffer) == OK) {
                sp<AMessage> notify = dupNotify();
                notify->setInt32("what", kWhatSubtitleData);
                notify->setBuffer("buffer", buffer);
                notify->post();

                int64_t timeUs, baseUs, durationUs, delayUs;
                CHECK(buffer->meta()->findInt64("baseUs", &baseUs));
                CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
                CHECK(buffer->meta()->findInt64("durationUs", &durationUs));
                delayUs = baseUs + timeUs - ALooper::GetNowUs();

                msg->post(delayUs > 0ll ? delayUs : 0ll);
            } else {
                // try again in 1 second
                msg->post(1000000ll);
            }

            break;
        }

        default:
            Source::onMessageReceived(msg);
            break;
    }
}

void NuPlayer::DASHSource::onSessionNotify(const sp<AMessage> &msg) {
    int32_t what;
    CHECK(msg->findInt32("what", &what));

    switch (what) {
        case DASHSession::kWhatPrepared:
        {
            // notify the current size here if we have it, otherwise report an initial size of (0,0)
            sp<AMessage> format = getFormat(false /* audio */);
            int32_t width;
            int32_t height;
            if (format != NULL &&
                    format->findInt32("width", &width) && format->findInt32("height", &height)) {
                notifyVideoSizeChanged(width, height);
            } else {
                notifyVideoSizeChanged(0, 0);
            }
            uint32_t flags = FLAG_CAN_PAUSE;

            if (mDASHSession->isSeekable()) {
                flags |= FLAG_CAN_SEEK;
                flags |= FLAG_CAN_SEEK_BACKWARD;
                flags |= FLAG_CAN_SEEK_FORWARD;
            }

            notifyFlagsChanged(flags);

            notifyPrepared();
            break;
        }

        case DASHSession::kWhatPreparationFailed:
        {
            status_t err;
            CHECK(msg->findInt32("err", &err));

            notifyPrepared(err);
            break;
        }

        case DASHSession::kWhatStreamsChanged:
        {
            uint32_t changedMask;
            CHECK(msg->findInt32(
                        "changedMask", (int32_t *)&changedMask));

            bool audio = changedMask & DASHSession::STREAMTYPE_AUDIO;
            bool video = changedMask & DASHSession::STREAMTYPE_VIDEO;

            sp<AMessage> reply;
            CHECK(msg->findMessage("reply", &reply));

            sp<AMessage> notify = dupNotify();
            notify->setInt32("what", kWhatQueueDecoderShutdown);
            notify->setInt32("audio", audio);
            notify->setInt32("video", video);
            notify->setMessage("reply", reply);
            notify->post();
            break;
        }

        case DASHSession::kWhatError:
        {
            status_t err;
            CHECK(msg->findInt32("err", &err));

            notifyError(err);
            break;
        }

        default:
            TRESPASS();
    }
}

}  // namespace android

