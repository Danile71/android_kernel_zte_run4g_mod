/* BEGIN PN:DTS2013053103858 , Added by d00238048, 2013.05.31*/
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <platform/mt_i2c.h> 
	#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_pm_ldo.h>
    #include <mach/mt_gpio.h>
#endif
#include <cust_gpio_usage.h>

#include <cust_i2c.h>

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif


static const unsigned int BL_MIN_LEVEL =20;
static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>  
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
//#include <linux/jiffies.h>
#include <linux/uaccess.h>
//#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
/***************************************************************************** 
 * Define
 *****************************************************************************/
#define TPS_I2C_BUSNUM  I2C_I2C_LCD_BIAS_CHANNEL//for I2C channel 0
#define I2C_ID_NAME "tps65132"
#define TPS_ADDR 0x3E
/***************************************************************************** 
 * GLobal Variable
 *****************************************************************************/
static struct i2c_board_info __initdata tps65132_board_info = {I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR)};
static struct i2c_client *tps65132_i2c_client = NULL;


/***************************************************************************** 
 * Function Prototype
 *****************************************************************************/ 
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tps65132_remove(struct i2c_client *client);
/***************************************************************************** 
 * Data Structure
 *****************************************************************************/

 struct tps65132_dev	{	
	struct i2c_client	*client;
	
};

static const struct i2c_device_id tps65132_id[] = {
	{ I2C_ID_NAME, 0 },
	{ }
};

//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
//static struct i2c_client_address_data addr_data = { .forces = forces,};
//#endif
static struct i2c_driver tps65132_iic_driver = {
	.id_table	= tps65132_id,
	.probe		= tps65132_probe,
	.remove		= tps65132_remove,
	//.detect		= mt6605_detect,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "tps65132",
	},
 
};
/***************************************************************************** 
 * Extern Area
 *****************************************************************************/ 
 
 

/***************************************************************************** 
 * Function
 *****************************************************************************/ 
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{  
	printk( "tps65132_iic_probe\n");
	printk("TPS: info==>name=%s addr=0x%x\n",client->name,client->addr);
	tps65132_i2c_client  = client;		
	return 0;      
}


static int tps65132_remove(struct i2c_client *client)
{  	
  printk( "tps65132_remove\n");
  tps65132_i2c_client = NULL;
   i2c_unregister_device(client);
  return 0;
}


 int tps65132_write_bytes(unsigned char addr, unsigned char value)
{	
	int ret = 0;
	struct i2c_client *client = tps65132_i2c_client;
	char write_data[2]={0};	
	write_data[0]= addr;
	write_data[1] = value;
    ret=i2c_master_send(client, write_data, 2);
	if(ret<0)
	printk("tps65132 write data fail !!\n");	
	return ret ;
}
EXPORT_SYMBOL_GPL(tps65132_write_bytes);



/*
 * module load/unload record keeping
 */

static int __init tps65132_iic_init(void)
{

   printk( "tps65132_iic_init\n");
   i2c_register_board_info(TPS_I2C_BUSNUM, &tps65132_board_info, 1);
   printk( "tps65132_iic_init2\n");
   i2c_add_driver(&tps65132_iic_driver);
   printk( "tps65132_iic_init success\n");	
   return 0;
}

static void __exit tps65132_iic_exit(void)
{
  printk( "tps65132_iic_exit\n");
  i2c_del_driver(&tps65132_iic_driver);  
}


module_init(tps65132_iic_init);
module_exit(tps65132_iic_exit);

MODULE_AUTHOR("Xiaokuan Shi");
MODULE_DESCRIPTION("MTK TPS65132 I2C Driver");
MODULE_LICENSE("GPL"); 
#endif

//static unsigned char lcd_id_pins_value = 0xFF;
static const unsigned char LCD_MODULE_ID = 0x01; //  haobing modified 2013.07.11
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define GPIO_65132_EN GPIO_LCD_BIAS_ENP_PIN

#define REGFLAG_DELAY             							0xFC
#define REGFLAG_END_OF_TABLE      							0xFD   // END OF REGISTERS MARKER


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif
//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------


struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28,0,{}},
	{0x10,0,{}},
	{REGFLAG_DELAY, 120, {}}
};

//update initial param for IC nt35520 0.01
static struct LCM_setting_table lcm_initialization_setting[] = {

	/*internal command*/
    //{0xff,  4,  {0xaa,0x55,0xa5,0x80}},
	//{0x6f,  1,  {0x11}},
	//{0xf3,  1,  {0x01}},


    {0xff,  4,  {0xaa,0x55,0xa5,0x80}},
	{0x6f,  2,  {0x11,0x00}},
	{0xf7,  2,  {0x20,0x00}},


    ////////////////////////////////////////////////////
#if 0
	/*page0*/
	{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x00}},
	{0xb1,  2,  {0x28,0x27}},
	{0xb5,  1,  {0xc8}},
	{0xb6,  1,  {0x14}},
	{0xb8,  4,  {0x00,0x00,0x14,0x0a}},
	{0xb9,  1,  {0x00}},
	{0xba,  1,  {0x00}},
	{0xbb,  2,  {0x63,0x63}},
	{0xbc,  2,  {0x00,0x00}},
	{0xbd,  5,  {0x02,0x78,0x0d,0x07,0x00}},
	{0xcc,  16, {0x41,0x36,0x87,0x54,0x46,0x65,0x10,0x12,0x14,0x10,0x12,0x14,0x40,0x08,0xa5,0x05}},
	{0xd0,  1,  {0x00}},
	{0xd1,  16, {0x00,0x04,0x08,0x0c,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c}},
	{0xd3,  1,  {0x00}},
	{0xd6,  2,  {0x44,0x44}},
	{0xd7,  12, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xd8,  13, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xd9,  3,  {0x00,0x28,0x00}},
	{0xe5,  2,  {0x00,0xff}},
	{0xe6,  4,  {0xf3,0xec,0xe7,0xdf}},
	{0xe7,  10, {0xf3,0xd9,0xcc,0xc0,0xb3,0xa6,0x99,0x99,0x99,0x95}},
	{0xe8,  10, {0xf3,0xd9,0xcc,0xc0,0xb3,0xa6,0x99,0x99,0x99,0x95}},
	{0xe9,  2,  {0x00,0x04}},
	{0xea,  1,  {0x00}},
	{0xf0,  5,  {0x55,0xaa,0x52,0x00,0x00}},

	/*page1*/
	{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x01}},
	{0xb0,  2,  {0x0d,0x0d}},
	{0xb1,  2,  {0x0d,0x0d}},
	{0xb3,  2,  {0x28,0x28}},
	{0xb4,  2,  {0x23,0x23}},
	{0xb6,  2,  {0x05,0x05}},
	{0xb7,  2,  {0x05,0x05}},
	{0xb8,  2,  {0x05,0x05}},
	{0xb9,  2,  {0x35,0x35}},
	{0xba,  2,  {0x34,0x34}},
	{0xbc,  2,  {0x50,0x00}},
	{0xbd,  2,  {0x50,0x00}},
	{0xbe,  1,  {0x00}},
	{0xbf,  1,  {0x00}},
	{0xc1,  1,  {0x01}},
	{0xc7,  3,  {0x00,0x00,0x00}},
	{0xc9,  6,  {0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xca,  1,  {0x00}},
	{0xcb,  2,  {0x0b,0x53}},
	{0xcd,  3,  {0x0b,0x52,0x53}},
	{0xcf,  3,  {0x00,0x50,0x50}},
	{0xf0,  5,  {0x55,0xaa,0x52,0x00,0x00}},

	/*page2-----gamma*/
	{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x02}},
		/*GMPR ctr*/
	{0xb0,  16, {0x00,0x00,0x00,0x13,0x00,0x2c,0x00,0x3f,0x00,0x62,0x00,0x84,0x00,0xa0,0x00,0xcf}},
	{0xb1,  16, {0x00,0xf4,0x01,0x2f,0x01,0x5e,0x01,0xa7,0x01,0xdf,0x01,0xe1,0x02,0x13,0x02,0x4f}},
	{0xb2,  16, {0x02,0x75,0x02,0xb9,0x02,0xed,0x03,0x3a,0x03,0x6b,0x03,0x88,0x03,0xa1,0x03,0xa3}},
	{0xb3,  4,  {0x03,0xB7,0x03,0xcc}},
		/*GMPG ctl*/
	{0xb4,	16, {0x00,0x00,0x00,0x13,0x00,0x2c,0x00,0x3f,0x00,0x62,0x00,0x84,0x00,0xa0,0x00,0xcf}},
	{0xb5,	16, {0x00,0xf4,0x01,0x2f,0x01,0x5e,0x01,0xa7,0x01,0xdf,0x01,0xe1,0x02,0x13,0x02,0x4f}},
	{0xb6,	16, {0x02,0x75,0x02,0xb9,0x02,0xed,0x03,0x3a,0x03,0x6b,0x03,0x88,0x03,0xa1,0x03,0xa3}},
	{0xb7,	4,	{0x03,0xB7,0x03,0xcc}},
		/*GMPB ctl*/
	{0xb8,	16, {0x00,0x00,0x00,0x13,0x00,0x2c,0x00,0x3f,0x00,0x62,0x00,0x84,0x00,0xa0,0x00,0xcf}},
	{0xb9,	16, {0x00,0xf4,0x01,0x2f,0x01,0x5e,0x01,0xa7,0x01,0xdf,0x01,0xe1,0x02,0x13,0x02,0x4f}},
	{0xba,	16, {0x02,0x75,0x02,0xb9,0x02,0xed,0x03,0x3a,0x03,0x6b,0x03,0x88,0x03,0xa1,0x03,0xa3}},
	{0xbb,	4,	{0x03,0xB7,0x03,0xcc}},	
		/*GMNR ctl*/
	{0xbc,  16, {0x00,0x00,0x00,0x13,0x00,0x2c,0x00,0x3f,0x00,0x62,0x00,0x84,0x00,0xa0,0x00,0xcf}},
	{0xbd,  16, {0x00,0xf4,0x01,0x2f,0x01,0x5e,0x01,0xa7,0x01,0xdf,0x01,0xe1,0x02,0x13,0x02,0x4f}},
	{0xbe,  16, {0x02,0x75,0x02,0xb9,0x02,0xed,0x03,0x3a,0x03,0x6b,0x03,0x88,0x03,0xa1,0x03,0xa3}},
	{0xbf,  4,  {0x03,0xB7,0x03,0xcc}},
		/*GMNG ctl*/
	{0xc0,	16, {0x00,0x00,0x00,0x13,0x00,0x2c,0x00,0x3f,0x00,0x62,0x00,0x84,0x00,0xa0,0x00,0xcf}},
	{0xc1,	16, {0x00,0xf4,0x01,0x2f,0x01,0x5e,0x01,0xa7,0x01,0xdf,0x01,0xe1,0x02,0x13,0x02,0x4f}},
	{0xc2,	16, {0x02,0x75,0x02,0xb9,0x02,0xed,0x03,0x3a,0x03,0x6b,0x03,0x88,0x03,0xa1,0x03,0xa3}},
	{0xc3,	4,	{0x03,0xB7,0x03,0xcc}},
		/*GMNB ctl*/
	{0xc4,	16, {0x00,0x00,0x00,0x13,0x00,0x2c,0x00,0x3f,0x00,0x62,0x00,0x84,0x00,0xa0,0x00,0xcf}},
	{0xc5,	16, {0x00,0xf4,0x01,0x2f,0x01,0x5e,0x01,0xa7,0x01,0xdf,0x01,0xe1,0x02,0x13,0x02,0x4f}},
	{0xc6,	16, {0x02,0x75,0x02,0xb9,0x02,0xed,0x03,0x3a,0x03,0x6b,0x03,0x88,0x03,0xa1,0x03,0xa3}},
	{0xc7,	4,	{0x03,0xB7,0x03,0xcc}},
		/*disable page2*/
	{0xf0,	5,	{0x55,0xaa,0x52,0x00,0x00}},

    /*page3*/
	{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x03}},
	{0xb0,  2,  {0xf5,0x00}},
	{0xb1,  2,  {0xf5,0x00}},
	{0xb2,  5,  {0x03,0x00,0x4b,0x00,0x00}},
	{0xb6,  5,  {0x03,0x00,0x4b,0x00,0x00}},
	{0xba,  5,  {0x35,0x00,0x4b,0x00,0x00}},
	{0xbb,  5,  {0x35,0x00,0x4b,0x00,0x00}},
	{0xc0,  4,  {0x30,0x00,0x00,0x00}},
	{0xc1,  4,  {0x03,0x00,0x00,0x00}},
	{0xc2,  4,  {0x21,0x00,0x00,0x00}},
	{0xc3,  4,  {0x12,0x00,0x00,0x00}},
	{0xc4,  1,  {0x40}},
	{0xc5,  1,  {0x40}},
	{0xef,  1,  {0x00}},
	{0xf0,	5,	{0x55,0xaa,0x52,0x00,0x00}},

    /*page5*/
	{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x05}},
	{0xcc,  3,  {0x00,0x00,0x04}},
	{0xcd,  3,  {0x00,0x00,0x04}},
	{0xce,  3,  {0x00,0x00,0x04}},
	{0xcf,  3,  {0x00,0x00,0x04}},
	{0xd0,	1,	{0x14}},
	{0xd1,  4,  {0x02,0x00,0x00,0x02}},
	{0xd2,  4,  {0x02,0x00,0x00,0x00}},
	{0xed,  1,  {0x30}},
	{0xf0,	5,	{0x55,0xaa,0x52,0x00,0x00}},

    /*page6*/
	{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x06}},
	{0xb0,  2,  {0x2e,0x00}},
	{0xb1,  2,  {0x2d,0x10}},
	{0xb2,  2,  {0x12,0x14}},
	{0xb3,  2,  {0x16,0x31}},
	{0xb4,  2,  {0x31,0x31}},
	{0xb5,  2,  {0x31,0x31}},
	{0xb6,  2,  {0x31,0x34}},
	{0xb7,  2,  {0x2b,0x2a}},
	{0xb8,  2,  {0x34,0x2c}},
	{0xb9,  2,  {0x29,0x34}},
	{0xba,  2,  {0x34,0x29}},
	{0xbb,  2,  {0x2c,0x34}},
	{0xbc,  2,  {0x2a,0x2b}},
	{0xbd,  2,  {0x34,0x31}},
	{0xbe,  2,  {0x31,0x31}},
	{0xbf,  2,  {0x31,0x31}},
	{0xc0,  2,  {0x31,0x17}},
	{0xc1,  2,  {0x15,0x13}},
	{0xc2,  2,  {0x11,0x2d}},
	{0xc3,  2,  {0x01,0x2e}},
	{0xc4,  2,  {0x2e,0x2d}},
	{0xc5,  2,  {0x01,0x17}},
	{0xc6,  2,  {0x15,0x13}},
	{0xc7,  2,  {0x11,0x31}},
	{0xc8,  2,  {0x31,0x31}},
	{0xc9,  2,  {0x31,0x31}},
	{0xca,  2,  {0x31,0x34}},
	{0xcb,  2,  {0x2b,0x2a}},
	{0xcc,  2,  {0x34,0x2c}},
	{0xcd,  2,  {0x29,0x34}},
	{0xce,  2,  {0x34,0x29}},
	{0xcf,  2,  {0x2c,0x34}},
	{0xd0,  2,  {0x2a,0x2b}},
	{0xd1,  2,  {0x34,0x31}},
	{0xd2,  2,  {0x31,0x31}},
	{0xd3,  2,  {0x31,0x31}},
	{0xd4,  2,  {0x31,0x10}},
	{0xd5,  2,  {0x12,0x14}},
	{0xd6,  2,  {0x16,0x00}},
	{0xd7,  2,  {0x2d,0x2e}},
	{0xd8,  5,  {0x00,0x00,0x00,0x00,0x00}},
	{0xd9,  5,  {0x00,0x00,0x00,0x00,0x00}},
	{0xe5,  2,  {0x34,0x34}},
	{0xe6,  2,  {0x34,0x34}},
	{0xe7,  1,  {0x00}},
	{0xf0,	5,	{0x55,0xaa,0x52,0x00,0x00}},

#endif
	/*internal command*/
    //{0xff,  4,  {0xaa,0x55,0xa5,0x80}},
	//{0x6f,  1,  {0x11}},
	//{0xf3,  1,  {0x01}},
	//{0xff,  4,  {0xaa,0x55,0xa5,0x80}},
	//{0x6f,  2,  {0x11,0x00}},
	//{0xf7,  2,  {0x20,0x00}},


	/* exit sleep*/
	{0x11,	0,  {}},
	{REGFLAG_DELAY, 150, {}},

	/*page0*/
	//{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x00}},
	//{0xb1,  2,  {0x28,0x27}},
	//{0xf0,	5,	{0x55,0xaa,0x52,0x00,0x00}},

	/*display on*/
	
	{0x29,	0,  {}},
	{REGFLAG_DELAY, 20, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}},

};


static void init_lcm_registers(void)
{
	unsigned int data_array[16];
	
		/*internal command*/	
		data_array[0]=0x00053902;
		data_array[1]=0xa555aaff; 
		data_array[2]=0x00000080;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x116f1500;
		dsi_set_cmdq(&data_array, 1, 1);
	
		data_array[0]=0x01f31500;
		dsi_set_cmdq(&data_array, 1, 1);
		
	
		/*page0*/
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000008;
		dsi_set_cmdq(&data_array, 3, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x002728b1;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0xc8b51500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x14b61500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00053902;
		data_array[1]=0x140000b8;
		data_array[2]=0x0000000a;
		dsi_set_cmdq(&data_array, 3, 1);

		data_array[0]=0x00b91500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00ba1500;
		dsi_set_cmdq(&data_array, 1, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x006363bb;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x000000bc;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00063902;
		data_array[1]=0x0d7802bd;
		data_array[2]=0x00000007;
		dsi_set_cmdq(&data_array, 3, 1);

		data_array[0]=0x00113902;
		data_array[1]=0x873641cc;
		data_array[2]=0x10654654;
		data_array[3]=0x12101412;
		data_array[4]=0xa5084014;
		data_array[5]=0x00000005;
		dsi_set_cmdq(&data_array, 6, 1);

		data_array[0]=0x00d01500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00113902;
		data_array[1]=0x080400d1;
		data_array[2]=0x1814100c;
		data_array[3]=0x2824201c;
		data_array[4]=0x3834302c;
		data_array[5]=0x0000003c;
		dsi_set_cmdq(&data_array, 6, 1);

		data_array[0]=0x00d31500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x004444d6;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x000d3902;
		data_array[1]=0x000000d7;
		data_array[2]=0x00000000;
		data_array[3]=0x00000000;
		data_array[4]=0x00000000;
		dsi_set_cmdq(&data_array, 5, 1);

		data_array[0]=0x000e3902;
		data_array[1]=0x000000d8;
		data_array[2]=0x00000000;
		data_array[3]=0x00000000;
		data_array[4]=0x00000000;
		dsi_set_cmdq(&data_array, 5, 1);
	
		data_array[0]=0x00043902;
		data_array[1]=0x002800d9;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x00ff00e5;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00053902;
		data_array[1]=0xe7ecf3e6;
		data_array[2]=0x000000df;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x000b3902;
		data_array[1]=0xccd9f3e7;
		data_array[2]=0x99a6b3c0;
		data_array[3]=0x00959999;
		dsi_set_cmdq(&data_array, 4, 1);

		data_array[0]=0x000b3902;
		data_array[1]=0xccd9f3e8;
		data_array[2]=0x99a6b3c0;
		data_array[3]=0x00959999;
		dsi_set_cmdq(&data_array, 4, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x000400e9;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00ea1500;
		dsi_set_cmdq(&data_array, 1, 1);
	
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		/*page1*/
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000108;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x000d0db0;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x000d0db1;
		dsi_set_cmdq(&data_array, 2, 1);
			
		data_array[0]=0x00033902;
		data_array[1]=0x002828b3;
		dsi_set_cmdq(&data_array, 2, 1);
			
		data_array[0]=0x00033902;
		data_array[1]=0x002323b4;
		dsi_set_cmdq(&data_array, 2, 1);
			
		data_array[0]=0x00033902;
		data_array[1]=0x000505b6;
		dsi_set_cmdq(&data_array, 2, 1);
			
		data_array[0]=0x00033902;
		data_array[1]=0x000505b7;
		dsi_set_cmdq(&data_array, 2, 1);
			
		data_array[0]=0x00033902;
		data_array[1]=0x000505b8;
		dsi_set_cmdq(&data_array, 2, 1);
			
		data_array[0]=0x00033902;
		data_array[1]=0x003535b9;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x003434ba;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x000050bc;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x000500bd;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00be1500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00bf1500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x01c11500;
		dsi_set_cmdq(&data_array, 1, 1);
		
		data_array[0]=0x00043902;
		data_array[1]=0x000000c7;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00073902;
		data_array[1]=0x000000c9;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);

		data_array[0]=0x00ca1500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x00530bcb;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00043902;
		data_array[1]=0x53520bcd;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00043902;
		data_array[1]=0x505000cf;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);

        /*page2-Gamma*/
	
		/*page3*/
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000308;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x0000f5b0;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x0000f5b1;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00063902;
		data_array[1]=0x4b0003b2;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00063902;
		data_array[1]=0x4b0003b6;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00063902;
		data_array[1]=0x4b0035ba;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);

		data_array[0]=0x00063902;
		data_array[1]=0x4b0035bb;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00053902;
		data_array[1]=0x000030c0;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
		
		data_array[0]=0x00053902;
		data_array[1]=0x000030c1;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
		
		data_array[0]=0x00053902;
		data_array[1]=0x000021c2;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00053902;
		data_array[1]=0x000012c3;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x40c41500;
		dsi_set_cmdq(&data_array, 1, 1);
		
		data_array[0]=0x40c51500;
		dsi_set_cmdq(&data_array, 1, 1);
	
		data_array[0]=0x00ef1500;
		dsi_set_cmdq(&data_array, 1, 1);
	
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		/*page5*/
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000508;
		dsi_set_cmdq(&data_array, 3, 1);
		
		data_array[0]=0x00043902;
		data_array[1]=0x040000cc;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00043902;
		data_array[1]=0x040000cd;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00043902;
		data_array[1]=0x040000ce;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00043902;
		data_array[1]=0x040000cf;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x14d01500;
		dsi_set_cmdq(&data_array, 1, 1);
	
		data_array[0]=0x00053902;
		data_array[1]=0x000002d1;
		data_array[2]=0x00000002;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00053902;
		data_array[1]=0x000002d2;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
			
		data_array[0]=0x30ed1500;
		dsi_set_cmdq(&data_array, 1, 1);
	
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		/*page6*/
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000608;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x00002eb0;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x00102db1;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x001412b2;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x003116b3;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00033902;
		data_array[1]=0x003131b4;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x003131b5;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00033902;
		data_array[1]=0x003431b6;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00033902;
		data_array[1]=0x002a2bb7;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00033902;
		data_array[1]=0x002c34b8;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00033902;
		data_array[1]=0x003429b9;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00033902;
		data_array[1]=0x002934ba;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x00342cbb;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x002b2abc;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x003134db;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x0000f5b0;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x003131bf;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x001731c0;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x001315c1;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x002d11c2;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x002e11c3;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x002d2ec4;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x001701c5;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x001315c6;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x003111c7;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x003131c8;
		dsi_set_cmdq(&data_array, 2, 1);		
	
		data_array[0]=0x00033902;
		data_array[1]=0x003131c9;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x003431ca;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x002a2bcb;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x002c34cc;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x003429cd;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x002934ce;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x00342ccf;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x002b2ad0;
		dsi_set_cmdq(&data_array, 2, 1);		

	    data_array[0]=0x00033902;
		data_array[1]=0x003134d1;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x003131d2;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x003131d3;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x001031d4;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x001412d5;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x000016d6;
		dsi_set_cmdq(&data_array, 2, 1);		

		data_array[0]=0x00033902;
		data_array[1]=0x002e2dd7;
		dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0]=0x00063902;
		data_array[1]=0x000000d8;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00063902;
		data_array[1]=0x000000d9;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x003434e5;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00033902;
		data_array[1]=0x003434e6;
		dsi_set_cmdq(&data_array, 2, 1);
	
		data_array[0]=0x00e71500;
		dsi_set_cmdq(&data_array, 1, 1);
		
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	


		data_array[0] = 0x00110500; // Sleep Out
		dsi_set_cmdq(&data_array, 1, 1);
	
		MDELAY(120);
	
		/*page0*/
		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000008;
		dsi_set_cmdq(&data_array, 3, 1);	

		data_array[0]=0x00033902;
		data_array[1]=0x002728b1;
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00063902;
		data_array[1]=0x52aa55f0;
		data_array[2]=0x00000000;
		dsi_set_cmdq(&data_array, 3, 1);
	
	
		data_array[0] = 0x00290508; // Display On  HS mode
		dsi_set_cmdq(&data_array, 1, 1);
	
		MDELAY(20);
}


							
#if 0
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
#if 0
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    //Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                if(table[i].count <= 10)
                    MDELAY(table[i].count);
                else
                    MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

    params->dsi.mode   = SYNC_EVENT_VDO_MODE; //BURST_VDO_MODE;//SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				    = LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order 	= LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      	= LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	params->dsi.packet_size=256;
	//video mode timing

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 1;//2;
	params->dsi.vertical_backporch					= 11;   // from Q driver
	params->dsi.vertical_frontporch					= 13;  // rom Q driver
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 5;//10;
	params->dsi.horizontal_backporch				= 40; // from Q driver
	params->dsi.horizontal_frontporch				= 84;  // from Q driver
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 210;//240; //this value must be in MTK suggested table

}

#ifdef BUILD_LK
#define TPS65132_SLAVE_ADDR_WRITE  0x7C  
static struct mt_i2c_t TPS65132_i2c;

int TPS65132_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    TPS65132_i2c.id = I2C_I2C_LCD_BIAS_CHANNEL;//I2C2;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    TPS65132_i2c.addr = (TPS65132_SLAVE_ADDR_WRITE >> 1);
    TPS65132_i2c.mode = ST_MODE;
    TPS65132_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&TPS65132_i2c, write_data, len);
    //printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}
#endif


static void lcm_init_power(void)
{
#ifdef BUILD_LK
	dprintf(0, "vgp3 on\n");
	mt6325_upmu_set_rg_vgp3_vosel(3);
	mt6325_upmu_set_rg_vgp3_en(1);
#else
	printk("vgp3 on\n");
	hwPowerOn(MT6325_POWER_LDO_VGP3, VOL_1800, "LCD");
#endif
	MDELAY(5); // min 1
}

static void lcm_suspend_power(void)
{
#ifdef BUILD_LK
		mt6325_upmu_set_rg_vgp3_en(0);
#else
		hwPowerDown(MT6325_POWER_LDO_VGP3, "LCD");
#endif
		MDELAY(5); // min 1
}

static void lcm_resume_power(void)
{
#ifdef BUILD_LK
		mt6325_upmu_set_rg_vgp3_vosel(3);
		mt6325_upmu_set_rg_vgp3_en(1);
#else
		hwPowerOn(MT6325_POWER_LDO_VGP3, VOL_1800, "LCD");
#endif
		MDELAY(5); // min 1
}


static void lcm_init(void)
{
	unsigned char cmd = 0x0;
	unsigned char data = 0xFF;
	unsigned int data_array[16];
	int ret=0;
	cmd=0x00;
	data=0x0A;

	//mt_set_gpio_mode(GPIO_LCM_BL_EN, GPIO_MODE_00);
	//mt_set_gpio_dir(GPIO_LCM_BL_EN, GPIO_DIR_OUT);
	//mt_set_gpio_out(GPIO_LCM_BL_EN, GPIO_OUT_ONE);

#ifdef BUILD_LK
		dprintf(0, "vgp3 on\n");
		mt6325_upmu_set_rg_vgp3_vosel(3);
		mt6325_upmu_set_rg_vgp3_en(1);
#else
		printk("vgp3 on\n");
		hwPowerOn(MT6325_POWER_LDO_VGP3, VOL_1800, "LCD");
#endif
		MDELAY(5); // min 1


/*-----------------DSV start---------------------*/

	mt_set_gpio_mode(GPIO_65132_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_65132_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_EN, GPIO_OUT_ONE);


#ifdef BUILD_LK
	ret=TPS65132_write_byte(cmd,data);
    if(ret)    	
    dprintf(0, "[LK]nt35595----tps6132----cmd=%0x--i2c write error----\n",cmd);    	
	else
	dprintf(0, "[LK]nt35595----tps6132----cmd=%0x--i2c write success----\n",cmd);    		
#else
	ret=tps65132_write_bytes(cmd,data);
	if(ret<0)
	printk("[KERNEL]nt35595----tps6132---cmd=%0x-- i2c write error-----\n",cmd);
	else
	printk("[KERNEL]nt35595----tps6132---cmd=%0x-- i2c write success-----\n",cmd);
#endif
	
	cmd=0x01;
	data=0x0A;
#ifdef BUILD_LK
	ret=TPS65132_write_byte(cmd,data);
    if(ret)    	
	    dprintf(0, "[LK]nt35595----tps6132----cmd=%0x--i2c write error----\n",cmd);    	
	else
		dprintf(0, "[LK]nt35595----tps6132----cmd=%0x--i2c write success----\n",cmd);   
#else
	ret=tps65132_write_bytes(cmd,data);
	if(ret<0)
	printk("[KERNEL]nt35595----tps6132---cmd=%0x-- i2c write error-----\n",cmd);
	else
	printk("[KERNEL]nt35595----tps6132---cmd=%0x-- i2c write success-----\n",cmd);
#endif

	/*-----------------DSV end---------------------*/
	MDELAY(50); // min 41
	SET_RESET_PIN(1);
	MDELAY(1);

	SET_RESET_PIN(0);
	MDELAY(1);  

	SET_RESET_PIN(1);
	MDELAY(150); // max 120

	// when phone initial , config output high, enable backlight drv chip  
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1); 
	
	//data_array[0] = 0x00290508; // Display On  HS mode
	///dsi_set_cmdq(&data_array, 1, 1);
	
	//MDELAY(20);

	//init_lcm_registers();

}

static void lcm_suspend(void)
{
    push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);  

	SET_RESET_PIN(1);
	MDELAY(5);
	
	SET_RESET_PIN(0);
	MDELAY(5);

	mt_set_gpio_mode(GPIO_65132_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_65132_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_EN, GPIO_OUT_ZERO);

}

static void lcm_resume(void)
{
	lcm_init(); 
}


LCM_DRIVER nt35521_hd720_dsi_vdo_jdi_tps65132_drv=
{
    .name           	= "nt35521_hd720_dsi_vdo_jdi_tps65132_drv",
    .set_util_funcs 	= lcm_set_util_funcs,
    .get_params     	= lcm_get_params,
    .init           	= lcm_init,/*tianma init fun.*/
    .suspend        	= lcm_suspend,
    .resume         	= lcm_resume,
     //.compare_id     	= lcm_compare_id,
     .init_power		= lcm_init_power,
     .resume_power = lcm_resume_power,
     .suspend_power = lcm_suspend_power,   
};
