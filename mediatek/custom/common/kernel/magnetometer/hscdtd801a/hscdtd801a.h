/* include/linux/ami304.h - HSCDTD801A compass driver
 *
 * Copyright (C) 2009 AMIT Technology Inc.
 * Author: Kyle Chen <sw-support@amit-inc.com>
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
 * Definitions for ami304 compass chip.
 */
#ifndef HSCDTD801A_H
#define HSCDTD801A_H

#include <linux/ioctl.h>
//#include <asm-arm/arch/regs-gpio.h>

#define HSCDTD801A_I2C_ADDRESS 			0x0C

#define CALIBRATION_DATA_SIZE	12  

/* HSCDTD801A Internal Register Address  (Please refer to HSCDTD801A Specifications) */
#define HSCDTD801A_CHIP_ID		0x1849
#define HSCDTD801A_REG_WIA		0x00
#define HSCDTD801A_REG_INFO	0x01
#define HSCDTD801A_REG_STATUS	0x02
#define HSCDTD801A_REG_XOUT	0x04
#define HSCDTD801A_REG_YOUT	0x06
#define HSCDTD801A_REG_ZOUT	0x08
#define HSCDTD801A_REG_TOUT	0x0A
#define HSCDTD801A_REG_DATAXL	0x04
#define HSCDTD801A_REG_DATAXH	0x05
#define HSCDTD801A_REG_DATAYL	0x06
#define HSCDTD801A_REG_DATAYH	0x07
#define HSCDTD801A_REG_DATAZL	0x08
#define HSCDTD801A_REG_DATAZH	0x09
#define HSCDTD801A_REG_DATATL	0x0A
#define HSCDTD801A_REG_DATATH	0x0B
#define HSCDTD801A_REG_STRA	0x0E
#define HSCDTD801A_REG_CTRL1	0x10
#define HSCDTD801A_REG_CTRL2	0x11
#define HSCDTD801A_REG_CCTRL	0x13
#define HSCDTD801A_REG_ACTRL	0x14
#define HSCDTD801A_REG_FACTL	0x15

#define HSCDTD801A_ACTL_MEA_MAG			0x01
#define HSCDTD801A_CTRL1_REG_ACT_MODE	0x11
#define HSCDTD801A_FACT_SRST				0x80  //SW reset

#define HSCDTD801A_BUFSIZE					256
#define HSCDTD801A_NORMAL_MODE			0
#define HSCDTD801A_ACTION_MODE			1

#define CONVERT_M                       1//25
#define CONVERT_O                       1//25
#define CONVERT_M_DIV		1000	
#define CONVERT_O_DIV		100	

// conversion of magnetic data to nT units
#define ORIENTATION_ACCURACY_RATE        	100

enum {
	ACTIVE_SS_NUL = 0x00 ,
	ACTIVE_SS_ACC = 0x01 ,
	ACTIVE_SS_MAG = 0x02 ,
	ACTIVE_SS_ORI = 0x04 ,
	ACTIVE_SS_GYR = 0x08 ,
	ACTIVE_SS_LIN = 0x10 ,
	ACTIVE_SS_ROT = 0x20 ,
};

struct TAIFD_HW_DATA {
	int activate;
	int delay;
	int acc[4];
	int mag[4];
};

struct TAIFD_SW_DATA {
	int acc[5];
	int mag[5];
	int ori[5];
	int gyr[5];
	int lin[4];
	int rot[5];
};



#endif
