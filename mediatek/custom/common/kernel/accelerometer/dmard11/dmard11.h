/* linux/drivers/hwmon/adxl345.c
 *
 * (C) Copyright 2008 
 * MediaTek <www.mediatek.com>
 *
 * BMA150 driver for MT6516
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  BMA150
 */
#ifndef DMARD11_H
#define DMARD11_H
	 
#include <linux/ioctl.h>
	 
	//#define DMARD08_I2C_SLAVE_WRITE_ADDR		0x38
	#define DMARD11_I2C_SLAVE_WRITE_ADDR		0x3a 
	
#define DMARD11_SUCCESS						0
#define DMARD11_ERR_I2C						-1
#define DMARD11_ERR_STATUS					-3
#define DMARD11_ERR_SETUP_FAILURE			-4
#define DMARD11_ERR_GETGSENSORDATA			-5
#define DMARD11_ERR_IDENTIFICATION			-6
	 
#define DMARD11_BUFSIZE				256

#define REG_ACTR 				0x00//
#define REG_STAT 				0x0A//
#define REG_DX					0x0C//
#define REG_DY					0x0E//
#define REG_DZ	 				0x10//
#define REG_DT	 				0x12//
#define REG_CNT_L1 				0x1B//
#define REG_CNT_L2				0x1C//
#define REG_CNT_L3				0x1D//
#define REG_FILTER				0x1A//
#define REG_INC 				0x1E//
#define REG_DSP 				0x20//?
#define REG_THR1 				0x62//
//#define REG_THR2 				0x64
#define	REG_SP					0x30//
#define REG_WHO	 				0x18	// Who AM I Register

#define MODE_ACTIVE				0x01//?	/* active  with rewind*/
#define MODE_POWERDOWN			0x00//?	/* powerdown */


#define VALUE_INIT_READY        0x01	//0x02    /*IC init ok*/
#define VALUE_WHO_AM_I			0x11//	/* D11 WMI */

#define USE_MTK_CALIBRATE
#ifndef USE_MTK_CALIBRATE
#define AVG_NUM 				16
#define SENSOR_DATA_SIZE 		3
 
//#define MANUAL_CHANGE_XYZ
#define G_SENSOR_SELF_TEST
//#define STABLE_VALUE_FUNCTION
#define CALIBRATE_INVERSE

#define IC_SENSITIVITY 	256
#define SELF_TEST_RANGE	IC_SENSITIVITY/2*3

#define DEFAULT_SENSITIVITY 	9800
#define	ACC_VALUE_03	DEFAULT_SENSITIVITY/100*3
#define	ACC_VALUE_04	DEFAULT_SENSITIVITY/100*4
#define	ACC_VALUE_15	DEFAULT_SENSITIVITY/10*15



typedef union {
	struct {
		s16	x;
		s16	y;
		s16	z;
	} u;
	s16	v[SENSOR_DATA_SIZE];
} raw_data;
#endif
	 
#endif

