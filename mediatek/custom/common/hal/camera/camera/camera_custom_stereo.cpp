#include "camera_custom_stereo.h"

/*******************************************************************************
* N3D sensor position
*******************************************************************************/
customSensorPos_STEREO_t const&
getSensorPosSTEREO()
{
    static customSensorPos_STEREO_t inst = {
        uSensorPos   : 1,   //0:LR 1:RL (L:tg1, R:tg2)
    };
    return inst;
}

/*******************************************************************************
* Author : cotta
* brief : Return enable/disable flag of STEREO
*******************************************************************************/
MBOOL get_STEREOFeatureFlag(void)
{
#ifdef MTK_STEREO_SUPPORT
    return MTRUE;
#else
    return MFALSE;
#endif
}
