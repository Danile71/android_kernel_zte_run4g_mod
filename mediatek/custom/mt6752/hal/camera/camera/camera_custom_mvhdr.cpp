#include "camera_custom_mvhdr.h"
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <cutils/properties.h>
#include <cutils/xlog.h> // For XLOG?().
#include <utils/Errors.h>

#define LOG_TAG "camera_custom_mvhdr"

#define CLAMP(x,min,max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
 // customize parameters
#define CUST_MVHDR_UPDATE_AWB_GAIN  1
#define CUST_MVHDR_INPUT_SE_SETTING  1
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

#define CUST_MVHDR_ISPGAIN_BASE 1024   // 1x = 1023
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/

MUINT16 g_strMergeAESts[16*16 + 16*16 + 16*16 + 16*16];  // Merge LE + Hist Merge LE + SE + Hist SE


MVOID decodemVHDRStatistic(const mVHDRInputParam_T& rInput, mVHDROutputParam_T& rOutput)
{
    MINT32 i, j;
    MUINT8 *pAEStat = reinterpret_cast<MUINT8 *>(rInput.pDataPointer);
    MUINT16 u2UperLSBData, u2UperMSBData, u2LowLSBData, u2LowMSBData;
    MUINT32 u4AEStaY, u4MergeStaY, i4LEHistY, i4SEHistY;

    MUINT32 u4DataMax, u4DataRange;
    #define MERGE_LE_BUFF_OFFSET            (0)
    #define MERGE_LE_HIST_BUFF_OFFSET       (16*16)
    #define SE_BUFF_OFFSET                  (16*16 + 16*16)
    #define SE_HIST_BUFF_OFFSET             (16*16 + 16*16 + 16*16)


//    if(rInput.u4SensorID == )   // TBD
    {
        memset(g_strMergeAESts, 0, sizeof(g_strMergeAESts));

        switch(rInput.u2ShutterRatio)
        {
            case 1:
                u4DataMax = (MUINT32)1 << 10;
                break;
            case 2:
                u4DataMax = (MUINT32)1 << 11;
                break;
            case 4:
                u4DataMax = (MUINT32)1 << 12;
                break;
            case 8:
                u4DataMax = (MUINT32)1 << 13;
                break;
            case 16:
                u4DataMax = (MUINT32)1 << 14;
                break;

            default:
                u4DataMax = (MUINT32)1 << 13;

        }

        u4DataRange = u4DataMax - 768;

        for(i=0; i <rInput.u2StatWidth; i++) {
            for(j=0; j<rInput.u2StatHight; j++) {
                u2UperMSBData = pAEStat[i*16*2 + j*2 + 8*i + j/2 ];
                u2LowMSBData = pAEStat[i*16*2 + j*2 + 1 + 8*i + j/2];

                u4AEStaY = (((u2UperMSBData << 6) & 0x3FC0) | ((u2LowMSBData >> 2) & 0x003F));

                if(u4AEStaY <= 768)
                {
                    u4MergeStaY = u4AEStaY;
                }
                else
                {
                    u4MergeStaY = (MUINT16)(768 + (((1023 - 768)*(u4AEStaY - 768) + (u4DataRange >> 1))/u4DataRange));
                }

                /*
                *   Always map to 8 bit
                */
                g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + j] = u4MergeStaY >> 2;
                g_strMergeAESts[SE_BUFF_OFFSET + i*16 + j] = u4AEStaY >> 5;

            }

#if 1
            XLOGD("[%s] Merge_RAW:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", __FUNCTION__,
            i,
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 0],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 1],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 2],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 3],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 4],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 5],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 6],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 7],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 8],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 9],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 10],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 11],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 12],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 13],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 14],
            g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + 15]);
#endif
#if 1
            XLOGD("[%s] SE_RAW:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", __FUNCTION__,
            i,
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 0],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 1],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 2],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 3],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 4],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 5],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 6],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 7],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 8],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 9],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 10],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 11],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 12],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 13],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 14],
            g_strMergeAESts[SE_BUFF_OFFSET + i*16 + 15]);
#endif

        }

        /*
        *   Stat to Histogram
        */
        for(int i = 0; i < 16; i++) {
            for(int j = 0; j < 16; j++) {
                /*
                *   Merge LE Histogram
                */
                i4LEHistY = g_strMergeAESts[MERGE_LE_BUFF_OFFSET + i*16 + j];
                i4LEHistY = CLAMP(i4LEHistY, 0, 255);
                g_strMergeAESts[MERGE_LE_HIST_BUFF_OFFSET + i4LEHistY]++ ;

                /*
                *   Merge SE Histogram
                */
                i4SEHistY = g_strMergeAESts[SE_BUFF_OFFSET + i*16 + j];
                i4SEHistY = CLAMP(i4SEHistY, 0, 255);
                g_strMergeAESts[SE_HIST_BUFF_OFFSET + i4SEHistY]++ ;
            }
        }
        rOutput.pDataPointer = g_strMergeAESts;
        rOutput.bUpdateSensorAWB = MTRUE;
    }

    #undef MERGE_LE_BUFF_OFFSET
    #undef MERGE_LE_HIST_BUFF_OFFSET
    #undef SE_BUFF_OFFSET
    #undef SE_HIST_BUFF_OFFSET

}

MVOID getMVHDR_AEInfo(const mVHDR_SWHDR_InputParam_T& rInput, mVHDR_SWHDR_OutputParam_T& rOutput)
{
    #define HDR_SE_TARGET                   (180)
    #define MAX(a,b)                        ((a) > (b) ? (a) : (b))
    #define LOG2(x)                         ((double)log(x)/log(2))
    MINT32 i4RatioEV;

    switch(rInput.i4Ratio) {
        case 1:
            i4RatioEV = 0;
            break;
        case 2:
            i4RatioEV = 100;
            break;
        case 4:
            i4RatioEV = 200;
            break;
        case 8:
            i4RatioEV = 300;
            break;
        case 16:
            i4RatioEV = 400;
            break;
        default:
            i4RatioEV = 300;

    }

    double fAOEGain = (double)HDR_SE_TARGET / MAX(1, rInput.SEMax);
    MINT32 i4SEEV = CLAMP((MINT32)(LOG2(fAOEGain) * 100 + 0.5f), 0, i4RatioEV);

    (rInput.LEMax < 170) ? (rOutput.i4SEDeltaEVx100 = 0)
                         : (rOutput.i4SEDeltaEVx100 = (-i4RatioEV + i4SEEV));

}

MVOID getmVHDRExpSetting(mVHDR_TRANSFER_Param_T &rInputParam, IVHDRExpSettingOutputParam_T& rOutput)
{
MUINT16 u2Ratio = 0;;

    if(rInputParam.u2SelectMode == 0) {   // 0 : Auto mode
        if(rInputParam.bSEInput == MTRUE) {
            rOutput.u4LEExpTimeInUS = rOutput.u4SEExpTimeInUS;       	
        } else {
            rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS;
        }       
    } else {   // manual mode
        if(rOutput.u4SEExpTimeInUS != 0) {
            u2Ratio = (rOutput.u4LEExpTimeInUS + (rOutput.u4SEExpTimeInUS >>2)) / rOutput.u4SEExpTimeInUS;
        } 
        if(u2Ratio < 1) {
            u2Ratio = 1;
        }
        
        if(rInputParam.bSEInput == MTRUE) {
            rOutput.u4LEExpTimeInUS = rOutput.u4SEExpTimeInUS;       	
            rOutput.u4SEExpTimeInUS = rOutput.u4SEExpTimeInUS / u2Ratio;
        } 
    }
}

MBOOL isSESetting()
{
    return CUST_MVHDR_INPUT_SE_SETTING;
}

