#ifndef ANDROID_GUI_BUFFERQUEUEDEBUG_H
#include <utils/Singleton.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>

#include <ui_ext/RWTable.h>

namespace android {
// ----------------------------------------------------------------------------

class String8;
class BufferQueueDump;
class BufferQueue;
class FpsCounter;
struct BufferQueueDebug : public RefBase {
    wp<BufferQueue> mBq;
    int32_t mId;
    int32_t mPid;
    int32_t mProducerPid;
    int32_t mConsumerPid;
    int mConnectedApi;
    String8 mConsumerName;

    String8 mConsumerProcName;
    String8 mProducerProcName;


    // track for producer buffer return
    FpsCounter mQueueFps;

    // track for consumer buffer return
    FpsCounter mReleaseFps;

    // whether the queue is hosted in SurfaceFlinger
    bool mIsInSF;

    // whether the queue is hosted in GuiExtService or not
    bool mIsInGuiExt;

    // if debug line enabled
    bool mLine;

    // debug line count
    uint32_t mLineCnt;

    // for buffer dump
    sp<BufferQueueDump> mDump;

    // whether dump mechanism of general buffer queue is enabled or not
    bool mGeneralDump;

    BufferQueueDebug();

    void onConstructor(wp<BufferQueue> bq, const int32_t id, const String8& consumerName);
    void onDestructor();
    void onSetConsumerName(const String8& consumerName);
    void setIonInfoOnDequeue(const sp<GraphicBuffer>& gb);
    void onDequeue(const int outBuf, sp<GraphicBuffer>& gb, sp<Fence>& fence);
    void onQueue(const int buf, const int64_t timestamp);
    void onCancel(const int buf);
    void onConnect(
            const sp<IBinder>& token,
            const int api,
            bool producerControlledByApp);

    void onDump(String8 &result, const String8& prefix);
    void onFreeBufferLocked(const int slot);
    void onAcquire(
            const int buf,
            const sp<GraphicBuffer>& gb,
            const sp<Fence>& fence,
            const int64_t timestamp,
            const BufferQueue::BufferItem* const buffer);
    void onRelease(const int buf);
    void onConsumerConnect(
            const sp<IConsumerListener>& consumerListener,
            const bool controlledByApp);
    void onConsumerDisconnectHead();
    void onConsumerDisconnectTail();
    void onDisconnect();
};

class BufferQueueProxy : public Singleton<BufferQueueProxy>, public RWTable<const BufferQueue*, BufferQueueDebug> {
    sp<BufferQueueDebug> createEntry();
};

inline sp<BufferQueueDebug> getBQDebugger(const BufferQueue * const ptr) {
    return BufferQueueProxy::getInstance()[ptr];
}

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_BUFFERQUEUEDEBUG_H
