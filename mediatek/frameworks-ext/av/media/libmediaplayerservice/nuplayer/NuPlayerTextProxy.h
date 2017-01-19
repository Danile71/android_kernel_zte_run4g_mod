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

#ifndef NU_PLAYER_TEXT_SOURCE_H_
#define NU_PLAYER_TEXT_SOURCE_H_

#include <media/stagefright/foundation/ABase.h>  // for DISALLOW_XXX macro.
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>  // for MediaSource::ReadOptions
#include <utils/RefBase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/ACodec.h>
#include <media/stagefright/MediaDefs.h>

namespace android {

class DataSource;
class MetaData;
class Parcel;

class NuPlayerTextProxy : public RefBase {
 public:
  
  static sp<NuPlayerTextProxy> CreateTextProxy(const sp<AMessage>& format);

  NuPlayerTextProxy() {}

  virtual status_t start() = 0;
  virtual status_t stop() = 0;
  
  // Returns subtitle parcel and its start time.
  virtual status_t parse(
          uint8_t* text,
          size_t size,
          int64_t startTimeUs,
          int64_t endTimeUs,
          Parcel *parcel) = 0;
  virtual status_t extractGlobalDescriptions(Parcel *parcel) {
      return INVALID_OPERATION;
  }
  virtual sp<MetaData> getFormat();

 protected:
  virtual ~NuPlayerTextProxy() { }

 private:
  DISALLOW_EVIL_CONSTRUCTORS(NuPlayerTextProxy);
};

}  // namespace android

#endif  // TIMED_TEXT_SOURCE_H_
