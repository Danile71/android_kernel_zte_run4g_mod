#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_hi551mipiraw.h"
#include "camera_info_hi551mipiraw.h"
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
                { // i4P00[9]
                8897296,-2989626,-787686,-1208954,6829026,-500072,170190,-2770908,7720566
                },
                { // i4P10[9]
                1867396,-1257886,-609516,-495040,-44440,539480,-147722,392332,-245110
                },
                { // i4P01[9]
                1628734,-988046,-640704,-716820,-361112,1077932,-114812,-380908,495378
                },
                { // i4P20[9]
                788014,-983900,196062,-43050,119624,-76574,281758,-1043902,762090
                },
                { // i4P11[9]
                -71500,-689612,761476,243148,119000,-362148,286776,-619070,332618
                },
                { // i4P02[9]
                -631502,130466,501236,302926,68298,-371224,43616,-17274,-25994
                }

      },
      AWBGain:{
        // Strobe
        {
          810,    // i4R
          512,    // i4G
          677    // i4B
        },
        // A
        {
            519,    // i4R
            512,    // i4G
            1450    // i4B
        },
        // TL84
        {
            605,    // i4R
            512,    // i4G
            1172    // i4B
        },
        // CWF
        {
            771,    // i4R
            512,    // i4G
            1293    // i4B
        },
        // D65
        {
            810,    // i4R
            512,    // i4G
            677    // i4B
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
            TRUE,            // bEnableStrobeThres
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
                        13,               // i4BVOffset delta BV = -2.3
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            3,                 // u4CaptureFlareThres
            64,                 // u4CaptureFlareOffset
            3,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            3,                 // u4VideoFlareThres
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
                128,    // u4BVCompRatio
                {
                    {
                        47,  //u4Y_Target
                        10,  //u4AOE_OE_percent
                        160,  //u4AOE_OEBound
                     15,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                     5,    //u4AOE_LowlightBound
                     100,    //u4AOESceneLV_L
                        150,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                     10,  //u4AOE_OE_percent
                        180,  //u4AOE_OEBound
                     20,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                     10,    //u4AOE_LowlightBound
                     100,    //u4AOESceneLV_L
                150,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                     10,  //u4AOE_OE_percent
                        200,  //u4AOE_OEBound
                        25,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                     15,    //u4AOE_LowlightBound
                     100,    //u4AOESceneLV_L
                        150,    //u4AOESceneLV_H
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
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        }
                },
                // Original XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                67,    // i4X
                                -273    // i4Y
                        },
                        // Horizon
                        {
                                -493,    // i4X
                                -398    // i4Y
                        },
                        // A
                        {
                                -379,    // i4X
                                -390    // i4Y
                        },
                        // TL84
                        {
                                -244,    // i4X
                                -367    // i4Y
                        },
                        // CWF
                        {
                                -191,    // i4X
                                -493    // i4Y
                        },
                        // DNP
                        {
                                -99,    // i4X
                                -325    // i4Y
                        },
                        // D65
                        {
                                67,    // i4X
                                -273    // i4Y
                        },
                        // DF
                        {
                                67,    // i4X
                                -273    // i4Y
                        }
                },
                // Rotated XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                -1,    // i4X
                                -281    // i4Y
                        },
                        // Horizon
                        {
                                -574,    // i4X
                                -266    // i4Y
                        },
                        // A
                        {
                                -462,    // i4X
                                -286    // i4Y
                        },
                        // TL84
                        {
                                -325,    // i4X
                                -296    // i4Y
                        },
                        // CWF
                        {
                                -304,    // i4X
                                -431    // i4Y
                        },
                        // DNP
                        {
                                -175,    // i4X
                                -291    // i4Y
                        },
                        // D65
                        {
                                -1,    // i4X
                                -281    // i4Y
                        },
                        // DF
                        {
                                -1,    // i4X
                                -281    // i4Y
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
                                512,    // i4R
                                583,    // i4G
                                1946    // i4B
                        },
                        // A
                        {
                                519,    // i4R
                                512,    // i4G
                                1450    // i4B
                        },
                        // TL84
                        {
                                605,    // i4R
                                512,    // i4G
                                1172    // i4B
                        },
                        // CWF
                        {
                                771,    // i4R
                                512,    // i4G
                                1293    // i4B
                        },
                        // DNP
                        {
                                695,    // i4R
                                512,    // i4G
                                909    // i4B
                        },
                        // D65
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        },
                        // DF
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        }
                },
                // Rotation matrix parameter
                {
                        14,    // i4RotationAngle
                        248,    // i4Cos
                        62    // i4Sin
                },
                // Daylight locus parameter
                {
                        -213,    // i4SlopeNumerator
                        128    // i4SlopeDenominator
                },
	            // Predictor gain
                {
                        // i4PrefRatio100
                        101,

                        // DaylightLocus_L
                        {
                            872,    // i4R
                            512,    // i4G
                            832     // i4B
                        },
                        // DaylightLocus_H
                        {
                            811,    // i4R
                            512,    // i4G
                            894     // i4B
                        },
                        // Temporal General
                        {
                            872,    // i4R
                            512,    // i4G
                            832     // i4B
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
                                -375,    // i4RightBound
                                -1025,    // i4LeftBound
                                -226,    // i4UpperBound
                                -326    // i4LowerBound
                        },
                        // Warm fluorescent
                        {
                                -375,    // i4RightBound
                                -1025,    // i4LeftBound
                                -326,    // i4UpperBound
                                -446    // i4LowerBound
                        },
                        // Fluorescent
                        {
                                -225,    // i4RightBound
                                -375,    // i4LeftBound
                                -213,    // i4UpperBound
                                -363    // i4LowerBound
                        },
                        // CWF
                        {
                                -225,    // i4RightBound
                                -375,    // i4LeftBound
                                -363,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        // Daylight
                        {
                                24,    // i4RightBound
                                -225,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        // Shade
                        {
                                384,    // i4RightBound
                                24,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        // Daylight Fluorescent
                        {
                                0,    // i4RightBound
                                0,    // i4LeftBound
                                0,    // i4UpperBound
                                0    // i4LowerBound
                        }
                },
                // PWB light area
                {
                        // Reference area
                        {
                                384,    // i4RightBound
                                -1025,    // i4LeftBound
                                -176,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        // Daylight
                        {
                                49,    // i4RightBound
                                -225,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        // Cloudy daylight
                        {
                                149,    // i4RightBound
                                -26,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        // Shade
                        {
                                249,    // i4RightBound
                                -26,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        // Twilight
                        {
                                -225,    // i4RightBound
                                -385,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        // Fluorescent
                        {
                                49,    // i4RightBound
                                -425,    // i4LeftBound
                                -231,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        // Warm fluorescent
                        {
                                -362,    // i4RightBound
                                -562,    // i4LeftBound
                                -231,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        // Incandescent
                        {
                                -362,    // i4RightBound
                                -562,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        // Gray World
                        {
                                5000,    // i4RightBound
                                -5000,    // i4LeftBound
                                5000,    // i4UpperBound
                                -5000    // i4LowerBound
                        }
                },
                // PWB default gain
                {
                        // Daylight
                        {
                                745,    // i4R
                                512,    // i4G
                                781    // i4B
                        },
                        // Cloudy daylight
                        {
                                863,    // i4R
                                512,    // i4G
                                611    // i4B
                        },
                        // Shade
                        {
                                907,    // i4R
                                512,    // i4G
                                562    // i4B
                        },
                        // Twilight
                        {
                                602,    // i4R
                                512,    // i4G
                                1115    // i4B
                        },
                        // Fluorescent
                        {
                                764,    // i4R
                                512,    // i4G
                                991    // i4B
                        },
                        // Warm fluorescent
                        {
                                583,    // i4R
                                512,    // i4G
                                1555    // i4B
                        },
                        // Incandescent
                        {
                                515,    // i4R
                                512,    // i4G
                                1444    // i4B
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
                                50,    // i4SliderValue
                                5250    // i4OffsetThr
                        },
                        // Warm fluorescent
                        {
                                50,    // i4SliderValue
                                5250    // i4OffsetThr
                        },
                        // Shade
                        {
                                50,    // i4SliderValue
                                851    // i4OffsetThr
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
                    FALSE,

                    // AWB LSC Gain
                    {
                        684,        // i4R
                        512,        // i4G
                        901         // i4B
                    },
                    // Parent block weight parameter
                    {
                        TRUE,      // bEnable
                        6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                    },
                    // AWB LV threshold for predictor
                    {
                            100,    // i4InitLVThr_L
                            140,    // i4InitLVThr_H
                            80      // i4EnqueueLVThr
                    },
                    // AWB number threshold for temporal predictor
                    {
                            65,     // i4Neutral_ParentBlk_Thr
                        //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                            { 100, 100, 100, 100, 100, 100, 100, 100, 50,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                    },
                    // AWB light neutral noise reduction for outdoor
                    {
                        //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        // Non neutral
		                { 10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // Flurescent
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // CWF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                // Daylight
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   2,   4,   4,   4,   4,   4,   4,   4,   4},  // (%)
		                // DF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    // AWB feature detection
                    {
                        // Sunset Prop
                        {
                            1,          // i4Enable
                            130,        // i4LVThr
                            {
                                71,   // i4Sunset_BoundXr_Thr
                                -425     // i4Sunset_BoundYr_Thr
                            },
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,          // i4Enable
                            105,        // i4LVThr
                            {

                                -78,   // i4BoundXrThr
                                -376    // i4BoundYrThr
                            },
                            192         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,          // i4Enable
                            95,         // i4LVThr
                            {
                                -112,   // i4BoundXrThr
                                -470    // i4BoundYrThr
                            },
                            192         // i4DaylightProb
                        },

                        // Low CCT
                        {
                            1,          // i4Enable
                            384,        // i4SpeedRatio
                            {
                            -465,       // i4BoundXrThr
                            237         // i4BoundYrThr
                            }
                        }

                    },

                    // AWB Gain Limit
                    {
                        // rNormalLowCCT
                        {
                            1,      // Gain Limit Enable
                            717     // Gain ratio
                        },
                        // rPrefLowCCT
                        {
                            1,      // Gain Limit Enable
                            870     // Gain ratio
                        }

                    },

                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 75,  50,   30,  20,  10,  0,   0,   0,   0,   0,   0}
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
		                {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  75,  35,  0,   0,   0,  0,   0,   0}, // Strobe
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Tungsten
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Warm fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 50,  25,   0,   0,  0,   0,   0}, // CWF
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // CWF
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  50,  50,  40, 30,  0,   0}, // Daylight
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // Shade
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}  // Daylight fluorescent
    		        }
                },
                {// CCT estimation
                        {// CCT
			                2300,	// i4CCT[0]
			                2850,	// i4CCT[1]
                                4100,    // i4CCT[2]
			                5100,	// i4CCT[3]
			                6500 	// i4CCT[4]
		            },
                        {// Rotated X coordinate
                                -573,    // i4RotatedXCoordinate[0]
                                -461,    // i4RotatedXCoordinate[1]
                                -324,    // i4RotatedXCoordinate[2]
                                -174,    // i4RotatedXCoordinate[3]
			                0 	    // i4RotatedXCoordinate[4]
		            }
	            }
        },
        {
                {
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        }
                },
                {
                        {
                                67,    // i4X
                                -273    // i4Y
                        },
                        {
                                -493,    // i4X
                                -398    // i4Y
                        },
                        {
                                -379,    // i4X
                                -390    // i4Y
                        },
                        {
                                -244,    // i4X
                                -367    // i4Y
                        },
                        {
                                -191,    // i4X
                                -493    // i4Y
                        },
                        {
                                -99,    // i4X
                                -325    // i4Y
                        },
                        {
                                67,    // i4X
                                -273    // i4Y
                        },
                        {
                                67,    // i4X
                                -273    // i4Y
                        }
                },
                {
                        {
                                -1,    // i4X
                                -281    // i4Y
                        },
                        {
                                -574,    // i4X
                                -266    // i4Y
                        },
                        {
                                -462,    // i4X
                                -286    // i4Y
                        },
                        {
                                -325,    // i4X
                                -296    // i4Y
                        },
                        {
                                -304,    // i4X
                                -431    // i4Y
                        },
                        {
                                -175,    // i4X
                                -291    // i4Y
                        },
                        {
                                -1,    // i4X
                                -281    // i4Y
                        },
                        {
                                -1,    // i4X
                                -281    // i4Y
                        }
                },
                {
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        },
                        {
                                512,    // i4R
                                583,    // i4G
                                1946    // i4B
                        },
                        {
                                519,    // i4R
                                512,    // i4G
                                1450    // i4B
                        },
                        {
                                605,    // i4R
                                512,    // i4G
                                1172    // i4B
                        },
                        {
                                771,    // i4R
                                512,    // i4G
                                1293    // i4B
                        },
                        {
                                695,    // i4R
                                512,    // i4G
                                909    // i4B
                        },
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        },
                        {
                                810,    // i4R
                                512,    // i4G
                                677    // i4B
                        }
                },
                {
                        14,    // i4RotationAngle
                        248,    // i4Cos
                        62    // i4Sin
                },
                {
                        -213,    // i4SlopeNumerator
                        128    // i4SlopeDenominator
                },
                {
                        101,
                        {
                            872,    // i4R
                            512, // i4G
                            832     // i4B
                        },
                        {
                            811,    // i4R
                            512, // i4G
                            894     // i4B
                        },
                        {
                            872,    // i4R
                            512, // i4G
                            832     // i4B
                        }
                },
                {
                        {
                                -100, // i4RightBound
                                -250, // i4LeftBound
                                -361, // i4UpperBound
                                -600  // i4LowerBound
                        },
                        {
                                -375,    // i4RightBound
                                -1025,    // i4LeftBound
                                -226,    // i4UpperBound
                                -326    // i4LowerBound
                        },
                        {
                                -375,    // i4RightBound
                                -1025,    // i4LeftBound
                                -326,    // i4UpperBound
                                -446    // i4LowerBound
                        },
                        {
                                -225,    // i4RightBound
                                -375,    // i4LeftBound
                                -213,    // i4UpperBound
                                -363    // i4LowerBound
                        },
                        {
                                -225,    // i4RightBound
                                -375,    // i4LeftBound
                                -363,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        {
                                24,    // i4RightBound
                                -225,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        {
                                384,    // i4RightBound
                                24,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        {
                                0,    // i4RightBound
                                0,    // i4LeftBound
                                0,    // i4UpperBound
                                0    // i4LowerBound
                        }
                },
                {
                        {
                                384,    // i4RightBound
                                -1025,    // i4LeftBound
                                -176,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        {
                                49,    // i4RightBound
                                -225,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        {
                                149,    // i4RightBound
                                -26,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        {
                                249,    // i4RightBound
                                -26,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        {
                                -225,    // i4RightBound
                                -385,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        {
                                49,    // i4RightBound
                                -425,    // i4LeftBound
                                -231,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        {
                                -362,    // i4RightBound
                                -562,    // i4LeftBound
                                -231,    // i4UpperBound
                                -481    // i4LowerBound
                        },
                        {
                                -362,    // i4RightBound
                                -562,    // i4LeftBound
                                -201,    // i4UpperBound
                                -361    // i4LowerBound
                        },
                        {
                                5000,    // i4RightBound
                                -5000,    // i4LeftBound
                                5000,    // i4UpperBound
                                -5000    // i4LowerBound
                        }
                },
                {
                        {
                                745,    // i4R
                                512,    // i4G
                                781    // i4B
                        },
                        {
                                863,    // i4R
                                512,    // i4G
                                611    // i4B
                        },
                        {
                                907,    // i4R
                                512,    // i4G
                                562    // i4B
                        },
                        {
                                602,    // i4R
                                512,    // i4G
                                1115    // i4B
                        },
                        {
                                764,    // i4R
                                512,    // i4G
                                991    // i4B
                        },
                        {
                                583,    // i4R
                                512,    // i4G
                                1555    // i4B
                        },
                        {
                                515,    // i4R
                                512,    // i4G
                                1444    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        }
                },
                {
                        {
                                50,    // i4SliderValue
                                5250    // i4OffsetThr
                        },
                        {
                                50,    // i4SliderValue
                                5250    // i4OffsetThr
                        },
                        {
                                50,	// i4SliderValue
                                851    // i4OffsetThr
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        }
                },
                {
                    FALSE,
                    {
                        684,        // i4R
                        512, // i4G
                        901         // i4B
                    },
                    {
                        TRUE,      // bEnable
                        6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                    },
                    {
                            100,    // i4InitLVThr_L
                            140,    // i4InitLVThr_H
                            80      // i4EnqueueLVThr
                    },
                    {
                            65,     // i4Neutral_ParentBlk_Thr
                            { 100, 100, 100, 100, 100, 100, 100, 100, 50,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                    },
                    {
		                { 10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   2,   4,   4,   4,   4,   4,   4,   4,   4},  // (%)
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    {
                        {
                            1,          // i4Enable
                            130,        // i4LVThr
                            {
                                71,   // i4Sunset_BoundXr_Thr
                                -425     // i4Sunset_BoundYr_Thr
                            },
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },
                        {
                            1,          // i4Enable
                            105,        // i4LVThr
                            {
                                -78,   // i4BoundXrThr
                                -376    // i4BoundYrThr
                            },
                            192         // i4DaylightProb
                        },
                        {
                            1,          // i4Enable
                            95,         // i4LVThr
                            {
                                -112,   // i4BoundXrThr
                                -470    // i4BoundYrThr
                            },
                            192         // i4DaylightProb
                        },
                        {
                            1,          // i4Enable
                            384,        // i4SpeedRatio
                            {
                            -465,       // i4BoundXrThr
                            237         // i4BoundYrThr
                            }
                        }
                    },
                    {
                        {
                            1,      // Gain Limit Enable
                            717     // Gain ratio
                        },
                        {
                            1,      // Gain Limit Enable
                            870     // Gain ratio
                        }
                    },
                    {
                        { 100, 100, 100, 100, 100, 100, 100, 100, 75,  50,   30,  20,  10,  0,   0,   0,   0,   0,   0}
                    },
                    {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
		                {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  75,  35,  0,   0,   0,  0,   0,   0}, // Strobe
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Tungsten
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Warm fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 50,  25,   0,   0,  0,   0,   0}, // Shade
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // CWF
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  50,  50,  40, 30,  0,   0}, // Daylight
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // Shade
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}  // Daylight fluorescent
    		        }
                },

	            // CCT estimation
	            {
		            // CCT
		            {
			                2300,	// i4CCT[0]
			                2850,	// i4CCT[1]
                                4100,    // i4CCT[2]
			                5100,	// i4CCT[3]
			                6500 	// i4CCT[4]
		            },
		            // Rotated X coordinate
		            {
                                -573,    // i4RotatedXCoordinate[0]
                                -461,    // i4RotatedXCoordinate[1]
                                -324,    // i4RotatedXCoordinate[2]
                                -174,    // i4RotatedXCoordinate[3]
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
                0,  // isTsfEn
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


