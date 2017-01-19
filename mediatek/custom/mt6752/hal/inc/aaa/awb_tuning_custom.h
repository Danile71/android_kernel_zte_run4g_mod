#ifndef _AWB_TUNING_CUSTOM_H_
#define _AWB_TUNING_CUSTOM_H_

#include <isp_tuning.h>

template <ESensorDev_T eSensorDev>
MBOOL isAWBEnabled();

template <ESensorDev_T eSensorDev>
AWB_PARAM_T const& getAWBParam();

template <ESensorDev_T eSensorDev>
AWB_STAT_PARAM_T const& getAWBStatParam();

#endif

