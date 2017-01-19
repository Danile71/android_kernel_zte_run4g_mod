/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 *
 * $Author: jc.wu $
 * $Date: 2012/6/5 $
 * $RCSfile: pi_basic_api.c,v $
 * $Revision: #5 $
 *
 *---------------------------------------------------------------------------*/

/** @file dramc_pi_basic_api.c
 *  Basic DRAMC API implementation
 */

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include "dramc_common.h"
#include "dramc_register.h"
#include "dramc_pi_api.h"
#include "emi.h"

#ifdef DUAL_RANKS
unsigned int uiDualRank = 1;
#endif

#define RISCWriteDRAM 	ucDram_Register_Write

U8 VCOK_Cha_Mempll2, VCOK_Cha_Mempll3, VCOK_Cha_Mempll4;
U8 VCOK_Chb_Mempll2, VCOK_Chb_Mempll3, VCOK_Chb_Mempll4;
U8 VCOK_05PHY_Mempll2, VCOK_05PHY_Mempll3, VCOK_05PHY_Mempll4;

#ifdef REG_ACCESS_DELAY
extern U32 uiWriteRegNum, uiRegRegNum;
#endif

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
#if 0
extern int A_Reg3e0, A_Reg3e4, B_Reg3e0, B_Reg3e4;
#endif
extern const U32 uiLPDDR_PHY_Mapping_POP_CHA[32];

extern U8 opt_gw_coarse_value_R0[2], opt_gw_fine_value_R0[2];
extern U8 opt_gw_coarse_value_R1[2], opt_gw_fine_value_R1[2];

#ifdef XTALK_SSO_STRESS

// only for SW worst pattern
const static U32 u4xtalk_pat[76] = { \
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
0x01010101, 0x02020202, 0x04040404, 0x08080808, 0x10101010, 0x20202020, 0x40404040, 0x80808080, \
0x00000000, 0x00000000, 0xffffffff, 0xffffffff, \
0xfefefefe, 0xfdfdfdfd, 0xfbfbfbfb, 0xf7f7f7f7, 0xefefefef, 0xdfdfdfdf, 0xbfbfbfbf, 0x7f7f7f7f, \
0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, \
0xfefefefe, 0x01010101, 0xfdfdfdfd, 0x02020202, 0xfbfbfbfb, 0x04040404, 0xf7f7f7f7, 0x08080808, \
0xefefefef, 0x10101010, 0xdfdfdfdf, 0x20202020, 0xbfbfbfbf, 0x40404040, 0x7f7f7f7f, 0x80808080, \
0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, \
0xfefefefe, 0x01010101, 0xfdfdfdfd, 0x02020202, 0xfbfbfbfb, 0x04040404, 0xf7f7f7f7, 0x08080808, \
0xefefefef, 0x10101010, 0xdfdfdfdf, 0x20202020, 0xbfbfbfbf, 0x40404040, 0x7f7f7f7f, 0x80808080, \
0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, \
};

const static U32 u4xtalk_pat_64[76*2] = { \
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
0x01010101, 0x01010101, 0x02020202, 0x02020202, 0x04040404, 0x04040404, 0x08080808, 0x08080808,\
0x10101010, 0x10101010, 0x20202020, 0x20202020, 0x40404040, 0x40404040, 0x80808080, 0x80808080,\
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xfefefefe, 0xfefefefe, 0xfdfdfdfd, 0xfdfdfdfd, 0xfbfbfbfb, 0xfbfbfbfb, 0xf7f7f7f7, 0xf7f7f7f7,\
0xefefefef, 0xefefefef, 0xdfdfdfdf, 0xdfdfdfdf, 0xbfbfbfbf, 0xbfbfbfbf, 0x7f7f7f7f, 0x7f7f7f7f,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\
0x00000000, 0x00000000, \
0xfefefefe, 0xfefefefe, 0x01010101, 0x01010101, 0xfdfdfdfd, 0xfdfdfdfd, 0x02020202, 0x02020202,\
0xfbfbfbfb, 0xfbfbfbfb, 0x04040404, 0x04040404, 0xf7f7f7f7, 0xf7f7f7f7, 0x08080808, 0x08080808,\
0xefefefef, 0xefefefef, 0x10101010, 0x10101010, 0xdfdfdfdf, 0xdfdfdfdf, 0x20202020, 0x20202020,\
0xbfbfbfbf, 0xbfbfbfbf, 0x40404040, 0x40404040, 0x7f7f7f7f, 0x7f7f7f7f, 0x80808080, 0x80808080,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,\
0x00000000, 0x00000000, 0x00000000, 0x00000000, \
0xfefefefe, 0xfefefefe, 0x01010101, 0x01010101, 0xfdfdfdfd, 0xfdfdfdfd, 0x02020202, 0x02020202,\
0xfbfbfbfb, 0xfbfbfbfb, 0x04040404, 0x04040404, 0xf7f7f7f7, 0xf7f7f7f7, 0x08080808, 0x08080808,\
0xefefefef, 0xefefefef, 0x10101010, 0x10101010, 0xdfdfdfdf, 0xdfdfdfdf, 0x20202020, 0x20202020,\
0xbfbfbfbf, 0xbfbfbfbf, 0x40404040, 0x40404040, 0x7f7f7f7f, 0x7f7f7f7f, 0x80808080, 0x80808080,\
0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff,\
};

const static U32 u4spb0x[64] = { \
0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff, \
};

const static U32 u4spb0x_64[64*2] = { \
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,\
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,\
};
#endif

// num of "1", num of "0", repeat, 
// num of "1", num of "0" (at the end)
static U8 ucsso_bsx[6][5] = { \
1, 2, 7, 0, 1, \
2, 1, 15,0, 2, \
3, 2, 7, 1, 0, \
2, 3, 7, 1, 0, \
4, 3, 7, 1, 0, \
3, 4, 7, 0, 0,\
};


#if fcFOR_CHIP_ID == fcK2

//-------------------------------------------------------------------------
/** ucDramC_Register_Read
 *  DRAMC register read (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  pu4reg_value  Pointer of register read value.
 *  @retval 0: OK, 1: FAIL 
 */
//-------------------------------------------------------------------------
// This function need to be porting by BU requirement
U8 ucDramC_Register_Read(U32 u4reg_addr, U32 *pu4reg_value)
{
    	U8 ucstatus = 0;

	*pu4reg_value = 	(*(volatile unsigned int *)(CHA_DRAMCAO_BASE + (u4reg_addr)));
	return ucstatus;
}

//-------------------------------------------------------------------------
/** ucDramC_Register_Write
 *  DRAMC register write (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  u4reg_value   register write value.
 *  @retval 0: OK, 1: FAIL 
 */
//-------------------------------------------------------------------------
// This function need to be porting by BU requirement
U8 ucDramC_Register_Write(U32 u4reg_addr, U32 u4reg_value)
{
	U8 ucstatus = 0;

	(*(volatile unsigned int *)(CHA_DRAMCAO_BASE + (u4reg_addr))) = u4reg_value;
	dsb();
    
	return ucstatus;
}

//-------------------------------------------------------------------------
/** ucDram_Register_Read
 *  DRAM register read (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  pu4reg_value  Pointer of register read value.
 *  @retval 0: OK, 1: FAIL 
 */
//-------------------------------------------------------------------------
// This function need to be porting by BU requirement
U8 ucDram_Register_Read(U32 u4reg_addr, U32 *pu4reg_value)
{
	U8 ucstatus = 0;

   	*pu4reg_value = 	(*(volatile unsigned int *)(CHA_DRAMCAO_BASE + (u4reg_addr))) |
					(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (u4reg_addr))) |
					(*(volatile unsigned int *)(CHA_DRAMCNAO_BASE + (u4reg_addr)));

#ifdef REG_ACCESS_DELAY
	uiRegRegNum += 3;
#endif

	return ucstatus;
}

//-------------------------------------------------------------------------
/** ucDram_Register_Write
 *  DRAM register write (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  u4reg_value   register write value.
 *  @retval 0: OK, 1: FAIL 
 */
//-------------------------------------------------------------------------
// This function need to be porting by BU requirement
U8 ucDram_Register_Write(U32 u4reg_addr, U32 u4reg_value)
{
	U8 ucstatus = 0;

	(*(volatile unsigned int *)(CHA_DRAMCAO_BASE + (u4reg_addr))) = u4reg_value;
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (u4reg_addr))) = u4reg_value;
	(*(volatile unsigned int *)(CHA_DRAMCNAO_BASE + (u4reg_addr))) = u4reg_value;
	dsb();
    
 #ifdef REG_ACCESS_DELAY
	uiWriteRegNum += 3;
#endif

	return ucstatus;
}

#endif

//-------------------------------------------------------------------------
/** ucDram_Read
 *  DRAM memory read (32-bit).
 *  @param  u4mem_addr    memory address in 32-bit.
 *  @param  pu4mem_value  Pointer of memory read value.
 *  @retval 0: OK, 1: FAIL 
 */
//-------------------------------------------------------------------------
// This function need to be porting by BU requirement
U8 ucDram_Read(U32 u4mem_addr, U32 *pu4mem_value)
{
U8 ucstatus = 0;

    *pu4mem_value = 	(*(volatile unsigned int *)(DDR_BASE + (u4mem_addr))) ;

    return ucstatus;
}

//-------------------------------------------------------------------------
/** ucDram_Write
 *  DRAM memory write (32-bit).
 *  @param  u4mem_addr    register address in 32-bit.
 *  @param  u4mem_value   register write value.
 *  @retval 0: OK, 1: FAIL 
 */
//-------------------------------------------------------------------------
// This function need to be porting by BU requirement
U8 ucDram_Write(U32 u4mem_addr, U32 u4mem_value)
{
U8 ucstatus = 0;

    (*(volatile unsigned int *)(DDR_BASE + (u4mem_addr))) = u4mem_value;
    
    return ucstatus;
}

//-------------------------------------------------------------------------
/** Round_Operation
 *  Round operation of A/B
 *  @param  A   
 *  @param  B   
 *  @retval round(A/B) 
 */
//-------------------------------------------------------------------------
U32 Round_Operation(U32 A, U32 B)
{
    U32 temp;

    if (B == 0)
    {
        return 0xffffffff;
    }
    
    temp = A/B;
        
    if ((A-temp*B) >= ((temp+1)*B-A))
    {
        return (temp+1);
    }
    else
    {
        return temp;
    }    
}


#if fcFOR_CHIP_ID == fcK2

#if 0

DRAM_STATUS_T MemPllInit(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 u4value;

#if 1
	// Reset PLL initial setting in order to be a correct mempll power on sequence.
	// RG_MEMPLL_BIAS_EN = 0, Reg.60ch[6]
	// RG_MEMPLL_BIAS_LPF_EN = 0, Reg.60ch[7]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x060c), &u4value);
	mcCLR_BIT(u4value, 6);
	mcCLR_BIT(u4value, 7);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x060c), u4value);
	// RG_MEMPLL_EN = 0, Reg.604h[28]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0604), &u4value);
	mcCLR_BIT(u4value, 28);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0604), u4value);
	// RG_MEMPLL_DIV_EN = 0, Reg.610h[16]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0610), &u4value);
	mcCLR_BIT(u4value, 16);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0610), u4value);
	// RG_MEMPLLx_EN = 0, Reg.614h[0], Reg.620h[0], Reg.62ch[0]
	// RG_MEMPLLx_AUTOK_EN=1 [23]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0614), &u4value);
	mcCLR_BIT(u4value, 0);
	mcSET_BIT(u4value, 23);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0614), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0620), &u4value);
	mcCLR_BIT(u4value, 0);
	mcSET_BIT(u4value, 23);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0620), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x062c), &u4value);
	mcCLR_BIT(u4value, 0);
	mcSET_BIT(u4value, 23);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x062c), u4value);

	// RG_MEMPLLx_FB_MCK_SEL = 0, Reg.618h[25], Reg.624h[25], Reg.630h[25]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0618), &u4value);
	mcCLR_BIT(u4value, 25);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0618), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0624), &u4value);
	mcCLR_BIT(u4value, 25);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0624), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0630), &u4value);
	mcCLR_BIT(u4value, 25);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0630), u4value);
#endif

	// --------------------------------------------------
	// MEMPLL configuration
	// --------------------------------------------------
	*(volatile unsigned *)MEM_DCM_CTRL = (0x1f << 21) | (0x0 << 16) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x0 << 6) | (0x1f << 1);
	(*(volatile unsigned *)MEM_DCM_CTRL) |= 0x1; // toggle enable
	(*(volatile unsigned *)MEM_DCM_CTRL) &= ~(0x1); // toggle enable

	if (p->dram_type == TYPE_LPDDR3)
	{
		// LPDDR3
#ifdef COMBO_MCP
		RISCWriteDRAM((0x0000001f << 2), emi_set->DRAMC_DDR2CTL_VAL);
#else		
		RISCWriteDRAM((0x0000001f << 2), LPDDR3_DDR2CTL_07C);
#endif		 
		RISCWriteDRAM(0x0080, 0x00f009e0);	// 114563ns, new DATLAT for TOP_PIPE, RX pipe [7:5]
	} 
	else if (p->dram_type == TYPE_LPDDR2)
	{
		// LPDDR2
#ifdef COMBO_MCP
		RISCWriteDRAM((0x0000001f << 2), emi_set->DRAMC_DDR2CTL_VAL);
#else		
		RISCWriteDRAM((0x0000001f << 2), LPDDR2_DDR2CTL_07C);
#endif		
		RISCWriteDRAM(0x0080, 0x00a903e0);	// 114563ns, new DATLAT for TOP_PIPE, RX pipe [7:5]
	}

	RISCWriteDRAM((0x0172<<2), 0x1111ff11);	// [15:12] bypass delay chain for mempllx enable. Need to check??
	RISCWriteDRAM((0x0173<<2), 0xd1111111);	// by_dmpll2_ck_en_dly first, manual enable mempll, [12] TBD
	RISCWriteDRAM(0x0640, 0x000000b1); 		// Sync off.

	mcDELAY_US(1);  	//WAIT 100ns. DA_SDM_ISO_EN=0 -> 100ns -> RG_MEMPLL_BIAS_EN=1
	
	RISCWriteDRAM(0x060c, 0xd0000040);	// 46857ns
	mcDELAY_US(2);	// RG_MEMPLL_BIAS_EN=1 -> 1us -> RG_MEMPLL_EN

	//RISCWriteDRAM(0x0604, 0x10080000);	// 50472ns
	//mcDELAY_US(20);	// RG_MEMPLL_EN=1 -> 20us -> RG_MEMPLL_DIV_EN

	if (p->dram_type == TYPE_LPDDR2)
	{
		RISCWriteDRAM((0x01a6<<2), 0x000e1e02);    // 4xck = VCO/5, RG_MEMPLL_RSV[2]=1

		RISCWriteDRAM(0x0600, 0x72cccccc); // Set RG_MEMPLL_N_INFO = 114.8
		RISCWriteDRAM(0x0610, 0x08700000); // Set RG_MEMPLL_DIV = 7'd56, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
		RISCWriteDRAM(0x0638, 0x10000000); // Set RG_MEMPLL_REFCK_SEL to 1 by PLL1
		RISCWriteDRAM(0x0604, 0x10080000); // Set RG_MEMPLL_EN = 1
		mcDELAY_US(20);	// RG_MEMPLL_EN=1 -> 20us -> RG_MEMPLL_DIV_EN		
		RISCWriteDRAM(0x0610, 0x08710000); // Set RG_MEMPLL_DIV = 7'd56, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
		mcDELAY_US(1);		// RG_MEMPLL_DIV_EN = 1 -> 1us -> RG_MEMPLLx_EN=1

		RISCWriteDRAM(0x0614, 0xc7850f00); // Set RG_MEMPLL2
		RISCWriteDRAM(0x0618, 0x4c00c000); // Set RG_MEMPLL2_RST_DLY[31:30]
		RISCWriteDRAM(0x061C, 0x00021401); // Set RG_MEMPLL2
		RISCWriteDRAM(0x062C, 0xc7850f00); // Set RG_MEMPLL4
		RISCWriteDRAM(0x0630, 0x4c00c000); // Set RG_MEMPLL4_RST_DLY[31:30]
		RISCWriteDRAM(0x0634, 0x00001401); // Set RG_MEMPLL4
		RISCWriteDRAM(0x0620, 0xc7850f00); // Set RG_MEMPLL3
		RISCWriteDRAM(0x0624, 0x4c00c000); // Set RG_MEMPLL3_RST_DLY[31:30]
		RISCWriteDRAM(0x0628, 0x00021401); // Set RG_MEMPLL3

		// wait 100ns
		RISCWriteDRAM(0x0688, 0x00000001); // Set RG_MEMPLL_REFCK_EN[0]

		mcDELAY_US(20);	// RG_MEMPLL_DIV_EN->1us->RG_MEMPLLx_EN

		RISCWriteDRAM(0x060c, 0xd000005d); // Set RG_MEMPLL_BIAS_EN[6], RG_MEMPLL2/3/4_GATE_EN=1(RG_MEMPLL_RESERVE1[2][3][4]=1)

		RISCWriteDRAM(0x0614, 0xc7850f01); // Set RG_MEMPLL2_EN[0]
		RISCWriteDRAM(0x062C, 0xc7850f01); // Set RG_MEMPLL4_EN[0]
		RISCWriteDRAM(0x0620, 0xc7850f01); // Set RG_MEMPLL3_EN[0]

		mcDELAY_US(106);	// RG_MEMPLLx_EN -> wait 100us -> RG_MEMPLL_BIAS_LPF_EN

		RISCWriteDRAM(0x060c, 0xd00000dd); // Set RG_MEMPLL_LPF_BIAS_EN[7], RG_MEMPLL2/3/4_GATE_EN=1(RG_MEMPLL_RESERVE1[2][3][4]=1)

		mcDELAY_US(20);	//PLL ready

		RISCWriteDRAM(0x0614, 0xc7850f00); // Set ~RG_MEMPLL2_EN[0] & RG_MEMPLL2_AUTOK_EN[23]
		RISCWriteDRAM(0x062C, 0xc7850f00); // Set ~RG_MEMPLL4_EN[0] & RG_MEMPLL4_AUTOK_EN[23]
		RISCWriteDRAM(0x0620, 0xc7850f00); // Set ~RG_MEMPLL3_EN[0] & RG_MEMPLL3_AUTOK_EN[23]
		RISCWriteDRAM(0x0614, 0xc7050f00); // Set ~RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
		RISCWriteDRAM(0x062C, 0xc7050f00); // Set ~RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
		RISCWriteDRAM(0x0620, 0xc7050f00); // Set ~RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]

		mcDELAY_US(1);	// RG_MEMPLL_BIAS_LPF_EN -> 1us -> RG_MEMPLLx_FB_MCK_SEL

		RISCWriteDRAM(0x0618, 0x4e00c000); // Set RG_MEMPLL2_FB_MCK_SEL[25]
		RISCWriteDRAM(0x0630, 0x4e00c000); // Set RG_MEMPLL4_FB_MCK_SEL[25]
		RISCWriteDRAM(0x0624, 0x4e00c000); // Set RG_MEMPLL3_FB_MCK_SEL[25]

		mcDELAY_US(1);	// RG_MEMPLLx_FB_MCK_SEL -> 100ns -> RG_MEMPLLx_EN 

		RISCWriteDRAM(0x0614, 0xc7050f01); // Set RG_MEMPLL2_EN[0]
		RISCWriteDRAM(0x062C, 0xc7050f01); // Set RG_MEMPLL4_EN[0]
		RISCWriteDRAM(0x0620, 0xc7050f01); // Set RG_MEMPLL3_EN[0]

#ifdef MEMPLL_NEW_POWERON
		*(volatile unsigned int*)(0x10006000) = 0x0b160001; 
		*(volatile unsigned int*)(0x10006010) |= 0x08000000;  //(4) 0x10006010[27]=1  //Request MEMPLL reset/pdn mode 
		mcDELAY_US(2);
		*(volatile unsigned int*)(0x10006010) &= ~(0x08000000);  //(1) 0x10006010[27]=0 //Unrequest MEMPLL reset/pdn mode and wait settle (1us for reset)
		mcDELAY_US(13);
#else
		mcDELAY_US(20); 
#endif

		RISCWriteDRAM(0x063C, 0x00000006); // set RG_M_CK_DYN_CG_EN_COMB[1], RG_M_CK_DYN_CG_EN[2]
		//RISCWriteDRAM(0x0640, 0x000000b0); // Set R_DMPLL2_CLK_EN[4]

		//set [19:17] back to 3'b000 for SPM use
		RISCWriteDRAM((0x01a6<<2), 0x00001e02);    // 4xck = VCO/5, RG_MEMPLL_RSV[2]=1
	}
	else if (p->dram_type == TYPE_LPDDR3)
	{
		RISCWriteDRAM((0x01a6<<2), 0x000e1e02);    // 4xck = VCO/5, RG_MEMPLL_RSV[2]=0,

		#ifdef DDR_800
		// PLL setting 
		RISCWriteDRAM(0x0600, 0x72492492); // Set RG_MEMPLL_N_INFO = 114.8
		RISCWriteDRAM(0x0610, 0x08940000); // Set RG_MEMPLL_DIV = 7'd74, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
		RISCWriteDRAM(0x0638, 0x10000000); // Set RG_MEMPLL_REFCK_SEL to 1 by PLL1
		RISCWriteDRAM(0x0604, 0x10080000); // Set RG_MEMPLL_EN = 1
		mcDELAY_US(20);	// RG_MEMPLL_EN=1 -> 20us -> RG_MEMPLL_DIV_EN		
		RISCWriteDRAM(0x0610, 0x08950000); // Set RG_MEMPLL_DIV = 7'd74, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
		mcDELAY_US(1);		// RG_MEMPLL_DIV_EN = 1 -> 1us -> RG_MEMPLLx_EN=1
		RISCWriteDRAM(0x0614, 0xc7850f00); // Set RG_MEMPLL2
		RISCWriteDRAM(0x0618, 0x4c00c000); // Set RG_MEMPLL2_RST_DLY[31:30]
		RISCWriteDRAM(0x061C, 0x00021401); // Set RG_MEMPLL2
		RISCWriteDRAM(0x062C, 0xc7850f00); // Set RG_MEMPLL4
		RISCWriteDRAM(0x0630, 0x4c00c000); // Set RG_MEMPLL4_RST_DLY[31:30]
		RISCWriteDRAM(0x0634, 0x00001401); // Set RG_MEMPLL4
		RISCWriteDRAM(0x0620, 0xc7850f00); // Set RG_MEMPLL3
		RISCWriteDRAM(0x0624, 0x4c00c000); // Set RG_MEMPLL3_RST_DLY[31:30]
		RISCWriteDRAM(0x0628, 0x00021401); // Set RG_MEMPLL3
		#else 
			#ifdef DDR_1066
			RISCWriteDRAM(0x0600, 0x70c00000); // Set RG_MEMPLL_N_INFO = 112.75
			RISCWriteDRAM(0x0610, 0x086e0000); // Set [23:17] RG_MEMPLL_DIV = 7'd55 => 37h => 6eh, RG_MEMPLL_DIV_EN = 0, RG_MEMPLL_RESERVE[3]=1
			RISCWriteDRAM(0x0638, 0x10000000); // Set RG_MEMPLL_REFCK_SEL to 1 by PLL1
			RISCWriteDRAM(0x0604, 0x10080000); // Set RG_MEMPLL_EN = 1
			mcDELAY_US(20);	// RG_MEMPLL_EN=1 -> 20us -> RG_MEMPLL_DIV_EN		
			RISCWriteDRAM(0x0610, 0x086f0000); // Set RG_MEMPLL_DIV = 7'd56, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
			mcDELAY_US(1);		// RG_MEMPLL_DIV_EN = 1 -> 1us -> RG_MEMPLLx_EN=1
			RISCWriteDRAM(0x0614, 0xd7840700); // Set RG_MEMPLL2. [31:28] IR=dh [22:16] FBDIV=04h [11:8] IC=07h
			RISCWriteDRAM(0x0618, 0x4c00c000); // Set RG_MEMPLL2_RST_DLY[31:30]
			RISCWriteDRAM(0x061C, 0x00022a01); // Set RG_MEMPLL2 [15:12]=2 [11:10] BR=2 [9:8]=2
			RISCWriteDRAM(0x062C, 0xd7840700); // Set RG_MEMPLL4
			RISCWriteDRAM(0x0630, 0x4c00c000); // Set RG_MEMPLL4_RST_DLY[31:30]
			RISCWriteDRAM(0x0634, 0x00002a01); // Set RG_MEMPLL4 [15:12]=2 [11:10] BR=2 [9:8]=2
			RISCWriteDRAM(0x0620, 0xd7840700); // Set RG_MEMPLL3
			RISCWriteDRAM(0x0624, 0x4c00c000); // Set RG_MEMPLL3_RST_DLY[31:30]
			RISCWriteDRAM(0x0628, 0x00022a01); // Set RG_MEMPLL3
			#else
			// DDR1600
			// PLL setting 
			RISCWriteDRAM(0x0600, 0x72492492); // Set RG_MEMPLL_N_INFO = 114.8
			RISCWriteDRAM(0x0610, 0x08680000); // Set RG_MEMPLL_DIV = 7'd52, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
			RISCWriteDRAM(0x0638, 0x10000000); // Set RG_MEMPLL_REFCK_SEL to 1 by PLL1
			RISCWriteDRAM(0x0604, 0x10080000); // Set RG_MEMPLL_EN = 1
			mcDELAY_US(20);	// RG_MEMPLL_EN=1 -> 20us -> RG_MEMPLL_DIV_EN		
			RISCWriteDRAM(0x0610, 0x08690000); // Set RG_MEMPLL_DIV = 7'd52, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
			mcDELAY_US(1);		// RG_MEMPLL_DIV_EN = 1 -> 1us -> RG_MEMPLLx_EN=1
			RISCWriteDRAM(0x0614, 0xc7860f00); // Set RG_MEMPLL2
			RISCWriteDRAM(0x0618, 0x4c00c000); // Set RG_MEMPLL2_RST_DLY[31:30]
			RISCWriteDRAM(0x061C, 0x00021401); // Set RG_MEMPLL2
			RISCWriteDRAM(0x062C, 0xc7860f00); // Set RG_MEMPLL4
			RISCWriteDRAM(0x0630, 0x4c00c000); // Set RG_MEMPLL4_RST_DLY[31:30]
			RISCWriteDRAM(0x0634, 0x00001401); // Set RG_MEMPLL4
			RISCWriteDRAM(0x0620, 0xc7860f00); // Set RG_MEMPLL3
			RISCWriteDRAM(0x0624, 0x4c00c000); // Set RG_MEMPLL3_RST_DLY[31:30]
			RISCWriteDRAM(0x0628, 0x00021401); // Set RG_MEMPLL3
			#endif
		#endif

		// wait 100ns
		RISCWriteDRAM(0x0688, 0x00000001); // Set RG_MEMPLL_REFCK_EN[0]

		mcDELAY_US(20);  // RG_MEMPLL_DIV_EN->1us->RG_MEMPLLx_EN

		RISCWriteDRAM(0x060c, 0xd000005d); // Set RG_MEMPLL_BIAS_EN[6], RG_MEMPLL2/3/4_GATE_EN=1(RG_MEMPLL_RESERVE1[2][3][4]=1)

		#ifdef DDR_800
		RISCWriteDRAM(0x0614, 0xc7850f01); // Set RG_MEMPLL2_EN[0]
		RISCWriteDRAM(0x062C, 0xc7850f01); // Set RG_MEMPLL4_EN[0]
		RISCWriteDRAM(0x0620, 0xc7850f01); // Set RG_MEMPLL3_EN[0]
		#else
			#ifdef DDR_1066
			RISCWriteDRAM(0x0614, 0xd7840701); // Set RG_MEMPLL2_EN[0]
			RISCWriteDRAM(0x062C, 0xd7840701); // Set RG_MEMPLL4_EN[0]
			RISCWriteDRAM(0x0620, 0xd7840701); // Set RG_MEMPLL3_EN[0]			
			#else		
			RISCWriteDRAM(0x0614, 0xc7860f01); // Set RG_MEMPLL2_EN[0]
			RISCWriteDRAM(0x062C, 0xc7860f01); // Set RG_MEMPLL4_EN[0]
			RISCWriteDRAM(0x0620, 0xc7860f01); // Set RG_MEMPLL3_EN[0]
			#endif
		#endif

		mcDELAY_US(106);	// RG_MEMPLLx_EN -> wait 100us -> RG_MEMPLL_BIAS_LPF_EN

		RISCWriteDRAM(0x060c, 0xd00000dd); // Set RG_MEMPLL_LPF_BIAS_EN[7], RG_MEMPLL2/3/4_GATE_EN=1(RG_MEMPLL_RESERVE1[2][3][4]=1)

		mcDELAY_US(20);	// PLL ready 

		#ifdef DDR_800
		RISCWriteDRAM(0x0614, 0xc7850f00); // Set RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
		RISCWriteDRAM(0x062C, 0xc7850f00); // Set RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
		RISCWriteDRAM(0x0620, 0xc7850f00); // Set RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]
		RISCWriteDRAM(0x0614, 0xc7050f00); // Set ~RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
		RISCWriteDRAM(0x062C, 0xc7050f00); // Set ~RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
		RISCWriteDRAM(0x0620, 0xc7050f00); // Set ~RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]
		#else
			#ifdef DDR_1066
			RISCWriteDRAM(0x0614, 0xd7840700); // Set ~RG_MEMPLL2_EN[0] & RG_MEMPLL2_AUTOK_EN[23]
			RISCWriteDRAM(0x062C, 0xd7840700); // Set ~RG_MEMPLL4_EN[0] & RG_MEMPLL4_AUTOK_EN[23]
			RISCWriteDRAM(0x0620, 0xd7840700); // Set ~RG_MEMPLL3_EN[0] & RG_MEMPLL3_AUTOK_EN[23]
			RISCWriteDRAM(0x0614, 0xd7040700); // Set ~RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
			RISCWriteDRAM(0x062C, 0xd7040700); // Set ~RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
			RISCWriteDRAM(0x0620, 0xd7040700); // Set ~RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]
			#else
			RISCWriteDRAM(0x0614, 0xc7860f00); // Set RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
			RISCWriteDRAM(0x062C, 0xc7860f00); // Set RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
			RISCWriteDRAM(0x0620, 0xc7860f00); // Set RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]
			RISCWriteDRAM(0x0614, 0xc7060f00); // Set ~RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
			RISCWriteDRAM(0x062C, 0xc7060f00); // Set ~RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
			RISCWriteDRAM(0x0620, 0xc7060f00); // Set ~RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]
			#endif
		#endif

		mcDELAY_US(1);		// RG_MEMPLL_BIAS_LPF_EN -> 1us -> RG_MEMPLLx_FB_MCK_SEL

		RISCWriteDRAM(0x0618, 0x4e00c000); // Set RG_MEMPLL2_FB_MCK_SEL[25]
		RISCWriteDRAM(0x0630, 0x4e00c000); // Set RG_MEMPLL4_FB_MCK_SEL[25]
		RISCWriteDRAM(0x0624, 0x4e00c000); // Set RG_MEMPLL3_FB_MCK_SEL[25]

		mcDELAY_US(1);		//** new added. RG_MEMPLLx_FB_MCK_SEL -> 100ns -> RG_MEMPLLx_EN 
		// wait 100ns
		#ifdef DDR_800
		RISCWriteDRAM(0x0614, 0xc7050f01); // Set RG_MEMPLL2_EN[0]
		RISCWriteDRAM(0x062C, 0xc7050f01); // Set RG_MEMPLL4_EN[0]
		RISCWriteDRAM(0x0620, 0xc7050f01); // Set RG_MEMPLL3_EN[0]
		#else
			#ifdef DDR_1066
			RISCWriteDRAM(0x0614, 0xd7040701); // Set RG_MEMPLL2_EN[0]
			RISCWriteDRAM(0x062C, 0xd7040701); // Set RG_MEMPLL4_EN[0]
			RISCWriteDRAM(0x0620, 0xd7040701); // Set RG_MEMPLL3_EN[0]			
			#else
			RISCWriteDRAM(0x0614, 0xc7060f01); // Set RG_MEMPLL2_EN[0]
			RISCWriteDRAM(0x062C, 0xc7060f01); // Set RG_MEMPLL4_EN[0]
			RISCWriteDRAM(0x0620, 0xc7060f01); // Set RG_MEMPLL3_EN[0]
			#endif
		#endif

#ifdef MEMPLL_NEW_POWERON
		*(volatile unsigned int*)(0x10006000) = 0x0b160001; 
		*(volatile unsigned int*)(0x10006010) |= 0x08000000;  //(4) 0x10006010[27]=1  //Request MEMPLL reset/pdn mode 
		mcDELAY_US(2);
		*(volatile unsigned int*)(0x10006010) &= ~(0x08000000);  //(1) 0x10006010[27]=0 //Unrequest MEMPLL reset/pdn mode and wait settle (1us for reset)
		mcDELAY_US(13);
#else
		mcDELAY_US(20);  
#endif

		RISCWriteDRAM(0x063C, 0x00000006); // set RG_M_CK_DYN_CG_EN_COMB[1], RG_M_CK_DYN_CG_EN[2]
		//RISCWriteDRAM(0x0640, 0x000000b0); // [0] =0 sync =1 Set R_DMPLL2_CLK_EN[4]

		//set [19:17] back to 3'b000 for SPM use
		RISCWriteDRAM((0x01a6<<2), 0x00001e02);    // 4xck = VCO/5, RG_MEMPLL_RSV[2]=0,
	}
	else
	{
		mcSHOW_ERR_MSG(("[MemPllInit] DDR type is wrong.\n"));
	}

	mcDELAY_US(20); 

	RISCWriteDRAM(0x0640, 0x000000b0); // [0] =0 sync =1 Set R_DMPLL2_CLK_EN[4]
	RISCWriteDRAM(0x0640, 0x000200b1); // [0] =1, Reg.5cch[12]=1 bypass delay chain. Reg.5cch[8]=1 from Reg.640h[5]=1 --> invert to 0. So sync = 0.
	mcDELAY_US(1); 


#ifdef fcMEMPLL_DBG_MONITOR
    // delay 20us
    mcDELAY_US(20);

    // Monitor enable
    // RG_MEMPLL*_MONCK_EN=1; RG_MEMPLL*_MONREF=1
    // PLL2
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x614), &u4value);
    mcSET_BIT(u4value, 3);
    mcSET_BIT(u4value, 1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x614), u4value);
    // PLL3
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x620), &u4value);
    mcSET_BIT(u4value, 3);
    mcSET_BIT(u4value, 1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x620), u4value);
    // PLL4
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x62c), &u4value);
    mcSET_BIT(u4value, 3);
    mcSET_BIT(u4value, 1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x62c), u4value);    

    // 0x61c[22] RG_MEMPLL2_MON_EN=1, 0x61c[21:19] RG_MEMPLL2_CKSEL_MON=100
    // RG_MEMPLL2_MON_EN -> to enable M5
    // RG_MEMPLL2_CKSEL_MON = 100 (select M5)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x61c), &u4value);
    mcSET_BIT(u4value, 22);
    mcSET_FIELD(u4value, 4, 0x00380000, 19);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x61c), u4value);

    // 0x628[22] RG_MEMPLL3_MON_EN=1, 0x628[21:19] RG_MEMPLL3_CKSEL_MON=100
    // RG_MEMPLL3_MON_EN -> to enable M5
    // RG_MEMPLL3_CKSEL_MON = 100 (select M5)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x628), &u4value);
    mcSET_BIT(u4value, 22);
    mcSET_FIELD(u4value, 4, 0x00380000, 19);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x628), u4value);

    // 0x634[22] RG_MEMPLL4_MON_EN=1, 0x634[21:19] RG_MEMPLL4_CKSEL_MON=100 
    // RG_MEMPLL4_MON_EN -> to enable M5
    // RG_MEMPLL4_CKSEL_MON = 100 (select M5)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x634), &u4value);
    mcSET_BIT(u4value, 22);
    mcSET_FIELD(u4value, 4, 0x00380000, 19);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x634), u4value);

    // 0x60c[23:8] RG_MEMPLL_TEST[15:0]; 
    // RG_MEMPLL_TEST[5:4]=01,RG_MEMPLL_TEST[3]=1;RG_MEMPLL_TEST[1:0]=11
    // RG_MEMPLL_TEST[5:4]=01 -> MEMPLL2
    // RG_MEMPLL_TEST[5:4]=10 -> MEMPLL3
    // RG_MEMPLL_TEST[5:4]=11 -> MEMPLL4
    // RG_MEMPLL_TEST[3,1] -> select CKMUX (measure clock or voltage)
    // RG_MEMPLL_TEST[0] -> RG_A2DCK_EN (for FT)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x60c), &u4value);
    mcSET_FIELD(u4value, 3, 0x00000300, 8);
    mcSET_BIT(u4value, 11);
    mcSET_FIELD(u4value, 01, 0x00003000, 12);	// 00: MEMPLL1. 01: MEMPLL2...
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x60c), u4value);

/*
    //0x638[26:24] RG_MEMPLL_TEST_DIV2_SEL=011, /8 (for FT)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x638), &u4value);
    mcSET_FIELD(u4value, 3, 0x07000000, 24);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x638), u4value);*/
#endif

#if 0
    // for debug
    u4value = DramOperateDataRate(p);
    mcSHOW_DBG_MSG(("DRAM Operation Data Rate: DDR-%d\n", u4value));
#endif
            
    if (ucstatus)
    {
        return DRAM_FAIL;
    }
    else
    {
        return DRAM_OK;
    }    
}

#else

DRAM_STATUS_T MemPllInit(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 u4value;
	U32 MEMPLL_N_INFO, MEMPLL_DIV;
	U32 MEMPLLx_FBDIV, MEMPLLx_FBDIV_0, MEMPLLx_BR, MEMPLLx_BC, MEMPLLx_IR, MEMPLLx_IC, MEMPLLx_BP, MEMPLLx_M4PDIV;
#ifdef COMBO_MCP
    int dram_type;
    EMI_SETTINGS *emi_set;
    
    if(emi_setting_index == -1)   
    {
        emi_set = default_emi_setting;
#if 0                    
        dram_type = mt_get_dram_type();
        if(dram_type == TYPE_LPDDR2)
            emi_set = emi_setting_default_lpddr2;
        else if(dram_type == TYPE_LPDDR3)
            emi_set = emi_setting_default_lpddr3;
#endif            
    }
    else
    {
        emi_set = &emi_settings[emi_setting_index];
    }                              
#endif

#ifndef FREQ_ADJUST
    #ifdef DDR_1866
    	MEMPLL_N_INFO = 0x6fa4158d;
    	MEMPLL_DIV = 0x38;
    	MEMPLLx_FBDIV = 0x08;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x00;
    	MEMPLLx_IR = 0x0d;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    #endif	
    #ifdef DDR_1780
    	MEMPLL_N_INFO = 0x7163b13b;
    	MEMPLL_DIV = 0x35;
    	MEMPLLx_FBDIV = 0x07;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x00;
    	MEMPLLx_IR = 0x0c;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    #endif	
    #ifdef DDR_1600
    	MEMPLL_N_INFO = 0x72492492;
    	MEMPLL_DIV = 0x34;
    	MEMPLLx_FBDIV = 0x06;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x0a;
    	MEMPLLx_IC = 0x07;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    #endif	
    #ifdef DDR_1333
    	MEMPLL_N_INFO = 0x71383483;
    	MEMPLL_DIV = 0x35;
    	MEMPLLx_FBDIV = 0x05;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x08;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    #endif	
    #ifdef DDR_1066
    	MEMPLL_N_INFO = 0x70c00000;
    	MEMPLL_DIV = 0x37;
    	MEMPLLx_FBDIV = 0x04;
    	MEMPLLx_BR = 0x02;
    	MEMPLLx_BC = 0x02;
    	MEMPLLx_IR = 0x0d;
    	MEMPLLx_IC = 0x07;
    	MEMPLLx_BP = 0x02;
    	MEMPLLx_M4PDIV = 0x0;
    #endif	
    #ifdef DDR_1160
    	MEMPLL_N_INFO = 0x71c4ec4e;
    	MEMPLL_DIV = 0x33;
    	MEMPLLx_FBDIV = 0x04;
    	MEMPLLx_BR = 0x02;
    	MEMPLLx_BC = 0x02;
    	MEMPLLx_IR = 0x0d;
    	MEMPLLx_IC = 0x07;
    	MEMPLLx_BP = 0x02;
    	MEMPLLx_M4PDIV = 0x0;
    #endif
    #ifdef DDR_800
    	MEMPLL_N_INFO = 0x6f89d89d;
    	MEMPLL_DIV = 0x3a;
    	MEMPLLx_FBDIV = 0x07;
    	MEMPLLx_FBDIV_0 = 0x03;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x0b;
    	MEMPLLx_IC = 0x0c;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x01;
    #endif	
    #ifdef DDR_667
    	MEMPLL_N_INFO = 0x714dee6b;
    	MEMPLL_DIV = 0x35;
    	MEMPLLx_FBDIV = 0x05;
    	MEMPLLx_FBDIV_0 = 0x02;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x08;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x01;
    #endif
#else
    if(p->frequency == 933) //DDR_1866
    {
    	MEMPLL_N_INFO = 0x6fa4158d;
    	MEMPLL_DIV = 0x38;
    	MEMPLLx_FBDIV = 0x08;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x00;
    	MEMPLLx_IR = 0x0d;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    }
    else if(p->frequency == 890) //DDR_1780	
    {
    	MEMPLL_N_INFO = 0x7163b13b;
    	MEMPLL_DIV = 0x35;
    	MEMPLLx_FBDIV = 0x07;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x00;
    	MEMPLLx_IR = 0x0c;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    }
    else if(p->frequency == 800)	//DDR_1600
    {
    	MEMPLL_N_INFO = 0x72492492;
    	MEMPLL_DIV = 0x34;
    	MEMPLLx_FBDIV = 0x06;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x0a;
    	MEMPLLx_IC = 0x07;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    }
    else if(p->frequency == 666) //DDR_1333	
    {
    	MEMPLL_N_INFO = 0x71383483;
    	MEMPLL_DIV = 0x35;
    	MEMPLLx_FBDIV = 0x05;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x08;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x0;
    }
    else if(p->frequency == 533)	//DDR_1066
    {
    	MEMPLL_N_INFO = 0x70c00000;
    	MEMPLL_DIV = 0x37;
    	MEMPLLx_FBDIV = 0x04;
    	MEMPLLx_BR = 0x02;
    	MEMPLLx_BC = 0x02;
    	MEMPLLx_IR = 0x0d;
    	MEMPLLx_IC = 0x07;
    	MEMPLLx_BP = 0x02;
    	MEMPLLx_M4PDIV = 0x0;
    }	
    else if(p->frequency == 580) //DDR_1160
    {
    	MEMPLL_N_INFO = 0x71c4ec4e;
    	MEMPLL_DIV = 0x33;
    	MEMPLLx_FBDIV = 0x04;
    	MEMPLLx_BR = 0x02;
    	MEMPLLx_BC = 0x02;
    	MEMPLLx_IR = 0x0d;
    	MEMPLLx_IC = 0x07;
    	MEMPLLx_BP = 0x02;
    	MEMPLLx_M4PDIV = 0x0;
    }
    else if(p->frequency == 400) //DDR_800
    {
    	MEMPLL_N_INFO = 0x6f89d89d;
    	MEMPLL_DIV = 0x3a;
    	MEMPLLx_FBDIV = 0x07;
    	MEMPLLx_FBDIV_0 = 0x03;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x0b;
    	MEMPLLx_IC = 0x0c;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x01;
    }
    else if(p->frequency == 333) //DDR_667	
    {
    	MEMPLL_N_INFO = 0x714dee6b;
    	MEMPLL_DIV = 0x35;
    	MEMPLLx_FBDIV = 0x05;
    	MEMPLLx_FBDIV_0 = 0x02;
    	MEMPLLx_BR = 0x01;
    	MEMPLLx_BC = 0x01;
    	MEMPLLx_IR = 0x08;
    	MEMPLLx_IC = 0x06;
    	MEMPLLx_BP = 0x01;
    	MEMPLLx_M4PDIV = 0x01;
    }
#endif    

#if 1

//	RISCWriteDRAM(0x0640, 0x09000000); 		
//	RISCWriteDRAM(0x0640, 0x09000010); 		

	// Reset PLL initial setting in order to be a correct mempll power on sequence.
	// RG_MEMPLL_BIAS_EN = 0, Reg.60ch[6]
	// RG_MEMPLL_BIAS_LPF_EN = 0, Reg.60ch[7]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x060c), &u4value);
	mcCLR_BIT(u4value, 6);
	mcCLR_BIT(u4value, 7);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x060c), u4value);
	// RG_MEMPLL_EN = 0, Reg.604h[28]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0604), &u4value);
	mcCLR_BIT(u4value, 28);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0604), u4value);
	// RG_MEMPLL_DIV_EN = 0, Reg.610h[16]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0610), &u4value);
	mcCLR_BIT(u4value, 16);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0610), u4value);
	// RG_MEMPLLx_EN = 0, Reg.614h[0], Reg.620h[0], Reg.62ch[0]
	// RG_MEMPLLx_AUTOK_EN=1 [23]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0614), &u4value);
	mcCLR_BIT(u4value, 0);
	mcSET_BIT(u4value, 23);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0614), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0620), &u4value);
	mcCLR_BIT(u4value, 0);
	mcSET_BIT(u4value, 23);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0620), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x062c), &u4value);
	mcCLR_BIT(u4value, 0);
	mcSET_BIT(u4value, 23);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x062c), u4value);

	// RG_MEMPLLx_FB_MCK_SEL = 0, Reg.618h[25], Reg.624h[25], Reg.630h[25]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0618), &u4value);
	mcCLR_BIT(u4value, 25);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0618), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0624), &u4value);
	mcCLR_BIT(u4value, 25);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0624), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0630), &u4value);
	mcCLR_BIT(u4value, 25);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0630), u4value);


	RISCWriteDRAM(0x0614, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL2
	RISCWriteDRAM(0x0618, 0x4c00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL2_RST_DLY[31:30]
	RISCWriteDRAM(0x061C, 0x00020001 | (MEMPLLx_BR << 10) | (MEMPLLx_BC << 8) | (MEMPLLx_BP << 12)); // Set RG_MEMPLL2
	RISCWriteDRAM(0x062C, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL4
	RISCWriteDRAM(0x0630, 0x4c00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL4_RST_DLY[31:30]
	RISCWriteDRAM(0x0634, 0x00000001 | (MEMPLLx_BR << 10) | (MEMPLLx_BC << 8) | (MEMPLLx_BP << 12)); // Set RG_MEMPLL4
	RISCWriteDRAM(0x0620, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL3
	RISCWriteDRAM(0x0624, 0x4c00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL3_RST_DLY[31:30]
	RISCWriteDRAM(0x0628, 0x00020001 | (MEMPLLx_BR << 10) | (MEMPLLx_BC << 8) | (MEMPLLx_BP << 12)); // Set RG_MEMPLL3
	RISCWriteDRAM((0x01a6<<2), 0x00001e00);    // 4xck = VCO/5, RG_MEMPLL_RSV[2]=1
	RISCWriteDRAM(0x063C, 0x00000000); 
	RISCWriteDRAM(0x0688, 0x00000000); // Set RG_MEMPLL_REFCK_EN[0]
	RISCWriteDRAM(0x0638, 0x00000000); // Set RG_MEMPLL_REFCK_SEL to 1 by PLL1
#endif


	// --------------------------------------------------
	// MEMPLL configuration
	// --------------------------------------------------
	*(volatile unsigned *)MEM_DCM_CTRL = (0x1f << 21) | (0x0 << 16) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x0 << 6) | (0x1f << 1);
	(*(volatile unsigned *)MEM_DCM_CTRL) |= 0x1; // toggle enable
	(*(volatile unsigned *)MEM_DCM_CTRL) &= ~(0x1); // toggle enable

	if (p->dram_type == TYPE_LPDDR3)
	{
		// LPDDR3
#ifdef COMBO_MCP
		RISCWriteDRAM((0x0000001f << 2), emi_set->DRAMC_DDR2CTL_VAL); 
#else		
		RISCWriteDRAM((0x0000001f << 2), LPDDR3_DDR2CTL_07C); 
#endif
		RISCWriteDRAM(0x0080, 0x00f00ae0);	// 114563ns, new DATLAT for TOP_PIPE, RX pipe [7:5]
	}  
	else if  (p->dram_type == TYPE_LPDDR2)
	{
		// LPDDR2
#ifdef COMBO_MCP		
		RISCWriteDRAM((0x0000001f << 2), emi_set->DRAMC_DDR2CTL_VAL);
#else
		RISCWriteDRAM((0x0000001f << 2), LPDDR2_DDR2CTL_07C);
#endif		
		RISCWriteDRAM(0x0080, 0x00f00ae0);	// 114563ns, new DATLAT for TOP_PIPE, RX pipe [7:5]
	}

	RISCWriteDRAM((0x0172<<2), 0x1111ff11);	// [15:12] bypass delay chain for mempllx enable. Need to check??
	RISCWriteDRAM((0x0173<<2), 0xd1111111);	// by_dmpll2_ck_en_dly first, manual enable mempll, [12] TBD
	RISCWriteDRAM(0x0640, 0x000000b1); 		// Sync off.
	mcDELAY_US(1); 	//WAIT 100ns. DA_SDM_ISO_EN=0 -> 100ns -> RG_MEMPLL_BIAS_EN=1
	
	RISCWriteDRAM(0x060c, 0xd0000040);	// 46857ns
	mcDELAY_US(2);	// RG_MEMPLL_BIAS_EN=1 -> 1us -> RG_MEMPLL_EN

	//RISCWriteDRAM(0x0604, 0x10080000);	// 50472ns
	//mcDELAY_US(20);	// RG_MEMPLL_EN=1 -> 20us -> RG_MEMPLL_DIV_EN

	RISCWriteDRAM((0x01a6<<2), 0x00001e02);    // 4xck = VCO/5, RG_MEMPLL_RSV[2]=1

	RISCWriteDRAM(0x0600, MEMPLL_N_INFO); // Set RG_MEMPLL_N_INFO = 114.8
	RISCWriteDRAM(0x0610, 0x08000000 | (MEMPLL_DIV << 17)); // Set RG_MEMPLL_DIV = 7'd56, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
	RISCWriteDRAM(0x0638, 0x10000000); // Set RG_MEMPLL_REFCK_SEL to 1 by PLL1
	RISCWriteDRAM(0x0604, 0x10080000); // Set RG_MEMPLL_EN = 1
	mcDELAY_US(20);	// RG_MEMPLL_EN=1 -> 20us -> RG_MEMPLL_DIV_EN		
	RISCWriteDRAM(0x0610, 0x08010000 | (MEMPLL_DIV << 17)); // Set RG_MEMPLL_DIV = 7'd56, RG_MEMPLL_DIV_EN = 1, RG_MEMPLL_RESERVE[3]=1
	mcDELAY_US(1);		// RG_MEMPLL_DIV_EN = 1 -> 1us -> RG_MEMPLLx_EN=1
	
	RISCWriteDRAM(0x0614, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL2
	RISCWriteDRAM(0x0618, 0x4c00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL2_RST_DLY[31:30]
	RISCWriteDRAM(0x061C, 0x00020001 | (MEMPLLx_BR << 10) | (MEMPLLx_BC << 8) | (MEMPLLx_BP << 12)); // Set RG_MEMPLL2
	RISCWriteDRAM(0x062C, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL4
	RISCWriteDRAM(0x0630, 0x4c00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL4_RST_DLY[31:30]
	RISCWriteDRAM(0x0634, 0x00000001 | (MEMPLLx_BR << 10) | (MEMPLLx_BC << 8) | (MEMPLLx_BP << 12)); // Set RG_MEMPLL4
	RISCWriteDRAM(0x0620, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL3
	RISCWriteDRAM(0x0624, 0x4c00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL3_RST_DLY[31:30]
	RISCWriteDRAM(0x0628, 0x00020001 | (MEMPLLx_BR << 10) | (MEMPLLx_BC << 8) | (MEMPLLx_BP << 12)); // Set RG_MEMPLL3

	// wait 100ns
	RISCWriteDRAM(0x0688, 0x00000001); // Set RG_MEMPLL_REFCK_EN[0]

	mcDELAY_US(20);	// RG_MEMPLL_DIV_EN->1us->RG_MEMPLLx_EN

	RISCWriteDRAM(0x060c, 0xd000005d); // Set RG_MEMPLL_BIAS_EN[6], RG_MEMPLL2/3/4_GATE_EN=1(RG_MEMPLL_RESERVE1[2][3][4]=1)

	RISCWriteDRAM(0x0614, 0x07800001 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL2_EN[0]
	RISCWriteDRAM(0x062C, 0x07800001 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL4_EN[0]
	RISCWriteDRAM(0x0620, 0x07800001 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL3_EN[0]

	mcDELAY_US(106);	// RG_MEMPLLx_EN -> wait 100us -> RG_MEMPLL_BIAS_LPF_EN

	RISCWriteDRAM(0x060c, 0xd00000dd); // Set RG_MEMPLL_LPF_BIAS_EN[7], RG_MEMPLL2/3/4_GATE_EN=1(RG_MEMPLL_RESERVE1[2][3][4]=1)

	mcDELAY_US(20);	//PLL ready (internal loop)

#ifdef MEMPLL_BAND_INFO
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x3e0), &u4value);
	VCOK_Cha_Mempll2 = (u4value >> 16) & 0x7f;
	VCOK_Cha_Mempll3 = (u4value >> 8) & 0x7f;
        mcSHOW_DBG_MSG3(("0x3e0=0x%x, VCOK_Cha_Mempll2=0x%x, VCOK_Cha_Mempll3=0x%x \n", 
        	u4value, VCOK_Cha_Mempll2, VCOK_Cha_Mempll3));

        // RG_MEMPLL_RESERVE[2]=1, to select MEMPLL4 band register
        // RGS_MEMPLL4_AUTOK_BAND[6:0]= RGS_MEMPLL4_AUTOK_BAND[6]+RGS_MEMPLL3_AUTOK_BAND[5:0]
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x610), &u4value);
        mcSET_BIT(u4value, 26);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x610), u4value);

        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x3e0), &u4value);
        VCOK_Cha_Mempll4 = ((u4value >> 8) & 0x3f) | (u4value & 0x40);
        mcSHOW_DBG_MSG3(("0x3e0=0x%x, VCOK_Cha_Mempll4=0x%x\n", u4value, VCOK_Cha_Mempll4));        

        // RG_MEMPLL_RESERVE[2]=0, recover back
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x610), &u4value);
        mcCLR_BIT(u4value, 26);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x610), u4value);        
#endif

	RISCWriteDRAM(0x0614, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL2_EN[0] & RG_MEMPLL2_AUTOK_EN[23]
	RISCWriteDRAM(0x062C, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL4_EN[0] & RG_MEMPLL4_AUTOK_EN[23]
	RISCWriteDRAM(0x0620, 0x07800000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL3_EN[0] & RG_MEMPLL3_AUTOK_EN[23]
	RISCWriteDRAM(0x0614, 0x07000000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
	RISCWriteDRAM(0x062C, 0x07000000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
	RISCWriteDRAM(0x0620, 0x07000000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]

#if defined(DDR_800) ||defined(DDR_667)
	MEMPLLx_FBDIV = MEMPLLx_FBDIV_0;	
	RISCWriteDRAM(0x0614, 0x07000000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL2_EN[0] & ~RG_MEMPLL2_AUTOK_EN[23]
	RISCWriteDRAM(0x062C, 0x07000000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL4_EN[0] & ~RG_MEMPLL4_AUTOK_EN[23]
	RISCWriteDRAM(0x0620, 0x07000000 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set ~RG_MEMPLL3_EN[0] & ~RG_MEMPLL3_AUTOK_EN[23]
#endif
	
	mcDELAY_US(1);	// RG_MEMPLL_BIAS_LPF_EN -> 1us -> RG_MEMPLLx_FB_MCK_SEL

	RISCWriteDRAM(0x0618, 0x4e00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL2_FB_MCK_SEL[25]
	RISCWriteDRAM(0x0630, 0x4e00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL4_FB_MCK_SEL[25]
	RISCWriteDRAM(0x0624, 0x4e00c000 | (MEMPLLx_M4PDIV << 28)); // Set RG_MEMPLL3_FB_MCK_SEL[25]

	mcDELAY_US(1);	// RG_MEMPLLx_FB_MCK_SEL -> 100ns -> RG_MEMPLLx_EN 

	RISCWriteDRAM(0x0614, 0x07000001 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL2_EN[0]
	RISCWriteDRAM(0x062C, 0x07000001 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL4_EN[0]
	RISCWriteDRAM(0x0620, 0x07000001 | (MEMPLLx_FBDIV << 16) | (MEMPLLx_IR << 28) | (MEMPLLx_IC << 8)); // Set RG_MEMPLL3_EN[0]

#ifdef MEMPLL_NEW_POWERON
	*(volatile unsigned int*)(0x10006000) = 0x0b160001; 
	*(volatile unsigned int*)(0x10006010) |= 0x08000000;  //(4) 0x10006010[27]=1  //Request MEMPLL reset/pdn mode 
	mcDELAY_US(2);
	*(volatile unsigned int*)(0x10006010) &= ~(0x08000000);  //(1) 0x10006010[27]=0 //Unrequest MEMPLL reset/pdn mode and wait settle (1us for reset)
	mcDELAY_US(13);
#else
	mcDELAY_US(20);
#endif

	#ifndef DYNAMIC_CLK_OFF
//	RISCWriteDRAM(0x063C, 0x00000006); // set RG_M_CK_DYN_CG_EN_COMB[1], RG_M_CK_DYN_CG_EN[2]
	#endif
	
	//RISCWriteDRAM(0x0640, 0x000000b0); // Set R_DMPLL2_CLK_EN[4]

	//set [19:17] back to 3'b000 for SPM use
	RISCWriteDRAM((0x01a6<<2), 0x00001e02);    // 4xck = VCO/5, RG_MEMPLL_RSV[2]=1

	mcDELAY_US(20);

	RISCWriteDRAM(0x0640, 0x000000b0); // Set R_DMPLL2_CLK_EN[4]
	RISCWriteDRAM(0x0640, 0x000200b1); // [0] =1, Reg.5cch[12]=1 bypass delay chain. Reg.5cch[8]=1 from Reg.640h[5]=1 --> invert to 0. So sync = 0.
	mcDELAY_US(1);

	#ifndef DYNAMIC_CLK_OFF
	// Need to move after DIV2 sync.
	RISCWriteDRAM(0x063C, 0x00000006); // set RG_M_CK_DYN_CG_EN_COMB[1], RG_M_CK_DYN_CG_EN[2]
	#endif

#ifdef fcMEMPLL_DBG_MONITOR
    // delay 20us
    mcDELAY_US(20);

    // Monitor enable
    // RG_MEMPLL*_MONCK_EN=1; RG_MEMPLL*_MONREF=1
    // PLL2
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x614), &u4value);
    mcSET_BIT(u4value, 3);
    mcSET_BIT(u4value, 1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x614), u4value);
    // PLL3
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x620), &u4value);
    mcSET_BIT(u4value, 3);
    mcSET_BIT(u4value, 1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x620), u4value);
    // PLL4
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x62c), &u4value);
    mcSET_BIT(u4value, 3);
    mcSET_BIT(u4value, 1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x62c), u4value);    

    // 0x61c[22] RG_MEMPLL2_MON_EN=1, 0x61c[21:19] RG_MEMPLL2_CKSEL_MON=100
    // RG_MEMPLL2_MON_EN -> to enable M5
    // RG_MEMPLL2_CKSEL_MON = 100 (select M5)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x61c), &u4value);
    mcSET_BIT(u4value, 22);
    mcSET_FIELD(u4value, 4, 0x00380000, 19);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x61c), u4value);

    // 0x628[22] RG_MEMPLL3_MON_EN=1, 0x628[21:19] RG_MEMPLL3_CKSEL_MON=100
    // RG_MEMPLL3_MON_EN -> to enable M5
    // RG_MEMPLL3_CKSEL_MON = 100 (select M5)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x628), &u4value);
    mcSET_BIT(u4value, 22);
    mcSET_FIELD(u4value, 4, 0x00380000, 19);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x628), u4value);

    // 0x634[22] RG_MEMPLL4_MON_EN=1, 0x634[21:19] RG_MEMPLL4_CKSEL_MON=100 
    // RG_MEMPLL4_MON_EN -> to enable M5
    // RG_MEMPLL4_CKSEL_MON = 100 (select M5)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x634), &u4value);
    mcSET_BIT(u4value, 22);
    mcSET_FIELD(u4value, 4, 0x00380000, 19);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x634), u4value);

    // 0x60c[23:8] RG_MEMPLL_TEST[15:0]; 
    // RG_MEMPLL_TEST[5:4]=01,RG_MEMPLL_TEST[3]=1;RG_MEMPLL_TEST[1:0]=11
    // RG_MEMPLL_TEST[5:4]=01 -> MEMPLL2
    // RG_MEMPLL_TEST[5:4]=10 -> MEMPLL3
    // RG_MEMPLL_TEST[5:4]=11 -> MEMPLL4
    // RG_MEMPLL_TEST[3,1] -> select CKMUX (measure clock or voltage)
    // RG_MEMPLL_TEST[0] -> RG_A2DCK_EN (for FT)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x60c), &u4value);
    mcSET_BIT(u4value, 6);
    mcSET_BIT(u4value, 7);
    mcSET_FIELD(u4value, 3, 0x00000300, 8);
    mcSET_BIT(u4value, 11);
    mcSET_FIELD(u4value, 3, 0x00003000, 12);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x60c), u4value);

    (*(volatile unsigned int *)(0x1000F638)) = ((*(volatile unsigned int *)(0x1000F638)) & 0xf8ffffff) | 0x03000000;
    mcSHOW_DBG_MSG(("mt_get_mem_freq()=%d\n", mt_get_mem_freq()));


/*
    //0x638[26:24] RG_MEMPLL_TEST_DIV2_SEL=011, /8 (for FT)
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x638), &u4value);
    mcSET_FIELD(u4value, 3, 0x07000000, 24);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x638), u4value);*/
#endif

#if 0
    // for debug
    u4value = DramOperateDataRate(p);
    mcSHOW_DBG_MSG(("DRAM Operation Data Rate: DDR-%d\n", u4value));
#endif
            
    if (ucstatus)
    {
        return DRAM_FAIL;
    }
    else
    {
        return DRAM_OK;
    }    
}

#endif

#endif

U32 DramOperateDataRate(DRAMC_CTX_T *p)
{
    U32 u4value1, u4value2, MPLL_POSDIV, MPLL_PCW, MPLL_FOUT;
    U32 MEMPLL_FBKDIV, MEMPLL_M4PDIV, MEMPLL_FOUT;
    U8 ucstatus = 0;
    
    u4value1 = (*(volatile unsigned int *)(0x10209280));
    u4value2 = mcGET_FIELD(u4value1, 0x00000070, 4);
    if (u4value2 == 0)
    {
        MPLL_POSDIV = 1;
    }
    else if (u4value2 == 1)
    {
        MPLL_POSDIV = 2;
    }
    else if (u4value2 == 2)
    {
        MPLL_POSDIV = 4;
    }
    else if (u4value2 == 3)
    {
        MPLL_POSDIV = 8;
    }
    else
    {
        MPLL_POSDIV = 16;
    }

    u4value1 = (*(volatile unsigned int *)(0x10209284));
    MPLL_PCW = mcGET_FIELD(u4value1, 0x001fffff, 0);

    MPLL_FOUT = 26/1*MPLL_PCW;
    MPLL_FOUT = Round_Operation(MPLL_FOUT, MPLL_POSDIV*28); // freq*16384

    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x614), &u4value1);
    MEMPLL_FBKDIV = mcGET_FIELD(u4value1, 0x007f0000, 16);

    MEMPLL_FOUT = MPLL_FOUT*1*4*(MEMPLL_FBKDIV+1);
    MEMPLL_FOUT = Round_Operation(MEMPLL_FOUT, 16384);

    //mcSHOW_DBG_MSG(("MPLL_POSDIV=%d, MPLL_PCW=0x%x, MPLL_FOUT=%d, MEMPLL_FBKDIV=%d, MEMPLL_FOUT=%d\n", MPLL_POSDIV, MPLL_PCW, MPLL_FOUT, MEMPLL_FBKDIV, MEMPLL_FOUT));

    return MEMPLL_FOUT;
}

#if fcFOR_CHIP_ID == fcK2

//#if defined(DUAL_FREQ_DIFF_ACTIMING) || defined(DUAL_FREQ_DIFF_RLWL)
#if 1
// The following routine should be enabled after bring-up.
// Add dual frequency AC timing, RL/WL and selph for different MR2 (RL/WL)
//-------------------------------------------------------------------------
/** DramcInit
 *  DRAMC Initialization.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 u4value;
#ifdef COMBO_MCP
    EMI_SETTINGS *emi_set;
    
    if(emi_setting_index == -1)
        emi_set = default_emi_setting;
    else
        emi_set = &emi_settings[emi_setting_index];   
#endif
    
	// error handling
	if (!p)
	{
		mcSHOW_ERR_MSG(("context is NULL\n"));
		return DRAM_FAIL;
	}

#ifdef FRCLK_26M_DIV8
	 *(volatile unsigned *)(0x10001088) |= 0x80000000;	// MODULE_SW_CG_1_SET[31] = 1 ==> dramc_f26m_cg_set
	 *(volatile unsigned *)(0x10001098) |= (0x01 <<7);		// dramc_f26m_sel : 0-div 1, 1-div8
	 *(volatile unsigned *)(0x1000108C) |= 0x80000000;	// MODULE_SW_CG_1_CLR[31] = 1 ==> dramc_f26m_cg_set
#endif

	u4value = *(volatile unsigned *)(EMI_APB_BASE+0x0);

	// EMI_CONA[17]
	if (u4value & 0x00020000)	
	{
		uiDualRank = 1;
	}
	else
	{
		uiDualRank = 0;
	}		



	if (p->dram_type == TYPE_LPDDR2) 
	{
		// LPDDR2
#if 1
 		RISCWriteDRAM(0x0430, 0x10ff10ff);	// 112716ns
		RISCWriteDRAM(0x0434, 0xffffffff);	// 112740ns
		RISCWriteDRAM(0x0438, 0xffffffff);	// 112770ns
		RISCWriteDRAM(0x043c, 0x0000001f);	// 112800ns
		RISCWriteDRAM(0x0400, 0x11111111);	// 112830ns

		RISCWriteDRAM(0x0404, 0x11312111);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		//RISCWriteDRAM(0x0404, 0x11413111);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		RISCWriteDRAM(0x0408, 0x11111111);	// 112890ns
		RISCWriteDRAM(0x040c, 0x11111111);	// 112920ns

		RISCWriteDRAM(0x0410, 0x55d55555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
		//RISCWriteDRAM(0x0410, 0x54955555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
		RISCWriteDRAM(0x0418, 0x00000327);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
		//RISCWriteDRAM(0x0418, 0x00000432);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
	#ifdef DUAL_FREQ_DIFF_RLWL
		if (p->frequency == 367)
		{
			// WL=3
			RISCWriteDRAM(0x041c, 0x11112222);	// 112980ns
			RISCWriteDRAM(0x0420, 0x11112222);	// 113011ns
			RISCWriteDRAM(0x0424, 0x11112222);	// 113041ns
			RISCWriteDRAM(0x0428, 0xffff5555);	// 113071ns
			RISCWriteDRAM(0x042c, 0x00ff0055);	// 113101ns			
			
		}
		else
		{
			// 533MHz. 1066Mbps. WL=4
			RISCWriteDRAM(0x041c, 0x22222222);	// 112980ns
			RISCWriteDRAM(0x0420, 0x22222222);	// 113011ns
			RISCWriteDRAM(0x0424, 0x22222222);	// 113041ns
			RISCWriteDRAM(0x0428, 0x5555ffff);	// 113071ns
			RISCWriteDRAM(0x042c, 0x005500ff);	// 113101ns			
		}
	#else		
		RISCWriteDRAM(0x041c, 0x22222222);	// 112980ns
		RISCWriteDRAM(0x0420, 0x22222222);	// 113011ns
		RISCWriteDRAM(0x0424, 0x22222222);	// 113041ns
		RISCWriteDRAM(0x0428, 0x5555ffff);	// 113071ns
		RISCWriteDRAM(0x042c, 0x005500ff);	// 113101ns
	#endif	

#else

		RISCWriteDRAM(0x0430, 0x10ff10ff);	// 112716ns
		RISCWriteDRAM(0x0434, 0xffffffff);	// 112740ns
		RISCWriteDRAM(0x0438, 0xffffffff);	// 112770ns
		RISCWriteDRAM(0x043c, 0x0000001f);	// 112800ns
		RISCWriteDRAM(0x0400, 0x00000000);	// 112830ns

		RISCWriteDRAM(0x0404, 0x00302000);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		RISCWriteDRAM(0x0408, 0x00000000);	// 112890ns
		RISCWriteDRAM(0x040c, 0x00000000);	// 112920ns

		RISCWriteDRAM(0x0410, 0x05d55555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
		RISCWriteDRAM(0x0418, 0x00000327);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
#ifdef DUAL_FREQ_DIFF_RLWL
		if (p->frequency == 373)
		{
			// WL=3
			RISCWriteDRAM(0x041c, 0x00001111);	// 112980ns
			RISCWriteDRAM(0x0420, 0x00001111);	// 113011ns
			RISCWriteDRAM(0x0424, 0x00001111);	// 113041ns
			RISCWriteDRAM(0x0428, 0xffff5555);	// 113071ns
			RISCWriteDRAM(0x042c, 0x00ff0055);	// 113101ns			
			
		}
		else
		{
			// 533MHz. 1066Mbps. WL=4
			RISCWriteDRAM(0x041c, 0x11111111);	// 112980ns
			RISCWriteDRAM(0x0420, 0x11111111);	// 113011ns
			RISCWriteDRAM(0x0424, 0x11111111);	// 113041ns
			RISCWriteDRAM(0x0428, 0x5555ffff);	// 113071ns
			RISCWriteDRAM(0x042c, 0x005500ff);	// 113101ns			
		}
#else		
		RISCWriteDRAM(0x041c, 0x11111111);	// 112980ns
		RISCWriteDRAM(0x0420, 0x11111111);	// 113011ns
		RISCWriteDRAM(0x0424, 0x11111111);	// 113041ns
		RISCWriteDRAM(0x0428, 0x5555ffff);	// 113071ns
		RISCWriteDRAM(0x042c, 0x005500ff);	// 113101ns
#endif

#endif

		//RISCWriteDRAM(0x01e0, 0x3601ffff);	// 113131ns
		RISCWriteDRAM(0x01e0, 0x3201ffff);	// 113131ns
		//RISCWriteDRAM(0x01e0, 0x2201ffff);	// 113131ns
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x01f8, LPDDR2_AC_TIME_05T_1f8_LOW);	// new
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x01f8, emi_set->DRAMC_ACTIM05T_VAL);	// new
    #else
			RISCWriteDRAM(0x01f8, LPDDR2_AC_TIME_05T_1f8);	// new
    #endif			
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x01f8, emi_set->DRAMC_ACTIM05T_VAL);	// new
    #else
		RISCWriteDRAM(0x01f8, LPDDR2_AC_TIME_05T_1f8);	// new    
    #endif
#endif		
		RISCWriteDRAM(0x023c, 0x2201ffff);	// 113161ns

		RISCWriteDRAM(0x0640, 0x00020091);	// 113544ns, [0] TBD
		RISCWriteDRAM(0x0640, 0x000200b1);	// 113904ns, [0] TBD 

		RISCWriteDRAM(0x01dc, 0x90006442);	// enable DDRPHY dynamic clk gating
		//RISCWriteDRAM(0x0008, 0x03801600);	  // FR_CLK div 8
#ifdef COMBO_MCP		
		RISCWriteDRAM(0x0008, emi_set->DRAMC_CONF2_VAL & 0xffffff00);		
#else
		RISCWriteDRAM(0x0008, LPDDR2_CONF2_008 & 0xffffff00);		
#endif		

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0048, LPDDR2_TEST2_4_048_LOW);	// 113965ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0048, emi_set->DRAMC_TEST2_4_VAL);	// 113965ns
	#else
			RISCWriteDRAM(0x0048, LPDDR2_TEST2_4_048);	// 113965ns	
	#endif		
		}
#else		
    #ifdef COMBO_MCP		    
		RISCWriteDRAM(0x0048, emi_set->DRAMC_TEST2_4_VAL);	// 113965ns
	#else
		RISCWriteDRAM(0x0048, LPDDR2_TEST2_4_048);	// 113965ns	
	#endif	
#endif		
		RISCWriteDRAM(0x008c, 0x00e00001);	// 113988ns, TX pipe [23:21]
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{	
			if (uiDualRank) 
			{
				RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110_LOW  | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
			}
			else
			{
				RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110_LOW & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
			}

		}
		else
		{
			if (uiDualRank) 
			{
    #ifdef COMBO_MCP
				RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL  | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
    #else
				RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110  | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
    #endif
			}
			else
			{
    #ifdef COMBO_MCP
				RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
    #else
				RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110 & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
    #endif
			}
		}
#else		
		if (uiDualRank) 
		{
    #ifdef COMBO_MCP
			RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
    #else
			RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110 | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
    #endif
		}
		else
		{
   #ifdef COMBO_MCP
			RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
   #else
			RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110 & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
   #endif
		}
#endif		
		RISCWriteDRAM(0x00d8, 0x00100110);	// 114018ns, [31:30] PINMUX TBD
		RISCWriteDRAM(0x00e4, 0x00000001);	// 114048ns, no ODT (different from local), DATLAT3[4], DDR3 RESET[1] for toggle rate
		if (p->frequency > 800)
		{
			// Reg.138[4] tCKEH/tCKEL extend 1T
			RISCWriteDRAM(0x0138, 0x80000c10);	// fix rd_period
	        }
		else	
		{
			RISCWriteDRAM(0x0138, 0x80000c00);	// fix rd_period
		}
		
#ifdef DUAL_FREQ_K
		// Impedance calibration only do once in high frequency. Set in low frequency will overwrite the settings of calibration value.
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif
		{
			RISCWriteDRAM(0x00b8, 0x99709970);	// 114078ns
			RISCWriteDRAM(0x00bc, 0x99709970);	// 114108ns
		}
#else		
		RISCWriteDRAM(0x00b8, 0x99709970);	// 114078ns
		RISCWriteDRAM(0x00bc, 0x99709970);	// 114108ns
#endif		
		RISCWriteDRAM(0x0090, 0x00000000);	// 114139ns
		RISCWriteDRAM(0x0094, 0x80000000);	// 114169ns
		RISCWriteDRAM(0x00dc, 0x83200200);	// 114199ns
		#ifdef DDR_1866
		RISCWriteDRAM(0x00e0, 0x15200200);	// 114229ns, R_DMDQSINCTL[27:24] different from local, TBD
		#else
		RISCWriteDRAM(0x00e0, 0x14200200);	// 114229ns, R_DMDQSINCTL[27:24] different from local, TBD
		#endif
		#ifdef DDR_1866
		RISCWriteDRAM(0x0118, 0x00000005);	// 12584724ns, R_DMR1DQSINCTL[3:0] different from local, TBD
		#else
		RISCWriteDRAM(0x0118, 0x00000004);	// 12584724ns, R_DMR1DQSINCTL[3:0] different from local, TBD
		#endif
		RISCWriteDRAM(0x00f0, 0x02000000);	// 114259ns, DATLAT4[25]
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4_LOW);	// 114289ns, R_DMPHYSYNCM
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 114289ns, R_DMPHYSYNCM
	#else
			RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4);	// 114289ns, R_DMPHYSYNCM	
	#endif		
		}
#else		
    #ifdef COMBO_MCP		    
		RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 114289ns, R_DMPHYSYNCM
	#else
		RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4);	// 114289ns, R_DMPHYSYNCM	
	#endif
#endif		
		RISCWriteDRAM(0x0168, 0x00000000);	// 114319ns
		RISCWriteDRAM(0x0130, 0x30000000);	// 114349ns
		mcDELAY_US(1);

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0004, LPDDR2_CONF1_004_LOW);	// 114439ns
		}
		else
		{
    #ifdef COMBO_MCP
			RISCWriteDRAM(0x0004, emi_set->DRAMC_CONF1_VAL);	// 114439ns
    #else
			RISCWriteDRAM(0x0004, LPDDR2_CONF1_004);	// 114439ns
    #endif
		}
#else
    #ifdef COMBO_MCP
		RISCWriteDRAM(0x0004, emi_set->DRAMC_CONF1_VAL);	// 114439ns
    #else
		RISCWriteDRAM(0x0004, LPDDR2_CONF1_004);	// 114439ns    
    #endif
#endif		
		RISCWriteDRAM(0x0124, 0xc0000011);	// 114469ns, R_DMDQSGDUALP[30], reg_dly_sel_rxdqs_com1[7:4], reg_dly_sel_rxdqs_com0[3:0] different from local, TBD
		RISCWriteDRAM(0x0094, 0x40404040);	// 114500ns
		RISCWriteDRAM(0x01c0, 0x00000000);	// 114530ns, disable DQS calibration

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0000, LPDDR2_ACTIM_000_LOW);	// 114563ns
			RISCWriteDRAM(0x00fc, LPDDR2_MISCTL0_VAL_0fc_LOW);	// 114563ns
		}
		else
		{
    #ifdef COMBO_MCP 
			RISCWriteDRAM(0x0000, emi_set->DRAMC_ACTIM_VAL);	// 114563ns
			RISCWriteDRAM(0x00fc, emi_set->DRAMC_MISCTL0_VAL);	// 114563ns
    #else
			RISCWriteDRAM(0x0000, LPDDR2_ACTIM_000);	// 114563ns
			RISCWriteDRAM(0x00fc, LPDDR2_MISCTL0_VAL_0fc);	// 114563ns
    #endif
		}
#else
    #ifdef COMBO_MCP
		RISCWriteDRAM(0x0000, emi_set->DRAMC_ACTIM_VAL);	// 114563ns
		RISCWriteDRAM(0x00fc, emi_set->DRAMC_MISCTL0_VAL);	// 114563ns
    #else
		RISCWriteDRAM(0x0000, LPDDR2_ACTIM_000);	// 114563ns
		RISCWriteDRAM(0x00fc, LPDDR2_MISCTL0_VAL_0fc);	// 114563ns
    #endif
#endif		
		RISCWriteDRAM(0x01ec, 0x00100001);	// 118899ns, dual rank

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x007c, LPDDR2_DDR2CTL_07C_LOW);	// 114563ns, DATLAT210[6:4]
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x007c, emi_set->DRAMC_DDR2CTL_VAL);	// 114563ns, DATLAT210[6:4]
	#else
			RISCWriteDRAM(0x007c, LPDDR2_DDR2CTL_07C);	// 114563ns, DATLAT210[6:4]	
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x007c, emi_set->DRAMC_DDR2CTL_VAL);	// 114563ns, DATLAT210[6:4]
	#else
		RISCWriteDRAM(0x007c, LPDDR2_DDR2CTL_07C);	// 114563ns, DATLAT210[6:4]	
	#endif	
#endif
		#ifdef DDR_1866
		RISCWriteDRAM(0x0080, 0x00f20ce0);	// 114563ns, R_DMDATLAT_DSEL[12:8]
		#else
		RISCWriteDRAM(0x0080, 0x00f00ae0);	// 114563ns, R_DMDATLAT_DSEL[12:8]
		#endif
		RISCWriteDRAM(0x0028, 0xf1200f01);	// 114590ns

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x01e8, LPDDR2_ACTIM1_1E8_LOW);
		}
		else
		{
    #ifdef COMBO_MCP
			RISCWriteDRAM(0x01e8, emi_set->DRAMC_ACTIM1_VAL);
    #else
			RISCWriteDRAM(0x01e8, LPDDR2_ACTIM1_1E8);
    #endif
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x01e8, emi_set->DRAMC_ACTIM1_VAL);
    #else
		RISCWriteDRAM(0x01e8, LPDDR2_ACTIM1_1E8);
    #endif
#endif		
		RISCWriteDRAM(0x0158, 0x00000000);	// 114620ns
		RISCWriteDRAM(0x0054, 0x00000001);	// 114620ns
          
		RISCWriteDRAM(0x00e4, 0x00000005);	// 114650ns, turn on CKE before initial, different from local simulation, TBD
		mcDELAY_US(200);
		
		RISCWriteDRAM(0x01ec, 0x00100000);	// 118899ns, dual rank

#ifdef COMBO_MCP
		RISCWriteDRAM(0x0088, emi_set->iLPDDR2_MODE_REG_63);	// 116193ns
#else
		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_63);	// 116193ns
#endif		
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116253ns
		mcDELAY_US(10);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 116655ns

#ifdef COMBO_MCP		
		RISCWriteDRAM(0x0088, emi_set->iLPDDR2_MODE_REG_10);	// 116716ns
#else
		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_10);	// 116716ns
#endif		
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116776ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117178ns

		#ifdef LPDDR_MAX_DRIVE
		// MR3, driving stregth
		// for testing, set to max
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00010003);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
		mcDELAY_US(1);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
		#elif defined(LPDDR_MIN_DRIVE)
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00030003);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
		mcDELAY_US(1);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
		#endif

#ifdef COMBO_MCP
		RISCWriteDRAM(0x0088, emi_set->iLPDDR2_MODE_REG_1);	// 117239ns
#else
		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_1);	// 117239ns
#endif
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117299ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117701ns

#ifdef DUAL_FREQ_DIFF_RLWL
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_2_LOW);	// 117762ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0088, emi_set->iLPDDR2_MODE_REG_2);	// 117762ns
	#else
			RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_2);	// 117762ns	
	#endif		
		}
		
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x0088, emi_set->iLPDDR2_MODE_REG_2);	// 117762ns
	#else
		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_2);	// 117762ns	
	#endif	
#endif
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117822ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117822ns

		if (uiDualRank) {
#ifdef DUAL_FREQ_DIFF_ACTIMING
			if (p->frequency == DUAL_FREQ_LOW)
			{		
				RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR2_GDDR3CTL1_0F4_LOW);	// 37270775ns
			}
			else
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x00f4, 0x00100000 | emi_set->DRAMC_GDDR3CTL1_VAL);	// 37270775ns
	#else
				RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR2_GDDR3CTL1_0F4);	// 37270775ns	
	#endif
			}
#else
    #ifdef COMBO_MCP			
			RISCWriteDRAM(0x00f4, 0x00100000 | emi_set->DRAMC_GDDR3CTL1_VAL);	// 37270775ns
	#else
			RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR2_GDDR3CTL1_0F4);	// 37270775ns	
	#endif		
#endif

#ifdef COMBO_MCP			
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR2_MODE_REG_63);	// 37372970ns
#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_63);	// 37372970ns
#endif			
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37388346ns
			mcDELAY_US(10);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37409274ns

#ifdef COMBO_MCP
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR2_MODE_REG_10);	// 37427213ns
#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_10);	// 37427213ns
#endif			
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37444297ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37465226ns

			#ifdef LPDDR_MAX_DRIVE
			// MR3, driving stregth
			// for testing, set to max
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x10010003);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
			mcDELAY_US(1);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
			#elif defined(LPDDR_MIN_DRIVE)
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x10030003);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
			mcDELAY_US(1);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
			#endif

#ifdef COMBO_MCP
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR2_MODE_REG_1);	// 37482737ns
#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_1);	// 37482737ns            
#endif			
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37625819ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37645039ns

#ifdef DUAL_FREQ_DIFF_RLWL
			if (p->frequency == DUAL_FREQ_LOW)			
			{
				RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_2_LOW);	// 37662978ns
			}
			else
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR2_MODE_REG_2);	// 37662978ns
    #else
				RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_2);	// 37662978ns    
    #endif				
			}
#else
    #ifdef COMBO_MCP				
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR2_MODE_REG_2);	// 37662978ns
	#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_2);	// 37662978ns	
	#endif		
#endif			
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37680063ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37700991ns
			
#ifdef DUAL_FREQ_DIFF_ACTIMING
			if (p->frequency == DUAL_FREQ_LOW)
			{	
				RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4_LOW);	// 37806675ns
			}
			else
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 37806675ns
	#else
				RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4);	// 37806675ns	
	#endif			
			}
#else
    #ifdef COMBO_MCP			
			RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 37806675ns
	#else
			RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4);	// 37806675ns	
	#endif		
#endif			
		}

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x01dc, LPDDR2_PD_CTRL_1DC_LOW);
		}
		else
		{
    #ifdef COMBO_MCP 
			RISCWriteDRAM(0x01dc, emi_set->DRAMC_PD_CTRL_VAL);
    #else
			RISCWriteDRAM(0x01dc, LPDDR2_PD_CTRL_1DC);
    #endif
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x01dc, emi_set->DRAMC_PD_CTRL_VAL);
    #else
		RISCWriteDRAM(0x01dc, LPDDR2_PD_CTRL_1DC);
    #endif
#endif
		RISCWriteDRAM(0x00e4, 0x00000001);	// 114650ns, turn on CKE before initial, different from local simulation, TBD
		RISCWriteDRAM(0x01ec, 0x00100001);	// 118899ns, sync rome's esl setting
		RISCWriteDRAM(0x0084, 0x00000a56);	// 118929ns

#ifdef DUAL_FREQ_K
		// This register has no shuffle registers and is used in calibration. So only K in high frequency and use high frequency settings.
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif
		{
			RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
		}
#else
		RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
#endif	
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{	
			RISCWriteDRAM(0x0044, LPDDR2_TEST2_3_044_LOW);	// 119050ns
			RISCWriteDRAM(0x0008, LPDDR2_CONF2_008_LOW);	// 119109ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0044, emi_set->DRAMC_TEST2_3_VAL);	// 119050ns
			RISCWriteDRAM(0x0008, emi_set->DRAMC_CONF2_VAL);	// 119109ns
	#else
			RISCWriteDRAM(0x0044, LPDDR2_TEST2_3_044);	// 119050ns
			RISCWriteDRAM(0x0008, LPDDR2_CONF2_008);	// 119109ns
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x0044, emi_set->DRAMC_TEST2_3_VAL);	// 119050ns
		RISCWriteDRAM(0x0008, emi_set->DRAMC_CONF2_VAL);	// 119109ns
	#else
		RISCWriteDRAM(0x0044, LPDDR2_TEST2_3_044);	// 119050ns
		RISCWriteDRAM(0x0008, LPDDR2_CONF2_008);	// 119109ns
	#endif	
#endif

		RISCWriteDRAM(0x0010, 0x00000000);	// 119139ns
		RISCWriteDRAM(0x0100, 0x00008110);	// 119169ns
		RISCWriteDRAM(0x001c, 0x12121212);	// 119200ns
		RISCWriteDRAM(0x00f8, 0x00000000);	// 119230ns

		#if 0
		// Delay default value.
		// DQS RX input delay
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x018), 0x08080908);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x01c), 0x08080908);		// Need to adjust further
		// DQ input delay
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x210), 0x01010300);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x214), 0x06030002);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x218), 0x01010201);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x21c), 0x03020002);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x220), 0x00010103);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x224), 0x02010201);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x228), 0x02040200);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x22c), 0x02020201);
#endif		
	
	}
	else if (p->dram_type == TYPE_LPDDR3) 
	{
		// LPDDR3
		RISCWriteDRAM(0x0430, 0x10ff10ff);	// 112716ns
		RISCWriteDRAM(0x0434, 0xffffffff);	// 112740ns
		RISCWriteDRAM(0x0438, 0xffffffff);	// 112770ns
		RISCWriteDRAM(0x043c, 0x0000001f);	// 112800ns
		RISCWriteDRAM(0x0400, 0x11111111);	// 112830ns

		#ifdef DDR_800
		  RISCWriteDRAM(0x0404, 0x11312111);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		#else
		  RISCWriteDRAM(0x0404, 0x11413111);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		#endif
		RISCWriteDRAM(0x0408, 0x11111111);	// 112890ns
		RISCWriteDRAM(0x040c, 0x11111111);	// 112920ns

		#ifdef DDR_1866
		RISCWriteDRAM(0x0410, 0x54955555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
		#else
			#ifdef DDR_800
			RISCWriteDRAM(0x0410, 0x55d55555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
			#else
			RISCWriteDRAM(0x0410, 0x54955555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
			#endif		
		#endif
		
		#ifdef DDR_800
		RISCWriteDRAM(0x0418, 0x00000327);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
		#else
		RISCWriteDRAM(0x0418, 0x00000432);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
		#endif

#ifdef DUAL_FREQ_DIFF_RLWL
		if (p->frequency == 367)	// WL=3	
		{
			RISCWriteDRAM(0x041c, 0x11112222);	
			RISCWriteDRAM(0x0420, 0x11112222);	
			RISCWriteDRAM(0x0424, 0x11112222);		
			RISCWriteDRAM(0x0428, 0xffff5555);	
			RISCWriteDRAM(0x042c, 0x00ff0055);					
		}
		else if ((p->frequency == 800) ||(p->frequency == 666))	// WL=6
		{
			RISCWriteDRAM(0x041c, 0x33333333);	
			RISCWriteDRAM(0x0420, 0x33333333);	
			RISCWriteDRAM(0x0424, 0x33333333);	
			RISCWriteDRAM(0x0428, 0x5555ffff);	
			RISCWriteDRAM(0x042c, 0x005500ff);			
		}
		else if (p->frequency == 933)	// WL=8
		{
			RISCWriteDRAM(0x041c, 0x44444444);	
			RISCWriteDRAM(0x0420, 0x44444444);	
			RISCWriteDRAM(0x0424, 0x44444444);	
			RISCWriteDRAM(0x0428, 0x5555ffff);	
			RISCWriteDRAM(0x042c, 0x005500ff);		
		}
#else		
		#ifdef DDR_1866
		RISCWriteDRAM(0x041c, 0x44444444);	// 112980ns
		RISCWriteDRAM(0x0420, 0x44444444);	// 113011ns
		RISCWriteDRAM(0x0424, 0x44444444);	// 113041ns
		#else
		RISCWriteDRAM(0x041c, 0x33333333);	// 112980ns
		RISCWriteDRAM(0x0420, 0x33333333);	// 113011ns
		RISCWriteDRAM(0x0424, 0x33333333);	// 113041ns
		#endif
		RISCWriteDRAM(0x0428, 0x5555ffff);	// 113071ns
		RISCWriteDRAM(0x042c, 0x005500ff);	// 113101ns
		
	
#endif		
		
	
		RISCWriteDRAM(0x01e0, 0x2201ffff);	// 113131ns

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x01f8, LPDDR3_AC_TIME_05T_1f8_LOW);	// new
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x01f8, emi_set->DRAMC_ACTIM05T_VAL);	// new
	#else
			RISCWriteDRAM(0x01f8, LPDDR3_AC_TIME_05T_1f8);	// new	
	#endif		
		}
#else
    #ifdef COMBO_MCP	
		RISCWriteDRAM(0x01f8, emi_set->DRAMC_ACTIM05T_VAL);	// new
	#else
		RISCWriteDRAM(0x01f8, LPDDR3_AC_TIME_05T_1f8);	// new	
	#endif	
#endif		
		RISCWriteDRAM(0x023c, 0x2201ffff);	// 113161ns

		RISCWriteDRAM(0x0640, 0x00020091);	// 113544ns, [0] TBD
		RISCWriteDRAM(0x0640, 0x000200b1);	// 113904ns, [0] TBD

		RISCWriteDRAM(0x01dc, 0x90006442);	// enable DDRPHY dynamic clk gating
		//RISCWriteDRAM(0x0008, 0x03801600);	  // FR_CLK div 8
#ifdef COMBO_MCP		
		RISCWriteDRAM(0x0008, emi_set->DRAMC_CONF2_VAL & 0xffffff00);
#else
		RISCWriteDRAM(0x0008, LPDDR3_CONF2_008 & 0xffffff00);
#endif				
								
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0048, LPDDR3_TEST2_4_048_LOW);	// 113965ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0048, emi_set->DRAMC_TEST2_4_VAL);	// 113965ns
	#else
			RISCWriteDRAM(0x0048, LPDDR3_TEST2_4_048);	// 113965ns	
	#endif			
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x0048, emi_set->DRAMC_TEST2_4_VAL);	// 113965ns
	#else
		RISCWriteDRAM(0x0048, LPDDR3_TEST2_4_048);	// 113965ns	
	#endif	
#endif		
		RISCWriteDRAM(0x008c, 0x00e00001);	// 113988ns, TX pipe [23:21]
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{	
			if (uiDualRank) 
			{
				RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110_LOW  | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
			}
			else
			{
				RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110_LOW & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
			}

		}
		else
		{
			if (uiDualRank) 
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL  | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
	#else
				RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110  | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1	
	#endif			
			}
			else
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
	#else
				RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110 & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1	
	#endif			
			}
		}
#else		
		if (uiDualRank) 
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
	#else
			RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110 | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1	
	#endif		
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0110, emi_set->DRAMC_RKCFG_VAL & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
	#else
			RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110 & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1	
	#endif		
		}
#endif		
		RISCWriteDRAM(0x00d8, 0x00100110);	// 114018ns, [31:30] PINMUX TBD
		RISCWriteDRAM(0x00e4, 0x00000001);	// 114048ns, no ODT (different from local), DATLAT3[4], DDR3 RESET[1] for toggle rate
		if (p->frequency > 800)
		{
			// Reg.138[4] tCKEH/tCKEL extend 1T
			RISCWriteDRAM(0x0138, 0x80000c10);	// fix rd_period
	        }
		else	
		{
			RISCWriteDRAM(0x0138, 0x80000c00);	// fix rd_period
		}
		
#ifdef DUAL_FREQ_K
		// Impedance calibration only do once in high frequency. Set in low frequency will overwrite the settings of calibration value.
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif
		{
			RISCWriteDRAM(0x00b8, 0x99709970);	// 114078ns
			RISCWriteDRAM(0x00bc, 0x99709970);	// 114108ns
		}
#else			
		RISCWriteDRAM(0x00b8, 0x99709970);	// 114078ns
		RISCWriteDRAM(0x00bc, 0x99709970);	// 114108ns
#endif		
		RISCWriteDRAM(0x0090, 0x00000000);	// 114139ns
		RISCWriteDRAM(0x0094, 0x80000000);	// 114169ns
		RISCWriteDRAM(0x00dc, 0x83200200);	// 114199ns
		#ifdef DDR_1866
		RISCWriteDRAM(0x00e0, 0x15200200);	// 114229ns, R_DMDQSINCTL[27:24] different from local, TBD
		#else
		RISCWriteDRAM(0x00e0, 0x14200200);	// 114229ns, R_DMDQSINCTL[27:24] different from local, TBD
		#endif
		#ifdef DDR_1866
		RISCWriteDRAM(0x0118, 0x00000005);	// 12584724ns, R_DMR1DQSINCTL[3:0] different from local, TBD
		#else
		RISCWriteDRAM(0x0118, 0x00000004);	// 12584724ns, R_DMR1DQSINCTL[3:0] different from local, TBD
		#endif
		RISCWriteDRAM(0x00f0, 0x02000000);	// 114259ns, DATLAT4[25]
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{	
			RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4_LOW);	// 114289ns, R_DMPHYSYNCM
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 114289ns, R_DMPHYSYNCM
	#else
			RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4);	// 114289ns, R_DMPHYSYNCM	
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 114289ns, R_DMPHYSYNCM
	#else
		RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4);	// 114289ns, R_DMPHYSYNCM	
	#endif	
#endif		
		RISCWriteDRAM(0x0168, 0x00000000);	// 114319ns
		RISCWriteDRAM(0x0130, 0x30000000);	// 114349ns
		mcDELAY_US(1);
	  
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{	
			RISCWriteDRAM(0x0004, LPDDR3_CONF1_004_LOW);	// 114439ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0004, emi_set->DRAMC_CONF1_VAL);	// 114439ns
	#else
			RISCWriteDRAM(0x0004, LPDDR3_CONF1_004);	// 114439ns	
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x0004, emi_set->DRAMC_CONF1_VAL);	// 114439ns
	#else
		RISCWriteDRAM(0x0004, LPDDR3_CONF1_004);	// 114439ns	
	#endif	
#endif		
		RISCWriteDRAM(0x0124, 0xc0000011);	// 114469ns, R_DMDQSGDUALP[30], reg_dly_sel_rxdqs_com1[7:4], reg_dly_sel_rxdqs_com0[3:0] different from local, TBD
		RISCWriteDRAM(0x0094, 0x40404040);	// 114500ns
		RISCWriteDRAM(0x01c0, 0x00000000);	// 114530ns, disable DQS calibration

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0000, LPDDR3_ACTIM_000_LOW);	// 114563ns
			RISCWriteDRAM(0x00fc, LPDDR3_MISCTL0_VAL_0fc_LOW);	// 114563ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0000, emi_set->DRAMC_ACTIM_VAL);	// 114563ns
			RISCWriteDRAM(0x00fc, emi_set->DRAMC_MISCTL0_VAL);	// 114563ns
	#else
			RISCWriteDRAM(0x0000, LPDDR3_ACTIM_000);	// 114563ns
			RISCWriteDRAM(0x00fc, LPDDR3_MISCTL0_VAL_0fc);	// 114563ns	
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x0000, emi_set->DRAMC_ACTIM_VAL);	// 114563ns
		RISCWriteDRAM(0x00fc, emi_set->DRAMC_MISCTL0_VAL);	// 114563ns
	#else
		RISCWriteDRAM(0x0000, LPDDR3_ACTIM_000);	// 114563ns
		RISCWriteDRAM(0x00fc, LPDDR3_MISCTL0_VAL_0fc);	// 114563ns	
	#endif	
#endif		
		RISCWriteDRAM(0x01ec, 0x00100001);	// 118899ns, dual rank

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x007c, LPDDR3_DDR2CTL_07C_LOW);	// 114563ns, DATLAT210[6:4]
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x007c, emi_set->DRAMC_DDR2CTL_VAL);	// 114563ns, DATLAT210[6:4]
	#else
			RISCWriteDRAM(0x007c, LPDDR3_DDR2CTL_07C);	// 114563ns, DATLAT210[6:4]	
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x007c, emi_set->DRAMC_DDR2CTL_VAL);	// 114563ns, DATLAT210[6:4]
	#else
		RISCWriteDRAM(0x007c, LPDDR3_DDR2CTL_07C);	// 114563ns, DATLAT210[6:4]	
	#endif	
#endif		
		#ifdef DDR_1866
		RISCWriteDRAM(0x0080, 0x00f20ce0);	// 114563ns, R_DMDATLAT_DSEL[12:8]
		#else
		RISCWriteDRAM(0x0080, 0x00f00ae0);	// 114563ns, R_DMDATLAT_DSEL[12:8]
		#endif
		RISCWriteDRAM(0x0028, 0xf1200f01);	// 114590ns

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x01e8, LPDDR3_ACTIM1_1E8_LOW);
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x01e8, emi_set->DRAMC_ACTIM1_VAL);
	#else
			RISCWriteDRAM(0x01e8, LPDDR3_ACTIM1_1E8);	
	#endif		
		}
#else   
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x01e8, emi_set->DRAMC_ACTIM1_VAL);
	#else
		RISCWriteDRAM(0x01e8, LPDDR3_ACTIM1_1E8);	
	#endif	
#endif
		RISCWriteDRAM(0x0158, 0x00000000);	// 114620ns
		RISCWriteDRAM(0x0054, 0x00000001);	// 114620ns

		RISCWriteDRAM(0x00e4, 0x00000005);	// 114650ns, turn on CKE before initial, different from local simulation, TBD
		mcDELAY_US(200);

		RISCWriteDRAM(0x01ec, 0x00100000);	// 118899ns, dual rank

#ifdef COMBO_MCP
		RISCWriteDRAM(0x0088, emi_set->iLPDDR3_MODE_REG_63);	// 116193ns
#else
		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_63);	// 116193ns
#endif		
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116253ns
		mcDELAY_US(10);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 116655ns

#ifdef COMBO_MCP
		RISCWriteDRAM(0x0088, emi_set->iLPDDR3_MODE_REG_10);	// 116716ns
#else
		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_10);	// 116716ns
#endif		
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116776ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117178ns

		#ifdef LPDDR_MAX_DRIVE
		// MR3, driving stregth
		// for testing, set to max
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00010003);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
		mcDELAY_US(1);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
		#elif defined(LPDDR_MIN_DRIVE)
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00030003);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
		mcDELAY_US(1);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
		#endif

#ifdef COMBO_MCP
		RISCWriteDRAM(0x0088, emi_set->iLPDDR3_MODE_REG_1);	// 117239ns
#else
		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_1);	// 117239ns
#endif
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117299ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117701ns

#ifdef DUAL_FREQ_DIFF_RLWL
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_2_LOW);	// 117762ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0088, emi_set->iLPDDR3_MODE_REG_2);	// 117762ns
	#else
			RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_2);	// 117762ns	
	#endif		
		}
#else
    #ifdef COMBO_MCP
		RISCWriteDRAM(0x0088, emi_set->iLPDDR3_MODE_REG_2);	// 117762ns
	#else
		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_2);	// 117762ns	
	#endif	
#endif		
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117822ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117822ns
		
		RISCWriteDRAM(0x0088, 0x0000000b);	// 117762ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117822ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117822ns

		if (uiDualRank) {
#ifdef DUAL_FREQ_DIFF_ACTIMING
			if (p->frequency == DUAL_FREQ_LOW)
			{
				RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR3_GDDR3CTL1_0F4_LOW);	// 37270775ns
			}
			else
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x00f4, 0x00100000 | emi_set->DRAMC_GDDR3CTL1_VAL);	// 37270775ns
	#else
				RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR3_GDDR3CTL1_0F4);	// 37270775ns	
	#endif			
			}
#else
    #ifdef COMBO_MCP		
			RISCWriteDRAM(0x00f4, 0x00100000 | emi_set->DRAMC_GDDR3CTL1_VAL);	// 37270775ns
	#else
			RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR3_GDDR3CTL1_0F4);	// 37270775ns	
	#endif		
#endif

#ifdef COMBO_MCP			
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR3_MODE_REG_63);	// 37372970ns
#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_63);	// 37372970ns
#endif			
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37388346ns
			mcDELAY_US(10);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37409274ns

#ifdef COMBO_MCP
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR3_MODE_REG_10);	// 37427213ns
#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_10);	// 37427213ns
#endif			
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37444297ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37465226ns

			#ifdef LPDDR_MAX_DRIVE
			// MR3, driving stregth
			// for testing, set to max
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x10010003);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
			mcDELAY_US(1);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
			#elif defined(LPDDR_MIN_DRIVE)
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x10030003);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
			mcDELAY_US(1);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
			#endif

#ifdef COMBO_MCP
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR3_MODE_REG_1);	// 37482737ns
#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_1);	// 37482737ns
#endif
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37625819ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37645039ns

#ifdef DUAL_FREQ_DIFF_RLWL
			if (p->frequency == DUAL_FREQ_LOW)
			{
				RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_2_LOW);	// 37662978ns
			}
			else
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR3_MODE_REG_2);	// 37662978ns
	#else
				RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_2);	// 37662978ns	
	#endif			
			}
#else
    #ifdef COMBO_MCP
			RISCWriteDRAM(0x0088, 0x10000000 | emi_set->iLPDDR3_MODE_REG_2);	// 37662978ns
	#else
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_2);	// 37662978ns	
	#endif		
#endif			
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37680063ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37700991ns
			
			RISCWriteDRAM(0x0088, 0x1000000b);	// 37735587ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37752671ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37774027ns

#ifdef DUAL_FREQ_DIFF_ACTIMING
			if (p->frequency == DUAL_FREQ_LOW)
			{
				RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4_LOW);	// 37806675ns
			}
			else
			{
    #ifdef COMBO_MCP			    
				RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 37806675ns
	#else
				RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4);	// 37806675ns	
	#endif			
			}
#else
    #ifdef COMBO_MCP			
			RISCWriteDRAM(0x00f4, emi_set->DRAMC_GDDR3CTL1_VAL);	// 37806675ns
	#else
			RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4);	// 37806675ns	
	#endif		
#endif			
		}

#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x01dc, LPDDR3_PD_CTRL_1DC_LOW);
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x01dc, emi_set->DRAMC_PD_CTRL_VAL);
	#else
			RISCWriteDRAM(0x01dc, LPDDR3_PD_CTRL_1DC);	
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x01dc, emi_set->DRAMC_PD_CTRL_VAL);
    #else
 		RISCWriteDRAM(0x01dc, LPDDR3_PD_CTRL_1DC);   		
	#endif	
#endif
		RISCWriteDRAM(0x00e4, 0x00000001);	// 114650ns, turn on CKE before initial, different from local simulation, TBD
		RISCWriteDRAM(0x01ec, 0x00100001);	// 118899ns, sync rome's esl setting
		RISCWriteDRAM(0x0084, 0x00000a56);	// 118929ns

#ifdef DUAL_FREQ_K
		// This register has no shuffle registers and is used in calibration. So only K in high frequency and use high frequency settings.
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif
		{
			RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
		}
#else		
		RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
#endif	
#ifdef DUAL_FREQ_DIFF_ACTIMING
		if (p->frequency == DUAL_FREQ_LOW)
		{
			RISCWriteDRAM(0x0044, LPDDR3_TEST2_3_044_LOW);	// 119050ns
			RISCWriteDRAM(0x0008, LPDDR3_CONF2_008_LOW);	// 119109ns
		}
		else
		{
    #ifdef COMBO_MCP		    
			RISCWriteDRAM(0x0044, emi_set->DRAMC_TEST2_3_VAL);	// 119050ns
			RISCWriteDRAM(0x0008, emi_set->DRAMC_CONF2_VAL);	// 119109ns
	#else
			RISCWriteDRAM(0x0044, LPDDR3_TEST2_3_044);	// 119050ns
			RISCWriteDRAM(0x0008, LPDDR3_CONF2_008);	// 119109ns	
	#endif		
		}
#else
    #ifdef COMBO_MCP		
		RISCWriteDRAM(0x0044, emi_set->DRAMC_TEST2_3_VAL);	// 119050ns
		RISCWriteDRAM(0x0008, emi_set->DRAMC_CONF2_VAL);	// 119109ns
	#else
		RISCWriteDRAM(0x0044, LPDDR3_TEST2_3_044);	// 119050ns
		RISCWriteDRAM(0x0008, LPDDR3_CONF2_008);	// 119109ns	
	#endif	
#endif		

		RISCWriteDRAM(0x0010, 0x00000000);	// 119139ns
		RISCWriteDRAM(0x0100, 0x00008110);	// 119169ns
		RISCWriteDRAM(0x001c, 0x12121212);	// 119200ns
		RISCWriteDRAM(0x00f8, 0x00000000);	// 119230ns

		#if 0
		// Delay default value.
		// DQS RX input delay
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x018), 0x08080908);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x01c), 0x08080908);		// Need to adjust further
		// DQ input delay
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x210), 0x01010300);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x214), 0x06030002);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x218), 0x01010201);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x21c), 0x03020002);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x220), 0x00010103);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x224), 0x02010201);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x228), 0x02040200);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x22c), 0x02020201);
		#endif
	}
	else
	{
		mcSHOW_ERR_MSG(("unknow dram type  should be lpddr2 or lpddr3 \n"));
		return DRAM_FAIL;
	}

	// K2?? TDN default values?
#ifdef CLKTDN_ENABLE
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0d4), &u4value);
        mcSET_BIT(u4value, 1);	// CLKTDN
        mcSET_BIT(u4value, 31);	// DS3TDN
        mcSET_BIT(u4value, 29);	// DS2TDN
        mcSET_BIT(u4value, 27);	// DS1TDN
        mcSET_BIT(u4value, 25);	// DS0TDN
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0d4), u4value);     
#endif       

	DramcEnterSelfRefresh(p, 1);
#ifdef DUAL_FREQ_K
	// This register has no shuffle registers and is used in calibration. So only K in high frequency and use high frequency settings.
    #ifndef FREQ_ADJUST
	if (p->frequency == DUAL_FREQ_HIGH)
    #else
    if (p->frequency != DUAL_FREQ_LOW)   
    #endif
	{
		// Duty default value.
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x148), 0x10010000);     
	}
#else
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x148), 0x10010000);     
#endif		

	mcDELAY_US(20);	// Wait PLL lock.
	DramcEnterSelfRefresh(p, 0);
	
#ifdef REFTHD_ADJUST_0
	// Clear R_DMREFTHD(dramc AO) 0x8[26:24]=0 for reduce special command (MR4) wait refresh queue time. 
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x008), &u4value);
	u4value &= 0xf8ffffff;
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x008), u4value);     
#endif

#ifdef SEQ_FREERUN
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x138), &u4value);
	u4value |= (0x01 << 7);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x138), u4value);     
#endif

#ifdef DDRCLK_NOGATING
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x1dc), &u4value);
	u4value |= (0x01 << 26);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1dc), u4value);     
#endif

#ifdef TWPST_ENLARGE
    if (p->dram_type == TYPE_LPDDR3)
    {     
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x1ec), &u4value);
	u4value |= (0x01 << 17);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1ec), u4value);
    }         
#endif

#ifdef MAX_DCMDLYREF
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x1dc), &u4value);
	u4value = u4value | 0x00000070;
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1dc), u4value);     
#endif

#ifdef DISABLE_FASTWAKE
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x1e8), &u4value);
        mcCLR_BIT(u4value, 31);	
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e8), u4value);     
#endif

#ifdef MAX_SREF_CK_DLY
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x004), &u4value);
       u4value |= 0xC0000000;	// [31:30] SREF_CK_DLY = 11b.	
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x004), u4value);     
#endif

	// Duty default value.
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x148), 0x10010000);     

  ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0640), 0x000220b1);  //0x1000F640[13] = 1 -> ALL_CLK_ECO_EN = 1    
  
	if (ucstatus)
	{
		mcSHOW_ERR_MSG(("register access fail!\n"));
		return DRAM_FAIL;
	}
	else
	{
		return DRAM_OK;
	}
}

#else

//-------------------------------------------------------------------------
/** DramcInit
 *  DRAMC Initialization.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 u4value;

	// error handling
	if (!p)
	{
		mcSHOW_ERR_MSG(("context is NULL\n"));
		return DRAM_FAIL;
	}

	u4value = *(volatile unsigned *)(EMI_APB_BASE+0x0);

	// EMI_CONA[17]
	if (u4value & 0x00020000)	
	{
		uiDualRank = 1;
	}
	else
	{
		uiDualRank = 0;
	}		


	if (p->dram_type == TYPE_LPDDR2) 
	{
		// LPDDR2
		RISCWriteDRAM(0x0430, 0x10ff10ff);	// 112716ns
		RISCWriteDRAM(0x0434, 0xffffffff);	// 112740ns
		RISCWriteDRAM(0x0438, 0xffffffff);	// 112770ns
		RISCWriteDRAM(0x043c, 0x0000001f);	// 112800ns
		RISCWriteDRAM(0x0400, 0x00000000);	// 112830ns

		RISCWriteDRAM(0x0404, 0x00302000);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		RISCWriteDRAM(0x0408, 0x00000000);	// 112890ns
		RISCWriteDRAM(0x040c, 0x00000000);	// 112920ns

		RISCWriteDRAM(0x0410, 0x05d55555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
		RISCWriteDRAM(0x0418, 0x00000327);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
		RISCWriteDRAM(0x041c, 0x11111111);	// 112980ns
		RISCWriteDRAM(0x0420, 0x11111111);	// 113011ns
		RISCWriteDRAM(0x0424, 0x11111111);	// 113041ns
		RISCWriteDRAM(0x0428, 0x5555ffff);	// 113071ns
		RISCWriteDRAM(0x042c, 0x005500ff);	// 113101ns
		RISCWriteDRAM(0x01e0, 0x3601ffff);	// 113131ns
		RISCWriteDRAM(0x01f8, LPDDR2_AC_TIME_05T_1f8);	// new
		RISCWriteDRAM(0x023c, 0x2201ffff);	// 113161ns

		RISCWriteDRAM(0x0640, 0x00020091);	// 113544ns, [0] TBD
		RISCWriteDRAM(0x0640, 0x000200b1);	// 113904ns, [0] TBD

		RISCWriteDRAM(0x01dc, 0x90002342);	// enable DDRPHY dynamic clk gating
		RISCWriteDRAM(0x0008, 0x30000000);	// new, removed dummy action
		
		RISCWriteDRAM(0x0048, LPDDR2_TEST2_4_048);	// 113965ns
		RISCWriteDRAM(0x008c, 0x00e00001);	// 113988ns, TX pipe [23:21]
		RISCWriteDRAM(0x00d8, 0x00100110);	// 114018ns, [31:30] PINMUX TBD
		RISCWriteDRAM(0x00e4, 0x00002101);	// 114048ns, no ODT (different from local)
		RISCWriteDRAM(0x0138, 0x80000c00);	
		RISCWriteDRAM(0x00b8, 0x99709970);	// 114078ns
		RISCWriteDRAM(0x00bc, 0x99709970);	// 114108ns
		RISCWriteDRAM(0x0090, 0x00000000);	// 114139ns
		RISCWriteDRAM(0x0094, 0x80000000);	// 114169ns
		RISCWriteDRAM(0x00dc, 0x83200200);	// 114199ns
		RISCWriteDRAM(0x00e0, 0x11200200);	// 114229ns, R_DMDQSINCTL[27:24] different from local, TBD
		RISCWriteDRAM(0x0118, 0x00000001);	// 12584724ns
		RISCWriteDRAM(0x00f0, 0x00000000);	// 114259ns
		RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4);	// 114289ns
		RISCWriteDRAM(0x0168, 0x00000000);	// 114319ns
		RISCWriteDRAM(0x0130, 0x30000000);	// 114349ns
		mcDELAY_US(1);

		if (uiDualRank) {
			//change RKMODE setting before CKE high
			RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110 |0x00000001 );	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
		} else {
			RISCWriteDRAM(0x0110, LPDDR2_RKCFG_110 & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
		}

		RISCWriteDRAM(0x0004, LPDDR2_CONF1_004);	// 114439ns
		RISCWriteDRAM(0x0124, 0xc0000000);	// 114469ns, R_DMDQSGDUALP[30], reg_dly_sel_rxdqs_com1[7:4], reg_dly_sel_rxdqs_com0[3:0] different from local, TBD

		RISCWriteDRAM(0x0094, 0x40404040);	// 114500ns

		RISCWriteDRAM(0x01c0, 0x00000000);	// 114530ns, disable DQS calibration
		RISCWriteDRAM(0x007c, LPDDR2_DDR2CTL_07C);	// 114563ns
		RISCWriteDRAM(0x0028, 0xf1200f01);	// 114590ns
		RISCWriteDRAM(0x0158, 0x00000000);	// 114620ns
		RISCWriteDRAM(0x0054, 0x00000001);	// 114620ns
          
		RISCWriteDRAM(0x00e4, 0x00000005);	// 114650ns, turn on CKE before initial, different from local simulation, TBD

		mcDELAY_US(200); //WAIT 200us for tINIT3
		
		RISCWriteDRAM(0x01ec, 0x00100000);	// 118899ns, dual rank

		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_63);	// 116193ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116253ns
		mcDELAY_US(10);	// Should wait at least 10us if not checking DAI.
		RISCWriteDRAM(0x01e4, 0x00000000);	// 116655ns
		
		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_10);	// 116716ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116776ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117178ns

		#ifdef LPDDR_MAX_DRIVE
		// MR3, driving stregth
		// for testing, set to max
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00010003);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
		mcDELAY_US(1);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
		#elif defined(LPDDR_MIN_DRIVE)
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00030003);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
		mcDELAY_US(1);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
		#endif

		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_1);	// 117239ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117299ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117701ns

		RISCWriteDRAM(0x0088, LPDDR2_MODE_REG_2);	// 117762ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117822ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 118224ns

		if (uiDualRank) 
		{
			RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR2_GDDR3CTL1_0F4);	// 37270775ns
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_63);	// 37372970ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37388346ns
			mcDELAY_US(10);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37409274ns

			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_10);	// 37427213ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37444297ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37465226ns

			#ifdef LPDDR_MAX_DRIVE
			// MR3, driving stregth
			// for testing, set to max
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x10010003);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
			mcDELAY_US(1);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
			#elif defined(LPDDR_MIN_DRIVE)
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x10030003);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
			mcDELAY_US(1);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
			#endif

			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_1);	// 37482737ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37625819ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37645039ns
			
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR2_MODE_REG_2);	// 37662978ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37680063ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37700991ns
			
			RISCWriteDRAM(0x0088, 0x1000000b);	// 37735587ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37752671ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37774027ns

			RISCWriteDRAM(0x00f4, LPDDR2_GDDR3CTL1_0F4);	// 37806675ns
		}


		RISCWriteDRAM(0x00e4, 0x00000001);	// 114650ns, turn on CKE before initial, different from local simulation, TBD

		RISCWriteDRAM(0x01ec, 0x00000001);	// 118899ns
		RISCWriteDRAM(0x0084, 0x00000a56);	// 118929ns

#ifdef DUAL_FREQ_K
		// This register has no shuffle registers and is used in calibration. So only K in high frequency and use high frequency settings.
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif
		{
			RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
		}
#else
		RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
#endif	
		RISCWriteDRAM(0x0000, LPDDR2_ACTIM_000);	// 119019ns, TWTR different from local
		RISCWriteDRAM(0x0044, LPDDR2_TEST2_3_044);	// 119050ns
		RISCWriteDRAM(0x01e8, LPDDR2_ACTIM1_1E8);	// 119079ns
		RISCWriteDRAM(0x0008, LPDDR2_CONF2_008);	// 119109ns

		RISCWriteDRAM(0x01dc, LPDDR2_PD_CTRL_1DC);	// twch10231: set [24]R_DMREFFRERUN
		RISCWriteDRAM(0x0010, 0x00000000);	// 119139ns
		RISCWriteDRAM(0x001c, 0x12121212);	// 119200ns
		RISCWriteDRAM(0x00fc, LPDDR2_MISCTL0_VAL_0fc);	// 119260ns
	}
	else if (p->dram_type == TYPE_LPDDR3) 
	{
		// LPDDR3
		RISCWriteDRAM(0x0430, 0x10ff10ff);	// 112716ns
		RISCWriteDRAM(0x0434, 0xffffffff);	// 112740ns
		RISCWriteDRAM(0x0438, 0xffffffff);	// 112770ns
		RISCWriteDRAM(0x043c, 0x0000001f);	// 112800ns
		RISCWriteDRAM(0x0400, 0x11111111);	// 112830ns

		#ifdef DDR_800
		  RISCWriteDRAM(0x0404, 0x11312111);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		#else
		  RISCWriteDRAM(0x0404, 0x11413111);	// 112860ns, [28,24,20] TBD, reg_TX_DLY_DQSgated_P1[22:20], reg_TX_DLY_DQSgated[14:12] different from local
		#endif
		RISCWriteDRAM(0x0408, 0x11111111);	// 112890ns
		RISCWriteDRAM(0x040c, 0x11111111);	// 112920ns

		#ifdef DDR_1866
		RISCWriteDRAM(0x0410, 0x54955555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
		#else
			#ifdef DDR_800
			RISCWriteDRAM(0x0410, 0x55d55555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
			#else
			RISCWriteDRAM(0x0410, 0x54955555);	// 112950ns, reg_dly_DQSgated_P1[25:24] and reg_dly_DQSgated[23:22] different from local
			#endif		
		#endif
		
		#ifdef DDR_800
		RISCWriteDRAM(0x0418, 0x00000327);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
		#else
		RISCWriteDRAM(0x0418, 0x00000432);	// reg_TX_DLY_R1DQSgated_P1[10:8], reg_TX_DLY_R1DQSgated[6:4], reg_dly_R1DQSgated_P1[3:2], and reg_dly_R1DQSgated[1:0] different from local
		#endif
		
		#ifdef DDR_1866
		RISCWriteDRAM(0x041c, 0x44444444);	// 112980ns
		RISCWriteDRAM(0x0420, 0x44444444);	// 113011ns
		RISCWriteDRAM(0x0424, 0x44444444);	// 113041ns
		#else
		RISCWriteDRAM(0x041c, 0x33333333);	// 112980ns
		RISCWriteDRAM(0x0420, 0x33333333);	// 113011ns
		RISCWriteDRAM(0x0424, 0x33333333);	// 113041ns
		#endif
		RISCWriteDRAM(0x0428, 0x5555ffff);	// 113071ns
		RISCWriteDRAM(0x042c, 0x005500ff);	// 113101ns
		RISCWriteDRAM(0x01e0, 0x2201ffff);	// 113131ns
		
		RISCWriteDRAM(0x01f8, LPDDR3_AC_TIME_05T_1f8);	// new
		RISCWriteDRAM(0x023c, 0x2201ffff);	// 113161ns

		RISCWriteDRAM(0x0640, 0x00020091);	// 113544ns, [0] TBD
		RISCWriteDRAM(0x0640, 0x000200b1);	// 113904ns, [0] TBD

		RISCWriteDRAM(0x01dc, 0x90006442);	// enable DDRPHY dynamic clk gating
		RISCWriteDRAM(0x0008, 0x03801600);	  // FR_CLK div 8

		RISCWriteDRAM(0x0048, LPDDR3_TEST2_4_048);	// 113965ns
		RISCWriteDRAM(0x008c, 0x00e00001);	// 113988ns, TX pipe [23:21]
		if (uiDualRank) 
		{
			RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110 | 0x00000001);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
		}
		else
		{
			RISCWriteDRAM(0x0110, LPDDR3_RKCFG_110 & 0xfffffffe);	// dual-rank enable [2:0], rank address [26:24] - cpu_address[27] for rank_1
		}
		RISCWriteDRAM(0x00d8, 0x00100110);	// 114018ns, [31:30] PINMUX TBD
		RISCWriteDRAM(0x00e4, 0x00000001);	// 114048ns, no ODT (different from local), DATLAT3[4], DDR3 RESET[1] for toggle rate
		RISCWriteDRAM(0x0138, 0x80000c00);	// fix rd_period
		RISCWriteDRAM(0x00b8, 0x99709970);	// 114078ns
		RISCWriteDRAM(0x00bc, 0x99709970);	// 114108ns
		RISCWriteDRAM(0x0090, 0x00000000);	// 114139ns
		RISCWriteDRAM(0x0094, 0x80000000);	// 114169ns
		RISCWriteDRAM(0x00dc, 0x83200200);	// 114199ns
		#ifdef DDR_1866
		RISCWriteDRAM(0x00e0, 0x15200200);	// 114229ns, R_DMDQSINCTL[27:24] different from local, TBD
		#else
		RISCWriteDRAM(0x00e0, 0x14200200);	// 114229ns, R_DMDQSINCTL[27:24] different from local, TBD
		#endif
		#ifdef DDR_1866
		RISCWriteDRAM(0x0118, 0x00000005);	// 12584724ns, R_DMR1DQSINCTL[3:0] different from local, TBD
		#else
		RISCWriteDRAM(0x0118, 0x00000004);	// 12584724ns, R_DMR1DQSINCTL[3:0] different from local, TBD
		#endif
		RISCWriteDRAM(0x00f0, 0x02000000);	// 114259ns, DATLAT4[25]
		RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4);	// 114289ns, R_DMPHYSYNCM
		RISCWriteDRAM(0x0168, 0x00000000);	// 114319ns
		RISCWriteDRAM(0x0130, 0x30000000);	// 114349ns
		mcDELAY_US(1);
	  
		RISCWriteDRAM(0x0004, LPDDR3_CONF1_004);	// 114439ns
		RISCWriteDRAM(0x0124, 0xc0000011);	// 114469ns, R_DMDQSGDUALP[30], reg_dly_sel_rxdqs_com1[7:4], reg_dly_sel_rxdqs_com0[3:0] different from local, TBD
		RISCWriteDRAM(0x0094, 0x40404040);	// 114500ns
		RISCWriteDRAM(0x01c0, 0x00000000);	// 114530ns, disable DQS calibration

		RISCWriteDRAM(0x0000, LPDDR3_ACTIM_000);	// 114563ns
		RISCWriteDRAM(0x00fc, LPDDR3_MISCTL0_VAL_0fc);	// 114563ns
		RISCWriteDRAM(0x01ec, 0x00100001);	// 118899ns, dual rank

		RISCWriteDRAM(0x007c, LPDDR3_DDR2CTL_07C);	// 114563ns, DATLAT210[6:4]
		#ifdef DDR_1866
		RISCWriteDRAM(0x0080, 0x00f20ce0);	// 114563ns, R_DMDATLAT_DSEL[12:8]
		#else
		RISCWriteDRAM(0x0080, 0x00f00ae0);	// 114563ns, R_DMDATLAT_DSEL[12:8]
		#endif
		RISCWriteDRAM(0x0028, 0xf1200f01);	// 114590ns

		RISCWriteDRAM(0x01e8, LPDDR3_ACTIM1_1E8);
		RISCWriteDRAM(0x0158, 0x00000000);	// 114620ns
		RISCWriteDRAM(0x0054, 0x00000001);	// 114620ns

		RISCWriteDRAM(0x00e4, 0x00000005);	// 114650ns, turn on CKE before initial, different from local simulation, TBD
		mcDELAY_US(200);

		RISCWriteDRAM(0x01ec, 0x00100000);	// 118899ns, dual rank

		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_63);	// 116193ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116253ns
		mcDELAY_US(10);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 116655ns

		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_10);	// 116716ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 116776ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117178ns

		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_1);	// 117239ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117299ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117701ns
		
		RISCWriteDRAM(0x0088, LPDDR3_MODE_REG_2);	// 117762ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117822ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117822ns
		
		RISCWriteDRAM(0x0088, 0x0000000b);	// 117762ns
		RISCWriteDRAM(0x01e4, 0x00000001);	// 117822ns
		mcDELAY_US(1);
		RISCWriteDRAM(0x01e4, 0x00000000);	// 117822ns

		if (uiDualRank) {
			RISCWriteDRAM(0x00f4, 0x00100000 | LPDDR3_GDDR3CTL1_0F4);	// 37270775ns
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_63);	// 37372970ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37388346ns
			mcDELAY_US(10);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37409274ns

			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_10);	// 37427213ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37444297ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37465226ns

			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_1);	// 37482737ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37625819ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37645039ns
			
			RISCWriteDRAM(0x0088, 0x10000000 | LPDDR3_MODE_REG_2);	// 37662978ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37680063ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37700991ns
			
			RISCWriteDRAM(0x0088, 0x1000000b);	// 37735587ns
			RISCWriteDRAM(0x01e4, 0x00000001);	// 37752671ns
			mcDELAY_US(1);
			RISCWriteDRAM(0x01e4, 0x00000000);	// 37774027ns

			RISCWriteDRAM(0x00f4, LPDDR3_GDDR3CTL1_0F4);	// 37806675ns
		}

		RISCWriteDRAM(0x01dc, LPDDR3_PD_CTRL_1DC);
		RISCWriteDRAM(0x00e4, 0x00000001);	// 114650ns, turn on CKE before initial, different from local simulation, TBD
		RISCWriteDRAM(0x01ec, 0x00100001);	// 118899ns, sync rome's esl setting
		RISCWriteDRAM(0x0084, 0x00000a56);	// 118929ns

#ifdef DUAL_FREQ_K
		// This register has no shuffle registers and is used in calibration. So only K in high frequency and use high frequency settings.
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif
		{
			RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
		}
#else		
		RISCWriteDRAM(0x000c, 0x00000000);	// 118993ns
#endif	
		RISCWriteDRAM(0x0044, LPDDR3_TEST2_3_044);	// 119050ns
		RISCWriteDRAM(0x0008, LPDDR3_CONF2_008);	// 119109ns

		RISCWriteDRAM(0x0010, 0x00000000);	// 119139ns
		RISCWriteDRAM(0x0100, 0x00008110);	// 119169ns
		RISCWriteDRAM(0x001c, 0x12121212);	// 119200ns
		RISCWriteDRAM(0x00f8, 0x00000000);	// 119230ns

		#if 0
		// Delay default value.
		// DQS RX input delay
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x018), 0x08080908);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x01c), 0x08080908);		// Need to adjust further
		// DQ input delay
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x210), 0x01010300);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x214), 0x06030002);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x218), 0x01010201);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x21c), 0x03020002);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x220), 0x00010103);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x224), 0x02010201);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x228), 0x02040200);
		ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x22c), 0x02020201);
		#endif
	}
	else
	{
		mcSHOW_ERR_MSG(("unknow dram type  should be lpddr2 or lpddr3 \n"));
		return DRAM_FAIL;
	}

	// K2?? TDN default values?
#ifdef CLKTDN_ENABLE
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0d4), &u4value);
        mcSET_BIT(u4value, 1);	// CLKTDN
        mcSET_BIT(u4value, 31);	// DS3TDN
        mcSET_BIT(u4value, 29);	// DS2TDN
        mcSET_BIT(u4value, 27);	// DS1TDN
        mcSET_BIT(u4value, 25);	// DS0TDN
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0d4), u4value);     
#endif       

#ifdef DUAL_FREQ_K
	// This register has no shuffle registers and is used in calibration. So only K in high frequency and use high frequency settings.
	// K2?? duty default settings?
    #ifndef FREQ_ADJUST
	if (p->frequency == DUAL_FREQ_HIGH)
    #else
    if (p->frequency != DUAL_FREQ_LOW)   
    #endif
	{
		// Duty default value.
		// ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x148), 0x10010000);     
	}
#else
        //ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x148), 0x10010000);     
#endif		

	if (ucstatus)
	{
		mcSHOW_ERR_MSG(("register access fail!\n"));
		return DRAM_FAIL;
	}
	else
	{
		return DRAM_OK;
	}
}
#endif

#endif


void DramcEnterSelfRefresh(DRAMC_CTX_T *p, U8 op)
{
U8 ucstatus = 0;
U32 uiTemp;

    if (op == 1) // enter self refresh
    {
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), &uiTemp);
        mcSET_BIT(uiTemp, POS_CONF1_SELFREF);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), uiTemp);
        mcDELAY_US(2);
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMDRESP), &uiTemp);
        while ( (mcTEST_BIT(uiTemp, POS_SPCMDRESP_SREF_STATE))==0)
        {
            mcSHOW_DBG_MSG3(("Still not enter self refresh...\n"));
    	    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMDRESP), &uiTemp);
        }
    }
    else // exit self refresh
    {
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), &uiTemp);
        mcCLR_BIT(uiTemp, POS_CONF1_SELFREF);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), uiTemp);
        mcDELAY_US(2);
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMDRESP), &uiTemp);
        while ( (mcTEST_BIT(uiTemp, POS_SPCMDRESP_SREF_STATE))!=0)
        {
            mcSHOW_DBG_MSG3(("Still not exit self refresh...\n"));
    	    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMDRESP), &uiTemp);
        }
    }

    // tREFI/4 may cause self refresh fail. Set to tREFI manual first. After SF, switch back.
//    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x114), uiReg114h);         
}

#ifdef DUAL_FREQ_K

#if fcFOR_CHIP_ID == fcK2

void Reg_Sync_Writel(addr, val)
{
	(*(volatile unsigned int *)(addr)) = val;
	dsb();
}

unsigned int Reg_Readl(addr)
{
	return (*(volatile unsigned int *)(addr));
}

void DramcSwitchFreq(DRAMC_CTX_T *p)
{
	mcSHOW_DBG_MSG(("Switch frequency to %d...\n", p->frequency));
	mcFPRINTF((fp_A60808, "Switch frequency to %d...\n", p->frequency));

	DramcEnterSelfRefresh(p, 1); // enter self refresh
  #ifndef FREQ_ADJUST
    if (p->frequency == DUAL_FREQ_HIGH)
  #else
    if (p->frequency != DUAL_FREQ_LOW)   
  #endif
	{
		// Shuffle to high
		U32 read_data;
		U32 bak_data1;
		U32 bak_data2;
		U32 bak_data3;
		U32 bak_data4;

		bak_data1 = Reg_Readl((CHA_DRAMCAO_BASE + (0x77 << 2)));
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x77 << 2)), bak_data1 & ~(0xc0000000));

		Reg_Sync_Writel((CLK_CFG_0_CLR), 0x300);
		Reg_Sync_Writel((CLK_CFG_0_SET), 0x100);

		bak_data2 = Reg_Readl((CHA_DDRPHY_BASE + (0x190 << 2)));
		bak_data4 = Reg_Readl((PCM_INI_PWRON0_REG));

		// [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00001e41); 

	        Reg_Sync_Writel((PCM_INI_PWRON0_REG), bak_data4 & (~0x8000000));  // K2?? *(volatile unsigned int*)(0x10006000) = 0x0b160001??
		mcDELAY_US(1);		// Wait 1us.

		Reg_Sync_Writel((CLK_CFG_UPDATE), 0x02);

		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 & (~0x01)); 	//sync = 1
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 | 0x01);		// sync back to original. Should be 0. Bu in case of SPM control, need to make sure SPM is not toggling.

		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x77 << 2)), bak_data1);

		Reg_Sync_Writel((MEM_DCM_CTRL), (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x01 << 7) | (0x0 << 6) | (0x1f<<1));
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) |0x01);
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) & (~0x01));
		
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), (0x01 << 28) | (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x0 << 7) | (0x0 << 6) | (0x1f << 1));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) & (~0x01));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) |(0x01 << 31));

		// [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=0 ==> disable output.
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00001e01);

	}
	else
	{
		// Shuffle to low
		U32 read_data;
		U32 bak_data1;
		U32 bak_data2;
		U32 bak_data3;
	
		
		// [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00001e41);
		
	  Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x186 << 2)), Reg_Readl(CHA_DDRPHY_BASE + (0x186 << 2)) | 0x10000);	// Switch MEMPLL2 reset mode select 
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x189 << 2)), Reg_Readl(CHA_DDRPHY_BASE + (0x189 << 2)) | 0x10000);	// Switch MEMPLL3 reset mode select		

		Reg_Sync_Writel((MEM_DCM_CTRL), (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x0 << 7) | (0x0 << 6) | (0x1f << 1));
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) |0x01);
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) & (~0x01));
		
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), (0x01 << 31));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | ((0x01 << 28) | (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x01 << 7) | (0x0 << 6) | (0x1f << 1)));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) &  (~(0x01 << 31)));

		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) & (~0x01));
		
		bak_data1 = Reg_Readl((CHA_DRAMCAO_BASE + (0x20a << 2)));
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x20a << 2)), bak_data1 & (~0xc0000000));

		Reg_Sync_Writel((CLK_CFG_0_CLR), 0x300);
		Reg_Sync_Writel((CLK_CFG_0_SET), 0x200);

		bak_data2 = Reg_Readl((CHA_DDRPHY_BASE + (0x190 << 2)));

#if 0
		// [3:0]=1 : VCO/4 . [4]=1 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1. [15]=1??
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00009e51);		
#else
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00069e41);	// Set 4xck = VCO/4, RG_MEMPLL_RSV[1]=1
		                                                                // Also set MEMPLL2/3 enter reset mode
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00069e51);  // Set CLKSW=1 			
#endif		

		Reg_Sync_Writel((CLK_CFG_UPDATE), 0x02);

		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 & (~0x01));
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 | 0x01);

		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x20a << 2)), bak_data1);

		Reg_Sync_Writel((PCM_INI_PWRON0_REG), Reg_Readl(PCM_INI_PWRON0_REG) |0x8000000);

	}
	DramcEnterSelfRefresh(p, 0); // enter self refresh

	mcSHOW_DBG_MSG(("mt_pll_post_init: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq()));
}

void DramcShuffle(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 u4value;
#ifdef COMBO_MCP
    EMI_SETTINGS *emi_set;
    
    if(emi_setting_index == -1)
        emi_set = default_emi_setting;
    else
        emi_set = &emi_settings[emi_setting_index];  
#endif

	mcSHOW_DBG_MSG(("Switch frequency to %d...\n", p->frequency));
	mcFPRINTF((fp_A60808, "Switch frequency to %d...\n", p->frequency));

  #ifndef FREQ_ADJUST
	if (p->frequency == DUAL_FREQ_HIGH)
  #else
    if (p->frequency != DUAL_FREQ_LOW)   
  #endif
	{
		// Shuffle to high
		U32 read_data;
		U32 bak_data1;
		U32 bak_data2;
		U32 bak_data3;
		U32 bak_data4;

		// MR2 RL/WL set
		#ifdef DUAL_FREQ_DIFF_RLWL
		    #ifdef COMBO_MCP
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? emi_set->iLPDDR3_MODE_REG_2 : emi_set->iLPDDR2_MODE_REG_2);
	        #else
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);	        
	        #endif     
		#else
		    #ifdef COMBO_MCP
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? emi_set->iLPDDR3_MODE_REG_2 : emi_set->iLPDDR2_MODE_REG_2;
	        #else
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);	        
	        #endif    
		#endif

		// Shuffle to high start.
	        // Reg.28h[17]=1 R_DMSHU_DRAMC
	        // Reg.28h[16]=0 R_DMSHU_LOW
		bak_data3 = Reg_Readl((CHA_DRAMCAO_BASE + (0x00a << 2)));
		bak_data3 = (bak_data3 & (~0x00030000)) | 0x20000;
		// Block EMI start.
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3);
		
		// Wait shuffle_end Reg.16ch[0] == 1
	        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        while ((read_data & 0x01)  != 0x01)
	        {
		        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        }

		//block EMI end.
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3 & (~0x20000));
	}
	else
	{
		// Shuffle to low
		U32 read_data;
		U32 bak_data1;
		U32 bak_data2;
		U32 bak_data3;

		// MR2 RL/WL set
		#ifdef DUAL_FREQ_DIFF_RLWL
	        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2_LOW : LPDDR2_MODE_REG_2_LOW);
		#else
		    #ifdef COMBO_MCP
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? emi_set->iLPDDR3_MODE_REG_2 : emi_set->iLPDDR2_MODE_REG_2);
	            #else
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);	        
	            #endif    
		#endif

		// Shuffle to low. block EMI start.
	        // Reg.28h[17]=1 R_DMSHU_DRAMC
	        // Reg.28h[16]=1 R_DMSHU_LOW
		bak_data3 = Reg_Readl((CHA_DRAMCAO_BASE + (0x00a << 2)));
		bak_data3 = bak_data3 | 0x30000;
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3),
		
		// Wait shuffle_end Reg.16ch[0] == 1
	        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        while ((read_data & 0x01)  != 0x01)
	        {
		        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        }

		// Block EMI end.
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3 & (~0x20000));
	}
	mcSHOW_DBG_MSG(("mt_pll_post_init: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq()));
}
	        
void DramcDFS(DRAMC_CTX_T *p, U32 Init)
{
	U8 ucstatus = 0;
	U32 u4value, u4HWTrackR0, u4HWTrackR1, u4HWGatingEnable;
#ifdef TEST_DFS_DMABUSY
	U32 uiCmpErr;
	static U32 uiOffset;
#endif
#ifdef COMBO_MCP
    EMI_SETTINGS *emi_set;
    
    if(emi_setting_index == -1)
        emi_set = default_emi_setting;
    else
        emi_set = &emi_settings[emi_setting_index];  
#endif

	mcSHOW_DBG_MSG(("Switch frequency to %d...\n", p->frequency));
	mcFPRINTF((fp_A60808, "Switch frequency to %d...\n", p->frequency));

	//DramcEnterSelfRefresh(p, 1); // enter self refresh
	//mcDELAY_US(1);
	//Read back HW tracking first. After shuffle finish, need to copy this value into SW fine tune.

	if (Init)
	{
		u4HWGatingEnable = 0;
	}
	else
	{
		if (Reg_Readl((CHA_DRAMCAO_BASE + 0x1c0)) & 0x80000000)
		{
			u4HWGatingEnable = 1;
		}
		else
		{
			u4HWGatingEnable = 0;
		}
	}
	
	if (u4HWGatingEnable)
	{
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + 0x028), Reg_Readl((CHA_DRAMCAO_BASE + 0x028)) & (~(0x01<<30)));     // cha DLLFRZ=0
		u4HWTrackR0 = Reg_Readl((CHA_DRAMCNAO_BASE + 0x374));	// cha r0 
		u4HWTrackR1 = Reg_Readl((CHA_DRAMCNAO_BASE + 0x378));	// cha r1
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + 0x028), Reg_Readl((CHA_DRAMCAO_BASE + 0x028)) |(0x01<<30));     	// cha DLLFRZ=1
	}

  #ifndef FREQ_ADJUST
	if (p->frequency == DUAL_FREQ_HIGH)
  #else
    if (p->frequency != DUAL_FREQ_LOW)   
  #endif
	{
		// Shuffle to high
		U32 read_data;
		U32 bak_data1;
		U32 bak_data2;
		U32 bak_data3;
		U32 bak_data4;

		if (u4HWGatingEnable)
		{
			// Current is low frequency. Save to low frequency fine tune here because shuffle enable will cause HW GW reload.
			Reg_Sync_Writel((CHA_DRAMCAO_BASE + 0x840), u4HWTrackR0);
			Reg_Sync_Writel((CHA_DRAMCAO_BASE + 0x844), u4HWTrackR1);
		}
		
		// MR2 RL/WL set
		#ifdef DUAL_FREQ_DIFF_RLWL
		    #ifdef COMBO_MCP
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? emi_set->iLPDDR3_MODE_REG_2 : emi_set->iLPDDR2_MODE_REG_2);
	            #else
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);	        
	            #endif    
		#else
		    #ifdef COMBO_MCP
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? emi_set->iLPDDR3_MODE_REG_2 : emi_set->iLPDDR2_MODE_REG_2);
	            #else		
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);
	            #endif    
		#endif

		bak_data1 = Reg_Readl((CHA_DRAMCAO_BASE + (0x77 << 2)));
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x77 << 2)), bak_data1 & ~(0xc0000000));		

		Reg_Sync_Writel((CLK_CFG_0_CLR), 0x300);
		Reg_Sync_Writel((CLK_CFG_0_SET), 0x100);

		bak_data2 = Reg_Readl((CHA_DDRPHY_BASE + (0x190 << 2)));
		bak_data4 = Reg_Readl((PCM_INI_PWRON0_REG));
		
		// Shuffle to high start.
	        // Reg.28h[17]=1 R_DMSHU_DRAMC
	        // Reg.28h[16]=0 R_DMSHU_LOW
		bak_data3 = Reg_Readl((CHA_DRAMCAO_BASE + (0x00a << 2)));
		bak_data3 = (bak_data3 & (~0x00030000)) | 0x20000;
#if (TEST_DFS_DMABUSY==1)
		if (!Init)
		{
			if (uiDualRank) 
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x80000004, 0xff000, 8, 0, 1);
			}
			else
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x60000004, 0xff000, 8, 0, 1);
			}
		}
#endif

		// Block EMI start.
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3);
		
#if (TEST_DFS_DMABUSY==2)
		if (!Init)
		{
			if (uiDualRank) 
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x80000004, 0xff000, 8, 0, 1);
			}
			else
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x60000004, 0xff000, 8, 0, 1);
			}
		}
#endif

		// Wait shuffle_end Reg.16ch[0] == 1
	        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        while ((read_data & 0x01)  != 0x01)
	        {
		        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        }

		// [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00001e41); 		
	        Reg_Sync_Writel((PCM_INI_PWRON0_REG), bak_data4 & (~0x8000000));  // K2?? *(volatile unsigned int*)(0x10006000) = 0x0b160001??		
		mcDELAY_US(1);		// Wait 1us.

		Reg_Sync_Writel((CLK_CFG_UPDATE), 0x02);

		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 & (~0x01)); 	//sync = 1
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 | 0x01);		// sync back to original. Should be 0. Bu in case of SPM control, need to make sure SPM is not toggling.
		
		//block EMI end.
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3 & (~0x20000));

#ifdef TEST_DFS_DMABUSY
		if (!Init)
		{
			if (uiDualRank) 
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x80000004, 0xff000, 8, 0x0f, 1);
			}
			else
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x60000004, 0xff000, 8, 0x0f, 1);
			}
			if (uiCmpErr)
			{
				mcSHOW_DBG_MSG(("DMA Fail in DFS... %xh, source=%xh\n", uiCmpErr, 0x50000000+uiOffset));
				while(1);
			}
			else
			{
				mcSHOW_DBG_MSG(("DMA Pass in DFS... %xh, source=%xh\n", uiCmpErr, 0x50000000+uiOffset));
			}	
			if (uiOffset < 0x100)
			{
				uiOffset += 4;
			}
			else
			{
				uiOffset = 0;
			}
		}
#endif	

		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x77 << 2)), bak_data1);

		Reg_Sync_Writel((MEM_DCM_CTRL), (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x01 << 7) | (0x0 << 6) | (0x1f<<1));
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) |0x01);
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) & (~0x01));
		
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), (0x01 << 28) | (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x0 << 7) | (0x0 << 6) | (0x1f << 1));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) & (~0x01));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) |(0x01 << 31));
		
		// [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=0 ==> disable output.
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00001e01);		
	}
	else
	{
		// Shuffle to low
		U32 read_data;
		U32 bak_data1;
		U32 bak_data2;
		U32 bak_data3;

		if (u4HWGatingEnable)
		{
			// Current is low frequency. Save to high frequency fine tune here because shuffle enable will cause HW GW reload.
			Reg_Sync_Writel((CHA_DRAMCAO_BASE + 0x94), u4HWTrackR0);
			Reg_Sync_Writel((CHA_DRAMCAO_BASE + 0x98), u4HWTrackR1);
		}

		// MR2 RL/WL set
		#ifdef DUAL_FREQ_DIFF_RLWL
	        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2_LOW : LPDDR2_MODE_REG_2_LOW);
		#else
		    #ifdef COMBO_MCP
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? emi_set->iLPDDR3_MODE_REG_2 : emi_set->iLPDDR2_MODE_REG_2);
	        #else
	            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), (p->dram_type==TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);   
	        #endif    
		#endif

		// [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00001e41);		

		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x186 << 2)), Reg_Readl(CHA_DDRPHY_BASE + (0x186 << 2)) | 0x10000);	// Switch MEMPLL2 reset mode select 
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x189 << 2)), Reg_Readl(CHA_DDRPHY_BASE + (0x189 << 2)) | 0x10000);	// Switch MEMPLL3 reset mode select		
		
		Reg_Sync_Writel((MEM_DCM_CTRL), (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x0 << 7) | (0x0 << 6) | (0x1f << 1));
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) |0x01);
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) & (~0x01));
		
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), (0x01 << 31));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | ((0x01 << 28) | (0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x01 << 7) | (0x0 << 6) | (0x1f << 1)));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) &  (~(0x01 << 31)));
		
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) & (~0x01));

		bak_data1 = Reg_Readl((CHA_DRAMCAO_BASE + (0x20a << 2)));
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x20a << 2)), bak_data1 & (~0xc0000000));
		
		Reg_Sync_Writel((CLK_CFG_0_CLR), 0x300);
		Reg_Sync_Writel((CLK_CFG_0_SET), 0x200);

		bak_data2 = Reg_Readl((CHA_DDRPHY_BASE + (0x190 << 2)));

		// Shuffle to low.
	        // Reg.28h[17]=1 R_DMSHU_DRAMC
	        // Reg.28h[16]=1 R_DMSHU_LOW
		bak_data3 = Reg_Readl((CHA_DRAMCAO_BASE + (0x00a << 2)));
		bak_data3 = bak_data3 | 0x30000;
#if (TEST_DFS_DMABUSY==1)
		if (!Init)
		{
			if (uiDualRank) 
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x80000004, 0xff000, 8, 0, 1);
			}
			else
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x60000004, 0xff000, 8, 0, 1);
			}
		}
#endif		
		// Block EMI start.
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3);

#if (TEST_DFS_DMABUSY==2)
		if (!Init)
		{
			if (uiDualRank) 
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x80000004, 0xff000, 8, 0, 1);
			}
			else
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x60000004, 0xff000, 8, 0, 1);
			}
		}
#endif
		
		// Wait shuffle_end Reg.16ch[0] == 1
	        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        while ((read_data & 0x01)  != 0x01)
	        {
		        read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
	        }
	        
#if 0
		// [3:0]=1 : VCO/4 . [4]=1 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1. [15]=1??
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00009e51);		
#else
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00069e41);	// Set 4xck = VCO/4, RG_MEMPLL_RSV[1]=1
		                                                                // Also set MEMPLL2/3 enter reset mode
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00069e51);  // Set CLKSW=1 			
#endif

		Reg_Sync_Writel((CLK_CFG_UPDATE), 0x02);

		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 & (~0x01));
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 | 0x01);
		
		// Block EMI end.
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3 & (~0x20000));

#ifdef TEST_DFS_DMABUSY
		if (!Init)
		{
			if (uiDualRank) 
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x80000004, 0xff000, 8, 0x0f, 1);
			}
			else
			{
				uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, 0x50000000+uiOffset, 0x60000004, 0xff000, 8, 0x0f, 1);
			}
			if (uiCmpErr)
			{
				mcSHOW_DBG_MSG(("DMA Fail in DFS... %xh, source=%xh\n", uiCmpErr, 0x50000000+uiOffset));
				while(1);
			}
			else
			{
				mcSHOW_DBG_MSG(("DMA Pass in DFS... %xh, source=%xh\n", uiCmpErr, 0x50000000+uiOffset));
			}	
			if (uiOffset < 0x100)
			{
				uiOffset += 4;
			}
			else
			{
				uiOffset = 0;
			}
		}
#endif	
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x20a << 2)), bak_data1);

		Reg_Sync_Writel((PCM_INI_PWRON0_REG), Reg_Readl(PCM_INI_PWRON0_REG) |0x8000000);
	}

	mcSHOW_DBG_MSG(("mt_pll_post_init: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq()));

	//DramcEnterSelfRefresh(p, 0); // exit self refresh
}

#define FREQREG_SIZE 52

typedef struct _SHUFFLE_REG
{
	U32 uiShuffleAddr;
	U32 uiSourceAddr;
} SHUFFLE_REG;

const SHUFFLE_REG LowFreq_ShuffleReg[FREQREG_SIZE] =
{
	{CHA_DRAMCAO_BASE + 0x800, CHA_DRAMCAO_BASE + 0x000},
	{CHA_DRAMCAO_BASE + 0x804, CHA_DRAMCAO_BASE + 0x004},
	{CHA_DRAMCAO_BASE + 0x808, CHA_DRAMCAO_BASE + 0x008},
	{CHA_DRAMCAO_BASE + 0x80C, CHA_DRAMCAO_BASE + 0x1F0},
	{CHA_DRAMCAO_BASE + 0x810, CHA_DRAMCAO_BASE + 0x1F8},
	{CHA_DRAMCAO_BASE + 0x814, CHA_DRAMCAO_BASE + 0x110},
	{CHA_DRAMCAO_BASE + 0x818, CHA_DRAMCAO_BASE + 0x018},
	{CHA_DRAMCAO_BASE + 0x81C, CHA_DRAMCAO_BASE + 0x01C},
	{CHA_DRAMCAO_BASE + 0x820, CHA_DRAMCAO_BASE + 0x044},
	{CHA_DRAMCAO_BASE + 0x824, CHA_DRAMCAO_BASE + 0x07C},
	{CHA_DRAMCAO_BASE + 0x828, CHA_DRAMCAO_BASE + 0x1DC},
	{CHA_DRAMCAO_BASE + 0x82C, CHA_DRAMCAO_BASE + 0x1E8},
	{CHA_DRAMCAO_BASE + 0x830, CHA_DRAMCAO_BASE + 0x0FC},
	{CHA_DRAMCAO_BASE + 0x834, CHA_DRAMCAO_BASE + 0x08C},
	{CHA_DRAMCAO_BASE + 0x838, CHA_DRAMCAO_BASE + 0x080},
	{CHA_DRAMCAO_BASE + 0x83C, CHA_DRAMCAO_BASE + 0x0E0},
	{CHA_DRAMCAO_BASE + 0x840, CHA_DRAMCAO_BASE + 0x094},
	{CHA_DRAMCAO_BASE + 0x844, CHA_DRAMCAO_BASE + 0x098},
	{CHA_DRAMCAO_BASE + 0x84C, CHA_DRAMCAO_BASE + 0x118},
	{CHA_DRAMCAO_BASE + 0x850, CHA_DRAMCAO_BASE + 0x1C4},
	{CHA_DRAMCAO_BASE + 0x854, CHA_DRAMCAO_BASE + 0x138},
	{CHA_DRAMCAO_BASE + 0x858, CHA_DRAMCAO_BASE + 0x124},
	{CHA_DRAMCAO_BASE + 0x85C, CHA_DRAMCAO_BASE + 0x048},
	{CHA_DRAMCAO_BASE + 0x860, CHA_DRAMCAO_BASE + 0x400},
	{CHA_DRAMCAO_BASE + 0x864, CHA_DRAMCAO_BASE + 0x404},
	{CHA_DRAMCAO_BASE + 0x868, CHA_DRAMCAO_BASE + 0x408},
	{CHA_DRAMCAO_BASE + 0x86C, CHA_DRAMCAO_BASE + 0x40C},
	{CHA_DRAMCAO_BASE + 0x870, CHA_DRAMCAO_BASE + 0x410},
	{CHA_DRAMCAO_BASE + 0x874, CHA_DRAMCAO_BASE + 0x414},
	{CHA_DRAMCAO_BASE + 0x878, CHA_DRAMCAO_BASE + 0x418},
	{CHA_DRAMCAO_BASE + 0x87C, CHA_DRAMCAO_BASE + 0x41C},
	{CHA_DRAMCAO_BASE + 0x880, CHA_DRAMCAO_BASE + 0x420},
	{CHA_DRAMCAO_BASE + 0x884, CHA_DRAMCAO_BASE + 0x424},
	{CHA_DRAMCAO_BASE + 0x888, CHA_DRAMCAO_BASE + 0x428},
	{CHA_DRAMCAO_BASE + 0x88C, CHA_DRAMCAO_BASE + 0x42C},
	{CHA_DRAMCAO_BASE + 0x890, CHA_DRAMCAO_BASE + 0x1C0},
	{CHA_DRAMCAO_BASE + 0x894, CHA_DRAMCAO_BASE + 0x1C8},
	{CHA_DRAMCAO_BASE + 0x8A0, CHA_DRAMCAO_BASE + 0x210},
	{CHA_DRAMCAO_BASE + 0x8A4, CHA_DRAMCAO_BASE + 0x214},
	{CHA_DRAMCAO_BASE + 0x8A8, CHA_DRAMCAO_BASE + 0x218},
	{CHA_DRAMCAO_BASE + 0x8AC, CHA_DRAMCAO_BASE + 0x21C},
	{CHA_DRAMCAO_BASE + 0x8B0, CHA_DRAMCAO_BASE + 0x220},
	{CHA_DRAMCAO_BASE + 0x8B4, CHA_DRAMCAO_BASE + 0x224},
	{CHA_DRAMCAO_BASE + 0x8B8, CHA_DRAMCAO_BASE + 0x228},
	{CHA_DRAMCAO_BASE + 0x8BC, CHA_DRAMCAO_BASE + 0x22C},
	{CHA_DDRPHY_BASE + 0x8D0, CHA_DDRPHY_BASE + 0x200},
	{CHA_DDRPHY_BASE + 0x8D4, CHA_DDRPHY_BASE + 0x204},
	{CHA_DDRPHY_BASE + 0x8D8, CHA_DDRPHY_BASE + 0x208},
	{CHA_DDRPHY_BASE + 0x8DC, CHA_DDRPHY_BASE + 0x20C},
	{CHA_DDRPHY_BASE + 0x8E0, CHA_DDRPHY_BASE + 0x010},
	{CHA_DDRPHY_BASE + 0x8E4, CHA_DDRPHY_BASE + 0x014},
	{CHA_DDRPHY_BASE + 0x8F0, CHA_DDRPHY_BASE + 0x124},
};

static U32 FreqReg_Backup[FREQREG_SIZE];

void DramcSaveToBackup(DRAMC_CTX_T *p)
{
	U32 Offset;
	
        mcSHOW_DBG_MSG(("Save  frequency registers setting into backup. \n"));
	for (Offset = 0; Offset < FREQREG_SIZE; Offset++)
	{
		FreqReg_Backup[Offset] = (*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiSourceAddr));
	}
}

void DramcRestoreBackup(DRAMC_CTX_T *p)
{
	U32 Offset;
	
        mcSHOW_DBG_MSG(("Restore backup to frequency registers. \n"));
	for (Offset = 0; Offset < FREQREG_SIZE; Offset++)
	{
		(*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiSourceAddr)) = FreqReg_Backup[Offset];
	}
}

void DramcSaveToShuffleReg(DRAMC_CTX_T *p)
{
	U32 Offset;
	
        mcSHOW_DBG_MSG(("Save  frequency registers setting into shuffle register. \n"));
	for (Offset = 0; Offset < FREQREG_SIZE; Offset++)
	{
		(*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiShuffleAddr)) = 
			(*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiSourceAddr));
	}
}

/*
void DramcExchangeFreqSetting(DRAMC_CTX_T *p)
{
	U32 Offset, RegVal;

        mcSHOW_DBG_MSG(("Exchange  frequency registers setting with shuffle register. \n"));
	
	for (Offset = 0; Offset < FREQREG_SIZE; Offset++)
	{
		RegVal = (*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiShuffleAddr));
		(*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiShuffleAddr)) = 
			(*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiSourceAddr));
		(*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiSourceAddr)) = RegVal;
	}	
}*/

void DramcDumpFreqSetting(DRAMC_CTX_T *p)
{
	U32 Offset;

        mcSHOW_DBG_MSG(("Dump shuffle registers...\n"));
        mcSHOW_DBG_MSG(("Shuffle registers number = %d\n", FREQREG_SIZE));
	
	for (Offset = 0; Offset < FREQREG_SIZE; Offset++)
	{
        	mcSHOW_DBG_MSG(("[Source] Addr %xh = %xh [Shuffle] Addr %xh = %xh\n", 
        		LowFreq_ShuffleReg[Offset].uiSourceAddr, (*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiSourceAddr)),
        		LowFreq_ShuffleReg[Offset].uiShuffleAddr, (*(volatile unsigned int *)(LowFreq_ShuffleReg[Offset].uiShuffleAddr))));
	}
}

#endif  
#endif  // DUAL_FREQ_K

void DramcLowFreqWrite(DRAMC_CTX_T *p)
{
    U8 ucstatus = 0;
    U16 u2freq_orig;
    U32 u4err_value;

    if (p->fglow_freq_write_en == ENABLE)
    {
        u2freq_orig = p->frequency;
        p->frequency = p->frequency_low;
        mcSHOW_DBG_MSG(("Enable low speed write function...\n"));
        mcFPRINTF((fp_A60808, "Enable low speed write function...\n"));
        // we will write data in memory on a low frequency,to make sure the data we write is  right
        // then use engine2 read to do the calibration
        // so ,we will do :
        // 1.change freq 
        // 2. use self test engine2 write to write data ,and check the data is right or not
        // 3.change freq to original value        

        // 1. change freq
        p->channel = CHANNEL_A;
        DramcEnterSelfRefresh(p, 1); // enter self refresh
        mcDELAY_US(1);
        MemPllInit(p);
        mcDELAY_US(1);
        DramcEnterSelfRefresh(p, 0); // exit self refresh

        // double check frequency
        //mcSHOW_DBG_MSG(("Low Speed Write: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq()));
        
        // 2. use self test engine2 to write data (only support AUDIO or XTALK pattern)
        if (p->test_pattern== TEST_AUDIO_PATTERN)
        {
            u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 1, 0, 0, 0);            
        }
        else if (p->test_pattern== TEST_XTALK_PATTERN)
        {
            u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 2, 0, 0, 0);
        }
        else
        {
            mcSHOW_ERR_MSG(("ERROR! Only support AUDIO or XTALK in Low Speed Write and High Speed Read calibration!! \n"));
            u4err_value = 0xffffffff;
        }

        // May error due to gating not calibrate @ low speed
        mcSHOW_DBG_MSG(("Low speed write error value: 0x%x\n", u4err_value));
        mcFPRINTF((fp_A60808, "Low speed write error value: 0x%x\n", u4err_value));

        // do phy reset due to ring counter may be wrong
        DramcPhyReset(p);

        // 3. change to original freq 
        p->frequency = u2freq_orig;
        DramcEnterSelfRefresh(p, 1); // enter self refresh
        mcDELAY_US(1);
        MemPllInit(p);
        mcDELAY_US(1);
        DramcEnterSelfRefresh(p, 0); // exit self refresh

        // double check frequency
        //mcSHOW_DBG_MSG(("High Speed Read: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq()));
    }    
}

void DramcDiv2PhaseSync(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 u4value, u4Backup;
	
    	// Reg.5cch[12]=1 bypass delay chain, Reg.5cch[8]=1 from Reg.640h[5]. 
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x5cc), &u4value);
    	u4Backup = u4value;
	mcSET_BIT(u4value, 8);
	mcSET_BIT(u4value, 12);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x5cc), u4value);
    	
	RISCWriteDRAM(0x0640, 0x00020091);		// Reg.640h[5] = 0 invert 1. Sync =1.
	RISCWriteDRAM(0x0640, 0x000200b1);	 	// Reg.640h[5] = 1 invert 0. Sync =0.

	// Restore.
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x5cc), u4Backup);
}

void DramcPhyReset(DRAMC_CTX_T *p)
{
    U8 ucstatus = 0;//, ucref_cnt;
    U32 u4value;

    // reset phy
    // 0x0f0[28] = 1 -> 0
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), &u4value);
    mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

    // read data counter reset
    // 0x0f4[25] = 1 -> 0
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), &u4value);
    mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);

    //delay 10ns, need to change when porting
    mcDELAY_US(1);  

    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), &u4value);
    mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);    

    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), &u4value);
    mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);

}

void DramcHWGWEnable(DRAMC_CTX_T *p)
{
    U8 ucstatus = 0;
    U32 u4value;  
    
#ifdef HW_GATING    
    // 0x1c0[31]
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQSCAL0), &u4value);
    mcSET_BIT(u4value, POS_DQSCAL0_STBCALEN);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQSCAL0), u4value);

    // Shuffle register 0x890[31]
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x890), &u4value);
    mcSET_BIT(u4value, 31);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x890), u4value);    
#endif   

#ifdef CKE_FIXON
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0xe4), &u4value);
	u4value |= (0x01 << 2);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0xe4), u4value);     
#endif

#ifdef EMI_OPTIMIZATION
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x1ec), &u4value);
    #ifdef RWSPLIT_ENABLE
    u4value |= 0x0008cf11;
    #else
    u4value |= 0x00084f11;
    #endif
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1ec), u4value);  
#endif    

#ifdef ZQCS_ENABLE
    if (p->dram_type == TYPE_LPDDR3)
    {
        // Set (0x1e4[23:16] ZQCSCNT)
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x1e4), &u4value);
        mcSET_FIELD(u4value, 0x20, 0x00ff0000, 16);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), u4value);  
    }	
#endif    
}

void DramcRunTimeConfig(DRAMC_CTX_T *p)
{
    U8 ucstatus = 0;
    U32 u4value;  
    U32 u4CoarseTune_MCK;

#ifdef DUAL_RANKS
    if (uiDualRank) 
    {
    	// RANKINCTL_ROOT1 = DQSINCTL+reg_TX_DLY_DQSGATE (min of RK0 and RK1).
	if (opt_gw_coarse_value_R0[p->channel] < opt_gw_coarse_value_R1[p->channel])
	{
		u4CoarseTune_MCK = opt_gw_coarse_value_R0[p->channel] >> 2;
	}
	else
	{
		u4CoarseTune_MCK = opt_gw_coarse_value_R1[p->channel] >> 2;
	}
	
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x138), &u4value);
        mcSET_FIELD(u4value, u4CoarseTune_MCK, 0x0f, 0);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x138), u4value);         

	// RANKINCTL = RANKINCTL_ROOT1+0
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x1c4), &u4value);
	//u4CoarseTune_MCK += 2;
	u4CoarseTune_MCK += 0;
        mcSET_FIELD(u4value, u4CoarseTune_MCK, 0x000f0000, 16);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1c4), u4value);         
    }
#endif

#ifdef TEMP_SENSOR_ENABLE
    if ((p->dram_type == TYPE_LPDDR3) || (p->dram_type == TYPE_LPDDR2))
    {
        // Enable rum time refresh rate auto update
        // important!!

	// The following derating AC timings need to be modified according to different parts AC timings + 1.875ns + 1 DRAMC clk
        unsigned int TRRD_DERATE = 0x05 <<28;
	unsigned int TRPAB_DERATE = 0x02 << 24;
	unsigned int  TRP_DERATE = 0x09 << 20;
	unsigned int  TRAS_DERATE = 0x0d <<16;
	unsigned int  TRC_DERATE = 0x17 << 8;
	unsigned int  TRCD_DERATE = 0x09 <<4;

	/* setup derating AC timing & enable */
	 ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1f0), 
	 	TRRD_DERATE | TRPAB_DERATE | TRP_DERATE | TRAS_DERATE | TRC_DERATE | TRCD_DERATE | 0x01);            

        // set DQ bit 0, 1, 2 pinmux
        if (p->channel == CHANNEL_A)
        {
            if(p->dram_type == TYPE_LPDDR3)
            {
                // refer to CA training pinmux array for LPDDR3
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), &u4value);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[0], MASK_RRRATE_CTL_BIT0_SEL, POS_RRRATE_CTL_BIT0_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[1], MASK_RRRATE_CTL_BIT1_SEL, POS_RRRATE_CTL_BIT1_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[2], MASK_RRRATE_CTL_BIT2_SEL, POS_RRRATE_CTL_BIT2_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[3], MASK_RRRATE_CTL_BIT3_SEL, POS_RRRATE_CTL_BIT3_SEL);
                ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), u4value);         
            }
            else
            {
                // refer to CA training pinmux array
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), &u4value);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[0], MASK_RRRATE_CTL_BIT0_SEL, POS_RRRATE_CTL_BIT0_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[1], MASK_RRRATE_CTL_BIT1_SEL, POS_RRRATE_CTL_BIT1_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[2], MASK_RRRATE_CTL_BIT2_SEL, POS_RRRATE_CTL_BIT2_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[3], MASK_RRRATE_CTL_BIT3_SEL, POS_RRRATE_CTL_BIT3_SEL);
                ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), u4value);            
            }
        }
 

        //Set MRSMA to MR4.
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x88), 0x04);

        // set refrcnt
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM1), &u4value);
        mcSET_FIELD(u4value, 0x10, MASK_ACTIM1_REFRCNT, POS_ACTIM1_REFRCNT);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM1), u4value);    

        mcDELAY_MS(1);

        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x03B8), &u4value);

        mcSHOW_ERR_MSG(("MRR(MR4) Reg.3B8h[10:8]=%x\n", (u4value & 0x700)>>8));
    }
#endif

#ifdef tREFI_DIV4_MANUAL
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x114), &u4value);
        mcSET_BIT(u4value, 31);	
        mcSET_FIELD(u4value, 5, 0x70000000, 28);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x114), u4value);     
	
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x03B8), &u4value);
        mcSHOW_ERR_MSG(("MRR(MR4) Reg.3B8h[10:8]=%x\n", (u4value & 0x700)>>8));
#endif

#ifdef tREFI_DIV2_MANUAL
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x114), &u4value);
        mcSET_BIT(u4value, 31);	
        mcSET_FIELD(u4value, 4, 0x70000000, 28);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x114), u4value);     
	
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x03B8), &u4value);
        mcSHOW_ERR_MSG(("MRR(MR4) Reg.3B8h[10:8]=%x\n", (u4value & 0x700)>>8));
#endif

    if (p->dram_type == TYPE_LPDDR3)
    {
    // Disable Per-bank Refresh when refresh rate >= 5 (only for LPDDR3)
        // Set (0x110[6])
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RKCFG), &u4value);
        mcSET_BIT(u4value, POS_RKCFG_PBREF_DISBYRATE);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RKCFG), u4value);  
    }   
}

#ifdef SPM_CONTROL_AFTERK

#if fcFOR_CHIP_ID == fcK2

void TransferToSPMControl(void)
{
	// K2?? : need to check 5c8h, 5cch detailed definition.
	//(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5c8))) = 0x00001010;
	//(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5cc))) = 0x40000000;
	
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5c8))) = 0x1010F030;
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5cc))) = 0x50101010;
}

void TransferToRegControl(void)
{
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5c8))) = 0x1111ff11;
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5cc))) = 0xd1111111;
}


#endif

#endif

//-------------------------------------------------------------------------
/** DramcEngine1
 *  start the self test engine inside dramc to test dram w/r.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  test2_0         (U32): 16bits,set pattern1 [31:24] and set pattern0 [23:16].
 *  @param  test2_1         (U32): 28bits,base address[27:0].
 *  @param  test2_2         (U32): 28bits,offset address[27:0].
 *  @param  loopforever     (S16):  0 read\write one time ,then exit
 *                                 >0 enable eingie1, after "loopforever" second ,write log and exit
 *                                 -1 loop forever to read\write, every "period" seconds ,check result ,only when we find error,write log and exit
 *                                 -2 loop forever to read\write, every "period" seconds ,write log ,only when we find error,write log and exit
 *                                 -3 just enable loop forever ,then exit
 *  @param period           (U8):  it is valid only when loopforever <0; period should greater than 0
 *  @retval status          (U32): return the value of DM_CMP_ERR  ,0  is ok ,others mean  error
 */
//-------------------------------------------------------------------------
U32 DramcEngine1(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, S16 loopforever, U8 period)
{
    // This function may not need to be modified unless test engine-1 design has changed

    U8 ucengine_status;
    U8 ucstatus = 0, ucnumber;
    U32 u4value, u4result = 0xffffffff;
    U8 ucloop_count = 0;

    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return u4result;;
    }

    // This is TA1 limitation
    // offset must be 0x7ff
    if ((test2_2&0x00ffffff) != 0x000007ff)
    {
        mcSHOW_ERR_MSG(("TA1 offset must be 0x7ff!!\n"));
        mcSET_FIELD(test2_2, 0x7ff, 0x00ffffff, 0);
        mcSHOW_DBG_MSG2(("Force test2_2 to 0x%8x\n", test2_2));
    }
        
    // we get the status 
    // loopforever    period    status    mean 
    //     0             x         1       read\write one time ,then exit ,don't write log 
    //    >0             x         2       read\write in a loop,after "loopforever" seconds ,disable it ,return the R\W status  
    //    -1            >0         3       read\write in a loop,every "period" seconds ,check result ,only when we find error,write log and exit 
    //    -2            >0         4       read\write in a loop,every "period" seconds ,write log ,only when we find error,write log and exit
    //    -3             x         5       just enable loop forever , then exit (so we should disable engine1 outside the function)
    if (loopforever == 0)
    {
        ucengine_status = 1;
    }
    else if (loopforever > 0)
    {
        ucengine_status = 2;
    }
    else if (loopforever == -1)
    {
        if (period > 0)
        {
            ucengine_status = 3;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'status' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -2)
    {
        if (period > 0)
        {
            ucengine_status = 4;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'status' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -3)
    {
        ucengine_status = 5;
    }
    else
    {
        mcSHOW_ERR_MSG(("wrong parameter!\n"));
        mcSHOW_ERR_MSG(("loopforever    period    status    mean \n"));
        mcSHOW_ERR_MSG(("      0                x           1         read/write one time ,then exit ,don't write log\n"));
        mcSHOW_ERR_MSG(("    >0                x           2         read/write in a loop,after [loopforever] seconds ,disable it ,return the R/W status\n"));
        mcSHOW_ERR_MSG(("    -1              >0           3         read/write in a loop,every [period] seconds ,check result ,only when we find error,write log and exit\n"));
        mcSHOW_ERR_MSG(("    -2              >0           4         read/write in a loop,every [period] seconds ,write log ,only when we find error,write log and exit\n"));
        mcSHOW_ERR_MSG(("    -3                x           5         just enable loop forever , then exit (so we should disable engine1 outside the function)\n"));
        return u4result;
    }

    // set ADRDECEN=0,address decode not by DRAMC
    //2012/10/03, the same as A60806, for TA&UART b'31=1; for TE b'31=0
    //2013/7/9, for A60808, always set to 1
#if 0
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), &u4value);
    mcCLR_BIT(u4value, POS_LPDDR2_ADRDECEN);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), u4value);
#endif

    // step
    // 1.select loop forever or not
    // 2.set pattern, base address,offset address
    // 3.enable test1 
    // 4.run different code according status
    // 5.diable test1
    // 6.return DM_CMP_ERR

    if (ucengine_status == 4)
    {
        mcSHOW_DBG_MSG(("============================================\n"));
        mcSHOW_DBG_MSG(("enable test egine1 loop forever\n"));        
        mcSHOW_DBG_MSG(("============================================\n"));
        ucnumber = 1;
    }

    // 1.
    if (loopforever != 0)
    {
        // enable infinite loop
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), &u4value);
        mcSET_BIT(u4value, POS_CONF1_TESTLP);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);
    }
    else
    {
        // disable infinite loop
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), &u4value);
        mcCLR_BIT(u4value, POS_CONF1_TESTLP);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);
    }
    // 2.
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_1), test2_1);    
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_2), test2_2);    
    // 3.
    // enable test engine 1 (first write and then read???)
    // disable it before enable ,DM_CMP_ERR may not be 0,because may be loopforever and don't disable it before
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
    mcCLR_BIT(u4value, POS_CONF2_TEST1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
    mcSET_BIT(u4value, POS_CONF2_TEST1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);
    // 4.
    if (ucengine_status == 1)
    {
        // read data compare ready check
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
        // infinite loop??? check DE about the time???
        ucloop_count = 0;
        while(mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0)
        {            
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
            mcDELAY_MS(CMP_CPT_POLLING_PERIOD);
            ucloop_count++;
            if (ucloop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
                mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT polling timeout\n"));
                break;
            }
        }

        // delay 10ns after ready check from DE suggestion (1ms here)
        mcDELAY_MS(1);

        // save  DM_CMP_ERR, 0 is ok ,others are fail,disable test engine 1
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
        u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);
        mcSHOW_DBG_MSG2(("0x3fc = %d\n", u4value));
    }
    else if (ucengine_status == 2)
    {
        // wait "loopforever" seconds
        mcDELAY_MS(loopforever*1000);
        // get result, no need to check read data compare ready???
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
        u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);
    }
    else if (ucengine_status == 3)
    {
        while(1)
        {
            // wait "period" seconds
            mcDELAY_MS(period*1000);
            // get result
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
            u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);
            if (u4result == 0)
            {
                // pass, continue to check
                continue;
            }
            // some bit error
            // write log
            mcSHOW_DBG_MSG(("%d#    CMP_ERR = 0x%8x\n", ucnumber, u4result));
            break;
        }
    }
    else if (ucengine_status == 4)
    {
        while(1)
        {
            // wait "period" seconds
            mcDELAY_MS(period*1000);
            // get result
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
            u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);

            // write log
            mcSHOW_DBG_MSG(("%d#    CMP_ERR = 0x%8x\n", ucnumber, u4result));
            
            if (u4result == 0)
            {
                // pass, continue to check
                continue;
            }
            // some bit error            
            break;
        }
    }
    else if (ucengine_status == 5)
    {
        // loopforever is  enable ahead ,we just exit this function
        return 0;
    }
    else
    {
    }

    // 5. disable engine1
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
    mcCLR_BIT(u4value, POS_CONF2_TEST1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);
    
    // 6. 
    // set ADRDECEN to 1
    //2013/7/9, for A60808, always set to 1
#if 0
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), &u4value);
    mcSET_BIT(u4value, POS_LPDDR2_ADRDECEN);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), u4value);
#endif

    return u4result;
}

//-------------------------------------------------------------------------
/** DramcEngine2
 *  start the self test engine 2 inside dramc to test dram w/r.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  wr              (DRAM_TE_OP_T): TE operation
 *  @param  test2_1         (U32): 28bits,base address[27:0].
 *  @param  test2_2         (U32): 28bits,offset address[27:0]. (unit is 16-byte, i.e: 0x100 is 0x1000).
 *  @param  loopforever     (S16): 0    read\write one time ,then exit
 *                                >0 enable eingie2, after "loopforever" second ,write log and exit
 *                                -1 loop forever to read\write, every "period" seconds ,check result ,only when we find error,write log and exit
 *                                -2 loop forever to read\write, every "period" seconds ,write log ,only when we find error,write log and exit
 *                                -3 just enable loop forever ,then exit
 *  @param period           (U8):  it is valid only when loopforever <0; period should greater than 0
 *  @param log2loopcount    (U8): test loop number of test agent2 loop number =2^(log2loopcount) ,0 one time
 *  @retval status          (U32): return the value of DM_CMP_ERR  ,0  is ok ,others mean  error
 */
//-------------------------------------------------------------------------
U32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U32 test2_1, U32 test2_2, U8 testaudpat, S16 loopforever, U8 period, U8 log2loopcount)
{
    U8 ucengine_status;
    U8 ucstatus = 0, ucloop_count = 0;
    U32 u4value, u4result = 0xffffffff;
    U32 u4log2loopcount = (U32) log2loopcount;

    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return u4result;
    }

    // check loop number validness
//    if ((log2loopcount > 15) || (log2loopcount < 0))		// U8 >=0 always.
    if (log2loopcount > 15)
    {
        mcSHOW_ERR_MSG(("wrong parameter log2loopcount:    log2loopcount just 0 to 15 !\n"));
        return u4result;
    }

    // disable self test engine1 and self test engine2 
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
    mcCLR_MASK(u4value, MASK_CONF2_TE12_ENABLE);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

    // we get the status 
    // loopforever    period    status    mean 
    //     0             x         1       read\write one time ,then exit ,don't write log 
    //    >0             x         2       read\write in a loop,after "loopforever" seconds ,disable it ,return the R\W status  
    //    -1            >0         3       read\write in a loop,every "period" seconds ,check result ,only when we find error,write log and exit 
    //    -2            >0         4       read\write in a loop,every "period" seconds ,write log ,only when we find error,write log and exit
    //    -3             x         5       just enable loop forever , then exit (so we should disable engine1 outside the function)
    if (loopforever == 0)
    {
        ucengine_status = 1;
    }
    else if (loopforever > 0)
    {
        ucengine_status = 2;
    }
    else if (loopforever == -1)
    {
        if (period > 0)
        {
            ucengine_status = 3;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'period' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -2)
    {
        if (period > 0)
        {
            ucengine_status = 4;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'period' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -3)
    {
        if (period > 0)
        {
            ucengine_status = 5;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'period' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else
    {
        mcSHOW_ERR_MSG(("parameter 'loopforever' should be 0 -1 -2 -3 or greater than 0\n"));        
        return u4result;
    }

    // set ADRDECEN=0, address decode not by DRAMC
    //2012/10/03, the same as A60806, for TA&UART b'31=1; for TE b'31=0
    //2013/7/9, for A60808, always set to 1
#ifdef fcFOR_A60806_TEST     
    ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), &u4value);
    mcCLR_BIT(u4value, POS_LPDDR2_ADRDECEN);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), u4value);
#endif

    // 1.set pattern ,base address ,offset address
    // 2.select  ISI pattern or audio pattern or xtalk pattern
    // 3.set loop number
    // 4.enable read or write
    // 5.loop to check DM_CMP_CPT
    // 6.return CMP_ERR
    // currently only implement ucengine_status = 1, others are left for future extension    
    /*if (ucengine_status == 4)
    {
        mcSHOW_DBG_MSG(("============================================\n"));
        mcSHOW_DBG_MSG(("enable test egine2 loop forever\n"));        
        mcSHOW_DBG_MSG(("============================================\n"));        
    }*/
    u4result = 0;
    while(1)
    {
        // 1
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_1), test2_1);    
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_2), test2_2);    

        // 2 & 3
        // (TESTXTALKPAT, TESTAUDPAT) = 00 (ISI), 01 (AUD), 10 (XTALK), 11 (UNKNOWN)
        if (testaudpat == 2)   // xtalk
        {
            // select XTALK pattern
            // set addr 0x044 [7] to 0
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), &u4value);
            mcCLR_BIT(u4value, POS_TEST2_3_TESTAUDPAT);
            mcSET_FIELD(u4value, u4log2loopcount, MASK_TEST2_3_TESTCNT, POS_TEST2_3_TESTCNT);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), u4value);

            // set addr 0x48[16] to 1, TESTXTALKPAT = 1
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), &u4value);
            mcSET_BIT(u4value,POS_TEST2_4_TESTXTALKPAT);
            mcCLR_BIT(u4value, POS_TEST2_4_TESTAUDBITINV);
            mcCLR_BIT(u4value, POS_TEST2_4_TESTAUDMODE);  // for XTALK pattern characteristic, we don' t enable write after read
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), u4value);
        }
        else if (testaudpat == 1)   // audio
        {
            // set AUDINIT=0x11 AUDINC=0x0d AUDBITINV=1 AUDMODE=1
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), &u4value);
            mcSET_FIELD(u4value, 0x00000011, MASK_TEST2_4_TESTAUDINIT, POS_TEST2_4_TESTAUDINIT);
            mcSET_FIELD(u4value, 0x0000000d, MASK_TEST2_4_TESTAUDINC, POS_TEST2_4_TESTAUDINC);
            mcSET_BIT(u4value, POS_TEST2_4_TESTAUDBITINV);
            mcSET_BIT(u4value, POS_TEST2_4_TESTAUDMODE);
            mcCLR_BIT(u4value,POS_TEST2_4_TESTXTALKPAT);     // Edward : This bit needs to be disable in audio. Otherwise will fail.
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), u4value);

            // set addr 0x044 [7] to 1 ,select audio pattern
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), &u4value);
            mcSET_BIT(u4value, POS_TEST2_3_TESTAUDPAT);
            mcSET_FIELD(u4value, u4log2loopcount, MASK_TEST2_3_TESTCNT, POS_TEST2_3_TESTCNT);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), u4value);
        } 
        else   // ISI
        {
            // select ISI pattern
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), &u4value);
            mcCLR_BIT(u4value, POS_TEST2_3_TESTAUDPAT);
            mcSET_FIELD(u4value, u4log2loopcount, MASK_TEST2_3_TESTCNT, POS_TEST2_3_TESTCNT);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), u4value);  

            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), &u4value);
            mcCLR_BIT(u4value,POS_TEST2_4_TESTXTALKPAT);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), u4value);
        }

        // 4
        if (wr == TE_OP_READ_CHECK)
        {
            if ((testaudpat == 1) || (testaudpat == 2))
            {
                //if audio pattern, enable read only (disable write after read), AUDMODE=0x48[15]=0
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), &u4value);
                mcCLR_BIT(u4value, POS_TEST2_4_TESTAUDMODE);
                ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), u4value);
            }
            
            // enable read, 0x008[31:29]
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
            mcSET_FIELD(u4value, (U32) 2, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);            
        }
        else if (wr == TE_OP_WRITE_READ_CHECK)
        {
            // enable write
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
            mcSET_FIELD(u4value, (U32) 4, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

            // read data compare ready check
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
            ucloop_count = 0;
            while(mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0)
            {
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
/*            mcDELAY_US(CMP_CPT_POLLING_PERIOD);
                ucloop_count++;
                if (ucloop_count > MAX_CMP_CPT_WAIT_LOOP)
                {
                    //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT polling timeout: %d\n", ucloop_count));
                #ifndef fcWAVEFORM_MEASURE // for testing, waveform ,measurement
                    break;
                #endif
                }*/
            }
            
            // disable write
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
            mcSET_FIELD(u4value, (U32) 0, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

            // enable read
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
            mcSET_FIELD(u4value, (U32) 2, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);
        }

        // 5
        // read data compare ready check
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);        
        ucloop_count = 0;
        while(mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0)
        {
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
/*            mcDELAY_US(CMP_CPT_POLLING_PERIOD);
            ucloop_count++;
            if (ucloop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
                mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT polling timeout\n"));
            #ifndef fcWAVEFORM_MEASURE // for testing, waveform ,measurement
                break;
            #endif
            }*/
        }

        // delay 10ns after ready check from DE suggestion (1ms here)
        mcDELAY_US(1);
        
        // 6
        // return CMP_ERR, 0 is ok ,others are fail,diable test2w or test2r
        // get result
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CMP_ERR), &u4value);
        // or all result
        u4result |= u4value;
        // disable read
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), &u4value);
        mcCLR_MASK(u4value, MASK_CONF2_TE12_ENABLE);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

        // handle status
        if (ucengine_status == 1)
        {
            // set ADRDECEN to 1
            //2013/7/9, for A60808, always set to 1
        #ifdef fcFOR_A60806_TEST
            ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), &u4value);
            mcSET_BIT(u4value, POS_LPDDR2_ADRDECEN);
            ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), u4value);
        #endif
            break;    
        }
        else if (ucengine_status == 2)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else if (ucengine_status == 3)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else if (ucengine_status == 4)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else if (ucengine_status == 5)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
    }

    return u4result;    
}


#ifdef XTALK_SSO_STRESS
//-------------------------------------------------------------------------
/** DramcWorstPat_mem
 *  Write HFID worst pattern to memory
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param src_addr         (U32): DRAM src address
 *  @retval status          (U32): return the value of ERR_VALUE, 0 is ok, others mean  error
 */
//-------------------------------------------------------------------------
void DramcWorstPat_mem(DRAMC_CTX_T *p, U32 src_addr)
{
    U8 ii, jj, kk, repeat;
    U32 size=src_addr;

    //mcSHOW_DBG_MSG(("START to write worst pattern to memory...\n"));
    
    // XTALK worst pattern
    for (ii =0 ; ii < 76; ii++)
    {
        ucDram_Write(src_addr, u4xtalk_pat[ii]);
        src_addr +=4;
    }
    // SSO worst pattern
    //  sso_bs1A
    //   sso1x_bs ~ sso6x_bs
    for (ii = 1; ii <= 6; ii++)
    {
        repeat = 7;

        for (jj = 0; jj <= repeat; jj++)
        {
            for (kk = 0; kk < ii; kk++)
            {
                // send "1"
                ucDram_Write(src_addr, 0xffffffff);
                src_addr +=4;
            }
            for (kk = 0; kk < ii; kk++)
            {
                // send "0"
                ucDram_Write(src_addr, 0x00000000);
                src_addr +=4;
            } 
        }

        // step_1x
        ucDram_Write(src_addr, 0x00000000);
        src_addr +=4;
    }

    //   sso6x_bs
    repeat = 7;
    for (jj = 0; jj <= repeat; jj++)
    {
        for (kk = 0; kk < 6; kk++)
        {
            // send "1"
            ucDram_Write(src_addr, 0xffffffff);
            src_addr +=4;
        }
        for (kk = 0; kk < 6; kk++)
        {
            // send "0"
            ucDram_Write(src_addr, 0x00000000);
            src_addr +=4;
        } 
    }

    //   step_1y
    ucDram_Write(src_addr, 0xffffffff);
    src_addr +=4;

    //   spb0x
    for (ii =0 ; ii < 64; ii++)
    {
        ucDram_Write(src_addr, u4spb0x[ii]);
        src_addr +=4;
    }

    //  sso_bs2A
    for (ii = 0; ii < 6; ii++)
    {
        repeat = ucsso_bsx[ii][2];

        for (jj = 0; jj <= repeat; jj++)
        {
            for (kk = 0; kk < ucsso_bsx[ii][0]; kk++)
            {
                // send "1"
                ucDram_Write(src_addr, 0xffffffff);
                src_addr +=4;
            }
            for (kk = 0; kk < ucsso_bsx[ii][1]; kk++)
            {
                // send "0"
                ucDram_Write(src_addr, 0x00000000);
                src_addr +=4;
            } 
        }

        // END; step_xx
        for (kk = 0; kk < ucsso_bsx[ii][3]; kk++)
        {
            // send "1"
            ucDram_Write(src_addr, 0xffffffff);
            src_addr +=4;
        }
        for (kk = 0; kk < ucsso_bsx[ii][4]; kk++)
        {
            // send "0"
            ucDram_Write(src_addr, 0x00000000);
            src_addr +=4;
        }                
    }

//    mcSHOW_DBG_MSG(("END to write worst pattern to memory! addr=%xh %d size=%xh %d\n", 
//    	src_addr, src_addr, src_addr-size, src_addr-size));
}

#ifdef NEW_WORST_PAT_64

void DramcWorstPat_mem_64(DRAMC_CTX_T *p, U32 src_addr)
{
#ifdef COMBO_MCP
    EMI_SETTINGS *emi_set;
    
    if(emi_setting_index == -1)
        emi_set = default_emi_setting;
    else
        emi_set = &emi_settings[emi_setting_index];  
#endif
    
	*(volatile unsigned *)(EMI_APB_BASE+0x00000000) &= (~0x01);	// disable dual channel.

	p->channel = CHANNEL_A;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000000) &= (~(0x01<<18));
	DramcWorstPat_mem((DRAMC_CTX_T *)p, (U32)src_addr>>1);

	p->channel = CHANNEL_B;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000000) |= ((0x01<<18));
	DramcWorstPat_mem((DRAMC_CTX_T *)p, (U32)src_addr>>1);

#ifdef COMBO_MCP
    *(volatile unsigned *)(EMI_APB_BASE+0x00000000) = emi_set->EMI_CONA_VAL;
#else
    *(volatile unsigned *)(EMI_APB_BASE+0x00000000) = LPDDR3_EMI_CONA;
#endif    
}

#else

void DramcWorstPat_mem_64(DRAMC_CTX_T *p, U32 src_addr)
{
    U8 ii, jj, kk, repeat;
    U32 size=src_addr;

    //mcSHOW_DBG_MSG(("START to write worst pattern to memory...\n"));
    
    // XTALK worst pattern
    for (ii =0 ; ii < 76*2; ii++)
    {
        ucDram_Write(src_addr, u4xtalk_pat_64[ii]);
        src_addr +=4;
    }
    // SSO worst pattern
    //  sso_bs1A
    //   sso1x_bs ~ sso6x_bs
    for (ii = 1; ii <= 6; ii++)
    {
        repeat = 7;

        for (jj = 0; jj <= repeat; jj++)
        {
            for (kk = 0; kk < ii; kk++)
            {
                // send "1"
                ucDram_Write(src_addr, 0xffffffff);
                src_addr +=4;
                ucDram_Write(src_addr, 0xffffffff);
                src_addr +=4;
            }
            for (kk = 0; kk < ii; kk++)
            {
                // send "0"
                ucDram_Write(src_addr, 0x00000000);
                src_addr +=4;
                ucDram_Write(src_addr, 0x00000000);
                src_addr +=4;
            } 
        }

        // step_1x
        ucDram_Write(src_addr, 0x00000000);
        src_addr +=4;
        ucDram_Write(src_addr, 0x00000000);
        src_addr +=4;
    }

    //   sso6x_bs
    repeat = 7;
    for (jj = 0; jj <= repeat; jj++)
    {
        for (kk = 0; kk < 6; kk++)
        {
            // send "1"
            ucDram_Write(src_addr, 0xffffffff);
            src_addr +=4;
            ucDram_Write(src_addr, 0xffffffff);
            src_addr +=4;        }
        for (kk = 0; kk < 6; kk++)
        {
            // send "0"
            ucDram_Write(src_addr, 0x00000000);
            src_addr +=4;
            ucDram_Write(src_addr, 0x00000000);
            src_addr +=4;
        } 
    }

    //   step_1y
    ucDram_Write(src_addr, 0xffffffff);
    src_addr +=4;
    ucDram_Write(src_addr, 0xffffffff);
    src_addr +=4;
    
    //   spb0x
    for (ii =0 ; ii < 64*2; ii++)
    {
        ucDram_Write(src_addr, u4spb0x_64[ii]);
        src_addr +=4;
    }

    //  sso_bs2A
    for (ii = 0; ii < 6; ii++)
    {
        repeat = ucsso_bsx[ii][2];

        for (jj = 0; jj <= repeat; jj++)
        {
            for (kk = 0; kk < (ucsso_bsx[ii][0]*2); kk++)
            {
                // send "1"
                ucDram_Write(src_addr, 0xffffffff);
                src_addr +=4;
            }
            for (kk = 0; kk < (ucsso_bsx[ii][1]*2); kk++)
            {
                // send "0"
                ucDram_Write(src_addr, 0x00000000);
                src_addr +=4;
            } 
        }

        // END; step_xx
        for (kk = 0; kk < (ucsso_bsx[ii][3]*2); kk++)
        {
            // send "1"
            ucDram_Write(src_addr, 0xffffffff);
            src_addr +=4;
        }
        for (kk = 0; kk < (ucsso_bsx[ii][4]*2); kk++)
        {
            // send "0"
            ucDram_Write(src_addr, 0x00000000);
            src_addr +=4;
        }                
    }

//    mcSHOW_DBG_MSG(("END to write worst pattern to memory! addr=%xh %d size=%xh %d\n", 
//    	src_addr, src_addr, src_addr-size, src_addr-size));
}

#endif

//U32 uiPatLenB = 3584;  // Xtalk + SSO
U32 uiPatLenB;  // Xtalk + SSO

void DramcWorstPat(DRAMC_CTX_T *p, U32 src_addr, U8 ChannelNum)
{

}

U32 DramcDmaEngine(DRAMC_CTX_T *p, DRAM_DMA_OP_T op, U32 src_addr, U32 dst_addr, U32 trans_len, U8 burst_len, U8 check_result, U8 ChannelNum)
{
	int err =  0;
	unsigned int data, uiPatRange;
	int i;
	unsigned int uiCompErr = 0, len;
	unsigned int *src_buffp1;
	unsigned int *dst_buffp1;

	if (ChannelNum==1)
	{
		uiPatLenB = XTALK_SSO_LEN;
	}
	else
	{
		uiPatLenB = XTALK_SSO_LEN << 1;
	}
	
	if (src_addr) 
	{
		src_buffp1 = src_addr;
	} 
	else 
	{
		src_buffp1 = 0x40000000;
	}
	
	if (dst_addr) 
	{
		dst_buffp1 = dst_addr;
	} 
	else 
	{
		dst_buffp1 = 0x50000000;
	}

	len = trans_len;
	
	if (len < uiPatLenB) {
		// Should be larger.
		len = uiPatLenB;
	}
	// Have the DMA length to be the multiple of uiPatternLen.
	len = ((len+uiPatLenB-1)/uiPatLenB) * uiPatLenB;


	if (check_result == 0x0f)
	{
		// Only do comparison.
		goto DMA_COMPARE;
	}

	if (check_result != 0)
	{
	for (i = 0 ; i < (len/sizeof(unsigned int)) ; i++) {
	    *((unsigned int *)dst_buffp1+i) = 0;
	}

		if (ChannelNum==1)
		{
			DramcWorstPat_mem((DRAMC_CTX_T *)p, (U32)src_buffp1-DDR_BASE);
		}
		else
		{
			DramcWorstPat_mem_64((DRAMC_CTX_T *)p, (U32)src_buffp1-DDR_BASE);
		}
		
		for (i = 0 ; i < len ; i+=uiPatLenB) {
			memcpy((void *)src_buffp1 + i, src_buffp1, uiPatLenB);
		}
	}

DMA_INIT:	
	 *((volatile unsigned int *)(0x10212c18)) = (DMA_BURST_LEN & 0x07)  << 16; //BURST_LEN:7-8,R/W
	 *((volatile unsigned int *)(0x10212c1c)) = src_buffp1;
	 *((volatile unsigned int *)(0x10212c20)) = dst_buffp1;
	 *((volatile unsigned int *)(0x10212c24)) = len;
	 dsb();
	 *((volatile unsigned int *)(0x10212c08)) = 0x1; //start dma

	if (check_result == 0)
	{	
		// Not wait and return.
		return uiCompErr;
	}

DMA_COMPARE:
	
	 while(*((volatile unsigned int *)(0x10212c08))) ;

#ifdef WAVEFORM_MEASURE
	 goto DMA_INIT;
#endif

	for (i = 0 ; i < (len/sizeof(unsigned int)) ; i++)
	{
		#ifdef DUMMY_READ
		unsigned int DummyRead = *((volatile unsigned int *)dst_buffp1+i);
		DummyRead = *((volatile unsigned int *)dst_buffp1+i);
		DummyRead = *((volatile unsigned int *)src_buffp1+i);
		DummyRead = *((volatile unsigned int *)src_buffp1+i);
		#endif
		if (*((unsigned int *)dst_buffp1+i) != *((unsigned int *)src_buffp1+i)) 
		{
			err = 1;
	   		uiCompErr |= ((*((unsigned int *)dst_buffp1+i)) ^ (*((unsigned int *)src_buffp1+i)));
			#ifdef DMA_ERROR_DISPLAY
			mcSHOW_DBG_MSG2(("Source %xh = %xh Destination %xh= %xh diff=%xh\n", 
				(unsigned int)((unsigned int *)src_buffp1+i), (*((unsigned int *)src_buffp1+i)), 
				(unsigned int)((unsigned int *)dst_buffp1+i), (*((unsigned int *)dst_buffp1+i)),
				(*((unsigned int *)src_buffp1+i)) ^ (*((unsigned int *)dst_buffp1+i))));
			#endif      	   	
		}
        }
	
	#ifdef DMA_ERROR_STOP
	    if (uiCompErr) {
		mcSHOW_DBG_MSG2(("Enter to continue...\n"));
		while (1);
		/*
		{
			if ( UART_Get_Command() )  {    
				 break;
			 }
		}*/
	    }
	#endif

    return uiCompErr;	 
}


#endif

#ifdef SUSPEND_TEST

void Suspend_Resume(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 u4value;

	mcSHOW_DBG_MSG(("MEMPLL 1 suspend...\n"));

	TransferToRegControl();

#if 1	
	// RG_MEMPLLx_EN = 0, Reg.614h[0], Reg.620h[0], Reg.62ch[0]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0614), &u4value);
	mcCLR_BIT(u4value, 0);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0614), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0620), &u4value);
	mcCLR_BIT(u4value, 0);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0620), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x062c), &u4value);
	mcCLR_BIT(u4value, 0);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x062c), u4value);

	// RG_MEMPLL_EN = 0, Reg.604h[28]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0604), &u4value);
	mcCLR_BIT(u4value, 28);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0604), u4value);

	mcDELAY_US(20);	

	// RG_MEMPLL_EN = 1, Reg.604h[28]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0604), &u4value);
	mcSET_BIT(u4value, 28);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0604), u4value);

	mcDELAY_US(20);

	// RG_MEMPLLx_EN = 0, Reg.614h[0], Reg.620h[0], Reg.62ch[0]
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0614), &u4value);
	mcSET_BIT(u4value, 0);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0614), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x0620), &u4value);
	mcSET_BIT(u4value, 0);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0620), u4value);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x062c), &u4value);
	mcSET_BIT(u4value, 0);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x062c), u4value);
	
	mcDELAY_US(20);

	DramcPhyReset(p);
	DramcDiv2PhaseSync(p);
#endif

	TransferToSPMControl();

	mcSHOW_DBG_MSG(("MEMPLL 1 resume...\n"));
	
}
#endif

#ifdef DDR_FT_LOAD_BOARD
void LoadBoardGpioInit(void)
{
	U32 u4temp;
	
	//Set GPIO mode registers (3bits each)
	u4temp = (*(volatile unsigned int *)(0x10005000 + (0x600)));
	// GPIO0
	mcSET_FIELD(u4temp, 0x0, 0x00000007, 0);
	// GPIO2
	mcSET_FIELD(u4temp, 0x0, 0x000001c0, 6);
	// GPIO3
	mcSET_FIELD(u4temp, 0x0, 0x00000e00, 9);
	// GPIO4
	mcSET_FIELD(u4temp, 0x0, 0x00007000, 12);	
	(*(volatile unsigned int *)(0x10005000 + (0x600))) = u4temp;

	u4temp = (*(volatile unsigned int *)(0x10005000 + (0x610)));
	// GPIO5
	//mcSET_FIELD(u4temp, 0x0, 0x00000007, 0);
	// GPIO6
	mcSET_FIELD(u4temp, 0x0, 0x00000038, 3);
	// GPIO7
	mcSET_FIELD(u4temp, 0x0, 0x000001c0, 6);
	// GPIO8
	mcSET_FIELD(u4temp, 0x0, 0x00000e00, 9);
	// GPIO9
	mcSET_FIELD(u4temp, 0x0, 0x00007000, 12);
	(*(volatile unsigned int *)(0x10005000 + (0x610))) = u4temp;

	// Set GPIO direction
	u4temp = (*(volatile unsigned int *)(0x10005000 + (0x000)));
	mcSET_BIT(u4temp, 0);
	mcSET_BIT(u4temp, 2);
	mcSET_BIT(u4temp, 3);
	mcSET_BIT(u4temp, 4);
	//mcSET_BIT(u4temp, 5);
	mcSET_BIT(u4temp, 6);
	mcSET_BIT(u4temp, 7);
	mcSET_BIT(u4temp, 8);
	mcSET_BIT(u4temp, 9);
	(*(volatile unsigned int *)(0x10005000 + (0x000))) = u4temp;
}
void GpioOutput(U8 gpio_no, U8 low_high)
{
	U32 u4temp;
	
	if(low_high == 0)
	{
		u4temp = (*(volatile unsigned int *)(0x10005000 + (0x400)));
		mcCLR_BIT(u4temp, gpio_no); // GPIO0~15
		(*(volatile unsigned int *)(0x10005000 + (0x400))) = u4temp;
	}
	else
	{
		u4temp = (*(volatile unsigned int *)(0x10005000 + (0x400)));
		mcSET_BIT(u4temp, gpio_no); // GPIO0~15
		(*(volatile unsigned int *)(0x10005000 + (0x400))) = u4temp;
	}
}
void LoadBoardShowResult(U8 step, U8 error_type, U8 channel, U8 complete)
{
	mcSHOW_DBG_MSG(("result: "));
	switch(complete)
	{
		case FLAG_NOT_COMPLETE_OR_FAIL:
			//GpioOutput(7, 0);
			GpioOutput(9, 0);
			mcSHOW_DBG_MSG(("0"));
			break;
		case FLAG_COMPLETE_AND_PASS:
			//GpioOutput(7, 1);
			GpioOutput(9, 1);
			mcSHOW_DBG_MSG(("1"));
			break;
		default:
			break;
	}

	switch(channel)
	{
		case CHANNEL_A:
			GpioOutput(8, 0);
			mcSHOW_DBG_MSG(("0"));
			break;
		case CHANNEL_B:
			GpioOutput(8, 1);
			mcSHOW_DBG_MSG(("1"));
			break;
		default:
			break;
	}

	switch(error_type)
	{
		case FLAG_CALIBRATION_PASS:
			GpioOutput(7, 0);
			GpioOutput(6, 0);
			mcSHOW_DBG_MSG(("00"));
			break;
		case FLAG_WINDOW_TOO_SMALL:
			GpioOutput(7, 0);
			GpioOutput(6, 1);
			mcSHOW_DBG_MSG(("01"));
			break;
		case FLAG_WINDOW_TOO_BIG:
			GpioOutput(7, 1);
			GpioOutput(6, 0);
			mcSHOW_DBG_MSG(("10"));
			break;
		case FLAG_CALIBRATION_FAIL:
			GpioOutput(7, 1);
			GpioOutput(6, 1);
			mcSHOW_DBG_MSG(("11"));
			break;
		default:
			break;
	}

	switch(step)
	{
		case FLAG_PLLPHASE_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 0);
			GpioOutput(2, 0);
			GpioOutput(0, 0);
			mcSHOW_DBG_MSG(("0000"));
			break;
		case FLAG_PLLGPPHASE_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 0);
			GpioOutput(2, 0);
			GpioOutput(0, 1);
			mcSHOW_DBG_MSG(("0001"));
			break;
		case FLAG_IMPEDANCE_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 0);
			GpioOutput(2, 1);
			GpioOutput(0, 0);
			mcSHOW_DBG_MSG(("0010"));
			break;
		case FLAG_CA_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 0);
			GpioOutput(2, 1);
			GpioOutput(0, 1);
			mcSHOW_DBG_MSG(("0011"));
			break;
		case FLAG_WL_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 1);
			GpioOutput(2, 0);
			GpioOutput(0, 0);
			mcSHOW_DBG_MSG(("0100"));
			break;
		case FLAG_GATING_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 1);
			GpioOutput(2, 0);
			GpioOutput(0, 1);
			mcSHOW_DBG_MSG(("0101"));
			break;
		case FLAG_RX_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 1);
			GpioOutput(2, 1);
			GpioOutput(0, 0);
			mcSHOW_DBG_MSG(("0110"));
			break;
		case FLAG_DATLAT_CALIBRATION:
			GpioOutput(4, 0);
			GpioOutput(3, 1);
			GpioOutput(2, 1);
			GpioOutput(0, 1);
			mcSHOW_DBG_MSG(("0111"));
			break;
		case FLAG_TX_CALIBRATION:
			GpioOutput(4, 1);
			GpioOutput(3, 0);
			GpioOutput(2, 0);
			GpioOutput(0, 0);
			mcSHOW_DBG_MSG(("1000"));
			break;
		default:
			break;
	}

	mcSHOW_DBG_MSG(("\n"));
}
#endif


