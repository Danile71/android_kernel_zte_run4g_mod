/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file vhdr_drv.cpp
*
* VHDR Driver Source File
*
*/
 
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <linux/cache.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <queue>

using namespace std;
using namespace android;
 
#include "mtkcam/imageio/ispio_stddef.h"   // for  register struct
 
using namespace NSImageio;
using namespace NSIspio;     
 
#include "mtkcam/drv/isp_reg.h" // for register name
#include "mtkcam/drv/isp_drv.h" // for isp driver object

#include "mtkcam/v1/config/PriorityDefs.h" 
#include "mtkcam/iopipe/CamIO/INormalPipe.h"    // for pass1 register setting API
#include "mtkcam/hal/IHalSensor.h"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe; 

#include "mtkcam/drv/imem_drv.h"
//#include "camera_custom_vhdr.h"    // TODO-not suer and file is not exist

#include "isp_mgr/isp_mgr.h"

using namespace NSIspTuning;

#include <mtkcam/utils/common.h>
#include <mtkcam/hwutils/HwMisc.h>
using namespace NSCam::Utils;

#include "vhdr_drv_imp.h"

/*******************************************************************************
*
********************************************************************************/
#define VHDR_DRV_DEBUG

#ifdef VHDR_DRV_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef LOG_TAG
#define LOG_TAG "VHdrDrv"
#define VHDR_LOG(fmt, arg...)    XLOGD("[%s]" fmt, __func__, ##arg)
#define VHDR_INF(fmt, arg...)    XLOGI("[%s]" fmt, __func__, ##arg)
#define VHDR_WRN(fmt, arg...)    XLOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define VHDR_ERR(fmt, arg...)    XLOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
                                  
#else
#define VHDR_LOG(a,...)
#define VHDR_INF(a,...)
#define VHDR_WRN(a,...)
#define VHDR_ERR(a,...)
#endif

#define VHDR_DRV_NAME "VHdrDrv"
#define LCSO_BUFFER_NUM 10
#define ROUND_TO_2X(x) ((x) & (~0x1))
#define ALIGN_SIZE(in,align) (in & ~(align-1))

/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump = 0;

/*******************************************************************************
*
********************************************************************************/
VHdrDrv *VHdrDrv::CreateInstance(const MUINT32 &aSensorIdx)
{
    return VHdrDrvImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
VHdrDrv *VHdrDrvImp::GetInstance(const MUINT32 &aSensorIdx)
{
    VHDR_LOG("aSensorIdx(%u)",aSensorIdx);    
    return new VHdrDrvImp(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::DestroyInstance() 
{
    VHDR_LOG("+");
	delete this;
}

/*******************************************************************************
*
********************************************************************************/
VHdrDrvImp::VHdrDrvImp(const MUINT32 &aSensorIdx) : VHdrDrv()
{
    mUsers = 0;
    
    m_pNormalPipe = NULL;
    m_pISPDrvObj = NULL;
    m_pISPVirtDrv = NULL;  // for command queue
    m_pIMemDrv = NULL;
   
    mSensorIdx = aSensorIdx;

    mSensorDev = 0;
    mSensorTg  = CAM_TG_NONE;

    mState = VHDR_STATE_NONE;
       
    mpVHdrP1Cb = NULL;    
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::Init()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);        
        return VHDR_RETURN_NO_ERROR;
    }

    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Dynamic Debug ======

#if (VHDR_DEBUG_FLAG)

    g_debugDump = 3;

#else

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.vhdr.dump", value, "0");
    g_debugDump = atoi(value);

#endif

    //====== Set State ======

    SetVHdrState(VHDR_STATE_ALIVE);
    
    VHDR_LOG("mSensorIdx(%u) init",mSensorIdx);

    //====== Create ISP Driver Object  ======

    m_pISPDrvObj = IspDrv::createInstance();
    if(m_pISPDrvObj == NULL)
    {
        VHDR_ERR("m_pISPDrvObj create instance fail");
        err = VHDR_RETURN_NULL_OBJ;
        return err;
    }

    if(MFALSE == m_pISPDrvObj->init(VHDR_DRV_NAME))
    {
        VHDR_ERR("m_pISPDrvObj->init() fail");
        err = VHDR_RETURN_API_FAIL;
        return err;
    }

    #if 0   // opend it when needed
    // Command Queue
    m_pISPVirtDrv = m_pISPDrvObj->getCQInstance(ISP_DRV_CQ0);
    if(m_pISPVirtDrv == NULL)
    {
        VHDR_ERR("m_pISPVirtDrv create instance fail");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }
    #endif

    //====== Create INormalPipe Object ======

    m_pNormalPipe = INormalPipe::createInstance(mSensorIdx, VHDR_DRV_NAME);
    if(m_pNormalPipe == NULL)
    {
        VHDR_ERR("create INormalPipe fail");
        err = VHDR_RETURN_NULL_OBJ;
        return err;
    }  

    //====== Create IMem Object ======
    
    m_pIMemDrv = IMemDrv::createInstance();
    if(m_pIMemDrv == NULL)
    {
        VHDR_ERR("Null IMemDrv Obj");
        err = VHDR_RETURN_NULL_OBJ;
        return err;
    }

    if(MFALSE == m_pIMemDrv->init())
    {
        VHDR_ERR("m_pIMemDrv->init fail");
        err = VHDR_RETURN_API_FAIL;
        return err;
    }

    //====== Create Pass1 Callback Class ======        
    
    //mpVHdrP1Cb = new VHdrP1Cb(this);    

    android_atomic_inc(&mUsers);    // increase reference count
    VHDR_LOG("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers <= 0) // No more users
    {
        VHDR_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return VHDR_RETURN_NO_ERROR;
    }

    // >= one user
    android_atomic_dec(&mUsers);   

    MINT32 err = VHDR_RETURN_NO_ERROR;

    if(mUsers == 0)
    {
        //====== Set State ======

        SetVHdrState(VHDR_STATE_UNINIT);
        
        VHDR_LOG("mSensorIdx(%u) uninit",mSensorIdx);        
    
        //====== Destory ISP Driver Object ======

        if(m_pISPVirtDrv != NULL)
        {
            m_pISPVirtDrv = NULL;
        }

        if(m_pISPDrvObj != NULL)
        {
            if(MFALSE == m_pISPDrvObj->uninit(VHDR_DRV_NAME))
            {
                VHDR_ERR("m_pISPDrvObj->uninit fail");
                err = VHDR_RETURN_API_FAIL; 
            }

            m_pISPDrvObj->destroyInstance();
            m_pISPDrvObj = NULL;
        }

        //====== Destory INormalPipe ======

        if(m_pNormalPipe != NULL)
        {                       
            m_pNormalPipe->destroyInstance(VHDR_DRV_NAME);
            m_pNormalPipe = NULL;
        }

        //====== Destory IMem ======        
        
        if(m_pIMemDrv != NULL)
        {
            if(MFALSE == m_pIMemDrv->uninit())
            {
                VHDR_ERR("m_pIMemDrv->uninit fail");
                err = VHDR_RETURN_API_FAIL;                
            }
            
            m_pIMemDrv->destroyInstance();
            m_pIMemDrv = NULL;
        }

        //====== Rest Member Variable ======

        mUsers = 0;        
        mSensorIdx = 0;

        mSensorDev = 0;
        mSensorTg  = CAM_TG_NONE;
        
        // delete pass1 callback class

        if(mpVHdrP1Cb != NULL)
        {
            delete mpVHdrP1Cb;
            mpVHdrP1Cb = NULL;
        }
        
        //====== Set State ======

        SetVHdrState(VHDR_STATE_NONE);
    }
    else
    {
         VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    VHDR_LOG("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetSensorInfo(const MUINT32 &aSensorDev,const MUINT32 &aSensorTg)
{
    mSensorDev = aSensorDev;
    mSensorTg  = aSensorTg;
    VHDR_LOG("(dev,tg)=(%u,%u)",mSensorDev,mSensorTg);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MINT32 err = VHDR_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    VHDR_LOG("Cnt(%u),Size(%u),alingSize(%u)",bufCnt, memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                VHDR_ERR("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
                err = VHDR_RETURN_API_FAIL;
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                VHDR_ERR("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
                err = VHDR_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            VHDR_ERR("m_pIMemDrv->allocVirtBuf() error");
            err = VHDR_RETURN_API_FAIL;
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            VHDR_ERR("m_pIMemDrv->mapPhyAddr() error");
            err = VHDR_RETURN_API_FAIL;
        }
    }
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 VHdrDrvImp::DestroyMemBuf(const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    VHDR_LOG("Cnt(%u)", bufCnt);

    MINT32 err = VHDR_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                VHDR_LOG("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                VHDR_ERR("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = VHDR_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                VHDR_ERR("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = VHDR_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            VHDR_LOG("Buffer doesn't exist");
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            VHDR_ERR("m_pIMemDrv->unmapPhyAddr() error");
            err = VHDR_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            VHDR_ERR("m_pIMemDrv->freeVirtBuf() error");
            err = VHDR_RETURN_API_FAIL;
        }
    }

    return err;
}

/******************************************************************************
*
*******************************************************************************/
ESensorDev_T VHdrDrvImp::ConvertSensorDevType(const MUINT32 &aSensorDev)
{
    switch(aSensorDev)
    {
        case SENSOR_DEV_MAIN: return ESensorDev_Main;
        case SENSOR_DEV_SUB: return ESensorDev_Sub;
        case SENSOR_DEV_MAIN_2: return ESensorDev_MainSecond;
        case SENSOR_DEV_MAIN_3D: return ESensorDev_Main3D;
        default : VHDR_ERR("wrong sensorDev(%d), return ESensorDev_Main",aSensorDev);
                  return ESensorDev_Main;
    }
}

/******************************************************************************
*
*******************************************************************************/
ESensorTG_T VHdrDrvImp::ConvertSensorTgType(const MUINT32 &aSensorTg)
{
    switch(aSensorTg)
    {
        case CAM_TG_1: return ESensorTG_1;
        case CAM_TG_2: return ESensorTG_2;        
        default : VHDR_ERR("wrong sensorTg(%d), return ESensorTG_1",aSensorTg);
                  return ESensorTG_1;
    }
}

/******************************************************************************
*
*******************************************************************************/
MBOOL VHdrDrvImp::SaveToFile(char const *filepath,const MUINT32 &addr)
{
    MBOOL ret = MFALSE;
    MINT32 fd = -1;
    MUINT8 *pBuf = (MUINT8 *)addr;
    MUINT32 size = 0;   // need modified
    MUINT32 written = 0;
    MINT32 nw = 0, cnt = 0;    
    
    VHDR_LOG("save to %s",filepath);

    //====== open file descriptor =======
    
    fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if( fd < 0 )
    {
        VHDR_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto saveToFileExit;
    }  

    //====== Save File ======
    
    while(written < size )
    {
        nw = ::write(fd, pBuf+written, size-written);
        if(nw < 0)
        {
            VHDR_ERR("fail to write %s, write-count:%d, written-bytes:%d : %s",filepath,cnt,written,::strerror(errno));
            goto saveToFileExit;
        }
        written += nw;
        cnt ++;
    }
    VHDR_LOG("[%d-th plane] write %d bytes to %s",size,filepath);    
    
    ret = MTRUE;
saveToFileExit:
    
    if  ( fd >= 0 )
    {
        ::close(fd);
    }
    
    return  ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetVHdrState(const VHDR_STATE_ENUM &aState)
{
    mState = aState;
    VHDR_LOG("aState(%d),mState(%d)",aState,mState);
}


/*******************************************************************************
*
********************************************************************************/
VHDR_STATE_ENUM VHdrDrvImp::GetVHdrState()
{
    return mState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::ChangeThreadSetting(char const *userName)
{
    //> set name 
    
    ::prctl(PR_SET_NAME,userName, 0, 0, 0);

    //> set policy/priority
    {
        const MINT32 expect_policy   = SCHED_OTHER;
        const MINT32 expect_priority = NICE_CAMERA_3A_MAIN;
        MINT32 policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        
        VHDR_LOG("policy:(expect,result)=(%d,%d),priority:(expect,result)=(%d, %d)",expect_policy,policy,expect_priority,priority);
    }
}

/*******************************************************************************
*
********************************************************************************/
VHdrP1Cb::VHdrP1Cb(MVOID *arg)
{
    m_pClassObj = arg;
}

/*******************************************************************************
*
********************************************************************************/
VHdrP1Cb::~VHdrP1Cb()
{
    m_pClassObj = NULL;
}

/*******************************************************************************
*
********************************************************************************/
void VHdrP1Cb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{    
}

