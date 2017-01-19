#ifndef _AE_TUNING_CUSTOM_H
#define _AE_TUNING_CUSTOM_H

#include <isp_tuning.h>

#define AE_CYCLE_NUM (3)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//         P U B L I C    F U N C T I O N    D E C L A R A T I O N              //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
template <ESensorDev_T eSensorDev>
MBOOL isAEEnabled();

template <ESensorDev_T eSensorDev>
AE_PARAM_T const& getAEParam();

template <ESensorDev_T eSensorDev>
const MINT32* getAEActiveCycle();

template <ESensorDev_T eSensorDev>
MINT32 getAECycleNum();

#endif

