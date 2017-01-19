#ifndef _ISP_TUNING_H_
#define _ISP_TUNING_H_


namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
typedef enum MERROR_ENUM
{
    MERR_OK         = 0,
    MERR_UNKNOWN    = 0x80000000, // Unknown error
    MERR_UNSUPPORT,
    MERR_BAD_PARAM,
    MERR_BAD_CTRL_CODE,
    MERR_BAD_FORMAT,
    MERR_BAD_ISP_DRV,
    MERR_BAD_NVRAM_DRV,
    MERR_BAD_SENSOR_DRV,
    MERR_BAD_SYSRAM_DRV,
    MERR_SET_ISP_REG,
    MERR_NO_MEM,
    MERR_NO_SYSRAM_MEM,
    MERR_NO_RESOURCE,
    MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND,
    MERR_CUSTOM_NOT_READY,
    MERR_PREPARE_HW,
    MERR_APPLY_TO_HW
} MERROR_ENUM_T;


/*******************************************************************************
* Operation Mode
*******************************************************************************/
typedef enum
{
    EOperMode_Normal    = 0,
    EOperMode_PureRaw,
    EOperMode_Meta,
    EOperMode_EM
} EOperMode_T;

/*******************************************************************************
* Sensor Mode
*******************************************************************************/
typedef enum
{
    ESensorMode_Preview    = 0,
    ESensorMode_Capture,
    ESensorMode_Video,
    ESensorMode_SlimVideo1,
    ESensorMode_SlimVideo2,
    ESensorMode_Custom1,
    ESensorMode_Custom2,
    ESensorMode_Custom3,
    ESensorMode_Custom4,
    ESensorMode_Custom5,
    ESensorMode_NUM
} ESensorMode_T;

/*******************************************************************************
* PCA Mode
*******************************************************************************/
typedef enum
{
    EPCAMode_180BIN    = 0,
    EPCAMode_360BIN,
    EPCAMode_NUM
} EPCAMode_T;

/*******************************************************************************
*
*******************************************************************************/
typedef enum
{
    // Camera1.0/Camera3.0
    EIspProfile_Preview = 0,          // Preview
    EIspProfile_Video,                // Video
    EIspProfile_Capture,              // Capture
    EIspProfile_ZSD_Capture,          // ZSD Capture
    EIspProfile_VSS_Capture,          // VSS Capture
    // Camera1.0
    EIspProfile_PureRAW_Capture,      // Pure RAW Capture
    // N3D
    EIspProfile_N3D_Preview,          // N3D Preview
    EIspProfile_N3D_Video,            // N3D Video
    EIspProfile_N3D_Capture,          // N3D Capture
    // MFB
    EIspProfile_MFB_Capture_EE_Off,   // MFB capture: EE off
    EIspProfile_MFB_Capture_EE_Off_SWNR, // MFB capture with SW NR: EE off
    EIspProfile_MFB_Blending_All_Off, // MFB blending: all off
    EIspProfile_MFB_Blending_All_Off_SWNR, // MFB blending with SW NR: all off
    EIspProfile_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
    EIspProfile_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
    EIspProfile_MFB_PostProc_ANR_EE_SWNR,  // MFB post process with SW NR: capture + ANR + EE
    EIspProfile_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
    EIspProfile_MFB_PostProc_Mixing_SWNR,  // MFB post process with SW NR: mixing + all off
    // vFB
    EIspProfile_VFB_PostProc,         // VFB post process: all off + ANR + CCR + PCA
    // iHDR
    EIspProfile_IHDR_Preview,         // IHDR preview
    EIspProfile_IHDR_Video,           // IHDR video
    // Multi-pass ANR
    EIspProfile_Capture_MultiPass_ANR_1,     // Capture multi pass ANR 1
    EIspProfile_Capture_MultiPass_ANR_2,     // Capture multi pass ANR 2
    EIspProfile_VSS_Capture_MultiPass_ANR_1, // VSS capture multi Pass ANR 1
    EIspProfile_VSS_Capture_MultiPass_ANR_2, // VSS capture multi Pass ANR 2
    EIspProfile_MFB_MultiPass_ANR_1, // MFB multi Pass ANR 1
    EIspProfile_MFB_MultiPass_ANR_2, // MFB multi Pass ANR 2
    EIspProfile_Capture_SWNR, // Capture with SW NR
    EIspProfile_VSS_Capture_SWNR, // VSS capture with SW NR
    EIspProfile_PureRAW_Capture_SWNR, // Pure RAW capture with SW NR
    // mHDR
    EIspProfile_MHDR_Preview,         // MHDR preview
    EIspProfile_MHDR_Video,           // MHDR video
    EIspProfile_MHDR_Capture, // TODO: reserve dedicated NVRAM

    EIspProfile_NUM
} EIspProfile_T;

/*******************************************************************************
*
*******************************************************************************/
typedef enum
{
    ESensorDev_None         = 0x00,
    ESensorDev_Main         = 0x01,
    ESensorDev_Sub          = 0x02,
	ESensorDev_MainSecond   = 0x04,
    ESensorDev_Main3D       = 0x05,
	ESensorDev_Atv          = 0x08
}   ESensorDev_T;

typedef enum
{
    ESensorTG_None = 0,
    ESensorTG_1,
    ESensorTG_2,
}   ESensorTG_T;



};  //  NSIspTuning

#endif //  _ISP_TUNING_H_

