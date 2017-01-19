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

#ifndef NU_PLAYER_TEXT_PLAYER_H_
#define NU_PLAYER_TEXT_PLAYER_H_

#include <binder/Parcel.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/MediaSource.h>
#include <utils/RefBase.h>

#include "NuPlayerTextProxy.h"

namespace android {

class AMessage;
class MediaPlayerBase;

class NuPlayerTextPlayer : public AHandler {
public:
    NuPlayerTextPlayer(const sp<AMessage> &notify,
        const wp<MediaPlayerBase> &listener);

    virtual ~NuPlayerTextPlayer();
    
    void show();
    void hide();
    void start();
    void stop();
    void pause();
    void resume();
    void seekToAsync(int64_t timeUs);
    void setTextProxy(sp<NuPlayerTextProxy> source);
    virtual void onFirstRef();
protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatPause = 'paus',
        kWhatResume = 'resm',
        kWhatStart = 'strt',
        kWhatSeek = 'seek',
        kWhatRead = 'read',
        kWhatReadNext = 'redn',
        kWhatSendSubtitle = 'send',
        kWhatSetProxy = 'spro',
        kWhatShow = 'show',
        kWhatHide = 'hide',
    };

    // To add Parcel into an AMessage as an object, it should be 'RefBase'.
    struct ParcelEvent : public RefBase {
        Parcel parcel;
    };
    
    sp<AMessage> mNotify;
    sp<ALooper> mLooper;
    wp<MediaPlayerBase> mListener;
    sp<NuPlayerTextProxy> mProxy;
    int64_t mPendingSeekTimeUs;
    bool mPaused;
    int32_t mSendSubtitleGeneration;
    bool    mInited;
    bool    mShow;


    void doRead();
    void onTextEvent();
    void postTextEvent(const sp<ParcelEvent>& parcel = NULL, int64_t timeUs = -1);
    int64_t delayUsFromCurrentTime(int64_t fireTimeUs);
    void notifyError(int error = 0);
    void notifyListener(const Parcel *parcel = NULL);

    DISALLOW_EVIL_CONSTRUCTORS(NuPlayerTextPlayer);
};

}  // namespace android

#endif  // TIMEDTEXT_PLAYER_H_
