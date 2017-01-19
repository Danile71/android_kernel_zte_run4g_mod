#ifndef _ISP_TUNING_CAM_INFO_H_
#define _ISP_TUNING_CAM_INFO_H_

#include <mtkcam/metadata/mtk_metadata_tag.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>

namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/

//  Scene index
typedef mtk_camera_metadata_enum_android_control_scene_mode_t EIndex_Scene_T;
enum { eNUM_OF_SCENE_IDX = MTK_CONTROL_SCENE_MODE_NUM };


//  Color Effect Index
typedef mtk_camera_metadata_enum_android_control_effect_mode_t EIndex_Effect_T;


//  ISP End-User-Define Tuning Index:
//  Edge, Hue, Saturation, Brightness, Contrast
typedef mtk_camera_metadata_enum_android_control_isp_edge_t        EIndex_Isp_Edge_T;
typedef mtk_camera_metadata_enum_android_control_isp_hue_t         EIndex_Isp_Hue_T;
typedef mtk_camera_metadata_enum_android_control_isp_saturation_t  EIndex_Isp_Saturation_T;
typedef mtk_camera_metadata_enum_android_control_isp_brightness_t  EIndex_Isp_Brightness_T;
typedef mtk_camera_metadata_enum_android_control_isp_contrast_t    EIndex_Isp_Contrast_T;

typedef struct IspUsrSelectLevel
{
    EIndex_Isp_Edge_T           eIdx_Edge;
    EIndex_Isp_Hue_T            eIdx_Hue;
    EIndex_Isp_Saturation_T     eIdx_Sat;
    EIndex_Isp_Brightness_T     eIdx_Bright;
    EIndex_Isp_Contrast_T       eIdx_Contrast;

    IspUsrSelectLevel()
        : eIdx_Edge     (MTK_CONTROL_ISP_EDGE_MIDDLE)
        , eIdx_Hue      (MTK_CONTROL_ISP_HUE_MIDDLE)
        , eIdx_Sat      (MTK_CONTROL_ISP_SATURATION_MIDDLE)
        , eIdx_Bright   (MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE)
        , eIdx_Contrast (MTK_CONTROL_ISP_CONTRAST_MIDDLE)
    {}
} IspUsrSelectLevel_T;


//  ISO index.
typedef enum EIndex_ISO
{
    eIDX_ISO_100 = 0,
    eIDX_ISO_200,
    eIDX_ISO_400,
    eIDX_ISO_800,
    eIDX_ISO_1200,
    eIDX_ISO_1600,
    eIDX_ISO_2000,
    eIDX_ISO_2400,
    eIDX_ISO_2800,
    eIDX_ISO_3200,
    eNUM_OF_ISO_IDX
} EIndex_ISO_T;

// CCM index
typedef enum EIndex_CCM
{
    eIDX_CCM_D65  = 0,
    eIDX_CCM_TL84,
    eIDX_CCM_CWF,
    eIDX_CCM_A,
    eIDX_CCM_NUM
} EIndex_CCM_T;

// PCA LUT index
typedef enum EIndex_PCA_LUT
{
    eIDX_PCA_LOW  = 0,
    eIDX_PCA_MIDDLE,
    eIDX_PCA_HIGH,
    eIDX_PCA_LOW_2,
    eIDX_PCA_MIDDLE_2,
    eIDX_PCA_HIGH_2    
} EIndex_PCA_LUT_T;


//  Correlated color temperature index for shading.
typedef enum EIndex_Shading_CCT
{
    eIDX_Shading_CCT_BEGIN  = 0,
    eIDX_Shading_CCT_ALight   = eIDX_Shading_CCT_BEGIN,
    eIDX_Shading_CCT_CWF,
    eIDX_Shading_CCT_D65,
    eIDX_Shading_CCT_RSVD
} EIndex_Shading_CCT_T;

// ISP interpolation info
typedef struct {
    MUINT32 u4CfaUpperISO;
    MUINT32 u4CfaLowerISO;
    MUINT16 u2CfaUpperIdx;
    MUINT16 u2CfaLowerIdx;
    MUINT32 u4AnrUpperISO;
    MUINT32 u4AnrLowerISO;
    MUINT16 u2AnrUpperIdx;
    MUINT16 u2AnrLowerIdx;
    MUINT32 u4EEUpperISO;
    MUINT32 u4EELowerISO;
    MUINT16 u2EEUpperIdx;
    MUINT16 u2EELowerIdx;    
} ISP_INT_INFO_T;

/*******************************************************************************
*
*******************************************************************************/
struct IspCamInfo
{
public:
    EIspProfile_T       eIspProfile; // ISP profile.
    ESensorMode_T       eSensorMode; // sensor mode
    EIndex_Scene_T      eIdx_Scene;  // scene mode.

public:
    IspCamInfo()
    : eIspProfile(EIspProfile_Preview)
    , eSensorMode(ESensorMode_Preview)
    , eIdx_Scene(MTK_CONTROL_SCENE_MODE_UNSUPPORTED)
    {}

public:
    void dump() const
    {
        MY_LOG("[IspCamInfo][dump](eIspProfile, eSensorMode, eIdx_Scene)=(%d, %d, %d)", eIspProfile, eSensorMode, eIdx_Scene);
    }
};


/*******************************************************************************
*
*******************************************************************************/
struct RAWIspCamInfo : public IspCamInfo
{
public:
    MUINT32              u4ISOValue;       // ISO value
    EIndex_ISO_T         eIdx_ISO;         // ISO index
    EIndex_PCA_LUT_T     eIdx_PCA_LUT;     // Index for PCA
    EIndex_CCM_T         eIdx_CCM;         // Index for CCM.
    EIndex_Shading_CCT_T eIdx_Shading_CCT; // Index for Shading.
    AWB_INFO_T           rAWBInfo;         // AWB info for ISP tuning
    AE_INFO_T            rAEInfo;          // AE info for ISP tuning
    AF_INFO_T            rAFInfo;          // AF info for ISP tuning
    FLASH_INFO_T         rFlashInfo;       // Flash info for ISP tuning
    ISP_INT_INFO_T       rIspIntInfo;      // ISP interpolation info
    MINT32               i4ZoomRatio_x100; // Zoom ratio x 100
    MINT32               i4LightValue_x10; // LV x 10

public:
    RAWIspCamInfo()
        : IspCamInfo()
        , u4ISOValue(0)
        , eIdx_ISO(eIDX_ISO_100)
        , eIdx_PCA_LUT(eIDX_PCA_LOW)
        , eIdx_CCM(eIDX_CCM_D65)
        , eIdx_Shading_CCT(eIDX_Shading_CCT_CWF)
        , rAWBInfo()
        , rAEInfo()
        , rAFInfo()
        , rFlashInfo()
        , rIspIntInfo()
        , i4ZoomRatio_x100(0)
        , i4LightValue_x10(0)
    {}

public:
    void dump() const
    {
        IspCamInfo::dump();
        MY_LOG(
            "[RAWIspCamInfo][dump]"
            "(eIdx_ISO, u4ISOValue, i4ZoomRatio_x100, i4LightValue_x10)"
            "=(%d, %d, %d, %d)"
            , eIdx_ISO, u4ISOValue, i4ZoomRatio_x100, i4LightValue_x10
        );
    }
};


/*******************************************************************************
*
*******************************************************************************/
struct YUVIspCamInfo : public IspCamInfo
{
public:
    YUVIspCamInfo()
        : IspCamInfo()
    {}
};


/*******************************************************************************
*
*******************************************************************************/
};  //  NSIspTuning
#endif //  _ISP_TUNING_CAM_INFO_H_

