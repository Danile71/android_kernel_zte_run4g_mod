#ifndef _CAMERA_CUSTOM_STEREO_IF_
#define _CAMERA_CUSTOM_STEREO_IF_
//
#include "camera_custom_types.h"
//
/*******************************************************************************
* STEREO sensor baseline
******************************************************************************/
#define STEREO_BASELINE 2.5
/*******************************************************************************
* STEREO sensor position
******************************************************************************/
typedef struct customSensorPos_STEREO_s
{
    MUINT32  uSensorPos;
} customSensorPos_STEREO_t;

customSensorPos_STEREO_t const&  getSensorPosSTEREO();

/*******************************************************************************
* Return enable/disable flag of STEREO to ISP
*******************************************************************************/
MBOOL get_STEREOFeatureFlag(void);  //cotta : added for STEREO

#endif  //  _CAMERA_CUSTOM_STEREO_IF_