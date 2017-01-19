#include "camera_custom_3dnr.h"
#include <stdlib.h>  // For atio().
#include <cutils/properties.h>  // For property_get().



int get_3dnr_iso_enable_threshold_low(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdLowTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.3dnr.lowiso", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdLowTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdLowTemp = ISO_ENABLE_THRESHOLD_LOW;
    }

    return IsoEnableThresholdLowTemp;
}

int get_3dnr_iso_enable_threshold_high(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdHighTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.3dnr.highiso", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdHighTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdHighTemp = ISO_ENABLE_THRESHOLD_HIGH;
    }

    return IsoEnableThresholdHighTemp;
}

#if 0   // Obsolete
int get_3dnr_iso_enable_threshold_low_percentage(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdLowPercentageTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.3dnr.lowpercent", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdLowPercentageTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdLowPercentageTemp = ISO_ENABLE_THRESHOLD_LOW_PERCENTAGE;
    }

    return IsoEnableThresholdLowPercentageTemp;
}

int get_3dnr_iso_enable_threshold_high_percentage(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdHighPercentageTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.3dnr.highpercent", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdHighPercentageTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdHighPercentageTemp = ISO_ENABLE_THRESHOLD_HIGH_PERCENTAGE;
    }

    return IsoEnableThresholdHighPercentageTemp;
}
#endif  // Obsolete

int get_3dnr_max_iso_increase_percentage(void)
{
    // Force change ISO Limit.
    unsigned int MaxIsoIncreasePercentageTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.3dnr.forceisolimit", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        MaxIsoIncreasePercentageTemp = i4TempInputValue;
    }
    else
    {
        MaxIsoIncreasePercentageTemp = MAX_ISO_INCREASE_PERCENTAGE;
    }

    return MaxIsoIncreasePercentageTemp;
}

int get_3dnr_hw_power_off_threshold(void)
{
    return HW_POWER_OFF_THRESHOLD;
}

int get_3dnr_hw_power_reopen_delay(void)
{
    return HW_POWER_REOPEN_DELAY;
}

