/*
 *  apds9960.c - Linux kernel modules for Gesture + RGB + ambient light + proximity sensor
 *
 *  Copyright (C) 2013 Lee Kai Koon <kai-koon.lee@avagotech.com>
 *  Copyright (C) 2013 Avago Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/earlysuspend.h>
#include "gn_avago_apds9960.h"

#include <linux/platform_device.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <mach/mt_gpio.h>
#include <cust_eint.h>
#include <cust_alsps_apds9960.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_pm_ldo.h>

//dma
#include <linux/dma-mapping.h>

#include <alsps.h>


#include <linux/batch.h>
#ifdef CUSTOM_KERNEL_SENSORHUB
#include <SCP_sensorHub.h>
#endif

#define USE_DEFAULT_CAILMODE
#ifdef USE_DEFAULT_CAILMODE
#include <asm/atomic.h>
#endif

/* Gionee BSP1 chengx 20140826 modify for CR01371160 begin */
#ifdef GN_MTK_BSP_DEVICECHECK
#include <linux/gn_device_check.h>
extern int gn_set_device_info(struct gn_device_info gn_dev_info);
#endif
/* Gionee BSP1 chengx 20140826 modify for CR01371160 end */

#define POWER_NONE_MACRO MT65XX_POWER_NONE


//#define GESTURE_SUPPORT

#ifndef GESTURE_SUPPORT
///add for complier error
#ifndef ID_GESTURE
#define ID_GESTURE MAX_ANDROID_SENSOR_NUM +10
#endif

#endif
/************************************************
Change history

Ver		When			Who		Why
---		----			---		---
1.0.0	19-Aug-2013		KK		Initial draft
1.0.1	26-Aug-2013		KK		Revise gesture algorithm
1.0.2	29-Aug-2013		KK		Change GTHR_IN and GTHR_OUT
1.0.3	03-Sep-2013		KK		Correct divide by zero error in AveragingRawData()
1.0.4	05-Sep-2013		KK		Accept old and latest ID value
1.0.5	17-Sep-2013		KK		Return if sample size is less than or equal to 4 in GestureDataProcessing();
								Correct error in AveragingRawData()
1.0.6	27-Sep-2013		KK		Simplify GestureDataProcessing() and revise Gesture Calibration
								Added Up/Down/Left/Right U-Turn gesture detection
************************************************/

/* Global data */
static struct i2c_client *apds9960_i2c_client;  // global i2c_client to support ioctl
static struct workqueue_struct *apds_workqueue;
#if defined(CONFIG_HAS_EARLYSUSPEND)
static void apds9960_early_suspend(struct early_suspend *h);
static void apds9960_late_resume(struct early_suspend *h);
#endif

/* For interrup work mode support --add by liaoxl.lenovo 12.08.2011 */
extern void mt_eint_unmask(unsigned int line);
extern void mt_eint_mask(unsigned int line);
extern void mt_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void mt_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern struct alsps_hw *get_cust_alsps_hw_apds9960(void);


static int apds9960_init_flag =-1; // 0<==>OK -1 <==> fail
static int ps_status=0;




/*****************************************************/
#define APS_TAG                 "[ALS/PS] "
//#define APDS9960_DEBUG          1
#if defined (APDS9960_DEBUG)
#define APS_FUN(f)              printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)   printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)   printk(KERN_INFO APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)   printk(KERN_INFO  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#else
#define APS_FUN(f)              printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)   printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)   printk(KERN_INFO APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)
#endif

#define _DMA_RW_MODE_
#if defined(_DMA_RW_MODE_)
static uint8_t *g_pDMABuf_va = NULL;
static uint32_t *g_pDMABuf_pa = NULL;

static void dma_buffer_alloct()
{	
	g_pDMABuf_va = (u8 *)dma_alloc_coherent(NULL, 4096, &g_pDMABuf_pa, GFP_KERNEL);
	if (!g_pDMABuf_va)
    {        
		APS_LOG("[DMA][Error] Allocate DMA I2C Buffer failed!\n");    
    }
}

static void dma_buffer_release()
{
	if(g_pDMABuf_va)
	{
		dma_free_coherent(NULL, 4096, g_pDMABuf_va, g_pDMABuf_pa);  
		g_pDMABuf_va = NULL;
		g_pDMABuf_pa = NULL;
		APS_LOG("[DMA][release] Allocate DMA I2C Buffer release!\n");
	}
}
#endif

typedef enum
{
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;

#define APDS9960_DEV_NAME     "APDS9960"
static const struct i2c_device_id apds9960_i2c_id[] = {{APDS9960_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_apds9960={ I2C_BOARD_INFO(APDS9960_DEV_NAME, 0x39)};
static struct i2c_driver apds9960_i2c_driver;
struct apds9960_data *apds9960_obj = NULL;
static struct apds9960_data *g_apds9960_ptr = NULL;

/*****************************************************/
static unsigned char apds9960_als_atime_tb[] = { 0xF6, 0xEB, 0xD6 };
static unsigned short apds9960_als_integration_tb[] = {2400,5040, 10080}; // DO NOT use beyond 100.8ms
static unsigned short apds9960_als_res_tb[] = { 10240, 21504, 43008 };
static unsigned char apds9960_als_again_tb[] = { 1, 4, 16, 64 };
static unsigned char apds9960_als_again_bit_tb[] = { 0x00, 0x01, 0x02, 0x03 };

#if !defined(PLATFORM_SENSOR_APDS9960)
/* Ensure the coefficients do not exceed 9999 */
static int RGB_COE_X[3] = {-1882, 10240, -8173}; // {-1.8816, 10.24, -8.173};
static int RGB_COE_Y[3] = {-2100, 10130, -7708}; // {-2.0998, 10.13, -7.708};
static int RGB_COE_Z[3] = {-1937, 5201, -2435}; // {-1.937, 5.201, -2.435};
#endif

static int RGB_CIE_N1 = 332; // 0.332;
static int RGB_CIE_N2 = 186; // 0.1858;

static int RGB_CIE_CCT1 = 449; // 449.0;
static int RGB_CIE_CCT2 = 3525; // 3525.0;
static int RGB_CIE_CCT3 = 6823; // 6823.3;
static int RGB_CIE_CCT4 = 5520; // 5520.33;

/* Gesture data storage */
static GESTURE_DATA_TYPE gesture_data;

static int gesture_motion_test=DIR_NONE;  // for factory mode test
static int gesture_motion=DIR_NONE;
static int gesture_prev_motion=DIR_NONE;
static int fMotionMapped=1;  // 0;

int gesture_ud_delta=0;
int gesture_lr_delta=0;
int gesture_state=0;

int gesture_ud_count=0;
int gesture_lr_count=0;

int gesture_distance=0;
int gesture_prev_distance=0;
int gesture_far_near_flag=0;
int gesture_far_near_started=0;

int gesture_fundamentals=0;	// 0 = fundamentals, 1 = extra

unsigned int gesture_start_time=0;

unsigned int ps_data;			// to store PS data
unsigned int last_ps_data;      // to store previous pdata for gesture

/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
static unsigned int xtalk_pdata[10];
static int xtalk_pdata_index=0;
/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */

int FilterGestureRawData(GESTURE_DATA_TYPE *, GESTURE_DATA_TYPE *);
int GestureDataProcessing(void);
int DecodeGesture(int gesture_mode);
void ResetGestureParameters(void); 
static int apds9960_calibration(struct i2c_client *client, int val);
static int apds9960_Enable_runtime_calibration(struct i2c_client *client);

/* Gionee BSP1 yang_yang 20140802 modify for CR01341808 begin */
#if defined(GN_MTK_BSP_PS_STATIC_CALIBRATION)
struct PS_CALI_DATA_STRUCT
{
    int close;
    int far_away;
    int valid;
};
#endif
/* Gionee BSP1 yang_yang 20140802 modify for CR01341808 end */

/*
 * Management functions
 */

static int apds9960_local_init(void);
static int apds9960_remove();

static struct alsps_init_info apds9960_init_info = {
		.name = APDS9960_DEV_NAME,
		.init = apds9960_local_init,
		.uninit = apds9960_remove,
	
};


#define I2C_FLAG_WRITE  0
#define I2C_FLAG_READ   1
static DEFINE_MUTEX(apds9960_mutex);
int chip_i2c_master_operate(struct i2c_client *client, const char *buf, int count, int i2c_flag)
{
	int res = 0;

	mutex_lock(&apds9960_mutex);
	switch(i2c_flag)
	{
		case I2C_FLAG_WRITE:
            client->addr &=I2C_MASK_FLAG;
            res = i2c_master_send(client, buf, count);
            client->addr &=I2C_MASK_FLAG;
            break;
		
		case I2C_FLAG_READ:
            client->addr &=I2C_MASK_FLAG;
            client->addr |=I2C_WR_FLAG;
            client->addr |=I2C_RS_FLAG;
            res = i2c_master_send(client, buf, count);
            client->addr &=I2C_MASK_FLAG;
            break;

		default:
            APS_LOG("chip_i2c_master_operate i2c_flag command not support!\n");
            break;
	}

	if (res < 0)
	{
		goto EXIT_ERR;
	}
	mutex_unlock(&apds9960_mutex);
	return res;

EXIT_ERR:
	mutex_unlock(&apds9960_mutex);
	APS_ERR("chip_i2c_master_operate fail\n");
	return res;
}

static void I2C_dma_Readdata(u8 addr, u8* read_data, u16 size)
{   
    /* according to your platform */
	int rc;
	unsigned short temp_addr = 0;
	temp_addr = apds9960_i2c_client->addr ;

#if defined(_DMA_RW_MODE_)
	if (NULL == g_pDMABuf_va)		
	    return;	
#endif	

	struct i2c_msg msgs[] = {		
	    {						
		    .flags = I2C_M_RD,			
		    .len = size,			
		#if defined(_DMA_RW_MODE_)			
		    .addr = addr & I2C_MASK_FLAG | I2C_DMA_FLAG,			
		    .buf = apds9960_i2c_client,			
		#else			
		    .addr = addr,			
		    .buf = read_data,			
		#endif		
		},	
	};	
    rc = i2c_transfer(apds9960_i2c_client->adapter, msgs, 1);	
    if (rc < 0)  
    {
	    APS_ERR("I2C_dma_Readdata error %d\n", rc);
	}
#if defined(_DMA_RW_MODE_)
	else
	{
		memcpy(read_data, g_pDMABuf_va, size);
	}
#endif
    apds9960_i2c_client->addr = temp_addr;
}

#define I2C_MASTER_CLOCK 100
static void I2C_dma_write_Readdata(u8 cmd, u8* data, u16 size)
{    
	/* according to your platform */ 	
	int rc;
	struct i2c_msg msg[2];
	unsigned short temp_addr = 0;
	temp_addr = apds9960_i2c_client->addr ;

#if defined(_DMA_RW_MODE_)
	if (NULL == g_pDMABuf_va)		
		return;		
#endif		

	msg[0].addr = apds9960_i2c_client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &cmd;
	msg[0].ext_flag = apds9960_i2c_client->ext_flag;
    msg[0].timing = I2C_MASTER_CLOCK;
	
#if defined(_DMA_RW_MODE_)
	msg[1].addr = apds9960_i2c_client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG,			
	msg[1].buf = g_pDMABuf_pa,			
#else
    msg[1].addr = apds9960_i2c_client->addr;
	msg[1].buf =data;
#endif
    msg[1].flags = I2C_M_RD;
    msg[1].len = size;
	msg[1].ext_flag = apds9960_i2c_client->ext_flag;
    msg[1].timing = I2C_MASTER_CLOCK;

	rc = i2c_transfer(apds9960_i2c_client->adapter, msg, 2);	
	if (rc < 0)
	{
		APS_ERR("I2C_dma_write_Readdata error %d,addr = %d\n", rc,apds9960_i2c_client->addr);
	}
#if defined(_DMA_RW_MODE_)
	else
	{
	   memcpy(data, g_pDMABuf_va, size);
	}
#endif
	apds9960_i2c_client->addr = temp_addr;
}

/*----------------------------------------------------------------------------*/
static int apds9960_clear_interrupt(struct i2c_client *client, int command)
{
	int ret;
		
	ret = i2c_smbus_write_byte(client, command);

	return ret;
}

static int apds9960_set_enable(struct i2c_client *client, int enable)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_ENABLE_REG, enable);

	data->enable = enable;

	return ret;
}

static int apds9960_set_atime(struct i2c_client *client, int atime)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;
	
	ret = i2c_smbus_write_byte_data(client, APDS9960_ATIME_REG, atime);

	data->atime = atime;

	return ret;
}

static int apds9960_set_wtime(struct i2c_client *client, int wtime)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_WTIME_REG, wtime);

	data->wtime = wtime;

	return ret;
}

static int apds9960_set_ailt(struct i2c_client *client, int threshold)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_word_data(client, APDS9960_AILTL_REG, threshold);

	data->ailt = threshold;

	return ret;
}

static int apds9960_set_aiht(struct i2c_client *client, int threshold)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_word_data(client, APDS9960_AIHTL_REG, threshold);

	data->aiht = threshold;

	return ret;
}

static int apds9960_set_pilt(struct i2c_client *client, int threshold)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_PITLO_REG, threshold);

	data->pilt = threshold;

	return ret;
}

static int apds9960_set_piht(struct i2c_client *client, int threshold)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_PITHI_REG, threshold);

	data->piht = threshold;

	return ret;
}

static int apds9960_set_pers(struct i2c_client *client, int pers)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_PERS_REG, pers);

	data->pers = pers;

	return ret;
}

static int apds9960_set_config(struct i2c_client *client, int config)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_CONFIG_REG, config);

	data->config = config;

	return ret;
}

static int apds9960_set_ppulse(struct i2c_client *client, int ppulse)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_PPULSE_REG, ppulse);

	data->ppulse = ppulse;

	return ret;
}

static int apds9960_set_control(struct i2c_client *client, int control)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_CONTROL_REG, control);

	data->control = control;

	return ret;
}

static int apds9960_set_aux(struct i2c_client *client, int aux)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_AUX_REG, aux);

	data->aux = aux;

	return ret;
}

static int apds9960_set_poffset_ur(struct i2c_client *client, int poffset_ur)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_POFFSET_UR_REG, poffset_ur);

	data->poffset_ur = poffset_ur;

	return ret;
}

static int apds9960_set_poffset_dl(struct i2c_client *client, int poffset_dl)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_POFFSET_DL_REG, poffset_dl);

	data->poffset_dl = poffset_dl;

	return ret;
}

/****************** Gesture related registers ************************/
static int apds9960_set_config2(struct i2c_client *client, int config2)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_CONFIG2_REG, config2);

	data->config2= config2;

	return ret;
}

static int apds9960_set_gthr_in(struct i2c_client *client, int gthr_in)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GTHR_IN_REG, gthr_in);

	data->gthr_in = gthr_in;

	return ret;
}

static int apds9960_set_gthr_out(struct i2c_client *client, int gthr_out)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GTHR_OUT_REG, gthr_out);

	data->gthr_out = gthr_out;

	return ret;
}

static int apds9960_set_gconf1(struct i2c_client *client, int gconf1)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GCONF1_REG, gconf1);

	data->gconf1 = gconf1;

	return ret;
}

static int apds9960_set_gconf2(struct i2c_client *client, int gconf2)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GCONF2_REG, gconf2);

	data->gconf2 = gconf2;

	return ret;
}

static int apds9960_set_goffset_u(struct i2c_client *client, int goffset_u)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GOFFSET_U_REG, goffset_u);

	data->goffset_u = goffset_u;

	return ret;
}

static int apds9960_set_goffset_d(struct i2c_client *client, int goffset_d)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GOFFSET_D_REG, goffset_d);

	data->goffset_d = goffset_d;

	return ret;
}

static int apds9960_set_gpulse(struct i2c_client *client, int gpulse)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GPULSE_REG, gpulse);

	data->gpulse = gpulse;

	return ret;
}

static int apds9960_set_goffset_l(struct i2c_client *client, int goffset_l)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GOFFSET_L_REG, goffset_l);

	data->goffset_l = goffset_l;

	return ret;
}

static int apds9960_set_goffset_r(struct i2c_client *client, int goffset_r)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GOFFSET_R_REG, goffset_r);

	data->goffset_r = goffset_r;

	return ret;
}

static int apds9960_set_gconf3(struct i2c_client *client, int gconf3)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GCONF3_REG, gconf3);

	data->gconf3 = gconf3;

	return ret;
}

static int apds9960_set_gctrl(struct i2c_client *client, int gctrl)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;

	ret = i2c_smbus_write_byte_data(client, APDS9960_GCTRL_REG, gctrl);

	data->gctrl = gctrl;

	return ret;
}


static int apds9960_set_psensor_threshold(struct i2c_client *client,int lthreshold,int hthreshold)
{
	struct apds9960_data *obj = i2c_get_clientdata(client);
 
	int res = 0;

#ifdef CUSTOM_KERNEL_SENSORHUB
SCP_SENSOR_HUB_DATA data;
  APDS9960_CUST_DATA *pCustData;
  int len;
  int32_t ps_thd_val_low, ps_thd_val_high;

  ps_thd_val_low = lthreshold;
  ps_thd_val_high = hthreshold;

  ps_thd_val_low -= obj->ps_cali;
  ps_thd_val_high -= obj->ps_cali;

  data.set_cust_req.sensorType = ID_PROXIMITY;
  data.set_cust_req.action = SENSOR_HUB_SET_CUST;
  pCustData = (APDS9960_CUST_DATA *)(&data.set_cust_req.custData);

  pCustData->setPSThreshold.action = APDS9960_CUST_ACTION_SET_PS_THRESHODL;
  pCustData->setPSThreshold.threshold[0] = ps_thd_val_low;
  pCustData->setPSThreshold.threshold[1] = ps_thd_val_high;
  len = offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + sizeof(pCustData->setPSThreshold);

  res = SCP_sensorHub_req_send(&data, &len, 1);

#else

		apds9960_set_pilt(client, lthreshold);		
		apds9960_set_piht(client, hthreshold);	
#endif
		return 1;
}

/*********************************************************************/

static int LuxCalculation(struct i2c_client *client)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int X1, Y1, Z1;
	int x1, y1, z1;
	int n;
	//unsigned int lux;
	unsigned int cct;
	int sum_XYZ=0;

	APS_DBG("phase 0 :: cdata = %d, rdata = %d, gdata = %d, bdata = %d\n", data->cdata, data->rdata, data->gdata, data->bdata);
	APS_DBG("phase 0.5 :: atime = %d, again = %d\n", apds9960_als_integration_tb[data->als_atime_index], apds9960_als_again_tb[data->als_again_index]);

	X1 = (data->RGB_COE_X[0]*data->rdata) + (data->RGB_COE_X[1]*data->gdata) + (data->RGB_COE_X[2]*data->bdata);
	Y1 = (data->RGB_COE_Y[0]*data->rdata) + (data->RGB_COE_Y[1]*data->gdata) + (data->RGB_COE_Y[2]*data->bdata);
	Z1 = (data->RGB_COE_Z[0]*data->rdata) + (data->RGB_COE_Z[1]*data->gdata) + (data->RGB_COE_Z[2]*data->bdata);
 
	//APS_DBG("%s phase 1 :: X1 = %d, Y1 = %d, Z1 = %d\n", __func__, X1, Y1, Z1);
 
	if ((X1 == 0) && (Y1 == 0) && (Z1 == 0))
    {
		x1 = y1 = z1 = 0;
	}
	else
    {	
		sum_XYZ = (X1 + Y1 + Z1)/1000;	// scale down
		if (sum_XYZ > 0)
        {
			if (((X1+Y1+Z1)%1000) >= 500)
				sum_XYZ++;	
		}
		else
        {
			if (((X1+Y1+Z1)%1000) <= -500)
				sum_XYZ--;
		}

		x1 = X1/sum_XYZ;
		y1 = Y1/sum_XYZ;
		z1 = Z1/sum_XYZ;
	}
 
	//APS_DBG("%s phase 2 :: x1 = %d, y1 = %d, z1 = %d, sum = %d\n", __func__, x1, y1, z1, sum_XYZ);

	if (data->cdata > 10)
    {
		n = ((x1 - RGB_CIE_N1)*1000)/(RGB_CIE_N2 - y1);
		cct = (((RGB_CIE_CCT1*(n*n*n))/1000000000) + 
			   ((RGB_CIE_CCT2*(n*n))/1000000) + 
			   ((RGB_CIE_CCT3*n)/1000) + RGB_CIE_CCT4);
	}
	else
    {
		n = 0;
		cct = 0;
	}
 
	data->lux = (data->cdata*10080)/(apds9960_als_integration_tb[data->als_atime_index]*apds9960_als_again_tb[data->als_again_index]);

	data->cct = cct;

	//APS_DBG("%s phase 3 :: n = %d, cct = %d, data->cct = %d, data->lux = %d\n", __func__, n, cct, data->cct, data->lux);
	APS_DBG("GO data->lux_GA1:%d", data->lux_GA1);
	if (data->cdata > 0)
    {
		if(((data->rdata * 100)/data->cdata) >= 65)     // Incandescent 2600K
        {
			data->cct=(data->cct*data->cct_GA2)/1000;
			data->lux=(data->lux*data->lux_GA2)/1000;
		}
		else if (((data->rdata * 100)/data->cdata) >= 45)   // Fluorescent Warm White 2700K
        {
			data->cct=(data->cct*data->cct_GA3)/1000;
			data->lux=(data->lux*data->lux_GA3)/1000;
		}
		else    // Fluorescent Daylight 6500K
        {
			data->cct=(data->cct*data->cct_GA1)/1000;
 			data->lux=(data->lux*data->lux_GA1)/1000;
		}
	}

 
	APS_DBG("%s phase 4 :: cct = %d, data->cct = %d, data->lux = %d\n", __func__, cct, data->cct, data->lux);

	return SUCCESS;
}

/* Gionee BSP1 yang_yang 20140802 modify for CR01341808 begin */
#if defined(GN_MTK_BSP_PS_STATIC_CALIBRATION)
static void apds9960_WriteCalibration(struct PS_CALI_DATA_STRUCT *data_cali)
{
	APS_DBG("apds9930_WriteCalibration  %d,%d,%d\n",data_cali->close,data_cali->far_away,data_cali->valid);
	if (data_cali->valid ==1)
    {
     #ifdef CUSTOM_KERNEL_SENSORHUB
		apds9960_set_psensor_threshold(apds9960_obj->client,data_cali->far_away,data_cali->close);
	#else
		apds9960_set_piht(apds9960_obj->client, data_cali->close);
		apds9960_set_pilt(apds9960_obj->client, data_cali->far_away);
	#endif
	}
	else
    {
    #ifdef CUSTOM_KERNEL_SENSORHUB
		apds9960_set_psensor_threshold(apds9960_obj->client,data_cali->far_away,data_cali->close);
	#else
		apds9960_set_piht(apds9960_obj->client, data_cali->close);
		apds9960_set_pilt(apds9960_obj->client, data_cali->far_away);
	#endif
	}
}
#endif
/* Gionee BSP1 yang_yang 20140802 modify for CR01341808 end */

static void apds9960_change_ps_threshold(struct i2c_client *client)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	//hwm_sensor_data sensor_data;
	int err;
	//int ps_status;

	APS_FUN();
	data->ps_data =	i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
    APS_DBG("ps_data=%d, piht=%d, pilt=%d\n", data->ps_data, data->piht, data->pilt);
	if ((data->ps_data > data->piht) && (data->ps_data > data->pilt))
    {
		/* far-to-near detected */
		ps_status = 0;

		/* setup to detect far now */
	#ifdef CUSTOM_KERNEL_SENSORHUB
		apds9960_set_psensor_threshold(client,data->ps_hysteresis_threshold,APDS9960_FAR_THRESHOLD_HIGH);
	#else		
		apds9960_set_pilt(client, data->ps_hysteresis_threshold);
		apds9960_set_piht(client, APDS9960_FAR_THRESHOLD_HIGH);
	#endif
		APS_DBG("far-to-near detected\n");
	}
	else if ((data->ps_data < data->pilt) && (data->ps_data < data->piht))
    {
		/* near-to-far detected */
		ps_status = 1;

		/* setup to detect near now */
	#ifdef CUSTOM_KERNEL_SENSORHUB
		apds9960_set_psensor_threshold(client,APDS9960_NEAR_THRESHOLD_LOW,data->ps_threshold);
	#else		
		apds9960_set_pilt(client, APDS9960_NEAR_THRESHOLD_LOW);
		apds9960_set_piht(client, data->ps_threshold);
	#endif
		APS_DBG("near-to-far detected\n");
	}
	else if ((data->pilt == APDS9960_FAR_THRESHOLD_HIGH) && 
			  (data->piht == APDS9960_NEAR_THRESHOLD_LOW))  // force interrupt
    {
		/* special case */
        /* Gionee BSP1 chengx 20140905 modify for xxxx begin */
	    if (data->ps_data >= APDS9960_PS_DETECTION_THRESHOLD)
        {
            ps_status = 0;
        }
        else
        {
            ps_status = 1;
        }
        /* Gionee BSP1 chengx 20140905 modify for xxxx begin */

		/* setup to detect near now */
	#ifdef CUSTOM_KERNEL_SENSORHUB
		apds9960_set_psensor_threshold(client,APDS9960_NEAR_THRESHOLD_LOW,data->ps_threshold);
	#else	
		apds9960_set_pilt(client, APDS9960_NEAR_THRESHOLD_LOW);
		apds9960_set_piht(client, data->ps_threshold);
	#endif	
		APS_DBG("first enable! values=%d\n", ps_status);
	}

if ((err =ps_report_interrupt_data(ps_status)))
    {
        APS_ERR("call ps_report_interrupt_data fail = %d\n", err);
	}

}

static void apds9960_change_als_threshold(struct i2c_client *client)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	unsigned char change_again=0;
	unsigned char control_data=0;
	unsigned char i2c_data[16];
	int status;

	status = i2c_smbus_read_i2c_block_data(client, APDS9960_CDATAL_REG, 8, (unsigned char*)i2c_data);

	if (status < 0)
        return;
	if (status != 8)
        return;

	data->cdata = (i2c_data[1]<<8)|i2c_data[0];
	data->rdata = (i2c_data[3]<<8)|i2c_data[2];
	data->gdata = (i2c_data[5]<<8)|i2c_data[4];
	data->bdata = (i2c_data[7]<<8)|i2c_data[6];

	LuxCalculation(client);

	if (data->lux >= 0)
    {
		data->lux = data->lux<30000 ? data->lux : 30000;
		data->als_prev_lux = data->lux;
	}

	if (data->cct >= 0)
    {
		data->cct = data->cct<10000 ? data->cct : 10000;
	}

	APS_DBG("cct=%d, lux=%d cdata=%d rdata=%d gdata=%d bdata=%d atime=%x again=%d\n", 
		data->cct, data->lux, data->cdata, data->rdata, data->gdata, 
		data->bdata, apds9960_als_atime_tb[data->als_atime_index], apds9960_als_again_tb[data->als_again_index]);
	
	data->als_data = data->cdata;

	data->als_threshold_l = (data->als_data * (100-APDS9960_ALS_THRESHOLD_HSYTERESIS) ) /100;
	data->als_threshold_h = (data->als_data * (100+APDS9960_ALS_THRESHOLD_HSYTERESIS) ) /100;

	if (data->als_threshold_h >= apds9960_als_res_tb[data->als_atime_index])
    {
		data->als_threshold_h = apds9960_als_res_tb[data->als_atime_index];
	}

	if (data->als_data >= (apds9960_als_res_tb[data->als_atime_index]*99)/100)
    {
		// lower AGAIN if possible
		if (data->als_again_index != APDS9960_ALS_GAIN_1X)
        {
			data->als_again_index--;
			change_again = 1;
		}
		else
        {
			//input_report_abs(data->input_dev_als, ABS_LIGHT, data->lux); // report lux level
			//input_report_abs(data->input_dev_als, ABS_CCT, data->cct); // report color temperature cct
			//input_sync(data->input_dev_als);
		}
	}
	else if (data->als_data <= (apds9960_als_res_tb[data->als_atime_index]*1)/100)
    {
		// increase AGAIN if possible
	 	if (data->als_again_index != APDS9960_ALS_GAIN_64X)
        {
			data->als_again_index++;
			change_again = 1;
		}
		else
        {
			//input_report_abs(data->input_dev_als, ABS_LIGHT, data->lux); // report lux level
			//input_report_abs(data->input_dev_als, ABS_CCT, data->cct); // report color temperature cct
			//input_sync(data->input_dev_als);
		}
	}
	else
    {
		//input_report_abs(data->input_dev_als, ABS_LIGHT, data->lux); // report lux level
		//input_report_abs(data->input_dev_als, ABS_CCT, data->cct); // report color temperature cct
		//input_sync(data->input_dev_als);
	}

	if (change_again)
    {
		control_data = i2c_smbus_read_byte_data(client, APDS9960_CONTROL_REG);
		control_data = control_data & 0xFC;

		control_data = control_data | apds9960_als_again_bit_tb[data->als_again_index];
		i2c_smbus_write_byte_data(client, APDS9960_CONTROL_REG, control_data);
	}
	
	i2c_smbus_write_word_data(client, APDS9960_AILTL_REG, data->als_threshold_l);
	i2c_smbus_write_word_data(client, APDS9960_AIHTL_REG, data->als_threshold_h);
}

static void apds9960_reschedule_work(struct apds9960_data *data,
					  unsigned long delay)
{
	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&data->psensor_dwork);
	queue_delayed_work(apds_workqueue, &data->psensor_dwork, delay);
}

void ResetGestureParameters()
{
	APS_FUN();

	gesture_data.index = 0;
	gesture_data.total_gestures = 0;
   
	gesture_ud_delta = 0;
	gesture_lr_delta = 0;
	gesture_state = 0;
	gesture_ud_count = 0;
	gesture_lr_count = 0;
	gesture_motion_test	= DIR_NONE;
	gesture_motion = DIR_NONE;
	gesture_prev_motion = DIR_NONE;
	gesture_distance = 0;
	gesture_prev_distance = 0;
	gesture_far_near_flag = 0;
	gesture_far_near_started = 0;
	
	last_ps_data = 0;
}
unsigned int GetTickCount(void)
{
	struct timeval tv;

	do_gettimeofday(&tv);

	return ((tv.tv_sec * 1000) + (tv.tv_usec/1000)); // return in msec
}

int FilterGestureRawData(GESTURE_DATA_TYPE *gesture_in_data, GESTURE_DATA_TYPE *gesture_out_data)
{
	int i;

	if (gesture_in_data->total_gestures > 32 || gesture_in_data->total_gestures <= 0) return -1;

	gesture_out_data->total_gestures = 0;

	for (i=0; i<gesture_in_data->total_gestures; i++)
    {
		if (gesture_in_data->u_data[i] > gesture_in_data->out_threshold &&
			  gesture_in_data->d_data[i] > gesture_in_data->out_threshold &&
			  gesture_in_data->l_data[i] > gesture_in_data->out_threshold &&
			  gesture_in_data->r_data[i] > gesture_in_data->out_threshold)
		{
			gesture_out_data->u_data[gesture_out_data->total_gestures] = gesture_in_data->u_data[i];
			gesture_out_data->d_data[gesture_out_data->total_gestures] = gesture_in_data->d_data[i];
			gesture_out_data->l_data[gesture_out_data->total_gestures] = gesture_in_data->l_data[i];
			gesture_out_data->r_data[gesture_out_data->total_gestures] = gesture_in_data->r_data[i];

			gesture_out_data->total_gestures++;
		}
	}

	if (gesture_out_data->total_gestures == 0)
        return -1;
	
	for (i=1; i<gesture_out_data->total_gestures-1; i++)
    {
		gesture_out_data->u_data[i] = (gesture_out_data->u_data[i]+(gesture_out_data->u_data[i-1])+gesture_out_data->u_data[i+1])/3;
		gesture_out_data->d_data[i] = (gesture_out_data->d_data[i]+(gesture_out_data->d_data[i-1])+gesture_out_data->d_data[i+1])/3;
		gesture_out_data->l_data[i] = (gesture_out_data->l_data[i]+(gesture_out_data->l_data[i-1])+gesture_out_data->l_data[i+1])/3;
		gesture_out_data->r_data[i] = (gesture_out_data->r_data[i]+(gesture_out_data->r_data[i-1])+gesture_out_data->r_data[i+1])/3;
	}

	return 1;
}

int GetZoomState(int zoom_value, int last_zoom_value, int prev_zoom_distance)
{
    if (zoom_value >= last_zoom_value)
    {
        if ((zoom_value - last_zoom_value) < 20)
        {
            return prev_zoom_distance;
        }

        if (zoom_value < 50)
        {
            return ZOOM_1;
        }
        else if (zoom_value < 90)
        {
            return ZOOM_2;
        }
        else if (zoom_value < 130)
        {
            return ZOOM_3;
        }
        else if (zoom_value < 170)
        {
            return ZOOM_4;
        }
        else if (zoom_value < 210)
        {
            return ZOOM_5;
        }
        else if (zoom_value <= 255)
        {
            return ZOOM_6;
        }
        else
        {
            return ZOOM_NA;
        }
    }
    else
    {
        if ((last_zoom_value - zoom_value) < 20)
        {
            return prev_zoom_distance;
        }

        if (zoom_value >= 240)
        {
            return ZOOM_6;
        }
        else if (zoom_value > 200)
        {
            return ZOOM_5;
        }
        else if (zoom_value > 160)
        {
            return ZOOM_4;
        }
        else if (zoom_value > 120)
        {
            return ZOOM_3;
        }
        else if (zoom_value > 80)
        {
            return ZOOM_2;
        }
        else if (zoom_value > 40)
        {
            return ZOOM_1;
        }
        else
        {
            return ZOOM_1;
        }
    }
}

int DecodeMappedGesture(int mapped, int motion) 
{
	APS_DBG("enter mapped:%d,motion:%d\n", mapped, motion);
	if (!mapped)
    {
		return motion;
	}
	else
    {
		switch (motion)
		{
		    case DIR_UP:
			    //return DIR_RIGHT;
			    return DIR_LEFT;
		    case DIR_DOWN:
			    //return DIR_LEFT;
			    return DIR_RIGHT;
		    case DIR_LEFT:
			    //return DIR_UP;
			    return DIR_DOWN;
		    case DIR_RIGHT:
			    //return DIR_DOWN;
			    return DIR_UP;
		    default:
			    return DIR_NONE;
		}
	}
}

int DecodeGesture(int gesture_mode)
{
	if (gesture_state == NEAR_STATE)
    {
		gesture_motion = DIR_FORWARD;
		return 1;
	}
	else if (gesture_state == FAR_STATE)
    {
		gesture_motion = DIR_BACKWARD;
		return 1;
	}

	if (gesture_ud_count == -1 &&
		gesture_lr_count == 0 )
    {
		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_UP); 
		}
	}
	else if (gesture_ud_count == 1 &&
			  gesture_lr_count == 0)
    {
		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_DOWN); 
		}
	}
	else if (gesture_ud_count == 0 &&
			  gesture_lr_count == 1)
    {

		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_RIGHT); 
		}
	}
	else if (gesture_ud_count == 0 &&
			  gesture_lr_count == -1)
    {
		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_LEFT); 
		}
	}
	else if (gesture_ud_count == -1 &&
			  gesture_lr_count == 1)
    {
		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			if (abs(gesture_ud_delta) > abs(gesture_lr_delta))
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_UP); 
			}
			else
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_RIGHT); 
			}
		}
	}
	else if (gesture_ud_count == 1 &&
			  gesture_lr_count == -1)
    {
		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			if (abs(gesture_ud_delta) > abs(gesture_lr_delta))
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_DOWN); 
			}
			else
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_LEFT); 
			}
		}
	}
	else if (gesture_ud_count == -1 &&
			  gesture_lr_count == -1)
    {
		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			if (abs(gesture_ud_delta) > abs(gesture_lr_delta))
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_UP); 
			}
			else
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_LEFT); 
			}
		}
	}
	else if (gesture_ud_count == 1 &&
			  gesture_lr_count == 1)
    {
		if ((gesture_prev_motion !=DIR_BACKWARD) && 
			(gesture_prev_motion !=DIR_FORWARD))
        {
			if (abs(gesture_ud_delta) > abs(gesture_lr_delta))
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_DOWN); 
			}
			else
            {
				gesture_motion = DecodeMappedGesture(fMotionMapped, DIR_RIGHT); 
			}
		}
	}
	else
    {
	    gesture_motion = DIR_NONE;
		return -1;
	}

	return 1;
}

int GestureDataProcessing()
{
	GESTURE_DATA_TYPE gesture_out_data;
	int ud_delta, lr_delta;
	int sensitivity1_threshold=GESTURE_SENSITIVITY_LEVEL2;
	int gesture_u_d_ratio_first, gesture_u_d_ratio_last;
	int gesture_l_r_ratio_first, gesture_l_r_ratio_last;

	if (gesture_data.total_gestures <= 6)
        return -1;

	/************** This is to detect fundamentals gesture ****************************/
	gesture_data.in_threshold = GESTURE_GTHR_IN;   
	gesture_data.out_threshold = GESTURE_GTHR_OUT-10;

	FilterGestureRawData(&gesture_data, &gesture_out_data);	// for fundamental 

	if (gesture_out_data.total_gestures == 0)
        return -1;
	if (gesture_out_data.total_gestures < 6)
        return -1;

	if (gesture_out_data.u_data[0]==0)
        gesture_out_data.u_data[0] = 1;
	if (gesture_out_data.d_data[0]==0)
        gesture_out_data.d_data[0] = 1;
	if (gesture_out_data.l_data[0]==0)
        gesture_out_data.l_data[0] = 1;
	if (gesture_out_data.r_data[0]==0)
        gesture_out_data.r_data[0] = 1;

	if (gesture_out_data.u_data[gesture_out_data.total_gestures-1]==0)
        gesture_out_data.u_data[gesture_out_data.total_gestures-1] = 1;
	if (gesture_out_data.d_data[gesture_out_data.total_gestures-1]==0)
        gesture_out_data.d_data[gesture_out_data.total_gestures-1] = 1;
	if (gesture_out_data.l_data[gesture_out_data.total_gestures-1]==0)
        gesture_out_data.l_data[gesture_out_data.total_gestures-1] = 1;
	if (gesture_out_data.r_data[gesture_out_data.total_gestures-1]==0)
        gesture_out_data.r_data[gesture_out_data.total_gestures-1] = 1;

	gesture_u_d_ratio_first = (gesture_out_data.u_data[0] - gesture_out_data.d_data[0])*100/(gesture_out_data.u_data[0]+gesture_out_data.d_data[0]);
	gesture_l_r_ratio_first = (gesture_out_data.l_data[0] - gesture_out_data.r_data[0])*100/(gesture_out_data.l_data[0]+gesture_out_data.r_data[0]);
	gesture_u_d_ratio_last = (gesture_out_data.u_data[gesture_out_data.total_gestures-1] - gesture_out_data.d_data[gesture_out_data.total_gestures-1])*100/(gesture_out_data.u_data[gesture_out_data.total_gestures-1]+gesture_out_data.d_data[gesture_out_data.total_gestures-1]);
	gesture_l_r_ratio_last = (gesture_out_data.l_data[gesture_out_data.total_gestures-1] - gesture_out_data.r_data[gesture_out_data.total_gestures-1])*100/(gesture_out_data.l_data[gesture_out_data.total_gestures-1]+gesture_out_data.r_data[gesture_out_data.total_gestures-1]);

	ud_delta = (gesture_u_d_ratio_last - gesture_u_d_ratio_first);
	lr_delta = (gesture_l_r_ratio_last - gesture_l_r_ratio_first);

	gesture_ud_delta = ud_delta + gesture_ud_delta;
	gesture_lr_delta = lr_delta + gesture_lr_delta;

	/**************** for Left/Right/Up/Down ****************/
	if (gesture_ud_delta >= sensitivity1_threshold)
    {
		gesture_ud_count = 1;
	}
	else if (gesture_ud_delta <= -sensitivity1_threshold)
    {
		gesture_ud_count = -1;
	}
	else
    {
		gesture_ud_count = 0;
	}
	//APS_DBG("gesture_ud_count:%d\n", gesture_ud_count);
	if (gesture_lr_delta >= sensitivity1_threshold)
    {
		gesture_lr_count = 1;
	}
	else if (gesture_lr_delta <= -sensitivity1_threshold)
    {
		gesture_lr_count = -1;
	}
	else
		gesture_lr_count = 0;

	if ((gesture_lr_count != 0) && (gesture_ud_count != 0))
	{
		if (abs(gesture_lr_delta) > abs(gesture_ud_delta))
		{
			if (abs(gesture_lr_delta) > (2*abs(gesture_ud_delta)))
			{
				gesture_ud_count = 0;
			}
		}
		else
		{
			if (abs(gesture_ud_delta) > (2*abs(gesture_lr_delta)))
			{
				gesture_lr_count = 0;
			}     
		} 
	}

	/**************** for Left/Right/Up/Down ****************/
	if (last_ps_data == 0)
	{
		last_ps_data = ps_data;
		return 1;
	}
	else
	{
		if (gesture_ud_count == 0 && gesture_lr_count == 0)
		{
			gesture_far_near_flag++;
			if (gesture_far_near_flag == 3)
				gesture_far_near_started = 1;
		}
		else if (ps_data >= 255)
		{
			gesture_far_near_flag++;
			if (gesture_far_near_flag == 3)
				gesture_far_near_started = 1;
		}
		else if (gesture_state == NA_STATE && !gesture_far_near_started)
			return 1;
	}

	if ((gesture_far_near_started == 1) || (gesture_state == FAR_STATE || gesture_state == NEAR_STATE))
	{
		gesture_distance = GetZoomState(ps_data, last_ps_data, gesture_prev_distance);
		
		if (gesture_prev_distance != gesture_distance)
		{
			last_ps_data = ps_data;

			if (gesture_distance > gesture_prev_distance)
			{
				gesture_prev_distance = gesture_distance;
				gesture_state = NEAR_STATE;
				return 1;
			}
			else
			{
				gesture_prev_distance = gesture_distance;
				gesture_state = FAR_STATE;
				return 1;
			}
		}
		else if (ps_data >= 255)
		{
			last_ps_data = ps_data; 
			gesture_ud_count = 0;
			gesture_lr_count = 0;
			gesture_ud_delta = 0;
			gesture_lr_delta = 0;
			gesture_prev_motion = DIR_NONE;
			gesture_motion = DIR_NONE;
			gesture_state = NA_STATE;
		}
	}
  	
    return 1;
}

static void apds9960_gesture_processing(struct i2c_client *client)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int gstatus;
	int gfifo_level;
	int gfifo_read;
	unsigned char gfifo_data[128];
	int i, j;
	unsigned char overflow_flag = 0;
	int value;
	hwm_sensor_data sensor_data;
    int gfifo0, gfifo1, gfifo2, gfifo3;

    APS_FUN();
	__cancel_delayed_work(&data->gesture_dwork);
	flush_delayed_work(&data->gesture_dwork);

	/* need to loop gstatus until fifo is empty */
	gesture_start_time = GetTickCount();
	dma_buffer_alloct();


	ps_data = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
    APS_DBG("APDS9960_PDATA_REG(0x9C):0x%x\n", ps_data);

	gstatus = i2c_smbus_read_byte_data(client, APDS9960_GSTATUS_REG);
	if (gstatus < 0)
	{
		APS_DBG("(%d): exit 1\n", gstatus);
		dma_buffer_release();
		return;
	}

	if ((gstatus & APDS9960_GFIFO_OV) == APDS9960_GFIFO_OV)
		overflow_flag = 1;

	if ((gstatus & APDS9960_GVALID) == APDS9960_GVALID)
	{
	    apds9960_set_gctrl(client,0x02);
		gfifo_level = i2c_smbus_read_byte_data(client, APDS9960_GFIFO_LVL_REG);	
		APS_DBG("gfifo_level = %d\n", gfifo_level);
		gfifo_read = gfifo_level*4;
		if (gfifo_level > 0)
		{
			I2C_dma_write_Readdata(0xFC, (u8*)gfifo_data, gfifo_read);
			APS_DBG("gfifo_read = %d\n", gfifo_read);

			if (gfifo_read >= 4)
			{
				if (overflow_flag && gfifo_read==128)
					j = 8;
				else
					j = 0;

				for (i=j; i<gfifo_read; i+=4)
                {
					gesture_data.u_data[gesture_data.index] = gfifo_data[i+0];
					gesture_data.d_data[gesture_data.index] = gfifo_data[i+1];
					gesture_data.l_data[gesture_data.index] = gfifo_data[i+2];
					gesture_data.r_data[gesture_data.index] = gfifo_data[i+3];
					gesture_data.index++;
					gesture_data.total_gestures++;
				}
					
				if (GestureDataProcessing() > 0)
				{
					if (DecodeGesture(gesture_fundamentals) > 0)
					{
						/* this is to detect far/near/tilt/circle gestures */
						if (gesture_prev_motion != gesture_motion && gesture_motion != DIR_NONE)
						{
							/* event->value
							 PS_NEAR = 0 
							 DIR_LEFT = 1
							 DIR_RIGHT =2
							 DIR_UP = 3
							 DIR_DOWN = 4
							 DIR_FORWARD = 5
							 DIR_BACKWARD = 6
							 DIR_ZOOM_1 = 7
							 DIR_ZOOM_2 = 8
							 DIR_ZOOM_3 = 9
							 DIR_ZOOM_4 = 10
							 DIR_ZOOM_5 = 11
							 DIR_ZOOM_6 = 12
							 PS_FAR = 20
							*/

							if (gesture_motion == DIR_BACKWARD)
							{
				                gesture_motion = ((DIR_ZOOM_1-1)+gesture_distance); 
							
								gesture_motion_test = gesture_motion;
								APS_LOG("%s %d gesture_motion:%d\n", __func__, __LINE__, gesture_motion);
								sensor_data.values[0] = gesture_motion;
								sensor_data.value_divide = 1;
								sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
								//hwmsen_get_interrupt_data(ID_GESTURE, &sensor_data);

								gesture_motion = DIR_BACKWARD; 
								gesture_prev_motion = gesture_motion;
							}
							else if (gesture_motion == DIR_FORWARD)
							{
				                gesture_motion = ((DIR_ZOOM_1-1)+gesture_distance);

								gesture_motion_test = gesture_motion;
								APS_LOG("%s %d gesture_motion:%d\n", __func__, __LINE__, gesture_motion);
								sensor_data.values[0] = gesture_motion;
								sensor_data.value_divide = 1;
								sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
							//	hwmsen_get_interrupt_data(ID_GESTURE, &sensor_data);

								gesture_motion = DIR_FORWARD; 
								gesture_prev_motion = gesture_motion;
							}
							else
							{
								gesture_prev_motion = gesture_motion;
							}	
						}
			            else if (gesture_motion == DIR_FORWARD)
						{
							if (gesture_prev_motion == DIR_FORWARD || gesture_prev_motion == DIR_BACKWARD)
							{
								gesture_motion = ((DIR_ZOOM_1-1)+gesture_distance); 

								gesture_motion_test = gesture_motion;
								APS_LOG("%s %d gesture_motion:%d\n", __func__, __LINE__, gesture_motion);
								sensor_data.values[0] = gesture_motion;
								sensor_data.value_divide = 1;
								sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
								//hwmsen_get_interrupt_data(ID_GESTURE, &sensor_data);
								
								gesture_motion = DIR_FORWARD; 
								gesture_prev_motion = gesture_motion;
							}
						}
						else if (gesture_motion == DIR_BACKWARD)
						{
							if (gesture_prev_motion == DIR_FORWARD || gesture_prev_motion == DIR_BACKWARD)
							{
								gesture_motion = ((DIR_ZOOM_1-1)+gesture_distance); 
								
								gesture_motion_test = gesture_motion;
								APS_LOG("%s %d gesture_motion:%d\n", __func__, __LINE__, gesture_motion);
								sensor_data.values[0] = gesture_motion;
								sensor_data.value_divide = 1;
								sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
							//	hwmsen_get_interrupt_data(ID_GESTURE, &sensor_data);

								gesture_motion = DIR_BACKWARD; 
								gesture_prev_motion = gesture_motion;
							}
						}
					}
				}

				gesture_data.index = 0;
				gesture_data.total_gestures = 0;

				/* set timer - to report gesture only after hand is over */
				__cancel_delayed_work(&data->gesture_dwork);
				flush_delayed_work(&data->gesture_dwork);
				queue_delayed_work(apds_workqueue, &data->gesture_dwork, msecs_to_jiffies(100));
			}
		}
	}

	dma_buffer_release();
}

/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
/* kk 08-Jul-2014, Crosstalk monitorinng routine */
/* Lgc 08-Jul-2014, Xtalk */
static void apds9960_xtalk_monitoring_handler(struct work_struct *work)
{
	struct apds9960_data *data = container_of(work, struct apds9960_data, xtalk_dwork.work);
	struct i2c_client *client=data->client;
	unsigned int pdata;
	unsigned int delta_max,delta_min,delta_ava;
	int i;
	
	if (data->enable_gesture_sensor != APDS_ENABLE_GESTURE)
	{
	    APS_ERR("Gesture sensor is not actived!\n");
		return;
    }
    
	pdata = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);

	xtalk_pdata[xtalk_pdata_index++] = pdata;

	if (xtalk_pdata_index >= 5)
	{
		delta_ava = xtalk_pdata[0];
		delta_max = delta_min = xtalk_pdata[0];
		for (i=1; i<5; i++)
		{
			APS_DBG("[%d]=0x%x\n", i, xtalk_pdata[i]);
			if(xtalk_pdata[i]>delta_max)
				 delta_max = xtalk_pdata[i];
			if(xtalk_pdata[i]<delta_min)
				 delta_min = xtalk_pdata[i];
		
			delta_ava += xtalk_pdata[i];
		}
		delta_ava /= 5;
		if (((delta_ava - delta_min) < 10) && ((delta_max - delta_ava) < 10)
                && (delta_ava > APDS9960_PS_CALIBRATED_XTALK) && 
                ((abs(delta_ava-APDS9960_PS_CALIBRATED_XTALK)) < APDS9960_CAL_THRESHOLD))
        {
			apds9960_Enable_runtime_calibration(client);
		}

		xtalk_pdata_index = 0;
	}

	/* restart timer */
	queue_delayed_work(apds_workqueue, &data->xtalk_dwork, msecs_to_jiffies(APDS9960_XTALK_MONITORING_TIMER_VALUE));
}
/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */

/* Gesture Reporting routine */
static void apds9960_gesture_reporting_handler(struct work_struct *work)
{
	struct apds9960_data *data = container_of(work, struct apds9960_data, gesture_dwork.work);
	struct i2c_client *client=data->client;
	hwm_sensor_data sensor_data;
	
	if (data->enable_gesture_sensor != APDS_ENABLE_GESTURE)
	{
	    APS_ERR("Gesture sensor is not actived!\n");
		return;
    }
    
	/* TODO - DO NOT need this, this is executed in Timer */
	if (gesture_motion == DIR_LEFT ||
		gesture_motion == DIR_RIGHT ||
		gesture_motion == DIR_UP ||
		gesture_motion == DIR_DOWN)
	{
		if ((gesture_prev_motion != DIR_FORWARD) || (gesture_prev_motion != DIR_BACKWARD))
		{
			gesture_motion_test = gesture_motion;
			APS_LOG("%s %d gesture_motion:%d\n", __func__, __LINE__, gesture_motion);
			sensor_data.values[0] = gesture_motion;
			sensor_data.value_divide = 1;
			sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
			//hwmsen_get_interrupt_data(ID_GESTURE, &sensor_data);
		}
	}
	
	ResetGestureParameters();
	apds9960_set_gctrl(client, 0x06);	
}

/* ALS_PS interrupt routine */
static void apds9960_work_handler(struct work_struct *work)
{
#ifdef CUSTOM_KERNEL_SENSORHUB
		int res = 0;
	
		res = ps_report_interrupt_data(ps_status);
		if(res != 0)
		{
			APS_ERR("epl2182_eint_work err: %d\n", res);
		}
#else

	struct apds9960_data *data = container_of(work, struct apds9960_data, psensor_dwork.work);
	struct i2c_client *client=data->client;
	int status;

	status = i2c_smbus_read_byte_data(client, APDS9960_STATUS_REG);

	APS_DBG("==>isr : status=%x, enable=%x\n", status, data->enable);

	if ((status & APDS9960_STATUS_GINT) && 
		 (data->enable & (APDS9960_GESTURE_ENABLE|APDS9960_PWR_ON)) && 
		 (data->gctrl & 0x02))
    {
		/* Gesture is enabled with interrupte */
		//apds9960_gesture_processing(client);
		//return;
	}
	
	if ((status & APDS9960_STATUS_PINT) && 
		 ((data->enable & (APDS9960_PS_INT_ENABLE|APDS9960_PS_ENABLE|APDS9960_PWR_ON)) == 
            (APDS9960_PS_INT_ENABLE|APDS9960_PS_ENABLE|APDS9960_PWR_ON)) )
    {
		/* PS is interrupted */

		/* check if this is triggered by background ambient noise */
		if (status & APDS9960_STATUS_PSAT)
        {
			APS_DBG("PS is triggered by background ambient noise\n");
		}
		else if (status & APDS9960_STATUS_PVALID)
        {
			apds9960_change_ps_threshold(client);		
		}
	}
	
	if ((status & APDS9960_STATUS_AINT) && 
		 ((data->enable & (APDS9960_ALS_INT_ENABLE|APDS9960_ALS_ENABLE|APDS9960_PWR_ON)) == 
            (APDS9960_ALS_INT_ENABLE|APDS9960_ALS_ENABLE|APDS9960_PWR_ON)))
    {
		/* ALS is interrupted */	
		/* check if this is triggered by background ambient noise */
		if (status & APDS9960_STATUS_ASAT)
        {
			APS_DBG("ALS is saturated\n");
		}
		else if (status & APDS9960_STATUS_AVALID)
        {
			apds9960_change_als_threshold(client);
		}
	}
	
	apds9960_clear_interrupt(client, CMD_CLR_ALL_INT);
	mt_eint_unmask(CUST_EINT_ALS_NUM); 
#endif	
}

/* assume this is ISR */
#ifndef CUSTOM_KERNEL_SENSORHUB

void apds9960_interrupt()
{
	struct apds9960_data *data = g_apds9960_ptr;
	mt_eint_mask(CUST_EINT_ALS_NUM);
	if (!data)
	{
        APS_ERR("data is NULL!\n");
		return;
	}
	APS_DBG("==> apds9960_interrupt\n");
	//apds9960_reschedule_work(data, 0);	/* ensure PS cycle is completed if ALS interrupt asserts */	
	  schedule_delayed_work(&data->psensor_dwork,100);
	APS_DBG("<== apds9960_interrupt\n");
}
#else
static int alsps_irq_handler(void* data, uint len)
{
	struct apds9960_data *obj = apds9960_obj;
    SCP_SENSOR_HUB_DATA_P rsp = (SCP_SENSOR_HUB_DATA_P)data;
    
	if(!obj)
	{
		return -1;
	}

    APS_LOG("len = %d, type = %d, sction = %d, event = %d\n", len, rsp->rsp.sensorType, rsp->rsp.action, rsp->rsp.errCode);

	switch(rsp->rsp.action)
    {
        case SENSOR_HUB_NOTIFY:
            switch(rsp->notify_rsp.event)
            {
                case SCP_INIT_DONE:
                    schedule_delayed_work(&obj->init_done_work,1);
                    break;
                case SCP_NOTIFY:
                    if (APDS9960_NOTIFY_PROXIMITY_CHANGE == rsp->notify_rsp.data[0])
                    {
                        ps_status = rsp->notify_rsp.data[1];
						APS_LOG("ps_status=%d\n",ps_status);
						schedule_delayed_work(&obj->psensor_dwork,0);
                    }
					else if(APDS9960_NOTIFY_PROXIMITY_NOT_CHANGE == rsp->notify_rsp.data[0])
					{
                        ps_status = rsp->notify_rsp.data[1];
						//schedule_work(&obj->data_work);
					}
                    else
                    {
                        APS_ERR("Unknow notify\n");
                    }
                    break;
                default:
                    APS_ERR("Error sensor hub notify\n");
                    break;
            }
            break;
        default:
            APS_ERR("Error sensor hub action\n");
            break;
    }

    return 0;
}

#endif
/*
 * IOCTL support
 */
static int apds9960_enable_als_sensor(struct i2c_client *client, int val)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
 	
	APS_LOG("%s: enable als sensor (%d)(enable=%d)\n", __func__, val,data->enable_als_sensor);
	
	if ((APDS_DISABLE_ALS != val) && (APDS_ENABLE_ALS_WITH_INT != val) && (APDS_ENABLE_ALS_NO_INT != val))
    {
		APS_ERR("enable als sensor=%d\n", val);
		return -EINVAL;
	}
	
	if ((APDS_ENABLE_ALS_WITH_INT == val) || (APDS_ENABLE_ALS_NO_INT == val))
    {
		/* turn on light  sensor */
		if (APDS_DISABLE_ALS == data->enable_als_sensor)
        {
			data->enable_als_sensor = val;
			apds9960_set_enable(client, APDS9960_PWR_DOWN); // Power Off
				
			if (APDS_ENABLE_ALS_NO_INT == data->enable_als_sensor)
            {	
				if (data->enable_ps_sensor)
                {
					apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|
                                        APDS9960_PS_ENABLE|APDS9960_PS_INT_ENABLE);  // Enable PS with interrupt
				}
				else
                {
					apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE);  // no interrupt
				}
			}
			else    // als with int
            {
				apds9960_set_ailt(client, APDS9960_FAR_THRESHOLD_HIGH);      // force first ALS interrupt in order to get environment reading
				apds9960_set_aiht(client, APDS9960_NEAR_THRESHOLD_LOW);
	
				if (data->enable_ps_sensor)
                {
					apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|APDS9960_PS_ENABLE|
                                        APDS9960_ALS_INT_ENABLE|APDS9960_PS_INT_ENABLE);  // Enable both ALS and PS with interrupt
				}
				else
                {
                    /* only enable light sensor with interrupt */
					apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|APDS9960_ALS_INT_ENABLE);
				}
			}		
		}
	}
	else
    {
		/* turn off light sensor */
		/* what if the p sensor is active? */
		data->enable_als_sensor = val;

		if ((APDS_ENABLE_PS == data->enable_ps_sensor) &&
			 (APDS_DISABLE_GESTURE == data->enable_gesture_sensor))
        {
			apds9960_set_enable(client, APDS9960_PWR_DOWN); // Power Off
		#ifdef CUSTOM_KERNEL_SENSORHUB
			apds9960_set_psensor_threshold(client,APDS9960_FAR_THRESHOLD_HIGH,APDS9960_NEAR_THRESHOLD_LOW);
		#else	
			apds9960_set_pilt(client, APDS9960_FAR_THRESHOLD_HIGH);		
			apds9960_set_piht(client, APDS9960_NEAR_THRESHOLD_LOW);
		#endif
            /* only enable prox sensor with interrupt */
			apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE|APDS9960_PS_INT_ENABLE);
		}
		else if ((APDS_ENABLE_PS == data->enable_ps_sensor) &&
			 	  (APDS_ENABLE_GESTURE == data->enable_gesture_sensor))
        {
            /* only enable gesture sensor with ps */
			apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE|APDS9960_GESTURE_ENABLE);
		}
		else
        {
			apds9960_set_enable(client, APDS9960_PWR_DOWN);
		}						
	}
	
	return SUCCESS;
}

static int apds9960_set_als_poll_delay(struct i2c_client *client, unsigned int val)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int ret;
	int atime_index=0;
 	
	APS_DBG("%d\n", val);

	if ((APDS_ALS_POLL_SLOW != val) && (APDS_ALS_POLL_MEDIUM != val) && (APDS_ALS_POLL_FAST != val))
    {
		APS_ERR("invalid value=%d\n", val);
		return -EINVAL;
	}
	
	if (APDS_ALS_POLL_FAST == val)
    {
		data->als_poll_delay = 50;		// 50ms
		atime_index = APDS9960_ALS_RES_24MS;
	}
	else if (APDS_ALS_POLL_MEDIUM == val)
    {
		data->als_poll_delay = 200;		// 200ms
		atime_index = APDS9960_ALS_RES_50MS;
	}
	else	// APDS_ALS_POLL_SLOW
    {
		data->als_poll_delay = 1000;		// 1000ms
		atime_index = APDS9960_ALS_RES_100MS;
	}

	ret = apds9960_set_atime(client, apds9960_als_atime_tb[atime_index]);
	if (ret >= 0)
    {
		data->als_atime_index = atime_index;
		APS_DBG("poll delay %d, atime_index %d\n", data->als_poll_delay, data->als_atime_index);
	}
	else
		return -EIO;
		
	return SUCCESS;
}

static int apds9960_enable_ps_sensor(struct i2c_client *client, int val)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	APS_DBG("enable ps senosr (%d)\n", val);
	if ((APDS_DISABLE_PS != val) && (APDS_ENABLE_PS != val))
    {
		APS_ERR("invalid value=%d\n", val);
		return -EINVAL;
	}
	
	if (APDS_ENABLE_PS == val)      // interrupt mode only
    {
		/* turn on p sensor */
		if (APDS_DISABLE_PS == data->enable_ps_sensor)
        {
			data->enable_ps_sensor= APDS_ENABLE_PS;
		
			apds9960_set_enable(client, APDS9960_PWR_DOWN); // Power Off
		
			apds9960_set_ppulse(client, data->ps_ppulse);

			apds9960_set_control(client, APDS9960_PDRVIE_FOR_PS|APDS9960_PGAIN_FOR_PS|apds9960_als_again_bit_tb[data->als_again_index]);

			apds9960_set_poffset_ur(client, data->ps_poffset_ur);
			apds9960_set_poffset_dl(client, data->ps_poffset_dl);

			/* force first interrupt to inform HAL */
	#ifdef CUSTOM_KERNEL_SENSORHUB
			apds9960_set_psensor_threshold(client,APDS9960_FAR_THRESHOLD_HIGH,APDS9960_NEAR_THRESHOLD_LOW);
	#else
			apds9960_set_pilt(client, APDS9960_FAR_THRESHOLD_HIGH);		
			apds9960_set_piht(client, APDS9960_NEAR_THRESHOLD_LOW);
	#endif		
			apds9960_set_aux(client, APDS9960_PS_LED_BOOST|0x01);

			apds9960_change_ps_threshold(client);       // jing.zhao@qingcheng.com add force first input
			/* disable gesture if it was enabled previously */
			if (APDS_DISABLE_ALS == data->enable_als_sensor)
            {
				apds9960_set_wtime(client, APDS9960_246_WAIT_TIME);
                /* only enable PS interrupt */
				apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE|APDS9960_PS_INT_ENABLE);
			}
			else if (APDS_ENABLE_ALS_WITH_INT == data->enable_als_sensor)
            {
                /* enable ALS and PS interrupt */
				apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|APDS9960_PS_ENABLE|
                                        APDS9960_ALS_INT_ENABLE|APDS9960_PS_INT_ENABLE);
			}
			else    // APDS_ENABLE_ALS_NO_INT
            {
                /* enable PS interrupt only */
				apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|APDS9960_PS_ENABLE|APDS9960_PS_INT_ENABLE);
			}
		}
	} 
	else
    {
		/* turn off p sensor - can't turn off the entire sensor, the light sensor may be needed by HAL */
		data->enable_ps_sensor = APDS_DISABLE_PS;
		if ((APDS_ENABLE_ALS_NO_INT == data->enable_als_sensor) &&
			 (APDS_DISABLE_GESTURE == data->enable_gesture_sensor))
        {
			apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE);          // no ALS interrupt

		}
		else if ((APDS_ENABLE_ALS_WITH_INT == data->enable_als_sensor) &&
				  (APDS_DISABLE_GESTURE == data->enable_gesture_sensor))
        {
			/* reconfigute light sensor setting */	
			apds9960_set_enable(client, APDS9960_PWR_DOWN);              // Power Off
			apds9960_set_ailt( client, APDS9960_FAR_THRESHOLD_HIGH);         // Force ALS interrupt
			apds9960_set_aiht( client, APDS9960_NEAR_THRESHOLD_LOW);
						
			apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|APDS9960_ALS_INT_ENABLE);          // enable ALS interrupt
		}
		else if ((APDS_DISABLE_ALS == data->enable_als_sensor) &&
				  (APDS_DISABLE_GESTURE == data->enable_gesture_sensor))
        {
			apds9960_set_enable(client, APDS9960_PWR_DOWN);			
		}
		else if (APDS_ENABLE_GESTURE == data->enable_gesture_sensor)
        {
			apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE|APDS9960_GESTURE_ENABLE);		
		}
	}
	
	return SUCCESS;
}

static int apds9960_enable_gesture_sensor(struct i2c_client *client, int val)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
 	//int value; //test
	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
	int goffset_u, goffset_d, goffset_l, goffset_r, goffset_ur, goffset_dl;
	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */

	APS_DBG("enable gesture senosr (%d)\n", val);
	
	if ((APDS_DISABLE_GESTURE != val) && (APDS_ENABLE_GESTURE != val))
    {
		APS_ERR("invalid value=%d\n", val);
		return -EINVAL;
	}
	
    //value = i2c_smbus_read_byte_data(client, APDS9960_ID_REG);
    //APS_DBG("ID:0x%x\n", value);
	if (APDS_ENABLE_GESTURE == val)     // interrupt mode only
    {
		apds9960_Enable_runtime_calibration(client);    //cal gesture sensor
		if (APDS_DISABLE_GESTURE == data->enable_gesture_sensor)
        {
			data->enable_gesture_sensor= APDS_ENABLE_GESTURE;

			//apds9960_calibration(client, 2);  //cal gesture sensor

			goffset_u = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_U_REG);
			goffset_d = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_D_REG);
			goffset_l = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_L_REG);
			goffset_r = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_R_REG);

			/*Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
			goffset_ur = i2c_smbus_read_byte_data(client, APDS9960_POFFSET_UR_REG);
			goffset_dl = i2c_smbus_read_byte_data(client, APDS9960_POFFSET_DL_REG);

			APS_DBG("GO GOFFSET_U(0x%x)\n", goffset_u);
            APS_DBG("GO GOFFSET_D(0x%x)\n", goffset_d);
            APS_DBG("GO GOFFSET_L(0x%x)\n", goffset_l);
            APS_DBG("GO GOFFSET_R(0x%x)\n", goffset_r);
            APS_DBG("GO GOFFSET_L(0x%x)\n", goffset_ur);
            APS_DBG("GO GOFFSET_R(0x%x)\n", goffset_dl);
			/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */

			ResetGestureParameters();
			apds9960_set_enable(client, APDS9960_PWR_DOWN);          // Power Off
			apds9960_set_wtime(client, APDS9960_255_WAIT_TIME);
			apds9960_set_ppulse(client, data->gesture_ppulse);
			apds9960_set_control(client, APDS9960_PDRVIE_FOR_GESTURE|APDS9960_PGAIN_FOR_GESTURE|apds9960_als_again_bit_tb[data->als_again_index]);
			apds9960_set_poffset_ur(client, data->gesture_poffset_ur);
			apds9960_set_poffset_dl(client, data->gesture_poffset_dl);
			apds9960_set_aux(client, APDS9960_GESTURE_LED_BOOST|0x01);
			apds9960_set_gctrl(client, 0x07);       // gesture registers
			
			if (APDS_ENABLE_ALS_NO_INT == data->enable_als_sensor)
            {
				/* need to turn on p sensor for gesture mode */
				apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|APDS9960_PS_ENABLE|APDS9960_GESTURE_ENABLE);
			}
			else if (data->enable_als_sensor == APDS_ENABLE_ALS_WITH_INT)
            {
				/* need to turn on p sensor for gesture mode */
				apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_ALS_ENABLE|
                                        APDS9960_PS_ENABLE|APDS9960_ALS_INT_ENABLE|APDS9960_GESTURE_ENABLE);				
			}
			else  // APDS_DISABLE_ALS
            {
				/* need to turn on p sensor for gesture mode */
				apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_WAIT_ENABLE|APDS9960_PS_ENABLE|APDS9960_GESTURE_ENABLE);
			}

/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
			/* kk 08-Jul-2014, start timer here */
			xtalk_pdata_index = 0;
			__cancel_delayed_work(&data->xtalk_dwork);
			flush_delayed_work(&data->xtalk_dwork);
			queue_delayed_work(apds_workqueue, &data->xtalk_dwork, msecs_to_jiffies(APDS9960_XTALK_MONITORING_TIMER_VALUE));
/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */	
		}
	} 
	else
    {
		/* turn off gesture sensor - can't turn off the entire sensor, the light/proximity sensor may be needed by HAL */
		data->enable_gesture_sensor = APDS_DISABLE_GESTURE;
		if ((APDS_ENABLE_ALS_NO_INT == data->enable_als_sensor) &&
			 (APDS_DISABLE_PS == data->enable_ps_sensor))
        {
			apds9960_set_enable(client, 0x03);  // no ALS interrupt	
		}
		else if ((APDS_ENABLE_ALS_WITH_INT == data->enable_als_sensor) && 
				  (APDS_DISABLE_PS == data->enable_ps_sensor))
        {
			/* reconfigute light sensor setting */	
			apds9960_set_enable(client, APDS9960_PWR_DOWN);      // Power Off
			apds9960_set_ailt( client, 0xFFFF);	// Force ALS interrupt
			apds9960_set_aiht( client, 0);
						
			apds9960_set_enable(client, 0x13);  // enable ALS interrupt
		}
		else if ((APDS_DISABLE_ALS == data->enable_als_sensor) && 
				  (APDS_DISABLE_PS == data->enable_ps_sensor))
        {
			apds9960_set_enable(client, APDS9960_PWR_DOWN);
		}
		else if ((APDS_ENABLE_ALS_NO_INT == data->enable_als_sensor) &&
			 	  (APDS_ENABLE_PS == data->enable_ps_sensor))
        {
			apds9960_set_enable(client, 0x27);  // no ALS interrupt
		}
		else if ((APDS_ENABLE_ALS_WITH_INT == data->enable_als_sensor) && 
				  (APDS_ENABLE_PS == data->enable_ps_sensor))
        {
			/* reconfigute light sensor setting	*/		
			apds9960_set_enable(client, APDS9960_PWR_DOWN);      // Power Off
			apds9960_set_ailt( client, 0xFFFF);	// Force ALS interrupt
			apds9960_set_aiht( client, 0);
			apds9960_set_enable(client, 0x37);  //enable ALS interrupt
		}
		else    // APDS_DISBLE_ALS & APDS_ENALBE_PS
        {
			apds9960_set_enable(client, APDS9960_PWR_DOWN);
			
			/* force first interrupt to inform HAL */
	#ifdef CUSTOM_KERNEL_SENSORHUB
			apds9960_set_psensor_threshold(client,APDS9960_FAR_THRESHOLD_HIGH,APDS9960_NEAR_THRESHOLD_LOW);
	#else
			apds9960_set_pilt(client, APDS9960_FAR_THRESHOLD_HIGH);		
			apds9960_set_piht(client, APDS9960_NEAR_THRESHOLD_LOW);
	#endif
			apds9960_set_enable(client, 0x2D);  // only enable PS interrupt
		}
/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
		/* kk 08-Jul-2014, stop timer here */
		__cancel_delayed_work(&data->xtalk_dwork);
		flush_delayed_work(&data->xtalk_dwork);
/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */
	}
	
	return SUCCESS;
}

static int apds9960_calibration(struct i2c_client *client, int val)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int status;
	unsigned char i2c_data[10];
	unsigned int old_lux_GA1, old_cct_GA1;
	int loop=0;
	unsigned int old_poffset_ur, old_poffset_dl, old_wtime, old_config2, old_gctrl;
	unsigned int old_goffset_u, old_goffset_d, old_goffset_l, old_goffset_r;
	unsigned int old_gthr_in=0, old_gthr_out=0;
	unsigned int old_control, old_ppulse;
	unsigned int temp_offset=0;
	unsigned int temp_goffset_u=0;
	unsigned int temp_goffset_d=0;
	unsigned int temp_goffset_l=0;
	unsigned int temp_goffset_r=0;	
	int gstatus;
	unsigned int gesture_u, gesture_d, gesture_l, gesture_r;
	unsigned int gesture_u_cal_done = 0;
	unsigned int gesture_d_cal_done = 0;
	unsigned int gesture_l_cal_done = 0;
	unsigned int gesture_r_cal_done = 0;
 	
	APS_DBG("apds9960_calibration (%d)\n", val);
	
	if ((APDS_DISABLE_PS != data->enable_ps_sensor) ||
		(APDS_DISABLE_ALS != data->enable_als_sensor) ||
		(APDS_DISABLE_GESTURE != data->enable_gesture_sensor))
    {
		APS_ERR("sensor is in active mode, no calibration\n");
		return -1;
	}
	
	if (APDS_ALS_CALIBRATION == val)
    {
		apds9960_set_enable(client,APDS9960_PWR_ON|APDS9960_ALS_ENABLE);
		//mdelay(100);		
		status = i2c_smbus_read_i2c_block_data(client, APDS9960_CDATAL_REG, 8, (unsigned char*)i2c_data);

		if (status < 0)
            return status;
		if (status != 8)
            return -1;

		data->cdata = (i2c_data[1]<<8) | i2c_data[0];
		data->rdata = (i2c_data[3]<<8) | i2c_data[2];
		data->gdata = (i2c_data[5]<<8) | i2c_data[4];
		data->bdata = (i2c_data[7]<<8) | i2c_data[6];

		old_lux_GA1 = data->lux_GA1;
		old_cct_GA1 = data->cct_GA1;
		data->lux_GA1 = 100;
		LuxCalculation(client);

		if ((data->lux >= (APDS9960_ALS_CALIBRATED_LUX*70)/100) && 
			(data->lux <= (APDS9960_ALS_CALIBRATED_LUX*130)/100))
        {
			data->lux_GA1 = (APDS9960_ALS_CALIBRATED_LUX*100)/data->lux;
			data->cal.l_cal = data->lux_GA1;    //jing.zhao@qingcheng.com add
			return 1;
		}
		else
        {
			data->lux_GA1 = old_lux_GA1;
            data->cal.l_cal = data->lux_GA1;    //jing.zhao@qingcheng.com add
			return -2;
		}

		if ((data->cct >= (APDS9960_ALS_CALIBRATED_CCT*90)/100) && 
			(data->cct <= (APDS9960_ALS_CALIBRATED_CCT*110)/100))
        {
			data->cct_GA1 = (APDS9960_ALS_CALIBRATED_CCT*100)/data->cct;
			return 1;
		}
		else
        {
			data->cct_GA1 = old_cct_GA1;
			return -3;
		}

		APS_DBG("ALS cal done : %d lux\n", data->lux);
	}
	else if (APDS_PS_CALIBRATION == val)
    {
		old_control = data->control;
		old_ppulse = data->ppulse;
		old_poffset_ur = data->poffset_ur;
		old_poffset_dl = data->poffset_dl;
		old_wtime = data->wtime;
		old_config2 = data->config2;

		apds9960_set_wtime(client, APDS9960_255_WAIT_TIME);
		apds9960_set_ppulse(client, data->ps_ppulse);
		apds9960_set_control(client, APDS9960_PDRVIE_FOR_PS|APDS9960_PGAIN_FOR_PS); 
		apds9960_set_config2(client, 0x26);
		apds9960_set_aux(client, APDS9960_PS_LED_BOOST|0x01);
		apds9960_set_poffset_ur(client, 0x00);
		apds9960_set_poffset_dl(client, 0x00);
		apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE);
		
		/* POFFSET_UR */
		loop = 0;
		temp_offset = data->poffset_ur;

		while (loop++ <= 127)
        {
			mdelay(10);	
			data->ps_data =	i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);

			if ((data->ps_data <= APDS9960_PS_CALIBRATED_XTALK) && (data->ps_data >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
            {
				APS_DBG("cal ur %d (%d) - done\n", temp_offset, data->ps_data);
				break;
			}
			else
            {
				if (data->ps_data > APDS9960_PS_CALIBRATED_XTALK)
                {
				    // reduce 
					if ((temp_offset >= 0) && (temp_offset <= 127))
                    {
						temp_offset += 1;
					}
					else
                    {
						if (temp_offset == 127)
                        {
                            temp_offset = 1;
                        }
						temp_offset -= 1;
					}
				}		
				else if (data->ps_data < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                {
					// increase
					if ((temp_offset > 0) && (temp_offset <= 127))
                    {
						temp_offset -= 1;
					}
					else
                    {
        				if (temp_offset == 255)
                            temp_offset = 0;        // something is wrong					
        				if (temp_offset == 0)
                            temp_offset = 127;      // start from 128
        				
						temp_offset += 1;           // start from 128
						temp_offset = temp_offset&0xFF;										
					}					
				}
			}

			apds9960_set_poffset_ur(client, temp_offset);
		}

		if (loop >= 128)
        {
			apds9960_set_wtime(client, old_wtime);
			apds9960_set_ppulse(client, old_ppulse);
			apds9960_set_control(client, old_control); 
			apds9960_set_poffset_ur(client, old_poffset_ur);
			apds9960_set_poffset_dl(client, old_poffset_dl);
			apds9960_set_config2(client, old_config2);
			apds9960_set_enable(client, APDS9960_PWR_DOWN);

			return -4;
		}

		data->ps_poffset_ur = temp_offset;
		data->cal.p_cal_ur = temp_offset;       //jing.zhao@qingcheng.com add
		
		apds9960_set_enable(client, APDS9960_PWR_DOWN);
		apds9960_set_config2(client, 0x29);
		apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE);

		loop = 0;
		temp_offset = data->poffset_dl;

		while (loop++ <= 127)
        {
			mdelay(10);		
			data->ps_data =	i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);

			if ((data->ps_data <= APDS9960_PS_CALIBRATED_XTALK) && (data->ps_data >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
            {
				APS_DBG("cal dl %d (%d) - done\n", temp_offset, data->ps_data);
				break;
			}
			else
            {
				if (data->ps_data > APDS9960_PS_CALIBRATED_XTALK)
                {
				    // reduce 
					if ((temp_offset >= 0) && (temp_offset <= 127))
                    {
						temp_offset += 1;
					}
					else
                    {
						if (temp_offset == 127)
                        {
                            temp_offset = 1;
                        }
						temp_offset -= 1;
					}
				}		
				else if (data->ps_data < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                {
					// increase
					if ((temp_offset > 0) && (temp_offset <= 127))
                    {
						temp_offset -= 1;
					}
					else
                    {
        				if (temp_offset == 255) temp_offset = 0;; // something is wrong						
        				if (temp_offset == 0) temp_offset = 127; // start from 128
        				
						temp_offset += 1;	// start from 128
						temp_offset = temp_offset&0xFF;										
					}					
				}
			}

			apds9960_set_poffset_dl(client, temp_offset);
		}
		
		if (loop >= 128)
        {
			apds9960_set_wtime(client, old_wtime);
			apds9960_set_ppulse(client, old_ppulse);
			apds9960_set_control(client, old_control); 
			apds9960_set_poffset_ur(client, old_poffset_ur);
			apds9960_set_poffset_dl(client, old_poffset_dl);
			apds9960_set_config2(client, old_config2);
			apds9960_set_enable(client, APDS9960_PWR_DOWN);
			APS_DBG("loop %d -5\n", loop);
			return -5;
		}

		apds9960_set_ppulse(client, old_ppulse);
		apds9960_set_control(client, old_control); 
		apds9960_set_wtime(client, old_wtime);
		apds9960_set_config2(client, old_config2);

		data->ps_poffset_dl = temp_offset;
		data->cal.p_cal_dl = temp_offset; //jing.zhao@qingcheng.com add
		
		APS_DBG("PS cal done : ur=%d dl=%d\n", data->ps_poffset_ur, data->ps_poffset_dl);
	} 
	else if (val == APDS_PS_GESTURE_CALIBRATION)
    {
        APS_DBG("Gesture calibration\n");
		old_control = data->control;
		old_ppulse = data->ppulse;
		old_poffset_ur = data->poffset_ur;
		old_poffset_dl = data->poffset_dl;
		old_wtime = data->wtime;
		old_config2 = data->config2;

		apds9960_set_wtime(client, APDS9960_255_WAIT_TIME);
		apds9960_set_ppulse(client, data->gesture_ppulse);
		apds9960_set_control(client, APDS9960_PDRVIE_FOR_GESTURE|APDS9960_PGAIN_FOR_GESTURE); 

		apds9960_set_config2(client, 0x26);
		apds9960_set_aux(client, APDS9960_GESTURE_LED_BOOST|0x01);
		apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE);
		
		/* POFFSET_UR */
		loop = 0;
		temp_offset = data->poffset_ur;
		while (loop++ <= 127)
        {
			mdelay(10);	
			data->ps_data =	i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
			APS_DBG("G_UR ps_data=0x%x\n", data->ps_data);
			if ((data->ps_data <= APDS9960_PS_CALIBRATED_XTALK) && (data->ps_data >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
            {
				APS_DBG("cal ur %d (%d) - done\n", temp_offset, data->ps_data);
				break;
			}
			else
            {
				APS_DBG("G_UR 000\n");
				if (data->ps_data > APDS9960_PS_CALIBRATED_XTALK)
                {
				    // reduce 
					if ((temp_offset >= 0) && (temp_offset <= 127))
                    {
						temp_offset += 1;
					}
					else
                    {
						if (temp_offset == 127)
                        {
                            temp_offset = 1;
                        }

						temp_offset -= 1;
					}
				}		
				else if (data->ps_data < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                {
					// increase
					if ((temp_offset > 0) && (temp_offset <= 127))
                    {
						temp_offset -= 1;
					}
					else
                    {
        				if (temp_offset == 255) temp_offset = 0;; // something is wrong						
        				if (temp_offset == 0) temp_offset = 127; // start from 128
        				
						temp_offset += 1;	// start from 128
						temp_offset = temp_offset&0xFF;										
					}					
				}
			}
			APS_DBG("temp_offset:0x%x\n", temp_offset);
			apds9960_set_poffset_ur(client, temp_offset);
		}
		APS_DBG("G_UR loop:%d\n", loop);
		if (loop >= 128)
        {
			apds9960_set_wtime(client, old_wtime);

			apds9960_set_ppulse(client, old_ppulse);
			apds9960_set_control(client, old_control); 
			apds9960_set_poffset_ur(client, old_poffset_ur);
			apds9960_set_poffset_dl(client, old_poffset_dl);
			apds9960_set_config2(client, old_config2);
			apds9960_set_enable(client, APDS9960_PWR_DOWN);
			return -4;
		}

		data->gesture_poffset_ur = temp_offset;
		data->cal.g_cal_ur = temp_offset;   //jing.zhao@qingcheng.com add
		
		apds9960_set_enable(client, APDS9960_PWR_DOWN);
		apds9960_set_config2(client, 0x29);
		apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_PS_ENABLE|APDS9960_WAIT_ENABLE);

		loop = 0;
		temp_offset = data->poffset_dl;
		while (loop++ <= 127)
        {
			mdelay(10);	
			data->ps_data =	i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
            APS_DBG("G_DL ps_data=0x%x\n", data->ps_data);
			if ((data->ps_data <= APDS9960_PS_CALIBRATED_XTALK) && (data->ps_data >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
            {
				APS_DBG("cal dl %d (%d) - done\n", temp_offset, data->ps_data);
				break;
			}
			else
            {
                APS_DBG("G_DL 000\n");
				if (data->ps_data > APDS9960_PS_CALIBRATED_XTALK)
                {
				    // reduce 
					if ((temp_offset >= 0) && (temp_offset <= 127))
                    {
						temp_offset += 1;
					}
					else
                    {
						if (temp_offset == 127)
                        {
                            temp_offset = 1;
                        }


						temp_offset -= 1;
					}
				}		
				else if (data->ps_data < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                {
					// increase
					if ((temp_offset > 0) && (temp_offset <= 127))
                    {
						temp_offset -= 1;
					}
					else
                    {
        				if (temp_offset == 255)
                            temp_offset = 0;    // something is wrong						
        				if (temp_offset == 0)
                            temp_offset = 127;  // start from 128
        				
						temp_offset += 1;       // start from 128
						temp_offset = temp_offset&0xFF;										
					}					
				}
			}
            APS_DBG("G_DL temp_offset=0x%x\n", temp_offset);
			apds9960_set_poffset_dl(client, temp_offset);
		}
		APS_DBG("G_DL loop:%d\n", loop);
		if (loop >= 128)
        {
			apds9960_set_wtime(client, old_wtime);
			apds9960_set_ppulse(client, old_ppulse);
			apds9960_set_control(client, old_control); 
			apds9960_set_poffset_ur(client, old_poffset_ur);
			apds9960_set_poffset_dl(client, old_poffset_dl);
			apds9960_set_config2(client, old_config2);
			apds9960_set_enable(client, APDS9960_PWR_DOWN);
			APS_DBG("loop %d -5\n", loop);
			return -5;
		}

		apds9960_set_ppulse(client, old_ppulse);
		apds9960_set_control(client, old_control); 
		apds9960_set_wtime(client, old_wtime);
		apds9960_set_config2(client, old_config2);

		data->gesture_poffset_dl = temp_offset;
		data->cal.g_cal_dl = temp_offset;       //jing.zhao@qingcheng.com add
		
		old_config2 = data->config2;
		old_gctrl = data->gctrl;
		old_goffset_u = data->goffset_u;
		old_goffset_d = data->goffset_d;
		old_goffset_l = data->goffset_l;
		old_goffset_r = data->goffset_r;
		old_gthr_in = data->gthr_in;
		old_gthr_out = data->gthr_out;

		apds9960_set_enable(client, APDS9960_PWR_DOWN);
		apds9960_set_config2(client, 0x00);

		apds9960_set_gthr_in(client, 0x00);
		apds9960_set_gthr_out(client, 0x00);

		apds9960_set_enable(client, APDS9960_PWR_ON|APDS9960_GESTURE_ENABLE);
		apds9960_set_gctrl(client, 0x05);

		loop = 0;
		temp_goffset_u = data->goffset_u;
		temp_goffset_d = data->goffset_u;
		temp_goffset_l = data->goffset_u;
		temp_goffset_r = data->goffset_u;

		while(loop++ <= 127)
        {
            APS_DBG("G_U D L R\n");
			mdelay(10);	
			gstatus = i2c_smbus_read_byte_data(client, APDS9960_GSTATUS_REG);

			if (gstatus < 0)
            {
				APS_ERR("gstatus error %d\n", gstatus);
				return -6;
			}
		
			APS_DBG("gstatus = %x (%d)\n", gstatus, loop);
			if (gstatus & APDS9960_GVALID)
            {
				status = i2c_smbus_read_i2c_block_data(client, APDS9960_GFIFO0_REG, 
													4, (unsigned char*)i2c_data);
				gesture_u = i2c_data[0];
				gesture_d = i2c_data[1];
				gesture_l = i2c_data[2];
				gesture_r = i2c_data[3];
				APS_DBG("gesture_u(0x%x),gesture_d(0x%x),gesture_l(0x%x),gesture_r(0x%x)\n", gesture_u, gesture_d, gesture_l, gesture_r);
				//if (!gesture_u_cal_done) 
				{
					if ((gesture_u <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_u >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
                    {
						gesture_u_cal_done = 1;
						APS_DBG("cal u %d (%d) - done\n", gesture_u, temp_goffset_u);
					}
					else
                    {
						APS_DBG("cal U 000\n");
						gesture_u_cal_done = 0;
						if (gesture_u > APDS9960_PS_CALIBRATED_XTALK)
                        {
							// reduce 
							if ((temp_goffset_u >= 0) && (temp_goffset_u <= 127))
                            {
								temp_goffset_u += 1;	
							}
							else
                            {
						        if (temp_goffset_u == 127)
                                {
                                    temp_goffset_u = 1;
                                }

						        temp_goffset_u -= 1;
							}
						}		
						else if (gesture_u < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                        {
							// increase
							if ((temp_goffset_u > 0) && (temp_goffset_u <= 127))
                            {
								temp_goffset_u -= 1;
							}
							else
                            {
                				if (temp_goffset_u == 255)
                                    temp_goffset_u = 0;     // something is wrong						
                				if (temp_goffset_u == 0)
                                    temp_goffset_u = 127;   // start from 128
                				
								temp_goffset_u += 1;        // start from 128
								temp_goffset_u = temp_goffset_u&0xFF;
							}					
						}
						APS_DBG("cal U temp_goffset_u:0x%x\n", temp_goffset_u);
						apds9960_set_goffset_u(client, temp_goffset_u);
					}
				}

				//if (!gesture_d_cal_done) 
				{
					if ((gesture_d <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_d >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
                    {
						gesture_d_cal_done = 1;
						APS_DBG("cal d %d (%d) - done\n", gesture_d, temp_goffset_d);
					}
					else
                    {
                        APS_DBG("cal D 000\n");
						gesture_d_cal_done = 0;
						if (gesture_d > APDS9960_PS_CALIBRATED_XTALK)
                        {
							// reduce 
							if ((temp_goffset_d >= 0) && (temp_goffset_d <= 127))
                            {
								temp_goffset_d += 1;
							}
							else
                            {
						        if (temp_goffset_d == 127)
                                {
                                    temp_goffset_d = 1;
                                }
						        temp_goffset_d -= 1;
							}
						}		
						else if (gesture_d < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                        {
							// increase
							if ((temp_goffset_d > 0) && (temp_goffset_d <= 127))
                            {
								temp_goffset_d -= 1;
							}
							else
                            {
               				    if (temp_goffset_d == 255)
                                    temp_goffset_d = 0;     // something is wrong						
                				if (temp_goffset_d == 0)
                                    temp_goffset_d = 127;   // start from 128
                				
								temp_goffset_d += 1;        // start from 128
								temp_goffset_d = temp_goffset_d&0xFF;
							}					
						}
						APS_DBG("cal U temp_goffset_d:0x%x\n", temp_goffset_d);
						apds9960_set_goffset_d(client, temp_goffset_d);
					}
				}

				//if (!gesture_l_cal_done) 
				{
					if ((gesture_l <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_l >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
                    {
						gesture_l_cal_done = 1;
						APS_DBG("cal l %d (%d) - done\n", gesture_l, temp_goffset_l);
					}
					else
                    {
                        APS_DBG("cal L 000\n");
						gesture_l_cal_done = 0;
						if (gesture_l > APDS9960_PS_CALIBRATED_XTALK)
                        {
							// reduce 
							if ((temp_goffset_l >= 0) && (temp_goffset_l <= 127))
                            {
								temp_goffset_l += 1;
							}
							else
                            {
						        if (temp_goffset_l == 127)
                                {
                                    temp_goffset_l = 1;
                                }

						        temp_goffset_l -= 1;
							}
						}		
						else if (gesture_l < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                        {
							// increase
							if ((temp_goffset_l > 0) && (temp_goffset_l <= 127))
                            {
								temp_goffset_l -= 1;
							}
							else
                            {
               				    if (temp_goffset_l == 255)
                                    temp_goffset_l = 0;     // something is wrong						
                				if (temp_goffset_l == 0)
                                    temp_goffset_l = 127;   // start from 128
                				
								temp_goffset_l += 1;        // start from 128
								temp_goffset_l = temp_goffset_l&0xFF;
							}					
						}
						APS_DBG("cal U temp_goffset_l:0x%x\n", temp_goffset_l);
						apds9960_set_goffset_l(client, temp_goffset_l);
					}
				}

				//if (!gesture_r_cal_done) 
				{
					if ((gesture_r <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_r >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
                    {
						gesture_r_cal_done = 1;
						APS_DBG("cal r %d (%d) - done\n", gesture_r, temp_goffset_r);
					}
					else
                    {
                        APS_DBG("cal R 000\n");
						gesture_r_cal_done = 0;
						if (gesture_r > APDS9960_PS_CALIBRATED_XTALK)
                        {
							// reduce 
							if ((temp_goffset_r >= 0) && (temp_goffset_r <= 127))
                            {
								temp_goffset_r += 1;
							}
							else
                            {
						        if (temp_goffset_r == 127)
                                {
                                    temp_goffset_r = 1;
                                }
						        temp_goffset_r -= 1;
							}
						}		
						else if (gesture_r < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
                        {
							// increase
							if ((temp_goffset_r > 0) && (temp_goffset_r <= 127))
                            {
								temp_goffset_r -= 1;
							}
							else
                            {
               				    if (temp_goffset_r == 255)
                                    temp_goffset_r = 0;     // something is wrong						
                				if (temp_goffset_r == 0)
                                    temp_goffset_r = 127;   // start from 128
                				
								temp_goffset_r += 1;        // start from 128
								temp_goffset_r = temp_goffset_r&0xFF;
							}					
						}
						APS_DBG("cal U temp_goffset_r:0x%x\n", temp_goffset_r);
						apds9960_set_goffset_r(client, temp_goffset_r);
					}
				}

				apds9960_set_gctrl(client, 0x05);
			}
			APS_DBG("cal U(%d) D(%d) L(%d) R(%d)\n", gesture_u_cal_done, gesture_d_cal_done, gesture_l_cal_done, gesture_r_cal_done);
			if (gesture_u_cal_done && gesture_d_cal_done && gesture_l_cal_done && gesture_r_cal_done)
                break;
		}
		APS_DBG("cal U D L R loop:%d\n", loop);
		if (loop >= 128)
        {
			apds9960_set_config2(client, old_config2);

			apds9960_set_gthr_in(client, old_gthr_in);
			apds9960_set_gthr_out(client, old_gthr_out);

			apds9960_set_goffset_u(client, old_goffset_u);
			apds9960_set_goffset_d(client, old_goffset_d);
			apds9960_set_goffset_l(client, old_goffset_l);
			apds9960_set_goffset_r(client, old_goffset_r);

			apds9960_set_gctrl(client, old_gctrl);
			apds9960_set_enable(client, APDS9960_PWR_DOWN);
			return -5;
		}

		apds9960_set_config2(client, old_config2);
		apds9960_set_gthr_in(client, old_gthr_in);
		apds9960_set_gthr_out(client, old_gthr_out);
		apds9960_set_gctrl(client, old_gctrl);
		apds9960_set_enable(client, APDS9960_PWR_DOWN);

		data->gesture_goffset_u = temp_goffset_u;
		data->gesture_goffset_d = temp_goffset_d;
		data->gesture_goffset_l = temp_goffset_l;
		data->gesture_goffset_r = temp_goffset_r;
		
		//[jing.zhao@qingcheng.com add for alsps cal]
		data->cal.g_cal_u = temp_goffset_u;
		data->cal.g_cal_d = temp_goffset_d;
		data->cal.g_cal_l = temp_goffset_l;
		data->cal.g_cal_r = temp_goffset_r;
		//[end]
		
		APS_DBG("Gesture cal done %d %d %d %d\n", gesture_u, gesture_d, gesture_l, gesture_r);
	}
	
	return SUCCESS;
}

/*********************run-time-calibration_2014_06_27_lgc*********************************/
/*********************Enable Calibration**************************/
static int apds9960_Enable_runtime_calibration(struct i2c_client *client)
{
#if defined(PLATFORM_SENSOR_APDS9960)
	struct apds9960_platform_data* platform_data = client->dev.platform_data;
#endif	
   
    unsigned char i2c_data[10];
    int fifo_status;
	uint8_t status;
	uint8_t temp_offset=0;
	uint8_t temp_goffset_u=0;
	uint8_t temp_goffset_d=0;
	uint8_t temp_goffset_l=0;
	uint8_t temp_goffset_r=0;	
	uint8_t gstatus;
	uint8_t gesture_u, gesture_d, gesture_l, gesture_r;
	uint8_t gesture_u_cal_done = 0;
	uint8_t gesture_d_cal_done = 0;
	uint8_t gesture_l_cal_done = 0;
	uint8_t gesture_r_cal_done = 0;
	uint8_t gthr_in, gthr_out;
	uint8_t gconf1;

    /* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
	uint8_t gconf;
	uint8_t config2;
	uint8_t goffset_u, goffset_d, goffset_l, goffset_r;
	uint8_t poffset_ur, poffset_dl;
	uint8_t pdata;
	uint8_t i, j;
	uint8_t Loop_ur = 0;
	uint8_t enable = 0;
	
	/* 2in1 */
	gconf= i2c_smbus_read_byte_data(client, APDS9960_GCTRL_REG);
	config2 = i2c_smbus_read_byte_data(client, APDS9960_CONFIG2_REG);
	enable = i2c_smbus_read_byte_data(client, APDS9960_ENABLE_REG);
    /* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */
	
    apds9960_set_enable(client , APDS9960_PWR_DOWN);
    apds9960_set_enable(client , APDS9960_PWR_ON|APDS9960_PS_ENABLE);
   
	for (j=0; j<10; j++)
    {
        status = i2c_smbus_read_byte_data(client, APDS9960_STATUS_REG);
		
        if (APDS9960_STATUS_PVALID == (status & APDS9960_STATUS_PVALID)) 
            break;

		mdelay(4);	
	}

	pdata = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
	gthr_in = i2c_smbus_read_byte_data(client, APDS9960_GTHR_IN_REG);
	
    apds9960_set_gthr_in(client , 0x00);

	gthr_out = i2c_smbus_read_byte_data(client, APDS9960_GTHR_OUT_REG);
	
    apds9960_set_gthr_out(client , 0x00);

	gconf1 = i2c_smbus_read_byte_data(client, APDS9960_GCONF1_REG);
	
    apds9960_set_gconf1(client , APDS9960_GFIFO_1_LEVEL);

	goffset_u = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_U_REG);
	goffset_d = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_D_REG);
	goffset_l = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_L_REG);
	goffset_r = i2c_smbus_read_byte_data(client, APDS9960_GOFFSET_R_REG);
	poffset_ur = i2c_smbus_read_byte_data(client, APDS9960_POFFSET_UR_REG);
	poffset_dl = i2c_smbus_read_byte_data(client, APDS9960_POFFSET_DL_REG);

    /* 2in1 */
    Loop_ur    = 64;
    poffset_ur = 64;
    poffset_dl = 64;
    goffset_u  = 64;
    goffset_d  = 64;
    goffset_l  = 64;
    goffset_r  = 64;

    apds9960_set_poffset_ur(client , poffset_ur);
    apds9960_set_poffset_dl(client , poffset_dl);
    apds9960_set_goffset_u(client , goffset_u);
    apds9960_set_goffset_d(client , goffset_d);
    apds9960_set_goffset_l(client , goffset_l);
    apds9960_set_goffset_r(client , goffset_r);

    apds9960_set_enable(client, APDS9960_PWR_DOWN);
   
    /* poffset_ur */
    apds9960_set_config2(client , 0x26);    // D and L are masked	
    apds9960_set_enable(client , APDS9960_PWR_ON|APDS9960_PS_ENABLE);     // PEN and PON

    temp_offset = poffset_ur;

	for (i=0; i<10; i++)
    {
        Loop_ur = Loop_ur/2;      // 2014_04_26 Lgc Modified
        for (j=0; j<10; j++) 
        {
			status = i2c_smbus_read_byte_data(client, APDS9960_STATUS_REG);
		
            if ((status & 0x02) == 0x02) 
				break;

			mdelay(4);
		}

		pdata = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);

		if ((pdata <= APDS9960_PS_CALIBRATED_XTALK) && (pdata >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
        {
			break;
		}
		else
        {
			if (pdata > APDS9960_PS_CALIBRATED_XTALK)
            {
				// reduce 
				if ((temp_offset >= 0) && (temp_offset < 127))
                {
					temp_offset += Loop_ur; // 2014_04_26 Lgc Modified
				}
				else
                {
					if (temp_offset == 127)
                    {
                        temp_offset = 1;
                    }
                    temp_offset -= 1;
				}
			}		
			else if (pdata < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
            {
				// increase
				if ((temp_offset > 0) && (temp_offset <= 127))
                {
					temp_offset -= Loop_ur;     // 2014_04_26 Lgc Modified
				}
				else
                {        			
        			if (temp_offset == 0)
                    {
                        temp_offset = 127;      // start from 128
                        Loop_ur = 64;           // 2014_04_26 Lgc Modified
                    }
             
                    if (temp_offset == 255)
                        temp_offset = 0;        // something is wrong
					
                    temp_offset += 1;           // start from 128
                    temp_offset = temp_offset&0xFF;
				}
			}
		}

        apds9960_set_poffset_ur(client , temp_offset);
        apds9960_set_enable(client , APDS9960_PWR_DOWN);
        apds9960_set_enable(client , APDS9960_PWR_ON|APDS9960_PS_ENABLE);
	}

    /* poffset_dl */
	apds9960_set_enable(client , APDS9960_PWR_DOWN);
	apds9960_set_config2(client , 0x29);        // U and R are masked
	apds9960_set_enable(client , APDS9960_PWR_ON|APDS9960_PS_ENABLE);

	temp_offset = poffset_dl;
    Loop_ur = 64;

	for (i=0; i<10; i++)
    {   
		Loop_ur = Loop_ur/2;

        for (j=0; j<10; j++)
        {
			status = i2c_smbus_read_byte_data(client, APDS9960_STATUS_REG);

			if ((status & 0x02) == 0x02) 
				break;

			mdelay(4);
		}

		pdata = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);

		if ((pdata <= APDS9960_PS_CALIBRATED_XTALK) && (pdata >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
        {
			break;
		}
		else
        {
			if (pdata > APDS9960_PS_CALIBRATED_XTALK)
            {
                // reduce 
				if ((temp_offset >= 0) && (temp_offset < 127))
                {
					temp_offset += Loop_ur;
				}
				else
                {
					if (temp_offset == 127)
                    {
                        temp_offset = 1;
                    }
                    temp_offset -= 1;
				}
			}
			else if (pdata < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
            {
				// increase
				if ((temp_offset > 0) && (temp_offset <= 127))
                {
					temp_offset -= Loop_ur;
				}
				else
                {
                    if( temp_offset == 0 )
                    {
                        temp_offset = 127;
                        Loop_ur = 64;
                    }

                    if (temp_offset == 255)
                        temp_offset = 0;// something is wrong	
                  
                    temp_offset += 1;
                    temp_offset = temp_offset&0xFF;	
				}
			}
		}

	    apds9960_set_poffset_dl(client , temp_offset);
	    apds9960_set_enable(client , 0x00);
	    apds9960_set_enable(client , 0x05); 
	}

	apds9960_set_config2(client , 0);
	apds9960_set_enable(client , APDS9960_PWR_DOWN);
	apds9960_set_gctrl(client , APDS9960_GFIFO_CLR|APDS9960_GMODE);
	apds9960_set_enable(client , APDS9960_PWR_ON|APDS9960_GESTURE_ENABLE);

	temp_goffset_u = goffset_u;
	temp_goffset_d = goffset_d;
	temp_goffset_l = goffset_l;
	temp_goffset_r = goffset_r;
	Loop_ur = 64;

	for (i=0; i<10; i++)
    {
		Loop_ur = Loop_ur/2;
	
        for (j=0; j<10; j++)
        {
			gstatus = i2c_smbus_read_byte_data(client, APDS9960_GSTATUS_REG);

			if ((gstatus & APDS9960_GVALID) == APDS9960_GVALID) 
				break;

			mdelay(5);
		}

		fifo_status = i2c_smbus_read_i2c_block_data(client, APDS9960_GFIFO0_REG, 
													4, (unsigned char*)i2c_data);

		gesture_u = i2c_data[0];
		gesture_d = i2c_data[1];
		gesture_l = i2c_data[2];
		gesture_r = i2c_data[3];

		if ((gesture_u <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_u >= APDS9960_PS_CALIBRATED_XTALK_BASELINE)) 
        {
			gesture_u_cal_done = 1;
		}
		else
        {
			gesture_u_cal_done = 0;

			if (gesture_u > APDS9960_PS_CALIBRATED_XTALK)
            {
				// reduce 
				if ((temp_goffset_u >= 0) && (temp_goffset_u < 127))
                {
					temp_goffset_u += Loop_ur;
				}
				else
                {
			        if (temp_goffset_u == 127)
                    {
                        temp_goffset_u = 1;
                    }
			        temp_goffset_u -= 1;
				}
			}		
			else if (gesture_u < APDS9960_PS_CALIBRATED_XTALK_BASELINE)
            {
				// increase
				if ((temp_goffset_u > 0) && (temp_goffset_u <= 127))
                {
					temp_goffset_u -= Loop_ur;
				}
				else
                {        			
        			if (temp_goffset_u == 0)
                    {
                        temp_goffset_u = 127; // start from 128
                        Loop_ur = 64;
                    }
                   
                    if (temp_goffset_u == 255)
                        temp_goffset_u = 0; // something is wrong

                    temp_goffset_u += 1;	// start from 128
                    temp_goffset_u = temp_goffset_u&0xFF;
                }
            }

            apds9960_set_goffset_u(client , temp_goffset_u);
        }

        if ((gesture_d <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_d >= APDS9960_PS_CALIBRATED_XTALK_BASELINE))
        {
            gesture_d_cal_done = 1;
        }
        else
        {
            gesture_d_cal_done = 0;

            if (gesture_d > APDS9960_PS_CALIBRATED_XTALK)
            {
                // reduce 
                if ((temp_goffset_d >= 0) && (temp_goffset_d < 127)) 
                {
                    temp_goffset_d += Loop_ur;
                }
                else 
                {
                    if (temp_goffset_d == 127)
                    {
                        temp_goffset_d = 1;
                    }
                    temp_goffset_d -= 1;
                }
            }		
            else if (gesture_d < APDS9960_PS_CALIBRATED_XTALK_BASELINE) 
            {
                // increase
                if ((temp_goffset_d > 0) && (temp_goffset_d <= 127)) 
                {
                    temp_goffset_d -= Loop_ur;
                }
                else 
                {    			    
                    if (temp_goffset_d == 0)
                    {
                        temp_goffset_d = 127; // start from 128  
                        Loop_ur = 64;
                    }

                    if (temp_goffset_d == 255)
                        temp_goffset_d = 0; // something is wrong

                    temp_goffset_d += 1;	// start from 128
                    temp_goffset_d = temp_goffset_d&0xFF;
                }
            }

            apds9960_set_goffset_d(client , temp_goffset_d);
        }

        if ((gesture_l <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_l >= APDS9960_PS_CALIBRATED_XTALK_BASELINE)) 
        {
            gesture_l_cal_done = 1;
        }
        else 
        {
            gesture_l_cal_done = 0;

            if (gesture_l > APDS9960_PS_CALIBRATED_XTALK) 
            {
                // reduce 
                if ((temp_goffset_l >= 0) && (temp_goffset_l < 127)) 
                {
                    temp_goffset_l += Loop_ur;
                }
                else 
                {
                    if (temp_goffset_l == 127)
                        temp_goffset_l = 1;

                    temp_goffset_l -= 1;
                }
            }
            else if (gesture_l < APDS9960_PS_CALIBRATED_XTALK_BASELINE) 
            {
                // increase
                if ((temp_goffset_l > 0) && (temp_goffset_l <= 127)) 
                {
                    temp_goffset_l -= Loop_ur;
                }
                else 
                {        		    
                    if (temp_goffset_l == 0)
                    {
                        temp_goffset_l = 127; // start from 128   
                        Loop_ur = 64;
                    }
                   
                    if (temp_goffset_l == 255)
                        temp_goffset_l = 0; // something is wrong

                    temp_goffset_l += 1;	// start from 128
                    temp_goffset_l = temp_goffset_l&0xFF;
                }
            }

            apds9960_set_goffset_l(client , temp_goffset_l);
        }

        if ((gesture_r <= APDS9960_PS_CALIBRATED_XTALK) && (gesture_r >= APDS9960_PS_CALIBRATED_XTALK_BASELINE)) 
        {
            gesture_r_cal_done = 1;
        }
        else 
        {
            gesture_r_cal_done = 0;

            if (gesture_r > APDS9960_PS_CALIBRATED_XTALK) 
            {
                // reduce 
                if ((temp_goffset_r >= 0) && (temp_goffset_r < 127)) 
                {
                    temp_goffset_r += Loop_ur;
                }
                else 
                {
                    if (temp_goffset_r == 127)
                    {
                        temp_goffset_r = 1;
                    }
                    temp_goffset_r -= 1;
                }
            }
            else if (gesture_r < APDS9960_PS_CALIBRATED_XTALK_BASELINE) 
            {
                // increase
                if ((temp_goffset_r > 0) && (temp_goffset_r <= 127)) 
                {
                    temp_goffset_r -= Loop_ur;
                }
                else 
                {           		   
                    if (temp_goffset_r == 0)
                    {
                        temp_goffset_r = 127; // start from 128
                        Loop_ur = 64;
                    }
                      
                    if (temp_goffset_r == 255)
                        temp_goffset_r = 0; // something is wrong

                    temp_goffset_r += 1;	// start from 128
                    temp_goffset_r = temp_goffset_r&0xFF;
                }
            }

            apds9960_set_goffset_r(client , temp_goffset_r);
        }

        if (gesture_u_cal_done && gesture_d_cal_done && gesture_l_cal_done && gesture_r_cal_done)
            break;					

        apds9960_set_enable(client , APDS9960_PWR_DOWN);
        apds9960_set_gctrl(client , APDS9960_GFIFO_CLR|APDS9960_GMODE);
        apds9960_set_enable(client , APDS9960_PWR_ON|APDS9960_GESTURE_ENABLE);
    }
   
    Loop_ur = 0;
	apds9960_set_gthr_in(client , gthr_in);
	apds9960_set_gthr_out(client , gthr_out);
	apds9960_set_gconf1(client , gconf1);

    /* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
    apds9960_set_config2(client ,config2);
	apds9960_set_gctrl(client , gconf);

	apds9960_set_enable(client , enable);
	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */
   
	return SUCCESS;
}

static int apds9960_ps_open(struct inode *inode, struct file *file)

{
    APS_FUN();
	return SUCCESS; 
}

static int apds9960_ps_release(struct inode *inode, struct file *file)
{
    APS_FUN();
	return SUCCESS;
}

static long apds9960_ps_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct apds9960_data *data;
    struct i2c_client *client;
    int enable;
    int ret = -1;

    if (0 == arg)
    {
        APS_ERR("arg = 0\n");
        return -1;
    }
    
    if (NULL == apds9960_i2c_client)
    {
		APS_ERR("error: i2c driver not installed\n");
		return -EFAULT;
	}

    client = apds9960_i2c_client;   
    data = i2c_get_clientdata(apds9960_i2c_client);

    switch (cmd)
    {
		case APDS_IOCTL_PS_ENABLE:
			if (copy_from_user(&enable, (void __user *)arg, sizeof(enable)))
            {
				APS_ERR("APDS_IOCTL_PS_ENABLE: copy_from_user failed\n");
				return -EFAULT;
			}

			ret = apds9960_enable_ps_sensor(client, enable);        
			if (ret < 0)
            {
            	APS_ERR("apds9960_enable_ps_sensor() failed!\n");
				return ret;
			}
            break;

     	case APDS_IOCTL_PS_GET_ENABLE:
			if (copy_to_user((void __user *)arg, &data->enable_ps_sensor, sizeof(data->enable_ps_sensor)))
            {
				APS_ERR("APDS_IOCTL_PS_GET_ENABLE: copy_to_user failed\n");
				return -EFAULT;
			}

            break;

        case APDS_IOCTL_PS_GET_PDATA:
			data->ps_data =	i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);

			if (copy_to_user((void __user *)arg, &data->ps_data, sizeof(data->ps_data)))
            {
				APS_ERR("APDS_IOCTL_PS_GET_PDATA: copy_to_user failed\n");
				return -EFAULT;
			}
            break;

		case APDS_IOCTL_GESTURE_ENABLE:              
			if (copy_from_user(&enable, (void __user *)arg, sizeof(enable)))
            {
				APS_ERR("APDS_IOCTL_GESTURE_ENABLE: copy_from_user failed\n");
				return -EFAULT;
			}

			ret = apds9960_enable_gesture_sensor(client, enable);        
			if(ret < 0) {
			    APS_ERR("apds9960_enable_gesture_sensor() failed!\n");
				return ret;
			}
            break;

     	case APDS_IOCTL_GESTURE_GET_ENABLE:
			if (copy_to_user((void __user *)arg, &data->enable_gesture_sensor, sizeof(data->enable_gesture_sensor)))
            {
				APS_ERR("APDS_IOCTL_GESTURE_GET_ENABLE: copy_to_user failed\n");
				return -EFAULT;
			}

            break;

		default:
            break;
    }
	
    return SUCCESS;
}

static int apds9960_als_open(struct inode *inode, struct file *file)
{
    APS_FUN();
	return SUCCESS;
}

static int apds9960_als_release(struct inode *inode, struct file *file)
{
    APS_FUN();
	return SUCCESS;
}

static long apds9960_als_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct apds9960_data *data;
	struct i2c_client *client;
	int enable;
	int ret = -1;
	unsigned int delay;

	if (arg == 0)
        return -1;

	if (NULL == apds9960_i2c_client)
    {    
		APS_ERR("i2c driver not installed\n");
		return -EFAULT;
	}

	client = apds9960_i2c_client;   
	data = i2c_get_clientdata(apds9960_i2c_client);

	switch (cmd)
    {
		case APDS_IOCTL_ALS_ENABLE:
			if (copy_from_user(&enable, (void __user *)arg, sizeof(enable)))
            {
				APS_ERR("APDS_IOCTL_ALS_ENABLE: copy_from_user failed\n");
				return -EFAULT;
			}

			ret = apds9960_enable_als_sensor(client, enable); 
			if (ret < 0)
            {
				return ret;
			}
            break;

        case APDS_IOCTL_ALS_POLL_DELAY:
			if (APDS_ENABLE_ALS_NO_INT == data->enable_als_sensor)
            {	
				if (copy_from_user(&delay, (void __user *)arg, sizeof(delay)))
                {
					APS_ERR("APDS_IOCTL_ALS_POLL_DELAY: copy_to_user failed\n");
					return -EFAULT;
				}
        
				ret = apds9960_set_als_poll_delay (client, delay); 
				if (ret < 0)
                {
					return ret;
				}
			}
			else
            {
				APS_ERR("APDS_IOCTL_ALS_POLL_DELAY: als is not in polling mode!\n");
				return -EFAULT;
			}
            break;

        case APDS_IOCTL_ALS_GET_ENABLE:
			if (copy_to_user((void __user *)arg, &data->enable_als_sensor, sizeof(data->enable_als_sensor)))
            {
				APS_ERR("APDS_IOCTL_ALS_GET_ENABLE: copy_to_user failed\n");
				return -EFAULT;
			}
            break;

        case APDS_IOCTL_ALS_GET_CDATA:
			data->als_data = i2c_smbus_read_word_data(client, APDS9960_CDATAL_REG);

            if (copy_to_user((void __user *)arg, &data->als_data, sizeof(data->als_data)))
            {
				APS_ERR("APDS_IOCTL_ALS_GET_CDATA: copy_to_user failed\n");
				return -EFAULT;
			}
            break;

        case APDS_IOCTL_ALS_GET_RDATA:
			data->als_data = i2c_smbus_read_word_data(client, APDS9960_RDATAL_REG);

            if (copy_to_user((void __user *)arg, &data->als_data, sizeof(data->als_data)))
            {
				APS_ERR("APDS_IOCTL_ALS_GET_RDATA: copy_to_user failed\n");
				return -EFAULT;
			}
            break;

        case APDS_IOCTL_ALS_GET_GDATA:
			data->als_data = i2c_smbus_read_word_data(client, APDS9960_GDATAL_REG);

            if (copy_to_user((void __user *)arg, &data->als_data, sizeof(data->als_data))) {
				APS_ERR("APDS_IOCTL_ALS_GET_GDATA: copy_to_user failed\n");
				return -EFAULT;
			}
            break;

        case APDS_IOCTL_ALS_GET_BDATA:
			data->als_data = i2c_smbus_read_word_data(client, APDS9960_BDATAL_REG);

            if (copy_to_user((void __user *)arg, &data->als_data, sizeof(data->als_data))) {
				APS_ERR("APDS_IOCTL_ALS_GET_BDATA: copy_to_user failed\n");
				return -EFAULT;
			}
            break;

		default:
            break;
	}

	return SUCCESS;
}

/*****************************SysFS support***********************************/
static ssize_t apds9960_show_cdata(struct device_driver *ddri, char *buf)
{
	int cdata;

	cdata = i2c_smbus_read_word_data(apds9960_i2c_client, APDS9960_CDATAL_REG);
	
	return sprintf(buf, "%d\n", cdata);
}

static ssize_t apds9960_show_rdata(struct device_driver *ddri, char *buf)
{
	int rdata;

	rdata = i2c_smbus_read_word_data(apds9960_i2c_client, APDS9960_RDATAL_REG);
	
	return sprintf(buf, "%d\n", rdata);
}

static ssize_t apds9960_show_gdata(struct device_driver *ddri, char *buf)
{
	int gdata;

	gdata = i2c_smbus_read_word_data(apds9960_i2c_client, APDS9960_GDATAL_REG);
	
	return sprintf(buf, "%d\n", gdata);
}

static ssize_t apds9960_show_bdata(struct device_driver *ddri, char *buf)
{
	int bdata;

	bdata = i2c_smbus_read_word_data(apds9960_i2c_client, APDS9960_BDATAL_REG);
	
	return sprintf(buf, "%d\n", bdata);
}

static ssize_t apds9960_show_pdata(struct device_driver *ddri, char *buf)
{
	int pdata;

	pdata = i2c_smbus_read_byte_data(apds9960_i2c_client, APDS9960_PDATA_REG);

	return sprintf(buf, "%d\n", pdata);
}

static ssize_t apds9960_show_alsps_cal(struct device_driver *ddri, char *buf)
{
	struct apds9960_data *data = i2c_get_clientdata(apds9960_i2c_client);
	
	return sprintf(buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", data->cal.l_cal,
		data->cal.p_cal_ur, data->cal.p_cal_dl, data->cal.g_cal_u, data->cal.g_cal_d,
		data->cal.g_cal_l, data->cal.g_cal_r, data->cal.g_cal_ur, data->cal.g_cal_dl);
}

static ssize_t apds9960_store_alsps_cal(struct device_driver *ddri, const char *buf, size_t count)
{
	struct i2c_client *client = apds9960_i2c_client;
	struct apds9960_data *data = i2c_get_clientdata(client);
	struct alsps_cal cali;
	int rc = 0;

	rc = sscanf(buf, "%d %d %d %d %d %d %d %d %d", &cali.l_cal, &cali.p_cal_ur, &cali.p_cal_dl,
		&cali.g_cal_u, &cali.g_cal_d, &cali.g_cal_l, &cali.g_cal_r, &cali.g_cal_ur,
		&cali.g_cal_dl);
	if (9 != rc)
    {
		APS_ERR("Invalid arguments\n");
		rc = -EINVAL;
		return rc;
	}
	APS_DBG("GO l_cal(%d)p_ur(%d)p_dl(%d)g_ur(%d)g_dl(%d)gu(%d)gd(%d)gl(%d)gr(%d)\n", cali.l_cal,
               cali.p_cal_ur, cali.p_cal_dl, cali.g_cal_ur, cali.g_cal_dl, cali.g_cal_u, cali.g_cal_d,
               cali.g_cal_l, cali.g_cal_r);
	//data->lux_GA1 = cali.l_cal;
	data->ps_poffset_ur = cali.p_cal_ur;
	data->ps_poffset_dl = cali.p_cal_dl;
	data->gesture_poffset_ur = cali.g_cal_ur;
	data->gesture_poffset_dl = cali.g_cal_dl;

	apds9960_set_goffset_u(client, cali.g_cal_u);
	apds9960_set_goffset_d(client, cali.g_cal_d);
	apds9960_set_goffset_l(client, cali.g_cal_l);
	apds9960_set_goffset_r(client, cali.g_cal_r);
	
	return count;
}

static ssize_t apds9960_show_proximity_enable(struct device_driver *ddri, char *buf)
{
	struct apds9960_data *data = i2c_get_clientdata(apds9960_i2c_client);
	
	return sprintf(buf, "%d\n", data->enable_ps_sensor);
}

static ssize_t apds9960_store_proximity_enable(struct device_driver *ddri, const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);
	
	APS_DBG("enable ps senosr (%ld)\n", val);
	
	if ((APDS_DISABLE_PS != val) && (APDS_ENABLE_PS != val))
    {
		APS_ERR("store invalid value=%ld\n", val);
		return count;
	}

	apds9960_enable_ps_sensor(apds9960_i2c_client, val);	
	
	return count;
}

static ssize_t apds9960_show_light_enable(struct device_driver *ddri, char *buf)
{
	struct apds9960_data *data = i2c_get_clientdata(apds9960_i2c_client);
	
	return sprintf(buf, "%d\n", data->enable_als_sensor);
}

static ssize_t apds9960_store_light_enable(struct device_driver *ddri, const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);
	
	APS_DBG("enable als sensor (%ld)\n", val);
	
	if ((APDS_DISABLE_ALS != val) && (APDS_ENABLE_ALS_WITH_INT != val) && (APDS_ENABLE_ALS_NO_INT != val))
	{
		APS_ERR("store invalid valeu=%ld\n", val);
		return count;
	}

	apds9960_enable_als_sensor(apds9960_i2c_client, val); 
	
	return count;
}

static ssize_t apds9960_show_gesture_enable(struct device_driver *ddri, char *buf)
{
	struct apds9960_data *data = i2c_get_clientdata(apds9960_i2c_client);
	
	return sprintf(buf, "%d\n", data->enable_gesture_sensor);
}

static ssize_t apds9960_store_gesture_enable(struct device_driver *ddri, const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);
	
	APS_DBG("enable gesture sensor (%ld)\n", val);
	
	if ((APDS_DISABLE_GESTURE != val) && (APDS_ENABLE_GESTURE != val))
	{
		APS_ERR("store invalid valeu=%ld\n", val);
		return count;
	}

	apds9960_enable_gesture_sensor(apds9960_i2c_client, val); 
	
	return count;
}

static ssize_t apds9960_show_calibration(struct device_driver *ddri, char *buf)
{
	struct apds9960_data *data = i2c_get_clientdata(apds9960_i2c_client);
	
	return sprintf(buf, "%d\n", data->enable_gesture_sensor);
}

static ssize_t apds9960_store_calibration(struct device_driver *ddri, const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);
	
	APS_DBG("calibration (%ld)\n", val);
	
	if ((APDS_ALS_CALIBRATION != val) && (APDS_PS_CALIBRATION != val) && (APDS_PS_GESTURE_CALIBRATION != val))
	{
		APS_ERR("store invalid valeu=%ld\n", val);
		return count;
	}

	apds9960_calibration(apds9960_i2c_client, val); 
	
	return count;
}

static ssize_t apds9960_show_reg(struct device_driver *ddri, char *buf)
{
    int i = 0;
    u8 	addr = 0x80;
    u8 	dat = 0;
    int len = 0;

    if (!apds9960_obj) {
        APS_ERR("apds9960_obj is null!!\n");
        return 0;
    }

    for (i = 0; i < 30; i++) {
        dat = i2c_smbus_read_byte_data(apds9960_i2c_client, addr);
        len += snprintf(buf+len,16,"Reg addr=%x ",addr);
        len += snprintf(buf+len,16,"Reg regdata=%x\n",dat);
        addr++;
        if (addr == 0x84) {
            addr += 0x04;
		}
        if (addr == 0x90) {
            addr += 0x03;
		}
        if (addr > 0x9F) {
            break;
		}
    }

    return len;
}

#define IS_SPACE(CH) (((CH) == ' ') || ((CH) == '\n'))
static int read_int_from_buf(struct apds9960_priv *obj, const char* buf, size_t count,
							 u32 data[], int len)
{
	int 	idx = 0;
	char 	*cur = (char*)buf, *end = (char*)(buf+count);
	while (idx < len) {
		while ( (cur < end) && IS_SPACE(*cur) ) {
			cur++;
		}
		if (1 != sscanf(cur, "%d", &data[idx])) {
			break;
		}
		idx++;
		while ((cur < end) && !IS_SPACE(*cur)) {
			cur++;
		}
	}
    return idx;
}

static ssize_t apds9960_store_reg(struct device_driver *ddri, char *buf, ssize_t count)
{
    u32 data[2];
    if (read_int_from_buf(apds9960_obj, buf, count, data, 2) != 2) {
        APS_ERR("%s the buff is not available", __func__);
    }    
    APS_LOG("the data[0] =  %d, data[1] = %d ", data[0], data[1]);
    i2c_smbus_write_byte_data(apds9960_obj->client, data[0], data[1]);
    return count;
}

static ssize_t apds9960_show_high_threshold(struct device_driver *ddri, char *buf)
{
	struct apds9960_data *data = i2c_get_clientdata(apds9960_i2c_client);

	return sprintf(buf, "%d\n", data->piht);
}

static ssize_t apds9960_show_low_threshold(struct device_driver *ddri, char *buf)
{
	struct apds9960_data *data = i2c_get_clientdata(apds9960_i2c_client);

	return sprintf(buf, "%d\n", data->pilt);
}

static DRIVER_ATTR(cdata, S_IRUGO, apds9960_show_cdata, NULL);
static DRIVER_ATTR(rdata, S_IRUGO, apds9960_show_rdata, NULL);
static DRIVER_ATTR(gdata, S_IRUGO, apds9960_show_gdata, NULL);
static DRIVER_ATTR(bdata, S_IRUGO, apds9960_show_bdata, NULL);
static DRIVER_ATTR(pdata, S_IRUGO, apds9960_show_pdata, NULL);
static DRIVER_ATTR(alsps_cal, S_IWUGO | S_IRUGO, apds9960_show_alsps_cal, apds9960_store_alsps_cal);
static DRIVER_ATTR(proximity_enable, S_IWUGO | S_IRUGO, apds9960_show_proximity_enable, apds9960_store_proximity_enable);
static DRIVER_ATTR(light_enable, S_IWUGO | S_IRUGO, apds9960_show_light_enable, apds9960_store_light_enable);
static DRIVER_ATTR(gesture_enable, S_IWUGO | S_IRUGO, apds9960_show_gesture_enable, apds9960_store_gesture_enable);
static DRIVER_ATTR(calibration, S_IWUGO | S_IRUGO, apds9960_show_calibration, apds9960_store_calibration);
static DRIVER_ATTR(reg, S_IWUSR | S_IRUGO, apds9960_show_reg, apds9960_store_reg);
static DRIVER_ATTR(high_threshold, S_IWUSR | S_IRUGO, apds9960_show_high_threshold, NULL);
static DRIVER_ATTR(low_threshold, S_IWUSR | S_IRUGO, apds9960_show_low_threshold, NULL);

static struct device_attribute *apds9960_attr_list[] = {
	&driver_attr_cdata,
    &driver_attr_rdata,
    &driver_attr_gdata,
    &driver_attr_bdata,
    &driver_attr_pdata,
    &driver_attr_alsps_cal,
    &driver_attr_proximity_enable,
    &driver_attr_light_enable,
    &driver_attr_gesture_enable,
    &driver_attr_calibration,
    &driver_attr_reg,
    &driver_attr_high_threshold,
    &driver_attr_low_threshold,
};

static int apds9960_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(apds9960_attr_list)/sizeof(apds9960_attr_list[0]));

	if (driver == NULL) {
		return -EINVAL;
	}

	for (idx = 0; idx < num; idx++) {
		if (err = driver_create_file(driver, apds9960_attr_list[idx])) {
			APS_ERR("driver_create_file (%s) = %d\n", apds9960_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}

static int apds9960_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(apds9960_attr_list)/sizeof(apds9960_attr_list[0]));

	if (!driver) {
		return -EINVAL;
	}

	for (idx = 0; idx < num; idx++) {
		driver_remove_file(driver, apds9960_attr_list[idx]);
	}

	return err;
}

/************************************************************/
static struct file_operations apds9960_ps_fops = {
	.owner = THIS_MODULE,
	.open = apds9960_ps_open,
	.release = apds9960_ps_release,
	.unlocked_ioctl = apds9960_ps_ioctl,
};

static struct miscdevice apds9960_ps_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "apds_ps_dev",
	.fops = &apds9960_ps_fops,
};

static struct file_operations apds9960_als_fops = {
	.owner = THIS_MODULE,
	.open = apds9960_als_open,
	.release = apds9960_als_release,
	.unlocked_ioctl = apds9960_als_ioctl,
};

static struct miscdevice apds9960_als_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "apds_als_dev",
	.fops = &apds9960_als_fops,
};

int apds9960_setup_eint(struct i2c_client *client)
{
#ifdef CUSTOM_KERNEL_SENSORHUB
		int err = 0;
	
		err = SCP_sensorHub_rsp_registration(ID_PROXIMITY, alsps_irq_handler);
	
	   
#else

	struct apds9960_data *obj = i2c_get_clientdata(client);        

    APS_FUN();
	g_apds9960_ptr = obj;
	
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

	mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, apds9960_interrupt, 0);

	mt_eint_unmask(CUST_EINT_ALS_NUM);  
	APS_DBG("exit!\n");
#endif	
    return SUCCESS;
}

/*
 * Initialization function
 */
static int apds9960_init_client(struct i2c_client *client)
{
	struct apds9960_data *data = i2c_get_clientdata(client);
	int err;
	int id;

	err = apds9960_set_enable(client, APDS9960_PWR_DOWN);
	if (err < 0)
    {

		APS_ERR("apds9960_set_enable error!\n");
		return err;
	}

	id = i2c_smbus_read_byte_data(client, APDS9960_ID_REG);
	if (id == 0x9F || id == 0x9C || id == 0xAB)
    {
		APS_LOG("APDS-9960\n");
	}
	else
    {
		APS_ERR("Not APDS-9960\n");
		return -EIO;
	}

	err = apds9960_set_atime(client, apds9960_als_atime_tb[data->als_atime_index]);	
	if (err < 0)
        return err;

	err = apds9960_set_wtime(client, APDS9960_246_WAIT_TIME);	// 27ms Wait time
	if (err < 0)
        return err;

	err = apds9960_set_ppulse(client, data->ps_ppulse);	
	if (err < 0)
        return err;

	err = apds9960_set_poffset_ur(client, data->ps_poffset_ur);	
	if (err < 0)
        return err;

	err = apds9960_set_poffset_dl(client, data->ps_poffset_dl);	
	if (err < 0)
        return err;

	err = apds9960_set_config(client, 0x60);		// no long wait
	if (err < 0)
        return err;

	err = apds9960_set_control(client, APDS9960_PDRVIE_FOR_PS|APDS9960_PGAIN_FOR_PS|apds9960_als_again_bit_tb[data->als_again_index]);
	if (err < 0)
        return err;

	err = apds9960_set_pilt(client, 0);		// init threshold for proximity
	if (err < 0)
        return err;

	err = apds9960_set_piht(client, data->ps_threshold);
	if (err < 0)
        return err;

	err = apds9960_set_ailt(client, 0xFFFF);	// force first ALS interrupt to get the environment reading
	if (err < 0)
        return err;

	err = apds9960_set_aiht(client, 0);
	if (err < 0)
        return err;

	err = apds9960_set_pers(client, APDS9960_PPERS_2|APDS9960_APERS_2);	// 2 consecutive persistence
	if (err < 0)
        return err;

	// gesture register
	err = apds9960_set_aux(client, 0x01);
	if (err < 0)
        return err;

	err = apds9960_set_config2(client, 0);
	if (err < 0)
        return err;

	err = apds9960_set_gthr_in(client, data->gthr_in);
	if (err < 0)
        return err;

	err = apds9960_set_gthr_out(client, data->gthr_out);
	if (err < 0)
        return err;

	err = apds9960_set_gconf1(client, APDS9960_GESTURE_FIFO);
	if (err < 0)
        return err;

	err = apds9960_set_gconf2(client, APDS9960_GDRIVE|APDS9960_GGAIN|APDS9960_GTIME);
	if (err < 0)
        return err;

	err = apds9960_set_goffset_u(client, 0);
	if (err < 0)
        return err;

	err = apds9960_set_goffset_d(client, 0);
	if (err < 0)
        return err;

	err = apds9960_set_gpulse(client, (APDS9960_GPULSE-1)|APDS9960_GPULSE_LEN);
	if (err < 0)
        return err;

	err = apds9960_set_goffset_l(client, 0);
	if (err < 0)
        return err;

	err = apds9960_set_goffset_r(client, 0);
	if (err < 0)
        return err;

	err = apds9960_set_gconf3(client, 0);
	if (err < 0)
        return err;

	err = apds9960_set_gctrl(client, 0x04);
	if (err < 0)
        return err;

	// sensor is in disabled mode but all the configurations are preset
	err = apds9960_setup_eint(client);
	if(err != 0)
        return err;

	return SUCCESS;
}

/*----------------------------------------------------------------------------*/
static int apds9960_get_als_value(struct apds9960_data *obj, u16 als)
{
	int idx;
	
	for (idx = 0; idx < obj->als_level_num; idx++)
	{
		if (als < obj->hw->als_level[idx])
		{
			break;
		}
	}
	
	if (idx >= obj->als_value_num)
	{
		APS_ERR("APDS9930_get_als_value exceed range\n"); 
		idx = obj->als_value_num - 1;
	}
    APS_DBG("ALS: %05d => %05dï¼?idx=%d\n", als, obj->hw->als_value[idx], idx);	
    return obj->hw->als_value[idx];
	
}

/* Gionee BSP1 yang_yang modify for 20140802 CR01341808 begin */
static int apds9960_read_ps(struct apds9960_data *data, u16 *value)
{

#if 0
	struct i2c_client *client = data->client;
	int pdata;
	int tmp_data1, tmp_data2;
	APS_FUN();
	while(1)
    {
		pdata = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
		tmp_data1 = pdata;

		pdata = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);	
		tmp_data2 = pdata;

		if ((tmp_data2 - tmp_data1 > 27) || (tmp_data1 - tmp_data2 > 27))
			continue;
		break;
	}
	*value = pdata;
	#else
		struct i2c_client *client = data->client;
	int pdata;
	//int tmp_data1;
	pdata = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
	*value = pdata;
#endif	
	return SUCCESS;
}
/* Gionee BSP1 yang_yang modify for 20140802 CR01341808 end */

int apds9960_read_als(struct apds9960_data *data, u16 *value)
{
	//struct apds9960_data *data;
	struct i2c_client *client;
	unsigned char i2c_data[10];
	int status;
	int rflg=0;
	
	APS_FUN();
	//*data = *obj;
	client = data->client;
	if (APDS_ENABLE_ALS_NO_INT != data->enable_als_sensor)
		return ;
	
	status = i2c_smbus_read_i2c_block_data(client, APDS9960_CDATAL_REG, 8, (unsigned char*)i2c_data);
	if (status < 0)
        return ;
	if (status != 8)
        return ;
	
	data->cdata = (i2c_data[1]<<8)|i2c_data[0];
	data->rdata = (i2c_data[3]<<8)|i2c_data[2];
	data->gdata = (i2c_data[5]<<8)|i2c_data[4];
	data->bdata = (i2c_data[7]<<8)|i2c_data[6];
	APS_DBG("c:%d,r:%d,g:%d,b:%d", data->cdata, data->rdata, data->gdata, data->bdata);
	LuxCalculation(client);
	APS_DBG("lux:%d",data->lux);
	if (data->lux >= 0)
    {
		data->lux = data->lux<30000 ? data->lux : 30000;
	}
	*value = data->lux;
	//rflg=1;
	return SUCCESS;
}

//Gionee BSP1 yang_yang 20140802 modify for CR01341808 begin */
#if defined(GN_MTK_BSP_PS_DYNAMIC_CALIBRATION)
static int apds9960_dynamic_calibrate(void)
{
	int ret = 0;
	int i = 0;
	u16 data = 0;
	int noise = 0;
	int len = 0;
	int err = 0;
	int max = 0;
	int idx_table = 0;
	unsigned long data_total = 0;
	struct apds9960_data *obj = apds9960_obj;

	APS_FUN(f);
	if (!obj)
		goto err;

	for (i = 0; i < COUNT; i++)
    {
		//yang_yang 
		#if 0
		if (max++ > 10)
        {
			//atomic_set(&obj->ps_thd_val_high, PS_MAX_VALUE);
			//atomic_set(&obj->ps_thd_val_low, PS_MAX_VALUE);
			obj->piht = PS_MAX_VALUE;
			obj->pilt = PS_MAX_VALUE;
			goto err;
		}
		#endif
		mdelay(50);
		ret = apds9960_read_ps(obj, &data);
		#if 0
		if (ret != 0)
        {
			if (i>0) 
				i--;
			continue;
		}
		#endif
		if (data_total && (abs(data - data_total/(COUNT-1)) >50))
		{
			i=-1;
			data_total =0;
			APS_LOG("apds9930_dynamic_calibrate : data value >50 \n");
		}	
		else	
		{	
			data_total += data;
			APS_LOG("apds9930_dynamic_calibrate : data_total value ++ \n");
		}

		data_total += data;
	}
	noise = data_total/COUNT;

	for (idx_table = 0; idx_table < obj->ps_cali_noise_num; idx_table++)
    {
		if (noise <= obj->ps_cali_noise[idx_table])
			break;
	}
	
	if (noise >= 255) 
		goto err;

	obj->piht = obj->ps_cali_offset_high[idx_table] + noise;
	if (obj->piht >= PS_MAX_VALUE)
		obj->piht = PS_MAX_VALUE;

	obj->pilt = obj->ps_cali_offset_low[idx_table] + noise;
	if (obj->pilt >= PS_MAX_VALUE)
		obj->pilt = PS_MAX_VALUE;

	if (i2c_smbus_write_byte_data(obj->client, APDS9960_PITHI_REG, obj->piht))
    {
		goto err;
	}

	if (i2c_smbus_write_byte_data(obj->client, APDS9960_PITLO_REG, obj->pilt))
    {
		goto err;
	}
	
	APS_LOG("apds9960_dynamic_calibrate end:noise = %d, obj->pilt = %d , obj->piht = %d\n", 
			noise, obj->pilt, obj->piht);
	return SUCCESS;
	
err:
	APS_ERR("apds9930_dynamic_calibrate fail!!!\n");
	return -1;
}
#endif
//Gionee BSP1 yang_yang 20140802 modify for CR01341808 end */


int apds9960_gesture_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int value;
	int err = 0;
	hwm_sensor_data* sensor_data;
	struct apds9960_data *obj = (struct apds9960_data *)self;
    struct i2c_client *client = apds9960_i2c_client;

    APS_FUN();
	if (NULL == client)
    {
		APS_ERR("apds9960_ps_ioctl error: i2c driver not installed\n");
		return -EFAULT;
	}

	switch (command)
	{
		case SENSOR_DELAY:
			if ((NULL == buff_in) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if ((NULL == buff_in) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{	
				value = *(int *)buff_in;
				if (value)
				{
					if ((err = apds9960_enable_gesture_sensor(client, 1)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -EIO;
					}
					set_bit(CMC_BIT_PS, &obj->apds_enable);
				}
				else
				{
					if ((err = apds9960_enable_gesture_sensor(client, 0)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_PS, &obj->apds_enable);
				}
			}
			break;

		case SENSOR_GET_DATA:
			if ((NULL == buff_out) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			break;

		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -EINVAL;
			break;
	}
	
	return err;
}

int apds9960_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int value;
	int err = 0;
	struct i2c_client *client = apds9960_i2c_client;
	hwm_sensor_data* sensor_data;
	struct apds9960_data *obj = (struct apds9960_data *)self;
	
	APS_FUN();
	if (NULL == client)
    {
		APS_ERR("apds9960_ps_ioctl error: i2c driver not installed\n");
		return -EFAULT;
	}

	APS_DBG("%x\n", obj->client->addr);
	switch (command)
	{
		case SENSOR_DELAY:
			if ((NULL == buff_in) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if ((NULL == buff_in) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{	
				value = *(int *)buff_in;
				APS_LOG("%s enable value:%d\n", __func__, value);
				if (value)
				{
					if ((err = apds9960_enable_ps_sensor(client, APDS_ENABLE_PS)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -EIO;
					}
					set_bit(CMC_BIT_PS, &obj->apds_enable);
				}
				else
				{
					if ((err = apds9960_enable_ps_sensor(client, APDS_DISABLE_PS)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -EIO;
					}
					clear_bit(CMC_BIT_PS, &obj->apds_enable);
				}
			}
			break;

		case SENSOR_GET_DATA:
			if ((NULL == buff_out) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;	
				sensor_data->values[0] = i2c_smbus_read_byte_data(client, APDS9960_PDATA_REG);
				APS_LOG("%s value:%d", __func__, sensor_data->values[0]);
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;			
			}
			break;

		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -EINVAL;
			break;
	}
	
	return err;
}

int apds9960_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct apds9960_data *obj = (struct apds9960_data *)self;
	struct i2c_client *client = apds9960_i2c_client;

	if (NULL == client)
    {
		APS_ERR("apds9960_ps_ioctl error: i2c driver not installed\n");
		return -EFAULT;
	}

	switch (command)
	{
		case SENSOR_DELAY:
			if ((NULL == buff_in) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if ((NULL == buff_in) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;	
				APS_LOG("%s, value:%d", __func__, value);
				if (value)
				{
					if ((err = apds9960_enable_als_sensor(client, APDS_ENABLE_ALS_NO_INT)))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -EIO;
					}
					set_bit(CMC_BIT_ALS, &obj->apds_enable);
				}
				else
				{
					if ((err = apds9960_enable_als_sensor(client, APDS_DISABLE_ALS)))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -EIO;
					}
					clear_bit(CMC_BIT_ALS, &obj->apds_enable);
				}
				
			}
			break;

		case SENSOR_GET_DATA:
			if ((NULL == buff_out) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;
				apds9960_read_als(obj, &obj->als_data);				
//Gionee BSP1 yangqb 20140912 modify for CR01380122 LCM MODULE AAL SUPPORT start
#ifdef MTK_AAL_SUPPORT
				sensor_data->values[0] = obj->als_data;
#else
				sensor_data->values[0] = apds9960_get_als_value(obj, obj->als_data);
#endif
//Gionee BSP1 yangqb 20140912 modify for CR01380122 LCM MODULE AAL SUPPORT end
				APS_LOG("%s,sensor_data->values[0]:%d", __func__, sensor_data->values[0]);
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
			}
			break;

		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -EINVAL;
			break;
	}
	
	return err;
}

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int apds9960_get_ps_value(struct apds9960_data *obj, u16 ps)
{
	int val;
	static int val_temp=1;
	if (ps > obj->ps_threshold)
	{
		val = 0;  /*close*/
		val_temp = 0;
	}
	else if (ps < obj->ps_hysteresis_threshold)
	{
		val = 1;  /*far away*/
		val_temp = 1;
	}
	else
		val = val_temp;

	return val;
	
}

static int apds9960_open(struct inode *inode, struct file *file)
{
	file->private_data = apds9960_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}

static int apds9960_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;

	return SUCCESS;
}

static long apds9960_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct apds9960_data *obj = i2c_get_clientdata(client);  
	long err = 0;
	void __user *ptr = (void __user*) arg;
	int dat;
	uint32_t enable;
	int ps_result;
	int alsps_cali;
	int threshold[2];
	int i;
	int ps_noise[8];
#ifdef USE_DEFAULT_CAILMODE	
	int ps_cali;
#endif
#if defined(GN_MTK_BSP_PS_STATIC_CALIBRATION)
    struct PS_CALI_DATA_STRUCT ps_cali_temp = {{0,0,0},};
    //int dat;
#endif

#ifdef CUSTOM_KERNEL_SENSORHUB
		SCP_SENSOR_HUB_DATA data;
	    APDS9960_CUST_DATA *pCustData;
		int len;
	
		data.set_cust_req.sensorType = ID_PROXIMITY;
		data.set_cust_req.action = SENSOR_HUB_SET_CUST;
		pCustData = (APDS9960_CUST_DATA *)(&data.set_cust_req.custData);
#endif


	struct alsps_cal cal_data = {0};

	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if (copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if (enable)
			{
				if ((err = apds9960_enable_ps_sensor(obj->client, APDS_ENABLE_PS)))
				{
					APS_ERR("enable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				set_bit(CMC_BIT_PS, &obj->apds_enable);
			}
			else
			{
				if ((err = apds9960_enable_ps_sensor(obj->client, APDS_DISABLE_PS)))
				{
					APS_ERR("disable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				clear_bit(CMC_BIT_PS, &obj->apds_enable);
			}
			break;

		case ALSPS_GET_PS_MODE:
			enable = test_bit(CMC_BIT_PS, &obj->apds_enable) ? (1) : (0);
			if (copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:    
			obj->ps_data =	i2c_smbus_read_byte_data(obj->client, APDS9960_PDATA_REG);
			dat = apds9960_get_ps_value(obj, obj->ps_data);
			if (copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;

		case ALSPS_GET_PS_RAW_DATA:    		
			dat = i2c_smbus_read_byte_data(obj->client, APDS9960_PDATA_REG);
			if (copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;              

		case ALSPS_SET_ALS_MODE:
			if (copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if (enable)
			{
				if (err = apds9960_enable_als_sensor(client, APDS_ENABLE_ALS_NO_INT))
				{
					APS_ERR("enable als fail: %ld\n", err); 
					goto err_out;
				}
				set_bit(CMC_BIT_ALS, &obj->apds_enable);
			}
			else
			{
				if (err = apds9960_enable_als_sensor(client, APDS_DISABLE_ALS))
				{
					APS_ERR("disable als fail: %ld\n", err); 
					goto err_out;
				}
				clear_bit(CMC_BIT_ALS, &obj->apds_enable);
			}
			break;

		case ALSPS_GET_ALS_MODE:
			enable = test_bit(CMC_BIT_ALS, &obj->apds_enable) ? (1) : (0);
			if (copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_DATA: 
			apds9960_read_als(obj, &obj->als_data);
			dat = apds9960_get_als_value(obj, obj->als_data);
			if (copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		case ALSPS_GET_ALS_RAW_DATA:    
			apds9960_read_als(obj, &obj->als_data);
			dat = obj->als_data;
			if (copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		/* for factory mode test */
		case ALSPS_GET_PS_TEST_RESULT:
			dat = i2c_smbus_read_byte_data(obj->client, APDS9960_PDATA_REG);
			if (dat > obj->ps_threshold)
            {
                ps_result = 0;
            }
			else
                ps_result = 1;
				
			if (copy_to_user(ptr, &ps_result, sizeof(ps_result)))
			{
				err = -EFAULT;
				goto err_out;
			}			   
                break;

		case ALSPS_IOCTL_CLR_CALI:
#ifdef USE_DEFAULT_CAILMODE
			if (copy_from_user(&dat, ptr, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if (dat == 0)
				obj->ps_cali = 0;
	

#ifdef CUSTOM_KERNEL_SENSORHUB
                            pCustData->clearCali.action = APDS9960_CUST_ACTION_CLR_CALI;
                            len = offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + sizeof(pCustData->clearCali);
                            
                            err = SCP_sensorHub_req_send(&data, &len, 1);
#endif	
#endif

			break;

		case ALSPS_IOCTL_GET_CALI:
#ifdef USE_DEFAULT_CAILMODE
#ifdef CUSTOM_KERNEL_SENSORHUB
			ps_cali = obj->ps_cali ;
			APS_ERR("%s set ps_calix%x\n", __func__, obj->ps_cali);
			if(copy_to_user(ptr, &ps_cali, sizeof(ps_cali)))
			{
				err = -EFAULT;
				goto err_out;
			}			
#else
			cal_data = obj->cal ;
			//APS_LOG("%s, LUX_GA1:%d\n", __func__, cal_data.l_cal);
			//APS_LOG("%s, ur:%d,dl:%d\n", __func__, cal_data.p_cal_ur, cal_data.p_cal_dl);
			//APS_LOG("%s, u(%d)d(%d)l(%d)r(%d)\n", __func__, cal_data.g_cal_u, cal_data.g_cal_d,
			//	cal_data.g_cal_l, cal_data.g_cal_r);
			//APS_LOG("%s, g_ur:%d,g_dl:%d\n", __func__, cal_data.g_cal_ur, cal_data.g_cal_dl);
			if (copy_to_user(ptr, &cal_data, sizeof(cal_data)))
			{
				err = -EFAULT;
				goto err_out;
			}
#endif			
#endif			
			break;

		case ALSPS_IOCTL_SET_CALI:
#ifdef USE_DEFAULT_CAILMODE					
			if (copy_from_user(&alsps_cali, ptr, sizeof(alsps_cali)))
			{
				err = -EFAULT;
				goto err_out;
			}
			obj->ps_cali = ps_cali;
#ifdef CUSTOM_KERNEL_SENSORHUB
					pCustData->setCali.action =APDS9960_CUST_ACTION_SET_CALI;
					pCustData->setCali.cali = ps_cali;
				 	len = offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + sizeof(pCustData->setCali);
										
				     err = SCP_sensorHub_req_send(&data, &len, 1);
#endif
#else

			//APS_LOG("%s,alsps_cali:%d\n", __func__, alsps_cali);
			apds9960_calibration(client, alsps_cali);
#endif			
			break;

		case ALSPS_SET_PS_THRESHOLD:
		#ifdef USE_DEFAULT_CAILMODE
			if (copy_from_user(threshold, ptr, sizeof(threshold)))
			{
				err = -EFAULT;
				goto err_out;
			}
			APS_ERR("%s set threshold high: 0x%x, low: 0x%x\n", __func__, threshold[0],threshold[1]); 
			obj->ps_threshold=threshold[0]+obj->ps_cali;//atomic_set(&obj->ps_threshold,  (threshold[0]+obj->ps_cali));
			obj->ps_hysteresis_threshold=threshold[1]+obj->ps_cali;//atomic_set(&obj->ps_hysteresis_threshold,  (threshold[1]+obj->ps_cali));//need to confirm

			apds9960_set_psensor_threshold(obj->client,obj->ps_hysteresis_threshold,obj->ps_threshold);//set_psensor_threshold(obj->client);
		#endif
			break;
				
		case ALSPS_GET_PS_THRESHOLD_HIGH:
	#ifdef USE_DEFAULT_CAILMODE
			threshold[0] = obj->ps_threshold-obj->ps_cali; //atomic_read(&obj->ps_thd_val_high) - obj->ps_cali;
	#else
			threshold[0] = obj->ps_threshold; //atomic_read(&obj->ps_thd_val_high) - obj->ps_cali;
	#endif		
			APS_LOG("%s get threshold high: %d,cali:%d\n", __func__, threshold[0],obj->ps_cali); 
			if (copy_to_user(ptr, &threshold[0], sizeof(threshold[0])))
			{
				err = -EFAULT;
				goto err_out;
			}
			
			break;
				
		case ALSPS_GET_PS_THRESHOLD_LOW:
		#ifdef USE_DEFAULT_CAILMODE	
			threshold[1] = obj->ps_hysteresis_threshold-obj->ps_cali; //atomic_read(&obj->ps_thd_val_low) - obj->ps_cali;
		#else
			threshold[1] = obj->ps_hysteresis_threshold;
		#endif
			APS_LOG("%s get threshold low: 0x%x\n", __func__, threshold[1]); 
			if (copy_to_user(ptr, &threshold[1], sizeof(threshold[1])))
			{
				err = -EFAULT;
				goto err_out;
			}
			
			break;

        /* GESTURE */
#ifdef GESTURE_SUPPORT		
		case ALSPS_SET_GESTURE_MODE:
			if (copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if (enable)
			{
				if (err = apds9960_enable_gesture_sensor(client, 1))
				{
					APS_ERR("enable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				set_bit(CMC_BIT_PS, &obj->apds_enable);
			}
			else
			{
				if (err = apds9960_enable_gesture_sensor(client, 0))
				{
					APS_ERR("disable ps fail: %ld\n", err); 
					goto err_out;
				}
				gesture_motion_test = DIR_NONE;
				clear_bit(CMC_BIT_PS, &obj->apds_enable);
			}
            break;

	     case ALSPS_GET_GESTURE_MODE:
			enable = test_bit(CMC_BIT_PS, &obj->apds_enable) ? (1) : (0);
			if (copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
		     break;

		 case ALSPS_GET_GESTURE_DATA:
		 	dat = gesture_motion_test;
			if (copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
            break;

        /* Gionee BSP1 yang_yang 20140802 modify for CR01341808 begin */
		case ALSPS_GET_PS_NOISE:
			APS_LOG("%s command ALSPS_GET_PS_NOISE = 0x%0x4\n", __FUNCTION__, ALSPS_GET_PS_NOISE);
			if (err = apds9960_enable_ps_sensor(obj->client, APDS_ENABLE_PS))
            {
				APS_ERR("ads9960 ioctl enable ps fail: %d\n", err);
			}
			for (i=0; i<8; i++)
            {
				err = apds9960_read_ps(obj, &ps_noise[i]);
				if (err)
                {
					i--;
					continue;
				}
				msleep(50);
				APS_LOG("apds9960_read_ps noise[i]= %d\n", ps_noise[i]);
			}
			
			if (err = apds9960_enable_ps_sensor(obj->client, APDS_DISABLE_PS))
            {
				APS_ERR("ads9960 ioctl enable ps fail: %d\n", err);
			}
			
			if (copy_to_user(ptr, ps_noise, sizeof(ps_noise)))
            {	
				err = -EFAULT;
				goto err_out;
			}	 
			break;
#endif

#if defined(GN_MTK_BSP_PS_STATIC_CALIBRATION)
		case ALSPS_SET_PS_CALI:
			dat = (void __user*)arg;
			if (NULL == dat)
            {
				APS_ERR("dat is NULL\n");
				return -EINVAL;
			}
			if (copy_from_user(&ps_cali_temp, dat, sizeof(ps_cali_temp)))
            {
				APS_ERR("alsps set ps cali_copy from user fail\n");
				return -EFAULT;
			}
					
			apds9960_WriteCalibration(&ps_cali_temp);
			APS_LOG(" ALSPS_SET_PS_CALI %d,%d,%d\t", ps_cali_temp.close, ps_cali_temp.far_away, ps_cali_temp.valid);
			break;

		case ALSPS_GET_PS_CALI:
			if (err = apds9960_enable_ps_sensor(client, APDS_ENABLE_PS))
            {
				APS_ERR("ads9930 ioctl enable ps fail: %d\n", err);
			}
			else
            {
				apds9960_dynamic_calibrate();
				ps_cali_temp.valid = 1;
				ps_cali_temp.close = obj->piht;
				ps_cali_temp.far_away = obj->pilt;
			}

			if (err = apds9960_enable_ps_sensor(client, APDS_DISABLE_PS))
            {
				APS_ERR("apds9930 ioctl disable ps fail: %d\n", err);
			}
	
			if (copy_to_user((void __user *)arg, &ps_cali_temp, sizeof(ps_cali_temp)))
            {
				APS_ERR("alsps get ps cali_copy from user fail\n");
				return -EFAULT;
			}
			APS_LOG("apds9960 ALSPS_GET_PS_CALI %d,%d,%d\t",ps_cali_temp.close, ps_cali_temp.far_away,ps_cali_temp.valid);
			break;
#endif
        /* Gionee BSP1 yang_yang 20140802 modify for CR01341808 end */
		default:
		    APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
		    err = -ENOIOCTLCMD;
		    break;
	}

err_out:
	return err;    
}

/*----------------------------------------------------------------------------*/
static struct file_operations apds9960_fops = {
	.owner = THIS_MODULE,
	.open = apds9960_open,
	.release = apds9960_release,
	.unlocked_ioctl = apds9960_unlocked_ioctl,
};

static struct miscdevice apds9960_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &apds9960_fops,
};

/*
 * I2C init/probing/exit functions
 */
static struct platform_driver apds9960_alsps_driver;

/*----------------------------------------------------------------------------*/
static int apds9960_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) 
{    
	strcpy(info->type, APDS9960_DEV_NAME);
	return SUCCESS;
}

/*----------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
static int als_open_report_data(int open)
{
	//should queuq work to report event if  is_report_input_direct=true
	return 0;
}
/*--------------------------------------------------------------------------------*/
// if use  this typ of enable , Gsensor only enabled but not report inputEvent to HAL
static int als_enable_nodata(int en)
{
	int res = 0;
#ifdef CUSTOM_KERNEL_SENSORHUB
		SCP_SENSOR_HUB_DATA req;
		int len;
#endif
#ifdef CUSTOM_KERNEL_SENSORHUB
		req.activate_req.sensorType = ID_LIGHT;
		req.activate_req.action = SENSOR_HUB_ACTIVATE;
		req.activate_req.enable = en;
		len = sizeof(req.activate_req);
		res = SCP_sensorHub_req_send(&req, &len, 1);
#else

	
	if(!apds9960_obj)
	{
		APS_ERR("apds9960_obj is null!!\n");
		return -1;
	}
	APS_LOG("apds9960_obj als enable value = %d\n", en);

    if(en)
	{
		if((res = apds9960_enable_als_sensor(apds9960_i2c_client, APDS_ENABLE_ALS_NO_INT)))  //for polling mode
		{
			APS_ERR("enable als fail: %d\n", res); 
			return -1;
		}
		set_bit(CMC_BIT_ALS, &apds9960_obj->apds_enable);
	}
	else
	{
		if((res = apds9960_enable_als_sensor(apds9960_i2c_client, APDS_DISABLE_ALS)))
		{
			APS_ERR("disable als fail: %d\n", res); 
			return -1;
		}
		clear_bit(CMC_BIT_ALS, &apds9960_obj->apds_enable);
	}
    
	if(res){
		APS_ERR("als_enable_nodata is failed!!\n");
		return -1;
	}
#endif	
	return 0;
}
/*--------------------------------------------------------------------------------*/
static int als_set_delay(u64 ns)
{
	return 0;
}
/*--------------------------------------------------------------------------------*/
static int als_get_data(int* value, int* status)
{
	int err = 0;
#ifdef CUSTOM_KERNEL_SENSORHUB
    SCP_SENSOR_HUB_DATA req;
    int len;
#endif	
	
#ifdef CUSTOM_KERNEL_SENSORHUB
		req.get_data_req.sensorType = ID_LIGHT;
		req.get_data_req.action = SENSOR_HUB_GET_DATA;
		len = sizeof(req.get_data_req);
		err = SCP_sensorHub_req_send(&req, &len, 1);
		if(err)
		{
		  APS_ERR("SCP_sensorHub_req_send fail!\n");
		}
		else
		{
		  *value = req.get_data_rsp.int16_Data[0];
		  *status = SENSOR_STATUS_ACCURACY_MEDIUM;
		}
		
#else
	if(!apds9960_obj)
	{
		APS_ERR("apds9960_obj is null!!\n");
		return -1;
	}

	if((err = apds9960_read_als(apds9960_obj, &apds9960_obj->als_data))!=0)
	{
		err = -1;
	}
	else
	{
		*value = apds9960_get_als_value(apds9960_obj, apds9960_obj->als_data);
		*status = SENSOR_STATUS_ACCURACY_MEDIUM;
	}
#endif

	return err;
	
}
/*--------------------------------------------------------------------------------*/
// if use  this typ of enable , Gsensor should report inputEvent(x, y, z ,stats, div) to HAL
static int ps_open_report_data(int open)
{
	//should queuq work to report event if  is_report_input_direct=true
	return 0;
}
/*--------------------------------------------------------------------------------*/
// if use  this typ of enable , Gsensor only enabled but not report inputEvent to HAL
static int ps_enable_nodata(int en)
{
	int res = 0;
#ifdef CUSTOM_KERNEL_SENSORHUB
		SCP_SENSOR_HUB_DATA req;
		int len;
#endif

#ifdef CUSTOM_KERNEL_SENSORHUB
	APS_LOG("s_enable_nodata startp!!\n");

    req.activate_req.sensorType = ID_PROXIMITY;
    req.activate_req.action = SENSOR_HUB_ACTIVATE;
	req.activate_req.enable = en;
	len = sizeof(req.activate_req);
	res = SCP_sensorHub_req_send(&req, &len, 1);
#else	
	if(!apds9960_obj)
	{
		APS_ERR("apds9960_obj is null!!\n");
		return -1;
	}
	APS_LOG("apds9960_obj als enable value = %d\n", en);

    if(en)
	{
		if((res = apds9960_enable_ps_sensor(apds9960_i2c_client, 1)))
		{
			APS_ERR("enable ps fail: %d\n", res); 
			return -1;
		}
		set_bit(CMC_BIT_PS, &apds9960_obj->apds_enable);
	}
	else
	{
		if((res = apds9960_enable_ps_sensor(apds9960_i2c_client, 0)))
		{
			APS_ERR("disable ps fail: %d\n", res); 
			return -1;
		}
		clear_bit(CMC_BIT_PS, &apds9960_obj->apds_enable);
	}
  #endif  
	if(res){
		APS_ERR("als_enable_nodata is failed!!\n");
		return -1;
	}
    
	return 0;

}
/*--------------------------------------------------------------------------------*/
static int ps_set_delay(u64 ns)
{
	return 0;
}
/*--------------------------------------------------------------------------------*/
static int ps_get_data(int* value, int* status)
{
    int err = 0;
#ifdef CUSTOM_KERNEL_SENSORHUB
     SCP_SENSOR_HUB_DATA req;
     int len;
#endif

#ifdef CUSTOM_KERNEL_SENSORHUB
		 req.get_data_req.sensorType = ID_PROXIMITY;
		 req.get_data_req.action = SENSOR_HUB_GET_DATA;
		 len = sizeof(req.get_data_req);
		 err = SCP_sensorHub_req_send(&req, &len, 1);
		 if (err)
		 {
			  APS_ERR("SCP_sensorHub_req_send fail!\n");
		 }
		 else
		 {
			  *value = req.get_data_rsp.int16_Data[0];
			  *status = SENSOR_STATUS_ACCURACY_MEDIUM;
			  APS_LOG("get_data_rsp.int16_Data[0]= 0x%x\n", req.get_data_rsp.int16_Data[0]);
		 }
	
#else

    if(!apds9960_obj)
	{
		APS_ERR("apds9960_obj is null!!\n");
		return -1;
	}
    
    if((err = apds9960_read_ps(apds9960_obj, &apds9960_obj->ps_data)))
    {
        err = -1;
    }
    else
    {
        *value = apds9960_get_ps_value(apds9960_obj, apds9960_obj->ps_data);
        *status = SENSOR_STATUS_ACCURACY_MEDIUM;
    }
#endif  
	return err;
}

#ifdef CUSTOM_KERNEL_SENSORHUB
static void alsps_init_done_work(struct work_struct *work)
{
    struct apds9960_data *obj = apds9960_obj;
    APDS9960_CUST_DATA *p_cust_data;
    SCP_SENSOR_HUB_DATA data;
    int max_cust_data_size_per_packet;
    int i;
    uint sizeOfCustData;
    uint len;
    char *p = (char *)obj->hw;

	APS_FUN();

    p_cust_data = (APDS9960_CUST_DATA *)data.set_cust_req.custData;
    sizeOfCustData = sizeof(*(obj->hw));
    max_cust_data_size_per_packet = sizeof(data.set_cust_req.custData) - offsetof(APDS9960_SET_CUST, data);
    
    for (i=0;sizeOfCustData>0;i++)
    {

		data.set_cust_req.sensorType = ID_PROXIMITY;
		data.set_cust_req.action = SENSOR_HUB_SET_CUST;
		p_cust_data->setCust.action = APDS9960_CUST_ACTION_SET_CUST;
		p_cust_data->setCust.part = i;
        if (sizeOfCustData > max_cust_data_size_per_packet)
        {
            len = max_cust_data_size_per_packet;
        }
        else
        {
            len = sizeOfCustData;
        }

        memcpy(p_cust_data->setCust.data, p, len);
        sizeOfCustData -= len;
        p += len;
        len += offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + offsetof(APDS9960_SET_CUST, data);
        SCP_sensorHub_req_send(&data, &len, 1);
    }

	data.set_cust_req.sensorType = ID_PROXIMITY;
    data.set_cust_req.action = SENSOR_HUB_SET_CUST;
    p_cust_data->setEintInfo.action = APDS9960_CUST_ACTION_SET_EINT_INFO;
    p_cust_data->setEintInfo.gpio_mode = GPIO_ALS_EINT_PIN_M_EINT;
    p_cust_data->setEintInfo.gpio_pin = GPIO_ALS_EINT_PIN;
    p_cust_data->setEintInfo.eint_num = CUST_EINT_ALS_NUM;
    p_cust_data->setEintInfo.eint_is_deb_en = CUST_EINT_ALS_DEBOUNCE_EN;
    p_cust_data->setEintInfo.eint_type = CUST_EINT_ALS_TYPE;
    len = offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + sizeof(p_cust_data->setEintInfo);
    SCP_sensorHub_req_send(&data, &len, 1);
}
#endif

static int  apds9960_i2c_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct apds9960_data *data;
	//struct hwmsen_object obj_ps, obj_als, obj_gesture;
	int err = 0;


    struct als_control_path als_ctl={0};
	struct als_data_path als_data={0};
	struct ps_control_path ps_ctl={0};
	struct ps_data_path ps_data={0};


	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
    {
		err = -EIO;
		goto exit;
	}

/* Gionee BSP1 chengx 20140826 modify for CR01371160 begin */
#ifdef GN_MTK_BSP_DEVICECHECK
	struct gn_device_info gn_dev_info_lsensor = {0};
	gn_dev_info_lsensor.gn_dev_type = GN_DEVICE_TYPE_LIGHT;
	strcpy(gn_dev_info_lsensor.name, APDS9960_DEV_NAME);
	gn_set_device_info(gn_dev_info_lsensor);
#endif
/* Gionee BSP1 chengx 20140826 modify for CR01371160 end */

	data = kzalloc(sizeof(struct apds9960_data), GFP_KERNEL);
	if (!data)
    {
		err = -ENOMEM;
		goto exit_kfree;
	}
		memset(data, 0, sizeof(*data));

	apds9960_obj=data;
	data->client = client;
	apds9960_i2c_client = client;

	i2c_set_clientdata(client, data);
    data->hw = get_cust_alsps_hw_apds9960();
	data->enable = 0;                   // default mode is standard
	data->enable_als_sensor = 0;        // default to 0
	data->enable_ps_sensor = 0;         // default to 0
	data->enable_gesture_sensor = 0;	// default to 0

	data->als_poll_delay = 1000;	    // default to 1000ms
	data->als_atime_index = APDS9960_ALS_RES_24MS;  // APDS9960_ALS_RES_100MS;	// 100ms ATIME
	data->als_again_index = APDS9960_ALS_GAIN_1X;	// 1x AGAIN
	data->als_prev_lux = 0;
	data->enable_suspended_value = 0;

	data->ps_threshold = APDS9960_PS_DETECTION_THRESHOLD;
	data->ps_hysteresis_threshold = APDS9960_PS_HSYTERESIS_THRESHOLD;
	data->gesture_ppulse = (APDS9960_PPULSE_FOR_GESTURE-1)|APDS9960_PPULSE_LEN_FOR_GESTURE;
	data->ps_ppulse = (APDS9960_PPULSE_FOR_PS-1)|APDS9960_PPULSE_LEN_FOR_PS;
	data->ps_poffset_ur = 0;
	data->ps_poffset_dl = 0;
	data->gthr_in = GESTURE_GTHR_IN;
	data->gthr_out = GESTURE_GTHR_OUT;
	data->gesture_poffset_dl = 0;
	data->gesture_poffset_ur = 0;
	data->gesture_goffset_u = 0;
	data->gesture_goffset_d = 0;
	data->gesture_goffset_l = 0;
	data->gesture_goffset_r = 0;

	data->RGB_COE_X[0] = RGB_COE_X[0];
	data->RGB_COE_X[1] = RGB_COE_X[1];
	data->RGB_COE_X[2] = RGB_COE_X[2];

	data->RGB_COE_Y[0] = RGB_COE_Y[0];
	data->RGB_COE_Y[1] = RGB_COE_Y[1];
	data->RGB_COE_Y[2] = RGB_COE_Y[2];

	data->RGB_COE_Z[0] = RGB_COE_Z[0];
	data->RGB_COE_Z[1] = RGB_COE_Z[1];
	data->RGB_COE_Z[2] = RGB_COE_Z[2];

	data->lux_GA1 = APDS9960_LUX_GA1;
	data->lux_GA2 = APDS9960_LUX_GA2;
	data->lux_GA3 = APDS9960_LUX_GA3;
	
	data->cct_GA1 = APDS9960_CCT_GA1;
	data->cct_GA2 = APDS9960_CCT_GA2;
	data->cct_GA3 = APDS9960_CCT_GA3;

	data->ps_cali = 0;
	data->apds_enable = 0;	data->als_level_num = sizeof(data->hw->als_level)/sizeof(data->hw->als_level[0]);

	data->als_value_num = sizeof(data->hw->als_value)/sizeof(data->hw->als_value[0]);
	BUG_ON(sizeof(data->als_level) != sizeof(data->hw->als_level));
	memcpy(data->als_level, data->hw->als_level, sizeof(data->als_level));
	BUG_ON(sizeof(data->als_value) != sizeof(data->hw->als_value));
	memcpy(data->als_value, data->hw->als_value, sizeof(data->als_value));
	//set_bit(CMC_BIT_ALS, &data->apds_enable);
	//set_bit(CMC_BIT_PS, &data->apds_enable);

/* Gionee BSP1 yang_yang 20140802 modify for CR01341808 begin */
	ps_status=0;
#if defined(GN_MTK_BSP_PS_DYNAMIC_CALIBRATION)
	data->ps_cali_noise_num =  sizeof(data->hw->ps_cali_noise)/sizeof(data->hw->ps_cali_noise[0]);
	data->ps_cali_offset_high_num =	sizeof(data->hw->ps_cali_offset_high)/sizeof(data->hw->ps_cali_offset_high[0]);
	data->ps_cali_offset_low_num =  sizeof(data->hw->ps_cali_offset_low)/sizeof(data->hw->ps_cali_offset_low[0]);

	BUG_ON(sizeof(data->ps_cali_noise) != sizeof(data->hw->ps_cali_noise));
	memcpy(data->ps_cali_noise, data->hw->ps_cali_noise, sizeof(data->ps_cali_noise));

	BUG_ON(sizeof(data->ps_cali_offset_high) != sizeof(data->hw->ps_cali_offset_high));
	memcpy(data->ps_cali_offset_high, data->hw->ps_cali_offset_high, sizeof(data->ps_cali_offset_high));

	BUG_ON(sizeof(data->ps_cali_offset_low) != sizeof(data->hw->ps_cali_offset_low));
	memcpy(data->ps_cali_offset_low, data->hw->ps_cali_offset_low, sizeof(data->ps_cali_offset_low));
#endif
/* Gionee BSP1 yang_yang 20140802 modify for CR01341808 end */


	INIT_DELAYED_WORK(&data->psensor_dwork, apds9960_work_handler);
#ifdef CUSTOM_KERNEL_SENSORHUB
		INIT_DELAYED_WORK(&data->init_done_work, alsps_init_done_work);
#endif


	INIT_DELAYED_WORK(&data->gesture_dwork, apds9960_gesture_reporting_handler); 

	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
	/* kk 08-Jul-2014 */
	INIT_DELAYED_WORK(&data->xtalk_dwork, apds9960_xtalk_monitoring_handler); 
	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */

	APS_LOG("%s interrupt is hooked\n", __func__);

	/* Initialize the APDS9960 chip */
	err = apds9960_init_client(client);
	if (err)
    {
        APS_ERR("apds9960_init_client error!\n");
        goto exit_init_client_failed;
	}

	/* Register sysfs hooks */
	if (err = apds9960_create_attr(&apds9960_init_info.platform_diver_addr->driver)) {
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
	APS_LOG("%s before misc_register \n", __func__);

	/* Register for sensor ioctl */
	if ((err = misc_register(&apds9960_device)))
	{
		APS_ERR("APDS9930_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	if ((err = misc_register(&apds9960_ps_device)))
    {
		APS_ERR("Unalbe to register ps ioctl: %d", err);
		goto exit_misc_ps_device_register_failed;
	}

	if ((err = misc_register(&apds9960_als_device)))
    {
		APS_ERR("Unalbe to register als ioctl: %d", err);
		goto exit_misc_als_device_register_failed;
	}


	als_ctl.open_report_data= als_open_report_data;
	als_ctl.enable_nodata = als_enable_nodata;
	als_ctl.set_delay  = als_set_delay;
	als_ctl.is_report_input_direct = false;
#ifdef CUSTOM_KERNEL_SENSORHUB
	als_ctl.is_support_batch = data->hw->is_batch_supported_als;
#else
    als_ctl.is_support_batch = false;
#endif
	
	err = als_register_control_path(&als_ctl);
	if(err)
	{
		APS_ERR("register fail = %d\n", err);
		goto exit_unregister_sensor_operate;
	}

	als_data.get_data = als_get_data;
	als_data.vender_div = 100;
	err = als_register_data_path(&als_data);	
	if(err)
	{
		APS_ERR("tregister fail = %d\n", err);
		goto exit_unregister_sensor_operate;
	}

	
	ps_ctl.open_report_data= ps_open_report_data;
	ps_ctl.enable_nodata = ps_enable_nodata;
	ps_ctl.set_delay  = ps_set_delay;
	ps_ctl.is_report_input_direct = false;
#ifdef CUSTOM_KERNEL_SENSORHUB
	ps_ctl.is_support_batch = data->hw->is_batch_supported_ps;
#else
    ps_ctl.is_support_batch = false;
#endif
	
	err = ps_register_control_path(&ps_ctl);
	if(err)
	{
		APS_ERR("register fail = %d\n", err);
		goto exit_unregister_sensor_operate;
	}

	ps_data.get_data = ps_get_data;
	ps_data.vender_div = 100;
	err = ps_register_data_path(&ps_data);	
	if(err)
	{
		APS_ERR("tregister fail = %d\n", err);
		goto exit_unregister_sensor_operate;
	}



#if defined(CONFIG_HAS_EARLYSUSPEND)
	data->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	data->early_drv.suspend  = apds9960_early_suspend,
	data->early_drv.resume   = apds9960_late_resume,    
	register_early_suspend(&data->early_drv);
#endif
	APS_LOG("%s support ver. %s enabled\n", __func__, DRIVER_VERSION);
    apds9960_init_flag = 0;

	return SUCCESS;

exit_misc_als_device_register_failed:
	misc_deregister(&apds9960_ps_device);
exit_misc_ps_device_register_failed:
	misc_deregister(&apds9960_device);
exit_misc_device_register_failed:
    apds9960_delete_attr(&apds9960_init_info.platform_diver_addr->driver);
    
	//misc_deregister(&apds9960_device);
exit_create_attr_failed:
exit_init_client_failed:
exit_unregister_sensor_operate:
	kfree(data);
    apds9960_obj = NULL;
	apds9960_i2c_client = NULL;
exit_kfree:
exit:
	
    apds9960_init_flag = -1;
	return err;
}

static int  apds9960_i2c_remove(struct i2c_client *client)
{
	struct apds9960_data *data = i2c_get_clientdata(client);

    APS_FUN();
	__cancel_delayed_work(&data->psensor_dwork);
	__cancel_delayed_work(&data->gesture_dwork);

	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
	/* kk 08-Jul-2014 */
	__cancel_delayed_work(&data->xtalk_dwork);
	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */

	/* Power down the device */
	apds9960_set_enable(client, APDS9960_PWR_DOWN);

	misc_deregister(&apds9960_als_device);
	misc_deregister(&apds9960_ps_device);	

	apds9960_delete_attr(&apds9960_init_info.platform_diver_addr->driver);

	apds9960_i2c_client = NULL;

	kfree(data);

	return SUCCESS;
}

#if !defined(CONFIG_HAS_EARLYSUSPEND)
static int apds9960_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct apds9960_data *data = i2c_get_clientdata(client);

	APS_FUN();
	
	/* Do nothing as p-sensor is in active */
	if (data->enable_ps_sensor)
		return SUCCESS;

	data->enable_suspended_value = data->enable;
	
	apds9960_set_enable(client, APDS9960_PWR_DOWN);
	apds9960_clear_interrupt(client, CMD_CLR_ALL_INT);
	
	__cancel_delayed_work(&data->gesture_dwork);
	flush_delayed_work(&data->gesture_dwork);

	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */ 
	/* kk 08-Jul-2014 */
	__cancel_delayed_work(&data->xtalk_dwork);
	flush_delayed_work(&data->xtalk_dwork);
	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 end */

	return SUCCESS;
}

static int apds9960_i2c_resume(struct i2c_client *client)
{
	struct apds9960_data *data = i2c_get_clientdata(client);	
	int err = 0;

    APS_FUN();

	/* Do nothing as it was not suspended */
	APS_LOG("apds9960_i2c_resume (enable=%d)\n", data->enable_suspended_value);
	if (data->enable_ps_sensor)
    {
        APS_LOG("p-sensor is active!\n");
		return SUCCESS;     // if previously not suspended, leave it
    }

	err = apds9960_set_enable(client, data->enable_suspended_value);
	if (err < 0)
    {
		APS_ERR("enable set Fail\n");
		return 0;
	}

	apds9960_clear_interrupt(client, CMD_CLR_ALL_INT);	// clear pending interrupt

	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */
	/* kk 08-Jul-2014 */
	__cancel_delayed_work(&data->xtalk_dwork);
	flush_delayed_work(&data->xtalk_dwork);
	queue_delayed_work(apds_workqueue, &data->xtalk_dwork, msecs_to_jiffies(APDS9960_XTALK_MONITORING_TIMER_VALUE));
	/* Gionee BSP1 yang_yang 20140814 modify for CR01357229 begin */

	return SUCCESS;
}

#else

static void apds9960_early_suspend(struct early_suspend *h) 
{
	APS_FUN(); 
	struct apds9960_data *data = container_of(h, struct apds9960_data, early_drv);
	struct i2c_client *client = data->client;
	
	/* Do nothing as p-sensor is in active */
	if (data->enable_ps_sensor || data->enable_gesture_sensor)
    {
        APS_LOG("ps-sensor is enable!\n");
		return;
    }

	data->enable_suspended_value = data->enable;
	
	apds9960_set_enable(client, APDS9960_PWR_DOWN);
	apds9960_clear_interrupt(client, CMD_CLR_ALL_INT);
	
	__cancel_delayed_work(&data->gesture_dwork);
	flush_delayed_work(&data->gesture_dwork);
	
    APS_DBG("enable_suspended_value=%d\n", data->enable_suspended_value);
	return;
}

static void apds9960_late_resume(struct early_suspend *h)
{
	APS_FUN(); 
	struct apds9960_data *data = container_of(h, struct apds9960_data, early_drv);
	struct i2c_client *client = data->client;
	int err = 0;

	/* Do nothing as it was not suspended */
	APS_LOG("apds9960_late_resume (enable=%d)\n", data->enable_suspended_value);
	if (data->enable_ps_sensor || data->enable_gesture_sensor)
    {
        APS_LOG("ps-sensor is enable!\n");
		return;
    }

	err = apds9960_set_enable(client, data->enable_suspended_value);
	if (err < 0)
    {
		APS_ERR("enable set Fail\n");
		return;
	}

	apds9960_clear_interrupt(client, CMD_CLR_ALL_INT);	// clear pending interrupt

	return;
}
#endif

static struct i2c_driver apds9960_i2c_driver = {	
	.probe      = apds9960_i2c_probe,
	.remove     = apds9960_i2c_remove,
	.detect     = apds9960_i2c_detect,
#if !defined(CONFIG_HAS_EARLYSUSPEND)
	.suspend    = apds9960_i2c_suspend,
	.resume     = apds9960_i2c_resume,
#endif
	.id_table   = apds9960_i2c_id,
	.driver = {
//		.owner          = THIS_MODULE,
		.name           = APDS9960_DEV_NAME,
	},
};

/*----------------------------------------------------------------------------*/
static void apds9960_power(struct alsps_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	APS_DBG("power %s\n", on ? "on" : "off");
	if (POWER_NONE_MACRO != hw->power_id)
	{
		if (power_on == on)
		{
			APS_DBG("ignore power control: %d\n", on);
		}
		else if (on)
		{
			APS_DBG("hw->power_id:%d,hw->power_vol:%d\n", hw->power_id, hw->power_vol);
			if (!hwPowerOn(hw->power_id, hw->power_vol, APDS9960_DEV_NAME)) 
			{
				APS_ERR("power on fails!!\n");
			}
		}
		else
		{
			if (!hwPowerDown(hw->power_id, APDS9960_DEV_NAME)) 
			{
				APS_ERR("power off fail!!\n");   
			}
		}
	}
	power_on = on;
}

/*----------------------------------------------------------------------------*/
static int apds9960_probe(struct platform_device *pdev) 
{
	struct alsps_hw *hw = get_cust_alsps_hw_apds9960();

	APS_FUN();
	apds_workqueue = create_workqueue("proximity_als");
	if (!apds_workqueue)
    {
        APS_ERR("create_workqueue proximity_als failed!\n");
		return -ENOMEM;
	}

	apds9960_power(hw, POWER_ON);
	if (i2c_add_driver(&apds9960_i2c_driver))
	{
		APS_ERR("add driver error!\n");
		return -ENODEV;
	} 

	APS_LOG("%s exit!\n", __func__);
	return SUCCESS;
}

#if 0

static int apds9960_remove(struct platform_device *pdev)
{
	struct alsps_hw *hw = get_cust_alsps_hw_apds9960();

	APS_FUN(); 
 	if (apds_workqueue)
    {
		destroy_workqueue(apds_workqueue);
    }

	apds_workqueue = NULL;
	apds9960_power(hw, POWER_OFF);    
	i2c_del_driver(&apds9960_i2c_driver);

	return SUCCESS;
}

#if defined(CONFIG_OF)
static const struct of_device_id apds9960_of_match[] = {
	{ .compatible = "mediatek,als_ps", },
	{},
};
#endif


static struct platform_driver apds9960_alsps_driver = {
	.probe      = apds9960_probe,
	.remove     = apds9960_remove,    
	.driver     = {
        .name       = "als_ps",
    #if defined(CONFIG_OF)
        .of_match_table = apds9960_of_match,
    #endif
	}
};
#endif

static int apds9960_local_init(void)
{
    struct alsps_hw *hw = get_cust_alsps_hw_apds9960();

	APS_FUN();
	
	apds9960_power(hw, 1);
	
	if(i2c_add_driver(&apds9960_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	}

	APS_ERR("add driver epl2182_i2c_driver ok!\n");
	
	if(-1 == apds9960_init_flag)
	{
	   return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int apds9960_remove()
{
    struct alsps_hw *hw = get_cust_alsps_hw_apds9960();
    APS_FUN();
    apds9960_power(hw, 0);

    APS_ERR("apds9960 remove \n");
    i2c_del_driver(&apds9960_i2c_driver);
    return 0;
}




static int __init apds9960_init(void)
{
    APS_FUN();
	struct alsps_hw *hw = get_cust_alsps_hw_apds9960();
	APS_DBG("i2c_number=%d\n", hw->i2c_num);

	i2c_register_board_info(hw->i2c_num, &i2c_apds9960, 1);
	
	#if 1
	 alsps_driver_add(&apds9960_init_info);//(&epl2182_init_info);
	#else
	if (platform_driver_register(&apds9960_alsps_driver))
	{
		APS_ERR("failed to register driver!\n");
		return -ENODEV;
	}
	#endif
	return SUCCESS;
}

static void __exit apds9960_exit(void)
{
	APS_FUN();
//	platform_driver_unregister(&apds9960_alsps_driver);
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("apds9960 gesture + RGB + ambient light + proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds9960_init);
module_exit(apds9960_exit);

