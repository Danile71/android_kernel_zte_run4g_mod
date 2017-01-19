/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "CamShot/CamShotimp"
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)

#include <mtkcam/common.h>
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>
//
#include "./inc/CamShotImp.h"
//


/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {

////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
CamShotImp::
CamShotImp(
    EShotMode const eShotMode,
    char const*const szCamShotName
)
    : meShotMode(eShotMode)
    , mszCamShotName(szCamShotName)
    //
    , mi4ErrorCode(0)
    //
    , mpCbUser(NULL)
    , mi4NotifyMsgSet(0)
    , mNotifyCb(NULL)
    , mi4DataMsgSet(0)
    , mDataCb(NULL)
    //
{
}

/*******************************************************************************
*
********************************************************************************/
MVOID
CamShotImp::
setCallbacks(CamShotNotifyCallback_t notify_cb, CamShotDataCallback_t data_cb, MVOID* user)
{
    MY_LOGV("(notify_cb, data_cb, user)=(%p, %p, %p)", notify_cb, data_cb, user);
    mpCbUser = user;
    mNotifyCb = notify_cb;
    mDataCb = data_cb;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamShotImp::
onNotifyCallback(CamShotNotifyInfo const& msg) const
{
    MBOOL   ret = MTRUE;
    //
    if  ( mNotifyCb )
    {
        mNotifyCb(mpCbUser, msg);
        ret = MTRUE;
    }
    else
    {
        MY_LOGW("Notify Callback is NULL");
        ret = MFALSE;
    }
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamShotImp::
onDataCallback(CamShotDataInfo const& msg) const
{
    MBOOL   ret = MTRUE;
    //
    if  ( mDataCb )
    {
        mDataCb(mpCbUser, msg);
        ret = MTRUE;
    }
    else
    {
        MY_LOGW("Data Callback is NULL");
        ret = MFALSE;
    }
    return  ret;
}


/*******************************************************************************
* 
********************************************************************************/
#if 0
MVOID 
CamShotImp::
dumpPipeProperty(vector<PortProperty> const &vInPorts, vector<PortProperty> const &vOutPorts)
{
    //
    MY_LOGD("PipeProperty In Ports:"); 
    for (MUINT32 i = 0 ; i < vInPorts.size(); i++) 
    {
        MY_LOGD("(type, index, inout, format, rotate, flip): (%d, %d, %d, 0x%x, %d, %d)", 
                               vInPorts.at(i).type, vInPorts.at(i).index, vInPorts.at(i).inout, 
                               vInPorts.at(i).u4SupportFmt,vInPorts.at(i).fgIsSupportRotate,vInPorts.at(i).fgIsSupportFlip);                                                        
    }
    //
    MY_LOGD("PipeProperty Out Ports:"); 
    for (MUINT32 i = 0 ; i < vOutPorts.size(); i++) 
    {
        MY_LOGD("(type, index, inout, format, rotate, flip): (%d, %d, %d, 0x%x, %d, %d)", 
                               vOutPorts.at(i).type, vOutPorts.at(i).index, vOutPorts.at(i).inout, 
                               vOutPorts.at(i).u4SupportFmt,vOutPorts.at(i).fgIsSupportRotate,vOutPorts.at(i).fgIsSupportFlip);                                                        
    }
}
#endif


/*******************************************************************************
* 
********************************************************************************/
MVOID  
CamShotImp::
dumpSensorParam(SensorParam const & rParam)
{
    //MY_LOGD("+"); 
    MY_LOGD("(id, scenario, bitdepth) = (%d, %d, %d)",
            rParam.u4OpenID, rParam.u4Scenario, rParam.u4Bitdepth); 
    MY_LOGD("(fgBypassDelay, fgBypassScenaio, raw) = (%d, %d, %d)",
            rParam.fgBypassDelay, rParam.fgBypassScenaio, rParam.u4RawType); 
    //MY_LOGD("-"); 

}

/*******************************************************************************
* 
********************************************************************************/
MVOID  
CamShotImp::
dumpShotParam(ShotParam const & rParam)
{
    //MY_LOGD("+"); 
    MY_LOGD("Picture (fmt, width, height, transform) = (%x, %d, %d, %d)", 
                       rParam.ePictureFmt, rParam.u4PictureWidth, rParam.u4PictureHeight,
                       rParam.u4PictureTransform); 
    MY_LOGD("PostView (fmt, width, height, transform) = (%x, %d, %d, %d)", 
                       rParam.ePostViewFmt, rParam.u4PostViewWidth, rParam.u4PostViewHeight, 
                       rParam.u4PostViewTransform); 
    MY_LOGD("ZoomRatoio = %d", rParam.u4ZoomRatio);    

    //MY_LOGD("-"); 

}
 
/*******************************************************************************
* 
********************************************************************************/
MVOID  
CamShotImp::
dumpJpegParam(JpegParam const & rParam)
{
    //MY_LOGD("+"); 
    MY_LOGD("Jpeg (Quality, fgIsSOI) = (%d, %d)", rParam.u4Quality, rParam.fgIsSOI); 
    MY_LOGD("Jps (Width, Height) = (%d, %d)", rParam.u4JpsWidth, rParam.u4JpsHeight);
    MY_LOGD("Thumbnail (Width, Height, Quality, fgIsSOI) = (%d, %d, %d, %d)", rParam.u4ThumbWidth, rParam.u4ThumbHeight,
                                                                     rParam.u4ThumbQuality, rParam.fgThumbIsSOI); 
 
    //MY_LOGD("-"); 
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL      
CamShotImp::
handleNotifyCallback(MINT32 const i4Msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MBOOL ret = MTRUE; 
    if (isNotifyMsgEnabled(i4Msg)) 
    {
        CamShotNotifyInfo rCbNotifyInfo(i4Msg, ext1, ext2); 
        ret = onNotifyCallback(rCbNotifyInfo);         
    }
    return ret; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL      
CamShotImp::
handleDataCallback(MINT32 const i4Msg, MUINT32 const ext1, MUINT32 const ext2, IImageBuffer* pImgBuf)
{
    MBOOL ret = MTRUE; 
    if (isDataMsgEnabled(i4Msg)) 
    {
        CamShotDataInfo rCbDataInfo(i4Msg, 
                                    ext1, 
                                    ext2, 
                                    pImgBuf 
                                   ); 
        ret = onDataCallback(rCbDataInfo); 
    }
    return ret; 
}



////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot


