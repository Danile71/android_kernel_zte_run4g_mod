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
#ifndef _ISPIO_I_CAMIO_PIPE_H_
#define _ISPIO_I_CAMIO_PIPE_H_

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Pipe Interface
********************************************************************************/
class ICamIOPipe : public IPipe
{
public:
    static EPipeID const ePipeID = ePipeID_1x1_Sensor_Tg_Mem;

public:     ////    Instantiation.
    static ICamIOPipe* createInstance(EDrvScenario const eScenarioID, EScenarioFmt const eScenarioFmt, MINT8 const szUsrName[32] = (MINT8*)"anonymous");

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Class.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    //  Interface of Command Class.
    class ICmd : public IPipeCommand
    {
    public:     ////    Interfaces.
        ICmd(IPipe*const pIPipe);
        virtual MBOOL   verifySelf();
        //
    protected:
        MVOID*const     mpIPipe;
    };

    //
    //  Command: Set 2 parameters.
    class Cmd_Set2Params : public ICmd
    {
    public:     ////    Interfaces.
        Cmd_Set2Params(IPipe*const pIPipe, MUINT32 const u4Param1, MUINT32 const u4Param2);
        virtual MBOOL   execute();
        //
    protected:  ////    Data Members.
        MUINT32         mu4Param1;
        MUINT32         mu4Param2;
    };

    //
    //  Command: Get 1 parameter based on 1 input parameter.
    class Cmd_Get1ParamBasedOn1Input : public ICmd
    {
    public:     ////    Interfaces.
        Cmd_Get1ParamBasedOn1Input(IPipe*const pIPipe, MUINT32 const u4InParam, MUINT32*const pu4OutParam);
        virtual MBOOL   execute();
        //
    protected:  ////    Data Members.
        MUINT32         mu4InParam;
        MUINT32*        mpu4OutParam;
    };

    //
    virtual MBOOL   startCQ0() = 0;
    virtual MBOOL   startCQ0B() = 0;    
    virtual MBOOL   startCQ0_D() = 0;
    virtual MBOOL   startCQ0B_D() = 0;    
    virtual MBOOL   irq(EPipePass pass, EPipeIRQ irq_int, int userEnumss) = 0;
    virtual MBOOL   registerIrq(Irq_t irq) = 0;    
    virtual MBOOL   irq(Irq_t irq) = 0;    
    virtual MUINT32 getIspReg(MUINT32 path) = 0;    
    public:
    //
    //virtual MBOOL   queryRawDMAOutInfo(MUINT32 const imgFmt, MUINT32& u4ImgWidth, MUINT32& u4Stride, MUINT32& pixel_byte);
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_I_CAMIO_PIPE_H_

