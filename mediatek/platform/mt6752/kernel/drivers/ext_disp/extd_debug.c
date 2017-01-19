#if defined(MTK_HDMI_SUPPORT)
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <cust_gpio_usage.h>

#include "ddp_hal.h"
#include "ddp_reg.h"
#include "ddp_info.h"

#include "extd_drv.h"

void DBG_Init(void); 
void DBG_Deinit(void);

extern void hdmi_log_enable(int enable);
extern void hdmi_cable_fake_plug_in(void);
extern void hdmi_cable_fake_plug_out(void);
extern void hdmi_mmp_enable(int enable);
extern void hdmi_hwc_enable(int enable);

extern void hdmi_power_on(void);
extern void hdmi_power_off();
extern void hdmi_suspend();
extern void hdmi_resume();

extern int ext_disp_diagnose();
extern DDP_MODULE_DRIVER  *ddp_modules_driver[DISP_MODULE_NUM];
extern int I2S_Enable;

// ---------------------------------------------------------------------------
//  External variable declarations
// ---------------------------------------------------------------------------

//extern LCM_DRIVER *lcm_drv;
// ---------------------------------------------------------------------------
//  Debug Options
// ---------------------------------------------------------------------------

static char STR_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > hdmi\n"
    "\n"
    "ACTION\n"
    "        hdmitx:[on|off]\n"
    "             enable hdmi video output\n"
    "\n";

extern void hdmi_log_enable(int enable);
// TODO: this is a temp debug solution
static void process_dbg_opt(const char *opt)
{
    if (0)
    {

    }
    else if (0 == strncmp(opt, "on", 2))
    {
        hdmi_power_on();
    }
    else if (0 == strncmp(opt, "off", 3))
    {
        hdmi_power_off();
    }
    else if (0 == strncmp(opt, "suspend", 7))
    {
        hdmi_suspend();
    }
    else if (0 == strncmp(opt, "resume", 6))
    {
        hdmi_resume();
    }
    else if (0 == strncmp(opt, "colorbar", 8))
    {

    }
    else if (0 == strncmp(opt, "ldooff", 6))
    {

    }
    else if (0 == strncmp(opt, "log:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2))
        {
            hdmi_log_enable(true);
        }
        else if (0 == strncmp(opt + 4, "off", 3))
        {
            hdmi_log_enable(false);
        }
        else
        {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "fakecablein:", 12))
    {
        if (0 == strncmp(opt + 12, "enable", 6))
        {
            hdmi_cable_fake_plug_in();
        }
        else if (0 == strncmp(opt + 12, "disable", 7))
        {
            hdmi_cable_fake_plug_out();
        }
        else
        {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "hdmimmp:", 8))
    {
        if (0 == strncmp(opt + 8, "on", 2))
        {
            hdmi_mmp_enable(1);
        }
        else if (0 == strncmp(opt + 8, "off", 3))
        {
            hdmi_mmp_enable(0);
        }
        else if (0 == strncmp(opt + 8, "img", 3))
        {
            hdmi_mmp_enable(7);
        }
        else
        {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "hdmireg", 7))
    {
        ext_disp_diagnose();
    }
    else if (0 == strncmp(opt, "enablehwc:", 10))
    {
        if (0 == strncmp(opt + 10, "on", 2))
        {
            hdmi_hwc_enable(1);
        }
        else if (0 == strncmp(opt + 10, "off", 3))
        {
            hdmi_hwc_enable(0);
        }
    }
    else if (0 == strncmp(opt, "I2S1:", 5))
    {
#ifdef GPIO_MHL_I2S_OUT_WS_PIN
        if (0 == strncmp(opt + 5, "on", 2))
        {
            printk("[hdmi][Debug] Enable I2S1 \n");
            mt_set_gpio_mode(GPIO_MHL_I2S_OUT_WS_PIN, GPIO_MODE_01);
            mt_set_gpio_mode(GPIO_MHL_I2S_OUT_CK_PIN, GPIO_MODE_01);
            mt_set_gpio_mode(GPIO_MHL_I2S_OUT_DAT_PIN, GPIO_MODE_01);
        }
        else if (0 == strncmp(opt + 5, "off", 3))
        {
            printk("[hdmi][Debug] Disable I2S1 \n");
            mt_set_gpio_mode(GPIO_MHL_I2S_OUT_WS_PIN, GPIO_MODE_02);
            mt_set_gpio_mode(GPIO_MHL_I2S_OUT_CK_PIN, GPIO_MODE_01);
            mt_set_gpio_mode(GPIO_MHL_I2S_OUT_DAT_PIN, GPIO_MODE_02);
        }
#endif
    }
    else if (0 == strncmp(opt, "I2SEnable:", 10))
    {
        if (0 == strncmp(opt + 10, "1", 1))
        {
            I2S_Enable = 1;
        }
        else if (0 == strncmp(opt + 10, "0", 1))
        {
            I2S_Enable = 0;
        }
    }
    else if (0 == strncmp(opt, "DPI_IO_DRIVING:", 15))
    {
        if (0 == strncmp(opt + 15, "4", 1))
        {
            MASKREG32(DISPSYS_IO_DRIVING, 0x3C00, 0x0000);
        }
        else if (0 == strncmp(opt + 15, "8", 1))
        {
            MASKREG32(DISPSYS_IO_DRIVING, 0x3C00, 0x1400);
        }
        else if (0 == strncmp(opt + 15, "12", 2))
        {
            MASKREG32(DISPSYS_IO_DRIVING, 0x3C00, 0x2800);
        }
        else if (0 == strncmp(opt + 15, "16", 2))
        {
            MASKREG32(DISPSYS_IO_DRIVING, 0x3C00, 0x3C00);
        }
    }
    else if (0 == strncmp(opt, "DPI_DUMP:", 9))
    {
        if (ddp_modules_driver[DISP_MODULE_DPI]->dump_info != NULL)
        {
            ddp_modules_driver[DISP_MODULE_DPI]->dump_info(DISP_MODULE_DPI, 1);
        }
    }
    else if (0 == strncmp(opt, "forceon", 7))
    {
        hdmi_force_on(false);
    }
    else
    {
        goto Error;
    }

    return;

Error:
    printk("[hdmitx] parse command error!\n\n%s", STR_HELP);
}

static void process_dbg_cmd(char *cmd)
{
    char *tok;

    printk("[hdmitx] %s\n", cmd);

    while ((tok = strsep(&cmd, " ")) != NULL)
    {
        process_dbg_opt(tok);
    }
}

// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

struct dentry *hdmitx_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}


static char debug_buffer[2048];

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
    int n = 0;

    n += scnprintf(debug_buffer + n, debug_bufmax - n, STR_HELP);
    debug_buffer[n++] = 0;

    return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, n);
}


static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
    size_t ret;

    ret = count;

    if (count > debug_bufmax)
    {
        count = debug_bufmax;
    }

    if (copy_from_user(&debug_buffer, ubuf, count))
    {
        return -EFAULT;
    }

    debug_buffer[count] = 0;

    process_dbg_cmd(debug_buffer);

    return ret;
}


static struct file_operations debug_fops =
{
    .read  = debug_read,
    .write = debug_write,
    .open  = debug_open,
};


void HDMI_DBG_Init(void)
{
    hdmitx_dbgfs = debugfs_create_file("hdmi",
                                       S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);
}


void HDMI_DBG_Deinit(void)
{
    debugfs_remove(hdmitx_dbgfs);
}

#endif
