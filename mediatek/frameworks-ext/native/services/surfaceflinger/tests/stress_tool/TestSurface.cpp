#define LOG_TAG "stresstool"

#include "TestSurface.h"

#include <cutils/compiler.h>
#include <utils/String8.h>

#include <log/log.h>

#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferExtra.h>
#include <ui/gralloc_extra.h>

#include "Frame.h"
#include "Action.h"

using namespace android;

TestSurface::TestSurface(const String8& name,
                         const Vector<sp<Frame> >& frames,
                         const Config& cfg,
                         const State& state)
    :   mFrames(frames),
        mComposer(new SurfaceComposerClient()),
        mANWBuf(NULL),
        mCfg(cfg),
        mState(state) {
    if (CC_UNLIKELY(mComposer == NULL)) {
        XLOGE("Create SurfaceComposerClient failed");
    }

    if (CC_UNLIKELY(mFrames.size() == 0)) {
        XLOGE("No frames in the Frames list");
    }
    mControl = mComposer->createSurface(name,
                                        mState.mW,
                                        mState.mH,
                                        mState.mFormat,
                                        mState.mFlags);
    if (CC_UNLIKELY(mControl == NULL)) {
        XLOGE("Create SurfaceControl failed");
    }

    mSurface = mControl->getSurface();
    if (CC_UNLIKELY(mSurface == NULL)) {
        XLOGE("Create SurfaceControl failed");
    }

    mWindow = mSurface.get();
    if (CC_UNLIKELY(mWindow == NULL)) {
        XLOGE("Get ANativeWindow failed");
    }

    mShowIdx = 0;
    const Frame::Config& frameCfg = mFrames[0]->getConfig();
    XLOGI("TestSurface(): s:%d h:%d fmt:%d api:%d", mState.mW, mState.mH, mState.mFormat, frameCfg.mApi );




    accept(new Move(mState.mX, mState.mY));
    accept(new Scale(mState.mW, mState.mH));
    accept(new Blend(1.f));
    accept(new SetZOrder(mState.mZOrder));
}

TestSurface::~TestSurface() {

}

bool TestSurface::accept(const sp<Action>& action) {
    if (action->check(mCfg, mState, mFrames[mShowIdx])) {
        mActions.push_back(action);
        return true;
    } else {
        XLOGE("Action checking failed");
        return false;
    }
}

void TestSurface::beginFrame() {
    const Frame::Config& frameCfg = mFrames[mShowIdx]->getConfig();

    XLOGI("prepare(): pitch:%d s:%d h:%d fmt:%d", frameCfg.mPitch, frameCfg.mS, frameCfg.mH, frameCfg.mFormat);
    for (List<sp<Action> >::iterator iter = mActions.begin(); iter!= mActions.end(); ++iter) {
        (*iter)->bufferApply(mWindow, &mState);
    }

    // set api connection type as register
    // TODO: How to get the api type
    if (native_window_api_connect(mWindow, frameCfg.mApi) == -EINVAL) {
        XLOGE("native_window_api_connect() failed");
    }

    // TODO: Setting buffer count may take as an Action
    //  set buffer count
    uint32_t frameSize = mFrames.size() < 2 ? 3 : mFrames.size();
    if (native_window_set_buffer_count(mWindow, frameSize) == -EINVAL) {
        XLOGE("native_window_set_buffer_count() failed");
    }

    // set usage software write-able and hardware texture bind-able
    if (native_window_set_usage(mWindow, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE) != NO_ERROR) {
        XLOGE("setting usage failed");
        return;
    }

    // set scaling to match window display size
    if (native_window_set_scaling_mode(mWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW) != NO_ERROR) {
        XLOGE("setting scaling mode failed");
        return;
    }

    // set buffer geometry
    if (native_window_set_buffers_geometry(mWindow, frameCfg.mS, frameCfg.mH, frameCfg.mFormat) != NO_ERROR) {
        XLOGE("setting buffer geometry failed");
        return;
    }

    int fenceFd = -1;
    if (mWindow->dequeueBuffer(mWindow, &mANWBuf, &fenceFd) != NO_ERROR) {
        XLOGE("dequeueBuffer failed");
        return;
    }

    sp<Fence> fence = new Fence(fenceFd);
    fence->wait(Fence::TIMEOUT_NEVER);

    sp<GraphicBuffer> gb = new GraphicBuffer(mANWBuf, false);
    if (gb == NULL) {
        XLOGE("creating GraphicBuffer failed");
    }
    const Rect rect(frameCfg.mW, frameCfg.mH);
    void* ptr = NULL;

    GraphicBufferExtra::get().setBufParameter(gb->handle,
                                              GRALLOC_EXTRA_MASK_TYPE,
                                              frameCfg.mUsageEx);

    if (CC_UNLIKELY(gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN |
                    GRALLOC_USAGE_HW_TEXTURE, rect, &ptr) != NO_ERROR)) {
        XLOGE("GraphicBuffer lock failed");
    }

    memcpy(ptr, mFrames[mShowIdx]->data(), frameCfg.mPitch * frameCfg.mH);
    XLOGE("frame:%s", frameCfg.mPath.string());

    if (CC_UNLIKELY(gb->unlock() != NO_ERROR)) {
        XLOGE("GraphicBuffer unlock failed");
    }
    if (CC_UNLIKELY(mWindow->queueBuffer(mWindow, mANWBuf, -1) != NO_ERROR)) {
        XLOGE("refresh(): queueBuffer() failed");
    }
}


void TestSurface::refresh() {
    XLOGI("refresh(): mActions.size():%d x:%d y:%d w:%d h:%d zOrder:%d", mActions.size(),
                                                                         mState.mX,
                                                                         mState.mY,
                                                                         mState.mW,
                                                                         mState.mH,
                                                                         mState.mZOrder);

    for (List<sp<Action> >::iterator iter = mActions.begin(); iter!= mActions.end(); ++iter) {
        (*iter)->surfaceApply(mControl, &mState);
    }

    XLOGD("close transaction");

    mActions.clear();
}

void TestSurface::dump() const {
}

void TestSurface::endFrame() {
    native_window_api_disconnect(mWindow, 0);
    if (++mShowIdx == mFrames.size()) {
        mShowIdx = 0;
    }
}
