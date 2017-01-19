#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_gc2355mipiraw.h"
#include "camera_info_gc2355mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"


const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      }
    },
    ISPPca: {
#include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
#include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    ISPMfbMixer:{{
      0x01FF0001, // MIX3_CTRL_0
      0x00FF0000, // MIX3_CTRL_1
      0xFFFF0000  // MIX3_SPARE
    }},
    ISPMulitCCM:{
      Poly22:{
        84275,    // i4R_AVG
        18921,    // i4R_STD
        100000,    // i4B_AVG
        22677,    // i4B_STD
        4095,      // i4R_MAX
        512,      // i4R_MIN
        4095,      // i4G_MAX
        512,      // i4G_MIN   
        4095,      // i4B_MAX
        512,      // i4B_MIN
        {  // i4P00[9]
            10325000, -5117500, -85000, -1777500, 8217500, -1322500, -147500, -4402500, 9670000
        },
        {  // i4P10[9]
            3441010,  -4434013,  965617, -337221, -429252,   753645,  314179,  729378, -1063664
        },
        {  // i4P01[9]
            2964535,  -3090097,   97979, -673020, -478382,  1138187,  -50414, -417525,   447540
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
                }

      },
      AWBGain:{
        // Strobe
        {
          1016,    // i4R
          512,    // i4G
          1101    // i4B
        },
        // A
        {
          614,    // i4R
            512,    // i4G
          1253    // i4B
        },
        // TL84
        {
          811,    // i4R
            512,    // i4G
          974    // i4B
        },
        // CWF
        {
          872,    // i4R
            512,    // i4G
          1075    // i4B
        },
        // D65
        {
          1082,    // i4R
            512,    // i4G
          696    // i4B
        },
        // Reserved 1
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        },
        // Reserved 2
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        },
        // Reserved 3
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        }
      },
      Weight:{
        1, // Strobe
        1, // A
        1, // TL84
        1, // CWF
        1, // D65
        1, // Reserved 1
        1, // Reserved 2
        1  // Reserved 3
      }
    },

    //bInvokeSmoothCCM
    bInvokeSmoothCCM: MTRUE
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1195,   // u4MinGain, 1024 base =  1x
            8192,  // u4MaxGain, 16x
            85,     // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            19770,     // u4PreExpUnit
            30,     // u4PreMaxFrameRate
            19770,     // u4VideoExpUnit
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            13139,     // u4CapExpUnit
            24,     // u4CapMaxFrameRate
            1024,   // u4Cap2PreRatio, 1024 base = 1x
            19770,     // u4Video1ExpUnit
            30,     // u4Video1MaxFrameRate
            1024,   // u4Video12PreRatio, 1024 base = 1x
            19770,     // u4Video2ExpUnit
            30,     // u4Video2MaxFrameRate
            1024,   // u4Video22PreRatio, 1024 base = 1x
            19770,     // u4Custom1ExpUnit
            30,     // u4Custom1MaxFrameRate
            1024,   // u4Custom12PreRatio, 1024 base = 1x
            19770,     // u4Custom2ExpUnit
            30,     // u4Custom2MaxFrameRate
            1024,   // u4Custom22PreRatio, 1024 base = 1x
            19770,     // u4Custom3ExpUnit
            30,     // u4Custom3MaxFrameRate
            1024,   // u4Custom32PreRatio, 1024 base = 1x
            19770,     // u4Custom4ExpUnit
            30,     // u4Custom4MaxFrameRate
            1024,   // u4Custom42PreRatio, 1024 base = 1x
            19770,     // u4Custom5ExpUnit
            30,     // u4Custom5MaxFrameRate
            1024,   // u4Custom52PreRatio, 1024 base = 1x
            28,      // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
        },
        // rHistConfig
        {
            4, // 2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            TRUE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            TRUE,            // bEnableCaptureThres
            TRUE,            // bEnableVideoThres
            TRUE,            // bEnableVideo1Thres
            TRUE,            // bEnableVideo2Thres
            TRUE,            // bEnableCustom1Thres
            TRUE,            // bEnableCustom2Thres
            TRUE,            // bEnableCustom3Thres
            TRUE,            // bEnableCustom4Thres
            TRUE,            // bEnableCustom5Thres
            TRUE,            // bEnableStrobeThres
            47,                // u4AETarget
            47,                // u4StrobeAETarget

            50,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            3,               // i4BVOffset delta BV = -2.3
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            3,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            3,                 // u4VideoFlareThres
            64,               // u4CustomFlareOffset
            3,                 //  u4CustomFlareThres
            64,                 // u4StrobeFlareOffset //12 bits
            3,                 // u4StrobeFlareThres // 0.5%
            160,                 // u4PrvMaxFlareThres //12 bit
            0,                 // u4PrvMinFlareThres
            160,                 // u4VideoMaxFlareThres // 12 bit
            0,                 // u4VideoMinFlareThres
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            75,    // u4FlatnessStrength
            //rMeteringSpec
            {
                //rHS_Spec
                {
                    TRUE,//bEnableHistStretch           // enable histogram stretch
                    1024,//u4HistStretchWeight          // Histogram weighting value
                    40, //50, //20,//u4Pcent                      // 1%=10, 0~1000
                    160, //166,//176,//u4Thd                        // 0~255
                    75, //54, //74,//u4FlatThd                    // 0~255

                    120,//u4FlatBrightPcent
                    120,//u4FlatDarkPcent
                    //sFlatRatio
                    {
                        1000,  //i4X1
                        1024,  //i4Y1
                        2400, //i4X2
                        0     //i4Y2
                    },
                    TRUE, //bEnableGreyTextEnhance
                    1800, //u4GreyTextFlatStart, > sFlatRatio.i4X1, < sFlatRatio.i4X2
                    {
                        10,     //i4X1
                        1024,   //i4Y1
                        80,     //i4X2
                        0       //i4Y2
                    }
                },
                //rAOE_Spec
                {
                    TRUE, //bEnableAntiOverExposure
                    1024, //u4AntiOverExpWeight
                    10,    //u4Pcent
                    200,  //u4Thd

                    TRUE, //bEnableCOEP
                    1,    //u4COEPcent
                    106,  //u4COEThd
                    0,  // u4BVCompRatio
                    //sCOEYRatio;     // the outer y ratio
                    {
                        23,   //i4X1
                        1024,  //i4Y1
                        47,   //i4X2
                        0     //i4Y2
                    },
                    //sCOEDiffRatio;  // inner/outer y difference ratio
                    {
                        1500, //i4X1
                        0,    //i4Y1
                        2100, //i4X2
                        1024   //i4Y2
                    }
                },
                //rABL_Spec
                {
                    TRUE,//bEnableBlackLigh
                    1024,//u4BackLightWeigh
                    400,//u4Pcent
                    22,//u4Thd,
                    255, // center luminance
                    256, // final target limitation, 256/128 = 2x
                    //sFgBgEVRatio
                    {
                        2200, //i4X1
                        0,    //i4Y1
                        4000, //i4X2
                        1024   //i4Y2
                    },
                    //sBVRatio
                    {
                        3800,//i4X1
                        0,   //i4Y1
                        5000,//i4X2
                        1024  //i4Y2
                    }
                },
                //rNS_Spec
                {
                    TRUE, // bEnableNightScene
                    5,    //u4Pcent
                    170,  //u4Thd
                    72, //52,   //u4FlatThd

                    200,  //u4BrightTonePcent
                    92, //u4BrightToneThd

                    500,  //u4LowBndPcent
                    5,    //u4LowBndThd
                    26,    //u4LowBndThdLimit

                    50,  //u4FlatBrightPcent;
                    300,   //u4FlatDarkPcent;
                    //sFlatRatio
                    {
                        1200, //i4X1
                        1024, //i4Y1
                        2400, //i4X2
                        0    //i4Y2
                    },
                    //sBVRatio
                    {
                        -500, //i4X1
                        1024,  //i4Y1
                        3000, //i4X2
                        0     //i4Y2
                    },
                    TRUE, // bEnableNightSkySuppresion
                    //sSkyBVRatio
                    {
                        -4000, //i4X1
                        1024, //i4X2
                        -2000,  //i4Y1
                        0     //i4Y2
                    }
                },
                // rTOUCHFD_Spec
                {
                    40, //uMeteringYLowBound;
                    50, //uMeteringYHighBound;
                    40, //uFaceYLowBound;
                    50, //uFaceYHighBound;
                    3,  //uFaceCentralWeight;
                    120,//u4MeteringStableMax;
                    80, //u4MeteringStableMin;
                }
            }, //End rMeteringSpec
            // rFlareSpec
            {
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                96,                                               //u4FlareStdThrHigh;
                48,                                               //u4FlareStdThrLow;
                0,                                                //u4PrvCapFlareDiff;
                4,                                                //u4FlareMaxStepGap_Fast;
                0,                                                //u4FlareMaxStepGap_Slow;
                1800,                                             //u4FlarMaxStepGapLimitBV;
                0,                                                //u4FlareAEStableCount;
            },
            //rAEMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190, //u4Bright2TargetEnd
                20,   //u4Dark2TargetStart
                90, //u4B2TEnd
                70,  //u4B2TStart
                60,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideoMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo2MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom2MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom3MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom4MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom5MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEFaceMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190,  //u4Bright2TargetEnd
                10,    //u4Dark2TargetStart
                80, //u4B2TEnd
                30,  //u4B2TStart
                20,  //u4D2TEnd
                60,  //u4D2TStart
            },

            //rAETrackingMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190,  //u4Bright2TargetEnd
                10,    //u4Dark2TargetStart
                80, //u4B2TEnd
                30,  //u4B2TStart
                20,  //u4D2TEnd
                60,  //u4D2TStart
            },
            //rAEAOENVRAMParam =
            {
                1,      // i4AOEStrengthIdx: 0 / 1 / 2
                130,    // u4BVCompRatio
                {
                    {
                        47,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        10,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        1,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        15, //20,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        3, //10,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        25,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        8,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    }
                }
            }
        }
    },

        // AWB NVRAM
        {
        {
                // AWB calibration data
                {
                        // rUnitGain (unit gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rGoldenGain (golden sample gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rD65Gain (D65 WB gain: 1.0 = 512)
                        {
                                1088, // D65Gain_R
                                512, // D65Gain_G
                                689 // D65Gain_B
                        }
                },
                // Original XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
                            -382, // OriX_Hor
                            -365 // OriY_Hor
                        },
                        // A
                        {
                            -258, // OriX_A
                            -386 // OriY_A
                        },
                        // TL84
                        {
                            -69, // OriX_TL84
                            -429 // OriY_TL84
                        },
                        // CWF
                        {
                            -69, // OriX_CWF
                            -485 // OriY_CWF
                        },
                        // DNP
                        {
                            60, // OriX_DNP
                            -449 // OriY_DNP
                        },
                        // D65
                        {
                            169, // OriX_D65
                            -388 // OriY_D65
                        },
                        // DF
                        {
                            0, // OriX_DF
                            0 // OriY_DF
                        }
                },
                // Rotated XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
                            -382, // RotX_Hor
                            -365 // RotY_Hor
                        },
                        // A
                        {
                            -258, // RotX_A
                            -386 // RotY_A
                        },
                        // TL84
                        {
                            -69, // RotX_TL84
                            -429 // RotY_TL84
                        },
                        // CWF
                        {
                            -69, // RotX_CWF
                            -485 // RotY_CWF
                        },
                        // DNP
                        {
                            60, // RotX_DNP
                            -449 // RotY_DNP
                        },
                        // D65
                        {
                            169, // RotX_D65
                            -388 // RotY_D65
                        },
                        // DF
                        {
                            161, // RotX_DF
                            -473 // RotY_DF
                        }
                },
                // AWB gain of AWB light source
                {
                        // Strobe
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        },
                        // Horizon
                        {
                            512, // AWBGAIN_HOR_R
                            524, // AWBGAIN_HOR_G
                            1441 // AWBGAIN_HOR_B
                        },
                        // A
                        {
                            609, // AWBGAIN_A_R
                            512, // AWBGAIN_A_G
                            1225 // AWBGAIN_A_B
                        },
                        // TL84
                        {
                            833, // AWBGAIN_TL84_R
                            512, // AWBGAIN_TL84_G
                            1004 // AWBGAIN_TL84_B
                        },
                        // CWF
                        {
                            900, // AWBGAIN_CWF_R
                            512, // AWBGAIN_CWF_G
                            1084 // AWBGAIN_CWF_B
                        },
                        // DNP
                        {
                            1020, // AWBGAIN_DNP_R
                            512, // AWBGAIN_DNP_G
                            866 // AWBGAIN_DNP_B
                        },
                        // D65
                        {
                            1088, // AWBGAIN_D65_R
                            512, // AWBGAIN_D65_G
                            689 // AWBGAIN_D65_B
                        },
                        // DF
                        {
                            512, // AWBGAIN_DF_R
                            512, // AWBGAIN_DF_G
                            512 // AWBGAIN_DF_B
                        }
                },
                // Rotation matrix parameter
                {
                        0, // RotationAngle
                        256, // Cos
                        0 // Sin
                },
                // Daylight locus parameter
                {
                        -125, //126,    // i4SlopeNumerator
                        128    // i4SlopeDenominator
                },
	            // Predictor gain
                {
                        141, // i4PrefRatio100
                        // DaylightLocus_L
                        {
                            1088, // i4R
                            512, // i4G
                            689 // i4B
                        },
                        // DaylightLocus_H
                        {
                            854, //839, // i4R
                            512, // i4G
                            878 //893, // i4B
                        },
                        // Temporal General
                        {
                            1088, // i4R
                            512, // i4G
                            689, // i4B
                        }
                },
                // AWB light area
                {
                        // Strobe:FIXME
                        {
                                -100, // i4RightBound
                                -250, // i4LeftBound
                                -361, // i4UpperBound
                                -600  // i4LowerBound
                        },
                        // Tungsten
                        {
                            -145, // TungRightBound
                            -782, // TungLeftBound
                            -330, // TungUpperBound
                            -408 // TungLowerBound
                        },
                        // Warm fluorescent
                        {
                            -145, // WFluoRightBound
                            -782, // WFluoLeftBound
                            -408, // WFluoUpperBound
                            -515 //-485 // WFluoLowerBound
                        },
                        // Fluorescent
                        {
                            28, // FluoRightBound
                            -145, //-164, // FluoLeftBound
                            -348, // FluoUpperBound
                            -464 // FluoLowerBound
                        },
                        // CWF
                        {
                            23, // CWFRightBound
                            -145, //-164, // CWFLeftBound
                            -464, // CWFUpperBound
                            -535 // CWFLowerBound
                        },
                        // Daylight
                        {
                            199, // DayRightBound
                            28, // DayLeftBound
                            -348, // DayUpperBound
                            -464 // DayLowerBound
                        },
                        // Shade
                        {
                            529, // ShadeRightBound
                            199, // ShadeLeftBound
                            -348, // ShadeUpperBound
                            -427 // ShadeLowerBound
                        },
                        // Daylight Fluorescent
                        {
                            199, // DFRightBound
                            23, // DFLeftBound
                            -464, // DFUpperBound
                            -530 // DFLowerBound
                        }
                },
                // PWB light area
                {
                        // Reference area
                        {
                            529, // PRefRightBound
                            -782, // PRefLeftBound
                            -305, // PRefUpperBound
                            -530 // PRefLowerBound
                        },
                        // Daylight
                        {
                            224, // PDayRightBound
                            28, // PDayLeftBound
                            -348, // PDayUpperBound
                            -464 // PDayLowerBound
                        },
                        // Cloudy daylight
                        {
                            324, // PCloudyRightBound
                            149, // PCloudyLeftBound
                            -348, // PCloudyUpperBound
                            -464 // PCloudyLowerBound
                        },
                        // Shade
                        {
                            424, // PShadeRightBound
                            149, // PShadeLeftBound
                            -348, // PShadeUpperBound
                            -464 // PShadeLowerBound
                        },
                        // Twilight
                        {
                            28, // PTwiRightBound
                            -132, // PTwiLeftBound
                            -348, // PTwiUpperBound
                            -464 // PTwiLowerBound
                        },
                        // Fluorescent
                        {
                            219, // PFluoRightBound
                            -169, // PFluoLeftBound
                            -338, // PFluoUpperBound
                            -535 // PFluoLowerBound
                        },
                        // Warm fluorescent
                        {
                            -158, // PWFluoRightBound
                            -358, // PWFluoLeftBound
                            -338, // PWFluoUpperBound
                            -535 // PWFluoLowerBound
                        },
                        // Incandescent
                        {
                            -158, // PIncaRightBound
                            -358, // PIncaLeftBound
                            -348, // PIncaUpperBound
                            -464 // PIncaLowerBound
                        },
                        // Gray World
                        {
                            5000, // PGWRightBound
                            -5000, // PGWLeftBound
                            5000, // PGWUpperBound
                            -5000 // PGWLowerBound
                        }
                },
                // PWB default gain
                {
                        // Daylight
                        {
                            1052, // PWB_Day_R
                            512, // PWB_Day_G
                            748 // PWB_Day_B
                        },
                        // Cloudy daylight
                        {
                            1222, // PWB_Cloudy_R
                            512, // PWB_Cloudy_G
                            644 // PWB_Cloudy_B
                        },
                        // Shade
                        {
                            1307, // PWB_Shade_R
                            512, // PWB_Shade_G
                            602 // PWB_Shade_B
                        },
                        // Twilight
                        {
                            827, // PWB_Twi_R
                            512, // PWB_Twi_G
                            952 // PWB_Twi_B
                        },
                        // Fluorescent
                        {
                            956, // PWB_Fluo_R
                            512, // PWB_Fluo_G
                            894 // PWB_Fluo_B
                        },
                        // Warm fluorescent
                        {
                            652, // PWB_WFluo_R
                            512, // PWB_WFluo_G
                            1311 // PWB_WFluo_B
                        },
                        // Incandescent
                        {
                            626, // PWB_Inca_R
                            512, // PWB_Inca_G
                            1258 // PWB_Inca_B
                        },
                        // Gray World
                        {
                            512, // PWB_GW_R
                            512, // PWB_GW_G
                            512 // PWB_GW_B
                        }
                },
                // AWB preference color
                {
                        // Tungsten
                        {
                            65, // TUNG_SLIDER
                            3972 //3772 //4030//3837 //3537 // TUNG_OFFS
                        },
                        // Warm fluorescent
                        {
                            40, //65, // WFluo_SLIDER
                            4472 //4272 //3972 //4030//3837 //3537 // WFluo_OFFS
                        },
                        // Shade
                        {
                            50, // Shade_SLIDER
                            909 // Shade_OFFS
                        },

                        // Preference gain: strobe
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: tungsten
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: warm fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: CWF
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: daylight
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: shade
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: daylight fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        }
                },

                // Algorithm Tuning Paramter
                {
                    // AWB Backup Enable
                    0,

                    // AWB LSC Gain
                    {
                        854, //839, // i4R
                        512, // i4G
                        878 //893, // i4B
                    },
                    // Parent block weight parameter
                    {
                        1,      // bEnable
                        6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                    },
                    // AWB LV threshold for predictor
                    {
                            115, //100,    // i4InitLVThr_L
                            155, //140,    // i4InitLVThr_H
                            100 //80      // i4EnqueueLVThr
                    },
                    // AWB number threshold for temporal predictor
                    {
                            65,     // i4Neutral_ParentBlk_Thr
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                    },
                    // AWB light neutral noise reduction for outdoor
                    {
                        //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        // Non neutral
		                { 3,   3,   3,   3,   3,   3,   3,   3,    3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // Flurescent
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // CWF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // Daylight
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
		                // DF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    // AWB feature detection
                    {
                        // Sunset Prop
                        {
                            1,          // i4Enable
                            140,        // i4LVThr
                            {
                                92, //64,   // i4Sunset_BoundXr_Thr
                                -449 //-463     // i4Sunset_BoundYr_Thr
                            },
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,          // i4Enable
                            115, //105,        // i4LVThr
                            {

                                -37, //-82,   // i4BoundXrThr
                                -388 //-415    // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,          // i4Enable
                            105, //95,         // i4LVThr
                            {
                                -69, //-89,   // i4BoundXrThr
                                -485 //-509    // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Low CCT
                        {
                            1,          // i4Enable
                            256,        // i4SpeedRatio
                            {
                                -402, //-432, //-450,       // i4BoundXrThr
                                232 //411 //450         // i4BoundYrThr
                            }
                        }

                    },

                    // AWB Gain Limit
                    {
                        // rNormalLowCCT
                        {
                            1,      // Gain Limit Enable
                            1536 //845 //819//768//717     // Gain ratio
                        },
                        // rPrefLowCCT
                        {
                            1,      // Gain Limit Enable
                            1536 //832     // Gain ratio
                        }

                    },

                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  70,  30,  20,  10,   0,   0,   0,   0}
                    },

                    // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                    {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
		                //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  75,  35,  0,   0,   0,  0,   0,   0}, // Strobe
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Tungsten
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Warm fluorescent
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 50,  25,  0,   0,  0,   0,   0}, // Fluorescent
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // CWF
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  50,  50,  40, 30,  0,   0}, // Daylight
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // Shade
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}  // Daylight fluorescent
                        //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Tungsten
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Warm fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  70,  40,  20,  0,   0,   0}, // Fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  50,  25,  12,   0,  0,   0,   0}, // CWF
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 50,  30,  20}, // Daylight
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Shade
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  50,  25,  12,   0,  0,   0,   0}  // Daylight fluorescent
    		        }
                },

	            // CCT estimation
	            {
		            // CCT
		            {
			                2300,	// i4CCT[0]
			                2850,	// i4CCT[1]
			                3750,	// i4CCT[2]
			                5100,	// i4CCT[3]
			                6500 	// i4CCT[4]
		            },
		            // Rotated X coordinate
		            {
			                -551,	// i4RotatedXCoordinate[0]
			                -427,	// i4RotatedXCoordinate[1]
			                -238,	// i4RotatedXCoordinate[2]
			                -109,	// i4RotatedXCoordinate[3]
			                0 	    // i4RotatedXCoordinate[4]
		            }
	            }
        },
                {
                // AWB calibration data
                {
                        // rUnitGain (unit gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rGoldenGain (golden sample gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rD65Gain (D65 WB gain: 1.0 = 512)
                        {
                                1088, // D65Gain_R
                                512, // D65Gain_G
                                689 // D65Gain_B
                        }
                },
                // Original XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
                            -382, // OriX_Hor
                            -365 // OriY_Hor
                        },
                        // A
                        {
                            -258, // OriX_A
                            -386 // OriY_A
                        },
                        // TL84
                        {
                            -69, // OriX_TL84
                            -429 // OriY_TL84
                        },
                        // CWF
                        {
                            -69, // OriX_CWF
                            -485 // OriY_CWF
                        },
                        // DNP
                        {
                            60, // OriX_DNP
                            -449 // OriY_DNP
                        },
                        // D65
                        {
                            169, // OriX_D65
                            -388 // OriY_D65
                        },
                        // DF
                        {
                            0, // OriX_DF
                            0 // OriY_DF
                        }
                },
                // Rotated XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
                            -382, // RotX_Hor
                            -365 // RotY_Hor
                        },
                        // A
                        {
                            -258, // RotX_A
                            -386 // RotY_A
                        },
                        // TL84
                        {
                            -69, // RotX_TL84
                            -429 // RotY_TL84
                        },
                        // CWF
                        {
                            -69, // RotX_CWF
                            -485 // RotY_CWF
                        },
                        // DNP
                        {
                            60, // RotX_DNP
                            -449 // RotY_DNP
                        },
                        // D65
                        {
                            169, // RotX_D65
                            -388 // RotY_D65
                        },
                        // DF
                        {
                            161, // RotX_DF
                            -473 // RotY_DF
                        }
                },
                // AWB gain of AWB light source
                {
                        // Strobe
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        },
                        // Horizon
                        {
                            512, // AWBGAIN_HOR_R
                            524, // AWBGAIN_HOR_G
                            1441 // AWBGAIN_HOR_B
                        },
                        // A
                        {
                            609, // AWBGAIN_A_R
                            512, // AWBGAIN_A_G
                            1225 // AWBGAIN_A_B
                        },
                        // TL84
                        {
                            833, // AWBGAIN_TL84_R
                            512, // AWBGAIN_TL84_G
                            1004 // AWBGAIN_TL84_B
                        },
                        // CWF
                        {
                            900, // AWBGAIN_CWF_R
                            512, // AWBGAIN_CWF_G
                            1084 // AWBGAIN_CWF_B
                        },
                        // DNP
                        {
                            1020, // AWBGAIN_DNP_R
                            512, // AWBGAIN_DNP_G
                            866 // AWBGAIN_DNP_B
                        },
                        // D65
                        {
                            1088, // AWBGAIN_D65_R
                            512, // AWBGAIN_D65_G
                            689 // AWBGAIN_D65_B
                        },
                        // DF
                        {
                            512, // AWBGAIN_DF_R
                            512, // AWBGAIN_DF_G
                            512 // AWBGAIN_DF_B
                        }
                },
                // Rotation matrix parameter
                {
                        0, // RotationAngle
                        256, // Cos
                        0 // Sin
                },
                // Daylight locus parameter
                {
                        -125, //126,    // i4SlopeNumerator
                        128    // i4SlopeDenominator
                },
	            // Predictor gain
                {
                        141, // i4PrefRatio100
                        // DaylightLocus_L
                        {
                            1088, // i4R
                            512, // i4G
                            689 // i4B
                        },
                        // DaylightLocus_H
                        {
                            854, //839, // i4R
                            512, // i4G
                            878 //893, // i4B
                        },
                        // Temporal General
                        {
                            1088, // i4R
                            512, // i4G
                            689, // i4B
                        }
                },
                // AWB light area
                {
                        // Strobe:FIXME
                        {
                                -100, // i4RightBound
                                -250, // i4LeftBound
                                -361, // i4UpperBound
                                -600  // i4LowerBound
                        },
                        // Tungsten
                        {
                            -145, // TungRightBound
                            -782, // TungLeftBound
                            -330, // TungUpperBound
                            -408 // TungLowerBound
                        },
                        // Warm fluorescent
                        {
                            -145, // WFluoRightBound
                            -782, // WFluoLeftBound
                            -408, // WFluoUpperBound
                            -515 //-485 // WFluoLowerBound
                        },
                        // Fluorescent
                        {
                            28, // FluoRightBound
                            -145, //-164, // FluoLeftBound
                            -348, // FluoUpperBound
                            -464 // FluoLowerBound
                        },
                        // CWF
                        {
                            23, // CWFRightBound
                            -145, //-164, // CWFLeftBound
                            -464, // CWFUpperBound
                            -535 // CWFLowerBound
                        },
                        // Daylight
                        {
                            199, // DayRightBound
                            28, // DayLeftBound
                            -348, // DayUpperBound
                            -464 // DayLowerBound
                        },
                        // Shade
                        {
                            529, // ShadeRightBound
                            199, // ShadeLeftBound
                            -348, // ShadeUpperBound
                            -427 // ShadeLowerBound
                        },
                        // Daylight Fluorescent
                        {
                            199, // DFRightBound
                            23, // DFLeftBound
                            -464, // DFUpperBound
                            -530 // DFLowerBound
                        }
                },
                // PWB light area
                {
                        // Reference area
                        {
                            529, // PRefRightBound
                            -782, // PRefLeftBound
                            -305, // PRefUpperBound
                            -530 // PRefLowerBound
                        },
                        // Daylight
                        {
                            224, // PDayRightBound
                            28, // PDayLeftBound
                            -348, // PDayUpperBound
                            -464 // PDayLowerBound
                        },
                        // Cloudy daylight
                        {
                            324, // PCloudyRightBound
                            149, // PCloudyLeftBound
                            -348, // PCloudyUpperBound
                            -464 // PCloudyLowerBound
                        },
                        // Shade
                        {
                            424, // PShadeRightBound
                            149, // PShadeLeftBound
                            -348, // PShadeUpperBound
                            -464 // PShadeLowerBound
                        },
                        // Twilight
                        {
                            28, // PTwiRightBound
                            -132, // PTwiLeftBound
                            -348, // PTwiUpperBound
                            -464 // PTwiLowerBound
                        },
                        // Fluorescent
                        {
                            219, // PFluoRightBound
                            -169, // PFluoLeftBound
                            -338, // PFluoUpperBound
                            -535 // PFluoLowerBound
                        },
                        // Warm fluorescent
                        {
                            -158, // PWFluoRightBound
                            -358, // PWFluoLeftBound
                            -338, // PWFluoUpperBound
                            -535 // PWFluoLowerBound
                        },
                        // Incandescent
                        {
                            -158, // PIncaRightBound
                            -358, // PIncaLeftBound
                            -348, // PIncaUpperBound
                            -464 // PIncaLowerBound
                        },
                        // Gray World
                        {
                            5000, // PGWRightBound
                            -5000, // PGWLeftBound
                            5000, // PGWUpperBound
                            -5000 // PGWLowerBound
                        }
                },
                // PWB default gain
                {
                        // Daylight
                        {
                            1052, // PWB_Day_R
                            512, // PWB_Day_G
                            748 // PWB_Day_B
                        },
                        // Cloudy daylight
                        {
                            1222, // PWB_Cloudy_R
                            512, // PWB_Cloudy_G
                            644 // PWB_Cloudy_B
                        },
                        // Shade
                        {
                            1307, // PWB_Shade_R
                            512, // PWB_Shade_G
                            602 // PWB_Shade_B
                        },
                        // Twilight
                        {
                            827, // PWB_Twi_R
                            512, // PWB_Twi_G
                            952 // PWB_Twi_B
                        },
                        // Fluorescent
                        {
                            956, // PWB_Fluo_R
                            512, // PWB_Fluo_G
                            894 // PWB_Fluo_B
                        },
                        // Warm fluorescent
                        {
                            652, // PWB_WFluo_R
                            512, // PWB_WFluo_G
                            1311 // PWB_WFluo_B
                        },
                        // Incandescent
                        {
                            626, // PWB_Inca_R
                            512, // PWB_Inca_G
                            1258 // PWB_Inca_B
                        },
                        // Gray World
                        {
                            512, // PWB_GW_R
                            512, // PWB_GW_G
                            512 // PWB_GW_B
                        }
                },
                // AWB preference color
                {
                        // Tungsten
                        {
                            65, // TUNG_SLIDER
                            3972 //3772 //4030//3837 //3537 // TUNG_OFFS
                        },
                        // Warm fluorescent
                        {
                            40, //65, // WFluo_SLIDER
                            4472 //4272 //3972 //4030//3837 //3537 // WFluo_OFFS
                        },
                        // Shade
                        {
                            50, // Shade_SLIDER
                            909 // Shade_OFFS
                        },

                        // Preference gain: strobe
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: tungsten
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: warm fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: CWF
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: daylight
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: shade
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: daylight fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        }
                },

                // Algorithm Tuning Paramter
                {
                    // AWB Backup Enable
                    0,

                    // AWB LSC Gain
                    {
                        854, //839, // i4R
                        512, // i4G
                        878 //893, // i4B
                    },
                    // Parent block weight parameter
                    {
                        1,      // bEnable
                        6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                    },
                    // AWB LV threshold for predictor
                    {
                            115, //100,    // i4InitLVThr_L
                            155, //140,    // i4InitLVThr_H
                            100 //80      // i4EnqueueLVThr
                    },
                    // AWB number threshold for temporal predictor
                    {
                            65,     // i4Neutral_ParentBlk_Thr
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                    },
                    // AWB light neutral noise reduction for outdoor
                    {
                        //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        // Non neutral
		                { 3,   3,   3,   3,   3,   3,   3,   3,    3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // Flurescent
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // CWF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // Daylight
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
		                // DF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    // AWB feature detection
                    {
                        // Sunset Prop
                        {
                            1,          // i4Enable
                            140,        // i4LVThr
                            {
                                92, //64,   // i4Sunset_BoundXr_Thr
                                -449 //-463     // i4Sunset_BoundYr_Thr
                            },
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,          // i4Enable
                            115, //105,        // i4LVThr
                            {

                                -37, //-82,   // i4BoundXrThr
                                -388 //-415    // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,          // i4Enable
                            105, //95,         // i4LVThr
                            {
                                -69, //-89,   // i4BoundXrThr
                                -485 //-509    // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Low CCT
                        {
                            1,          // i4Enable
                            256,        // i4SpeedRatio
                            {
                                -402, //-432, //-450,       // i4BoundXrThr
                                232 //411 //450         // i4BoundYrThr
                            }
                        }

                    },

                    // AWB Gain Limit
                    {
                        // rNormalLowCCT
                        {
                            1,      // Gain Limit Enable
                            1536 //845 //819//768//717     // Gain ratio
                        },
                        // rPrefLowCCT
                        {
                            1,      // Gain Limit Enable
                            1536 //832     // Gain ratio
                        }

                    },

                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  70,  30,  20,  10,   0,   0,   0,   0}
                    },

                    // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                    {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
		                //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  75,  35,  0,   0,   0,  0,   0,   0}, // Strobe
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Tungsten
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Warm fluorescent
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 50,  25,  0,   0,  0,   0,   0}, // Fluorescent
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // CWF
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  50,  50,  40, 30,  0,   0}, // Daylight
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // Shade
                        //{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}  // Daylight fluorescent
                        //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Tungsten
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Warm fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  70,  40,  20,  0,   0,   0}, // Fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  50,  25,  12,   0,  0,   0,   0}, // CWF
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 50,  30,  20}, // Daylight
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Shade
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  50,  25,  12,   0,  0,   0,   0}  // Daylight fluorescent
    		        }
                },

	            // CCT estimation
	            {
		            // CCT
		            {
			                2300,	// i4CCT[0]
			                2850,	// i4CCT[1]
			                3750,	// i4CCT[2]
			                5100,	// i4CCT[3]
			                6500 	// i4CCT[4]
		            },
		            // Rotated X coordinate
		            {
			                -551,	// i4RotatedXCoordinate[0]
			                -427,	// i4RotatedXCoordinate[1]
			                -238,	// i4RotatedXCoordinate[2]
			                -109,	// i4RotatedXCoordinate[3]
			                0 	    // i4RotatedXCoordinate[4]
		            }
	            }
        }
        },

    // Flash AWB NVRAM
    {
#include INCLUDE_FILENAME_FLASH_AWB_PARA
    },

    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    {
        1,  // isTsfEn
        2,  // tsfCtIdx
        {20, 2000, -110, -110, 512, 512, 512, 0}    // rAWBInput[8]
    },

#include INCLUDE_FILENAME_TSF_PARA
#include INCLUDE_FILENAME_TSF_DATA
};

const NVRAM_CAMERA_FEATURE_STRUCT CAMERA_FEATURE_DEFAULT_VALUE =
{
#include INCLUDE_FILENAME_FEATURE_PARA
};

typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
  template <>
  UINT32
  SensorInfoSingleton_T::
  impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
  {
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
        sizeof(NVRAM_CAMERA_3A_STRUCT),
        sizeof(NVRAM_CAMERA_SHADING_STRUCT),
        sizeof(NVRAM_LENS_PARA_STRUCT),
        sizeof(AE_PLINETABLE_T),
        0,
        sizeof(CAMERA_TSF_TBL_STRUCT),
        0,
        sizeof(NVRAM_CAMERA_FEATURE_STRUCT)
    };

    if (CameraDataType > CAMERA_NVRAM_DATA_FEATURE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
      return 1;
    }

    switch(CameraDataType)
    {
      case CAMERA_NVRAM_DATA_ISP:
        memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_3A:
        memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_SHADING:
        memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
        break;
      case CAMERA_DATA_AE_PLINETABLE:
        memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
        break;
      case CAMERA_DATA_TSF_TABLE:
        memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_FEATURE:
        memcpy(pDataBuf,&CAMERA_FEATURE_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
        break;
      default:
        break;
    }
    return 0;
  }};  //  NSFeature


