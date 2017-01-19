#include "camera_custom_hdr.h"
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <cutils/properties.h>
#include <cutils/xlog.h>	// For XLOG?().
#include <utils/Errors.h>



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define MAX_HDR_GAIN_ARRAY_ELEM		11	// Maximun HDR GainArray element number.

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static MUINT32 au4HdrGainArray[MAX_HDR_GAIN_ARRAY_ELEM] =
{
	CUST_HDR_GAIN_00,
	CUST_HDR_GAIN_01,
	CUST_HDR_GAIN_02,
	CUST_HDR_GAIN_03,
	CUST_HDR_GAIN_04,
	CUST_HDR_GAIN_05,
	CUST_HDR_GAIN_06,
	CUST_HDR_GAIN_07,
	CUST_HDR_GAIN_08,
	CUST_HDR_GAIN_09,
	CUST_HDR_GAIN_10,
};

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for Core Number.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrCoreNumberGet(void)
{
	return CUST_HDR_CORE_NUMBER;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get prolonged VD number.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrProlongedVdGet(void)
{
	return CUST_HDR_PROLONGED_VD;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for BRatio.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrBRatioGet(void)
{
	return CUST_HDR_BRATIO;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for Gain.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrGainArrayGet(MUINT32 u4ArrayIndex)
{
	if (u4ArrayIndex > MAX_HDR_GAIN_ARRAY_ELEM-1)
		u4ArrayIndex = MAX_HDR_GAIN_ARRAY_ELEM-1;

	return au4HdrGainArray[u4ArrayIndex];
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for BottomFRatio.
///////////////////////////////////////////////////////////////////////////
double CustomHdrBottomFRatioGet(void)
{
	return CUST_HDR_BOTTOM_FRATIO;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for TopFRatio.
///////////////////////////////////////////////////////////////////////////
double CustomHdrTopFRatioGet(void)
{
	return CUST_HDR_TOP_FRATIO;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for BottomFBound.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrBottomFBoundGet(void)
{
	return CUST_HDR_BOTTOM_FBOUND;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for TopFBound.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrTopFBoundGet(void)
{
	return CUST_HDR_TOP_FBOUND;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for ThHigh.
///////////////////////////////////////////////////////////////////////////
MINT32 CustomHdrThHighGet(void)
{
	return CUST_HDR_TH_HIGH;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for ThLow.
///////////////////////////////////////////////////////////////////////////
MINT32 CustomHdrThLowGet(void)
{
	return CUST_HDR_TH_LOW;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for TargetLevelSub.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrTargetLevelSubGet(void)
{
	return CUST_HDR_TARGET_LEVEL_SUB;
}

/*******************************************************************************
* HDR exposure setting
*******************************************************************************/
#define MAX_LOG_BUF_SIZE	5000
static unsigned char GS_ucLogBuf[MAX_LOG_BUF_SIZE];	// Buffer to put log message. Will be outputed to file.
static char* pucLogBufPosition = NULL;	// A pointer pointing to some position in the GS_ucLogBuf[].
static unsigned int S_u4RunningNumber = 0;	// Record execution counts.

static unsigned int DumpToFile(
    char *fname,
    unsigned char *pbuf,
    unsigned int size
)
{
    int nw, cnt = 0;
    unsigned int written = 0;


    XLOGD("[DumpToFile] S_u4RunningNumber: %d.\n", S_u4RunningNumber);
    XLOGD("[DumpToFile] opening file [%s]\n", fname);
	FILE* pFp = fopen(fname, "a+t");	// a+: Opens for reading and appending. t: Open in text (translated) mode.
    if (pFp < 0) {
        XLOGE("[DumpToFile] failed to create file [%s]: %s", fname, strerror(errno));
        return 0x80000000;
    }

    XLOGD("[DumpToFile] writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = fwrite(pbuf + written, sizeof( char ), size - written, pFp);
        if (nw < 0) {
            XLOGE("[DumpToFile] failed to write to file [%s]: %s", fname, strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    XLOGD("[DumpToFile] done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    fclose(pFp);

    return 0;

}

/*
HDRFlag = 0;  // original version, always capture 3 frames
HDRFlag = 1;  // adaptive version, if original version use -2EV less, we only capture 2 frames (0EV and +2EV). If original version use -2EV a lot, we still capture 3 frames.
HDRFlag = 2;  // performance priority version, always capture 2 frames. The EV settings are decided adapively.
HDR_NEOverExp_Percent = 15; // this is a customer tuning parameter. When HDRFlag==1, it means if there is less than HDR_NEOverExp_Percent/1000 pixels over saturation in 0EV, we capture 2 frames instead.
*/


MVOID getHDRExpSetting(const HDRExpSettingInputParam_T& rInput, HDRExpSettingOutputParam_T& rOutput)
{
    static MUINT32 HDRFlag = CUST_HDR_CAPTURE_ALGORITHM;
    MUINT32 HDR_NEOverExp_Percent   = CUST_HDR_NEOverExp_Percent;
    MUINT32 u4MaxHDRExpTimeInUS     = 200000; // Manually set, no longer than 0.5s (unit: us)
    MUINT32 u4MaxSafeHDRExpTimeInUS = 31250; // Manually set, no longer than 0.5s (unit: us)
    MUINT32 u4MaxHDRSensorGain      = 4848; //Manually set, no larger than max gain in normal capture
    MUINT32 u4TimeMode  = 1;                // 0:Depend on default AE parameters; 1: Manually set
    MUINT32 u4GainMode  = 1;                // 0:Depend on default AE parameters; 1: Manually set
    MBOOL bGain0EVLimit = MFALSE;           // True: Limit the gain of 0EV and short exposure image; False: Keep it
    MUINT32 HISsum = 0;
    double DarkAvg = 0;
    double dfEVDiffTh   = EVDIFFTH;// 3EV
    double dfGainDiffTh = pow(2, dfEVDiffTh);
    
    // Temporary parameters
    MUINT32 u4MaxExpTimeInUS;
    MUINT32 u4MaxSensorGain;
    MUINT32 i;
    double dfRemainGain[3];
    double dfGainDiff[2];

    double dfTargetTopEV     = 1.5; // Target EV of long exposure image
    double dfSafeTargetTopEV = 0.5; // Target EV of long exposure image
    double dfTargetBottomEV  = -2; // Target EV of short exposure image
    double dfTopGain     = pow(2, dfTargetTopEV);
    double dfSafeTopGain = pow(2, dfSafeTargetTopEV);
    double dfBottomGain  = pow(2, dfTargetBottomEV);

    XLOGD("u4AOEMode(%d), u4MaxSensorAnalogGain(%d), u4MaxAEExpTimeInUS(%d), u4MinAEExpTimeInUS(%d), u4ShutterLineTime(%d)"
            , rInput.u4AOEMode
            , rInput.u4MaxSensorAnalogGain
            , rInput.u4MaxAEExpTimeInUS
            , rInput.u4MinAEExpTimeInUS
            , rInput.u4ShutterLineTime
            );
    XLOGD("u4MaxAESensorGain(%d), u4MinAESensorGain(%d), u4ExpTimeInUS0EV(%d), u4SensorGain0EV(%d), u1FlareOffset0EV(%d)"
            , rInput.u4MaxAESensorGain
            , rInput.u4MinAESensorGain
            , rInput.u4ExpTimeInUS0EV
            , rInput.u4SensorGain0EV
            , rInput.u1FlareOffset0EV
            );
    XLOGD("i4GainBase0EV(%d), i4LE_LowAvg(%d), i4SEDeltaEVx100(%d)"
            , rInput.i4GainBase0EV
            , rInput.i4LE_LowAvg
            , rInput.i4SEDeltaEVx100
            );
	MINT32 	i4GainBase0EV;		// AOE application for LE calculation
	MINT32 	i4LE_LowAvg;		// AOE application for LE calculation, def: 0 ~ 39 avg
	MINT32 	i4SEDeltaEVx100;	// AOE application for SE calculation

    if(rInput.u4SensorGain0EV == 0
        || rInput.u4ExpTimeInUS0EV == 0
        || rInput.u4MaxAEExpTimeInUS == 0
        || rInput.u4MaxAESensorGain == 0
        || rInput.u4MaxSensorAnalogGain == 0
        )
    {
        XLOGD("u4SensorGain0EV(%d), u4ExpTimeInUS0EV(%d), u4MaxAEExpTimeInUS(%d), u4MaxAESensorGain(%d), u4MaxSensorAnalogGain(%d) shouldn't be 0"
                , rInput.u4SensorGain0EV
                , rInput.u4ExpTimeInUS0EV
                , rInput.u4MaxAEExpTimeInUS
                , rInput.u4MaxAESensorGain
                , rInput.u4MaxSensorAnalogGain
                );
        
        XLOGD("Invalid AE Setting for HDR Capture ! \n");
        exit(-1);
    }

    if(rInput.u4AOEMode==1) //Dynamic EV Bracket
    {      
    	if (u4TimeMode == 0) 
        {
    		u4MaxExpTimeInUS = rInput.u4MaxAEExpTimeInUS;
        }
    	else // Manually set
        {
    		u4MaxExpTimeInUS = u4MaxHDRExpTimeInUS; 
        }

    	if (u4GainMode == 0) 
        {
    		u4MaxSensorGain = rInput.u4MaxAESensorGain;
    	}
    	else // Manually set
        {
    		u4MaxSensorGain = u4MaxHDRSensorGain; 

    	    if (u4MaxSensorGain > rInput.u4MaxSensorAnalogGain) 
            {
    			u4MaxSensorGain = rInput.u4MaxSensorAnalogGain;
    		}
    	}

        //Initial AE Setting of LE Calculation without any limitation
        double LOG10mean[20]={0, 0.3979, 0.6532, 0.8129, 0.9294, 1.0212, 1.0969, 1.1614, 1.2175, 1.2672, 1.3118, 1.3522, 1.3892, 1.4232, 1.4548, 1.4843, 1.5119, 1.5378, 1.5623, 1.5855};
		HISsum = 0;
        DarkAvg  = 0;
		for(i=0 ; i<SE_HIST_BIN_NUM ; i++)
        {      
            HISsum += rInput.u4Histogram[i];
            DarkAvg  += LOG10mean[i]*rInput.u4Histogram[i]; 
        }
        if(HISsum!=0)
        {
            DarkAvg /= HISsum; //Average of Bin0~Bin39, should same as i4LEAvg0_40      
        }
        else
        {
            XLOGD("Invalid AE histogram for HDR Capture ! \n");
        }
        
        if(INTENSITY_TARGET>DarkAvg)
        {
            dfTargetTopEV = (INTENSITY_TARGET - DarkAvg)/0.3; //0.3 diff in log10 base = 1EV
            dfTargetTopEV = (dfTargetTopEV>MAX_LE_EV) ? MAX_LE_EV : dfTargetTopEV;
            dfTopGain     = pow(2, dfTargetTopEV);
        }
        else
        {
            XLOGD("AE Setting Needs Check for HDR Capture ! \n");
        }
    	if(rInput.u4SensorGain0EV > u4MaxHDRSensorGain)
        {   
        	rOutput.u4ExpTimeInUS[2] = static_cast<MUINT32>(rInput.u4ExpTimeInUS0EV * dfSafeTopGain + 0.5);
        }
    	else
        {   
    		rOutput.u4ExpTimeInUS[2] = static_cast<MUINT32>(rInput.u4ExpTimeInUS0EV * dfTopGain + 0.5);
        }
    	rOutput.u4SensorGain[2] = rInput.u4SensorGain0EV;

        //SE Calculation
        dfTargetBottomEV = (double)(rInput.i4SEDeltaEVx100 / 100.0) - SE_OFFSET; // i4SEDeltaEVx100 = diff to no over exposure SE
        dfBottomGain     = pow(2, dfTargetBottomEV);
        rOutput.u4ExpTimeInUS[0] = static_cast<MUINT32>(rInput.u4ExpTimeInUS0EV * dfBottomGain + 0.5);
    	rOutput.u4SensorGain[0]  = rInput.u4SensorGain0EV;

        //LE Setting Refine by Max Time/Gain Constraintss
    	dfRemainGain[2] = 1;
    	if(rInput.u4ExpTimeInUS0EV > u4MaxSafeHDRExpTimeInUS)
        {   
           u4MaxSafeHDRExpTimeInUS = rInput.u4ExpTimeInUS0EV;
        }

        if (rOutput.u4ExpTimeInUS[2] > u4MaxSafeHDRExpTimeInUS) 
        {
            dfRemainGain[2] = static_cast<double>(rOutput.u4ExpTimeInUS[2]) / u4MaxSafeHDRExpTimeInUS;
            rOutput.u4ExpTimeInUS[2] =  u4MaxSafeHDRExpTimeInUS;
            rOutput.u4SensorGain[2] = static_cast<MUINT32>(rOutput.u4SensorGain[2] * dfRemainGain[2] + 0.5);
            if (rOutput.u4SensorGain[2] > u4MaxSensorGain) 
            {
                dfRemainGain[2] = static_cast<double>(rOutput.u4SensorGain[2]) / u4MaxSensorGain;
                rOutput.u4SensorGain[2] = u4MaxSensorGain;
                rOutput.u4ExpTimeInUS[2] = static_cast<MUINT32>(rOutput.u4ExpTimeInUS[2] * dfRemainGain[2] + 0.5);

                if (rOutput.u4ExpTimeInUS[2] > u4MaxExpTimeInUS)
                {
                    dfRemainGain[2] = static_cast<double>(rOutput.u4ExpTimeInUS[2]) / u4MaxExpTimeInUS;
                    rOutput.u4ExpTimeInUS[2]  = u4MaxExpTimeInUS;
                }
            }
        }
        
        //NE Calculation
        rOutput.u4ExpTimeInUS[1] = (MUINT32)(NE_AOE_GAIN*(sqrt((double)rOutput.u4ExpTimeInUS[2]*rOutput.u4ExpTimeInUS[0])+0.5));
        rOutput.u4SensorGain[1]  = (MUINT32)(NE_AOE_GAIN*(sqrt((double)rOutput.u4SensorGain[2]*rOutput.u4SensorGain[0])+0.5));

        //FlareOffset 0 and 2: calculated from input flareoffset and gain (SE or LE/input 0EV)
        dfGainDiff[0] = static_cast<double>(rOutput.u4SensorGain[0]*rOutput.u4ExpTimeInUS[0]) / (rInput.u4SensorGain0EV*rInput.u4ExpTimeInUS0EV);
        dfGainDiff[1] = static_cast<double>(rOutput.u4SensorGain[2]*rOutput.u4ExpTimeInUS[2]) / (rInput.u4SensorGain0EV*rInput.u4ExpTimeInUS0EV);

        rOutput.u1FlareOffset[1] = rInput.u1FlareOffset0EV;
        if(rOutput.u1FlareOffset[1]>1) // u1FlareOffset0EV is SE of iHDR, upper bound is then lower
        {
            rOutput.u1FlareOffset[1] = 1;
        } 
        rOutput.u1FlareOffset[0] = static_cast<MUINT8>(rOutput.u1FlareOffset[1] * dfGainDiff[0] + 0.5);
        rOutput.u1FlareOffset[2] = static_cast<MUINT8>(rOutput.u1FlareOffset[1] * dfGainDiff[1] + 0.5);

        //FlareOffset 1: calculated from FlareOffset 2 and gain (new NE/LE)
        if(rOutput.u4ExpTimeInUS[1]*rOutput.u4SensorGain[1]!=0)
        {
            dfGainDiff[0] = static_cast<double>(rOutput.u4SensorGain[0]*rOutput.u4ExpTimeInUS[0]) / (rOutput.u4ExpTimeInUS[1]*rOutput.u4SensorGain[1]);
            dfGainDiff[1] = static_cast<double>(rOutput.u4SensorGain[2]*rOutput.u4ExpTimeInUS[2]) / (rOutput.u4ExpTimeInUS[1]*rOutput.u4SensorGain[1]);
        }
        else
        {
            XLOGD("SWHDR Error calculation of GainDiff = %f %f\n",dfGainDiff[0], dfGainDiff[1]);
        }
        //recalculate flareoffset of NE
        rOutput.u1FlareOffset[1] = static_cast<MUINT8>(rOutput.u1FlareOffset[2]/dfGainDiff[1] + 0.5);     

        if(rOutput.u1FlareOffset[2]>63)
        {
            rOutput.u1FlareOffset[2] = 63; 
        }

        //Target tone and Gain for Img Normalization
        rOutput.u4TargetTone = 150;
        if(dfGainDiff[0]!=0 && dfGainDiff[1]!=0)
        {
            rOutput.u4FinalGainDiff[0] = static_cast<MUINT32>(1024 / dfGainDiff[0] + 0.5);
            rOutput.u4FinalGainDiff[1] = static_cast<MUINT32>(1024 / dfGainDiff[1] + 0.5);
        }
        else
        {
            XLOGD("getHDRExpSetting dfGainDiff Error ! %d %d\n", dfGainDiff[0], dfGainDiff[1]);
        }

        //Decide taking 2 or 3 by EV diff of SE and LE
        if(dfGainDiff[0]!=0)
        {
            XLOGD("SWHDR GainDiff = %f\n",dfGainDiff[1]/dfGainDiff[0]);

            if((dfGainDiff[1]/dfGainDiff[0]) < dfGainDiffTh)//Use SE, LE only
            {
                rOutput.u4OutputFrameNum   = 2;
                rOutput.u4ExpTimeInUS[1]   = rOutput.u4ExpTimeInUS[2];
                rOutput.u4SensorGain[1]    = rOutput.u4SensorGain[2];
                rOutput.u1FlareOffset[1]   = rOutput.u1FlareOffset[2];

                //Normalization gain (align to LE)
                if((rOutput.u4ExpTimeInUS[0]*rOutput.u4SensorGain[0])!=0)
                {
                    dfGainDiff[0] = static_cast<double>(rOutput.u4ExpTimeInUS[0]*rOutput.u4SensorGain[0]) / (rOutput.u4SensorGain[2]*rOutput.u4ExpTimeInUS[2]);
                }
                else
                {
                    XLOGD("SWHDR Error calculation of SE = %d %d\n",rOutput.u4ExpTimeInUS[0], rOutput.u4SensorGain[0]);
                }
                if(dfGainDiff[0]!=0)
                {
                    rOutput.u4FinalGainDiff[0] = static_cast<MUINT32>(1024 / dfGainDiff[0] + 0.5);
                }
                else
                {
                    XLOGD("SWHDR Error calculation of 2 frm dfGainDiff = %f\n",dfGainDiff[0]);
                }
            }
            else
            {
                rOutput.u4OutputFrameNum = 3;
            }
        }
        else
        {
            XLOGD("getHDRExpSetting dfGainDiff Error ! %d \n", dfGainDiff[0]);
        }

    }
    else //Static EV Bracket
    {  
        if( HDRFlag == 2)
        {   
            HISsum = 0;
            for(i=0 ; i<128 ; i++)
                HISsum = HISsum + rInput.u4Histogram[i];
           
            if( (int)((rInput.u4Histogram[126] + rInput.u4Histogram[127]) * 1000 / HISsum + 0.5) < HDR_NEOverExp_Percent) //calculate the percentage of bin[126]+bin[127]
            {
                dfTargetBottomEV         = -1;
                rOutput.u4OutputFrameNum = 2;
            }
            else
            {
                dfTargetBottomEV          = -2;
                rOutput.u4OutputFrameNum  = 3;
            }
            dfBottomGain  = pow(2, dfTargetBottomEV);
        }
        if (u4TimeMode == 0) 
        {
            u4MaxExpTimeInUS = rInput.u4MaxAEExpTimeInUS;
        }
        else // Manually set
        {
            u4MaxExpTimeInUS = u4MaxHDRExpTimeInUS; 
        }
    
        if (u4GainMode == 0) 
        {
            u4MaxSensorGain = rInput.u4MaxAESensorGain;
        }
        else // Manually set
        {
            u4MaxSensorGain = u4MaxHDRSensorGain; 
    
            if (u4MaxSensorGain > rInput.u4MaxSensorAnalogGain) 
            {
                u4MaxSensorGain = rInput.u4MaxSensorAnalogGain;
            }
        }
        //LE Init
        if(rInput.u4SensorGain0EV > u4MaxHDRSensorGain)
        {
            rOutput.u4ExpTimeInUS[2] = static_cast<MUINT32>(rInput.u4ExpTimeInUS0EV * dfSafeTopGain + 0.5);
        }
        else
        { 
            rOutput.u4ExpTimeInUS[2] = static_cast<MUINT32>(rInput.u4ExpTimeInUS0EV * dfTopGain + 0.5);
        }
        rOutput.u4SensorGain[2] = rInput.u4SensorGain0EV;
        // NE
        rOutput.u4ExpTimeInUS[1] = rInput.u4ExpTimeInUS0EV; 
        rOutput.u4SensorGain[1] = rInput.u4SensorGain0EV;
        //SE Calculation
        rOutput.u4ExpTimeInUS[0] = static_cast<MUINT32>(rInput.u4ExpTimeInUS0EV * dfBottomGain + 0.5);
        rOutput.u4SensorGain[0] = rInput.u4SensorGain0EV;
        
        //LE Setting Refine by Max Time/Gain Constraintss
        dfRemainGain[2] = 1;
        if(rInput.u4ExpTimeInUS0EV > u4MaxSafeHDRExpTimeInUS)
        {   
           u4MaxSafeHDRExpTimeInUS = rInput.u4ExpTimeInUS0EV;
        }
        
        if (rOutput.u4ExpTimeInUS[2] > u4MaxSafeHDRExpTimeInUS) 
        {
            dfRemainGain[2] = static_cast<double>(rOutput.u4ExpTimeInUS[2]) / u4MaxSafeHDRExpTimeInUS;
            rOutput.u4ExpTimeInUS[2] =  u4MaxSafeHDRExpTimeInUS;
            rOutput.u4SensorGain[2] = static_cast<MUINT32>(rOutput.u4SensorGain[2] * dfRemainGain[2] + 0.5);
            if (rOutput.u4SensorGain[2] > u4MaxSensorGain) 
            {
                dfRemainGain[2] = static_cast<double>(rOutput.u4SensorGain[2]) / u4MaxSensorGain;
                rOutput.u4SensorGain[2] = u4MaxSensorGain;
                rOutput.u4ExpTimeInUS[2] = static_cast<MUINT32>(rOutput.u4ExpTimeInUS[2] * dfRemainGain[2] + 0.5);
    
                if (rOutput.u4ExpTimeInUS[2] > u4MaxExpTimeInUS)
                {
                    dfRemainGain[2] = static_cast<double>(rOutput.u4ExpTimeInUS[2]) / u4MaxExpTimeInUS;
                    rOutput.u4ExpTimeInUS[2]  = u4MaxExpTimeInUS;
                }
            }
        }
    
        //FlareOffset 1: calculated from FlareOffset 2 and gain (new NE/LE)
        if(rOutput.u4ExpTimeInUS[1]*rOutput.u4SensorGain[1]!=0)
        {
            dfGainDiff[0] = static_cast<double>(rOutput.u4SensorGain[0]*rOutput.u4ExpTimeInUS[0]) / (rOutput.u4ExpTimeInUS[1]*rOutput.u4SensorGain[1]);
            dfGainDiff[1] = static_cast<double>(rOutput.u4SensorGain[2]*rOutput.u4ExpTimeInUS[2]) / (rOutput.u4ExpTimeInUS[1]*rOutput.u4SensorGain[1]);
        }
        else
        {
            XLOGD("SWHDR Error calculation of GainDiff = %f %f\n",dfGainDiff[0], dfGainDiff[1]);
        }
        rOutput.u1FlareOffset[1] = rInput.u1FlareOffset0EV;
        if(rOutput.u1FlareOffset[1]>4)
        {
            rOutput.u1FlareOffset[1] = 4;
        }   
    
        rOutput.u1FlareOffset[0] = static_cast<MUINT8>(rOutput.u1FlareOffset[1] * dfGainDiff[0] + 0.5);
        rOutput.u1FlareOffset[2] = static_cast<MUINT8>(rOutput.u1FlareOffset[1] * dfGainDiff[1] + 0.5);
    
        if(rOutput.u1FlareOffset[2]>63)
        {
            rOutput.u1FlareOffset[2] = 63; 
        }
    
        //Target tone and Gain for Img Normalization
        rOutput.u4TargetTone = 150;
        if(dfGainDiff[0]!=0 && dfGainDiff[1]!=0)
        {
            rOutput.u4FinalGainDiff[0] = static_cast<MUINT32>(1024 / dfGainDiff[0] + 0.5);
            rOutput.u4FinalGainDiff[1] = static_cast<MUINT32>(1024 / dfGainDiff[1] + 0.5);
        }
        else
        {
            XLOGD("getHDRExpSetting dfGainDiff Error ! %d %d\n", dfGainDiff[0], dfGainDiff[1]);
        }

        if( HDRFlag == 1)
        {
            HISsum = 0;
            for(i=0 ; i<128 ; i++)
                HISsum = HISsum + rInput.u4Histogram[i];
        
            if( (int)((rInput.u4Histogram[126] + rInput.u4Histogram[127]) * 1000 / HISsum + 0.5) < HDR_NEOverExp_Percent)
            {   
                //NE + LE only
                rOutput.u4OutputFrameNum   = 2;
                rOutput.u4ExpTimeInUS[0]   = rOutput.u4ExpTimeInUS[2];
                rOutput.u4SensorGain[0]    = rOutput.u4SensorGain[2];
                rOutput.u1FlareOffset[0]   = rOutput.u1FlareOffset[2];
                rOutput.u4FinalGainDiff[0] = rOutput.u4FinalGainDiff[1];
            }
            else
            {
                rOutput.u4OutputFrameNum = 3;
            }        
        }
        
        if( HDRFlag == 2)
        {     
            if(rOutput.u4OutputFrameNum == 2)//Use LE(2EV) + SE(-1EV) Only
            {
                rOutput.u4ExpTimeInUS[1]   = rOutput.u4ExpTimeInUS[2];
                rOutput.u4SensorGain[1]    = rOutput.u4SensorGain[2];
                rOutput.u1FlareOffset[1]   = rOutput.u1FlareOffset[2];

                //Normalization gain (align to LE)
                if((rOutput.u4ExpTimeInUS[0]*rOutput.u4SensorGain[0])!=0)
                {
                    dfGainDiff[0] = static_cast<double>(rOutput.u4ExpTimeInUS[0]*rOutput.u4SensorGain[0]) / (rOutput.u4SensorGain[2]*rOutput.u4ExpTimeInUS[2]);
                }
                else
                {
                    XLOGD("SWHDR Error calculation of SE = %d %d\n",rOutput.u4ExpTimeInUS[0], rOutput.u4SensorGain[0]);
                }
                if(dfGainDiff[0]!=0)
                {
                    rOutput.u4FinalGainDiff[0] = static_cast<MUINT32>(1024 / dfGainDiff[0] + 0.5);
                }
                else
                {
                    XLOGD("SWHDR Error calculation of 2 frm dfGainDiff = %f\n",dfGainDiff[0]);
                }
            }
        }
    }

#if 1
    char value[256] = {'\0'};
    property_get("mediatek.hdr.debug", value, "0");
    int hdr_debug_mode = atoi(value) || CUST_HDR_DEBUG;
	if(hdr_debug_mode) 
    {
		// Increase 4-digit running number (range: 1 ~ 9999).
		if (S_u4RunningNumber >= 9999)
			S_u4RunningNumber = 1;
		else
			S_u4RunningNumber++;

		pucLogBufPosition = (char*)GS_ucLogBuf;
	    ::sprintf(pucLogBufPosition, "< No.%04d > ----------------------------------------------------------------------\n", S_u4RunningNumber);
	    pucLogBufPosition += strlen(pucLogBufPosition);
	    ::sprintf(pucLogBufPosition, "[System Paramters]\n");
	    pucLogBufPosition += strlen(pucLogBufPosition);
	    ::sprintf(pucLogBufPosition, "0EV Exposure Time = %d 0EV Sensor Gain = %d 0EV Flare Offset %d\n", rInput.u4ExpTimeInUS0EV, rInput.u4SensorGain0EV, rInput.u1FlareOffset0EV);
	    pucLogBufPosition += strlen(pucLogBufPosition);
	    ::sprintf(pucLogBufPosition, "Max Exposure Time Sensor= %d\nMaxSensor Gain Sensor= %d\n", rInput.u4MaxAEExpTimeInUS, rInput.u4MaxAESensorGain);
	    pucLogBufPosition += strlen(pucLogBufPosition);
	    ::sprintf(pucLogBufPosition, "Max Exposure Time Manual= %d\nMaxSensor Gain Manual= %d\n", u4MaxHDRExpTimeInUS, u4MaxHDRSensorGain);
	    pucLogBufPosition += strlen(pucLogBufPosition);
	    ::sprintf(pucLogBufPosition, "Max Exposure Time = %d\nMaxSensor Gain = %d\n", u4MaxExpTimeInUS, u4MaxSensorGain);
	    pucLogBufPosition += strlen(pucLogBufPosition);

	    ::sprintf(pucLogBufPosition, "\n[Tuning Paramters]\n");
	    pucLogBufPosition += strlen(pucLogBufPosition);
        ::sprintf(pucLogBufPosition, "SWHDR AE Mode = %d\n",rInput.u4AOEMode);
	    pucLogBufPosition += strlen(pucLogBufPosition);
        ::sprintf(pucLogBufPosition, "SWHDR INTENSITY_TARGET = %f, NE_AOE_GAIN = %f, SE_OFFSET = %f, MAX_LE_EV = %f\n",INTENSITY_TARGET, NE_AOE_GAIN, SE_OFFSET, MAX_LE_EV);
	    pucLogBufPosition += strlen(pucLogBufPosition);

        ::sprintf(pucLogBufPosition, "SWHDR Input delta EV = %f, Avg0_40 = %d Avg0_40_2 = %f\n", dfTargetBottomEV, rInput.i4LE_LowAvg, DarkAvg);
	    pucLogBufPosition += strlen(pucLogBufPosition);
        
        ::sprintf(pucLogBufPosition, "SWHDR AE HDRFlag Mode = %d\n",HDRFlag);
	    pucLogBufPosition += strlen(pucLogBufPosition);

		::sprintf(pucLogBufPosition, "Target Top EV = %f\nTarget Bottom EV = %f\n", dfTargetTopEV, dfTargetBottomEV);
	    pucLogBufPosition += strlen(pucLogBufPosition);

		::sprintf(pucLogBufPosition, "bGain0EVLimit = %s\n", (bGain0EVLimit ? "true" : "false"));
	    pucLogBufPosition += strlen(pucLogBufPosition);

		::sprintf(pucLogBufPosition, "\n[Output Paramters]\n");
	    pucLogBufPosition += strlen(pucLogBufPosition);
		for (i = 0; i < 3; i++) {
			::sprintf(pucLogBufPosition, "Final Frame %d ExposureTime = %d SensorGain = %d Flare Offset = %d \n", i, rOutput.u4ExpTimeInUS[i], rOutput.u4SensorGain[i], rOutput.u1FlareOffset[i]);
		    pucLogBufPosition += strlen(pucLogBufPosition);
	    }
	    ::sprintf(pucLogBufPosition, "Final EVdiff[0] = %d\nFinal EVdiff[1] = %d\n", rOutput.u4FinalGainDiff[0], rOutput.u4FinalGainDiff[1]);
	    pucLogBufPosition += strlen(pucLogBufPosition);
		::sprintf(pucLogBufPosition, "OutputFrameNum = %d\n", rOutput.u4OutputFrameNum);
	    pucLogBufPosition += strlen(pucLogBufPosition);
		::sprintf(pucLogBufPosition, "Final FlareOffsetOut[0]= %d\nFinal FlareOffsetOut[1]= %d\nFinal FlareOffsetOut[2]= %d\n", rOutput.u1FlareOffset[0], rOutput.u1FlareOffset[1], rOutput.u1FlareOffset[2]);
	    pucLogBufPosition += strlen(pucLogBufPosition);
		::sprintf(pucLogBufPosition, "Final TargetTone= %d\n", rOutput.u4TargetTone);
	    pucLogBufPosition += strlen(pucLogBufPosition);

		char szFileName[100];
		//::sprintf(szFileName, "sdcard/Photo/%04d_HDR_ExposureSetting.txt", S_u4RunningNumber);	// For ALPS.GB2.
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER"%04d_HDR_ExposureSetting.txt", S_u4RunningNumber);	// For ALPS.ICS.
		DumpToFile(szFileName, (unsigned char *)GS_ucLogBuf, MAX_LOG_BUF_SIZE);
	}
        XLOGD("SWHDR AE Mode = %d\n",rInput.u4AOEMode);
        XLOGD("SWHDR FrmNum = %d\n",rOutput.u4OutputFrameNum);
        XLOGD("SWHDR Input delta EV(Bottom) = %f, TopEV = %f, Avg0_40 = %d Avg0_40_2 = %f\n",dfTargetBottomEV, dfTargetTopEV, rInput.i4LE_LowAvg, DarkAvg);
        
        XLOGD("SWHDR AE HDRFlag Mode = %d\n",HDRFlag);
        XLOGD("SWHDR Input 0EV Time = %d, Gain = %d Flare = %d \n",rInput.u4ExpTimeInUS0EV, rInput.u4SensorGain0EV,rInput.u1FlareOffset0EV);
        XLOGD("SWHDR Output 0 Time = %d, Gain = %d \n",rOutput.u4ExpTimeInUS[0], rOutput.u4SensorGain[0]);
        XLOGD("SWHDR Output 1 Time = %d, Gain = %d \n",rOutput.u4ExpTimeInUS[1], rOutput.u4SensorGain[1]);
        XLOGD("SWHDR Output 2 Time = %d, Gain = %d \n",rOutput.u4ExpTimeInUS[2], rOutput.u4SensorGain[2]);
        XLOGD("SWHDR Flare Ofset = %d %d %d \n",rOutput.u1FlareOffset[0], rOutput.u1FlareOffset[1], rOutput.u1FlareOffset[2]);    
        XLOGD("SWHDR Gain Diff = %d %d\n",rOutput.u4FinalGainDiff[0], rOutput.u4FinalGainDiff[1]);     
#endif  
    //HDRFlag = (HDRFlag==2)?1:2;
}

/*******************************************************************************
*
*******************************************************************************/
