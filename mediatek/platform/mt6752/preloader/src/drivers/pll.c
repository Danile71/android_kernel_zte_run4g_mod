/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "platform.h"

#include "pll.h"
#include "timer.h"
#include "spm.h"
#include "spm_mtcmos.h"

#include "dramc_pi_api.h"
#include "dramc_common.h"
#include "dramc_register.h"
#include "wdt.h"
#include "emi.h"

#include "../security/inc/sec_devinfo.h"


//int A_Reg3e0=0, A_Reg3e4=0;
//int B_Reg3e0=0, B_Reg3e4=0;
extern DRAMC_CTX_T DramCtx_LPDDR2;
extern DRAMC_CTX_T DramCtx_LPDDR3;
extern DRAMC_CTX_T DramCtx_PCDDR3;

void mt_mempll_init(DRAMC_CTX_T *p)
{
    /*p->channel = CHANNEL_A;
    MemPllInit((DRAMC_CTX_T *) p);

    p->channel = CHANNEL_B;*/

#ifdef DDR_FT_LOAD_BOARD
    LoadBoardGpioInit();
#endif

    MemPllInit((DRAMC_CTX_T *) p);	
    return;
}

void mt_mempll_cali(DRAMC_CTX_T *p)
{
#if fcFOR_CHIP_ID == fcK2
    p->channel = CHANNEL_A;
    DramcPllPhaseCal(p);
#endif

    return;
}

void mt_mempll_pre(void)
{
#ifdef DDR_RESERVE_MODE  
    unsigned int wdt_mode;
    unsigned int wdt_dbg_ctrl;
#endif
    DRAMC_CTX_T *psDramCtx;

#ifdef COMBO_MCP
    int dram_type = mt_get_dram_type();
    
    if(dram_type == TYPE_LPDDR2)
    {
        psDramCtx = &DramCtx_LPDDR2;
        default_emi_setting = &emi_setting_default_lpddr2;        
    }
    else if(dram_type == TYPE_LPDDR3)
    {
        psDramCtx = &DramCtx_LPDDR3;
        default_emi_setting = &emi_setting_default_lpddr3;   
    }
#else
    #ifdef DDRTYPE_LPDDR2
        psDramCtx = &DramCtx_LPDDR2;
    #endif

    #ifdef DDRTYPE_LPDDR3
        psDramCtx = &DramCtx_LPDDR3;
    #endif

    #ifdef DDRTYPE_DDR3
        psDramCtx = &DramCtx_PCDDR3;
    #endif
#endif
    
#if defined(DUAL_FREQ_K) & defined(FREQ_ADJUST)
    psDramCtx->frequency = mt_get_freq_setting(psDramCtx); 
#endif
    
#ifdef DDR_RESERVE_MODE  
    wdt_mode = READ_REG(MTK_WDT_MODE);
    wdt_dbg_ctrl = READ_REG(MTK_WDT_DEBUG_CTL);

    print("before mt_mempll_init, wdt_mode = 0x%x, wdt_dbg_ctrl = 0x%x\n", wdt_mode, wdt_dbg_ctrl);     
    if(((wdt_mode & MTK_WDT_MODE_DDR_RESERVE) !=0) && ((wdt_dbg_ctrl & MTK_DDR_RESERVE_RTA) != 0) ) {
        print("[PLL] skip mt_mempll_init!!!\n");
        return;
    }
#endif
    
    print("[PLL] mempll_init\n");
    mt_mempll_init(psDramCtx);
    return;
}

void mt_mempll_post(void)
{
#ifdef DDR_RESERVE_MODE  
    unsigned int wdt_mode;
    unsigned int wdt_dbg_ctrl;
#endif
    DRAMC_CTX_T *psDramCtx;

#ifdef COMBO_MCP
    int dram_type = mt_get_dram_type();
    
    if(dram_type == TYPE_LPDDR2)
        psDramCtx = &DramCtx_LPDDR2;
    else if(dram_type == TYPE_LPDDR3)
        psDramCtx = &DramCtx_LPDDR3;
#else
    #ifdef DDRTYPE_LPDDR2
        psDramCtx = &DramCtx_LPDDR2;
    #endif

    #ifdef DDRTYPE_LPDDR3
        psDramCtx = &DramCtx_LPDDR3;
    #endif

    #ifdef DDRTYPE_DDR3
        psDramCtx = &DramCtx_PCDDR3;
    #endif
#endif

#ifdef DDR_RESERVE_MODE  
    wdt_mode = READ_REG(MTK_WDT_MODE);
    wdt_dbg_ctrl = READ_REG(MTK_WDT_DEBUG_CTL);

    print("before mt_mempll_cali, wdt_mode = 0x%x, wdt_dbg_ctrl = 0x%x\n", wdt_mode, wdt_dbg_ctrl);     
    if(((wdt_mode & MTK_WDT_MODE_DDR_RESERVE) !=0) && ((wdt_dbg_ctrl & MTK_DDR_RESERVE_RTA) != 0) ) {
        print("[PLL] skip mt_mempll_cali!!!\n");
        return;
    }
#endif
    print("[PLL] mempll_cali\n");
    mt_mempll_cali(psDramCtx);
    return;
}


#if 1
unsigned int mt_get_cpu_freq(void)
{
	int output = 0, i = 0;
    unsigned int temp, clk26cali_0, clk_dbg_cfg, clk_misc_cfg_0, clk26cali_1;

    clk_dbg_cfg = DRV_Reg32(CLK_DBG_CFG);
    DRV_WriteReg32(CLK_DBG_CFG, 2<<16); //sel abist_cksw and enable freq meter sel abist
    
    clk_misc_cfg_0 = DRV_Reg32(CLK_MISC_CFG_0);
    DRV_WriteReg32(CLK_MISC_CFG_0, (clk_misc_cfg_0 & 0x0000FFFF) | (0x07 << 16)); // select divider

    clk26cali_1 = DRV_Reg32(CLK26CALI_1);
    DRV_WriteReg32(CLK26CALI_1, 0x00ff0000); // 

    //temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, 0x1000);
    DRV_WriteReg32(CLK26CALI_0, 0x1010);

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        mdelay(10);
        i++;
        if(i > 10)
        	break;
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = (((temp * 26000) ) / 256)*8; // Khz

    DRV_WriteReg32(CLK_DBG_CFG, clk_dbg_cfg);
    DRV_WriteReg32(CLK_MISC_CFG_0, clk_misc_cfg_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);
    DRV_WriteReg32(CLK26CALI_1, clk26cali_1);

    //print("freq = %d\n", output);

    if(i>10)
        return 0;
    else
        return output;
}


unsigned int mt_get_mem_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_dbg_cfg, clk_misc_cfg_0, clk26cali_1;

    clk_dbg_cfg = DRV_Reg32(CLK_DBG_CFG);
    DRV_WriteReg32(CLK_DBG_CFG, 0x1901); //sel ckgen_cksw and enable freq meter sel ckgen

    clk_misc_cfg_0 = DRV_Reg32(CLK_MISC_CFG_0);
    DRV_WriteReg32(CLK_MISC_CFG_0, (clk_misc_cfg_0 & 0x00FFFFFF) | (0x07 << 24)); // select divider

    clk26cali_1 = DRV_Reg32(CLK26CALI_1);
    DRV_WriteReg32(CLK26CALI_1, 0x00ff0000); // 

    //temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, 0x1000);
    DRV_WriteReg32(CLK26CALI_0, 0x1010);

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        print("wait for frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = (((temp * 26000) ) / 256)*8; // Khz

    DRV_WriteReg32(CLK_DBG_CFG, clk_dbg_cfg);
    DRV_WriteReg32(CLK_MISC_CFG_0, clk_misc_cfg_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);
    DRV_WriteReg32(CLK26CALI_1, clk26cali_1);

    //print("CLK26CALI = 0x%x, bus frequency = %d Khz\n", temp, output);

    return output;
}

unsigned int mt_get_bus_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_dbg_cfg, clk_misc_cfg_0, clk26cali_1;

    clk_dbg_cfg = DRV_Reg32(CLK_DBG_CFG);
    DRV_WriteReg32(CLK_DBG_CFG, 0x101); //sel ckgen_cksw and enable freq meter sel ckgen

    clk_misc_cfg_0 = DRV_Reg32(CLK_MISC_CFG_0);
    DRV_WriteReg32(CLK_MISC_CFG_0, (clk_misc_cfg_0 & 0x00FFFFFF) | (0x07 << 24)); // select divider

    clk26cali_1 = DRV_Reg32(CLK26CALI_1);
    DRV_WriteReg32(CLK26CALI_1, 0x00ff0000); // 

    //temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, 0x1000);
    DRV_WriteReg32(CLK26CALI_0, 0x1010);

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        print("wait for frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = (((temp * 26000) ) / 256)*8; // Khz

    DRV_WriteReg32(CLK_DBG_CFG, clk_dbg_cfg);
    DRV_WriteReg32(CLK_MISC_CFG_0, clk_misc_cfg_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);
    DRV_WriteReg32(CLK26CALI_1, clk26cali_1);

    //print("CLK26CALI = 0x%x, bus frequency = %d Khz\n", temp, output);

    return output;
}
#endif

//after pmic_init
void mt_pll_post_init(void)
{
    unsigned int temp;
    mt_mempll_pre();
	mt_mempll_post();

    //set mem_clk
    DRV_WriteReg32(CLK_CFG_0, 0x01000101); //mem_ck =mempll
    DRV_WriteReg32(CLK_CFG_UPDATE, 0x2);

    //step 48
    temp = DRV_Reg32(AP_PLL_CON3);
    //DRV_WriteReg32(AP_PLL_CON3, temp & 0xFFF44440); // Only UNIVPLL SW Control
    DRV_WriteReg32(AP_PLL_CON3, temp & 0xFFF4ccc0); // Only UNIVPLL/MPLL SW Control

    //step 49
    temp = DRV_Reg32(AP_PLL_CON4);
    //DRV_WriteReg32(AP_PLL_CON4, temp & 0xFFFFFFF4); // Only UNIVPLL SW Control
    DRV_WriteReg32(AP_PLL_CON4, temp & 0xFFFFFFFc); // Only UNIVPLL/MPLL SW Control


//    print("mt_pll_post_init: mt_get_cpu_freq = %dKhz\n", mt_get_cpu_freq());    
//    print("mt_pll_post_init: mt_get_bus_freq = %dKhz\n", mt_get_bus_freq());
//    print("mt_pll_post_init: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq());

#if 0
    print("mt_pll_post_init: AP_PLL_CON3        = 0x%x, GS = 0x00000000\n", DRV_Reg32(AP_PLL_CON3));
    print("mt_pll_post_init: AP_PLL_CON4        = 0x%x, GS = 0x00000000\n", DRV_Reg32(AP_PLL_CON4));
    print("mt_pll_post_init: AP_PLL_CON6        = 0x%x, GS = 0x00000000\n", DRV_Reg32(AP_PLL_CON6));
    print("mt_pll_post_init: CLKSQ_STB_CON0     = 0x%x, GS = 0x05010501\n", DRV_Reg32(CLKSQ_STB_CON0));
    print("mt_pll_post_init: PLL_ISO_CON0       = 0x%x, GS = 0x00080008\n", DRV_Reg32(PLL_ISO_CON0));
    print("mt_pll_post_init: ARMCA7PLL_CON0     = 0x%x, GS = 0xF1000101\n", DRV_Reg32(ARMCA7PLL_CON0));
    print("mt_pll_post_init: ARMCA7PLL_CON1     = 0x%x, GS = 0x800E8000\n", DRV_Reg32(ARMCA7PLL_CON1));
    print("mt_pll_post_init: ARMCA7PLL_PWR_CON0 = 0x%x, GS = 0x00000001\n", DRV_Reg32(ARMCA7PLL_PWR_CON0));
    print("mt_pll_post_init: MAINPLL_CON0       = 0x%x, GS = 0xF1000101\n", DRV_Reg32(MAINPLL_CON0));
    print("mt_pll_post_init: MAINPLL_CON1       = 0x%x, GS = 0x800A8000\n", DRV_Reg32(MAINPLL_CON1));
    print("mt_pll_post_init: MAINPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(MAINPLL_PWR_CON0));
    print("mt_pll_post_init: UNIVPLL_CON0       = 0x%x, GS = 0xFF000011\n", DRV_Reg32(UNIVPLL_CON0));
    print("mt_pll_post_init: UNIVPLL_CON1       = 0x%x, GS = 0x80180000\n", DRV_Reg32(UNIVPLL_CON1));
    print("mt_pll_post_init: UNIVPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(UNIVPLL_PWR_CON0));
    print("mt_pll_post_init: MMPLL_CON0         = 0x%x, GS = 0x00000101\n", DRV_Reg32(MMPLL_CON0));
    print("mt_pll_post_init: MMPLL_CON1         = 0x%x, GS = 0x820D8000\n", DRV_Reg32(MMPLL_CON1));
    print("mt_pll_post_init: MMPLL_PWR_CON0     = 0x%x, GS = 0x00000001\n", DRV_Reg32(MMPLL_PWR_CON0));
    print("mt_pll_post_init: MSDCPLL_CON0       = 0x%x, GS = 0x00000111\n", DRV_Reg32(MSDCPLL_CON0));
    print("mt_pll_post_init: MSDCPLL_CON1       = 0x%x, GS = 0x800F6276\n", DRV_Reg32(MSDCPLL_CON1));
    print("mt_pll_post_init: MSDCPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(MSDCPLL_PWR_CON0));
    print("mt_pll_post_init: TVDPLL_CON0        = 0x%x, GS = 0x00000101\n", DRV_Reg32(TVDPLL_CON0));
    print("mt_pll_post_init: TVDPLL_CON1        = 0x%x, GS = 0x80112276\n", DRV_Reg32(TVDPLL_CON1));
    print("mt_pll_post_init: TVDPLL_PWR_CON0    = 0x%x, GS = 0x00000001\n", DRV_Reg32(TVDPLL_PWR_CON0));
    print("mt_pll_post_init: VENCPLL_CON0       = 0x%x, GS = 0x00000111\n", DRV_Reg32(VENCPLL_CON0));
    print("mt_pll_post_init: VENCPLL_CON1       = 0x%x, GS = 0x800E989E\n", DRV_Reg32(VENCPLL_CON1));
    print("mt_pll_post_init: VENCPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(VENCPLL_PWR_CON0));
    print("mt_pll_post_init: MPLL_CON0          = 0x%x, GS = 0x00010111\n", DRV_Reg32(MPLL_CON0));
    print("mt_pll_post_init: MPLL_CON1          = 0x%x, GS = 0x801C0000\n", DRV_Reg32(MPLL_CON1));
    print("mt_pll_post_init: MPLL_PWR_CON0      = 0x%x, GS = 0x00000001\n", DRV_Reg32(MPLL_PWR_CON0));
    print("mt_pll_post_init: APLL1_CON0         = 0x%x, GS = 0xF0000131\n", DRV_Reg32(APLL1_CON0));
    print("mt_pll_post_init: APLL1_CON1         = 0x%x, GS = 0xB7945EA6\n", DRV_Reg32(APLL1_CON1));
    print("mt_pll_post_init: APLL1_PWR_CON0     = 0x%x, GS = 0x00000001\n", DRV_Reg32(APLL1_PWR_CON0));
    print("mt_pll_post_init: APLL2_CON0         = 0x%x, GS = 0x00000131\n", DRV_Reg32(APLL2_CON0));
    print("mt_pll_post_init: APLL2_CON1         = 0x%x, GS = 0xBC7EA932\n", DRV_Reg32(APLL2_CON1));
    print("mt_pll_post_init: APLL2_PWR_CON0     = 0x%x, GS = 0x00000001\n", DRV_Reg32(APLL2_PWR_CON0));
    
    print("mt_pll_post_init:  SPM_PWR_STATUS    = 0x%x, \n", DRV_Reg32(SPM_PWR_STATUS));
    print("mt_pll_post_init:  DISP_CG_CON0    = 0x%x, \n", DRV_Reg32(DISP_CG_CON0));
    print("mt_pll_post_init:  DISP_CG_CON1    = 0x%x, \n", DRV_Reg32(DISP_CG_CON1));
#endif
}

#if 0
//after pmic_init
void mt_arm_pll_sel(void)
{
    unsigned int temp;
    
    temp = DRV_Reg32(TOP_CKMUXSEL);
    //DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x5); // switch CA7_ck to ARMCA7PLL, and CA15_ck to ARMCA15PLL
    DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x1); // switch CA7_ck to ARMCA7PLL

    print("[PLL] mt_arm_pll_sel done\n");
}
#endif

void mt_pll_init(void)
{
    int ret = 0;
    unsigned int temp;

    DRV_WriteReg32(ACLKEN_DIV, 0x12); // MCU Bus DIV2

    //step 1
    DRV_WriteReg32(CLKSQ_STB_CON0, 0x05010501); // reduce CLKSQ disable time
    
    //step 2
    DRV_WriteReg32(PLL_ISO_CON0, 0x00080008); // extend PWR/ISO control timing to 1us
    
    //step 3
    DRV_WriteReg32(AP_PLL_CON6, 0x00000000); //

    /*************
    * xPLL PWR ON 
    **************/
    //step 4
    //temp = DRV_Reg32(ARMCA15PLL_PWR_CON0);
    //DRV_WriteReg32(ARMCA15PLL_PWR_CON0, temp | 0x1);

    temp = DRV_Reg32(ARMCA7PLL_PWR_CON0);
    DRV_WriteReg32(ARMCA7PLL_PWR_CON0, temp | 0x1);

    //step 5
    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp | 0x1);
    
    //step 6
    temp = DRV_Reg32(UNIVPLL_PWR_CON0);
    DRV_WriteReg32(UNIVPLL_PWR_CON0, temp | 0x1);
    
    //step 7
    temp = DRV_Reg32(MMPLL_PWR_CON0);
    DRV_WriteReg32(MMPLL_PWR_CON0, temp | 0x1);
    
    //step 8
    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp | 0x1);
    
    //step 9
    temp = DRV_Reg32(VENCPLL_PWR_CON0);
    DRV_WriteReg32(VENCPLL_PWR_CON0, temp | 0x1);
    
    //step 10
    temp = DRV_Reg32(TVDPLL_PWR_CON0);
    DRV_WriteReg32(TVDPLL_PWR_CON0, temp | 0x1);

    //step 11
    temp = DRV_Reg32(MPLL_PWR_CON0);
    DRV_WriteReg32(MPLL_PWR_CON0, temp | 0x1);
    
    //step 12
    //temp = DRV_Reg32(VCODECPLL_PWR_CON0);
    //DRV_WriteReg32(VCODECPLL_PWR_CON0, temp | 0x1);

    //step 13
    temp = DRV_Reg32(APLL1_PWR_CON0);
    DRV_WriteReg32(APLL1_PWR_CON0, temp | 0x1);
    
    //step 14
    temp = DRV_Reg32(APLL2_PWR_CON0);
    DRV_WriteReg32(APLL2_PWR_CON0, temp | 0x1);

    gpt_busy_wait_us(5); // wait for xPLL_PWR_ON ready (min delay is 1us)

    /******************
    * xPLL ISO Disable
    *******************/
    //step 15
    //temp = DRV_Reg32(ARMCA15PLL_PWR_CON0);
    //DRV_WriteReg32(ARMCA15PLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(ARMCA7PLL_PWR_CON0);
    DRV_WriteReg32(ARMCA7PLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 16
    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 17
    temp = DRV_Reg32(UNIVPLL_PWR_CON0);
    DRV_WriteReg32(UNIVPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 18
    temp = DRV_Reg32(MMPLL_PWR_CON0);
    DRV_WriteReg32(MMPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 19
    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 20
    temp = DRV_Reg32(VENCPLL_PWR_CON0);
    DRV_WriteReg32(VENCPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 21
    temp = DRV_Reg32(TVDPLL_PWR_CON0);
    DRV_WriteReg32(TVDPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 22
    temp = DRV_Reg32(MPLL_PWR_CON0);
    DRV_WriteReg32(MPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 23
    //temp = DRV_Reg32(VCODECPLL_PWR_CON0);
    //DRV_WriteReg32(VCODECPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 24
    temp = DRV_Reg32(APLL1_PWR_CON0);
    DRV_WriteReg32(APLL1_PWR_CON0, temp & 0xFFFFFFFD);
    
    //step 25
    temp = DRV_Reg32(APLL2_PWR_CON0);
    DRV_WriteReg32(APLL2_PWR_CON0, temp & 0xFFFFFFFD);

    /********************
    * xPLL Frequency Set
    *********************/
    //step 26
    DRV_WriteReg32(ARMCA7PLL_CON1, 0x800B4000); // 1170MHz
    
    //step 27
    DRV_WriteReg32(MAINPLL_CON1, 0x800A8000); //1092MHz
    
    //step 28
    DRV_WriteReg32(MMPLL_CON1, 0x82118000); //455MHz
    
    //step 29
    DRV_WriteReg32(MSDCPLL_CON1, 0x800F6276); //800MHz
    
    //step 30
    //FIXME, change to 410MHz
    //DRV_WriteReg32(VENCPLL_CON1, 0x800F6276); //800MHz
    DRV_WriteReg32(VENCPLL_CON1, 0x800FC4EC); //410MHz
    
    //step 31
    //FIXME, change to 594MHz
    //DRV_WriteReg32(TVDPLL_CON1, 0x80112276); // 445.5MHz
    DRV_WriteReg32(TVDPLL_CON1, 0x8016D89E); // 594MHz

    //step 32
    //FIXME, change to 208MHz
    //DRV_WriteReg32(MPLL_CON1, 0x801C0000);
    //DRV_WriteReg32(MPLL_CON0, 0x00010110); //52MHz
    DRV_WriteReg32(MPLL_CON1, 0x80100000);
    DRV_WriteReg32(MPLL_CON0, 0x00010130); //208MHz

    //step 33
//#if 1
//    DRV_WriteReg32(VCODECPLL_CON1, 0x80130000); // 494MHz
//#else
//    DRV_WriteReg32(VCODECPLL_CON1, 0x80150000); // 546MHz
//#endif

    //APLL1 and APLL2 use the default setting 
    /***********************
    * xPLL Frequency Enable
    ************************/
    //step 34
    //temp = DRV_Reg32(ARMCA15PLL_CON0);
    //DRV_WriteReg32(ARMCA15PLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(ARMCA7PLL_CON0);
    DRV_WriteReg32(ARMCA7PLL_CON0, temp | 0x1);
    
    //step 35
    temp = DRV_Reg32(MAINPLL_CON0) & (~ 0x70);
/* CC: modify for testing */
#if 1
    /* CCI400 @ 500MHz */
    /* not divide by 2 */
#else
    /* CCI400 @ 250MHz */
    /* divide by 2 */
    temp |= 0x10;
#endif
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x1);
    
    //step 36
    temp = DRV_Reg32(UNIVPLL_CON0);
    DRV_WriteReg32(UNIVPLL_CON0, temp | 0x1);
    
    //step 37
    temp = DRV_Reg32(MMPLL_CON0);
    DRV_WriteReg32(MMPLL_CON0, temp | 0x1);
    
    //step 38
    temp = DRV_Reg32(MSDCPLL_CON0);
    DRV_WriteReg32(MSDCPLL_CON0, temp | 0x1);
    
    //step 39
    temp = DRV_Reg32(VENCPLL_CON0);
    DRV_WriteReg32(VENCPLL_CON0, temp | 0x1);
    
    //step 40
    temp = DRV_Reg32(TVDPLL_CON0);
    DRV_WriteReg32(TVDPLL_CON0, temp | 0x1); 

    //step 41
    temp = DRV_Reg32(MPLL_CON0);
    DRV_WriteReg32(MPLL_CON0, temp | 0x1); 
    
    //step 42
    //temp = DRV_Reg32(VCODECPLL_CON0);
    //DRV_WriteReg32(VCODECPLL_CON0, temp | 0x1); 
    
    //step 43
    temp = DRV_Reg32(APLL1_CON0);
    DRV_WriteReg32(APLL1_CON0, temp | 0x1); 
    
    //step 44
    temp = DRV_Reg32(APLL2_CON0);
    DRV_WriteReg32(APLL2_CON0, temp | 0x1); 
    
    gpt_busy_wait_us(40); // wait for PLL stable (min delay is 20us)

    /***************
    * xPLL DIV RSTB
    ****************/
    //step 45
    temp = DRV_Reg32(ARMCA7PLL_CON0);
    DRV_WriteReg32(ARMCA7PLL_CON0, temp | 0x01000000);
    
    //step 46
    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x01000000);
    
    //step 47
    temp = DRV_Reg32(UNIVPLL_CON0);
    DRV_WriteReg32(UNIVPLL_CON0, temp | 0x01000000);

    /*****************
    * xPLL HW Control
    ******************/
#if 0
    //default is SW mode, set HW mode after MEMPLL caribration
    //step 48
    temp = DRV_Reg32(AP_PLL_CON3);
    DRV_WriteReg32(AP_PLL_CON3, temp & 0xFFF4CCC0); // UNIVPLL SW Control

    //step 49
    temp = DRV_Reg32(AP_PLL_CON4);
    DRV_WriteReg32(AP_PLL_CON4, temp & 0xFFFFFFFC); // UNIVPLL,  SW Control
#endif
    /*************
    * MEMPLL Init
    **************/

//    mt_mempll_pre();

    /**************
    * INFRA CLKMUX
    ***************/

    //temp = DRV_Reg32(TOP_DCMCTL);
    //DRV_WriteReg32(TOP_DCMCTL, temp | 0x1); // enable infrasys DCM
    
    DRV_WriteReg32(INFRA_BUS_DCM_CTRL, 0x001f0603);
    DRV_WriteReg32(PERI_BUS_DCM_CTRL, 0xb01f0603);
    temp = DRV_Reg32(INFRA_BUS_DCM_CTRL);
    DRV_WriteReg32(INFRA_BUS_DCM_CTRL, temp | (0x1<<22));

    //CA7: INFRA_TOPCKGEN_CKDIV1[4:0](0x10001008)
    temp = DRV_Reg32(TOP_CKDIV1);
    DRV_WriteReg32(TOP_CKDIV1, temp & 0xFFFFFFE0); // CPU clock divide by 1

    //CA7: INFRA_TOPCKGEN_CKMUXSEL[1:0] (0x10001000) =1
    temp = DRV_Reg32(TOP_CKMUXSEL);
    DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x1); // switch CA7_ck to ARMCA7PLL

    /************
    * TOP CLKMUX
    *************/

    //DRV_WriteReg32(CLK_CFG_0, 0x01000101); //ddrphycfg_ck = 26MHz
    DRV_WriteReg32(CLK_CFG_0, 0x01000001);//mm_ck=syspll_d3, ddrphycfg_ck=26M, mem_ck=26M, axi=syspll1_d4

    DRV_WriteReg32(CLK_CFG_1, 0x01000100);//mfg_ck=mmpll_ck, Null, vdec_ck=syspll_d3, pwm_ck=26Mhz 

    DRV_WriteReg32(CLK_CFG_2, 0x00010000);//Null, spi_ck=syspll3_d2, uart=26M, camtg=26M

    DRV_WriteReg32(CLK_CFG_3, 0x02060200);//msdc30_1=msdcpll_d4, msdc50_0=msdcpll_d4, msdc50_0_hclk=syspll2_d2, Null

    DRV_WriteReg32(CLK_CFG_4, 0x01000202);//aud_intbus=syspll1_d4, aud=26M, msdc30_3=MSDCPLL_d2, msdc30_2=MSDCPLL_d2

    DRV_WriteReg32(CLK_CFG_5, 0x01010100);//mjc_ck=syspll_d5, atb_ck=SYSPLL1_D2, scp_ck=mpll, pmicspi=26MHz

    DRV_WriteReg32(CLK_CFG_6, 0x01010102);//aud2_ck=apll2 , aud1_ck=apll1, scam_ck=syspll3_d2, dpi0_ck=tvdpll_d4

    DRV_WriteReg32(CLK_CFG_UPDATE, 0x07ffffff);

    DRV_WriteReg32(CLK_SCP_CFG_0, 0x3FF); // enable scpsys clock off control
    DRV_WriteReg32(CLK_SCP_CFG_1, 0x7); // enable scpsys clock off control

    /*for MTCMOS*/
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    spm_mtcmos_ctrl_disp(STA_POWER_ON);
    spm_mtcmos_ctrl_vdec(STA_POWER_ON);
    spm_mtcmos_ctrl_venc(STA_POWER_ON);
    spm_mtcmos_ctrl_isp(STA_POWER_ON);
    spm_mtcmos_ctrl_aud(STA_POWER_ON);
    spm_mtcmos_ctrl_mfg_ASYNC(STA_POWER_ON);
    spm_mtcmos_ctrl_mfg(STA_POWER_ON);
    
    temp = seclib_get_devinfo_with_index(3);
    if( !(temp & (1<<16)))
        spm_mtcmos_ctrl_mjc(STA_POWER_ON);

    spm_mtcmos_ctrl_connsys(STA_POWER_ON);

    /*for CG*/
    DRV_WriteReg32(INFRA_PDN_CLR0, 0xFFFFFFFF);
    DRV_WriteReg32(INFRA_PDN_CLR1, 0xFFFFFFFF);
    /*DISP CG*/
    //DRV_WriteReg32(DISP_CG_CLR0, 0xFFFFFFFF);
    //DRV_WriteReg32(DISP_CG_CLR1, 0x3F);
    //AUDIO
    DRV_WriteReg32(AUDIO_TOP_CON0, 0);
    //MFG
    DRV_WriteReg32(MFG_CG_CLR, 0x00000001);
    //ISP
    DRV_WriteReg32(IMG_CG_CLR, 0x00000BE1);
    //VDE
    DRV_WriteReg32(VDEC_CKEN_SET, 0x00000001);
    DRV_WriteReg32(LARB_CKEN_SET, 0x00000001);
    //VENC
    DRV_WriteReg32(VENC_CG_SET, 0x00001111);
    //MJC
    DRV_WriteReg32(MJC_CG_CLR, 0x0000002F);
}

#if 0
int spm_mtcmos_ctrl_disp(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | SRAM_PDN);
#if 1
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK) != DIS_SRAM_ACK) {
        }
#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_DIS_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_DIS_PWR_CON, val);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON | PWR_ON_S));

        while ((spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & DIS_PWR_STA_MASK)) {
        }
    } else {    /* STA_POWER_ON */
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S);

        while (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_2ND) & DIS_PWR_STA_MASK)) {
        }

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~SRAM_PDN);

#if 1
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK)) {
        }
#endif
    }

    return err;
}
#endif

#if 0
int spm_mtcmos_ctrl_mdsys1(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;
    int count = 0;

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) | MD1_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & MD1_PROT_MASK) != MD1_PROT_MASK) {
            count++;
            if(count>1000)
                break;
        }

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | MD_SRAM_PDN);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_MD_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_MD_PWR_CON, val);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~(PWR_ON | PWR_ON_S));

        while ((spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & MD1_PWR_STA_MASK)) {
        }

        spm_write(AP_PLL_CON7, (spm_read(AP_PLL_CON7) | 0xF)); //force off LTE
        spm_write(0x10007054, 0x80000000);

    } else {    /* STA_POWER_ON */
        
        spm_write(AP_PLL_CON7, (spm_read(AP_PLL_CON7) & (~0x4))); //turn on LTE, clk
        spm_write(AP_PLL_CON7, (spm_read(AP_PLL_CON7) & (~0x1))); //turn on LTE, mtcmos
        spm_write(AP_PLL_CON7, (spm_read(AP_PLL_CON7) & (~0x8))); //turn on LTE, mtcmos + iso
        spm_write(AP_PLL_CON7, (spm_read(AP_PLL_CON7) & (~0x2))); //turn on LTE, memory
    
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ON);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ON_S);

        while (!(spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_2ND) & MD1_PWR_STA_MASK)) {
        }

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_RST_B);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~MD_SRAM_PDN);

        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) & ~MD1_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & MD1_PROT_MASK) {
        }
    }

    return err;
}
#endif

