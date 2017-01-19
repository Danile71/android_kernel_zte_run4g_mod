#include "aaa_types.h"
#include "aaa_yuv_tuning_custom.h"

namespace NSYuvTuning
{
/*******************************************************************************
* Author : cotta
* Functionality : custom flashlight gain between preview/capture flash
*******************************************************************************/
#define FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X 10
MUINT32 custom_GetFlashlightGain10X(MINT32 i4SensorDevId)
{   
    // x10 , 1 mean 0.1x gain    
    //10 means no difference. use torch mode for preflash and cpaflash
    //> 10 means capture flashlight is lighter than preflash light. < 10 is opposite condition.    
    return (MUINT32)FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X;
}

MUINT32 custom_BurstFlashlightGain10X(MINT32 i4SensorDevId)
{
    return (MUINT32)FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X;
}
/*******************************************************************************
* Author : Jiale
* Functionality : custom yuv flashlight threshold
*******************************************************************************/
#define FLASHLIGHT_YUV_THRESHOlD 3.0
MDOUBLE custom_GetYuvFlashlightThreshold(MINT32 i4SensorDevId)
{    
    return (MDOUBLE)FLASHLIGHT_YUV_THRESHOlD;
}

/*******************************************************************************
* Author : Jiale
* Functionality : custom yuv sensor convergence frame count
*******************************************************************************/
#define FLASHLIGHT_YUV_CONVERGENCE_FRAME 7
MINT32 custom_GetYuvFlashlightFrameCnt(MINT32 i4SensorDevId)
{    
    return (int)FLASHLIGHT_YUV_CONVERGENCE_FRAME;
}

/*******************************************************************************
* Author : CD
* Functionality : custom yuv sensor preflash duty
*******************************************************************************/
#define FLASHLIGHT_YUV_NORMAL_LEVEL 12
MINT32 custom_GetYuvFlashlightDuty(MINT32 i4SensorDevId)
{    
    return (int)FLASHLIGHT_YUV_NORMAL_LEVEL;
}

/*******************************************************************************
* Author : CD
* Functionality : custom yuv sensor capture flash duty (high current mode)
*******************************************************************************/
#define FLASHLIGHT_YUV_MAIN_HI_LEVEL 12
MINT32 custom_GetYuvFlashlightHighCurrentDuty(MINT32 i4SensorDevId)
{
    // if FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X > 10 (high current mode),
    // it means capture flashlight is lighter than preflash light.
    // In this case, you need to specify the level for capture flash accordingly.
    return (int)FLASHLIGHT_YUV_MAIN_HI_LEVEL;
}

/*******************************************************************************
* Author : CD
* Functionality : custom yuv sensor capture flash timeout (high current mode)
*******************************************************************************/
#define FLASHLIGHT_YUV_MAIN_HI_TIMEOUT 500
MINT32 custom_GetYuvFlashlightHighCurrentTimeout(MINT32 i4SensorDevId)
{
    // if FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X > 10 (high current mode),
    // it means capture flashlight is lighter than preflash light.
    // In this case, you may need to set the timeout in ms in case of LED burning out.
    return (int)FLASHLIGHT_YUV_MAIN_HI_TIMEOUT;
}


/*******************************************************************************
* Author : CD
* Functionality : custom yuv sensor flashlight step
*******************************************************************************/
#define FLASHLIGHT_YUV_STEP 7
MINT32 custom_GetYuvFlashlightStep(MINT32 i4SensorDevId)
{    
    return (int)FLASHLIGHT_YUV_STEP;
}

/*******************************************************************************
* Author : CD
* Functionality : custom yuv flashlight AF Lamp support
*******************************************************************************/
#define FLASHLIGHT_YUV_AF_LAMP 1
MINT32 custom_GetYuvAfLampSupport(MINT32 i4SensorDevId)
{
    // 0: indicates no AF lamp when touch AF
    // 1: indicates AF lamp support for touch AF
    return (int)FLASHLIGHT_YUV_AF_LAMP;
}

/*******************************************************************************
* Author : CD
* Functionality : custom yuv flashlight AF Lamp support
*******************************************************************************/
#define FLASHLIGHT_YUV_AF_PREFLASH 0
MINT32 custom_GetYuvPreflashAF(MINT32 i4SensorDevId)
{
    return (int)FLASHLIGHT_YUV_AF_PREFLASH;
}

}

