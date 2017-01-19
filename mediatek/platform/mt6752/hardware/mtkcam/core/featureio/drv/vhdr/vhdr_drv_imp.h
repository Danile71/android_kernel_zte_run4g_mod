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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file vhdr_drv_imp.h
*
* VHDR Driver Implementation Header File
*
*/


#ifndef _VHDR_DRV_IMP_H_
#define _VHDR_DRV_IMP_H_

#include "vhdr_drv.h"

/**
  *@brief VHDR Pass1 HW Setting Callback
*/
class VHdrP1Cb : public P1_TUNING_NOTIFY
{
 	public:
 	   VHdrP1Cb(MVOID *arg);
       ~VHdrP1Cb();
        
 	virtual void p1TuningNotify(MVOID *pInput,MVOID *pOutput);
    virtual const char* TuningName() { return "Update VHDR"; };
 };


/**
  *@brief Implementation of VHdrDrv class
*/
class VHdrDrvImp : public VHdrDrv
{
public:

    /**
         *@brief Create VHdrDrv object
         *@param[in] sensorIdx : sensor index
         *@return
         *-VHdrDrv object
       */
    static VHdrDrv *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy VHdrDrv object
         *@param[in] userName : user name,i.e. who destroy VHdrDrv object
       */
    virtual MVOID DestroyInstance();

    /**
         *@brief Initialization function
       */
    virtual MINT32 Init();

    /**
         *@brief Unitialization function
       */
    virtual MINT32 Uninit();

    /**
         *@brief Set sensor info
         *@param[in] aSensorDev : sensor device
         *@param[in] aSensorTg : flare sensor TG         
       */
    virtual MVOID SetSensorInfo(const MUINT32 &aSensorDev,const MUINT32 &aSensorTg);

     //---------------------------------------------------------------------------------------------

private:
  
    /**
         *@brief VHdrDrvImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    VHdrDrvImp(const MUINT32 &aSensorIdx);

    /**
         *@brief VHdrDrvImp destructor
       */
    ~VHdrDrvImp() {}

    /**
         *@brief Create memory by using IMem
         *@param[in,out] memSize : input already calculated size and will set to alingSize
         *@param[in] bufCnt : how many memory need to be created
         *@param[in,out] bufInfo : pointer to IMEM_BUF_INFO
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo);

    /**
         *@brief Destroy memory by using IMem
         *@param[in] bufCnt : how many memory need to be destroyed
         *@param[in,out] bufInfo : pointer to IMEM_BUF_INFO
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMemBuf(const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo);

    /**
         *@brief Convert sensor device  enum to ESensorDev_T type
         *@param[in] aSensorDev : sensor device  enum
         *@return
         *-ESensorDev_T type
       */
    ESensorDev_T ConvertSensorDevType(const MUINT32 &aSensorDev);

    /**
         *@brief Convert sensor TG  enum to ESensorTG_T type
         *@param[in] aSensorTg : sensor Tg  enum
         *@return
         *-ESensorTG_T type
       */
    ESensorTG_T ConvertSensorTgType(const MUINT32 &aSensorTg);
   
    /**
         *@brief Save data to file
         *@param[in] filepath : file path
         *@param[in] addr : virtual address
       */
    MBOOL SaveToFile(char const *filepath,const MUINT32 &addr);

    /**
         *@brief Set state
         *@param[in] aState : state
       */
    MVOID SetVHdrState(const VHDR_STATE_ENUM &aState);

    /**
         *@brief Get state
         *@return
         *-state
       */
    VHDR_STATE_ENUM GetVHdrState();

    /**
         *@brief Change thread setting
         *@param[in] userName : thread name
       */
    MVOID ChangeThreadSetting(char const *userName);

    /***************************************************************************************/

    volatile MINT32 mUsers;
    mutable Mutex mLock;

    // INormalPipe
    INormalPipe *m_pNormalPipe;

    // ISP driver object
    IspDrv *m_pISPDrvObj;
    IspDrv *m_pISPVirtDrv;  // for command queue

    //IMem object
    IMemDrv *m_pIMemDrv;
    
    //member variable    
    MUINT32 mSensorIdx;

    // sensor
    MUINT32 mSensorTg;
    MUINT32 mSensorDev;

    // state
    VHDR_STATE_ENUM mState;

    // Pass1 HW setting callback
    VHdrP1Cb *mpVHdrP1Cb;    
};

#endif

