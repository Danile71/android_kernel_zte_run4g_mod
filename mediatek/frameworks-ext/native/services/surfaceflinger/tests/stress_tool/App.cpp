#define LOG_TAG "stresstool"
#include "TestSurface.h"
#include "Stretegy.h"
#include "App.h"
#include "Action.h"

using namespace android;

App::App()
//    :   mDelay(1E6),
    :   mDelay(16666),
        mActionFlag(0),
        mCount(0),
        mChangeFreq(60) {
    srand(time(NULL));
}

App::~App() {
}

status_t App::readyToRun() {
    for (uint32_t i = 0; i < mSurfaceData.size(); ++i) {
        mSurfaces.push_back(new TestSurface(mSurfaceData[i]->mName,
                                            mSurfaceData[i]->mFrames,
                                            mSurfaceData[i]->mCfg,
                                            mSurfaceData[i]->mInitState));
    }
    return NO_ERROR;
}

void App::addSurfaceData(const sp<SurfaceData>& data) {
    mSurfaceData.push_back(data);
}

void App::addStretegy(sp<Stretegy> stretegy) {
    mStretegys.push_back(stretegy);
}

void App::setActionFlag(const uint32_t& actionFlag) {
    mMutexActionFlag.lock();
    {
        mActionFlag = actionFlag;
    }
    mMutexActionFlag.unlock();
}

uint32_t App::getActionFlag() const {
    uint32_t retValue;
    mMutexActionFlag.lock();
    {
        retValue = mActionFlag;
    }
    mMutexActionFlag.unlock();
    return retValue;
}

bool App::threadLoop() {

    XLOGI("ActionFlag:%d", getActionFlag());
    for (uint32_t i = 0; i < mStretegys.size(); ++i) {
        mStretegys[i]->applyTo(mSurfaces);
    }
    for (uint32_t i = 0; i < mSurfaces.size(); ++i) {
        const sp<TestSurface>& surface = mSurfaces[i];
        XLOGI("Play(%d) beginFrame()", i);
        surface->beginFrame();
    }

    SurfaceComposerClient::openGlobalTransaction();
    for (uint32_t i = 0; i < mSurfaces.size(); ++i) {
        const sp<TestSurface>& surface = mSurfaces[i];
        XLOGI("Play(%d) refresh()", i);
        surface->refresh();
    }
    SurfaceComposerClient::closeGlobalTransaction();

    for (uint32_t i = 0; i < mSurfaces.size(); ++i) {
        const sp<TestSurface>& surface = mSurfaces[i];
        XLOGI("+ Play(%d) endFrame()", i);
        surface->endFrame();
        XLOGI("- Play(%d) endFrame()", i);
    }


    ++mCount;
    usleep(mDelay);

    return true;
}

void App::dump() const {
}
