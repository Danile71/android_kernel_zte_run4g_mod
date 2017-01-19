#ifndef __AAL_SERVICE_CLIENT_H__
#define __AAL_SERVICE_CLIENT_H__

#include <stdint.h>
#include <sys/types.h>

#include <binder/Binder.h>
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>
#include "IAALService.h"

namespace android {

class IAALService;
class AALClient : public Singleton<AALClient>
{
    friend class Singleton<AALClient>;
    
public:
    status_t setMode(int32_t mode);
    status_t setFunction(uint32_t funcFlags);
    status_t setBacklightColor(int32_t color);
    status_t setBacklightBrightness(int32_t level);
    status_t setLightSensorValue(int32_t value);
    status_t setAniTargetBrightness(int32_t brightness);
    status_t setScreenBrightness(int32_t brightness) {
        return setAniTargetBrightness(brightness);
    }
    status_t setScreenState(int32_t state, int32_t brightness);
    status_t setUserBrightness(int32_t level);

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
    AALClient();
    
    // DeathRecipient interface
    void serviceDied();
    
    status_t assertStateLocked() const;

    mutable Mutex mLock;
    mutable sp<IAALService> mAALService;
    mutable sp<IBinder::DeathRecipient> mDeathObserver;
};

};


#define AALClient_readField(field, value_ptr) \
    android::AALClient::getInstance().readField((field), (value_ptr))

#define AALClient_writeField(field, value) \
    android::AALClient::getInstance().writeField((field), (value))

#endif
