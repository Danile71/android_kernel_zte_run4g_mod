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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _ISP_MGR_G2C_H_
#define _ISP_MGR_G2C_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  G2C
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_G2C : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_G2C    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0x0BA0

    enum
    {
        ERegInfo_CAM_G2C_CONV_0A,
        ERegInfo_CAM_G2C_CONV_0B,
        ERegInfo_CAM_G2C_CONV_1A,
        ERegInfo_CAM_G2C_CONV_1B,
        ERegInfo_CAM_G2C_CONV_2A,
        ERegInfo_CAM_G2C_CONV_2B,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_G2C(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)     
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_G2C_CONV_0A))
    {
 
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_0A); // 0x0BA0
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_0B); // 0x0BA4
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_1A); // 0x0BA8
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_1B); // 0x0BAC
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_2A); // 0x0BB0
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_2B); // 0x0BB4       
    }

    virtual ~ISP_MGR_G2C() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_G2C_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_G2C_DEV : public ISP_MGR_G2C_T
{
public:
    static
    ISP_MGR_G2C_T&
    getInstance()
    {
        static ISP_MGR_G2C_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_G2C_DEV()
        : ISP_MGR_G2C_T(eSensorDev)
    {}

    virtual ~ISP_MGR_G2C_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  G2C_SHADE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_G2C_SHADE : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_G2C_SHADE    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0x0BB8

    enum
    {
        ERegInfo_CAM_G2C_SHADE_CON_1,
        ERegInfo_CAM_G2C_SHADE_CON_2,
        ERegInfo_CAM_G2C_SHADE_CON_3,
        ERegInfo_CAM_G2C_SHADE_TAR,
        ERegInfo_CAM_G2C_SHADE_SP,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_G2C_SHADE(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)      
        , m_u4StartAddr(REG_ADDR(CAM_G2C_SHADE_CON_1))
    {
 
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_G2C_SHADE_CON_1); // 0x0BBC
        INIT_REG_INFO_ADDR(CAM_G2C_SHADE_CON_2); // 0x0BC0
        INIT_REG_INFO_ADDR(CAM_G2C_SHADE_CON_3); // 0x0BC4
        INIT_REG_INFO_ADDR(CAM_G2C_SHADE_TAR);   // 0x0BC8
        INIT_REG_INFO_ADDR(CAM_G2C_SHADE_SP);    // 0x0BCC
    }

    virtual ~ISP_MGR_G2C_SHADE() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_G2C_SHADE_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_G2C_SHADE_DEV : public ISP_MGR_G2C_SHADE_T
{
public:
    static
    ISP_MGR_G2C_SHADE_T&
    getInstance()
    {
        static ISP_MGR_G2C_SHADE_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_G2C_SHADE_DEV()
        : ISP_MGR_G2C_SHADE_T(eSensorDev)
    {}

    virtual ~ISP_MGR_G2C_SHADE_DEV() {}

};


#endif

