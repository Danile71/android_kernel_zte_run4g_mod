#ifndef _CAMERA_CUSTOM_CAPTURE_NR_H_
#define _CAMERA_CUSTOM_CAPTURE_NR_H_

#include "camera_custom_types.h"

#define DISABLE_CAPTURE_NR (12800)

typedef struct
{
    int hwth;
    int swth;
}Capture_NR_Th_t;

enum EIdxSwNR
{
    eIDX_SWNR_SINGLE_ISO_100 = 0,
    eIDX_SWNR_SINGLE_ISO_200,
    eIDX_SWNR_SINGLE_ISO_400,
    eIDX_SWNR_SINGLE_ISO_800,
    eIDX_SWNR_SINGLE_ISO_1200,
    eIDX_SWNR_SINGLE_ISO_1600,
    eIDX_SWNR_SINGLE_ISO_2000,
    eIDX_SWNR_SINGLE_ISO_2400,
    eIDX_SWNR_SINGLE_ISO_2800,
    eIDX_SWNR_SINGLE_ISO_3200,
    eIDX_SWNR_SINGLE_ISO_ANR_ENC_OFF,
    eIDX_SWNR_SINGLE_ISO_ANR_ENC_ON,
    eIDX_SWNR_MFLL_ISO_100,
    eIDX_SWNR_MFLL_ISO_200,
    eIDX_SWNR_MFLL_ISO_400,
    eIDX_SWNR_MFLL_ISO_800,
    eIDX_SWNR_MFLL_ISO_1200,
    eIDX_SWNR_MFLL_ISO_1600,
    eIDX_SWNR_MFLL_ISO_2000,
    eIDX_SWNR_MFLL_ISO_2400,
    eIDX_SWNR_MFLL_ISO_2800,
    eIDX_SWNR_MFLL_ISO_3200,
    eIDX_SWNR_MFLL_ISO_ANR_ENC_OFF,
    eIDX_SWNR_MFLL_ISO_ANR_ENC_ON,
    eNUM_OF_SWNR_IDX,
};

bool get_capture_nr_th(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        MBOOL const isMfll,
        Capture_NR_Th_t* pTh
        );

// method: <0: nearest, 1: lower, 2: upper>
EIdxSwNR map_ISO_value_to_index(MUINT32 const u4Iso, MBOOL const isMfll, MUINT32 method = 0);
MUINT32  map_index_to_ISO_value(EIdxSwNR const u4Idx);

MBOOL is_to_invoke_swnr_interpolation(MBOOL const isMfll, MUINT32 const u4Iso);

#endif /* _CAMERA_CUSTOM_CAPTURE_NR_H_ */
