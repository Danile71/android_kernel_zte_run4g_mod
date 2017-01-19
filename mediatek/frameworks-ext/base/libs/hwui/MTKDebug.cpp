/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "OpenGLRenderer"

#include <SkImageEncoder.h>
#include "MTKDebug.h"
#include <GLES3/gl3.h>
#include <unistd.h>

#include <stdlib.h>
#include <utils/Thread.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <SkCanvas.h>

#if defined(MTK_ENG_BUILD)

int g_HWUI_debug_opengl = 1;
int g_HWUI_debug_extensions = 0;
int g_HWUI_debug_init = 0;
int g_HWUI_debug_memory_usage = 0;
int g_HWUI_debug_cache_flush = 1;
int g_HWUI_debug_layers_as_regions = 0;
int g_HWUI_debug_clip_regions = 0;
int g_HWUI_debug_programs = 0;
int g_HWUI_debug_layers = 1;
int g_HWUI_debug_render_buffers = 0;
int g_HWUI_debug_stencil = 0;
int g_HWUI_debug_patches = 0;
int g_HWUI_debug_patches_vertices = 0;
int g_HWUI_debug_patches_empty_vertices = 0;
int g_HWUI_debug_paths = 0;
int g_HWUI_debug_textures = 1;
int g_HWUI_debug_layer_renderer = 1;
int g_HWUI_debug_font_renderer = 0;
int g_HWUI_debug_defer = 0;
int g_HWUI_debug_display_list = 0;
int g_HWUI_debug_display_ops_as_events = 1;
int g_HWUI_debug_merge_behavior = 0;

//MTK debug dump functions
int g_HWUI_debug_texture_tracker = 0;
int g_HWUI_debug_duration = 0;
int g_HWUI_debug_dumpDisplayList = 0;
int g_HWUI_debug_dumpDraw = 0;
int g_HWUI_debug_dumpTexture = 0;
int g_HWUI_debug_dumpAlphaTexture = 0;
int g_HWUI_debug_dumpLayer = 0;
int g_HWUI_debug_dumpTextureLayer = 0;
int g_HWUI_debug_enhancement = 1;

//MTK sync with egl trace
int g_HWUI_debug_egl_trace = 0;

#else /* defined(MTK_ENG_BUILD) */

// keep usedebug load log off
int g_HWUI_debug_opengl = 1;
int g_HWUI_debug_extensions = 0;
int g_HWUI_debug_init = 0;
int g_HWUI_debug_memory_usage = 0;
int g_HWUI_debug_cache_flush = 1;
int g_HWUI_debug_layers_as_regions = 0;
int g_HWUI_debug_clip_regions = 0;
int g_HWUI_debug_programs = 0;
int g_HWUI_debug_layers = 0;
int g_HWUI_debug_render_buffers = 0;
int g_HWUI_debug_stencil = 0;
int g_HWUI_debug_patches = 0;
int g_HWUI_debug_patches_vertices = 0;
int g_HWUI_debug_patches_empty_vertices = 0;
int g_HWUI_debug_paths = 0;
int g_HWUI_debug_textures = 0;
int g_HWUI_debug_layer_renderer = 0;
int g_HWUI_debug_font_renderer = 0;
int g_HWUI_debug_defer = 0;
int g_HWUI_debug_display_list = 0;
int g_HWUI_debug_display_ops_as_events = 0;
int g_HWUI_debug_merge_behavior = 0;

//MTK debug dump functions
int g_HWUI_debug_texture_tracker = 0;
int g_HWUI_debug_duration = 0;
int g_HWUI_debug_dumpDisplayList = 0;
int g_HWUI_debug_dumpDraw = 0;
int g_HWUI_debug_dumpTexture = 0;
int g_HWUI_debug_dumpAlphaTexture = 0;
int g_HWUI_debug_dumpLayer = 0;
int g_HWUI_debug_dumpTextureLayer = 0;
int g_HWUI_debug_enhancement = 1;

//MTK sync with egl trace
int g_HWUI_debug_egl_trace = 0;

#endif /* defined(MTK_ENG_BUILD) */


#if defined(MTK_DEBUG_RENDERER)

namespace android {

#ifdef USE_OPENGL_RENDERER
using namespace uirenderer;
ANDROID_SINGLETON_STATIC_INSTANCE(TextureTracker);
#endif

namespace uirenderer {

#define TTLOGD(...) \
{                            \
    if (g_HWUI_debug_texture_tracker) \
        ALOGD(__VA_ARGS__); \
}

class Barrier {
    public:
        Barrier(Condition::WakeUpType type = Condition::WAKE_UP_ALL) : mType(type), mSignaled(false) { }
        ~Barrier() { }

        void signal() {
            Mutex::Autolock l(mLock);
            mSignaled = true;
            mCondition.signal(mType);
        }

        void wait() {
            Mutex::Autolock l(mLock);
            while (!mSignaled) {
                mCondition.wait(mLock);
            }
            mSignaled = false;
        }

    private:
        Condition::WakeUpType mType;
        volatile bool mSignaled;
        mutable Mutex mLock;
        mutable Condition mCondition;
};

class DumpTask {
public:
    const static float TARGET_SIZE = 102480; // 240 * 427
    DumpTask(int w, int h, const char* f, bool c):
        width(w), height(h), size(width * height * 4), compress(c) {
        memcpy(filename, f, 512);
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bitmap.allocPixels();
    }

    DumpTask(SkBitmap* b, const char* f, bool c):
        width(b->width()), height(b->height()), size(b->getSize()), compress(c) {
        memcpy(filename, f, 512);

        uint64_t start = systemTime(SYSTEM_TIME_MONOTONIC);
        b->copyTo(&bitmap, SkBitmap::kARGB_8888_Config);
        uint64_t end = systemTime(SYSTEM_TIME_MONOTONIC);
        ALOGD("copyTo %dx%d, time %dms", width, height, (int) ((end - start) / 1000000));
    }

    ~DumpTask() {
    }

    void preProcess() {
        if (bitmap.readyToDraw() && compress) {
            uint64_t start = systemTime(SYSTEM_TIME_MONOTONIC);
            float ratio = sqrt(TARGET_SIZE / width / height) ;

            if (ratio < 1) {
                int w = (int)(width * ratio + 0.5);;
                int h = (int)(height * ratio + 0.5);
                SkBitmap dst;
                dst.setConfig(bitmap.config(), w, h);
                dst.allocPixels();
                dst.eraseColor(0);

                SkPaint paint;
                paint.setFilterBitmap(true);

                SkCanvas canvas(dst);
                canvas.scale(ratio, ratio);
                canvas.drawBitmap(bitmap, 0.0f, 0.0f, &paint);
                dst.copyTo(&bitmap, SkBitmap::kARGB_8888_Config);
                uint64_t end = systemTime(SYSTEM_TIME_MONOTONIC);
                ALOGD("scale ratio %f, %dx%d, time %dms", ratio, bitmap.width(), bitmap.height(), (int) ((end - start) / 1000000));
            } else {
                ALOGD("scale ratio %f >= 1, %dx%d not needed", ratio, bitmap.width(), bitmap.height());
            }

        }
    }

    void onProcess() {
        uint64_t start = systemTime(SYSTEM_TIME_MONOTONIC);
        if (!SkImageEncoder::EncodeFile(filename, bitmap, SkImageEncoder::kPNG_Type, 40))
        {
            ALOGE("Failed to encode image %s\n", filename);

        }
        uint64_t end = systemTime(SYSTEM_TIME_MONOTONIC);
        ALOGD("encodeFile %dx%d, time %dms", bitmap.width(), bitmap.height(), (int) ((end - start) / 1000000));
    }

    int width;
    int height;
    size_t size;
    char filename[512];
    SkBitmap bitmap;
    bool compress;
};

class Dumper {
public:
    const static float MAX_BUFFER_SIZE = 64 * 1048576; // max 64MB for all threads
    Dumper() {
        // Get the number of available CPUs. This value does not change over time.
        mThreadCount = sysconf(_SC_NPROCESSORS_CONF) / 2;
        ALOGD("Dumper init %d threads <%p>", mThreadCount, this);

        for (int i = 0; i < mThreadCount; i++) {
            String8 name;
            name.appendFormat("HwuiDumperThread%d", i + 1);
            mThreads.add(new DumperThread(name));
        }
    }
    ~Dumper() {
        for (size_t i = 0; i < mThreads.size(); i++) {
            mThreads[i]->exit();
        }
    }

    bool addTask(DumpTask *task) {
        task->preProcess();
        if (mThreads.size() > 0) {
            size_t minQueueSize = MAX_BUFFER_SIZE / mThreadCount / task->bitmap.getSize();
            sp<DumperThread> thread;

            for (size_t i = 0; i < mThreads.size(); i++) {
                if (mThreads[i]->getTaskCount() < minQueueSize) {
                    thread = mThreads[i];
                    minQueueSize = mThreads[i]->getTaskCount();
                }
            }

            if (thread.get() == NULL)
                return false;

            return thread->addTask(task);
        }
        return false;
    }

private:
    class DumperThread: public Thread {
    public:
        DumperThread(const String8 name): mSignal(Condition::WAKE_UP_ONE), mName(name) { }

        bool addTask(DumpTask *task) {
            if (!isRunning()) {
                run(mName.string(), PRIORITY_DEFAULT);
            }

            Mutex::Autolock l(mLock);
            ssize_t index = mTasks.add(task);
            mSignal.signal();

            return index >= 0;
        }
        size_t getTaskCount() const {
            Mutex::Autolock l(mLock);
            return mTasks.size();
        }
        void exit() {
            {
                Mutex::Autolock l(mLock);
                for (size_t i = 0; i < mTasks.size(); i++) {
                    const DumpTask* task = mTasks.itemAt(i);
                    delete task;
                }
                mTasks.clear();
            }
            requestExit();
            mSignal.signal();
        }

    private:
        virtual bool threadLoop() {
            mSignal.wait();
            Vector<DumpTask*> tasks;
            {
                Mutex::Autolock l(mLock);
                tasks = mTasks;
                mTasks.clear();
            }

            for (size_t i = 0; i < tasks.size(); i++) {
                DumpTask* task = tasks.itemAt(i);
                task->onProcess();
                delete task;
            }
            return true;
        }

        // Lock for the list of tasks
        mutable Mutex mLock;
        Vector<DumpTask *> mTasks;

        // Signal used to wake up the thread when a new
        // task is available in the list
        mutable Barrier mSignal;

        const String8 mName;
    };

    int mThreadCount;

    Vector<sp<DumperThread> > mThreads;
};

///////////////////////////////////////////////////////////////////////////////
// Constructors/destructor
///////////////////////////////////////////////////////////////////////////////

TextureTracker::TextureTracker() : dumper(NULL) {
    TTLOGD("[TT]TextureTracker +");
    dumper = new Dumper();
    mPid = getpid();
}

TextureTracker::~TextureTracker() {
    TTLOGD("[TT]TextureTracker -");
    delete dumper;
}

///////////////////////////////////////////////////////////////////////////////
// Monitoring
///////////////////////////////////////////////////////////////////////////////

void TextureTracker::startMark(String8 name) {
    Mutex::Autolock _l(mLock);
    mViews.push(name);
}

void TextureTracker::endMark() {
    Mutex::Autolock _l(mLock);
    mViews.pop();
}

void TextureTracker::add(int textureId, int w, int h, int format, int type, String8 purpose, const char* comment) {
    Mutex::Autolock _l(mLock);

    if (mViews.size() == 0) {
        if (comment != NULL) {
            ALOGE("[TT]add error %s %d %d %d 0x%x 0x%x %s", comment, textureId, w, h, format, type, purpose.string());
        } else {
            ALOGE("[TT]add error %d %d %d 0x%x 0x%x %s", textureId, w, h, format, type, purpose.string());
        }
        return;
    }
    TextureEntry entry(mViews.top(), textureId, w, h, format, type, purpose);
    mMemoryList.add(entry);


    if (comment != NULL) {
        TTLOGD("[TT]%s %s %d %d %d 0x%x 0x%x => %d %s", comment, entry.mName.string(), textureId, w, h, format, type, entry.mMemory, purpose.string());
    }

}

void TextureTracker::add(String8 name, int textureId, int w, int h, int format, int type, String8 purpose, const char* comment) {
    Mutex::Autolock _l(mLock);
    TextureEntry entry(name, textureId, w, h, format, type, purpose);
    mMemoryList.add(entry);

    if (comment != NULL) {
        TTLOGD("[TT]%s %s %d %d %d 0x%x 0x%x => %d %s", comment, name.string(), textureId, w, h, format, type, entry.mMemory, purpose.string());
    }
}

void TextureTracker::remove(int textureId, const char* comment) {
    Mutex::Autolock _l(mLock);
    TextureEntry entry(textureId);
    ssize_t index = mMemoryList.indexOf(entry);

    if (index >= 0) {
        entry = mMemoryList.itemAt(index);
        mMemoryList.removeAt(index);

        TTLOGD("[TT]%s %s %d", comment, entry.mName.string(), textureId);
    } else {
        TTLOGD("[TT]%s already %d", comment, textureId);
    }

}

void TextureTracker::update(int textureId, bool ghost, String8 name) {
    Mutex::Autolock _l(mLock);
    TextureEntry entry(textureId);
    ssize_t index = mMemoryList.indexOf(entry);

    if (index >= 0) {
        TextureEntry& item = mMemoryList.editItemAt(index);
        TTLOGD("[TT]update before %s %d %d %d %d %d\n", item.mName.string(), item.mId, item.mWidth, item.mHeight,
                        item.mMemory, item.mGhost);

        item.mGhost = ghost;

        if (name.isEmpty()) {
            if (!ghost) {
                item.mName = mViews.top();
            }
        } else {
            item.mName = name;
        }


        entry = mMemoryList.itemAt(index);
        TTLOGD("[TT]update after %s %d %d %d %d %d\n", entry.mName.string(), entry.mId, entry.mWidth, entry.mHeight,
                    entry.mMemory, entry.mGhost);
    } else {
        TTLOGD("[TT]update not found %d", textureId);
    }

}

int TextureTracker::estimateMemory(int w, int h, int format, int type) {

    int power2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    int width = 0;
    int height = 0;
    int i;
    for (i = 0; i < 14; ++i) {
        if (power2[i] >= w)
            break;
    }
    if (i < 14) {
        width = power2[i];
    }

    for (i = 0; i < 14; ++i) {
        if (power2[i] >= h)
            break;
    }
    if (i < 14) {
        height = power2[i];
    }


    int bytesPerPixel = 0;

    switch (type) {
        case GL_UNSIGNED_BYTE:
            switch (format) {
                case GL_RGBA:
                    bytesPerPixel = 4;
                    break;
                case GL_RGB:
                    bytesPerPixel = 3;
                    break;
                case GL_LUMINANCE_ALPHA:
                    bytesPerPixel = 2;
                    break;
                case GL_ALPHA:
                case GL_LUMINANCE:
                    bytesPerPixel = 1;
                    break;
                default:
                    ALOGE("[TT]estimateMemory Error!! type:0x%x, format:0x%x", type, format);
                    break;
            }
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4: // GL_RGBA format
        case GL_UNSIGNED_SHORT_5_5_5_1: // GL_RGBA format
        case GL_UNSIGNED_SHORT_5_6_5:   // GL_RGB
            bytesPerPixel = 2;
            break;
        case GL_FLOAT:
            switch (format) {
                case GL_RED:
                    bytesPerPixel = 2;
                    break;
                case GL_RGBA:
                    bytesPerPixel = 8;
                    break;
                default:
                    ALOGE("[TT]estimateMemory Error!! type:0x%x, format:0x%x", type, format);
                    break;
            }
            break;
        default:
            ALOGE("[TT]estimateMemory Error!! type:0x%x, format:0x%x", type, format);
            break;
    }

    int size = 0;

    if (bytesPerPixel != 0) {
        size = width * height * bytesPerPixel;
        size = size < 4096 ? 4096 : size;
    }

    return size;

}

void TextureTracker::dumpMemoryUsage(String8 &log) {
    // Reload properties here, so we can dynamically switch log on/off by
    // adb shell dumpsys gfxinfo PID
    setDebugLog(true);

    log.appendFormat("\nTextureTracker:\n");

    int sum = 0;
    SortedList<String8> list;
    size_t count = mMemoryList.size();
    for (size_t i = 0; i < count; i++) {
        const String8& current = mMemoryList.itemAt(i).mName;
        size_t tmp = list.size();
        bool found = false;
        for (size_t j = 0; j < tmp; j++) {
            if (current == list.itemAt(j)) {
                found = true;
                break;
            }
        }
        if (!found) {
            list.add(current);
        }
    }
    size_t tmp = list.size();

    for (size_t i = 0; i < tmp; i++) {
        const String8& current = list.itemAt(i);
        String8 tmpString;
        int tmpsum = 0;
        for (size_t j = 0; j < count; j++) {
            const TextureEntry& entry = mMemoryList.itemAt(j);
            if (entry.mName == current) {
                String8 format;
                String8 type;
                char s[64];

                switch (entry.mFormat) {
                    case GL_RGBA:
                        format = String8("GL_RGBA");
                        break;
                    case GL_RGB:
                        format = String8("GL_RGB");
                        break;
                    case GL_ALPHA:
                        format = String8("GL_ALPHA");
                        break;
                    case GL_LUMINANCE:
                        format = String8("GL_LUMINANCE");
                        break;
                    case GL_LUMINANCE_ALPHA:
                        format = String8("GL_LUMINANCE_ALPHA");
                        break;
                    default:
                        sprintf(s, "0x%x", entry.mFormat);
                        format = String8(s);
                        break;
                }

                switch (entry.mType) {
                    case GL_UNSIGNED_BYTE:
                        type = String8("GL_UNSIGNED_BYTE");
                        break;
                    case GL_UNSIGNED_SHORT_4_4_4_4:
                        type = String8("GL_UNSIGNED_SHORT_4_4_4_4");
                        break;
                    case GL_UNSIGNED_SHORT_5_5_5_1:
                        type = String8("GL_UNSIGNED_SHORT_5_5_5_1");
                        break;
                    case GL_UNSIGNED_SHORT_5_6_5:
                        type = String8("GL_UNSIGNED_SHORT_5_6_5");
                        break;
                    case GL_FLOAT:
                        type = String8("GL_FLOAT");
                        break;
                    default:
                        sprintf(s, "0x%x", entry.mType);
                        type = String8(s);
                        break;
                }

                tmpString.appendFormat("        %d (%d, %d) (%s, %s) %d <%s> %s\n", entry.mId, entry.mWidth,
                                        entry.mHeight, format.string(), type.string(), entry.mMemory,
                                        entry.mPurpose.string(), entry.mGhost ? "g" : "");
                tmpsum += entry.mMemory;
            }
        }

        sum += tmpsum;
        log.appendFormat("%s: %d bytes, %.2f KB, %.2f MB\n", current.string(), tmpsum, tmpsum / 1024.0f, tmpsum / 1048576.0f);
        log.append(tmpString);
        log.append("\n");
    }

    log.appendFormat("\nTotal monitored:\n  %d bytes, %.2f KB, %.2f MB\n", sum, sum / 1024.0f, sum / 1048576.0f);
}

static bool dumpImage(int width, int height, const char *filename)
{
    DumpTask* task = new DumpTask(width, height, filename, true);
    GLenum error;
    uint64_t start = systemTime(SYSTEM_TIME_MONOTONIC);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, task->bitmap.getPixels());
    uint64_t end = systemTime(SYSTEM_TIME_MONOTONIC);
    ALOGD("%s: readpixel %dx%d time %dms", __FUNCTION__, width, height, (int) ((end - start) / 1000000));

    if ((error = glGetError()) != GL_NO_ERROR)
    {
        ALOGE("%s: get GL error 0x%x \n", __FUNCTION__, error);
        delete task;
        return false;
    }

    if (TextureTracker::getInstance().dumper->addTask(task)) {
        // dumper will help to delete task when task finished
    } else {
        task->onProcess();
        delete task;
    }

    return true;
}

bool dumpDisplayList(int width, int height, int frameCount, void* renderer)
{
    char procName[256];
    char file[512];

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    sprintf(file, "/data/data/%s/dp_%p_%09d.png", pch, renderer, frameCount);

    ALOGD("%s: %dx%d %s", __FUNCTION__, width, height, file);
    return dumpImage(width, height, file);
}

bool dumpDraw(int width, int height, int frameCount, int index, void* renderer, void* drawOp, int sub)
{
    char procName[256];
    char file[512];

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    sprintf(file, "/data/data/%s/draw_%p_%09d_%02d_%02d_%p.png", pch, renderer, frameCount, index, sub, drawOp);

    ALOGD("%s: %dx%d %s", __FUNCTION__, width, height, file);
    return dumpImage(width, height, file);
}

bool dumpLayer(int width, int height, int fbo, int frameCount, void* renderer, void* layer)
{
    char procName[256];
    char file[512];

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    uint64_t time = systemTime(SYSTEM_TIME_MONOTONIC);
    sprintf(file, "/data/data/%s/layer_%p_%p_%d_%dx%d_%09d_%09u.png", pch, renderer, layer, fbo, width, height, frameCount, (unsigned int) time / 1000);

    ALOGD("%s: %dx%d %s", __FUNCTION__, width, height, file);
    return dumpImage(width, height, file);
}

bool dumpTexture(int texture, int width, int height, SkBitmap *bitmap, bool isLayer)
{
    char procName[256];
    char file[512];

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    if (isLayer) {
        uint64_t time = systemTime(SYSTEM_TIME_MONOTONIC);
        sprintf(file, "/data/data/%s/texLayer_%d_%dx%d_%u.png", pch,texture, width, height, (unsigned int) time / 1000);
    } else {
        sprintf(file, "/data/data/%s/tex_%d_%dx%d_%p.png", pch, texture, width, height, bitmap);
    }

    ALOGD("%s: %dx%d %s", __FUNCTION__, width, height, file);

    DumpTask* task = new DumpTask(bitmap, file, isLayer);
    if (!task->bitmap.readyToDraw()) {
        ALOGE("%s: failed to copy bitmap %p\n", __FUNCTION__, bitmap);
        delete task;
        return false;
    }

    if (TextureTracker::getInstance().dumper->addTask(task)) {
        // dumper will help to delete task when task finished
    } else {
        task->onProcess();
        delete task;
    }

    return true;
}

bool dumpAlphaTexture(int width, int height, uint8_t *data, const char *prefix, SkBitmap::Config format)
{
    static int count = 0;
    char procName[256];
    char file[512];
    SkBitmap bitmap;
    SkBitmap bitmapCopy;

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    sprintf(file, "/data/data/%s/%s_%04d.png", procName, prefix, count++);

    ALOGD("%s: %dx%d %s", __FUNCTION__, width, height, file);

    bitmap.setConfig(format, width, height);
    bitmap.setPixels(data, NULL);

    DumpTask* task = new DumpTask(&bitmap, file, false);

    if (!task->bitmap.readyToDraw()) {
        ALOGE("%s: failed to copy data %p", __FUNCTION__, data);
        delete task;
        return false;
    }

    if (TextureTracker::getInstance().dumper->addTask(task)) {
        // dumper will help to delete task when task finished
    } else {
        task->onProcess();
        delete task;
    }

    return true;
}

}; // namespace uirenderer
}; // namespace android

#endif /* defined(MTK_DEBUG_RENDERER) */
