#define LOG_TAG "stresstool"
#include "Action.h"

using namespace android;

Scale::Scale(const int32_t& w, const int32_t& h)
    :   mW(w),
        mH(h) {
}

bool Scale::check(const TestSurface::Config& cfg,
                  const TestSurface::State& state,
                  const android::sp<Frame>& frame) const {
    if (!cfg.mScale) {
        XLOGE("Surface do not allow scaling");
        return false;
    }
    XLOGD("Scale::check() mW:%d mH:%d", mW, mH);
    if (!(mW >= 0 && mH >= 0 &&
            state.mX + static_cast<uint32_t>(mW) >= cfg.mMinSize &&
            static_cast<uint32_t>(mW) >= cfg.mMinSize && static_cast<uint32_t>(mW) < cfg.mDispW * 1.5 &&
            state.mY + static_cast<uint32_t>(mH) >= cfg.mMinSize &&
            static_cast<uint32_t>(mH) >= cfg.mMinSize && static_cast<uint32_t>(mH) < cfg.mDispH * 1.5)) {
        XLOGW("Scaling's checking failed x:%d y:%d w:%d h:%d minSize:%d", state.mX, state.mY, mW, mH, cfg.mMinSize);
        return false;
    }
    return true;
}

bool Scale::surfaceApply(const sp<SurfaceControl>& control,
                         TestSurface::State* const state) const {
    if (CC_UNLIKELY(control->setSize(mW, mH) != NO_ERROR)) {
        XLOGE("set size(%d, %d) failed", mW, mH);
        return false;
    }
    XLOGD("set size(%d,%d)", mW, mH);
    state->mW = mW;
    state->mH = mH;
    return true;
}

// =============================================================

Move::Move(const int32_t& x, const int32_t& y)
    :   mX(x),
        mY(y) {
}

bool Move::check(const TestSurface::Config& cfg,
                 const TestSurface::State& state,
                 const android::sp<Frame>& frame) const {
    if (!cfg.mMove) {
        XLOGE("Surface do not allow moving");
        return false;
    }

    if (!(mX + state.mW >= static_cast<int32_t>(cfg.mMinSize) &&
                mX < static_cast<int32_t>(cfg.mDispW - cfg.mMinSize) &&
                mY + state.mH >= static_cast<int32_t>(cfg.mMinSize) &&
                mY < static_cast<int32_t>(cfg.mDispH - cfg.mMinSize))) {
        XLOGW("Moving's checking failed x:%d y:%d w:%d h:%d minSize:%d", mX, mY, state.mW, state.mH, cfg.mMinSize);
        return false;
    }
    return true;
}

bool Move::surfaceApply(const sp<SurfaceControl>& control,
                        TestSurface::State* const state) const {
    if (CC_UNLIKELY(control->setPosition(mX, mY) != NO_ERROR)) {
        XLOGE("set pos(%d, %d) failed", mX, mY);
        return false;
    }
    XLOGD("set pos(%d,%d)", mX, mY);
    state->mX = mX;
    state->mY = mY;
    return true;
}

// =============================================================

SurfaceCrop::SurfaceCrop(const Rect& crop)
    :   mCrop(crop) {
}

bool SurfaceCrop::check(const TestSurface::Config& cfg,
                        const TestSurface::State& state,
                        const android::sp<Frame>& frame) const {
    if (!cfg.mSurfaceCrop) {
        XLOGE("Surface do not allow crop");
        return false;
    }

    if (!(mCrop.left >= 0 && mCrop.top >= 0 &&
            mCrop.right < static_cast<int>(state.mW) &&
            mCrop.bottom < static_cast<int>(state.mH))) {
        XLOGW("SurfaceCrop checking failed- left:%d top:%d right:%d bottom:%d w:%d h:%d",
                mCrop.left,
                mCrop.top,
                mCrop.right,
                mCrop.bottom,
                state.mW,
                state.mH);
        return false;
    }
    return true;
}

bool SurfaceCrop::surfaceApply(const sp<SurfaceControl>& control,
                               TestSurface::State* const state) const {
    if (CC_UNLIKELY(control->setCrop(mCrop) != NO_ERROR)) {
        XLOGE("set surface crop(%d, %d, %d, %d) failed", mCrop.left,
                                                         mCrop.top,
                                                         mCrop.right,
                                                         mCrop.bottom);
        return false;
    }
    XLOGD("set surface crop(%d, %d, %d, %d)", mCrop.left,
                                              mCrop.top,
                                              mCrop.right,
                                              mCrop.bottom);
    state->mSurfaceCrop.set(mCrop);
    return true;
}

// =============================================================

BufferCrop::BufferCrop(const Rect& crop)
    :   mCrop(crop) {
}

bool BufferCrop::check(const TestSurface::Config& cfg,
                       const TestSurface::State& state,
                       const android::sp<Frame>& frame) const {
    if (!cfg.mBufferCrop) {
        XLOGE("Surface do not allow buffer crop");
        return false;
    }

    if (!(mCrop.left >= 0 && mCrop.top >= 0 &&
            mCrop.right < static_cast<int>(state.mW)
            && mCrop.bottom < static_cast<int>(state.mH))) {
        XLOGE("BufferCrop checking failed- left:%d top:%d right:%d bottom:%d w:%d h:%d",
                mCrop.left,
                mCrop.top,
                mCrop.right,
                mCrop.bottom,
                state.mW,
                state.mH);
        return false;
    }
    return true;
}

bool BufferCrop::bufferApply(ANativeWindow* const mWindow,
                             TestSurface::State* const state) const {
    if (native_window_set_crop(mWindow, ((android_native_rect_t*)(&mCrop))) == -EINVAL) {
        XLOGE("set buffer crop(%d, %d, %d, %d) failed", mCrop.left,
                                                        mCrop.top,
                                                        mCrop.right,
                                                        mCrop.bottom);
        return false;
    }
    XLOGD("set buffer crop(%d, %d, %d, %d)", mCrop.left,
                                             mCrop.top,
                                             mCrop.right,
                                             mCrop.bottom);
    state->mSurfaceCrop.set(mCrop);
    return true;
}

// =============================================================

Blend::Blend(const float& alpha)
    :   mAlpha(alpha) {
}

bool Blend::check(const TestSurface::Config& cfg,
                  const TestSurface::State& state,
                  const android::sp<Frame>& frame) const {
    XLOGE("Blending check alpha:%f", mAlpha);
    if (!cfg.mBlend) {
        XLOGE("Surface do not allow blending");
        return false;
    }
    if (mAlpha < 0.f || mAlpha > 1.f) {
        XLOGE("Blending checking failed- alpha:%f", mAlpha);
        return false;
    }
    return true;
}

bool Blend::surfaceApply(const sp<SurfaceControl>& control,
                         TestSurface::State* const state) const {
    if (CC_UNLIKELY(control->setAlpha(mAlpha) != NO_ERROR)) {
        XLOGE("set alpha(%f) failed", mAlpha);
        return false;
    }
    XLOGD("set alpha(%f)", mAlpha);
    state->mAlpha = mAlpha;
    return true;
}

// =============================================================

SetZOrder::SetZOrder(const int32_t& zOrder)
    :   mZOrder(zOrder) {
}

bool SetZOrder::check(const TestSurface::Config& cfg,
                  const TestSurface::State& state,
                  const android::sp<Frame>& frame) const {
    if (!cfg.mZOrder) {
        XLOGE("Surface do not allow shuffling");
        return false;
    }
    return true;
}

bool SetZOrder::surfaceApply(const sp<SurfaceControl>& control,
                         TestSurface::State* const state) const {
    if (CC_UNLIKELY(control->setLayer(mZOrder) != NO_ERROR)) {
        XLOGE("set ZOrder(%d) failed", mZOrder);
        return false;
    }
    XLOGD("set ZOrder(%d)", mZOrder);
    state->mZOrder = mZOrder;
    return true;
}
