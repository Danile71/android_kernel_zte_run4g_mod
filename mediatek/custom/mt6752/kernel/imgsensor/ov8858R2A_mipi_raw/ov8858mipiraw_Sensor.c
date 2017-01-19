/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 OV8858mipiraw_sensor.c
 *
 * Project:
 * --------
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 *	PengtaoFan
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>
#include <linux/xlog.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov8858mipiraw_Sensor.h"

#define PFX "OV8858R2A"
//#define LOG_WRN(format, args...) xlog_printk(ANDROID_LOG_WARN ,PFX, "[%S] " format, __FUNCTION__, ##args)
//#defineLOG_INF(format, args...) xlog_printk(ANDROID_LOG_INFO ,PFX, "[%s] " format, __FUNCTION__, ##args)
//#define LOG_DBG(format, args...) xlog_printk(ANDROID_LOG_DEBUG ,PFX, "[%S] " format, __FUNCTION__, ##args)
#define LOG_INF(format, args...)	xlog_printk(ANDROID_LOG_INFO   , PFX, "[%s] " format, __FUNCTION__, ##args)

static DEFINE_SPINLOCK(imgsensor_drv_lock);


static imgsensor_info_struct imgsensor_info = { 
	.sensor_id = OV8858_SENSOR_ID,		//record sensor id defined in Kd_imgsensor.h
	
	.checksum_value = 0xd6d43c1f,		//checksum value for Camera Auto Test
	
	.pre = {
		.pclk = 72000000,				//record different mode's pclk
		.linelength  = 1928,				//record different mode's linelength
		.framelength = 1244,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 1632,		//record different mode's width of grabwindow
		.grabwindow_height = 1224,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 23,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,	
	},
	.cap = {
		.pclk = 144000000,
		.linelength  = 1940,
		.framelength = 2474,
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 300,
	},
	.cap1 = {							//capture for PIP 24fps relative information, capture1 mode must use same framelength, linelength with Capture mode for shutter calculate
		.pclk = 144000000,
		.linelength  = 2424,
		.framelength = 2474,
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 240,
	},
	.normal_video = {
		.linelength  = 1940,
		.framelength = 2474,
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 144000000,				//record different mode's pclk
		.linelength  = 2566,				//record different mode's linelength
		.framelength = 1872,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 3264,		//record different mode's width of grabwindow
		.grabwindow_height = 2448,		//record different mode's height of grabwindow
		/*	 following for  MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 30,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,	
	},
	.slim_video = {
		.pclk = 72000000,				//record different mode's pclk
		.linelength  = 1928,				//record different mode's linelength
		.framelength = 1244,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 1632,		//record different mode's width of grabwindow
		.grabwindow_height = 1224,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 30,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,	

	},
	.margin = 4,			//sensor framelength & shutter margin
	.min_shutter = 4,		//min shutter
	.max_frame_length = 0x90f7,//max framelength by sensor register's limitation
	.ae_shut_delay_frame = 0,	//shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2
	.ae_sensor_gain_delay_frame = 0,//sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2
	.ae_ispGain_delay_frame = 2,//isp gain delay frame for AE cycle
	.ihdr_support = 0,	  //1, support; 0,not support
	.ihdr_le_firstline = 0,  //1,le first ; 0, se first
	.sensor_mode_num = 3,	  //support sensor mode num ,don't support Slow motion
	
	.cap_delay_frame = 3,		//enter capture delay frame num
	.pre_delay_frame = 3, 		//enter preview delay frame num
	.video_delay_frame = 3,		//enter video delay frame num
	.hs_video_delay_frame = 3,	//enter high speed video  delay frame num
	.slim_video_delay_frame = 3,//enter slim video delay frame num
	
	.isp_driving_current = ISP_DRIVING_8MA, //mclk driving current
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,//sensor_interface_type
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_B,//sensor output first pixel color
	.mclk = 24,//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
	.mipi_lane_num = SENSOR_MIPI_4_LANE,//mipi lane num
	.i2c_addr_table = {0x42, 0xff},//record sensor support all write id addr, only supprt 4must end with 0xff
};


static imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,				//mirrorflip information
	.sensor_mode = IMGSENSOR_MODE_INIT, //IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video,High Speed Video, Slim Video
	.shutter = 0x4C00,					//current shutter
	.gain = 0x200,						//current gain
	.dummy_pixel = 0,					//current dummypixel
	.dummy_line = 0,					//current dummyline
	.current_fps = 0,  //full size current fps : 24fps for PIP, 30fps for Normal or ZSD
	.autoflicker_en = KAL_FALSE,  //auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker
	.test_pattern = KAL_FALSE,		//test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,//current scenario id
	.ihdr_en = 0, //sensor need support LE, SE with HDR feature
	.i2c_write_id = 0x42,//record current sensor's i2c write id
};


/* Sensor output window information*/
/*
static SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[5] =	 
{{ 3296, 2528,	  12,	12, 3272, 2456, 3264, 2452,   04,	02, 1632, 1224,   0,	0, 1632, 1224}, // Preview 
 { 3296, 2528,	         12,	12, 3272, 2468, 3264, 2452,   04,   02, 3264, 2448,   0,	0, 3264, 2448}, // capture 
 { 3296, 2528,	  12,	12, 3272, 2456, 3264, 2452,   04,	02, 3264, 2448,   0,	0, 3264, 2448}, // video 
 { 3296, 2528,	  12,	12, 3272, 2456, 3264, 2452,   04,	02, 3264, 2448,   0,	0, 3264, 2448}, //hight speed video 
 { 3296, 2528,	  12,	12, 3272, 2456, 3264, 2452,   04,	02, 3264, 2448,   0,	0, 1632, 1224}};// slim video 
 */
static SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[5] =	 
{{ 3296, 2480,	  12,	12, 3272, 2456, 3264, 2452,   4,	2, 1632, 1224,	 0, 0, 1632, 1224}, // Preview 
 { 3296, 2480,	  12,	12, 3272, 2456, 3264, 2452,   4,	2, 3264, 2448,	 0, 0, 3264, 2448}, // capture 
 { 3296, 2480,	  12,	12, 3272, 2456, 3264, 2452,   4,	2, 3264, 2448,	 0, 0, 3264, 2448}, // video 
 { 3296, 2480,	  12,	12, 3272, 2456, 3264, 2452,   4,	2, 3264, 2448,	 0, 0, 3264, 2448}, //hight speed video 
 { 3296, 2480,	  12,	12, 3272, 2456, 3264, 2452,   4,	2, 1632, 1224,	 0, 0, 1632, 1224}};// slim video 

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };
	iReadRegI2C(pu_send_cmd, 2, (u8*)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}

static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}

static void set_dummy()
{
	LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line, imgsensor.dummy_pixel);
	/* you can set dummy by imgsensor.dummy_line and imgsensor.dummy_pixel, or you can set dummy by imgsensor.frame_length and imgsensor.line_length */
	write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
	write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);	  
	write_cmos_sensor(0x380c, imgsensor.line_length >> 8);
	write_cmos_sensor(0x380d, imgsensor.line_length & 0xFF);
  
}	/*	set_dummy  */


static void set_max_framerate(UINT16 framerate,kal_bool min_framelength_en)
{
	kal_int16 dummy_line;
	kal_uint32 frame_length = imgsensor.frame_length;
	//unsigned long flags;

	LOG_INF("framerate = %d, min framelength should enable? \n", framerate,min_framelength_en);
   
	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length; 
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	//dummy_line = frame_length - imgsensor.min_frame_length;
	//if (dummy_line < 0)
		//imgsensor.dummy_line = 0;
	//else
		//imgsensor.dummy_line = dummy_line;
	//imgsensor.frame_length = frame_length + imgsensor.dummy_line;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
	{
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */


static void write_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;
	kal_uint32 frame_length = 0;
	   
	/* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
	/* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */
	
	// OV Recommend Solution
	// if shutter bigger than frame_length, should extend frame length first
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)		
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

    // Framelength should be an even number
    shutter = (shutter >> 1) << 1;
    imgsensor.frame_length = (imgsensor.frame_length >> 1) << 1;
	
	if (imgsensor.autoflicker_en) { 
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296,0);
		else if(realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146,0);	
	} else {
		// Extend frame length
		write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
	}

	// Update Shutter
	write_cmos_sensor(0x3500, (shutter>>12) & 0x0F);
	write_cmos_sensor(0x3501, (shutter>>4) & 0xFF);
	write_cmos_sensor(0x3502, (shutter<<4) & 0xF0);	
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);

	//LOG_INF("frame_length = %d ", frame_length);
	
}	/*	write_shutter  */



/*************************************************************************
* FUNCTION
*	set_shutter
*
* DESCRIPTION
*	This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*	iShutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	kal_uint32 frame_length = 0;
	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);
	
	//write_shutter(shutter);
	/* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
	/* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */
	
	// OV Recommend Solution
	// if shutter bigger than frame_length, should extend frame length first
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)		
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;
	
	if (imgsensor.autoflicker_en) { 
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296,0);
		else if(realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146,0);	
		else {
		// Extend frame length
		write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
		}
	} else {
		// Extend frame length
		write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
	}

	// Update Shutter
	write_cmos_sensor(0x3502, (shutter << 4) & 0xFF);
	write_cmos_sensor(0x3501, (shutter >> 4) & 0xFF);	  
	write_cmos_sensor(0x3500, (shutter >> 12) & 0x0F);	
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);

}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0000;
	
	reg_gain = gain*2;
	//reg_gain = reg_gain & 0xFFFF;
	return (kal_uint16)reg_gain;
}

/*************************************************************************
* FUNCTION
*	set_gain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*	iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	LOG_INF("set_gain %d \n", gain);

	kal_uint16 reg_gain;
	if (gain < BASEGAIN || gain > 32 * BASEGAIN) {
		LOG_INF("Error gain setting");

		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 32 * BASEGAIN)
			gain = 32 * BASEGAIN;		 
	}

	//reg_gain = gain2reg(gain);
	reg_gain = gain*2;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain; 
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

	write_cmos_sensor(0x3508, (reg_gain>>8));
	write_cmos_sensor(0x3509, (reg_gain&0xFF));    

	return gain;
}	/*	set_gain  */

static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
	LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n",le,se,gain);
	if (imgsensor.ihdr_en) {
		
		spin_lock(&imgsensor_drv_lock);
			if (le > imgsensor.min_frame_length - imgsensor_info.margin)		
				imgsensor.frame_length = le + imgsensor_info.margin;
			else
				imgsensor.frame_length = imgsensor.min_frame_length;
			if (imgsensor.frame_length > imgsensor_info.max_frame_length)
				imgsensor.frame_length = imgsensor_info.max_frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (le < imgsensor_info.min_shutter) le = imgsensor_info.min_shutter;
			if (se < imgsensor_info.min_shutter) se = imgsensor_info.min_shutter;
			
			
		// Extend frame length first
		write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);

		write_cmos_sensor(0x3502, (le << 4) & 0xFF);
		write_cmos_sensor(0x3501, (le >> 4) & 0xFF);	 
		write_cmos_sensor(0x3500, (le >> 12) & 0x0F);
		
		write_cmos_sensor(0x3508, (se << 4) & 0xFF); 
		write_cmos_sensor(0x3507, (se >> 4) & 0xFF);
		write_cmos_sensor(0x3506, (se >> 12) & 0x0F); 

		set_gain(gain);
	}

}



static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

	/********************************************************
	   *
	   *   0x3820[2] ISP Vertical flip
	   *   0x3820[1] Sensor Vertical flip
	   *
	   *   0x3821[2] ISP Horizontal mirror
	   *   0x3821[1] Sensor Horizontal mirror
	   *
	   *   ISP and Sensor flip or mirror register bit should be the same!!
	   *
	   ********************************************************/
	
	switch (image_mirror) {
		case IMAGE_NORMAL:
			write_cmos_sensor(0x3820,((read_cmos_sensor(0x3820) & 0xF9) | 0x00));
			write_cmos_sensor(0x3821,((read_cmos_sensor(0x3821) & 0xF9) | 0x06));
			break;
		case IMAGE_H_MIRROR:
			write_cmos_sensor(0x3820,((read_cmos_sensor(0x3820) & 0xF9) | 0x00));
			write_cmos_sensor(0x3821,((read_cmos_sensor(0x3821) & 0xF9) | 0x00));
			break;
		case IMAGE_V_MIRROR:
			write_cmos_sensor(0x3820,((read_cmos_sensor(0x3820) & 0xF9) | 0x06));
			write_cmos_sensor(0x3821,((read_cmos_sensor(0x3821) & 0xF9) | 0x06));		
			break;
		case IMAGE_HV_MIRROR:
			write_cmos_sensor(0x3820,((read_cmos_sensor(0x3820) & 0xF9) | 0x06));
			write_cmos_sensor(0x3821,((read_cmos_sensor(0x3821) & 0xF9) | 0x00));
			break;
		default:
			LOG_INF("Error image_mirror setting\n");
	}

}

/*************************************************************************
* FUNCTION
*	night_mode
*
* DESCRIPTION
*	This function night mode of sensor.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/ 
}	/*	night_mode	*/
static void r1asensor_init(void)
{
	LOG_INF("E\n");
	//OV8858 R1A key initial setting for 4lane
	write_cmos_sensor(0x103 , 0x01);
	write_cmos_sensor(0x100 , 0x00);
	write_cmos_sensor(0x302 , 0x1e);
	write_cmos_sensor(0x303 , 0x00);
	write_cmos_sensor(0x304 , 0x03);
	write_cmos_sensor(0x30e , 0x02);
	write_cmos_sensor(0x30f , 0x04);
	write_cmos_sensor(0x312 , 0x03);
	write_cmos_sensor(0x31e , 0x0c);
	write_cmos_sensor(0x3600, 0x00);
	write_cmos_sensor(0x3601, 0x00);
	write_cmos_sensor(0x3602, 0x00);
	write_cmos_sensor(0x3603, 0x00);
	write_cmos_sensor(0x3604, 0x22);
	write_cmos_sensor(0x3605, 0x30);
	write_cmos_sensor(0x3606, 0x00);
	write_cmos_sensor(0x3607, 0x20);
	write_cmos_sensor(0x3608, 0x11);
	write_cmos_sensor(0x3609, 0x28);
	write_cmos_sensor(0x360a, 0x00);
	write_cmos_sensor(0x360b, 0x06);
	write_cmos_sensor(0x360c, 0xdc);
	write_cmos_sensor(0x360d, 0x40);
	write_cmos_sensor(0x360e, 0x0c);
	write_cmos_sensor(0x360f, 0x20);
	write_cmos_sensor(0x3610, 0x07);
	write_cmos_sensor(0x3611, 0x20);
	write_cmos_sensor(0x3612, 0x88);
	write_cmos_sensor(0x3613, 0x80);
	write_cmos_sensor(0x3614, 0x58);
	write_cmos_sensor(0x3615, 0x00);
	write_cmos_sensor(0x3616, 0x4a);
	write_cmos_sensor(0x3617, 0x90);
	write_cmos_sensor(0x3618, 0x56);
	write_cmos_sensor(0x3619, 0x70);
	write_cmos_sensor(0x361a, 0x99);
	write_cmos_sensor(0x361b, 0x00);
	write_cmos_sensor(0x361c, 0x07);
	write_cmos_sensor(0x361d, 0x00);
	write_cmos_sensor(0x361e, 0x00);
	write_cmos_sensor(0x361f, 0x00);
	write_cmos_sensor(0x3638, 0xff);
	write_cmos_sensor(0x3633, 0x0c);
	write_cmos_sensor(0x3634, 0x0c);
	write_cmos_sensor(0x3635, 0x0c);
	write_cmos_sensor(0x3636, 0x0c);
	write_cmos_sensor(0x3645, 0x13);
	write_cmos_sensor(0x3646, 0x83);
	write_cmos_sensor(0x364a, 0x07);
	write_cmos_sensor(0x3015, 0x00);
	write_cmos_sensor(0x3018, 0x72);
	write_cmos_sensor(0x3020, 0x93);
	write_cmos_sensor(0x3022, 0x01);
	write_cmos_sensor(0x3031, 0x0a);
	write_cmos_sensor(0x3034, 0x00);
	write_cmos_sensor(0x3106, 0x01);
	write_cmos_sensor(0x3305, 0xf1);
	write_cmos_sensor(0x3308, 0x00);
	write_cmos_sensor(0x3309, 0x28);
	write_cmos_sensor(0x330a, 0x00);
	write_cmos_sensor(0x330b, 0x20);
	write_cmos_sensor(0x330c, 0x00);
	write_cmos_sensor(0x330d, 0x00);
	write_cmos_sensor(0x330e, 0x00);
	write_cmos_sensor(0x330f, 0x40);
	write_cmos_sensor(0x3307, 0x04);
	write_cmos_sensor(0x3500, 0x00);
	write_cmos_sensor(0x3501, 0x4b);
	write_cmos_sensor(0x3502, 0xc0);
	write_cmos_sensor(0x3503, 0x00);
	write_cmos_sensor(0x3505, 0x80);
	write_cmos_sensor(0x3508, 0x01);
	write_cmos_sensor(0x3509, 0x00);
	write_cmos_sensor(0x350c, 0x00);
	write_cmos_sensor(0x350d, 0x80);
	write_cmos_sensor(0x3510, 0x00);
	write_cmos_sensor(0x3511, 0x02);
	write_cmos_sensor(0x3512, 0x00);
	write_cmos_sensor(0x3700, 0x18);
	write_cmos_sensor(0x3701, 0x0c);
	write_cmos_sensor(0x3702, 0x28);
	write_cmos_sensor(0x3703, 0x19);
	write_cmos_sensor(0x3704, 0x14);
	write_cmos_sensor(0x3705, 0x00);
	write_cmos_sensor(0x3706, 0x6a);
	write_cmos_sensor(0x3707, 0x04);
	write_cmos_sensor(0x3708, 0x24);
	write_cmos_sensor(0x3709, 0x33);
	write_cmos_sensor(0x370a, 0x01);
	write_cmos_sensor(0x370b, 0x6a);
	write_cmos_sensor(0x370c, 0x04);
	write_cmos_sensor(0x3718, 0x12);
	write_cmos_sensor(0x3719, 0x31);
	write_cmos_sensor(0x3712, 0x42);
	write_cmos_sensor(0x3714, 0x24);
	write_cmos_sensor(0x371e, 0x19);
	write_cmos_sensor(0x371f, 0x40);
	write_cmos_sensor(0x3720, 0x05);
	write_cmos_sensor(0x3721, 0x05);
	write_cmos_sensor(0x3724, 0x06);
	write_cmos_sensor(0x3725, 0x01);
	write_cmos_sensor(0x3726, 0x06);
	write_cmos_sensor(0x3728, 0x05);
	write_cmos_sensor(0x3729, 0x02);
	write_cmos_sensor(0x372a, 0x03);
	write_cmos_sensor(0x372b, 0x53);
	write_cmos_sensor(0x372c, 0xa3);
	write_cmos_sensor(0x372d, 0x53);
	write_cmos_sensor(0x372e, 0x06);
	write_cmos_sensor(0x372f, 0x10);
	write_cmos_sensor(0x3730, 0x01);
	write_cmos_sensor(0x3731, 0x06);
	write_cmos_sensor(0x3732, 0x14);
	write_cmos_sensor(0x3733, 0x10);
	write_cmos_sensor(0x3734, 0x40);
	write_cmos_sensor(0x3736, 0x20);
	write_cmos_sensor(0x373a, 0x05);
	write_cmos_sensor(0x373b, 0x06);
	write_cmos_sensor(0x373c, 0x0a);
	write_cmos_sensor(0x373e, 0x03);
	write_cmos_sensor(0x3755, 0x10);
	write_cmos_sensor(0x3758, 0x00);
	write_cmos_sensor(0x3759, 0x4c);
	write_cmos_sensor(0x375a, 0x06);
	write_cmos_sensor(0x375b, 0x13);
	write_cmos_sensor(0x375c, 0x20);
	write_cmos_sensor(0x375d, 0x02);
	write_cmos_sensor(0x375e, 0x00);
	write_cmos_sensor(0x375f, 0x14);
	write_cmos_sensor(0x3768, 0x22);
	write_cmos_sensor(0x3769, 0x44);
	write_cmos_sensor(0x376a, 0x44);
	write_cmos_sensor(0x3761, 0x00);
	write_cmos_sensor(0x3762, 0x00);
	write_cmos_sensor(0x3763, 0x00);
	write_cmos_sensor(0x3766, 0xff);
	write_cmos_sensor(0x376b, 0x00);
	write_cmos_sensor(0x3772, 0x23);
	write_cmos_sensor(0x3773, 0x02);
	write_cmos_sensor(0x3774, 0x16);
	write_cmos_sensor(0x3775, 0x12);
	write_cmos_sensor(0x3776, 0x04);
	write_cmos_sensor(0x3777, 0x00);
	write_cmos_sensor(0x3778, 0x17);
	write_cmos_sensor(0x37a0, 0x44);
	write_cmos_sensor(0x37a1, 0x3d);
	write_cmos_sensor(0x37a2, 0x3d);
	write_cmos_sensor(0x37a3, 0x00);
	write_cmos_sensor(0x37a4, 0x00);
	write_cmos_sensor(0x37a5, 0x00);
	write_cmos_sensor(0x37a6, 0x00);
	write_cmos_sensor(0x37a7, 0x44);
	write_cmos_sensor(0x37a8, 0x4c);
	write_cmos_sensor(0x37a9, 0x4c);
	write_cmos_sensor(0x3760, 0x00);
	write_cmos_sensor(0x376f, 0x01);
	write_cmos_sensor(0x37aa, 0x44);
	write_cmos_sensor(0x37ab, 0x2e);
	write_cmos_sensor(0x37ac, 0x2e);
	write_cmos_sensor(0x37ad, 0x33);
	write_cmos_sensor(0x37ae, 0x0d);
	write_cmos_sensor(0x37af, 0x0d);
	write_cmos_sensor(0x37b0, 0x00);
	write_cmos_sensor(0x37b1, 0x00);
	write_cmos_sensor(0x37b2, 0x00);
	write_cmos_sensor(0x37b3, 0x42);
	write_cmos_sensor(0x37b4, 0x42);
	write_cmos_sensor(0x37b5, 0x33);
	write_cmos_sensor(0x37b6, 0x00);
	write_cmos_sensor(0x37b7, 0x00);
	write_cmos_sensor(0x37b8, 0x00);
	write_cmos_sensor(0x37b9, 0xff);
	write_cmos_sensor(0x3800, 0x00);
	write_cmos_sensor(0x3801, 0x0c);
	write_cmos_sensor(0x3802, 0x00);
	write_cmos_sensor(0x3803, 0x0c);
	write_cmos_sensor(0x3804, 0x0c);
	write_cmos_sensor(0x3805, 0xd3);
	write_cmos_sensor(0x3806, 0x09);
	write_cmos_sensor(0x3807, 0xa3);
	write_cmos_sensor(0x3808, 0x06);
	write_cmos_sensor(0x3809, 0x60);
	write_cmos_sensor(0x380a, 0x04);
	write_cmos_sensor(0x380b, 0xc8);
	write_cmos_sensor(0x380c, 0x07);
	write_cmos_sensor(0x380d, 0x88);
	write_cmos_sensor(0x380e, 0x04);
	write_cmos_sensor(0x380f, 0xdc);
	write_cmos_sensor(0x3810, 0x00);
	write_cmos_sensor(0x3811, 0x04);
	write_cmos_sensor(0x3813, 0x02);
	write_cmos_sensor(0x3814, 0x03);
	write_cmos_sensor(0x3815, 0x01);
	write_cmos_sensor(0x3820, 0x00);
	write_cmos_sensor(0x3821, 0x67);
	write_cmos_sensor(0x382a, 0x03);
	write_cmos_sensor(0x382b, 0x01);
	write_cmos_sensor(0x3830, 0x08);
	write_cmos_sensor(0x3836, 0x02);
	write_cmos_sensor(0x3837, 0x18);
	write_cmos_sensor(0x3841, 0xff);
	write_cmos_sensor(0x3846, 0x48);
	write_cmos_sensor(0x3d85, 0x14);
	write_cmos_sensor(0x3f08, 0x08);
	write_cmos_sensor(0x3f0a, 0x00);
	write_cmos_sensor(0x4000, 0xf1);
	write_cmos_sensor(0x4001, 0x10);
	write_cmos_sensor(0x4005, 0x10);
	write_cmos_sensor(0x4002, 0x27);
	write_cmos_sensor(0x4009, 0x81);
	write_cmos_sensor(0x400b, 0x0c);
	write_cmos_sensor(0x401b, 0x00);
	write_cmos_sensor(0x401d, 0x00);
	write_cmos_sensor(0x4020, 0x00);
	write_cmos_sensor(0x4021, 0x04);
	write_cmos_sensor(0x4022, 0x04);
	write_cmos_sensor(0x4023, 0xb9);
	write_cmos_sensor(0x4024, 0x05);
	write_cmos_sensor(0x4025, 0x2a);
	write_cmos_sensor(0x4026, 0x05);
	write_cmos_sensor(0x4027, 0x2b);
	write_cmos_sensor(0x4028, 0x00);
	write_cmos_sensor(0x4029, 0x02);
	write_cmos_sensor(0x402a, 0x04);
	write_cmos_sensor(0x402b, 0x04);
	write_cmos_sensor(0x402c, 0x02);
	write_cmos_sensor(0x402d, 0x02);
	write_cmos_sensor(0x402e, 0x08);
	write_cmos_sensor(0x402f, 0x02);
	write_cmos_sensor(0x401f, 0x00);
	write_cmos_sensor(0x4034, 0x3f);
	write_cmos_sensor(0x403d, 0x04);
	write_cmos_sensor(0x4300, 0xff);
	write_cmos_sensor(0x4301, 0x00);
	write_cmos_sensor(0x4302, 0x0f);
	write_cmos_sensor(0x4316, 0x00);
	write_cmos_sensor(0x4500, 0x58);
	write_cmos_sensor(0x4503, 0x18);
	write_cmos_sensor(0x4600, 0x00);
	write_cmos_sensor(0x4601, 0xcb);
	write_cmos_sensor(0x481f, 0x32);
	write_cmos_sensor(0x4837, 0x16);
	write_cmos_sensor(0x4850, 0x10);
	write_cmos_sensor(0x4851, 0x32);
	write_cmos_sensor(0x4b00, 0x2a);
	write_cmos_sensor(0x4b0d, 0x00);
	write_cmos_sensor(0x4d00, 0x04);
	write_cmos_sensor(0x4d01, 0x18);
	write_cmos_sensor(0x4d02, 0xc3);
	write_cmos_sensor(0x4d03, 0xff);
	write_cmos_sensor(0x4d04, 0xff);
	write_cmos_sensor(0x4d05, 0xff);
	write_cmos_sensor(0x5000, 0x7e);
	write_cmos_sensor(0x5001, 0x01);
	write_cmos_sensor(0x5002, 0x08);
	write_cmos_sensor(0x5003, 0x20);
	write_cmos_sensor(0x5046, 0x12);
	write_cmos_sensor(0x5780, 0xfc);
	write_cmos_sensor(0x5784, 0x0c);
	write_cmos_sensor(0x5787, 0x40);
	write_cmos_sensor(0x5788, 0x08);
	write_cmos_sensor(0x578a, 0x02);
	write_cmos_sensor(0x578b, 0x01);
	write_cmos_sensor(0x578c, 0x01);
	write_cmos_sensor(0x578e, 0x02);
	write_cmos_sensor(0x578f, 0x01);
	write_cmos_sensor(0x5790, 0x01);
	write_cmos_sensor(0x5901, 0x00);
	write_cmos_sensor(0x5b00, 0x02);
	write_cmos_sensor(0x5b01, 0x10);
	write_cmos_sensor(0x5b02, 0x03);
	write_cmos_sensor(0x5b03, 0xcf);
	write_cmos_sensor(0x5b05, 0x6c);
	write_cmos_sensor(0x5e00, 0x00);
	write_cmos_sensor(0x5e01, 0x41);
	write_cmos_sensor(0x382d, 0x7f);
	write_cmos_sensor(0x4825, 0x3a);
	write_cmos_sensor(0x4826, 0x40);
	write_cmos_sensor(0x4808, 0x25);
	write_cmos_sensor(0x0100, 0x01);

}
static void sensor_init(void)
{
	LOG_INF("E\n");
	//OV8858 R2A setting
	//3.1 Initialization (Global Setting)
	//XVCLK=24Mhz, SCLK=72Mhz, MIPI 720Mbps, DACCLK=180Mhz
	write_cmos_sensor(0x103 , 0x01);
	write_cmos_sensor(0x100 , 0x00);
	write_cmos_sensor(0x302 , 0x1e);
	write_cmos_sensor(0x303 , 0x00);
	write_cmos_sensor(0x304 , 0x03);
	write_cmos_sensor(0x30e , 0x02);
	write_cmos_sensor(0x30f , 0x04);
	write_cmos_sensor(0x312 , 0x03);
	write_cmos_sensor(0x31e , 0x0c);
	write_cmos_sensor(0x3600, 0x00);
	write_cmos_sensor(0x3601, 0x00);
	write_cmos_sensor(0x3602, 0x00);
	write_cmos_sensor(0x3603, 0x00);
	write_cmos_sensor(0x3604, 0x22);
	write_cmos_sensor(0x3605, 0x20);
	write_cmos_sensor(0x3606, 0x00);
	write_cmos_sensor(0x3607, 0x20);
	write_cmos_sensor(0x3608, 0x11);
	write_cmos_sensor(0x3609, 0x28);
	write_cmos_sensor(0x360a, 0x00);
	write_cmos_sensor(0x360b, 0x05);
	write_cmos_sensor(0x360c, 0xd4);
	write_cmos_sensor(0x360d, 0x40);
	write_cmos_sensor(0x360e, 0x0c);
	write_cmos_sensor(0x360f, 0x20);
	write_cmos_sensor(0x3610, 0x07);
	write_cmos_sensor(0x3611, 0x20);
	write_cmos_sensor(0x3612, 0x88);
	write_cmos_sensor(0x3613, 0x80);
	write_cmos_sensor(0x3614, 0x58);
	write_cmos_sensor(0x3615, 0x00);
	write_cmos_sensor(0x3616, 0x4a);
	write_cmos_sensor(0x3617, 0x90);
	write_cmos_sensor(0x3618, 0x5a);
	write_cmos_sensor(0x3619, 0x70);
	write_cmos_sensor(0x361a, 0x99);
	write_cmos_sensor(0x361b, 0x0a);
	write_cmos_sensor(0x361c, 0x07);
	write_cmos_sensor(0x361d, 0x00);
	write_cmos_sensor(0x361e, 0x00);
	write_cmos_sensor(0x361f, 0x00);
	write_cmos_sensor(0x3638, 0xff);
	write_cmos_sensor(0x3633, 0x0f);
	write_cmos_sensor(0x3634, 0x0f);
	write_cmos_sensor(0x3635, 0x0f);
	write_cmos_sensor(0x3636, 0x12);
	write_cmos_sensor(0x3645, 0x13);
	write_cmos_sensor(0x3646, 0x83);
	write_cmos_sensor(0x364a, 0x07);
	write_cmos_sensor(0x3015, 0x00);
	write_cmos_sensor(0x3018, 0x72);
	write_cmos_sensor(0x3020, 0x93);
	write_cmos_sensor(0x3022, 0x01);
	write_cmos_sensor(0x3031, 0x0a);
	write_cmos_sensor(0x3034, 0x00);
	write_cmos_sensor(0x3106, 0x01);
	write_cmos_sensor(0x3305, 0xf1);
	write_cmos_sensor(0x3308, 0x00);
	write_cmos_sensor(0x3309, 0x28);
	write_cmos_sensor(0x330a, 0x00);
	write_cmos_sensor(0x330b, 0x20);
	write_cmos_sensor(0x330c, 0x00);
	write_cmos_sensor(0x330d, 0x00);
	write_cmos_sensor(0x330e, 0x00);
	write_cmos_sensor(0x330f, 0x40);
	write_cmos_sensor(0x3307, 0x04);
	write_cmos_sensor(0x3500, 0x00);
	write_cmos_sensor(0x3501, 0x4d);
	write_cmos_sensor(0x3502, 0x40);
	write_cmos_sensor(0x3503, 0x80);
	write_cmos_sensor(0x3505, 0x80);
	write_cmos_sensor(0x3508, 0x02);
	write_cmos_sensor(0x3509, 0x00);
	write_cmos_sensor(0x350c, 0x00);
	write_cmos_sensor(0x350d, 0x80);
	write_cmos_sensor(0x3510, 0x00);
	write_cmos_sensor(0x3511, 0x02);
	write_cmos_sensor(0x3512, 0x00);
	write_cmos_sensor(0x3700, 0x18);
	write_cmos_sensor(0x3701, 0x0c);
	write_cmos_sensor(0x3702, 0x28);
	write_cmos_sensor(0x3703, 0x19);
	write_cmos_sensor(0x3704, 0x14);
	write_cmos_sensor(0x3705, 0x00);
	write_cmos_sensor(0x3706, 0x82);
	write_cmos_sensor(0x3707, 0x04);
	write_cmos_sensor(0x3708, 0x24);
	write_cmos_sensor(0x3709, 0x33);
	write_cmos_sensor(0x370a, 0x01);
	write_cmos_sensor(0x370b, 0x82);
	write_cmos_sensor(0x370c, 0x04);
	write_cmos_sensor(0x3718, 0x12);
	write_cmos_sensor(0x3719, 0x31);
	write_cmos_sensor(0x3712, 0x42);
	write_cmos_sensor(0x3714, 0x24);
	write_cmos_sensor(0x371e, 0x19);
	write_cmos_sensor(0x371f, 0x40);
	write_cmos_sensor(0x3720, 0x05);
	write_cmos_sensor(0x3721, 0x05);
	write_cmos_sensor(0x3724, 0x06);
	write_cmos_sensor(0x3725, 0x01);
	write_cmos_sensor(0x3726, 0x06);
	write_cmos_sensor(0x3728, 0x05);
	write_cmos_sensor(0x3729, 0x02);
	write_cmos_sensor(0x372a, 0x03);
	write_cmos_sensor(0x372b, 0x53);
	write_cmos_sensor(0x372c, 0xa3);
	write_cmos_sensor(0x372d, 0x53);
	write_cmos_sensor(0x372e, 0x06);
	write_cmos_sensor(0x372f, 0x10);
	write_cmos_sensor(0x3730, 0x01);
	write_cmos_sensor(0x3731, 0x06);
	write_cmos_sensor(0x3732, 0x14);
	write_cmos_sensor(0x3733, 0x10);
	write_cmos_sensor(0x3734, 0x40);
	write_cmos_sensor(0x3736, 0x20);
	write_cmos_sensor(0x373a, 0x05);
	write_cmos_sensor(0x373b, 0x06);
	write_cmos_sensor(0x373c, 0x0a);
	write_cmos_sensor(0x373e, 0x03);
	write_cmos_sensor(0x3750, 0x0a);
	write_cmos_sensor(0x3751, 0x0e);
	write_cmos_sensor(0x3755, 0x10);
	write_cmos_sensor(0x3758, 0x00);
	write_cmos_sensor(0x3759, 0x4c);
	write_cmos_sensor(0x375a, 0x06);
	write_cmos_sensor(0x375b, 0x13);
	write_cmos_sensor(0x375c, 0x20);
	write_cmos_sensor(0x375d, 0x02);
	write_cmos_sensor(0x375e, 0x00);
	write_cmos_sensor(0x375f, 0x14);
	write_cmos_sensor(0x3768, 0x22);
	write_cmos_sensor(0x3769, 0x44);
	write_cmos_sensor(0x376a, 0x44);
	write_cmos_sensor(0x3761, 0x00);
	write_cmos_sensor(0x3762, 0x00);
	write_cmos_sensor(0x3763, 0x00);
	write_cmos_sensor(0x3766, 0xff);
	write_cmos_sensor(0x376b, 0x00);
	write_cmos_sensor(0x3772, 0x23);
	write_cmos_sensor(0x3773, 0x02);
	write_cmos_sensor(0x3774, 0x16);
	write_cmos_sensor(0x3775, 0x12);
	write_cmos_sensor(0x3776, 0x04);
	write_cmos_sensor(0x3777, 0x00);
	write_cmos_sensor(0x3778, 0x17);
	write_cmos_sensor(0x37a0, 0x44);
	write_cmos_sensor(0x37a1, 0x3d);
	write_cmos_sensor(0x37a2, 0x3d);
	write_cmos_sensor(0x37a3, 0x00);
	write_cmos_sensor(0x37a4, 0x00);
	write_cmos_sensor(0x37a5, 0x00);
	write_cmos_sensor(0x37a6, 0x00);
	write_cmos_sensor(0x37a7, 0x44);
	write_cmos_sensor(0x37a8, 0x4c);
	write_cmos_sensor(0x37a9, 0x4c);
	write_cmos_sensor(0x3760, 0x00);
	write_cmos_sensor(0x376f, 0x01);
	write_cmos_sensor(0x37aa, 0x44);
	write_cmos_sensor(0x37ab, 0x2e);
	write_cmos_sensor(0x37ac, 0x2e);
	write_cmos_sensor(0x37ad, 0x33);
	write_cmos_sensor(0x37ae, 0x0d);
	write_cmos_sensor(0x37af, 0x0d);
	write_cmos_sensor(0x37b0, 0x00);
	write_cmos_sensor(0x37b1, 0x00);
	write_cmos_sensor(0x37b2, 0x00);
	write_cmos_sensor(0x37b3, 0x42);
	write_cmos_sensor(0x37b4, 0x42);
	write_cmos_sensor(0x37b5, 0x31);
	write_cmos_sensor(0x37b6, 0x00);
	write_cmos_sensor(0x37b7, 0x00);
	write_cmos_sensor(0x37b8, 0x00);
	write_cmos_sensor(0x37b9, 0xff);
	write_cmos_sensor(0x3800, 0x00);
	write_cmos_sensor(0x3801, 0x0c);
	write_cmos_sensor(0x3802, 0x00);
	write_cmos_sensor(0x3803, 0x0c);
	write_cmos_sensor(0x3804, 0x0c);
	write_cmos_sensor(0x3805, 0xd3);
	write_cmos_sensor(0x3806, 0x09);
	write_cmos_sensor(0x3807, 0xa3);
	write_cmos_sensor(0x3808, 0x06);
	write_cmos_sensor(0x3809, 0x60);
	write_cmos_sensor(0x380a, 0x04);
	write_cmos_sensor(0x380b, 0xc8);
	write_cmos_sensor(0x380c, 0x07);
	write_cmos_sensor(0x380d, 0x88);
	write_cmos_sensor(0x380e, 0x04);
	write_cmos_sensor(0x380f, 0xdc);
	write_cmos_sensor(0x3810, 0x00);
	write_cmos_sensor(0x3811, 0x04);
	write_cmos_sensor(0x3813, 0x02);
	write_cmos_sensor(0x3814, 0x03);
	write_cmos_sensor(0x3815, 0x01);
	write_cmos_sensor(0x3820, 0x00);
	write_cmos_sensor(0x3821, 0x67);
	write_cmos_sensor(0x382a, 0x03);
	write_cmos_sensor(0x382b, 0x01);
	write_cmos_sensor(0x3830, 0x08);
	write_cmos_sensor(0x3836, 0x02);
	write_cmos_sensor(0x3837, 0x18);
	write_cmos_sensor(0x3841, 0xff);
	write_cmos_sensor(0x3846, 0x48);
	write_cmos_sensor(0x3d85, 0x16);
	write_cmos_sensor(0x3d8c, 0x73);
	write_cmos_sensor(0x3d8d, 0xde);
	write_cmos_sensor(0x3f08, 0x08);
	write_cmos_sensor(0x3f0a, 0x00);
	write_cmos_sensor(0x4000, 0xf1);
	write_cmos_sensor(0x4001, 0x10);
	write_cmos_sensor(0x4005, 0x10);
	write_cmos_sensor(0x4002, 0x27);
	write_cmos_sensor(0x4009, 0x81);
	write_cmos_sensor(0x400b, 0x0c);
	write_cmos_sensor(0x401b, 0x00);
	write_cmos_sensor(0x401d, 0x00);
	write_cmos_sensor(0x4020, 0x00);
	write_cmos_sensor(0x4021, 0x04);
	write_cmos_sensor(0x4022, 0x06);
	write_cmos_sensor(0x4023, 0x00);
	write_cmos_sensor(0x4024, 0x0f);
	write_cmos_sensor(0x4025, 0x2a);
	write_cmos_sensor(0x4026, 0x0f);
	write_cmos_sensor(0x4027, 0x2b);
	write_cmos_sensor(0x4028, 0x00);
	write_cmos_sensor(0x4029, 0x02);
	write_cmos_sensor(0x402a, 0x04);
	write_cmos_sensor(0x402b, 0x04);
	write_cmos_sensor(0x402c, 0x00);
	write_cmos_sensor(0x402d, 0x02);
	write_cmos_sensor(0x402e, 0x04);
	write_cmos_sensor(0x402f, 0x04);
	write_cmos_sensor(0x401f, 0x00);
	write_cmos_sensor(0x4034, 0x3f);
	write_cmos_sensor(0x403d, 0x04);
	write_cmos_sensor(0x4300, 0xff);
	write_cmos_sensor(0x4301, 0x00);
	write_cmos_sensor(0x4302, 0x0f);
	write_cmos_sensor(0x4316, 0x00);
	write_cmos_sensor(0x4500, 0x58);
	write_cmos_sensor(0x4503, 0x18);
	write_cmos_sensor(0x4600, 0x00);
	write_cmos_sensor(0x4601, 0xcb);
	write_cmos_sensor(0x481f, 0x32);
	write_cmos_sensor(0x4837, 0x16);
	write_cmos_sensor(0x4850, 0x10);
	write_cmos_sensor(0x4851, 0x32);
	write_cmos_sensor(0x4b00, 0x2a);
	write_cmos_sensor(0x4b0d, 0x00);
	write_cmos_sensor(0x4d00, 0x04);
	write_cmos_sensor(0x4d01, 0x18);
	write_cmos_sensor(0x4d02, 0xc3);
	write_cmos_sensor(0x4d03, 0xff);
	write_cmos_sensor(0x4d04, 0xff);
	write_cmos_sensor(0x4d05, 0xff);
	write_cmos_sensor(0x5000, 0x7e);
	write_cmos_sensor(0x5001, 0x01);
	write_cmos_sensor(0x5002, 0x08);
	write_cmos_sensor(0x5003, 0x20);
	write_cmos_sensor(0x5046, 0x12);
	write_cmos_sensor(0x5780, 0x3e);
	write_cmos_sensor(0x5781, 0x0f);
	write_cmos_sensor(0x5782, 0x44);
	write_cmos_sensor(0x5783, 0x02);
	write_cmos_sensor(0x5784, 0x01);
	write_cmos_sensor(0x5785, 0x00);
	write_cmos_sensor(0x5786, 0x00);
	write_cmos_sensor(0x5787, 0x04);
	write_cmos_sensor(0x5788, 0x02);
	write_cmos_sensor(0x5789, 0x0f);
	write_cmos_sensor(0x578a, 0xfd);
	write_cmos_sensor(0x578b, 0xf5);
	write_cmos_sensor(0x578c, 0xf5);
	write_cmos_sensor(0x578d, 0x03);
	write_cmos_sensor(0x578e, 0x08);
	write_cmos_sensor(0x578f, 0x0c);
	write_cmos_sensor(0x5790, 0x08);
	write_cmos_sensor(0x5791, 0x04);
	write_cmos_sensor(0x5792, 0x00);
	write_cmos_sensor(0x5793, 0x52);
	write_cmos_sensor(0x5794, 0xa3);
	write_cmos_sensor(0x5871, 0x0d);
	write_cmos_sensor(0x5870, 0x18);
	write_cmos_sensor(0x586e, 0x10);
	write_cmos_sensor(0x586f, 0x08);
	write_cmos_sensor(0x5901, 0x00);
	write_cmos_sensor(0x5b00, 0x02);
	write_cmos_sensor(0x5b01, 0x10);
	write_cmos_sensor(0x5b02, 0x03);
	write_cmos_sensor(0x5b03, 0xcf);
	write_cmos_sensor(0x5b05, 0x6c);
	write_cmos_sensor(0x5e00, 0x00);
	write_cmos_sensor(0x5e01, 0x41);
	write_cmos_sensor(0x382d, 0x7f);
	write_cmos_sensor(0x4825, 0x3a);
	write_cmos_sensor(0x4826, 0x40);
	write_cmos_sensor(0x4808, 0x25);
	write_cmos_sensor(0x3763, 0x18);
	write_cmos_sensor(0x3768, 0xcc);
	write_cmos_sensor(0x470b, 0x28);
	write_cmos_sensor(0x4202, 0x00);
	write_cmos_sensor(0x400d, 0x10);
	write_cmos_sensor(0x4040, 0x04);
	write_cmos_sensor(0x403e, 0x04);
	write_cmos_sensor(0x4041, 0xc6);
	write_cmos_sensor(0x3007, 0x80);
	write_cmos_sensor(0x400a, 0x01);
	write_cmos_sensor(0x4009, 0x83);
	write_cmos_sensor(0x4020, 0x00);
	write_cmos_sensor(0x4021, 0x04);
	write_cmos_sensor(0x4022, 0x04);
	write_cmos_sensor(0x4023, 0xb9);
	write_cmos_sensor(0x4024, 0x05);
	write_cmos_sensor(0x4025, 0x2a);
	write_cmos_sensor(0x4026, 0x05);
	write_cmos_sensor(0x4027, 0x2b);
	write_cmos_sensor(0x4028, 0x00);
	write_cmos_sensor(0x4029, 0x02);
	write_cmos_sensor(0x402a, 0x04);
	write_cmos_sensor(0x402b, 0x04);
	write_cmos_sensor(0x402c, 0x02);
	write_cmos_sensor(0x402d, 0x02);
	write_cmos_sensor(0x402e, 0x08);
	write_cmos_sensor(0x402f, 0x02);
	write_cmos_sensor(0x100 , 0x01);

}	/*	sensor_init  */


static void preview_setting(void)
{
	LOG_INF("E\n");

//3.2 Raw 10bit 1632x1224 30fps 4lane 720M bps/lane
//;XVCLK=24Mhz, SCLK=72Mhz, MIPI 720Mbps, DACCLK=180Mhz
	write_cmos_sensor(0x100 , 0x00);
	write_cmos_sensor(0x30e , 0x02);
	write_cmos_sensor(0x312 , 0x03);
	write_cmos_sensor(0x3015, 0x00);
	write_cmos_sensor(0x3700, 0x18);
	write_cmos_sensor(0x3701, 0x0c);
	write_cmos_sensor(0x3702, 0x28);
	write_cmos_sensor(0x3703, 0x19);
	write_cmos_sensor(0x3704, 0x14);
	write_cmos_sensor(0x3707, 0x04);
	write_cmos_sensor(0x3708, 0x24);
	write_cmos_sensor(0x3709, 0x33);
	write_cmos_sensor(0x370c, 0x04);
	write_cmos_sensor(0x3718, 0x12);
	write_cmos_sensor(0x3712, 0x42);
	write_cmos_sensor(0x371e, 0x19);
	write_cmos_sensor(0x371f, 0x40);
	write_cmos_sensor(0x3720, 0x05);
	write_cmos_sensor(0x3721, 0x05);
	write_cmos_sensor(0x3724, 0x06);
	write_cmos_sensor(0x3725, 0x01);
	write_cmos_sensor(0x3726, 0x06);
	write_cmos_sensor(0x3728, 0x05);
	write_cmos_sensor(0x3729, 0x02);
	write_cmos_sensor(0x372a, 0x03);
	write_cmos_sensor(0x372b, 0x53);
	write_cmos_sensor(0x372c, 0xa3);
	write_cmos_sensor(0x372d, 0x53);
	write_cmos_sensor(0x372e, 0x06);
	write_cmos_sensor(0x372f, 0x10);
	write_cmos_sensor(0x3730, 0x01);
	write_cmos_sensor(0x3731, 0x06);
	write_cmos_sensor(0x3732, 0x14);
	write_cmos_sensor(0x3736, 0x20);
	write_cmos_sensor(0x373a, 0x05);
	write_cmos_sensor(0x373b, 0x06);
	write_cmos_sensor(0x373c, 0x0a);
	write_cmos_sensor(0x373e, 0x03);
	write_cmos_sensor(0x375a, 0x06);
	write_cmos_sensor(0x375b, 0x13);
	write_cmos_sensor(0x375d, 0x02);
	write_cmos_sensor(0x375f, 0x14);
	write_cmos_sensor(0x3772, 0x23);
	write_cmos_sensor(0x3773, 0x02);
	write_cmos_sensor(0x3774, 0x16);
	write_cmos_sensor(0x3775, 0x12);
	write_cmos_sensor(0x3776, 0x04);
	write_cmos_sensor(0x3778, 0x17);
	write_cmos_sensor(0x37a0, 0x44);
	write_cmos_sensor(0x37a1, 0x3d);
	write_cmos_sensor(0x37a2, 0x3d);
	write_cmos_sensor(0x37a7, 0x44);
	write_cmos_sensor(0x37a8, 0x4c);
	write_cmos_sensor(0x37a9, 0x4c);
	write_cmos_sensor(0x37aa, 0x44);
	write_cmos_sensor(0x37ab, 0x2e);
	write_cmos_sensor(0x37ac, 0x2e);
	write_cmos_sensor(0x37ad, 0x33);
	write_cmos_sensor(0x37ae, 0x0d);
	write_cmos_sensor(0x37af, 0x0d);
	write_cmos_sensor(0x37b3, 0x42);
	write_cmos_sensor(0x37b4, 0x42);
	write_cmos_sensor(0x37b5, 0x31);
	write_cmos_sensor(0x3808, 0x06);
	write_cmos_sensor(0x3809, 0x60);
	write_cmos_sensor(0x380a, 0x04);
	write_cmos_sensor(0x380b, 0xc8);
	write_cmos_sensor(0x380d, 0x88);  //HTS
	write_cmos_sensor(0x380e, 0x04);  //VTS
	write_cmos_sensor(0x380f, 0xdc);  //1928 1244
	write_cmos_sensor(0x3814, 0x03);
	write_cmos_sensor(0x3821, 0x67);
	write_cmos_sensor(0x382a, 0x03);
	write_cmos_sensor(0x3830, 0x08);
	write_cmos_sensor(0x3836, 0x02);
	write_cmos_sensor(0x3f08, 0x08);
	write_cmos_sensor(0x4001, 0x10);
	write_cmos_sensor(0x4022, 0x04);
	write_cmos_sensor(0x4023, 0xb9);
	write_cmos_sensor(0x4024, 0x05);
	write_cmos_sensor(0x4025, 0x2a);
	write_cmos_sensor(0x4026, 0x05);
	write_cmos_sensor(0x4027, 0x2b);
	write_cmos_sensor(0x402b, 0x04);
	write_cmos_sensor(0x402e, 0x08);
	write_cmos_sensor(0x4600, 0x00);
	write_cmos_sensor(0x4601, 0xcb);
	write_cmos_sensor(0x382d, 0x7f);
	write_cmos_sensor(0x100 , 0x01);


}	/*	preview_setting  */

static void capture_setting(kal_uint16 currefps)
{
	LOG_INF("E! currefps:%d\n",currefps);
	if (currefps == 240) {}
	else 
	{   
		
	//3.3 Raw 10bit 3264x2448 30fps 4lane 720M bps/lane
	//;XVCLK=24Mhz, SCLK=144Mhz, MIPI 720Mbps, DACCLK=180Mhz
	write_cmos_sensor(0x100 , 0x00);
	write_cmos_sensor(0x30e , 0x00);
	write_cmos_sensor(0x312 , 0x01);
	write_cmos_sensor(0x3015, 0x01);
	write_cmos_sensor(0x3700, 0x30);
	write_cmos_sensor(0x3701, 0x18);
	write_cmos_sensor(0x3702, 0x50);
	write_cmos_sensor(0x3703, 0x32);
	write_cmos_sensor(0x3704, 0x28);
	write_cmos_sensor(0x3707, 0x08);
	write_cmos_sensor(0x3708, 0x48);
	write_cmos_sensor(0x3709, 0x66);
	write_cmos_sensor(0x370c, 0x07);
	write_cmos_sensor(0x3718, 0x14);
	write_cmos_sensor(0x3712, 0x44);
	write_cmos_sensor(0x371e, 0x31);
	write_cmos_sensor(0x371f, 0x7f);
	write_cmos_sensor(0x3720, 0x0a);
	write_cmos_sensor(0x3721, 0x0a);
	write_cmos_sensor(0x3724, 0x0c);
	write_cmos_sensor(0x3725, 0x02);
	write_cmos_sensor(0x3726, 0x0c);
	write_cmos_sensor(0x3728, 0x0a);
	write_cmos_sensor(0x3729, 0x03);
	write_cmos_sensor(0x372a, 0x06);
	write_cmos_sensor(0x372b, 0xa6);
	write_cmos_sensor(0x372c, 0xa6);
	write_cmos_sensor(0x372d, 0xa6);
	write_cmos_sensor(0x372e, 0x0c);
	write_cmos_sensor(0x372f, 0x20);
	write_cmos_sensor(0x3730, 0x02);
	write_cmos_sensor(0x3731, 0x0c);
	write_cmos_sensor(0x3732, 0x28);
	write_cmos_sensor(0x3736, 0x30);
	write_cmos_sensor(0x373a, 0x0a);
	write_cmos_sensor(0x373b, 0x0b);
	write_cmos_sensor(0x373c, 0x14);
	write_cmos_sensor(0x373e, 0x06);
	write_cmos_sensor(0x375a, 0x0c);
	write_cmos_sensor(0x375b, 0x26);
	write_cmos_sensor(0x375d, 0x04);
	write_cmos_sensor(0x375f, 0x28);
	write_cmos_sensor(0x3772, 0x46);
	write_cmos_sensor(0x3773, 0x04);
	write_cmos_sensor(0x3774, 0x2c);
	write_cmos_sensor(0x3775, 0x13);
	write_cmos_sensor(0x3776, 0x08);
	write_cmos_sensor(0x3778, 0x17);
	write_cmos_sensor(0x37a0, 0x88);
	write_cmos_sensor(0x37a1, 0x7a);
	write_cmos_sensor(0x37a2, 0x7a);
	write_cmos_sensor(0x37a7, 0x88);
	write_cmos_sensor(0x37a8, 0x98);
	write_cmos_sensor(0x37a9, 0x98);
	write_cmos_sensor(0x37aa, 0x88);
	write_cmos_sensor(0x37ab, 0x5c);
	write_cmos_sensor(0x37ac, 0x5c);
	write_cmos_sensor(0x37ad, 0x55);
	write_cmos_sensor(0x37ae, 0x19);
	write_cmos_sensor(0x37af, 0x19);
	write_cmos_sensor(0x37b3, 0x84);
	write_cmos_sensor(0x37b4, 0x84);
	write_cmos_sensor(0x37b5, 0x60);
	write_cmos_sensor(0x3808, 0x0c);
	write_cmos_sensor(0x3809, 0xc0);
	write_cmos_sensor(0x380a, 0x09);
	write_cmos_sensor(0x380b, 0x90);
	write_cmos_sensor(0x380d, 0x94);  	//HTS
	write_cmos_sensor(0x380e, 0x09);    //VTS
	write_cmos_sensor(0x380f, 0xaa);    //1940 2474
	write_cmos_sensor(0x3814, 0x01);
	write_cmos_sensor(0x3821, 0x46);
	write_cmos_sensor(0x382a, 0x01);
	write_cmos_sensor(0x3830, 0x06);
	write_cmos_sensor(0x3836, 0x01);
	write_cmos_sensor(0x3f08, 0x10);
	write_cmos_sensor(0x4001, 0x00);
	write_cmos_sensor(0x4022, 0x0b);
	write_cmos_sensor(0x4023, 0xc3);
	write_cmos_sensor(0x4024, 0x0c);
	write_cmos_sensor(0x4025, 0x36);
	write_cmos_sensor(0x4026, 0x0c);
	write_cmos_sensor(0x4027, 0x37);
	write_cmos_sensor(0x402b, 0x08);
	write_cmos_sensor(0x402e, 0x0c);
	write_cmos_sensor(0x4600, 0x01);
	write_cmos_sensor(0x4601, 0x97);
	write_cmos_sensor(0x382d, 0xff);
	write_cmos_sensor(0x100 , 0x01);
	}
		
}

static void normal_video_setting(kal_uint16 currefps)
{
	LOG_INF("E! currefps:%d\n",currefps);

}
static void hs_video_setting()
{
	LOG_INF("E\n");
	capture_setting(300);
}

static void slim_video_setting()
{
	LOG_INF("E\n");
	preview_setting();
}


/*************************************************************************
* FUNCTION
*	get_imgsensor_id
*
* DESCRIPTION
*	This function get the sensor ID 
*
* PARAMETERS
*	*sensorID : return the sensor ID 
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id) 
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	//sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = ((read_cmos_sensor(0x300B) << 8) | read_cmos_sensor(0x300C));
			if (*sensor_id == imgsensor_info.sensor_id) {
				if((read_cmos_sensor(0x302A)) == 0XB2){
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,*sensor_id);	  
				return ERROR_NONE;
				}
				else if((read_cmos_sensor(0x302A)) == 0XB1){
				LOG_INF("=================R1A sensor,contact OV please!====================\n");	  
				return ERROR_SENSOR_CONNECT_FAIL;
				}
				else{
				LOG_INF("read ov8858 R1A R2A bate fail\n");	  
				return ERROR_SENSOR_CONNECT_FAIL;
				}
			}	
			LOG_INF("Read sensor id fail, id: 0x%x\n", imgsensor.i2c_write_id,*sensor_id);
			retry--;
		} while(retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		// if Sensor ID is not correct, Must set *sensor_id to 0xFFFFFFFF 
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*	open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
	//const kal_uint8 i2c_addr[] = {IMGSENSOR_WRITE_ID_1, IMGSENSOR_WRITE_ID_2};
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint16 sensor_id = 0; 
	LOG_INF("MIPI 4LANE\n");
	//LOG_INF("preview 1280*960@30fps,864Mbps/lane; video 1280*960@30fps,864Mbps/lane; capture 5M@30fps,864Mbps/lane\n");
	
	//sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = ((read_cmos_sensor(0x300B) << 8) | read_cmos_sensor(0x300C));
			if (sensor_id == imgsensor_info.sensor_id) {				
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);	  
				break;
			}	
			LOG_INF("Read sensor id fail, id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
			retry--;
		} while(retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}		 
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;
	
	/* initail sequence write in  */
	sensor_init();
	mdelay(10);
	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en= KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.shutter = 0x2D00;
	imgsensor.gain = 0x100;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}	/*	open  */



/*************************************************************************
* FUNCTION
*	close
*
* DESCRIPTION
*	
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	/*No Need to implement this function*/ 
	
	return ERROR_NONE;
}	/*	close  */


/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	//imgsensor.video_mode = KAL_FALSE;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength; 
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();
	mdelay(10);
	return ERROR_NONE;
}	/*	preview   */

/*************************************************************************
* FUNCTION
*	capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {//PIP capture: 24fps for less than 13M, 20fps for 16M,15fps for 20M
		imgsensor.pclk = imgsensor_info.cap1.pclk;
		imgsensor.line_length = imgsensor_info.cap1.linelength;
		imgsensor.frame_length = imgsensor_info.cap1.framelength;  
		imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	} else {
		if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
			LOG_INF("Warning: current_fps %d fps is not support, so use cap1's setting: %d fps!\n",imgsensor_info.cap1.max_framerate/10);
		imgsensor.pclk = imgsensor_info.cap.pclk;
		imgsensor.line_length = imgsensor_info.cap.linelength;
		imgsensor.frame_length = imgsensor_info.cap.framelength;  
		imgsensor.min_frame_length = imgsensor_info.cap.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	}
	spin_unlock(&imgsensor_drv_lock);

	capture_setting(imgsensor.current_fps); 
	mdelay(10);

	#if 0
	if(imgsensor.test_pattern == KAL_TRUE)
	{
		write_cmos_sensor(0x3282,0x01);//DPU OFF
		
		write_cmos_sensor(0x0600,0x00);
		write_cmos_sensor(0x0601,0x02);
		
	}
#endif
	return ERROR_NONE;
}	/* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;  
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	capture_setting(imgsensor.current_fps);
	mdelay(10);
	
	
	return ERROR_NONE;
}	/*	normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength; 
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	mdelay(10);
	
	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength; 
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	mdelay(10);
	
	return ERROR_NONE;
}	/*	slim_video	 */



static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;
	
	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;		

	
	sensor_resolution->SensorHighSpeedVideoWidth	 = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight	 = imgsensor_info.hs_video.grabwindow_height;
	
	sensor_resolution->SensorSlimVideoWidth	 = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight	 = imgsensor_info.slim_video.grabwindow_height;
	return ERROR_NONE;
}	/*	get_resolution	*/

static kal_uint32 get_info(MSDK_SCENARIO_ID_ENUM scenario_id,
					  MSDK_SENSOR_INFO_STRUCT *sensor_info,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	
	//sensor_info->SensorVideoFrameRate = imgsensor_info.normal_video.max_framerate/10; /* not use */
	//sensor_info->SensorStillCaptureFrameRate= imgsensor_info.cap.max_framerate/10; /* not use */
	//imgsensor_info->SensorWebCamCaptureFrameRate= imgsensor_info.v.max_framerate; /* not use */

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	//sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	//sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame; 
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame; 
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;
	
	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame; 		 /* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;	/* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;	
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	
	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num; 
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */
	
	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  // 0 is default 1x
	sensor_info->SensorHightSampling = 0;	// 0 is default 1x 
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;		
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			sensor_info->SensorGrabStartX = imgsensor_info.cap.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.cap.mipi_data_lp2hs_settle_dc; 

			break;	 
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			
			sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;
	   
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc; 

			break;	  
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:			
			sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc; 

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc; 

			break;
		default:			
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;		
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}
	
	return ERROR_NONE;
}	/*	get_info  */


static kal_uint32 control(MSDK_SCENARIO_ID_ENUM scenario_id, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			preview(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			capture(image_window, sensor_config_data);
			break;	
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			normal_video(image_window, sensor_config_data);
			break;	  
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			hs_video(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			slim_video(image_window, sensor_config_data);
			break;	  
		default:
			LOG_INF("Error ScenarioId setting");
			preview(image_window, sensor_config_data);
			return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}	/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);
	// SetVideoMode Function should fix framerate
	if (framerate == 0)
		// Dynamic frame rate
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps,1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d \n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) //enable auto flicker	  
		imgsensor.autoflicker_en = KAL_TRUE;
	else //Cancel Auto flick
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate) 
{
	kal_uint32 frame_length;
  
	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if(framerate == 0)
				return ERROR_NONE;
			frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength) : 0;			
			imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();			
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();			
			break;	
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();			
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength): 0;	
			imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();			
			break;		
		default:  //coding with  preview scenario by default
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();	
			LOG_INF("error scenario_id = %d, we use preview scenario \n", scenario_id);
			break;
	}	
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate) 
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*framerate = imgsensor_info.pre.max_framerate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*framerate = imgsensor_info.normal_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*framerate = imgsensor_info.cap.max_framerate;
			break;		
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*framerate = imgsensor_info.hs_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO: 
			*framerate = imgsensor_info.slim_video.max_framerate;
			break;
		default:
			break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
		// 0x5E00[8]: 1 enable,  0 disable
		// 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK
		write_cmos_sensor(0x5E00, 0x80);
	} else {
		// 0x5E00[8]: 1 enable,  0 disable
		// 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK
		write_cmos_sensor(0x5E00, 0x00);
	}	 
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
							 UINT8 *feature_para,UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16=(UINT16 *) feature_para;
	UINT16 *feature_data_16=(UINT16 *) feature_para;
	UINT32 *feature_return_para_32=(UINT32 *) feature_para;
	UINT32 *feature_data_32=(UINT32 *) feature_para;
	
	SENSOR_WINSIZE_INFO_STRUCT *wininfo;	
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data=(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;
 
	LOG_INF("feature_id = %d\n", feature_id);
	switch (feature_id) {
		case SENSOR_FEATURE_GET_PERIOD:
			*feature_return_para_16++ = imgsensor.line_length;
			*feature_return_para_16 = imgsensor.frame_length;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:	 
			*feature_return_para_32 = imgsensor.pclk;
			*feature_para_len=4;
			break;		   
		case SENSOR_FEATURE_SET_ESHUTTER:
			set_shutter(*feature_data_16);
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			night_mode((BOOL) *feature_data_16);
			break;
		case SENSOR_FEATURE_SET_GAIN:		
			set_gain((UINT16) *feature_data_16);
			break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
			break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
			break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*feature_return_para_32=LENS_DRIVER_ID_DO_NOT_CARE;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
			set_video_mode(*feature_data_16);
			break; 
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			get_imgsensor_id(feature_return_para_32); 
			break; 
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			set_auto_flicker_mode((BOOL)*feature_data_16,*(feature_data_16+1));
			break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			set_max_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*feature_data_32, *(feature_data_32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			get_default_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*feature_data_32, (MUINT32 *)(*(feature_data_32+1)));
			break;
		case SENSOR_FEATURE_SET_TEST_PATTERN:
			set_test_pattern_mode((BOOL)*feature_data_16);
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: //for factory mode auto testing			 
			*feature_return_para_32 = imgsensor_info.checksum_value;
			*feature_para_len=4;							 
			break;				
		case SENSOR_FEATURE_SET_FRAMERATE:
			LOG_INF("current fps :%d\n", *feature_data_16);
			spin_lock(&imgsensor_drv_lock);
			imgsensor.current_fps = *feature_data_16;
			spin_unlock(&imgsensor_drv_lock);		
			break;
		case SENSOR_FEATURE_SET_HDR:
			LOG_INF("ihdr enable :%d\n", *feature_data_16);
			spin_lock(&imgsensor_drv_lock);
			imgsensor.ihdr_en = *feature_data_16;
			spin_unlock(&imgsensor_drv_lock);		
			break;
		case SENSOR_FEATURE_GET_CROP_INFO:
			LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n", *feature_data_32);
			wininfo = (SENSOR_WINSIZE_INFO_STRUCT *)(*(feature_data_32+1));
		
			switch (*feature_data_32) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[1],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;	  
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[2],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[3],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[4],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				default:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[0],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
			}
		case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
			LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",*feature_data_32,*(feature_data_32+1),*(feature_data_32+2)); 
			ihdr_write_shutter_gain(*feature_data_32,*(feature_data_32+1),*(feature_data_32+2));	
			break;
		default:
			break;
	}
  
	return ERROR_NONE;
}	/*	feature_control()  */

static SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 OV8858_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&sensor_func;
	return ERROR_NONE;
}	/*	OV5693_MIPI_RAW_SensorInit	*/
