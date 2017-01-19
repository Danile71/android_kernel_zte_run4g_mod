#ifndef _FLASH_AWB_PARAM_H
#define _FLASH_AWB_PARAM_H

#include <camera_custom_nvram.h>

//#define FLASH_AWB_PC_SIM



typedef struct
{
    AWB_GAIN_T NoFlashWBGain;
    AWB_GAIN_T PureFlashWBGain;
    MUINT32 PureFlashWeight;
}FLASH_AWB_CAL_GAIN_INPUT_T;

typedef struct
{
    MINT32 flashDuty;
    MINT32 flashStep;
    MUINT32 flashAwbWeight;
}FLASH_AWB_PASS_FLASH_INFO_T;



typedef struct
{
    FLASH_AWB_TUNING_PARAM_T flash_awb_tuning_param;
}FLASH_AWB_INIT_T, *PFLASH_AWB_INIT_T;

typedef struct
{
    MUINT32 Hr;
    MUINT32 Mr;
    MUINT32 Lr;
    MUINT32 Midx;
}FLASH_AWB_DISTANCE_INFO_T;


typedef struct
{
    int x1;
    int x2;
    int x3;
    int x4;
    int x5;

    int y0;
    int y1;
    int y2;
    int y3;
    int y4;
    int y5;
}REFLECTION_TBL_IDX;


typedef struct
{
    int* pEstNoFlashY;
    int* pEstFlashY;
    double* aeCoef;
    double* pureCoef;
    
    int i4SceneLV; // current scene LV

    int FlashDuty;
    int FlashStep;
    
}FLASH_AWB_INPUT_T;



typedef struct
{
	AWB_GAIN_T rAWBGain;
} FLASH_AWB_OUTPUT_T;

#endif
