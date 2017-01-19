#define LOG_TAG "BufferQueue"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <gui/BufferQueue.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>

// TODO: check whether emulator support GraphicBufferUtil
#include <ui_ext/GraphicBufferUtil.h>

#include <gui/mediatek/BufferQueueDump.h>

// ----------------------------------------------------------------------------
#define PROP_DUMP_NAME      "debug.bq.dump"
#define PROP_DUMP_BUFCNT    "debug.bq.bufscnt"
#define DEFAULT_DUMP_NAME   "GOD'S IN HIS HEAVEN, ALL'S RIGHT WITH THE WORLD."
#define DEFAULT_BUFCNT      "0"
#define STR_DUMPALL         "dump_all"
#define PREFIX_NODUMP       "[:::nodump:::]"
#define DUMP_FILE_PATH      "/data/SF_dump/"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define BQD_LOGV(x, ...) XLOGV("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGD(x, ...) XLOGD("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGI(x, ...) XLOGI("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGW(x, ...) XLOGW("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGE(x, ...) XLOGE("[%s] "x, mName.string(), ##__VA_ARGS__)


namespace android {

// ----------------------------------------------------------------------------
void BufferQueueDump::setName(const String8& name) {
    mName = name;

    // update dumper's name
    if (mBackupBufDumper != NULL) {
        mBackupBufDumper->setName(name);
    }

    // check and reset current dump setting
    checkBackupCount();
}

void BufferQueueDump::dump(String8& result, const char* prefix) {
    // dump status to log buffer first
    const char* bufName = (TRACK_PRODUCER == mMode) ? "Dequeued" : "Acquired";

    result.appendFormat("%s*BufferQueueDump mIsBackupBufInited=%d, m%sBufs(size=%d), mMode=%s\n",
        prefix, mIsBackupBufInited, bufName, mObtainedBufs.size(),
        (TRACK_PRODUCER == mMode) ? "TRACK_PRODUCER" : "TRACK_CONSUMER");

    if ((mLastObtainedBuf != NULL) && (mLastObtainedBuf->mGraphicBuffer != NULL)) {
        result.appendFormat("%s [-1] mLast%sBuf->mGraphicBuffer->handle=%p\n",
            prefix, bufName, mLastObtainedBuf->mGraphicBuffer->handle);
    }

    for (size_t i = 0; i < mObtainedBufs.size(); i++) {
        const sp<DumpBuffer>& buffer = mObtainedBufs[i];
        result.appendFormat("%s [%02u] handle=%p, fence=%p, time=%#llx\n",
            prefix, i, buffer->mGraphicBuffer->handle, buffer->mFence.get(), buffer->mTimeStamp);
    }

    // start buffer dump check and process
    String8 name;
    String8 name_prefix;

    char value[PROPERTY_VALUE_MAX];
    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);

    // check if prefix for no dump
    if (strstr(value, PREFIX_NODUMP) == value) {
        goto check;
    }

    // check if not dump for me
    if (!((!strcmp(value, STR_DUMPALL)) || (-1 != mName.find(value)))) {
        goto check;
    }

    // at first, dump backup buffer
    if (mBackupBuf.getSize() > 0) {
        mBackupBuf.dump(result, prefix);
    }

    getDumpFileName(name, mName);

    // dump acquired buffers
    if (0 == mObtainedBufs.size()) {
        // if no acquired buf, try to dump the last one kept
        if (mLastObtainedBuf != NULL) {
            name_prefix = String8::format("[%s](LAST_ts%lld)",
                                          name.string(), ns2ms(mLastObtainedBuf->mTimeStamp));
            mLastObtainedBuf->dump(name_prefix);

            BQD_LOGD("[dump] LAYER, handle(%p)", mLastObtainedBuf->mGraphicBuffer->handle);
        }
    } else {
        // dump acquired buf old to new
        for (uint32_t i = 0; i < mObtainedBufs.size(); i++) {
            const sp<DumpBuffer>& buffer = mObtainedBufs[i];
            if (buffer->mGraphicBuffer != NULL) {
                name_prefix = String8::format("[%s](%s%02u_ts%lld)",
                                              name.string(), bufName, i, ns2ms(buffer->mTimeStamp));
                buffer->dump(name_prefix);

                BQD_LOGD("[dump] %s:%02u, handle(%p)", bufName, i, buffer->mGraphicBuffer->handle);
            }
        }
    }

check:
    checkBackupCount();
}

void BufferQueueDump::getDumpFileName(String8& fileName, const String8& name) {
    fileName = name;

    // check file name, filter out invalid chars
    const char invalidChar[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};
    size_t size = fileName.size();
    char *buf = fileName.lockBuffer(size);
    for (unsigned int i = 0; i < ARRAY_SIZE(invalidChar); i++) {
        for (size_t c = 0; c < size; c++) {
            if (buf[c] == invalidChar[i]) {
                // find invalid char, replace it with '_'
                buf[c] = '_';
            }
        }
    }
    fileName.unlockBuffer(size);
}

int BufferQueueDump::checkBackupCount() {
    char value[PROPERTY_VALUE_MAX];
    char *name = value;
    int count;

    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);

    if (strstr(value, PREFIX_NODUMP) == value) {
        // find prefix for no dump, skip it
        name = &value[strlen(PREFIX_NODUMP)];
    }

    if ((!strcmp(name, STR_DUMPALL)) || (-1 != mName.find(name))) {
        property_get(PROP_DUMP_BUFCNT, value, DEFAULT_BUFCNT);
        count = atoi(value);
    } else {
        count = 0;
    }

    if (count > 0) {
        // create backup buffer if needed
        if (!mIsBackupBufInited) {
            mBackupBufPusher = new BackupBufPusher(mBackupBuf);
            mBackupBufDumper = new BackupBufDumper(mBackupBuf);
            if ((mBackupBufPusher != NULL) && (mBackupBufDumper != NULL)) {
                mBackupBufDumper->setName(mName);
                sp< RingBuffer< sp<BackupBuffer> >::Pusher > proxyPusher = mBackupBufPusher;
                sp< RingBuffer< sp<BackupBuffer> >::Dumper > proxyDumper = mBackupBufDumper;
                mBackupBuf.setPusher(proxyPusher);
                mBackupBuf.setDumper(proxyDumper);
                mIsBackupBufInited = true;
            } else {
                mBackupBufPusher.clear();
                mBackupBufDumper.clear();
                count = 0;
                BQD_LOGE("[%s] create Backup pusher or dumper failed", __func__);
            }
        }

        // resize backup buffer
        mBackupBuf.resize(count);
    } else {
        mBackupBuf.resize(0);
    }

    return count;
}

void BufferQueueDump::addBuffer(const int& slot,
                                const sp<GraphicBuffer>& buffer,
                                const sp<Fence>& fence,
                                const int64_t& timestamp) {
    if (buffer == NULL) {
        return;
    }

    sp<DumpBuffer> v = mObtainedBufs.valueFor(slot);
    if (v == NULL) {
        sp<DumpBuffer> b = new DumpBuffer(buffer, fence, timestamp);
        mObtainedBufs.add(slot, b);
        mLastObtainedBuf = NULL;
    } else {
        BQD_LOGW("[%s] slot(%d) acquired, seems to be abnormal, just update ...", __func__, slot);
        v->mGraphicBuffer = buffer;
        v->mFence = fence;
        v->mTimeStamp = timestamp;
    }
}

void BufferQueueDump::updateBuffer(const int& slot, const int64_t& timestamp) {
    if (mBackupBuf.getSize() > 0) {
        const sp<DumpBuffer>& v = mObtainedBufs.valueFor(slot);
        if (v != NULL) {
            // push GraphicBuffer into backup buffer if buffer ever Acquired
            sp<BackupBuffer> buffer = NULL;
            if (timestamp != -1)
                buffer = new BackupBuffer(v->mGraphicBuffer, timestamp);
            else
                buffer = new BackupBuffer(v->mGraphicBuffer, v->mTimeStamp);
            mBackupBuf.push(buffer);
        }
    }

    // keep for the last one before removed
    if (1 == mObtainedBufs.size()) {
        if (timestamp != -1)
            mObtainedBufs[0]->mTimeStamp = timestamp;

        mLastObtainedBuf = mObtainedBufs[0];
    }
    mObtainedBufs.removeItem(slot);
}

void BufferQueueDump::onAcquireBuffer(const int& slot,
                                      const sp<GraphicBuffer>& buffer,
                                      const sp<Fence>& fence,
                                      const int64_t& timestamp) {
    if (TRACK_CONSUMER == mMode) {
        addBuffer(slot, buffer, fence, timestamp);
    }
}

void BufferQueueDump::onReleaseBuffer(const int& slot) {
    if (TRACK_CONSUMER == mMode)
        updateBuffer(slot);
}

void BufferQueueDump::onFreeBuffer(const int& slot) {
    if (TRACK_CONSUMER == mMode)
        updateBuffer(slot);
}

void BufferQueueDump::onDequeueBuffer(const int& slot,
                                      const sp<GraphicBuffer>& buffer,
                                      const sp<Fence>& fence) {
    if (TRACK_PRODUCER == mMode)
        addBuffer(slot, buffer, fence);
}

void BufferQueueDump::onQueueBuffer(const int& slot, const int64_t& timestamp) {
    if (TRACK_PRODUCER == mMode)
        updateBuffer(slot, timestamp);
}

void BufferQueueDump::onCancelBuffer(const int& slot) {
    if (TRACK_PRODUCER == mMode)
        updateBuffer(slot);
}


void BufferQueueDump::onConsumerDisconnect() {
    mName += "(consumer disconnected)";

    mBackupBuf.resize(0);
    mBackupBufPusher = NULL;
    mBackupBufDumper = NULL;
    mIsBackupBufInited = false;

    mObtainedBufs.clear();
    mLastObtainedBuf = NULL;
}

status_t BufferQueueDump::kickDump(String8& result, const char* prefix) {
    sp<BufferQueue> bq = mBq.promote();
    if (bq != NULL) {
        bq->dump(result, prefix);
        return NO_ERROR;
    } else {
        XLOGE("kickDump() failed because BufferQueue(%p) is dead", mBq.unsafe_get());
        return DEAD_OBJECT;
    }
}
// ----------------------------------------------------------------------------
bool BackupBufPusher::push(const sp<BackupBuffer>& in) {
    if ((in == NULL) || (in->mGraphicBuffer == NULL)) {
        return false;
    }

    sp<BackupBuffer>& buffer = editHead();

    // check property of GraphicBuffer, realloc if needed
    bool needCreate = false;
    if ((buffer == NULL) || (buffer->mGraphicBuffer == NULL)) {
        needCreate = true;
    } else {
        if ((buffer->mGraphicBuffer->width != in->mGraphicBuffer->width) ||
            (buffer->mGraphicBuffer->height != in->mGraphicBuffer->height) ||
            (buffer->mGraphicBuffer->format != in->mGraphicBuffer->format)) {
            needCreate = true;
            XLOGD("[%s] geometry changed, backup=(%d, %d, %d) => active=(%d, %d, %d)",
                __func__, buffer->mGraphicBuffer->width, buffer->mGraphicBuffer->height,
                buffer->mGraphicBuffer->format, in->mGraphicBuffer->width,
                in->mGraphicBuffer->height, in->mGraphicBuffer->format);
        }
    }

    if (needCreate) {
        sp<GraphicBuffer> newGraphicBuffer = new GraphicBuffer(
                                             in->mGraphicBuffer->width, in->mGraphicBuffer->height,
                                             in->mGraphicBuffer->format, in->mGraphicBuffer->usage);
        if (newGraphicBuffer == NULL) {
            XLOGE("[%s] alloc GraphicBuffer failed", __func__);
            return false;
        }

        if (buffer == NULL) {
            buffer = new BackupBuffer();
            if (buffer == NULL) {
                XLOGE("[%s] alloc BackupBuffer failed", __func__);
                return false;
            }
        }

        buffer->mGraphicBuffer = newGraphicBuffer;
    }

    int width = in->mGraphicBuffer->width;
    int height = in->mGraphicBuffer->height;
    int format = in->mGraphicBuffer->format;
    int usage = in->mGraphicBuffer->usage;
    int stride = in->mGraphicBuffer->stride;

    uint32_t bits = getGraphicBufferUtil().getBitsPerPixel(format);
    status_t err;

    // backup
    void *src;
    void *dst;
    err = in->mGraphicBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &src);
    if (err != NO_ERROR) {
        XLOGE("[%s] lock GraphicBuffer failed", __func__);
        return false;
    }

    err = buffer->mGraphicBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN, &dst);
    if (err != NO_ERROR) {
        in->mGraphicBuffer->unlock();
        XLOGE("[%s] lock backup buffer failed", __func__);
        return false;
    }

    memcpy(dst, src, (stride * height * bits) >> 3);

    buffer->mGraphicBuffer->unlock();
    in->mGraphicBuffer->unlock();

    // update timestamp
    buffer->mTimeStamp = in->mTimeStamp;
    buffer->mSourceHandle = in->mGraphicBuffer->handle;

    return true;
}

// ----------------------------------------------------------------------------
void BackupBufDumper::dump(String8 &result, const char* prefix) {
    // dump status to log buffer first
    result.appendFormat("%s*BackupBufDumper mRingBuffer(size=%u, count=%u)\n",
        prefix, mRingBuffer.getSize(), mRingBuffer.getCount());

    for (size_t i = 0; i < mRingBuffer.getValidSize(); i++) {
        const sp<BackupBuffer>& buffer = getItem(i);
        result.appendFormat("%s [%02u] handle(source=%p, backup=%p)\n",
            prefix, i, buffer->mSourceHandle, buffer->mGraphicBuffer->handle);
    }

    // start buffer dump check and process
    String8 name;
    String8 name_prefix;

    BufferQueueDump::getDumpFileName(name, mName);

    for (size_t i = 0; i < mRingBuffer.getValidSize(); i++) {
        const sp<BackupBuffer>& buffer = getItem(i);
        name_prefix = String8::format("[%s](Backup%02u_H%p_ts%lld)",
                                      name.string(), i, buffer->mSourceHandle, ns2ms(buffer->mTimeStamp));
        getGraphicBufferUtil().dump(buffer->mGraphicBuffer, name_prefix.string(), DUMP_FILE_PATH);

        BQD_LOGI("[dump] Backup:%02u, handle(source=%p, backup=%p)",
            i, buffer->mSourceHandle, buffer->mGraphicBuffer->handle);
    }
}

// ----------------------------------------------------------------------------
void DumpBuffer::dump(const String8& prefix) {
    if (mFence != NULL) {
        mFence->waitForever(__func__);
    }
    getGraphicBufferUtil().dump(mGraphicBuffer, prefix.string(), DUMP_FILE_PATH);
}

}