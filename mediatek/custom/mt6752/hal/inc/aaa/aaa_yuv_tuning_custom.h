#ifndef __AAA_YUV_TUNING_CUSTOM_H__
#define __AAA_YUV_TUNING_CUSTOM_H__

namespace NSYuvTuning
{
    MUINT32     custom_GetFlashlightGain10X(MINT32 i4SensorDevId);
    MUINT32     custom_BurstFlashlightGain10X(MINT32 i4SensorDevId);
    MDOUBLE     custom_GetYuvFlashlightThreshold(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightFrameCnt(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightDuty(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightStep(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightHighCurrentDuty(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightHighCurrentTimeout(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvAfLampSupport(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvPreflashAF(MINT32 i4SensorDevId);
}

#endif //__AAA_YUV_TUNING_CUSTOM_H__

