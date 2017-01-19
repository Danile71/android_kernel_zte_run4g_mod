/*
 * Copyright (C) 2012 UPI semi <Finley_huang@upi-semi.com>. All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include <linux/hwmsen_helper.h>
#include "us5182.h"
#include <alsps.h>


#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#ifdef  US5182_HEARTRATE_DETECTION
#include "us5182_heartrate.h"
#endif  ///< end of US5182_HEARTRATE_DETECTION

//#define FEATURE_GET_EINT_EDGE                      ///< [AT-PM] : Get the edge of eint pin ; 10/29/2014
//#define FEATURE_GET_PROX_EDGE                      ///< [AT-PM] : Get the edge of PROX state ; 10/29/2014
//#define FEATURE_PROX_FOR_INT_TYPE                  ///< [AT-PM] : Change interrupt source according to PROX state ; 10/29/2014
//#define FEATURE_PS_ISR                             ///< [AT-PM] : Report PS status in interrupt ; 10/29/2014
//#define FEATURE_PS_ISR_CORE_RST_DELAY              ///< [AT-PM] : Enable delay time after in the interrupt after core_rst ; 10/29/2014
#define FEATURE_PS_ISR_SW_FAULT_Q                  ///< [AT-PM] : Enable software fault queue for PS interrupt ; 10/29/2014
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
  //#define FEATURE_PS_ISR_SW_FAULT_Q_ONCE           ///< [AT-PM] : Software fault queue for PS interrupt is an one-shot execution ; 10/29/2014
  #define FEATURE_PS_ISR_SW_FAULT_Q_AT_START       ///< [AT-PM] : Start software fault queue for PS interrupt at enable ; 10/29/2014
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
#define FEATURE_SW_FAULT_Q_DYNA_POLL               ///< [AT-PM] : Enable synamic polling time of software fault queue ; 10/29/2014
//#define FEATURE_POWER_SAVE_WAIT_SEL                ///< [AT-PM] : Change WAIT_SEL for power saving mode ; 10/29/2014
//#define FEATURE_POWER_SAVE_LEDRI_SEL               ///< [AT-PM] : Change LEDRI_SEL for power saving mode ; 10/29/2014
//#define FEATURE_POWER_SAVE_FREQ_SEL                ///< [AT-PM] : Change FREQ_SEL for power saving mode ; 10/29/2014
//#define FEATURE_POWER_SAVE_CKRATE_SEL              ///< [AT-PM] : Change CKRATE_SEL for power saving mode ; 10/29/2014
#define FEATURE_NO_PS_ONLY_OP_MODE                 ///< [AT-PM] : Remove PS ONLY operation mode, i.e. only ALS + PS and ALS ONLY are available ; 10/29/2014
#define FEATURE_FULL_DEBUG_MSG                     ///< [AT-PM] : All debug message are KERN_ERR level ; 10/29/2014
//#define FEATURE_IRLED_100MA                        ///< [AT-PM] : Set IRLED at 100mA ; 10/30/2014
#define FEATURE_IRLED_50MA                         ///< [AT-PM] : Set IRLED at 50mA ; 10/30/2014
//#define FEATURE_IRLED_25MA                         ///< [AT-PM] : Set IRLED at 25mA ; 10/30/2014
//#define FEATURE_IRLED_12MA                         ///< [AT-PM] : Set IRLED at 12.5mA ; 10/30/2014
/*
static struct alsps_hw cust_alsps_hw = {
  .i2c_num            = 2,
  .polling_mode_ps    = 0,
  .polling_mode_als   = 1,
  .power_id           = MT65XX_POWER_NONE,
  .power_vol          = VOL_DEFAULT,
  .i2c_addr           = { 0x72, 0x00, 0x00, 0x00, },
  .als_level          = { 4, 40, 80, 120, 160, 250, 400, 800, 1200, 1600, 2000, 3000, 5000, 10000, 65535,       },
  .als_value          = { 1, 5,  9,  13,  17,  25,  38,  70,  100,  128,  156,  223,  350,  643,   3355,  3355, },
  .als_window_loss    = 75,
#ifdef  FEATURE_IRLED_100MA
  .ps_threshold_high  = 0x3E8,
  .ps_threshold_low   = 0x1F4,
  .ps_threshold       = 0x3E8,
#endif  ///< end of FEATURE_IRLED_100MA
#ifdef  FEATURE_IRLED_50MA
  .ps_threshold_high  = 0x1F4,
  .ps_threshold_low   = 0xFA,
  .ps_threshold       = 0x1F4,
#endif  ///< end of FEATURE_IRLED_50MA
#ifdef  FEATURE_IRLED_25MA
  .ps_threshold_high  = 0xFA,
  .ps_threshold_low   = 0x7D,
  .ps_threshold       = 0xFA,
#endif  ///< end of FEATURE_IRLED_25MA
#ifdef  FEATURE_IRLED_12MA
  .ps_threshold_high  = 0x7D,
  .ps_threshold_low   = 0x3E,
  .ps_threshold       = 0x7D,
#endif  ///< end of FEATURE_IRLED_12MA
};

struct alsps_hw *get_cust_alsps_hw(void)
{
  return (&cust_alsps_hw);
}*/
#define KK2_TRY  1

#define POWER_NONE_MACRO MT65XX_POWER_NONE
/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_US5182           (5182)

#define US5182_I2C_ADDR_RAR           (0)         ///< the index in obj->hw->i2c_addr: alert response address
#define US5182_I2C_ADDR_ALS           (1)         ///< the index in obj->hw->i2c_addr: ALS address
#define US5182_I2C_ADDR_PS            (2)         ///< the index in obj->hw->i2c_addr: PS address
#define US5182_DEV_NAME               ("US5182")

#ifdef  FEATURE_PS_ISR_CORE_RST_DELAY
#define US5182_CORE_RESET_TIME        (50)
#endif  ///< end of FEATURE_PS_ISR_CORE_RST_DELAY 
#define US5182_FORCE_INT_TIME         (100)

#define US5182_GET_PS_INTERVAL_MS     (10)
#define US5182_GET_PS_MS_TO_NS        (1000 * 1000)
#define US5182_GET_PS_INTERVAL_NS     (US5182_GET_PS_INTERVAL_MS * US5182_GET_PS_MS_TO_NS)
#define US5182_GET_PS_SW_FQ_MS        (100)

#define US5182_GET_ALS_INTERVAL_MS    (100)

#define US5182_ALS_WINDOW_LOSS_CONST  (100)
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[US5182D] "
#ifdef  FEATURE_FULL_DEBUG_MSG
#define APS_FUN(f)               printk(KERN_ERR APS_TAG"- FUN - V4.02 : %s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_ERR APS_TAG"%s : " fmt, __FUNCTION__, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_ERR APS_TAG"%s : " fmt, __FUNCTION__, ##args)
#else   ///< else of FEATURE_FULL_DEBUG_MSG
#define APS_FUN(f)               printk(KERN_INFO    APS_TAG"- FUN : %s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR     APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_WARNING APS_TAG"%s : " fmt, __FUNCTION__, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_DEBUG   APS_TAG"%s : " fmt, __FUNCTION__, ##args)
#endif  ///< end of FEATURE_FULL_DEBUG_MSG
/*----------------------------------------------------------------------------*/
static struct i2c_client *us5182_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id us5182_i2c_id[] = {{US5182_DEV_NAME,0},{}};
static const struct i2c_board_info __initdata i2c_US5182= {I2C_BOARD_INFO("US5182",0x39)};
/******************************************************************************

 * function declaration
*******************************************************************************/
#ifdef CUST_EINT_ALS_TYPE
	 extern void mt_eint_mask(unsigned int eint_num);
	 extern void mt_eint_unmask(unsigned int eint_num);
	 extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	 extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
	 extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
	 extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
	 extern void mt_eint_print_status(void);
#else
	 extern void mt65xx_eint_mask(unsigned int line);
	 extern void mt65xx_eint_unmask(unsigned int line);
	 extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	 extern void mt65xx_eint_set_polarity(unsigned int eint_num, unsigned int pol);
	 extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
	 extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif

static int us5182_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int us5182_i2c_remove(struct i2c_client *client);
static int us5182_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int us5182_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int us5182_i2c_resume(struct i2c_client *client);
static struct us5182_priv *g_us5182_ptr = NULL;
static void us5182_eint_func(void);
/******************************************************************************
 * structure
*******************************************************************************/
typedef enum {
  CMC_TRC_ALS_DATA = 0x0001,
  CMC_TRC_PS_DATA  = 0x0002,
  CMC_TRC_EINT     = 0x0004,
  CMC_TRC_IOCTL    = 0x0008,
  CMC_TRC_I2C      = 0x0010,
  CMC_TRC_CVT_ALS  = 0x0020,
  CMC_TRC_CVT_PS   = 0x0040,
  CMC_TRC_DEBUG    = 0x8000,
} CMC_TRC;

typedef enum {
  CMC_BIT_ALS    = 1,
  CMC_BIT_PS     = 2,
} CMC_BIT;

static bool apk_tool_active = false;

static struct i2c_driver us5182_i2c_driver = {  
  .probe      = us5182_i2c_probe,
  .remove     = us5182_i2c_remove,
  .suspend    = us5182_i2c_suspend,
  .resume     = us5182_i2c_resume,
  .id_table   = us5182_i2c_id,
  .driver = {
    .name           = US5182_DEV_NAME,
  },
};

struct us5182_priv *us5182_obj = NULL;

#if defined(MTK_AUTO_DETECT_ALSPS)
static struct sensor_init_info us5182_init_info;
#else
static struct platform_driver us5182_alsps_driver;
#endif

#ifdef  FEATURE_GET_EINT_EDGE
static int us5182_eint_prev_state;
#endif  ///< end of FEATURE_GET_EINT_EDGE
#ifdef  FEATURE_GET_PROX_EDGE
static int us5182_prox_prev_state;
#endif  ///< end of FEATURE_GET_PROX_EDGE

static int us5182_get_ps_value(struct us5182_priv *obj, u32 ps);
static int us5182_get_als_value(struct us5182_priv *obj, u32 als);
/*----------------------------------------------------------------------------*/
static int hwmsen_read_byte_sr(struct i2c_client *client, u8 addr, u8 *data)
{
  u8 buf;
  int ret = 0;
  int retry;
  
  retry = 0;
  while(1)
  {
    client->addr = (client->addr & I2C_MASK_FLAG) | I2C_WR_FLAG |I2C_RS_FLAG;
    buf = addr;
    ret = i2c_master_send(client, (const char*)&buf, 1<<8 | 1);
    if(ret < 0)
    {
      retry = retry + 1;
      APS_ERR("send command error!! (%d)\n", retry);

      if(retry > 3)
      {
        return (-EFAULT);
      }
    }
    else
    {
      break;
    }
  }

  *data = buf;
  client->addr = client->addr& I2C_MASK_FLAG;
  return (0);
}
/*----------------------------------------------------------------------------*/
static int hwmsen_write_byte_sr(struct i2c_client *client, u8 reg, u8 reg_data)
{
  int retry;
  int ret = 0;
  
  retry = 0;
  while(1)
  {
    ret = hwmsen_write_byte(client, reg, reg_data);
    if(ret<0)
	{
      retry = retry + 1;
      APS_ERR("retry = %d\n", retry);

      if(retry > 3)
      {
        return (-EFAULT);
      }
    }
    else
    {
      break;
    }
  }
  return (0);
}
/*----------------------------------------------------------------------------*/
int us5182_reg_write(struct i2c_client *client, u8 reg,u8 mask, u8 shift, int val )
{
  u8 regdata;
    
  hwmsen_read_byte_sr(client,reg,&regdata);
  regdata &= ~mask;
  regdata |= val << shift;
  
  if(hwmsen_write_byte_sr(client,reg,regdata))
  {
    return (-EFAULT);
  }
  return (0);
}
/*----------------------------------------------------------------------------*/
static u8 dump_reg_addr_table[] = 
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x16, 0x17,
  0x20, 0x21, 0x22, 0x27, 0x28, 0x29, 0x2A, 0x2B,
  0xB2,
  0xFF,
};

static int us5182_dumpReg(struct i2c_client *client, char *buf)
{
  int idx;
  u8 reg_data;
  int len;

  idx = 0;
  len = 0;
  while(1)
  {
    if(dump_reg_addr_table[idx] == 0xFF)
    {
      break;
    }

    hwmsen_read_byte_sr(client, dump_reg_addr_table[idx], &reg_data);
    len += snprintf(buf + len, PAGE_SIZE - len, "%02x ", reg_data);
    APS_DBG("[%s]: REG ADDR = %02x, DATA = %02x\n", __func__, dump_reg_addr_table[idx], reg_data);

    idx = idx + 1;
  }
  
  len += snprintf(buf + len, PAGE_SIZE - len, "\n");
  return (len);
}
/*----------------------------------------------------------------------------*/
static int us5182_setup_eint(struct i2c_client *client)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);        

  g_us5182_ptr = obj;
  /*configure to GPIO function, external interrupt*/
  mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
  mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_GPIO);
  mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, GPIO_PULL_ENABLE);
  mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

#ifdef CUST_EINT_ALS_TYPE
  mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
  mt_eint_set_polarity(CUST_EINT_ALS_NUM, 0);
  mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, us5182_eint_func, 0);
 #else
 mt65xx_eint_set_sens(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
 mt65xx_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
 mt65xx_eint_set_polarity(CUST_EINT_ALS_NUM, 0);
 mt65xx_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_POLARITY, us5182_eint_func, 0);
#endif

#ifdef CUST_EINT_ALS_TYPE
	mt_eint_unmask(CUST_EINT_ALS_NUM);
 #else
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);  
#endif
  return (0);
}
/*----------------------------------------------------------------------------*/
static int us5182_enable_eint(struct i2c_client *client)
{
  if(g_us5182_ptr == NULL)
  {
    us5182_setup_eint(client);
  }

  mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
  mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
  mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, GPIO_PULL_ENABLE);
  mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

#ifdef CUST_EINT_ALS_TYPE
  mt_eint_unmask(CUST_EINT_ALS_NUM);
#else
  mt65xx_eint_unmask(CUST_EINT_ALS_NUM);
#endif

#ifdef  FEATURE_GET_EINT_EDGE
  us5182_eint_prev_state = mt_get_gpio_in(GPIO_ALS_EINT_PIN);
  APS_DBG("us5182_eint_prev_state = %d\n", us5182_eint_prev_state);
#endif  ///< end of FEATURE_GET_EINT_EDGE
#ifdef  FEATURE_GET_PROX_EDGE
  us5182_prox_prev_state = -1;
  APS_DBG("us5182_prox_prev_state = %d\n", us5182_prox_prev_state);
#endif  ///< end of FEATURE_GET_PROX_EDGE
  return (0);
}

static int us5182_disable_eint(struct i2c_client *client)
{
  mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
  mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_GPIO);
  mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, GPIO_PULL_ENABLE);
  mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);
 #ifdef CUST_EINT_ALS_TYPE
  mt_eint_mask(CUST_EINT_ALS_NUM);
 #else
		mt65xx_eint_mask(CUST_EINT_ALS_NUM);
 #endif
  return (0);
}

/*----------------------------------------------------------------------------*/
int us5182_get_timing(void)
{
  return (200);
}
/*----------------------------------------------------------------------------*/
/* resolution for als*/
static u8 us5182_get_als_resolution(struct i2c_client *client)
{   
  u8 res;
  
  hwmsen_read_byte_sr(client, REGS_CR01, &res); 
  res = (res & 0x18) >> 3;
    
  return (res);
}

static int us5182_set_als_resolution(struct i2c_client *client, int res)
{
  return (us5182_reg_write(client, REGS_CR01, CR1_ALS_RES_MASK, CR1_ALS_RES_SHIFT, res));
}

int us5182_read_als_data(struct i2c_client *client, u32 *data)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);    
  u32 ret = 0;
  u8 lsb;
  u8 msb;
  u8 bitdepth;

  if(hwmsen_read_byte_sr(client, REGS_LSB_SENSOR, &lsb))
  {
    APS_ERR("reads als lsb = %d\n", ret);
    return (-EFAULT);
  }
  
  if(hwmsen_read_byte_sr(client, REGS_MSB_SENSOR, &msb))
  {
    APS_ERR("reads als msb = %d\n", ret);
    return (-EFAULT);
  }
  
  bitdepth = us5182_get_als_resolution(client);
  
  switch(bitdepth)
  {
    case 0: 
      lsb &= 0xF0;
      break;
    case 1:
      lsb &= 0xFC;
      break;
    default:  
      break;
  }
  
  *data = (msb << 8) | lsb;
  
  if(atomic_read(&obj->trace) & CMC_TRC_ALS_DATA)
  {
    APS_LOG("ALS:  0x%04X\n", (u32)(*data));
  }
  return (0);
}
/*----------------------------------------------------------------------------*/
static u8 us5182_get_ps_resolution(struct i2c_client *client)
{   
  u8 res;
  
  hwmsen_read_byte_sr(client, REGS_CR02, &res); 
  res = (res & 0x18) >> 3;
    
  return (res);
}

static int us5182_set_ps_resolution(struct i2c_client *client, int res)
{
  return (us5182_reg_write(client, REGS_CR02, CR2_PS_RES_MASK, CR2_PS_RES_SHIFT, res));
}

int us5182_read_ps_data(struct i2c_client *client, u32 *data)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);    
  u32 ret = 0;
  u8 lsb;
  u8 msb;
  u8 bitdepth;

  if(hwmsen_read_byte_sr(client, REGS_LSB_SENSOR_PS, &lsb))
  {
    APS_ERR("reads ps lsb = %d\n", ret);
    return (-EFAULT);
  }
  
  if(hwmsen_read_byte_sr(client, REGS_MSB_SENSOR_PS, &msb))
  {
    APS_ERR("reads ps msb = %d\n", ret);
    return (-EFAULT);
  }
  
  bitdepth = us5182_get_ps_resolution(client);
  
  switch(bitdepth)
  {
    case 0: 
      lsb &= 0xF0;
      break;
    case 1:
      lsb &= 0xFC;
      break;
    default:  
      break;
  }
  
  *data = (msb << 8) | lsb;
  APS_DBG("us5182_read_ps_data: %x", *data);
  if(atomic_read(&obj->trace) & CMC_TRC_PS_DATA)
  {
    APS_LOG("PS:  0x%04X\n", *data);
  }
  return (0);    
}
/*----------------------------------------------------------------------------*/
static cmd_table_type us5182_init_device_cmd[] =
{
  { REGS_CR10,              0x08, },  ///< LED current always on, 50/60 inactive 
  { REGS_CR29,              0x10, },  ///< ALS code < Dark_Threshold, dark code offset cancelling gain
  { REGS_CR2A,              0x00, },  ///< ALS code > Dark_Threshold, dark code offset cancelling gain
  { REGS_CR27,              0x85, },  ///< Dark_Threshold high byte
  { REGS_CR28,              0x00, },  ///< Dark_Threshold low byte
  { REGS_CR2B,              0x80, },  ///< Dark auto-cancelling enable
  { REGS_CR01,              0x00, },  ///< ALS_FQ once, ALS 12bit; als gain 1
  { REGS_CR16,              0x14, },  ///< ALS Dark code offset cancelling gain 1.25
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
  { REGS_CR02,              0x00, },  ///< PS_FQ 1, INT level, PS 12bit; PS gain*1
#else   ///< else of FEATURE_PS_ISR_SW_FAULT_Q
  { REGS_CR02,              0x80, },  ///< PS_FQ 8, INT level, PS 12bit; PS gain*1
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
#ifdef  FEATURE_IRLED_100MA
  { REGS_CR03,              0x38, },  ///< 0-slots wait time; LED_DR 100ma; PS only
#endif  ///< end of FEATURE_IRLED_100MA
#ifdef  FEATURE_IRLED_50MA
  { REGS_CR03,              0x28, },  ///< 0-slots wait time; LED_DR 50ma; PS only
#endif  ///< end of FEATURE_IRLED_50MA
#ifdef  FEATURE_IRLED_25MA
  { REGS_CR03,              0x18, },  ///< 0-slots wait time; LED_DR 25ma; PS only
#endif  ///< end of FEATURE_IRLED_25MA
#ifdef  FEATURE_IRLED_12MA
  { REGS_CR03,              0x08, },  ///< 0-slots wait time; LED_DR 12ma; PS only
#endif  ///< end of FEATURE_IRLED_12MA
  { REGS_CR17,              0x20, },  ///< PS Dark code offset cancelling gain 1
  { REGS_CR20,              0x00, },  ///< 56.32kHz x 8
  { REGS_INT_LSB_TH_LO,     0x00, },  ///< ALS Low_Threshold low byte
  { REGS_INT_MSB_TH_LO,     0x00, },  ///< ALS Low_Threshold high byte
  { REGS_INT_LSB_TH_HI,     0xFF, },  ///< ALS High_Threshold low byte
  { REGS_INT_MSB_TH_HI,     0xFF, },  ///< ALS High_Threshold high byte
  { REGS_INT_LSB_TH_LO_PS,  0xF4, },  ///< PS Low_Threshold low byte
  { REGS_INT_MSB_TH_LO_PS,  0x01, },  ///< PS Low_Threshold high byte
  { REGS_INT_LSB_TH_HI_PS,  0xE8, },  ///< PS High_Threshold low byte
  { REGS_INT_MSB_TH_HI_PS,  0x03, },  ///< PS High_Threshold high byte
  { REGS_CR00,              0x10, },  ///< ALS only
  { REGS_CR21,              0x01, },  ///< Core reset
  { 0xFF,                   0xFF, },
};

int us5182_init_device(struct i2c_client *client)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);        
  u8 uMsb;
  u8 uLsb;
  u8 uIdx;

  APS_DBG("us5182_init_device.........\r\n");

  uIdx = 0;
  while(1)
  {
    if(hwmsen_write_byte_sr(client, us5182_init_device_cmd[uIdx].reg, us5182_init_device_cmd[uIdx].data))
    {
      APS_ERR("write %02x to register %02x fail\n", us5182_init_device_cmd[uIdx].data, us5182_init_device_cmd[uIdx].reg);
      return (-EFAULT);
    }

    uIdx = uIdx + 1;
    if((us5182_init_device_cmd[uIdx].reg == 0xFF) &&
       (us5182_init_device_cmd[uIdx].data == 0xFF))
    {
      break;
    }
  }

  if(obj->hw->polling_mode_ps == 0)
  { 
    APS_DBG("interrupt mode\n");

    uLsb = (u8)(obj->hw->ps_threshold_low & 0x00FF);
    uMsb = (u8)(obj->hw->ps_threshold_low >> 8);
    APS_DBG("PS low threshold : 0x%02X%02X\n", uMsb, uLsb);
    /// PS high and low threshold level setting
    if(hwmsen_write_byte_sr(client, REGS_INT_LSB_TH_LO_PS, uLsb)) 
    {
      return (-EFAULT);
    }
    if(hwmsen_write_byte_sr(client, REGS_INT_MSB_TH_LO_PS, uMsb)) 
    {
      return (-EFAULT);
    }

    uLsb = (u8)(obj->hw->ps_threshold_high & 0x00FF);
    uMsb = (u8)(obj->hw->ps_threshold_high >> 8);
    APS_DBG("PS high threshold :  0x%02X%02X\n", uMsb, uLsb);
    if(hwmsen_write_byte_sr(client, REGS_INT_LSB_TH_HI_PS, uLsb))
    {
      return (-EFAULT);
    }
    if(hwmsen_write_byte_sr(client, REGS_INT_MSB_TH_HI_PS, uMsb)) 
    {
      return (-EFAULT);
    }
  }     
  return (0);
}
/*----------------------------------------------------------------------------*/
static void us5182_power(struct alsps_hw *hw, unsigned int on) 
{
  static unsigned int power_on = 0;

  if(hw->power_id != POWER_NONE_MACRO)
  {
    if(power_on == on)
    {
      APS_DBG("ignore power control: %d\n", on);
    }
    else if(on)
    {
      if(!hwPowerOn(hw->power_id, hw->power_vol, "US5182")) 
      {
        APS_ERR("power on fails!!\n");
      }
    }
    else
    {
      if(!hwPowerDown(hw->power_id, "US5182")) 
      {
        APS_ERR("power off fail!!\n");   
      }
    }
  }
  power_on = on;
}
/*----------------------------------------------------------------------------*/
static int power_saving_wait_sel(struct i2c_client *client, bool bSwitch)
{
#ifdef  FEATURE_POWER_SAVE_WAIT_SEL
  u8 regData;

  if(hwmsen_read_byte_sr(client, REGS_CR03, &regData))
  {
    APS_ERR("Read 0x%02x fail\n", REGS_CR03);
    return (-1);
  }
  if(bSwitch == true)
  {
    regData = regData | 0b11000000;
    APS_DBG("Set Wait_Sel[1:0] = 11\n");
  }
  else
  {
    regData = regData & 0b00111111;
    APS_DBG("Set Wait_Sel[1:0] = 00\n");
  }
  if(hwmsen_write_byte_sr(client, REGS_CR03, regData))
  {
    APS_ERR("Write 0x%02x to 0x%02x fail\n", regData, REGS_CR03);
    return (-1);
  }
#endif  ///< end of FEATURE_POWER_SAVE_WAIT_SEL
  return (0);
}
//*----------------------------------------------------------------------------*/
static int power_saving_ledri_sel(struct i2c_client *client, bool bSwitch)
{
#ifdef  FEATURE_POWER_SAVE_LEDRI_SEL
  u8 regData;

  if(hwmsen_read_byte_sr(client, REGS_CR03, &regData))
  {
    APS_ERR("Read 0x%02x fail\n", REGS_CR03);
    return (-1);
  }
  if(bSwitch == true)
  {
    regData = regData & 0b11001111;
    APS_DBG("Set LEDri_Sel[1:0] = 00\n");
  }
  else
  {
    regData = regData | 0b00110000;
    APS_DBG("Set LEDri_Sel[1:0] = 11\n");
  }
  if(hwmsen_write_byte_sr(client, REGS_CR03, regData))
  {
    APS_ERR("Write 0x%02x to 0x%02x fail\n", regData, REGS_CR03);
    return (-1);
  }
#endif  ///< end of FEATURE_POWER_SAVE_LEDRI_SEL
  return (0);
}
//*----------------------------------------------------------------------------*/
static int power_saving_freq_sel(struct i2c_client *client, bool bSwitch, bool bPSEnable)
{
#ifdef  FEATURE_POWER_SAVE_FREQ_SEL
  u8 regData;

  if(hwmsen_read_byte_sr(client, REGS_CR10, &regData))
  {
    APS_ERR("Read 0x%02x fail\n", REGS_CR10);
    return (-1);
  }
  if(bSwitch == true)
  {
    regData = regData | 0b00000110;
    regData = regData & 0b11110111;
    APS_DBG("Set FREQ_Sel[2:0] = 011\n");
  }
  else
  {
    if(bPSEnable == true)
    {
      regData = regData & 0b11111001;
      regData = regData | 0b00001000;
      APS_DBG("Set FREQ_Sel[2:0] = 100\n");
    }
    else
    {
      regData = regData & 0b11110001;
      APS_DBG("Set FREQ_Sel[2:0] = 000\n");
    }
  }
  if(hwmsen_write_byte_sr(client, REGS_CR10, regData))
  {
    APS_ERR("Write 0x%02x to 0x%02x fail\n", regData, REGS_CR03);
    return (-1);
  }
#endif  ///< end of FEATURE_POWER_SAVE_FREQ_SEL
  return (0);
}
///*----------------------------------------------------------------------------*/
static int power_saving_ckrate_sel(struct i2c_client *client, bool bSwitch)
{
#ifdef  FEATURE_POWER_SAVE_CKRATE_SEL
  u8 regData;

  if(hwmsen_read_byte_sr(client, REGS_CR20, &regData))
  {
    APS_ERR("Read 0x%02x fail\n", REGS_CR20);
    return (-1);
  }
  if(bSwitch == true)
  {
    regData = regData | 0b11000000;
    APS_DBG("Set CKRate_Sel[1:0] = 11\n");
  }
  else
  {
    regData = regData & 0b00111111;
    APS_DBG("Set CKRate_Sel[1:0] = 00\n");
  }
  if(hwmsen_write_byte_sr(client, REGS_CR20, regData))
  {
    APS_ERR("Write 0x%02x to 0x%02x fail\n", regData, REGS_CR20);
    return (-1);
  }
#endif  ///< end of FEATURE_POWER_SAVE_CKRATE_SEL
  return (0);
}
///*----------------------------------------------------------------------------*/
static int us5182_power_saving(struct i2c_client *client, bool bSwitch, bool bPSEnable)
{
  /// [AT-PM] : Set Wait_Sel[1:0] ; 09/01/2014
  if(power_saving_wait_sel(client, bSwitch))
  {
    return (-1);
  }

  /// [AT-PM] : Set LEDri_Sel[1:0] ; 09/11/2014
  if(power_saving_ledri_sel(client, bSwitch))
  {
    return (-1);
  }

  /// [AT-PM] : Set FREQ_Sel[2:0] ; 09/11/2014
  if(power_saving_freq_sel(client, bSwitch, bPSEnable))
  {
    return (-1);
  }

  /// [AT-PM] : Set CKRate_Sel[1:0] ; 09/01/2014
  if(power_saving_ckrate_sel(client, bSwitch))
  {
    return (-1);
  }
  return (0);
}
/*----------------------------------------------------------------------------*/
static void us5182_core_rst_work_func(struct work_struct *work)
{
  struct us5182_priv *obj = (struct us5182_priv *)container_of((struct delayed_work *)work, struct us5182_priv, core_rst_work);

  APS_FUN();
  atomic_set(&obj->core_rst, 0);
}
/*----------------------------------------------------------------------------*/
static int us5182_core_reset(struct i2c_client *client)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);

  APS_FUN();

  cancel_delayed_work_sync(&obj->core_rst_work);

  if(hwmsen_write_byte_sr(client, REGS_CR21, 0x01))
  {
    APS_ERR("write core reset err!\n");
    return (-1);
  }

  atomic_set(&obj->core_rst, 1);
  schedule_delayed_work(&obj->core_rst_work, 1 * HZ);
  return (0);
}
/*----------------------------------------------------------------------------*/
static void us5182_chk_last_ps_data(int *last_ps_data, hwm_sensor_data *sensor_data)
{
  if((*last_ps_data) != sensor_data->values[0])
  {
    APS_ERR("PS STATE changed from %d to %d\n", *last_ps_data, sensor_data->values[0]);
  }
  else
  {
    APS_LOG("PS STATE keeped at %d\n", *last_ps_data);
  }
  *last_ps_data = sensor_data->values[0];
}
/*----------------------------------------------------------------------------*/
static void us5182_get_als_work_func(struct work_struct *work)
{
  struct us5182_priv *obj = (struct us5182_priv *)container_of((struct delayed_work *)work, struct us5182_priv, get_als_work);
  int err;

  mutex_lock(&obj->op_mutex);

  APS_FUN();

  err = us5182_read_als_data(obj->client, &obj->als);
  if(err != 0)
  {
    APS_ERR("[%s]: us5182_read_als_data error = %d\n", __func__, err);
    goto EXIT_GET_ALS_WORK;
  }

  obj->als_data = us5182_get_als_value(obj, obj->als);
  if(obj->als_data == -1)
  {
    APS_ERR("[%s]: us5182_get_als_value error = %d\n", __func__, obj->als_data);
    goto EXIT_GET_ALS_WORK;
  }
  APS_LOG("[%s]: ALS DATA = %d (%d)\n", __func__, obj->als_data, obj->als);

EXIT_GET_ALS_WORK:
  schedule_delayed_work(&obj->get_als_work, msecs_to_jiffies(US5182_GET_ALS_INTERVAL_MS));

  mutex_unlock(&obj->op_mutex);
}
/*----------------------------------------------------------------------------*/
static void us5182_get_ps_work_func(struct work_struct *work)
{
  struct us5182_priv *obj = (struct us5182_priv *)container_of((struct delayed_work *)work, struct us5182_priv, get_ps_work);
  u32 current_jiffies;
  u32 delta_jiffies;
  int ps_data;
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
  hwm_sensor_data sensor_data;
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
 
  mutex_lock(&obj->op_mutex);

  APS_FUN();

  ps_data = obj->ps_data_prev_raw;

  if(us5182_read_ps_data(obj->client, &obj->ps) != 0)
  {
    APS_ERR("us5182 read ps code error\n");
    goto GET_PS_WORK_END;
  }

  /// [AT-PM] : Get delta time ; 09/12/2014
  current_jiffies = jiffies_to_msecs(jiffies);
  if(current_jiffies >= obj->ps_last_jiffies)
  {
    delta_jiffies = current_jiffies - obj->ps_last_jiffies;
  }
  else
  {
    delta_jiffies = current_jiffies;
  }
  obj->ps_last_jiffies = current_jiffies;

  /// [AT-PM] : Get PS data ; 09/12/2014
  ps_data = us5182_get_ps_value(obj, obj->ps);

  /// [AT-PM] : Software fault queue ; 09/12/2014
  if(ps_data == 0)
  {
    obj->ps_high_time = 0;
    obj->ps_low_time = obj->ps_low_time + delta_jiffies;

    if(obj->ps_low_time > US5182_GET_PS_SW_FQ_MS)
    {
      if(obj->ps_data != ps_data)
      {
        if(obj->ps_data > 0)
        {
          obj->ps_high_to_low = obj->ps_high_to_low + 1;
        }

        obj->ps_data = ps_data;
        APS_LOG("SW_FQ set PS to LOW (%d)\n", obj->ps_low_time);
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
        if(obj->hw->polling_mode_ps == 0)
        {
          sensor_data.values[0] = ps_data;
          sensor_data.value_divide = 1;
          sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
          APS_LOG("[%s]: PS DATA = %d\n", __func__, sensor_data.values[0]);

          if(hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data))
          {
            APS_ERR("call hwmsen_get_interrupt_data fail\n");
          }
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q_ONCE
          else
          {
            us5182_chk_last_ps_data(&obj->last_report_ps, &sensor_data);
            goto GET_PS_WORK_EXIT;
          }
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q_ONCE
          us5182_chk_last_ps_data(&obj->last_report_ps, &sensor_data);
        }
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
      }

      obj->ps_low_time = US5182_GET_PS_SW_FQ_MS;
    }
  }
  else if(ps_data < 0)
  {
    APS_LOG("Invalid ps value (%d)\n", ps_data);
  }
  else
  {
    obj->ps_high_time = obj->ps_high_time + delta_jiffies;
    obj->ps_low_time = 0;

    if(obj->ps_high_time > US5182_GET_PS_SW_FQ_MS)
    {
      if(obj->ps_data != ps_data)
      {
        if(obj->ps_data == 0)
        {
          obj->ps_low_to_high = obj->ps_low_to_high + 1;
        }

        obj->ps_data = ps_data;
        APS_LOG("SW_FQ set PS to HIGH (%d)\n", obj->ps_high_time);
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
        if(obj->hw->polling_mode_ps == 0)
        {
          sensor_data.values[0] = ps_data;
          sensor_data.value_divide = 1;
          sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
          APS_LOG("[%s]: PS DATA = %d\n", __func__, sensor_data.values[0]);

          if(hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data))
          {
            APS_ERR("call hwmsen_get_interrupt_data fail\n");
          }
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q_ONCE
          else
          {
            us5182_chk_last_ps_data(&obj->last_report_ps, &sensor_data);
            goto GET_PS_WORK_EXIT;
          }
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q_ONCE
          us5182_chk_last_ps_data(&obj->last_report_ps, &sensor_data);
        }
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
      }

      obj->ps_high_time = US5182_GET_PS_SW_FQ_MS;
    }
  }

GET_PS_WORK_END:
  /// [AT-PM] : Set for next sampling ; 09/12/2014
#ifdef  FEATURE_SW_FAULT_Q_DYNA_POLL
  if(obj->ps_data != -1)
  {
    if(obj->ps_data_prev_raw == ps_data)
    {
      obj->ps_poll_time = obj->ps_poll_time * 2;
      if(obj->ps_poll_time > (US5182_GET_PS_SW_FQ_MS * US5182_GET_PS_MS_TO_NS))
      {
        obj->ps_poll_time = US5182_GET_PS_SW_FQ_MS;
        obj->ps_poll_time = obj->ps_poll_time * US5182_GET_PS_MS_TO_NS;
        obj->ps_poll_time = obj->ps_poll_time * 8;
        obj->ps_poll_time = obj->ps_poll_time / 10;
      }
      APS_DBG("[UPI_DEBUG] SAME - %d %d -> %d\n", obj->ps_data_prev_raw, ps_data, obj->ps_poll_time);
    }
    else
    {
      obj->ps_poll_time = US5182_GET_PS_INTERVAL_NS;
      APS_DBG("[UPI_DEBUG] DIFF - %d %d -> %d\n", obj->ps_data_prev_raw, ps_data, obj->ps_poll_time);
      obj->ps_data_prev_raw = ps_data;
    }
  }
#else   ///< else of FEATURE_SW_FAULT_Q_DYNA_POLL
  obj->ps_poll_time = US5182_GET_PS_INTERVAL_NS;
#endif  ///< end of FEATURE_SW_FAULT_Q_DYNA_POLL
  hrtimer_start(&obj->get_ps_timer, ktime_set(0, obj->ps_poll_time), HRTIMER_MODE_REL);
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q_ONCE
GET_PS_WORK_EXIT:
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q_ONCE
  mutex_unlock(&obj->op_mutex);
}
/*----------------------------------------------------------------------------*/
static enum hrtimer_restart us5182_get_ps_timer_func(struct hrtimer *timer)
{
  schedule_delayed_work(&us5182_obj->get_ps_work, 0);
  return (HRTIMER_NORESTART);
}
/*----------------------------------------------------------------------------*/
static int us5182_get_ps_start(struct us5182_priv *obj)
{
  obj->ps_last_jiffies = jiffies_to_msecs(jiffies);
  obj->ps_high_time = 0;
  obj->ps_low_time = 0;
  obj->ps_high_to_low = 0;
  obj->ps_low_to_high = 0;
  obj->ps_data = -1;
  obj->ps_data_prev_raw = -1;
  obj->ps_get_start = true;
  obj->ps_poll_time = US5182_GET_PS_INTERVAL_NS;

  hrtimer_start(&obj->get_ps_timer, ktime_set(0, obj->ps_poll_time), HRTIMER_MODE_REL);
  APS_LOG("start SW_FQ timer\n");
  return (0);
}
/*----------------------------------------------------------------------------*/
static int us5182_get_ps_stop(struct us5182_priv *obj)
{
  if(obj->ps_get_start == true)
  {
    obj->ps_get_start = false;
    cancel_delayed_work_sync(&obj->get_ps_work);
    hrtimer_cancel(&obj->get_ps_timer);
    APS_LOG("stop SW_FQ timer\n");
  }
  return (0);
}
/*----------------------------------------------------------------------------*/
static int us5182_enable_als(struct i2c_client *client, bool enable)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);
  u8 regData;
  u8 prevCR00;

  APS_LOG("us5182_enable_als %d \n", enable); 
  
  if(hwmsen_read_byte_sr(client, REGS_CR00, &prevCR00))
  {
    APS_ERR("read APS_CONFIGUATION register err!\n");
    return (-1);
  }

  /// [AT-PM] : First set 00 to clear bit ; 08/29/2014
  regData = prevCR00 & (0b01001111);
  
  if(enable == TRUE)
  {
    APS_DBG("first enable als!\n");
    if(obj->ps_enable == true)
    {
      APS_DBG("ALS(1): enable both \n");
    }
    else
    {
      APS_DBG("ALS(1): enable als only \n");
      regData |= 0b00010000;
    }
    atomic_set(&obj->als_deb_on, 1);
    atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)/(1000/HZ));
  }
  else
  {
    if(obj->ps_enable == true)
    {
      APS_DBG("ALS(0):enable ps only \n");
      regData |= 0b00100000;
    }
    else
    {
      APS_DBG("ALS(0): disable both \n");
      regData |= 0b10010000;
    }
    atomic_set(&obj->als_deb_on, 0);
  }
  
  if(us5182_power_saving(client, (regData & 0x80 ? true : false), (obj->ps_enable == true ? true : false)))
  {
    APS_ERR("us5182_power_saving fail\n");
    return (-1);
  }
  regData = regData & 0b01111111;

#ifdef  FEATURE_NO_PS_ONLY_OP_MODE
  /// [AT-PM] : Remove PS ONLY mode ; 10/28/2014
  if(regData & 0b00100000)
  {
    regData = regData & (0b01001111);
    APS_DBG("remove PS ONLY mode (%x)\n", regData);
  }
#endif  ///< end of FEATURE_NO_PS_ONLY_OP_MODE

  if(prevCR00 != regData)
  {
    if(hwmsen_write_byte_sr(client, REGS_CR00, regData))
    {
      APS_ERR("us5182_enable_als failed!\n");
      return (-1);
    }

    us5182_core_reset(client);
  }

  obj->als_enable = enable;
  return (0);
}
/*----------------------------------------------------------------------------*/
static int us5182_enable_ps(struct i2c_client *client, bool enable)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);
  int res;
  u8 regData;
  u8 prevCR00;
  
  APS_LOG("us5182_enable_ps %d\n", enable); 

  if(hwmsen_read_byte_sr(client, REGS_CR00, &prevCR00))
  {
    APS_ERR("read APS_CONFIGUATION register err!\n");
    return (-1);
  }
  /// [AT-PM] : First set 00 to clear bit ; 08/29/2014
  regData = prevCR00 & (0b01001111);
  
  if(enable == TRUE)
  {
    APS_DBG("first enable ps!\n");

    if(obj->als_enable == true)
    {
      APS_DBG("PS(1): enable ps both !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    else
    {
      regData |= 0b00100000;
      APS_DBG("PS(1): enable ps only !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    atomic_set(&obj->ps_deb_on, 1);
    atomic_set(&obj->ps_deb_end, jiffies+atomic_read(&obj->ps_debounce)/(1000/HZ));
    obj->last_report_ps = -1;
  }
  else
  {
    if(obj->hw->polling_mode_ps == 0)
    { 
      mutex_unlock(&obj->op_mutex);
      us5182_get_ps_stop(obj);
      mutex_lock(&obj->op_mutex);

      res = us5182_disable_eint(client);
      if(res!=0)
      {
        APS_ERR("disable eint fail: %d\n", res);
        return (res);
      }
    } 
    else
    {
      mutex_unlock(&obj->op_mutex);
      us5182_get_ps_stop(obj);
      mutex_lock(&obj->op_mutex);
    }
    
    if(obj->als_enable == true)
    {
      APS_DBG("PS(0): disable ps only enalbe als !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      regData |= 0b00010000;
    }
    else
    {
      APS_DBG("PS(0): disable both !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      regData |= 0b10010000;
    }
    atomic_set(&obj->ps_deb_on, 0);
  }
  
  if(us5182_power_saving(client, (regData & 0x80 ? true : false), (enable == TRUE ? true : false)))
  {
    APS_ERR("us5182_power_saving fail\n");
    return (-1);
  }
  regData = regData & 0b01111111;

#ifdef  FEATURE_NO_PS_ONLY_OP_MODE
  /// [AT-PM] : Remove PS ONLY mode ; 10/28/2014
  if(regData & 0b00100000)
  {
    regData = regData & (0b01001111);
    APS_DBG("remove PS ONLY mode (%x)\n", regData);
  }
#endif  ///< end of FEATURE_NO_PS_ONLY_OP_MODE

  if(prevCR00 != regData)
  {
    if(hwmsen_write_byte_sr(client, REGS_CR00, regData))
    {
      APS_ERR("us5182_enable_ps failed!\n");
      return (-1);
    }

    us5182_core_reset(client);
  }
    
  if(enable == TRUE)
  {
    if(obj->hw->polling_mode_ps == 0)
    {
      APS_DBG("enable interrupt mode\n");
      res = us5182_enable_eint(client);
      if(res!=0)
      {
        APS_ERR("enable eint fail: %d\n", res);
        return (res);
      }
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q_AT_START
      mutex_unlock(&obj->op_mutex);
      us5182_get_ps_start(obj);
      mutex_lock(&obj->op_mutex);
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q_AT_START
    }   
    else
    {
      mutex_unlock(&obj->op_mutex);
      us5182_get_ps_start(obj);
      mutex_lock(&obj->op_mutex);
    }
  }

  obj->ps_enable = enable;
  return (0);
}
/*----------------------------------------------------------------------------*/
static void us5182_enable_ps_work_func(struct work_struct *work)
{
  struct us5182_priv *obj = (struct us5182_priv *)container_of((struct delayed_work *)work, struct us5182_priv, get_ps_work);
  int err;

  err = us5182_enable_ps(obj->client, true);
  if(err != 0)
  {
    cancel_delayed_work_sync(&obj->enable_ps_work);
    schedule_delayed_work(&obj->enable_ps_work, msecs_to_jiffies(50));
    APS_ERR("disable ps fail: %d\n", err);
  }
}
/*----------------------------------------------------------------------------*/
static void us5182_disable_ps_work_func(struct work_struct *work)
{
  struct us5182_priv *obj = (struct us5182_priv *)container_of((struct delayed_work *)work, struct us5182_priv, get_ps_work);
  int err;

  err = us5182_enable_ps(obj->client, false);
  if(err != 0)
  {
    cancel_delayed_work_sync(&obj->disable_ps_work);
    schedule_delayed_work(&obj->disable_ps_work, msecs_to_jiffies(50));
    APS_ERR("disable ps fail: %d\n", err);
  }
}
/*----------------------------------interrupt functions--------------------------------*/
static int intr_flag = 0;
/*----------------------------------------------------------------------------*/
static int us5182_check_intr(struct i2c_client *client) 
{
  struct us5182_priv *obj = i2c_get_clientdata(client);
  int err;
  u8 data = 0;

  err = hwmsen_read_byte_sr(client, REGS_CR00, &data);
  APS_DBG("INT flag: = %x\n", data);

  if(err)
  {
    APS_ERR("WARNING: read int status: %d\n", err);
    return (0);
  }
  
  if(data & 0x08)   ///< bit3 = 1, approach
  {
    intr_flag = 0;  ///< for close
  }
  else              ///< bit3 = 0, not approach
  {
    intr_flag = 1;  ///< for away
  }
  APS_DBG("INT flag: = %x\n", intr_flag);
  
  if(atomic_read(&obj->trace) & CMC_TRC_DEBUG)
  {
    APS_LOG("check intr: 0x%08lX\n", obj->pending_intr);
  }

  return (0);
}
/*----------------------------------------------------------------------------*/
static int us5182_clear_ps_int(struct i2c_client *client)
{
  u8 regData;
  int rtn;

  if(hwmsen_read_byte_sr(client, REGS_CR00, &regData))
  {
    APS_ERR("read APS_CONFIGUATION register err!\n");
    return (-1);
  }
  rtn = (regData & 0b00001000) ? 1 : 0;

  /// [AT-PM] : Clear PS_INT_STATUS ; 10/17/2014
  APS_LOG("PS_INT_STATUS %x (%d)\n", regData, rtn);
  regData &= 0b11111011;

  if(hwmsen_write_byte_sr(client, REGS_CR00, regData))
  {
    APS_ERR("Clear PS_INT_STATUS failed!\n");
    return (-1);
  }
  return (rtn);
}
/*----------------------------------------------------------------------------*/
static int us5182_get_prox_state(struct i2c_client *client)
{
  u8 regData;

  if(hwmsen_read_byte_sr(client, REGS_CR00, &regData))
  {
    APS_ERR("read APS_CONFIGUATION register err!\n");
    return (-1);
  }
  return ((regData & 0b00001000) ? 1 : 0);
}
/*----------------------------------------------------------------------------*/
static int us5182_change_prox_int_type(struct i2c_client *client, int prox_state)
{
#ifdef  FEATURE_PROX_FOR_INT_TYPE
  u8 regData;

  if(hwmsen_read_byte_sr(client, REGS_CR03, &regData))
  {
    APS_ERR("read APS_SOURCE_CTL register err!\n");
    return (-1);
  }

  regData = regData & 0b11110011;
  if(prox_state)
  {
    regData = regData | 0b00001000;
  }
  else
  {
    regData = regData | 0b00001100;
  }

  if(hwmsen_write_byte_sr(client, REGS_CR03, regData))
  {
    APS_ERR("Update to %x failed!\n", regData);
    return (-1);
  }
#endif  ///< end of FEATURE_PROX_FOR_INT_TYPE
  return (0);
}
/*----------------------------------------------------------------------------*/
static void us5182_eint_func(void)
{
  struct us5182_priv *obj = g_us5182_ptr;

  if(!obj)
  {
    APS_ERR("NULL g_us5182_ptr\n");
    return;
  }
  
  if(obj->eint_init == true)
  {
    return;
  }

  APS_FUN();

 if(atomic_read(&obj->core_rst))
  {
    APS_LOG("interrupt caused by core_rst\n");
    atomic_set(&obj->core_rst, 0);
#ifdef  FEATURE_PS_ISR_CORE_RST_DELAY
    schedule_delayed_work(&obj->eint_work, msecs_to_jiffies(US5182_CORE_RESET_TIME));
#else   ///< else of FEATURE_PS_ISR_CORE_RST_DELAY
    schedule_delayed_work(&obj->eint_work, 0);
#endif  ///< end of FEATURE_PS_ISR_CORE_RST_DELAY
  }
  else
  {
    schedule_delayed_work(&obj->eint_work, 0);
  }
}
/*----------------------------------------------------------------------------*/
static void us5182_eint_work(struct work_struct *work)
{
  struct us5182_priv *obj = (struct us5182_priv *)container_of((struct delayed_work *)work, struct us5182_priv, eint_work);
#ifdef  FEATURE_PS_ISR
  hwm_sensor_data sensor_data;
#endif  ///< end of FEATURE_PS_ISR
  int res = 0;
  int eint_state;
  int prox_state;
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
  bool op_mutex_locked;
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
  
  mutex_lock(&obj->op_mutex);
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
  op_mutex_locked = true;
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q

  APS_FUN();

  eint_state = mt_get_gpio_in(GPIO_ALS_EINT_PIN);

  /// [AT-PM] : Clear interrupt status ; 10/17/2014
  if(eint_state == 0)
  {
    prox_state = us5182_clear_ps_int(obj->client);
  }
  else
  {
    prox_state = us5182_get_prox_state(obj->client);
  }
  if(prox_state < 0)
  {
    goto EINT_DISABLE;
  }

  if(obj->ps_enable == FALSE)
  {
    goto EINT_DISABLE;
  }

  /// [AT-PM] : Change interrupt attribute ; 10/24/2014
  res = us5182_change_prox_int_type(obj->client, prox_state);
  if(res < 0)
  {
    goto EINT_DISABLE;
  }

  /// [AT-PM] : Get the edge ; 10/23/2014
#ifdef  FEATURE_GET_EINT_EDGE
  if(us5182_eint_prev_state == eint_state)
  {
    APS_DBG("EINT SAME -> %d\n", us5182_eint_prev_state);
    goto EINT_WORK_EXIT;
  }
  APS_DBG("EINT DIFF -> %d <- %d\n", eint_state, us5182_eint_prev_state);
#endif  ///< end of FEATURE_GET_EINT_EDGE
#ifdef  FEATURE_GET_PROX_EDGE
  if(us5182_prox_prev_state == prox_state)
  {
    APS_DBG("PROX SAME -> %d\n", us5182_prox_prev_state);
    goto EINT_WORK_EXIT;
  }
  APS_DBG("PROX DIFF -> %d <- %d\n", prox_state, us5182_prox_prev_state);
#endif  ///< end of FEATURE_GET_PROX_EDGE
#ifdef  FEATURE_PS_ISR
  if((res = us5182_read_ps_data(obj->client,&obj->ps)))
  {
    APS_ERR("us5182 read ps data error %d\n", res);
    goto EINT_WORK_EXIT;
  }
  sensor_data.values[0] = us5182_get_ps_value(obj, obj->ps);
  sensor_data.value_divide = 1;
  sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
  APS_LOG("[%s]: PS DATA = %d\n", __func__, sensor_data.values[0]);

  if(sensor_data.values[0] < 0)
  {
    APS_ERR("Invalid ps value = %d\n", sensor_data.values[0]);
    goto EINT_WORK_EXIT;
  }
  else if(sensor_data.values[0] > 0)
  {
    obj->polarity = 1;
    obj->eint_high_count = obj->eint_high_count + 1;
  }
  else
  {
    obj->polarity = 0;
    obj->eint_low_count = obj->eint_low_count + 1;
  }

  if((res = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
  {
    APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", res);
    goto EINT_WORK_EXIT;
  }
  us5182_chk_last_ps_data(&obj->last_report_ps, &sensor_data);
#endif  ///< end of FEATURE_PS_ISR
  obj->eint_count = obj->eint_count + 1;
  APS_LOG("[%s]: POLARITY = %d (%d,%d,%d)\n", __func__, 
          obj->polarity,
          obj->eint_count,
          obj->eint_high_count,
          obj->eint_low_count);
#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
  mutex_unlock(&obj->op_mutex);
  op_mutex_locked = false;

  us5182_get_ps_stop(obj);
  us5182_get_ps_start(obj);
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
#if defined(FEATURE_GET_EINT_EDGE) || defined(FEATURE_GET_PROX_EDGE) || defined(FEATURE_PS_ISR)
EINT_WORK_EXIT:
#endif  ///< end of defined(FEATURE_GET_EINT_EDGE) || defined(FEATURE_GET_PROX_EDGE) || defined(FEATURE_PS_ISR)
#ifdef  FEATURE_GET_EINT_EDGE
  us5182_eint_prev_state = eint_state;
#endif  ///< end of FEATURE_GET_EINT_EDGE
#ifdef  FEATURE_GET_PROX_EDGE
  us5182_prox_prev_state = prox_state;
#endif  ///< end of FEATURE_GET_PROX_EDGE
EINT_DISABLE:
#ifdef CUST_EINT_ALS_TYPE
  mt_eint_unmask(CUST_EINT_ALS_NUM);
#else
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);
#endif

#ifdef  FEATURE_PS_ISR_SW_FAULT_Q
  if(op_mutex_locked == false)
  {
    return;
  }
#endif  ///< end of FEATURE_PS_ISR_SW_FAULT_Q
  mutex_unlock(&obj->op_mutex);
}
/*----------------------------------------------------------------------------*/
static int us5182_init_client(struct i2c_client *client)
{
  struct us5182_priv *obj = i2c_get_clientdata(client);
  int err = 0;
  APS_DBG("us5182_init_client.........\r\n");

  if((err = us5182_init_device(client)))
  {
    APS_ERR("init dev error: %d\n", err);
    return (err);
  }

  if(obj->hw->polling_mode_ps == 0)
  {
    us5182_setup_eint(client);
    us5182_disable_eint(client);
    obj->eint_init = false;
  }
  return (err);
}
/******************************************************************************

 * Sysfs attributes
*******************************************************************************/
static int als_open_report_data(int open)
{
		 //should queuq work to report event if  is_report_input_direct=true
		 return 0;
}
static int als_enable_nodata(int en)
{
	int res = 0;
    APS_LOG("us5182_obj als enable value = %d\n", en);
	if(!us5182_obj)
	{
		APS_ERR("us5182_obj is null!!\n");
		return -1;
	}
	res=us5182_enable_als(us5182_obj->client, en);
	if(res){
		APS_ERR("als_enable_nodata is failed!!\n");
		return -1;
	}
	return 0;
}
	 
static int als_set_delay(u64 ns)
{
		 return 0;
}

static int als_get_data(int* value, int* status)
{
	int err = 0;
    struct us5182_priv *obj = NULL;
	if(!us5182_obj)
	{
		APS_ERR("us5182_obj is null!!\n");
		return -1;
	}
	obj = us5182_obj;
	if((err = us5182_read_als_data(obj->client, &obj->als)))
	{
		err = -1;
	}
	else
	{
		*value = us5182_get_als_value(obj, obj->als);
		*status = SENSOR_STATUS_ACCURACY_MEDIUM;
	}

	return err;
}

static int ps_open_report_data(int open)
{
	//should queuq work to report event if  is_report_input_direct=true
	return 0;
}
static int ps_enable_nodata(int en)
{
	int res = 0;
    APS_LOG("us5182_obj als enable value = %d\n", en);
	if(!us5182_obj)
	{
		APS_ERR("us5182_obj is null!!\n");
		return -1;
	}
	res= us5182_enable_ps(us5182_obj->client, en);    
	if(res){
		APS_ERR("als_enable_nodata is failed!!\n");
		return -1;
	}
	return 0;
}

static int ps_set_delay(u64 ns)
{
	return 0;
}

static int ps_get_data(int* value, int* status)
{
    int err = 0;
    if(!us5182_obj)
	{
		APS_ERR("us5182_obj is null!!\n");
		return -1;
	}
    if((err = us5182_read_ps_data(us5182_obj->client, &us5182_obj->ps)))
    {
        err = -1;;
    }
    else
    {
        *value = us5182_get_ps_value(us5182_obj, us5182_obj->ps);
        *status = SENSOR_STATUS_ACCURACY_MEDIUM;
    }
    
	return 0;
}

static ssize_t us5182_show_config(struct device_driver *ddri, char *buf)
{
  ssize_t res;
  
  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  res = snprintf(buf, PAGE_SIZE, "(%d %d %d %d %d)\n", 
                 atomic_read(&us5182_obj->i2c_retry), 
                 atomic_read(&us5182_obj->als_debounce), 
                 atomic_read(&us5182_obj->ps_mask), 
                 us5182_obj->ps_thd_val, 
                 atomic_read(&us5182_obj->ps_debounce));
  return (res);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_store_config(struct device_driver *ddri, const char *buf, size_t count)
{
  int retry;
  int als_deb;
  int ps_deb;
  int mask;
  int thres;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  if(5 == sscanf(buf, "%d %d %d %d %d", 
                 &retry, 
                 &als_deb, 
                 &mask, 
                 &thres, 
                 &ps_deb))
  { 
    atomic_set(&us5182_obj->i2c_retry, retry);
    atomic_set(&us5182_obj->als_debounce, als_deb);
    atomic_set(&us5182_obj->ps_mask, mask);
    us5182_obj->ps_thd_val= thres;        
    atomic_set(&us5182_obj->ps_debounce, ps_deb);
  }
  else
  {
    APS_ERR("invalid content: '%s', length = %d\n", buf, count);
  }
  return (count);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_trace(struct device_driver *ddri, char *buf)
{
  ssize_t res;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }

  res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&us5182_obj->trace));     
  return (res);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_store_trace(struct device_driver *ddri, const char *buf, size_t count)
{
  int trace;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  if(1 == sscanf(buf, "0x%x", &trace))
  {
    atomic_set(&us5182_obj->trace, trace);
  }
  else 
  {
    APS_ERR("invalid content: '%s', length = %d\n", buf, count);
  }
  return (count);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_als(struct device_driver *ddri, char *buf)
{
  int res;
  u32 dat;
  int len;
  
  APS_FUN();

  dat = 0;

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  if((res = us5182_read_als_data(us5182_obj->client, &us5182_obj->als)))
  {
    return (snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res));
  }
  else
  {
    len = snprintf(buf, PAGE_SIZE, "0x%04X ", us5182_obj->als);

    dat = us5182_get_als_value(us5182_obj, us5182_obj->als);
    len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", dat);
    return (len);
  }
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_ps(struct device_driver *ddri, char *buf)
{
  ssize_t res;
  u32 dat;
  int len;

  APS_FUN();

  dat = 0;
  
  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  if((res = us5182_read_ps_data(us5182_obj->client, &us5182_obj->ps)))
  {
    return (snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res));
  }
  else
  {
    len = snprintf(buf, PAGE_SIZE, "0x%04X ", us5182_obj->ps);
    
    dat = us5182_get_ps_value(us5182_obj, us5182_obj->ps);
    if(dat == 0)
    {
      len += snprintf(buf + len, PAGE_SIZE - len, "L\n");
    }
    else
    {
      len += snprintf(buf + len, PAGE_SIZE - len, "H\n");
    }
    return (len);
  }
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_reg(struct device_driver *ddri, char *buf)
{
  int len;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  /*read*/
  len = us5182_dumpReg(us5182_obj->client, buf);
  APS_DBG("%d : %s", len, buf);
  return (len);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_reg_addr(struct device_driver *ddri, char *buf)
{
  int len;
  int idx;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  idx = 0;
  len = 0;
  while(1)
  {
    if(dump_reg_addr_table[idx] == 0xFF)
    {
      break;
    }

    len += snprintf(buf + len, PAGE_SIZE - len, "%02x ", dump_reg_addr_table[idx]);

    idx = idx + 1;
  }

  len += snprintf(buf + len, PAGE_SIZE - len, "\n");
  return (len);
}
//*----------------------------------------------------------------------------*/
static ssize_t us5182_show_status(struct device_driver *ddri, char *buf)
{
  ssize_t len = 0;
  
  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  if(us5182_obj->hw)
  {
    len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d, (%d %d)\n", 
    us5182_obj->hw->i2c_num, us5182_obj->hw->power_id, us5182_obj->hw->power_vol);
    
  }
  else
  {
    len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
  }

  #ifdef MT6516
  len += snprintf(buf+len, PAGE_SIZE-len, "EINT: %d (%d %d %d %d)\n", 
                  mt_get_gpio_in(GPIO_ALS_EINT_PIN),
                  CUST_EINT_ALS_NUM, 
                  CUST_EINT_ALS_POLARITY, 
                  CUST_EINT_ALS_DEBOUNCE_EN, 
                  CUST_EINT_ALS_DEBOUNCE_CN);

  len += snprintf(buf+len, PAGE_SIZE-len, "GPIO: %d (%d %d %d %d)\n", 
                  GPIO_ALS_EINT_PIN, 
                  mt_get_gpio_dir(GPIO_ALS_EINT_PIN), 
                  mt_get_gpio_mode(GPIO_ALS_EINT_PIN), 
                  mt_get_gpio_pull_enable(GPIO_ALS_EINT_PIN), 
                  mt_get_gpio_pull_select(GPIO_ALS_EINT_PIN));
  #endif

  len += snprintf(buf+len, PAGE_SIZE-len, "MISC: %d %d (%d,%d)\n", 
                  atomic_read(&us5182_obj->als_suspend), 
                  atomic_read(&us5182_obj->ps_suspend),
                  us5182_obj->ps_high_to_low,
                  us5182_obj->ps_low_to_high);
  return (len);
}
/*----------------------------------------------------------------------------*/
static u8 us5182_i2c_reg_addr = 0;

static ssize_t us5182_show_i2c(struct device_driver *ddri, char *buf)
{
  u8 reg_data;
  int len;

  APS_FUN();

  hwmsen_read_byte_sr(us5182_obj->client, us5182_i2c_reg_addr, &reg_data);

  len = snprintf(buf, PAGE_SIZE, "%02x\n", reg_data);
  return (len);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_store_i2c(struct device_driver *ddri, const char *buf, size_t count)
{
  char *write_data;
  unsigned int tmp_data;
  u8 reg_data;

  APS_FUN();

  sscanf(buf, "%x", &tmp_data);
  us5182_i2c_reg_addr = (u8)tmp_data;

  write_data = strstr(buf, " ");
  if(write_data == NULL)
  {
    return (count);
  }

  sscanf(write_data + 1, "%x", &tmp_data);
  reg_data = (u8)tmp_data;

  hwmsen_write_byte_sr(us5182_obj->client, us5182_i2c_reg_addr, reg_data);
  return (count);
}
/*----------------------------------------------------------------------------*/
#define IS_SPACE(CH) (((CH) == ' ') || ((CH) == '\n'))
/*----------------------------------------------------------------------------*/
static int read_int_from_buf(struct us5182_priv *obj, const char* buf, size_t count,
                             u32 data[], int len)
{
  int idx = 0;
  char *cur = (char *)buf;
  char *end = (char *)(buf + count);

  while(idx < len)
  {
    while((cur < end) && IS_SPACE(*cur))
    {
      cur ++;       
    }

    if(1 != sscanf(cur, "%d", &data[idx]))
    {
      break;
    }

    idx ++; 
    while((cur < end) && !IS_SPACE(*cur))
    {
      cur ++;
    }
  }
  return (idx);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_alslv(struct device_driver *ddri, char *buf)
{
  ssize_t len = 0;
  int idx;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  for(idx = 0; idx < us5182_obj->als_level_num; idx ++)
  {
    len += snprintf(buf+len, PAGE_SIZE-len, "%d ", us5182_obj->hw->als_level[idx]);
  }
  len += snprintf(buf+len, PAGE_SIZE-len, "\n");
  return (len);
}

static ssize_t us5182_store_alslv(struct device_driver *ddri, const char *buf, size_t count)
{
  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  else if(!strcmp(buf, "def"))
  {
    memcpy(us5182_obj->als_level, us5182_obj->hw->als_level, sizeof(us5182_obj->als_level));
  }
  else if(us5182_obj->als_level_num != 
          read_int_from_buf(us5182_obj, buf, count, us5182_obj->hw->als_level, us5182_obj->als_level_num))
  {
    APS_ERR("invalid format: '%s'\n", buf);
  }    
  return (count);
} 
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_alsval(struct device_driver *ddri, char *buf)
{
  ssize_t len = 0;
  int idx;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  for(idx = 0; idx < us5182_obj->als_value_num; idx ++)
  {
    len += snprintf(buf+len, PAGE_SIZE-len, "%d ", us5182_obj->hw->als_value[idx]);
  }
  len += snprintf(buf+len, PAGE_SIZE-len, "\n");
  return (len);
}

static ssize_t us5182_store_alsval(struct device_driver *ddri, const char *buf, size_t count)
{
  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  else if(!strcmp(buf, "def"))
  {
    memcpy(us5182_obj->als_value, us5182_obj->hw->als_value, sizeof(us5182_obj->als_value));
  }
  else if(us5182_obj->als_value_num != 
          read_int_from_buf(us5182_obj, buf, count, us5182_obj->hw->als_value, us5182_obj->als_value_num))
  {
    APS_ERR("invalid format: '%s'\n", buf);
  }    
  return (count);
}
/*----------------------------------------------------------------------------*/
/**
 * @brief us5182_show_eint
 *
 * Show INT pin status
 *
 * @para ddri address of struct device_driver
 * @para buf address of char
 * @return size of buf
 */
static ssize_t us5182_show_eint(struct device_driver *ddri, char *buf)
{
  ssize_t len = 0;
  int eint_state;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }
  
  eint_state = mt_get_gpio_in(GPIO_ALS_EINT_PIN);

  len = snprintf(buf + len, PAGE_SIZE, "%d %d %d %d\n", 
                 us5182_obj->eint_count, 
                 us5182_obj->eint_high_count, 
                 us5182_obj->eint_low_count,
                 eint_state);

  us5182_obj->eint_count = 0;
  us5182_obj->eint_high_count = 0;
  us5182_obj->eint_low_count = 0;
  return (len);
}
/*----------------------------------------------------------------------------*/
#define US5182_ARGUMENT_LENGTH    (16)
#define US5182_ARGUMENT_COUNT     (4)
#define US5182_LINE_LENGTH        (2 * (US5182_ARGUMENT_COUNT * US5182_ARGUMENT_LENGTH))

typedef struct us5182_command_st {
  char command[US5182_ARGUMENT_LENGTH];
  ssize_t (*show)(char *buf);
  ssize_t (*store)(void);
} us5182_command_type;

typedef struct us5182_command_package_st {
  char line[US5182_LINE_LENGTH];

  int argc;
  char argv[US5182_ARGUMENT_COUNT][US5182_ARGUMENT_LENGTH];
} us5182_command_package_type;

static us5182_command_package_type us5182_command_package;

typedef struct reg_mapping_st {
  u8 value;
  char text[US5182_ARGUMENT_LENGTH];
} reg_mapping_type;

static reg_mapping_type us5182_shutdown_mapping[] = {
  { 0 << 7, "DISABLE",  },
  { 1 << 7, "ENABLE",   },
  { 0xFF,   "",         },
};

static reg_mapping_type us5182_one_shot_mapping[] = {
  { 0 << 6, "CONTINUOUS", },
  { 1 << 6, "SINGLE",     },
  { 0xFF,   "",           },
};

static reg_mapping_type us5182_op_mode_mapping[] = {
  { 0 << 4, "PS_AND_ALS", },
  { 1 << 4, "ALS_ONLY",   },
  { 2 << 4, "PS_ONLY",    },
  { 3 << 4, "PS_ONLY",    },
  { 0xFF,   "",           },
};

static reg_mapping_type us5182_word_mapping[] = {
  { 0 << 0, "DISABLE",  },
  { 1 << 0, "ENABLE",   },
  { 0xFF,   "",         },
};

static reg_mapping_type us5182_als_fq_mapping[] = {
  { 7 << 5, "63", },
  { 6 << 5, "48", },
  { 5 << 5, "32", },
  { 4 << 5, "24", },
  { 3 << 5, "16", },
  { 2 << 5, "8",  },
  { 1 << 5, "4",  },
  { 0 << 5, "1",  },
  { 0xFF,   "",   },
};

static reg_mapping_type us5182_als_res_mapping[] = {
  { 0 << 3, "12", },
  { 1 << 3, "14", },
  { 2 << 3, "16", },
  { 3 << 3, "16", },
  { 0xFF,   "",   },
};

static reg_mapping_type us5182_als_gain_mapping[] = {
  { 7 << 0, "128",  },
  { 6 << 0, "64",   },
  { 5 << 0, "32",   },
  { 4 << 0, "16",   },
  { 3 << 0, "8",    },
  { 2 << 0, "4",    },
  { 1 << 0, "2",    },
  { 0 << 0, "1",    },
  { 0xFF,   "",     },
};

static reg_mapping_type us5182_ps_fq_mapping[] = {
  { 3 << 6, "15", },
  { 2 << 6, "8",  },
  { 1 << 6, "4",  },
  { 0 << 6, "1",  },
  { 0xFF,   "",   },
};

static reg_mapping_type us5182_int_type_mapping[] = {
  { 0 << 5, "LEVEL",  },
  { 1 << 5, "PULSE",  },
  { 0xFF,   "",       },
};

static reg_mapping_type us5182_ps_res_mapping[] = {
  { 0 << 3, "12", },
  { 1 << 3, "14", },
  { 2 << 3, "16", },
  { 3 << 3, "16", },
  { 0xFF,   "",   },
};

static reg_mapping_type us5182_ps_gain_mapping[] = {
  { 7 << 0, "128",  },
  { 6 << 0, "64",   },
  { 5 << 0, "32",   },
  { 4 << 0, "16",   },
  { 3 << 0, "8",    },
  { 2 << 0, "4",    },
  { 1 << 0, "2",    },
  { 0 << 0, "1",    },
  { 0xFF,   "",     },
};

static reg_mapping_type us5182_wait_sel_mapping[] = {
  { 3 << 6, "16", },
  { 2 << 6, "8",  },
  { 1 << 6, "4",  },
  { 0 << 6, "0",  },
  { 0xFF,   "",   },
};

static reg_mapping_type us5182_led_dr_mapping[] = {
  { 0 << 4, "12.5", },
  { 1 << 4, "25",   },
  { 2 << 4, "50",   },
  { 3 << 4, "100",  },
  { 0xFF,   "",     },
};

static reg_mapping_type us5182_int_source_sel_mapping[] = {
  { 0 << 2, "ALS_OR_PS",    },
  { 1 << 2, "ALS_ONLY",     },
  { 2 << 2, "PS_ONLY",      },
  { 3 << 2, "PS_APPROACH",  },
  { 0xFF,   "",             },
};

typedef struct command_op_st {
  char cmd[US5182_ARGUMENT_LENGTH];

  u8 reg_addr;
  u8 reg_mask;
  reg_mapping_type *reg_mapping;
} command_op_type;

static command_op_type us5182_command_op[] = {
  { "SHUTDOWN",       REGS_CR00,  0x80, us5182_shutdown_mapping,        },
  { "ONE_SHOT",       REGS_CR00,  0x40, us5182_one_shot_mapping,        },
  { "OP_MODE",        REGS_CR00,  0x30, us5182_op_mode_mapping,         },
  { "WORD",           REGS_CR00,  0x01, us5182_word_mapping,            },
  { "ALS_FQ",         REGS_CR01,  0xE0, us5182_als_fq_mapping,          },
  { "ALS_RES",        REGS_CR01,  0x18, us5182_als_res_mapping,         },
  { "ALS_GAIN",       REGS_CR01,  0x07, us5182_als_gain_mapping,        },
  { "PS_FQ",          REGS_CR02,  0xC0, us5182_ps_fq_mapping,           },
  { "INT_TYPE",       REGS_CR02,  0x20, us5182_int_type_mapping,        },
  { "PS_RES",         REGS_CR02,  0x18, us5182_ps_res_mapping,          },
  { "PS_GAIN",        REGS_CR02,  0x07, us5182_ps_gain_mapping,         },
  { "WAIT_SEL",       REGS_CR03,  0xC0, us5182_wait_sel_mapping,        },
  { "LED_DR",         REGS_CR03,  0x30, us5182_led_dr_mapping,          },
  { "INT_SOURCE_SEL", REGS_CR03,  0x0C, us5182_int_source_sel_mapping,  },
  { "",               0xFF,       0xFF, NULL,                           },
};

/**
 * @brief us5182_cmd_op_show
 *
 * Show command op status 
 * 
 * @para buf address of buf
 * @return size of buf
 */
static ssize_t us5182_cmd_op_show(char *buf)
{
  int idx;
  int len;
  u8 reg_data;
  int idx_map;

  idx = 0;
  len = 0;

  while(1)
  {
    if(!strncmp(us5182_command_package.argv[0], 
                us5182_command_op[idx].cmd, 
                strlen(us5182_command_op[idx].cmd)))
    {
      hwmsen_read_byte_sr(us5182_obj->client, us5182_command_op[idx].reg_addr, &reg_data);

      idx_map = 0;
      reg_data = reg_data & us5182_command_op[idx].reg_mask;

      while(1)
      {
        if(reg_data == us5182_command_op[idx].reg_mapping[idx_map].value)
        {
          len = snprintf(buf, PAGE_SIZE, "%s\n", us5182_command_op[idx].reg_mapping[idx_map].text);
          break;
        }

        if(us5182_command_op[idx].reg_mapping[idx_map].value == 0xFF)
        {
          APS_ERR("[%s]: unknown value. (%x)\n", __func__, reg_data);
          break;
        }

        idx_map = idx_map + 1;
      }
      break;
    }

    idx = idx + 1;
    if(us5182_command_op[idx].reg_addr == 0xFF)
    {
      break;
    }
  }
  return (len);
}

/**
 * @brief us5182_cmd_op_store
 *
 * Set command op status
 *
 * @return 0
 */
static ssize_t us5182_cmd_op_store(void)
{
  int idx;
  u8 reg_data;
  int idx_map;

  idx = 0;

  while(1)
  {
    APS_DBG("[%s]: %x %s <-> %s\n", __func__, us5182_command_op[idx].reg_addr, us5182_command_package.argv[0], us5182_command_op[idx].cmd);
    if(!strncmp(us5182_command_package.argv[0], 
                us5182_command_op[idx].cmd, 
                strlen(us5182_command_op[idx].cmd)))
    {
      hwmsen_read_byte_sr(us5182_obj->client, us5182_command_op[idx].reg_addr, &reg_data);

      idx_map = 0;
      reg_data = reg_data & (~us5182_command_op[idx].reg_mask);

      while(1)
      {
        APS_DBG("[%s]: %x %s <-> %s\n", __func__, us5182_command_op[idx].reg_mapping[idx_map].value, us5182_command_op[idx].reg_mapping[idx_map].text, us5182_command_package.argv[1]);
        if(us5182_command_op[idx].reg_mapping[idx_map].value == 0xFF)
        {
          APS_ERR("[%s]: unknown argument. (%x)\n", __func__, reg_data);
          break;
        }

        if(!strncmp(us5182_command_package.argv[1], 
                    us5182_command_op[idx].reg_mapping[idx_map].text, 
                    strlen(us5182_command_op[idx].reg_mapping[idx_map].text)))
        {
          reg_data = reg_data | us5182_command_op[idx].reg_mapping[idx_map].value;

          hwmsen_write_byte_sr(us5182_obj->client, us5182_command_op[idx].reg_addr, reg_data);
          break;
        }

        idx_map = idx_map + 1;
      }
      break;
    }

    idx = idx + 1;
    if(us5182_command_op[idx].reg_addr == 0xFF)
    {
      break;
    }
  }
  return (0);
}

/**
 * @brief us5182_softrst_store
 *
 * Set softrst status
 *
 * @return 0
 */
static ssize_t us5182_softrst_store(void)
{
  u8 reg_data;

  hwmsen_read_byte_sr(us5182_obj->client, REGS_CR03, &reg_data);

  reg_data = reg_data | 0x01;

  hwmsen_write_byte_sr(us5182_obj->client, REGS_CR03, reg_data);
  return (0);
}

/**
 * @brief us5182_start_store
 *
 * Start uS5182 operation 
 *
 * @return 0
 */
static ssize_t us5182_start_store(void)
{
  apk_tool_active = true;
  us5182_enable_ps(us5182_obj->client, true);
  us5182_enable_als(us5182_obj->client, true);
  return (0);
}

/**
 * @brief us5182_stop_store
 *
 * Stop uS5182 operation 
 *
 * @return 0
 */
static ssize_t us5182_stop_store(void)
{
  us5182_enable_ps(us5182_obj->client, false);
  us5182_enable_als(us5182_obj->client, false);
  apk_tool_active = false;
  return (0);
}

#define TEST_MODE_SHUTDOWN  ("SHUTDOWN")
#define TEST_MODE_ALS_ONLY  ("ALS_ONLY")
#define TEST_MODE_PS_ONLY   ("PS_ONLY")
#define TEST_MODE_ALS_PS    ("ALS_PS")

/**
 * @brief us5182_test_mode_store
 *
 * Test OP_MODE switch
 *
 * @return 0
 */
static ssize_t us5182_test_mode_store(void)
{
  if(!strncmp(us5182_command_package.argv[1], TEST_MODE_SHUTDOWN, strlen(TEST_MODE_SHUTDOWN)))
  {
    /// [AT-PM] : Switch to SHUTDOWN ; 09/01/2014
    us5182_enable_ps(us5182_obj->client, false);
    us5182_enable_als(us5182_obj->client, false);
    APS_DBG("[%s]: Switch to SHUTDOWN\n", __func__);
  }
  else if(!strncmp(us5182_command_package.argv[1], TEST_MODE_ALS_ONLY, strlen(TEST_MODE_ALS_ONLY)))
  {
    /// [AT-PM] : Switch to ALS ONLY ; 09/01/2014
    us5182_enable_ps(us5182_obj->client, false);
    us5182_enable_als(us5182_obj->client, true);
    APS_DBG("[%s]: Switch to ALS ONLY\n", __func__);
  }
  else if(!strncmp(us5182_command_package.argv[1], TEST_MODE_PS_ONLY, strlen(TEST_MODE_PS_ONLY)))
  {
    /// [AT-PM] : Switch to PS ONLY ; 09/01/2014
    us5182_enable_ps(us5182_obj->client, true);
    us5182_enable_als(us5182_obj->client, false);
    APS_DBG("[%s]: Switch to PS ONLY\n", __func__);
  }
  else if(!strncmp(us5182_command_package.argv[1], TEST_MODE_ALS_PS, strlen(TEST_MODE_ALS_PS)))
  {
    /// [AT-PM] : Switch to ALS AND PS ; 09/01/2014
    us5182_enable_ps(us5182_obj->client, true);
    us5182_enable_als(us5182_obj->client, true);
    APS_DBG("[%s]: Switch to ALS AND PS\n", __func__);
  }
  else
  {
    /// [AT-PM] : Unknown argument ; 09/01/2014
    APS_ERR("[%s]: Un-known argument\n", __func__);
  }
  return (0);
}

static us5182_command_type us5182_command_table[] = {
  { "SHUTDOWN",       us5182_cmd_op_show,   us5182_cmd_op_store,    }, 
  { "ONE_SHOT",       us5182_cmd_op_show,   us5182_cmd_op_store,    }, 
  { "OP_MODE",        us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "WORD",           us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "ALS_FQ",         us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "ALS_RES",        us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "ALS_GAIN",       us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "PS_FQ",          us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "INT_TYPE",       us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "PS_RES",         us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "PS_GAIN",        us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "WAIT_SEL",       us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "LED_DR",         us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "INT_SOURCE_SEL", us5182_cmd_op_show,   us5182_cmd_op_store,    },
  { "SOFTRST",        NULL,                 us5182_softrst_store,   },
  { "START",          NULL,                 us5182_start_store,     },
  { "STOP",           NULL,                 us5182_stop_store,      },
  { "TEST_MODE",      NULL,                 us5182_test_mode_store, },
  { "",               NULL,                 NULL,                   },
};

/**
 * @brief us5182_parse_command
 *
 * Parse command of sysfs
 *
 * @para buf address of buf
 * @return NULL
 */
static void us5182_parse_command(const char *buf)
{
  char *start;
  char *end;
  int length;

  memset((char *)(&us5182_command_package), 0, sizeof(us5182_command_package));

  memcpy(us5182_command_package.line, buf, ((strlen(buf) > US5182_ARGUMENT_LENGTH) ? US5182_ARGUMENT_LENGTH : strlen(buf)));

  us5182_command_package.argc = 0;
  start = buf;
  while(1)
  {
    end = strstr(start, " ");
    if(end == NULL)
    {
      length = (strlen(start) > US5182_ARGUMENT_LENGTH) ? US5182_ARGUMENT_LENGTH : strlen(start);
      memcpy(us5182_command_package.argv[us5182_command_package.argc], start, length);
      APS_DBG("[%s]: END -> %d = %s\n", __func__, us5182_command_package.argc, us5182_command_package.argv[us5182_command_package.argc]);
      us5182_command_package.argc = us5182_command_package.argc + 1;
      break;
    }

    length = strlen(start) - strlen(end);
    length = (length > US5182_ARGUMENT_LENGTH) ? US5182_ARGUMENT_LENGTH : length;
    memcpy(us5182_command_package.argv[us5182_command_package.argc], start, length);
    APS_DBG("[%s]: RUN -> %d = %s\n", __func__, us5182_command_package.argc, us5182_command_package.argv[us5182_command_package.argc]);
    us5182_command_package.argc = us5182_command_package.argc + 1;
    if(us5182_command_package.argc >= US5182_ARGUMENT_COUNT)
    {
      break;
    }

    start = end + 1;
  }
}

/**
 * @brief us5182_show_command
 *
 * Show command status
 *
 * @para ddri address of struct device_driver
 * @para buf address of content
 * @return size of buf
 */
static ssize_t us5182_show_command(struct device_driver *ddri, char *buf)
{
  int idx;

  APS_FUN();

  idx = 0;
  while((us5182_command_table[idx].show != NULL) ||
        (us5182_command_table[idx].store != NULL))
  {
    if((!strncmp(us5182_command_table[idx].command, 
                 us5182_command_package.argv[0], 
                 strlen(us5182_command_table[idx].command))) &&
       (us5182_command_table[idx].show != NULL))
    {
      return (us5182_command_table[idx].show(buf));
    }

    idx = idx + 1;
  }

  return (snprintf(buf, PAGE_SIZE, "No such command\n"));
}

/**
 * @brief us5182_store_command
 *
 * Execute command
 *
 * @para ddri address of struct device_driver
 * @para buf address of content
 * @para count size of buf
 * @return count
 */
static ssize_t us5182_store_command(struct device_driver *ddri, const char *buf, size_t count)
{
  int idx;

  APS_FUN();
  APS_DBG("[%s]: %s\n", __func__, buf);

  us5182_parse_command(buf);

  idx = 0;
  while((us5182_command_table[idx].show != NULL) ||
        (us5182_command_table[idx].store != NULL))
  {
    if(!strncmp(us5182_command_table[idx].command, 
                us5182_command_package.argv[0],
                strlen(us5182_command_table[idx].command))) 
    {
      if(us5182_command_table[idx].show == NULL)
      {
        /// [AT-PM] : Write only ; 08/08/2014
        us5182_command_table[idx].store();
      }
      else if(us5182_command_table[idx].store == NULL)
      {
        /// [AT-PM] : Read only ; 08/08/2014
        if(us5182_command_package.argc > 1)
        {
          us5182_command_table[idx].store();
        }
      }
      else
      {
        /// [AT-PM] : Read and Write ; 08/08/2014
        if(us5182_command_package.argc > 1)
        {
          us5182_command_table[idx].store();
        }
      }
      break;
    }

    idx = idx + 1;
  }
  return (count);
}
/*----------------------------------------------------------------------------*/
static ssize_t us5182_show_als_loss(struct device_driver *ddri, char *buf)
{
  ssize_t len = 0;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }

  len += snprintf(buf + len, PAGE_SIZE - len, "%d %%\n", us5182_obj->als_window_loss);
  return (len);
}

static ssize_t us5182_store_als_loss(struct device_driver *ddri, const char *buf, size_t count)
{
  int als_loss;

  APS_FUN();

  if(!us5182_obj)
  {
    APS_ERR("us5182_obj is null!!\n");
    return (0);
  }

  if(sscanf(buf, "%d", &als_loss) == 1)
  {
    APS_ERR("invalid argument\n");
    return (0);
  }
  
  us5182_obj->als_window_loss = (unsigned int)als_loss;
  return (count);
} 
/*----------------------------------------------------------------------------*/
#define US5182_DRIVER_ATTR_MODE     (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

static DRIVER_ATTR(als,       US5182_DRIVER_ATTR_MODE, us5182_show_als,       NULL);
static DRIVER_ATTR(ps,        US5182_DRIVER_ATTR_MODE, us5182_show_ps,        NULL);
static DRIVER_ATTR(config,    US5182_DRIVER_ATTR_MODE, us5182_show_config,    us5182_store_config);
static DRIVER_ATTR(alslv,     US5182_DRIVER_ATTR_MODE, us5182_show_alslv,     us5182_store_alslv);
static DRIVER_ATTR(alsval,    US5182_DRIVER_ATTR_MODE, us5182_show_alsval,    us5182_store_alsval);
static DRIVER_ATTR(trace,     US5182_DRIVER_ATTR_MODE, us5182_show_trace,     us5182_store_trace);
static DRIVER_ATTR(status,    US5182_DRIVER_ATTR_MODE, us5182_show_status,    NULL);
static DRIVER_ATTR(reg,       US5182_DRIVER_ATTR_MODE, us5182_show_reg,       NULL);
static DRIVER_ATTR(regaddr,   US5182_DRIVER_ATTR_MODE, us5182_show_reg_addr,  NULL);
static DRIVER_ATTR(i2c,       US5182_DRIVER_ATTR_MODE, us5182_show_i2c,       us5182_store_i2c);
#ifdef  US5182_HEARTRATE_DETECTION
static DRIVER_ATTR(heartrate, US5182_DRIVER_ATTR_MODE, us5182_show_heartrate, us5182_store_heartrate);
#endif  ///< end of US5182_HEARTRATE_DETECTION
static DRIVER_ATTR(eint,      US5182_DRIVER_ATTR_MODE, us5182_show_eint,      NULL);
static DRIVER_ATTR(cmd,       US5182_DRIVER_ATTR_MODE, us5182_show_command,   us5182_store_command);
static DRIVER_ATTR(als_loss,  US5182_DRIVER_ATTR_MODE, us5182_show_als_loss,  us5182_store_als_loss);
/*----------------------------------------------------------------------------*/
static struct driver_attribute *us5182_attr_list[] = {
    &driver_attr_als,
    &driver_attr_ps,    
    &driver_attr_trace,        /*trace log*/
    &driver_attr_config,
    &driver_attr_alslv,
    &driver_attr_alsval,
    &driver_attr_status,
    &driver_attr_i2c,
    &driver_attr_reg,
    &driver_attr_regaddr,
#ifdef  US5182_HEARTRATE_DETECTION
    &driver_attr_heartrate,
#endif  ///< end of US5182_HEARTRATE_DETECTION
    &driver_attr_eint,
    &driver_attr_cmd,
    &driver_attr_als_loss,
}; 
/*----------------------------------------------------------------------------*/
static int us5182_create_attr(struct device_driver *driver) 
{
  int idx, err = 0;
  int num = (int)(sizeof(us5182_attr_list)/sizeof(us5182_attr_list[0]));

  if (driver == NULL)
  {
    return (-EINVAL);
  }

  for(idx = 0; idx < num; idx++)
  {
    if((err = driver_create_file(driver, us5182_attr_list[idx])))
    {            
      APS_ERR("driver_create_file (%s) = %d\n", us5182_attr_list[idx]->attr.name, err);
      break;
    }
  }    
  return (err);
}
/*----------------------------------------------------------------------------*/
  static int us5182_delete_attr(struct device_driver *driver)
  {
  int idx;
  int err = 0;
  int num = (int)(sizeof(us5182_attr_list)/sizeof(us5182_attr_list[0]));

  if (!driver)
  {
    return (-EINVAL);
  }

  for(idx = 0; idx < num; idx ++) 
  {
    driver_remove_file(driver, us5182_attr_list[idx]);
  }
  
  return (err);
} 
/****************************************************************************** 
 * Function 
******************************************************************************/
static int us5182_get_als_value(struct us5182_priv *obj, u32 als)
{
#ifdef US5182_LUX_CONVERSTION
  int idx;
  int invalid = 0;
  unsigned long endt;
  int als_value;

  for(idx = 0; idx < obj->als_level_num; idx ++)
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
    endt = atomic_read(&obj->als_deb_end);

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
    if(idx == 0)
    {
      als_value = obj->hw->als_value[idx];
    }
    else
    {
      als_value = obj->hw->als_value[idx] - obj->hw->als_value[idx - 1];
      als_value = als_value * (als - obj->hw->als_level[idx - 1]);
      als_value = als_value / (obj->hw->als_level[idx] - obj->hw->als_level[idx - 1]);
      als_value = als_value + obj->hw->als_value[idx - 1];
    }

    /// [AT-PM] : Apply window loss ; 09/15/2014
    als_value = als_value * US5182_ALS_WINDOW_LOSS_CONST;
    als_value = als_value / (US5182_ALS_WINDOW_LOSS_CONST - obj->als_window_loss);

    if (atomic_read(&obj->trace) & CMC_TRC_CVT_ALS)
    {
      APS_LOG("ALS: %05d => %05d\n", als, als_value);
    }
    return (als_value);
  }
  else
  {
    if(atomic_read(&obj->trace) & CMC_TRC_CVT_ALS)
    {
      APS_LOG("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
    }
    return (-1);
  }
#else   ///< else of US5182_LUX_CONVERSTION
  return (als);
#endif  ///< end of US5182_LUX_CONVERSTION
}
/*----------------------------------------------------------------------------*/
static int us5182_get_ps_value(struct us5182_priv *obj, u32 ps)
{
  int val= -1;
  int invalid = 0;
  unsigned long endt;

  APS_DBG("1.us5182_get_ps_value : %x", ps);
  if(ps > obj->hw->ps_threshold_high)
  {
    val = 0;  /*close*/
    APS_DBG("2.us5182_get_ps_value : %x", val);
  }
  else //if(ps < obj->hw->ps_threshold_low)
  {
    val = 1;  /*far away*/
    APS_DBG("3.us5182_get_ps_value : %x", val);
  }

  if(atomic_read(&obj->ps_suspend))
  {
    invalid = 1;
    APS_DBG("4.us5182_get_ps_value");
  }
  else if(1 == atomic_read(&obj->ps_deb_on))
  {
    endt = atomic_read(&obj->ps_deb_end);

    if(time_after(jiffies, endt))
    {
      atomic_set(&obj->ps_deb_on, 0);
      APS_DBG("5.us5182_get_ps_value");
    }
    
    if (1 == atomic_read(&obj->ps_deb_on))
    {
      invalid = 1;
      APS_DBG("6.us5182_get_ps_value");
    }
  }

  if(!invalid)
  {
    if(unlikely(atomic_read(&obj->trace) & CMC_TRC_CVT_PS))
    {
      APS_LOG("PS:  %05d => %05d\n", ps, val);
    }
    APS_LOG("7.us5182_get_ps_value : %x (%d)", ps, val);
    return (val);
    
  } 
  else
  {
    if(unlikely(atomic_read(&obj->trace) & CMC_TRC_CVT_PS))
    {
      APS_LOG("PS:  %05d => %05d (-1)\n", ps, val);    
    }
    APS_LOG("8.us5182_get_ps_value : %x (%d)", ps, val);
    return (-1);
  } 

  APS_LOG("9.us5182_get_ps_value : %x (%d)", ps, val);
  return (val);
}
/*----------------------------------------------------------------------------*/
static int us5182_open(struct inode *inode, struct file *file)
{
  file->private_data = us5182_i2c_client;

  if (!file->private_data)
  {
    APS_ERR("null pointer!!\n");
    return (-EINVAL);
  }
  
  return (nonseekable_open(inode, file));
}
/*----------------------------------------------------------------------------*/
static int us5182_release(struct inode *inode, struct file *file)
{
  file->private_data = NULL;
  return (0);
}
/*----------------------------------------------------------------------------*/
static long us5182_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  struct i2c_client *client = (struct i2c_client *)file->private_data;
  struct us5182_priv *obj = i2c_get_clientdata(client);  
  long err = 0;
  void __user *ptr = (void __user *) arg;
  u32 dat;
  uint32_t enable;

  APS_FUN();

  switch (cmd)
  {
    case ALSPS_SET_PS_MODE:
      APS_LOG("ALSPS_SET_PS_MODE\n");
      if(copy_from_user(&enable, ptr, sizeof(enable)))
      {
        err = -EFAULT;
        goto err_out;
      }
      if(enable)
      {
        if((err = us5182_enable_ps(obj->client, true)))
        {
          APS_ERR("enable ps fail: %ld\n", err); 
          goto err_out;
        }
        
        set_bit(CMC_BIT_PS, &obj->enable);
      }
      else
      {
        if((err = us5182_enable_ps(obj->client, false)))
        {
          APS_ERR("disable ps fail: %ld\n", err); 
          goto err_out;
        }
        
        clear_bit(CMC_BIT_PS, &obj->enable);
      }
      break;

    case ALSPS_GET_PS_MODE:
      APS_LOG("ALSPS_GET_PS_MODE\n");
      enable = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
      if(copy_to_user(ptr, &enable, sizeof(enable)))
      {
        err = -EFAULT;
        goto err_out;
      }
      break;

    case ALSPS_GET_PS_DATA:    
      APS_LOG("ALSPS_GET_PS_DATA\n");
      if((err = us5182_read_ps_data(obj->client, &obj->ps)))
      {
        goto err_out;
      }
      dat = us5182_get_ps_value(obj, obj->ps);
      if(copy_to_user(ptr, &dat, sizeof(dat)))
      {
        err = -EFAULT;
        goto err_out;
      }  
      break;

    case ALSPS_GET_PS_RAW_DATA:    
      APS_LOG("ALSPS_GET_PS_RAW_DATA\n");
      if((err = us5182_read_ps_data(obj->client, &obj->ps)))
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
      APS_LOG("ALSPS_SET_ALS_MODE\n");
      if(copy_from_user(&enable, ptr, sizeof(enable)))
      {
        err = -EFAULT;
        goto err_out;
      }
      if(enable)
      {
        if((err = us5182_enable_als(obj->client, true)))
        {
          APS_ERR("enable als fail: %ld\n", err); 
          goto err_out;
        }
        set_bit(CMC_BIT_ALS, &obj->enable);
      }
      else
      {
        if((err = us5182_enable_als(obj->client, false)))
        {
          APS_ERR("disable als fail: %ld\n", err); 
          goto err_out;
        }
        clear_bit(CMC_BIT_ALS, &obj->enable);
      }
      break;

    case ALSPS_GET_ALS_MODE:
      APS_LOG("ALSPS_GET_ALS_MODE\n");
      enable = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);
      if(copy_to_user(ptr, &enable, sizeof(enable)))
      {
        err = -EFAULT;
        goto err_out;
      }
      break;

    case ALSPS_GET_ALS_DATA: 
      APS_LOG("ALSPS_GET_ALS_DATA\n");
      if((err = us5182_read_als_data(obj->client, &obj->als)))
      {
        goto err_out;
      }

      dat = us5182_get_als_value(obj, obj->als);
      if(copy_to_user(ptr, &dat, sizeof(dat)))
      {
        err = -EFAULT;
        goto err_out;
      }              
      break;

    case ALSPS_GET_ALS_RAW_DATA:    
      APS_LOG("ALSPS_GET_ALS_RAW_DATA\n");
      if((err = us5182_read_als_data(obj->client, &obj->als)))
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
  return (err);
}
/*----------------------------------------------------------------------------*/
static struct file_operations us5182_fops = {
  .open = us5182_open,
  .release = us5182_release,
  .unlocked_ioctl = us5182_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice us5182_device = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "als_ps",
  .fops = &us5182_fops,
}; 
/*----------------------------------------------------------------------------*/
static int us5182_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
  APS_FUN();
  return (0);
}
/*----------------------------------------------------------------------------*/
static int us5182_i2c_resume(struct i2c_client *client)
{
  APS_FUN();
  return 0;
} 
/*----------------------------------------------------------------------------*/
static void us5182_early_suspend(struct early_suspend *h) 
{
  struct us5182_priv *obj = container_of(h, struct us5182_priv, early_drv);   
  int err;

  APS_FUN();    

  if(!obj)
  {
    APS_ERR("null pointer!!\n");
    return;
  }
  
  atomic_set(&obj->als_suspend, 1);    
  if((err = us5182_enable_als(obj->client, false)))
  {
    APS_ERR("disable als fail: %d\n", err); 
  }
}
/*----------------------------------------------------------------------------*/
static void us5182_late_resume(struct early_suspend *h)
{
  struct us5182_priv *obj = container_of(h, struct us5182_priv, early_drv);         
  int err;

  APS_FUN();

  if(!obj)
  {
    APS_ERR("null pointer!!\n");
    return;
  }

  atomic_set(&obj->als_suspend, 0);
  if(test_bit(CMC_BIT_ALS, &obj->enable))
  {
    if((err = us5182_enable_als(obj->client, true)))
    {
      APS_ERR("enable als fail: %d\n", err);        

    }
  }
}
/*----------------------------------------------------------------------------*/
int us5182_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
                      void* buff_out, int size_out, int* actualout)
{
  int err = 0;
  int value;
  hwm_sensor_data* sensor_data;
  struct us5182_priv *obj = (struct us5182_priv *)self;
  
  APS_FUN();

  if(apk_tool_active == true)
  {
    APS_ERR("[%s]: APK is active\n", __func__);
    return (0);
  }

  mutex_lock(&obj->op_mutex);

  APS_LOG("[%s]: command = %d\n", __func__, command);
  switch (command)
  {
    case SENSOR_DELAY:
      APS_DBG("[%s]: SENSOR_DELAY\n", __func__);
      if((buff_in == NULL) || (size_in < sizeof(int)))
      {
        APS_ERR("Set delay parameter error!\n");
        err = -EINVAL;
      }
      // Do nothing
      break;
    case SENSOR_ENABLE:
      APS_DBG("[%s]: SENSOR_ENABLE\n", __func__);
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
          APS_LOG("[%s]: ENABLE PS\n", __func__);
          if((err = us5182_enable_ps(obj->client, true)))
          {
            cancel_delayed_work_sync(&obj->enable_ps_work);
            cancel_delayed_work_sync(&obj->disable_ps_work);
            schedule_delayed_work(&obj->enable_ps_work, msecs_to_jiffies(50));
            APS_ERR("enable ps fail: %d\n", err); 
            break;
          }
          set_bit(CMC_BIT_PS, &obj->enable);
        }
        else
        {
          APS_LOG("[%s]: DISABLE PS\n", __func__);
          if((err = us5182_enable_ps(obj->client, false)))
          {
            cancel_delayed_work_sync(&obj->enable_ps_work);
            cancel_delayed_work_sync(&obj->disable_ps_work);
            schedule_delayed_work(&obj->disable_ps_work, msecs_to_jiffies(50));
            APS_ERR("disable ps fail: %d\n", err); 
            break;
          }
          clear_bit(CMC_BIT_PS, &obj->enable);
        }
      }
      break;
    case SENSOR_GET_DATA:
      APS_DBG("[%s]: SENSOR_GET_DATA\n", __func__);
      if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
      {
        APS_ERR("get sensor data parameter error!\n");
        err = -EINVAL;
      }
      else
      {
        sensor_data = (hwm_sensor_data *)buff_out;        
        
        if(obj->ps_data >= 0)
        {
          sensor_data->values[0] = obj->ps_data;
          sensor_data->value_divide = 1;
          sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
          APS_DBG("[%s]: PS DATA = %d\n", __func__, sensor_data->values[0]);
          us5182_chk_last_ps_data(&obj->last_report_ps, sensor_data);
        }
      }
      break;
    default:
      APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
      err = -1;
      break;
  }
  
  mutex_unlock(&obj->op_mutex);
  return (err);
}
/*----------------------------------------------------------------------------*/ 
int us5182_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
                       void* buff_out, int size_out, int* actualout)
{
  int err = 0;
  int value;
  hwm_sensor_data* sensor_data;
  struct us5182_priv *obj = (struct us5182_priv *)self;
  
  APS_FUN();

  if(apk_tool_active == true)
  {
    APS_ERR("[%s]: APK is active\n", __func__);
    return (0);
  }

  APS_LOG("[%s]: command = %d\n", __func__, command);
  switch (command)
  {
    case SENSOR_DELAY:
      APS_DBG("[%s]: SENSOR_DELAY\n", __func__);
      if((buff_in == NULL) || (size_in < sizeof(int)))
      {
        APS_ERR("Set delay parameter error!\n");
        err = -EINVAL;
      }
      // Do nothing
      break;

    case SENSOR_ENABLE:
      APS_DBG("[%s]: SENSOR_ENABLE\n", __func__);
      if((buff_in == NULL) || (size_in < sizeof(int)))
      {
        APS_ERR("Enable sensor parameter error!\n");
        err = -EINVAL;
      }
      else
      {
        cancel_delayed_work_sync(&obj->get_als_work);

        value = *(int *)buff_in;        
        if(value)
        {
          mutex_lock(&obj->op_mutex);
          APS_LOG("[%s]: ENABLE ALS\n", __func__);
          err = us5182_enable_als(obj->client, true);
          mutex_unlock(&obj->op_mutex);
          if(err != 0)
          {
            APS_ERR("enable als fail: %d\n", err); 
            break;
          }
          set_bit(CMC_BIT_ALS, &obj->enable);

          schedule_delayed_work(&obj->get_als_work, 0);
        }
        else
        {
          mutex_lock(&obj->op_mutex);
          APS_LOG("[%s]: DISABLE ALS\n", __func__);
          err = us5182_enable_als(obj->client, false);
          mutex_unlock(&obj->op_mutex);
          if(err != 0)
          {
            APS_ERR("disable als fail: %d\n", err); 
            break;
          }
          clear_bit(CMC_BIT_ALS, &obj->enable);
        }
        
      }
      break;

    case SENSOR_GET_DATA:
      APS_DBG("[%s]: SENSOR_GET_DATA\n", __func__);
      if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
      {
        APS_ERR("get sensor data parameter error!\n");
        err = -EINVAL;
      }
      else
      {
        sensor_data = (hwm_sensor_data *)buff_out;
        sensor_data->values[0] = obj->als_data;
        sensor_data->value_divide = 1;
        sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
      }
      break;
    default:
      APS_ERR("light sensor operate function no this parameter %d!\n", command);
      err = -1;
      break;
  }
  
  return (err);
}
/*----------------------------------------------------------------------------*/
static int us5182_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
  strcpy(info->type, US5182_DEV_NAME);
  return (0);
} 
/*----------------------------------------------------------------------------*/
static int us5182_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
  struct us5182_priv *obj;
  struct hwmsen_object obj_ps;
  struct hwmsen_object obj_als;
  
 #ifdef KK2_TRY
  struct als_control_path als_ctl={0};
  struct als_data_path als_data={0};
  struct ps_control_path ps_ctl={0};
  struct ps_data_path ps_data={0};
 #endif
  int err = 0;

  APS_FUN();
  if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
  {
    err = -ENOMEM;
    goto exit;
  }
  memset(obj, 0, sizeof(*obj));
  us5182_obj = obj;

  obj->hw = get_cust_alsps_hw();

  INIT_DELAYED_WORK(&obj->eint_work,       us5182_eint_work);
  INIT_DELAYED_WORK(&obj->core_rst_work,   us5182_core_rst_work_func);
  INIT_DELAYED_WORK(&obj->get_ps_work,     us5182_get_ps_work_func);
  INIT_DELAYED_WORK(&obj->enable_ps_work,  us5182_enable_ps_work_func);
  INIT_DELAYED_WORK(&obj->disable_ps_work, us5182_disable_ps_work_func);
  INIT_DELAYED_WORK(&obj->get_als_work,    us5182_get_als_work_func);

  mutex_init(&obj->op_mutex);

  hrtimer_init(&obj->get_ps_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  obj->get_ps_timer.function = us5182_get_ps_timer_func;

  obj->client = client;
  i2c_set_clientdata(client, obj);  
  atomic_set(&obj->als_debounce, 30);
  atomic_set(&obj->als_deb_on, 0);
  atomic_set(&obj->als_deb_end, 0);
  atomic_set(&obj->ps_debounce, 10);
  atomic_set(&obj->ps_deb_on, 0);
  atomic_set(&obj->ps_deb_end, 0);
  atomic_set(&obj->ps_mask, 0);
  atomic_set(&obj->trace, 0x00);
  atomic_set(&obj->als_suspend, 0);
  atomic_set(&obj->core_rst, 0);

  obj->ps_enable = 0;
  obj->als_enable = 0;
  obj->enable = 0;
  obj->pending_intr = 0;
  obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
  obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);   
  obj->polarity = 0;
  obj->eint_count = 0;
  obj->eint_high_count = 0;
  obj->eint_low_count = 0;
  obj->eint_init = true;
  obj->ps_get_start = false;
  obj->als_window_loss = obj->hw->als_window_loss;
  BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
  memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
  BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
  memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
  atomic_set(&obj->i2c_retry, 3);
  
  set_bit(CMC_BIT_ALS, &obj->enable);
  set_bit(CMC_BIT_PS, &obj->enable);
  obj->ps_thd_val = obj->hw->ps_threshold;

  us5182_i2c_client = client;

  if((err = us5182_init_client(client)))
  {
    goto exit_init_failed;
  }
  
  if((err = misc_register(&us5182_device)))
  {
    APS_ERR("us5182_device register failed\n");
    goto exit_misc_device_register_failed;
  }
  if((err = us5182_create_attr(&us5182_alsps_driver.driver)))
  {
    APS_ERR("create attribute err = %d\n", err);
    goto exit_create_attr_failed;
  }
  #ifdef KK2_TRY
  als_ctl.open_report_data= als_open_report_data;
  als_ctl.enable_nodata = als_enable_nodata;
  als_ctl.set_delay  = als_set_delay;
  als_ctl.is_report_input_direct = false;
  als_ctl.is_support_batch = false;
  
  err = als_register_control_path(&als_ctl);
  if(err)
  {
		  APS_ERR("register fail = %d\n", err);
		  goto exit_sensor_obj_attach_fail;
   }
  als_data.get_data = als_get_data;
  als_data.vender_div = 100;
  err = als_register_data_path(&als_data);	
  if(err)
  {
		APS_ERR("tregister fail = %d\n", err);
		goto exit_sensor_obj_attach_fail;
   }
	
  ps_ctl.open_report_data= ps_open_report_data;
  ps_ctl.enable_nodata = ps_enable_nodata;
  ps_ctl.set_delay  = ps_set_delay;
  ps_ctl.is_report_input_direct = true;
  ps_ctl.is_support_batch = false;
  err = ps_register_control_path(&ps_ctl);
  if(err)
  {
		APS_ERR("register fail = %d\n", err);
		goto exit_sensor_obj_attach_fail;
   }
	ps_data.get_data = ps_get_data;
	ps_data.vender_div = 100;
	err = ps_register_data_path(&ps_data);	
	if(err)
	{
		APS_ERR("tregister fail = %d\n", err);
		goto exit_sensor_obj_attach_fail;
	}
#endif
  obj_ps.self = us5182_obj;
  obj_ps.polling = obj->hw->polling_mode_ps;
  obj_ps.sensor_operate = us5182_ps_operate;
  if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
  {
    APS_ERR("attach fail = %d\n", err);
    goto exit_create_attr_failed;
  }
  
  obj_als.self = us5182_obj;
  obj_als.polling = obj->hw->polling_mode_als;
  obj_als.sensor_operate = us5182_als_operate;
  if((err = hwmsen_attach(ID_LIGHT, &obj_als)))
  {
    APS_ERR("attach fail = %d\n", err);
    goto exit_create_attr_failed;
  }
  
#if defined(CONFIG_HAS_EARLYSUSPEND)
  obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1;
  obj->early_drv.suspend  = us5182_early_suspend;
  obj->early_drv.resume   = us5182_late_resume;  
  register_early_suspend(&obj->early_drv);
#endif

  APS_LOG("%s: OK\n", __func__);
  return (0);

exit_create_attr_failed:
exit_sensor_obj_attach_fail: 
exit_misc_device_register_failed:
	misc_deregister(&us5182_device);
exit_init_failed:
  kfree(obj);
exit:
  us5182_i2c_client = NULL;           
#ifdef MT6516        
  MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
#endif
  APS_ERR("%s: err = %d\n", __func__, err);
  return (err);
}
/*----------------------------------------------------------------------------*/
static int us5182_i2c_remove(struct i2c_client *client)
{
  int err;  
  
  if((err = us5182_delete_attr(&us5182_i2c_driver.driver)))
  {
    APS_ERR("us5182_delete_attr fail: %d\n", err);
  } 

  if((err = misc_deregister(&us5182_device)))
  {
    APS_ERR("misc_deregister fail: %d\n", err);    
  }
  
  us5182_i2c_client = NULL;
  i2c_unregister_device(client);
  kfree(i2c_get_clientdata(client));

  return (0);
} 
/*----------------------------------------------------------------------------*/
static int us5182_probe(struct platform_device *pdev) 
{
  struct alsps_hw *hw = get_cust_alsps_hw();
  
  APS_FUN();
  us5182_power(hw, 1);    
  if(i2c_add_driver(&us5182_i2c_driver))
  {
    APS_ERR("add driver error\n");
    return (-1);
  } 
  return (0);
}
/*----------------------------------------------------------------------------*/
static int us5182_remove(struct platform_device *pdev)
{
  struct alsps_hw *hw = get_cust_alsps_hw();

  APS_FUN();    
  us5182_power(hw, 0);    
  i2c_del_driver(&us5182_i2c_driver);
  return (0);
}
/*----------------------------------------------------------------------------*/
#if defined(MTK_AUTO_DETECT_ALSPS)
static struct sensor_init_info us5182_init_info = {
		.name = "us5182",
		.init = us5182_probe,
		.uninit = us5182_remove,
};
#else
static struct platform_driver us5182_alsps_driver = {
  .probe      = us5182_probe,
  .remove     = us5182_remove,    
  .driver     = {
    .name  = "als_ps",
  }
};
#endif
/*----------------------------------------------------------------------------*/
static int __init us5182_init(void)
{
  struct alsps_hw *hw = get_cust_alsps_hw();
  
  APS_FUN();
  APS_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 

  i2c_register_board_info(hw->i2c_num, &i2c_US5182, 1);
#if defined(MTK_AUTO_DETECT_ALSPS)
	  hwmsen_alsps_sensor_add(&us5182_init_info);
#else
  if(platform_driver_register(&us5182_alsps_driver))
  {
    APS_ERR("failed to register driver");
    return (-ENODEV);
  }
#endif  
  return (0);
}
/*----------------------------------------------------------------------------*/
static void __exit us5182_exit(void)
{
  APS_FUN();
#if !defined(MTK_AUTO_DETECT_ALSPS)
  platform_driver_unregister(&us5182_alsps_driver);
#endif
}
/*----------------------------------------------------------------------------*/
module_init(us5182_init);
module_exit(us5182_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Allen Teng");
MODULE_DESCRIPTION("uS5182D Proximity & Ambient light sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("V4.02");
