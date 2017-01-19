#include "camera_custom_capture_nr.h"
#include <mtkcam/Modes.h>
#include <mtkcam/hal/IHalSensor.h>
#include <camera_custom_nvram.h>
#include "isp_tuning/isp_tuning_custom_swnr.h"

using namespace NSCam;

bool get_capture_nr_th(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        MBOOL const isMfll,
        Capture_NR_Th_t* pTh
        )
{
    if( sensorDev == SENSOR_DEV_MAIN   ||
        sensorDev == SENSOR_DEV_SUB    ||
        sensorDev == SENSOR_DEV_MAIN_2
            )
    {
        if( !isMfll )
        {
            switch(shotmode)
            {
                case eShotMode_NormalShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_ContinuousShot:
                case eShotMode_ContinuousShotCc:
                    pTh->hwth = DISABLE_CAPTURE_NR;
                    pTh->swth = DISABLE_CAPTURE_NR;
                    break;
                case eShotMode_HdrShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_ZsdShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_FaceBeautyShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_VideoSnapShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                default:
                    pTh->hwth = DISABLE_CAPTURE_NR;
                    pTh->swth = DISABLE_CAPTURE_NR;
                    break;
                // note: special case
                //  eShotMode_SmileShot, eShotMode_AsdShot
                //      --> NormalShot or ZsdShot
            }
        }
        else
        {
            switch(shotmode)
            {
                case eShotMode_NormalShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_FaceBeautyShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                default:
                    pTh->hwth = DISABLE_CAPTURE_NR;
                    pTh->swth = DISABLE_CAPTURE_NR;
                    break;
                // note: special case
                //  eShotMode_SmileShot, eShotMode_AsdShot
                //      --> NormalShot or ZsdShot
            }
        }
    }
    else
    {
        pTh->hwth = DISABLE_CAPTURE_NR;
        pTh->swth = DISABLE_CAPTURE_NR;
    }

    return MTRUE;
}


EIdxSwNR
map_ISO_value_to_index(MUINT32 const u4Iso, MBOOL const isMfll, MUINT32 method)
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if( method == 0 ) //nearest
    {
        if ( u4Iso < 150 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_100 : eIDX_SWNR_SINGLE_ISO_100;
        }
        else if ( u4Iso < 300 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_200 : eIDX_SWNR_SINGLE_ISO_200;
        }
        else if ( u4Iso < 600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_400 : eIDX_SWNR_SINGLE_ISO_400;
        }
        else if ( u4Iso < 1000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_800 : eIDX_SWNR_SINGLE_ISO_800;
        }
        else if ( u4Iso < 1400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1200 : eIDX_SWNR_SINGLE_ISO_1200;
        }    
        else if ( u4Iso < 1800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1600 : eIDX_SWNR_SINGLE_ISO_1600;
        }
        else if ( u4Iso < 2200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2000 : eIDX_SWNR_SINGLE_ISO_2000;
        }    
        else if ( u4Iso < 2600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2400 : eIDX_SWNR_SINGLE_ISO_2400;
        }    
        else if ( u4Iso < 3000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2800 : eIDX_SWNR_SINGLE_ISO_2800;
        }
        return  isMfll ? eIDX_SWNR_MFLL_ISO_3200 : eIDX_SWNR_SINGLE_ISO_3200;
    }
    else if( method == 1 ) //lower bound
    {
        if ( u4Iso < 200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_100 : eIDX_SWNR_SINGLE_ISO_100;
        }
        else if ( u4Iso < 400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_200 : eIDX_SWNR_SINGLE_ISO_200;
        }
        else if ( u4Iso < 800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_400 : eIDX_SWNR_SINGLE_ISO_400;
        }
        else if ( u4Iso < 1200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_800 : eIDX_SWNR_SINGLE_ISO_800;
        }
        else if ( u4Iso < 1600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1200 : eIDX_SWNR_SINGLE_ISO_1200;
        }    
        else if ( u4Iso < 2000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1600 : eIDX_SWNR_SINGLE_ISO_1600;
        }
        else if ( u4Iso < 2400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2000 : eIDX_SWNR_SINGLE_ISO_2000;
        }    
        else if ( u4Iso < 2800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2400 : eIDX_SWNR_SINGLE_ISO_2400;
        }    
        else if ( u4Iso < 3200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2800 : eIDX_SWNR_SINGLE_ISO_2800;
        }
        return isMfll ? eIDX_SWNR_MFLL_ISO_3200 : eIDX_SWNR_SINGLE_ISO_3200;
    }
    else if( method == 2 ) //upper bound
    {
        if ( u4Iso <= 100 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_100 : eIDX_SWNR_SINGLE_ISO_100;
        }
        else if ( u4Iso <= 200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_200 : eIDX_SWNR_SINGLE_ISO_200;
        }
        else if ( u4Iso <= 400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_400 : eIDX_SWNR_SINGLE_ISO_400;
        }
        else if ( u4Iso <= 800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_800 : eIDX_SWNR_SINGLE_ISO_800;
        }
        else if ( u4Iso <= 1200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1200 : eIDX_SWNR_SINGLE_ISO_1200;
        }    
        else if ( u4Iso <= 1600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1600 : eIDX_SWNR_SINGLE_ISO_1600;
        }
        else if ( u4Iso <= 2000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2000 : eIDX_SWNR_SINGLE_ISO_2000;
        }    
        else if ( u4Iso <= 2400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2400 : eIDX_SWNR_SINGLE_ISO_2400;
        }    
        else if ( u4Iso <= 2800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2800 : eIDX_SWNR_SINGLE_ISO_2800;
        }
        return isMfll ? eIDX_SWNR_MFLL_ISO_3200 : eIDX_SWNR_SINGLE_ISO_3200;
    }
    // error
    return eNUM_OF_SWNR_IDX;
}


MUINT32
map_index_to_ISO_value(EIdxSwNR const u4Idx)
{
    switch(u4Idx)
    {
        case eIDX_SWNR_SINGLE_ISO_100:
            return 100;
        case eIDX_SWNR_SINGLE_ISO_200:
            return 200;
        case eIDX_SWNR_SINGLE_ISO_400:
            return 400;
        case eIDX_SWNR_SINGLE_ISO_800:
            return 800;
        case eIDX_SWNR_SINGLE_ISO_1200:
            return 1200;
        case eIDX_SWNR_SINGLE_ISO_1600:
            return 1600;
        case eIDX_SWNR_SINGLE_ISO_2000:
            return 2000;
        case eIDX_SWNR_SINGLE_ISO_2400:
            return 2400;
        case eIDX_SWNR_SINGLE_ISO_2800:
            return 2800;
        case eIDX_SWNR_SINGLE_ISO_3200:
            return 3200;
        case eIDX_SWNR_SINGLE_ISO_ANR_ENC_OFF:
        case eIDX_SWNR_SINGLE_ISO_ANR_ENC_ON:
            //TODO: interpolation
            return -1;
        case eIDX_SWNR_MFLL_ISO_100:
            return 100;
        case eIDX_SWNR_MFLL_ISO_200:
            return 200;
        case eIDX_SWNR_MFLL_ISO_400:
            return 400;
        case eIDX_SWNR_MFLL_ISO_800:
            return 800;
        case eIDX_SWNR_MFLL_ISO_1200:
            return 1200;
        case eIDX_SWNR_MFLL_ISO_1600:
            return 1600;
        case eIDX_SWNR_MFLL_ISO_2000:
            return 2000;
        case eIDX_SWNR_MFLL_ISO_2400:
            return 2400;
        case eIDX_SWNR_MFLL_ISO_2800:
            return 2800;
        case eIDX_SWNR_MFLL_ISO_3200:
            return 3200;
        case eIDX_SWNR_MFLL_ISO_ANR_ENC_OFF:
        case eIDX_SWNR_MFLL_ISO_ANR_ENC_ON:
            //TODO: interpolation
            return -1;
        default:
            break;
    }
    //MY_LOGE("cannot map idx %d to iso", u4Idx);
    return -1;
}

MBOOL
is_to_invoke_swnr_interpolation(MBOOL const isMfll, MUINT32 const u4Iso)
{
    return MTRUE;
}
