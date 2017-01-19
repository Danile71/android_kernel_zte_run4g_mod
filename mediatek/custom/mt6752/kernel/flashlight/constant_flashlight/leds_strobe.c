#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include "kd_camera_hw.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <mach/upmu_common.h>

#include <mach/mt_gpio.h>		// For gpio control

/*
0  1  2  3   4   5   6   7   8   9   10  11  12  13
25 50 75 100 125 150 300 400 500 600 700 800 900 1000
*/

/******************************************************************************
 * Debug configuration
******************************************************************************/
// availible parameter
// ANDROID_LOG_ASSERT
// ANDROID_LOG_ERROR
// ANDROID_LOG_WARNING
// ANDROID_LOG_INFO
// ANDROID_LOG_DEBUG
// ANDROID_LOG_VERBOSE
#define TAG_NAME "leds_strobe.c"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME, KERN_INFO  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_WARN(fmt, arg...)        xlog_printk(ANDROID_LOG_WARNING, TAG_NAME, KERN_WARNING  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_NOTICE(fmt, arg...)      xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME, KERN_NOTICE  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_INFO(fmt, arg...)        xlog_printk(ANDROID_LOG_INFO   , TAG_NAME, KERN_INFO  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_TRC_FUNC(f)              xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME,  "<%s>\n", __FUNCTION__);
#define PK_TRC_VERBOSE(fmt, arg...) xlog_printk(ANDROID_LOG_VERBOSE, TAG_NAME,  fmt, ##arg)
#define PK_ERROR(fmt, arg...)       xlog_printk(ANDROID_LOG_ERROR  , TAG_NAME, KERN_ERR "%s: " fmt, __FUNCTION__ ,##arg)


#define DEBUG_LEDS_STROBE
#ifdef  DEBUG_LEDS_STROBE
	#define PK_DBG PK_DBG_FUNC
	#define PK_VER PK_TRC_VERBOSE
	#define PK_ERR PK_ERROR
	#define logI PK_DBG_FUNC
	#define logE(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#else
	#define PK_DBG(a,...)
	#define PK_VER(a,...)
	#define PK_ERR(a,...)
#endif

/******************************************************************************
 * local variables
******************************************************************************/
static DEFINE_SPINLOCK(g_strobeSMPLock);
static u32 strobe_Res = 0;
static BOOL g_strobe_On = 0;
static int g_duty=0;
//static int g_step=-1;
static int g_timeOutTimeMs=0;

static struct work_struct workTimeOut;
/*****************************************************************************
Functions
*****************************************************************************/
static void work_timeOutFunc(struct work_struct *data);


extern U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT);

static int setReg6332(int gReg, int gRegV, int gRegM, int gRegSh)
{
    pmic_config_interface(gReg,gRegV,gRegM,gRegSh);
    logI("\nQQQS pmic_config_interface %d %d %d %d (0x%x 0x%x 0x%x 0x%x)\n",gReg, gRegV, gRegM, gRegSh,gReg, gRegV, gRegM, gRegSh);
    return 0;
}
/*
static int getReg6332(void)
{
    //int regV;
    //pmic_read_interface(gReg, &regV, 0xFFFF, 0x0);
    //logI("\nQQQR pmic_read_interface %d %d (0x%x 0x%x)\n",gReg, regV, gReg, regV);
    //return regV;
    return 0;
}*/



int FL_enable(void)
{
	PK_DBG("FL_enable");
	if(g_duty<=5)
	{
        setReg6332(0x80b4,	0x1,	0x1,	6);
        setReg6332(0x8086,	0x1,	0x1,	0);
        mdelay(1);
        setReg6332(0x8554,	0x12,	0xffff,	0);
        setReg6332(0x853a,	0x0,	0xffff,	0);
        setReg6332(0x8536,	0x0,	0xffff,	0);
        setReg6332(0x8532,	0x1c0,	0xffff,	0);
        setReg6332(0x80b2,	0x3ff,	0xffff,	0);
        setReg6332(0x854a,	0x0,	0xffff,	0);
        setReg6332(0x854a,	0x1,	0xffff,	0);
        setReg6332(0x854c,	0x1,	0xffff,	0);
        setReg6332(0x8542,	0x40,	0xffff,	0);
        setReg6332(0x854e,	0x8,	0xffff,	0);
        setReg6332(0x853e,	0x21,	0xffff,	0);
        setReg6332(0x854e,	0x0,	0xffff,	0);
        setReg6332(0x8088,	g_duty,	0x7,	0);
        setReg6332(0x8086,	0x1,	0x1,	1);
    }
    else
    {
        //code
        setReg6332(0x8d22,	0x1,	0x1,	12);
        setReg6332(0x8d14,	0x1,	0x1,	12);
        setReg6332(0x803c,	0x3,	0x3,	0);
        setReg6332(0x803c,	0x2,	0x3,	2);
        setReg6332(0x803c,	0x1,	0x1,	14);
        setReg6332(0x8036,	0x0,	0x1,	0);
        setReg6332(0x8d24,	0xf,	0xf,	12);
        setReg6332(0x8d16,	0x1,	0x1,	15);
        setReg6332(0x803a,	0x1,	0x1,	6);
        setReg6332(0x8046,	0xa0,	0xffff,	0);
        setReg6332(0x803e,	0x1,	0x1,	2);
        setReg6332(0x803e,	0x1,	0x1,	3);
        setReg6332(0x803e,	0x3,	0x3,	8);
        setReg6332(0x803e,	0x1,	0x1,	10);
        setReg6332(0x8044,	0x3,	0x3,	0);
        setReg6332(0x8044,	0x3,	0x3,	8);
        setReg6332(0x8044,	0x1,	0x1,	11);
        setReg6332(0x809c,	0x8000,	0xffff,	0);
        //read reg(0x809a);
        setReg6332(0x8084,	0x1,	0x1,	2);
        mdelay(50);
        //read reg(0x8060);
        //read reg(0x8062);
        //read reg(0x8178);
        //read reg(0x8179);
        setReg6332(0x808a,	g_duty-6,	0xf,	0);
        setReg6332(0x8086,	0x1,	0x1,	4);
    }



    return 0;
}

int FL_disable(void)
{
	PK_DBG("FL_disable");
	setReg6332(0x8086,	0, 1, 1);
	mdelay(1);
	setReg6332( 0x8086, 0, 1, 0);
    return 0;
}

int FL_dim_duty(kal_uint32 duty)
{

    g_duty = duty;

    return 0;
}

int FL_init(void)
{

    g_duty=0;


	FL_disable();
	INIT_WORK(&workTimeOut, work_timeOutFunc);
    return 0;
}

int FL_uninit(void)
{
	PK_DBG("FL_uninit");

	FL_disable();
    return 0;
}

/*****************************************************************************
User interface
*****************************************************************************/
static void work_timeOutFunc(struct work_struct *data)
{
	FL_disable();
    PK_DBG("ledTimeOut_callback\n");
    //printk(KERN_ALERT "work handler function./n");
}
enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
	PK_DBG("ledTimeOut_callback\n");
	schedule_work(&workTimeOut);

    return HRTIMER_NORESTART;
}
static struct hrtimer g_timeOutTimer;
void timerInit(void)
{
	g_timeOutTimeMs=1000; //1s
	hrtimer_init( &g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	g_timeOutTimer.function=ledTimeOutCallback;

}

static int constant_flashlight_ioctl(MUINT32 cmd, MUINT32 arg)
{
	int i4RetValue = 0;
	int iFlashType = (int)FLASHLIGHT_NONE;
	int ior;
	int iow;
	int iowr;
	ior = _IOR(FLASHLIGHT_MAGIC,0, int);
	iow = _IOW(FLASHLIGHT_MAGIC,0, int);
	iowr = _IOWR(FLASHLIGHT_MAGIC,0, int);
	PK_DBG("constant_flashlight_ioctl() line=%d cmd=%d, ior=%d, iow=%d iowr=%d arg=%d\n",__LINE__, cmd, ior, iow, iowr, arg);
	PK_DBG("constant_flashlight_ioctl() line=%d cmd-ior=%d, cmd-iow=%d cmd-iowr=%d arg=%d\n",__LINE__, cmd-ior, cmd-iow, cmd-iowr, arg);
    switch(cmd)
    {

		case FLASH_IOC_SET_TIME_OUT_TIME_MS:
			PK_DBG("FLASH_IOC_SET_TIME_OUT_TIME_MS: %d\n",arg);
			g_timeOutTimeMs=arg;
			break;

    	case FLASH_IOC_SET_DUTY :
    		PK_DBG("FLASHLIGHT_DUTY: %d\n",arg);
    		g_duty=arg;
    		FL_dim_duty(arg);
    		break;



    	case FLASH_IOC_SET_ONOFF :
    		PK_DBG("FLASHLIGHT_ONOFF: %d\n",arg);
    		if(arg==1)
    		{
				if(g_timeOutTimeMs!=0)
	            {
	            	ktime_t ktime;
					ktime = ktime_set( 0, g_timeOutTimeMs*1000000 );
					hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );
	            }
    			FL_enable();
    			g_strobe_On=1;
    		}
    		else
    		{
    			FL_disable();
				hrtimer_cancel( &g_timeOutTimer );
				g_strobe_On=0;
    		}
    		break;
        case FLASHLIGHTIOC_G_FLASHTYPE:
            iFlashType = FLASHLIGHT_LED_CONSTANT;
            if(copy_to_user((void __user *) arg , (void*)&iFlashType , _IOC_SIZE(cmd)))
            {
                PK_DBG("[strobe_ioctl] ioctl copy to user failed\n");
                return -EFAULT;
            }
            break;
		default :
    		PK_DBG(" No such command \n");
    		i4RetValue = -EPERM;
    		break;
    }
    return i4RetValue;
}

static int constant_flashlight_open(void *pArg)
{
    int i4RetValue = 0;
    PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

	if (0 == strobe_Res)
	{
	    FL_init();
		timerInit();
	}
	spin_lock_irq(&g_strobeSMPLock);

    if(strobe_Res)
    {
        PK_ERR(" busy!\n");
        i4RetValue = -EBUSY;
    }
    else
    {
        strobe_Res += 1;
    }

    spin_unlock_irq(&g_strobeSMPLock);

    return i4RetValue;

}

static int constant_flashlight_release(void *pArg)
{
    PK_DBG(" constant_flashlight_release\n");

    if (strobe_Res)
    {
        spin_lock_irq(&g_strobeSMPLock);

        strobe_Res = 0;

        /* LED On Status */
        g_strobe_On = FALSE;

        spin_unlock_irq(&g_strobeSMPLock);

    	FL_uninit();
    }

    PK_DBG(" Done\n");

    return 0;

}

FLASHLIGHT_FUNCTION_STRUCT	constantFlashlightFunc=
{
	constant_flashlight_open,
	constant_flashlight_release,
	constant_flashlight_ioctl
};

MUINT32 constantFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc != NULL)
    {
        *pfFunc = &constantFlashlightFunc;
    }
    return 0;
}

/* LED flash control for high current capture mode*/
ssize_t strobe_VDIrq(void)
{

    return 0;
}

EXPORT_SYMBOL(strobe_VDIrq);
