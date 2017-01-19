#include "camera_custom_types.h"
#include "camera_custom_nvram.h"
#include "awb_feature.h"
#include "awb_param.h"
#include "awb_tuning_custom.h"

using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
MBOOL
isAWBEnabled<ESensorDev_MainSecond>()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
AWB_PARAM_T const&
getAWBParam<ESensorDev_MainSecond>()
{
    static AWB_PARAM_T rAWBParam =
    {
    	// Chip dependent parameter
    	{
    	    512, // i4AWBGainOutputScaleUnit: 1.0x = 512
    	   8191, // i4AWBGainOutputUpperLimit: format 4.9 (11 bit)
    	    256  // i4RotationMatrixUnit: 1.0x = 256
    	},

        // AWB Light source probability look-up table (Max: 100; Min: 0)
    	{
            AWB_LV_INDEX_NUM, // i4SizeX: horizontal dimension
    	    AWB_LIGHT_NUM, // i4SizeY: vertical dimension
    	    // LUT
    		{ //  LV0    1      2     3     4     5      6     7     8     9      10    11    12     13    14    15    16     17    18
			    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, // Strobe
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Tungsten
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Warm fluorescent
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,  33,  66, 100, 100, 100, 100, 100}, // Fluorescent
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // CWF
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, // Daylight
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}, // Shade
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}  // Daylight fluorescent
    		}
    	},

    	// AWB convergence parameter
    	{
            10, // i4Speed: Convergence speed: (0 ~ 100)
            100,//225 // i4StableThr: Stable threshold ((currentRgain - targetRgain)^2 + (currentBgain - targetBgain)^2), WB gain format: 4.9
    	},

        // AWB daylight locus target offset ratio LUT for tungsten
    	{
    	    AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus new offset (0~10000) as index to get daylight locus target offset ratio (0~100)
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
    	       50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  100
    		}
    	},

        // AWB daylight locus target offset ratio LUT for warm fluorescent
    	{
    	    AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus new offset (0~10000) as index to get daylight locus target offset ratio (0~100)
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
    	       50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  100
    		}
    	},

    	// AWB green offset threshold for warm fluorescent
    	{
    	    AWB_DAYLIGHT_LOCUS_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus offset (0~10000) as index to get green offset threshold
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000  6500  7000  7500  8000  8500  9000  9500 10000
    	      600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,  600,  600,  600,  750,   900, 1050, 1200
    		}
    	},

        // AWB light source weight LUT for tungsten light
    	{
            AWB_TUNGSTEN_MAGENTA_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
    	     //  0  100  200  300  400  500  600  700  800  900 1000
    	       256, 256, 256, 256, 256, 256, 256, 128,  64,  32,  16
    		}
    	},

        // AWB light source weight LUT for warm fluorescent
    	{
            AWB_WARM_FLUORESCENT_GREEN_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use green offset (0~2000) as index to get fluorescent0 weight (x/256)
    	     //  0  200  400  600  800 1000 1200 1400 1600 1800 2000
    	       256, 256, 256, 256, 128,  64,  32,  16,  16,  16,  16
    		}
    	},

        // AWB light source weight LUT for shade light
    	{
            AWB_SHADE_MAGENTA_OFFSET_INDEX_NUM, // i4MagentaLUTSize: Magenta LUT dimension
    		{// MagentaLUT: use magenta offset (0~1000) as index to get shade light weight (x/256)
    	     //  0  100  200  300  400  500  600  700 800 900 1000
    	       256, 256, 128, 56, 28,  16,  16,  16, 16, 16, 16
    		},
    	    AWB_SHADE_GREEN_OFFSET_INDEX_NUM, // i4GreenLUTSize: Green LUT dimension
    		{// GreenLUT: use green offset (0~1000) as index to get shade light weight (x/256)
    	     // 0   100  200  300  400  500  600  700 800 900 1000
    	     //  256, 256, 256, 256, 256, 128,  64,  32, 16, 16, 16
    	       256, 256, 128, 64, 32, 32,  16,  16, 16, 16, 16
    		}
    	},

    	// One-shot AWB parameter
    	{
            MFALSE,
    	    10, // LV 1.0
    	    50  // LV 5.0
    	},

    	// AWB gain prediction parameter
    	{
            // Strobe
		    {
			    0,      // i4IntermediateSceneLvThr_L1: useless
                0,      // i4IntermediateSceneLvThr_H1: useless
    			105, //100, //90,     // i4IntermediateSceneLvThr_L2
                135, //130, //120,    // i4IntermediateSceneLvThr_H2
			    0,      // i4DaylightLocusLvThr_L: useless
                0       // i4DaylightLocusLvThr_H: useless
		    },
            // Tungsten
    		{
    	        100,    // i4IntermediateSceneLvThr_L1
                130,    // i4IntermediateSceneLvThr_H1
    			120, //115, //105,    // i4IntermediateSceneLvThr_L2
                160, //155, //145,    // i4IntermediateSceneLvThr_H2
    			 50,    // i4DaylightLocusLvThr_L
                100     // i4DaylightLocusLvThr_H
    		},
            // Warm fluorescent
    		{
    			100,    // i4IntermediateSceneLvThr_L1
                130,    // i4IntermediateSceneLvThr_H1
    			120, //115, //105,    // i4IntermediateSceneLvThr_L2
                160, //155, //145,    // i4IntermediateSceneLvThr_H2
    			50,     // i4DaylightLocusLvThr_L
                100     // i4DaylightLocusLvThr_H
    		},
            // Fluorescent
    		{
    		    0,      // i4IntermediateSceneLvThr_L1: useless
                0,      // i4IntermediateSceneLvThr_H1: useless
    			125, //120 ,//110,    // i4IntermediateSceneLvThr_L2
                165, //160, //150,    // i4IntermediateSceneLvThr_H2
    			0,      // i4DaylightLocusLvThr_L: useless
                0       // i4DaylightLocusLvThr_H: useless
    		},
            // CWF
    		{
    			0,      // i4IntermediateSceneLvThr_L1: useless
                0,      // i4IntermediateSceneLvThr_H1: useless
    			115, //110, //100,    // i4IntermediateSceneLvThr_L2
                155, //150, //140,    // i4IntermediateSceneLvThr_H2
    			0,      // i4DaylightLocusLvThr_L: useless
                0       // i4DaylightLocusLvThr_H: useless
    		},
            // Daylight
    		{
    			0,      // i4IntermediateSceneLvThr_L1: useless
                0,      // i4IntermediateSceneLvThr_H1: useless
    			135, //130, //120,    // i4IntermediateSceneLvThr_L2
                175, //170, //160,    // i4IntermediateSceneLvThr_H2
    			0,      // i4DaylightLocusLvThr_L: useless
                0       // i4DaylightLocusLvThr_H: useless
    		},
            // Daylight fluorescent
    		{
    			0,      // i4IntermediateSceneLvThr_L1: useless
                0,      // i4IntermediateSceneLvThr_H1: useless
    			115, //110, //100,    // i4IntermediateSceneLvThr_L2
                155, //150, //140 ,   // i4IntermediateSceneLvThr_H2
    			0,      // i4DaylightLocusLvThr_L: useless
                0       // i4DaylightLocusLvThr_H: useless
    		},
            // Shade
    		{
    			100,    // i4IntermediateSceneLvThr_L1
                130,    // i4IntermediateSceneLvThr_H1
    			115, //110, //100,    // i4IntermediateSceneLvThr_L2
                145, //140, //130,    // i4IntermediateSceneLvThr_H2
    			50,     // i4DaylightLocusLvThr_L
                100     // i4DaylightLocusLvThr_H
    		}
    	},

    	// Daylight locus offset LUTs for tungsten
        {
            AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
            {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
            {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000} // i4LUTOut
        },

        // Daylight locus offset LUTs for WF
        {
            AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
            {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
            {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000} // i4LUTOut
        },

        // Daylight locus offset LUTs for shade
        {
            AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
            {0, 100, 200,  300,  400,  500,  600,  700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000}, // i4LUTIn
            {0, 100, 200,  300,  400,  500,  600,  700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000}  // i4LUTOut
        },

        // Preference gain for each light source
        {

            MFALSE,
            {
                //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 515, 512}
                }, // STROBE
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // TUNGSTEN
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // WARM F
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // F
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // CWF
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 522}, {502, 512, 522}, {502, 512, 522}
                }, // DAYLIGHT
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // SHADE
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                } // DAYLIGHT F
            }
        }
    };

    return (rAWBParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
AWB_STAT_PARAM_T const&
getAWBStatParam<ESensorDev_MainSecond>()
{
    // AWB Statistics Parameter
    static AWB_STAT_PARAM_T rAWBStatParam =
    {
        // Number of AWB windows
	    120, // Number of horizontal AWB windows
	    90, // Number of vertical AWB windows

        // Thresholds
    	  1, // Low threshold of R
    	  1, // Low threshold of G
    	  1, // Low threshold of B
    	254, // High threshold of R
    	254, // High threshold of G
    	254, // High threshold of B

        // Pre-gain maximum limit clipping
       	0xFFF, // Maximum limit clipping for R color
       	0xFFF, // Maximum limit clipping for G color
       	0xFFF, // Maximum limit clipping for B color

        // AWB error threshold
       	0, // Programmable threshold for the allowed total over-exposed and under-exposed pixels in one main stat window

        // AWB error count shift bits
        0 // Programmable error count shift bits: 0 ~ 7; note: AWB statistics provide 4-bits error count output only
    };

    return (rAWBStatParam);
}
