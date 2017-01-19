#ifndef _CAMERA_CUSTOM_TSF_TBL_H_
#define _CAMERA_CUSTOM_TSF_TBL_H_

typedef struct
{
    int isTsfEn;        // isEnableTSF
    int tsfCtIdx;       // getTSFD65Idx
    int rAWBInput[8];   // getTSFAWBForceInput: lv, cct, fluo, day fluo, rgain, bgain, ggain, rsvd
} CMAERA_TSF_CFG_STRUCT, *PCMAERA_TSF_CFG_STRUCT;

typedef struct
{
    int TSF_para[1620];
} CAMERA_TSF_PARA_STRUCT, *PCAMERA_TSF_PARA_STRUCT;

typedef struct
{
    unsigned int tsf_data[16000];
} CAMERA_TSF_DATA_STRUCT, *PCAMERA_TSF_DATA_STRUCT;

typedef struct
{
    CMAERA_TSF_CFG_STRUCT TSF_CFG;
    int TSF_PARA[1620];
    unsigned int TSF_DATA[16000];
} CAMERA_TSF_TBL_STRUCT, *PCAMERA_TSF_TBL_STRUCT;

#endif //_CAMERA_CUSTOM_TSF_TBL_H_

