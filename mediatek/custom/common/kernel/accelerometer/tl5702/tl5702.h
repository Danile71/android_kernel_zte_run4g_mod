/*
 * TLi TL5702. Digital, triaxial acceleration sensor driver.
 *
 * Copyright (C) 2014 Technology Leaders & Innovators.
 *
 * File Name	: tl5702.c
 * Author		: TLi <http://www.tli.co.kr>
 * Version	: V 1.0.1
 *
 * REVISION HISTORY
 * 2014/07/08 : fine offset calibration variable add 
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __TL5702_H
#define __TL5702_H

#include <linux/ioctl.h>

#define TLI_TL5702_DEBUG
#ifdef TLI_TL5702_DEBUG
#define TL5702PRINTK(fmt, args...) \
	printk(KERN_INFO "[TL5702] %s(%d): " fmt, __FUNCTION__, __LINE__ , ## args)
#else
#define TL5702PRINTK(fmt, args...)
#endif

#define TL5702_VERSION "1.0.0"
#define TL5702_NAME    "tl5702"

#ifdef bestone
//add by znh 2013.5.21
#define TL5702_ACC_I2C_ADDR	0x56
#define TL5702_ACC_I2C_NAME	TL5702_NAME
#define CONFIG_SENSORS_TL5702_POSITION_V2  0	
#define CONFIG_SENSORS_TL5702_I2C
#define I2C_BUS_NUM_STATIC_ALLOC 
#define I2C_STATIC_BUS_NUM        (0)
#endif

//by wskim 20140704
#define	FINEOFFSET_RATIO				2
#define	OFFSET_CAL_CNT					1

#define	before_lot_403
#ifdef	before_lot_403
#define	OFFSET_CAL_X					-19
#define	OFFSET_CAL_Y					 2
#define	OFFSET_CAL_Z					 34
#else
#define	OFFSET_CAL_X					-26
#define	OFFSET_CAL_Y					-3
#define	OFFSET_CAL_Z					 29
#endif

/*
 * Registers
 */
#define TL5702_DEV_ID_REG               0x00
#define TL5702_DEV_ID                   0xA0

#define TL5702_ACC_REG                  0x02

#define TL5702_TEMPER_REG               0x08

#define TL5702_INT_FLAG_REG             0x09

#define TL5702_RANGE_REG                0x0E
#define TL5702_RANGE_MASK               0x03
#define TL5702_RANGE_SHIFT              0
#define TL5702_RANGE_2G                 0
#define TL5702_RANGE_4G                 1
#define TL5702_RANGE_8G                 2
#define TL5702_RANGE_16G                3

#define TL5702_BANDWIDTH_REG            0x0E
#define TL5702_BANDWIDTH_MASK           0x70
#define TL5702_BANDWIDTH_SHIFT          4
#define TL5702_BANDWIDTH_7_81HZ         7
#define TL5702_BANDWIDTH_15_63HZ        6
#define TL5702_BANDWIDTH_31_25HZ        5
#define TL5702_BANDWIDTH_62_5HZ         4
#define TL5702_BANDWIDTH_125HZ          3
#define TL5702_BANDWIDTH_250HZ          2
#define TL5702_BANDWIDTH_500HZ          1
#define TL5702_BANDWIDTH_1000HZ         0

#define TL5702_LPWR_MODE_REG            0x0F
#define TL5702_DSLP_EN_MASK             0x80
#define TL5702_LPWR_EN_MASK             0x40
#define TL5702_LPWR_SLEEP_DUR_MASK      0x0F
#define TL5702_DSLP_EN_SHIFT            7
#define TL5702_LPWR_EN_SHIFT            6
#define TL5702_LPWR_SLEEP_DUR_SHIFT     0
#define TL5702_LPWR_SLEEP_DUR_0_5ms     0
#define TL5702_LPWR_SLEEP_DUR_1ms	    1
#define TL5702_LPWR_SLEEP_DUR_2ms	    2
#define TL5702_LPWR_SLEEP_DUR_5ms	    3
#define TL5702_LPWR_SLEEP_DUR_8ms	    4
#define TL5702_LPWR_SLEEP_DUR_10ms	    5
#define TL5702_LPWR_SLEEP_DUR_20ms	    6
#define TL5702_LPWR_SLEEP_DUR_50ms	    7
#define TL5702_LPWR_SLEEP_DUR_80ms	    8
#define TL5702_LPWR_SLEEP_DUR_100ms	    9
#define TL5702_LPWR_SLEEP_DUR_200ms	    10
#define TL5702_LPWR_SLEEP_DUR_500ms	    11
#define TL5702_LPWR_SLEEP_DUR_800ms	    12
#define TL5702_LPWR_SLEEP_DUR_1s	    13
#define TL5702_LPWR_SLEEP_DUR_1_25s	    14
#define TL5702_LPWR_SLEEP_DUR_1_5s	    15

#define TL5702_ACCFLT_REG				0x10

#define TL5702_SPI_MODE_REG				0x12

#define TL5702_SOFT_RESET_REG           0x15
#define TL5702_SOFT_RESET_MASK          0xA5

#define TL5702_INTEN_0_REG	            0x16

#define TL5702_INTEN_1_REG	            0x17

#define TL5702_INT1_MAP_REG	            0x18

#define TL5702_INT2_MAP_REG	            0x19

#define TL5702_INT_NEW_REG	            0x1A

#define TL5702_INT_RESET_REG            0x2F
#define TL5702_INT_RESET_MASK           0x01
#define TL5702_INT_RESET_SHIFT          0

#define TL5702_I2CWDT_REG	            0x12

#define TL5702_STEST_CTL_REG            0x13
#define TL5702_STEST_AXIS_MASK          0x06
#define TL5702_STEST_SIGN_MASK          0x01
#define TL5702_STEST_AXIS_SHIFT	        1
#define TL5702_STEST_SIGN_SHIFT	        0
#define TL5702_STEST_AXIS_X		        1
#define TL5702_STEST_AXIS_Y		        2
#define TL5702_STEST_AXIS_Z		        3

#define TL5702_NVM_ADDR_REG             0x30
#define TL5702_NVM_DW_0_REG             0x31
#define TL5702_NVM_DW_1_REG             0x32
#define TL5702_NVM_CTL_REG              0x33
#define TL5702_NVM_RST_REG              0x34
#define TL5702_NVM_MTR_0_REG            0x35
#define TL5702_NVM_MTR_1_REG            0x36

#define TL5702_BG_TRIM_REG	            0x5B
#define TL5702_TMODESEL_REG	            0x5E
#define TL5702_F_X_OFF_H_REG            0x5F
#define TL5702_F_X_OFF_L_REG            0x60
#define TL5702_F_Y_OFF_H_REG            0x61
#define TL5702_F_Y_OFF_L_REG            0x62
#define TL5702_F_Z_OFF_H_REG            0x63
#define TL5702_F_Z_OFF_L_REG            0x64

#ifdef bestone
/*
 * Acceleration measurement
 */
#define TL5702_RESOLUTION               256

/* ABS axes parameter range [um/s^2] (for input event) */
#define GRAVITY_EARTH                   9806550
#define ABSMIN_2G                       (-GRAVITY_EARTH * 2)
#define ABSMAX_2G                       (GRAVITY_EARTH * 2)
#endif

#define TL5702_SUCCESS					0
#define TL5702_ERR_I2C					-1
#define TL5702_ERR_STATUS				-3
#define TL5702_ERR_SETUP_FAILURE			-4
#define TL5702_ERR_GETGSENSORDATA			-5
#define TL5702_ERR_IDENTIFICATION			-6

#endif  /* __TL5702_H */

