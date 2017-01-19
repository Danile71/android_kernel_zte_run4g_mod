#ifndef _AF_TUNING_CUSTOM_H
#define _AF_TUNING_CUSTOM_H

AF_PARAM_T const& getAFParam();
AF_CONFIG_T const& getAFConfig();
#ifdef AFEXTENDCOEF
CustAF_THRES_T const& getAFExtendCoef(MINT32 w, MINT32 h, MINT32 SensorId);
#endif

#endif

