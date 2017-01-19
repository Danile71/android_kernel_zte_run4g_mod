#define LOG_TAG "BufferQueue"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#include <cmath>

#include <cutils/properties.h>

#include <binder/IPCThreadState.h>

#include <ui/gralloc_extra.h>
#include <ui_ext/GraphicBufferUtil.h>

#include <gui/BufferQueue.h>
#include <gui/IConsumerListener.h>

#include <gui/mediatek/FpsCounter.h>
#include <gui/mediatek/BufferQueueDebug.h>
#include <gui/mediatek/BufferQueueDump.h>

#define ST_LOGV(x, ...) do { \
    XLOGV("[%s](this:%p,id:%d,api:%d,p:%d,c:%d) "x, mConsumerName.string(), mBq.unsafe_get(), mId, mConnectedApi, mProducerPid, mConsumerPid, ##__VA_ARGS__); \
} while(0)

#define ST_LOGD(x, ...) do { \
    XLOGD("[%s](this:%p,id:%d,api:%d,p:%d,c:%d) "x, mConsumerName.string(), mBq.unsafe_get(), mId, mConnectedApi, mProducerPid, mConsumerPid, ##__VA_ARGS__); \
} while(0)


#define ST_LOGI(x, ...) do { \
    XLOGI("[%s](this:%p,id:%d,api:%d,p:%d,c:%d) "x, mConsumerName.string(), mBq.unsafe_get(), mId, mConnectedApi, mProducerPid, mConsumerPid, ##__VA_ARGS__); \
} while(0)

#define ST_LOGD(x, ...) do { \
    XLOGD("[%s](this:%p,id:%d,api:%d,p:%d,c:%d) "x, mConsumerName.string(), mBq.unsafe_get(), mId, mConnectedApi, mProducerPid, mConsumerPid, ##__VA_ARGS__); \
} while(0)


#define ST_LOGE(x, ...) do { \
    XLOGE("[%s](this:%p,id:%d,api:%d,p:%d,c:%d) "x, mConsumerName.string(), mBq.unsafe_get(), mId, mConnectedApi, mProducerPid, mConsumerPid, ##__VA_ARGS__); \
} while(0)

namespace android {
// -----------------------------------------------------------------------------
ANDROID_SINGLETON_STATIC_INSTANCE(BufferQueueProxy)

sp<BufferQueueDebug> BufferQueueProxy::createEntry() {
    return new BufferQueueDebug();
}
// -----------------------------------------------------------------------------

BufferQueueDebug::BufferQueueDebug() :
    mBq(NULL),
    mId(-1),
    mPid(-1),
    mProducerPid(-1),
    mConsumerPid(-1),
    mConnectedApi(BufferQueue::NO_CONNECTED_API),
    mIsInSF(false),
    mIsInGuiExt(false),
    mLine(false),
    mLineCnt(0),
    mDump(NULL),
    mGeneralDump(false)
{
}

status_t getProcessName(int pid, String8& name) {
    FILE *fp = fopen(String8::format("/proc/%d/cmdline", pid), "r");
    if (NULL != fp) {
        const size_t size = 64;
        char proc_name[size];
        fgets(proc_name, size, fp);
        fclose(fp);

        name = proc_name;
        return NO_ERROR;
    }

    return INVALID_OPERATION;
}

void BufferQueueDebug::onConstructor(
        wp<BufferQueue> bq, const int32_t id, const String8& consumerName) {
    mBq = bq;
    mId = id;
    mPid = getpid();
    mConsumerName = consumerName;

    if (NO_ERROR == getProcessName(mPid, mConsumerProcName)) {
        ST_LOGI("BufferQueue queue=(%d:%s)", mPid, mConsumerProcName.string());
    } else {
        ST_LOGI("BufferQueue queue=(%d:\?\?\?)", mPid);
    }
    mIsInSF = (mConsumerProcName == String8("/system/bin/surfaceflinger"));
    mIsInGuiExt = (mConsumerProcName.find("guiext-server") != -1);
    mDump = new BufferQueueDump(mIsInGuiExt ?
            BufferQueueDump::TRACK_PRODUCER : BufferQueueDump::TRACK_CONSUMER, mBq.unsafe_get());

    if (mDump == NULL) {
        ST_LOGE("new BufferQueueDump() failed in BufferQueue()");
    }
    // update dump name
    mDump->setName(consumerName);

    // check property for drawing debug line
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.bq.line", value, "GOD'S IN HIS HEAVEN, ALL'S RIGHT WITH THE WORLD.");
    mLine = (-1 != consumerName.find(value));
    mLineCnt = 0;

    if (true == mLine) {
        ST_LOGI("switch on debug line");
    }

    // check property for general buffer queue
    property_get("debug.bq.general", value, "0");
    mGeneralDump = (atoi(value) == 1);

    if (mGeneralDump && !mIsInSF && !mIsInGuiExt) {
        DumpTunnelHelper::getInstance().regDump(
                mDump, String8::format("BQ-%s-%p", consumerName.string(), mBq.unsafe_get()));
        mDump->setRegistered(true);
    }
}

void BufferQueueDebug::onDestructor() {
    // mGeneralDump cannot be checked here, or unregDump() is not executed.
    // this makes memory leak at GuiExtService
    if (mDump->isRegistered()) {
        DumpTunnelHelper::getInstance().unregDump(String8::format("BQ-%s-%p", mConsumerName.string(), mBq.unsafe_get()));
        mDump->setRegistered(false);
    }
}

void BufferQueueDebug::onSetConsumerName(const String8& consumerName) {
    mConsumerName = consumerName;
    // update dump info
    mDump->setName(mConsumerName);

    // check property for drawing debug line
    ST_LOGI("setConsumerName: %s", mConsumerName.string());
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.bq.line", value, "GOD'S IN HIS HEAVEN, ALL'S RIGHT WITH THE WORLD.");
    mLine = (-1 != mConsumerName.find(value));
    mLineCnt = 0;

    if (true == mLine) {
        ST_LOGI("switch on debug line");
    }
}

void BufferQueueDebug::setIonInfoOnDequeue(const sp<GraphicBuffer>& gb) {
#ifndef MTK_EMULATOR_SUPPORT
    // need to watch
    const uint32_t ION_MM_DBG_NAME_LEN = 16;
    char infoMsg[ION_MM_DBG_NAME_LEN];
    snprintf(infoMsg, sizeof(infoMsg), "p:%d c:%d", mProducerPid, mConsumerPid);
    gralloc_extra_setBufInfo(gb->handle, infoMsg);
#endif // MTK_EMULATOR_SUPPORT
}

void BufferQueueDebug::onDequeue(
        const int outBuf, sp<GraphicBuffer>& gb, sp<Fence>& fence) {
    mDump->onDequeueBuffer(outBuf, gb, fence);
}

void BufferQueueDebug::onQueue(const int buf, const int64_t timestamp) {
    // count FPS after queueBuffer() success, for producer side
    if (true == mQueueFps.update()) {
        ST_LOGI("queueBuffer: fps=%.2f dur=%.2f max=%.2f min=%.2f",
                mQueueFps.getFps(),
                mQueueFps.getLastLogDuration() / 1e6,
                mQueueFps.getMaxDuration() / 1e6,
                mQueueFps.getMinDuration() / 1e6);
    }

    // also inform queueBuffer to mDump
    mDump->onQueueBuffer(buf, timestamp);
}

void BufferQueueDebug::onCancel(const int buf) {
    mDump->onCancelBuffer(buf);
}

void BufferQueueDebug::onConnect(
        const sp<IBinder>& token, const int api, bool producerControlledByApp) {
    mProducerPid = (token != NULL && NULL != token->localBinder())
        ? getpid()
        : IPCThreadState::self()->getCallingPid();
    mConnectedApi = api;

    if (NO_ERROR == getProcessName(mProducerPid, mProducerProcName)) {
        ST_LOGI("connect: api=%d producer=(%d:%s) producerControlledByApp=%s", mConnectedApi,
                mProducerPid, mProducerProcName.string(), producerControlledByApp ? "true" : "false");
    } else {
        ST_LOGI("connect: api=%d producer=(%d:\?\?\?) producerControlledByApp=%s", mConnectedApi,
                mProducerPid, producerControlledByApp ? "true" : "false");
    }
}

void BufferQueueDebug::onDump(String8 &result, const String8& prefix) {
    mDump->dump(result, prefix.string());
}

void BufferQueueDebug::onFreeBufferLocked(const int slot) {
    mDump->onFreeBuffer(slot);
}

status_t drawDebugLineToGraphicBuffer(
        const sp<GraphicBuffer>& gb, uint32_t cnt, uint8_t val = 0xff) {
#ifndef MTK_EMULATOR_SUPPORT
    const uint32_t DEFAULT_LINE_W = 4;
    const uint32_t DEFAULT_LINE_H = 4;
    if (gb == NULL) {
        return INVALID_OPERATION;
    }

    int line_number_w = DEFAULT_LINE_W;
    int line_number_h = DEFAULT_LINE_H;
    int line_w = DEFAULT_LINE_W;
    int line_h = DEFAULT_LINE_H;

    char value[PROPERTY_VALUE_MAX];
    property_get("debug.bq.line_p", value, "-1");
    int line_pos = atoi(value);
    if (line_pos >= 0)
        cnt = line_pos;

    property_get("debug.bq.line_g", value, "-1");
    sscanf(value, "%d:%d", &line_w, &line_h);
    if (line_w > 0)
        line_number_w = line_w;
    if (line_h > 0)
        line_number_h = line_h;

    property_get("debug.bq.line_c", value, "-1");
    int8_t line_c = atoi(value);
    if (line_c >= 0)
        val = line_c;

    getGraphicBufferUtil().drawLine(gb, val, line_number_w, line_number_h, cnt);
#endif
    return NO_ERROR;
}

void BufferQueueDebug::onAcquire(
        const int buf,
        const sp<GraphicBuffer>& gb,
        const sp<Fence>& fence,
        const int64_t timestamp,
        const BufferQueue::BufferItem* const buffer) {
    // also inform acquireBuffer to mDump
    mDump->onAcquireBuffer(buf, gb, fence, timestamp);

    // draw white debug line
    if (true == mLine) {
        if (buffer->mFence.get())
            buffer->mFence->waitForever("BufferItemConsumer::acquireBuffer");

        drawDebugLineToGraphicBuffer(gb, mLineCnt);
        mLineCnt += 1;
    }
}

void BufferQueueDebug::onRelease(const int buf) {
    // also inform releaseBuffer to mDump
    mDump->onReleaseBuffer(buf);
}

void BufferQueueDebug::onConsumerConnect(
        const sp<IConsumerListener>& consumerListener,
        const bool controlledByApp) {
    // check if local or remote connection by the consumer listener
    // (in most cases, consumer side is a local connection)
    mConsumerPid = (NULL != consumerListener->asBinder()->localBinder())
                 ? getpid()
                 : IPCThreadState::self()->getCallingPid();

    if (NO_ERROR == getProcessName(mConsumerPid, mConsumerProcName)) {
        ST_LOGI("consumerConnect consumer=(%d:%s) controlledByApp=%s",
            mConsumerPid, mConsumerProcName.string(), controlledByApp ? "true" : "false");
    } else {
        ST_LOGI("consumerConnect consumer=(%d:\?\?\?) controlledByApp=%s",
            mConsumerPid, controlledByApp ? "true" : "false");
    }
}

void BufferQueueDebug::onConsumerDisconnectHead() {
    mConsumerPid = -1;
}

void BufferQueueDebug::onConsumerDisconnectTail() {
    mDump->onConsumerDisconnect();
}

void BufferQueueDebug::onDisconnect() {
    mProducerPid = -1;
}

// -----------------------------------------------------------------------------
}; // namespace android
