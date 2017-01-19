#ifndef __IAALSERVICE_H__
#define __IAALSERVICE_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/BinderService.h>

namespace android
{

struct AALParameters
{
    int brightnessLevel;
    int darkeningSpeedLevel;
    int brighteningSpeedLevel;
    int readabilityLevel;
    int smartBacklightStrength;
    int smartBacklightRange;
};

//
//  Holder service for pass objects between processes.
//
class IAALService : public IInterface 
{
protected:
    enum {
        AAL_SET_FUNCTION = IBinder::FIRST_CALL_TRANSACTION,
        AAL_SET_BACKLIGHT_COLOR,
        AAL_SET_BACKLIGHT_BRIGHTNESS,
        AAL_SET_LIGHT_SENSOR_VALUE,
        AAL_SET_ANI_TARGET_BRIGHTNESS,
        AAL_SET_SCREEN_STATE,
        AAL_SET_USER_BRIGHTNESS,
        AAL_SET_BRIGHTNESS_LEVEL,
        AAL_SET_DARKENING_SPEED_LEVEL,
        AAL_SET_BRIGHTENING_SPEED_LEVEL,
        AAL_SET_SMART_BACKLIGHT_STRENGTH,
        AAL_SET_SMART_BACKLIGHT_RANGE,
        AAL_SET_READABILITY_LEVEL,
        AAL_GET_PARAMETERS,
        AAL_CUST_INVOKE,
        AAL_WRITE_FIELD,
        AAL_READ_FIELD
    };

public:

    // screen brightenss mode copy from Settings.System
    enum BrightnessMode {
        /** SCREEN_BRIGHTNESS_MODE value for manual mode. */
        SCREEN_BRIGHTNESS_MODE_MANUAL = 0,
        /** SCREEN_BRIGHTNESS_MODE value for automatic mode. */
        SCREEN_BRIGHTNESS_MODE_AUTOMATIC,
        /** SCREEN_ECO_BRIGHTNESS_MODE value for automatic eco backlight mode. */
        SCREEN_BRIGHTNESS_ECO_MODE_AUTOMATIC,
    };


    enum {
        SCREEN_STATE_OFF = 0,
        SCREEN_STATE_ON = 1,
        SCREEN_STATE_DIM = 2
    };

    enum {
        FUNC_NONE = 0x0,
        FUNC_LABC = 0x1,
        FUNC_CABC = 0x2,
        FUNC_DRE  = 0x4
    };

    DECLARE_META_INTERFACE(AALService);

    virtual status_t setFunction(uint32_t funcFlags) = 0;
    virtual status_t setBacklightColor(int32_t color) = 0;
    virtual status_t setBacklightBrightness(int32_t brightness) = 0;
    virtual status_t setLightSensorValue(int32_t value) = 0;
    virtual status_t setAniTargetBrightness(int32_t brightness) = 0;
    virtual status_t setScreenState(int32_t state, int32_t brightness) = 0;
    virtual status_t setUserBrightness(int32_t level) = 0;
    
    virtual status_t setBrightnessLevel(int32_t level) = 0;
    virtual status_t setDarkeningSpeedLevel(int32_t level) = 0;
    virtual status_t setBrighteningSpeedLevel(int32_t level) = 0;
    virtual status_t setSmartBacklightStrength(int32_t level) = 0;
    virtual status_t setSmartBacklightRange(int32_t level) = 0;
    virtual status_t setReadabilityLevel(int32_t level) = 0;
    virtual status_t getParameters(AALParameters *outParam) = 0;

    virtual status_t custInvoke(int32_t cmd, int64_t arg) = 0;

    virtual status_t readField(uint32_t field, uint32_t *value) = 0;
    virtual status_t writeField(uint32_t field, uint32_t value) = 0;
};

class BnAALService : public BnInterface<IAALService> 
{
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};    

};

#endif




