#ifndef MTK_TEST_SURFACE_TEST_SURFACE_H
#define MTK_TEST_SURFACE_TEST_SURFACE_H

#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/List.h>

#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <ui/PixelFormat.h>

#include "Dump.h"

class Action;
class Frame;
struct SurfaceData;

class TestSurface : public android::RefBase, Dump {
public:
    struct Config {
        uint32_t mDispW, mDispH;
        uint32_t mMinSize;
        bool mScale;
        bool mMove;
        bool mZOrder;
        bool mSurfaceCrop;
        bool mBufferCrop;
        bool mBlend;
        uint32_t mMagic;
        Config()
            :   mDispW(0),
                mDispH(0),
                mMinSize(0),
                mScale(true),
                mMove(true),
                mZOrder(true),
                mSurfaceCrop(true),
                mBufferCrop(true),
                mBlend(true),
                mMagic(1) {
        }
        Config(const Config& cfg)
            : mDispW(cfg.mDispW),
              mDispH(cfg.mDispH),
              mMinSize(cfg.mMinSize),
              mScale(cfg.mScale),
              mMove(cfg.mMove),
              mZOrder(cfg.mZOrder),
              mSurfaceCrop(cfg.mSurfaceCrop),
              mBufferCrop(cfg.mBufferCrop),
              mMagic(cfg.mMagic) {
        }
    };

    struct State {
        int32_t mX, mY;
        int32_t mW, mH;
        int32_t mZOrder;
        android::Rect mBufferCrop;
        android::Rect mSurfaceCrop;
        float mAlpha;
        android::PixelFormat mFormat;
        uint32_t mFlags;
        State()
            :   mX(0),
                mY(0),
                mW(0),
                mH(0),
                mZOrder(0),
                mAlpha(1.f),
                mFormat(android::PIXEL_FORMAT_NONE),
                mFlags(0) {
            mBufferCrop.left = mBufferCrop.top = mBufferCrop.right = mBufferCrop.bottom = 0;
            mSurfaceCrop.left = mSurfaceCrop.top = mSurfaceCrop.right = mSurfaceCrop.bottom = 0;
        }
    };



    TestSurface(const android::String8& name,
            const android::Vector<android::sp<Frame> >& frames,
            const Config& cfg,
            const State& state);

    TestSurface(const android::sp<SurfaceData>& data);
    ~TestSurface();




    inline const Config& getConfig() const { return mCfg; }
    inline const State& getState() const {return mState; }

    bool accept(const android::sp<Action>& action);

    void beginFrame();
    void refresh();
    void endFrame();
    void dump() const;
    inline uint32_t getFrameIdx() {
        return mShowIdx;
    }

private:

    const android::Vector<android::sp<Frame> > mFrames;
    const State mInitState;

    const android::sp<android::SurfaceComposerClient> mComposer;
    android::sp<android::SurfaceControl> mControl;
    android::sp<android::Surface> mSurface;
    ANativeWindow* mWindow;
    ANativeWindowBuffer* mANWBuf;
    android::List<android::sp<Action> > mActions;
    uint32_t mShowIdx;

    const Config mCfg;
    State mState;

    bool apply(const State& state);
    void init(const android::String8& name);
};

struct SurfaceData : public android::RefBase {
    const android::String8 mName;
    const TestSurface::Config mCfg;
    TestSurface::State mInitState;
    const android::Vector<android::sp<Frame> >& mFrames;

//    SurfaceData(const SurfaceData& surfaceData)
//        :   mName(surfaceData.mName),
//            mCfg(surfaceData.mCfg),
//            mInitState(surfaceData.mInitState),
//            mFrames(surfaceData.mFrames) {}

    SurfaceData(const android::String8& name,
                const TestSurface::Config& cfg,
                const TestSurface::State& initState,
                const android::Vector<android::sp<Frame> >& frames)
        :   mName(name),
            mCfg(cfg),
            mInitState(initState),
            mFrames(frames) {
    }
};

#endif // MTK_TEST_SURFACE_TEST_SURFACE_H
