/* tl5702 motion sensor driver
 *
 * Copyright (C) 2014 Technology Leaders & Innovators.
 *
 * File Name	 : tl5702.c
 * Author	 : TLi <http://www.tli.co.kr>
 * Version  	 : V 1.0.1
 *
 * REVISION HISTORY
 * 2014/07/08 : Fine offset calibration add 
 * 
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE


#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include "tl5702.h"
#include <linux/hwmsen_helper.h>

#define DATA_MAPPING	//for test
#define GSENSOR_NEW_ARCH  //for compatible
#ifdef GSENSOR_NEW_ARCH
#include <accel.h>
#endif
/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_TL5702 		0xA0
#define TL5702_I2C_SLAVE_ADDR 		0x56
#define TL5702_BUFSIZE				256
/*----------------------------------------------------------------------------*/
//jacky li //Drv. 2014_06_25 //
#define G_SENSOR_DEBUG   //open or close gsensor log
/*----------------------------------------------------------------------------*/
//#define CONFIG_TL5702_LOWPASS   /*apply low pass filter on output*/       
#define SW_CALIBRATION
//#define OFFSET_CALIBRATION		//by wskim 20140704	

/*----------------------------------------------------------------------------*/
#define TL5702_AXIS_X          0
#define TL5702_AXIS_Y          1
#define TL5702_AXIS_Z          2
#define TL5702_AXES_NUM        3
#define TL5702_DATA_LEN        6
#define TL5702_DEV_NAME        "tl5702"
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static const struct i2c_device_id tl5702_i2c_id[] = {{TL5702_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_tl5702={ I2C_BOARD_INFO(TL5702_DEV_NAME, (TL5702_I2C_SLAVE_ADDR))};


/*the adapter id will be available in customization*/
//static unsigned short tl5702_force[] = {0x00, TL5702_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const tl5702_forces[] = { tl5702_force, NULL };
//static struct i2c_client_address_data tl5702_addr_data = { .forces = tl5702_forces,};

/*----------------------------------------------------------------------------*/
static int tl5702_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int tl5702_i2c_remove(struct i2c_client *client);
static int tl5702_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(USE_EARLY_SUSPEND)  
static void tl5702_early_suspend(struct early_suspend *h); 
static void tl5702_late_resume(struct early_suspend *h);
#else
static int tl5702_suspend(struct i2c_client *client, pm_message_t msg); 
static int tl5702_resume(struct i2c_client *client);

#endif

#ifdef GSENSOR_NEW_ARCH
static int tl5702_local_init(void);
static int tl5702_local_remove(void);

#endif
/*----------------------------------------------------------------------------*/
typedef enum {
	ADX_TRC_DEBUG 	= 0x01,
    ADX_TRC_FILTER  = 0x02,
    ADX_TRC_RAWDATA = 0x04,
    ADX_TRC_DATA 	= 0x08,
    ADX_TRC_IOCTL   = 0x10,
    ADX_TRC_CALI	= 0X20,
    ADX_TRC_INFO	= 0X40,
} ADX_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};
/*----------------------------------------------------------------------------*/
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][TL5702_AXES_NUM];
    int sum[TL5702_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct tl5702_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;
    int 					layout;
    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                position;	
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[TL5702_AXES_NUM+1];

    /*data*/
    s8                      offset[TL5702_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[TL5702_AXES_NUM+1];

#if defined(CONFIG_TL5702_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif 
    /*early suspend*/
//#if defined(CONFIG_HAS_EARLYSUSPEND)
#if defined(USE_EARLY_SUSPEND)  
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver tl5702_i2c_driver = {
    .driver = {
//        .owner          = THIS_MODULE,
        .name           = TL5702_DEV_NAME,
    },
	.probe      		= tl5702_i2c_probe,
	.remove    			= tl5702_i2c_remove,
//	.detect				= tl5702_i2c_detect,
//#if !defined(CONFIG_HAS_EARLYSUSPEND) || 
#if !defined(CONFIG_HAS_EARLYSUSPEND) || !defined(USE_EARLY_SUSPEND) 

    .suspend            = tl5702_suspend,
    .resume             = tl5702_resume,
#endif
	.id_table = tl5702_i2c_id,
//	.address_data = &tl5702_addr_data,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *tl5702_i2c_client = NULL;
static struct platform_driver tl5702_gsensor_driver;
static struct tl5702_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = true;
static GSENSOR_VECTOR3D gsensor_gain;
static char selftestRes[8]= {0}; 
static DEFINE_MUTEX(tl5702_mutex);	
static bool enable_status = false;	

#ifdef GSENSOR_NEW_ARCH
static int tl5702_init_flag = -1;	//-1:init fail ,0: init ok

static struct acc_init_info tl5702_init_info =
{
	.name = TL5702_DEV_NAME,
	.init = tl5702_local_init,
	.uninit = tl5702_local_remove,
};
#endif


/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk( GSE_TAG"%s,%d\n", __FUNCTION__,__LINE__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#if defined(G_SENSOR_DEBUG)
#define GSE_LOG(fmt, args...)    printk( GSE_TAG fmt, ##args)
#else
#define GSE_LOG(fmt, args...) ((void)0)
#endif
/*----------------------------------------------------------------------------*/
static struct data_resolution tl5702_data_resolution[1] = {
 /* combination by {FULL_RES,RANGE}*/
    {{ 3, 9}, 256}, // dataformat +/-2g  in 10-bit resolution;  { 3, 9} = 3.9 = (2*2*1000)/(2^10);  256 = (2^10)/(2*2)          
};
/*----------------------------------------------------------------------------*/
static struct data_resolution tl5702_offset_resolution = {{3, 9}, 256};
/*----------------------------------------------------------------------------*/
static int tl5702_SetPowerMode(struct i2c_client *client, bool enable);
/*--------------------tl5702 power control function----------------------------------*/
static void tl5702_power(struct acc_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{        
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "TL5702"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "TL5702"))
			{
				GSE_ERR("power off fail!!\n");
			}			  
		}
	}
	power_on = on;    
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int tl5702_SetDataResolution(struct tl5702_i2c_data *obj)
{
/*set g sensor dataresolution here*/

/*TL5702 only can set to 10-bit dataresolution, so do nothing in tl5702 driver here*/

/*end of set dataresolution*/
 
/*we set measure range from -2g to +2g in tl5702_SetDataFormat(client, TL5702_RANGE_2G), 
                                                    and set 10-bit dataresolution tl5702_SetDataResolution()*/
                                                    
 /*so tl5702_data_resolution[0] set value as {{ 3, 9}, 256} when declaration, and assign the value to obj->reso here*/  

	tl5702_SetPowerMode(obj->client, true);		
 
 	obj->reso = &tl5702_data_resolution[0];
	return 0;
	
/*if you changed the measure range, for example call: tl5702_SetDataFormat(client, TL5702_RANGE_4G), 
you must set the right value to tl5702_data_resolution*/
}
/*----------------------------------------------------------------------------*/
#ifdef	OFFSET_CALIBRATION
static int tl5702_OffsetCalibration(struct i2c_client *client)
{
	u8 buf[6];
	int err = 0;
	int C_offset[3], fine_offset[3];
	int i;
	int fine_off_cal_flag = 0; 

	/* read fine_off_cal_flag */
	hwmsen_write_byte(client, 0x3D, 0x08);
	mdelay(5);
	hwmsen_read_byte(client, TL5702_TMODESEL_REG, &buf[0]);

	buf[0] &= 0xC0;
	buf[0] >>= 6;
	fine_off_cal_flag = buf[0];
	GSE_LOG("fine_off_cal_flag = %d\n", fine_off_cal_flag);

	if(fine_off_cal_flag < OFFSET_CAL_CNT)
	{
		C_offset[0] = (OFFSET_CAL_X/FINEOFFSET_RATIO);
		C_offset[1] = (OFFSET_CAL_Y/FINEOFFSET_RATIO);
		C_offset[2] = (OFFSET_CAL_Z/FINEOFFSET_RATIO);

		GSE_LOG("OFFSET_CAL_X = %d, OFFSET_CAL_Y = %d, OFFSET_CAL_Z = %d)\n", OFFSET_CAL_X, OFFSET_CAL_Y, OFFSET_CAL_Z);
		GSE_LOG("raw(C_offset[0] = %d, C_offset[1] = %d, C_offset[2] = %d)\n", C_offset[0], C_offset[1], C_offset[2]);
		
		/* read fine offset */
		hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x20);
		mdelay(5);
		for (i = 0; i < 3; i++)
			fine_offset[i] = 0;
		
		if(err = hwmsen_read_block(client, TL5702_F_X_OFF_H_REG, buf, 0x06))
		{
			GSE_ERR("error: %d\n", err);
		}

		for (i = 0; i < 3; i ++)
		{
			buf[i*2] &= 0x01;
			fine_offset[i] = (int)(s8)buf[i*2];
			fine_offset[i] <<= 8;
			buf[i*2+1] &= 0xFF;
			fine_offset[i] |= (int)buf[i*2+1];
		}
		GSE_LOG("fine(fine_offset[0] = 0x%02X, fine_offset[1] = 0x%02X, fine_offset[2] = 0x%02X)\n", fine_offset[0], fine_offset[1], fine_offset[2]);
		GSE_LOG("fine(fine_offset[0] = %d, fine_offset[1] = %d, fine_offset[2] = %d)\n", fine_offset[0], fine_offset[1], fine_offset[2]);

		fine_offset[0] = fine_offset[0] + C_offset[0];
		fine_offset[1] = fine_offset[1] + C_offset[1];
		fine_offset[2] = fine_offset[2] + C_offset[2];

		GSE_LOG("fine + C_off(fine_offset[0] = 0x%02X, fine_offset[1] = 0x%02X, fine_offset[2] = 0x%02X)\n", fine_offset[0], fine_offset[1], fine_offset[2]);
		GSE_LOG("fine + C_off(fine_offset[0] = %d, fine_offset[1] = %d, fine_offset[2] = %d)\n", fine_offset[0], fine_offset[1], fine_offset[2]);

		for (i = 0; i < 3; i ++)
		{
			buf[i*2] = (fine_offset[i] & 0x0100) >> 8; 
			buf[i*2+1] = fine_offset[i] & 0xFF;
		}

		GSE_LOG("buf(buf[0] = 0x%02X, buf[1] = 0x%02X)\n", buf[0], buf[1]);
		GSE_LOG("buf(buf[2] = 0x%02X, buf[3] = 0x%02X)\n", buf[2], buf[3]);
		GSE_LOG("buf(buf[4] = 0x%02X, buf[5] = 0x%02X)\n", buf[4], buf[5]);

		/* write fine offset */
		for (i = 0; i < 3; i ++)
		{
			buf[i*2] = (fine_offset[i] & 0x0100) >> 8; 
			buf[i*2+1] = fine_offset[i] & 0xFF;
			hwmsen_write_byte(client, TL5702_NVM_ADDR_REG, 0x06 + i);
			mdelay(20);
			hwmsen_write_byte(client, TL5702_NVM_DW_0_REG, buf[i*2]);
			mdelay(20);		
			hwmsen_write_byte(client, TL5702_NVM_DW_1_REG, buf[i*2+1]);
			mdelay(20); 	
			hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x08);
			mdelay(20);		
			hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x02);
			mdelay(20);		
			hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x00);
			mdelay(20);				
		}

		/* read fine offset */
		hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x20);
		mdelay(5);
		
		for (i = 0; i < 3; i++)
			fine_offset[i] = 0;
		
		if(err = hwmsen_read_block(client, TL5702_F_X_OFF_H_REG, buf, 0x06))
		{
			GSE_ERR("error: %d\n", err);
		}
		
		for (i = 0; i < 3; i ++)
		{
			buf[i*2] &= 0x01;
			fine_offset[i] = (int)(s8)buf[i*2];
			fine_offset[i] <<= 8;
			buf[i*2+1] &= 0xFF;
			fine_offset[i] |= (int)buf[i*2+1];
		}
		GSE_LOG("fine(fine_offset[0] = 0x%02X, fine_offset[1] = 0x%02X, fine_offset[2] = 0x%02X)\n", fine_offset[0], fine_offset[1], fine_offset[2]);
		GSE_LOG("fine(fine_offset[0] = %d, fine_offset[1] = %d, fine_offset[2] = %d)\n", fine_offset[0], fine_offset[1], fine_offset[2]);

		/* read fine_off_cal_flag */
		hwmsen_read_byte(client, TL5702_TMODESEL_REG, &buf[0]);
		buf[1] = buf[0] & 0xC0;
		buf[2] = buf[0] & 0x3F;

		buf[1] >>= 6;
		buf[1] += 1;
		GSE_LOG("fine_off_cal_flag = %d\n", buf[1]);

		buf[1] <<= 6;
		buf[0] = buf[1] | buf[2];
		
		hwmsen_read_byte(client, TL5702_BG_TRIM_REG, &buf[1]);

		hwmsen_write_byte(client, TL5702_NVM_ADDR_REG, 0x04);
		mdelay(20);
		hwmsen_write_byte(client, TL5702_NVM_DW_0_REG, buf[0]);
		mdelay(20);		
		hwmsen_write_byte(client, TL5702_NVM_DW_1_REG, buf[1]);
		mdelay(20); 	
		hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x08);
		mdelay(20);		
		hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x02);
		mdelay(20);		
		hwmsen_write_byte(client, TL5702_NVM_CTL_REG, 0x00);
		mdelay(20);				
	}
	else
	{
		GSE_LOG("fine_off_cal_flag is greater than %d\n",OFFSET_CAL_CNT);
	}
}
#endif
/*----------------------------------------------------------------------------*/
static int tl5702_ReadData(struct i2c_client *client, s16 data[TL5702_AXES_NUM])
{
	struct tl5702_i2c_data *priv = i2c_get_clientdata(client);        
	u8 addr = TL5702_ACC_REG;
	u8 buf[TL5702_DATA_LEN] = {0};
	int err = 0;
	int i;
	int tmp=0;
	u8 ofs[3];
	
	if(NULL == client)
	{
		err = -EINVAL;
	}
	else if(err = hwmsen_read_block(client, addr, buf, 0x06))
	{
		GSE_ERR("error: %d\n", err);
	}
	else
	{
		data[TL5702_AXIS_X] = (s16)((buf[TL5702_AXIS_X*2] << 2) |
		         (buf[TL5702_AXIS_X*2+1] & 0x3));
		data[TL5702_AXIS_Y] = (s16)((buf[TL5702_AXIS_Y*2] << 2) |
		         (buf[TL5702_AXIS_Y*2+1] & 0x3));
		data[TL5702_AXIS_Z] = (s16)((buf[TL5702_AXIS_Z*2] << 2) |
		         (buf[TL5702_AXIS_Z*2+1] & 0x3));

		for(i=0;i<3;i++)				
		{								//because the data is store in binary complement number formation in computer system
			if ( data[i] == 0x0200 )	//so we want to calculate actual number here
				data[i]= -512;			//10bit resolution, 512= 2^(10-1)
			else if ( data[i] & 0x0200 )//transfor format
			{							//GSE_LOG("data 0 step %x \n",data[i]);
				data[i] -= 0x1; 		//GSE_LOG("data 1 step %x \n",data[i]);
				data[i] = ~data[i]; 	//GSE_LOG("data 2 step %x \n",data[i]);
				data[i] &= 0x01ff;		//GSE_LOG("data 3 step %x \n\n",data[i]);
				data[i] = -data[i]; 	
			}
		}	

		if(atomic_read(&priv->trace) & ADX_TRC_RAWDATA)		//by wskim 20140318	
	//	if(1)
		{
			GSE_LOG("[%08X %08X %08X] => [%5d %5d %5d]\n", data[TL5702_AXIS_X], data[TL5702_AXIS_Y], data[TL5702_AXIS_Z],
		                               data[TL5702_AXIS_X], data[TL5702_AXIS_Y], data[TL5702_AXIS_Z]);
		}
#ifdef CONFIG_TL5702_LOWPASS
		if(atomic_read(&priv->filter))
		{
			if(atomic_read(&priv->fir_en) && !atomic_read(&priv->suspend))
			{
				int idx, firlen = atomic_read(&priv->firlen);   
				if(priv->fir.num < firlen)
				{                
					priv->fir.raw[priv->fir.num][TL5702_AXIS_X] = data[TL5702_AXIS_X];
					priv->fir.raw[priv->fir.num][TL5702_AXIS_Y] = data[TL5702_AXIS_Y];
					priv->fir.raw[priv->fir.num][TL5702_AXIS_Z] = data[TL5702_AXIS_Z];
					priv->fir.sum[TL5702_AXIS_X] += data[TL5702_AXIS_X];
					priv->fir.sum[TL5702_AXIS_Y] += data[TL5702_AXIS_Y];
					priv->fir.sum[TL5702_AXIS_Z] += data[TL5702_AXIS_Z];
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d]\n", priv->fir.num,
							priv->fir.raw[priv->fir.num][TL5702_AXIS_X], priv->fir.raw[priv->fir.num][TL5702_AXIS_Y], priv->fir.raw[priv->fir.num][TL5702_AXIS_Z],
							priv->fir.sum[TL5702_AXIS_X], priv->fir.sum[TL5702_AXIS_Y], priv->fir.sum[TL5702_AXIS_Z]);
					}
					priv->fir.num++;
					priv->fir.idx++;
				}
				else
				{
					idx = priv->fir.idx % firlen;
					priv->fir.sum[TL5702_AXIS_X] -= priv->fir.raw[idx][TL5702_AXIS_X];
					priv->fir.sum[TL5702_AXIS_Y] -= priv->fir.raw[idx][TL5702_AXIS_Y];
					priv->fir.sum[TL5702_AXIS_Z] -= priv->fir.raw[idx][TL5702_AXIS_Z];
					priv->fir.raw[idx][TL5702_AXIS_X] = data[TL5702_AXIS_X];
					priv->fir.raw[idx][TL5702_AXIS_Y] = data[TL5702_AXIS_Y];
					priv->fir.raw[idx][TL5702_AXIS_Z] = data[TL5702_AXIS_Z];
					priv->fir.sum[TL5702_AXIS_X] += data[TL5702_AXIS_X];
					priv->fir.sum[TL5702_AXIS_Y] += data[TL5702_AXIS_Y];
					priv->fir.sum[TL5702_AXIS_Z] += data[TL5702_AXIS_Z];
					priv->fir.idx++;
					data[TL5702_AXIS_X] = priv->fir.sum[TL5702_AXIS_X]/firlen;
					data[TL5702_AXIS_Y] = priv->fir.sum[TL5702_AXIS_Y]/firlen;
					data[TL5702_AXIS_Z] = priv->fir.sum[TL5702_AXIS_Z]/firlen;
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d] : [%5d %5d %5d]\n", idx,
						priv->fir.raw[idx][TL5702_AXIS_X], priv->fir.raw[idx][TL5702_AXIS_Y], priv->fir.raw[idx][TL5702_AXIS_Z],
						priv->fir.sum[TL5702_AXIS_X], priv->fir.sum[TL5702_AXIS_Y], priv->fir.sum[TL5702_AXIS_Z],
						data[TL5702_AXIS_X], data[TL5702_AXIS_Y], data[TL5702_AXIS_Z]);
					}
				}
			}
		}	
#endif         
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int tl5702_ReadOffset(struct i2c_client *client, s8 ofs[TL5702_AXES_NUM])
{    
	int err;

	ofs[1]=ofs[2]=ofs[0]=0x00;

	GSE_LOG("offesx=%x, y=%x, z=%x",ofs[0],ofs[1],ofs[2]);
	
	return err;    
}
/*----------------------------------------------------------------------------*/
static int tl5702_ResetCalibration(struct i2c_client *client)
{
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	u8 ofs[4]={0,0,0,0};
	int err;

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	memset(obj->offset, 0x00, sizeof(obj->offset));
	return err;    
}
/*----------------------------------------------------------------------------*/
static int tl5702_ReadCalibration(struct i2c_client *client, int dat[TL5702_AXES_NUM])
{
    struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
    int err;
    int mul;

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
	    if ((err = tl5702_ReadOffset(client, obj->offset))) {
        GSE_ERR("read offset fail, %d\n", err);
        return err;
    	}    
    	mul = obj->reso->sensitivity/tl5702_offset_resolution.sensitivity;
	#endif
#ifdef DATA_MAPPING	
	dat[TL5702_AXIS_X] = obj->cvt.sign[TL5702_AXIS_X]*(obj->offset[obj->cvt.map[TL5702_AXIS_X]]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[obj->cvt.map[TL5702_AXIS_X]]);
    dat[TL5702_AXIS_Y] = obj->cvt.sign[TL5702_AXIS_Y]*(obj->offset[obj->cvt.map[TL5702_AXIS_Y]]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[obj->cvt.map[TL5702_AXIS_Y]]);
    dat[TL5702_AXIS_Z] = obj->cvt.sign[TL5702_AXIS_Z]*(obj->offset[obj->cvt.map[TL5702_AXIS_Z]]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[obj->cvt.map[TL5702_AXIS_Z]]);                        
#else
    dat[obj->cvt.map[TL5702_AXIS_X]] = obj->cvt.sign[TL5702_AXIS_X]*(obj->offset[TL5702_AXIS_X]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[TL5702_AXIS_X]);
    dat[obj->cvt.map[TL5702_AXIS_Y]] = obj->cvt.sign[TL5702_AXIS_Y]*(obj->offset[TL5702_AXIS_Y]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[TL5702_AXIS_Y]);
    dat[obj->cvt.map[TL5702_AXIS_Z]] = obj->cvt.sign[TL5702_AXIS_Z]*(obj->offset[TL5702_AXIS_Z]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[TL5702_AXIS_Z]);                        
#endif                                       
    return 0;
}
/*----------------------------------------------------------------------------*/
static int tl5702_ReadCalibrationEx(struct i2c_client *client, int act[TL5702_AXES_NUM], int raw[TL5702_AXES_NUM])
{  
	/*raw: the raw calibration data; act: the actual calibration data*/
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int mul;

 

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
		if(err = tl5702_ReadOffset(client, obj->offset))
		{
			GSE_ERR("read offset fail, %d\n", err);
			return err;
		}   
		mul = obj->reso->sensitivity/tl5702_offset_resolution.sensitivity;
	#endif
	
	raw[TL5702_AXIS_X] = obj->offset[TL5702_AXIS_X]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[TL5702_AXIS_X];
	raw[TL5702_AXIS_Y] = obj->offset[TL5702_AXIS_Y]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[TL5702_AXIS_Y];
	raw[TL5702_AXIS_Z] = obj->offset[TL5702_AXIS_Z]*mul*GRAVITY_EARTH_1000/(obj->reso->sensitivity) + obj->cali_sw[TL5702_AXIS_Z];
#ifdef DATA_MAPPING
	act[TL5702_AXIS_X] = obj->cvt.sign[TL5702_AXIS_X]*raw[obj->cvt.map[TL5702_AXIS_X]];
	act[TL5702_AXIS_Y] = obj->cvt.sign[TL5702_AXIS_Y]*raw[obj->cvt.map[TL5702_AXIS_Y]];
	act[TL5702_AXIS_Z] = obj->cvt.sign[TL5702_AXIS_Z]*raw[obj->cvt.map[TL5702_AXIS_Z]];                        
#else
	act[obj->cvt.map[TL5702_AXIS_X]] = obj->cvt.sign[TL5702_AXIS_X]*raw[TL5702_AXIS_X];
	act[obj->cvt.map[TL5702_AXIS_Y]] = obj->cvt.sign[TL5702_AXIS_Y]*raw[TL5702_AXIS_Y];
	act[obj->cvt.map[TL5702_AXIS_Z]] = obj->cvt.sign[TL5702_AXIS_Z]*raw[TL5702_AXIS_Z];                        
#endif                       
	return 0;
}
/*----------------------------------------------------------------------------*/
static int tl5702_WriteCalibration(struct i2c_client *client, int dat[TL5702_AXES_NUM])
{
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int cali[TL5702_AXES_NUM], raw[TL5702_AXES_NUM];
	int lsb = tl5702_offset_resolution.sensitivity;
	int divisor = obj->reso->sensitivity/lsb;

	if(err = tl5702_ReadCalibrationEx(client, cali, raw))	/*offset will be updated in obj->offset*/
	{ 
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}

	GSE_LOG("OLDOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		raw[TL5702_AXIS_X], raw[TL5702_AXIS_Y], raw[TL5702_AXIS_Z],
		obj->offset[TL5702_AXIS_X], obj->offset[TL5702_AXIS_Y], obj->offset[TL5702_AXIS_Z],
		obj->cali_sw[TL5702_AXIS_X], obj->cali_sw[TL5702_AXIS_Y], obj->cali_sw[TL5702_AXIS_Z]);

	/*calculate the real offset expected by caller*/
	cali[TL5702_AXIS_X] += dat[TL5702_AXIS_X];
	cali[TL5702_AXIS_Y] += dat[TL5702_AXIS_Y];
	cali[TL5702_AXIS_Z] += dat[TL5702_AXIS_Z];

	GSE_LOG("UPDATE: (%+3d %+3d %+3d)\n", 
		dat[TL5702_AXIS_X], dat[TL5702_AXIS_Y], dat[TL5702_AXIS_Z]);

#ifdef SW_CALIBRATION
#ifdef DATA_MAPPING
	obj->cali_sw[obj->cvt.map[TL5702_AXIS_X]] = obj->cvt.sign[TL5702_AXIS_X]*(cali[TL5702_AXIS_X]);
	obj->cali_sw[obj->cvt.map[TL5702_AXIS_Y]] = obj->cvt.sign[TL5702_AXIS_Y]*(cali[TL5702_AXIS_Y]);
	obj->cali_sw[obj->cvt.map[TL5702_AXIS_Z]] = obj->cvt.sign[TL5702_AXIS_Z]*(cali[TL5702_AXIS_Z]);	

#else
	obj->cali_sw[TL5702_AXIS_X] = obj->cvt.sign[TL5702_AXIS_X]*(cali[obj->cvt.map[TL5702_AXIS_X]]);
	obj->cali_sw[TL5702_AXIS_Y] = obj->cvt.sign[TL5702_AXIS_Y]*(cali[obj->cvt.map[TL5702_AXIS_Y]]);
	obj->cali_sw[TL5702_AXIS_Z] = obj->cvt.sign[TL5702_AXIS_Z]*(cali[obj->cvt.map[TL5702_AXIS_Z]]);	
#endif
#else
#ifdef DATA_MAPPING
	obj->offset[obj->cvt.map[TL5702_AXIS_X]] = (s8)(obj->cvt.sign[TL5702_AXIS_X]*(cali[TL5702_AXIS_X])/(divisor)*(obj->reso->sensitivity)/GRAVITY_EARTH_1000);
	obj->offset[obj->cvt.map[TL5702_AXIS_Y]] = (s8)(obj->cvt.sign[TL5702_AXIS_Y]*(cali[TL5702_AXIS_Y])/(divisor)*(obj->reso->sensitivity)/GRAVITY_EARTH_1000);
	obj->offset[obj->cvt.map[TL5702_AXIS_Z]] = (s8)(obj->cvt.sign[TL5702_AXIS_Z]*(cali[TL5702_AXIS_Z])/(divisor)*(obj->reso->sensitivity)/GRAVITY_EARTH_1000);

	/*convert software calibration using standard calibration*/
	obj->cali_sw[obj->cvt.map[TL5702_AXIS_X]] = obj->cvt.sign[TL5702_AXIS_X]*(cali[TL5702_AXIS_X])%(divisor);
	obj->cali_sw[obj->cvt.map[TL5702_AXIS_Y]] = obj->cvt.sign[TL5702_AXIS_Y]*(cali[TL5702_AXIS_Y])%(divisor);
	obj->cali_sw[obj->cvt.map[TL5702_AXIS_Z]] = obj->cvt.sign[TL5702_AXIS_Z]*(cali[TL5702_AXIS_Z])%(divisor);

#else

	obj->offset[TL5702_AXIS_X] = (s8)(obj->cvt.sign[TL5702_AXIS_X]*(cali[obj->cvt.map[TL5702_AXIS_X]])/(divisor)*(obj->reso->sensitivity)/GRAVITY_EARTH_1000);
	obj->offset[TL5702_AXIS_Y] = (s8)(obj->cvt.sign[TL5702_AXIS_Y]*(cali[obj->cvt.map[TL5702_AXIS_Y]])/(divisor)*(obj->reso->sensitivity)/GRAVITY_EARTH_1000);
	obj->offset[TL5702_AXIS_Z] = (s8)(obj->cvt.sign[TL5702_AXIS_Z]*(cali[obj->cvt.map[TL5702_AXIS_Z]])/(divisor)*(obj->reso->sensitivity)/GRAVITY_EARTH_1000);

	/*convert software calibration using standard calibration*/
	obj->cali_sw[TL5702_AXIS_X] = obj->cvt.sign[TL5702_AXIS_X]*(cali[obj->cvt.map[TL5702_AXIS_X]])%(divisor);
	obj->cali_sw[TL5702_AXIS_Y] = obj->cvt.sign[TL5702_AXIS_Y]*(cali[obj->cvt.map[TL5702_AXIS_Y]])%(divisor);
	obj->cali_sw[TL5702_AXIS_Z] = obj->cvt.sign[TL5702_AXIS_Z]*(cali[obj->cvt.map[TL5702_AXIS_Z]])%(divisor);
#endif
	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		obj->offset[TL5702_AXIS_X]*divisor + obj->cali_sw[TL5702_AXIS_X], 
		obj->offset[TL5702_AXIS_Y]*divisor + obj->cali_sw[TL5702_AXIS_Y], 
		obj->offset[TL5702_AXIS_Z]*divisor + obj->cali_sw[TL5702_AXIS_Z], 
		obj->offset[TL5702_AXIS_X], obj->offset[TL5702_AXIS_Y], obj->offset[TL5702_AXIS_Z],
		obj->cali_sw[TL5702_AXIS_X], obj->cali_sw[TL5702_AXIS_Y], obj->cali_sw[TL5702_AXIS_Z]);

	if(err = hwmsen_write_block(obj->client, TL5702_REG_OFSX, obj->offset, TL5702_AXES_NUM))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
#endif

	return err;
}
/*----------------------------------------------------------------------------*/
static int tl5702_CheckDeviceID(struct i2c_client *client)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = TL5702_DEV_ID_REG;   

	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto exit_TL5702_CheckDeviceID;
	}
	
	udelay(500);

	databuf[0] = 0x0;        
	res = i2c_master_recv(client, databuf, 0x01);
	if(res <= 0)
	{
		goto exit_TL5702_CheckDeviceID;
	}
	
	if(databuf[0]!=I2C_DRIVERID_TL5702)
	{
		GSE_LOG("TL5702_CheckDeviceID 0x%x failt!\n ", databuf[0]);
		return TL5702_ERR_IDENTIFICATION;
	}

	GSE_LOG("TL5702_CheckDeviceID 0x%x pass!\n ", databuf[0]);
	return TL5702_SUCCESS;
	
exit_TL5702_CheckDeviceID:
	if (res <= 0)
	{
		return TL5702_ERR_I2C;
	}	
}
/*----------------------------------------------------------------------------*/
static int tl5702_SetPowerMode(struct i2c_client *client, bool enable)
{
	u8 databuf[2];    
	int res = 0;
	u8 addr = TL5702_LPWR_MODE_REG;
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	
	
	if(enable == sensor_power)
	{
		if((obj!=NULL) && (atomic_read(&(obj->trace))!=0))
		{
			GSE_LOG("Sensor power status is newest!\n");
		}
		return TL5702_SUCCESS;
	}

	if(hwmsen_read_block(client, addr, databuf, 0x01))
	{
		GSE_ERR("read power ctl register err!\n");
		return TL5702_ERR_I2C;
	}

	
	if(enable == FALSE)
	{
		databuf[0] |= TL5702_DSLP_EN_MASK;
	}
	else
	{
		databuf[0] &= ~TL5702_DSLP_EN_MASK;
	}
	databuf[1] = databuf[0];
	databuf[0] = TL5702_LPWR_MODE_REG;
	

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return TL5702_ERR_I2C;
	}


	GSE_LOG("TL5702_SetPowerMode %d!\n ",enable);


	sensor_power = enable;

	mdelay(1);	//by wskim 20140317	
	
	return TL5702_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int tl5702_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);  

	tl5702_SetPowerMode(client, false);

	if(hwmsen_read_block(client, TL5702_RANGE_REG, databuf, 0x01))
	{
		GSE_LOG("tl5702 read Dataformat failt \n");
		return TL5702_ERR_I2C;
	}

	databuf[0] &= ~TL5702_RANGE_MASK;
	databuf[0] |= dataformat;
	databuf[1] = databuf[0];
	databuf[0] = TL5702_RANGE_REG;
	
	GSE_LOG("databuf[0] = 0x%02X,databuf[1] = 0x%02X\n",databuf[0], databuf[1]);	//by wskim 20140318

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return TL5702_ERR_I2C;
	}

	//tl5702_SetPowerMode(client, true);
	tl5702_SetPowerMode(client, enable_status);

	GSE_LOG("TL5702_SetDataFormat OK! \n");

	return tl5702_SetDataResolution(obj);    
}
/*----------------------------------------------------------------------------*/
static int tl5702_SetBWRate(struct i2c_client *client, u8 bwrate)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    

	tl5702_SetPowerMode(client, false);

	if(hwmsen_read_block(client, TL5702_BANDWIDTH_REG, databuf, 0x01))
	{
		GSE_LOG("tl5702 read rate failt \n");
		return TL5702_ERR_I2C;
	}

	//GSE_ERR("databuf[0] = 0x%02X,databuf[1] = 0x%02X\n",databuf[0], databuf[1]);	//by wskim 20140324
	//GSE_ERR("TL5702_BANDWIDTH_MASK = 0x%02X\n",TL5702_BANDWIDTH_MASK);	//by wskim 20140324
	databuf[0] &= ~TL5702_BANDWIDTH_MASK;
	//GSE_ERR("databuf[0] = 0x%02X,databuf[1] = 0x%02X\n",databuf[0], databuf[1]);	//by wskim 20140324
	databuf[0] |= bwrate << TL5702_BANDWIDTH_SHIFT;		//by wskim 20140324	
	//GSE_ERR("databuf[0] = 0x%02X,dtabuf[1] = 0x%02X\n",databuf[0], databuf[1]);	//by wskim 20140324
	databuf[1] = databuf[0];
	//GSE_ERR("databuf[0] = 0x%02X,databuf[1] = 0x%02X\n",databuf[0], databuf[1]);	//by wskim 20140324
	databuf[0] = TL5702_BANDWIDTH_REG;

	GSE_ERR("databuf[0] = 0x%02X,databuf[1] = 0x%02X\n",databuf[0], databuf[1]);	//by wskim 20140318

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return TL5702_ERR_I2C;
	}


	//tl5702_SetPowerMode(client, true);
	tl5702_SetPowerMode(client, enable_status);


	GSE_LOG("TL5702_SetBWRate OK! \n");

	return TL5702_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int tl5702_SetIntEnable(struct i2c_client *client, u8 intenable)
{
	u8 databuf[10];    
	int res = 0;

	res = hwmsen_write_byte(client, TL5702_INTEN_0_REG, 0x00);
	if(res != TL5702_SUCCESS) 
	{
		return res;
	}
	res = hwmsen_write_byte(client, TL5702_INTEN_1_REG, 0x00);
	if(res != TL5702_SUCCESS) 
	{
		return res;
	}
	GSE_LOG("TL5702 disable interrupt ...\n");

	return TL5702_SUCCESS;   
}
/*----------------------------------------------------------------------------*/
static int tl5702_init_client(struct i2c_client *client, int reset_cali)
{
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;
	GSE_LOG("tl5702_init_client \n");

	res = tl5702_CheckDeviceID(client); 
	if(res != TL5702_SUCCESS)
	{		
		GSE_ERR("tl5702_CheckDeviceID failed! \n");
		return res;
	}
#if 0
	res = tl5702_SetPowerMode(client, enable_status);//false
	if(res != TL5702_SUCCESS)
	{
		GSE_ERR("tl5702_SetPowerMode failed! \n");
		return res;
	}
#endif

	res = tl5702_SetBWRate(client, TL5702_BANDWIDTH_31_25HZ);
	//res = tl5702_SetBWRate(client, TL5702_BANDWIDTH_125HZ);		//wskim140704
	if(res != TL5702_SUCCESS ) 
	{
		GSE_ERR("tl5702_SetBWRate failed! \n");
		return res;
	}

	res = tl5702_SetDataFormat(client, TL5702_RANGE_2G);
	if(res != TL5702_SUCCESS) 
	{
		GSE_ERR("tl5702_SetDataFormat failed! \n");
		return res;
	}

	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = obj->reso->sensitivity;

	res = tl5702_SetIntEnable(client, 0x00);		
		if(res != TL5702_SUCCESS)
		{
			GSE_ERR("tl5702_SetIntEnable failed! \n");
			return res;
		}
#if 1
		res = tl5702_SetPowerMode(client, enable_status);//false
		if(res != TL5702_SUCCESS)
		{
			GSE_ERR("tl5702_SetPowerMode failed! \n");
			return res;
		}
#endif

	if(0 != reset_cali)
	{ 
		/*reset calibration only in power on*/
		res = tl5702_ResetCalibration(client);
		if(res != TL5702_SUCCESS)
		{
			GSE_ERR("tl5702_ResetCalibration failed! \n");
			return res;
		}
	}
	GSE_LOG("tl5702_init_client OK!\n");
#ifdef CONFIG_TL5702_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif

	return TL5702_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int tl5702_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];    

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "TL5702 Chip");
	return 0;
}
/*----------------------------------------------------------------------------*/
static int tl5702_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
	struct tl5702_i2c_data *obj = (struct tl5702_i2c_data*)i2c_get_clientdata(client);
	u8 databuf[20];
	int acc[TL5702_AXES_NUM];
	int res = 0;
	//memset(databuf, 0, sizeof(u8)*10);
	memset(databuf, 0, sizeof(databuf));
//	printk("#################################gsensor\n");
//	GSE_LOG("#################################gsensor\n");			
	
	if(NULL == buf)
	{
		return -1;
	}
	
	memset(buf, 0, bufsize);
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	if (atomic_read(&obj->suspend))
	{
		return 0;
	}
	
	if(sensor_power == FALSE)
	{
		res = tl5702_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on tl5702 error %d!\n", res);
		}
	}

	if(res = tl5702_ReadData(client,  obj->data)) 
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
	#if 1
		obj->data[TL5702_AXIS_X] = obj->data[TL5702_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		obj->data[TL5702_AXIS_Y] = obj->data[TL5702_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		obj->data[TL5702_AXIS_Z] = obj->data[TL5702_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;		
	#endif

//		GSE_LOG("raw data x=%d, y=%d, z=%d \n",obj->data[TL5702_AXIS_X],obj->data[TL5702_AXIS_Y],obj->data[TL5702_AXIS_Z]);		//by wskim 20140318	
		obj->data[TL5702_AXIS_X] += obj->cali_sw[TL5702_AXIS_X];
		obj->data[TL5702_AXIS_Y] += obj->cali_sw[TL5702_AXIS_Y];
		obj->data[TL5702_AXIS_Z] += obj->cali_sw[TL5702_AXIS_Z];
		
//		GSE_LOG("cali_sw x=%d, y=%d, z=%d \n",obj->cali_sw[TL5702_AXIS_X],obj->cali_sw[TL5702_AXIS_Y],obj->cali_sw[TL5702_AXIS_Z]);	//by wskim 20140318
		
		/*remap coordinate*/
#ifdef DATA_MAPPING
		acc[TL5702_AXIS_X] = obj->cvt.sign[TL5702_AXIS_X]*obj->data[obj->cvt.map[TL5702_AXIS_X]];
		acc[TL5702_AXIS_Y] = obj->cvt.sign[TL5702_AXIS_Y]*obj->data[obj->cvt.map[TL5702_AXIS_Y]];
		acc[TL5702_AXIS_Z] = obj->cvt.sign[TL5702_AXIS_Z]*obj->data[obj->cvt.map[TL5702_AXIS_Z]];

#else
		acc[obj->cvt.map[TL5702_AXIS_X]] = obj->cvt.sign[TL5702_AXIS_X]*obj->data[TL5702_AXIS_X];
		acc[obj->cvt.map[TL5702_AXIS_Y]] = obj->cvt.sign[TL5702_AXIS_Y]*obj->data[TL5702_AXIS_Y];
		acc[obj->cvt.map[TL5702_AXIS_Z]] = obj->cvt.sign[TL5702_AXIS_Z]*obj->data[TL5702_AXIS_Z];
#endif
//		GSE_LOG("cvt x=%d, y=%d, z=%d \n",obj->cvt.sign[TL5702_AXIS_X],obj->cvt.sign[TL5702_AXIS_Y],obj->cvt.sign[TL5702_AXIS_Z]);	//by wskim 20140318	


//		GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[TL5702_AXIS_X], acc[TL5702_AXIS_Y], acc[TL5702_AXIS_Z]);   	//by wskim 20140318	

		//Out put the mg
//		GSE_LOG("mg acc=%d, GRAVITY=%d, sensityvity=%d \n",acc[TL5702_AXIS_X],GRAVITY_EARTH_1000,obj->reso->sensitivity);//by wskim 20140318			
	#if 0
		acc[TL5702_AXIS_X] = acc[TL5702_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[TL5702_AXIS_Y] = acc[TL5702_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[TL5702_AXIS_Z] = acc[TL5702_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;		
	#endif	
	

		sprintf(buf, "%04x %04x %04x", acc[TL5702_AXIS_X], acc[TL5702_AXIS_Y], acc[TL5702_AXIS_Z]);

		if(atomic_read(&obj->trace) & ADX_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
		}
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
static int tl5702_ReadRawData(struct i2c_client *client, char *buf)
{
	struct tl5702_i2c_data *obj = (struct tl5702_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return -EINVAL;
	}
	
	if(res = tl5702_ReadData(client, obj->data))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return -EIO;
	}
	else
	{
		sprintf(buf, "TL5702_ReadRawData %04x %04x %04x", obj->data[TL5702_AXIS_X], 
			obj->data[TL5702_AXIS_Y], obj->data[TL5702_AXIS_Z]);
	
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int tl5702_InitSelfTest(struct i2c_client *client)	
{
#if 0	//by wskim 20140317	
	int res = 0;
	u8  data,result;
	
	res = hwmsen_read_byte(client, TL5702_REG_CTL_REG3, &data);
	if(res != TL5702_SUCCESS)
	{
		return res;
	}
//enable selftest bit
	res = hwmsen_write_byte(client, TL5702_REG_CTL_REG3,  TL5702_SELF_TEST|data);
	if(res != TL5702_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
//step 1
	res = hwmsen_read_byte(client, TL5702_DCST_RESP, &result);
	if(res != TL5702_SUCCESS)
	{
		return res;
	}
	GSE_LOG("step1: result = %x",result);
	if(result != 0xaa)
		return -EINVAL;

//step 2
	res = hwmsen_write_byte(client, TL5702_REG_CTL_REG3,  TL5702_SELF_TEST|data);
	if(res != TL5702_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
//step 3
	res = hwmsen_read_byte(client, TL5702_DCST_RESP, &result);
	if(res != TL5702_SUCCESS)
	{
		return res;
	}
	GSE_LOG("step3: result = %x",result);
	if(result != 0xAA)
		return -EINVAL;
		
//step 4
	res = hwmsen_read_byte(client, TL5702_DCST_RESP, &result);
	if(res != TL5702_SUCCESS)
	{
		return res;
	}
	GSE_LOG("step4: result = %x",result);
	if(result != 0x55)
		return -EINVAL;
	else
#endif		
		return TL5702_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int tl5702_JudgeTestResult(struct i2c_client *client, s32 prv[TL5702_AXES_NUM], s32 nxt[TL5702_AXES_NUM])
{

    int res=0;
	u8 test_result=0;
    if(res = hwmsen_read_byte(client, 0x0c, &test_result))
        return res;

	GSE_LOG("test_result = %x \n",test_result);
    if ( test_result != 0xaa ) 
	{
        GSE_ERR("TL5702_JudgeTestResult failt\n");
        res = -EINVAL;
    }
    return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = tl5702_i2c_client;
	char strbuf[TL5702_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	
	tl5702_ReadChipInfo(client, strbuf, TL5702_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}

static ssize_t gsensor_init(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = tl5702_i2c_client;
		char strbuf[TL5702_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("i2c client is null!!\n");
			return 0;
		}
		tl5702_init_client(client, 1);
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}
/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = tl5702_i2c_client;
	char strbuf[TL5702_BUFSIZE];
	hwm_sensor_data gsensor_data;
	
	memset(&gsensor_data, 0,sizeof(gsensor_data));
	memset(strbuf, 0,sizeof(strbuf));
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	if(tl5702_ReadSensorData(client, strbuf, TL5702_BUFSIZE) < 0)
	{
		return 0;
	}
	sscanf(strbuf, "%x %x %x", &gsensor_data.values[0], 
					&gsensor_data.values[1], &gsensor_data.values[2]);	
	//tl5702_ReadRawData(client, strbuf);
	return snprintf(buf, PAGE_SIZE, "x = %d,y = %d,z = %d\n", gsensor_data.values[0], 
					gsensor_data.values[1], gsensor_data.values[2]);          
}

static ssize_t show_sensorrawdata_value(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = tl5702_i2c_client;
		char strbuf[TL5702_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("i2c client is null!!\n");
			return 0;
		}
		//tl5702_ReadSensorData(client, strbuf, TL5702_BUFSIZE);		
		if(tl5702_ReadRawData(client, strbuf) < 0)
		{
			return 0;
		}
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}

/*----------------------------------------------------------------------------*/
static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = tl5702_i2c_client;
	struct tl5702_i2c_data *obj;
	int err, len = 0, mul;
	int tmp[TL5702_AXES_NUM];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);



	if(err = tl5702_ReadOffset(client, obj->offset))
	{
		return -EINVAL;
	}
	else if(err = tl5702_ReadCalibration(client, tmp))
	{
		return -EINVAL;
	}
	else
	{    
		mul = obj->reso->sensitivity/tl5702_offset_resolution.sensitivity;
		len += snprintf(buf+len, PAGE_SIZE-len, "[HW ][%d] (%+3d, %+3d, %+3d) : (0x%02X, 0x%02X, 0x%02X)\n", mul,                        
			obj->offset[TL5702_AXIS_X], obj->offset[TL5702_AXIS_Y], obj->offset[TL5702_AXIS_Z],
			obj->offset[TL5702_AXIS_X], obj->offset[TL5702_AXIS_Y], obj->offset[TL5702_AXIS_Z]);
		len += snprintf(buf+len, PAGE_SIZE-len, "[SW ][%d] (%+3d, %+3d, %+3d)\n", 1, 
			obj->cali_sw[TL5702_AXIS_X], obj->cali_sw[TL5702_AXIS_Y], obj->cali_sw[TL5702_AXIS_Z]);

		len += snprintf(buf+len, PAGE_SIZE-len, "[ALL]    (%+3d, %+3d, %+3d) : (%+3d, %+3d, %+3d)\n", 
			obj->offset[TL5702_AXIS_X]*mul + obj->cali_sw[TL5702_AXIS_X]*(obj->reso->sensitivity)/GRAVITY_EARTH_1000,
			obj->offset[TL5702_AXIS_Y]*mul + obj->cali_sw[TL5702_AXIS_Y]*(obj->reso->sensitivity)/GRAVITY_EARTH_1000,
			obj->offset[TL5702_AXIS_Z]*mul + obj->cali_sw[TL5702_AXIS_Z]*(obj->reso->sensitivity)/GRAVITY_EARTH_1000,
			tmp[TL5702_AXIS_X], tmp[TL5702_AXIS_Y], tmp[TL5702_AXIS_Z]);
		
		return len;
    }
}
/*----------------------------------------------------------------------------*/
static ssize_t store_cali_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct i2c_client *client = tl5702_i2c_client;  
	struct tl5702_i2c_data *obj = obj_i2c_data;
	int err, x, y, z;
	int dat[TL5702_AXES_NUM];
	if((client==NULL) || (obj==NULL))
	{
		GSE_ERR("i2c client or tl5702_i2c_data is null!!\n");
		return -1;
	}
	if(!strncmp(buf, "rst", 3))
	{
		if(err = tl5702_ResetCalibration(client))
		{
			GSE_ERR("reset offset err = %d\n", err);
		}	
	}
	else if(3 == sscanf(buf, "0x%02X 0x%02X 0x%02X", &x, &y, &z))
	{
	#if 1
		dat[TL5702_AXIS_X] = x*GRAVITY_EARTH_1000/(obj->reso->sensitivity);
		dat[TL5702_AXIS_Y] = y*GRAVITY_EARTH_1000/(obj->reso->sensitivity);
		dat[TL5702_AXIS_Z] = z*GRAVITY_EARTH_1000/(obj->reso->sensitivity);
	#else
		dat[TL5702_AXIS_X] = x*1000;
		dat[TL5702_AXIS_Y] = y*1000;
		dat[TL5702_AXIS_Z] = z*1000;

	#endif
		if(err = tl5702_WriteCalibration(client, dat))
		{
			GSE_ERR("write calibration err = %d\n", err);
		}		
	}
	else
	{
		GSE_ERR("invalid format\n");
	}
	
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_self_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = tl5702_i2c_client;
	struct tl5702_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	//obj = i2c_get_clientdata(client);
	
    return snprintf(buf, 8, "%s\n", selftestRes);
}
/*----------------------------------------------------------------------------*/
static ssize_t store_self_value(struct device_driver *ddri, char *buf, size_t count)
{   /*write anything to this register will trigger the process*/

	struct item{
	s16 raw[TL5702_AXES_NUM];
	};
	
	struct i2c_client *client = tl5702_i2c_client;  
	int idx, res, num;
	struct item *prv = NULL, *nxt = NULL;
	s32 avg_prv[TL5702_AXES_NUM] = {0, 0, 0};
	s32 avg_nxt[TL5702_AXES_NUM] = {0, 0, 0};
	u8 data;


	if(1 != sscanf(buf, "%d", &num))
	{
		GSE_ERR("parse number fail\n");
		return count;
	}
	else if(num == 0)
	{
		GSE_ERR("invalid data count\n");
		return count;
	}

	prv = kzalloc(sizeof(*prv) * num, GFP_KERNEL);
	nxt = kzalloc(sizeof(*nxt) * num, GFP_KERNEL);
	if (!prv || !nxt)
	{
		goto exit;
	}


	GSE_LOG("NORMAL:\n");
	tl5702_SetPowerMode(client,true); 

	/*initial setting for self test*/
	if(!tl5702_InitSelfTest(client))
	{
		GSE_LOG("SELFTEST : PASS\n");
		strcpy(selftestRes,"y");
	}	
	else
	{
		GSE_LOG("SELFTEST : FAIL\n");		
		strcpy(selftestRes,"n");
	}

//	res = hwmsen_read_byte(client, TL5702_REG_CTL_REG3, &data);		//by wskim 20140313
	if(res != TL5702_SUCCESS)
	{
		return res;
	}

//	res = hwmsen_write_byte(client, TL5702_REG_CTL_REG3,  ~TL5702_SELF_TEST&data);	//by wskim 20140313 
	if(res != TL5702_SUCCESS) 
	{
		return res;
	}
	
	exit:
	/*restore the setting*/    
	tl5702_init_client(client, 0);
	kfree(prv);
	kfree(nxt);
	return count;

}
/*----------------------------------------------------------------------------*/
static ssize_t show_selftest_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = tl5702_i2c_client;
	struct tl5702_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->selftest));
}
/*----------------------------------------------------------------------------*/
static ssize_t store_selftest_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct tl5702_i2c_data *obj = obj_i2c_data;
	int tmp;

	if(NULL == obj)
	{
		GSE_ERR("i2c data obj is null!!\n");
		return -1;
	}
	
	
	if(1 == sscanf(buf, "%d", &tmp))
	{        
		if(atomic_read(&obj->selftest) && !tmp)
		{
			/*enable -> disable*/
			tl5702_init_client(obj->client, 0);
		}
		else if(!atomic_read(&obj->selftest) && tmp)
		{
			/*disable -> enable*/
			tl5702_InitSelfTest(obj->client);            
		}
		
		GSE_LOG("selftest: %d => %d\n", atomic_read(&obj->selftest), tmp);
		atomic_set(&obj->selftest, tmp); 
	}
	else
	{ 
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);   
	}
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_firlen_value(struct device_driver *ddri, char *buf)
{
#ifdef CONFIG_TL5702_LOWPASS
	struct i2c_client *client = tl5702_i2c_client;
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	if(atomic_read(&obj->firlen))
	{
		int idx, len = atomic_read(&obj->firlen);
		GSE_LOG("len = %2d, idx = %2d\n", obj->fir.num, obj->fir.idx);

		for(idx = 0; idx < len; idx++)
		{
			GSE_LOG("[%5d %5d %5d]\n", obj->fir.raw[idx][TL5702_AXIS_X], obj->fir.raw[idx][TL5702_AXIS_Y], obj->fir.raw[idx][TL5702_AXIS_Z]);
		}
		
		GSE_LOG("sum = [%5d %5d %5d]\n", obj->fir.sum[TL5702_AXIS_X], obj->fir.sum[TL5702_AXIS_Y], obj->fir.sum[TL5702_AXIS_Z]);
		GSE_LOG("avg = [%5d %5d %5d]\n", obj->fir.sum[TL5702_AXIS_X]/len, obj->fir.sum[TL5702_AXIS_Y]/len, obj->fir.sum[TL5702_AXIS_Z]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->firlen));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}
/*----------------------------------------------------------------------------*/
static ssize_t store_firlen_value(struct device_driver *ddri, char *buf, size_t count)
{
#ifdef CONFIG_TL5702_LOWPASS
	//struct i2c_client *client = tl5702_i2c_client;  
	//struct tl5702_i2c_data *obj = i2c_get_clientdata(client);
	//struct i2c_client *client = tl5702_i2c_client;  
	struct tl5702_i2c_data *obj = obj_i2c_data;
	int firlen;

	if(obj == NULL)
	{
		GSE_ERR("i2c client or tl5702_i2c_data is null!!\n");
		return -1;
	}

	if(1 != sscanf(buf, "%d", &firlen))
	{
		GSE_ERR("invallid format\n");
	}
	else if(firlen > C_MAX_FIR_LENGTH)
	{
		GSE_ERR("exceeds maximum filter length\n");
	}
	else
	{ 
		atomic_set(&obj->firlen, firlen);
		if(NULL == firlen)
		{
			atomic_set(&obj->fir_en, 0);
		}
		else
		{
			memset(&obj->fir, 0x00, sizeof(obj->fir));
			atomic_set(&obj->fir_en, 1);
		}
	}
#endif    
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct tl5702_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct tl5702_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return -1;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}	
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;    
	struct tl5702_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	
	
	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n", 
	            obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol);   
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	return len;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_power_status_value(struct device_driver *ddri, char *buf)
{
	u8 databuf[2];    
	u8 addr = TL5702_LPWR_MODE_REG;
	if(hwmsen_read_block(tl5702_i2c_client, addr, databuf, 0x01))
	{
		GSE_ERR("read power ctl register err!\n");
		return TL5702_ERR_I2C;
	}
    
	if(sensor_power)
		GSE_LOG("G sensor is in work mode, sensor_power = %d\n", sensor_power);
	else
		GSE_LOG("G sensor is in standby mode, sensor_power = %d\n", sensor_power);

	return snprintf(buf, PAGE_SIZE, "%x\n", databuf[0]);
}

/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,   S_IWUSR | S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata, S_IWUSR | S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(cali,       S_IWUSR | S_IRUGO, show_cali_value,          store_cali_value);
static DRIVER_ATTR(selftest, S_IWUSR | S_IRUGO, show_self_value,  store_self_value);
static DRIVER_ATTR(self,   S_IWUSR | S_IRUGO, show_selftest_value,      store_selftest_value);
static DRIVER_ATTR(firlen,     S_IWUSR | S_IRUGO, show_firlen_value,        store_firlen_value);
static DRIVER_ATTR(trace,      S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,               S_IRUGO, show_status_value,        NULL);
static DRIVER_ATTR(powerstatus,               S_IRUGO, show_power_status_value,        NULL);

/*----------------------------------------------------------------------------*/
static u8 i2c_dev_reg =0 ;

static ssize_t show_register(struct device_driver *pdri, char *buf)
{
	int input_value;
		
	GSE_LOG("i2c_dev_reg is 0x%2x \n", i2c_dev_reg);

	return 0;
}

static ssize_t store_register(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long input_value;

	i2c_dev_reg = simple_strtoul(buf, NULL, 16);
	GSE_LOG("set i2c_dev_reg = 0x%2x \n", i2c_dev_reg);

	return count;
}
static ssize_t store_register_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct tl5702_i2c_data *obj = obj_i2c_data;
	u8 databuf[2];  
	unsigned long input_value;
	int res;
	
	memset(databuf, 0, sizeof(u8)*2);    

	input_value = simple_strtoul(buf, NULL, 16);
	GSE_LOG("input_value = 0x%2x \n", input_value);

	if(NULL == obj)
	{
		GSE_ERR("i2c data obj is null!!\n");
		return -1;
	}

	databuf[0] = i2c_dev_reg;
	databuf[1] = input_value;
	GSE_LOG("databuf[0]=0x%2x  databuf[1]=0x%2x \n", databuf[0],databuf[1]);

	res = i2c_master_send(obj->client, databuf, 0x2);

	if(res <= 0)
	{
		return TL5702_ERR_I2C;
	}
	return count;
	
}

static ssize_t show_register_value(struct device_driver *ddri, char *buf)
{
		struct tl5702_i2c_data *obj = obj_i2c_data;
		u8 databuf[1];	
		
		memset(databuf, 0, sizeof(u8)*1);	 
	
		if(NULL == obj)
		{
			GSE_ERR("i2c data obj is null!!\n");
			return 0;
		}
		
		if(hwmsen_read_block(obj->client, i2c_dev_reg, databuf, 0x01))
		{
			GSE_ERR("read power ctl register err!\n");
			return TL5702_ERR_I2C;
		}

		GSE_LOG("i2c_dev_reg=0x%2x  data=0x%2x \n", i2c_dev_reg,databuf[0]);
	
		return 0;
		
}
static ssize_t store_layout_value(struct device_driver *ddri, char *buf, size_t count)
{
		struct tl5702_i2c_data *obj = obj_i2c_data;
		//struct tl5702_i2c_data *obj = NULL; //for test
		int layout = 0; 
		int err = 0;
		
		if(NULL == obj)
		{
			GSE_ERR("i2c data obj is null!!\n");
			return -1;
		}
		if(NULL == buf)
		{
			GSE_ERR("buf is null!!\n");
			return -1;
		}
		layout = obj->layout;
		if(1 == sscanf(buf, "%d", &layout))
		{
			if(layout != obj->layout)
			{
				if((err = hwmsen_get_convert(layout, &(obj->cvt)))<0)
				{
					GSE_ERR("invalid direction: %d\n", layout);
					return count;
				}
				obj->layout = layout;
			}
		}
		else
		{
			GSE_ERR("invalid direction\n");
			return count;
		}
		return count;
		
}

static ssize_t show_layout_value(struct device_driver *ddri, char *buf)
{
	struct tl5702_i2c_data *obj = obj_i2c_data; 
	
	if(NULL == obj)
	{
		GSE_ERR("i2c data obj is null!!\n");
		return 0;
	}
		
	return snprintf(buf, PAGE_SIZE, "%d\n",obj->layout);	
}

static DRIVER_ATTR(i2c,      S_IWUSR | S_IRUGO, show_register_value,         store_register_value);
static DRIVER_ATTR(register,      S_IWUSR | S_IRUGO, show_register,         store_register);

static DRIVER_ATTR(layout,      S_IWUSR | S_IRUGO, show_layout_value,         store_layout_value);

/*----------------------------------------------------------------------------*/
static struct driver_attribute *tl5702_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
	&driver_attr_self,         /*self test demo*/
	&driver_attr_selftest,     /*self control: 0: disable, 1: enable*/
	&driver_attr_firlen,       /*filter length: 0: disable, others: enable*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,
	&driver_attr_powerstatus,
//	&driver_attr_register,
//	&driver_attr_i2c,
	&driver_attr_layout,
};
/*----------------------------------------------------------------------------*/
static int tl5702_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(tl5702_attr_list)/sizeof(tl5702_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(err = driver_create_file(driver, tl5702_attr_list[idx]))
		{            
			GSE_ERR("driver_create_file (%s) = %d\n", tl5702_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int tl5702_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(tl5702_attr_list)/sizeof(tl5702_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, tl5702_attr_list[idx]);
	}
	

	return err;
}
/*----------------------------------------------------------------------------*/
#ifndef GSENSOR_NEW_ARCH
int gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value = 0;
	int sample_delay = 0;	
	struct tl5702_i2c_data *priv = (struct tl5702_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[TL5702_BUFSIZE];
	if(atomic_read(&priv->trace) & ADX_TRC_DEBUG)
	{
		GSE_FUN(f);
	}
	memset(buff,0,sizeof(buff));

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(value <= 5)
				{
					sample_delay = TL5702_BANDWIDTH_62_5HZ;
				}
				else if(value <= 10)
				{
					sample_delay = TL5702_BANDWIDTH_31_25HZ;
				}
				else
				{
					sample_delay = TL5702_BANDWIDTH_15_63HZ;

				}
						
				err = tl5702_SetBWRate(priv->client, sample_delay);
				mdelay(10);		//wskim140314
				if(err != TL5702_SUCCESS ) 
				{
					GSE_ERR("Set delay parameter error!\n");
				}
			
				if(value >= 50)
				{
					atomic_set(&priv->filter, 0);
				}
				else
				{	
				#if defined(CONFIG_TL5702_LOWPASS)
					priv->fir.num = 0;
					priv->fir.idx = 0;
					priv->fir.sum[TL5702_AXIS_X] = 0;
					priv->fir.sum[TL5702_AXIS_Y] = 0;
					priv->fir.sum[TL5702_AXIS_Z] = 0;
					atomic_set(&priv->filter, 1);
				#endif
				}
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				mutex_lock(&tl5702_mutex);
				if(((value == 0) && (sensor_power == false)) ||((value == 1) && (sensor_power == true)))
				{
					enable_status = sensor_power;
					GSE_LOG("Gsensor enable_status have updated!\n");
				}
				else
				{
					enable_status = !sensor_power;
					if (atomic_read(&priv->suspend) == 0)
					{
						err = tl5702_SetPowerMode( priv->client, enable_status);
						GSE_LOG("Gsensor not in suspend TL5702_SetPowerMode!, enable_status = %d\n",enable_status);
					}
					else
					{
						GSE_LOG("Gsensor in suspend and can not enable or disable!enable_status = %d\n",enable_status);
					}
				}
				mutex_unlock(&tl5702_mutex);
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				gsensor_data = (hwm_sensor_data *)buff_out;
				mutex_lock(&tl5702_mutex);
				err = tl5702_ReadSensorData(priv->client, buff, TL5702_BUFSIZE);
				mutex_unlock(&tl5702_mutex);
				if(err < 0)
				{
					GSE_ERR("tl5702_ReadSensorData error!\n");
					break;
				}
				sscanf(buff, "%x %x %x", &gsensor_data->values[0], 
					&gsensor_data->values[1], &gsensor_data->values[2]);	
				gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
				gsensor_data->value_divide = 1000;
				if(atomic_read(&priv->trace) & ADX_TRC_DATA)
				{
					GSE_LOG("gsensor_operate X :%d,Y: %d, Z: %d\n",gsensor_data->values[0],gsensor_data->values[1],gsensor_data->values[2]);
				}
			}
			break;
		default:
			GSE_ERR("gsensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}
#endif
/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int tl5702_open(struct inode *inode, struct file *file)
{
	file->private_data = tl5702_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int tl5702_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int tl5702_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long tl5702_unlocked_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct tl5702_i2c_data *obj = (struct tl5702_i2c_data*)i2c_get_clientdata(client);	
	char strbuf[TL5702_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
	long err = 0;
	int cali[3];
	memset(strbuf,0,sizeof(strbuf));//add
	memset(&sensor_data,0,sizeof(sensor_data));//add
	//GSE_FUN(f);
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case GSENSOR_IOCTL_INIT:
			tl5702_init_client(client, 0);			
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			memset(strbuf,0,sizeof(strbuf));//add
			tl5702_ReadChipInfo(client, strbuf, TL5702_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}				 
			break;	  

		case GSENSOR_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			memset(strbuf,0,sizeof(strbuf));//add
			tl5702_SetPowerMode(obj->client, true);
			tl5702_ReadSensorData(client, strbuf, TL5702_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}				 
			break;

		case GSENSOR_IOCTL_READ_GAIN:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &gsensor_gain, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

		case GSENSOR_IOCTL_READ_RAW_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			memset(strbuf,0,sizeof(strbuf));//add
			tl5702_ReadRawData(client, strbuf);
			if(copy_to_user(data, &strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}
			break;	  

		case GSENSOR_IOCTL_SET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;	  
			}
			if(atomic_read(&obj->suspend))
			{
				GSE_ERR("Perform calibration in suspend state!!\n");
				err = -EINVAL;
			}
			else
			{
			#if 0
				cali[TL5702_AXIS_X] = sensor_data.x * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[TL5702_AXIS_Y] = sensor_data.y * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[TL5702_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;			  
			#else
				cali[TL5702_AXIS_X] = sensor_data.x;
				cali[TL5702_AXIS_Y] = sensor_data.y;
				cali[TL5702_AXIS_Z] = sensor_data.z;			  			
			#endif
				err = tl5702_WriteCalibration(client, cali);			 
			}
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = tl5702_ResetCalibration(client);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(err = tl5702_ReadCalibration(client, cali))
			{
				break;
			}
			#if 0
			sensor_data.x = cali[TL5702_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.y = cali[TL5702_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.z = cali[TL5702_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			#else
			sensor_data.x = cali[TL5702_AXIS_X];
			sensor_data.y = cali[TL5702_AXIS_Y];
			sensor_data.z = cali[TL5702_AXIS_Z];
			#endif
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}		
			break;
		

		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}
 

/*----------------------------------------------------------------------------*/
static struct file_operations tl5702_fops = {
	.owner = THIS_MODULE,
	.open = tl5702_open,
	.release = tl5702_release,
	.unlocked_ioctl = tl5702_unlocked_ioctl,
	//.ioctl = tl5702_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice tl5702_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &tl5702_fops,
};
#ifdef GSENSOR_NEW_ARCH
static int tl5702_open_report_data(int open)
{
	//should queuq work to report event if  is_report_input_direct=true
	return 0;
}
static int tl5702_enable_nodata(int en)
{
	int err = 0;
	struct tl5702_i2c_data *priv = obj_i2c_data;
	
	if(NULL == priv)
	{
		GSE_ERR("tl5702_i2c_data is null!\n");
		return -1;
	}
	mutex_lock(&tl5702_mutex);
	if(((en == 0) && (sensor_power == false)) ||((en == 1) && (sensor_power == true)))
	{
		enable_status = sensor_power;
		GSE_LOG("Gsensor device have updated!\n");
	}
	else
	{
		enable_status = !sensor_power;
		if (atomic_read(&priv->suspend) == 0)
		{
			err = tl5702_SetPowerMode( priv->client, enable_status);
			GSE_LOG("Gsensor not in suspend TL5702_SetPowerMode!, enable_status = %d\n",enable_status);
		}
		else
		{
			GSE_LOG("Gsensor in suspend and can not enable or disable!enable_status = %d\n",enable_status);
		}
	}
	mutex_unlock(&tl5702_mutex);
	GSE_LOG("tl5702_enable_nodata ok!\n");

	return 0;

}
static int tl5702_set_delay(u64 ns)
{
	int sample_delay = 0;
	int err = 0;
	int delay = (int)ns/1000/1000;
	struct tl5702_i2c_data *priv = obj_i2c_data;
	
	if(NULL == priv)
	{
		GSE_ERR("tl5702_i2c_data is null!\n");
		return -1;
	}

	if(delay <= 5)
	{
		sample_delay = TL5702_BANDWIDTH_62_5HZ;
	}
	else if(delay <= 10)
	{
		sample_delay = TL5702_BANDWIDTH_31_25HZ;
	}
	else
	{
		sample_delay = TL5702_BANDWIDTH_15_63HZ;
	
	}
							
	err = tl5702_SetBWRate(priv->client, sample_delay);
	mdelay(10); 	//wskim140314
	if(err != TL5702_SUCCESS ) 
	{
		GSE_ERR("Set delay parameter error!\n");
	}
				
	if(delay >= 50)
	{
		atomic_set(&priv->filter, 0);
	}
	else
	{	
#if defined(CONFIG_TL5702_LOWPASS)
		priv->fir.num = 0;
		priv->fir.idx = 0;
		priv->fir.sum[TL5702_AXIS_X] = 0;
		priv->fir.sum[TL5702_AXIS_Y] = 0;
		priv->fir.sum[TL5702_AXIS_Z] = 0;
		atomic_set(&priv->filter, 1);
#endif
	}
	GSE_LOG("Set delay parameter OK!\n");
	return 0;
}
static int tl5702_get_data(int *x,int *y, int *z,int *status)
{
	char buff[TL5702_BUFSIZE];
	int err = 0;
	struct tl5702_i2c_data *priv = obj_i2c_data;
	
	if(NULL == priv)
	{
		GSE_ERR("tl5702_i2c_data is null!\n");
		return -1;
	}
	memset(buff,0,sizeof(buff));
		
	mutex_lock(&tl5702_mutex);
	err = tl5702_ReadSensorData(priv->client, buff, TL5702_BUFSIZE);
	mutex_unlock(&tl5702_mutex);
	if(err < 0)
	{
		return err;
	}
	sscanf(buff, "%x %x %x", x, y, z);	
	status = SENSOR_STATUS_ACCURACY_MEDIUM;				
	if(atomic_read(&priv->trace) & ADX_TRC_DATA)
	{
		GSE_LOG("tl5702_get_data X :%d,Y: %d, Z: %d\n",*x,*y,*z);
	}
	return 0;
}

#endif


/*----------------------------------------------------------------------------*/
//#ifndef CONFIG_HAS_EARLYSUSPEND
#if !defined(CONFIG_HAS_EARLYSUSPEND) || !defined(USE_EARLY_SUSPEND)  

/*----------------------------------------------------------------------------*/
static int tl5702_suspend(struct i2c_client *client, pm_message_t msg) 
{
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);    
	int err = 0;
	GSE_FUN();    

	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(obj == NULL)
		{
			GSE_ERR("null pointer!!\n");
			return -EINVAL;
		}
		atomic_set(&obj->suspend, 1);
		if(err = tl5702_SetPowerMode(obj->client, false))
		{
			GSE_ERR("write power control fail!!\n");
			return;
		}
		tl5702_power(obj->hw, 0);
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int tl5702_resume(struct i2c_client *client)
{
	struct tl5702_i2c_data *obj = i2c_get_clientdata(client);        
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	tl5702_power(obj->hw, 1);
	if(err = tl5702_init_client(client, 0))
	{
		GSE_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->suspend, 0);

	return 0;
}
/*----------------------------------------------------------------------------*/
#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void tl5702_early_suspend(struct early_suspend *h) 
{
	struct tl5702_i2c_data *obj = container_of(h, struct tl5702_i2c_data, early_drv);   
	int err;
	GSE_FUN();    

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	mutex_lock(&tl5702_mutex);
	atomic_set(&obj->suspend, 1); 
	if(err = tl5702_SetPowerMode(obj->client, false))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}
	mutex_unlock(&tl5702_mutex);

	//sensor_power = false;
	
	tl5702_power(obj->hw, 0);
}
/*----------------------------------------------------------------------------*/
static void tl5702_late_resume(struct early_suspend *h)
{
	struct tl5702_i2c_data *obj = container_of(h, struct tl5702_i2c_data, early_drv);         
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}

	tl5702_power(obj->hw, 1);
	mutex_lock(&tl5702_mutex);
	if(err = tl5702_init_client(obj->client, 0))
	{
		GSE_ERR("initialize client fail!!\n");
		return;        
	}
	atomic_set(&obj->suspend, 0); 
	mutex_unlock(&tl5702_mutex);
}
/*----------------------------------------------------------------------------*/
#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
static int tl5702_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
	strcpy(info->type, TL5702_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int tl5702_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct tl5702_i2c_data *obj;
#ifdef GSENSOR_NEW_ARCH
	struct acc_control_path ctl = {0};
	struct acc_data_path data = {0};
#else
	struct hwmsen_object sobj;
#endif
	int err = 0;
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	
	memset(obj, 0, sizeof(struct tl5702_i2c_data));

	obj->hw = get_cust_acc_hw();
	obj->layout = obj->hw->direction;
	if(err = hwmsen_get_convert(obj->hw->direction, &obj->cvt))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
	
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);
	atomic_set(&obj->position, obj->hw->direction);
	
#ifdef CONFIG_TL5702_LOWPASS
	if(obj->hw->firlen > C_MAX_FIR_LENGTH)
	{
		atomic_set(&obj->firlen, C_MAX_FIR_LENGTH);
	}	
	else
	{
		atomic_set(&obj->firlen, obj->hw->firlen);
	}
	
	if(atomic_read(&obj->firlen) > 0)
	{
		atomic_set(&obj->fir_en, 1);
	}
	
#endif

	tl5702_i2c_client = new_client;	

#ifdef OFFSET_CALIBRATION
	tl5702_SetPowerMode(client, true);
	tl5702_OffsetCalibration(new_client);
	tl5702_SetPowerMode(client, false);
#endif
	if(err = tl5702_init_client(new_client, 1))
	{
		goto exit_init_failed;
	}
	

	if(err = misc_register(&tl5702_device))
	{
		GSE_ERR("tl5702_device register failed\n");
		goto exit_misc_device_register_failed;
	}
#ifdef GSENSOR_NEW_ARCH
	if(err = tl5702_create_attr(&(tl5702_init_info.platform_diver_addr->driver)))
#else
	if(err = tl5702_create_attr(&tl5702_gsensor_driver.driver))
#endif
	{
		GSE_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
#ifdef GSENSOR_NEW_ARCH
	ctl.open_report_data = tl5702_open_report_data;
	ctl.enable_nodata = tl5702_enable_nodata;
	ctl.set_delay = tl5702_set_delay;
	ctl.is_report_input_direct = false;

	if(acc_register_control_path(&ctl))
	{
		GSE_ERR("acc_register_control_path err\n");
		goto exit_kfree;
	}

	data.get_data = tl5702_get_data;
	data.vender_div = 1000;
	if(acc_register_data_path(&data))
	{
		GSE_ERR("acc_register_control_path err\n");
		goto exit_kfree;
	}
	
#else
	sobj.self = obj;
    sobj.polling = 1;
    sobj.sensor_operate = gsensor_operate;
	if(err = hwmsen_attach(ID_ACCELEROMETER, &sobj))
	{
		GSE_ERR("attach fail = %d\n", err);
		goto exit_kfree;
	}
#endif

//#ifdef CONFIG_HAS_EARLYSUSPEND
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(USE_EARLY_SUSPEND) 

	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = tl5702_early_suspend,
	obj->early_drv.resume   = tl5702_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif 
#ifdef GSENSOR_NEW_ARCH
	tl5702_init_flag = 0;
#endif
	GSE_LOG("%s: OK\n", __func__);    
	return 0;

	exit_create_attr_failed:
	misc_deregister(&tl5702_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
#ifdef GSENSOR_NEW_ARCH
	tl5702_init_flag = -1;
#endif
	GSE_ERR("%s: err = %d\n", __func__, err);        
	return err;
}

/*----------------------------------------------------------------------------*/
static int tl5702_i2c_remove(struct i2c_client *client)
{
	int err = 0;	
#ifdef GSENSOR_NEW_ARCH
	if(err = tl5702_delete_attr(&(tl5702_init_info.platform_diver_addr->driver)))
#else
	if(err = tl5702_delete_attr(&tl5702_gsensor_driver.driver))
#endif
	{
		GSE_ERR("tl5702_delete_attr fail: %d\n", err);
	}
	
	if(err = misc_deregister(&tl5702_device))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}
#ifdef GSENSOR_NEW_ARCH
	
#else
	if(err = hwmsen_detach(ID_ACCELEROMETER))
#endif	    

	tl5702_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
#ifdef GSENSOR_NEW_ARCH
static int tl5702_local_init(void)
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();
	
	tl5702_power(hw, 1);
	//tl5702_force[0] = hw->i2c_num;
	if(i2c_add_driver(&tl5702_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	if(-1 == tl5702_init_flag)
	{
		GSE_ERR("gsensor tl5702 driver local init failed\n");
		return -1;
	}
	return 0;
}
static int tl5702_local_remove(void)
{
	struct acc_hw *hw = get_cust_acc_hw();
	
	GSE_FUN();	  
	tl5702_power(hw, 0);	
	i2c_del_driver(&tl5702_i2c_driver);
	tl5702_init_flag = -1;
	return 0;
}
#else
static int tl5702_probe(struct platform_device *pdev) 
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();

	tl5702_power(hw, 1);
	//tl5702_force[0] = hw->i2c_num;
	if(i2c_add_driver(&tl5702_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int tl5702_remove(struct platform_device *pdev)
{
    struct acc_hw *hw = get_cust_acc_hw();

    GSE_FUN();    
    tl5702_power(hw, 0);    
    i2c_del_driver(&tl5702_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/
#if 1
#ifdef CONFIG_OF
static const struct of_device_id gsensor_of_match[] = {
	{ .compatible = "mediatek,gsensor", },
	{},
};
#endif

static struct platform_driver tl5702_gsensor_driver = {
	.probe      = tl5702_probe,
	.remove     = tl5702_remove,    
	.driver     = 
	{
		.name  = "gsensor",
		.owner = THIS_MODULE,
        #ifdef CONFIG_OF
		.of_match_table = gsensor_of_match,
		#endif
	}
};
#else
static struct platform_driver tl5702_gsensor_driver = {
	.probe      = tl5702_probe,
	.remove     = tl5702_remove,    
	.driver     = {
	.name  		= "gsensor",
	.owner 		= THIS_MODULE,
	}
};
#endif
#endif
/*----------------------------------------------------------------------------*/
static int __init tl5702_init(void)
{
	GSE_FUN();
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num);
	i2c_register_board_info(hw->i2c_num, &i2c_tl5702, 1);
#ifdef GSENSOR_NEW_ARCH
	if(acc_driver_add(&tl5702_init_info))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
#else
	if(platform_driver_register(&tl5702_gsensor_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
#endif
	return 0;    
}
/*----------------------------------------------------------------------------*/
static void __exit tl5702_exit(void)
{
	GSE_FUN();
#ifndef GSENSOR_NEW_ARCH
	platform_driver_unregister(&tl5702_gsensor_driver);
#endif
}
/*----------------------------------------------------------------------------*/
module_init(tl5702_init);
module_exit(tl5702_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TL5702 I2C driver");
MODULE_AUTHOR("wskim@tli.co.kr");
