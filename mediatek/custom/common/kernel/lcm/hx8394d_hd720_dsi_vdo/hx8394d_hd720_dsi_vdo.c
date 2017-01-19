


/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/xlog.h>
#include <mach/mt_pm_ldo.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                         (720)
#define FRAME_HEIGHT                                        (1280)

#define REGFLAG_DELAY                                       0xFE
#define REGFLAG_END_OF_TABLE                                0xFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE                                    0

#define LCM_ID_HX8394D                                     0x94
#define GPIO_LCD_RST_EN      GPIO131

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

     //unsigned int tmp=0x50;
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                  lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)              lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                            lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};

static void init_lcm_registers(void)
{
     unsigned int data_array[16];
   
    data_array[0] = 0x00043902;
    data_array[1] = 0x9483ffb9;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x00033902;
    data_array[1] = 0x008372ba; //0x008373ba;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x00103902;
    data_array[1] = 0x10106cb1; //0x0d0d6cb1;
    data_array[2] = 0xf1110426;
    data_array[3] = 0x23543a81;
    data_array[4] = 0x58d2c080;
    dsi_set_cmdq(data_array, 5, 1);
	
    data_array[0] = 0x000c3902;
    data_array[1] = 0x0e6400b2;
    data_array[2] = 0x081c220d;
    data_array[3] = 0x004d1c08;
    dsi_set_cmdq(data_array, 4, 1);
	
    data_array[0] = 0x000d3902;
    data_array[1] = 0x51ff00b4;
    data_array[2] = 0x03525952; //0x035a595a;
    data_array[3] = 0x20600152; //0x2070015a;
    data_array[4] = 0x00000060; //0x00000070;
    dsi_set_cmdq(data_array, 5, 1);
	
    data_array[0] = 0x00023902;
    data_array[1] = 0x000007bc;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x00043902;
    data_array[1] = 0x010e41bf;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x001f3902;
    data_array[1] = 0x000f00d3;
    data_array[2] = 0x00100740;
    data_array[3] = 0x00081008;
    data_array[4] = 0x0e155408;
    data_array[5] = 0x15020e05;
    data_array[6] = 0x47060506;
    data_array[7] = 0x4b0a0a44;
    data_array[8] = 0x00070710;
    dsi_set_cmdq(data_array, 9, 1);
	
    data_array[0] = 0x002d3902;
    data_array[1] = 0x1b1a1ad5;
    data_array[2] = 0x0201001b;
    data_array[3] = 0x06050403;
    data_array[4] = 0x0a090807;
    data_array[5] = 0x1825240b;
    data_array[6] = 0x18272618;
    data_array[7] = 0x18181818;
    data_array[8] = 0x18181818;
    data_array[9] = 0x18181818;
    data_array[10] = 0x20181818;
    data_array[11] = 0x18181821;
    data_array[12] = 0x00000018;
    dsi_set_cmdq(data_array, 13, 1);
	
    data_array[0] = 0x002d3902;
    data_array[1] = 0x1b1a1ad6;
    data_array[2] = 0x090a0b1b;
    data_array[3] = 0x05060708;
    data_array[4] = 0x01020304;
    data_array[5] = 0x58202100;
    data_array[6] = 0x18262758;
    data_array[7] = 0x18181818;
    data_array[8] = 0x18181818;
    data_array[9] = 0x18181818;
    data_array[10] = 0x25181818;
    data_array[11] = 0x18181824;
    data_array[12] = 0x00000018;
    dsi_set_cmdq(data_array, 13, 1);	

    data_array[0] = 0x002b3902;
    data_array[1] = 0x100b00e0; //0x150e00e0;
    data_array[2] = 0x1d3f322c; //0x213f2c20;
    data_array[3] = 0x0d0b0639; //0x0c0a073b;
    data_array[4] = 0x14110e17;
    data_array[5] = 0x13081311; //0x160c1312;
    data_array[6] = 0x0b001614; //0x0e001c18;
    
    data_array[7] = 0x3f322c10; //0x3f2c2015;
    data_array[8] = 0x0b06391d; //0x0a073b21;
    data_array[9] = 0x110e170d; //0x110e170c;
    data_array[10] = 0x08131114; //0x0c131214;
    data_array[11] = 0x00161413; //0x001c1816;
    dsi_set_cmdq(data_array, 12, 1);
	
    data_array[0] = 0x00023902;
    data_array[1] = 0x000009cc;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x00033902;
    data_array[1] = 0x001430c0;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x00053902;
    data_array[1] = 0x40c000c7;
    data_array[2] = 0x000000c0;
    dsi_set_cmdq(data_array, 3, 1);
	
    data_array[0] = 0x00033902;
    data_array[1] = 0x007070b6; //0x006a6ab6;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(200);
    data_array[0] = 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(10);
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode                 = LCM_DBI_TE_MODE_DISABLED;
    params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;

    params->dsi.mode   = SYNC_PULSE_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM                = LCM_THREE_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.word_count=720*3;
    params->dsi.vertical_sync_active                = 2;
    params->dsi.vertical_backporch                  = 16;
    params->dsi.vertical_frontporch                 = 9;
    params->dsi.vertical_active_line                = FRAME_HEIGHT;
    params->dsi.horizontal_sync_active              = 18;
    params->dsi.horizontal_backporch                = 50;
    params->dsi.horizontal_frontporch               = 50;
    params->dsi.horizontal_active_pixel             = FRAME_WIDTH;


    // Video mode setting
    //params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    //params->dsi.pll_select=1;
    //params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_253_5;//LCM_DSI_6589_PLL_CLOCK_240_5;//LCM_DSI_6589_PLL_CLOCK_227_5;//this value must be in MTK suggested table 227_5
	params->dsi.PLL_CLOCK = 250;
}
static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);

	init_lcm_registers();
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	
	SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
}


static void lcm_resume(void)
{
	lcm_init();
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
  unsigned int x0 = x;
  unsigned int y0 = y;
  unsigned int x1 = x0 + width - 1;
  unsigned int y1 = y0 + height - 1;

  unsigned char x0_MSB = ((x0>>8)&0xFF);
  unsigned char x0_LSB = (x0&0xFF);
  unsigned char x1_MSB = ((x1>>8)&0xFF);
  unsigned char x1_LSB = (x1&0xFF);
  unsigned char y0_MSB = ((y0>>8)&0xFF);
  unsigned char y0_LSB = (y0&0xFF);
  unsigned char y1_MSB = ((y1>>8)&0xFF);
  unsigned char y1_LSB = (y1&0xFF);

  unsigned int data_array[16];

  data_array[0]= 0x00052902;
  data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
  data_array[2]= (x1_LSB);
  dsi_set_cmdq(&data_array, 3, 1);

  data_array[0]= 0x00052902;
  data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
  data_array[2]= (y1_LSB);
  dsi_set_cmdq(&data_array, 3, 1);

  data_array[0]= 0x00290508; //HW bug, so need send one HS packet
  dsi_set_cmdq(&data_array, 1, 1);

  data_array[0]= 0x002c3909;
  dsi_set_cmdq(&data_array, 1, 0);

}


static unsigned int lcm_compare_id(void)
{
    unsigned int id=0;
    unsigned char buffer[2];
    unsigned int array[16];


    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);

    array[0]=0x00043902;
    array[1]=0x9483FFB9;// page enable
    dsi_set_cmdq(&array, 2, 1);
    MDELAY(10);

    array[0]=0x00023902;
    array[1]=0x000013ba;
    dsi_set_cmdq(&array, 2, 1);
    MDELAY(10);

    array[0] = 0x00023700;// return byte number
    dsi_set_cmdq(&array, 1, 1);
    MDELAY(10);

    read_reg_v2(0xF4, buffer, 2);
    id = buffer[0];

#ifdef BUILD_LK
    printf("=====>compare id for test %s, id = 0x%08x\n", __func__, id);
#else
    printk("=====>compare id for test %s, id = 0x%08x\n", __func__, id);
#endif

    return (LCM_ID_HX8394D == id)?1:0;

}

static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
  char  buffer[3];
  int   array[16];

  if(lcm_esd_test)
  {
    lcm_esd_test = FALSE;
    return TRUE;
  }


  array[0] = 0x00013700;
  dsi_set_cmdq(array, 1, 1);

  read_reg_v2(0x0A, buffer, 1);
  if(buffer[0]==0x1c)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
 #endif

}

static unsigned int lcm_esd_recover(void)
{
  lcm_init();

  return TRUE;
}


// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------

LCM_DRIVER hx8394d_hd720_dsi_vdo_lcm_drv =
{
	.name           = "hx8394d_hd720_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check = lcm_esd_check,
	//.esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
        //.set_backlight    = lcm_setbacklight,
    //.set_pwm        = lcm_setpwm,
    //.get_pwm        = lcm_getpwm,
     //   .update         = lcm_update
#endif
};



