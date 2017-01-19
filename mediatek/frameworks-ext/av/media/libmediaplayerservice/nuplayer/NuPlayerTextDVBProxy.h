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

#ifndef NU_PLAYER_TEXT_DVB_PROXY_H_
#define NU_PLAYER_TEXT_DVB_PROXY_H_

#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>

#include <sys/mman.h>
#include <fcntl.h>

#include "NuPlayerTextProxy.h"
#include "dvbparser.h"

namespace android {

class MediaBuffer;
class Parcel;


class DVBParser;

class NuPlayerTextDVBProxy : public NuPlayerTextProxy {
public:
    NuPlayerTextDVBProxy(const sp<AMessage>& format);
    virtual status_t start();
    virtual status_t stop();
    virtual status_t parse(
          uint8_t* text,
          size_t size,
          int64_t startTimeUs,
          int64_t endTimeUs,
          Parcel *parcel);
    virtual status_t extractGlobalDescriptions(Parcel *parcel);
    virtual sp<MetaData> getFormat();
    DVBParser * mDvbParser;

protected:
    virtual ~NuPlayerTextDVBProxy();

private:
    //sp<MediaSource> mSource;

    status_t extractAndAppendLocalDescriptions(
            int64_t timeUs, const uint8_t* text,size_t size, Parcel *parcel);

    DISALLOW_EVIL_CONSTRUCTORS(NuPlayerTextDVBProxy);
};

}  // namespace android

#endif

