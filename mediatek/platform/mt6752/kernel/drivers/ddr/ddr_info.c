#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/xlog.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "mach/mt_reg_base.h"
#include "mach/emi_bwl.h"
#include "mach/sync_write.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <asm/setup.h>

static dev_t MEMPllinfo_dev;
static struct cdev MEMPllinfo_cdev;
static struct class *MEMPllinfo_class;
void __iomem *DDRPHY_BASE_ADDR = NULL;

static int MEMPll_open(struct inode *inode, struct file *filp);
static int MEMPll_release(struct inode *inode, struct file *filp);
static long MEMPll_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations MEMPllinfo_fops = {        
    .open = MEMPll_open,        
    .release = MEMPll_release,        
    .unlocked_ioctl   = MEMPll_ioctl,
    .owner = THIS_MODULE,
};

static int MEMPll_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int MEMPll_release(struct inode *inode, struct file *filp)
{        
    return 0;
}

/************************************************************************** 
*  DEV DRIVER IOCTL 
**************************************************************************/ 
static long MEMPll_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret   = 0;
    u32 data_read = 0;
    
    data_read = readl(IOMEM(DDRPHY_BASE_ADDR + 0x600));
    ret = copy_to_user((void __user *)arg, (void *)&(data_read), sizeof(u32));
    
    return 0;
}

static int __init MEMPllinfo_init(void)
{
    int ret = 0;
    struct device_node *node;

    xlog_printk(ANDROID_LOG_INFO, "MEMPll" ," init\n");  

    ret=alloc_chrdev_region(&MEMPllinfo_dev,  0, 1, "MEMPll");
    if (ret)    
	{        
	    xlog_printk(ANDROID_LOG_ERROR, "MEMPll" ,"[%s]alloc_chrdev_region  failed, ret:%d\n", "MEMPll", ret);
	    return ret;
    } 

    node = of_find_compatible_node(NULL, NULL, "mediatek,DDRPHY");
    if(node) {
        DDRPHY_BASE_ADDR = of_iomap(node, 0);
        printk("get DDRPHY_BASE_ADDR @ %p\n", DDRPHY_BASE_ADDR);
      } else {
        printk("can't find compatible node\n");
        return -1;
    }

	/*create class*/
	 MEMPllinfo_class = class_create(THIS_MODULE, "MEMPll");
	 if (IS_ERR(MEMPllinfo_class)) {
		ret = PTR_ERR(MEMPllinfo_class);
        xlog_printk(ANDROID_LOG_ERROR, "MEMPll" ,"[%s] register class failed, ret:%d\n", "MEMPll", ret);
	    return ret;
    }

	/* initialize the device structure and register the device  */
    cdev_init(&MEMPllinfo_cdev, &MEMPllinfo_fops);
    MEMPllinfo_cdev.owner = THIS_MODULE;
    if ((ret = cdev_add(&MEMPllinfo_cdev, MEMPllinfo_dev, 1)) < 0) 
    {
        xlog_printk(ANDROID_LOG_ERROR, "MEMPll" ,"[%s] could not allocate chrdev for the device, ret:%d\n",  ret);
        return ret;
    }
	
	/*create device node*/
	device_create(MEMPllinfo_class, NULL, MEMPllinfo_dev, NULL, "MEMPll");
		 
	return 0;
}

/*
 * MEMPllinfo_exit: module exit function.
 */
static void __exit MEMPllinfo_exit(void)
{

}

late_initcall(MEMPllinfo_init);
module_exit(MEMPllinfo_exit);


