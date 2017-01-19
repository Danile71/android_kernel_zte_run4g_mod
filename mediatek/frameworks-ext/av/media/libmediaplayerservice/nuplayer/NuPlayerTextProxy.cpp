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

#define LOG_TAG "NuPlayerTextSource"
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>  // CHECK_XX macro
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaDefs.h>  // for MEDIA_MIMETYPE_xxx
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>

#include "NuPlayerTextProxy.h"
#include "NuPlayerTextDVBProxy.h"

namespace android {

// static
sp<NuPlayerTextProxy> NuPlayerTextProxy::CreateTextProxy(
        const sp<AMessage>& format) {
    AString mime;
    CHECK(format->findString("mime", &mime));
    ALOGD("CreateTextProxy() mime: %s", mime.c_str());
    
    if (strcasecmp(mime.c_str(), MEDIA_MIMETYPE_TEXT_DVB) == 0) {
        return new NuPlayerTextDVBProxy(format);
    }
    else
    {
        ALOGE("Unsupported mime type for subtitle. : %s", mime.c_str());
    }
    return NULL;
}


sp<MetaData> NuPlayerTextProxy::getFormat() {
    return NULL;
}

}  // namespace android
#endif

