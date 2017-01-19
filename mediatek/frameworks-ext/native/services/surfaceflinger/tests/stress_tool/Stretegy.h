#ifndef MTK_TEST_SURFACE_STRETEGY_H
#define MTK_TEST_SURFACE_STRETEGY_H
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>

#include "TestSurface.h"
#include "App.h"

class TestSurface;

class Stretegy : public android::RefBase {
public:
    Stretegy(const uint64_t& changeFreq)
        :   mCount(0),
            mChangeFreq(changeFreq) {}
    virtual ~Stretegy() {}
    virtual bool applyTo(const android::Vector<android::sp<TestSurface> >& surfaces) = 0;

protected:
    uint64_t mCount;
    const uint64_t mChangeFreq;
};

/* Only SelfBurst is the friend class of class App, now.
 * Because creating or destroying surface needs to access private data member of App
 * I don't want to add public accessor because there is only SelfBurst requiring the privilege
 */
class SelfBurst : public Stretegy {
public:
    SelfBurst(const uint64_t& changeFreq, android::sp<App> app)
        :   Stretegy(changeFreq),
            mApp(app) {}
    virtual ~SelfBurst() {}
    bool applyTo(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    const android::sp<App> mApp;
    void createSurface();
};

class FreeMove : public Stretegy {
public:
    FreeMove(const uint64_t& changeFreq)
        :   Stretegy(changeFreq) {}
    virtual ~FreeMove() {}
    bool applyTo(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};

class FreeScale : public Stretegy {
public:
    FreeScale(const uint64_t& changeFreq)
        :   Stretegy(changeFreq) {}
    virtual ~FreeScale() {}
    bool applyTo(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};

class FreeBlend : public Stretegy {
public:
    FreeBlend(const uint64_t& changeFreq)
        :   Stretegy(changeFreq) {}
    virtual ~FreeBlend() {}
    bool applyTo(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};

class Shuffle : public Stretegy {
public:
    Shuffle(const uint64_t& changeFreq)
        :   Stretegy(changeFreq) {}
    virtual ~Shuffle() {}
    bool applyTo(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::Vector<TestSurface::State> mStates;
};

class ClockwiseMove : public Stretegy {
public:
    ClockwiseMove(const uint64_t& changeFreq)
        :   Stretegy(changeFreq) {}
    virtual ~ClockwiseMove() {}
    bool applyTo(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};
#endif // MTK_TEST_SURFACE_STRETEGY_H
