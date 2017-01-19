/*
 * Copyright (C) 2012 UPI finley_huang <finley_huang@upi-semi.com>. All Rights Reserved.
 * us5182 Light Sensor Driver for Linux 2.6
 */

/*
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

#ifndef __us5182_H__
#define __us5182_H__

#include <linux/types.h>

#define US5182_LUX_CONVERSTION      ///< Report raw code to framework instead of lux
//#define US5182_HEARTRATE_DETECTION  ///< Enable heartrate detection feature

///< us5182 Slave Addr
#define LIGHT_ADDR      (0x72)    ///< 7bit 0x39

///< Interrupt PIN for S3C6410
#define IRQ_LIGHT_INT IRQ_EINT  (6)

///< Register Set
#define REGS_CR00           (0x00)
#define REGS_CR01           (0x01)
#define REGS_CR02           (0x02)
#define REGS_CR03           (0x03)
///< ALS
#define REGS_INT_LSB_TH_LO      (0x04)
#define REGS_INT_MSB_TH_LO      (0x05)
#define REGS_INT_LSB_TH_HI      (0x06)
#define REGS_INT_MSB_TH_HI      (0x07)
///< PS
#define REGS_INT_LSB_TH_LO_PS   (0x08)
#define REGS_INT_MSB_TH_LO_PS   (0x09)
#define REGS_INT_LSB_TH_HI_PS   (0x0A)
#define REGS_INT_MSB_TH_HI_PS   (0x0B)
///< ALS data
#define REGS_LSB_SENSOR         (0x0C)
#define REGS_MSB_SENSOR         (0x0D)
///< PS data
#define REGS_LSB_SENSOR_PS      (0x0E)
#define REGS_MSB_SENSOR_PS      (0x0F)

#define REGS_CR10           (0x10)
#define REGS_CR11           (0x11)
#define REGS_CR16           (0x16)
#define REGS_CR17           (0x17)
#define REGS_CR20           (0x20)
#define REGS_CR21           (0x21)
#define REGS_CR22           (0x22)
#define REGS_CR23           (0x23)
#define REGS_CR27           (0x27)
#define REGS_CR28           (0x28)
#define REGS_CR29           (0x29)
#define REGS_CR2A           (0x2A)
#define REGS_CR2B           (0x2B)
#define REGS_VERSION_ID     (0x1F)
#define REGS_CHIP_ID        (0xB2)

/*ShutDown_EN*/
#define CR0_OPERATION     (0x0)
#define CR0_SHUTDOWN_EN   (0x1)

#define CR0_SHUTDOWN_SHIFT    (7)
#define CR0_SHUTDOWN_MASK     (0x1 << CR0_SHUTDOWN_SHIFT)

/*OneShot_EN*/
#define CR0_ONESHOT_EN    (0x01)

#define CR0_ONESHOT_SHIFT     (6)
#define CR0_ONESHOT_MASK      (0x1 << CR0_ONESHOT_SHIFT)

/*Operation Mode*/
#define CR0_OPMODE_ALSANDPS (0x0)
#define CR0_OPMODE_ALSONLY  (0x1)
#define CR0_OPMODE_IRONLY   (0x2)

#define CR0_OPMODE_SHIFT        (4)
#define CR0_OPMODE_MASK         (0x3 << CR0_OPMODE_SHIFT)

/*all int flag (PROX, INT_A, INT_P)*/
#define CR0_ALL_INT_CLEAR (0x0)

#define CR0_ALL_INT_SHIFT       (1)
#define CR0_ALL_INT_MASK        (0x7 << CR0_ALL_INT_SHIFT)


/*indicator of object proximity detection*/
#define CR0_PROX_CLEAR    (0x0)

#define CR0_PROX_SHIFT        (3)
#define CR0_PROX_MASK         (0x1 << CR0_PROX_SHIFT)

/*interrupt status of proximity sensor*/
#define CR0_INTP_CLEAR    (0x0)

#define CR0_INTP_SHIFT        (2)
#define CR0_INTP_MASK         (0x1 << CR0_INTP_SHIFT)

/*interrupt status of ambient sensor*/
#define CR0_INTA_CLEAR    (0x0)

#define CR0_INTA_SHIFT        (1)
#define CR0_INTA_MASK         (0x1 << CR0_INTA_SHIFT)

/*Word mode enable*/
#define CR0_WORD_EN   (0x1)

#define CR0_WORD_SHIFT        (0)
#define CR0_WORD_MASK         (0x1 << CR0_WORD_SHIFT)


/*ALS fault queue depth for interrupt enent output*/
#define CR1_ALS_FQ_1    (0x0)
#define CR1_ALS_FQ_4    (0x1)
#define CR1_ALS_FQ_8    (0x2)
#define CR1_ALS_FQ_16   (0x3)
#define CR1_ALS_FQ_24   (0x4)
#define CR1_ALS_FQ_32   (0x5)
#define CR1_ALS_FQ_48   (0x6)
#define CR1_ALS_FQ_63   (0x7)

#define CR1_ALS_FQ_SHIFT        (5)
#define CR1_ALS_FQ_MASK         (0x7 << CR1_ALS_FQ_SHIFT)

/*resolution for ALS*/
#define CR1_ALS_RES_12BIT   (0x0)
#define CR1_ALS_RES_14BIT   (0x1)
#define CR1_ALS_RES_16BIT   (0x2)
#define CR1_ALS_RES_16BIT_2 (0x3)

#define CR1_ALS_RES_SHIFT       (3)
#define CR1_ALS_RES_MASK        (0x3 << CR1_ALS_RES_SHIFT)

/*sensing amplifier selection for ALS*/
#define CR1_ALS_GAIN_X1   (0x0)
#define CR1_ALS_GAIN_X2   (0x1)
#define CR1_ALS_GAIN_X4   (0x2)
#define CR1_ALS_GAIN_X8   (0x3)
#define CR1_ALS_GAIN_X16  (0x4)
#define CR1_ALS_GAIN_X32  (0x5)
#define CR1_ALS_GAIN_X64  (0x6)
#define CR1_ALS_GAIN_X128 (0x7)

#define CR1_ALS_GAIN_SHIFT      (0)
#define CR1_ALS_GAIN_MASK       (0x7 << CR1_ALS_GAIN_SHIFT)


/*PS fault queue depth for interrupt event output*/
#define CR2_PS_FQ_1     (0x0)
#define CR2_PS_FQ_4     (0x1)
#define CR2_PS_FQ_8     (0x2)
#define CR2_PS_FQ_15    (0x3)

#define CR2_PS_FQ_SHIFT       (6)
#define CR2_PS_FQ_MASK        (0x3 << CR2_PS_FQ_SHIFT)

/*interrupt type setting */
/*low active*/
#define CR2_INT_LEVEL   0x0 
/*low pulse*/
#define CR2_INT_PULSE   0x1 

#define CR2_INT_SHIFT       (5)
#define CR2_INT_MASK        (0x1 << CR2_INT_SHIFT)

/*resolution for PS*/
#define CR2_PS_RES_12     (0x0)
#define CR2_PS_RES_14     (0x1)
#define CR2_PS_RES_16     (0x2)
#define CR2_PS_RES_16_2   (0x3)

#define CR2_PS_RES_SHIFT        (3)
#define CR2_PS_RES_MASK         (0x3 << CR2_PS_RES_SHIFT)

/*sensing amplifier selection for PS*/
#define CR2_PS_GAIN_1     (0x0)
#define CR2_PS_GAIN_2     (0x1)
#define CR2_PS_GAIN_4     (0x2)
#define CR2_PS_GAIN_8     (0x3)
#define CR2_PS_GAIN_16    (0x4)
#define CR2_PS_GAIN_32    (0x5)
#define CR2_PS_GAIN_64    (0x6)
#define CR2_PS_GAIN_128   (0x7)

#define CR2_PS_GAIN_SHIFT       (0)
#define CR2_PS_GAIN_MASK        (0x7 << CR2_PS_GAIN_SHIFT)

/*wait-time slot selection*/
#define CR3_WAIT_SEL_0    (0x0)
#define CR3_WAIT_SEL_4    (0x1)
#define CR3_WAIT_SEL_8    (0x2)
#define CR3_WAIT_SEL_16   (0x3)

#define CR3_WAIT_SEL_SHIFT      (6)
#define CR3_WAIT_SEL_MASK       (0x3 << CR3_WAIT_SEL_SHIFT)

/*IR-LED drive peak current setting*/
#define CR3_LEDDR_12_5  (0x0)
#define CR3_LEDDR_25    (0x1)
#define CR3_LEDDR_50    (0x2)
#define CR3_LEDDR_100   (0x3)

#define CR3_LEDDR_SHIFT       (4)
#define CR3_LEDDR_MASK        (0x3 << CR3_LEDDR_SHIFT)

/*INT pin source selection*/
#define CR3_INT_SEL_BATH  (0x0)
#define CR3_INT_SEL_ALS   (0x1)
#define CR3_INT_SEL_PS    (0x2)
#define CR3_INT_SEL_PSAPP (0x3)

#define CR3_INT_SEL_SHIFT       (2)
#define CR3_INT_SEL_MASK        (0x3 << CR3_INT_SEL_SHIFT)

/*software reset for register and core*/
#define CR3_SOFTRST_EN    (0x1)

#define CR3_SOFTRST_SHIFT       (0)
#define CR3_SOFTRST_MASK        (0x1 << CR3_SOFTRST_SHIFT)

/*modulation frequency of LED driver*/
#define CR10_FREQ_DIV2    (0x0)
#define CR10_FREQ_DIV4    (0x1)
#define CR10_FREQ_DIV8    (0x2)
#define CR10_FREQ_DIV16   (0x3)

#define CR10_FREQ_SHIFT       (1)
#define CR10_FREQ_MASK        (0x3 << CR10_FREQ_SHIFT)

/*50/60 Rejection enable*/
#define CR10_REJ_5060_DIS (0x00)
#define CR10_REJ_5060_EN  (0x01)

#define CR10_REJ_5060_SHIFT     (0)
#define CR10_REJ_5060_MASK      (0x1 << CR10_REJ_5060_SHIFT)

#define us5182_NUM_CACHABLE_REGS (0x12)

extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);

typedef struct cmd_table_st {
  u8  reg;
  u8  data;
} cmd_table_type;

struct us5182_priv {
  struct alsps_hw     *hw;
  struct i2c_client   *client;
  struct delayed_work eint_work;
  struct delayed_work core_rst_work;
  struct delayed_work get_ps_work;
  struct delayed_work enable_ps_work;
  struct delayed_work disable_ps_work;
  struct delayed_work get_als_work;
  struct hrtimer      get_ps_timer;
  struct mutex        op_mutex;
    
  /*misc*/
  atomic_t  trace;
  atomic_t  i2c_retry;
  atomic_t  als_suspend;
  atomic_t  als_debounce;   /*debounce time after enabling als*/
  atomic_t  als_deb_on;     /*indicates if the debounce is on*/
  atomic_t  als_deb_end;    /*the jiffies representing the end of debounce*/
  atomic_t  ps_mask;        /*mask ps: always return far away*/
  atomic_t  ps_debounce;    /*debounce time after enabling ps*/
  atomic_t  ps_deb_on;      /*indicates if the debounce is on*/
  atomic_t  ps_deb_end;     /*the jiffies representing the end of debounce*/
  atomic_t  ps_suspend;
  atomic_t  core_rst;

  /*data*/
  u8  _align1;
  u8  _align2;
  u8  _align;
  u16 als_level_num;
  u16 als_value_num;
  u32 als_level[C_CUST_ALS_LEVEL-1];
  u32 als_value[C_CUST_ALS_LEVEL];
  u32 als;
  u32 ps;
  int als_data;
  
  bool          als_enable;         /*record current als status*/
  unsigned int  als_window_loss; 
  
  bool          ps_enable;          /*record current ps status*/
  unsigned int  ps_thd_val;     /*the cmd value can't be read, stored in ram*/
  ulong         enable;           /*record HAL enalbe status*/
  ulong         pending_intr;     /*pending interrupt*/
  u8            polarity;
  u8            eint_count;       ///< [AT-PM] : Count of interrupt work ; 09/01/2014
  u8            eint_high_count;  ///< [AT-PM] : Count of high polarity ; 09/01/2014
  u8            eint_low_count;   ///< [AT-PM] : Count of low polarity ; 09/01/2014
  bool          eint_init;
  int           last_report_ps;

  bool  ps_get_start;
  u32   ps_last_jiffies;
  u32   ps_high_time;
  u32   ps_low_time;
  int   ps_data;
  int   ps_data_prev_raw;
  u32   ps_high_to_low;
  u32   ps_low_to_high;
  u32   ps_poll_time;

#if defined(CONFIG_HAS_EARLYSUSPEND)
  struct early_suspend  early_drv;
#endif     
};

extern struct us5182_priv *us5182_obj;

#endif
