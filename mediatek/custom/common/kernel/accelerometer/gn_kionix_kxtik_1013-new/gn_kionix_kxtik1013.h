/* linux/drivers/hwmon/adxl345.c
 *
 * (C) Copyright 2008 
 * MediaTek <www.mediatek.com>
 *
 * KXTIK1013 driver for MT6575
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  KXTIK
 */
#ifndef KXTIK1013_H
#define KXTIK1013_H
	 
#include <linux/ioctl.h>
	 
#define KXTIK1013_I2C_SLAVE_ADDR				0x1e
	 
 /* KXTIK1013 Register Map  (Please refer to KXTIK1013 Specifications) */
#define KXTIK1013_REG_DEVID						0x0F
#define	KXTIK1013_REG_BW_RATE						0x21
#define KXTIK1013_REG_POWER_CTL					0x1B
#define KXTIK1013_REG_CTL_REG3					0x1D
#define KXTIK1013_DCST_RESP						0x0C
#define KXTIK1013_REG_DATA_FORMAT				0x1B
#define KXTIK1013_REG_DATA_RESOLUTION			0x1B
#define KXTIK1013_RANGE_DATA_RESOLUTION_MASK	0x40
#define KXTIK1013_REG_DATAX0						0x06	 
#define KXTIK1013_FIXED_DEVID					0x12	 
#define KXTIK1013_BW_200HZ						0x04
#define KXTIK1013_BW_100HZ						0x03
#define KXTIK1013_BW_50HZ						0x02	 
#define KXTIK1013_MEASURE_MODE					0x80		 
#define KXTIK1013_RANGE_MASK						0x18
#define KXTIK1013_RANGE_2G						0x00
#define KXTIK1013_RANGE_4G						0x08
#define KXTIK1013_RANGE_8G						0x10
#define KXTIK1013_REG_INT_ENABLE				0x1E

/* Gionee BSP1 chengx 20140828 modify for xxxx begin */
#define KXTIK1013_INT_REL                     0x1A
#define KXTIK1013_MOTION_REG                  0x1B
#define KXTIK1013_MOTION                      0x02
#define KXTIK1013_WAKEUP_TIMER                0x29
#define KXTIK1013_WAKEUP_THRESHOLD            0x6A
/* Gionee BSP1 chengx 20140828 modify for xxxx end */

#define KXTIK1013_SELF_TEST						0x10
	 
#define KXTIK1013_SUCCESS						0
#define KXTIK1013_ERR_I2C						-1
#define KXTIK1013_ERR_STATUS						-3
#define KXTIK1013_ERR_SETUP_FAILURE				-4
#define KXTIK1013_ERR_GETGSENSORDATA			-5
#define KXTIK1013_ERR_IDENTIFICATION			-6
	 
#define KXTIK1013_BUFSIZE						256

#define KXTIK1013_AXES_NUM        3



/****** aimi add *******/
/*----------------------------------------------------------------------------*/
typedef enum{
   KXTIK1013_CUST_ACTION_SET_CUST = 1,
   KXTIK1013_CUST_ACTION_SET_CALI,
   KXTIK1013_CUST_ACTION_RESET_CALI
}CUST_ACTION;
/*----------------------------------------------------------------------------*/
typedef struct
{
    uint16_t    action;
}KXTIK1013_CUST;

/*----------------------------------------------------------------------------*/
typedef struct
{
    uint16_t    action;
    uint16_t    part;
    int32_t     data[0];
}KXTIK1013_SET_CUST;
/*----------------------------------------------------------------------------*/
typedef struct
{
    uint16_t    action;
    int32_t     data[KXTIK1013_AXES_NUM];
}KXTIK1013_SET_CALI;
/*----------------------------------------------------------------------------*/
typedef KXTIK1013_CUST KXTIK1013_RESET_CALI;
/*----------------------------------------------------------------------------*/
typedef union
{
	uint32_t				data[10];
	KXTIK1013_CUST		 cust;
	KXTIK1013_SET_CUST	 setCust;
	KXTIK1013_SET_CALI	 setCali;
	KXTIK1013_RESET_CALI	 resetCali;
}KXTIK1013_CUST_DATA;
/****** aimi add end*******/ 
#endif

