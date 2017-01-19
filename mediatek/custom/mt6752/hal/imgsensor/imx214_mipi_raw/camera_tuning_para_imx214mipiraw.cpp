#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_imx214mipiraw.h"
#include "camera_info_imx214mipiraw.h"
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
        87900,    // i4R_AVG
        18363,    // i4R_STD
        101980,   // i4B_AVG
        20674,    // i4B_STD
        4095,      // i4R_MAX
        512,      // i4R_MIN
        4095,      // i4G_MAX
        512,      // i4G_MIN   
        4095,      // i4B_MAX
        512,      // i4B_MIN
        {  // i4P00[9]
            8646000, -3416000, -108000, -1086000, 6972000, -760000, -68000, -2688000, 7880000
        },
        {  // i4P10[9]
            2298742, -2312280, 19097, 270146, -428082, 161722, 236426, 492700, -740576
        },
        {  // i4P01[9]
            2057908, -1854188, -195203, 25033, -305900, 284797, 111752, -510586, 392214
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
            -13,               // i4BVOffset delta BV = -2.3
            0,//64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            3,                 // u4CaptureFlareThres
            0,//64,                 // u4VideoFlareOffset
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
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo2MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom2MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom3MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom4MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom5MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                50,  //u4B2TStart
                40,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEFaceMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                40,  //u4B2TStart
                30,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAETrackingMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                40,  //u4B2TStart
                30,  //u4D2TEnd
                90,  //u4D2TStart
            },
            //rAEAOENVRAMParam =
            {
                1,      // i4AOEStrengthIdx: 0 / 1 / 2
                128,    // u4BVCompRatio
                {
                    {
                        47,  //u4Y_Target
                     25,//10,  //u4AOE_OE_percent
                     210,//160,  //u4AOE_OEBound
                     10,//15,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                     1,//5,    //u4AOE_LowlightBound
                     145,//100,    //u4AOESceneLV_L
                     180,//170,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                     32,  //u4Y_Target
                     20,//10,  //u4AOE_OE_percent
                     190,  //u4AOE_OEBound
                     15,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                     10,    //u4AOE_LowlightBound
                     145,//100,    //u4AOESceneLV_L
                     180,//170,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                     25,//10,  //u4AOE_OE_percent
                     210,//200,  //u4AOE_OEBound
                        25,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                     8,//15,    //u4AOE_LowlightBound
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
                1075,    // i4R
                512,    // i4G
                693    // i4B
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
                -427,    // i4X
                -402    // i4Y
                        },
                        // A
                        {
                -287,    // i4X
                -409    // i4Y
                        },
                        // TL84
                        {
                -94,    // i4X
                -451    // i4Y
                        },
                        // CWF
                        {
                -56,    // i4X
                -492    // i4Y
                        },
                        // DNP
                        {
                7,    // i4X
                -412    // i4Y
                        },
                        // D65
                        {
                162,    // i4X
                -386    // i4Y
                        },
                        // DF
                        {
                0,    // i4X
                0    // i4Y
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
                -447,    // i4X
                -380    // i4Y
                        },
                        // A
                        {
                -308,    // i4X
                -394    // i4Y
                        },
                        // TL84
                        {
                -117,    // i4X
                -446    // i4Y
                        },
                        // CWF
                        {
                -81,    // i4X
                -489    // i4Y
                        },
                        // DNP
                        {
                -14,    // i4X
                -412    // i4Y
                        },
                        // D65
                        {
                142,    // i4X
                -394    // i4Y
                        },
                        // DF
                        {
                120,    // i4X
                -478    // i4Y
                        }
                },
                // AWB gain of AWB light source
                {
                        // Strobe
                        {
                512,    // i4R
                                512,    // i4G
                512    // i4B
                        },
                        // Horizon
                        {
                512,    // i4R
                530,    // i4G
                1625    // i4B
                        },
                        // A
                        {
                604,    // i4R
                512,    // i4G
                1314    // i4B
                        },
                        // TL84
                        {
                831,    // i4R
                512,    // i4G
                1070    // i4B
                        },
                        // CWF
                        {
                923,    // i4R
                512,    // i4G
                1075    // i4B
                        },
                        // DNP
                        {
                903,    // i4R
                512,    // i4G
                885    // i4B
                        },
                        // D65
                        {
                1075,    // i4R
                512,    // i4G
                693    // i4B
                        },
                        // DF
                        {
                1142,    // i4R
                512,    // i4G
                808    // i4B
                        }
                },
                // Rotation matrix parameter
                {
            3,    // i4RotationAngle
            256,    // i4Cos
            13    // i4Sin
                },
                // Daylight locus parameter
                {
            -143,    // i4SlopeNumerator
                        128    // i4SlopeDenominator
                },
	            // Predictor gain
                {
                        // i4PrefRatio100
                        141,

                        // DaylightLocus_L
                        {
                            1075,    // i4R
                            512, // i4G
                            693,    // i4B
                        },
                        // DaylightLocus_H
                        {
                            774,    // i4R
                            512, // i4G
                            998,    // i4B
                        },
                        // Temporal General
                        {
                            1075,    // i4R
                            512, // i4G
                            693,    // i4B
                        }
                },
                // AWB light area
                {
		// Strobe
                        {
			0,	// StrobeRightBound
			0,	// StrobeLeftBound
			0,	// StrobeUpperBound
			0	// StrobeLowerBound
                        },
                        // Tungsten
                        {
            -193,    // i4RightBound
            -847,    // i4LeftBound
            -345,    // i4UpperBound
            -420    // i4LowerBound
                        },
                        // Warm fluorescent
                        {
            -193,    // i4RightBound
            -847,    // i4LeftBound
            -420,    // i4UpperBound
            -519    // i4LowerBound
                        },
                        // Fluorescent
                        {
            -40,    // i4RightBound
            -193,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // CWF
                        {
            -1,    // i4RightBound
            -193,    // i4LeftBound
            -473,    // i4UpperBound
            -534    // i4LowerBound
                        },
                        // Daylight
                        {
            172,    // i4RightBound
            -40,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Shade
                        {
            502,    // i4RightBound
            172,    // i4LeftBound
            -354,    // i4UpperBound
            -432    // i4LowerBound
                        },
                        // Daylight Fluorescent
                        {
            172,    // i4RightBound
            -1,    // i4LeftBound
            -473,    // i4UpperBound
            -534    // i4LowerBound
                        }
                },
                // PWB light area
                {
                        // Reference area
                        {
            502,    // i4RightBound
            -847,    // i4LeftBound
            -320,    // i4UpperBound
            -534    // i4LowerBound
                        },
                        // Daylight
                        {
            197,    // i4RightBound
            -40,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Cloudy daylight
                        {
            297,    // i4RightBound
            122,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Shade
                        {
            397,    // i4RightBound
            122,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Twilight
                        {
            -40,    // i4RightBound
            -200,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Fluorescent
                        {
            192,    // i4RightBound
            -217,    // i4LeftBound
            -344,    // i4UpperBound
            -539    // i4LowerBound
                        },
                        // Warm fluorescent
                        {
            -208,    // i4RightBound
            -408,    // i4LeftBound
            -344,    // i4UpperBound
            -539    // i4LowerBound
                        },
                        // Incandescent
                        {
            -208,    // i4RightBound
            -408,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
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
            1018,    // i4R
            512,    // i4G
            778    // i4B
                        },
                        // Cloudy daylight
                        {
            1204,    // i4R
            512,    // i4G
            646    // i4B
                        },
                        // Shade
                        {
            1284,    // i4R
            512,    // i4G
            602    // i4B
                        },
                        // Twilight
                        {
            789,    // i4R
            512,    // i4G
            1031    // i4B
                        },
                        // Fluorescent
                        {
            943,    // i4R
            512,    // i4G
            918    // i4B
                        },
                        // Warm fluorescent
                        {
            645,    // i4R
            512,    // i4G
            1396    // i4B
                        },
                        // Incandescent
                        {
            620,    // i4R
            512,    // i4G
            1347    // i4B
                        },
                        // Gray World
                        {
            512,    // i4R
            512,    // i4G
            512    // i4B
                        }
                },
                // AWB preference color
                {
                        // Tungsten
                        {
            60,    // i4SliderValue
            4372    // i4OffsetThr
                        },
                        // Warm fluorescent
                        {
            50,    // i4SliderValue
            4572    // i4OffsetThr
                        },
                        // Shade
                        {
            50,    // i4SliderValue
            909    // i4OffsetThr
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
            515    // i4B
                        },
                        // Preference gain: warm fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: fluorescent
                        {
            514,    // i4R
                                512,    // i4G
            514    // i4B
                        },
                        // Preference gain: CWF
                        {
            508,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: daylight
                        {
            506,    // i4R
                                512,    // i4G
            514    // i4B
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
                        774,        // i4R
                        512, // i4G
                        998,        // i4B
                    },
                    // Parent block weight parameter
                    {
                        1,      // bEnable
                        6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                    },
                    // AWB LV threshold for predictor
                    {
                            115,    // i4InitLVThr_L
                            155,    // i4InitLVThr_H
                            100      // i4EnqueueLVThr
                    },
                    // AWB number threshold for temporal predictor
                    {
                            65,     // i4Neutral_ParentBlk_Thr
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  25,  2,  2,  2,  2,  2,  2,  2,  2}  // (%) i4CWFDF_LUTThr
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
                                -12,   // i4Sunset_BoundXr_Thr
                                -412    // i4Sunset_BoundYr_Thr
                            },
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,          // i4Enable
                            115,        // i4LVThr
                            {
                                -91,   // i4BoundXrThr
                                -394//-91     // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,          // i4Enable
                            110,         // i4LVThr
                            {
                                -81,   // i4BoundXrThr
                                -489//-499//-81     // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Low CCT
                        {
                            1,          // i4Enable
                            256,//384,        // i4LVThr
                            {
                              -467,//-450,// -897,// -497,   // i4BoundXrThr
                                220   // i4BoundYrThr
                            }
                        }
                    },

                    // AWB Gain Limit
                    {
                        // rNormalLowCCT
                        {
                            1,      // Gain Limit Enable
                        1280     // Gain ratio
                        },
                        // rPrefLowCCT
                        {
                            1,      // Gain Limit Enable
                        1280     // Gain ratio
                        }
                    },

                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
		                {100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  70,  30,  20,  10,  0,  0,  0,  0}
                    },

                    // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                    {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Tungsten
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Warm fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  70,  40,  20,  0,   0,   0}, // Fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  50,  25,  12,   0,  0,   0,   0}, // CWF
		                { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  50,  30,  20}, // Daylight
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
            {// Rotated X coordinate
                -589,    // i4RotatedXCoordinate[0]
                -450,    // i4RotatedXCoordinate[1]
                -259,    // i4RotatedXCoordinate[2]
                -156,    // i4RotatedXCoordinate[3]
			    0  // RotatedXCoordinate4
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
                1075,    // i4R
                512,    // i4G
                693    // i4B
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
                -427,    // i4X
                -402    // i4Y
                        },
                        // A
                        {
                -287,    // i4X
                -409    // i4Y
                        },
                        // TL84
                        {
                -94,    // i4X
                -451    // i4Y
                        },
                        // CWF
                        {
                -56,    // i4X
                -492    // i4Y
                        },
                        // DNP
                        {
                7,    // i4X
                -412    // i4Y
                        },
                        // D65
                        {
                162,    // i4X
                -386    // i4Y
                        },
                        // DF
                        {
                0,    // i4X
                0    // i4Y
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
                -447,    // i4X
                -380    // i4Y
                        },
                        // A
                        {
                -308,    // i4X
                -394    // i4Y
                        },
                        // TL84
                        {
                -117,    // i4X
                -446    // i4Y
                        },
                        // CWF
                        {
                -81,    // i4X
                -489    // i4Y
                        },
                        // DNP
                        {
                -14,    // i4X
                -412    // i4Y
                        },
                        // D65
                        {
                142,    // i4X
                -394    // i4Y
                        },
                        // DF
                        {
                120,    // i4X
                -478    // i4Y
                        }
                },
                // AWB gain of AWB light source
                {
                        // Strobe
                        {
                512,    // i4R
                                512,    // i4G
                512    // i4B
                        },
                        // Horizon
                        {
                512,    // i4R
                530,    // i4G
                1625    // i4B
                        },
                        // A
                        {
                604,    // i4R
                512,    // i4G
                1314    // i4B
                        },
                        // TL84
                        {
                831,    // i4R
                512,    // i4G
                1070    // i4B
                        },
                        // CWF
                        {
                923,    // i4R
                512,    // i4G
                1075    // i4B
                        },
                        // DNP
                        {
                903,    // i4R
                512,    // i4G
                885    // i4B
                        },
                        // D65
                        {
                1075,    // i4R
                512,    // i4G
                693    // i4B
                        },
                        // DF
                        {
                1142,    // i4R
                512,    // i4G
                808    // i4B
                        }
                },
                // Rotation matrix parameter
                {
            3,    // i4RotationAngle
            256,    // i4Cos
            13    // i4Sin
                },
                // Daylight locus parameter
                {
            -143,    // i4SlopeNumerator
                        128    // i4SlopeDenominator
                },
	            // Predictor gain
                {
                        // i4PrefRatio100
                        141,

                        // DaylightLocus_L
                        {
                            1075,    // i4R
                            512, // i4G
                            693,    // i4B
                        },
                        // DaylightLocus_H
                        {
                            774,    // i4R
                            512, // i4G
                            998,    // i4B
                        },
                        // Temporal General
                        {
                            1075,    // i4R
                            512, // i4G
                            693,    // i4B
                        }
                },
                // AWB light area
                {
		// Strobe
                        {
			0,	// StrobeRightBound
			0,	// StrobeLeftBound
			0,	// StrobeUpperBound
			0	// StrobeLowerBound
                        },
                        // Tungsten
                        {
            -193,    // i4RightBound
            -847,    // i4LeftBound
            -345,    // i4UpperBound
            -420    // i4LowerBound
                        },
                        // Warm fluorescent
                        {
            -193,    // i4RightBound
            -847,    // i4LeftBound
            -420,    // i4UpperBound
            -519    // i4LowerBound
                        },
                        // Fluorescent
                        {
            -40,    // i4RightBound
            -193,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // CWF
                        {
            -1,    // i4RightBound
            -193,    // i4LeftBound
            -473,    // i4UpperBound
            -534    // i4LowerBound
                        },
                        // Daylight
                        {
            172,    // i4RightBound
            -40,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Shade
                        {
            502,    // i4RightBound
            172,    // i4LeftBound
            -354,    // i4UpperBound
            -432    // i4LowerBound
                        },
                        // Daylight Fluorescent
                        {
            172,    // i4RightBound
            -1,    // i4LeftBound
            -473,    // i4UpperBound
            -534    // i4LowerBound
                        }
                },
                // PWB light area
                {
                        // Reference area
                        {
            502,    // i4RightBound
            -847,    // i4LeftBound
            -320,    // i4UpperBound
            -534    // i4LowerBound
                        },
                        // Daylight
                        {
            197,    // i4RightBound
            -40,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Cloudy daylight
                        {
            297,    // i4RightBound
            122,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Shade
                        {
            397,    // i4RightBound
            122,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Twilight
                        {
            -40,    // i4RightBound
            -200,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
                        },
                        // Fluorescent
                        {
            192,    // i4RightBound
            -217,    // i4LeftBound
            -344,    // i4UpperBound
            -539    // i4LowerBound
                        },
                        // Warm fluorescent
                        {
            -208,    // i4RightBound
            -408,    // i4LeftBound
            -344,    // i4UpperBound
            -539    // i4LowerBound
                        },
                        // Incandescent
                        {
            -208,    // i4RightBound
            -408,    // i4LeftBound
            -354,    // i4UpperBound
            -473    // i4LowerBound
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
            1018,    // i4R
            512,    // i4G
            778    // i4B
                        },
                        // Cloudy daylight
                        {
            1204,    // i4R
            512,    // i4G
            646    // i4B
                        },
                        // Shade
                        {
            1284,    // i4R
            512,    // i4G
            602    // i4B
                        },
                        // Twilight
                        {
            789,    // i4R
            512,    // i4G
            1031    // i4B
                        },
                        // Fluorescent
                        {
            943,    // i4R
            512,    // i4G
            918    // i4B
                        },
                        // Warm fluorescent
                        {
            645,    // i4R
            512,    // i4G
            1396    // i4B
                        },
                        // Incandescent
                        {
            620,    // i4R
            512,    // i4G
            1347    // i4B
                        },
                        // Gray World
                        {
            512,    // i4R
            512,    // i4G
            512    // i4B
                        }
                },
                // AWB preference color
                {
                        // Tungsten
                        {
            60,    // i4SliderValue
            4372    // i4OffsetThr
                        },
                        // Warm fluorescent
                        {
            50,    // i4SliderValue
            4572    // i4OffsetThr
                        },
                        // Shade
                        {
            50,    // i4SliderValue
            909    // i4OffsetThr
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
            515    // i4B
                        },
                        // Preference gain: warm fluorescent
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: fluorescent
                        {
            514,    // i4R
                                512,    // i4G
            514    // i4B
                        },
                        // Preference gain: CWF
                        {
            508,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Preference gain: daylight
                        {
            506,    // i4R
                                512,    // i4G
            514    // i4B
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
                        774,        // i4R
                        512, // i4G
                        998,        // i4B
                    },
                    // Parent block weight parameter
                    {
                        1,      // bEnable
                        6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                    },
                    // AWB LV threshold for predictor
                    {
                            115,    // i4InitLVThr_L
                            155,    // i4InitLVThr_H
                            100      // i4EnqueueLVThr
                    },
                    // AWB number threshold for temporal predictor
                    {
                            65,     // i4Neutral_ParentBlk_Thr
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  25,  2,  2,  2,  2,  2,  2,  2,  2}  // (%) i4CWFDF_LUTThr
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
                                -12,   // i4Sunset_BoundXr_Thr
                                -412    // i4Sunset_BoundYr_Thr
                            },
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,          // i4Enable
                            115,        // i4LVThr
                            {
                                -91,   // i4BoundXrThr
                                -394//-91     // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,          // i4Enable
                            110,         // i4LVThr
                            {
                                -81,   // i4BoundXrThr
                                -489//-499//-81     // i4BoundYrThr
                            },
                            128         // i4DaylightProb
                        },

                        // Low CCT
                        {
                            1,          // i4Enable
                            256,//384,        // i4LVThr
                            {
                              -467,//-450,// -897,// -497,   // i4BoundXrThr
                                220   // i4BoundYrThr
                            }
                        }
                    },

                    // AWB Gain Limit
                    {
                        // rNormalLowCCT
                        {
                            1,      // Gain Limit Enable
                        1280     // Gain ratio
                        },
                        // rPrefLowCCT
                        {
                            1,      // Gain Limit Enable
                        1280     // Gain ratio
                        }
                    },

                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  70,  30,  20,  10,   0,   0,   0,   0}
                    },

                    // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                    {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Tungsten
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  50,  25,  0,   0,   0}, // Warm fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  70,  40,  20,  0,   0,   0}, // Fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  50,  25,  12,   0,  0,   0,   0}, // CWF
		                { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  50,  30,  20}, // Daylight
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
            {// Rotated X coordinate
                -589,    // i4RotatedXCoordinate[0]
                -450,    // i4RotatedXCoordinate[1]
                -259,    // i4RotatedXCoordinate[2]
                -156,    // i4RotatedXCoordinate[3]
			    0  // RotatedXCoordinate4
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


