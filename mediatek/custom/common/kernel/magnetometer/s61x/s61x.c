/* s61x.c - s61x compass driver
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

#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>

//#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE

#include <cust_mag.h>
#include <linux/hwmsen_helper.h>

#define S61X_NEW_ARCH   //susport kk new msensor arch

#ifdef S61X_NEW_ARCH
#include "mag.h"
//#include <linux/batch.h>

#endif



/*----------------------------------------------------------------------------*/

/* SLAVE_ADDRESS:
   - 0x0E: for S61X with CAD=Low
   - 0x0F: for S61X with CAD=High  */

/****** Begin of Customization ******/
#define SLAVE_ADDRESS           0x0E
#define USE_ALTERNATE_ADDRESS   0
/****** End of Customization ********/

/*----------------------------------------------------------------------------*/
#if 0
struct mag_hw* s61x_get_cust_mag_hw(void);
#define get_cust_mag_hw()       s61x_get_cust_mag_hw()
#endif
#define PLATFORM_DRIVER_NAME    "msensor"

/*----------------------------------------------------------------------------*/
#define UMAP(x)                 (x)

#define ALTER_ADDRESS           0x1E

#if USE_ALTERNATE_ADDRESS
#define S61X_I2C_ADDRESS        UMAP(ALTER_ADDRESS)
#else
#define S61X_I2C_ADDRESS        UMAP(SLAVE_ADDRESS)
#endif

#define S61X_DEV_NAME           "s61x"
#define DRIVER_VERSION          "2.4.1"

/*----------------------------------------------------------------------------*/
#define DEBUG                   0
#define S61X_DEBUG              1
#define S61X_DEBUG_MSG          1
#define S61X_DEBUG_FUNC         1
#define S61X_DEBUG_DATA         1
#define S61X_RETRY_COUNT        9
#define S61X_DEFAULT_DELAY      100

#define MSE_TAG                  "MSENSOR"

#if S61X_DEBUG_MSG
#define SSMDBG(format, ...)     printk(KERN_INFO "S61X " format "\n", ## __VA_ARGS__)

#define MSE_FUN(f)               printk(MSE_TAG" %s\r\n", __FUNCTION__)
#define MSE_ERR(fmt, args...)    printk(KERN_ERR MSE_TAG" %s %d : \r\n"fmt, __FUNCTION__, __LINE__, ##args)
#define MSE_LOG(fmt, args...)    printk(MSE_TAG fmt, ##args)

#else
#define SSMDBG(format, ...)

#define MSE_FUN(f)               
#define MSE_ERR(fmt, args...)    
#define MSE_LOG(fmt, args...)    

#endif

#if S61X_DEBUG_FUNC
#define SSMFUNC(func)           printk(KERN_INFO "S61X " func " is called\n")
#else
#define SSMFUNC(func)
#endif

//Don't change this if you don't know why! (refer comment:ticket:35:2)
#define PROJECT_ID              "S611"

/*----------------------------------------------------------------------------*/
#define SENSOR_DATA_COUNT       3
#define SENSOR_DATA_SIZE        (SENSOR_DATA_COUNT*sizeof(short))
#define CALIBRATION_DATA_SIZE   12

#define RWBUF_SIZE              32
#define S61X_BUFSIZE            32

#define CONVERT_O               1
#define CONVERT_O_DIV           100
#define CONVERT_M               1
#define CONVERT_M_DIV           10000

/*----------------------------------------------------------------------------*/
#define SS_SENSOR_MODE_OFF      0
#define SS_SENSOR_MODE_MEASURE  1

/*----------------------------------------------------------------------------*/
#define S61X_IDX_DEVICE_ID      0x00
#define DEVICE_ID_VALUE         0x21

#define S61X_IDX_DEVICE_INFO    0x01
#define DEVICE_INFO_M1301       0x13

#define S61X_IDX_STA1           0x02
#define STA1_DRDY               0x01

#define S61X_IDX_X_LSB          0x03

#define S61X_IDX_MODE           0x0A
#define MODE_TRIGGER            0x01

#define S61X_IDX_I2CDIS         0x0F
#define I2CDIS_NEWCIC           0x07

#define S61X_IDX_GAIN_X         0x13
#define S61X_IDX_GAIN_Y         0x14
#define S61X_IDX_GAIN_Z         0x15
#define GAIN_DEFAULT            0x07

#define S61X_IDX_OS_X           0x16
#define S61X_IDX_OS_Y           0x17
#define S61X_IDX_OS_Z           0x18
#define S61X_IDX_OS_POL         0x19

#define S61X_IDX_ECO1           0x1F
#define ECO1_DEFAULT(x)         (((x)&I2C_MASK_FLAG)!=UMAP(0x1E) ? 0x00 : 0x04)

#define S61X_IDX_ECO2           0x20
#define ECO2_MRAS_NO_RST        0x80

#define S61X_IDX_CHOP           0x21
#define S61X_IDX_CONFIG         0x22
#define S61X_IDX_DATA_POL       0x25

#define S61X_IDX_PROBE_OE       0x37
#define PROBE_OE_WATCHDOG       0x10

#define S61X_IDX_WDT_VALUE      0x38

#define S61X_MUL_X              1
#define S61X_DIV_X              8
#define S61X_OFF_X              2048
#define S61X_MUL_Y              1
#define S61X_DIV_Y              8
#define S61X_OFF_Y              2048
#define S61X_MUL_Z              1
#define S61X_DIV_Z              8
#define S61X_OFF_Z              2048

/*----------------------------------------------------------------------------*/
static struct i2c_client *this_client = NULL;

static short last_m_data[SENSOR_DATA_COUNT];
static struct mutex last_m_data_mutex;

static int sensor_data[CALIBRATION_DATA_SIZE];
static struct mutex sensor_data_mutex;
static struct mutex i2c_mutex; //add by wxj

static DECLARE_WAIT_QUEUE_HEAD(open_wq);

static short ssmd_delay = S61X_DEFAULT_DELAY;
static char ssmd_status[RWBUF_SIZE];

static atomic_t open_flag = ATOMIC_INIT(0);
static atomic_t m_flag = ATOMIC_INIT(0);
static atomic_t o_flag = ATOMIC_INIT(0);
static atomic_t m_get_data;
static atomic_t o_get_data;
static atomic_t dev_open_count;
static atomic_t init_phase = ATOMIC_INIT(2);  // 1 = id check ok, 0 = init ok



/*----------------------------------------------------------------------------*/
static const struct i2c_device_id s61x_i2c_id[] = { { S61X_DEV_NAME, 0 }, {} };
static struct i2c_board_info __initdata i2c_s61x = { I2C_BOARD_INFO(S61X_DEV_NAME, S61X_I2C_ADDRESS) };

/*----------------------------------------------------------------------------*/
static int s61x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int s61x_i2c_remove(struct i2c_client *client);
static int s61x_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);

#ifdef S61X_NEW_ARCH

static int s61x_init_flag = -1;  // 0 = OK, -1 = fail

static int s61x_remove(void);
static int s61x_local_init(void);

static int s61x_m_open_report_data(int en);
static int s61x_m_set_delay(u64 delay);
static int s61x_m_enable(int en);
static int s61x_o_open_report_data(int en);
static int s61x_o_set_delay(u64 delay);
static int s61x_o_enable(int en);
static int s61x_get_data_m(int *x,int *y, int *z,int *status);
static int s61x_get_data_o(int *x,int *y, int *z,int *status);


#else
static int s61x_probe(struct platform_device *pdev);

static int s61x_remove(struct platform_device *pdev);
#endif


/*----------------------------------------------------------------------------*/
typedef enum {
        SS_FUN_DEBUG  = 0x01,
        SS_DATA_DEBUG = 0X02,
        SS_HWM_DEBUG  = 0X04,
        SS_CTR_DEBUG  = 0X08,
        SS_I2C_DEBUG  = 0x10,
} SS_TRACE;

/*----------------------------------------------------------------------------*/
struct s61x_i2c_data {
        struct i2c_client *client;
        struct mag_hw *hw;
        atomic_t layout;
        atomic_t trace;
        atomic_t info;
		atomic_t suspend;
        struct hwmsen_convert   cvt;
#if defined(CONFIG_HAS_EARLYSUSPEND)
        struct early_suspend    early_drv;
#endif
};

#define L2CHIP(x)       ((x)/10)        //layout to chip id
#define L2CVTI(x)       l2cvti((x)%10)  //layout to cvt index

/*----------------------------------------------------------------------------*/
#ifdef S61X_NEW_ARCH
static struct mag_init_info s61x_init_info = {
        .name   = S61X_DEV_NAME,
        .init   = s61x_local_init,
        .uninit = s61x_remove,
};
#else

#ifdef CONFIG_OF
static const struct of_device_id msensor_of_match[] = {
	{ .compatible = "mediatek,msensor", },
	{},
};
#endif

static struct platform_driver s61x_msensor_driver = {
	.probe      = s61x_probe,
	.remove     = s61x_remove,    
	.driver     = 
	{
		.name  = "msensor",
	//	.owner  = THIS_MODULE,
        #ifdef CONFIG_OF
		.of_match_table = msensor_of_match,
		#endif
	}
};

#endif
/*----------------------------------------------------------------------------*/
static struct i2c_driver s61x_i2c_driver = {
        .driver = {
                .owner = THIS_MODULE,
                .name  = S61X_DEV_NAME,
        },
        .probe         = s61x_i2c_probe,
        .remove        = s61x_i2c_remove,
        .detect        = s61x_i2c_detect,
#if !defined(CONFIG_HAS_EARLYSUSPEND)
        .suspend       = s61x_suspend,
        .resume        = s61x_resume,
#endif
        .id_table      = s61x_i2c_id,
};

/*----------------------------------------------------------------------------*/
static void s61x_power(struct mag_hw *hw, unsigned int on)
{
        static unsigned int power_on = 0;

        if (hw->power_id != POWER_NONE_MACRO) {
                SSMDBG("power %s", on ? "on" : "off");
                if (power_on == on) {
                        SSMDBG("ignore power control: %d", on);
                } else if (on) {
                        if (!hwPowerOn(hw->power_id, hw->power_vol, S61X_DEV_NAME)) {
                                printk(KERN_ERR "S61X power on fail\n");
                        }
                } else {
                        if (!hwPowerDown(hw->power_id, S61X_DEV_NAME)) {
                                printk(KERN_ERR "S61X power off fail\n");
                        }
                }
        }

        power_on = on;
}

/*----------------------------------------------------------------------------*/
static int I2C_RxData(char *rxData, int length)
{
        uint8_t loop_i;

#if DEBUG
        int i;
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
        char addr = rxData[0];
#endif

        /* Caller should check parameter validity.*/
        if ((rxData == NULL) || (length < 1)) {
                return -EINVAL;
        }
		
		mutex_lock(&i2c_mutex);
        for (loop_i = 0; loop_i < S61X_RETRY_COUNT; loop_i++) {
                this_client->addr = (this_client->addr & I2C_MASK_FLAG) | (I2C_WR_FLAG);
                if (i2c_master_send(this_client, (const char*)rxData, ((length<<0X08) | 0X01))) {
                        break;
                }
                mdelay(10);
        }
		
		this_client->addr = (this_client->addr & I2C_MASK_FLAG);
		mutex_unlock(&i2c_mutex);

        if (loop_i >= S61X_RETRY_COUNT) {
                printk(KERN_ERR "S61X %s retry over %d\n", __func__, S61X_RETRY_COUNT);
                return -EIO;
        }

#if DEBUG
        if (atomic_read(&data->trace) & SS_I2C_DEBUG) {
                MSE_LOG("S61X RxData len=%02x addr=%02x\n data=", length, addr);
                for (i = 0; i < length; i++) {
                        MSE_LOG(" %02x", rxData[i]);
                }
                MSE_LOG("\n");
        }
#endif
        return 0;
}

static int I2C_TxData(char *txData, int length)
{
        uint8_t loop_i;

#if DEBUG
        int i;
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
#endif

        /* Caller should check parameter validity.*/
        if ((txData == NULL) || (length < 2)) {
                return -EINVAL;
        }
		
		mutex_lock(&i2c_mutex);

        this_client->addr = this_client->addr & I2C_MASK_FLAG;
        for (loop_i = 0; loop_i < S61X_RETRY_COUNT; loop_i++) {
                if (i2c_master_send(this_client, (const char*)txData, length) > 0) {
                        break;
                }
                mdelay(10);
        }
		mutex_unlock(&i2c_mutex);

        if (loop_i >= S61X_RETRY_COUNT) {
                printk(KERN_ERR "S61X %s retry over %d\n", __func__, S61X_RETRY_COUNT);
                return -EIO;
        }

#if DEBUG
        if (atomic_read(&data->trace) & SS_I2C_DEBUG) {
                MSE_LOG("S61X TxData len=%02x addr=%02x\n data=", length, txData[0]);
                for (i = 0; i < (length-1); i++) {
                        MSE_LOG(" %02x", txData[i + 1]);
                }
                MSE_LOG("\n");
        }
#endif
        return 0;
}

static int I2C_TxData2(unsigned char c1, unsigned char c2)
{
        unsigned char data[2];

        data[0] = c1;
        data[1] = c2;

        return I2C_TxData(data, 2);
}

static int ECS_InitDevice(void)
{
        char *err_desc = NULL;

        if (I2C_TxData2(S61X_IDX_PROBE_OE, PROBE_OE_WATCHDOG) < 0) {
                err_desc = "PROBE_OE";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_I2CDIS, I2CDIS_NEWCIC) < 0) {
                err_desc = "I2CDIS";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_GAIN_X, GAIN_DEFAULT) < 0) {
                err_desc = "GAIN_X";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_GAIN_Y, GAIN_DEFAULT) < 0) {
                err_desc = "GAIN_Y";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_GAIN_Z, GAIN_DEFAULT) < 0) {
                err_desc = "GAIN_Z";
                goto end_of_func;
        }

        /* for safety */

        if (I2C_TxData2(S61X_IDX_OS_X, 0x00) < 0) {
                err_desc = "OS_X";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_OS_Y, 0x00) < 0) {
                err_desc = "OS_Y";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_OS_Z, 0x00) < 0) {
                err_desc = "OS_Z";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_OS_POL, 0x00) < 0) {
                err_desc = "OS_POL";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_CHOP, 0x07) < 0) {
                err_desc = "CHOP";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_DATA_POL, 0x00) < 0) {
                err_desc = "DATA_POL";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_WDT_VALUE, 0x51) < 0) {
                err_desc = "WDT_VALUE";
                goto end_of_func;
        }

end_of_func:

        if (err_desc) {
                printk(KERN_ERR "S61X_IDX_%s failed\n", err_desc);
                return -EFAULT;
        }

#if DEBUG
{
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);

        if ((atomic_read(&data->trace) & 0xF000) == 0x9000) return -EFAULT;
        else atomic_set(&init_phase, 0);
}
#endif
        return 0;
}

static int ECS_SetMode_Off(void)
{
        char *err_desc = NULL;

        if (I2C_TxData2(S61X_IDX_MODE, 0x00) < 0) {
                err_desc = "MODE";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_ECO2, 0x00) < 0) {
                err_desc = "ECO2";
                goto end_of_func;
        }

end_of_func:

        if (err_desc) {
                printk(KERN_ERR "S61X_IDX_%s failed\n", err_desc);
                return -EFAULT;
        }

        return 0;
}

static int ECS_SetMode_Measure(void)
{
        char *err_desc = NULL;

        if (I2C_TxData2(S61X_IDX_ECO2, ECO2_MRAS_NO_RST) < 0) {
                err_desc = "ECO2";
                goto end_of_func;
        }

        if (I2C_TxData2(S61X_IDX_MODE, MODE_TRIGGER) < 0) {
                err_desc = "MODE";
                goto end_of_func;
        }

end_of_func:

        if (err_desc) {
                printk(KERN_ERR "S61X_IDX_%s failed\n", err_desc);
                return -EFAULT;
        }

        return 0;
}

static int ECS_SetMode(char mode)
{
        int ret;

        switch (mode) {

        case SS_SENSOR_MODE_OFF:
                ret = ECS_SetMode_Off();
                break;

        case SS_SENSOR_MODE_MEASURE:
                ret = ECS_SetMode_Measure();
                break;

        default:
                SSMDBG("%s: Unknown mode(%d)", __func__, mode);
                return -EINVAL;
        }

        return ret;
}

static int ECS_CheckDevice(void)
{
        char id1, id2;
        int err;

        id1 = S61X_IDX_DEVICE_ID;
        err = I2C_RxData(&id1, 1);
        if (err < 0) {
                return err;
        }

        id2 = S61X_IDX_DEVICE_INFO;
        err = I2C_RxData(&id2, 1);
        if (err < 0) {
                return err;
        }

#if DEBUG
{
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);

        if ((atomic_read(&data->trace) & 0xF000) == 0x8000) id1 = 0x00, id2= 0x01;
        else atomic_set(&init_phase, 1);
}
#endif
        if (id1 != DEVICE_ID_VALUE || id2 != DEVICE_INFO_M1301) {
                printk(KERN_ERR "S61X incorrect id %02X:%02X\n", id1, id2);
                return -EFAULT;
        }

        return id2;
}

static int ECS_SaveData(int *buf)
{
#if DEBUG
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
#endif

        mutex_lock(&sensor_data_mutex);
        memcpy(sensor_data, buf, sizeof(sensor_data));
        mutex_unlock(&sensor_data_mutex);

#if DEBUG
        if (atomic_read(&data->trace) & SS_HWM_DEBUG) {
                SSMDBG("Get daemon data: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d!",
                       sensor_data[0],sensor_data[1],sensor_data[2],sensor_data[3],
                       sensor_data[4],sensor_data[5],sensor_data[6],sensor_data[7],
                       sensor_data[8],sensor_data[9],sensor_data[10],sensor_data[11]);
        }
#endif
        return 0;
}

#define V(c1,c2)        (int)((((unsigned)(c1))<<8)|((unsigned)(c2)))
#define VMAX            32767

static int ECS_GetData(short *mag)
{
        int loop_i,ret;
        struct i2c_client *client = this_client;

		if(NULL == client)
		{
			MSE_ERR("client is null!!\n");
			return -1;
		}
		
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
        short v;
        char buf[6];

		if(NULL == data)
		{
			MSE_ERR("s61x_i2c_data is null!!\n");
			return -1;
		}
		if(1 == atomic_read(&data->suspend))
		{
			MSE_LOG("in suspend mode!\n");
			return -1;
		}

        for (loop_i = 0; loop_i < S61X_RETRY_COUNT; loop_i++) {
                buf[0] = S61X_IDX_STA1;
                if (I2C_RxData(buf, 1)) {
                        printk(KERN_ERR "S61X_IDX_STA1 %s fail\n", __func__);
                        return -1;
                }

                if (buf[0] == STA1_DRDY) {
                        break;
                }
                msleep(10);
        }

        if (loop_i >= S61X_RETRY_COUNT) {
                printk(KERN_ERR "S61X %s retry over\n", __func__);
                return -1;
        }

        buf[0] = S61X_IDX_X_LSB;
        ret = I2C_RxData(buf, sizeof(buf));
        if (ret < 0) {
                printk(KERN_ERR "S61X_IDX_X_LSB %s fail\n", __func__);
                return -1;
        }
        v = V(buf[1], buf[0]), mag[data->cvt.map[0]] = (data->cvt.sign[0] > 0) ? v : (VMAX - v);
        v = V(buf[5], buf[4]), mag[data->cvt.map[1]] = (data->cvt.sign[1] > 0) ? v : (VMAX - v);
        v = V(buf[3], buf[2]), mag[data->cvt.map[2]] = (data->cvt.sign[2] > 0) ? v : (VMAX - v);

        /* for debug only */
        mutex_lock(&last_m_data_mutex);
        memcpy(last_m_data, mag, sizeof(last_m_data));
        mutex_unlock(&last_m_data_mutex);

#if DEBUG
        if (atomic_read(&data->trace) & SS_DATA_DEBUG) {
                SSMDBG("Get device data: (%d,%d,%d)", mag[0], mag[1], mag[2]);
        }
#endif

        return 0;
}

static int ECS_GetOpenStatus(void)
{
        wait_event_interruptible(open_wq, (atomic_read(&open_flag) != 0));
        return atomic_read(&open_flag);
}

static int ECS_GetCloseStatus(void)
{
        wait_event_interruptible(open_wq, (atomic_read(&open_flag) <= 0));
        return atomic_read(&open_flag);
}

/*----------------------------------------------------------------------------*/
static int ECS_ReadChipInfo(char *buf, int bufsize, int more)
{
        static const char *supported_chip[] = { "S611" };
        int phase = atomic_read(&init_phase);
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
        unsigned chip = L2CHIP(atomic_read(&data->layout));
        int ret;

        if (!buf) {
                return -1;
        }

        if (!this_client) {
                *buf = 0;
                return -2;
        }

        if (phase != 0) {
                *buf = 0;
                return -3;
        }

        if (chip < sizeof(supported_chip)/sizeof(char*)) {
                ret = sprintf(buf, supported_chip[chip]);
        }
        else {
                ret = sprintf(buf, "?%u", chip);
        }

        if (more) ret += sprintf(buf + ret, "-%x", atomic_read(&data->info));

        return ret;
}

/*----------------------------------------------------------------------------*/
static int l2cvti(int layout)
{
        int cvti = layout;
        if (cvti < 0 || cvti > 7) {
                printk(KERN_ERR "S61X invalid layout (%d), force to 0\n", layout);
                cvti = 0;
        }

        return cvti;
}

/*----------------------------------------------------------------------------*/
static ssize_t show_daemon_value(struct device_driver *ddri, char *buf)
{
        char strbuf[S61X_BUFSIZE];
        sprintf(strbuf, "s61xd");
        return sprintf(buf, "%s", strbuf);
}

/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
        char strbuf[S61X_BUFSIZE];
        ECS_ReadChipInfo(strbuf, S61X_BUFSIZE, 1);
        return sprintf(buf, "%s\n", strbuf);
}

/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
        short mdata[SENSOR_DATA_COUNT];

        mutex_lock(&last_m_data_mutex);
        memcpy(mdata, last_m_data, sizeof(mdata));
        mutex_unlock(&last_m_data_mutex);

        return sprintf(buf, "%d %d %d\n", mdata[0], mdata[1], mdata[2]);
}

/*----------------------------------------------------------------------------*/
static ssize_t show_posturedata_value(struct device_driver *ddri, char *buf)
{
        int tmp[3];
		
		mutex_lock(&last_m_data_mutex);

        tmp[0] = sensor_data[0] * CONVERT_O / CONVERT_O_DIV;
        tmp[1] = sensor_data[1] * CONVERT_O / CONVERT_O_DIV;
        tmp[2] = sensor_data[2] * CONVERT_O / CONVERT_O_DIV;
		mutex_unlock(&last_m_data_mutex);

        return sprintf(buf, "%d, %d, %d\n", tmp[0], tmp[1], tmp[2]);
}

/*----------------------------------------------------------------------------*/
static ssize_t show_layout_value(struct device_driver *ddri, char *buf)
{
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);

        return sprintf(buf, "(%d, %d)\n[%+2d %+2d %+2d]\n[%+2d %+2d %+2d]\n",
                       data->hw->direction, atomic_read(&data->layout),
                       data->cvt.sign[0], data->cvt.sign[1], data->cvt.sign[2],
                       data->cvt.map[0], data->cvt.map[1], data->cvt.map[2]);
}

/*----------------------------------------------------------------------------*/
static ssize_t store_layout_value(struct device_driver *ddri, const char *buf, size_t count)
{
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
        int layout = 0;

        if (sscanf(buf, "%d", &layout) == 1) {
                atomic_set(&data->layout, layout);
                if (!hwmsen_get_convert(L2CVTI(layout), &data->cvt)) {
                        printk(KERN_ERR "HWMSEN_GET_CONVERT function error!\n");
                } else if (!hwmsen_get_convert(L2CVTI(data->hw->direction), &data->cvt)) {
                        printk(KERN_ERR "invalid layout: %d, restore to %d\n", layout, data->hw->direction);
                } else {
                        printk(KERN_ERR "invalid layout: (%d, %d)\n", layout, data->hw->direction);
                        hwmsen_get_convert(0, &data->cvt);
                }
        } else {
                printk(KERN_ERR "invalid format = '%s'\n", buf);
        }

        return count;
}

/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
        ssize_t len = 0;

        len += snprintf(buf+len, PAGE_SIZE-len, "VERS: %s (%s)\n", DRIVER_VERSION, ssmd_status);

        if (data->hw) {
                len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n",
                                data->hw->i2c_num, data->hw->direction, data->hw->power_id, data->hw->power_vol);
        } else {
                len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
        }

        switch (atomic_read(&init_phase)) {

        case 2:
                len += snprintf(buf+len, PAGE_SIZE-len, "IDCK: %d\n", ECS_CheckDevice());
                break;

        case 1:
                len += snprintf(buf+len, PAGE_SIZE-len, "INIT: %d\n", ECS_InitDevice());
                break;

        case 0:
                len += snprintf(buf+len, PAGE_SIZE-len, "OPEN: %d (%d/%d)\n",
                                atomic_read(&dev_open_count), atomic_read(&m_get_data), atomic_read(&o_get_data));
                break;
        }

        return len;
}

/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
        ssize_t res;
        struct s61x_i2c_data *obj = i2c_get_clientdata(this_client);
        if (NULL == obj) {
                printk(KERN_ERR "S61X data is null!!\n");
                return 0;
        }

        res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));
        return res;
}

/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, const char *buf, size_t count)
{
        struct s61x_i2c_data *obj = i2c_get_clientdata(this_client);
        int trace;
        if (NULL == obj) {
                printk(KERN_ERR "S61X data is null!!\n");
                return -1;
        }

        if (1 == sscanf(buf, "0x%x", &trace)) {
                atomic_set(&obj->trace, trace);
        } else {
                printk(KERN_ERR "S61X invalid content: '%s', length = %d\n", buf, count);
        }

        return count;
}

/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(daemon,      S_IRUGO, show_daemon_value, NULL);
static DRIVER_ATTR(chipinfo,    S_IRUGO, show_chipinfo_value, NULL);
static DRIVER_ATTR(sensordata,  S_IRUGO, show_sensordata_value, NULL);
static DRIVER_ATTR(posturedata, S_IRUGO, show_posturedata_value, NULL);
static DRIVER_ATTR(status,      S_IRUGO, show_status_value, NULL);
static DRIVER_ATTR(layout,      S_IRUGO | S_IWUSR, show_layout_value, store_layout_value);
static DRIVER_ATTR(trace,       S_IRUGO | S_IWUSR, show_trace_value,  store_trace_value);

/*----------------------------------------------------------------------------*/
static struct driver_attribute *s61x_attr_list[] = {
        &driver_attr_daemon,
        &driver_attr_chipinfo,
        &driver_attr_sensordata,
        &driver_attr_posturedata,
        &driver_attr_status,
        &driver_attr_layout,
        &driver_attr_trace,
};

/*----------------------------------------------------------------------------*/
static int s61x_create_attr(struct device_driver *driver)
{
        int idx, err = 0;
        int num = (int)(sizeof(s61x_attr_list)/sizeof(s61x_attr_list[0]));
        if (driver == NULL) {
                return -EINVAL;
        }

        for (idx = 0; idx < num; idx++) {
                if ((err = driver_create_file(driver, s61x_attr_list[idx])) !=0 ) {
                        printk(KERN_ERR "S61X driver_create_file (%s) = %d\n", s61x_attr_list[idx]->attr.name, err);
                        break;
                }
        }
        return err;
}

/*----------------------------------------------------------------------------*/
static int s61x_delete_attr(struct device_driver *driver)
{
        int idx ,err = 0;
        int num = (int)(sizeof(s61x_attr_list)/sizeof(s61x_attr_list[0]));

        if (driver == NULL) {
                return -EINVAL;
        }

        for (idx = 0; idx < num; idx++) {
                driver_remove_file(driver, s61x_attr_list[idx]);
        }

        return err;
}

/*----------------------------------------------------------------------------*/
static int s61x_open(struct inode *inode, struct file *file)
{
        int ret = -1;
#if DEBUG
        struct s61x_i2c_data *obj = i2c_get_clientdata(this_client);

        if (atomic_read(&obj->trace) & SS_CTR_DEBUG) {
                SSMDBG("open device node");
        }
#endif

        atomic_inc(&dev_open_count);
        ret = nonseekable_open(inode, file);

        return ret;
}

/*----------------------------------------------------------------------------*/
static int s61x_release(struct inode *inode, struct file *file)
{
#if DEBUG
        struct s61x_i2c_data *obj = i2c_get_clientdata(this_client);

        if (atomic_read(&obj->trace) & SS_CTR_DEBUG) {
                SSMDBG("release device node");
        }
#endif
        atomic_dec(&dev_open_count);

        return 0;
}

/*----------------------------------------------------------------------------*/
static int factory_mode(void)
{
        /* for factory mode (without open from the Daemon and successfully initialized) */
        return (atomic_read(&dev_open_count) == 1 && atomic_read(&init_phase) == 0);
}

/*----------------------------------------------------------------------------*/
static long s61x_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        void __user *argp = (void __user *)arg;

        /* NOTE: In this function the size of "char" should be 1-byte. */
        short mdata[SENSOR_DATA_COUNT];
        char rwbuf[RWBUF_SIZE];
        char buff[S61X_BUFSIZE];
        char mode;
        int value[CALIBRATION_DATA_SIZE];
        uint32_t enable;
        hwm_sensor_data* hwm_data;
        int status;
        int ret = -1;

        switch (cmd) {

        case ECS_IOCTL_WRITE:
                if (argp == NULL) {
                        SSMDBG("invalid argument.");
                        return -EINVAL;
                }
                if (copy_from_user(rwbuf, argp, sizeof(rwbuf))) {
                        SSMDBG("copy_from_user failed.");
                        return -EFAULT;
                }

                if (rwbuf[0] == 0) {
                        memset(ssmd_status, 0, sizeof(ssmd_status));
                        strncpy(ssmd_status, &rwbuf[1], sizeof(ssmd_status)-1);
                        break;
                }

                if ((rwbuf[0] < 2) || (rwbuf[0] > (RWBUF_SIZE-1))) {
                        SSMDBG("invalid argument.");
                        return -EINVAL;
                }
                ret = I2C_TxData(&rwbuf[1], rwbuf[0]);
                if (ret < 0) {
                        return ret;
                }
                break;

        case ECS_IOCTL_READ:
                if (argp == NULL) {
                        SSMDBG("invalid argument.");
                        return -EINVAL;
                }

                if (copy_from_user(rwbuf, argp, sizeof(rwbuf))) {
                        SSMDBG("copy_from_user failed.");
                        return -EFAULT;
                }

                if (rwbuf[0] == 0) {
                        if (rwbuf[1] == 1 || rwbuf[1] == 2) {
                                struct s61x_i2c_data *data = i2c_get_clientdata(this_client);
                                uint32_t layout;
                                if (rwbuf[1] == 2) {
                                        memcpy(&layout, &rwbuf[2], sizeof(layout));
                                        atomic_set(&data->layout, layout);
                                        hwmsen_get_convert(L2CVTI(layout), &data->cvt);
                                }
                                layout = atomic_read(&data->layout);
                                if (copy_to_user(argp, &layout, sizeof(layout))) {
                                        SSMDBG("copy_to_user failed.");
                                        return -EFAULT;
                                }
                                break;
                        }
                }

                if ((rwbuf[0] < 1) || (rwbuf[0] > (RWBUF_SIZE-1))) {
                        SSMDBG("invalid argument.");
                        return -EINVAL;
                }
                ret = I2C_RxData(&rwbuf[1], rwbuf[0]);
                if (ret < 0) {
                        return ret;
                }
                if (copy_to_user(argp, rwbuf, rwbuf[0]+1)) {
                        SSMDBG("copy_to_user failed.");
                        return -EFAULT;
                }
                break;

        case ECS_IOCTL_SET_MODE:
                if (argp == NULL) {
                        SSMDBG("invalid argument.");
                        return -EINVAL;
                }
                if (copy_from_user(&mode, argp, sizeof(mode))) {
                        SSMDBG("copy_from_user failed.");
                        return -EFAULT;
                }
                ret = ECS_SetMode(mode);
                if (ret < 0) {
                        return ret;
                }
                break;

        case ECS_IOCTL_GETDATA:
                ret = ECS_GetData(mdata);
                if (ret < 0) {
                        return ret;
                }

                if (copy_to_user(argp, mdata, sizeof(mdata))) {
                        SSMDBG("copy_to_user failed.");
                        return -EFAULT;
                }
                break;

        case ECS_IOCTL_SET_YPR:
                if (argp == NULL) {
                        SSMDBG("invalid argument.");
                        return -EINVAL;
                }
                if (copy_from_user(value, argp, sizeof(value))) {
                        SSMDBG("copy_from_user failed.");
                        return -EFAULT;
                }
                ECS_SaveData(value);
                break;

        case ECS_IOCTL_GET_OPEN_STATUS:
                status = ECS_GetOpenStatus();
                SSMDBG("ECS_GetOpenStatus returned (%d)", status);
                if (copy_to_user(argp, &status, sizeof(status))) {
                        SSMDBG("copy_to_user failed.");
                        return -EFAULT;
                }
                break;

        case ECS_IOCTL_GET_CLOSE_STATUS:
                status = ECS_GetCloseStatus();
                SSMDBG("ECS_GetCloseStatus returned (%d)", status);
                if (copy_to_user(argp, &status, sizeof(status))) {
                        SSMDBG("copy_to_user failed.");
                        return -EFAULT;
                }
                break;

        case ECS_IOCTL_GET_DELAY:
                if (copy_to_user(argp, &ssmd_delay, sizeof(ssmd_delay))) {
                        SSMDBG("copy_to_user failed.");
                        return -EFAULT;
                }
                break;

        case ECS_IOCTL_GET_PROJECT_NAME:
                if (argp == NULL) {
                        printk(KERN_ERR "S61X IO parameter pointer is NULL!\n");
                        break;
                }

                sprintf(buff, PROJECT_ID);
                status = ECS_ReadChipInfo(buff + sizeof(PROJECT_ID), S61X_BUFSIZE - sizeof(PROJECT_ID), 1);
                if (status > 0) {
                        status = sizeof(PROJECT_ID) + status + 1;
                }
                else {
                        *buff = 0;
                        status = 1;
                }
                if (copy_to_user(argp, buff, status)) {
                        return -EFAULT;
                }
                break;

        case MSENSOR_IOCTL_READ_CHIPINFO:
                if (argp == NULL) {
                        printk(KERN_ERR "S61X IO parameter pointer is NULL!\n");
                        break;
                }

                ECS_ReadChipInfo(buff, S61X_BUFSIZE, 0);
                if (copy_to_user(argp, buff, strlen(buff)+1)) {
                        return -EFAULT;
                }
                break;

        case MSENSOR_IOCTL_READ_SENSORDATA:
                if (argp == NULL) {
                        printk(KERN_ERR "S61X IO parameter pointer is NULL!\n");
                        break;
                }

                if (factory_mode()) {
                        ret = ECS_GetData(mdata);
                        if (ret < 0) {
                                return -EFAULT;
                        }
                }

                mutex_lock(&last_m_data_mutex);
                memcpy(mdata, last_m_data, sizeof(mdata));
                mutex_unlock(&last_m_data_mutex);
                mdata[0] = mdata[0] * S61X_MUL_X / S61X_DIV_X;
                mdata[1] = mdata[1] * S61X_MUL_Y / S61X_DIV_Y;
                mdata[2] = mdata[2] * S61X_MUL_Z / S61X_DIV_Z;
                sprintf(buff, "%x %x %x", (int)mdata[0], (int)mdata[1], (int)mdata[2]);
                if (copy_to_user(argp, buff, strlen(buff)+1)) {
                        return -EFAULT;
                }
                break;

        case MSENSOR_IOCTL_SENSOR_ENABLE:
                if (argp == NULL) {
                        printk(KERN_ERR "S61X IO parameter pointer is NULL!\n");
                        break;
                }

                if (copy_from_user(&enable, argp, sizeof(enable))) {
                        SSMDBG("copy_from_user failed.");
                        return -EFAULT;
                } else {
                        SSMDBG("MSENSOR_IOCTL_SENSOR_ENABLE enable=%d", enable);
                        if (enable == 1) {
                                atomic_set(&o_flag, 1);
                                atomic_set(&open_flag, 1);
                        } else {
                                atomic_set(&o_get_data, 0);
                                atomic_set(&o_flag, 0);
                                if (atomic_read(&m_flag) == 0) {
                                        atomic_set(&open_flag, 0);
                                }
                        }
                        wake_up(&open_wq);

                        if (factory_mode()) {
                                int mode = enable ? SS_SENSOR_MODE_MEASURE : SS_SENSOR_MODE_OFF;
                                SSMDBG("MSENSOR_IOCTL_SENSOR_ENABLE SetMode(%d)\n", mode);
                                ECS_SetMode(mode);
                        }
                }
                break;

        case MSENSOR_IOCTL_READ_FACTORY_SENSORDATA:
                if (argp == NULL) {
                        printk(KERN_ERR "S61X IO parameter pointer is NULL!\n");
                        break;
                }

                hwm_data = (hwm_sensor_data *)buff;
                mutex_lock(&sensor_data_mutex);
                hwm_data->values[0] = sensor_data[0] * CONVERT_O;
                hwm_data->values[1] = sensor_data[1] * CONVERT_O;
                hwm_data->values[2] = sensor_data[2] * CONVERT_O;
                hwm_data->status = sensor_data[4];
                hwm_data->value_divide = CONVERT_O_DIV;
                mutex_unlock(&sensor_data_mutex);

                sprintf(buff, "%x %x %x %x %x", hwm_data->values[0], hwm_data->values[1],
                        hwm_data->values[2], hwm_data->status, hwm_data->value_divide);
                if (copy_to_user(argp, buff, strlen(buff)+1)) {
                        return -EFAULT;
                }
                break;

        default:
                printk(KERN_ERR "S61X %s not supported = 0x%04x\n", __FUNCTION__, cmd);
                return -ENOIOCTLCMD;
                break;
        }

        return 0;
}

/*----------------------------------------------------------------------------*/
static struct file_operations s61x_fops = {
        .owner   = THIS_MODULE,
        .open    = s61x_open,
        .release = s61x_release,
        .unlocked_ioctl = s61x_unlocked_ioctl,
};

/*----------------------------------------------------------------------------*/
static struct miscdevice s61x_device = {
        .minor   = MISC_DYNAMIC_MINOR,
        .name    = "msensor",
        .fops    = &s61x_fops,
};

/*----------------------------------------------------------------------------*/
#ifdef S61X_NEW_ARCH
static int s61x_m_open_report_data(int en)
{
#if 0
	struct s61x_i2c_data *data = i2c_get_clientdata(this_client);

	if(NULL == data)
	{
		MSE_ERR("data IS NULL !\n");
		return -1;
	}
#endif
	return 0;
}
static int s61x_m_set_delay(u64 delay)
{
	int value = 0;
	struct s61x_i2c_data *data = i2c_get_clientdata(this_client);

	if(NULL == data)
	{
		MSE_ERR("data IS NULL !\n");
		return -1;
	}
	
	value = (int)delay;
	if (value <= 20) {
			ssmd_delay = 20;
	}
	ssmd_delay = value;

	return 0;
}
static int s61x_m_enable(int en)
{
	int value = 0;
	struct s61x_i2c_data *data = i2c_get_clientdata(this_client);

	if(NULL == data)
	{
		MSE_ERR("data IS NULL !\n");
		return -1;
	}

	value = en;
	
	if (value == 1) {
			atomic_set(&m_flag, 1);
			atomic_set(&open_flag, 1);
	} else {
			atomic_set(&m_get_data, 0);
			atomic_set(&m_flag, 0);
			if (atomic_read(&o_flag) == 0) {
					atomic_set(&open_flag, 0);
			}
	}
	wake_up(&open_wq);

	return 0;
}
static int s61x_o_open_report_data(int en)
{
#if 0
		struct s61x_i2c_data *data = i2c_get_clientdata(this_client);
	
		if(NULL == data)
		{
			MSE_ERR("data IS NULL !\n");
			return -1;
		}
#endif
	return 0;

}
static int s61x_o_set_delay(u64 delay)
{
	return s61x_m_set_delay(delay);//the same is msensor set_delay
}
static int s61x_o_enable(int en)
{
	int value = 0;
	struct s61x_i2c_data *data = i2c_get_clientdata(this_client);

	if(NULL == data)
	{
		MSE_ERR("data IS NULL !\n");
		return -1;
	}

	value = en;
	
	if (value == 1) {
			atomic_set(&o_flag, 1);
			atomic_set(&open_flag, 1);
	} else {
			atomic_set(&o_get_data, 0);
			atomic_set(&o_flag, 0);
			if (atomic_read(&m_flag) == 0) {
					atomic_set(&open_flag, 0);
			}
	}
	wake_up(&open_wq);

	return 0;
}
static int s61x_get_data_m(int *x,int *y, int *z,int *status)
{
	struct s61x_i2c_data *data = i2c_get_clientdata(this_client);

	if(NULL == data)
	{
		MSE_ERR("data IS NULL !\n");
		return -1;
	}

	atomic_inc(&m_get_data);

	mutex_lock(&sensor_data_mutex);
	
	*x = sensor_data[9] * CONVERT_M;
	*y = sensor_data[10] * CONVERT_M;
	*z = sensor_data[11] * CONVERT_M;
	*status = sensor_data[4];
	
	mutex_unlock(&sensor_data_mutex);
#if DEBUG
	if (atomic_read(&data->trace) & SS_HWM_DEBUG) {
			SSMDBG("%s get data: %d, %d, %d. divide %d, status %d!", __func__,
			*x, *y, *z, CONVERT_M_DIV, *status);
	}
#endif

	return 0;
}
static int s61x_get_data_o(int *x,int *y, int *z,int *status)
{
	struct s61x_i2c_data *data = i2c_get_clientdata(this_client);

	if(NULL == data)
	{
		MSE_ERR("data IS NULL !\n");
		return -1;
	}

	atomic_inc(&o_get_data);

	mutex_lock(&sensor_data_mutex);
	
	*x = sensor_data[0] * CONVERT_O;
	*y = sensor_data[1] * CONVERT_O;
	*z = sensor_data[2] * CONVERT_O;
	*status = sensor_data[4];
	
	mutex_unlock(&sensor_data_mutex);
#if DEBUG
	if (atomic_read(&data->trace) & SS_HWM_DEBUG) {
			SSMDBG("%s get data: %d, %d, %d. divide %d, status %d!", __func__,
			*x, *y, *z,	CONVERT_O_DIV, *status);
	}
#endif
	return 0;
}

#else
static int s61x_operate(void* self, uint32_t command,
                void* buff_in, int size_in, void* buff_out, int size_out, int* actualout)
{
        int err = 0;
        int value;
        hwm_sensor_data* hwm_data;
#if DEBUG
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
#endif

#if DEBUG
        if (atomic_read(&data->trace) & SS_FUN_DEBUG) {
                SSMFUNC(__func__);
        }
#endif

        switch (command) {

        case SENSOR_DELAY:
                if ((buff_in == NULL) || (size_in < sizeof(int))) {
                        printk(KERN_ERR "S61X Set delay parameter error!\n");
                        err = -EINVAL;
                } else {
                        value = *(int *)buff_in;
                        if (value <= 20) {
                                ssmd_delay = 20;
                        }
                        ssmd_delay = value;
                }
                break;

        case SENSOR_ENABLE:
                if ((buff_in == NULL) || (size_in < sizeof(int))) {
                        printk(KERN_ERR "S61X Enable sensor parameter error!\n");
                        err = -EINVAL;
                } else {

                        value = *(int *)buff_in;

                        if (value == 1) {
                                atomic_set(&m_flag, 1);
                                atomic_set(&open_flag, 1);
                        } else {
                                atomic_set(&m_get_data, 0);
                                atomic_set(&m_flag, 0);
                                if (atomic_read(&o_flag) == 0) {
                                        atomic_set(&open_flag, 0);
                                }
                        }
                        wake_up(&open_wq);
                }
                break;

        case SENSOR_GET_DATA:
                atomic_inc(&m_get_data);
                if ((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data))) {
                        printk(KERN_ERR "S61X get sensor data parameter error!\n");
                        err = -EINVAL;
                } else {
                        hwm_data = (hwm_sensor_data *)buff_out;
                        mutex_lock(&sensor_data_mutex);

                        hwm_data->values[0] = sensor_data[9] * CONVERT_M;
                        hwm_data->values[1] = sensor_data[10] * CONVERT_M;
                        hwm_data->values[2] = sensor_data[11] * CONVERT_M;
                        hwm_data->status = sensor_data[4];
                        hwm_data->value_divide = CONVERT_M_DIV;

                        mutex_unlock(&sensor_data_mutex);
#if DEBUG
                        if (atomic_read(&data->trace) & SS_HWM_DEBUG) {
                                SSMDBG("%s get data: %d, %d, %d. divide %d, status %d!", __func__,
                                       hwm_data->values[0], hwm_data->values[1], hwm_data->values[2],
                                       hwm_data->value_divide, hwm_data->status);
                        }
#endif
                }
                break;

        default:
                printk(KERN_ERR "S61X %s has no this parameter %d!\n", __func__, command);
                err = -1;
                break;
        }

        return err;
}

/*----------------------------------------------------------------------------*/
static int s61x_orientation_operate(void* self, uint32_t command,
                void* buff_in, int size_in, void* buff_out, int size_out, int* actualout)
{
        int err = 0;
        int value;
        hwm_sensor_data* hwm_data;
#if DEBUG
        struct i2c_client *client = this_client;
        struct s61x_i2c_data *data = i2c_get_clientdata(client);
#endif

#if DEBUG
        if (atomic_read(&data->trace) & SS_FUN_DEBUG) {
                SSMFUNC(__func__);
        }
#endif

        switch (command) {

        case SENSOR_DELAY:
                if ((buff_in == NULL) || (size_in < sizeof(int))) {
                        printk(KERN_ERR "S61X Set delay parameter error!\n");
                        err = -EINVAL;
                } else {
                        value = *(int *)buff_in;
                        if (value <= 20) {
                                ssmd_delay = 20;
                        }
                        ssmd_delay = value;
                }
                break;

        case SENSOR_ENABLE:
                if ((buff_in == NULL) || (size_in < sizeof(int))) {
                        printk(KERN_ERR "S61X Enable sensor parameter error!\n");
                        err = -EINVAL;
                } else {

                        value = *(int *)buff_in;

                        if (value == 1) {
                                atomic_set(&o_flag, 1);
                                atomic_set(&open_flag, 1);
                        } else {
                                atomic_set(&o_get_data, 0);
                                atomic_set(&o_flag, 0);
                                if (atomic_read(&m_flag) == 0) {
                                        atomic_set(&open_flag, 0);
                                }
                        }
                        wake_up(&open_wq);
                }
                break;

        case SENSOR_GET_DATA:
                atomic_inc(&o_get_data);
                if ((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data))) {
                        printk(KERN_ERR "S61X get sensor data parameter error!\n");
                        err = -EINVAL;
                } else {
                        hwm_data = (hwm_sensor_data *)buff_out;
                        mutex_lock(&sensor_data_mutex);

                        hwm_data->values[0] = sensor_data[0] * CONVERT_O;
                        hwm_data->values[1] = sensor_data[1] * CONVERT_O;
                        hwm_data->values[2] = sensor_data[2] * CONVERT_O;
                        hwm_data->status = sensor_data[4];
                        hwm_data->value_divide = CONVERT_O_DIV;

                        mutex_unlock(&sensor_data_mutex);
#if DEBUG
                        if (atomic_read(&data->trace) & SS_HWM_DEBUG) {
                                SSMDBG("%s get data: %d, %d, %d. divide %d, status %d!", __func__,
                                       hwm_data->values[0], hwm_data->values[1], hwm_data->values[2],
                                       hwm_data->value_divide, hwm_data->status);
                        }
#endif
                }
                break;

        default:
                printk(KERN_ERR "S61X %s has no this parameter %d!\n", __func__, command);
                err = -1;
                break;
        }

        return err;
}
#endif

#ifndef CONFIG_HAS_EARLYSUSPEND

/*----------------------------------------------------------------------------*/
static int s61x_suspend(struct i2c_client *client, pm_message_t msg)
{
        int err;
        struct s61x_i2c_data *obj = i2c_get_clientdata(client)


        if (msg.event == PM_EVENT_SUSPEND) {
                s61x_power(obj->hw, 0);
        }
        return 0;
}

/*----------------------------------------------------------------------------*/
static int s61x_resume(struct i2c_client *client)
{
        int err;
        struct s61x_i2c_data *obj = i2c_get_clientdata(client)


                                    s61x_power(obj->hw, 1);


        return 0;
}

#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/

/*----------------------------------------------------------------------------*/
static void s61x_early_suspend(struct early_suspend *h)
{
        struct s61x_i2c_data *obj = container_of(h, struct s61x_i2c_data, early_drv);
		MSE_FUN(f);
        if (NULL == obj) {
                printk(KERN_ERR "S61X null pointer!!\n");
                return;
        }
		atomic_set(&obj->suspend, 1);
		if(0 == atomic_read(&open_flag))
		{
			ECS_SetMode(SS_SENSOR_MODE_OFF);
		}
		s61x_power(obj->hw, 0);
}

/*----------------------------------------------------------------------------*/
static void s61x_late_resume(struct early_suspend *h)
{
        struct s61x_i2c_data *obj = container_of(h, struct s61x_i2c_data, early_drv);
		MSE_FUN(f);

        if (NULL == obj) {
                printk(KERN_ERR "S61X null pointer!!\n");
                return;
        }

        s61x_power(obj->hw, 1);
		if(1 == atomic_read(&open_flag))
		{
			ECS_SetMode(SS_SENSOR_MODE_MEASURE);
		}
		atomic_set(&obj->suspend, 0);
}

#endif /*CONFIG_HAS_EARLYSUSPEND*/

/*----------------------------------------------------------------------------*/
static int s61x_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
        strcpy(info->type, S61X_DEV_NAME);
        return 0;
}

/*----------------------------------------------------------------------------*/
static int s61x_change_address(const struct i2c_client *client)
{
        uint8_t loop_i;
        char cmd[] = { S61X_IDX_ECO1, ECO1_DEFAULT(client->addr) };
        struct i2c_msg msg[] = {
                {
                        .addr = UMAP(SLAVE_ADDRESS),
                        .flags = 0,
                        .len = sizeof(cmd),
                        .buf = cmd,
                },
        };

        for (loop_i = 0; loop_i < 3; loop_i++) {
                if (i2c_transfer(client->adapter, msg, 1) > 0) {
                        return 0;
                }
                mdelay(10);
        }

        //printk(KERN_ERR "S61X change address retry over %d\n", loop_i);
        return -EIO;
}

/*----------------------------------------------------------------------------*/
static int s61x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
        struct i2c_client *new_client;
        struct s61x_i2c_data *data;
        int err = 0;

#ifdef S61X_NEW_ARCH
		struct mag_control_path ctl_path ={0};
		struct mag_data_path dat_path = {0};
#else
        struct hwmsen_object sobj_m, sobj_o;
#endif
        if (!(data = kmalloc(sizeof(struct s61x_i2c_data), GFP_KERNEL))) {
                err = -ENOMEM;
                goto exit;
        }
        memset(data, 0, sizeof(struct s61x_i2c_data));
        data->hw = get_cust_mag_hw();

        atomic_set(&data->layout, data->hw->direction);
        atomic_set(&data->trace, 0x0000);

        mutex_init(&last_m_data_mutex);
        mutex_init(&sensor_data_mutex);
		mutex_init(&i2c_mutex);

        init_waitqueue_head(&open_wq);

        data->client = client;
        new_client = data->client;
        i2c_set_clientdata(new_client, data);

        this_client = new_client;
        this_client->timing = 100;

        if ((client->addr & I2C_MASK_FLAG) != UMAP(SLAVE_ADDRESS)) {
                err = s61x_change_address(client);
                printk(KERN_ERR "S61X address change %s\n", err == 0 ? "OK" : "NG");
        }

        /* Check connection */
        atomic_set(&data->info, ECS_CheckDevice());
        if (atomic_read(&data->info) < 0) {
                printk(KERN_ERR "S61X check device connect error\n");
                err = -EFAULT;
                goto exit_kfree;
        } else {
                atomic_set(&init_phase, 1);
                if (ECS_InitDevice() != 0) {
                        printk(KERN_ERR "S61X init device error\n");
                        err = -EFAULT;
                        goto exit_kfree;
                } else {
                        atomic_set(&init_phase, 0);
                        if (hwmsen_get_convert(L2CVTI(data->hw->direction), &data->cvt)) {
                                printk(KERN_ERR "S61X invalid direction: %d\n", data->hw->direction);
                                err = -EINVAL;
                                goto exit_kfree;
                        }
                }
        }

        /* Register sysfs attribute */
#ifdef S61X_NEW_ARCH
        if ((err = s61x_create_attr(&(s61x_init_info.platform_diver_addr->driver))) != 0) {
                printk(KERN_ERR "S61X create attribute err = %d\n", err);
                goto exit_init_failed;
        }
#else
		if ((err = s61x_create_attr(&(s61x_msensor_driver.driver))) != 0) {
		                printk(KERN_ERR "S61X create attribute err = %d\n", err);
		                goto exit_init_failed;
		        }

#endif
        if ((err = misc_register(&s61x_device)) != 0) {
                printk(KERN_ERR "S61X device register failed\n");
                goto exit_sysfs_create_group_failed;
        }
#ifdef S61X_NEW_ARCH
		ctl_path.m_open_report_data = s61x_m_open_report_data;
		ctl_path.m_enable 			= s61x_m_enable;
		ctl_path.m_set_delay 		= s61x_m_set_delay;
		
		ctl_path.o_open_report_data = s61x_o_open_report_data;
		ctl_path.o_enable 			= s61x_o_enable;
		ctl_path.o_set_delay 		= s61x_o_set_delay;

		ctl_path.is_report_input_direct = false;
		ctl_path.is_support_batch 		= data->hw->is_batch_supported;
		
		err = mag_register_control_path(&ctl_path);

		if(err < 0)
		{
			MSE_ERR("mag_register_control_path failed!\n");
			goto exit_misc_device_register_failed;
		}

		dat_path.div_m = CONVERT_M_DIV;
		dat_path.div_o = CONVERT_O_DIV;

		dat_path.get_data_m = s61x_get_data_m;
		dat_path.get_data_o = s61x_get_data_o;

		err = mag_register_data_path(&dat_path);
		if(err < 0)
		{
			MSE_ERR("mag_register_control_path failed!\n");
			goto exit_misc_device_register_failed;
		}

#else
        sobj_m.self = data;
        sobj_m.polling = 1;
        sobj_m.sensor_operate = s61x_operate;
        if ((err = hwmsen_attach(ID_MAGNETIC, &sobj_m)) != 0) {
                printk(KERN_ERR "S61X attach fail = %d\n", err);
                goto exit_misc_device_register_failed;
        }

        sobj_o.self = data;
        sobj_o.polling = 1;
        sobj_o.sensor_operate = s61x_orientation_operate;
        if ((err = hwmsen_attach(ID_ORIENTATION, &sobj_o)) != 0) {
                printk(KERN_ERR "S61X attach fail = %d\n", err);
                goto exit_misc_device_register_failed;
        }
#endif

#if CONFIG_HAS_EARLYSUSPEND
        data->early_drv.level   = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
        data->early_drv.suspend = s61x_early_suspend,
        data->early_drv.resume  = s61x_late_resume,
        register_early_suspend(&data->early_drv);
#endif

        SSMDBG("%s: %s", __func__, atomic_read(&init_phase) ? "NG" : "OK");
#ifdef S61X_NEW_ARCH

        s61x_init_flag = 0;
#endif
        return 0;



exit_misc_device_register_failed:

	misc_deregister(&s61x_device);

exit_sysfs_create_group_failed:
	
#ifdef S61X_NEW_ARCH
	s61x_delete_attr(&(s61x_init_info.platform_diver_addr->driver));
#else
	s61x_delete_attr(&(s61x_msensor_driver.driver));

#endif

exit_init_failed:

exit_kfree:
        kfree(data);

exit:
        printk(KERN_ERR "S61X %s: err = %d\n", __func__, err);
#ifdef S61X_NEW_ARCH

        s61x_init_flag = -1;
#endif
        return err;
}

/*----------------------------------------------------------------------------*/
static int s61x_i2c_remove(struct i2c_client *client)
{
        int err;

#ifdef S61X_NEW_ARCH

        if ((err = s61x_delete_attr(&(s61x_init_info.platform_diver_addr->driver))) != 0) {
                printk(KERN_ERR "S61X delete_attr fail: %d\n", err);
        }
#else
		s61x_delete_attr(&(s61x_msensor_driver.driver));

#endif
        this_client = NULL;
        i2c_unregister_device(client);
        kfree(i2c_get_clientdata(client));
        misc_deregister(&s61x_device);
        return 0;
}

/*----------------------------------------------------------------------------*/
#ifdef S61X_NEW_ARCH
static int s61x_local_init(void)
{
        struct mag_hw *hw = get_cust_mag_hw();
		MSE_FUN(f);
        s61x_power(hw, 1);

        atomic_set(&dev_open_count, 0);

        if (i2c_add_driver(&s61x_i2c_driver)) {
                printk(KERN_ERR "S61X add driver error\n");
                return -1;
        }

        if (s61x_init_flag == -1) {
                return -1;
        }

        return 0;
}

/*----------------------------------------------------------------------------*/
static int s61x_remove(void)
{
        struct mag_hw *hw = get_cust_mag_hw();

        s61x_power(hw, 0);
        atomic_set(&dev_open_count, 0);
        i2c_del_driver(&s61x_i2c_driver);
        return 0;
}
#else
static int s61x_probe(struct platform_device *pdev) 
{
	struct mag_hw *hw = get_cust_mag_hw();

	MSE_FUN(f);
	
	s61x_power(hw, 1);	
	atomic_set(&dev_open_count, 0);
	
	if (i2c_add_driver(&s61x_i2c_driver)) {
		MSE_ERR("S61X add driver error\n");
		return -1;
	}

	return 0;
}
static int s61x_remove(struct platform_device *pdev)
{
	struct mag_hw *hw = get_cust_mag_hw();

	MSE_FUN(f);
	s61x_power(hw, 0);
	atomic_set(&dev_open_count, 0);
	i2c_del_driver(&s61x_i2c_driver);

	return 0;
}

#endif


/*----------------------------------------------------------------------------*/
static int __init s61x_init(void)
{
	struct mag_hw *hw = get_cust_mag_hw();
	//printk(KERN_INFO "S61X s61x_init\n");
	MSE_FUN(f);
	
	i2c_register_board_info(hw->i2c_num, &i2c_s61x, 1);

#ifdef S61X_NEW_ARCH

	if(mag_driver_add(&s61x_init_info) < 0)
	{
		MSE_ERR("mag_driver_add failed!\n");
	}
#else
	if(platform_driver_register(&s61x_msensor_driver))
	{
		MSE_ERR("failed to register driver");
		return -ENODEV;
	}
#endif
        return 0;
}

/*----------------------------------------------------------------------------*/
static void __exit s61x_exit(void)
{
        //printk(KERN_INFO "S61X s61x_exit\n");
        MSE_FUN(f);
#ifndef S61X_NEW_ARCH
		platform_driver_unregister(&s61x_msensor_driver);
#endif
}

/*----------------------------------------------------------------------------*/
module_init(s61x_init);
module_exit(s61x_exit);

MODULE_AUTHOR("Rupert Li <rupert@smartsensor.com.tw>");
MODULE_DESCRIPTION("S61X Compass Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
