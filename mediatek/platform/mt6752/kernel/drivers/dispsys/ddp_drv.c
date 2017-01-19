#define LOG_TAG "ddp_drv"

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <mach/mt_smi.h>
#include <linux/xlog.h>
#include <linux/proc_fs.h>  //proc file use
//ION
//#include <linux/ion.h>
//#include <linux/ion_drv.h>
//#include <mach/m4u.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <asm/io.h>
#include <mach/mt_smi.h>
#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h> // ????
#include <mach/mt_irq.h>
#include <mach/sync_write.h>
#include <mach/m4u.h>

#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_hal.h"
#include "ddp_log.h"
#include "ddp_irq.h"
#include "ddp_info.h"
#include "ddp_dpi_reg.h"

extern PDPI_REGS DPI_REG;
#define DISP_DEVNAME "DISPSYS"
// device and driver
static dev_t disp_devno;
static struct cdev *disp_cdev;
static struct class *disp_class = NULL;

typedef struct
{
    pid_t open_pid;
    pid_t open_tgid;
    struct list_head testList;
    spinlock_t node_lock;
} disp_node_struct;

static unsigned int ddp_ms2jiffies(unsigned long ms)
{
    return ((ms*HZ + 512) >> 10);
}

static long disp_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    disp_node_struct *pNode = (disp_node_struct *)file->private_data;

    return 0;        
}

static int disp_open(struct inode *inode, struct file *file)
{
    disp_node_struct *pNode = NULL;

    DDPDBG("enter disp_open() process:%s\n",current->comm);

    //Allocate and initialize private data
    file->private_data = kmalloc(sizeof(disp_node_struct) , GFP_ATOMIC);
    if(NULL == file->private_data)
    {
        DDPMSG("Not enough entry for DDP open operation\n");
        return -ENOMEM;
    }

    pNode = (disp_node_struct *)file->private_data;
    pNode->open_pid = current->pid;
    pNode->open_tgid = current->tgid;
    INIT_LIST_HEAD(&(pNode->testList));
    spin_lock_init(&pNode->node_lock);

    return 0;

}

static ssize_t disp_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
    return 0;
}

static int disp_release(struct inode *inode, struct file *file)
{
    disp_node_struct *pNode = NULL;
    unsigned int index = 0;
    DDPDBG("enter disp_release() process:%s\n",current->comm);

    pNode = (disp_node_struct *)file->private_data;

    spin_lock(&pNode->node_lock);

    spin_unlock(&pNode->node_lock);

    if(NULL != file->private_data)
    {
        kfree(file->private_data);
        file->private_data = NULL;
    }

    return 0;
}

static int disp_flush(struct file * file , fl_owner_t a_id)
{
    return 0;
}

// remap register to user space
// #define DISP_SVP_DEBUG  // FIXME: should disable before SVP MP
static int disp_mmap(struct file * file, struct vm_area_struct * a_pstVMArea)
{
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT) && (CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
	#ifdef DISP_SVP_DEBUG
	    a_pstVMArea->vm_page_prot = pgprot_noncached(a_pstVMArea->vm_page_prot);
	    if(remap_pfn_range(a_pstVMArea , 
	                 a_pstVMArea->vm_start , 
	                 a_pstVMArea->vm_pgoff , 
	                 (a_pstVMArea->vm_end - a_pstVMArea->vm_start) , 
	                 a_pstVMArea->vm_page_prot))
	    {
	        DDPMSG("MMAP failed!!\n");
	        return -1;
	    }
	#else
	    DDPERR("disp_mmap not supported! \n");
	#endif
#endif

    return 0;
}

struct dispsys_device
{
	void __iomem *regs[DISP_REG_NUM];
	struct device *dev;
	int irq[DISP_REG_NUM];
};
static struct dispsys_device *dispsys_dev;
static int nr_dispsys_dev = 0;
unsigned int dispsys_irq[DISP_REG_NUM] = {0};
unsigned long dispsys_reg[DISP_REG_NUM] = {0};
unsigned long mipi_tx_reg=0;
unsigned long dsi_reg_va = 0;

extern irqreturn_t disp_irq_handler(int irq, void *dev_id);
extern int ddp_path_init();
// from DTS, for debug
unsigned int ddp_reg_pa_base[DISP_REG_NUM] = {
0x14007000,0x14008000,0x14009000,0x1400A000,
0x1400B000,0x1400C000,0x1400D000,0x1400E000,
0x1400F000,0x14010000,0x14011000,0x14014000,
0x14018000,0x14015000,0x14012000,0x14013000,
0x14000000,0x14016000,0x14017000,0x10206000, 0x10000000, 0x100020F0, 0x10215000};
unsigned int ddp_irq_num[DISP_REG_NUM] = {
184,185,186,187,188,189,190,191,192,193,
194,0  ,198,177,195,196,197  ,0  ,0,  0, 0, 0, 0};

static int disp_is_intr_enable(DISP_REG_ENUM module)
{
    switch(module)
    {
        case DISP_REG_OVL0   :
        case DISP_REG_OVL1   :
        case DISP_REG_RDMA0  :
        case DISP_REG_RDMA1  :
        case DISP_REG_WDMA0  :
        case DISP_REG_MUTEX  :
        case DISP_REG_DSI0   :  
        case DISP_REG_AAL    :
        case DISP_REG_CONFIG :
            return 1;            
        case DISP_REG_WDMA1  :  //FIXME: WDMA1 intr is abonrmal FPGA so mark first, enable after EVB works
        case DISP_REG_COLOR  :
        case DISP_REG_CCORR  :
        case DISP_REG_GAMMA  :
        case DISP_REG_DITHER :
        case DISP_REG_UFOE   :
        case DISP_REG_PWM    :
        case DISP_REG_DPI0   :
        case DISP_REG_SMI_LARB0 :
        case DISP_REG_SMI_COMMON:
        case DISP_REG_CONFIG2:
        case DISP_REG_CONFIG3:
		case DISP_REG_IO_DRIVING:
		case DISP_REG_MIPI:
            return 0;
        default: return 0;
    }
}

extern int m4u_enable_tf(int port, bool fgenable);
m4u_callback_ret_t disp_m4u_callback(int port, unsigned int mva, void* data)
{
    DISP_MODULE_ENUM module = DISP_MODULE_OVL0;
    DDPERR("fault call port=%d, mva=0x%x, data=0x%x ", port, mva, (int)data);
    switch(port)
    {
		case M4U_PORT_DISP_OVL0 : module = DISP_MODULE_OVL0; break;
		case M4U_PORT_DISP_RDMA0: module = DISP_MODULE_RDMA0; break;
		case M4U_PORT_DISP_WDMA0: module = DISP_MODULE_WDMA0; break;
		case M4U_PORT_DISP_OVL1 : module = DISP_MODULE_OVL1; break;
		case M4U_PORT_DISP_RDMA1: module = DISP_MODULE_RDMA1; break;
		case M4U_PORT_DISP_WDMA1: module = DISP_MODULE_WDMA1; break;
		default: DDPERR("unknown port=%d \n", port);
    }
    ddp_dump_analysis(module);
    ddp_dump_reg(module);
    m4u_enable_tf(port, 0);  // disable translation fault after it happens to avoid prinkt too much issues(log is override)
}

static int disp_probe(struct platform_device *pdev)
{
    struct class_device;
	int ret;
	int i;
    int new_count;
    static unsigned int disp_probe_cnt = 0;

    if(disp_probe_cnt!=0)
    {
        return 0;
    }
    
    new_count = nr_dispsys_dev + 1;
    dispsys_dev = krealloc(dispsys_dev, 
        sizeof(struct dispsys_device) * new_count, GFP_KERNEL);
    if (!dispsys_dev) {
        DDPERR("Unable to allocate dispsys_dev\n");
        return -ENOMEM;
    }
	
    dispsys_dev = &(dispsys_dev[nr_dispsys_dev]);
    dispsys_dev->dev = &pdev->dev;

    /* iomap registers and irq*/
    for(i=0;i<DISP_REG_NUM;i++)
    {
        dispsys_dev->regs[i] = of_iomap(pdev->dev.of_node, i);
        if (!dispsys_dev->regs[i]) {
            DDPERR("Unable to ioremap registers, of_iomap fail, i=%d \n", i);
            return -ENOMEM;
        }
		dispsys_reg[i] = dispsys_dev->regs[i];
        /* get IRQ ID and request IRQ */
        if(i<DISP_REG_CONFIG && (i!=DISP_REG_PWM))
        {
            dispsys_dev->irq[i] = irq_of_parse_and_map(pdev->dev.of_node, i);
            dispsys_irq[i] = dispsys_dev->irq[i];
            if(disp_is_intr_enable(i)==1)
            {
                ret = request_irq(dispsys_dev->irq[i], (irq_handler_t)disp_irq_handler, 
                            IRQF_TRIGGER_NONE, DISP_DEVNAME,  NULL);  // IRQF_TRIGGER_NONE dose not take effect here, real trigger mode set in dts file
                if (ret) {
                    DDPERR("Unable to request IRQ, request_irq fail, i=%d, irq=%d \n", i, dispsys_dev->irq[i]);
                    return ret;
                }
            }            
        }
        DDPMSG("DT, i=%d, module=%s, map_addr=0x%lx, map_irq=%d, reg_pa=0x%x, irq=%d \n", 
            i, ddp_get_reg_module_name(i), dispsys_dev->regs[i], dispsys_dev->irq[i], ddp_reg_pa_base[i], ddp_irq_num[i]);
    }    
    nr_dispsys_dev = new_count;
    //mipi tx reg map here
	dsi_reg_va = dispsys_reg[DISP_REG_DSI0];
    mipi_tx_reg = dispsys_reg[DISP_REG_MIPI];
	DPI_REG = (PDPI_REGS)dispsys_reg[DISP_REG_DPI0];

	////// power on MMSYS for early porting
#ifdef FPGA_EARLY_PORTING
	printk("[DISP Probe] power MMSYS:0x%lx,0x%lx\n",DISP_REG_CONFIG_MMSYS_CG_CLR0,DISP_REG_CONFIG_MMSYS_CG_CLR1);
	DISP_REG_SET(0,DISP_REG_CONFIG_MMSYS_CG_CLR0,0xFFFFFFFF);
	DISP_REG_SET(0,DISP_REG_CONFIG_MMSYS_CG_CLR1,0xFFFFFFFF);
	DISP_REG_SET(0,DISPSYS_CONFIG_BASE + 0xC04,0x1C000);//fpga should set this register
#endif
	//////

	// init arrays
	ddp_path_init();

    // init M4U callback
    DDPMSG("register m4u callback\n");
    m4u_register_fault_callback(M4U_PORT_DISP_OVL0, disp_m4u_callback, 0);
    m4u_register_fault_callback(M4U_PORT_DISP_RDMA0, disp_m4u_callback, 0);
    m4u_register_fault_callback(M4U_PORT_DISP_WDMA0, disp_m4u_callback, 0);
    m4u_register_fault_callback(M4U_PORT_DISP_OVL1, disp_m4u_callback, 0);
    m4u_register_fault_callback(M4U_PORT_DISP_RDMA1, disp_m4u_callback, 0);
    m4u_register_fault_callback(M4U_PORT_DISP_WDMA1, disp_m4u_callback, 0);
    

	DDPMSG("dispsys probe done.\n");	
	//NOT_REFERENCED(class_dev);

	// bus hang issue error intr enable
	// when MMSYS clock off but GPU/MJC/PWM clock on, avoid display hang and trigger error intr
	{
        DISP_REG_SET_FIELD(0, MMSYS_TO_MFG_TX_ERROR, DISP_REG_CONFIG_MMSYS_INTEN, 1);
        DISP_REG_SET_FIELD(0, MMSYS_TO_MJC_TX_ERROR, DISP_REG_CONFIG_MMSYS_INTEN, 1);
        DISP_REG_SET_FIELD(0, PWM0_APB_TX_ERROR, DISP_REG_CONFIG_MMSYS_INTEN, 1);

        DISP_REG_SET_FIELD(0, MFG_APB_TX_CON_FLD_MFG_APB_COUNTER_EN, DISP_REG_CONFIG_MFG_APB_TX_CON, 1);
        DISP_REG_SET_FIELD(0, MJC_APB_TX_CON_FLD_MJC_APB_COUNTER_EN, DISP_REG_CONFIG_MJC_APB_TX_CON, 1);
	}
	
	return 0;
}

static int disp_remove(struct platform_device *pdev)
{
    return 0;
}

static void disp_shutdown(struct platform_device *pdev)
{
	/* Nothing yet */
}


/* PM suspend */
static int disp_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    printk("\n\n==== DISP suspend is called ====\n");


    return 0;
}

/* PM resume */
static int disp_resume(struct platform_device *pdev)
{
    printk("\n\n==== DISP resume is called ====\n");


    return 0;
}

/* Kernel interface */
static struct file_operations disp_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = disp_unlocked_ioctl,
	.open		= disp_open,
	.release	= disp_release,
	.flush		= disp_flush,
	.read       = disp_read,
	.mmap       = disp_mmap
};

static const struct of_device_id dispsys_of_ids[] = {
	{ .compatible = "mediatek,DISPSYS", },
	{}
};

static struct platform_driver dispsys_of_driver = {
	.driver = {
		.name = DISP_DEVNAME,
		.owner = THIS_MODULE,
		.of_match_table = dispsys_of_ids,
	},
	.probe    = disp_probe,
	.remove   = disp_remove,
    .shutdown = disp_shutdown,
    .suspend  = disp_suspend,
    .resume   = disp_resume,
};

static int __init disp_init(void)
{
    int ret = 0;

    DDPMSG("Register the disp driver\n");
    if(platform_driver_register(&dispsys_of_driver))
    {
        DDPERR("failed to register disp driver\n");
        //platform_device_unregister(&disp_device);
        ret = -ENODEV;
        return ret;
    }

    return 0;
}

static void __exit disp_exit(void)
{
    cdev_del(disp_cdev);
    unregister_chrdev_region(disp_devno, 1);

    platform_driver_unregister(&dispsys_of_driver);

    device_destroy(disp_class, disp_devno);
    class_destroy(disp_class);

}

extern void disp_rdma_enable_preultra(unsigned int enable);
// called before enable SODI
void disp_sodi_enable()
{
    disp_rdma_enable_preultra(1);
}

// called before disable SODI
void disp_sodi_disable()
{
    disp_rdma_enable_preultra(0);

}

module_init(disp_init);
module_exit(disp_exit);
MODULE_AUTHOR("Tzu-Meng, Chung <Tzu-Meng.Chung@mediatek.com>");
MODULE_DESCRIPTION("Display subsystem Driver");
MODULE_LICENSE("GPL");
