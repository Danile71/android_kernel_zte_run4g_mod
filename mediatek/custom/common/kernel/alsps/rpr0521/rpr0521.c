/* 
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
/*
 * Definitions for rpr521 als/ps sensor chip.
 */

  /******************************************************************************
*Revision History
*Ver 0.9		Grace	Feb 2014		Make a new file based on original files.
*******************************************************************************/

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
//#include <mach/mt_gpio.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "rpr0521.h"

#define RPR0521_NEW_ARCH //for compatible
#ifdef RPR0521_NEW_ARCH
#include <alsps.h>

#endif

/******************************************************************************
 * configuration
*******************************************************************************/
/*----------------------------------------------------------------------------*/

#define RPR521_DEV_NAME     "rpr521"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define DEBUG

#if defined(DEBUG)
#define APS_FUN(f)               	 printk(APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(APS_TAG fmt, ##args)  
#else
#define APS_FUN(f)           
#define APS_ERR(fmt, args...)
#define APS_LOG(fmt, args...)
#define APS_DBG(fmt, args...)
#endif

/******************************************************************************
 * extern functions
*******************************************************************************/
/*for interrup work mode support */
#ifdef CUST_EINT_ALS_TYPE
extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);

#else
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif
/*----------------------------------------------------------------------------*/
static struct i2c_client *rpr521_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id rpr521_i2c_id[] = {{RPR521_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_RPR521={ I2C_BOARD_INFO(RPR521_DEV_NAME, (0X70>>1))};

/*----------------------------------------------------------------------------*/
static int rpr521_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int rpr521_i2c_remove(struct i2c_client *client);
static int rpr521_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
static int rpr521_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int rpr521_i2c_resume(struct i2c_client *client);
long rpr521_read_ps(struct i2c_client *client, u16 *data);
int rpr521_read_als(struct i2c_client *client, u16 *data);
static int rpr521_set_ps_threshold(struct i2c_client *client, kal_uint16 ps_tl,kal_uint16 ps_th);


#ifdef RPR0521_NEW_ARCH
static int rpr0521_local_init(void);
static int rpr0521_local_uninit(void);
static int rpr0521_als_open_report_data(int open);
static int rpr0521_als_enable_nodata(int en);
static int rpr0521_als_set_delay(u64 ns);
static int rpr0521_als_get_data(int* value, int* status);
static int rpr0521_ps_open_report_data(int open);
static int rpr0521_ps_enable_nodata(int en);
static int rpr0521_ps_set_delay(u64 ns);
static int rpr0521_ps_get_data(int* value, int* status);
#endif

static struct rpr521_priv *g_rpr521_ptr = NULL;
u16 prev_als_value = 0;

typedef struct {
    unsigned long long data;
    unsigned long long data0;
    unsigned long long data1;
    unsigned char      gain_data0;
    unsigned char      gain_data1;
    unsigned long      dev_unit;
    //unsigned char      als_time;
    unsigned short     als_time; //grace modify in 2014.4.21
    unsigned short     als_data0;
    unsigned short     als_data1;
} CALC_DATA;

typedef struct {
    unsigned long positive;
    unsigned long decimal;
} CALC_ANS;

/*----------------------------------------------------------------------------*/
typedef enum {
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;

static enum
{
	ALSPS_DEBUG 	= 0x01, 
};

/*----------------------------------------------------------------------------*/

struct rpr521_i2c_addr {    /*define a series of i2c slave address*/
    u8  write_addr;  
    u8  ps_thd;     /*PS INT threshold*/
};


struct rpr521_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
    struct work_struct  eint_work;

    /*i2c address group*/
    struct rpr521_i2c_addr  addr;
    
    /*misc*/
    u16		    als_modulus;
    atomic_t    i2c_retry;
    atomic_t    als_suspend;
    atomic_t    als_debounce;   /*debounce time after enabling als*/
    atomic_t    als_deb_on;     /*indicates if the debounce is on*/
    atomic_t    als_deb_end;    /*the jiffies representing the end of debounce*/
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;
	atomic_t	trace;		//add by wxj for debug

    /*data*/
    u16         als;
    u16          ps;
    u8          _align;
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL-1];
    u32         als_value[C_CUST_ALS_LEVEL];

    atomic_t    als_cmd_val;    /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_cmd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_thd_val;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/
    ulong       pending_intr;   /*pending interrupt*/
     u16              ps_th_h;
     u16              ps_th_l;
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};

#ifdef RPR0521_NEW_ARCH
static int rpr0521_init_flag = -1;	//-1:fail  0:ok
static struct alsps_init_info rpr0521_init_info = 
{
	.init 		= rpr0521_local_init,
	.uninit 	= rpr0521_local_uninit,
	.name 		= RPR521_DEV_NAME,
};
#endif

/*----------------------------------------------------------------------------*/
static struct i2c_driver rpr521_i2c_driver = {	
	.probe      = rpr521_i2c_probe,
	.remove     = rpr521_i2c_remove,
	.detect     = rpr521_i2c_detect,
	.suspend    = rpr521_i2c_suspend,
	.resume     = rpr521_i2c_resume,
	.id_table   = rpr521_i2c_id,
	.driver = {
		.owner          = THIS_MODULE,
		.name           = RPR521_DEV_NAME,
	},
};


static struct rpr521_priv *rpr521_obj = NULL;
static struct platform_driver rpr521_alsps_driver;

/*----------------------------------------------------------------------------*/
int rpr521_get_addr(struct alsps_hw *hw, struct rpr521_i2c_addr *addr)
{
	if(!hw || !addr)
	{
		return -EFAULT;
	}
	addr->write_addr= hw->i2c_addr[0];
	return 0;
}
/*----------------------------------------------------------------------------*/
static void rpr521_power(struct alsps_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	//APS_LOG("power %s\n", on ? "on" : "off");

	if(hw->power_id != POWER_NONE_MACRO)
	{
		if(power_on == on)
		{
			APS_LOG("ignore power control: %d\n", on);
		}
		else if(on)
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "rpr521")) 
			{
				APS_ERR("power on fails!!\n");
			}
		}
		else
		{
			if(!hwPowerDown(hw->power_id, "rpr521")) 
			{
				APS_ERR("power off fail!!\n");   
			}
		}
	}
	power_on = on;
}
/*----------------------------------------------------------------------------*/
static long rpr521_enable_als(struct i2c_client *client, int enable)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];	  
	int res = 0;
	u8 buffer[1];
	u8 reg_value[1];	
	u8 power_state, power_set;
	uint32_t testbit_ALS;
	PWR_ST  pwr_st;

	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
 		APS_DBG("rpr521_enable_als enable:%d \n",enable);
	}
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
		
	testbit_ALS = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);	
		
	if(enable)
	{
		//if(testbit_ALS == CTL_STANDBY)
		if(1)
		{
			buffer[0]= REG_MODECONTROL;
			res = i2c_master_send(client, buffer, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			power_set = reg_value[0] | 0x80;
			
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set;  
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}	
		}
		atomic_set(&obj->als_deb_on, 1);
		atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)/(1000/HZ));
		
#ifdef _TEST_
		databuf[0]  = REG_SYSTEMCONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: ALS_ENABLE REG_SYSTEMCONTROL    reg_value = %x \n", reg_value[0]);
	
		databuf[0]  = REG_MODECONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: ALS_ENABLE REG_MODECONTROL    reg_value = %x \n", reg_value[0]);
		
		databuf[0]  = REG_INTERRUPT;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: ALS_ENABLE REG_INTERRUPT    reg_value = %x \n", reg_value[0]);
#endif
				
	}
	else
	{
		//if(testbit_ALS == CTL_STANDALONE)
		if(1)
		{

			buffer[0]= REG_MODECONTROL;
			res = i2c_master_send(client, buffer, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			power_set = reg_value[0] & (~0x80);
			
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set;   

			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
		atomic_set(&obj->als_deb_on, 0);
		
#ifdef _TEST_
		databuf[0]  = REG_SYSTEMCONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: ALS_OFF REG_SYSTEMCONTROL    reg_value = %x \n", reg_value[0]);
	
		databuf[0]  = REG_MODECONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: ALS_OFF REG_MODECONTROL    reg_value = %x \n", reg_value[0]);
		
		databuf[0]  = REG_INTERRUPT;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: ALS_OFF REG_INTERRUPT    reg_value = %x \n", reg_value[0]);
#endif
	}
	return 0;
		
	EXIT_ERR:
		APS_ERR("rpr521_enable_als fail\n");
		return res;
}


/*----------------------------------------------------------------------------*/
static long rpr521_enable_ps(struct i2c_client *client, int enable)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
	u8 buffer[1];
	u8 reg_value[1];
	u8 power_state, power_set; 	
	PWR_ST  pwr_st;	
	uint32_t testbit_PS;
	
	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
 		APS_DBG("rpr521_enable_ps enable:%d \n",enable);
	}
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	//testbit_PS = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
 
	if(enable)
	{

		//if(testbit_PS == CTL_STANDBY)
		if(1)
		{
			buffer[0]= REG_MODECONTROL;
			res = i2c_master_send(client, buffer, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			power_set = reg_value[0] | 0x40;
			
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set;   
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}	
		}
		
		if(0 == obj->hw->polling_mode_ps)
		{
			
			databuf[0] = REG_PSTL_LSB;	
			if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
			{
				databuf[1] = (u8)(obj->ps_th_l & 0x00FF);
			}
			else
			{
				databuf[1] = (u8)(PS_ALS_SET_PS_TL & 0x00FF);
			}
					
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
						
			databuf[0] = REG_PSTL_MBS;
			if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
			{
				databuf[1] = (u8)((obj->ps_th_l & 0xFF00) >> 8);
			}
			else
			{
				databuf[1] = (u8)((PS_ALS_SET_PS_TL & 0xFF00) >> 8);
			}
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
						
			databuf[0] = REG_PSTH_LSB;	
			if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
			{
				databuf[1] = (u8)( obj->ps_th_h & 0x00FF);
			}
			else
			{
				databuf[1] = (u8)( PS_ALS_SET_PS_TH & 0x00FF);
			}
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
						
			databuf[0] = REG_PSTH_MBS;	
			if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
			{
				databuf[1] = (u8)(( obj->ps_th_h & 0xFF00) >> 8);
			}
			else
			{
				databuf[1] = (u8)(( PS_ALS_SET_PS_TH & 0xFF00) >> 8);
			}
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}


			databuf[0] = REG_INTERRUPT;
			databuf[1] = PS_ALS_SET_INTR|MODE_PROXIMITY; //caosq  reg_value
 
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
#ifdef CUST_EINT_ALS_TYPE
			mt_eint_unmask(CUST_EINT_ALS_NUM);
#else
			mt65xx_eint_unmask(CUST_EINT_ALS_NUM); 
#endif
		}
		else
		{
			atomic_set(&obj->ps_deb_on, 1);
			atomic_set(&obj->ps_deb_end, jiffies+atomic_read(&obj->ps_debounce)/(1000/HZ));
		}
		
#ifdef _TEST_
		databuf[0]  = REG_SYSTEMCONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: PS_ENABLE REG_SYSTEMCONTROL    reg_value = %x \n", reg_value[0]);
	
		databuf[0]  = REG_MODECONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: PS_ENABLE REG_MODECONTROL    reg_value = %x \n", reg_value[0]);
		
		databuf[0]  = REG_INTERRUPT;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: PS_ENABLE REG_INTERRUPT    reg_value = %x \n", reg_value[0]);
#endif
			
	}
	else
	{

		//if(testbit_PS == CTL_STANDALONE)
		if(1)
		{

			buffer[0]= REG_MODECONTROL;
			res = i2c_master_send(client, buffer, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			power_set = reg_value[0] & (~0x40);
			
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set;   

			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
		atomic_set(&obj->ps_deb_on, 0);
		
#ifdef _TEST_
		databuf[0]  = REG_SYSTEMCONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: PS_OFF REG_SYSTEMCONTROL    reg_value = %x \n", reg_value[0]);
	
		databuf[0]  = REG_MODECONTROL;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: PS_OFF REG_MODECONTROL    reg_value = %x \n", reg_value[0]);
		
		databuf[0]  = REG_INTERRUPT;
		res = i2c_master_send(client, databuf, 0x1);

		if(res <= 0)
		{
			return -1;
		}
		res = i2c_master_recv(client, reg_value, 0x1);

		if(res <= 0)
		{
			return -1;
		}
	
		printk(">>>>>>>rpr521: PS_OFF REG_INTERRUPT    reg_value = %x \n", reg_value[0]);
#endif

		/*for interrup work mode support */
		if(0 == obj->hw->polling_mode_ps)
		{
			cancel_work_sync(&obj->eint_work);
#ifdef CUST_EINT_ALS_TYPE
			mt_eint_mask(CUST_EINT_ALS_NUM);
#else
			mt65xx_eint_mask(CUST_EINT_ALS_NUM);		
#endif

			
		}
	}

	
	return 0;
	
EXIT_ERR:
	APS_ERR("rpr521_enable_ps fail\n");
	return res;
	
}


/*----------------------------------------------------------------------------*/
/*for interrup work mode support*/
static int rpr521_check_and_clear_intr(struct i2c_client *client) 
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);
	int res;
	u8 buffer[2], int_status[1];
	
	buffer[0] = REG_INTERRUPT;
	res = i2c_master_send(client, buffer, 0x1);

	if(res <= 0)
	{
		return -1;
	}
	res = i2c_master_recv(client, int_status, 0x1);

	if(res <= 0)
	{
		return -1;
	}

	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
		APS_DBG(">>>>>>>rpr521: 0x4A    int_status = %x \n", int_status[0]);
	}
	return int_status[0];
}
/*----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
void rpr521_eint_func(void)
{
	//APS_FUN();
	struct rpr521_priv *obj = g_rpr521_ptr;
	if(!obj)
	{
		return;
	}
	
	schedule_work(&obj->eint_work);
}

/*----------------------------------------------------------------------------*/
/*for interrup work mode support*/
int rpr521_setup_eint(struct i2c_client *client)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);        

	g_rpr521_ptr = obj;
	
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);
	
#if CUST_EINT_ALS_TYPE
	mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, rpr521_eint_func, 0);
	mt_eint_mask(CUST_EINT_ALS_NUM);
#else
	mt65xx_eint_set_sens(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
	mt65xx_eint_set_polarity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY);
	mt65xx_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_POLARITY, rpr521_eint_func, 0);

	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);

#endif
    return 0;
}

#ifdef _AUTO_THRESHOLD_CHANGE_
static int rpr521_calibrate(struct i2c_client *client);
#endif

static int rpr521_check_id(struct i2c_client *client)
{
	u8 databuf[2] = {0};
	int res = 0;

	databuf[0] = 0x92;	//manufact_id
	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, databuf, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	if(databuf[0] != 0xe0)
	{
		APS_DBG("check manufact_id failed!\n");
		goto EXIT_ERR;
	}

	databuf[0] = REG_SYSTEMCONTROL;	//part_id
	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, databuf, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	if((databuf[0]&0x3f) != 0x0a)
	{
		APS_DBG("check part_id failed!\n");
		goto EXIT_ERR;
	}
	APS_DBG("check id successfully!\n");
	return 0;
	
EXIT_ERR:
	return -1;
}

static int rpr521_init_client(struct i2c_client *client)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
	u8 data[1];
	
	databuf[0] = REG_SYSTEMCONTROL;    
	databuf[1] = REG_SW_RESET | REG_INT_RESET;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return rpr521_ERR_I2C;
	}	
	
	res = rpr521_check_id(client);
	if(res < 0)
	{
		goto EXIT_ERR;
	}

	
	databuf[0] = REG_MODECONTROL;    
	databuf[1] = PS_ALS_SET_MODE_CONTROL|PWRON_PS_ALS;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return rpr521_ERR_I2C;
	}
	
	databuf[0] = REG_ALSPSCONTROL;    
	databuf[1] = PS_ALS_SET_ALSPS_CONTROL;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return rpr521_ERR_I2C;
	}

	databuf[0] = REG_PERSISTENCE;    
	databuf[1] = PS_ALS_SET_INTR_PERSIST;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return rpr521_ERR_I2C;
	}
	
	/*for interrup work mode support */
	if(0 == obj->hw->polling_mode_ps)
	{
		databuf[0] = REG_PSTL_LSB;	
		if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
		{
			databuf[1] = (u8)(obj->ps_th_l & 0x00FF);
		}
		else
		{
			databuf[1] = (u8)(PS_ALS_SET_PS_TL & 0x00FF);
		}
		
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return rpr521_ERR_I2C;
		}
			
		databuf[0] = REG_PSTL_MBS;
		if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
		{
			databuf[1] = (u8)((obj->ps_th_l & 0xFF00) >> 8);
		}
		else
		{
			databuf[1] = (u8)((PS_ALS_SET_PS_TL & 0xFF00) >> 8);
		}
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return rpr521_ERR_I2C;
		}
			
		databuf[0] = REG_PSTH_LSB;	
		if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
		{
			databuf[1] = (u8)( obj->ps_th_h & 0x00FF);
		}
		else
		{
			databuf[1] = (u8)( PS_ALS_SET_PS_TH & 0x00FF);
		}
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return rpr521_ERR_I2C;
		}
			
		databuf[0] = REG_PSTH_MBS;	
		if((obj->ps_th_l > 0) && (obj->ps_th_h > 0))
		{
			databuf[1] = (u8)(( obj->ps_th_h & 0xFF00) >> 8);
		}
		else
		{
			databuf[1] = (u8)(( PS_ALS_SET_PS_TH & 0xFF00) >> 8);
		}
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return rpr521_ERR_I2C;
		}

		databuf[0] = REG_INTERRUPT;
		databuf[1] = PS_ALS_SET_INTR | MODE_PROXIMITY;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return rpr521_ERR_I2C;
		}

	}


	databuf[0] = REG_ALSDATA0TH_LSB;    
	databuf[1] = PS_ALS_SET_ALS_TH & 0x00FF ;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return rpr521_ERR_I2C;
	}

      
	databuf[0] = REG_ALSDATA0TH_MBS;    
	databuf[1] = (PS_ALS_SET_ALS_TH& 0xFF00) >> 8;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return rpr521_ERR_I2C;
	}

	/*for interrup work mode support */
	if(res = rpr521_setup_eint(client))
	{
		APS_ERR("setup eint: %d\n", res);
		return res;
	}

#ifdef _TEST_
	databuf[0]  = REG_SYSTEMCONTROL;
	res = i2c_master_send(client, databuf, 0x1);

	if(res <= 0)
	{
		return -1;
	}
	res = i2c_master_recv(client, data, 0x1);

	if(res <= 0)
	{
		return -1;
	}
	
	printk(">>>>>>>rpr521: REG_SYSTEMCONTROL    data = %x \n", data[0]);
	
	databuf[0]  = REG_MODECONTROL;
	res = i2c_master_send(client, databuf, 0x1);

	if(res <= 0)
	{
		return -1;
	}
	res = i2c_master_recv(client, data, 0x1);

	if(res <= 0)
	{
		return -1;
	}
	
	printk(">>>>>>>rpr521: REG_MODECONTROL    data = %x \n", data[0]);
#endif

#ifdef _AUTO_THRESHOLD_CHANGE_		
	rpr521_calibrate(client);//zzf add
#endif

	return 0;

EXIT_ERR:
	APS_ERR("init dev: %d\n", res);
	return res;
}

/******************************************************************************
 * NAME       : ps_als_driver_reset
 * FUNCTION   : reset rpr521 register
 * REMARKS    :
 *****************************************************************************/
static int rpr521_ps_als_driver_reset(struct i2c_client *client)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
   
	databuf[0] = REG_SYSTEMCONTROL;    
	databuf[1] = REG_SW_RESET | REG_INT_RESET;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return rpr521_ERR_I2C;
	}	

   	return 0;	 	
	
EXIT_ERR:

	APS_ERR("rpr521 reset fail\n");
	return res;
}


/******************************************************************************
 * NAME       : long_long_divider
 * FUNCTION   : calc divider of unsigned long long int or unsgined long
 * REMARKS    :
 *****************************************************************************/
static void long_long_divider(unsigned long long data, unsigned long base_divier, unsigned long *answer, unsigned long long *overplus)
{
    volatile unsigned long long divier;
    volatile unsigned long      unit_sft;

    if ((long long)data < 0)	// . If data MSB is 1, it may go to endless loop. 
    	{
	*answer = 0;
	*overplus = 0;
	return;		//Theorically, if data is negative, the program will have been returned CALC_ERROR earlier. 
    	}
    divier = base_divier;
    if (data > MASK_LONG) {
        unit_sft = 0;
        while (data > divier) {
            unit_sft++;
            divier = divier << 1;
        }
        while (data > base_divier) {
            if (data > divier) {
                *answer += 1 << unit_sft;
                data    -= divier;
            }
            unit_sft--;
            divier = divier >> 1;
        }
        *overplus = data;
    } else {
        *answer = (unsigned long)(data & MASK_LONG) / base_divier;
        /* calculate over plus and shift 16bit */
        *overplus = (unsigned long long)(data - (*answer * base_divier));
    }
}

/******************************************************************************
 * NAME       : calc_rohm_als_data
 * FUNCTION   : calculate illuminance data for rpr521
 * REMARKS    : final_data is 1000 times, which is defined as CUT_UNIT, of the actual lux value
 *****************************************************************************/
static int calc_rohm_als_data(READ_DATA_BUF data, DEVICE_VAL dev_val)
{
#define DECIMAL_BIT      (15)
#define JUDGE_FIXED_COEF (100)
#define MAX_OUTRANGE     (11357)
#define MAXRANGE_NMODE   (0xFFFF)
#define MAXSET_CASE      (4)
#define CUT_UNIT         10

	int                final_data, mid_data;
	CALC_DATA          calc_data;
	CALC_ANS           calc_ans;
	unsigned long      calc_judge;
	unsigned char      set_case;
	unsigned long      div_answer;
	unsigned long long div_overplus;
	unsigned long long overplus;
	unsigned long      max_range;

	/* set the value of measured als data */
	calc_data.als_data0  = data.als_data0;
	calc_data.als_data1  = data.als_data1;
	calc_data.gain_data0 = GAIN_TABLE[dev_val.gain].DATA0;

	/* set max range */
	if (calc_data.gain_data0 == 0) 
	{
		/* issue error value when gain is 0 */
		return (CALC_ERROR);
	}
	else
	{
		max_range = MAX_OUTRANGE / calc_data.gain_data0;
	}
	
	/* calculate data */
	if (calc_data.als_data0 == MAXRANGE_NMODE) 
	{
		calc_ans.positive = max_range;
		calc_ans.decimal  = 0;
	} 
	else 
	{
		/* get the value which is measured from power table */
		calc_data.als_time = MCTL_TABLE[dev_val.time].ALS;
		if (calc_data.als_time == 0) 
		{
			/* issue error value when time is 0 */
			return (CALC_ERROR);
		}

		calc_judge = calc_data.als_data1 * JUDGE_FIXED_COEF;
		if (calc_judge < (calc_data.als_data0 * judge_coefficient[0])) 
		{
			set_case = 0;
		} 
		else if (calc_judge < (data.als_data0 * judge_coefficient[1]))
		{
			set_case = 1;
		} 
		else if (calc_judge < (data.als_data0 * judge_coefficient[2])) 
		{
			set_case = 2;
		}
		else if (calc_judge < (data.als_data0 * judge_coefficient[3])) 
		{
			 set_case = 3;
		} 
		else
		{
			set_case = MAXSET_CASE;
		}
		calc_ans.positive = 0;
		if (set_case >= MAXSET_CASE) 
		{
			calc_ans.decimal = 0;	//which means that lux output is 0
		}
		else
		{
			calc_data.gain_data1 = GAIN_TABLE[dev_val.gain].DATA1;
			if (calc_data.gain_data1 == 0) 
			{
				/* issue error value when gain is 0 */
				return (CALC_ERROR);
			}
			calc_data.data0      = (unsigned long long )(data0_coefficient[set_case] * calc_data.als_data0) * calc_data.gain_data1;
			calc_data.data1      = (unsigned long long )(data1_coefficient[set_case] * calc_data.als_data1) * calc_data.gain_data0;
			if (calc_data.data0 < calc_data.data1) 
			{
				/* issue error value when data is negtive */
				return (CALC_ERROR);
			}
			calc_data.data       = (calc_data.data0 - calc_data.data1);
			calc_data.dev_unit   = calc_data.gain_data0 * calc_data.gain_data1 * calc_data.als_time * 10;
			if (calc_data.dev_unit == 0) 
			{
				/* issue error value when dev_unit is 0 */
				return (CALC_ERROR);
			}

			/* calculate a positive number */
			div_answer   = 0;
			div_overplus = 0;
			long_long_divider(calc_data.data, calc_data.dev_unit, &div_answer, &div_overplus);
			calc_ans.positive = div_answer;
			/* calculate a decimal number */
			calc_ans.decimal = 0;
			overplus         = div_overplus;
			if (calc_ans.positive < max_range)
			{
				if (overplus != 0)
				{
					overplus     = overplus << DECIMAL_BIT;
					div_answer   = 0;
					div_overplus = 0;
					long_long_divider(overplus, calc_data.dev_unit, &div_answer, &div_overplus);
					calc_ans.decimal = div_answer;
				}
			}
			else
			{
				calc_ans.positive = max_range;
			}
		}
	}
	
	mid_data = (calc_ans.positive << DECIMAL_BIT) + calc_ans.decimal;
	final_data = calc_ans.positive * CUT_UNIT + ((calc_ans.decimal * CUT_UNIT) >> DECIMAL_BIT);
					
	return (final_data);

#undef CUT_UNIT
#undef DECIMAL_BIT
#undef JUDGE_FIXED_COEF
#undef MAX_OUTRANGE
#undef MAXRANGE_NMODE
#undef MAXSET_CASE
}


/******************************************************************************
 * NAME       : get_from_device
 * FUNCTION   : periodically reads the data from sensor(thread of work)
 * REMARKS    :
 *****************************************************************************/
static int get_from_device(DEVICE_VAL *dev_val, struct i2c_client *client)
{
#define LEDBIT_MASK   (3)
#define GAIN_VAL_MASK (0xF)

#if 0
	struct PS_ALS_DATA *obj = i2c_get_clientdata(client);	 
	u8 buffer[1];
	int res = 0;
    	unsigned char alsps_ctl[1], read_time[1];

   	 /* initalize the returning value */
    	dev_val->time        = 6;
    	dev_val->gain        = (PS_ALS_SET_ALSPS_CONTROL >> 2) & GAIN_VAL_MASK;
    	dev_val->led_current = PS_ALS_SET_ALSPS_CONTROL & LEDBIT_MASK;

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	buffer[0]=REG_MODECONTROL;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, read_time, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	dev_val->time = read_time[0] & 0xF;

	buffer[0]=REG_ALSPSCONTROL;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, alsps_ctl, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

    	dev_val->led_current = alsps_ctl[0] & LEDBIT_MASK;
    	dev_val->gain        = (alsps_ctl[0] >> 2) & GAIN_VAL_MASK;

#else
    	dev_val->time        = 6;
    	dev_val->gain        = (PS_ALS_SET_ALSPS_CONTROL >> 2) & GAIN_VAL_MASK;
    	dev_val->led_current = PS_ALS_SET_ALSPS_CONTROL & LEDBIT_MASK;
#endif

    return (0);
		
EXIT_ERR:
	APS_ERR("rpr521_read_ps fail\n");

#undef LEDBIT_MASK
#undef GAIN_VAL_MASK

}

/************************************************************
 *                      sysfs interface                    					      *
 ***********************************************************/

/*----------------------------------------------------------------------------*/


static ssize_t show_als_value(struct device_driver *ddri, char *buf)
{
  struct i2c_client *client = rpr521_obj->client;
	u16 als;
	int res;

	if (NULL == client)
	{
		APS_ERR("i2c client is NULL!!!\n");
		return 0;
	}

	rpr521_read_als(client, &als);

	res = snprintf(buf, PAGE_SIZE, "%d\n", als);

	return res;
}

static ssize_t show_ps_value(struct device_driver *ddri, char *buf)
{
  struct i2c_client *client = rpr521_obj->client;
	u16 ps;
	int res;

	if (NULL == client)
	{
		APS_ERR("i2c client is NULL!!!\n");
		return 0;
	}

	rpr521_read_ps(client, &ps);

	res = snprintf(buf, PAGE_SIZE, "%d\n", ps);

	return res;
}

static ssize_t show_registers(struct device_driver *ddri, char *buf)
{
    struct i2c_client *client = rpr521_obj->client;
	int res = 0;
	int len = 0;
	u8 buffer[1];
	u8 reg_val;
	int i;

	if (NULL == client)
	{
		APS_ERR("i2c client is NULL!!!\n");
		return 0;
	}

	for (i = REG_SYSTEMCONTROL; i < REG_ALSDATA0TL_MBS + 1; i++)
	{
		buffer[0]= i;
		i2c_master_send(client, buffer, 0x1);
		i2c_master_recv(client, &reg_val, 0x1);
		
		len += snprintf(buf + len, PAGE_SIZE - len, "[0x%02x]:0x%02x", i, reg_val);
		if ((i + 1) % 4 == 0 || i == REG_ALSDATA0TL_MBS)
			len += snprintf(buf + len, PAGE_SIZE - len, "\n");
		else
			len += snprintf(buf + len, PAGE_SIZE - len, ", ");
	}
	
	return len;    
}

static ssize_t store_registers(struct device_driver *ddri, char *buf, size_t count)
{
	struct i2c_client *client = rpr521_obj->client;
	u8 addr;
	u8 val;
	int ret;
	u8 buffer[2];
           
	ret = sscanf(buf, "%x %x", &addr, &val);

	if (addr < REG_SYSTEMCONTROL || addr > REG_ALSDATA0TL_MBS) {
		APS_ERR("addr 0x%x is out of range!!!\n", addr);
		return -1;
	}

	buffer[0] = addr;
	buffer[1] = val;
	ret = i2c_master_send(client, buffer, 0x2);
	if (ret < 0) {
		APS_ERR("set addr 0x%x, value 0x%x set failed.\n", addr, val);
		return -1;
	}

    return count;
}
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	struct rpr521_priv *obj = rpr521_obj;

	if(obj == NULL)
	{
		APS_ERR("obj is null!\n");
		return 0;
	}

	return sprintf(buf, "0x%04X\n",atomic_read(&obj->trace));
}
static ssize_t store_trace_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct rpr521_priv *obj = rpr521_obj;
	int res = 0;
	int trace = 0;
	
	if(obj == NULL)
	{
		APS_ERR("obj is null!\n");
		return -1;
	}

	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}
	else 
	{
		APS_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	return count;
}
static ssize_t show_ps_threshould_value(struct device_driver *ddri, char * buf)
{
	struct rpr521_priv *obj = rpr521_obj;
	int res = 0;
	
	if(obj == NULL)
	{
		APS_ERR("obj is null!\n");
		return -1;
	}
	return sprintf(buf, "ps_th_l = %d,ps_th_h = %d\n", obj->ps_th_l, obj->ps_th_h);
}
static ssize_t store_ps_threshould_value(struct device_driver *ddri, const char * buf, size_t count)
{
	struct rpr521_priv *obj = rpr521_obj;
	int res = 0;
	unsigned int ps_h = 0;
	unsigned int ps_l = 0;
	
	if(obj == NULL)
	{
		APS_ERR("obj is null!\n");
		return -1;
	}
	if(2 == sscanf(buf, "%d %d", &ps_l, &ps_h))
	{
		if((ps_l>0) && (ps_h>0) && (ps_h>=ps_l))
		{
			obj->ps_th_l = ps_l;
			obj->ps_th_h = ps_h;
			rpr521_set_ps_threshold(obj->client, obj->ps_th_l, obj->ps_th_h);
		}
		else
		{
			APS_ERR("argu is invailed:ps_l = %d,ps_h = %d; ps_h>=ps_l>0: format:ps_l ps_h",ps_l,ps_h);
		}
	}
	return count;
}

static DRIVER_ATTR(regs,   S_IWUSR | S_IRUGO, show_registers, store_registers);
static DRIVER_ATTR(als,   S_IWUSR | S_IRUGO, show_als_value, NULL);
static DRIVER_ATTR(ps,   S_IWUSR | S_IRUGO, show_ps_value, NULL);
static DRIVER_ATTR(trace, S_IWUSR | S_IRUGO, show_trace_value, store_trace_value);
static DRIVER_ATTR(ps_threshould, S_IWUSR | S_IRUGO, show_ps_threshould_value , store_ps_threshould_value);


/*----------------------------------------------------------------------------*/
static struct driver_attribute *rpr521_attr_list[] = {
	&driver_attr_regs,				/* registers info */
	&driver_attr_als,				/* read als value */
	&driver_attr_ps,				/* read ps value */
	&driver_attr_trace,				/*trace value*/
	&driver_attr_ps_threshould,		/*ps_threshould*/

};
/*----------------------------------------------------------------------------*/
static int rpr521_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(rpr521_attr_list)/sizeof(rpr521_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(err = driver_create_file(driver, rpr521_attr_list[idx]))
		{            
			APS_ERR("driver_create_file (%s) = %d\n", rpr521_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int rpr521_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(rpr521_attr_list)/sizeof(rpr521_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, rpr521_attr_list[idx]);
	}
	
	return err;
}


/****************************************************************************** 
 * Function Configuration
******************************************************************************/
int rpr521_read_als(struct i2c_client *client, u16 *data)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);	
	u8 als_value_low[1], als_value_high[1];
	u8 buffer[1];
	u16 atio;

	int res = 0;
	READ_DATA_BUF   als_data;
	DEVICE_VAL  dev_val;
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
	
#ifdef _TEST_
	printk("rpr521 ENTER rpr521 read als\n"); 
#endif
	
//get adc channel 0 value
	buffer[0]=REG_ALSDATA0_LSB;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	buffer[0]=REG_ALSDATA0_MBS;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	 als_data.als_data0 = als_value_low[0] | (als_value_high[0]<<8);

	 if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	 {
	 	APS_DBG("\nROHM rpr521 read als: als_data0 = %d\n", als_data.als_data0);
	 }
	
//get adc channel 1 value
	buffer[0]=REG_ALSDATA1_LSB;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
		
	buffer[0]=REG_ALSDATA1_MBS;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
		
	als_data.als_data1 = als_value_low[0] | (als_value_high[0]<<8);

	 if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	 {
		 APS_DBG("\nROHM rpr521 read als: als_data1 = %d\n", als_data.als_data1);
	 }
	 
	get_from_device(&dev_val, client);

	*data = calc_rohm_als_data(als_data, dev_val);
	if(*data == 0)
		*data ++;
	if(*data == CALC_ERROR)
		*data = prev_als_value;	//Report same value as previous.
	else
		prev_als_value = *data;  //grace modified 

	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
 	   APS_DBG("\nROHM rpr521 read als output data = %d\n", *data);
	}

#ifdef _TEST_    
    rpr521_read_ps(obj->client, &obj->ps); 
#endif
    	
	return 0;	 
	
EXIT_ERR:
	APS_ERR("rpr521_read_als fail\n");
	return res;
}


int rpr521_read_als_ch0(struct i2c_client *client, u16 *data)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);	 
	u8 als_value_low[1], als_value_high[1];
	u8 buffer[1];
	int res = 0;
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
//get adc channel 0 value
	buffer[0]=REG_ALSDATA0_LSB;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	buffer[0]=REG_ALSDATA0_MBS;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	*data = als_value_low[0] | (als_value_high[0]<<8);

	return 0;	 

	
	
EXIT_ERR:
	APS_ERR("rpr521_read_als fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/

static int rpr521_get_als_value(struct rpr521_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als < obj->hw->als_level[idx])
		{
			break;
		}
	}
	
	if(idx >= obj->als_value_num)
	{
		APS_ERR("exceed range\n"); 
		idx = obj->als_value_num - 1;
	}
	
	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}
		
		if(1 == atomic_read(&obj->als_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
		{
			APS_DBG("ALS: raw data %05d => value = %05d\n", als, obj->hw->als_value[idx]);	
		}
		return obj->hw->als_value[idx];
	}
	else
	{
		APS_ERR("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
		return -1;
	}

}
/*----------------------------------------------------------------------------*/
long rpr521_read_ps(struct i2c_client *client, u16 *data)
{
   	struct rpr521_priv *obj = i2c_get_clientdata(client);	 
	u16 ps_value;
	u8 ps_value_low[1], ps_value_high[1];
	u8 buffer[1];
	int res = 0;
#ifdef _TEST_
	u8 mode[1], int_sts[1]; 
#endif

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	buffer[0]=REG_PSDATA_LSB;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, ps_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	buffer[0]=REG_PSDATA_MBS;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, ps_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	*data = (ps_value_low[0] | (ps_value_high[0]<<8))& 0xfff;

	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
		APS_DBG("ps_data=%d, low:%d  high:%d", *data, ps_value_low[0], ps_value_high[0]);
	}
	
#ifdef _TEST_

	buffer[0]=REG_MODECONTROL;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, mode, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	buffer[0]=REG_INTERRUPT;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, int_sts, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{	
		APS_DBG("ROHM rpr521--ps_data=%d, low:%d  high:%d", *data, ps_value_low[0], ps_value_high[0]); 
		APS_DBG("ROHM rpr521--mode=%x, int_sts:%x", mode[0], int_sts[0]);
	}
#endif	
	return 0;    

EXIT_ERR:
	APS_ERR("rpr521_read_ps fail\n");
	return res;
}


#define REG_PSTH_MAX	0xFFF
#define REG_PSTL_MAX	0xFFF

static int rpr521_set_ps_threshold(struct i2c_client *client, kal_uint16 ps_tl,kal_uint16 ps_th)
{
	u8 databuf[2];
  	u8 buffer[2];  
 	int res = 0;
	struct rpr521_priv *obj = (struct rpr521_priv *)i2c_get_clientdata(client);

	if(obj == NULL)
	{
		APS_ERR("obj is null !\n");
		return -1;
	}
	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
		APS_FUN();//20130109
	}
	databuf[0] = REG_PSTL_LSB;	
	databuf[1] = (u8)(ps_tl & 0x00FF);
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return rpr521_ERR_I2C;
	}
			
	databuf[0] = REG_PSTL_MBS;	
	databuf[1] = (u8)((ps_tl & 0xFF00) >> 8);
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return rpr521_ERR_I2C;
	}
			
	databuf[0] = REG_PSTH_LSB;	
	databuf[1] = (u8)( ps_th & 0x00FF);
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return rpr521_ERR_I2C;
	}
			
	databuf[0] = REG_PSTH_MBS;	
	databuf[1] = (u8)(( ps_th & 0xFF00) >> 8);
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return rpr521_ERR_I2C;
	}
	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
		APS_DBG(">>>>>>rpr521: ps_tl=%d , ps_th=%d\n", ps_tl, ps_th);//20130109
	}
}

/*----------------------------------------------------------------------------*/
static int rpr521_get_ps_value(struct rpr521_priv *obj, u16 ps)
{
	int val, mask = atomic_read(&obj->ps_mask);
	int invalid = 0;
	static int val_temp=1;
	u16 temp_ps[1];
	u8 buffer[1], infrared_data[1]; //grace modify in 2014.5.6
	int res = 0; //grace modify in 2014.5.6

	if(NULL == obj)
	{
		APS_ERR("obj is null\n");
		return -1;
	}

  	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
  	{
		APS_FUN();
	 	APS_DBG("PS raw data:  %05d => \n", ps);
	}


  //mdelay(160);
  	mdelay(50);
       
  //grace modify in 2014.5.6 begin
	buffer[0]=REG_PERSISTENCE;
	res = i2c_master_send(obj->client, buffer, 0x1);
	if(res <= 0)
	{
		return -1;
	}
	res = i2c_master_recv(obj->client, infrared_data, 0x1);
	if(res <= 0)
	{
		return -1;
	}
	//grace modify in 2014.5.6 end

//#ifdef _AUTO_THRESHOLD_CHANGE_
#if 1
	if(ps > obj->ps_th_h)
	{
		//grace modify in 2014.5.6 begin
		if((infrared_data[0]>>6) == 0) //ambient infrared is low
		{				
			val = 0;  /*close*/
			val_temp = 0;
			if(0 == obj->hw->polling_mode_ps)
				rpr521_set_ps_threshold(obj->client,obj->ps_th_l,REG_PSTH_MAX);
		}
		else
		{
			APS_DBG("ligh too high will result to failt proximiy\n");
			return 1;  /*far away*/
		}
		//grace modify in 2014.5.6 end
	}
	else if(ps < obj->ps_th_l)
	{
		val = 1;  /*far away*/
		val_temp = 1;
		if(0 == obj->hw->polling_mode_ps)
			rpr521_set_ps_threshold(obj->client,0,obj->ps_th_h);

	}
	else
		 val = val_temp;	
#else
	if(ps >= PS_ALS_SET_PS_TH)
	{
		//grace modify in 2014.5.6 begin
		if((infrared_data[0]>>6) == 0)
		{				
			val = 0;  /*close*/
			val_temp = 0;
			#if 1//def _AUTO_THRESHOLD_CHANGE_
			if(0 == obj->hw->polling_mode_ps)
				rpr521_set_ps_threshold(obj->client,PS_ALS_SET_PS_TL,REG_PSTH_MAX);
			#endif
		}
		else
		{
			APS_DBG("ligh too high will result to failt proximiy\n");
			return 1;  /*far away*/
		}
		//grace modify in 2014.5.6 end

	}
	else if(ps <= PS_ALS_SET_PS_TL)
	{
		val = 1;  /*far away*/
		val_temp = 1;
	#if 1//def _AUTO_THRESHOLD_CHANGE_
		if(0 == obj->hw->polling_mode_ps)
			rpr521_set_ps_threshold(obj->client,0,PS_ALS_SET_PS_TH);
      #endif

	}
	else
		 val = val_temp;	
#endif			
	
	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}
		
		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}
	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{	
		APS_DBG(">>>>>>>PS:  %05d => %05d\n", 	ps, val);
	}
	if(!invalid)
	{
		return val;
	}	
	else
	{
		return -1;
	}	
}


/*----------------------------------------------------------------------------*/
/*for interrup work mode support*/
static void rpr521_eint_work(struct work_struct *work)
{
	struct rpr521_priv *obj = (struct rpr521_priv *)container_of(work, struct rpr521_priv, eint_work);
	int err;
	hwm_sensor_data sensor_data;

	int res, int_status;
	u8 buffer[2];

	
	int result = 0;
	int status;    
	READ_DATA_BUF read_data_buf;

	DEVICE_VAL    dev_val;
	long          get_timer;
	long          wait_sec;
	unsigned long wait_nsec;

	if(0 == obj->hw->polling_mode_ps)
	{
		result =  rpr521_check_and_clear_intr(obj->client);
		
		if(result < 0)
		{
			APS_DBG("ERROR! read interrupt status. \n");
		}
		else  
		{
			status = result;
			if(status & PS_INT_MASK)
			{
				rpr521_read_ps(obj->client, &obj->ps);
			}
			if(status & ALS_INT_MASK) // 2 kinds of interrupt may occur at same time
			{
			}
			if(!((status & ALS_INT_MASK) || (status & PS_INT_MASK)))
			{
				APS_DBG( "Unknown interrupt source.\n");
				goto exit;
			}



			sensor_data.values[0] = rpr521_get_ps_value(obj, obj->ps);
			sensor_data.value_divide = 1;
			sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;	
			
#ifdef RPR0521_NEW_ARCH		
			if((err = ps_report_interrupt_data(sensor_data.values[0])))
			{
				APS_ERR("call ps_report_interrupt_data fail = %d\n", err);
			}

#else		//let up layer to know
			if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
			{
		 		 APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
			}
#endif
		}		
		//enable_irq(ps_als->client->irq);
	}
	else
	{
		rpr521_read_ps(obj->client, &obj->ps);
		//mdelay(160);
		if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
		{
			APS_DBG("rpr521_eint_work rawdata ps=%d als_ch0=%d!\n",obj->ps,obj->als);
		}
		sensor_data.values[0] = rpr521_get_ps_value(obj, obj->ps);
		sensor_data.value_divide = 1;
		sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;			

#ifdef RPR0521_NEW_ARCH		
					if((err = ps_report_interrupt_data(sensor_data.values[0])))
					{
						APS_ERR("call ps_report_interrupt_data fail = %d\n", err);
					}
		
#else		//let up layer to know
					if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
					{
						 APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
					}
#endif

		
		
	}

exit:
#ifdef CUST_EINT_ALS_TYPE
	mt_eint_unmask(CUST_EINT_ALS_NUM);
#else
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);
#endif    
}


/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int rpr521_open(struct inode *inode, struct file *file)
{
	file->private_data = rpr521_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int rpr521_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}


/*----------------------------------------------------------------------------*/
static long rpr521_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct rpr521_priv *obj = i2c_get_clientdata(client);  
	long err = 0;
	void __user *ptr = (void __user*) arg;
	int dat = 0;
	uint32_t enable = 0;
#ifdef _TEST_	
	printk("%s:cmd=%x\n",__func__,cmd);
#endif
	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
#ifdef _TEST
			printk("%s:enable=%x\n",__func__,enable);
#endif
			if(enable)
			{
				if((err = rpr521_enable_ps(obj->client, 1)))
				{
					APS_ERR("enable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				set_bit(CMC_BIT_PS, &obj->enable);
#ifdef _TEST_	
				printk("rpr521 enable ps success\n");
#endif
			}
			else
			{
				if((err = rpr521_enable_ps(obj->client, 0)))
				{
					APS_ERR("disable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				clear_bit(CMC_BIT_PS, &obj->enable);
#ifdef _TEST_	
				printk("rpr521 disable ps success\n");
#endif
			}
			break;

		case ALSPS_GET_PS_MODE:
			enable = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:    
			if((err = rpr521_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = rpr521_get_ps_value(obj, obj->ps);
#ifdef _TEST_	
			printk("rpr521 ALSPS_GET_PS_DATA %d\n", dat); 
#endif			
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;

		case ALSPS_GET_PS_RAW_DATA:    
			if((err = rpr521_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = obj->ps;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;              

		case ALSPS_SET_ALS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = rpr521_enable_als(obj->client, 1)))
				{
					APS_ERR("enable als fail: %ld\n", err); 
					goto err_out;
				}
				set_bit(CMC_BIT_ALS, &obj->enable);
#ifdef _TEST_	
				printk("rpr521 enable als success\n");
#endif
			}
			else
			{
				if((err = rpr521_enable_als(obj->client, 0)))
				{
					APS_ERR("disable als fail: %ld\n", err); 
					goto err_out;
				}
				clear_bit(CMC_BIT_ALS, &obj->enable);
#ifdef _TEST_	
				printk("rpr521 disable als success\n"); 
#endif 
			}
			break;

		case ALSPS_GET_ALS_MODE:
			enable = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_DATA: 
			if((err = rpr521_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = rpr521_get_als_value(obj, obj->als);
#ifdef _TEST_	
			printk("rpr521 ALSPS_GET_ALS_DATA data=%d, level=%d\n", obj->als,dat); 
#endif
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		case ALSPS_GET_ALS_RAW_DATA:    
			if((err = rpr521_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = obj->als;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;
			
		default:
			APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	err_out:
	return err;    
}
/*----------------------------------------------------------------------------*/
static struct file_operations rpr521_fops = {
	.owner = THIS_MODULE,
	.open = rpr521_open,
	.release = rpr521_release,
	.unlocked_ioctl = rpr521_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice rpr521_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &rpr521_fops,
};
/*----------------------------------------------------------------------------*/
static int rpr521_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
	//struct rpr521_priv *obj = i2c_get_clientdata(client);    
	//int err;
	APS_FUN();    
#if 0
	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(!obj)
		{
			APS_ERR("null pointer!!\n");
			return -EINVAL;
		}
		
		atomic_set(&obj->als_suspend, 1);
		if(err = rpr521_enable_als(client, 0))
		{
			APS_ERR("disable als: %d\n", err);
			return err;
		}

		atomic_set(&obj->ps_suspend, 1);
		if(err = rpr521_enable_ps(client, 0))
		{
			APS_ERR("disable ps:  %d\n", err);
			return err;
		}
		
		rpr521_power(obj->hw, 0);
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static int rpr521_i2c_resume(struct i2c_client *client)
{
	//struct rpr521_priv *obj = i2c_get_clientdata(client);        
	//int err;
	APS_FUN();
#if 0
	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	rpr521_power(obj->hw, 1);
	if(err = rpr521_init_client(client))
	{
		APS_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if(err = rpr521_enable_als(client, 1))
		{
			APS_ERR("enable als fail: %d\n", err);        
		}
	}
	atomic_set(&obj->ps_suspend, 0);
	if(test_bit(CMC_BIT_PS,  &obj->enable))
	{
		if(err = rpr521_enable_ps(client, 1))
		{
			APS_ERR("enable ps fail: %d\n", err);                
		}
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static void rpr521_early_suspend(struct early_suspend *h) 
{   /*early_suspend is only applied for ALS*/
	struct rpr521_priv *obj = container_of(h, struct rpr521_priv, early_drv);   
	int err;
	APS_FUN();    

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

	#if 1
	atomic_set(&obj->als_suspend, 1);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = rpr521_enable_als(obj->client, 0)))
		{
			APS_ERR("disable als fail: %d\n", err); 
		}
	}
	#endif
}
/*----------------------------------------------------------------------------*/
static void rpr521_late_resume(struct early_suspend *h)
{   /*early_suspend is only applied for ALS*/
	struct rpr521_priv *obj = container_of(h, struct rpr521_priv, early_drv);       
	int err;
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

        #if 1
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = rpr521_enable_als(obj->client, 1)))
		{
			APS_ERR("enable als fail: %d\n", err);        

		}
	}
	#endif
}
#ifdef RPR0521_NEW_ARCH
static int rpr0521_ps_open_report_data(int open)
{
	
	APS_FUN();

	return 0;
}
static int rpr0521_ps_enable_nodata(int en)
{
	int err = 0;
	int value = 0;
	struct rpr521_priv *obj = rpr521_obj;
	
	APS_FUN();

	if(NULL == obj)
	{
		APS_ERR("rpr521_obj is NULL!!!\n");
		return -1;
	}
	
	value = en;
	if(value)
	{
		if((err = rpr521_enable_ps(obj->client, 1)))
		{
			APS_ERR("enable ps fail: %d\n", err); 
			return -1;
		}
		set_bit(CMC_BIT_PS, &obj->enable);
	}
	else
	{
		if((err = rpr521_enable_ps(obj->client, 0)))
		{
			APS_ERR("disable ps fail: %d\n", err); 
			return -1;
		}
		clear_bit(CMC_BIT_PS, &obj->enable);
	}

	return 0;

}

static int rpr0521_ps_set_delay(u64 ns)
{
	struct rpr521_priv *obj = rpr521_obj;
	
	APS_FUN();

	if(NULL == obj)
	{
		APS_ERR("rpr521_obj is NULL!!!\n");
		return -1;
	}

	return 0;

}
static int rpr0521_ps_get_data(int* value, int* status)
{
	int tmp_val = 0;
	int tmp_status = 0;
	struct rpr521_priv *obj = rpr521_obj;
	
	APS_FUN();

	if(NULL == obj)
	{
		APS_ERR("rpr521_obj is NULL!!!\n");
		return -1;
	}
	
	rpr521_read_ps(obj->client, &obj->ps);

	tmp_val = rpr521_get_ps_value(obj, obj->ps);
	tmp_status = SENSOR_STATUS_ACCURACY_MEDIUM;	

	*value = tmp_val;
	*status = tmp_status;
	
	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
		APS_LOG("rpr521_ps_operate ps raw data=%d!, value=%d\n", obj->ps, tmp_val);	
	}
	
	return 0;

}

#else
int rpr521_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct rpr521_priv *obj = (struct rpr521_priv *)self;
	
	printk("rpr521_ps_operate command1:%d SENSOR_ENABLE:%d\n",command,(uint32_t)SENSOR_ENABLE);
	
	APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{				
				value = *(int *)buff_in;
				if(value)
				{
					if((err = rpr521_enable_ps(obj->client, 1)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_PS, &obj->enable);
					#if 0	
					if(err = rpr521_enable_als(obj->client, 1))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
					#endif
				}
				else
				{
					if((err = rpr521_enable_ps(obj->client, 0)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_PS, &obj->enable);
					#if 0
					if(err = rpr521_enable_als(obj->client, 0))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_ALS, &obj->enable);
					#endif
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;	
				rpr521_read_ps(obj->client, &obj->ps);
				
                                //mdelay(160);
				//APS_ERR("rpr521_ps_operate als data=%d!\n",obj->als);
				sensor_data->values[0] = rpr521_get_ps_value(obj, obj->ps);
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;	
				if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
				{
					APS_LOG("rpr521_ps_operate ps raw data=%d!, value=%d\n", obj->ps, sensor_data->values[0]);
				}
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}
#endif
static int temp_als = 0;

#ifdef RPR0521_NEW_ARCH
static int rpr0521_als_open_report_data(int open)
{
	APS_FUN();
	
	return 0;
}
static int rpr0521_als_enable_nodata(int en)
{
	int err = 0;
	int value = 0;
	struct rpr521_priv *obj = rpr521_obj;
	
	APS_FUN();

	if(NULL == obj)
	{
		APS_ERR("rpr521_obj is NULL!!!\n");
		return -1;
	}
	value = en;				
	if(value)
	{
		if((err = rpr521_enable_als(obj->client, 1)))
		{
			APS_ERR("enable als fail: %d\n", err); 
			return -1;
		}
		set_bit(CMC_BIT_ALS, &obj->enable);
	}
	else
	{
		if((err = rpr521_enable_als(obj->client, 0)))
		{
			APS_ERR("disable als fail: %d\n", err); 
			return -1;
		}
		clear_bit(CMC_BIT_ALS, &obj->enable);
	}

	
	return 0;
}
static int rpr0521_als_set_delay(u64 ns)
{
	struct rpr521_priv *obj = rpr521_obj;
	
	APS_FUN();

	if(NULL == obj)
	{
		APS_ERR("rpr521_obj is NULL!!!\n");
		return -1;
	}
	
	return 0;
}
static int rpr0521_als_get_data(int* value, int* status)
{
	int tmp_val = 0;
	int tmp_status = 0;
	struct rpr521_priv *obj = rpr521_obj;
	if(atomic_read(&(obj->trace)) & ALSPS_DEBUG)
	{
		APS_FUN();
	}

	if(NULL == obj)
	{
		APS_ERR("rpr521_obj is NULL!!!\n");
		return -1;
	}
	
	rpr521_read_als(obj->client, &obj->als);
#if defined(MTK_AAL_SUPPORT)
	tmp_val = obj->als;
#else
	tmp_val = rpr521_get_als_value(obj, obj->als);
#endif
	tmp_status = SENSOR_STATUS_ACCURACY_MEDIUM;

	*value = tmp_val;
	*status = tmp_status;

	return 0;
}


#else
int rpr521_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct rpr521_priv *obj = (struct rpr521_priv *)self;

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;				
				if(value)
				{
					if((err = rpr521_enable_als(obj->client, 1)))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
				}
				else
				{
					if((err = rpr521_enable_als(obj->client, 0)))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_ALS, &obj->enable);
				}
				
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;
				/*yucong MTK add for fixing know issue*/
				rpr521_read_als(obj->client, &obj->als);
				#if defined(MTK_AAL_SUPPORT)
				sensor_data->values[0] = obj->als;
				#else
				sensor_data->values[0] = rpr521_get_als_value(obj, obj->als);
				#endif
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
			}
			break;
		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}
#endif
#ifdef _AUTO_THRESHOLD_CHANGE_

/*----------------------------------------------------------------------------*/
#define THRES_TOLERANCE		20	//I think this is a proper value. It should not be too big.
#define THRES_DEFAULT_DIFF	       80

static int rpr521_calibrate(struct i2c_client *client)
{
	struct rpr521_priv *obj = i2c_get_clientdata(client);
	int average;
 	unsigned int i, tmp, ps_th_h, ps_th_l;	
  	u8 databuf[2];
 	 u8 buffer[2];  
  	int res = 0;
  	u8 ps_value_low[1], ps_value_high[1], syssta[1], modectl[1];

	average = 0;

  	printk("zzf rpr521_calibrate\n");
	
	databuf[0] = 0x41;    
	databuf[1] = 0x41;    //PS 10ms
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto err_exit;
	}	

	for(i = 0; i < 20; i++)
	{
		mdelay(20); //grace modify in 2014.5.6
		buffer[0]=REG_PSDATA_LSB;
	  	res = i2c_master_send(client, buffer, 0x1);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		res = i2c_master_recv(client, ps_value_low, 0x1);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}

		buffer[0]=REG_PSDATA_MBS;
		res = i2c_master_send(client, buffer, 0x1);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		res = i2c_master_recv(client, ps_value_high, 0x01);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}

		tmp = ps_value_low[0] | (ps_value_high[0]<<8);
        
		if(tmp < 0)
		{
			printk(KERN_ERR "%s: i2c read ps data fail. \n", __func__);
			goto EXIT_ERR;
		}
		average += tmp & 0xFFF;	// 12 bit data
		printk("%s: calibrate data %d = %d. \n", __func__, i + 1, tmp);
	}
	average /= 20;
  printk("zzf the average is :%d!\n", average);
 
	ps_th_h = average + THRES_TOLERANCE + THRES_DEFAULT_DIFF;
	ps_th_l = average + THRES_TOLERANCE;

	printk(KERN_INFO "[andy]%s: calculated threshold: %d -> %d.\n", __FUNCTION__, ps_th_l, ps_th_h);

	if(ps_th_h < 0)
	{
		printk(KERN_ERR "%s: high threshold is less than 0.\n", __func__);
		goto EXIT_ERR;
	}
	if(ps_th_h > 0xfff)
	{
		printk(KERN_ERR "%s: high threshold is greater than maximum allowed value.\n", __func__);
		goto EXIT_ERR;
	}

	if(ps_th_l < 0)
	{
		printk(KERN_ERR "%s: low threshold is less than 0.\n", __func__);
		goto EXIT_ERR;
   }
	if(ps_th_l > 0xfff)
	{
		printk(KERN_ERR "%s: low threshold is greater than maximum allowed value.\n", __func__);
		goto EXIT_ERR;
	}

     databuf[0] = REG_PSTL_LSB;
     databuf[1] = (u8)(ps_th_l & 0x00FF);
     res = i2c_master_send(client, databuf, 0x2);
     if(res <= 0)
               goto EXIT_ERR;
     
     databuf[0] = REG_PSTL_MBS;
     databuf[1] = (u8)( ps_th_l  & 0xFF00 >> 8);
     res = i2c_master_send(client, databuf, 0x2);
     if(res <= 0) 
               goto EXIT_ERR;

     databuf[0] = REG_PSTH_LSB;
     databuf[1] = (u8)(ps_th_h & 0x00FF);
     res = i2c_master_send(client, databuf, 0x2);
     if(res <= 0) 
               goto EXIT_ERR;
     
     databuf[0] = REG_PSTH_MBS;
     databuf[1] = (u8)(ps_th_h & 0xFF00 >> 8);
     res = i2c_master_send(client, databuf, 0x2);
     if(res <= 0) 
               goto EXIT_ERR;

	obj->ps_th_h = ps_th_h;
	obj->ps_th_l = ps_th_l;

	databuf[0] = 0x41;    
	databuf[1] = 0x6;    //disable ps
	res = i2c_master_send(client,databuf, 0x2);
	return 0;		

EXIT_ERR:
	databuf[0] = 0x41;    
	databuf[1] = 0x6;   //disable ps
	res = i2c_master_send(client,databuf, 0x2);
	return -1;
	
}
#endif


/*----------------------------------------------------------------------------*/
static int rpr521_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    	
	strcpy(info->type, RPR521_DEV_NAME);	
	return 0;
}



/*----------------------------------------------------------------------------*/
static int rpr521_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct rpr521_priv *obj;
#ifdef RPR0521_NEW_ARCH
	struct als_control_path als_ctl_path = {0};
	struct als_data_path als_dat_path = {0};
	struct ps_control_path ps_ctl_path = {0};
	struct ps_data_path ps_dat_path = {0};
#else
	struct hwmsen_object obj_ps, obj_als;
#endif
	int err = 0;

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	rpr521_obj = obj;

	obj->hw = get_cust_alsps_hw();
	rpr521_get_addr(obj->hw, &obj->addr);

	/*for interrup work mode support*/
	INIT_WORK(&obj->eint_work, rpr521_eint_work);
	obj->client = client;
	i2c_set_clientdata(client, obj);	
	atomic_set(&obj->als_debounce, 50);
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 10);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->als_cmd_val, 0xDF);
	atomic_set(&obj->ps_cmd_val,  0xC1);

	/**************************************************************************/
	obj->ps_th_h = obj->hw->ps_threshold_high;
	obj->ps_th_l = obj->hw->ps_threshold_low;
	/***********************************************************************/
	
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);  
	
	obj->als_modulus = (400*100*40)/(1*1500);
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);
	set_bit(CMC_BIT_ALS, &obj->enable);
	set_bit(CMC_BIT_PS, &obj->enable);

	
	rpr521_i2c_client = client;

	
	if((err = rpr521_init_client(client)))
	{
		goto exit_init_failed;
	}
	APS_LOG("rpr521_init_client() OK!\n");

	if((err = misc_register(&rpr521_device)))
	{
		APS_ERR("rpr521_device register failed\n");
		goto exit_misc_device_register_failed;
	}
#ifdef RPR0521_NEW_ARCH
	if(rpr521_create_attr(&(rpr0521_init_info.platform_diver_addr->driver)))
	{
		APS_ERR("create attr failed.\n");
		goto exit_create_attr_failed;
	}


	als_ctl_path.open_report_data	 = rpr0521_als_open_report_data;
	als_ctl_path.enable_nodata		 = rpr0521_als_enable_nodata;
	als_ctl_path.set_delay 			 = rpr0521_als_set_delay;
	als_ctl_path.is_support_batch 	 = false;
	if((err=als_register_control_path(&als_ctl_path)) < 0)
	{
		APS_ERR("als_register_control_path fail = %d\n", err);
		goto exit_create_attr_failed;
	}

	als_dat_path.get_data	 = rpr0521_als_get_data;
	als_dat_path.vender_div  = 1;
	if((err=als_register_data_path(&als_dat_path)) < 0)
	{
		APS_ERR("als_register_data_path fail = %d\n", err);
		goto exit_create_attr_failed;
	}

	/*for interrup work mode support*/
	if(1 == obj->hw->polling_mode_ps)
	{
		ps_ctl_path.is_polling_mode = true;
	}
	else
	{
		ps_ctl_path.is_polling_mode = false;
	}
	
	ps_ctl_path.open_report_data 	 = rpr0521_ps_open_report_data;
	ps_ctl_path.enable_nodata 		 = rpr0521_ps_enable_nodata;
	ps_ctl_path.set_delay 			 = rpr0521_ps_set_delay;
	ps_ctl_path.is_support_batch 	 = false;
	if((err=ps_register_control_path(&ps_ctl_path)) < 0)
	{
		APS_ERR("ps_register_control_path fail = %d\n", err);
		goto exit_create_attr_failed;
	}

	ps_dat_path.get_data	 = rpr0521_ps_get_data;
	ps_dat_path.vender_div  = 1;
	if((err=ps_register_data_path(&ps_dat_path)) < 0)
	{
		APS_ERR("als_register_data_path fail = %d\n", err);
		goto exit_create_attr_failed;
	}
#else
	if (rpr521_create_attr(&(rpr521_alsps_driver.driver)))
	{
		APS_ERR("create attr failed.\n");
		goto exit_create_attr_failed;
	}

	obj_ps.self = rpr521_obj;
	/*for interrup work mode support*/
	if(1 == obj->hw->polling_mode_ps)
	{
		obj_ps.polling = 1;
	}
	else
	{
		obj_ps.polling = 0;
	}

	obj_ps.sensor_operate = rpr521_ps_operate;
	if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	obj_als.self = rpr521_obj;
	obj_als.polling = 1;
	obj_als.sensor_operate = rpr521_als_operate;
	if((err = hwmsen_attach(ID_LIGHT, &obj_als)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
#endif

#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 2,
	obj->early_drv.suspend  = rpr521_early_suspend,
	obj->early_drv.resume   = rpr521_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif

#ifdef RPR0521_NEW_ARCH
	rpr0521_init_flag = 0;
#endif

	APS_LOG("%s: OK\n", __func__);
	return 0;

	exit_create_attr_failed:
	misc_deregister(&rpr521_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(client);
	exit_kfree:
	kfree(obj);
	exit:
#ifdef RPR0521_NEW_ARCH
	rpr0521_init_flag = -1;
#endif
	rpr521_i2c_client = NULL;           
//	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
	APS_ERR("%s: err = %d\n", __func__, err);
	return err;
}
/*----------------------------------------------------------------------------*/
static int rpr521_i2c_remove(struct i2c_client *client)
{
	int err;	

#ifdef RPR0521_NEW_ARCH
	if (rpr521_delete_attr(&(rpr0521_init_info.platform_diver_addr->driver)))
	{
		APS_ERR("delete attr failed.\n");
	}	

#else
	if (rpr521_delete_attr(&(rpr521_alsps_driver.driver)))
	{
		APS_ERR("delete attr failed.\n");
	}
#endif  
	if((err = misc_deregister(&rpr521_device)))
	{
		APS_ERR("misc_deregister fail: %d\n", err);    
	}
#ifdef RPR0521_NEW_ARCH
	rpr0521_init_flag = -1;
#endif	
	rpr521_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}
/*----------------------------------------------------------------------------*/
#ifdef RPR0521_NEW_ARCH
static int rpr0521_local_init(void)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();	

	rpr521_power(hw, 1);    
	
	if(i2c_add_driver(&rpr521_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 
	if(-1 == rpr0521_init_flag)
	{
		APS_ERR("rpr0521_init error,rpr0521_init_flag = %d\n", rpr0521_init_flag);
		return -1;
	}
	return 0;
}
static int rpr0521_local_uninit(void)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	rpr521_power(hw, 0);    
	i2c_del_driver(&rpr521_i2c_driver);
	
	rpr0521_init_flag = -1;

	return 0;
}

#else
static int rpr521_probe(struct platform_device *pdev) 
{
	struct alsps_hw *hw = get_cust_alsps_hw();

	rpr521_power(hw, 1);    
	
	if(i2c_add_driver(&rpr521_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 
	return 0;
}
/*----------------------------------------------------------------------------*/
static int rpr521_remove(struct platform_device *pdev)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	rpr521_power(hw, 0);    
	i2c_del_driver(&rpr521_i2c_driver);
	return 0;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_OF
static const struct of_device_id als_ps_of_match[] = {
	{ .compatible = "mediatek,als_ps", },
	{},
};
#endif

static struct platform_driver rpr521_alsps_driver = {
	.probe      = rpr521_probe,
	.remove     = rpr521_remove,    
	.driver     = {
		.name  = "als_ps",
		.owner = THIS_MODULE,
		#ifdef CONFIG_OF
		.of_match_table = als_ps_of_match,
		#endif
	}
};
#endif
/*----------------------------------------------------------------------------*/
static int __init rpr521_init(void)
{
	APS_FUN();
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_RPR521, 1);
#ifdef RPR0521_NEW_ARCH
	if(alsps_driver_add(&rpr0521_init_info) < 0)
	{
		APS_ERR("failed to alsps_driver_add driver");
		return -ENODEV;
	}
#else
	if(platform_driver_register(&rpr521_alsps_driver))
	{
		APS_ERR("failed to register driver");
		return -ENODEV;
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit rpr521_exit(void)
{
	APS_FUN();
#ifndef RPR0521_NEW_ARCH
	platform_driver_unregister(&rpr521_alsps_driver);
#endif
}
/*----------------------------------------------------------------------------*/
module_init(rpr521_init);
module_exit(rpr521_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Grace Huang");
MODULE_DESCRIPTION("rpr521 driver");
MODULE_LICENSE("GPL");
