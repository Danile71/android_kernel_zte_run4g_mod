/*****************************************************************************
 *
 * Filename:
 * ---------
 *   imx111mipiraw_sensor.c
 *
 * Project:
 * --------
 *   RAW
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *

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
//#include <asm/system.h>
#include <linux/xlog.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "imx220mipiraw_Sensor.h"
#include "imx220mipiraw_Camera_Sensor_para.h"
#include "imx220mipiraw_CameraCustomized.h"

kal_bool  IMX220MIPI_MPEG4_encode_mode = KAL_FALSE;
kal_bool IMX220MIPI_Auto_Flicker_mode = KAL_FALSE;


kal_uint8 IMX220MIPI_sensor_write_I2C_address = IMX220MIPI_WRITE_ID;
kal_uint8 IMX220MIPI_sensor_read_I2C_address = IMX220MIPI_READ_ID;

#define IMX220_TEST_PATTERN_CHECKSUM (0x51905ccf)

static struct IMX220MIPI_sensor_STRUCT IMX220MIPI_sensor=
{
	IMX220MIPI_WRITE_ID,											//kal_uint16 i2c_write_id;                                                 
	IMX220MIPI_READ_ID,                       //kal_uint16 i2c_read_id;                                                  
	KAL_TRUE,                                 //kal_bool first_init;                                                     
	KAL_FALSE,                                //kal_bool fix_video_fps;                                                  
	KAL_TRUE,                                 //kal_bool pv_mode;                                                        
	KAL_FALSE,                                //kal_bool video_mode; 				                                             
	KAL_FALSE,                                //kal_bool capture_mode; 				//True: Preview Mode; False: Capture Mode  
    KAL_FALSE,                                //kal_bool slimvideo_mode; 
    KAL_FALSE,                                //kal_bool capture_mode;           //True: Preview Mode; False: Capture Mode  
    KAL_FALSE,                                //kal_bool night_mode;             //True: Night Mode; False: Auto Mode         
    KAL_FALSE,                                //kal_uint8 mirror_flip;                                                   
	292000000,								  //kal_uint32 pv_pclk; 			//Preview Pclk								   
    292000000,                                //kal_uint32 video_pclk;          //video Pclk                               
	292000000,								  //kal_uint32 video_pclk;				//video Pclk
    292000000,                                //kal_uint32 video1_pclk;         //video Pclk 
    292000000,                                //kal_uint32 video2_pclk;         //video Pclk 
	292000000,                                //kal_uint32 cp_pclk;				//Capture Pclk 
	0,                                        //kal_uint32 pv_shutter;		                                               
	0,                                        //kal_uint32 video_shutter;		                                             
	0,                                        //kal_uint32 cp_shutter;                                                   
    0,                                        //kal_uint32 video2_shutter; 
    0,                                        //kal_uint32 cp_shutter;                                                   
	64,                                       //kal_uint32 pv_gain;                                                      
	64,                                       //kal_uint32 video_gain;                                                   
	64,                                       //kal_uint32 cp_gain;                                                      
    64,                                       //kal_uint32 video2_gain; 
    64,                                       //kal_uint32 cp_gain;                                                      
	IMX220MIPI_PV_LINE_LENGTH_PIXELS,         //kal_uint32 pv_line_length;                                               
	IMX220MIPI_PV_FRAME_LENGTH_LINES,         //kal_uint32 pv_frame_length;                                              
    IMX220MIPI_VIDEO_LINE_LENGTH_PIXELS,      //kal_uint32 video_line_length;                                            
    IMX220MIPI_VIDEO_FRAME_LENGTH_LINES,      //kal_uint32 video_frame_length;      
    IMX220MIPI_VIDEO1_LINE_LENGTH_PIXELS,     //kal_uint32 video_line_length;                                            
    IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES,     //kal_uint32 video_frame_length;   
    IMX220MIPI_VIDEO2_LINE_LENGTH_PIXELS,     //kal_uint32 video_line_length;                                            
    IMX220MIPI_VIDEO2_FRAME_LENGTH_LINES,     //kal_uint32 video_frame_length;   
	IMX220MIPI_FULL_LINE_LENGTH_PIXELS,       //kal_uint32 cp_line_length;                                               
	IMX220MIPI_FULL_FRAME_LENGTH_LINES,       //kal_uint32 cp_frame_length;                                              
    0,                                        //kal_uint16 pv_dummy_pixels;        //Dummy Pixels:must be 12s              
    0,                                        //kal_uint16 pv_dummy_lines;         //Dummy Lines                           
    0,                                        //kal_uint16 video_dummy_pixels;     //Dummy Pixels:must be 12s          
    0,                                        //kal_uint16 video_dummy_lines;      //Dummy Lines 
	0,                                        //kal_uint16 pv_dummy_pixels;		   //Dummy Pixels:must be 12s              
	0,                                        //kal_uint16 pv_dummy_lines;		   //Dummy Lines                           
	0,                                        //kal_uint16 video_dummy_pixels;		   //Dummy Pixels:must be 12s          
	0,                                        //kal_uint16 video_dummy_lines;		   //Dummy Lines                         
	0,                                        //kal_uint16 cp_dummy_pixels;		   //Dummy Pixels:must be 12s              
	0,                                        //kal_uint16 cp_dummy_lines;		   //Dummy Lines			                     
	30                                        //kal_uint16 video_current_frame_rate;                                     
};


MSDK_SCENARIO_ID_ENUM IMX220CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;

/* MAX/MIN Explosure Lines Used By AE Algorithm */
kal_uint16 IMX220MIPI_MAX_EXPOSURE_LINES = IMX220MIPI_PV_FRAME_LENGTH_LINES-5;
kal_uint8  IMX220MIPI_MIN_EXPOSURE_LINES = 2;
kal_uint32 IMX220MIPI_isp_master_clock;
static DEFINE_SPINLOCK(imx220_drv_lock);

#define LOG_TAG "[IMX220MIPIRaw]"
#define SENSORDB(fmt, arg...)    xlog_printk(ANDROID_LOG_DEBUG , LOG_TAG, fmt, ##arg)
//#define SENSORDB(fmt, arg...) printk( "[IMX220MIPIRaw] "  fmt, ##arg)

#define RETAILMSG(x,...)
#define TEXT
UINT8 IMX220MIPIPixelClockDivider=0;
kal_uint16 IMX220MIPI_sensor_id=0;
MSDK_SENSOR_CONFIG_STRUCT IMX220MIPISensorConfigData;
kal_uint32 IMX220MIPI_FAC_SENSOR_REG;
kal_uint16 IMX220MIPI_sensor_flip_value; 
static kal_uint16 IMX220MIPI_IHDR_EN = 0;
static kal_uint16 IMX220MIPI_CurretFPS = 0;

#define IMX220MIPI_MaxGainIndex (55)
kal_uint16 IMX220MIPI_sensorGainMapping[IMX220MIPI_MaxGainIndex][2] ={
	{71  ,25 },
	{76  ,42 },
	{83  ,59 },
	{89  ,73 },
	{96  ,85 },
	{102 ,96 },
	{108 ,105},
	{115 ,114},
	{121 ,121},
	{128 ,128},
	{134 ,134},
	{140 ,140},
	{147 ,145},
	{153 ,149},
	{160 ,154},
	{166 ,158},
	{172 ,161},
	{179 ,164},
	{185 ,168},
	{192 ,171},
	{200 ,174},
	{208 ,177},
	{216 ,180},
	{224 ,183},
	{232 ,185},
	{240 ,188},
	{248 ,190},
	{256 ,192},
	{264 ,194},
	{272 ,196},
	{280 ,197},
	{288 ,199},
	{296 ,201},
	{304 ,202},
	{312 ,203},
	{320 ,205},
	{328 ,206},
	{336 ,207},
	{344 ,208},
	{352 ,209},
	{360 ,210},
	{368 ,211},
	{376 ,212},
	{384 ,213},
	{390 ,214},
	{399 ,215},
	{409 ,216},
	{419 ,217},
	{431 ,218},
	{442 ,219},
	{455 ,220},
	{467 ,221},
	{481 ,222},
	{496 ,223},
	{512 ,224}	
};



/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT IMX220MIPISensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT IMX220MIPISensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
SENSOR_WINSIZE_INFO_STRUCT SENSOR_WINSIZE_INFO_IMX220[5]=    
{{ 5904, 3984, 0000, 0000, 5248, 3936, 2624, 1968, 0000, 0000, 2624, 1968,    4,    4, 2616, 1960}, // Preview 
 { 5904, 3984, 0000, 0000, 5248, 3936, 5248, 3936, 0000, 0000, 5248, 3936,    8,    8, 5232, 3920}, // capture 
 { 5904, 3984, 1144, 1172, 2960, 1590, 2960, 1590, 0000, 0000, 2960, 1590,    4,    4, 2952, 1582}, // video 
 { 5904, 3984,   48,  512, 5152, 2912, 1288,  728, 0000, 0000, 1288,  728,    4,    4, 1280,  720}, //hight speed video 
 { 5904, 3984,   48,  512, 5152, 2912, 1288,  728, 0000, 0000, 1288,  728,    4,    4, 1280,  720}};// slim video 
/* FIXME: old factors and DIDNOT use now. e*/
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
#define IMX220MIPI_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, IMX220MIPI_WRITE_ID)

kal_uint16 IMX220MIPI_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,IMX220MIPI_WRITE_ID);
    return get_byte;
}

void IMX220MIPI_write_shutter(kal_uint16 shutter)
{
	kal_uint32 frame_length = 0,line_length=0;
    kal_uint32 extra_lines = 0;
	kal_uint32 max_exp_shutter = 0;
	unsigned long flags;
//  SENSORDB("IMX220MIPI_write_shutter shutter = %d\n",shutter);
   // return ;
    if (IMX220MIPI_sensor.pv_mode == KAL_TRUE) 
	 {
	   max_exp_shutter = IMX220MIPI_PV_FRAME_LENGTH_LINES + IMX220MIPI_sensor.pv_dummy_lines-4;
     }
     else if (IMX220MIPI_sensor.video_mode== KAL_TRUE) 
     {
       max_exp_shutter = IMX220MIPI_VIDEO_FRAME_LENGTH_LINES + IMX220MIPI_sensor.video_dummy_lines-4;
	 }	 
     else if (IMX220MIPI_sensor.highspeedvideo_mode== KAL_TRUE) 
     {
       max_exp_shutter = IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES + IMX220MIPI_sensor.video1_dummy_lines-4;
     }  
     else if (IMX220MIPI_sensor.slimvideo_mode== KAL_TRUE) 
     {
       max_exp_shutter = IMX220MIPI_VIDEO2_FRAME_LENGTH_LINES + IMX220MIPI_sensor.video2_dummy_lines-4;
     }  
	 else 
     {
       max_exp_shutter = IMX220MIPI_FULL_FRAME_LENGTH_LINES + IMX220MIPI_sensor.cp_dummy_lines-4;
	 }	 
	 
	 if(shutter > max_exp_shutter)
	   extra_lines = shutter - max_exp_shutter;
	 else 
	   extra_lines = 0;
	 if (IMX220MIPI_sensor.pv_mode == KAL_TRUE) 
	 {
       frame_length =IMX220MIPI_PV_FRAME_LENGTH_LINES+ IMX220MIPI_sensor.pv_dummy_lines + extra_lines;
	   line_length = IMX220MIPI_PV_LINE_LENGTH_PIXELS+ IMX220MIPI_sensor.pv_dummy_pixels;
	   spin_lock_irqsave(&imx220_drv_lock,flags);
	   IMX220MIPI_sensor.pv_line_length = line_length;
	   IMX220MIPI_sensor.pv_frame_length = frame_length;
	   spin_unlock_irqrestore(&imx220_drv_lock,flags);
	 }
	 else if (IMX220MIPI_sensor.video_mode== KAL_TRUE) 
     {
	    frame_length = IMX220MIPI_VIDEO_FRAME_LENGTH_LINES+ IMX220MIPI_sensor.video_dummy_lines + extra_lines;
		line_length =IMX220MIPI_VIDEO_LINE_LENGTH_PIXELS + IMX220MIPI_sensor.video_dummy_pixels;
		spin_lock_irqsave(&imx220_drv_lock,flags);
		IMX220MIPI_sensor.video_line_length = line_length;
	    IMX220MIPI_sensor.video_frame_length = frame_length;
		spin_unlock_irqrestore(&imx220_drv_lock,flags);
	 } 
     else if (IMX220MIPI_sensor.highspeedvideo_mode== KAL_TRUE) 
     {
        frame_length = IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES+ IMX220MIPI_sensor.video1_dummy_lines + extra_lines;
        line_length =IMX220MIPI_VIDEO1_LINE_LENGTH_PIXELS + IMX220MIPI_sensor.video1_dummy_pixels;
        spin_lock_irqsave(&imx220_drv_lock,flags);
        IMX220MIPI_sensor.video1_line_length = line_length;
        IMX220MIPI_sensor.video1_frame_length = frame_length;
        spin_unlock_irqrestore(&imx220_drv_lock,flags);
     } 
     else if (IMX220MIPI_sensor.slimvideo_mode== KAL_TRUE) 
     {
        frame_length = IMX220MIPI_VIDEO2_FRAME_LENGTH_LINES+ IMX220MIPI_sensor.video2_dummy_lines + extra_lines;
        line_length =IMX220MIPI_VIDEO2_LINE_LENGTH_PIXELS + IMX220MIPI_sensor.video2_dummy_pixels;
        spin_lock_irqsave(&imx220_drv_lock,flags);
        IMX220MIPI_sensor.video2_line_length = line_length;
        IMX220MIPI_sensor.video2_frame_length = frame_length;
        spin_unlock_irqrestore(&imx220_drv_lock,flags);
     } 
	 else
	 	{
	    frame_length = IMX220MIPI_FULL_FRAME_LENGTH_LINES+ IMX220MIPI_sensor.cp_dummy_lines + extra_lines;
		line_length =IMX220MIPI_FULL_LINE_LENGTH_PIXELS + IMX220MIPI_sensor.cp_dummy_pixels;
		spin_lock_irqsave(&imx220_drv_lock,flags);
		IMX220MIPI_sensor.cp_line_length = line_length;
	    IMX220MIPI_sensor.cp_frame_length = frame_length;
		spin_unlock_irqrestore(&imx220_drv_lock,flags);
	 }
    IMX220MIPI_write_cmos_sensor(0x0104, 1);        
	IMX220MIPI_write_cmos_sensor(0x0340, (frame_length >>8) & 0xFF);
    IMX220MIPI_write_cmos_sensor(0x0341, frame_length & 0xFF);	

    IMX220MIPI_write_cmos_sensor(0x0202, (shutter >> 8) & 0xFF);
    IMX220MIPI_write_cmos_sensor(0x0203, shutter  & 0xFF);
    IMX220MIPI_write_cmos_sensor(0x0104, 0);    
	SENSORDB("IMX220MIPI_write_shutter frame_length = %d;shutter = %d\n",frame_length,shutter);

}   /* write_IMX220MIPI_shutter */

static kal_uint16 IMX220MIPIReg2Gain(const kal_uint8 iReg)
{

    kal_uint8 iI;
    // Range: 1x to 16x
    for (iI = 0; iI < IMX220MIPI_MaxGainIndex; iI++) {
        if(iReg <= IMX220MIPI_sensorGainMapping[iI][1]){
            break;
        }
    }
    return IMX220MIPI_sensorGainMapping[iI][0];

}
static kal_uint8 IMX220MIPIGain2Reg(const kal_uint16 iGain)
{

	kal_uint8 iI;
    
    for (iI = 0; iI < (IMX220MIPI_MaxGainIndex-1); iI++) {
        if(iGain <= IMX220MIPI_sensorGainMapping[iI][0]){    
            break;
        }
    }
    if(iGain != IMX220MIPI_sensorGainMapping[iI][0])
    {
         SENSORDB("[IMX220MIPIGain2Reg] Gain mapping don't correctly:%d %d \n", iGain, IMX220MIPI_sensorGainMapping[iI][0]);
    }
    return IMX220MIPI_sensorGainMapping[iI][1];
}


/*************************************************************************
* FUNCTION
*    IMX220MIPI_SetGain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    gain : sensor global gain(base: 0x40)
*
* RETURNS
*    the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
void IMX220MIPI_SetGain(UINT16 iGain)
{
    kal_uint8 iReg;
//  SENSORDB("[IMX220MIPI_SetGain] SetGain=%d\n",  iGain);
    iReg = IMX220MIPIGain2Reg(iGain);
//  SENSORDB("[IMX220MIPI_SetGain ] RegisterGain:%d\n", iReg);
	
	IMX220MIPI_write_cmos_sensor(0x0104, 1);
    IMX220MIPI_write_cmos_sensor(0x0205, (kal_uint8)iReg);
    IMX220MIPI_write_cmos_sensor(0x0104, 0);

}   /*  IMX220MIPI_SetGain_SetGain  */


/*************************************************************************
* FUNCTION
*    read_IMX220MIPI_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 read_IMX220MIPI_gain(void)
{
    return (kal_uint16)(IMX220MIPI_read_cmos_sensor(0x0205)) ;
}  /* read_IMX220MIPI_gain */

void write_IMX220MIPI_gain(kal_uint16 gain)
{
    IMX220MIPI_SetGain(gain);
}
void IMX220MIPI_camera_para_to_sensor(void)
{

	kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=IMX220MIPISensorReg[i].Addr; i++)
    {
        IMX220MIPI_write_cmos_sensor(IMX220MIPISensorReg[i].Addr, IMX220MIPISensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=IMX220MIPISensorReg[i].Addr; i++)
    {
        IMX220MIPI_write_cmos_sensor(IMX220MIPISensorReg[i].Addr, IMX220MIPISensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        IMX220MIPI_write_cmos_sensor(IMX220MIPISensorCCT[i].Addr, IMX220MIPISensorCCT[i].Para);
    }

}


/*************************************************************************
* FUNCTION
*    IMX220MIPI_sensor_to_camera_para
*
* DESCRIPTION
*    // update camera_para from sensor register
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
void IMX220MIPI_sensor_to_camera_para(void)
{

	kal_uint32    i,temp_data;
    for(i=0; 0xFFFFFFFF!=IMX220MIPISensorReg[i].Addr; i++)
    {
		temp_data=IMX220MIPI_read_cmos_sensor(IMX220MIPISensorReg[i].Addr);
		spin_lock(&imx220_drv_lock);
		IMX220MIPISensorReg[i].Para = temp_data;
		spin_unlock(&imx220_drv_lock);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=IMX220MIPISensorReg[i].Addr; i++)
    {
    	temp_data=IMX220MIPI_read_cmos_sensor(IMX220MIPISensorReg[i].Addr);
         spin_lock(&imx220_drv_lock);
        IMX220MIPISensorReg[i].Para = temp_data;
		spin_unlock(&imx220_drv_lock);
    }

}

/*************************************************************************
* FUNCTION
*    IMX220MIPI_get_sensor_group_count
*
* DESCRIPTION
*    //
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_int32  IMX220MIPI_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void IMX220MIPI_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{
   switch (group_idx)
   {
        case PRE_GAIN:
            sprintf((char *)group_name_ptr, "CCT");
            *item_count_ptr = 2;
            break;
        case CMMCLK_CURRENT:
            sprintf((char *)group_name_ptr, "CMMCLK Current");
            *item_count_ptr = 1;
            break;
        case FRAME_RATE_LIMITATION:
            sprintf((char *)group_name_ptr, "Frame Rate Limitation");
            *item_count_ptr = 2;
            break;
        case REGISTER_EDITOR:
            sprintf((char *)group_name_ptr, "Register Editor");
            *item_count_ptr = 2;
            break;
        default:
            ASSERT(0);
}
}

void IMX220MIPI_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{
    kal_int16 temp_reg=0;
    kal_uint16 temp_gain=0, temp_addr=0, temp_para=0;
    
    switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
              case 0:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-R");
                  temp_addr = PRE_GAIN_R_INDEX;
              break;
              case 1:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Gr");
                  temp_addr = PRE_GAIN_Gr_INDEX;
              break;
              case 2:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Gb");
                  temp_addr = PRE_GAIN_Gb_INDEX;
              break;
              case 3:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-B");
                  temp_addr = PRE_GAIN_B_INDEX;
              break;
              case 4:
                 sprintf((char *)info_ptr->ItemNamePtr,"SENSOR_BASEGAIN");
                 temp_addr = SENSOR_BASEGAIN;
              break;
              default:
                 SENSORDB("[IMX105MIPI][Error]get_sensor_item_info error!!!\n");
          }
           	spin_lock(&imx220_drv_lock);    
            temp_para=IMX220MIPISensorCCT[temp_addr].Para;	
			spin_unlock(&imx220_drv_lock);
            temp_gain = IMX220MIPIReg2Gain(temp_para);
            temp_gain=(temp_gain*1000)/BASEGAIN;
            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min=1000;
            info_ptr->Max=15875;//why
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");
                
                    //temp_reg=IMX220MIPISensorReg[CMMCLK_CURRENT_INDEX].Para;
                    temp_reg = ISP_DRIVING_2MA;
                    if(temp_reg==ISP_DRIVING_2MA)
                    {
                        info_ptr->ItemValue=2;
                    }
                    else if(temp_reg==ISP_DRIVING_4MA)
                    {
                        info_ptr->ItemValue=4;
                    }
                    else if(temp_reg==ISP_DRIVING_6MA)
                    {
                        info_ptr->ItemValue=6;
                    }
                    else if(temp_reg==ISP_DRIVING_8MA)
                    {
                        info_ptr->ItemValue=8;
                    }
                
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_TRUE;
                    info_ptr->Min=2;
                    info_ptr->Max=8;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Max Exposure Lines");
                    info_ptr->ItemValue=IMX220MIPI_MAX_EXPOSURE_LINES;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"Min Frame Rate");
                    info_ptr->ItemValue=12;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Addr.");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Value");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                default:
                ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
}

kal_bool IMX220MIPI_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
//   kal_int16 temp_reg;
   kal_uint16 temp_addr=0, temp_para=0;

   switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
              case 0:
                temp_addr = PRE_GAIN_R_INDEX;
              break;
              case 1:
                temp_addr = PRE_GAIN_Gr_INDEX;
              break;
              case 2:
                temp_addr = PRE_GAIN_Gb_INDEX;
              break;
              case 3:
                temp_addr = PRE_GAIN_B_INDEX;
              break;
              case 4:
                temp_addr = SENSOR_BASEGAIN;
              break;
              default:
                 SENSORDB("[IMX220MIPI][Error]set_sensor_item_info error!!!\n");
          }
            temp_para = IMX220MIPIGain2Reg(ItemValue);
            spin_lock(&imx220_drv_lock);    
            IMX220MIPISensorCCT[temp_addr].Para = temp_para;
			spin_unlock(&imx220_drv_lock);
            IMX220MIPI_write_cmos_sensor(IMX220MIPISensorCCT[temp_addr].Addr,temp_para);
			temp_para=read_IMX220MIPI_gain();	

            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    if(ItemValue==2)
                    {			
                    spin_lock(&imx220_drv_lock);    
                        IMX220MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_2MA;
					spin_unlock(&imx220_drv_lock);
                    }
                    else if(ItemValue==3 || ItemValue==4)
                    {
                    	spin_lock(&imx220_drv_lock);    
                        IMX220MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_4MA;
						spin_unlock(&imx220_drv_lock);
                    }
                    else if(ItemValue==5 || ItemValue==6)
                    {
                    	spin_lock(&imx220_drv_lock);    
                        IMX220MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_6MA;
						spin_unlock(&imx220_drv_lock);
                    }
                    else
                    {
                    	spin_lock(&imx220_drv_lock);    
                        IMX220MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_8MA;
						spin_unlock(&imx220_drv_lock);
                    }
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            ASSERT(0);
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
					spin_lock(&imx220_drv_lock);    
                    IMX220MIPI_FAC_SENSOR_REG=ItemValue;
					spin_unlock(&imx220_drv_lock);
                    break;
                case 1:
                    IMX220MIPI_write_cmos_sensor(IMX220MIPI_FAC_SENSOR_REG,ItemValue);
                    break;
                default:
                    ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
    return TRUE;
}

static void IMX220MIPI_SetDummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
  kal_uint32 frame_length = 0, line_length = 0;

//return;
   if(IMX220MIPI_sensor.pv_mode == KAL_TRUE)
   	{
   	 spin_lock(&imx220_drv_lock);    
   	 IMX220MIPI_sensor.pv_dummy_pixels = iPixels;
	 IMX220MIPI_sensor.pv_dummy_lines = iLines;
   	 IMX220MIPI_sensor.pv_line_length = IMX220MIPI_PV_LINE_LENGTH_PIXELS + iPixels;
	 IMX220MIPI_sensor.pv_frame_length = IMX220MIPI_PV_FRAME_LENGTH_LINES + iLines;
	 spin_unlock(&imx220_drv_lock);
	 line_length = IMX220MIPI_sensor.pv_line_length;
	 frame_length = IMX220MIPI_sensor.pv_frame_length;
	 	
   	}
   else if(IMX220MIPI_sensor.video_mode == KAL_TRUE)
   	{
   	 spin_lock(&imx220_drv_lock);    
   	 IMX220MIPI_sensor.video_dummy_pixels = iPixels;
	 IMX220MIPI_sensor.video_dummy_lines = iLines;
   	 IMX220MIPI_sensor.video_line_length = IMX220MIPI_VIDEO_LINE_LENGTH_PIXELS + iPixels;
	 IMX220MIPI_sensor.video_frame_length = IMX220MIPI_VIDEO_FRAME_LENGTH_LINES + iLines;
	 spin_unlock(&imx220_drv_lock);
	 line_length = IMX220MIPI_sensor.video_line_length;
	 frame_length = IMX220MIPI_sensor.video_frame_length;
   	}
    else if(IMX220MIPI_sensor.highspeedvideo_mode == KAL_TRUE)
    {
        spin_lock(&imx220_drv_lock);    
        IMX220MIPI_sensor.video1_dummy_pixels = iPixels;
        IMX220MIPI_sensor.video1_dummy_lines = iLines;
        IMX220MIPI_sensor.video1_line_length = IMX220MIPI_VIDEO1_LINE_LENGTH_PIXELS + iPixels;
        IMX220MIPI_sensor.video1_frame_length = IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES + iLines;
        spin_unlock(&imx220_drv_lock);
        line_length = IMX220MIPI_sensor.video1_line_length;
        frame_length = IMX220MIPI_sensor.video1_frame_length;
    }
    else if(IMX220MIPI_sensor.slimvideo_mode == KAL_TRUE)
    {
        spin_lock(&imx220_drv_lock);    
        IMX220MIPI_sensor.video2_dummy_pixels = iPixels;
        IMX220MIPI_sensor.video2_dummy_lines = iLines;
        IMX220MIPI_sensor.video2_line_length = IMX220MIPI_VIDEO2_LINE_LENGTH_PIXELS + iPixels;
        IMX220MIPI_sensor.video2_frame_length = IMX220MIPI_VIDEO2_FRAME_LENGTH_LINES + iLines;
        spin_unlock(&imx220_drv_lock);
        line_length = IMX220MIPI_sensor.video2_line_length;
        frame_length = IMX220MIPI_sensor.video2_frame_length;
    }
	else
		{
	  spin_lock(&imx220_drv_lock);	
   	  IMX220MIPI_sensor.cp_dummy_pixels = iPixels;
	  IMX220MIPI_sensor.cp_dummy_lines = iLines;
	  IMX220MIPI_sensor.cp_line_length = IMX220MIPI_FULL_LINE_LENGTH_PIXELS + iPixels;
	  IMX220MIPI_sensor.cp_frame_length = IMX220MIPI_FULL_FRAME_LENGTH_LINES + iLines;
	   spin_unlock(&imx220_drv_lock);
	  line_length = IMX220MIPI_sensor.cp_line_length;
	  frame_length = IMX220MIPI_sensor.cp_frame_length;
    }

      IMX220MIPI_write_cmos_sensor(0x0104, 1);        
	  
      IMX220MIPI_write_cmos_sensor(0x0340, (frame_length >>8) & 0xFF);
      IMX220MIPI_write_cmos_sensor(0x0341, frame_length & 0xFF);	
      IMX220MIPI_write_cmos_sensor(0x0342, (line_length >>8) & 0xFF);
      IMX220MIPI_write_cmos_sensor(0x0343, line_length & 0xFF);

      IMX220MIPI_write_cmos_sensor(0x0104, 0);

	  SENSORDB("IMX220MIPI_SetDummy,dumy_pixel=%d,dumy_line=%d,\n",iPixels,iLines);
  
}   /*  IMX220MIPI_SetDummy */

static void IMX220MIPI_Sensor_Init(void)
{
    // Initial settings for sensor
    IMX220MIPI_write_cmos_sensor(0x0101,0x00);
    IMX220MIPI_write_cmos_sensor(0x0220,0x01);
    IMX220MIPI_write_cmos_sensor(0x9278,0x19);
    IMX220MIPI_write_cmos_sensor(0x93B4,0x07);
    IMX220MIPI_write_cmos_sensor(0x93B5,0x07);
    IMX220MIPI_write_cmos_sensor(0x9412,0x29);
    IMX220MIPI_write_cmos_sensor(0x941B,0x1B);
    IMX220MIPI_write_cmos_sensor(0x941F,0x86);
    IMX220MIPI_write_cmos_sensor(0x9431,0x03);
    IMX220MIPI_write_cmos_sensor(0x943E,0x05);
    IMX220MIPI_write_cmos_sensor(0x9507,0x1B);
    IMX220MIPI_write_cmos_sensor(0x950B,0x1B);
    IMX220MIPI_write_cmos_sensor(0x950F,0x1B);
    IMX220MIPI_write_cmos_sensor(0x960F,0xCA);
    IMX220MIPI_write_cmos_sensor(0x9631,0x00);
    IMX220MIPI_write_cmos_sensor(0x9633,0x40);
    //Image Quality
    //Defect correction
    IMX220MIPI_write_cmos_sensor(0x3434,0x01);
    IMX220MIPI_write_cmos_sensor(0x3436,0x01);
    IMX220MIPI_write_cmos_sensor(0x344A,0x28);
    IMX220MIPI_write_cmos_sensor(0x344B,0x14);
    IMX220MIPI_write_cmos_sensor(0x344E,0x14);
    IMX220MIPI_write_cmos_sensor(0x3529,0x00);
    IMX220MIPI_write_cmos_sensor(0x352A,0x00);
    IMX220MIPI_write_cmos_sensor(0x352B,0x00);
    IMX220MIPI_write_cmos_sensor(0x352C,0x00);
    IMX220MIPI_write_cmos_sensor(0x352D,0x00);
    IMX220MIPI_write_cmos_sensor(0x352E,0x00);
    IMX220MIPI_write_cmos_sensor(0x352F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3530,0x00);
    IMX220MIPI_write_cmos_sensor(0x3531,0x00);
    IMX220MIPI_write_cmos_sensor(0x3532,0x00);
    IMX220MIPI_write_cmos_sensor(0x3533,0x00);
    IMX220MIPI_write_cmos_sensor(0x3534,0x00);
    IMX220MIPI_write_cmos_sensor(0x3535,0x00);
    IMX220MIPI_write_cmos_sensor(0x3536,0x00);
    IMX220MIPI_write_cmos_sensor(0x3537,0x00);
    IMX220MIPI_write_cmos_sensor(0x3538,0x00);
    IMX220MIPI_write_cmos_sensor(0x3539,0x00);
    IMX220MIPI_write_cmos_sensor(0x353A,0x00);
    IMX220MIPI_write_cmos_sensor(0x353B,0x00);
    IMX220MIPI_write_cmos_sensor(0x353C,0x00);
    IMX220MIPI_write_cmos_sensor(0x353D,0x00);
    IMX220MIPI_write_cmos_sensor(0x353E,0x00);
    IMX220MIPI_write_cmos_sensor(0x353F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3540,0x00);
    IMX220MIPI_write_cmos_sensor(0x3B0C,0x00);
    IMX220MIPI_write_cmos_sensor(0x3B0D,0x40);
    IMX220MIPI_write_cmos_sensor(0x3C54,0x00);
    IMX220MIPI_write_cmos_sensor(0x3C55,0x02);
    IMX220MIPI_write_cmos_sensor(0x3C56,0x00);
    IMX220MIPI_write_cmos_sensor(0x3C57,0x02);
    IMX220MIPI_write_cmos_sensor(0x3C58,0x00);
    IMX220MIPI_write_cmos_sensor(0x3C59,0x02);
    IMX220MIPI_write_cmos_sensor(0x3D0F,0x40);
    IMX220MIPI_write_cmos_sensor(0x3D10,0x40);
    IMX220MIPI_write_cmos_sensor(0x3D11,0x40);
    IMX220MIPI_write_cmos_sensor(0x3F01,0xC8);
    IMX220MIPI_write_cmos_sensor(0x3F03,0x64);
    IMX220MIPI_write_cmos_sensor(0x3F05,0x32);
    IMX220MIPI_write_cmos_sensor(0x3F07,0xC8);
    IMX220MIPI_write_cmos_sensor(0x3F09,0x64);
    IMX220MIPI_write_cmos_sensor(0x3F0B,0x32);
    //LNR Parameter setting
    IMX220MIPI_write_cmos_sensor(0x3523,0x00);
    IMX220MIPI_write_cmos_sensor(0x3524,0x00);
    IMX220MIPI_write_cmos_sensor(0x3525,0x00);
    IMX220MIPI_write_cmos_sensor(0x3526,0x00);
    IMX220MIPI_write_cmos_sensor(0x3527,0x00);
    IMX220MIPI_write_cmos_sensor(0x3528,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E01,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E03,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E06,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E07,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E08,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E0B,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E0D,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E10,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E11,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E12,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E15,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E17,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E1A,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E1B,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E1C,0x14);
    IMX220MIPI_write_cmos_sensor(0x3E1D,0x02);
    IMX220MIPI_write_cmos_sensor(0x3E1E,0x02);
    IMX220MIPI_write_cmos_sensor(0x3E20,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E21,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E23,0x02);
    IMX220MIPI_write_cmos_sensor(0x3E24,0x02);
    IMX220MIPI_write_cmos_sensor(0x3E26,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E27,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E29,0x02);
    IMX220MIPI_write_cmos_sensor(0x3E2A,0x02);
    IMX220MIPI_write_cmos_sensor(0x3E2C,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E2D,0x00);
    IMX220MIPI_write_cmos_sensor(0x3E2F,0x4C);
    IMX220MIPI_write_cmos_sensor(0x3E30,0x4C);
    //HDR setting
    IMX220MIPI_write_cmos_sensor(0x3201,0xA8);
    IMX220MIPI_write_cmos_sensor(0x3202,0x03);
    IMX220MIPI_write_cmos_sensor(0x3203,0x84);
    IMX220MIPI_write_cmos_sensor(0x3217,0x20);
    IMX220MIPI_write_cmos_sensor(0x321A,0x03);
    IMX220MIPI_write_cmos_sensor(0x321B,0x20);
    IMX220MIPI_write_cmos_sensor(0x3308,0x06);
    IMX220MIPI_write_cmos_sensor(0x3400,0x02);
    IMX220MIPI_write_cmos_sensor(0x3420,0x80);
    IMX220MIPI_write_cmos_sensor(0x3421,0x40);
    IMX220MIPI_write_cmos_sensor(0x3422,0x20);
    IMX220MIPI_write_cmos_sensor(0x3429,0x00);
    IMX220MIPI_write_cmos_sensor(0x342A,0xEC);
    IMX220MIPI_write_cmos_sensor(0x342B,0xD8);
    IMX220MIPI_write_cmos_sensor(0x3500,0x00);
    IMX220MIPI_write_cmos_sensor(0x3501,0x00);
    IMX220MIPI_write_cmos_sensor(0x3502,0x00);
    IMX220MIPI_write_cmos_sensor(0x3503,0x00);
    IMX220MIPI_write_cmos_sensor(0x3504,0x00);
    IMX220MIPI_write_cmos_sensor(0x3505,0x01);
    IMX220MIPI_write_cmos_sensor(0x3544,0x00);
    IMX220MIPI_write_cmos_sensor(0x3545,0x00);
    IMX220MIPI_write_cmos_sensor(0x3546,0x00);
    IMX220MIPI_write_cmos_sensor(0x3547,0x00);
    IMX220MIPI_write_cmos_sensor(0x3548,0x00);
    IMX220MIPI_write_cmos_sensor(0x3549,0x00);
    IMX220MIPI_write_cmos_sensor(0x354A,0x00);
    IMX220MIPI_write_cmos_sensor(0x3600,0x02);
    IMX220MIPI_write_cmos_sensor(0x3601,0x00);
    IMX220MIPI_write_cmos_sensor(0x3602,0x02);
    IMX220MIPI_write_cmos_sensor(0x3603,0x00);
    IMX220MIPI_write_cmos_sensor(0x3604,0x02);
    IMX220MIPI_write_cmos_sensor(0x3605,0x00);
    IMX220MIPI_write_cmos_sensor(0x360C,0x00);
    IMX220MIPI_write_cmos_sensor(0x360D,0x64);
    IMX220MIPI_write_cmos_sensor(0x360E,0x00);
    IMX220MIPI_write_cmos_sensor(0x360F,0x64);
    IMX220MIPI_write_cmos_sensor(0x3610,0x00);
    IMX220MIPI_write_cmos_sensor(0x3611,0x64);
    IMX220MIPI_write_cmos_sensor(0xAA1E,0x00);
    IMX220MIPI_write_cmos_sensor(0xAA1F,0x08);
    IMX220MIPI_write_cmos_sensor(0xAA53,0x01);
    IMX220MIPI_write_cmos_sensor(0xAA70,0x03);
    IMX220MIPI_write_cmos_sensor(0xAA71,0xFF);
    IMX220MIPI_write_cmos_sensor(0xAA72,0x03);
    IMX220MIPI_write_cmos_sensor(0xAA73,0xFF);
    IMX220MIPI_write_cmos_sensor(0xAE45,0x25);
    IMX220MIPI_write_cmos_sensor(0xAE47,0x90);
    IMX220MIPI_write_cmos_sensor(0xAE51,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE5B,0x00);
    //LSC setting
    IMX220MIPI_write_cmos_sensor(0x3801,0x78);
    IMX220MIPI_write_cmos_sensor(0x3803,0x78);
    IMX220MIPI_write_cmos_sensor(0x3805,0x78);
    IMX220MIPI_write_cmos_sensor(0x3809,0x3C);
    IMX220MIPI_write_cmos_sensor(0x380B,0x3C);
    IMX220MIPI_write_cmos_sensor(0x380D,0x78);
    IMX220MIPI_write_cmos_sensor(0x380F,0x78);
    IMX220MIPI_write_cmos_sensor(0x3811,0x78);
    IMX220MIPI_write_cmos_sensor(0x3815,0x3C);
    IMX220MIPI_write_cmos_sensor(0x3817,0x3C);
    IMX220MIPI_write_cmos_sensor(0x3819,0x78);
    IMX220MIPI_write_cmos_sensor(0x381B,0x78);
    IMX220MIPI_write_cmos_sensor(0x381D,0x78);
    IMX220MIPI_write_cmos_sensor(0x3821,0x3C);
    IMX220MIPI_write_cmos_sensor(0x3823,0x3C);
    IMX220MIPI_write_cmos_sensor(0xAC3E,0x70);
    //
	SENSORDB("IMX220MIPI_Sensor_Init exit\n");
    // The register only need to enable 1 time.    
    spin_lock(&imx220_drv_lock);  
    IMX220MIPI_Auto_Flicker_mode = KAL_FALSE;     // reset the flicker status    
	spin_unlock(&imx220_drv_lock);

}   /*  IMX220MIPI_Sensor_Init  */
void VideoSizeSetting_4K2K_30fps(void)//16:9   6M
{	
    // Initial settings for sensor
    //IMX220MIPI_write_cmos_sensor(0x0101,0x00);
    //IMX220MIPI_write_cmos_sensor(0x0220,0x01);
    IMX220MIPI_write_cmos_sensor(0x0100,0x00);// STREAM STop
    //Clock setting
    IMX220MIPI_write_cmos_sensor(0x011E,0x18);
    IMX220MIPI_write_cmos_sensor(0x011F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0301,0x06);
    IMX220MIPI_write_cmos_sensor(0x0303,0x02);
    IMX220MIPI_write_cmos_sensor(0x0304,0x02);
    IMX220MIPI_write_cmos_sensor(0x0305,0x06);
    IMX220MIPI_write_cmos_sensor(0x0306,0x00);
    IMX220MIPI_write_cmos_sensor(0x0307,0x49);
    IMX220MIPI_write_cmos_sensor(0x0309,0x0A);
    IMX220MIPI_write_cmos_sensor(0x030B,0x01);
    IMX220MIPI_write_cmos_sensor(0x030C,0x00);
    IMX220MIPI_write_cmos_sensor(0x030D,0xAE);
    //Size setting
    IMX220MIPI_write_cmos_sensor(0x0340,0x06);
    IMX220MIPI_write_cmos_sensor(0x0341,0x68);
    IMX220MIPI_write_cmos_sensor(0x0342,0x17);
    IMX220MIPI_write_cmos_sensor(0x0343,0x10);
    IMX220MIPI_write_cmos_sensor(0x0344,0x04);  //x_add_sta
    IMX220MIPI_write_cmos_sensor(0x0345,0x78);
    IMX220MIPI_write_cmos_sensor(0x0346,0x04);  //y_add_sta
    IMX220MIPI_write_cmos_sensor(0x0347,0x94);
    IMX220MIPI_write_cmos_sensor(0x0348,0x10);  //x_add_end
    IMX220MIPI_write_cmos_sensor(0x0349,0x07);
    IMX220MIPI_write_cmos_sensor(0x034A,0x0A);  //y_add_end
    IMX220MIPI_write_cmos_sensor(0x034B,0xCB);
    IMX220MIPI_write_cmos_sensor(0x034C,0x0B);  //x_out_size
    IMX220MIPI_write_cmos_sensor(0x034D,0x90);
    IMX220MIPI_write_cmos_sensor(0x034E,0x06);  //y_out_size
    IMX220MIPI_write_cmos_sensor(0x034F,0x36);
    IMX220MIPI_write_cmos_sensor(0x0350,0x00);  //dig_crop_s_strt
    IMX220MIPI_write_cmos_sensor(0x0351,0x00);
    IMX220MIPI_write_cmos_sensor(0x0352,0x00);  //dig_crop_y_strt
    IMX220MIPI_write_cmos_sensor(0x0353,0x02);
    IMX220MIPI_write_cmos_sensor(0x0354,0x0B);  //dig_crop_x_size
    IMX220MIPI_write_cmos_sensor(0x0355,0x90);
    IMX220MIPI_write_cmos_sensor(0x0356,0x06);  //dig_crop_y_size
    IMX220MIPI_write_cmos_sensor(0x0357,0x36);
    IMX220MIPI_write_cmos_sensor(0x901D,0x08);
    //Mode setting
    IMX220MIPI_write_cmos_sensor(0x0108,0x03);
    IMX220MIPI_write_cmos_sensor(0x0112,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0113,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0381,0x01);
    IMX220MIPI_write_cmos_sensor(0x0383,0x01);
    IMX220MIPI_write_cmos_sensor(0x0385,0x01);
    IMX220MIPI_write_cmos_sensor(0x0387,0x01);
    IMX220MIPI_write_cmos_sensor(0x0390,0x00);  //binning_en
    IMX220MIPI_write_cmos_sensor(0x0391,0x11);
    IMX220MIPI_write_cmos_sensor(0x0392,0x00);
    IMX220MIPI_write_cmos_sensor(0x1307,0x00);
    IMX220MIPI_write_cmos_sensor(0x9512,0x40);
    IMX220MIPI_write_cmos_sensor(0xAB07,0x01);
    IMX220MIPI_write_cmos_sensor(0xB802,0x01);
    IMX220MIPI_write_cmos_sensor(0xB933,0x00);
    //Optionnal function 
    IMX220MIPI_write_cmos_sensor(0x0700,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C0,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C1,0x02);
    IMX220MIPI_write_cmos_sensor(0x9909,0x01);
    IMX220MIPI_write_cmos_sensor(0x991E,0x01);
    IMX220MIPI_write_cmos_sensor(0xBA03,0x00);
    IMX220MIPI_write_cmos_sensor(0xBA07,0x00);
    //Global timing setting
    IMX220MIPI_write_cmos_sensor(0x0830,0x00);
    IMX220MIPI_write_cmos_sensor(0x0831,0x6F);
    IMX220MIPI_write_cmos_sensor(0x0832,0x00);
    IMX220MIPI_write_cmos_sensor(0x0833,0x2f);
    IMX220MIPI_write_cmos_sensor(0x0834,0x00);
    IMX220MIPI_write_cmos_sensor(0x0835,0x57);
    IMX220MIPI_write_cmos_sensor(0x0836,0x00);
    IMX220MIPI_write_cmos_sensor(0x0837,0x2f);
    IMX220MIPI_write_cmos_sensor(0x0838,0x00);
    IMX220MIPI_write_cmos_sensor(0x0839,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083A,0x00);
    IMX220MIPI_write_cmos_sensor(0x083B,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083C,0x00);
    IMX220MIPI_write_cmos_sensor(0x083D,0xBF);
    IMX220MIPI_write_cmos_sensor(0x083E,0x00);
    IMX220MIPI_write_cmos_sensor(0x083F,0x27);
    IMX220MIPI_write_cmos_sensor(0x0842,0x00);
    IMX220MIPI_write_cmos_sensor(0x0843,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0844,0x00);
    IMX220MIPI_write_cmos_sensor(0x0845,0x37);
    IMX220MIPI_write_cmos_sensor(0x0847,0x02);
    //Integration time setting
    IMX220MIPI_write_cmos_sensor(0x0202,0x06);
    IMX220MIPI_write_cmos_sensor(0x0203,0x60);
    //Gain setting
    IMX220MIPI_write_cmos_sensor(0x0205,0x00);
    IMX220MIPI_write_cmos_sensor(0x020E,0x01);
    IMX220MIPI_write_cmos_sensor(0x020F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0210,0x01);
    IMX220MIPI_write_cmos_sensor(0x0211,0x00);
    IMX220MIPI_write_cmos_sensor(0x0212,0x01);
    IMX220MIPI_write_cmos_sensor(0x0213,0x00);
    IMX220MIPI_write_cmos_sensor(0x0214,0x01);
    IMX220MIPI_write_cmos_sensor(0x0215,0x00);
    //HDR setting
    IMX220MIPI_write_cmos_sensor(0x0230,0x01);
    IMX220MIPI_write_cmos_sensor(0x0231,0xF4);
    IMX220MIPI_write_cmos_sensor(0x0233,0x00);
    IMX220MIPI_write_cmos_sensor(0x0234,0x00);
    IMX220MIPI_write_cmos_sensor(0x0235,0x40);
    IMX220MIPI_write_cmos_sensor(0x0238,0x00);
    IMX220MIPI_write_cmos_sensor(0x0239,0x08);
    IMX220MIPI_write_cmos_sensor(0x023B,0x02);
    IMX220MIPI_write_cmos_sensor(0x023C,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C2,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C3,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C4,0x00);
    IMX220MIPI_write_cmos_sensor(0x9376,0x35);
    IMX220MIPI_write_cmos_sensor(0x9377,0x01);
    IMX220MIPI_write_cmos_sensor(0x9800,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD34,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD35,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD36,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD37,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD38,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD39,0x5D);
    IMX220MIPI_write_cmos_sensor(0xAD3A,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3B,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3C,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD3D,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD3E,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD3F,0x5D);
    //Bypass setting 
    IMX220MIPI_write_cmos_sensor(0xA303,0x00);
    IMX220MIPI_write_cmos_sensor(0xA403,0x00);
    IMX220MIPI_write_cmos_sensor(0xA602,0x00);
    IMX220MIPI_write_cmos_sensor(0xA703,0x01);
    IMX220MIPI_write_cmos_sensor(0xA903,0x01);
    IMX220MIPI_write_cmos_sensor(0xA956,0x00);
    IMX220MIPI_write_cmos_sensor(0xAA03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAB03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAC03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE03,0x01);
    IMX220MIPI_write_cmos_sensor(0xB803,0x00);
    IMX220MIPI_write_cmos_sensor(0xB91A,0x01);
    IMX220MIPI_write_cmos_sensor(0xBB03,0x00);
    //Single correct setting
    IMX220MIPI_write_cmos_sensor(0x4003,0x00);
    IMX220MIPI_write_cmos_sensor(0x4004,0x00);
    IMX220MIPI_write_cmos_sensor(0x4005,0x00);
    IMX220MIPI_write_cmos_sensor(0x4006,0x80);
    IMX220MIPI_write_cmos_sensor(0x4007,0x80);
    IMX220MIPI_write_cmos_sensor(0x4008,0x80);
    IMX220MIPI_write_cmos_sensor(0x400C,0x00);
    IMX220MIPI_write_cmos_sensor(0x400D,0x00);
    IMX220MIPI_write_cmos_sensor(0x400E,0x00);
    IMX220MIPI_write_cmos_sensor(0x400F,0x80);
    IMX220MIPI_write_cmos_sensor(0x4010,0x80);
    IMX220MIPI_write_cmos_sensor(0x4011,0x80);
    //Mode etc setting 
    IMX220MIPI_write_cmos_sensor(0x3008,0x01);
    IMX220MIPI_write_cmos_sensor(0x3009,0x71);
    IMX220MIPI_write_cmos_sensor(0x300A,0x51);
    IMX220MIPI_write_cmos_sensor(0x3506,0x00);
    IMX220MIPI_write_cmos_sensor(0x3507,0x01);
    IMX220MIPI_write_cmos_sensor(0x3508,0x00);
    IMX220MIPI_write_cmos_sensor(0x3509,0x01);
    IMX220MIPI_write_cmos_sensor(0xA913,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC13,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC3F,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE8B,0x36);
    IMX220MIPI_write_cmos_sensor(0xB812,0x00);
    //LNR setting
    IMX220MIPI_write_cmos_sensor(0x3516,0x00);
    IMX220MIPI_write_cmos_sensor(0x3517,0x00);
    IMX220MIPI_write_cmos_sensor(0x3518,0x00);
    IMX220MIPI_write_cmos_sensor(0x3519,0x00);
    IMX220MIPI_write_cmos_sensor(0x351A,0x00);
    IMX220MIPI_write_cmos_sensor(0x351B,0x00);
    IMX220MIPI_write_cmos_sensor(0x351C,0x00);
    IMX220MIPI_write_cmos_sensor(0x351D,0x00);
    IMX220MIPI_write_cmos_sensor(0x351E,0x00);
    IMX220MIPI_write_cmos_sensor(0x351F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3520,0x00);
    IMX220MIPI_write_cmos_sensor(0x3521,0x00);
    IMX220MIPI_write_cmos_sensor(0x3522,0x00);
    //continue mode
    //IMX220MIPI_write_cmos_sensor(0x9306,0x00);
    //color pattern
    //IMX220MIPI_write_cmos_sensor(0x0601,0x00);
    mdelay(10);
    IMX220MIPI_write_cmos_sensor(0x0100,0x1 );
    
    SENSORDB("VideoSizeSetting_4K2K End\n"); 
}
void VideoSizeSetting_4K2K_22fps(void)//16:9   6M
{	
    // Initial settings for sensor
    //IMX220MIPI_write_cmos_sensor(0x0101,0x00);
    //IMX220MIPI_write_cmos_sensor(0x0220,0x01);
    IMX220MIPI_write_cmos_sensor(0x0100,0x00);// STREAM STop
    //Clock setting
    IMX220MIPI_write_cmos_sensor(0x011E,0x18);
    IMX220MIPI_write_cmos_sensor(0x011F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0301,0x06);
    IMX220MIPI_write_cmos_sensor(0x0303,0x02);
    IMX220MIPI_write_cmos_sensor(0x0304,0x02);
    IMX220MIPI_write_cmos_sensor(0x0305,0x06);
    IMX220MIPI_write_cmos_sensor(0x0306,0x00);
    IMX220MIPI_write_cmos_sensor(0x0307,0x49);
    IMX220MIPI_write_cmos_sensor(0x0309,0x0A);
    IMX220MIPI_write_cmos_sensor(0x030B,0x01);
    IMX220MIPI_write_cmos_sensor(0x030C,0x00);
    IMX220MIPI_write_cmos_sensor(0x030D,0xAE);
    //Size setting
    IMX220MIPI_write_cmos_sensor(0x0340,0x08);
    IMX220MIPI_write_cmos_sensor(0x0341,0xA2);
    IMX220MIPI_write_cmos_sensor(0x0342,0x17);
    IMX220MIPI_write_cmos_sensor(0x0343,0x10);
    IMX220MIPI_write_cmos_sensor(0x0344,0x02);
    IMX220MIPI_write_cmos_sensor(0x0345,0x40);
    IMX220MIPI_write_cmos_sensor(0x0346,0x03);
    IMX220MIPI_write_cmos_sensor(0x0347,0x78);
    IMX220MIPI_write_cmos_sensor(0x0348,0x12);
    IMX220MIPI_write_cmos_sensor(0x0349,0x3F);
    IMX220MIPI_write_cmos_sensor(0x034A,0x0B);
    IMX220MIPI_write_cmos_sensor(0x034B,0xE7);
    IMX220MIPI_write_cmos_sensor(0x034C,0x10);
    IMX220MIPI_write_cmos_sensor(0x034D,0x00);
    IMX220MIPI_write_cmos_sensor(0x034E,0x08);
    IMX220MIPI_write_cmos_sensor(0x034F,0x70);
    IMX220MIPI_write_cmos_sensor(0x0350,0x00);
    IMX220MIPI_write_cmos_sensor(0x0351,0x00);
    IMX220MIPI_write_cmos_sensor(0x0352,0x00);
    IMX220MIPI_write_cmos_sensor(0x0353,0x00);
    IMX220MIPI_write_cmos_sensor(0x0354,0x10);
    IMX220MIPI_write_cmos_sensor(0x0355,0x00);
    IMX220MIPI_write_cmos_sensor(0x0356,0x08);
    IMX220MIPI_write_cmos_sensor(0x0357,0x70);
    IMX220MIPI_write_cmos_sensor(0x901D,0x08);
    //Mode setting
    IMX220MIPI_write_cmos_sensor(0x0108,0x03);
    IMX220MIPI_write_cmos_sensor(0x0112,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0113,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0381,0x01);
    IMX220MIPI_write_cmos_sensor(0x0383,0x01);
    IMX220MIPI_write_cmos_sensor(0x0385,0x01);
    IMX220MIPI_write_cmos_sensor(0x0387,0x01);
    IMX220MIPI_write_cmos_sensor(0x0390,0x00);
    IMX220MIPI_write_cmos_sensor(0x0391,0x11);
    IMX220MIPI_write_cmos_sensor(0x0392,0x00);
    IMX220MIPI_write_cmos_sensor(0x1307,0x00);
    IMX220MIPI_write_cmos_sensor(0x9512,0x40);
    IMX220MIPI_write_cmos_sensor(0xAB07,0x01);
    IMX220MIPI_write_cmos_sensor(0xB802,0x01);
    IMX220MIPI_write_cmos_sensor(0xB933,0x00);
    //Optionnal function 
    IMX220MIPI_write_cmos_sensor(0x0700,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C0,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C1,0x02);
    IMX220MIPI_write_cmos_sensor(0x9909,0x01);
    IMX220MIPI_write_cmos_sensor(0x991E,0x01);
    IMX220MIPI_write_cmos_sensor(0xBA03,0x00);
    IMX220MIPI_write_cmos_sensor(0xBA07,0x00);
    //Global timing setting
    IMX220MIPI_write_cmos_sensor(0x0830,0x00);
    IMX220MIPI_write_cmos_sensor(0x0831,0x6F);
    IMX220MIPI_write_cmos_sensor(0x0832,0x00);
    IMX220MIPI_write_cmos_sensor(0x0833,0x2f);
    IMX220MIPI_write_cmos_sensor(0x0834,0x00);
    IMX220MIPI_write_cmos_sensor(0x0835,0x57);
    IMX220MIPI_write_cmos_sensor(0x0836,0x00);
    IMX220MIPI_write_cmos_sensor(0x0837,0x2f);
    IMX220MIPI_write_cmos_sensor(0x0838,0x00);
    IMX220MIPI_write_cmos_sensor(0x0839,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083A,0x00);
    IMX220MIPI_write_cmos_sensor(0x083B,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083C,0x00);
    IMX220MIPI_write_cmos_sensor(0x083D,0xBF);
    IMX220MIPI_write_cmos_sensor(0x083E,0x00);
    IMX220MIPI_write_cmos_sensor(0x083F,0x27);
    IMX220MIPI_write_cmos_sensor(0x0842,0x00);
    IMX220MIPI_write_cmos_sensor(0x0843,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0844,0x00);
    IMX220MIPI_write_cmos_sensor(0x0845,0x37);
    IMX220MIPI_write_cmos_sensor(0x0847,0x02);
    //Integration time setting
    IMX220MIPI_write_cmos_sensor(0x0202,0x07);
    IMX220MIPI_write_cmos_sensor(0x0203,0xD8);
    //Gain setting
    IMX220MIPI_write_cmos_sensor(0x0205,0x00);
    IMX220MIPI_write_cmos_sensor(0x020E,0x01);
    IMX220MIPI_write_cmos_sensor(0x020F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0210,0x01);
    IMX220MIPI_write_cmos_sensor(0x0211,0x00);
    IMX220MIPI_write_cmos_sensor(0x0212,0x01);
    IMX220MIPI_write_cmos_sensor(0x0213,0x00);
    IMX220MIPI_write_cmos_sensor(0x0214,0x01);
    IMX220MIPI_write_cmos_sensor(0x0215,0x00);
    //HDR setting
    IMX220MIPI_write_cmos_sensor(0x0230,0x01);
    IMX220MIPI_write_cmos_sensor(0x0231,0xF4);
    IMX220MIPI_write_cmos_sensor(0x0233,0x00);
    IMX220MIPI_write_cmos_sensor(0x0234,0x00);
    IMX220MIPI_write_cmos_sensor(0x0235,0x40);
    IMX220MIPI_write_cmos_sensor(0x0238,0x00);
    IMX220MIPI_write_cmos_sensor(0x0239,0x08);
    IMX220MIPI_write_cmos_sensor(0x023B,0x02);
    IMX220MIPI_write_cmos_sensor(0x023C,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C2,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C3,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C4,0x00);
    IMX220MIPI_write_cmos_sensor(0x9376,0x35);
    IMX220MIPI_write_cmos_sensor(0x9377,0x01);
    IMX220MIPI_write_cmos_sensor(0x9800,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD34,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD35,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD36,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD37,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD38,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD39,0x5D);
    IMX220MIPI_write_cmos_sensor(0xAD3A,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3B,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3C,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD3D,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD3E,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD3F,0x5D);
    //Bypass setting 
    IMX220MIPI_write_cmos_sensor(0xA303,0x00);
    IMX220MIPI_write_cmos_sensor(0xA403,0x00);
    IMX220MIPI_write_cmos_sensor(0xA602,0x00);
    IMX220MIPI_write_cmos_sensor(0xA703,0x01);
    IMX220MIPI_write_cmos_sensor(0xA903,0x01);
    IMX220MIPI_write_cmos_sensor(0xA956,0x00);
    IMX220MIPI_write_cmos_sensor(0xAA03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAB03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAC03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE03,0x01);
    IMX220MIPI_write_cmos_sensor(0xB803,0x00);
    IMX220MIPI_write_cmos_sensor(0xB91A,0x01);
    IMX220MIPI_write_cmos_sensor(0xBB03,0x00);
    //Single correct setting
    IMX220MIPI_write_cmos_sensor(0x4003,0x00);
    IMX220MIPI_write_cmos_sensor(0x4004,0x00);
    IMX220MIPI_write_cmos_sensor(0x4005,0x00);
    IMX220MIPI_write_cmos_sensor(0x4006,0x80);
    IMX220MIPI_write_cmos_sensor(0x4007,0x80);
    IMX220MIPI_write_cmos_sensor(0x4008,0x80);
    IMX220MIPI_write_cmos_sensor(0x400C,0x00);
    IMX220MIPI_write_cmos_sensor(0x400D,0x00);
    IMX220MIPI_write_cmos_sensor(0x400E,0x00);
    IMX220MIPI_write_cmos_sensor(0x400F,0x80);
    IMX220MIPI_write_cmos_sensor(0x4010,0x80);
    IMX220MIPI_write_cmos_sensor(0x4011,0x80);
    //Mode etc setting 
    IMX220MIPI_write_cmos_sensor(0x3008,0x01);
    IMX220MIPI_write_cmos_sensor(0x3009,0x71);
    IMX220MIPI_write_cmos_sensor(0x300A,0x51);
    IMX220MIPI_write_cmos_sensor(0x3506,0x00);
    IMX220MIPI_write_cmos_sensor(0x3507,0x01);
    IMX220MIPI_write_cmos_sensor(0x3508,0x00);
    IMX220MIPI_write_cmos_sensor(0x3509,0x01);
    IMX220MIPI_write_cmos_sensor(0xA913,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC13,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC3F,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE8B,0x36);
    IMX220MIPI_write_cmos_sensor(0xB812,0x00);
    //LNR setting
    IMX220MIPI_write_cmos_sensor(0x3516,0x00);
    IMX220MIPI_write_cmos_sensor(0x3517,0x00);
    IMX220MIPI_write_cmos_sensor(0x3518,0x00);
    IMX220MIPI_write_cmos_sensor(0x3519,0x00);
    IMX220MIPI_write_cmos_sensor(0x351A,0x00);
    IMX220MIPI_write_cmos_sensor(0x351B,0x00);
    IMX220MIPI_write_cmos_sensor(0x351C,0x00);
    IMX220MIPI_write_cmos_sensor(0x351D,0x00);
    IMX220MIPI_write_cmos_sensor(0x351E,0x00);
    IMX220MIPI_write_cmos_sensor(0x351F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3520,0x00);
    IMX220MIPI_write_cmos_sensor(0x3521,0x00);
    IMX220MIPI_write_cmos_sensor(0x3522,0x00);
    //continue mode
    //IMX220MIPI_write_cmos_sensor(0x9306,0x00);
    //color pattern
    //IMX220MIPI_write_cmos_sensor(0x0601,0x00);
    IMX220MIPI_write_cmos_sensor(0x0100,0x1 );
    
    SENSORDB("VideoSizeSetting_4K2K 22fps End\n"); 
}

void VideoHDSetting60fps(void)//16:9   6M
{      

   //PLL setting
    IMX220MIPI_write_cmos_sensor(0x0100,0x00);// STREAM STop
    //Clock setting
    IMX220MIPI_write_cmos_sensor(0x011E,0x18);
    IMX220MIPI_write_cmos_sensor(0x011F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0301,0x06);
    IMX220MIPI_write_cmos_sensor(0x0303,0x02);
    IMX220MIPI_write_cmos_sensor(0x0304,0x02);
    IMX220MIPI_write_cmos_sensor(0x0305,0x06);
    IMX220MIPI_write_cmos_sensor(0x0306,0x00);
    IMX220MIPI_write_cmos_sensor(0x0307,0x49);
    IMX220MIPI_write_cmos_sensor(0x0309,0x0A);
    IMX220MIPI_write_cmos_sensor(0x030B,0x01);
    IMX220MIPI_write_cmos_sensor(0x030C,0x00);
    IMX220MIPI_write_cmos_sensor(0x030D,0xAE);
    //Size setting
    IMX220MIPI_write_cmos_sensor(0x0340,0x03);
    IMX220MIPI_write_cmos_sensor(0x0341,0x36);
    IMX220MIPI_write_cmos_sensor(0x0342,0x17);
    IMX220MIPI_write_cmos_sensor(0x0343,0x10);
    IMX220MIPI_write_cmos_sensor(0x0344,0x00);  //x_add_sta
    IMX220MIPI_write_cmos_sensor(0x0345,0x30);
    IMX220MIPI_write_cmos_sensor(0x0346,0x02);  //y_add_sta
    IMX220MIPI_write_cmos_sensor(0x0347,0x00);
    IMX220MIPI_write_cmos_sensor(0x0348,0x14);  //x_add_end
    IMX220MIPI_write_cmos_sensor(0x0349,0x4F);
    IMX220MIPI_write_cmos_sensor(0x034A,0x0D);  //y_add_end
    IMX220MIPI_write_cmos_sensor(0x034B,0x5F);
    IMX220MIPI_write_cmos_sensor(0x034C,0x05);  //x_out_size
    IMX220MIPI_write_cmos_sensor(0x034D,0x08);
    IMX220MIPI_write_cmos_sensor(0x034E,0x02);  //y_out_size
    IMX220MIPI_write_cmos_sensor(0x034F,0xD8);
    IMX220MIPI_write_cmos_sensor(0x0350,0x00);  //dig_crop_s_strt
    IMX220MIPI_write_cmos_sensor(0x0351,0x00);
    IMX220MIPI_write_cmos_sensor(0x0352,0x00);  //dig_crop_y_strt
    IMX220MIPI_write_cmos_sensor(0x0353,0x00);
    IMX220MIPI_write_cmos_sensor(0x0354,0x05);  //dig_crop_x_size
    IMX220MIPI_write_cmos_sensor(0x0355,0x08);
    IMX220MIPI_write_cmos_sensor(0x0356,0x02);  //dig_crop_y_size
    IMX220MIPI_write_cmos_sensor(0x0357,0xD8);
    IMX220MIPI_write_cmos_sensor(0x901D,0x08);
    //Mode setting
    IMX220MIPI_write_cmos_sensor(0x0108,0x03);
    IMX220MIPI_write_cmos_sensor(0x0112,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0113,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0381,0x01);
    IMX220MIPI_write_cmos_sensor(0x0383,0x01);
    IMX220MIPI_write_cmos_sensor(0x0385,0x01);
    IMX220MIPI_write_cmos_sensor(0x0387,0x03);
    IMX220MIPI_write_cmos_sensor(0x0390,0x01);
    IMX220MIPI_write_cmos_sensor(0x0391,0x42);
    IMX220MIPI_write_cmos_sensor(0x0392,0x00);
    IMX220MIPI_write_cmos_sensor(0x1307,0x02);
    IMX220MIPI_write_cmos_sensor(0x9512,0x40);
    IMX220MIPI_write_cmos_sensor(0xAB07,0x01);
    IMX220MIPI_write_cmos_sensor(0xB802,0x01);
    IMX220MIPI_write_cmos_sensor(0xB933,0x01);
    //Optionnal function 
    IMX220MIPI_write_cmos_sensor(0x0700,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C0,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C1,0x01);
    IMX220MIPI_write_cmos_sensor(0x9909,0x01);
    IMX220MIPI_write_cmos_sensor(0x991E,0x01);
    IMX220MIPI_write_cmos_sensor(0xBA03,0x00);
    IMX220MIPI_write_cmos_sensor(0xBA07,0x00);
    //Global timing setting
    IMX220MIPI_write_cmos_sensor(0x0830,0x00);
    IMX220MIPI_write_cmos_sensor(0x0831,0x6F);
    IMX220MIPI_write_cmos_sensor(0x0832,0x00);
    IMX220MIPI_write_cmos_sensor(0x0833,0x2F);
    IMX220MIPI_write_cmos_sensor(0x0834,0x00);
    IMX220MIPI_write_cmos_sensor(0x0835,0x57);
    IMX220MIPI_write_cmos_sensor(0x0836,0x00);
    IMX220MIPI_write_cmos_sensor(0x0837,0x2F);
    IMX220MIPI_write_cmos_sensor(0x0838,0x00);
    IMX220MIPI_write_cmos_sensor(0x0839,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083A,0x00);
    IMX220MIPI_write_cmos_sensor(0x083B,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083C,0x00);
    IMX220MIPI_write_cmos_sensor(0x083D,0xBF);
    IMX220MIPI_write_cmos_sensor(0x083E,0x00);
    IMX220MIPI_write_cmos_sensor(0x083F,0x27);
    IMX220MIPI_write_cmos_sensor(0x0842,0x00);
    IMX220MIPI_write_cmos_sensor(0x0843,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0844,0x00);
    IMX220MIPI_write_cmos_sensor(0x0845,0x37);
    IMX220MIPI_write_cmos_sensor(0x0847,0x02);
    //Integration time setting
    IMX220MIPI_write_cmos_sensor(0x0202,0x03);
    IMX220MIPI_write_cmos_sensor(0x0203,0x2E);
    //Gain setting
    IMX220MIPI_write_cmos_sensor(0x0205,0x00);
    IMX220MIPI_write_cmos_sensor(0x020E,0x01);
    IMX220MIPI_write_cmos_sensor(0x020F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0210,0x01);
    IMX220MIPI_write_cmos_sensor(0x0211,0x00);
    IMX220MIPI_write_cmos_sensor(0x0212,0x01);
    IMX220MIPI_write_cmos_sensor(0x0213,0x00);
    IMX220MIPI_write_cmos_sensor(0x0214,0x01);
    IMX220MIPI_write_cmos_sensor(0x0215,0x00);
    //HDR setting
    IMX220MIPI_write_cmos_sensor(0x0230,0x01);
    IMX220MIPI_write_cmos_sensor(0x0231,0xF4);
    IMX220MIPI_write_cmos_sensor(0x0233,0x00);
    IMX220MIPI_write_cmos_sensor(0x0234,0x00);
    IMX220MIPI_write_cmos_sensor(0x0235,0x40);
    IMX220MIPI_write_cmos_sensor(0x0238,0x00);
    IMX220MIPI_write_cmos_sensor(0x0239,0x08);
    IMX220MIPI_write_cmos_sensor(0x023B,0x02);
    IMX220MIPI_write_cmos_sensor(0x023C,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C2,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C3,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C4,0x00);
    IMX220MIPI_write_cmos_sensor(0x9376,0x35);
    IMX220MIPI_write_cmos_sensor(0x9377,0x01);
    IMX220MIPI_write_cmos_sensor(0x9800,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD34,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD35,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD36,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD37,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD38,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD39,0x5D);
    IMX220MIPI_write_cmos_sensor(0xAD3A,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3B,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3C,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD3D,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD3E,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD3F,0x5D);
    //Bypass setting 
    IMX220MIPI_write_cmos_sensor(0xA303,0x00);
    IMX220MIPI_write_cmos_sensor(0xA403,0x00);
    IMX220MIPI_write_cmos_sensor(0xA602,0x00);
    IMX220MIPI_write_cmos_sensor(0xA703,0x01);
    IMX220MIPI_write_cmos_sensor(0xA903,0x01);
    IMX220MIPI_write_cmos_sensor(0xA956,0x00);
    IMX220MIPI_write_cmos_sensor(0xAA03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAB03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE03,0x01);
    IMX220MIPI_write_cmos_sensor(0xB803,0x00);
    IMX220MIPI_write_cmos_sensor(0xB91A,0x01);
    IMX220MIPI_write_cmos_sensor(0xBB03,0x00);
    //Single correct setting
    IMX220MIPI_write_cmos_sensor(0x4003,0x00);
    IMX220MIPI_write_cmos_sensor(0x4004,0x00);
    IMX220MIPI_write_cmos_sensor(0x4005,0x00);
    IMX220MIPI_write_cmos_sensor(0x4006,0x80);
    IMX220MIPI_write_cmos_sensor(0x4007,0x80);
    IMX220MIPI_write_cmos_sensor(0x4008,0x80);
    IMX220MIPI_write_cmos_sensor(0x400C,0x00);
    IMX220MIPI_write_cmos_sensor(0x400D,0x00);
    IMX220MIPI_write_cmos_sensor(0x400E,0x00);
    IMX220MIPI_write_cmos_sensor(0x400F,0x80);
    IMX220MIPI_write_cmos_sensor(0x4010,0x80);
    IMX220MIPI_write_cmos_sensor(0x4011,0x80);
    //Mode etc setting 
    IMX220MIPI_write_cmos_sensor(0x3008,0x01);
    IMX220MIPI_write_cmos_sensor(0x3009,0x71);
    IMX220MIPI_write_cmos_sensor(0x300A,0x51);
    IMX220MIPI_write_cmos_sensor(0x3506,0x00);
    IMX220MIPI_write_cmos_sensor(0x3507,0x01);
    IMX220MIPI_write_cmos_sensor(0x3508,0x00);
    IMX220MIPI_write_cmos_sensor(0x3509,0x01);
    IMX220MIPI_write_cmos_sensor(0xA913,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC13,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC3F,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE8B,0x29);
    IMX220MIPI_write_cmos_sensor(0xB812,0x01);
    //LNR setting
    IMX220MIPI_write_cmos_sensor(0x3516,0x00);
    IMX220MIPI_write_cmos_sensor(0x3517,0x00);
    IMX220MIPI_write_cmos_sensor(0x3518,0x00);
    IMX220MIPI_write_cmos_sensor(0x3519,0x00);
    IMX220MIPI_write_cmos_sensor(0x351A,0x00);
    IMX220MIPI_write_cmos_sensor(0x351B,0x00);
    IMX220MIPI_write_cmos_sensor(0x351C,0x00);
    IMX220MIPI_write_cmos_sensor(0x351D,0x00);
    IMX220MIPI_write_cmos_sensor(0x351E,0x00);
    IMX220MIPI_write_cmos_sensor(0x351F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3520,0x00);
    IMX220MIPI_write_cmos_sensor(0x3521,0x00);
    IMX220MIPI_write_cmos_sensor(0x3522,0x00);
    //continue mode
    //IMX220MIPI_write_cmos_sensor(0x9306,0x00);   
    //color pattern
    //IMX220MIPI_write_cmos_sensor(0x0601,0x00);
    mdelay(10);
    IMX220MIPI_write_cmos_sensor(0x0100,0x1 );
    
    SENSORDB("VideoHDSetting_60fps end\n"); 

 
}

void VideoHDSetting30fps(void)//16:9   6M
{   
   
    //PLL setting
    IMX220MIPI_write_cmos_sensor(0x0100,0x00);// STREAM STop
    //Clock setting
    IMX220MIPI_write_cmos_sensor(0x011E,0x18);
    IMX220MIPI_write_cmos_sensor(0x011F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0301,0x06);
    IMX220MIPI_write_cmos_sensor(0x0303,0x02);
    IMX220MIPI_write_cmos_sensor(0x0304,0x02);
    IMX220MIPI_write_cmos_sensor(0x0305,0x06);
    IMX220MIPI_write_cmos_sensor(0x0306,0x00);
    IMX220MIPI_write_cmos_sensor(0x0307,0x49);
    IMX220MIPI_write_cmos_sensor(0x0309,0x0A);
    IMX220MIPI_write_cmos_sensor(0x030B,0x01);
    IMX220MIPI_write_cmos_sensor(0x030C,0x00);
    IMX220MIPI_write_cmos_sensor(0x030D,0xAE);
    //Size setting
    IMX220MIPI_write_cmos_sensor(0x0340,0x06);
    IMX220MIPI_write_cmos_sensor(0x0341,0x6C);
    IMX220MIPI_write_cmos_sensor(0x0342,0x17);
    IMX220MIPI_write_cmos_sensor(0x0343,0x10);
    IMX220MIPI_write_cmos_sensor(0x0344,0x00);
    IMX220MIPI_write_cmos_sensor(0x0345,0x30);
    IMX220MIPI_write_cmos_sensor(0x0346,0x02);
    IMX220MIPI_write_cmos_sensor(0x0347,0x00);
    IMX220MIPI_write_cmos_sensor(0x0348,0x14);
    IMX220MIPI_write_cmos_sensor(0x0349,0x4F);
    IMX220MIPI_write_cmos_sensor(0x034A,0x0D);
    IMX220MIPI_write_cmos_sensor(0x034B,0x5F);
    IMX220MIPI_write_cmos_sensor(0x034C,0x05);
    IMX220MIPI_write_cmos_sensor(0x034D,0x08);
    IMX220MIPI_write_cmos_sensor(0x034E,0x02);
    IMX220MIPI_write_cmos_sensor(0x034F,0xD8);
    IMX220MIPI_write_cmos_sensor(0x0350,0x00);
    IMX220MIPI_write_cmos_sensor(0x0351,0x00);
    IMX220MIPI_write_cmos_sensor(0x0352,0x00);
    IMX220MIPI_write_cmos_sensor(0x0353,0x00);
    IMX220MIPI_write_cmos_sensor(0x0354,0x05);
    IMX220MIPI_write_cmos_sensor(0x0355,0x08);
    IMX220MIPI_write_cmos_sensor(0x0356,0x02);
    IMX220MIPI_write_cmos_sensor(0x0357,0xD8);
    IMX220MIPI_write_cmos_sensor(0x901D,0x08);
    //Mode setting
    IMX220MIPI_write_cmos_sensor(0x0108,0x03);
    IMX220MIPI_write_cmos_sensor(0x0112,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0113,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0381,0x01);
    IMX220MIPI_write_cmos_sensor(0x0383,0x01);
    IMX220MIPI_write_cmos_sensor(0x0385,0x01);
    IMX220MIPI_write_cmos_sensor(0x0387,0x03);
    IMX220MIPI_write_cmos_sensor(0x0390,0x01);
    IMX220MIPI_write_cmos_sensor(0x0391,0x42);
    IMX220MIPI_write_cmos_sensor(0x0392,0x00);
    IMX220MIPI_write_cmos_sensor(0x1307,0x02);
    IMX220MIPI_write_cmos_sensor(0x9512,0x40);
    IMX220MIPI_write_cmos_sensor(0xAB07,0x01);
    IMX220MIPI_write_cmos_sensor(0xB802,0x01);
    IMX220MIPI_write_cmos_sensor(0xB933,0x01);
    //Optionnal function 
    IMX220MIPI_write_cmos_sensor(0x0700,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C0,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C1,0x01);
    IMX220MIPI_write_cmos_sensor(0x9909,0x01);
    IMX220MIPI_write_cmos_sensor(0x991E,0x01);
    IMX220MIPI_write_cmos_sensor(0xBA03,0x00);
    IMX220MIPI_write_cmos_sensor(0xBA07,0x00);
    //Global timing setting
    IMX220MIPI_write_cmos_sensor(0x0830,0x00);
    IMX220MIPI_write_cmos_sensor(0x0831,0x6F);
    IMX220MIPI_write_cmos_sensor(0x0832,0x00);
    IMX220MIPI_write_cmos_sensor(0x0833,0x2F);
    IMX220MIPI_write_cmos_sensor(0x0834,0x00);
    IMX220MIPI_write_cmos_sensor(0x0835,0x57);
    IMX220MIPI_write_cmos_sensor(0x0836,0x00);
    IMX220MIPI_write_cmos_sensor(0x0837,0x2F);
    IMX220MIPI_write_cmos_sensor(0x0838,0x00);
    IMX220MIPI_write_cmos_sensor(0x0839,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083A,0x00);
    IMX220MIPI_write_cmos_sensor(0x083B,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083C,0x00);
    IMX220MIPI_write_cmos_sensor(0x083D,0xBF);
    IMX220MIPI_write_cmos_sensor(0x083E,0x00);
    IMX220MIPI_write_cmos_sensor(0x083F,0x27);
    IMX220MIPI_write_cmos_sensor(0x0842,0x00);
    IMX220MIPI_write_cmos_sensor(0x0843,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0844,0x00);
    IMX220MIPI_write_cmos_sensor(0x0845,0x37);
    IMX220MIPI_write_cmos_sensor(0x0847,0x02);
    //Integration time setting
    IMX220MIPI_write_cmos_sensor(0x0202,0x06);
    IMX220MIPI_write_cmos_sensor(0x0203,0x64);
    //Gain setting
    IMX220MIPI_write_cmos_sensor(0x0205,0x00);
    IMX220MIPI_write_cmos_sensor(0x020E,0x01);
    IMX220MIPI_write_cmos_sensor(0x020F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0210,0x01);
    IMX220MIPI_write_cmos_sensor(0x0211,0x00);
    IMX220MIPI_write_cmos_sensor(0x0212,0x01);
    IMX220MIPI_write_cmos_sensor(0x0213,0x00);
    IMX220MIPI_write_cmos_sensor(0x0214,0x01);
    IMX220MIPI_write_cmos_sensor(0x0215,0x00);
    //HDR setting
    IMX220MIPI_write_cmos_sensor(0x0230,0x01);
    IMX220MIPI_write_cmos_sensor(0x0231,0xF4);
    IMX220MIPI_write_cmos_sensor(0x0233,0x00);
    IMX220MIPI_write_cmos_sensor(0x0234,0x00);
    IMX220MIPI_write_cmos_sensor(0x0235,0x40);
    IMX220MIPI_write_cmos_sensor(0x0238,0x00);
    IMX220MIPI_write_cmos_sensor(0x0239,0x08);
    IMX220MIPI_write_cmos_sensor(0x023B,0x02);
    IMX220MIPI_write_cmos_sensor(0x023C,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C2,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C3,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C4,0x00);
    IMX220MIPI_write_cmos_sensor(0x9376,0x35);
    IMX220MIPI_write_cmos_sensor(0x9377,0x01);
    IMX220MIPI_write_cmos_sensor(0x9800,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD34,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD35,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD36,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD37,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD38,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD39,0x5D);
    IMX220MIPI_write_cmos_sensor(0xAD3A,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3B,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3C,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD3D,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD3E,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD3F,0x5D);
    //Bypass setting 
    IMX220MIPI_write_cmos_sensor(0xA303,0x00);
    IMX220MIPI_write_cmos_sensor(0xA403,0x00);
    IMX220MIPI_write_cmos_sensor(0xA602,0x00);
    IMX220MIPI_write_cmos_sensor(0xA703,0x01);
    IMX220MIPI_write_cmos_sensor(0xA903,0x01);
    IMX220MIPI_write_cmos_sensor(0xA956,0x00);
    IMX220MIPI_write_cmos_sensor(0xAA03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAB03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE03,0x01);
    IMX220MIPI_write_cmos_sensor(0xB803,0x00);
    IMX220MIPI_write_cmos_sensor(0xB91A,0x01);
    IMX220MIPI_write_cmos_sensor(0xBB03,0x00);
    //Single correct setting
    IMX220MIPI_write_cmos_sensor(0x4003,0x00);
    IMX220MIPI_write_cmos_sensor(0x4004,0x00);
    IMX220MIPI_write_cmos_sensor(0x4005,0x00);
    IMX220MIPI_write_cmos_sensor(0x4006,0x80);
    IMX220MIPI_write_cmos_sensor(0x4007,0x80);
    IMX220MIPI_write_cmos_sensor(0x4008,0x80);
    IMX220MIPI_write_cmos_sensor(0x400C,0x00);
    IMX220MIPI_write_cmos_sensor(0x400D,0x00);
    IMX220MIPI_write_cmos_sensor(0x400E,0x00);
    IMX220MIPI_write_cmos_sensor(0x400F,0x80);
    IMX220MIPI_write_cmos_sensor(0x4010,0x80);
    IMX220MIPI_write_cmos_sensor(0x4011,0x80);
    //Mode etc setting 
    IMX220MIPI_write_cmos_sensor(0x3008,0x01);
    IMX220MIPI_write_cmos_sensor(0x3009,0x71);
    IMX220MIPI_write_cmos_sensor(0x300A,0x51);
    IMX220MIPI_write_cmos_sensor(0x3506,0x00);
    IMX220MIPI_write_cmos_sensor(0x3507,0x01);
    IMX220MIPI_write_cmos_sensor(0x3508,0x00);
    IMX220MIPI_write_cmos_sensor(0x3509,0x01);
    IMX220MIPI_write_cmos_sensor(0xA913,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC13,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC3F,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE8B,0x29);
    IMX220MIPI_write_cmos_sensor(0xB812,0x01);
    //LNR setting
    IMX220MIPI_write_cmos_sensor(0x3516,0x00);
    IMX220MIPI_write_cmos_sensor(0x3517,0x00);
    IMX220MIPI_write_cmos_sensor(0x3518,0x00);
    IMX220MIPI_write_cmos_sensor(0x3519,0x00);
    IMX220MIPI_write_cmos_sensor(0x351A,0x00);
    IMX220MIPI_write_cmos_sensor(0x351B,0x00);
    IMX220MIPI_write_cmos_sensor(0x351C,0x00);
    IMX220MIPI_write_cmos_sensor(0x351D,0x00);
    IMX220MIPI_write_cmos_sensor(0x351E,0x00);
    IMX220MIPI_write_cmos_sensor(0x351F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3520,0x00);
    IMX220MIPI_write_cmos_sensor(0x3521,0x00);
    IMX220MIPI_write_cmos_sensor(0x3522,0x00);
    //continue mode
    //IMX220MIPI_write_cmos_sensor(0x9306,0x00);   
    //color pattern
    //IMX220MIPI_write_cmos_sensor(0x0601,0x00);
    mdelay(10);
    IMX220MIPI_write_cmos_sensor(0x0100,0x1 );
        

    SENSORDB("VideoHDSetting30fps exit\n");
}

void PreviewSizeSetting(void)
{	
    // Initial settings for sensor
    //IMX220MIPI_write_cmos_sensor(0x0101,0x00);
    //IMX220MIPI_write_cmos_sensor(0x0220,0x01);
    //PLL setting 
	IMX220MIPI_write_cmos_sensor(0x0100,0x00);// STREAM STop
    //Clock setting
    IMX220MIPI_write_cmos_sensor(0x011E,0x18);
    IMX220MIPI_write_cmos_sensor(0x011F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0301,0x06);
    IMX220MIPI_write_cmos_sensor(0x0303,0x02);
    IMX220MIPI_write_cmos_sensor(0x0304,0x02);
    IMX220MIPI_write_cmos_sensor(0x0305,0x06);
    IMX220MIPI_write_cmos_sensor(0x0306,0x00);
    IMX220MIPI_write_cmos_sensor(0x0307,0x49);
    IMX220MIPI_write_cmos_sensor(0x0309,0x0A);
    IMX220MIPI_write_cmos_sensor(0x030B,0x01);
    IMX220MIPI_write_cmos_sensor(0x030C,0x00);
    IMX220MIPI_write_cmos_sensor(0x030D,0x5c);
    //Size setting
    IMX220MIPI_write_cmos_sensor(0x0340,0x07);
    IMX220MIPI_write_cmos_sensor(0x0341,0xE0);
    IMX220MIPI_write_cmos_sensor(0x0342,0x17);
    IMX220MIPI_write_cmos_sensor(0x0343,0x10);
    IMX220MIPI_write_cmos_sensor(0x0344,0x00);
    IMX220MIPI_write_cmos_sensor(0x0345,0x00);
    IMX220MIPI_write_cmos_sensor(0x0346,0x00);
    IMX220MIPI_write_cmos_sensor(0x0347,0x00);
    IMX220MIPI_write_cmos_sensor(0x0348,0x14);
    IMX220MIPI_write_cmos_sensor(0x0349,0x7F);
    IMX220MIPI_write_cmos_sensor(0x034A,0x0F);
    IMX220MIPI_write_cmos_sensor(0x034B,0x5F);
    IMX220MIPI_write_cmos_sensor(0x034C,0x0A);
    IMX220MIPI_write_cmos_sensor(0x034D,0x40);
    IMX220MIPI_write_cmos_sensor(0x034E,0x07);
    IMX220MIPI_write_cmos_sensor(0x034F,0xB0);
    IMX220MIPI_write_cmos_sensor(0x0350,0x00);
    IMX220MIPI_write_cmos_sensor(0x0351,0x00);
    IMX220MIPI_write_cmos_sensor(0x0352,0x00);
    IMX220MIPI_write_cmos_sensor(0x0353,0x00);
    IMX220MIPI_write_cmos_sensor(0x0354,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0355,0x40);
    IMX220MIPI_write_cmos_sensor(0x0356,0x07);
    IMX220MIPI_write_cmos_sensor(0x0357,0xB0);
    IMX220MIPI_write_cmos_sensor(0x901D,0x08);
    //Mode setting
    IMX220MIPI_write_cmos_sensor(0x0108,0x03);
    IMX220MIPI_write_cmos_sensor(0x0112,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0113,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0381,0x01);
    IMX220MIPI_write_cmos_sensor(0x0383,0x01);
    IMX220MIPI_write_cmos_sensor(0x0385,0x01);
    IMX220MIPI_write_cmos_sensor(0x0387,0x01);
    IMX220MIPI_write_cmos_sensor(0x0390,0x01);
    IMX220MIPI_write_cmos_sensor(0x0391,0x22);
    IMX220MIPI_write_cmos_sensor(0x0392,0x00);
    IMX220MIPI_write_cmos_sensor(0x1307,0x02);
    IMX220MIPI_write_cmos_sensor(0x9512,0x40);
    IMX220MIPI_write_cmos_sensor(0xAB07,0x01);
    IMX220MIPI_write_cmos_sensor(0xB802,0x01);
    IMX220MIPI_write_cmos_sensor(0xB933,0x00);
    //Optionnal function 
    IMX220MIPI_write_cmos_sensor(0x0700,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C0,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C1,0x01);
    IMX220MIPI_write_cmos_sensor(0x9909,0x01);
    IMX220MIPI_write_cmos_sensor(0x991E,0x01);
    IMX220MIPI_write_cmos_sensor(0xBA03,0x00);
    IMX220MIPI_write_cmos_sensor(0xBA07,0x00);
    //Global timing setting
    IMX220MIPI_write_cmos_sensor(0x0830,0x00);
    IMX220MIPI_write_cmos_sensor(0x0831,0x57);
    IMX220MIPI_write_cmos_sensor(0x0832,0x00);
    IMX220MIPI_write_cmos_sensor(0x0833,0x1f);
    IMX220MIPI_write_cmos_sensor(0x0834,0x00);
    IMX220MIPI_write_cmos_sensor(0x0835,0x37);
    IMX220MIPI_write_cmos_sensor(0x0836,0x00);
    IMX220MIPI_write_cmos_sensor(0x0837,0x1f);
    IMX220MIPI_write_cmos_sensor(0x0838,0x00);
    IMX220MIPI_write_cmos_sensor(0x0839,0x17);
    IMX220MIPI_write_cmos_sensor(0x083A,0x00);
    IMX220MIPI_write_cmos_sensor(0x083B,0x17);
    IMX220MIPI_write_cmos_sensor(0x083C,0x00);
    IMX220MIPI_write_cmos_sensor(0x083D,0x77);
    IMX220MIPI_write_cmos_sensor(0x083E,0x00);
    IMX220MIPI_write_cmos_sensor(0x083F,0x17);
    IMX220MIPI_write_cmos_sensor(0x0842,0x00);
    IMX220MIPI_write_cmos_sensor(0x0843,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0844,0x00);
    IMX220MIPI_write_cmos_sensor(0x0845,0x37);
    IMX220MIPI_write_cmos_sensor(0x0847,0x02);
    //Integration time setting
    IMX220MIPI_write_cmos_sensor(0x0202,0x07);
    IMX220MIPI_write_cmos_sensor(0x0203,0xD8);
    //Gain setting
    IMX220MIPI_write_cmos_sensor(0x0205,0x00);
    IMX220MIPI_write_cmos_sensor(0x020E,0x01);
    IMX220MIPI_write_cmos_sensor(0x020F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0210,0x01);
    IMX220MIPI_write_cmos_sensor(0x0211,0x00);
    IMX220MIPI_write_cmos_sensor(0x0212,0x01);
    IMX220MIPI_write_cmos_sensor(0x0213,0x00);
    IMX220MIPI_write_cmos_sensor(0x0214,0x01);
    IMX220MIPI_write_cmos_sensor(0x0215,0x00);
    //HDR setting
    IMX220MIPI_write_cmos_sensor(0x0230,0x01);
    IMX220MIPI_write_cmos_sensor(0x0231,0xF4);
    IMX220MIPI_write_cmos_sensor(0x0233,0x00);
    IMX220MIPI_write_cmos_sensor(0x0234,0x00);
    IMX220MIPI_write_cmos_sensor(0x0235,0x40);
    IMX220MIPI_write_cmos_sensor(0x0238,0x00);
    IMX220MIPI_write_cmos_sensor(0x0239,0x08);
    IMX220MIPI_write_cmos_sensor(0x023B,0x02);
    IMX220MIPI_write_cmos_sensor(0x023C,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C2,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C3,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C4,0x00);
    IMX220MIPI_write_cmos_sensor(0x9376,0x35);
    IMX220MIPI_write_cmos_sensor(0x9377,0x01);
    IMX220MIPI_write_cmos_sensor(0x9800,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD34,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD35,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD36,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD37,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD38,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD39,0x5D);
    IMX220MIPI_write_cmos_sensor(0xAD3A,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3B,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3C,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD3D,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD3E,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD3F,0x5D);
    //Bypass setting 
    IMX220MIPI_write_cmos_sensor(0xA303,0x00);
    IMX220MIPI_write_cmos_sensor(0xA403,0x00);
    IMX220MIPI_write_cmos_sensor(0xA602,0x00);
    IMX220MIPI_write_cmos_sensor(0xA703,0x01);
    IMX220MIPI_write_cmos_sensor(0xA903,0x01);
    IMX220MIPI_write_cmos_sensor(0xA956,0x00);
    IMX220MIPI_write_cmos_sensor(0xAA03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAB03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE03,0x01);
    IMX220MIPI_write_cmos_sensor(0xB803,0x01);
    IMX220MIPI_write_cmos_sensor(0xB91A,0x01);
    IMX220MIPI_write_cmos_sensor(0xBB03,0x00);
    //Single correct setting
    IMX220MIPI_write_cmos_sensor(0x4003,0x00);
    IMX220MIPI_write_cmos_sensor(0x4004,0x00);
    IMX220MIPI_write_cmos_sensor(0x4005,0x00);
    IMX220MIPI_write_cmos_sensor(0x4006,0x80);
    IMX220MIPI_write_cmos_sensor(0x4007,0x80);
    IMX220MIPI_write_cmos_sensor(0x4008,0x80);
    IMX220MIPI_write_cmos_sensor(0x400C,0x00);
    IMX220MIPI_write_cmos_sensor(0x400D,0x00);
    IMX220MIPI_write_cmos_sensor(0x400E,0x00);
    IMX220MIPI_write_cmos_sensor(0x400F,0x80);
    IMX220MIPI_write_cmos_sensor(0x4010,0x80);
    IMX220MIPI_write_cmos_sensor(0x4011,0x80);
    //Mode etc setting 
    IMX220MIPI_write_cmos_sensor(0x3008,0x01);
    IMX220MIPI_write_cmos_sensor(0x3009,0x71);
    IMX220MIPI_write_cmos_sensor(0x300A,0x51);
    IMX220MIPI_write_cmos_sensor(0x3506,0x00);
    IMX220MIPI_write_cmos_sensor(0x3507,0x01);
    IMX220MIPI_write_cmos_sensor(0x3508,0x00);
    IMX220MIPI_write_cmos_sensor(0x3509,0x01);
    IMX220MIPI_write_cmos_sensor(0xA913,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC13,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC3F,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE8B,0x36);
    IMX220MIPI_write_cmos_sensor(0xB812,0x00);
    //LNR setting
    IMX220MIPI_write_cmos_sensor(0x3516,0x00);
    IMX220MIPI_write_cmos_sensor(0x3517,0x00);
    IMX220MIPI_write_cmos_sensor(0x3518,0x00);
    IMX220MIPI_write_cmos_sensor(0x3519,0x00);
    IMX220MIPI_write_cmos_sensor(0x351A,0x00);
    IMX220MIPI_write_cmos_sensor(0x351B,0x00);
    IMX220MIPI_write_cmos_sensor(0x351C,0x00);
    IMX220MIPI_write_cmos_sensor(0x351D,0x00);
    IMX220MIPI_write_cmos_sensor(0x351E,0x00);
    IMX220MIPI_write_cmos_sensor(0x351F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3520,0x00);
    IMX220MIPI_write_cmos_sensor(0x3521,0x00);
    IMX220MIPI_write_cmos_sensor(0x3522,0x00);
    //continue mode
    //IMX220MIPI_write_cmos_sensor(0x9306,0x00);
    //color pattern
    //IMX220MIPI_write_cmos_sensor(0x0601,0x00);
    mdelay(10);
    IMX220MIPI_write_cmos_sensor(0x0100,0x1 );
                   
    SENSORDB("[IMX220MIPIRaw] Set preview setting  End\n"); 
}

void IMX220MIPI_set_FullSize(void)
{
    // Initial settings for sensor
    //IMX220MIPI_write_cmos_sensor(0x0101,0x00);
    //IMX220MIPI_write_cmos_sensor(0x0220,0x01);
    IMX220MIPI_write_cmos_sensor(0x0100,0x00);// STREAM STop
    //Clock setting
    IMX220MIPI_write_cmos_sensor(0x011E,0x18);
    IMX220MIPI_write_cmos_sensor(0x011F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0301,0x06);
    IMX220MIPI_write_cmos_sensor(0x0303,0x02);
    IMX220MIPI_write_cmos_sensor(0x0304,0x02);
    IMX220MIPI_write_cmos_sensor(0x0305,0x06);
    IMX220MIPI_write_cmos_sensor(0x0306,0x00);
    IMX220MIPI_write_cmos_sensor(0x0307,0x49);
    IMX220MIPI_write_cmos_sensor(0x0309,0x0A);
    IMX220MIPI_write_cmos_sensor(0x030B,0x01);
    IMX220MIPI_write_cmos_sensor(0x030C,0x00);
    IMX220MIPI_write_cmos_sensor(0x030D,0xAE);
    //Size setting
    IMX220MIPI_write_cmos_sensor(0x0340,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0341,0x90);
    IMX220MIPI_write_cmos_sensor(0x0342,0x17);
    IMX220MIPI_write_cmos_sensor(0x0343,0x10);
    IMX220MIPI_write_cmos_sensor(0x0344,0x00);  //x_add_sta
    IMX220MIPI_write_cmos_sensor(0x0345,0x00);
    IMX220MIPI_write_cmos_sensor(0x0346,0x00);  //y_add_sta
    IMX220MIPI_write_cmos_sensor(0x0347,0x00);
    IMX220MIPI_write_cmos_sensor(0x0348,0x14);  //x_add_end
    IMX220MIPI_write_cmos_sensor(0x0349,0x7F);
    IMX220MIPI_write_cmos_sensor(0x034A,0x0F);  //y_add_end
    IMX220MIPI_write_cmos_sensor(0x034B,0x5F);
    IMX220MIPI_write_cmos_sensor(0x034C,0x14);  //x_out_size
    IMX220MIPI_write_cmos_sensor(0x034D,0x80);
    IMX220MIPI_write_cmos_sensor(0x034E,0x0F);  //y_out_size
    IMX220MIPI_write_cmos_sensor(0x034F,0x60);
    IMX220MIPI_write_cmos_sensor(0x0350,0x00);  //dig_crop_s_strt
    IMX220MIPI_write_cmos_sensor(0x0351,0x00);
    IMX220MIPI_write_cmos_sensor(0x0352,0x00);  //dig_crop_y_strt
    IMX220MIPI_write_cmos_sensor(0x0353,0x00);
    IMX220MIPI_write_cmos_sensor(0x0354,0x14);  //dig_crop_x_size
    IMX220MIPI_write_cmos_sensor(0x0355,0x80);
    IMX220MIPI_write_cmos_sensor(0x0356,0x0F);  //dig_crop_y_size
    IMX220MIPI_write_cmos_sensor(0x0357,0x60);
    IMX220MIPI_write_cmos_sensor(0x901D,0x08);
    //Mode setting
    IMX220MIPI_write_cmos_sensor(0x0108,0x03);
    IMX220MIPI_write_cmos_sensor(0x0112,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0113,0x0A);
    IMX220MIPI_write_cmos_sensor(0x0381,0x01);
    IMX220MIPI_write_cmos_sensor(0x0383,0x01);
    IMX220MIPI_write_cmos_sensor(0x0385,0x01);
    IMX220MIPI_write_cmos_sensor(0x0387,0x01);
    IMX220MIPI_write_cmos_sensor(0x0390,0x00);//BINNING 0:disable,1:enable
    IMX220MIPI_write_cmos_sensor(0x0391,0x11);
    IMX220MIPI_write_cmos_sensor(0x0392,0x00);
    IMX220MIPI_write_cmos_sensor(0x1307,0x00);
    IMX220MIPI_write_cmos_sensor(0x9512,0x40);
    IMX220MIPI_write_cmos_sensor(0xAB07,0x01);
    IMX220MIPI_write_cmos_sensor(0xB802,0x01);
    IMX220MIPI_write_cmos_sensor(0xB933,0x00);
    //Optionnal function 
    IMX220MIPI_write_cmos_sensor(0x0700,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C0,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C1,0x02);
    IMX220MIPI_write_cmos_sensor(0x9909,0x01);
    IMX220MIPI_write_cmos_sensor(0x991E,0x01);
    IMX220MIPI_write_cmos_sensor(0xBA03,0x00);
    IMX220MIPI_write_cmos_sensor(0xBA07,0x00);
    //Global timing setting
    IMX220MIPI_write_cmos_sensor(0x0830,0x00);
    IMX220MIPI_write_cmos_sensor(0x0831,0x6F);
    IMX220MIPI_write_cmos_sensor(0x0832,0x00);
    IMX220MIPI_write_cmos_sensor(0x0833,0x2F);
    IMX220MIPI_write_cmos_sensor(0x0834,0x00);
    IMX220MIPI_write_cmos_sensor(0x0835,0x57);
    IMX220MIPI_write_cmos_sensor(0x0836,0x00);
    IMX220MIPI_write_cmos_sensor(0x0837,0x2f);
    IMX220MIPI_write_cmos_sensor(0x0838,0x00);
    IMX220MIPI_write_cmos_sensor(0x0839,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083A,0x00);
    IMX220MIPI_write_cmos_sensor(0x083B,0x2F);
    IMX220MIPI_write_cmos_sensor(0x083C,0x00);
    IMX220MIPI_write_cmos_sensor(0x083D,0xBF);
    IMX220MIPI_write_cmos_sensor(0x083E,0x00);
    IMX220MIPI_write_cmos_sensor(0x083F,0x27);
    IMX220MIPI_write_cmos_sensor(0x0842,0x00);
    IMX220MIPI_write_cmos_sensor(0x0843,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0844,0x00);
    IMX220MIPI_write_cmos_sensor(0x0845,0x37);
    IMX220MIPI_write_cmos_sensor(0x0847,0x02);
    //Integration time setting
    IMX220MIPI_write_cmos_sensor(0x0202,0x0F);
    IMX220MIPI_write_cmos_sensor(0x0203,0x88);
    //Gain setting
    IMX220MIPI_write_cmos_sensor(0x0205,0x00);
    IMX220MIPI_write_cmos_sensor(0x020E,0x01);
    IMX220MIPI_write_cmos_sensor(0x020F,0x00);
    IMX220MIPI_write_cmos_sensor(0x0210,0x01);
    IMX220MIPI_write_cmos_sensor(0x0211,0x00);
    IMX220MIPI_write_cmos_sensor(0x0212,0x01);
    IMX220MIPI_write_cmos_sensor(0x0213,0x00);
    IMX220MIPI_write_cmos_sensor(0x0214,0x01);
    IMX220MIPI_write_cmos_sensor(0x0215,0x00);
    //HDR setting
    IMX220MIPI_write_cmos_sensor(0x0230,0x01);
    IMX220MIPI_write_cmos_sensor(0x0231,0xF4);
    IMX220MIPI_write_cmos_sensor(0x0233,0x00);
    IMX220MIPI_write_cmos_sensor(0x0234,0x00);
    IMX220MIPI_write_cmos_sensor(0x0235,0x40);
    IMX220MIPI_write_cmos_sensor(0x0238,0x00);
    IMX220MIPI_write_cmos_sensor(0x0239,0x08);
    IMX220MIPI_write_cmos_sensor(0x023B,0x02);
    IMX220MIPI_write_cmos_sensor(0x023C,0x01);
    IMX220MIPI_write_cmos_sensor(0x13C2,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C3,0x00);
    IMX220MIPI_write_cmos_sensor(0x13C4,0x00);
    IMX220MIPI_write_cmos_sensor(0x9376,0x35);
    IMX220MIPI_write_cmos_sensor(0x9377,0x01);
    IMX220MIPI_write_cmos_sensor(0x9800,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD34,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD35,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD36,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD37,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD38,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD39,0x5D);
    IMX220MIPI_write_cmos_sensor(0xAD3A,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3B,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD3C,0x14);
    IMX220MIPI_write_cmos_sensor(0xAD3D,0x7F);
    IMX220MIPI_write_cmos_sensor(0xAD3E,0x0F);
    IMX220MIPI_write_cmos_sensor(0xAD3F,0x5D);
    //Bypass setting 
    IMX220MIPI_write_cmos_sensor(0xA303,0x00);
    IMX220MIPI_write_cmos_sensor(0xA403,0x00);
    IMX220MIPI_write_cmos_sensor(0xA602,0x00);
    IMX220MIPI_write_cmos_sensor(0xA703,0x01);
    IMX220MIPI_write_cmos_sensor(0xA903,0x01);
    IMX220MIPI_write_cmos_sensor(0xA956,0x00);
    IMX220MIPI_write_cmos_sensor(0xAA03,0x01);
    IMX220MIPI_write_cmos_sensor(0xAB03,0x01);//0:No Bypass, 1:bypass
    IMX220MIPI_write_cmos_sensor(0xAC03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAD03,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE03,0x01);
    IMX220MIPI_write_cmos_sensor(0xB803,0x01);
    IMX220MIPI_write_cmos_sensor(0xB91A,0x01);
    IMX220MIPI_write_cmos_sensor(0xBB03,0x00);
    //Single correct setting
    IMX220MIPI_write_cmos_sensor(0x4003,0x00);
    IMX220MIPI_write_cmos_sensor(0x4004,0x00);
    IMX220MIPI_write_cmos_sensor(0x4005,0x00);
    IMX220MIPI_write_cmos_sensor(0x4006,0x80);
    IMX220MIPI_write_cmos_sensor(0x4007,0x80);
    IMX220MIPI_write_cmos_sensor(0x4008,0x80);
    IMX220MIPI_write_cmos_sensor(0x400C,0x00);
    IMX220MIPI_write_cmos_sensor(0x400D,0x00);
    IMX220MIPI_write_cmos_sensor(0x400E,0x00);
    IMX220MIPI_write_cmos_sensor(0x400F,0x80);
    IMX220MIPI_write_cmos_sensor(0x4010,0x80);
    IMX220MIPI_write_cmos_sensor(0x4011,0x80);
    //Mode etc setting 
    IMX220MIPI_write_cmos_sensor(0x3008,0x01);
    IMX220MIPI_write_cmos_sensor(0x3009,0x71);
    IMX220MIPI_write_cmos_sensor(0x300A,0x51);
    IMX220MIPI_write_cmos_sensor(0x3506,0x00);
    IMX220MIPI_write_cmos_sensor(0x3507,0x01);
    IMX220MIPI_write_cmos_sensor(0x3508,0x00);
    IMX220MIPI_write_cmos_sensor(0x3509,0x01);
    IMX220MIPI_write_cmos_sensor(0xA913,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC13,0x00);
    IMX220MIPI_write_cmos_sensor(0xAC3F,0x00);
    IMX220MIPI_write_cmos_sensor(0xAE8B,0x36);
    IMX220MIPI_write_cmos_sensor(0xB812,0x00);
    //LNR setting
    IMX220MIPI_write_cmos_sensor(0x3516,0x00);
    IMX220MIPI_write_cmos_sensor(0x3517,0x00);
    IMX220MIPI_write_cmos_sensor(0x3518,0x00);
    IMX220MIPI_write_cmos_sensor(0x3519,0x00);
    IMX220MIPI_write_cmos_sensor(0x351A,0x00);
    IMX220MIPI_write_cmos_sensor(0x351B,0x00);
    IMX220MIPI_write_cmos_sensor(0x351C,0x00);
    IMX220MIPI_write_cmos_sensor(0x351D,0x00);
    IMX220MIPI_write_cmos_sensor(0x351E,0x00);
    IMX220MIPI_write_cmos_sensor(0x351F,0x00);
    IMX220MIPI_write_cmos_sensor(0x3520,0x00);
    IMX220MIPI_write_cmos_sensor(0x3521,0x00);
    IMX220MIPI_write_cmos_sensor(0x3522,0x00);
    //continue mode
    //IMX220MIPI_write_cmos_sensor(0x9306,0x00);
    //color pattern
    //IMX220MIPI_write_cmos_sensor(0x0601,0x01);
    IMX220MIPI_write_cmos_sensor(0x0100,0x1 );
                   
    SENSORDB("[IMX220MIPIRaw] Set 20M End\n"); 
}



/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*   IMX220MIPIOpen
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/

UINT32 IMX220MIPIOpen(void)
{
    int  retry = 0; 
	kal_uint16 sensorid; 
    // check if sensor ID correct
    retry = 3; 
	SENSORDB("IMX220MIPIOpen enter:\n"); 
    do {
       SENSORDB("Read ID in the Open function"); 
	   sensorid=(kal_uint16)((IMX220MIPI_read_cmos_sensor(0x0016)<<8) | IMX220MIPI_read_cmos_sensor(0x0017));  
	   spin_lock(&imx220_drv_lock);    
	   IMX220MIPI_sensor_id =sensorid;  
	   spin_unlock(&imx220_drv_lock);
		if (IMX220MIPI_sensor_id == IMX220MIPI_SENSOR_ID)
		break; 
		SENSORDB("Read Sensor ID Fail = 0x%04x\n", IMX220MIPI_sensor_id); 
		retry--; 
	    }
	while (retry > 0);
    SENSORDB("Read Sensor ID = 0x%04x\n", IMX220MIPI_sensor_id); 
    if (IMX220MIPI_sensor_id != IMX220MIPI_SENSOR_ID)
        return ERROR_SENSOR_CONNECT_FAIL;
    IMX220MIPI_Sensor_Init();
	spin_lock(&imx220_drv_lock);    
	   IMX220MIPI_sensor.capture_mode=KAL_FALSE;
	   spin_unlock(&imx220_drv_lock);
	SENSORDB("IMX220MIPIOpen exit:"); 
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*   IMX220MIPIGetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 IMX220MIPIGetSensorID(UINT32 *sensorID) 
{
    int  retry = 3; 
    // check if sensor ID correct
    SENSORDB("IMX220MIPIGetSensorID\n"); 
    do {		
	  *sensorID =(kal_uint16)((IMX220MIPI_read_cmos_sensor(0x0016)<<8) | IMX220MIPI_read_cmos_sensor(0x0017));   
	  SENSORDB("GetSensorID = 0x%04x\n", *sensorID); 
        if (*sensorID == IMX220MIPI_SENSOR_ID)
            break;
        SENSORDB("Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
		
    } while (retry > 0);

//Debug
//*sensorID = IMX220MIPI_SENSOR_ID;

    if (*sensorID != IMX220MIPI_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*   IMX220MIPI_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of IMX220MIPI to change exposure time.
*
* PARAMETERS
*   shutter : exposured lines
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void IMX220MIPI_SetShutter(kal_uint16 iShutter)
{
//	 SENSORDB("IMX220MIPI_SetShutter:shutter=%d\n",iShutter);
   
    if (iShutter < 1)
        iShutter = 1; 
	else if(iShutter > 0xffff)
		iShutter = 0xffff;
    IMX220MIPI_write_shutter(iShutter);
}   /*  IMX220MIPI_SetShutter   */



/*************************************************************************
* FUNCTION
*   IMX220MIPI_read_shutter
*
* DESCRIPTION
*   This function to  Get exposure time.
*
* PARAMETERS
*   None
*
* RETURNS
*   shutter : exposured lines
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT16 IMX220MIPI_read_shutter(void)
{
    return (UINT16)( (IMX220MIPI_read_cmos_sensor(0x0202)<<8) | IMX220MIPI_read_cmos_sensor(0x0203) );
}

/*************************************************************************
* FUNCTION
*   IMX220MIPI_night_mode
*
* DESCRIPTION
*   This function night mode of IMX220MIPI.
*
* PARAMETERS
*   none
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/

///is not use in raw sensor
void IMX220MIPI_NightMode(kal_bool bEnable)
{
#if 0
    /************************************************************************/
    /*                      Auto Mode: 30fps                                                                                          */
    /*                      Night Mode:15fps                                                                                          */
    /************************************************************************/
    if(bEnable)
    {
        if(OV5642_MPEG4_encode_mode==KAL_TRUE)
        {
            OV5642_MAX_EXPOSURE_LINES = (kal_uint16)((OV5642_sensor_pclk/15)/(OV5642_PV_PERIOD_PIXEL_NUMS+OV5642_PV_dummy_pixels));
            OV5642_write_cmos_sensor(0x350C, (OV5642_MAX_EXPOSURE_LINES >> 8) & 0xFF);
            OV5642_write_cmos_sensor(0x350D, OV5642_MAX_EXPOSURE_LINES & 0xFF);
            OV5642_CURRENT_FRAME_LINES = OV5642_MAX_EXPOSURE_LINES;
            OV5642_MAX_EXPOSURE_LINES = OV5642_CURRENT_FRAME_LINES - OV5642_SHUTTER_LINES_GAP;
        }
    }
    else// Fix video framerate 30 fps
    {
        if(OV5642_MPEG4_encode_mode==KAL_TRUE)
        {
            OV5642_MAX_EXPOSURE_LINES = (kal_uint16)((OV5642_sensor_pclk/30)/(OV5642_PV_PERIOD_PIXEL_NUMS+OV5642_PV_dummy_pixels));
            if(OV5642_pv_exposure_lines < (OV5642_MAX_EXPOSURE_LINES - OV5642_SHUTTER_LINES_GAP)) // for avoid the shutter > frame_lines,move the frame lines setting to shutter function
            {
                OV5642_write_cmos_sensor(0x350C, (OV5642_MAX_EXPOSURE_LINES >> 8) & 0xFF);
                OV5642_write_cmos_sensor(0x350D, OV5642_MAX_EXPOSURE_LINES & 0xFF);
                OV5642_CURRENT_FRAME_LINES = OV5642_MAX_EXPOSURE_LINES;
            }
            OV5642_MAX_EXPOSURE_LINES = OV5642_MAX_EXPOSURE_LINES - OV5642_SHUTTER_LINES_GAP;
        }
    }
#endif	
}/*	IMX220MIPI_NightMode */



/*************************************************************************
* FUNCTION
*   IMX220MIPIClose
*
* DESCRIPTION
*   This function is to turn off sensor module power.
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 IMX220MIPIClose(void)
{
    return ERROR_NONE;
}	/* IMX220MIPIClose() */

void IMX220MIPISetFlipMirror(kal_int32 imgMirror)
{
    kal_uint8  iTemp; 
	
	iTemp = IMX220MIPI_read_cmos_sensor(0x0101);
	iTemp&= ~0x03; //Clear the mirror and flip bits.
	switch (imgMirror)
	{
		case IMAGE_NORMAL:
			IMX220MIPI_write_cmos_sensor(0x0101, iTemp);	//Set normal
			break;
		case IMAGE_V_MIRROR:
			IMX220MIPI_write_cmos_sensor(0x0101, iTemp | 0x02); //Set flip
			break;
		case IMAGE_H_MIRROR:
			IMX220MIPI_write_cmos_sensor(0x0101, iTemp | 0x01); //Set mirror
			break;
		case IMAGE_HV_MIRROR:
			IMX220MIPI_write_cmos_sensor(0x0101, iTemp | 0x03); //Set mirror and flip
			break;
	}
}


/*************************************************************************
* FUNCTION
*   IMX220MIPIPreview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 IMX220MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iStartX = 0, iStartY = 0;
    if(sensor_config_data->SensorOperationMode==MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {   
        SENSORDB("IMX220MIPIVideo enter:\n");
    	spin_lock(&imx220_drv_lock);    
        IMX220MIPI_MPEG4_encode_mode = KAL_TRUE;  
		IMX220MIPI_sensor.video_mode=KAL_TRUE;
        IMX220MIPI_sensor.highspeedvideo_mode=KAL_FALSE;
        IMX220MIPI_sensor.slimvideo_mode=KAL_FALSE;
		IMX220MIPI_sensor.pv_mode=KAL_FALSE;
		IMX220MIPI_sensor.capture_mode=KAL_FALSE;
		spin_unlock(&imx220_drv_lock);
		//VideoSizeSetting_4K2K_30fps();
		VideoSizeSetting_4K2K_30fps();
		iStartX += IMX220MIPI_IMAGE_SENSOR_VIDEO_STARTX;
		iStartY += IMX220MIPI_IMAGE_SENSOR_VIDEO_STARTY;
		spin_lock(&imx220_drv_lock);	
		IMX220MIPI_sensor.cp_dummy_pixels = 0;
		IMX220MIPI_sensor.cp_dummy_lines = 0;
		IMX220MIPI_sensor.pv_dummy_pixels = 0;
		IMX220MIPI_sensor.pv_dummy_lines = 0;
		IMX220MIPI_sensor.video_dummy_pixels = 0;
		IMX220MIPI_sensor.video_dummy_lines = 0;
        IMX220MIPI_sensor.video1_dummy_pixels = 0;
        IMX220MIPI_sensor.video1_dummy_lines = 0;
        IMX220MIPI_sensor.video2_dummy_pixels = 0;
        IMX220MIPI_sensor.video2_dummy_lines = 0;
		IMX220MIPI_sensor.video_line_length = IMX220MIPI_VIDEO_LINE_LENGTH_PIXELS+IMX220MIPI_sensor.video_dummy_pixels; 
		IMX220MIPI_sensor.video_frame_length = IMX220MIPI_VIDEO_FRAME_LENGTH_LINES+IMX220MIPI_sensor.video_dummy_lines;
		spin_unlock(&imx220_drv_lock);
		
		IMX220MIPI_SetDummy(IMX220MIPI_sensor.video_dummy_pixels,IMX220MIPI_sensor.video_dummy_lines);
		IMX220MIPI_SetShutter(IMX220MIPI_sensor.video_shutter);
		memcpy(&IMX220MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
		image_window->GrabStartX= iStartX;
		image_window->GrabStartY= iStartY;
	//	image_window->ExposureWindowWidth= IMX220MIPI_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
	//	image_window->ExposureWindowHeight= IMX220MIPI_IMAGE_SENSOR_PV_HEIGHT - 2*iStartY;

    }
    else
    {
		SENSORDB("IMX220MIPIPreview enter:\n");
    	spin_lock(&imx220_drv_lock);    
        IMX220MIPI_MPEG4_encode_mode = KAL_FALSE;
		IMX220MIPI_sensor.video_mode=KAL_FALSE;
        IMX220MIPI_sensor.highspeedvideo_mode=KAL_FALSE;
        IMX220MIPI_sensor.slimvideo_mode=KAL_FALSE;
		IMX220MIPI_sensor.pv_mode=KAL_TRUE;
		IMX220MIPI_sensor.capture_mode=KAL_FALSE;
		spin_unlock(&imx220_drv_lock);
        PreviewSizeSetting();		      
		//IMX220MIPISetFlipMirror(IMAGE_HV_MIRROR);
		iStartX += IMX220MIPI_IMAGE_SENSOR_PV_STARTX;
		iStartY += IMX220MIPI_IMAGE_SENSOR_PV_STARTY;
		spin_lock(&imx220_drv_lock);	
		IMX220MIPI_sensor.cp_dummy_pixels = 0;
		IMX220MIPI_sensor.cp_dummy_lines = 0;
		IMX220MIPI_sensor.pv_dummy_pixels = 0;
		IMX220MIPI_sensor.pv_dummy_lines = 0;
		IMX220MIPI_sensor.video_dummy_pixels = 0;
		IMX220MIPI_sensor.video_dummy_lines = 0;
        IMX220MIPI_sensor.video1_dummy_pixels = 0;
        IMX220MIPI_sensor.video1_dummy_lines = 0;
        IMX220MIPI_sensor.video2_dummy_pixels = 0;
        IMX220MIPI_sensor.video2_dummy_lines = 0;
		IMX220MIPI_sensor.pv_line_length = IMX220MIPI_PV_LINE_LENGTH_PIXELS+IMX220MIPI_sensor.pv_dummy_pixels; 
		IMX220MIPI_sensor.pv_frame_length = IMX220MIPI_PV_FRAME_LENGTH_LINES+IMX220MIPI_sensor.pv_dummy_lines;
		spin_unlock(&imx220_drv_lock);
		
		IMX220MIPI_SetDummy(IMX220MIPI_sensor.pv_dummy_pixels,IMX220MIPI_sensor.pv_dummy_lines);
		//IMX220MIPI_SetShutter(IMX220MIPI_sensor.pv_shutter);
		memcpy(&IMX220MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
		image_window->GrabStartX= iStartX;
		image_window->GrabStartY= iStartY;
		image_window->ExposureWindowWidth= IMX220MIPI_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
		image_window->ExposureWindowHeight= IMX220MIPI_IMAGE_SENSOR_PV_HEIGHT - 2*iStartY;
		SENSORDB("IMX220MIPIPreview exit:\n");

    }
	

	return ERROR_NONE;
}	/* IMX220MIPIPreview() */

UINT32 IMX220MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    // kal_uint32 shutter=IMX220MIPI_sensor.pv_shutter;
    //kal_uint16 iStartX = 0, iStartY = 0;
    if(IMX220MIPI_sensor.capture_mode==KAL_TRUE)
        return ERROR_NONE;
	SENSORDB("IMX220MIPICapture enter:\n"); 
	spin_lock(&imx220_drv_lock);	
	IMX220MIPI_sensor.video_mode=KAL_FALSE;
    IMX220MIPI_sensor.highspeedvideo_mode=KAL_FALSE;
    IMX220MIPI_sensor.slimvideo_mode=KAL_FALSE;
	IMX220MIPI_sensor.pv_mode=KAL_FALSE;
	IMX220MIPI_sensor.capture_mode=KAL_TRUE;
    IMX220MIPI_MPEG4_encode_mode = KAL_FALSE; 
    IMX220MIPI_Auto_Flicker_mode = KAL_FALSE;   
	spin_unlock(&imx220_drv_lock);
    spin_lock(&imx220_drv_lock);    
    IMX220MIPI_sensor.cp_dummy_pixels= 0;
    IMX220MIPI_sensor.cp_dummy_lines = 0;   
    spin_unlock(&imx220_drv_lock);
    IMX220MIPI_set_FullSize();
     
   // IMX220MIPISetFlipMirror(sensor_config_data->SensorImageMirror); 
   //IMX220MIPISetFlipMirror(IMAGE_HV_MIRROR);
	
    spin_lock(&imx220_drv_lock);    
    IMX220MIPI_sensor.cp_line_length=IMX220MIPI_FULL_LINE_LENGTH_PIXELS+IMX220MIPI_sensor.cp_dummy_pixels;
    IMX220MIPI_sensor.cp_frame_length=IMX220MIPI_FULL_FRAME_LENGTH_LINES+IMX220MIPI_sensor.cp_dummy_lines;
    spin_unlock(&imx220_drv_lock);
	//iStartX = IMX220MIPI_IMAGE_SENSOR_CAP_STARTX;
	//iStartY = IMX220MIPI_IMAGE_SENSOR_CAP_STARTY;
	//image_window->ExposureWindowWidth=IMX220MIPI_IMAGE_SENSOR_FULL_WIDTH -2*iStartX;
	//image_window->ExposureWindowHeight=IMX220MIPI_IMAGE_SENSOR_FULL_HEIGHT-2*iStartY;  
	spin_lock(&imx220_drv_lock);	
    memcpy(&IMX220MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	spin_unlock(&imx220_drv_lock);
	SENSORDB("IMX220MIPICapture exit:\n"); 


    return ERROR_NONE;
}	/* IMX220MIPICapture() */

UINT32 IMX220MIPIVideo1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iStartX = 0, iStartY = 0;

    SENSORDB("IMX220MIPIVideo1 enter:\n");
    spin_lock(&imx220_drv_lock);    
    IMX220MIPI_MPEG4_encode_mode = KAL_TRUE;  
    IMX220MIPI_sensor.video_mode=KAL_FALSE;
    IMX220MIPI_sensor.highspeedvideo_mode=KAL_TRUE;
    IMX220MIPI_sensor.slimvideo_mode=KAL_FALSE;
    IMX220MIPI_sensor.pv_mode=KAL_FALSE;
    IMX220MIPI_sensor.capture_mode=KAL_FALSE;
    spin_unlock(&imx220_drv_lock);
    VideoHDSetting60fps();

    iStartX += IMX220MIPI_IMAGE_SENSOR_VIDEO1_STARTX;
    iStartY += IMX220MIPI_IMAGE_SENSOR_VIDEO1_STARTY;
    spin_lock(&imx220_drv_lock);    
    IMX220MIPI_sensor.cp_dummy_pixels = 0;
    IMX220MIPI_sensor.cp_dummy_lines = 0;
    IMX220MIPI_sensor.pv_dummy_pixels = 0;
    IMX220MIPI_sensor.pv_dummy_lines = 0;
    IMX220MIPI_sensor.video_dummy_pixels = 0;
    IMX220MIPI_sensor.video_dummy_lines = 0;
    IMX220MIPI_sensor.video1_dummy_pixels = 0;
    IMX220MIPI_sensor.video1_dummy_lines = 0;
    IMX220MIPI_sensor.video2_dummy_pixels = 0;
    IMX220MIPI_sensor.video2_dummy_lines = 0;
    IMX220MIPI_sensor.video1_line_length = IMX220MIPI_VIDEO1_LINE_LENGTH_PIXELS+IMX220MIPI_sensor.video1_dummy_pixels; 
    IMX220MIPI_sensor.video1_frame_length = IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES+IMX220MIPI_sensor.video1_dummy_lines;
    spin_unlock(&imx220_drv_lock);

    IMX220MIPI_SetDummy(IMX220MIPI_sensor.video1_dummy_pixels,IMX220MIPI_sensor.video1_dummy_lines);   
    memcpy(&IMX220MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    image_window->GrabStartX= iStartX;
    image_window->GrabStartY= iStartY;
    SENSORDB("IMX220MIPIVideo exit:\n");
    return ERROR_NONE;
}  

UINT32 IMX220MIPIVideo2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iStartX = 0, iStartY = 0;

    SENSORDB("IMX220MIPIVideo2 enter:\n");
    spin_lock(&imx220_drv_lock);    
    IMX220MIPI_MPEG4_encode_mode = KAL_TRUE;  
    IMX220MIPI_sensor.video_mode=KAL_FALSE;
    IMX220MIPI_sensor.highspeedvideo_mode=KAL_FALSE;
    IMX220MIPI_sensor.slimvideo_mode=KAL_TRUE;
    IMX220MIPI_sensor.pv_mode=KAL_FALSE;
    IMX220MIPI_sensor.capture_mode=KAL_FALSE;
    spin_unlock(&imx220_drv_lock);
    VideoHDSetting30fps();

    iStartX += IMX220MIPI_IMAGE_SENSOR_VIDEO2_STARTX;
    iStartY += IMX220MIPI_IMAGE_SENSOR_VIDEO2_STARTY;
    spin_lock(&imx220_drv_lock);    
    IMX220MIPI_sensor.cp_dummy_pixels = 0;
    IMX220MIPI_sensor.cp_dummy_lines = 0;
    IMX220MIPI_sensor.pv_dummy_pixels = 0;
    IMX220MIPI_sensor.pv_dummy_lines = 0;
    IMX220MIPI_sensor.video_dummy_pixels = 0;
    IMX220MIPI_sensor.video_dummy_lines = 0;
    IMX220MIPI_sensor.video1_dummy_pixels = 0;
    IMX220MIPI_sensor.video1_dummy_lines = 0;
    IMX220MIPI_sensor.video2_dummy_pixels = 0;
    IMX220MIPI_sensor.video2_dummy_lines = 0;
    IMX220MIPI_sensor.video2_line_length = IMX220MIPI_VIDEO2_LINE_LENGTH_PIXELS+IMX220MIPI_sensor.video2_dummy_pixels; 
    IMX220MIPI_sensor.video2_frame_length = IMX220MIPI_VIDEO2_FRAME_LENGTH_LINES+IMX220MIPI_sensor.video2_dummy_lines;
    spin_unlock(&imx220_drv_lock);

    IMX220MIPI_SetDummy(IMX220MIPI_sensor.video2_dummy_pixels,IMX220MIPI_sensor.video2_dummy_lines);
    memcpy(&IMX220MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    image_window->GrabStartX= iStartX;
    image_window->GrabStartY= iStartY;
    SENSORDB("IMX220MIPIVideo exit:\n");
    return ERROR_NONE;
}   
UINT32 IMX220MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{

    pSensorResolution->SensorPreviewWidth	        = IMX220MIPI_REAL_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight	        = IMX220MIPI_REAL_PV_HEIGHT;
    pSensorResolution->SensorFullWidth		        = IMX220MIPI_REAL_CAP_WIDTH;
    pSensorResolution->SensorFullHeight		        = IMX220MIPI_REAL_CAP_HEIGHT;
    pSensorResolution->SensorVideoWidth             = IMX220MIPI_REAL_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight            = IMX220MIPI_REAL_VIDEO_HEIGHT;
    pSensorResolution->SensorHighSpeedVideoWidth    = IMX220MIPI_REAL_VIDEO1_WIDTH;
    pSensorResolution->SensorHighSpeedVideoHeight   = IMX220MIPI_REAL_VIDEO1_HEIGHT;
    pSensorResolution->SensorSlimVideoWidth         = IMX220MIPI_REAL_VIDEO2_WIDTH;
    pSensorResolution->SensorSlimVideoHeight        = IMX220MIPI_REAL_VIDEO2_HEIGHT;
    SENSORDB("IMX220MIPIGetResolution ");    

    return ERROR_NONE;
}   /* IMX220MIPIGetResolution() */

UINT32 IMX220MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch(ScenarioId){
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            pSensorInfo->SensorPreviewResolutionX=IMX220MIPI_REAL_CAP_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=IMX220MIPI_REAL_CAP_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate=24;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pSensorInfo->SensorPreviewResolutionX=IMX220MIPI_REAL_VIDEO_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=IMX220MIPI_REAL_VIDEO_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate=30;
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            pSensorInfo->SensorPreviewResolutionX=IMX220MIPI_REAL_VIDEO1_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=IMX220MIPI_REAL_VIDEO1_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate=60;
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            pSensorInfo->SensorPreviewResolutionX=IMX220MIPI_REAL_VIDEO2_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=IMX220MIPI_REAL_VIDEO2_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate=30;
            break;
		default:
        pSensorInfo->SensorPreviewResolutionX=IMX220MIPI_REAL_PV_WIDTH;
        pSensorInfo->SensorPreviewResolutionY=IMX220MIPI_REAL_PV_HEIGHT;
				pSensorInfo->SensorCameraPreviewFrameRate=24;
			break;
	}

    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=12;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=5;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_B;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW; /*??? */
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;
    pSensorInfo->CaptureDelayFrame = 2; 
    pSensorInfo->PreviewDelayFrame = 2; 
    pSensorInfo->VideoDelayFrame = 5; 
    pSensorInfo->HighSpeedVideoDelayFrame = 5;
    pSensorInfo->SlimVideoDelayFrame = 5;
    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;      
    pSensorInfo->AEShutDelayFrame = 0;		    /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 0;     /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;	
    pSensorInfo->IHDR_Support = 0;
    pSensorInfo->IHDR_LE_FirstLine = 1;
    pSensorInfo->SensorModeNum = 5;
    pSensorInfo->MIPIsensorType = MIPI_OPHY_NCSI2;
    pSensorInfo->SettleDelayMode = MIPI_SETTLEDEALY_AUTO;
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
     //   case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
            //pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = IMX220MIPI_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = IMX220MIPI_IMAGE_SENSOR_PV_STARTY;           		
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	     pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 85; 
	     pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
		
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			   pSensorInfo->SensorClockFreq=24;
			   pSensorInfo->SensorClockDividCount= 5;
			   pSensorInfo->SensorClockRisingCount= 0;
			   pSensorInfo->SensorClockFallingCount= 2;
			   pSensorInfo->SensorPixelClockCount= 3;
			   pSensorInfo->SensorDataLatchCount= 2;
			   pSensorInfo->SensorGrabStartX = IMX220MIPI_IMAGE_SENSOR_VIDEO_STARTX; 
			   pSensorInfo->SensorGrabStartY = IMX220MIPI_IMAGE_SENSOR_VIDEO_STARTY;				   
			   pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;		   
			   pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 85; 
			pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			   pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
			   pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
			   pSensorInfo->SensorPacketECCOrder = 1;

			break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
       // case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = IMX220MIPI_IMAGE_SENSOR_CAP_STARTX;	//2*IMX220MIPI_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = IMX220MIPI_IMAGE_SENSOR_CAP_STARTY;	//2*IMX220MIPI_IMAGE_SENSOR_PV_STARTY;          			
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;         
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 85; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount= 5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = IMX220MIPI_IMAGE_SENSOR_VIDEO1_STARTX ;
            pSensorInfo->SensorGrabStartY = IMX220MIPI_IMAGE_SENSOR_VIDEO1_STARTX ;               
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;         
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 85; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->SensorWidthSampling = 0; 
            pSensorInfo->SensorHightSampling = 0; 
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount= 5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = IMX220MIPI_IMAGE_SENSOR_VIDEO2_STARTX ;
            pSensorInfo->SensorGrabStartY = IMX220MIPI_IMAGE_SENSOR_VIDEO2_STARTX ;                   
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 85; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        default:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = 1; 
            pSensorInfo->SensorGrabStartY = 1;             
            break;
    }
	spin_lock(&imx220_drv_lock);	

    IMX220MIPIPixelClockDivider=pSensorInfo->SensorPixelClockCount;
	
	spin_unlock(&imx220_drv_lock);
    memcpy(pSensorConfigData, &IMX220MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* IMX220MIPIGetInfo() */


UINT32 IMX220MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
		spin_lock(&imx220_drv_lock);	
		IMX220CurrentScenarioId = ScenarioId;
		spin_unlock(&imx220_drv_lock);
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            IMX220MIPIPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            IMX220MIPICapture(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            IMX220MIPIVideo1(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            IMX220MIPIVideo2(pImageWindow, pSensorConfigData);
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
} /* IMX220MIPIControl() */

UINT32 IMX220MIPISetVideoMode(UINT16 u2FrameRate)
{
      kal_uint16 IMX220MIPI_Video_Max_Expourse_Time = 0;
      kal_uint32 u32FrameRate;
	  SENSORDB("IMX220MIPISetVideoMode  u2FrameRate = %d\n", u2FrameRate);
      if(u2FrameRate==0)
	{
		SENSORDB("Disable Video Mode or dynimac fps\n");
		return ERROR_NONE;
	}
		spin_lock(&imx220_drv_lock);
		IMX220MIPI_sensor.fix_video_fps = KAL_TRUE;
		spin_unlock(&imx220_drv_lock);
        u32FrameRate = u2FrameRate/10;
		SENSORDB("[IMX220MIPI][Enter Fix_fps func] IMX220MIPI_Fix_Video_Frame_Rate = %d\n", u2FrameRate);
    if(IMX220MIPI_sensor.highspeedvideo_mode == KAL_TRUE)
    {
        IMX220MIPI_Video_Max_Expourse_Time = (kal_uint16)((IMX220MIPI_sensor.video1_pclk/u32FrameRate)/IMX220MIPI_sensor.video1_line_length);
        
        if (IMX220MIPI_Video_Max_Expourse_Time > IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES/*IMX220MIPI_sensor.pv_frame_length*/) 
        {
            spin_lock(&imx220_drv_lock);    
            IMX220MIPI_sensor.video1_frame_length = IMX220MIPI_Video_Max_Expourse_Time;
            IMX220MIPI_sensor.video1_dummy_lines = IMX220MIPI_sensor.video1_frame_length-IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES;
            spin_unlock(&imx220_drv_lock);
            SENSORDB("[IMX220MIPI]%s():HS frame_length=%d,dummy_lines=%d\n",__FUNCTION__,IMX220MIPI_sensor.video1_frame_length,IMX220MIPI_sensor.video1_dummy_lines);
            IMX220MIPI_SetDummy(IMX220MIPI_sensor.video1_dummy_pixels,IMX220MIPI_sensor.video1_dummy_lines);
        }
    }
    else
    {
		IMX220MIPI_Video_Max_Expourse_Time = (kal_uint16)((IMX220MIPI_sensor.video_pclk/u32FrameRate)/IMX220MIPI_sensor.video_line_length);
		
		if (IMX220MIPI_Video_Max_Expourse_Time > IMX220MIPI_VIDEO_FRAME_LENGTH_LINES/*IMX220MIPI_sensor.pv_frame_length*/) 
			{
				spin_lock(&imx220_drv_lock);    
				IMX220MIPI_sensor.video_frame_length = IMX220MIPI_Video_Max_Expourse_Time;
				IMX220MIPI_sensor.video_dummy_lines = IMX220MIPI_sensor.video_frame_length-IMX220MIPI_VIDEO_FRAME_LENGTH_LINES;
				spin_unlock(&imx220_drv_lock);
				SENSORDB("[IMX220MIPI]%s():frame_length=%d,dummy_lines=%d\n",__FUNCTION__,IMX220MIPI_sensor.video_frame_length,IMX220MIPI_sensor.video_dummy_lines);
				IMX220MIPI_SetDummy(IMX220MIPI_sensor.video_dummy_pixels,IMX220MIPI_sensor.video_dummy_lines);
			}
    }
	spin_lock(&imx220_drv_lock);    
    IMX220MIPI_MPEG4_encode_mode = KAL_TRUE; 
	spin_unlock(&imx220_drv_lock);
	
    return ERROR_NONE;
}

UINT32 IMX220MIPISetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	kal_uint32 pv_max_frame_rate_lines=0;
return 0;
if(IMX220MIPI_sensor.pv_mode==TRUE)
	pv_max_frame_rate_lines=IMX220MIPI_PV_FRAME_LENGTH_LINES;
else
    pv_max_frame_rate_lines=IMX220MIPI_VIDEO_FRAME_LENGTH_LINES	;
//kal_uint32 pv_max_frame_rate_lines = IMX220MIPI_MAX_EXPOSURE_LINES;

    SENSORDB("[IMX220MIPISetAutoFlickerMode] frame rate(10base) = %d %d\n", bEnable, u2FrameRate);
    if(bEnable) {   // enable auto flicker   
    	spin_lock(&imx220_drv_lock);    
        IMX220MIPI_Auto_Flicker_mode = KAL_TRUE; 
		spin_unlock(&imx220_drv_lock);
        if(IMX220MIPI_MPEG4_encode_mode == KAL_TRUE) {    // in the video mode, reset the frame rate
            pv_max_frame_rate_lines = IMX220MIPI_MAX_EXPOSURE_LINES + (IMX220MIPI_MAX_EXPOSURE_LINES>>7);            
            IMX220MIPI_write_cmos_sensor(0x0104, 1);        
            IMX220MIPI_write_cmos_sensor(0x0340, (pv_max_frame_rate_lines >>8) & 0xFF);
            IMX220MIPI_write_cmos_sensor(0x0341, pv_max_frame_rate_lines & 0xFF);	
            IMX220MIPI_write_cmos_sensor(0x0104, 0);        	
        }
    } else {
    	spin_lock(&imx220_drv_lock);    
        IMX220MIPI_Auto_Flicker_mode = KAL_FALSE; 
		spin_unlock(&imx220_drv_lock);
        if(IMX220MIPI_MPEG4_encode_mode == KAL_TRUE) {    // in the video mode, restore the frame rate
            IMX220MIPI_write_cmos_sensor(0x0104, 1);        
            IMX220MIPI_write_cmos_sensor(0x0340, (IMX220MIPI_MAX_EXPOSURE_LINES >>8) & 0xFF);
            IMX220MIPI_write_cmos_sensor(0x0341, IMX220MIPI_MAX_EXPOSURE_LINES & 0xFF);	
            IMX220MIPI_write_cmos_sensor(0x0104, 0);        	
        }
        SENSORDB("Disable Auto flicker\n");    
    }
    return ERROR_NONE;
}

UINT32 IMX220MIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
		
	SENSORDB("IMX220MIPISetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk =IMX220MIPI_sensor.pv_pclk;
			lineLength = IMX220MIPI_PV_LINE_LENGTH_PIXELS;
            frameHeight = ((pclk)/lineLength*10)/frameRate;
			dummyLine = frameHeight - IMX220MIPI_PV_FRAME_LENGTH_LINES;
			if(dummyLine<0)
				dummyLine = 0;
			IMX220MIPI_SetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pclk = IMX220MIPI_sensor.video_pclk;
			lineLength = IMX220MIPI_VIDEO_LINE_LENGTH_PIXELS;
            frameHeight = ((pclk)/lineLength*10)/frameRate;
			dummyLine = frameHeight - IMX220MIPI_VIDEO_FRAME_LENGTH_LINES;
			if(dummyLine<0)
                dummyLine = 0;
            IMX220MIPI_SetDummy(0, dummyLine);          
			break;			
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            pclk = IMX220MIPI_sensor.cp_pclk;
			lineLength = IMX220MIPI_FULL_LINE_LENGTH_PIXELS;
            frameHeight = ((pclk)/lineLength*10)/frameRate;
			dummyLine = frameHeight - IMX220MIPI_FULL_FRAME_LENGTH_LINES;
			if(dummyLine<0)
				dummyLine = 0;
			IMX220MIPI_SetDummy(0, dummyLine);			
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            pclk = IMX220MIPI_sensor.video1_pclk;
            lineLength = IMX220MIPI_VIDEO1_LINE_LENGTH_PIXELS;
            frameHeight = ((pclk)/lineLength*10)/frameRate;
            dummyLine = frameHeight - IMX220MIPI_VIDEO1_FRAME_LENGTH_LINES;
            if(dummyLine<0)
                dummyLine = 0;
            IMX220MIPI_SetDummy(0, dummyLine);          
            break;  
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            pclk = IMX220MIPI_sensor.video2_pclk;
            lineLength = IMX220MIPI_VIDEO2_LINE_LENGTH_PIXELS;
            frameHeight = ((pclk)/lineLength*10)/frameRate;
            dummyLine = frameHeight - IMX220MIPI_VIDEO2_FRAME_LENGTH_LINES;
            if(dummyLine<0)
                dummyLine = 0;
            IMX220MIPI_SetDummy(0, dummyLine);          
			break;		
		default:
			break;
	}	
	return ERROR_NONE;
}



UINT32 IMX220MIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*pframeRate = 240;
			 break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            *pframeRate = 120;
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            *pframeRate = 600;
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            *pframeRate = 300;
            break;
		default:
			break;
	}

	return ERROR_NONE;
}




UINT32 IMX220MIPISetTestPatternMode(kal_bool bEnable)  //daikan
{
    SENSORDB("[IMX220MIPISetTestPatternMode] Test pattern enable:%d\n", bEnable);
    
    if(bEnable) {   // enable color bar   
        IMX220MIPI_write_cmos_sensor(0x30D8, 0x10);  // color bar test pattern
        IMX220MIPI_write_cmos_sensor(0x0600, 0x00);  // color bar test pattern
        IMX220MIPI_write_cmos_sensor(0x0601, 0x02);  // color bar test pattern 
    } else {
        IMX220MIPI_write_cmos_sensor(0x30D8, 0x00);  // disable color bar test pattern
    }
    return ERROR_NONE;
}

UINT32 IMX220MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                                                                UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;
    SENSOR_WINSIZE_INFO_STRUCT *pwininfo;

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
        		switch(IMX220CurrentScenarioId)
        		{
        		    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
 		            *pFeatureReturnPara16++=IMX220MIPI_sensor.cp_line_length;  
 		            *pFeatureReturnPara16=IMX220MIPI_sensor.cp_frame_length;
		            SENSORDB("Sensor period:%d %d\n",IMX220MIPI_sensor.cp_line_length, IMX220MIPI_sensor.cp_frame_length); 
		            *pFeatureParaLen=4;        				
        				break;
        			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
                    *pFeatureReturnPara16++=IMX220MIPI_sensor.video_line_length;  
                    *pFeatureReturnPara16=IMX220MIPI_sensor.video_frame_length;
                     SENSORDB("Sensor period:%d %d\n", IMX220MIPI_sensor.video_line_length, IMX220MIPI_sensor.video_frame_length); 
                     *pFeatureParaLen=4;
                    break;
                case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
                    *pFeatureReturnPara16++=IMX220MIPI_sensor.video1_line_length;  
                    *pFeatureReturnPara16=IMX220MIPI_sensor.video1_frame_length;
                     SENSORDB("Sensor period:%d %d\n", IMX220MIPI_sensor.video1_line_length, IMX220MIPI_sensor.video1_frame_length); 
                     *pFeatureParaLen=4;
                    break;
                case MSDK_SCENARIO_ID_SLIM_VIDEO:
					*pFeatureReturnPara16++=IMX220MIPI_sensor.video_line_length;  
					*pFeatureReturnPara16=IMX220MIPI_sensor.video_frame_length;
					 SENSORDB("Sensor period:%d %d\n", IMX220MIPI_sensor.video_line_length, IMX220MIPI_sensor.video_frame_length); 
					 *pFeatureParaLen=4;
						break;
        			default:	
					*pFeatureReturnPara16++=IMX220MIPI_sensor.pv_line_length;  
					*pFeatureReturnPara16=IMX220MIPI_sensor.pv_frame_length;
		            SENSORDB("Sensor period:%d %d\n", IMX220MIPI_sensor.pv_line_length, IMX220MIPI_sensor.pv_frame_length); 
		            *pFeatureParaLen=4;
	            break;
          	}
          	break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        		switch(IMX220CurrentScenarioId)
        		{
        			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                        if(IMX220MIPI_CurretFPS == 240)
                            *pFeatureReturnPara32 = IMX220MIPI_sensor.cp_pclk_2nd; 
                        else
		            *pFeatureReturnPara32 = IMX220MIPI_sensor.cp_pclk; 
		            *pFeatureParaLen=4;		         	
		         		break;
					case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
                        *pFeatureReturnPara32 = IMX220MIPI_sensor.video_pclk;
                        *pFeatureParaLen=4;
                        break;
                    case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
                        *pFeatureReturnPara32 = IMX220MIPI_sensor.video1_pclk;
                        *pFeatureParaLen=4;
                        break;
                    case MSDK_SCENARIO_ID_SLIM_VIDEO:
						*pFeatureReturnPara32 = IMX220MIPI_sensor.video_pclk;
						*pFeatureParaLen=4;
						break;
		         		default:
		            *pFeatureReturnPara32 = IMX220MIPI_sensor.pv_pclk;
		            *pFeatureParaLen=4;
		            break;
		         }
		         break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            IMX220MIPI_SetShutter(*pFeatureData16);
            //SENSORDB("shutter&gain test by hhl:IMX220MIPI_SetShutter in feature ctrl\n"); 
            break;
		case SENSOR_FEATURE_SET_SENSOR_SYNC:
            //SENSORDB("hhl'test the function of the sync cased\n"); 
			break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            IMX220MIPI_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
           IMX220MIPI_SetGain((UINT16) *pFeatureData16);
            
            //SENSORDB("shutter&gain test by hhl:IMX220MIPI_SetGain in feature ctrl\n"); 
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			spin_lock(&imx220_drv_lock);    
            IMX220MIPI_isp_master_clock=*pFeatureData32;
			spin_unlock(&imx220_drv_lock);
            break;
        case SENSOR_FEATURE_SET_REGISTER:
			//iWriteReg((u16) pSensorRegData->RegAddr , (u32) pSensorRegData->RegData , 1, 0x66);//to test the AF
			IMX220MIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = IMX220MIPI_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&imx220_drv_lock);    
                IMX220MIPISensorCCT[i].Addr=*pFeatureData32++;
                IMX220MIPISensorCCT[i].Para=*pFeatureData32++; 
				spin_unlock(&imx220_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=IMX220MIPISensorCCT[i].Addr;
                *pFeatureData32++=IMX220MIPISensorCCT[i].Para; 
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {	spin_lock(&imx220_drv_lock);    
                IMX220MIPISensorReg[i].Addr=*pFeatureData32++;
                IMX220MIPISensorReg[i].Para=*pFeatureData32++;
				spin_unlock(&imx220_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=IMX220MIPISensorReg[i].Addr;
                *pFeatureData32++=IMX220MIPISensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=IMX220MIPI_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, IMX220MIPISensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, IMX220MIPISensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &IMX220MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            IMX220MIPI_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            IMX220MIPI_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=IMX220MIPI_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            IMX220MIPI_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            IMX220MIPI_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            IMX220MIPI_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
            pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_Gb;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;

        case SENSOR_FEATURE_INITIALIZE_AF:
            break;
        case SENSOR_FEATURE_CONSTANT_AF:
            break;
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            IMX220MIPISetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            IMX220MIPIGetSensorID(pFeatureReturnPara32); 
            break;             
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            IMX220MIPISetAutoFlickerMode((BOOL)*pFeatureData16, *(pFeatureData16+1));            
            break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            IMX220MIPISetTestPatternMode((BOOL)*pFeatureData16);            
            break;
        case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
            IMX220MIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
            break;
        case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
            IMX220MIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
            break;
        case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE://for factory mode auto testing             
            *pFeatureReturnPara32= IMX220_TEST_PATTERN_CHECKSUM;
            *pFeatureParaLen=4;                             
             break; 
        case SENSOR_FEATURE_SET_FRAMERATE:
            SENSORDB("SENSOR_FEATURE_SET_FRAMERATE:%d\n", *pFeatureData16);
            IMX220MIPI_CurretFPS = *pFeatureData16;
            break;
            case SENSOR_FEATURE_SET_HDR:
            SENSORDB("SENSOR_FEATURE_SET_IHDR:%d\n", *pFeatureData16);
            IMX220MIPI_IHDR_EN = *pFeatureData16;
            break;
        case SENSOR_FEATURE_GET_CROP_INFO:
            SENSORDB("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n", *pFeatureData32);
            pwininfo = (SENSOR_WINSIZE_INFO_STRUCT *)(*(pFeatureData32+1));

            switch(*pFeatureData32)
            {
                case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                    memcpy((void *)pwininfo,(void *)&SENSOR_WINSIZE_INFO_IMX220[1],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;    
                case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
                    memcpy((void *)pwininfo,(void *)&SENSOR_WINSIZE_INFO_IMX220[2],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
                    memcpy((void *)pwininfo,(void *)&SENSOR_WINSIZE_INFO_IMX220[3],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_SLIM_VIDEO:
                    memcpy((void *)pwininfo,(void *)&SENSOR_WINSIZE_INFO_IMX220[4],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
                default:
                    memcpy((void *)pwininfo,(void *)&SENSOR_WINSIZE_INFO_IMX220[0],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
            }
       /* case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
            SENSORDB("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",*pFeatureData32,*(pFeatureData32+1),*(pFeatureData32+2)); 
            IMX220MIPI_IHDR_write_shutter_gain(*pFeatureData32,*(pFeatureData32+1),*(pFeatureData32+2));*/

            break;
        default:
            break;
    }
    return ERROR_NONE;
}   /* IMX220MIPIFeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncIMX220MIPI=
{
    IMX220MIPIOpen,
    IMX220MIPIGetInfo,
    IMX220MIPIGetResolution,
    IMX220MIPIFeatureControl,
    IMX220MIPIControl,
    IMX220MIPIClose
};

UINT32 IMX220_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncIMX220MIPI;

    return ERROR_NONE;
}   /* SensorInit() */

