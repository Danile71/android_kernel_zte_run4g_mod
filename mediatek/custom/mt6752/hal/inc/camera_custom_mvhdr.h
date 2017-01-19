#ifndef _CAMERA_CUSTOM_MVHDR_H_
#define _CAMERA_CUSTOM_MVHDR_H_

#include "camera_custom_types.h"  // For MUINT*/MINT*/MVOID/MBOOL type definitions.
#include "camera_custom_ivhdr.h"

#define CUST_MVHDR_DEBUG          0   // Enable this will dump HDR Debug Information into SDCARD
#define MVHDR_DEBUG_OUTPUT_FOLDER   "/storage/sdcard1/" // For ALPS.JB.
/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/*******************************************************************************
* IVHDR exposure setting
*******************************************************************************/
typedef struct mVHDRInputParam_S
{
    MUINT32 u4SensorID;               //
    MUINT32 u4OBValue;                 // 10 bits
    MUINT32 u4ISPGain;                 // 1x=1024
    MUINT16 u2StatWidth;              // statistic width
    MUINT16 u2StatHight;              // statistic height
    MUINT16 u2ShutterRatio;
    MVOID *pDataPointer;            //
} mVHDRInputParam_T;

typedef struct mVHDROutputParam_S
{
    MBOOL bUpdateSensorAWB;     // MTRUE : update, MFALSE : don't update
    MVOID *pDataPointer;           //
} mVHDROutputParam_T;

typedef struct mVHDR_SWHDR_InputParam_S
{
    MINT32 i4Ratio;
    MINT32 LEMax;
    MINT32 SEMax;
} mVHDR_SWHDR_InputParam_T;

typedef struct mVHDR_SWHDR_OutputParam_S
{
    MINT32 i4SEDeltaEVx100;
} mVHDR_SWHDR_OutputParam_T;

typedef struct mVHDR_TRANSFER_Param_S
{
    MBOOL bSEInput;
    MUINT16 u2SelectMode;    
} mVHDR_TRANSFER_Param_T;


MVOID decodemVHDRStatistic(const mVHDRInputParam_T& rInput, mVHDROutputParam_T& rOutput);
MVOID getMVHDR_AEInfo(const mVHDR_SWHDR_InputParam_T& rInput, mVHDR_SWHDR_OutputParam_T& rOutput);
MVOID getmVHDRExpSetting(mVHDR_TRANSFER_Param_T &rInputParam, IVHDRExpSettingOutputParam_T& rOutput);
MBOOL isSESetting();
	
/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

#endif // _CAMERA_CUSTOM_IVHDR_H_

