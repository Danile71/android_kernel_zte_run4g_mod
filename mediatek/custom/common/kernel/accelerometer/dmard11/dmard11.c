/* DMARD11 motion sensor driver
 *
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
#include <cust_eint.h>
#include <cust_alsps.h>


#include "dmard11.h"

#include <linux/hwmsen_helper.h>

#ifndef USE_MTK_CALIBRATE
#include <linux/fs.h>
#include <linux/string.h>
#endif

#define DMARD11_NEW_ARCH

#ifdef DMARD11_NEW_ARCH
#include <accel.h>
#endif

//#define DMARD11_DATA_MAPPING

/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_DMARD11 11
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
//#define CONFIG_DMARD11_LOWPASS   /*apply low pass filter on output*/       
#define SW_CALIBRATION

/*----------------------------------------------------------------------------*/
#define DMARD11_AXIS_X          0
#define DMARD11_AXIS_Y          1
#define DMARD11_AXIS_Z          2
#define DMARD11_AXES_NUM        3
#define DMARD11_DATA_LEN        8//cause we donot read from REG_X
#define DMARD11_DEV_NAME        "DMARD11"
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static const struct i2c_device_id dmard11_i2c_id[] = {{DMARD11_DEV_NAME,0},{}};
/*the adapter id will be available in customization*/
static struct i2c_board_info __initdata i2c_dmard11={ I2C_BOARD_INFO("DMARD11", DMARD11_I2C_SLAVE_WRITE_ADDR>>1)};

//static unsigned short dmard11_force[] = {0x00, DMARD11_I2C_SLAVE_WRITE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const dmard11_forces[] = { dmard11_force, NULL };
//static struct i2c_client_address_data dmard11_addr_data = { .forces = dmard11_forces,};

/*----------------------------------------------------------------------------*/
static int dmard11_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int dmard11_i2c_remove(struct i2c_client *client);
static int dmard11_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);

#ifdef DMARD11_NEW_ARCH
static int dmard11_local_init(void);
static int dmard11_local_uninit(void);
static int dmard11_init_flag = -1; //-1:fail   0:ok

static int dmard11_open_report_data(int open);//open data rerport to HAL
static int dmard11_enable_nodata(int en);//only enable not report event to HAL
static int dmard11_set_delay(u64 delay);

static int dmard11_get_data(int *x,int *y, int *z,int *status);

static struct acc_init_info dmard11_init_info = 
{
	.name 	= "DMARD11",
	.init 	= dmard11_local_init,
	.uninit = dmard11_local_uninit,
};


#endif

#ifndef USE_MTK_CALIBRATE
void D11_write_offset_to_file(struct i2c_client *client, char*);
void D11_read_offset_from_file(struct i2c_client *client);

char OffsetFileName[] = "/data/misc/gsensor_offset.txt";	/* FILE offset.txt */
char DmtXXFileName[] = "/data/misc/dmt_sensor.txt";
#endif
/*----------------------------------------------------------------------------*/
typedef enum {
    ADX_TRC_FILTER  = 0x01,
    ADX_TRC_RAWDATA = 0x02,
    ADX_TRC_IOCTL   = 0x04,
    ADX_TRC_CALI	= 0X08,
    ADX_TRC_INFO	= 0X10,
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
    s16 raw[C_MAX_FIR_LENGTH][DMARD11_AXES_NUM];
    int sum[DMARD11_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct dmard11_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;
    atomic_t layout;
    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[DMARD11_AXES_NUM+1];

    /*data*/
    s8                      offset[DMARD11_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[DMARD11_AXES_NUM+1];
#ifndef USE_MTK_CALIBRATE
    raw_data				offset;			/* D11 Offset */
#endif
#if defined(CONFIG_DMARD11_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif 
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver dmard11_i2c_driver = {
    .driver = {
      //  .owner          = THIS_MODULE,
        .name           = DMARD11_DEV_NAME,
    },
	.probe      		= dmard11_i2c_probe,
	.remove    			= dmard11_i2c_remove,
	.detect				= dmard11_i2c_detect,
#if !defined(CONFIG_HAS_EARLYSUSPEND)    
    .suspend            = dmard11_suspend,
    .resume             = dmard11_resume,
#endif
	.id_table = dmard11_i2c_id,
	//.address_data = &dmard11_addr_data,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *dmard11_i2c_client = NULL;
static struct platform_driver dmard11_gsensor_driver;
static struct dmard11_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = true;
static bool power_status = false;
static GSENSOR_VECTOR3D gsensor_gain;
static char selftestRes[8]= {0}; 
#define DEBUG
/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#ifdef DEBUG
#define GSE_FUN(f)               printk(GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(GSE_TAG fmt, ##args)
#else
#define GSE_ERR(fmt, args...)
#define GSE_LOG(fmt, args...)
#define GSE_FUN(f)
#define DMT_DATA(dev, format, ...)
#endif
/*----------------------------------------------------------------------------*/
static struct data_resolution dmard11_data_resolution[1] = {
 /* combination by {FULL_RES,RANGE}*/
    //{{ 2, 9}, 256},   // D08 dataformat +/-3g  in 11-bit resolution;  { 2, 9} = 2.9 = (6*1000)/(2^11);  341 = (2^11)/(6)   
    //{{ 7, 8}, 128},   // D10 dataformat +/-4g  in 10-bit resolution;  { 7, 8} = 7.8 = (4*1000)/(2^10);  341 = (2^10)/(4)
    //{{ 31, 2}, 32},   // D09 dataformat +/-8g  in 12-bit resolution;  { 3, 9} = 3.9 = (8*1000)/(2^12);  512 = (2^12)/(8)     
    {{ 31, 2}, 256},   // D11 dataformat +/-8g  in 12-bit resolution;  { 3, 9} = 3.9 = (8*1000)/(2^12);  512 = (2^12)/(8)    
//	{{ 3, 9}, 256},   // D11 dataformat +/-8g  in 12-bit resolution;  { 3, 9} = 3.9 = (8*1000)/(2^12);  512 = (2^12)/(8)    
  //   ^^^^^^^^^^^^ these values need MTK check============> note by D11 FAE 	
};
/*----------------------------------------------------------------------------*/
static struct data_resolution dmard11_offset_resolution = {{15, 6}, 64};	// uncertainty
/*--------------------DMARD11 power control function----------------------------------*/
static void DMARD11_power(struct acc_hw *hw, unsigned int on) 
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
			if(!hwPowerOn(hw->power_id, hw->power_vol, "DMARD11"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "DMARD11"))
			{
				GSE_ERR("power off fail!!\n");
			}			  
		}
	}
	power_on = on;    
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int DMARD11_SetDataResolution(struct i2c_client *client)
{
	int err;
	u8  dat, reso;
    struct dmard11_i2c_data *obj = i2c_get_clientdata(client);

 	obj->reso = &dmard11_data_resolution[0];
	return 0;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_ReadData(struct i2c_client *client, s16 data[DMARD11_AXES_NUM])
{
	struct dmard11_i2c_data *priv = i2c_get_clientdata(client);        
	u8 addr = REG_STAT;
	u8 buf[DMARD11_DATA_LEN] = {0};
	int err = 0;
	int i;
	int tmp=0;
	u8 ofs[3];

	if(NULL == client)
	{
		err = -EINVAL;
	}
	else if(err = hwmsen_read_block(client, addr, buf, DMARD11_DATA_LEN))
	{
		GSE_ERR("error: %d\n", err);
	}
	else
	{
		data[DMARD11_AXIS_X] = (s16)((buf[(DMARD11_AXIS_X+1)*2+1] << 8) |
		         (buf[(DMARD11_AXIS_X+1)*2] ));
		data[DMARD11_AXIS_Y] = (s16)((buf[(DMARD11_AXIS_Y+1)*2+1] << 8) |
		         (buf[(DMARD11_AXIS_Y+1)*2] ));
		data[DMARD11_AXIS_Z] = (s16)((buf[(DMARD11_AXIS_Z+1)*2+1] << 8) |
		         (buf[(DMARD11_AXIS_Z+1)*2] ));
		
		if(atomic_read(&priv->trace) & ADX_TRC_RAWDATA)
		{
			GSE_LOG("from register[%08X %08X %08X]\n",data[DMARD11_AXIS_X], data[DMARD11_AXIS_Y], data[DMARD11_AXIS_Z]);
		}

		for(i=0;i<3;i++)				
		{		
		//	data[i]>>=3;
			data[i]&=0x0fff;  //	data[i]&=0x01ff;
		//because the data is store in binary complement number formation in computer system
			//if ( data[i] == 0x0100 )	//so we want to calculate actual number here
			//	data[i]= -1024;			//11bit resolution, 512= 2^(11-1)
			//else 
				if ( data[i] & 0x0800 )//transfor format , neg num -
			{							//GSE_LOG("data 0 step %x \n",data[i]);
				data[i] -= 0x1;			//GSE_LOG("data 1 step %x \n",data[i]);
				data[i] = ~data[i];		//GSE_LOG("data 2 step %x \n",data[i]);
				data[i] &= 0x07ff;		//GSE_LOG("data 3 step %x \n\n",data[i]);
				data[i] = -data[i];		
			}
			
			  
			  //data[i]<<=3;
			  #ifndef USE_MTK_CALIBRATE
			  data[i] -= priv->offset.v[i]; //D11 raw data - D11 offset
			  #endif
		}	
		
		data[DMARD11_AXIS_Y] = -data[DMARD11_AXIS_Y];	//regulate the orientation
		
		if(atomic_read(&priv->trace) & ADX_TRC_RAWDATA)
		{
			GSE_LOG("after transfer[%08X %08X %08X] => [%5d %5d %5d]\n", data[DMARD11_AXIS_X], data[DMARD11_AXIS_Y], data[DMARD11_AXIS_Z],
		                               data[DMARD11_AXIS_X], data[DMARD11_AXIS_Y], data[DMARD11_AXIS_Z]);
		}
#ifdef CONFIG_DMARD11_LOWPASS
		if(atomic_read(&priv->filter))
		{
			if(atomic_read(&priv->fir_en) && !atomic_read(&priv->suspend))
			{
				int idx, firlen = atomic_read(&priv->firlen);   
				if(priv->fir.num < firlen)
				{                
					priv->fir.raw[priv->fir.num][DMARD11_AXIS_X] = data[DMARD11_AXIS_X];
					priv->fir.raw[priv->fir.num][DMARD11_AXIS_Y] = data[DMARD11_AXIS_Y];
					priv->fir.raw[priv->fir.num][DMARD11_AXIS_Z] = data[DMARD11_AXIS_Z];
					priv->fir.sum[DMARD11_AXIS_X] += data[DMARD11_AXIS_X];
					priv->fir.sum[DMARD11_AXIS_Y] += data[DMARD11_AXIS_Y];
					priv->fir.sum[DMARD11_AXIS_Z] += data[DMARD11_AXIS_Z];
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d]\n", priv->fir.num,
							priv->fir.raw[priv->fir.num][DMARD11_AXIS_X], priv->fir.raw[priv->fir.num][DMARD11_AXIS_Y], priv->fir.raw[priv->fir.num][DMARD11_AXIS_Z],
							priv->fir.sum[DMARD11_AXIS_X], priv->fir.sum[DMARD11_AXIS_Y], priv->fir.sum[DMARD11_AXIS_Z]);
					}
					priv->fir.num++;
					priv->fir.idx++;
				}
				else
				{
					idx = priv->fir.idx % firlen;
					priv->fir.sum[DMARD11_AXIS_X] -= priv->fir.raw[idx][DMARD11_AXIS_X];
					priv->fir.sum[DMARD11_AXIS_Y] -= priv->fir.raw[idx][DMARD11_AXIS_Y];
					priv->fir.sum[DMARD11_AXIS_Z] -= priv->fir.raw[idx][DMARD11_AXIS_Z];
					priv->fir.raw[idx][DMARD11_AXIS_X] = data[DMARD11_AXIS_X];
					priv->fir.raw[idx][DMARD11_AXIS_Y] = data[DMARD11_AXIS_Y];
					priv->fir.raw[idx][DMARD11_AXIS_Z] = data[DMARD11_AXIS_Z];
					priv->fir.sum[DMARD11_AXIS_X] += data[DMARD11_AXIS_X];
					priv->fir.sum[DMARD11_AXIS_Y] += data[DMARD11_AXIS_Y];
					priv->fir.sum[DMARD11_AXIS_Z] += data[DMARD11_AXIS_Z];
					priv->fir.idx++;
					data[DMARD11_AXIS_X] = priv->fir.sum[DMARD11_AXIS_X]/firlen;
					data[DMARD11_AXIS_Y] = priv->fir.sum[DMARD11_AXIS_Y]/firlen;
					data[DMARD11_AXIS_Z] = priv->fir.sum[DMARD11_AXIS_Z]/firlen;
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d] : [%5d %5d %5d]\n", idx,
						priv->fir.raw[idx][DMARD11_AXIS_X], priv->fir.raw[idx][DMARD11_AXIS_Y], priv->fir.raw[idx][DMARD11_AXIS_Z],
						priv->fir.sum[DMARD11_AXIS_X], priv->fir.sum[DMARD11_AXIS_Y], priv->fir.sum[DMARD11_AXIS_Z],
						data[DMARD11_AXIS_X], data[DMARD11_AXIS_Y], data[DMARD11_AXIS_Z]);
					}
				}
			}
		}	
#endif         
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_ReadOffset(struct i2c_client *client, s8 ofs[DMARD11_AXES_NUM])
{    
	int err;
#ifdef SW_CALIBRATION
	ofs[0]=ofs[1]=ofs[2]=0x0;
#else
	
#endif
	//GSE_LOG("offesx=%x, y=%x, z=%x",ofs[0],ofs[1],ofs[2]);
	
	return err;    
}
/*----------------------------------------------------------------------------*/
static int DMARD11_ResetCalibration_2(struct i2c_client *client)
{
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	u8 ofs[4]={0,0,0,0};
	int err;
	
	#ifdef SW_CALIBRATION
		
	#else
		
	#endif

	//memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	memset(obj->offset, 0x00, sizeof(obj->offset));
	#ifndef USE_MTK_CALIBRATE
	memset(obj->offset.v, 0x00, sizeof(obj->offset.v)); /* initialize the offset value */
	#endif
	return err;    
}

static int DMARD11_ResetCalibration(struct i2c_client *client)
{
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	u8 ofs[4]={0,0,0,0};
	int err;
	
	#ifdef SW_CALIBRATION
		
	#else
		
	#endif

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	memset(obj->offset, 0x00, sizeof(obj->offset));
	#ifndef USE_MTK_CALIBRATE
	memset(obj->offset.v, 0x00, sizeof(obj->offset.v)); /* initialize the offset value */
	#endif
	return err;    
}
/*----------------------------------------------------------------------------*/
static int DMARD11_ReadCalibration(struct i2c_client *client, int dat[DMARD11_AXES_NUM])
{
    struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
    int err;
    int mul;

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
	    
	#endif

    dat[obj->cvt.map[DMARD11_AXIS_X]] = obj->cvt.sign[DMARD11_AXIS_X]*(obj->offset[DMARD11_AXIS_X]*mul + obj->cali_sw[DMARD11_AXIS_X]);
    dat[obj->cvt.map[DMARD11_AXIS_Y]] = obj->cvt.sign[DMARD11_AXIS_Y]*(obj->offset[DMARD11_AXIS_Y]*mul + obj->cali_sw[DMARD11_AXIS_Y]);
    dat[obj->cvt.map[DMARD11_AXIS_Z]] = obj->cvt.sign[DMARD11_AXIS_Z]*(obj->offset[DMARD11_AXIS_Z]*mul + obj->cali_sw[DMARD11_AXIS_Z]);                        
                                       
    return 0;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_ReadCalibrationEx(struct i2c_client *client, int act[DMARD11_AXES_NUM], int raw[DMARD11_AXES_NUM])
{  
	/*raw: the raw calibration data; act: the actual calibration data*/
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int mul;

 

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
		
	#endif
	
	raw[DMARD11_AXIS_X] = obj->offset[DMARD11_AXIS_X]*mul + obj->cali_sw[DMARD11_AXIS_X];
	raw[DMARD11_AXIS_Y] = obj->offset[DMARD11_AXIS_Y]*mul + obj->cali_sw[DMARD11_AXIS_Y];
	raw[DMARD11_AXIS_Z] = obj->offset[DMARD11_AXIS_Z]*mul + obj->cali_sw[DMARD11_AXIS_Z];

	act[obj->cvt.map[DMARD11_AXIS_X]] = obj->cvt.sign[DMARD11_AXIS_X]*raw[DMARD11_AXIS_X];
	act[obj->cvt.map[DMARD11_AXIS_Y]] = obj->cvt.sign[DMARD11_AXIS_Y]*raw[DMARD11_AXIS_Y];
	act[obj->cvt.map[DMARD11_AXIS_Z]] = obj->cvt.sign[DMARD11_AXIS_Z]*raw[DMARD11_AXIS_Z];                        
	                       
	return 0;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_WriteCalibration(struct i2c_client *client, int dat[DMARD11_AXES_NUM])
{
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int cali[DMARD11_AXES_NUM], raw[DMARD11_AXES_NUM];
	int lsb = dmard11_offset_resolution.sensitivity;
	int divisor = obj->reso->sensitivity/lsb;

	if(err = DMARD11_ReadCalibrationEx(client, cali, raw))	/*offset will be updated in obj->offset*/
	{ 
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}

	GSE_LOG("OLDOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		raw[DMARD11_AXIS_X], raw[DMARD11_AXIS_Y], raw[DMARD11_AXIS_Z],
		obj->offset[DMARD11_AXIS_X], obj->offset[DMARD11_AXIS_Y], obj->offset[DMARD11_AXIS_Z],
		obj->cali_sw[DMARD11_AXIS_X], obj->cali_sw[DMARD11_AXIS_Y], obj->cali_sw[DMARD11_AXIS_Z]);

	/*calculate the real offset expected by caller*/
	cali[DMARD11_AXIS_X] += dat[DMARD11_AXIS_X];
	cali[DMARD11_AXIS_Y] += dat[DMARD11_AXIS_Y];
	cali[DMARD11_AXIS_Z] += dat[DMARD11_AXIS_Z];

	GSE_LOG("UPDATE: (%+3d %+3d %+3d)\n", 
		dat[DMARD11_AXIS_X], dat[DMARD11_AXIS_Y], dat[DMARD11_AXIS_Z]);

#ifdef SW_CALIBRATION
#ifdef DMARD11_DATA_MAPPING
	obj->cali_sw[obj->cvt.map[DMARD11_AXIS_X]] = obj->cvt.sign[DMARD11_AXIS_X]*(cali[DMARD11_AXIS_X]);
	obj->cali_sw[obj->cvt.map[DMARD11_AXIS_Y]] = obj->cvt.sign[DMARD11_AXIS_Y]*(cali[DMARD11_AXIS_Y]);
	obj->cali_sw[obj->cvt.map[DMARD11_AXIS_Z]] = obj->cvt.sign[DMARD11_AXIS_Z]*(cali[DMARD11_AXIS_Z]);

#else
	obj->cali_sw[DMARD11_AXIS_X] = obj->cvt.sign[DMARD11_AXIS_X]*(cali[obj->cvt.map[DMARD11_AXIS_X]]);
	obj->cali_sw[DMARD11_AXIS_Y] = obj->cvt.sign[DMARD11_AXIS_Y]*(cali[obj->cvt.map[DMARD11_AXIS_Y]]);
	obj->cali_sw[DMARD11_AXIS_Z] = obj->cvt.sign[DMARD11_AXIS_Z]*(cali[obj->cvt.map[DMARD11_AXIS_Z]]);	
#endif
#else
#ifdef DMARD11_DATA_MAPPING
	obj->offset[obj->cvt.map[DMARD11_AXIS_X]] = (s8)(obj->cvt.sign[DMARD11_AXIS_X]*(cali[DMARD11_AXIS_X])/(divisor));
	obj->offset[obj->cvt.map[DMARD11_AXIS_Y]] = (s8)(obj->cvt.sign[DMARD11_AXIS_Y]*(cali[DMARD11_AXIS_Y])/(divisor));
	obj->offset[obj->cvt.map[DMARD11_AXIS_Z]] = (s8)(obj->cvt.sign[DMARD11_AXIS_Z]*(cali[DMARD11_AXIS_Z])/(divisor));

	/*convert software calibration using standard calibration*/
	obj->cali_sw[obj->cvt.map[DMARD11_AXIS_X]] = obj->cvt.sign[DMARD11_AXIS_X]*(cali[DMARD11_AXIS_X])%(divisor);
	obj->cali_sw[obj->cvt.map[DMARD11_AXIS_Y]] = obj->cvt.sign[DMARD11_AXIS_Y]*(cali[DMARD11_AXIS_Y])%(divisor);
	obj->cali_sw[obj->cvt.map[DMARD11_AXIS_Z]] = obj->cvt.sign[DMARD11_AXIS_Z]*(cali[DMARD11_AXIS_Z])%(divisor);

#else
	obj->offset[DMARD11_AXIS_X] = (s8)(obj->cvt.sign[DMARD11_AXIS_X]*(cali[obj->cvt.map[DMARD11_AXIS_X]])/(divisor));
	obj->offset[DMARD11_AXIS_Y] = (s8)(obj->cvt.sign[DMARD11_AXIS_Y]*(cali[obj->cvt.map[DMARD11_AXIS_Y]])/(divisor));
	obj->offset[DMARD11_AXIS_Z] = (s8)(obj->cvt.sign[DMARD11_AXIS_Z]*(cali[obj->cvt.map[DMARD11_AXIS_Z]])/(divisor));

	/*convert software calibration using standard calibration*/
	obj->cali_sw[DMARD11_AXIS_X] = obj->cvt.sign[DMARD11_AXIS_X]*(cali[obj->cvt.map[DMARD11_AXIS_X]])%(divisor);
	obj->cali_sw[DMARD11_AXIS_Y] = obj->cvt.sign[DMARD11_AXIS_Y]*(cali[obj->cvt.map[DMARD11_AXIS_Y]])%(divisor);
	obj->cali_sw[DMARD11_AXIS_Z] = obj->cvt.sign[DMARD11_AXIS_Z]*(cali[obj->cvt.map[DMARD11_AXIS_Z]])%(divisor);
#endif
	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		obj->offset[DMARD11_AXIS_X]*divisor + obj->cali_sw[DMARD11_AXIS_X], 
		obj->offset[DMARD11_AXIS_Y]*divisor + obj->cali_sw[DMARD11_AXIS_Y], 
		obj->offset[DMARD11_AXIS_Z]*divisor + obj->cali_sw[DMARD11_AXIS_Z], 
		obj->offset[DMARD11_AXIS_X], obj->offset[DMARD11_AXIS_Y], obj->offset[DMARD11_AXIS_Z],
		obj->cali_sw[DMARD11_AXIS_X], obj->cali_sw[DMARD11_AXIS_Y], obj->cali_sw[DMARD11_AXIS_Z]);

	
#endif

	return err;
}
int hwmsen_read_byte_sr(struct i2c_client *client, u8 addr, u8 *data)
{
   u8 buf;
    int ret = 0;
	
    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
    buf = addr;
	ret = i2c_master_send(client, (const char*)&buf, 1<<8 | 1);
    //ret = i2c_master_send(client, (const char*)&buf, 1);
    if (ret < 0) {
        GSE_LOG("send command error!!\n");
        return -EFAULT;
    }
     *data = buf;
	client->addr = client->addr& I2C_MASK_FLAG;
    return 0;
}

/*----------------------------------------------------------------------------*/
static int DMARD11_CheckDeviceID(struct i2c_client *client)
{
	u8 databuf[3];    
	int res = 0;
	int retry_count=0;

	memset(databuf, 0, sizeof(u8)*3);
	/* 1. Active Mode  */
	databuf[0] = REG_ACTR;
	databuf[1] = MODE_ACTIVE;
	res = i2c_master_send(client,databuf,2);
	if (res < 0)
	{
	    GSE_ERR("write ACTR fail, %d\n", res);
	    return DMARD11_ERR_I2C;
	}
	/*check REG_ACTR to make sure IC Init ready*/
	while(retry_count<3)
	{
		res = hwmsen_read_block(client, REG_ACTR, databuf, 1);
	    if(res)
		{   
		    GSE_LOG("check REG_ACTR ready bit error, res is %d !!\n",res);
			return DMARD11_ERR_I2C;
		}

		if(databuf[0] & VALUE_INIT_READY)
		{
			GSE_LOG("DMARD11_IC_INIT_DONE!\n");
			break;
		}
		retry_count++;
		mdelay(1);
	}
	if(retry_count>=3)
	{
		GSE_LOG("DMARD11_IC_INIT_FAIL!!!\n");
	}

	/* 2. check D11 who am I */ 
    res = hwmsen_read_block(client, REG_WHO, databuf, 1);
    if(res)
	{   
	    GSE_LOG("DMARD11 check Who am I fail!! res is %d !!\n",res);
		return DMARD11_ERR_I2C;
	}

	if( databuf[0] == VALUE_WHO_AM_I)
	{
		//GSE_LOG("D11 WHO_AM_I_VALUE = %d \n", databuf[0]);
		GSE_LOG("DMARD11_CheckDeviceID %d success!\n ", databuf[0]);
	}
	else
	{
		GSE_LOG("DMARD11_CheckDeviceID %d failt!\n ", databuf[0]);
		return DMARD11_ERR_IDENTIFICATION;
	}
	
	
	/* 3. Write SP parameter*/
	databuf[0] = REG_SP;
	databuf[1] = 0x1e;
	res = i2c_master_send(client, databuf, 2);
	if (res < 0)
	{
	    GSE_ERR("write SP fail, %d\n", res);
	    return DMARD11_ERR_I2C;
	}
	
	/* 4. open HW filter*/
	databuf[0] = REG_FILTER;
	databuf[1] = 0x40;		//0x40:filter ODR=25hz;0x00:filter ODR=50hz;0x10:filter ODR=100hz;0x20:filter ODR=200hz;0x60:filter ODR=400hz;
	res = i2c_master_send(client, databuf, 2);
	if (res < 0)
	{
	    GSE_ERR("write HW filter fail, %d\n", res);
	    return DMARD11_ERR_I2C;
	}
	
	/* 5.Set Data conversion rate 1*/
	databuf[0] = REG_CNT_L1;
	databuf[1] = 0x99;   //LPF + 100hz data conver + watch dog enable
	res = i2c_master_send(client, databuf, 2);
	if (res < 0)
	{
	    GSE_ERR("Set Data conversion rate 1 fail, %d\n", res);
	    return DMARD11_ERR_I2C;
	}
	
	/* 6.Set Data moving avg */
	databuf[0] = 0x41;
	databuf[1] = 0x12;		//average by 4
	res = i2c_master_send(client, databuf, 2);
	if (res < 0)
	{
	    GSE_ERR("Set Data conversion rate 2 fail, %d\n", res);
	    return DMARD11_ERR_I2C;
	}
	
	return DMARD11_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_SetPowerMode(struct i2c_client *client, bool enable)
{
	u8 databuf[2];    
	int res = 0;	

	memset(databuf, 0, sizeof(databuf));

    if(enable == sensor_power )
	{
		GSE_LOG("Sensor power status is newest!\n");
		return DMARD11_SUCCESS;
	}
	else
	{  	
#if 0
		res = hwmsen_read_block(client, REG_ACTR, databuf, 1);
	    if(res)
		{   
		    GSE_LOG("check REG_ACTR ready bit error, res is %d !!\n",res);
			return DMARD11_ERR_I2C;
		}
		if(enable)
		{
			databuf[1] = databuf[0] | MODE_ACTIVE;
		}
		else
		{
			databuf[1] = databuf[0] & (~MODE_ACTIVE);
		}
		databuf[0] = REG_ACTR;
		mdelay(1);
#else		
		/* 1. Active Mode  */
		databuf[0] = REG_ACTR;
		databuf[1] = enable? MODE_ACTIVE: MODE_POWERDOWN;
#endif
		res = i2c_master_send(client,databuf,2);
		if (res < 0)
		{
		    GSE_ERR("write ACTR fail, %d\n", res);
			return DMARD11_ERR_I2C;
		}
	}
	sensor_power = enable;

	//mdelay(20);
	mdelay(1);
	
	return DMARD11_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int DMARD11_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    

	return DMARD11_SetDataResolution(client);    
}
/*----------------------------------------------------------------------------*/
static int DMARD11_SetBWRate(struct i2c_client *client, u8 bwrate)
{
	u8 databuf[2];    
	int res = 0;

	memset(databuf, 0, sizeof(databuf));  
	
	res = hwmsen_read_block(client, REG_ACTR, databuf, 1);
	if(res)
	{	
		GSE_LOG("check REG_ACTR ready bit error, res is %d !!\n",res);
		return DMARD11_ERR_I2C;
	}
	
	if(bwrate < 50)
	{
		databuf[1] = (databuf[0] & (~0x1c)) | 0x10;		//ODR=20HZ
	}
	else if((bwrate>=50) && (bwrate<100))
	{
		databuf[1] = (databuf[0] & (~0x1c)) | 0x14; 	//ODR=50HZ
	}
	else if((bwrate>=100) && (bwrate<200))
	{
		databuf[1] = (databuf[0] & (~0x1c)) | 0x18; 	//ODR=100HZ
	}
	else if(bwrate>=200)
	{
		databuf[1] = (databuf[0] & (~0x1c)) | 0x1c; 	//ODR=200HZ
	}
	/* 5.Set Data conversion rate 1*/
	databuf[0] = REG_CNT_L1;
	//databuf[1] = 0x99;   //LPF + 100hz data conver + watch dog enable
	res = i2c_master_send(client, databuf, 2);
	if (res < 0)
	{
	    GSE_ERR("Set Data conversion rate 1 fail, %d\n", res);
	    return DMARD11_ERR_I2C;
	}

	return DMARD11_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int DMARD11_SetIntEnable(struct i2c_client *client, u8 intenable)
{
			u8 databuf[10];    
			int res = 0;
		
			
			/*for disable interrupt function*/
			
			return DMARD11_SUCCESS;	  
}

/*----------------------------------------------------------------------------*/
static int dmard11_init_client(struct i2c_client *client, int reset_cali)
{
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;
	static bool firsttime=true;
	int acc_x,acc_y;
    static int valuecounter=0;
    res = DMARD11_CheckDeviceID(client); 
	if(res != DMARD11_SUCCESS)
	{
		return res;
	}	
	
#ifdef G_SENSOR_SELF_TEST
	if(reset_cali==1){
		do
		 {
			DMARD11_ReadData(client, obj->data);
			acc_x = obj->data[DMARD11_AXIS_X];
			acc_y = obj->data[DMARD11_AXIS_Y];
			//printk("-------(obj->data[DMARD11_AXIS_X]) is %d,(obj->data[DMARD11_AXIS_Y]) is %d-----\n",(obj->data[DMARD11_AXIS_X]),(obj->data[DMARD11_AXIS_Y]));
			if((abs(acc_x) > SELF_TEST_RANGE ) || (abs(acc_y) > SELF_TEST_RANGE ) || (abs(acc_x) >IC_SENSITIVITY && abs(acc_y) > IC_SENSITIVITY))
			{  
				valuecounter++;
				if(valuecounter>=30) 
				{
				  valuecounter=0;
				  return -1;	
				}
			}else{
				break;
			}
		 }while( abs(acc_x) >SELF_TEST_RANGE || abs(acc_y) > SELF_TEST_RANGE || (abs(acc_x) >IC_SENSITIVITY && abs(acc_y) > IC_SENSITIVITY)); 
	}
#endif
	
	DMARD11_SetDataResolution(client);
	//res = DMARD11_SetPowerMode(client, false);
	res = DMARD11_SetPowerMode(client, power_status);
	if(res != DMARD11_SUCCESS)
	{
		return res;
	}
	GSE_LOG("DMARD11_SetPowerMode OK!\n");
	
	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = obj->reso->sensitivity;

	if(0 != reset_cali)
	{ 
		/*reset calibration only in power on*/
		res = DMARD11_ResetCalibration(client);
		if(res != DMARD11_SUCCESS)
		{
			return res;
		}
	}
	#ifndef USE_MTK_CALIBRATE
	if(firsttime){
		D11_read_offset_from_file(client);	
	 	firsttime=false;
	}
	#endif
	GSE_LOG("dmard11_init_client OK!\n");
#ifdef CONFIG_DMARD11_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif

	mdelay(20);

	return DMARD11_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
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

	sprintf(buf, "DMARD11 Chip");
	return 0;
}
/*----------------------------------------------------------------------------*/
#ifndef USE_MTK_CALIBRATE
int stable_check(s16 val_check, s16 *value_tmp ,int *value_check_time_counter)
{	
	if (*value_check_time_counter ==0){
		*value_tmp = val_check;
		(*value_check_time_counter)++;
	}
	if (*value_check_time_counter > 0 ){
		if( abs(*value_tmp - val_check) < ACC_VALUE_04  ){
			(*value_check_time_counter)++;
		}else{
			//value_tmp = 0;
			*value_check_time_counter =0;
		}
	}
	if (*value_check_time_counter > 8  ){   //need to set the counter
		*value_check_time_counter = 0;
		return 1;
	}
	return 0;
}
#endif

/*----------------------------------------------------------------------------*/

static int  enter_calibrate = 0;
static int DMARD11_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
	struct dmard11_i2c_data *obj = (struct dmard11_i2c_data*)i2c_get_clientdata(client);
	u8 databuf[20];
	int acc[DMARD11_AXES_NUM];
	int res = 0;
	memset(databuf, 0, sizeof(u8)*10);
	static	s16 value_tmp1 = 0;	
	static	int value_check_time_counter1 = 0;	

	if(NULL == buf)
	{
		return -1;
	}
	
	memset(buf, 0, bufsize);
	
	if(NULL == client)
	{
		//*buf = 0;
		return -2;
	}

	if(sensor_power == false)
	{
		res = DMARD11_CheckDeviceID(client);
		//res = DMARD11_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on DMARD11 error %d!\n", res);
		}
	}

	if(res = DMARD11_ReadData(client, obj->data))
	{       
	/********************************************************/
		if(sensor_power == false)
		{
			res = DMARD11_SetPowerMode(client, sensor_power);
			if(res)
			{
				GSE_ERR("Power on DMARD11 error %d!\n", res);
			}
		}
	/********************************************************/
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
	/********************************************************/
		if(sensor_power == false)
		{
			res = DMARD11_SetPowerMode(client, sensor_power);
			if(res)
			{
				GSE_ERR("Power on DMARD11 error %d!\n", res);
			}
		}
	/********************************************************/
	    //printk("--0---RAW x:%d,RAW y:%d,RAW z: %d,offset X: %d,offset Y: %d,offset Z: %d---\n",obj->data[DMARD11_AXIS_X],obj->data[DMARD11_AXIS_Y],obj->data[DMARD11_AXIS_Z],obj->cali_sw[DMARD11_AXIS_X],obj->cali_sw[DMARD11_AXIS_Y],obj->cali_sw[DMARD11_AXIS_Z]);
		//GSE_LOG("raw data x=%d, y=%d, z=%d \n",obj->data[DMARD11_AXIS_X],obj->data[DMARD11_AXIS_Y],obj->data[DMARD11_AXIS_Z]);
#if 1
		obj->data[DMARD11_AXIS_X] = obj->data[DMARD11_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		obj->data[DMARD11_AXIS_Y] = obj->data[DMARD11_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		obj->data[DMARD11_AXIS_Z] = obj->data[DMARD11_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
#endif
		obj->data[DMARD11_AXIS_X] += obj->cali_sw[DMARD11_AXIS_X];
		obj->data[DMARD11_AXIS_Y] += obj->cali_sw[DMARD11_AXIS_Y];
		obj->data[DMARD11_AXIS_Z] += obj->cali_sw[DMARD11_AXIS_Z];
		
	   //printk("--0---acc x:%d,acc y:%d,acc z: %d-------\n",acc[DMARD11_AXIS_X],acc[DMARD11_AXIS_Y],acc[DMARD11_AXIS_Z]);

		//GSE_LOG("cali_sw x=%d, y=%d, z=%d \n",obj->cali_sw[DMARD11_AXIS_X],obj->cali_sw[DMARD11_AXIS_Y],obj->cali_sw[DMARD11_AXIS_Z]);
		//printk("--1---acc x:%d,acc y:%d,acc z: %d-------\n",acc[DMARD11_AXIS_X],acc[DMARD11_AXIS_Y],acc[DMARD11_AXIS_Z]);
		/*remap coordinate*/
#ifdef DMARD11_DATA_MAPPING
		acc[DMARD11_AXIS_X] = obj->cvt.sign[DMARD11_AXIS_X]*obj->data[obj->cvt.map[DMARD11_AXIS_X]];
		acc[DMARD11_AXIS_Y] = obj->cvt.sign[DMARD11_AXIS_Y]*obj->data[obj->cvt.map[DMARD11_AXIS_Y]];
		acc[DMARD11_AXIS_Z] = obj->cvt.sign[DMARD11_AXIS_Z]*obj->data[obj->cvt.map[DMARD11_AXIS_Z]];

#else
		acc[obj->cvt.map[DMARD11_AXIS_X]] = obj->cvt.sign[DMARD11_AXIS_X]*obj->data[DMARD11_AXIS_X];
		acc[obj->cvt.map[DMARD11_AXIS_Y]] = obj->cvt.sign[DMARD11_AXIS_Y]*obj->data[DMARD11_AXIS_Y];
		acc[obj->cvt.map[DMARD11_AXIS_Z]] = obj->cvt.sign[DMARD11_AXIS_Z]*obj->data[DMARD11_AXIS_Z];
#endif
		//GSE_LOG("cvt x=%d, y=%d, z=%d \n",obj->cvt.sign[BMA150_AXIS_X],obj->cvt.sign[BMA150_AXIS_Y],obj->cvt.sign[BMA150_AXIS_Z]);

      //printk("--2---acc x:%d,acc y:%d,acc z: %d-------\n",acc[DMARD11_AXIS_X],acc[DMARD11_AXIS_Y],acc[DMARD11_AXIS_Z]);
		//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[BMA150_AXIS_X], acc[BMA150_AXIS_Y], acc[BMA150_AXIS_Z]);

		//Out put the mg
		//GSE_LOG("mg acc=%d, GRAVITY=%d, sensityvity=%d \n",acc[DMARD11_AXIS_Z],GRAVITY_EARTH_1000,obj->reso->sensitivity);		
#if 0
		acc[DMARD11_AXIS_X] = acc[DMARD11_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[DMARD11_AXIS_Y] = acc[DMARD11_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[DMARD11_AXIS_Z] = acc[DMARD11_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;		
#endif
		//printk("--3---acc x:%d,acc y:%d,acc z: %d-------\n",acc[DMARD11_AXIS_X],acc[DMARD11_AXIS_Y],acc[DMARD11_AXIS_Z]);
		
#ifdef CALIBRATE_INVERSE   
	
		if((obj->cali_sw[DMARD11_AXIS_X]==0) && (obj->cali_sw[DMARD11_AXIS_Y]==0) && (obj->cali_sw[DMARD11_AXIS_Z]==0 ))
		{  
			//printk("------no calibrate---------\n ");
		}
		else
		{
			//printk("-------has clibrated ------\n");
			if((enter_calibrate == 0) && (abs(acc[DMARD11_AXIS_Z])>ACC_VALUE_15))
			{	
				if(stable_check(acc[DMARD11_AXIS_Z],&value_tmp1,&value_check_time_counter1) )
				{
					enter_calibrate =1;
				   //printk("----1---enter_calibrate:%d----------\n",enter_calibrate);
			    }
		 	}
			
			if(enter_calibrate == 1)
			{
				acc[DMARD11_AXIS_Z] = acc[DMARD11_AXIS_Z] - DEFAULT_SENSITIVITY*2;
			}
			//printk("----2---enter_calibrate:%d----------\n",enter_calibrate);
		
}

#endif		
#ifdef STABLE_VALUE_FUNCTION 
		if(abs(acc[DMARD11_AXIS_X]) < ACC_VALUE_03 ){ acc[DMARD11_AXIS_X]=0; }
		if(abs(acc[DMARD11_AXIS_Y]) < ACC_VALUE_03 ){ acc[DMARD11_AXIS_Y]=0; }
#endif				
		

		sprintf(buf, "%04x %04x %04x", acc[DMARD11_AXIS_X], acc[DMARD11_AXIS_Y], acc[DMARD11_AXIS_Z]);
		if(atomic_read(&obj->trace) & ADX_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
		}
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_ReadRawData(struct i2c_client *client, char *buf)
{
	struct dmard11_i2c_data *obj = (struct dmard11_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return EINVAL;
	}
	
	if(res = DMARD11_ReadData(client, obj->data))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{
		sprintf(buf, "DMARD11_ReadRawData %04x %04x %04x", obj->data[DMARD11_AXIS_X], 
			obj->data[DMARD11_AXIS_Y], obj->data[DMARD11_AXIS_Z]);
	
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int DMARD11_InitSelfTest(struct i2c_client *client)
{
	int res = 0;

	DMARD11_SetPowerMode(client, true);

	
	
	return DMARD11_SUCCESS;
}
/*----------------------------------------------------------------------------*/
/*
static int DMARD11_JudgeTestResult(struct i2c_client *client)
{

	struct dmard11_i2c_data *obj = (struct dmard11_i2c_data*)i2c_get_clientdata(client);
	int res = 0;
	s16  acc[DMARD11_AXES_NUM];
	int  self_result;

	
	if(res = DMARD11_ReadData(client, acc))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{			
		GSE_LOG("0 step: %d %d %d\n", acc[0],acc[1],acc[2]);

		acc[DMARD11_AXIS_X] = acc[DMARD11_AXIS_X] * 1000 / 128;
		acc[DMARD11_AXIS_Y] = acc[DMARD11_AXIS_Y] * 1000 / 128;
		acc[DMARD11_AXIS_Z] = acc[DMARD11_AXIS_Z] * 1000 / 128;
		
		GSE_LOG("1 step: %d %d %d\n", acc[0],acc[1],acc[2]);
		
		self_result = acc[DMARD11_AXIS_X]*acc[DMARD11_AXIS_X] 
			+ acc[DMARD11_AXIS_Y]*acc[DMARD11_AXIS_Y] 
			+ acc[DMARD11_AXIS_Z]*acc[DMARD11_AXIS_Z];
			
		
		GSE_LOG("2 step: result = %d", self_result);

	    if ( (self_result>550000) && (self_result<1700000) ) //between 0.55g and 1.7g 
	    {												 
			GSE_ERR("DMARD11_JudgeTestResult successful\n");
			return DMARD11_SUCCESS;
		}
		{
	        GSE_ERR("DMARD11_JudgeTestResult failt\n");
	        return -EINVAL;
	    }
	
	}
	
}
*/
/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = dmard11_i2c_client;
	char strbuf[DMARD11_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	memset(strbuf, 0,sizeof(strbuf));
	
	DMARD11_ReadChipInfo(client, strbuf, DMARD11_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}

static ssize_t gsensor_init(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = dmard11_i2c_client;
		char strbuf[DMARD11_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("i2c client is null!!\n");
			return 0;
		}
		dmard11_init_client(client, 1);
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}



/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = dmard11_i2c_client;
	char strbuf[DMARD11_BUFSIZE];
	int x = 0;
	int y = 0;
	int z = 0;
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	memset(strbuf, 0,sizeof(strbuf));
	
	DMARD11_ReadSensorData(client, strbuf, DMARD11_BUFSIZE);
	sscanf(strbuf, "%x %x %x", x, y, z);	

	return snprintf(buf, PAGE_SIZE, "%s===>[%d,%d,%d]\n", strbuf, x, y, z);            
}

static ssize_t show_sensorrawdata_value(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = dmard11_i2c_client;
		char strbuf[DMARD11_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("i2c client is null!!\n");
			return 0;
		}
		DMARD11_ReadRawData(client, strbuf);
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}

/*----------------------------------------------------------------------------*/
static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = dmard11_i2c_client;
	struct dmard11_i2c_data *obj;
	int err, len = 0, mul;
	int tmp[DMARD11_AXES_NUM];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);
	if(NULL == obj)
	{
		GSE_ERR("obj is null!!\n");
		return 0;
	}


	if(err = DMARD11_ReadOffset(client, obj->offset))
	{
		return -EINVAL;
	}
	else if(err = DMARD11_ReadCalibration(client, tmp))
	{
		return -EINVAL;
	}
	else
	{    
		mul = obj->reso->sensitivity/dmard11_offset_resolution.sensitivity;
		len += snprintf(buf+len, PAGE_SIZE-len, "[HW ][%d] (%+3d, %+3d, %+3d) : (0x%02X, 0x%02X, 0x%02X)\n", mul,                        
			obj->offset[DMARD11_AXIS_X], obj->offset[DMARD11_AXIS_Y], obj->offset[DMARD11_AXIS_Z],
			obj->offset[DMARD11_AXIS_X], obj->offset[DMARD11_AXIS_Y], obj->offset[DMARD11_AXIS_Z]);
		len += snprintf(buf+len, PAGE_SIZE-len, "[SW ][%d] (%+3d, %+3d, %+3d)\n", 1, 
			obj->cali_sw[DMARD11_AXIS_X], obj->cali_sw[DMARD11_AXIS_Y], obj->cali_sw[DMARD11_AXIS_Z]);

		len += snprintf(buf+len, PAGE_SIZE-len, "[ALL]    (%+3d, %+3d, %+3d) : (%+3d, %+3d, %+3d)\n", 
			obj->offset[DMARD11_AXIS_X]*mul + obj->cali_sw[DMARD11_AXIS_X],
			obj->offset[DMARD11_AXIS_Y]*mul + obj->cali_sw[DMARD11_AXIS_Y],
			obj->offset[DMARD11_AXIS_Z]*mul + obj->cali_sw[DMARD11_AXIS_Z],
			tmp[DMARD11_AXIS_X], tmp[DMARD11_AXIS_Y], tmp[DMARD11_AXIS_Z]);
		
		return len;
    }
}
/*----------------------------------------------------------------------------*/
static ssize_t store_cali_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct i2c_client *client = dmard11_i2c_client;  
	int err, x, y, z;
	int dat[DMARD11_AXES_NUM];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return count;
	}


	if(!strncmp(buf, "rst", 3))
	{
		if(err = DMARD11_ResetCalibration(client))
		{
			GSE_ERR("reset offset err = %d\n", err);
		}	
	}
	else if(3 == sscanf(buf, "0x%02X 0x%02X 0x%02X", &x, &y, &z))
	{
		dat[DMARD11_AXIS_X] = x;
		dat[DMARD11_AXIS_Y] = y;
		dat[DMARD11_AXIS_Z] = z;
		if(err = DMARD11_WriteCalibration(client, dat))
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
/*
static ssize_t show_self_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = dmard11_i2c_client;
	struct dmard11_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	//obj = i2c_get_clientdata(client);
	
    return snprintf(buf, 8, "%s\n", selftestRes);
}
*/
/*----------------------------------------------------------------------------*/
/*
static ssize_t store_self_value(struct device_driver *ddri, char *buf, size_t count)
{   
	struct item{
	s16 raw[DMARD11_AXES_NUM];
	};
	
	struct i2c_client *client = dmard11_i2c_client;  
	int idx, res, num;
	struct item *prv = NULL, *nxt = NULL;


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
	DMARD11_SetPowerMode(client,true); 

	
	DMARD11_InitSelfTest(client);
	GSE_LOG("SELFTEST:\n");    

	if(!DMARD11_JudgeTestResult(client))
	{
		GSE_LOG("SELFTEST : PASS\n");
		strcpy(selftestRes,"y");
	}	
	else
	{
		GSE_LOG("SELFTEST : FAIL\n");		
		strcpy(selftestRes,"n");
	}
	
	exit:
	   
	dmard11_init_client(client, 0);
	kfree(prv);
	kfree(nxt);
	return count;
}
*/
/*----------------------------------------------------------------------------*/
/*
static ssize_t show_selftest_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = dmard11_i2c_client;
	struct dmard11_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->selftest));
}
*/
/*----------------------------------------------------------------------------*/
/*
static ssize_t store_selftest_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct dmard11_i2c_data *obj = obj_i2c_data;
	int tmp;

	if(NULL == obj)
	{
		GSE_ERR("i2c data obj is null!!\n");
		return 0;
	}
	
	
	if(1 == sscanf(buf, "%d", &tmp))
	{        
		if(atomic_read(&obj->selftest) && !tmp)
		{
			
			dmard11_init_client(obj->client, 0);
		}
		else if(!atomic_read(&obj->selftest) && tmp)
		{
			
			DMARD11_InitSelfTest(obj->client);            
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
*/
/*----------------------------------------------------------------------------*/
static ssize_t show_firlen_value(struct device_driver *ddri, char *buf)
{
#ifdef CONFIG_DMARD11_LOWPASS
	struct i2c_client *client = dmard11_i2c_client;
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);
	if(NULL == obj)
	{
		GSE_ERR("obj is null!!\n");
		return 0;
	}
	if(atomic_read(&obj->firlen))
	{
		int idx, len = atomic_read(&obj->firlen);
		GSE_LOG("len = %2d, idx = %2d\n", obj->fir.num, obj->fir.idx);

		for(idx = 0; idx < len; idx++)
		{
			GSE_LOG("[%5d %5d %5d]\n", obj->fir.raw[idx][DMARD11_AXIS_X], obj->fir.raw[idx][DMARD11_AXIS_Y], obj->fir.raw[idx][DMARD11_AXIS_Z]);
		}
		
		GSE_LOG("sum = [%5d %5d %5d]\n", obj->fir.sum[DMARD11_AXIS_X], obj->fir.sum[DMARD11_AXIS_Y], obj->fir.sum[DMARD11_AXIS_Z]);
		GSE_LOG("avg = [%5d %5d %5d]\n", obj->fir.sum[DMARD11_AXIS_X]/len, obj->fir.sum[DMARD11_AXIS_Y]/len, obj->fir.sum[DMARD11_AXIS_Z]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->firlen));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}
/*----------------------------------------------------------------------------*/
static ssize_t store_firlen_value(struct device_driver *ddri, char *buf, size_t count)
{
#ifdef CONFIG_DMARD11_LOWPASS
	struct i2c_client *client = dmard11_i2c_client;  
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);
	int firlen;
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return count;
	}

	obj = i2c_get_clientdata(client);
	if(NULL == obj)
	{
		GSE_ERR("obj is null!!\n");
		return count;
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
	struct dmard11_i2c_data *obj = obj_i2c_data;
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
	struct dmard11_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return count;
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
	struct dmard11_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	
	
	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: i2c_num=%d direction=%d (power_id=%d power_vol=%d)\n", 
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
	if(sensor_power)
		GSE_LOG("G sensor is in work mode, sensor_power = %d\n", sensor_power);
	else
		GSE_LOG("G sensor is in standby mode, sensor_power = %d\n", sensor_power);

	//return 0;
	return sprintf(buf, "sensor_power = %d, power_status = %d\n", sensor_power, power_status);
}

/*----------------------------------------------------------------------------*/
static ssize_t show_layout_value(struct device_driver *ddri, char *buf)
{
	struct dmard11_i2c_data *data = obj_i2c_data;
	if (data == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	

	return sprintf(buf, "(%d, %d)\n[%+2d %+2d %+2d]\n[%+2d %+2d %+2d]\n",
		data->hw->direction,atomic_read(&data->layout),	data->cvt.sign[0], data->cvt.sign[1],
		data->cvt.sign[2],data->cvt.map[0], data->cvt.map[1], data->cvt.map[2]);            
}
/*----------------------------------------------------------------------------*/
static ssize_t store_layout_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct dmard11_i2c_data *data = obj_i2c_data;
	int layout = 0;
	
	if (data == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return count;
	}	

	if(1 == sscanf(buf, "%d", &layout))
	{		
		if(!hwmsen_get_convert(layout, &data->cvt))
		{
			atomic_set(&data->layout, layout);
			GSE_LOG("HWMSEN_GET_CONVERT function success! layout = %d.\n", layout);
		}
		else if(!hwmsen_get_convert(data->hw->direction, &data->cvt))
		{
			GSE_LOG("default layout: %d, restore to %d\n", layout, data->hw->direction);
		}
		else
		{
			hwmsen_get_convert(0, &data->cvt);
		}
	}
	else
	{
		GSE_LOG("invalid format = '%s'\n", buf);
	}
	
	return count;            
}

/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,   	S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata, 	S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(cali,       	S_IWUSR | S_IRUGO, show_cali_value,          store_cali_value);
//static DRIVER_ATTR(selftest, S_IWUSR | S_IRUGO, show_self_value,  store_self_value);
//static DRIVER_ATTR(self,   S_IWUSR | S_IRUGO, show_selftest_value,      store_selftest_value);
static DRIVER_ATTR(firlen,     	S_IWUSR | S_IRUGO, show_firlen_value,        store_firlen_value);
static DRIVER_ATTR(trace,     		S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,        	S_IRUGO, show_status_value,        NULL);
static DRIVER_ATTR(powerstatus,   	S_IRUGO, show_power_status_value,        NULL);
static DRIVER_ATTR(layout,     	S_IRUGO | S_IWUSR, show_layout_value, store_layout_value);


/*----------------------------------------------------------------------------*/
static struct driver_attribute *dmard11_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
//	&driver_attr_self,         /*self test demo*/
//	&driver_attr_selftest,     /*self control: 0: disable, 1: enable*/
	&driver_attr_firlen,       /*filter length: 0: disable, others: enable*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,
	&driver_attr_powerstatus,
	&driver_attr_layout,
};
/*----------------------------------------------------------------------------*/
static int dmard11_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(dmard11_attr_list)/sizeof(dmard11_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(err = driver_create_file(driver, dmard11_attr_list[idx]))
		{            
			GSE_ERR("driver_create_file (%s) = %d\n", dmard11_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int dmard11_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(dmard11_attr_list)/sizeof(dmard11_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, dmard11_attr_list[idx]);
	}
	

	return err;
}

/*----------------------------------------------------------------------------*/
#ifdef DMARD11_NEW_ARCH
static int dmard11_open_report_data(int open)//open data rerport to HAL
{
	return 0;
}
static int dmard11_enable_nodata(int en)//only enable not report event to HAL
{
	struct dmard11_i2c_data *priv = obj_i2c_data;
	int err = 0;
	int value = en;


	if(NULL == obj_i2c_data)
	{
		GSE_ERR("Gsensor obj_i2c_data is NULL\n");
		return -1;
	}
	
	if(atomic_read(&priv->trace) & ADX_TRC_INFO)
	{
		GSE_FUN();
	}	
	
	if(((value == 0) && (sensor_power == false)) ||((value == 1) && (sensor_power == true)))
	{
		GSE_LOG("Gsensor device have updated! sensor_power is %d \n",sensor_power);
	}
	else
	{
		power_status = !sensor_power;
		if(power_status)
		{
			err = DMARD11_SetPowerMode( priv->client, power_status);
			err = DMARD11_CheckDeviceID(priv->client);
		}
		else
		{
			err = DMARD11_SetPowerMode( priv->client, power_status);
		}
		//err = DMARD11_SetPowerMode( priv->client, power_status);
		//err = DMARD11_SetPowerMode( priv->client, !sensor_power);
	}

	return 0;
}
static int dmard11_set_delay(u64 delay)
{
	struct dmard11_i2c_data *priv = obj_i2c_data;
	int err = 0;
	int value = (int)delay/1000/1000;



	if(NULL == obj_i2c_data)
	{
		GSE_ERR("Gsensor obj_i2c_data is NULL\n");
		return -1;
	}
	
	if(atomic_read(&priv->trace) & ADX_TRC_INFO)
	{
		GSE_FUN();
	}	
	
	if(value >= 50)
	{
		atomic_set(&priv->filter, 0);
	}
	else
	{	
#if defined(CONFIG_DMARD11_LOWPASS)
		priv->fir.num = 0;
		priv->fir.idx = 0;
		priv->fir.sum[DMARD11_AXIS_X] = 0;
		priv->fir.sum[DMARD11_AXIS_Y] = 0;
		priv->fir.sum[DMARD11_AXIS_Z] = 0;
		atomic_set(&priv->filter, 1);
#endif
	}	

	return 0;
}

static int dmard11_get_data(int *x,int *y, int *z,int *status)
{
	struct dmard11_i2c_data *priv = obj_i2c_data;
	int err = 0;
	char buff[DMARD11_BUFSIZE];
	
	
	
	if(NULL == obj_i2c_data)
	{
		GSE_ERR("Gsensor obj_i2c_data is NULL\n");
		return -1;
	}
	
	if(atomic_read(&priv->trace) & ADX_TRC_INFO)
	{
		GSE_FUN();
	}

	memset(buff, 0, sizeof(buff));
	err = DMARD11_ReadSensorData(priv->client, buff, DMARD11_BUFSIZE);
	if(err < 0)
	{
		GSE_ERR("DMARD11_ReadSensorData FAIL err = %d\n", err);
		return err;
	}
	
	sscanf(buff, "%x %x %x", x, y, z);				
	*status = SENSOR_STATUS_ACCURACY_MEDIUM;				

	return 0;
}

#else
int gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value, sample_delay;	
	struct dmard11_i2c_data *priv = (struct dmard11_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[DMARD11_BUFSIZE];
	
	if(atomic_read(&priv->trace) & ADX_TRC_INFO)
	{
		GSE_FUN();
	}

	
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
				

				if(value >= 50)
				{
					atomic_set(&priv->filter, 0);
				}
				else
				{	
				#if defined(CONFIG_DMARD11_LOWPASS)
					priv->fir.num = 0;
					priv->fir.idx = 0;
					priv->fir.sum[DMARD11_AXIS_X] = 0;
					priv->fir.sum[DMARD11_AXIS_Y] = 0;
					priv->fir.sum[DMARD11_AXIS_Z] = 0;
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
				if(((value == 0) && (sensor_power == false)) ||((value == 1) && (sensor_power == true)))
				{
					GSE_LOG("Gsensor device have updated! sensor_power is %d \n",sensor_power);
				}
				else
				{
					power_status = !sensor_power;
					if(power_status)
					{
						err = DMARD11_SetPowerMode( priv->client, power_status);
						err = DMARD11_CheckDeviceID(priv->client);
					}
					else
					{
						err = DMARD11_SetPowerMode( priv->client, power_status);
					}
					//err = DMARD11_SetPowerMode( priv->client, !sensor_power);
				}
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
				DMARD11_ReadSensorData(priv->client, buff, DMARD11_BUFSIZE);
				sscanf(buff, "%x %x %x", &gsensor_data->values[0], 
					&gsensor_data->values[1], &gsensor_data->values[2]);				
				gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
				gsensor_data->value_divide = 1000;
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

#ifndef USE_MTK_CALIBRATE
int D11_calibrate(struct i2c_client *client)
{	
	struct dmard11_i2c_data *obj = (struct dmard11_i2c_data*)i2c_get_clientdata(client);

	raw_data avg;
	int i, j;
	long xyz_acc[SENSOR_DATA_SIZE];   
  	s16 xyz[SENSOR_DATA_SIZE];
	/* initialize the offset value */
	for(i = 0; i < SENSOR_DATA_SIZE; ++i)
		obj->offset.v[i] = 0;
	/* initialize the accumulation buffer */
  	for(i = 0; i < SENSOR_DATA_SIZE; ++i) 
		xyz_acc[i] = 0;

	for(i = 0; i < AVG_NUM; i++) 
	{      
		DMARD11_ReadData(client, obj->data);
		for(j = 0; j < SENSOR_DATA_SIZE; ++j) 
			xyz_acc[j] += obj->data[j];
  	}
	/* calculate averages */
  	for(i = 0; i < SENSOR_DATA_SIZE; ++i) 
		avg.v[i] = xyz_acc[i] / AVG_NUM;
		
	if(avg.v[2] < 0)
	{
		obj->offset.v[0] =  avg.v[0] ;    
		obj->offset.v[1] =  avg.v[1] ;
		ovj->offset.v[2] =  avg.v[2] + DEFAULT_SENSITIVITY;
	}
	else
	{	
		obj->offset.v[0] =  avg.v[0] ;    
		obj->offset.v[1] =  avg.v[1] ;
		obj->offset.v[2] =  avg.v[2] - DEFAULT_SENSITIVITY;
	}
	return 0;
}
#endif
/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int dmard11_open(struct inode *inode, struct file *file)
{
	file->private_data = dmard11_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int dmard11_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int dmard11_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
  //     unsigned long arg)
static int dmard11_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct dmard11_i2c_data *obj = (struct dmard11_i2c_data*)i2c_get_clientdata(client);	
	char strbuf[DMARD11_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
	int err = 0;
	int cali[3];
	//int intBuf[SENSOR_DATA_SIZE];
	//GSE_FUN(f);

	memset(strbuf, 0,sizeof(strbuf));
	
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
			dmard11_init_client(client, 0);			
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			DMARD11_ReadChipInfo(client, strbuf, DMARD11_BUFSIZE);
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
			
			DMARD11_ReadSensorData(client, strbuf, DMARD11_BUFSIZE);
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
			DMARD11_ReadRawData(client, strbuf);
			if(copy_to_user(data, &strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}
			break;	  

		case GSENSOR_IOCTL_SET_CALI:
			data = (void __user*)arg;
		//	err = DMARD11_ResetCalibration_2(client);  //dmt11
		//	enter_calibrate =0;
		   
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
				cali[DMARD11_AXIS_X] = sensor_data.x * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[DMARD11_AXIS_Y] = sensor_data.y * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				//cali[DMARD11_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;			 
				
				if(enter_calibrate ==0)
				{
					cali[DMARD11_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;	
				}else{
					cali[DMARD11_AXIS_Z] = (sensor_data.z - DEFAULT_SENSITIVITY*2) * obj->reso->sensitivity / GRAVITY_EARTH_1000;			//matt
					enter_calibrate =0;
				}
			#else
				cali[DMARD11_AXIS_X] = sensor_data.x;
				cali[DMARD11_AXIS_Y] = sensor_data.y;
			#ifdef USE_MTK_CALIBRATE
				cali[DMARD11_AXIS_Z] = sensor_data.z;			 
			#else				
				if(enter_calibrate ==0)
				{
					cali[DMARD11_AXIS_Z] = sensor_data.z; 
				}else{
					cali[DMARD11_AXIS_Z] = (sensor_data.z - DEFAULT_SENSITIVITY*2);			//matt
					enter_calibrate =0;
				}
			#endif
			#endif
				err = DMARD11_WriteCalibration(client, cali);			 
			}
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = DMARD11_ResetCalibration(client);
			enter_calibrate =0;  //dmt11
			break;

		case GSENSOR_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(err = DMARD11_ReadCalibration(client, cali))
			{
				break;
			}
		#if 0	
			sensor_data.x = cali[DMARD11_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.y = cali[DMARD11_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.z = cali[DMARD11_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		#else
			sensor_data.x = cali[DMARD11_AXIS_X];
			sensor_data.y = cali[DMARD11_AXIS_Y];
			sensor_data.z = cali[DMARD11_AXIS_Z];

		#endif
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}		
			break;
		#ifndef USE_MTK_CALIBRATE
		case SENSOR_CALIBRATION:
			err = D11_calibrate(client);
			GSE_LOG("Sensor_calibration:%d %d %d\n", s_dmt->offset.u.x, s_dmt->offset.u.y, s_dmt->offset.u.z);
			/* save file */
			D11_write_offset_to_file(client, OffsetFileName);
			
			/* return the offset */
			for(i = 0; i < SENSOR_DATA_SIZE; ++i)
				intBuf[i] = (int)obj->offset.v[i];

			copy_to_user((int *)arg, &intBuf, sizeof(intBuf));
			break;
		#endif
		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}


/*----------------------------------------------------------------------------*/
static struct file_operations dmard11_fops = {
	//.owner = THIS_MODULE,
	.open = dmard11_open,
	.release = dmard11_release,
	.unlocked_ioctl = dmard11_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice dmard11_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &dmard11_fops,
};
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int dmard11_suspend(struct i2c_client *client, pm_message_t msg) 
{
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);    
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
		if(err = DMARD11_SetPowerMode(obj->client, false))
		{
			GSE_ERR("write power control fail!!\n");
			return;
		}       
		DMARD11_power(obj->hw, 0);
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int dmard11_resume(struct i2c_client *client)
{
	struct dmard11_i2c_data *obj = i2c_get_clientdata(client);        
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	DMARD11_power(obj->hw, 1);
	if(err = dmard11_init_client(client, 0))
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
static void dmard11_early_suspend(struct early_suspend *h) 
{
	struct dmard11_i2c_data *obj = container_of(h, struct dmard11_i2c_data, early_drv);   
	int err;
	GSE_FUN();    

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	atomic_set(&obj->suspend, 1); 
	if(err = DMARD11_SetPowerMode(obj->client, false))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}

	sensor_power = false;
	
	DMARD11_power(obj->hw, 0);
}
/*----------------------------------------------------------------------------*/
static void dmard11_late_resume(struct early_suspend *h)
{
	struct dmard11_i2c_data *obj = container_of(h, struct dmard11_i2c_data, early_drv);         
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}

	DMARD11_power(obj->hw, 1);
#if 0	
	if(err = DMARD11_SetPowerMode(obj->client, power_status))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}	
#else	
	if(power_status)
	{
		//if(err = dmard11_init_client(obj->client, 0))
		if(err = DMARD11_CheckDeviceID(obj->client))
		{
			GSE_ERR("initialize client fail!!\n");
			return;        
		}
		sensor_power = power_status;
	}
#endif
	atomic_set(&obj->suspend, 0);    
}
/*----------------------------------------------------------------------------*/
#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
static int dmard11_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
	strcpy(info->type, DMARD11_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int dmard11_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct dmard11_i2c_data *obj;
#ifdef DMARD11_NEW_ARCH
	struct acc_control_path ctl_path = {0};
	struct acc_data_path data_path = {0};
#else
	struct hwmsen_object sobj = {0};
#endif
	int err = 0;
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	
	memset(obj, 0, sizeof(struct dmard11_i2c_data));

	obj->hw = get_cust_acc_hw();
#ifdef MANUAL_CHANGE_XYZ
    
	obj->cvt.sign[DMARD11_AXIS_X] = -1;  //set -1=> +X, set +1=> -X, 
	obj->cvt.sign[DMARD11_AXIS_Y] = -1;  //set -1=> +Y, set +1=> -Y, 
	obj->cvt.sign[DMARD11_AXIS_Z] = -1;  //set -1=> +Z, set +1=> -Z, 
	
	//XY no change: A
	//XY change YX: B
/******************A*********************/	
	obj->cvt.map[DMARD11_AXIS_X] = 0;  
    obj->cvt.map[DMARD11_AXIS_Y] = 1;  
/*-----------------B--------------------*/
	//obj->cvt.map[DMARD11_AXIS_X] = 1;  
    //obj->cvt.map[DMARD11_AXIS_Y] = 0;
/****************************************/	
	
	//Don't need change Z axis
	obj->cvt.map[DMARD11_AXIS_Z] = 2;  

#else
	if(err = hwmsen_get_convert(obj->hw->direction, &obj->cvt))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}
#endif

	obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
	
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);
	
#ifdef CONFIG_DMARD11_LOWPASS
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

	dmard11_i2c_client = new_client;	

	if(err = dmard11_init_client(new_client, 1))
	{
		goto exit_init_failed;
	}
	

	if(err = misc_register(&dmard11_device))
	{
		GSE_ERR("dmard11_device register failed\n");
		goto exit_misc_device_register_failed;
	}
#ifdef DMARD11_NEW_ARCH
	if(err = dmard11_create_attr(&(dmard11_init_info.platform_diver_addr->driver)))

#else
	if(err = dmard11_create_attr(&dmard11_gsensor_driver.driver))
#endif
	{
		GSE_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

#ifdef DMARD11_NEW_ARCH
	ctl_path.enable_nodata 			= dmard11_enable_nodata;
	ctl_path.open_report_data 		= dmard11_open_report_data;
	ctl_path.set_delay 				= dmard11_set_delay;
	ctl_path.is_report_input_direct = false;

	if((err=acc_register_control_path(&ctl_path)) < 0)
	{
		GSE_ERR("acc_register_control_path fail = %d\n", err);
		goto exit_create_attr_failed;
	}

	data_path.get_data 		= dmard11_get_data;
	data_path.vender_div 	= 1000;
	if((err=acc_register_data_path(&data_path)) < 0)
	{
		GSE_ERR("acc_register_data_path fail = %d\n", err);
		goto exit_create_attr_failed;
	}
#else
	sobj.self = obj;
    sobj.polling = 1;
    sobj.sensor_operate = gsensor_operate;
	if(err = hwmsen_attach(ID_ACCELEROMETER, &sobj))
	{
		GSE_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = dmard11_early_suspend,
	obj->early_drv.resume   = dmard11_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif 

#ifdef DMARD11_NEW_ARCH
	dmard11_init_flag = 0;	//probe finish
#endif

	GSE_LOG("%s: OK\n", __func__);    
	return 0;

	exit_create_attr_failed:
	misc_deregister(&dmard11_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);        
	return err;
}

/*----------------------------------------------------------------------------*/
static int dmard11_i2c_remove(struct i2c_client *client)
{
	int err = 0;	
#ifdef DMARD11_NEW_ARCH
	if(err = dmard11_delete_attr(&(dmard11_init_info.platform_diver_addr->driver)))

#else
	if(err = dmard11_delete_attr(&dmard11_gsensor_driver.driver))
#endif
	{
		GSE_ERR("dmard11_delete_attr fail: %d\n", err);
	}
	
	if(err = misc_deregister(&dmard11_device))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}
	
#ifdef DMARD11_NEW_ARCH
	dmard11_init_flag = -1;
#else
	err = hwmsen_detach(ID_ACCELEROMETER);
#endif	    

	dmard11_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
#ifndef DMARD11_NEW_ARCH
static int dmard11_probe(struct platform_device *pdev) 
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();

	DMARD11_power(hw, 1);
	//dmard11_force[0] = hw->i2c_num;
	if(i2c_add_driver(&dmard11_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	return 0;
}
#endif
#ifndef USE_MTK_CALIBRATE
void D11_write_offset_to_file(struct i2c_client *client , char * offset)
{
	struct dmard11_i2c_data *obj = (struct dmard11_i2c_data*)i2c_get_clientdata(client);
	char data[18];
	mm_segment_t orgfs;
	struct file *fp;

	sprintf(data,"%5d %5d %5d", obj->offset.v[0], obj->offset.v[1], obj->offset.v[2]);
	fp = filp_open(offset, O_RDWR | O_CREAT, 0666);
	if(IS_ERR(fp))
		GSE_ERR("filp_open %s error!!.\n",offset);
	else
	{
		orgfs = get_fs();
		/* Set segment descriptor associated to kernel space */
		set_fs(KERNEL_DS);
		GSE_LOG("filp_open %s SUCCESS!!.\n",offset);
		fp->f_op->write(fp,data,18, &fp->f_pos);
 		filp_close(fp,NULL);
	}
	set_fs(orgfs);
}

void D11_read_offset_from_file(struct i2c_client *client)
{
	struct dmard11_i2c_data *obj = (struct dmard11_i2c_data*)i2c_get_clientdata(client);
	unsigned int orgfs;
	char data[18];
	struct file *fp , *fs;/* *fp open gsensor_offset.txt , *fs open dmt_sensor.txt */
	s16 ux,uy,uz;
	orgfs = get_fs();
	/* Set segment descriptor associated to kernel space */
	set_fs(KERNEL_DS);

	fp = filp_open(OffsetFileName, O_RDWR , 0);
	if(IS_ERR(fp))
	{
		GSE_ERR("Sorry,filp_open ERROR !\n");
		D11_write_offset_to_file(client , OffsetFileName);
#ifdef AUTO_CALIBRATION
/* get acceleration average reading */
			D11_calibrate(client);
#endif
#ifdef DMT_BROADCAST_APK_ENABLE
		D11_write_offset_to_file(client , DmtXXFileName);
#endif
	}
	else{
		GSE_LOG("filp_open %s SUCCESS!!.\n",OffsetFileName);
		fp->f_op->read( fp, data, 18, &fp->f_pos); // read gsensor_offset.txt
		GSE_LOG("filp_read result %s\n",data);
		sscanf(data,"%d %d %d",&ux,&uy,&uz);
		if(ux == 0 && uy == 0 && uz == 0)
		{
			filp_close(fp,NULL);// close gsensor_offset.txt
			fs = filp_open(DmtXXFileName, O_RDWR , 0); // open dmt_sensor.txt
			fs->f_op->read( fs, data, 18, &fs->f_pos); // read dmt_sensor.txt
			GSE_LOG("filp_read result %s\n", data);
			GSE_FUN();
			sscanf(data,"%d %d %d",&ux,&uy,&uz);
			obj->offset.u.x=ux;
			obj->offset.u.y=uy;
			obj->offset.u.z=uz;
			GSE_LOG("ux/uy/uz = %d/%d/%d\n",ux,uy,uz);
			filp_close(fs,NULL);
		}	
		else
		{

			obj->offset.u.x=ux;
			obj->offset.u.y=uy;
			obj->offset.u.z=uz;
			GSE_LOG("ux/uy/uz = %d/%d/%d\n",ux,uy,uz);
			filp_close(fp,NULL);
		}
	}
	set_fs(orgfs);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef DMARD11_NEW_ARCH
static int dmard11_local_init(void)
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();

	DMARD11_power(hw, 1);
	//dmard11_force[0] = hw->i2c_num;
	if(i2c_add_driver(&dmard11_i2c_driver))
	{
		DMARD11_power(hw, 0);
		GSE_ERR("add driver error\n");
		return -1;
	}
	if(-1 == dmard11_init_flag)
	{
		DMARD11_power(hw, 0);
		GSE_ERR("local init driver error\n");
		return -1;
	}
	return 0;
}
static int dmard11_local_uninit(void)
{
	struct acc_hw *hw = get_cust_acc_hw();
	
	GSE_FUN();	  
	DMARD11_power(hw, 0);	 
	i2c_del_driver(&dmard11_i2c_driver);

	return 0;
}

#else
static int dmard11_remove(struct platform_device *pdev)
{
    struct acc_hw *hw = get_cust_acc_hw();

    GSE_FUN();    
    DMARD11_power(hw, 0);    
    i2c_del_driver(&dmard11_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_OF
static const struct of_device_id gsensor_of_match[] = {
	{ .compatible = "mediatek,gsensor", },
	{},
};
#endif

static struct platform_driver dmard11_gsensor_driver = {
	.probe      = dmard11_probe,
	.remove     = dmard11_remove,    
	.driver     = {
		.name  = "gsensor",
		.owner = THIS_MODULE,
		#ifdef CONFIG_OF
		.of_match_table = gsensor_of_match,
		#endif

	}
};
#endif
/*----------------------------------------------------------------------------*/
static int __init dmard11_init(void)
{
	GSE_FUN();
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_dmard11, 1);
#ifndef DMARD11_NEW_ARCH
	if(platform_driver_register(&dmard11_gsensor_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
#else
	if(acc_driver_add(&dmard11_init_info))
	{
		GSE_ERR("failed to add acc driver");
		return -ENODEV;
	}

#endif
	return 0;    
}
/*----------------------------------------------------------------------------*/
static void __exit dmard11_exit(void)
{
	GSE_FUN();
#ifndef DMARD11_NEW_ARCH
	platform_driver_unregister(&dmard11_gsensor_driver);
#endif
}
/*----------------------------------------------------------------------------*/
module_init(dmard11_init);
module_exit(dmard11_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DMARD11 I2C driver");
MODULE_AUTHOR("Zhilin.Chen@mediatek.com");
