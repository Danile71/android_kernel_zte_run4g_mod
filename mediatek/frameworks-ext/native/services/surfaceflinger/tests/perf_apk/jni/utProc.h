#ifndef __UT_PROC_H__
#define __UT_PROC_H__

#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>

#include "FpsCounter.h"

namespace android {

class SfUtProc :
    public Thread,
    public virtual RefBase
{
public:
    SfUtProc();
    virtual ~SfUtProc();

    void connect(int id, sp<ANativeWindow> window,
        bool useDefaultSize, int w, int h, bool updateScreen, int api, int format, int colorIndex);
    void disconnect(int id);
    void setGlobalConfig(int color){
        mFixedColor = color;
    };
    float getFps();

    uint32_t mId;

    class SurfaceItem : public virtual RefBase {
        public:
            sp<ANativeWindow> window;
            int colorIndex;
            bool isUpdateScreen;
    };
    DefaultKeyedVector< uint32_t, sp<SurfaceItem> > surfaceList;

private:
    virtual void onFirstRef();
    virtual status_t readyToRun();
    virtual bool threadLoop();

    void init(ANativeWindow *w, bool useDefaultSize, int requestWidth, int requestHeight, int api, int format);
    void render(ANativeWindow *w, uint32_t color, bool reDraw = true);
    int getBitPerPixel(int format);

    int mFixedColor;
    mutable Mutex mLock;
    uint32_t mFrameCount;
    uint64_t mStartTime;
    uint64_t mEndTime;

    FpsCounter mFpsCounter;

    bool mPrintDetailLog;
};

};

#endif

