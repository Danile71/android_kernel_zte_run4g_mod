#ifndef MTK_TEST_SURFACE_APP_H
#define MTK_TEST_SURFACE_APP_H

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/Thread.h>

#include "Dump.h"
#include "Stretegy.h"

class TestSurface;
class Stretegy;

class App : public android::Thread, Dump {
public:
    enum ActionList {
        ACTION_UNKNOWN       = 0x0,
        ACTION_SCALE         = 0x1,
        ACTION_MOVE          = 0x1 << 1,
        ACTION_SURFACECROP   = 0x1 << 2,
        ACTION_BUFFERCROP    = 0x1 << 3,
        ACTION_SHUFFLE       = 0x1 << 4,
        ACTION_BLEND         = 0x1 << 5,
        ACTION_CLOCKWISEMOVE = 0x1 << 6,
        ACTION_JUMP          = 0x1 << 7
    };

    App();
    virtual ~App();
    android::status_t readyToRun();

    void addSurfaceData(const android::sp<SurfaceData>& data);
    void addStretegy(android::sp<Stretegy> stretegy);
    void setActionFlag(const uint32_t& actionsFlag);
    uint32_t getActionFlag() const;
    const android::Vector<android::sp<SurfaceData> >& getSurfaceData() const { return mSurfaceData; }

    void dump() const;
private:
    friend class SelfBurst;
    bool threadLoop();

    android::Vector<android::sp<SurfaceData> > mSurfaceData;
    android::Vector<android::sp<TestSurface> > mSurfaces;
    android::Vector<android::sp<Stretegy> > mStretegys;
    useconds_t mDelay;
    uint32_t mActionFlag;
    mutable android::Mutex mMutexActionFlag;
    mutable uint32_t mCount;
    uint32_t mChangeFreq;
};

#endif // MTK_TEST_SURFACE_APP_H
