#ifndef __AAL_SERVICE_H__
#define __AAL_SERVICE_H__

#include <utils/threads.h>
#include "IAALService.h"

// HAL
#include <hardware/hardware.h>
#include <hardware/lights.h>

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>



class CAALFW;
struct AALInitReg;
struct AALInput;
struct AALOutput;

namespace android
{

class CustFunctions;

class AALService : 
        public BinderService<AALService>, 
        public BnAALService,
        public Thread
{
    friend class BinderService<AALService>;
public:
    AALService();
    ~AALService();
    
    static char const* getServiceName() { return "AAL"; }
    
    // IAALServic interface
    virtual status_t setFunction(uint32_t func_bitset);
    virtual status_t setBacklightColor(int32_t color);
    virtual status_t setBacklightBrightness(int32_t brightness);
    virtual status_t setLightSensorValue(int32_t value);
    virtual status_t setAniTargetBrightness(int32_t brightness);
    virtual status_t setScreenState(int32_t state, int32_t brightness);
    virtual status_t setUserBrightness(int32_t level);
    
    virtual status_t dump(int fd, const Vector<String16>& args);
    
    status_t setBrightnessLevel(int32_t level);
    status_t setDarkeningSpeedLevel(int32_t level);
    status_t setBrighteningSpeedLevel(int32_t level);
    status_t setSmartBacklightStrength(int32_t level);
    status_t setSmartBacklightRange(int32_t level);
    status_t setReadabilityLevel(int32_t level);
    status_t getParameters(AALParameters *outParam);
    status_t custInvoke(int32_t cmd, int64_t arg);
    status_t readField(uint32_t field, uint32_t *value);
    status_t writeField(uint32_t field, uint32_t value);
    
private:
    virtual void onFirstRef();
    virtual status_t readyToRun();
    void initDriverRegs();
    virtual bool threadLoop();
    void writeLedNode(const int value_1024);
    status_t enableAALEvent(bool enable);
    status_t setBacklight(int32_t level);
    status_t debugDump(unsigned int debugLevel);

    void updateDebugInfo(sp<SurfaceControl> surface);
    void clearDebugInfo(sp<SurfaceControl> surface);

    // hardware
    light_device_t *mLight;
    int mDispFd;

    mutable Mutex mLock;
    bool mEventEnabled;
    volatile bool mToEnableEvent;

    int mScrWidth;
    int mScrHeight;
    int mMode;
    bool mIsBacklightValid;
    bool mWriteDriverRegs;
    int mALI;
    int mPmsScrState; // Screen state of power manager
    int mPrevScrState;
    int mCurrScrState;
    int mBacklight;
    int mTargetBacklight;
    int mLongTermBacklight;
    int mOutBacklight;
    unsigned int mPrevFuncFlags;
    unsigned int mFuncFlags; // bit-set of AALFunction
    int mUserBrightness;

    // Store in member variable to debug
    AALInitReg *mAALInitReg;
    AALInput *mAALInput;
    AALOutput *mAALOutput;

    CAALFW *mAALFW;
    CustFunctions *mCustFunc;

    unsigned int mDebugLevel;
    struct {
        bool panelOn;
        bool panelDisplayed;
        bool alwaysEnable;
        int overwriteCABC;
        int overwriteDRE;
    } mDebug;

    unsigned int mEventFlags;
    
    void setEvent(unsigned int event) {
        mEventFlags |= event;
    }

    bool isEventSet(unsigned int event) {
        return ((mEventFlags & event) > 0);
    }

    void clearEvents(unsigned int events = 0xffff) {
        mEventFlags &= ~events;
    }

    void unitTest();
};
};

#endif
