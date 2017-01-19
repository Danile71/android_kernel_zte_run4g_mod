#define LOG_TAG "stresstool"
#include "Stretegy.h"

#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>

#include "Action.h"
#include "TestSurface.h"

using namespace android;

void SelfBurst::createSurface() {
    uint32_t idx = rand() % mApp->mSurfaceData.size();
    const sp<SurfaceData>& data = mApp->mSurfaceData[idx];
    data->mInitState.mW = rand() % (data->mCfg.mDispW - data->mCfg.mMinSize) + data->mCfg.mMinSize;
    data->mInitState.mH = rand() % (data->mCfg.mDispH - data->mCfg.mMinSize) + data->mCfg.mMinSize;
    data->mInitState.mX = rand() % (data->mCfg.mDispW - data->mInitState.mW);
    data->mInitState.mY = rand() % (data->mCfg.mDispH - data->mInitState.mH);
    mApp->mSurfaces.push_back(new TestSurface(data->mName,
                data->mFrames,
                data->mCfg,
                data->mInitState));
}

bool SelfBurst::applyTo(const Vector<sp<TestSurface> >& surfaces) {
    ++mCount;
    if (mCount % mChangeFreq == 0) {
        if (mApp->mSurfaces.size() == mApp->mSurfaceData.size()) {
            uint32_t idx = rand() % mApp->mSurfaces.size();
            mApp->mSurfaces.removeAt(idx);
        } else if (mApp->mSurfaces.size() == 0) {
            createSurface();
        } else {
            bool create = ((rand() / 2)% 100) < 50;
            if (create) {
                createSurface();
            } else {
                uint32_t idx = rand() % mApp->mSurfaces.size();
                mApp->mSurfaces.removeAt(idx);
            }
        }
    }
    return true;
}
// ============================================================================================
bool FreeMove::applyTo(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i)
    {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0) {
                XLOGE("Allocation state failed when FreeMove::applyTo()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);

        if (mCount % mChangeFreq == 0) {
            state.mX = rand() % 3 - 1;
            state.mY = rand() % 3 - 1;
        }
        XLOGD("App::run() idx:%d(%d) - dice_x:%d dice_x:%d", i, surfaces.size(), state.mX, state.mY);
        if (!surfaces[i]->accept(new Move(surfaceState.mX + state.mX, surfaceState.mY + state.mY))) {
            state.mX = rand() % 3 - 1;
            state.mY = rand() % 3 - 1;
        }
    }
    ++mCount;
    return true;
}

// =======================================================================================

bool FreeScale::applyTo(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i) {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0) {
                XLOGE("Allocation state failed when FreeMove::applyTo()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);
        if (mCount % mChangeFreq == 0) {
            state.mW = rand() % 3 - 1;
            state.mH = rand() % 3 - 1;
        }
        XLOGD("App::run() - dice_w:%d dice_w:%d", state.mW, state.mH);
        if (!surfaces[i]->accept(new Scale(surfaceState.mW + state.mW, surfaceState.mH + state.mH))) {
            state.mW = rand() % 3 - 1;
            state.mH = rand() % 3 - 1;
        }
    }
    ++mCount;
    return true;
}

// =======================================================================================

bool FreeBlend::applyTo(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i) {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0) {
                XLOGE("Allocation state failed when FreeMove::applyTo()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);
        if (mCount % mChangeFreq == 0) {
            state.mAlpha = (rand() % 3 - 1) / 10.f;
        }
        XLOGD("App::run() - alpha:%f", state.mAlpha);
        if (!surfaces[i]->accept(new Blend(surfaceState.mAlpha + state.mAlpha))) {
            state.mAlpha = (rand() % 3 - 1) / 10.f;
        }
    }
    ++mCount;
    return true;
}

// =======================================================================================

bool Shuffle::applyTo(const Vector<sp<TestSurface> >& surfaces) {
    if (mStates.size() != surfaces.size()) {
        if (mStates.resize(surfaces.size()) < 0) {
            XLOGE("Resizing mState failed when FreeShuffle::applyTo()");
            return false;
        }
    }

    for (uint32_t i = 0; i < surfaces.size(); ++i) {
        uint32_t chosen = 0;
        if (mCount % mChangeFreq == 0 && mStates[i].mZOrder == false) {
            chosen = rand() % mStates.size();
            if (mStates[chosen].mZOrder == false) {
                int32_t aZOrder = surfaces[chosen]->getState().mZOrder;
                int32_t bZOrder = surfaces[i]->getState().mZOrder;
                XLOGD("App::run() - i:%d chosen:%d  chosenZ:%d z:%d", i, chosen, bZOrder, aZOrder);
                surfaces[chosen]->accept(new SetZOrder(bZOrder));
                surfaces[i]->accept(new SetZOrder(aZOrder));

                mStates.editItemAt(chosen).mZOrder = true;
                mStates.editItemAt(i).mZOrder = true;
            }
        }
    }

    for (uint32_t i = 0; i< mStates.size(); ++i)
        mStates.editItemAt(i).mZOrder = false;

    ++mCount;
    return true;
}

// =======================================================================================

bool ClockwiseMove::applyTo(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i) {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0) {
                XLOGE("Allocation state failed when FreeMove::applyTo()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);

        if (mCount % mChangeFreq == 0) {
            if (state.mX == 0 && state.mY ==0)
                state.mX = 1;
        }
        XLOGD("App::run() - dice_x:%d dice_x:%d", state.mX, state.mY);
        if (!surfaces[i]->accept(new Move(surfaceState.mX + state.mX, surfaceState.mY + state.mY))) {
            if (state.mX == 1) {
                state.mX = 0;
                state.mY = 1;
            } else if (state.mY == 1) {
                state.mY = 0;
                state.mX = -1;
            } else if (state.mX == -1) {
                state.mX = 0;
                state.mY = -1;
            } else {
                state.mY = 0;
                state.mX = 1;
            }
        }
    }
    ++mCount;
    return true;
}
