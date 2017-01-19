/**
 * @file    mt_cpufreq.c
 * @brief   Driver for CPU DVFS
 *
 */

#define __MT_CPUFREQ_C__

/*=============================================================*/
/* Include files                                               */
/*=============================================================*/

/* system includes */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <linux/jiffies.h>
#include <linux/bitops.h>
#include <linux/uaccess.h>
#include <linux/aee.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_address.h>
#endif

#include <asm/system.h>


/* project includes */
#include "mach/mt_typedefs.h"
#include "mach/irqs.h"
#include "mach/mt_irq.h"
#include "mach/mt_thermal.h"
#include "mach/mt_spm_idle.h"
#include "mach/mt_pmic_wrap.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_freqhopping.h"
#include "mach/mt_ptp.h"
// TODO: disable to avoid build error
//#include "mach/mt_static_power.h"
#include "mach/upmu_sw.h"
#include "mach/mtk_rtc_hal.h"
#include "mach/mt_rtc_hw.h"
#include "mach/mt_hotplug_strategy.h"

#ifndef __KERNEL__
#include "freqhop_sw.h"
#include "mt_spm.h"
#include "mt6311.h"
#else
#include "mach/mt_spm.h"
#include "mach/mt6311.h"
#endif

/* local includes */
#include "mach/mt_cpufreq.h"

/* forward references */
extern int is_ext_buck_sw_ready(void);
extern int is_ext_buck_exist(void);
extern void mt6311_set_vdvfs11_vosel(kal_uint8 val);
extern void mt6311_set_vdvfs11_vosel_on(kal_uint8 val);
extern void mt6311_set_vdvfs11_vosel_ctrl(kal_uint8 val);
extern kal_uint32 mt6311_read_byte(kal_uint8 cmd, kal_uint8 *returnData);
extern void mt6311_set_buck_test_mode(kal_uint8 val);
extern unsigned int get_pmic_mt6325_cid(void);

extern u32 get_devinfo_with_index(u32 index);
//extern int mtktscpu_get_Tj_temp(void); // TODO: ask Jerry to provide the head file
extern void (*cpufreq_freq_check)(enum mt_cpu_dvfs_id id);

// Freq Meter API
#ifdef __KERNEL__
extern unsigned int mt_get_cpu_freq(void);
#endif

/*=============================================================*/
/* Macro definition                                            */
/*=============================================================*/

/*
 * CONFIG
 */
#define CONFIG_CPU_DVFS_SHOWLOG 1

//#define CONFIG_CPU_DVFS_BRINGUP 1               /* for bring up */
//#define CONFIG_CPU_DVFS_FFTT_TEST 1             /* FF TT SS volt test */
//#define CONFIG_CPU_DVFS_DOWNGRADE_FREQ 1        /* downgrade freq */
#define CONFIG_CPU_DVFS_POWER_THROTTLING  1     /* power throttling features */
// #define CONFIG_CPU_DVFS_RAMP_DOWN 1            /* ramp down to slow down freq change */

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) >= (b) ? (b) : (a))

/* used @ set_cur_volt_extBuck() */
//#define MIN_DIFF_VSRAM_PROC        1000   /* 10mv * 100 */
#define NORMAL_DIFF_VRSAM_VPROC    10000   /* 100mv * 100 */ 
#define MAX_DIFF_VSRAM_VPROC       20000  /* 200mv * 100 */ 
#define MIN_VSRAM_VOLT             93125  /* 931.25mv * 100 */
#define MAX_VSRAM_VOLT             115000  /* 1150mv * 100 */
#define MAX_VPROC_VOLT             115000  /* 1150mv * 100 */

 /* PMIC/PLL settle time (us), should not be changed */
#define PMIC_CMD_DELAY_TIME     5
#define MIN_PMIC_SETTLE_TIME    25
#define PMIC_VOLT_UP_SETTLE_TIME(old_volt, new_volt)    (((((new_volt) - (old_volt)) + 1250 - 1) / 1250) + PMIC_CMD_DELAY_TIME)
#define PMIC_VOLT_DOWN_SETTLE_TIME(old_volt, new_volt)    (((((old_volt) - (new_volt)) * 2)  / 625) + PMIC_CMD_DELAY_TIME)
#define PLL_SETTLE_TIME         (20)

#define RAMP_DOWN_TIMES         (2)             /* RAMP DOWN TIMES to postpone frequency degrade */
#define CPUFREQ_BOUNDARY_FOR_FHCTL   (CPU_DVFS_FREQ7)       /* if cross 1170MHz when DFS, don't used FHCTL */

#define DEFAULT_VOLT_VSRAM      (105000)
#define DEFAULT_VOLT_VPROC      (100000)
#define DEFAULT_VOLT_VGPU       (100000)
#define DEFAULT_VOLT_VCORE      (100000)
#define DEFAULT_VOLT_VLTE       (100000)

/* for DVFS OPP table */
#define CPU_DVFS_FREQ0   (1994000) /* KHz */
#define CPU_DVFS_FREQ1   (1794000) /* KHz */
#define CPU_DVFS_FREQ2   (1690000) /* KHz */
#define CPU_DVFS_FREQ3   (1612000) /* KHz */
#define CPU_DVFS_FREQ4   (1560000) /* KHz */
#define CPU_DVFS_FREQ4_2 (1495000) /* KHz */
#define CPU_DVFS_FREQ5   (1417000) /* KHz */
#define CPU_DVFS_FREQ6   (1287000) /* KHz */
#define CPU_DVFS_FREQ7   (1170000) /* KHz */
#define CPU_DVFS_FREQ8   (936000)  /* KHz */
#define CPU_DVFS_FREQ9   (702000)  /* KHz */
#define CPU_DVFS_FREQ10  (468000)  /* KHz */
#define CPUFREQ_LAST_FREQ_LEVEL    (CPU_DVFS_FREQ10)


#ifdef CONFIG_CPU_DVFS_POWER_THROTTLING
#define PWR_THRO_MODE_LBAT_936MHZ	BIT(0)
#define PWR_THRO_MODE_BAT_PER_936MHZ	BIT(1)
#define PWR_THRO_MODE_BAT_OC_1170MHZ	BIT(2)
#define PWR_THRO_MODE_BAT_OC_1287MHZ	BIT(3)
#define PWR_THRO_MODE_BAT_OC_1417MHZ	BIT(4)
#endif

/*
 * LOG and Test
 */
#ifndef __KERNEL__ // for CTP
#define USING_XLOG
#else
//#define USING_XLOG
#endif

#define HEX_FMT "0x%08x"
#undef TAG

#ifdef USING_XLOG
#include <linux/xlog.h>

#define TAG     "Power/cpufreq"

#define cpufreq_err(fmt, args...)       \
    xlog_printk(ANDROID_LOG_ERROR, TAG, "[ERROR]"fmt, ##args)
#define cpufreq_warn(fmt, args...)      \
    xlog_printk(ANDROID_LOG_WARN, TAG, "[WARNING]"fmt, ##args)
#define cpufreq_info(fmt, args...)      \
    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
#define cpufreq_dbg(fmt, args...)       \
    xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define cpufreq_ver(fmt, args...)       \
    do {                                \
        if (func_lv_mask)               \
            xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args);  \
    } while (0)

#else   /* USING_XLOG */

#define TAG     "[Power/cpufreq] "

#define cpufreq_err(fmt, args...)       \
    printk(KERN_ERR TAG KERN_CONT "[ERROR]"fmt, ##args)
#define cpufreq_warn(fmt, args...)      \
    printk(KERN_WARNING TAG KERN_CONT "[WARNING]"fmt, ##args)
#define cpufreq_info(fmt, args...)      \
    printk(KERN_NOTICE TAG KERN_CONT fmt, ##args)
#define cpufreq_dbg(fmt, args...)       \
    printk(KERN_INFO TAG KERN_CONT fmt, ##args)
#define cpufreq_ver(fmt, args...)       \
    do {                                \
        if (func_lv_mask)               \
            printk(KERN_DEBUG TAG KERN_CONT fmt, ##args);    \
    } while (0)    

#endif  /* USING_XLOG */

#define FUNC_LV_MODULE         BIT(0)  /* module, platform driver interface */
#define FUNC_LV_CPUFREQ        BIT(1)  /* cpufreq driver interface          */
#define FUNC_LV_API                BIT(2)  /* mt_cpufreq driver global function */
#define FUNC_LV_LOCAL            BIT(3)  /* mt_cpufreq driver local function  */
#define FUNC_LV_HELP              BIT(4)  /* mt_cpufreq driver help function   */

//static unsigned int func_lv_mask = (FUNC_LV_MODULE | FUNC_LV_CPUFREQ | FUNC_LV_API | FUNC_LV_LOCAL | FUNC_LV_HELP);
static unsigned int func_lv_mask = 0;
static unsigned int do_dvfs_stress_test = 0;

#ifdef CONFIG_CPU_DVFS_SHOWLOG
#define FUNC_ENTER(lv)          do { if ((lv) & func_lv_mask) cpufreq_dbg(">> %s()\n", __func__); } while (0)
#define FUNC_EXIT(lv)           do { if ((lv) & func_lv_mask) cpufreq_dbg("<< %s():%d\n", __func__, __LINE__); } while (0)
#else
#define FUNC_ENTER(lv)
#define FUNC_EXIT(lv)
#endif /* CONFIG_CPU_DVFS_SHOWLOG */

/*
 * BIT Operation
 */
#define _BIT_(_bit_)                    (unsigned)(1 << (_bit_))
#define _BITS_(_bits_, _val_)           ((((unsigned) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1)) & ((_val_)<<((0) ? _bits_)))
#define _BITMASK_(_bits_)               (((unsigned) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1))
#define _GET_BITS_VAL_(_bits_, _val_)   (((_val_) & (_BITMASK_(_bits_))) >> ((0) ? _bits_))

/*
 * REG ACCESS
 */
#define cpufreq_read(addr)                  DRV_Reg32(addr)
#define cpufreq_write(addr, val)            mt_reg_sync_writel(val, addr)
#define cpufreq_write_mask(addr, mask, val) cpufreq_write(addr, (cpufreq_read(addr) & ~(_BITMASK_(mask))) | _BITS_(mask, val))


/*=============================================================*/
/* Local type definition                                       */
/*=============================================================*/


/*=============================================================*/
/* Local variable definition                                   */
/*=============================================================*/


/*=============================================================*/
/* Local function definition                                   */
/*=============================================================*/


/*=============================================================*/
/* Gobal function definition                                   */
/*=============================================================*/

/*
 * LOCK
 */
#if 0   /* spinlock */ // TODO: FIXME, it would cause warning @ big because of i2c access with atomic operation
static DEFINE_SPINLOCK(cpufreq_lock);
#define cpufreq_lock(flags) spin_lock_irqsave(&cpufreq_lock, flags)
#define cpufreq_unlock(flags) spin_unlock_irqrestore(&cpufreq_lock, flags)
#else   /* mutex */
static DEFINE_MUTEX(cpufreq_mutex);
bool is_in_cpufreq = 0;
#define cpufreq_lock(flags) \
    do { \
        /* to fix compile warning */  \
        flags = (unsigned long)&flags; \
        mutex_lock(&cpufreq_mutex); \
        is_in_cpufreq = 1;\
        spm_mcdi_wakeup_all_cores();\
    } while (0)

#define cpufreq_unlock(flags) \
    do { \
        /* to fix compile warning */  \
        flags = (unsigned long)&flags; \
        is_in_cpufreq = 0;\
        mutex_unlock(&cpufreq_mutex); \
    } while (0)
#endif

/*
 * EFUSE
 */
#define CPUFREQ_EFUSE_INDEX     (3)
#define FUNC_CODE_EFUSE_INDEX	(28)

#define CPU_LEVEL_0             (0x0)
#define CPU_LEVEL_1             (0x1)
#define CPU_LEVEL_2             (0x2)
#define CPU_LEVEL_3             (0x3)

#define CPU_LV_TO_OPP_IDX(lv)   ((lv)) /* cpu_level to opp_idx */
unsigned int AllowTurboMode = 0;

#ifdef __KERNEL__
static unsigned int _mt_cpufreq_get_cpu_level(void)
{
    unsigned int lv = 0;
    unsigned int func_code_0 = _GET_BITS_VAL_(27 : 24, get_devinfo_with_index(FUNC_CODE_EFUSE_INDEX));
    unsigned int func_code_1 = _GET_BITS_VAL_(31 : 28, get_devinfo_with_index(FUNC_CODE_EFUSE_INDEX));

    cpufreq_info("from efuse: function code 0 = 0x%x, function code 1 = 0x%x\n", func_code_0, func_code_1);

    /* get CPU clock-frequency from DT */
#ifdef CONFIG_OF
    {
        struct device_node *node = of_find_node_by_type(NULL, "cpu");
        unsigned int cpu_speed = 0;
		unsigned int cpu_speed_bounding = _GET_BITS_VAL_(3 : 0, get_devinfo_with_index(CPUFREQ_EFUSE_INDEX));

		switch (cpu_speed_bounding) {
			case 0:
			case 1:
			case 2:
				AllowTurboMode = 1; /* 1.69 * 1.1 = 1.859G */
				break;

			default:
				AllowTurboMode = 0;
				break;
		}
		cpufreq_info("current CPU efuse is %d, AllowTurboMode=%d\n", cpu_speed_bounding, AllowTurboMode);

        if (!of_property_read_u32(node, "clock-frequency", &cpu_speed))
            cpu_speed = cpu_speed / 1000 / 1000;    // MHz
        else {
            cpufreq_err("@%s: missing clock-frequency property, use default CPU level\n", __func__);
            return CPU_LEVEL_1;
        }

        cpufreq_info("CPU clock-frequency from DT = %d MHz\n", cpu_speed);

        if (cpu_speed >= 1700)
            lv = CPU_LEVEL_1;   // 1.7G
        else if (cpu_speed >= 1500)
            lv = CPU_LEVEL_2;   // 1.5G
        else if (cpu_speed >= 1300)
            lv = CPU_LEVEL_3;   // 1.3G
        else {
            cpufreq_err("No suitable DVFS table, set to default CPU level! clock-frequency=%d\n", cpu_speed);
            lv = CPU_LEVEL_1;
        }
    }
#else   /* CONFIG_OF */
    /* no DT, we should check efuse for CPU speed HW bounding */
    {
        unsigned int cpu_speed_bounding = _GET_BITS_VAL_(3 : 0, get_devinfo_with_index(CPUFREQ_EFUSE_INDEX));

        cpufreq_info("No DT, get CPU frequency bounding from efuse = %x\n", cpu_speed_bounding);

        switch (cpu_speed_bounding) {
            case 0:
            case 1:
            case 2:
				AllowTurboMode = 1; /* 1.69 * 1.1 = 1.859G */
            case 3:
            case 4:
                lv = CPU_LEVEL_1;   // 1.7G
                break;
            case 5:
            case 6:
                lv = CPU_LEVEL_2;   // 1.5G
                break;
            case 7:
            case 8:
                lv = CPU_LEVEL_3;   // 1.3G
                break;
            default:
                cpufreq_err("No suitable DVFS table, set to default CPU level! efuse=0x%x\n", cpu_speed_bounding);
                lv = CPU_LEVEL_1;
                break;
        }
		cpufreq_info("current CPU efuse is %d, AllowTurboMode=%d\n", cpu_speed_bounding, AllowTurboMode);
    }
#endif

    return lv;
}
#else
static unsigned int _mt_cpufreq_get_cpu_level(void)
{
    return CPU_LEVEL_1;
}
#endif

/*
 * PMIC_WRAP
 */
// TODO: defined @ pmic head file???
#define VOLT_TO_PMIC_VAL(volt)  (((volt) - 60000 + 625 - 1) / 625) //((((volt) - 700 * 100 + 625 - 1) / 625)
#define PMIC_VAL_TO_VOLT(pmic)  (((pmic) * 625) + 60000) //(((pmic) * 625) / 100 + 700)

#define VOLT_TO_EXTBUCK_VAL(volt)   VOLT_TO_PMIC_VAL(volt)//(((((volt) - 300) + 9) / 10) & 0x7F)
#define EXTBUCK_VAL_TO_VOLT(val)    PMIC_VAL_TO_VOLT(val)//(300 + ((val) & 0x7F) * 10)

/* PMIC WRAP ADDR */ // TODO: include other head file
#ifdef CONFIG_OF
extern void __iomem *pwrap_base;
#define PWRAP_BASE_ADDR     ((unsigned int)pwrap_base)
#else
#include "mach/mt_reg_base.h"
#define PWRAP_BASE_ADDR     PWRAP_BASE
#endif
#define PMIC_WRAP_DVFS_ADR0     (PWRAP_BASE_ADDR + 0x0E8)
#define PMIC_WRAP_DVFS_WDATA0   (PWRAP_BASE_ADDR + 0x0EC)
#define PMIC_WRAP_DVFS_ADR1     (PWRAP_BASE_ADDR + 0x0F0)
#define PMIC_WRAP_DVFS_WDATA1   (PWRAP_BASE_ADDR + 0x0F4)
#define PMIC_WRAP_DVFS_ADR2     (PWRAP_BASE_ADDR + 0x0F8)
#define PMIC_WRAP_DVFS_WDATA2   (PWRAP_BASE_ADDR + 0x0FC)
#define PMIC_WRAP_DVFS_ADR3     (PWRAP_BASE_ADDR + 0x100)
#define PMIC_WRAP_DVFS_WDATA3   (PWRAP_BASE_ADDR + 0x104)
#define PMIC_WRAP_DVFS_ADR4     (PWRAP_BASE_ADDR + 0x108)
#define PMIC_WRAP_DVFS_WDATA4   (PWRAP_BASE_ADDR + 0x10C)
#define PMIC_WRAP_DVFS_ADR5     (PWRAP_BASE_ADDR + 0x110)
#define PMIC_WRAP_DVFS_WDATA5   (PWRAP_BASE_ADDR + 0x114)
#define PMIC_WRAP_DVFS_ADR6     (PWRAP_BASE_ADDR + 0x118)
#define PMIC_WRAP_DVFS_WDATA6   (PWRAP_BASE_ADDR + 0x11C)
#define PMIC_WRAP_DVFS_ADR7     (PWRAP_BASE_ADDR + 0x120)
#define PMIC_WRAP_DVFS_WDATA7   (PWRAP_BASE_ADDR + 0x124)

/* PMIC ADDR */ // TODO: include other head file
#define PMIC_ADDR_VPROC_VOSEL_ON     0x04BA  /* [6:0]                    */
#define PMIC_ADDR_VPROC_VOSEL        0x04B8  /* [6:0]                    */
#define PMIC_ADDR_VPROC_VOSEL_CTRL   0x04B0  /* [1]                      */
#define PMIC_ADDR_VPROC_EN           0x04B4  /* [0] (shared with others) */
#define PMIC_ADDR_VSRAM_VOSEL_ON     0x0506  /* [6:0]                    */
#define PMIC_ADDR_VSRAM_VOSEL        0x0A58  /* [15:9]                   */
#define PMIC_ADDR_VSRAM_VOSEL_CTRL   0x04FC  /* [1]                      */
#define PMIC_ADDR_VSRAM_EN           0x0A34  /* [1] (shared with others) */
#define PMIC_ADDR_VSRAM_FAST_TRSN_EN 0x0A62  /* [8]                      */
#define PMIC_ADDR_VGPU_VOSEL_ON      0x0618  /* [6:0]                    */
#define PMIC_ADDR_VCORE_VOSEL_ON     0x0664  /* [6:0]                    */
#define PMIC_ADDR_VLTE_VOSEL_ON      0x063E  /* [6:0]                    */

#define NR_PMIC_WRAP_CMD 8 /* num of pmic wrap cmd (fixed value) */

struct pmic_wrap_cmd{
        unsigned int cmd_addr;
        unsigned int cmd_wdata;
};

struct pmic_wrap_setting {
    enum pmic_wrap_phase_id phase;
    struct pmic_wrap_cmd addr[NR_PMIC_WRAP_CMD];
    struct {
        struct {
            unsigned int cmd_addr;
            unsigned int cmd_wdata;
        } _[NR_PMIC_WRAP_CMD];
        const int nr_idx;
    } set[NR_PMIC_WRAP_PHASE];
};

static struct pmic_wrap_setting pw = {
    .phase = NR_PMIC_WRAP_PHASE, /* invalid setting for init */
#if 0
    .addr = {
        { PMIC_WRAP_DVFS_ADR0, PMIC_WRAP_DVFS_WDATA0, },
        { PMIC_WRAP_DVFS_ADR1, PMIC_WRAP_DVFS_WDATA1, },
        { PMIC_WRAP_DVFS_ADR2, PMIC_WRAP_DVFS_WDATA2, },
        { PMIC_WRAP_DVFS_ADR3, PMIC_WRAP_DVFS_WDATA3, },
        { PMIC_WRAP_DVFS_ADR4, PMIC_WRAP_DVFS_WDATA4, },
        { PMIC_WRAP_DVFS_ADR5, PMIC_WRAP_DVFS_WDATA5, },
        { PMIC_WRAP_DVFS_ADR6, PMIC_WRAP_DVFS_WDATA6, },
        { PMIC_WRAP_DVFS_ADR7, PMIC_WRAP_DVFS_WDATA7, },
    },
#else
    .addr = { {0, 0} },
#endif
    .set[PMIC_WRAP_PHASE_NORMAL] = {
        ._[IDX_NM_VSRAM]          = { PMIC_ADDR_VSRAM_VOSEL_ON, VOLT_TO_PMIC_VAL(DEFAULT_VOLT_VSRAM), },
        ._[IDX_NM_VPROC]          = { PMIC_ADDR_VPROC_VOSEL_ON, VOLT_TO_PMIC_VAL(DEFAULT_VOLT_VPROC), },
        ._[IDX_NM_VGPU]           = { PMIC_ADDR_VGPU_VOSEL_ON,  VOLT_TO_PMIC_VAL(DEFAULT_VOLT_VGPU),  },
        ._[IDX_NM_VCORE]          = { PMIC_ADDR_VCORE_VOSEL_ON, VOLT_TO_PMIC_VAL(DEFAULT_VOLT_VCORE), },
        ._[IDX_NM_VLTE]           = { PMIC_ADDR_VLTE_VOSEL_ON,  VOLT_TO_PMIC_VAL(DEFAULT_VOLT_VLTE),  },
        .nr_idx = NR_IDX_NM,
    },

    .set[PMIC_WRAP_PHASE_SUSPEND] = {
        ._[IDX_SP_VPROC_PWR_ON]     = { PMIC_ADDR_VPROC_EN,         _BITS_(0  : 0,  1),       },
        ._[IDX_SP_VPROC_SHUTDOWN]   = { PMIC_ADDR_VPROC_EN,         _BITS_(0  : 0,  0),       },
        ._[IDX_SP_VSRAM_PWR_ON]     = { PMIC_ADDR_VSRAM_EN,         _BITS_(1  : 1,  1),       },
        ._[IDX_SP_VSRAM_SHUTDOWN]   = { PMIC_ADDR_VSRAM_EN,         _BITS_(1  : 1,  0),       }, 
        ._[IDX_SP_VCORE_NORMAL]     = { PMIC_ADDR_VCORE_VOSEL_ON,   VOLT_TO_PMIC_VAL(100000), }, 
        ._[IDX_SP_VCORE_SLEEP]      = { PMIC_ADDR_VCORE_VOSEL_ON,   VOLT_TO_PMIC_VAL(90000),  },
        ._[IDX_SP_VCORE_TEMP1]      = { PMIC_ADDR_VCORE_VOSEL_ON,   VOLT_TO_PMIC_VAL(96250),  },
        ._[IDX_SP_VCORE_TEMP2]      = { PMIC_ADDR_VCORE_VOSEL_ON,   VOLT_TO_PMIC_VAL(93125),  }, 
        .nr_idx = NR_IDX_SP,
    },

    .set[PMIC_WRAP_PHASE_DEEPIDLE] = {
        ._[IDX_DI_VCORE_TEMP1]         = { PMIC_ADDR_VCORE_VOSEL_ON,     VOLT_TO_PMIC_VAL(96250),  },
        ._[IDX_DI_VCORE_TEMP2]         = { PMIC_ADDR_VCORE_VOSEL_ON,     VOLT_TO_PMIC_VAL(93125),  }, 
        ._[IDX_DI_VSRAM_NORMAL]        = { PMIC_ADDR_VSRAM_VOSEL_CTRL,   _BITS_(1  : 1,  1),	   },
        ._[IDX_DI_VSRAM_SLEEP]	       = { PMIC_ADDR_VSRAM_VOSEL_CTRL,   _BITS_(1  : 1,  0),	   },
        ._[IDX_DI_VCORE_NORMAL]        = { PMIC_ADDR_VCORE_VOSEL_ON,     VOLT_TO_PMIC_VAL(100000), },
        ._[IDX_DI_VCORE_SLEEP]         = { PMIC_ADDR_VCORE_VOSEL_ON,     VOLT_TO_PMIC_VAL(90000),  },
        ._[IDX_DI_VSRAM_FAST_TRSN_DIS] = { PMIC_ADDR_VSRAM_FAST_TRSN_EN, _BITS_(8  : 8,  1),	   },
        ._[IDX_DI_VSRAM_FAST_TRSN_EN]  = { PMIC_ADDR_VSRAM_FAST_TRSN_EN, _BITS_(8  : 8,  0),	   },
        .nr_idx = NR_IDX_DI,
    },
};

#if 0   /* spinlock */
static DEFINE_SPINLOCK(pmic_wrap_lock);
#define pmic_wrap_lock(flags) spin_lock_irqsave(&pmic_wrap_lock, flags)
#define pmic_wrap_unlock(flags) spin_unlock_irqrestore(&pmic_wrap_lock, flags)
#else   /* mutex */
static DEFINE_MUTEX(pmic_wrap_mutex);

#define pmic_wrap_lock(flags) \
    do { \
        /* to fix compile warning */  \
        flags = (unsigned long)&flags; \
        mutex_lock(&pmic_wrap_mutex); \
    } while (0)

#define pmic_wrap_unlock(flags) \
    do { \
        /* to fix compile warning */  \
        flags = (unsigned long)&flags; \
        mutex_unlock(&pmic_wrap_mutex); \
    } while (0)
#endif

static int _spm_dvfs_ctrl_volt(u32 value)
{
#define MAX_RETRY_COUNT (100)

    u32 ap_dvfs_con;
    int retry = 0;

    FUNC_ENTER(FUNC_LV_HELP);

    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0)); // TODO: FIXME

    ap_dvfs_con = spm_read(SPM_AP_DVFS_CON_SET);
    spm_write(SPM_AP_DVFS_CON_SET, (ap_dvfs_con & ~(0x7)) | value);
    udelay(5);

    while ((spm_read(SPM_AP_DVFS_CON_SET) & (0x1 << 31)) == 0) {
        if (retry >= MAX_RETRY_COUNT) {
            cpufreq_err("FAIL: no response from PMIC wrapper\n");
            return -1;
        }

        retry++;
        //cpufreq_dbg("wait for ACK signal from PMIC wrapper, retry = %d\n", retry);

        udelay(5);
    }

    FUNC_EXIT(FUNC_LV_HELP);

    return 0;
}

void _mt_cpufreq_pmic_table_init(void)
{
    struct pmic_wrap_cmd pwrap_cmd_default[NR_PMIC_WRAP_CMD] = {
        { PMIC_WRAP_DVFS_ADR0, PMIC_WRAP_DVFS_WDATA0, },
        { PMIC_WRAP_DVFS_ADR1, PMIC_WRAP_DVFS_WDATA1, },
        { PMIC_WRAP_DVFS_ADR2, PMIC_WRAP_DVFS_WDATA2, },
        { PMIC_WRAP_DVFS_ADR3, PMIC_WRAP_DVFS_WDATA3, },
        { PMIC_WRAP_DVFS_ADR4, PMIC_WRAP_DVFS_WDATA4, },
        { PMIC_WRAP_DVFS_ADR5, PMIC_WRAP_DVFS_WDATA5, },
        { PMIC_WRAP_DVFS_ADR6, PMIC_WRAP_DVFS_WDATA6, },
        { PMIC_WRAP_DVFS_ADR7, PMIC_WRAP_DVFS_WDATA7, },
    };

    FUNC_ENTER(FUNC_LV_HELP);

    memcpy(pw.addr, pwrap_cmd_default, sizeof(pwrap_cmd_default));    

    FUNC_EXIT(FUNC_LV_HELP);
}

void mt_cpufreq_set_pmic_phase(enum pmic_wrap_phase_id phase)
{
    int i;
    unsigned long flags;

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(phase >= NR_PMIC_WRAP_PHASE);

#if 0   // TODO: FIXME, check IPO-H case

    if (pw.phase == phase)
        return;

#endif

    if (pw.addr[0].cmd_addr == 0) {
        cpufreq_warn("pmic table not initialized\n");
        _mt_cpufreq_pmic_table_init();
    }

    pmic_wrap_lock(flags);

    pw.phase = phase;

    for (i = 0; i < pw.set[phase].nr_idx; i++) {
        cpufreq_write(pw.addr[i].cmd_addr, pw.set[phase]._[i].cmd_addr);
        cpufreq_write(pw.addr[i].cmd_wdata, pw.set[phase]._[i].cmd_wdata);
    }

    pmic_wrap_unlock(flags);

    FUNC_EXIT(FUNC_LV_API);
}
EXPORT_SYMBOL(mt_cpufreq_set_pmic_phase);

void mt_cpufreq_set_pmic_cmd(enum pmic_wrap_phase_id phase, int idx, unsigned int cmd_wdata) /* just set wdata value */
{
    unsigned long flags;

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(phase >= NR_PMIC_WRAP_PHASE);
    BUG_ON(idx >= pw.set[phase].nr_idx);

    //cpufreq_dbg("@%s: phase = 0x%x, idx = %d, cmd_wdata = 0x%x\n", __func__, phase, idx, cmd_wdata);

    pmic_wrap_lock(flags);

    pw.set[phase]._[idx].cmd_wdata = cmd_wdata;

    if (pw.phase == phase)
        cpufreq_write(pw.addr[idx].cmd_wdata, cmd_wdata);

    pmic_wrap_unlock(flags);

    FUNC_EXIT(FUNC_LV_API);
}
EXPORT_SYMBOL(mt_cpufreq_set_pmic_cmd);

void mt_cpufreq_apply_pmic_cmd(int idx) /* kick spm */
{
    unsigned long flags;

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(idx >= pw.set[pw.phase].nr_idx);

    //cpufreq_dbg("@%s: idx = %d\n", __func__, idx);

    pmic_wrap_lock(flags);

    _spm_dvfs_ctrl_volt(idx);

    pmic_wrap_unlock(flags);

    FUNC_EXIT(FUNC_LV_API);
}
EXPORT_SYMBOL(mt_cpufreq_apply_pmic_cmd);

/* cpu voltage sampler */
static cpuVoltsampler_func g_pCpuVoltSampler = NULL;

void mt_cpufreq_setvolt_registerCB(cpuVoltsampler_func pCB)
{
    g_pCpuVoltSampler = pCB;
}
EXPORT_SYMBOL(mt_cpufreq_setvolt_registerCB);

/* SDIO */
unsigned int mt_get_cur_volt_vcore_ao(void)
{
    unsigned int rdata;

    FUNC_ENTER(FUNC_LV_LOCAL);

    pwrap_read(PMIC_ADDR_VCORE_VOSEL_ON, &rdata);

    rdata = PMIC_VAL_TO_VOLT(rdata);

    FUNC_EXIT(FUNC_LV_LOCAL);

    return rdata; /* 10 uV */
}

#if 0
unsigned int mt_get_cur_volt_vcore_pdn(void)
{
    unsigned int rdata;

    FUNC_ENTER(FUNC_LV_LOCAL);

    pwrap_read(PMIC_ADDR_VCORE_PDN_VOSEL_ON, &rdata);

    rdata = PMIC_VAL_TO_VOLT(rdata) / 100;

    FUNC_EXIT(FUNC_LV_LOCAL);

    return rdata; /* mv: vproc */
}
#endif

void mt_vcore_dvfs_disable_by_sdio(unsigned int type, bool disabled)
{
    // empty function
}

void mt_vcore_dvfs_volt_set_by_sdio(unsigned int volt) /* unit: mv x 1000 */
{
    unsigned int cur_pmic_val_vcore_ao;
    //unsigned int cur_pmic_val_vcore_pdn;
    unsigned int target_pmic_val_vcore_ao;
    int step;

    FUNC_ENTER(FUNC_LV_LOCAL);

    pwrap_read(PMIC_ADDR_VCORE_VOSEL_ON, &cur_pmic_val_vcore_ao);
    //pwrap_read(PMIC_ADDR_VCORE_PDN_VOSEL_ON, &cur_pmic_val_vcore_pdn);
    target_pmic_val_vcore_ao = VOLT_TO_PMIC_VAL(volt / 10); // mv * 100

    step = (target_pmic_val_vcore_ao > cur_pmic_val_vcore_ao) ? 1 : -1;

    while (target_pmic_val_vcore_ao != cur_pmic_val_vcore_ao) {
        cur_pmic_val_vcore_ao += step;
        //cur_pmic_val_vcore_pdn += step;

        mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_NORMAL, IDX_NM_VCORE, cur_pmic_val_vcore_ao);
        //mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_NORMAL, IDX_NM_VCORE_PDN, cur_pmic_val_vcore_pdn);

        mt_cpufreq_apply_pmic_cmd(IDX_NM_VCORE);
        //mt_cpufreq_apply_pmic_cmd(IDX_NM_VCORE_PDN);

        udelay(4);
    }

    FUNC_EXIT(FUNC_LV_LOCAL);
}

unsigned int mt_vcore_dvfs_volt_get_by_sdio(void) /* unit: mv x 1000 */
{
    return mt_get_cur_volt_vcore_ao() * 10;
}

/*
 * mt_cpufreq driver
 */
 #define OP(khz, volt) {            \
    .cpufreq_khz = khz,             \
    .cpufreq_volt = volt,           \
    .cpufreq_volt_org = volt,       \
}

#define for_each_cpu_dvfs(i, p)        for (i = 0, p = cpu_dvfs; i < NR_MT_CPU_DVFS; i++, p = &cpu_dvfs[i])
#define cpu_dvfs_is(p, id)                 (p == &cpu_dvfs[id])
#define cpu_dvfs_is_availiable(p)      (p->opp_tbl)
#define cpu_dvfs_get_name(p)         (p->name)

#define cpu_dvfs_get_cur_freq(p)                (p->opp_tbl[p->idx_opp_tbl].cpufreq_khz)
#define cpu_dvfs_get_freq_by_idx(p, idx)        (p->opp_tbl[idx].cpufreq_khz)
#define cpu_dvfs_get_max_freq(p)                (p->opp_tbl[0].cpufreq_khz)
#define cpu_dvfs_get_normal_max_freq(p)         (p->opp_tbl[p->idx_normal_max_opp].cpufreq_khz)
#define cpu_dvfs_get_min_freq(p)                (p->opp_tbl[p->nr_opp_tbl - 1].cpufreq_khz)

#define cpu_dvfs_get_cur_volt(p)                (p->opp_tbl[p->idx_opp_tbl].cpufreq_volt)
#define cpu_dvfs_get_volt_by_idx(p, idx)        (p->opp_tbl[idx].cpufreq_volt)

#define cpu_dvfs_is_extbuck_valid()     (is_ext_buck_exist() && is_ext_buck_sw_ready())


struct mt_cpu_freq_info {
    const unsigned int cpufreq_khz;
    unsigned int cpufreq_volt;  // mv * 100
    const unsigned int cpufreq_volt_org;    // mv * 100
};

struct mt_cpu_power_info {
    unsigned int cpufreq_khz;
    unsigned int cpufreq_ncpu;
    unsigned int cpufreq_power;
};

struct mt_cpu_dvfs {
    const char *name;
    unsigned int cpu_id;                    /* for cpufreq */
    unsigned int cpu_level;
    struct mt_cpu_dvfs_ops *ops;

    /* opp (freq) table */
    struct mt_cpu_freq_info *opp_tbl;       /* OPP table */
    int nr_opp_tbl;                         /* size for OPP table */
    int idx_opp_tbl;                        /* current OPP idx */
    int idx_normal_max_opp;                 /* idx for normal max OPP */
    int idx_opp_tbl_for_late_resume;	/* keep the setting for late resume */

    struct cpufreq_frequency_table *freq_tbl_for_cpufreq; /* freq table for cpufreq */

    /* power table */
    struct mt_cpu_power_info *power_tbl;
    unsigned int nr_power_tbl;

    /* enable/disable DVFS function */
    int dvfs_disable_count;
    bool dvfs_disable_by_ptpod;
    bool dvfs_disable_by_suspend;
    bool dvfs_disable_by_early_suspend;
    bool dvfs_disable_by_procfs;

    /* limit for thermal */
    unsigned int limited_max_ncpu;
    unsigned int limited_max_freq;
    unsigned int idx_opp_tbl_for_thermal_thro;
    unsigned int thermal_protect_limited_power;

    /* limit for HEVC (via. sysfs) */
    unsigned int limited_freq_by_hevc;

    /* for ramp down */
    int ramp_down_count;
    int ramp_down_count_const;

    /* param for micro throttling */
    bool downgrade_freq_for_ptpod;

    int over_max_cpu;
    int ptpod_temperature_limit_1;
    int ptpod_temperature_limit_2;
    int ptpod_temperature_time_1;
    int ptpod_temperature_time_2;

    int pre_online_cpu;
    unsigned int pre_freq;
    unsigned int downgrade_freq;

    unsigned int downgrade_freq_counter;
    unsigned int downgrade_freq_counter_return;

    unsigned int downgrade_freq_counter_limit;
    unsigned int downgrade_freq_counter_return_limit;

    /* turbo mode */
    unsigned int turbo_mode;

    /* power throttling */
#ifdef CONFIG_CPU_DVFS_POWER_THROTTLING
    int idx_opp_tbl_for_pwr_thro;           /* keep the setting for power throttling */
    int idx_pwr_thro_max_opp;               /* idx for power throttle max OPP */
    unsigned int pwr_thro_mode;
#endif
    
};

struct mt_cpu_dvfs_ops {
    /* for thermal */
    void (*protect)(struct mt_cpu_dvfs *p, unsigned int limited_power);      /* set power limit by thermal */ // TODO: sync with mt_cpufreq_thermal_protect()
    unsigned int (*get_temp)(struct mt_cpu_dvfs *p);                         /* return temperature         */ // TODO: necessary???
    int (*setup_power_table)(struct mt_cpu_dvfs *p);

    /* for freq change (PLL/MUX) */
    unsigned int (*get_cur_phy_freq)(struct mt_cpu_dvfs *p);                 /* return (physical) freq (KHz) */
    void (*set_cur_freq)(struct mt_cpu_dvfs *p, unsigned int cur_khz, unsigned int target_khz); /* set freq  */

    /* for volt change (PMICWRAP/extBuck) */
    unsigned int (*get_cur_volt)(struct mt_cpu_dvfs *p);             /* return volt (mV * 100) */
    int (*set_cur_volt)(struct mt_cpu_dvfs *p, unsigned int volt);   /* set volt (mv * 100), return 0 (success), -1 (fail) */
};


/* for thermal */
static int setup_power_table(struct mt_cpu_dvfs *p);

/* for freq change (PLL/MUX) */
static unsigned int get_cur_phy_freq(struct mt_cpu_dvfs *p);
static void set_cur_freq(struct mt_cpu_dvfs *p, unsigned int cur_khz, unsigned int target_khz);

/* for volt change (PMICWRAP/extBuck) */
static unsigned int get_cur_volt_pmic_wrap(struct mt_cpu_dvfs *p);
static int set_cur_volt_pmic_wrap(struct mt_cpu_dvfs *p, unsigned int volt); // volt: mv * 100
static unsigned int get_cur_volt_extbuck(struct mt_cpu_dvfs *p);
static int set_cur_volt_extbuck(struct mt_cpu_dvfs *p, unsigned int volt); // volt: mv * 100

static unsigned int max_cpu_num = 8; /* for limited_max_ncpu, it will be modified at driver initialization stage if needed */

static struct mt_cpu_dvfs_ops dvfs_ops_pmic_wrap = {
    .setup_power_table = setup_power_table,

    .get_cur_phy_freq = get_cur_phy_freq,
    .set_cur_freq = set_cur_freq,

    .get_cur_volt = get_cur_volt_pmic_wrap,
    .set_cur_volt = set_cur_volt_pmic_wrap,
};

static struct mt_cpu_dvfs_ops dvfs_ops_extbuck = {
    .setup_power_table = setup_power_table,

    .get_cur_phy_freq = get_cur_phy_freq,
    .set_cur_freq = set_cur_freq,

    .get_cur_volt = get_cur_volt_extbuck,
    .set_cur_volt = set_cur_volt_extbuck,
};


static struct mt_cpu_dvfs cpu_dvfs[] = {
    [MT_CPU_DVFS_LITTLE]    = {
        .name                           = __stringify(MT_CPU_DVFS_LITTLE),
        .cpu_id                         = MT_CPU_DVFS_LITTLE, // TODO: FIXME
        .cpu_level                    = CPU_LEVEL_1,  // 1.7GHz
        .ops                            = &dvfs_ops_extbuck,

        // TODO: check the following settings
        .over_max_cpu                   = 8, // 4
        .ptpod_temperature_limit_1      = 110000,
        .ptpod_temperature_limit_2      = 120000,
        .ptpod_temperature_time_1       = 1,
        .ptpod_temperature_time_2       = 4,
        .pre_online_cpu                 = 0,
        .pre_freq                       = 0,
        .downgrade_freq                 = 0,
        .downgrade_freq_counter         = 0,
        .downgrade_freq_counter_return  = 0,
        .downgrade_freq_counter_limit   = 0,
        .downgrade_freq_counter_return_limit = 0,

        .ramp_down_count_const		= RAMP_DOWN_TIMES,

        .turbo_mode			= 0,
#ifdef CONFIG_CPU_DVFS_POWER_THROTTLING
        .idx_opp_tbl_for_pwr_thro	= -1,
        .idx_pwr_thro_max_opp = 0,
#endif
    },
};


static struct mt_cpu_dvfs *id_to_cpu_dvfs(enum mt_cpu_dvfs_id id)
{
    return (id < NR_MT_CPU_DVFS) ? &cpu_dvfs[id] : NULL;
}


/* DVFS OPP table */
/* Notice: Each table MUST has 8 element to avoid ptpod error */

#define NR_MAX_OPP_TBL  8   
#define NR_MAX_CPU      8

/* CPU LEVEL 0, 2GHz segment (useless) */
static struct mt_cpu_freq_info opp_tbl_e1_0[] = {
    OP(CPU_DVFS_FREQ0,  110000),
    OP(CPU_DVFS_FREQ1,  106875),
    OP(CPU_DVFS_FREQ3,  103750),
    OP(CPU_DVFS_FREQ5,  100000),
    OP(CPU_DVFS_FREQ7,  95625),
    OP(CPU_DVFS_FREQ8,  91875),
    OP(CPU_DVFS_FREQ9,  87500),
    OP(CPU_DVFS_FREQ10, 83125),
};

/* CPU LEVEL 1, 1.7GHz segment */
static struct mt_cpu_freq_info opp_tbl_e1_1[] = {
    OP(CPU_DVFS_FREQ2,  110000),
    OP(CPU_DVFS_FREQ4,  107500),
    OP(CPU_DVFS_FREQ5,  104375),
    OP(CPU_DVFS_FREQ6,  101875),
    OP(CPU_DVFS_FREQ7,  99375),
    OP(CPU_DVFS_FREQ8,  93750),
    OP(CPU_DVFS_FREQ9,  88750),
    OP(CPU_DVFS_FREQ10, 83750),
};

//#define CPU_TURBO_15_TO_17

/* CPU LEVEL 2, 1.5GHz segment */

#ifdef CPU_TURBO_15_TO_17

/*by Danil_e71  freq up to 1.7 GHz !TEST!*/
static struct mt_cpu_freq_info opp_tbl_e1_2[] = {
    OP(CPU_DVFS_FREQ2,  110000),
    OP(CPU_DVFS_FREQ4,  107500),
    OP(CPU_DVFS_FREQ5,  104375),
    OP(CPU_DVFS_FREQ6,  101875),
    OP(CPU_DVFS_FREQ7,  99375),
    OP(CPU_DVFS_FREQ8,  93750),
    OP(CPU_DVFS_FREQ9,  88750),
    OP(CPU_DVFS_FREQ10, 83750),
};
#else

// default

static struct mt_cpu_freq_info opp_tbl_e1_2[] = {
    OP(CPU_DVFS_FREQ4_2, 106250),
    OP(CPU_DVFS_FREQ5,   104375),
    OP(CPU_DVFS_FREQ6,   101875),
    OP(CPU_DVFS_FREQ7,   99375),
    OP(CPU_DVFS_FREQ8,   93750),
    OP(CPU_DVFS_FREQ9,   88750),
    OP(CPU_DVFS_FREQ10,  83750),
    OP(CPU_DVFS_FREQ10,  83750),
};
#endif

/* CPU LEVEL 3, 1.3GHz segment */
static struct mt_cpu_freq_info opp_tbl_e1_3[] = {
    OP(CPU_DVFS_FREQ6,  101875),
    OP(CPU_DVFS_FREQ7,  99375),
    OP(CPU_DVFS_FREQ8,  93750),
    OP(CPU_DVFS_FREQ9,  88750),
    OP(CPU_DVFS_FREQ10, 83750),
    OP(CPU_DVFS_FREQ10, 83750),    
    OP(CPU_DVFS_FREQ10, 83750),
    OP(CPU_DVFS_FREQ10, 83750),    
};

struct opp_tbl_info {
    struct mt_cpu_freq_info *const opp_tbl;
    const int size;
};

#define ARRAY_AND_SIZE(x) (x), ARRAY_SIZE(x)

static struct opp_tbl_info opp_tbls[] = {
    [CPU_LV_TO_OPP_IDX(CPU_LEVEL_0)] = { ARRAY_AND_SIZE(opp_tbl_e1_0), },
    [CPU_LV_TO_OPP_IDX(CPU_LEVEL_1)] = { ARRAY_AND_SIZE(opp_tbl_e1_1), },
    [CPU_LV_TO_OPP_IDX(CPU_LEVEL_2)] = { ARRAY_AND_SIZE(opp_tbl_e1_2), },
    [CPU_LV_TO_OPP_IDX(CPU_LEVEL_3)] = { ARRAY_AND_SIZE(opp_tbl_e1_3), },
};

/* for freq change (PLL/MUX) */
#define PLL_FREQ_STEP		(13000)		/* KHz */

// #define PLL_MAX_FREQ		(1989000)	/* KHz */ // TODO: check max freq
#define PLL_MIN_FREQ		(130000)	/* KHz */
#define PLL_DIV1_FREQ		(1001000)	/* KHz */
#define PLL_DIV2_FREQ		(520000)	/* KHz */
#define PLL_DIV4_FREQ		(260000)	/* KHz */
#define PLL_DIV8_FREQ		(PLL_MIN_FREQ)	/* KHz */

#define DDS_DIV1_FREQ		(0x0009A000)	/* 1001MHz */
#define DDS_DIV2_FREQ		(0x010A0000)	/* 520MHz  */
#define DDS_DIV4_FREQ		(0x020A0000)	/* 260MHz  */
#define DDS_DIV8_FREQ		(0x030A0000)	/* 130MHz  */

/* for turbo mode */
#define TURBO_MODE_BOUNDARY_CPU_NUM	2

/* idx sort by temp from low to high */
enum turbo_mode {
    TURBO_MODE_2,
    TURBO_MODE_1,
    TURBO_MODE_NONE,

    NR_TURBO_MODE,
};

/* idx sort by temp from low to high */
struct turbo_mode_cfg {
    int temp;       /* degree x 1000 */
    int freq_delta; /* percentage    */
    int volt_delta; /* mv * 100       */
} turbo_mode_cfg[] = {
    [TURBO_MODE_2] = {
        .temp = 65000,
        .freq_delta = 10,
        .volt_delta = 4000,
    },
    [TURBO_MODE_1] = {
        .temp = 85000,
        .freq_delta = 5,
        .volt_delta = 2000,
    },
    [TURBO_MODE_NONE] = {
        .temp = 125000,
        .freq_delta = 0,
        .volt_delta = 0,
    },
};

#define TURBO_MODE_FREQ(mode, freq) (((freq * (100 + turbo_mode_cfg[mode].freq_delta)) / PLL_FREQ_STEP) / 100 * PLL_FREQ_STEP)
#define TURBO_MODE_VOLT(mode, volt) (volt + turbo_mode_cfg[mode].volt_delta)

static unsigned int num_online_cpus_delta = 0;
static bool is_in_turbo_mode = false; 

static enum turbo_mode get_turbo_mode(struct mt_cpu_dvfs *p, unsigned int target_khz)
{
    enum turbo_mode mode = TURBO_MODE_NONE;
    int temp = tscpu_get_temp_by_bank(THERMAL_BANK0);    // bank0 for CPU
    unsigned int online_cpus = num_online_cpus() + num_online_cpus_delta;
    int i;

    if (p->turbo_mode
            && target_khz == cpu_dvfs_get_freq_by_idx(p, 0)
            && online_cpus <= TURBO_MODE_BOUNDARY_CPU_NUM
       ) {
        for (i = 0; i < NR_TURBO_MODE; i++) {
            if (temp < turbo_mode_cfg[i].temp) {
                mode = i;
                break;
            }
        }
    }

    // enter turbo mode, SW workaround here
    if (mode < TURBO_MODE_NONE && is_in_turbo_mode == false) {
        // Make sure Vproc is lower than 1.155V
        if (TURBO_MODE_VOLT(mode, cpu_dvfs_get_volt_by_idx(p, 0)) > MAX_VPROC_VOLT)
            mode = TURBO_MODE_NONE;
        else {
            //cpufreq_dbg("@%s: turbo mode start\n", __func__);
            
            if (cpu_dvfs_is_extbuck_valid()) {
                // Remove 6311 voltage HB temporarily for turbo mode
                mt6311_set_buck_test_mode(0x1);
            } else {
                // 0x462[3] = 1'b1, force 2 phase
                pmic_config_interface(0x462, 0x1, 0x1, 0x3);
                // 0x466[15:14] = 2'b11, FPWM at VDVFS1 and VDVFS2
                pmic_config_interface(0x466, 0x3, 0x3, 0xE);
            }
            is_in_turbo_mode = true;
        }
    }

    cpufreq_ver("%s(), mode = %d, temp = %d, target_khz = %d (%d), num_online_cpus = %d\n", 
                __func__,
                mode,
                temp,
                target_khz,
                TURBO_MODE_FREQ(mode, target_khz),
                online_cpus
                ); // <-XXX

    return mode;
}

/* for PTP-OD */
static int _set_cur_volt_locked(struct mt_cpu_dvfs *p, unsigned int volt)  // volt: mv * 100
{
    int ret = -1;

    FUNC_ENTER(FUNC_LV_HELP);

    BUG_ON(NULL == p);

    if (!cpu_dvfs_is_availiable(p)) {
        FUNC_EXIT(FUNC_LV_HELP);
        return 0;
    }

#if 0 
    /* update for deep idle */
    mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_DEEPIDLE,
#ifdef MTK_FORCE_CLUSTER1
                IDX_DI_VSRAM_CA15L_NORMAL,
#else
                IDX_DI_VPROC_CA7_NORMAL,
#endif
                VOLT_TO_PMIC_VAL(mv + (cpu_dvfs_is(p, MT_CPU_DVFS_LITTLE) ? 0 : NORMAL_DIFF_VRSAM_VPROC))
                );

    /* update for suspend */
    if (   cpu_dvfs_is(p, MT_CPU_DVFS_BIG)
            && pw.set[PMIC_WRAP_PHASE_SUSPEND]._[IDX_SP_VSRAM_CA15L_PWR_ON].cmd_addr == PMIC_ADDR_VSRAM_CA15L_VOSEL_ON
       )
        mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_SUSPEND,
                IDX_SP_VSRAM_CA15L_PWR_ON,
                VOLT_TO_PMIC_VAL(volt + NORMAL_DIFF_VRSAM_VPROC)
                );
#endif

    /* set volt */
    ret = p->ops->set_cur_volt(p, volt);

    FUNC_EXIT(FUNC_LV_HELP);

    return ret;
}

static int _restore_default_volt(struct mt_cpu_dvfs *p)
{
    unsigned long flags;
    int i;
    int ret = -1;

    FUNC_ENTER(FUNC_LV_HELP);

    BUG_ON(NULL == p);

    if (!cpu_dvfs_is_availiable(p)) {
        FUNC_EXIT(FUNC_LV_HELP);
        return 0;
    }

    cpufreq_lock(flags);

    /* restore to default volt */
    for (i = 0; i < p->nr_opp_tbl; i++)
        p->opp_tbl[i].cpufreq_volt = p->opp_tbl[i].cpufreq_volt_org;

    /* set volt */
    ret = _set_cur_volt_locked(p,
		   TURBO_MODE_VOLT(get_turbo_mode(p, cpu_dvfs_get_cur_freq(p)),
   		   cpu_dvfs_get_cur_volt(p)
  		   )
   );

    cpufreq_unlock(flags);

    FUNC_EXIT(FUNC_LV_HELP);

    return ret;
}

unsigned int mt_cpufreq_get_freq_by_idx(enum mt_cpu_dvfs_id id, int idx)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(NULL == p);

    if (!cpu_dvfs_is_availiable(p)) {
        FUNC_EXIT(FUNC_LV_API);
        return 0;
    }

    BUG_ON(idx >= p->nr_opp_tbl);

    FUNC_EXIT(FUNC_LV_API);

    return cpu_dvfs_get_freq_by_idx(p, idx);
}
EXPORT_SYMBOL(mt_cpufreq_get_freq_by_idx);

int mt_cpufreq_update_volt(enum mt_cpu_dvfs_id id, unsigned int *volt_tbl, int nr_volt_tbl)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);
    unsigned long flags;
    int i;
    int ret = -1;

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(NULL == p);

    if (!cpu_dvfs_is_availiable(p)) {
        FUNC_EXIT(FUNC_LV_API);
        return 0;
    }

    BUG_ON(nr_volt_tbl > p->nr_opp_tbl);

    cpufreq_lock(flags);

    /* update volt table */
    for (i = 0; i < nr_volt_tbl; i++)
        p->opp_tbl[i].cpufreq_volt = PMIC_VAL_TO_VOLT(volt_tbl[i]);

    /* set volt */
    ret = _set_cur_volt_locked(p,
		   TURBO_MODE_VOLT(get_turbo_mode(p, cpu_dvfs_get_cur_freq(p)),
   		   cpu_dvfs_get_cur_volt(p)
  		   )
   );

    cpufreq_unlock(flags);

    FUNC_EXIT(FUNC_LV_API);

    return ret;
}
EXPORT_SYMBOL(mt_cpufreq_update_volt);

void mt_cpufreq_restore_default_volt(enum mt_cpu_dvfs_id id)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(NULL == p);

    if (!cpu_dvfs_is_availiable(p)) {
        FUNC_EXIT(FUNC_LV_API);
        return;
    }

    // Disable turbo mode since PTPOD is disabled
    if (p->turbo_mode) {
        cpufreq_info("@%s: Turbo mode disabled!\n", __func__);
        p->turbo_mode = 0;
    }

    _restore_default_volt(p);

    FUNC_EXIT(FUNC_LV_API);
}
EXPORT_SYMBOL(mt_cpufreq_restore_default_volt);

static unsigned int _cpu_freq_calc(unsigned int con1, unsigned int ckdiv1)
{
    unsigned int freq = 0;

#if 0   /* method 1 */
    static const unsigned int pll_vcodivsel_map[2] = {1, 2};
    static const unsigned int pll_prediv_map[4] = {1, 2, 4, 4};
    static const unsigned int pll_posdiv_map[8] = {1, 2, 4, 8, 16, 16, 16, 16};
    static const unsigned int pll_fbksel_map[4] = {1, 2, 4, 4};
    static const unsigned int pll_n_info_map[14] = { /* assume fin = 26MHz */
        13000000,
        6500000,
        3250000,
        1625000,
        812500,
        406250,
        203125,
        101563,
        50782,
        25391,
        12696,
        6348,
        3174,
        1587,
    };

    unsigned int posdiv    = _GET_BITS_VAL_(26 : 24, con1);
    unsigned int vcodivsel = 0; /* _GET_BITS_VAL_(19 : 19, con0); */ /* XXX: always zero */
    unsigned int prediv    = 0; /* _GET_BITS_VAL_(5 : 4, con0);   */ /* XXX: always zero */
    unsigned int n_info_i  = _GET_BITS_VAL_(20 : 14, con1);
    unsigned int n_info_f  = _GET_BITS_VAL_(13 : 0, con1);

    int i;
    unsigned int mask;
    unsigned int vco_i = 0;
    unsigned int vco_f = 0;

    posdiv = pll_posdiv_map[posdiv];
    vcodivsel = pll_vcodivsel_map[vcodivsel];
    prediv = pll_prediv_map[prediv];

    vco_i = 26 * n_info_i;

    for (i = 0; i < 14; i++) {
        mask = 1U << (13 - i);

        if (n_info_f & mask) {
            vco_f += pll_n_info_map[i];

            if (!(n_info_f & (mask - 1))) /* could break early if remaining bits are 0 */
                break;
        }
    }

    vco_f = (vco_f + 1000000 / 2) / 1000000; /* round up */

    freq = (vco_i + vco_f) * 1000 * vcodivsel / prediv / posdiv; /* KHz */
#else   /* method 2 */
    con1 &= _BITMASK_(26 : 0);

    if (con1 >= DDS_DIV8_FREQ) {
        freq = DDS_DIV8_FREQ;
        freq = PLL_DIV8_FREQ + (((con1 - freq) / 0x2000) * PLL_FREQ_STEP / 8);
    } else if (con1 >= DDS_DIV4_FREQ) {
        freq = DDS_DIV4_FREQ;
        freq = PLL_DIV4_FREQ + (((con1 - freq) / 0x2000) * PLL_FREQ_STEP / 4);
    } else if (con1 >= DDS_DIV2_FREQ) {
        freq = DDS_DIV2_FREQ;
        freq = PLL_DIV2_FREQ + (((con1 - freq) / 0x2000) * PLL_FREQ_STEP / 2);
    } else if (con1 >= DDS_DIV1_FREQ) {
        freq = DDS_DIV1_FREQ;
        freq = PLL_DIV1_FREQ + (((con1 - freq) / 0x2000) * PLL_FREQ_STEP);
    } else
        BUG();

#endif

    FUNC_ENTER(FUNC_LV_HELP);

    switch (ckdiv1) {
        case 9:
            freq = freq * 3 / 4;
            break;

        case 10:
            freq = freq * 2 / 4;
            break;

        case 11:
            freq = freq * 1 / 4;
            break;

        case 17:
            freq = freq * 4 / 5;
            break;

        case 18:
            freq = freq * 3 / 5;
            break;

        case 19:
            freq = freq * 2 / 5;
            break;

        case 20:
            freq = freq * 1 / 5;
            break;

        case 25:
            freq = freq * 5 / 6;
            break;

        case 26:
            freq = freq * 4 / 6;
            break;

        case 27:
            freq = freq * 3 / 6;
            break;

        case 28:
            freq = freq * 2 / 6;
            break;

        case 29:
            freq = freq * 1 / 6;
            break;

        case 8:
        case 16:
        case 24:
        default:
            break;
    }

    FUNC_EXIT(FUNC_LV_HELP);

    return freq; // TODO: adjust by ptp level???
}

static unsigned int get_cur_phy_freq(struct mt_cpu_dvfs *p)
{
    unsigned int con1;
    unsigned int ckdiv1;
    unsigned int cur_khz;

    FUNC_ENTER(FUNC_LV_LOCAL);

    BUG_ON(NULL == p);

#if 0
    // read from freq meter
    cur_khz = mt_get_cpu_freq();
    cpufreq_dbg("@%s: Meter = %d MHz\n", __func__, cur_khz);
#else
    con1 = cpufreq_read(ARMCA7PLL_CON1);
    ckdiv1 = cpufreq_read(TOP_CKDIV1);
    ckdiv1 = _GET_BITS_VAL_(4 : 0, ckdiv1);

    cur_khz = _cpu_freq_calc(con1, ckdiv1);

    cpufreq_ver("@%s: cur_khz = %d, con1 = 0x%x, ckdiv1_val = 0x%x\n", __func__, cur_khz, con1, ckdiv1);
#endif

    FUNC_EXIT(FUNC_LV_LOCAL);

    return cur_khz;
}

static unsigned int _mt_cpufreq_get_cur_phy_freq(enum mt_cpu_dvfs_id id)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    FUNC_ENTER(FUNC_LV_LOCAL);

    BUG_ON(NULL == p);

    FUNC_EXIT(FUNC_LV_LOCAL);

    return p->ops->get_cur_phy_freq(p);
}

static unsigned int _cpu_dds_calc(unsigned int khz) /* XXX: NOT OK FOR 1007.5MHz */
{
    unsigned int dds;

    FUNC_ENTER(FUNC_LV_HELP);

    if (khz >= PLL_DIV1_FREQ)
        dds = DDS_DIV1_FREQ + ((khz - PLL_DIV1_FREQ) / PLL_FREQ_STEP) * 0x2000;
    else if (khz >= PLL_DIV2_FREQ)
        dds = DDS_DIV2_FREQ + ((khz - PLL_DIV2_FREQ) * 2 / PLL_FREQ_STEP) * 0x2000;
    else if (khz >= PLL_DIV4_FREQ)
        dds = DDS_DIV4_FREQ + ((khz - PLL_DIV4_FREQ) * 4 / PLL_FREQ_STEP) * 0x2000;
    else if (khz >= PLL_DIV8_FREQ)
        dds = DDS_DIV8_FREQ + ((khz - PLL_DIV8_FREQ) * 8 / PLL_FREQ_STEP) * 0x2000;
    else
        BUG();

    FUNC_EXIT(FUNC_LV_HELP);

    return dds;
}

static void _cpu_clock_switch(struct mt_cpu_dvfs *p, enum top_ckmuxsel sel)
{
    FUNC_ENTER(FUNC_LV_HELP);

    switch (sel) {
        case TOP_CKMUXSEL_CLKSQ:
        case TOP_CKMUXSEL_ARMPLL:
            cpufreq_write_mask(TOP_CKMUXSEL, 1 : 0, sel);
            // disable gating cell (clear clk_misc_cfg_0[5:4])
            cpufreq_write_mask(CLK_MISC_CFG_0, 5 : 4, 0x0);
            break;
        case TOP_CKMUXSEL_MAINPLL:
        case TOP_CKMUXSEL_UNIVPLL:
            // enable gating cell (set clk_misc_cfg_0[5:4])
            cpufreq_write_mask(CLK_MISC_CFG_0, 5 : 4, 0x3);
            udelay(3);
            cpufreq_write_mask(TOP_CKMUXSEL, 1 : 0, sel);
            break;
        default:
            BUG();
            break;
    }

    FUNC_EXIT(FUNC_LV_HELP);
}

static enum top_ckmuxsel _get_cpu_clock_switch(struct mt_cpu_dvfs *p)
{
    unsigned int val = cpufreq_read(TOP_CKMUXSEL);
    unsigned int mask = _BITMASK_(1 : 0);

    FUNC_ENTER(FUNC_LV_HELP);

    val &= mask;                    /* _BITMASK_(1 : 0) */

    FUNC_EXIT(FUNC_LV_HELP);

    return val;
}

int mt_cpufreq_clock_switch(enum mt_cpu_dvfs_id id, enum top_ckmuxsel sel)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    if (!p)
        return -1;

    _cpu_clock_switch(p, sel);

    return 0;
}

enum top_ckmuxsel mt_cpufreq_get_clock_switch(enum mt_cpu_dvfs_id id)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    if (!p)
        return -1;

    return _get_cpu_clock_switch(p);
}

/*
 * CPU freq scaling
 *
 * above 1209MHz: use freq hopping
 * below 1209MHz: set CLKDIV1 
 * if cross 1209MHz, migrate to 1209MHz first.
 * 
 */
static void set_cur_freq(struct mt_cpu_dvfs *p, unsigned int cur_khz, unsigned int target_khz)
{
    unsigned int dds;
    unsigned int is_fhctl_used;
    unsigned int ckdiv1_val = _GET_BITS_VAL_(4 : 0, cpufreq_read(TOP_CKDIV1));
    unsigned int ckdiv1_mask = _BITMASK_(4 : 0);
    unsigned int sel = 0;
    unsigned int cur_volt = 0;    
    unsigned int mainpll_volt_idx = 0;

#define IS_CLKDIV_USED(clkdiv)  (((clkdiv < 8) || ((clkdiv % 8) == 0)) ? 0 : 1)

    FUNC_ENTER(FUNC_LV_LOCAL);

    if (cur_khz == target_khz)
        return;

    //cpufreq_ver("cur_khz = %d, ckdiv1_val = 0x%x\n", cur_khz, ckdiv1_val);

    if (((cur_khz < CPUFREQ_BOUNDARY_FOR_FHCTL) && (target_khz > CPUFREQ_BOUNDARY_FOR_FHCTL))
        || ((target_khz < CPUFREQ_BOUNDARY_FOR_FHCTL) && (cur_khz > CPUFREQ_BOUNDARY_FOR_FHCTL))) {
        set_cur_freq(p, cur_khz, CPUFREQ_BOUNDARY_FOR_FHCTL);
        cur_khz = CPUFREQ_BOUNDARY_FOR_FHCTL;
    }
    
    is_fhctl_used = ((target_khz >= CPUFREQ_BOUNDARY_FOR_FHCTL) && (cur_khz >= CPUFREQ_BOUNDARY_FOR_FHCTL)) ? 1 : 0;

    cpufreq_ver("@%s():%d, cur_khz = %d, target_khz = %d, is_fhctl_used = %d\n", 
                __func__, 
                __LINE__, 
                cur_khz, 
                target_khz, 
                is_fhctl_used
                );

    if (!is_fhctl_used) {    
        /* set ca7_clkdiv1_sel */
        switch (target_khz) {
            case CPU_DVFS_FREQ7:
                dds = _cpu_dds_calc(CPU_DVFS_FREQ7);
                sel = 8;    // 4/4
                break;
            case CPU_DVFS_FREQ8:
                dds = _cpu_dds_calc(CPU_DVFS_FREQ8);
                sel = 8;    // 4/4
                break;
            case CPU_DVFS_FREQ9:
                dds = _cpu_dds_calc(1404000);   // 702 = 1404 / 2
                sel = 10;    // 2/4
                break;
            case CPU_DVFS_FREQ10:
                dds = _cpu_dds_calc(936000);   // 468 = 936 / 2 
                sel = 10;    // 2/4
                break;
            default:
                BUG();
        }

        // adjust Vproc since MAINPLL is 1092 MHz (~= CPU_DVFS_FREQ7)
        cur_volt = p->ops->get_cur_volt(p);
        switch (p->cpu_level) {
            case CPU_LEVEL_0:
            case CPU_LEVEL_1:
                mainpll_volt_idx = 4;
                break;
            case CPU_LEVEL_2:
                mainpll_volt_idx = 3;
                break;
            case CPU_LEVEL_3:
            default:
                mainpll_volt_idx = 1;
                break;
        }
        if (cur_volt < cpu_dvfs_get_volt_by_idx(p, mainpll_volt_idx))
            p->ops->set_cur_volt(p, cpu_dvfs_get_volt_by_idx(p, mainpll_volt_idx));
        else
            cur_volt = 0;

        // set ARMPLL and CLKDIV
        _cpu_clock_switch(p, TOP_CKMUXSEL_MAINPLL);
        cpufreq_write(ARMCA7PLL_CON1, dds | _BIT_(31)); /* CHG */
        udelay(PLL_SETTLE_TIME);
        cpufreq_write(TOP_CKDIV1, (ckdiv1_val & ~ckdiv1_mask) | sel);
        _cpu_clock_switch(p, TOP_CKMUXSEL_ARMPLL);

        // restore Vproc
        if (cur_volt)
            p->ops->set_cur_volt(p, cur_volt);
    }
    else {
        dds = _cpu_dds_calc(target_khz);        
        BUG_ON(dds & _BITMASK_(26 : 24)); /* should not use posdiv */
 
#if !defined(__KERNEL__) && defined(MTKDRV_FREQHOP) 
        fhdrv_dvt_dvfs_enable(ARMCA7PLL_ID, dds);
#else  /* __KERNEL__ */
#if 1
        mt_dfs_armpll(FH_ARMCA7_PLLID, dds);
#else
        _cpu_clock_switch(p, TOP_CKMUXSEL_MAINPLL);
        cpufreq_write(ARMCA7PLL_CON1, dds | _BIT_(31)); /* CHG */
        udelay(PLL_SETTLE_TIME);
        _cpu_clock_switch(p, TOP_CKMUXSEL_ARMPLL);
#endif
#endif /* ! __KERNEL__ */
    }

    FUNC_EXIT(FUNC_LV_LOCAL);
}

/* for volt change (PMICWRAP/extBuck) */

static unsigned int get_cur_volt_pmic_wrap(struct mt_cpu_dvfs *p)
{
    unsigned int rdata = 0;
    unsigned int retry_cnt = 5;

    FUNC_ENTER(FUNC_LV_LOCAL);

    pwrap_read(PMIC_ADDR_VPROC_EN, &rdata);

    rdata &= _BITMASK_(0 : 0); /* enable or disable (i.e. 0mv or not) */

    if (rdata) { /* enabled i.e. not 0mv */
        do {
            pwrap_read(PMIC_ADDR_VPROC_VOSEL_ON, &rdata);
        } while (rdata == PMIC_VAL_TO_VOLT(0) && retry_cnt--);

        rdata = PMIC_VAL_TO_VOLT(rdata);
        //cpufreq_ver("@%s: volt = %d\n", __func__, rdata);
    } else
        cpufreq_err("@%s: read VPROC_EN failed, rdata = 0x%x\n", __func__, rdata);

    FUNC_EXIT(FUNC_LV_LOCAL);

    return rdata; /* vproc: mv*100 */
}

static unsigned int get_cur_vsram(struct mt_cpu_dvfs *p)
{
    unsigned int rdata = 0;
    unsigned int retry_cnt = 5;

    FUNC_ENTER(FUNC_LV_LOCAL);

    pwrap_read(PMIC_ADDR_VSRAM_EN, &rdata);

    rdata &= _BITMASK_(1 : 1); /* enable or disable (i.e. 0mv or not) */

    if (rdata) { /* enabled i.e. not 0mv */
        int ret = 0;
        do {
            ret = pwrap_read(PMIC_ADDR_VSRAM_VOSEL_ON, &rdata);
        } while (ret && retry_cnt--);

        rdata = PMIC_VAL_TO_VOLT(rdata);
        //cpufreq_ver("@%s: volt = %d\n", __func__, rdata);
    } else
        cpufreq_err("@%s: read VSRAM_EN failed, rdata = 0x%x\n", __func__, rdata);

    FUNC_EXIT(FUNC_LV_LOCAL);

    return rdata; /* vproc: mv*100 */
}

static unsigned int get_cur_volt_extbuck(struct mt_cpu_dvfs *p)
{
    unsigned char ret_val = 0;
    unsigned int ret_volt = 0; // volt: mv * 100
    unsigned int retry_cnt = 5;

    FUNC_ENTER(FUNC_LV_LOCAL);

    if (cpu_dvfs_is_extbuck_valid()) {
        do {
            if (!mt6311_read_byte(MT6311_VDVFS11_CON13, &ret_val)) {
                cpufreq_err("%s(), fail to read ext buck volt\n", __func__);
                ret_volt = 0;
            } else {
                ret_volt = EXTBUCK_VAL_TO_VOLT(ret_val);
                //cpufreq_ver("@%s: volt = %d\n", __func__, ret_volt);
            }
        } while (ret_volt == EXTBUCK_VAL_TO_VOLT(0) && retry_cnt--); // XXX: EXTBUCK_VAL_TO_VOLT(0) is impossible setting and need to retry
    }
    else
        cpufreq_err("%s(), can not use ext buck!\n", __func__);
    
    FUNC_EXIT(FUNC_LV_LOCAL);

    return ret_volt;
}

unsigned int mt_cpufreq_get_cur_volt(enum mt_cpu_dvfs_id id)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(NULL == p);
    BUG_ON(NULL == p->ops);

    FUNC_EXIT(FUNC_LV_API);

    return p->ops->get_cur_volt(p);  // mv * 100
}
EXPORT_SYMBOL(mt_cpufreq_get_cur_volt);

static unsigned int _calc_pmic_settle_time(unsigned int old_vproc, unsigned int old_vsram, unsigned int new_vproc, unsigned int new_vsram)
{
    unsigned delay = 100;

    if (new_vproc == old_vproc && new_vsram == old_vsram)
        return 0;

    // VPROC is UP
    if (new_vproc >= old_vproc) {
        // VSRAM is UP too, choose larger one to calculate settle time
        if (new_vsram >= old_vsram)
           delay = MAX (
                       PMIC_VOLT_UP_SETTLE_TIME(old_vsram, new_vsram),
                       PMIC_VOLT_UP_SETTLE_TIME(old_vproc, new_vproc)
                       );
        // VSRAM is DOWN, it may happen at bootup stage
        else
            delay = MAX (
                        PMIC_VOLT_DOWN_SETTLE_TIME(old_vsram, new_vsram),
                        PMIC_VOLT_UP_SETTLE_TIME(old_vproc, new_vproc)
                        );
    }
    // VPROC is DOWN
    else {
        // VSRAM is DOWN too, choose larger one to calculate settle time
        if (old_vsram >= new_vsram)
            delay = MAX (
                        PMIC_VOLT_DOWN_SETTLE_TIME(old_vsram, new_vsram),
                        PMIC_VOLT_DOWN_SETTLE_TIME(old_vproc, new_vproc)
                        );
        // VSRAM is UP, it may happen at bootup stage
        else
            delay = MAX (
                        PMIC_VOLT_UP_SETTLE_TIME(old_vsram, new_vsram),
                        PMIC_VOLT_DOWN_SETTLE_TIME(old_vproc, new_vproc)
                        );
    }

    if (delay < MIN_PMIC_SETTLE_TIME)
        delay = MIN_PMIC_SETTLE_TIME;

    return delay;
}

static int set_cur_volt_pmic_wrap(struct mt_cpu_dvfs *p, unsigned int volt) /* volt: vproc (mv*100) */
{
    unsigned int cur_volt = get_cur_volt_pmic_wrap(p);
    bool is_leaving_turbo_mode = false;

    FUNC_ENTER(FUNC_LV_LOCAL);

    if (is_in_turbo_mode 
        && cur_volt > cpu_dvfs_get_volt_by_idx(p, 0) 
        && volt <= cpu_dvfs_get_volt_by_idx(p, 0))
        is_leaving_turbo_mode = true;

    //mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_SODI, IDX_SO_VPROC_CA7_NORMAL, VOLT_TO_PMIC_VAL(volt));
    mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_NORMAL, IDX_NM_VPROC, VOLT_TO_PMIC_VAL(volt));
    mt_cpufreq_apply_pmic_cmd(IDX_NM_VPROC);

    /* delay for scaling up */
    if (volt > cur_volt)
        udelay(_calc_pmic_settle_time(cur_volt, 0, volt, 0));   // vsram is autotracking, bypass it

    if (NULL != g_pCpuVoltSampler)
        g_pCpuVoltSampler(MT_CPU_DVFS_LITTLE, volt / 100);  // mv

    if (is_leaving_turbo_mode) {
        //cpufreq_dbg("@%s: turbo mode end\n", __func__);
        
        // Need to delay settle time for scaling down
        udelay(_calc_pmic_settle_time(cur_volt, 0, volt, 0));
        // 0x466[15:14] = 2'b00, auto-mode at VDVFS1 and VDVFS2
        pmic_config_interface(0x466, 0x0, 0x3, 0xE);
        // 0x462[3] = 1'b0, release force 2 phase
        pmic_config_interface(0x462, 0x0, 0x1, 0x3);
        
        is_in_turbo_mode = false; 
    }

    FUNC_EXIT(FUNC_LV_LOCAL);

    return 0;
}

static void dump_opp_table(struct mt_cpu_dvfs *p)
{
    int i;

    cpufreq_err("[%s/%d]\n"
            "cpufreq_oppidx = %d\n",
            p->name, p->cpu_id,
            p->idx_opp_tbl
            );

    for (i = 0; i < p->nr_opp_tbl; i++) {
        cpufreq_err("\tOP(%d, %d),\n",
                cpu_dvfs_get_freq_by_idx(p, i),
                cpu_dvfs_get_volt_by_idx(p, i)
                );
    }
}

static int set_cur_volt_extbuck(struct mt_cpu_dvfs *p, unsigned int volt) /* volt: vproc (mv*100) */
{
    unsigned int cur_vsram = get_cur_vsram(p);
    unsigned int cur_vproc = get_cur_volt_extbuck(p);
    unsigned int delay_us = 0;
    bool is_leaving_turbo_mode = false;
    int ret = 0;

    FUNC_ENTER(FUNC_LV_LOCAL);

    if (cur_vproc == 0 || !cpu_dvfs_is_extbuck_valid()) {
        cpufreq_err("@%s():%d, can not use ext buck!\n", __func__, __LINE__);
        return -1;
    }

    if (is_in_turbo_mode 
        && cur_vproc > cpu_dvfs_get_volt_by_idx(p, 0) 
        && volt <= cpu_dvfs_get_volt_by_idx(p, 0))
        is_leaving_turbo_mode = true;

    if (unlikely(!((cur_vsram > cur_vproc) && (MAX_DIFF_VSRAM_VPROC >= (cur_vsram - cur_vproc))))) {
        extern kal_uint32 mt6311_get_chip_id(void);
        unsigned int i, val, extbuck_chip_id = mt6311_get_chip_id();

        dump_opp_table(p);
        cpufreq_err("@%s():%d, cur_vsram = %d, cur_vproc = %d, extbuck_chip_id = 0x%x\n", 
                    __func__, 
                    __LINE__, 
                    cur_vsram, 
                    cur_vproc,
                    extbuck_chip_id
                    );

        // read extbuck chip id to verify I2C is still worked or not
        for (i=0; i<10; i++) {
            val = ((mt6311_get_cid() << 8) | (mt6311_get_swcid()));
            cpufreq_err("read chip id from I2C, id: 0x%x\n", val);
        }

        // read pmic wrap chip id
        for (i=0; i<10; i++) {
            pwrap_read(0x200, &val);
            cpufreq_err("pmic wrap CID = %x\n", val);
        }
        
        aee_kernel_warning(TAG, "@%s():%d, cur_vsram = %d, cur_vproc = %d\n", 
                            __func__, 
                            __LINE__, 
                            cur_vsram, 
                            cur_vproc);

        cur_vproc = cpu_dvfs_get_cur_volt(p);
        cur_vsram = cur_vproc + NORMAL_DIFF_VRSAM_VPROC;
    }

    /* UP */
    if (volt > cur_vproc) {
        unsigned int target_vsram = volt + NORMAL_DIFF_VRSAM_VPROC;
        unsigned int next_vsram;

        do {
            unsigned int old_vproc = cur_vproc;
            unsigned int old_vsram = cur_vsram;
                
            next_vsram = MIN(((MAX_DIFF_VSRAM_VPROC-2500) + cur_vproc), target_vsram);

            /* update vsram */
            cur_vsram = MAX(next_vsram, MIN_VSRAM_VOLT);
            if (cur_vsram > MAX_VSRAM_VOLT) {
                cur_vsram = MAX_VSRAM_VOLT;
                target_vsram = MAX_VSRAM_VOLT; // to end the loop
            }    

            if (unlikely(!((cur_vsram > cur_vproc) && (MAX_DIFF_VSRAM_VPROC >= (cur_vsram - cur_vproc))))) {
                dump_opp_table(p);
                cpufreq_err("@%s():%d, cur_vsram = %d, cur_vproc = %d\n", __func__, __LINE__, cur_vsram, cur_vproc);
                BUG();
            }

            mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_NORMAL, IDX_NM_VSRAM, VOLT_TO_PMIC_VAL(cur_vsram));
            mt_cpufreq_apply_pmic_cmd(IDX_NM_VSRAM);

            /* update vproc */
            if (next_vsram > MAX_VSRAM_VOLT)
                cur_vproc = volt;   // Vsram was limited, set to target vproc directly
            else
                cur_vproc = next_vsram - NORMAL_DIFF_VRSAM_VPROC;

            if (unlikely(!((cur_vsram > cur_vproc) && (MAX_DIFF_VSRAM_VPROC >= (cur_vsram - cur_vproc))))) {
                dump_opp_table(p);
                cpufreq_err("@%s():%d, cur_vsram = %d, cur_vproc = %d\n", __func__, __LINE__, cur_vsram, cur_vproc);
                BUG();
            }

            if (cpu_dvfs_is_extbuck_valid()) {
                mt6311_set_vdvfs11_vosel_on(VOLT_TO_EXTBUCK_VAL(cur_vproc));
            } else {
                cpufreq_err("%s(), fail to set ext buck volt\n", __func__);
                ret = -1;
                break;
            }

            delay_us = _calc_pmic_settle_time(old_vproc, old_vsram, cur_vproc, cur_vsram);
            udelay(delay_us);
            
            cpufreq_ver("@%s(): UP --> old_vsram=%d, cur_vsram=%d, old_vproc=%d, cur_vproc=%d, delay=%d\n", 
                        __func__,
                        old_vsram,
                        cur_vsram,
                        old_vproc,
                        cur_vproc,
                        delay_us
                        );
        } while (target_vsram > cur_vsram);
    }
    /* DOWN */
    else if (volt < cur_vproc) {
        unsigned int next_vproc;
        unsigned int next_vsram = cur_vproc + NORMAL_DIFF_VRSAM_VPROC;

        do {
            unsigned int old_vproc = cur_vproc;
            unsigned int old_vsram = cur_vsram;
            
            next_vproc = MAX((next_vsram - (MAX_DIFF_VSRAM_VPROC-2500)), volt);

            /* update vproc */
            cur_vproc = next_vproc;

            if (unlikely(!((cur_vsram > cur_vproc) && (MAX_DIFF_VSRAM_VPROC >= (cur_vsram - cur_vproc))))) {
                dump_opp_table(p);
                cpufreq_err("@%s():%d, cur_vsram = %d, cur_vproc = %d\n", __func__, __LINE__, cur_vsram, cur_vproc);
                BUG();
            }

            if (cpu_dvfs_is_extbuck_valid()) {
                mt6311_set_vdvfs11_vosel_on(VOLT_TO_EXTBUCK_VAL(cur_vproc));
            } else {
                cpufreq_err("%s(), fail to set ext buck volt\n", __func__);
                ret = -1;
                break;
            }

            /* update vsram */
            next_vsram = cur_vproc + NORMAL_DIFF_VRSAM_VPROC;
            cur_vsram = MAX(next_vsram, MIN_VSRAM_VOLT);
            cur_vsram = MIN(cur_vsram, MAX_VSRAM_VOLT);

            if (unlikely(!((cur_vsram > cur_vproc) && (MAX_DIFF_VSRAM_VPROC >= (cur_vsram - cur_vproc))))) {
                dump_opp_table(p);
                cpufreq_err("@%s():%d, cur_vsram = %d, cur_vproc = %d\n", __func__, __LINE__, cur_vsram, cur_vproc);
                BUG();
            }

           // mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_SODI, IDX_SO_VSRAM_CA15L_NORMAL, VOLT_TO_PMIC_VAL(cur_vsram));
            mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_NORMAL, IDX_NM_VSRAM, VOLT_TO_PMIC_VAL(cur_vsram));
            mt_cpufreq_apply_pmic_cmd(IDX_NM_VSRAM);

            delay_us = _calc_pmic_settle_time(old_vproc, old_vsram, cur_vproc, cur_vsram);
            udelay(delay_us);
            
            cpufreq_ver("@%s(): DOWN --> old_vsram=%d, cur_vsram=%d, old_vproc=%d, cur_vproc=%d, delay=%d\n", 
                        __func__,
                        old_vsram,
                        cur_vsram,
                        old_vproc,
                        cur_vproc,
                        delay_us
                        );
        } while (cur_vproc > volt);
    }

    if (NULL != g_pCpuVoltSampler)
        g_pCpuVoltSampler(MT_CPU_DVFS_LITTLE, volt / 100); // mv

    cpufreq_ver("@%s():%d, cur_vsram = %d, cur_vproc = %d\n", __func__, __LINE__, cur_vsram, cur_vproc);

    if (is_leaving_turbo_mode) {
        //cpufreq_dbg("@%s: turbo mode end\n", __func__);
        // restore 6311 setting, it will limit voltage output to 1.1V
        mt6311_set_buck_test_mode(0x0);
        is_in_turbo_mode = false;
    }

    FUNC_EXIT(FUNC_LV_LOCAL);

    return ret;
}

/* cpufreq set (freq & volt) */

static unsigned int _search_available_volt(struct mt_cpu_dvfs *p, unsigned int target_khz)
{
    int i;

    FUNC_ENTER(FUNC_LV_HELP);

    BUG_ON(NULL == p);

    /* search available voltage */
    for (i = p->nr_opp_tbl - 1; i >= 0; i--) {
        if (target_khz <= cpu_dvfs_get_freq_by_idx(p, i))
            break;
    }

    BUG_ON(i < 0); /* i.e. target_khz > p->opp_tbl[0].cpufreq_khz */

    FUNC_EXIT(FUNC_LV_HELP);

    return cpu_dvfs_get_volt_by_idx(p, i); // mv * 100
}

static int _cpufreq_set_locked(struct mt_cpu_dvfs *p, unsigned int cur_khz, unsigned int target_khz, struct cpufreq_policy *policy)
{
    unsigned int volt; // mv * 100
    int ret = 0;
#ifdef CONFIG_CPU_FREQ
    struct cpufreq_freqs freqs;
    unsigned int cpu;
#endif

    enum turbo_mode mode = get_turbo_mode(p, target_khz);

    FUNC_ENTER(FUNC_LV_HELP);

    volt = _search_available_volt(p, target_khz);

    if (cur_khz != TURBO_MODE_FREQ(mode, target_khz))
        cpufreq_ver("@%s(), target_khz = %d (%d), volt = %d (%d), num_online_cpus = %d, cur_khz = %d\n", 
                    __func__, 
                    target_khz, 
                    TURBO_MODE_FREQ(mode, target_khz),
                    volt, 
                    TURBO_MODE_VOLT(mode, volt),
                    num_online_cpus(),
                    cur_khz
                    );

    volt = TURBO_MODE_VOLT(mode, volt);
    target_khz = TURBO_MODE_FREQ(mode, target_khz);

    if (cur_khz == target_khz)
        goto out;

    {
#if 0
        /* update for deep idle */
        mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_DEEPIDLE,
#ifdef MTK_FORCE_CLUSTER1
                IDX_DI_VSRAM_CA15L_NORMAL,
#else
                IDX_DI_VPROC_CA7_NORMAL,
#endif
                VOLT_TO_PMIC_VAL(volt + (cpu_dvfs_is(p, MT_CPU_DVFS_LITTLE) ? 0 : NORMAL_DIFF_VRSAM_VPROC))
                );

        /* update for suspend */
        if (   cpu_dvfs_is(p, MT_CPU_DVFS_BIG)
                && pw.set[PMIC_WRAP_PHASE_SUSPEND]._[IDX_SP_VSRAM_CA15L_PWR_ON].cmd_addr == PMIC_ADDR_VSRAM_CA15L_VOSEL_ON
           )
            mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_SUSPEND,
                    IDX_SP_VSRAM_CA15L_PWR_ON,
                    VOLT_TO_PMIC_VAL(volt + NORMAL_DIFF_VRSAM_VPROC)
                    );
#endif
    }

    /* set volt (UP) */
    if (cur_khz < target_khz) {
        ret = p->ops->set_cur_volt(p, volt);

        if (ret) /* set volt fail */
            goto out;
    }

#ifdef CONFIG_CPU_FREQ	
    freqs.old = cur_khz;
    freqs.new = target_khz;
    if (policy) {
        for_each_online_cpu(cpu) {
            freqs.cpu = cpu;
            cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
        }
    }
#endif

/* set freq (UP/DOWN) */
    if (cur_khz != target_khz)
        p->ops->set_cur_freq(p, cur_khz, target_khz);

#ifdef CONFIG_CPU_FREQ	
    if (policy) {
        for_each_online_cpu(cpu) {
            freqs.cpu = cpu;
            cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
        }
    }
#endif

    /* set volt (DOWN) */
    if (cur_khz > target_khz) {
        ret = p->ops->set_cur_volt(p, volt);

        if (ret) /* set volt fail */
            goto out;
    }

    cpufreq_dbg("@%s(): Vproc = %dmv, Vsram = %dmv, freq = %d KHz\n", 
                __func__, 
                (p->ops->get_cur_volt(p)) / 100,
                (get_cur_vsram(p) / 100),
                p->ops->get_cur_phy_freq(p)
                );

    // trigger exception if freq/volt not correct during stress
    if (do_dvfs_stress_test) {
        BUG_ON(p->ops->get_cur_volt(p) < volt);
        BUG_ON(p->ops->get_cur_phy_freq(p) != target_khz);
    }

    FUNC_EXIT(FUNC_LV_HELP);
out:
    return ret;
}

static unsigned int _calc_new_opp_idx(struct mt_cpu_dvfs *p, int new_opp_idx);

static void _mt_cpufreq_set(enum mt_cpu_dvfs_id id, int new_opp_idx)
{
    unsigned long flags;
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);
    unsigned int cur_freq;
    unsigned int target_freq;
#ifdef CONFIG_CPU_FREQ
    struct cpufreq_policy *policy;
#endif

    FUNC_ENTER(FUNC_LV_LOCAL);

    BUG_ON(NULL == p);
    BUG_ON(new_opp_idx >= p->nr_opp_tbl);

#ifdef CONFIG_CPU_FREQ
    policy = cpufreq_cpu_get(p->cpu_id);
#endif

    cpufreq_lock(flags);	// <-XXX

    // get current idx here to avoid idx synchronization issue
    if (new_opp_idx == -1)
        new_opp_idx = p->idx_opp_tbl;

    if (do_dvfs_stress_test)
        new_opp_idx = jiffies & 0x7; /* 0~7 */
    else {
#if defined(CONFIG_CPU_DVFS_BRINGUP)
        new_opp_idx = id_to_cpu_dvfs(id)->idx_normal_max_opp;
#else
        new_opp_idx = _calc_new_opp_idx(id_to_cpu_dvfs(id), new_opp_idx);
#endif
    }

    cur_freq = p->ops->get_cur_phy_freq(p);
    target_freq = cpu_dvfs_get_freq_by_idx(p, new_opp_idx);
#ifdef CONFIG_CPU_FREQ
    _cpufreq_set_locked(p, cur_freq, target_freq, policy);
#else
    _cpufreq_set_locked(p, cur_freq, target_freq, NULL);
#endif
    p->idx_opp_tbl = new_opp_idx;
    
    cpufreq_unlock(flags);	// <-XXX

#ifdef CONFIG_CPU_FREQ
    if (policy)
        cpufreq_cpu_put(policy);
#endif

    FUNC_EXIT(FUNC_LV_LOCAL);
}

static int __cpuinit turbo_mode_cpu_callback(struct notifier_block *nfb,
        unsigned long action, void *hcpu)
{
#if 1
    unsigned int cpu = (unsigned long)hcpu;
    unsigned int online_cpus = num_online_cpus();
    struct device *dev;
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(0); // TODO: FIXME, for E1

    cpufreq_ver("@%s():%d, cpu = %d, action = %lu, oppidx = %d, num_online_cpus = %d, num_online_cpus_delta = %d\n", 
                            __func__, 
                            __LINE__, 
                            cpu, 
                            action, 
                            p->idx_opp_tbl, 
                            online_cpus,
                            num_online_cpus_delta
                            ); // <-XXX

    dev = get_cpu_device(cpu);

    if (dev) {
        if (TURBO_MODE_BOUNDARY_CPU_NUM == online_cpus) {
            switch (action) {
                case CPU_UP_PREPARE:
                case CPU_UP_PREPARE_FROZEN:
                    num_online_cpus_delta = 1;
                case CPU_DEAD:
                case CPU_DEAD_FROZEN:
                    _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, -1);
                    break;
            }
        } else {
            switch (action) {
                case CPU_ONLINE:    // CPU UP done
                case CPU_ONLINE_FROZEN:
                case CPU_UP_CANCELED:   // CPU UP failed
                case CPU_UP_CANCELED_FROZEN:
                    num_online_cpus_delta = 0;
                    break;
            }
        }

        cpufreq_ver("@%s():%d, cpu = %d, action = %lu, oppidx = %d, num_online_cpus = %d, num_online_cpus_delta = %d\n", 
                                __func__, 
                                __LINE__, 
                                cpu, 
                                action, 
                                p->idx_opp_tbl, 
                                online_cpus,
                                num_online_cpus_delta
                                ); // <-XXX
    }

#else	// XXX: DON'T USE cpufreq_driver_target() for the case which cur_freq == target_freq
    struct cpufreq_policy *policy;
    unsigned int cpu = (unsigned long)hcpu;
    struct device *dev;
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(0); // TODO: FIXME, for E1

    cpufreq_ver("@%s():%d, cpu = %d, action = %d, oppidx = %d, num_online_cpus = %d\n", __func__, __LINE__, cpu, action, p->idx_opp_tbl, num_online_cpus()); // <-XXX

    dev = get_cpu_device(cpu);

    if (dev
            && 0 == p->idx_opp_tbl
            && TURBO_MODE_BOUNDARY_CPU_NUM == num_online_cpus()
       ) {
        switch (action) {
            case CPU_UP_PREPARE:
            case CPU_DEAD:

                policy = cpufreq_cpu_get(p->cpu_id);

                if (policy) {
                    cpufreq_driver_target(policy, cpu_dvfs_get_cur_freq(p), CPUFREQ_RELATION_L);
                    cpufreq_cpu_put(policy);
                }

                cpufreq_ver("@%s():%d, cpu = %d, action = %d, oppidx = %d, num_online_cpus = %d\n", __func__, __LINE__, cpu, action, p->idx_opp_tbl, num_online_cpus()); // <-XXX
                break;
        }
    }

#endif
    return NOTIFY_OK;
}

static struct notifier_block __refdata turbo_mode_cpu_notifier = {
    .notifier_call = turbo_mode_cpu_callback,
};

static void _set_no_limited(struct mt_cpu_dvfs *p)
{
    FUNC_ENTER(FUNC_LV_HELP);

    BUG_ON(NULL == p);

    p->limited_max_freq = cpu_dvfs_get_max_freq(p);
    p->limited_max_ncpu = max_cpu_num;

    FUNC_EXIT(FUNC_LV_HELP);
}

#ifdef CONFIG_CPU_DVFS_DOWNGRADE_FREQ
static void _downgrade_freq_check(enum mt_cpu_dvfs_id id)
{
    struct cpufreq_policy *policy;
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);
    int temp = 0;

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(NULL == p);

    /* if not CPU_LEVEL0 */
    if (p->cpu_level != CPU_LEVEL_0)
        goto out;

    /* get temp */
#if 0 // TODO: FIXME

    if (mt_ptp_status((MT_CPU_DVFS_LITTLE == id) ? PTP_DET_LITTLE : PTP_DET_BIG) == 1)
        temp = (((DRV_Reg32(PTP_TEMP) & 0xff)) + 25) * 1000; // TODO: mt_ptp.c provide mt_ptp_get_temp()
    else
        temp = mtktscpu_get_Tj_temp(); // TODO: FIXME, what is the difference for big & LITTLE

#else
    temp = tscpu_get_temp_by_bank(THERMAL_BANK0);    // bank0 for CPU
#endif


    if (temp < 0 || 125000 < temp) {
        // cpufreq_dbg("%d (temp) < 0 || 125000 < %d (temp)\n", temp, temp);
        goto out;
    }

    {
        static enum turbo_mode pre_mode = TURBO_MODE_NONE;
        enum turbo_mode cur_mode = get_turbo_mode(p, cpu_dvfs_get_cur_freq(p));

        if (pre_mode != cur_mode) {
            _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, -1);
            cpufreq_ver("@%s():%d, oppidx = %d, num_online_cpus = %d, pre_mode = %d, cur_mode = %d\n", 
                        __func__, 
                        __LINE__, 
                        p->idx_opp_tbl,
                        num_online_cpus(),
                        pre_mode, 
                        cur_mode
                        ); // <-XXX
            pre_mode = cur_mode;
        }
    }

    if (temp <= p->ptpod_temperature_limit_1) {
        p->downgrade_freq_for_ptpod  = false;
        // cpufreq_dbg("%d (temp) < %d (limit_1)\n", temp, p->ptpod_temperature_limit_1);
        goto out;
    } else if ((temp > p->ptpod_temperature_limit_1) && (temp < p->ptpod_temperature_limit_2)) {
        p->downgrade_freq_counter_return_limit = p->downgrade_freq_counter_limit * p->ptpod_temperature_time_1;
        // cpufreq_dbg("%d (temp) > %d (limit_1)\n", temp, p->ptpod_temperature_limit_1);
    } else {
        p->downgrade_freq_counter_return_limit = p->downgrade_freq_counter_limit * p->ptpod_temperature_time_2;
        // cpufreq_dbg("%d (temp) > %d (limit_2)\n", temp, p->ptpod_temperature_limit_2);
    }

    if (p->downgrade_freq_for_ptpod == false) {
        if ((num_online_cpus() == p->pre_online_cpu) && (cpu_dvfs_get_cur_freq(p) == p->pre_freq)) {
            if ((num_online_cpus() >= p->over_max_cpu) && (p->idx_opp_tbl == 0)) {
                p->downgrade_freq_counter++;
                // cpufreq_dbg("downgrade_freq_counter_limit = %d\n", p->downgrade_freq_counter_limit);
                // cpufreq_dbg("downgrade_freq_counter = %d\n", p->downgrade_freq_counter);

                if (p->downgrade_freq_counter >= p->downgrade_freq_counter_limit) {
                    p->downgrade_freq = cpu_dvfs_get_freq_by_idx(p, 1);

                    p->downgrade_freq_for_ptpod = true;
                    p->downgrade_freq_counter = 0;

                    cpufreq_info("freq limit, downgrade_freq_for_ptpod = %d\n", p->downgrade_freq_for_ptpod);

                    policy = cpufreq_cpu_get(p->cpu_id);

                    if (!policy)
                        goto out;

                    cpufreq_driver_target(policy, p->downgrade_freq, CPUFREQ_RELATION_L);

                    cpufreq_cpu_put(policy);
                }
            } else
                p->downgrade_freq_counter = 0;
        } else {
            p->pre_online_cpu = num_online_cpus();
            p->pre_freq = cpu_dvfs_get_cur_freq(p);

            p->downgrade_freq_counter = 0;
        }
    } else {
        p->downgrade_freq_counter_return++;

        // cpufreq_dbg("downgrade_freq_counter_return_limit = %d\n", p->downgrade_freq_counter_return_limit);
        // cpufreq_dbg("downgrade_freq_counter_return = %d\n", p->downgrade_freq_counter_return);

        if (p->downgrade_freq_counter_return >= p->downgrade_freq_counter_return_limit) {
            p->downgrade_freq_for_ptpod  = false;
            p->downgrade_freq_counter_return = 0;

            // cpufreq_dbg("Release freq limit, downgrade_freq_for_ptpod = %d\n", p->downgrade_freq_for_ptpod);
        }
    }

out:
    FUNC_EXIT(FUNC_LV_API);
}

static void _init_downgrade(struct mt_cpu_dvfs *p, unsigned int cpu_level)
{
    FUNC_ENTER(FUNC_LV_HELP);

    switch (cpu_level) {
        case CPU_LEVEL_0:
        case CPU_LEVEL_1:
        case CPU_LEVEL_2:
        case CPU_LEVEL_3:
        default:
#if 0 // TODO: check this setting
            p->downgrade_freq_counter_limit = cpu_dvfs_is(p, MT_CPU_DVFS_LITTLE) ? 10 : 10;
            p->ptpod_temperature_time_1     = cpu_dvfs_is(p, MT_CPU_DVFS_LITTLE) ? 2  : 1;
            p->ptpod_temperature_time_2     = cpu_dvfs_is(p, MT_CPU_DVFS_LITTLE) ? 8  : 4;
#else
            p->downgrade_freq_counter_limit = 10;
            p->ptpod_temperature_time_1     = 2;
            p->ptpod_temperature_time_2     = 8;
#endif
            break;
    }

#ifdef __KERNEL__
    /* install callback */
    cpufreq_freq_check = _downgrade_freq_check;
#endif

    FUNC_EXIT(FUNC_LV_HELP);
}
#endif

static int _sync_opp_tbl_idx(struct mt_cpu_dvfs *p)
{
    int ret = -1;
    unsigned int freq;
    int i;

    FUNC_ENTER(FUNC_LV_HELP);

    BUG_ON(NULL == p);
    BUG_ON(NULL == p->opp_tbl);
    BUG_ON(NULL == p->ops);

    freq = p->ops->get_cur_phy_freq(p);

    for (i = p->nr_opp_tbl - 1; i >= 0; i--) {
        if (freq <= cpu_dvfs_get_freq_by_idx(p, i)) {
            p->idx_opp_tbl = i;
            break;
        }

    }

    if (i >= 0) {
        cpufreq_info("%s freq = %d\n", cpu_dvfs_get_name(p), cpu_dvfs_get_cur_freq(p));

        // TODO: apply correct voltage???

        ret = 0;
    } else
        cpufreq_warn("%s can't find freq = %d\n", cpu_dvfs_get_name(p), freq);

    FUNC_EXIT(FUNC_LV_HELP);

    return ret;
}

static void _mt_cpufreq_sync_opp_tbl_idx(void)
{
    struct mt_cpu_dvfs *p;
    int i;

    FUNC_ENTER(FUNC_LV_LOCAL);

    for_each_cpu_dvfs(i, p) {
        if (cpu_dvfs_is_availiable(p))
            _sync_opp_tbl_idx(p);
    }

    FUNC_EXIT(FUNC_LV_LOCAL);
}

static enum mt_cpu_dvfs_id _get_cpu_dvfs_id(unsigned int cpu_id)
{
#if 1
    // Little core only for K2
    return MT_CPU_DVFS_LITTLE;
#else
#if 1	// TODO: FIXME, just for E1
    return (enum mt_cpu_dvfs_id)((cpu_id < 4) ? 0 : 1);
#else	// TODO: FIXME, just for E1
    struct mt_cpu_dvfs *p;
    int i;

    for_each_cpu_dvfs(i, p) {
        if (p->cpu_id == cpu_id)
            break;
    }

    BUG_ON(i >= NR_MT_CPU_DVFS);

    return i;
#endif	// TODO: FIXME, just for E1
#endif
}

int mt_cpufreq_state_set(int enabled) // TODO: state set by id??? keep this function???
{
#if 0
    bool set_normal_max_opp = false;
    struct mt_cpu_dvfs *p;
    int i;
    unsigned long flags;
#endif
    int ret = 0;

    FUNC_ENTER(FUNC_LV_API);

#if 0
    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        cpufreq_lock(flags);

        if (enabled) {
            /* enable CPU DVFS */
            if (p->dvfs_disable_by_suspend) {
                p->dvfs_disable_count--;
                cpufreq_dbg("enable %s DVFS: dvfs_disable_count = %d\n", p->name, p->dvfs_disable_count);

                if (p->dvfs_disable_count <= 0)
                    p->dvfs_disable_by_suspend = false;
                else
                    cpufreq_dbg("someone still disable %s DVFS and cant't enable it\n", p->name);
            } else
                cpufreq_dbg("%s DVFS already enabled\n", p->name);
        } else {
            /* disable DVFS */
            p->dvfs_disable_count++;

            if (p->dvfs_disable_by_suspend)
                cpufreq_dbg("%s DVFS already disabled\n", p->name);
            else {
                p->dvfs_disable_by_suspend = true;
                set_normal_max_opp = true;
            }
        }

        cpufreq_unlock(flags);

        if (set_normal_max_opp) {
            struct cpufreq_policy *policy = cpufreq_cpu_get(p->cpu_id);

            if (policy) {
                cpufreq_driver_target(policy, cpu_dvfs_get_normal_max_freq(p), CPUFREQ_RELATION_L);
                cpufreq_cpu_put(policy);
            } else {
                cpufreq_warn("can't get cpufreq policy to disable %s DVFS\n", p->name);
                ret = -1;
            }
        }

        set_normal_max_opp = false;
    }
#endif
    FUNC_EXIT(FUNC_LV_API);

    return ret;
}
EXPORT_SYMBOL(mt_cpufreq_state_set);

/* Power Table */
#if 0
#define P_MCU_L         (1243)  /* MCU Leakage Power          */
#define P_MCU_T         (2900)  /* MCU Total Power            */
#define P_CA7_L         (110)   /* CA7 Leakage Power          */
#define P_CA7_T         (305)   /* Single CA7 Core Power      */

#define P_MCL99_105C_L  (1243)  /* MCL99 Leakage Power @ 105C */
#define P_MCL99_25C_L   (93)    /* MCL99 Leakage Power @ 25C  */
#define P_MCL50_105C_L  (587)   /* MCL50 Leakage Power @ 105C */
#define P_MCL50_25C_L   (35)    /* MCL50 Leakage Power @ 25C  */

#define T_105           (105)   /* Temperature 105C           */
#define T_65            (65)    /* Temperature 65C            */
#define T_25            (25)    /* Temperature 25C            */

#define P_MCU_D ((P_MCU_T - P_MCU_L) - 8 * (P_CA7_T - P_CA7_L)) /* MCU dynamic power except of CA7 cores */

#define P_TOTAL_CORE_L ((P_MCL99_105C_L  * 27049) / 100000) /* Total leakage at T_65 */
#define P_EACH_CORE_L  ((P_TOTAL_CORE_L * ((P_CA7_L * 1000) / P_MCU_L)) / 1000) /* 1 core leakage at T_65 */

#define P_CA7_D_1_CORE ((P_CA7_T - P_CA7_L) * 1) /* CA7 dynamic power for 1 cores turned on */
#define P_CA7_D_2_CORE ((P_CA7_T - P_CA7_L) * 2) /* CA7 dynamic power for 2 cores turned on */
#define P_CA7_D_3_CORE ((P_CA7_T - P_CA7_L) * 3) /* CA7 dynamic power for 3 cores turned on */
#define P_CA7_D_4_CORE ((P_CA7_T - P_CA7_L) * 4) /* CA7 dynamic power for 4 cores turned on */

#define A_1_CORE (P_MCU_D + P_CA7_D_1_CORE) /* MCU dynamic power for 1 cores turned on */
#define A_2_CORE (P_MCU_D + P_CA7_D_2_CORE) /* MCU dynamic power for 2 cores turned on */
#define A_3_CORE (P_MCU_D + P_CA7_D_3_CORE) /* MCU dynamic power for 3 cores turned on */
#define A_4_CORE (P_MCU_D + P_CA7_D_4_CORE) /* MCU dynamic power for 4 cores turned on */

static void _power_calculation(struct mt_cpu_dvfs *p, int idx, int ncpu)
{
    int multi = 0, p_dynamic = 0, p_leakage = 0, freq_ratio = 0, volt_square_ratio = 0;
    int possible_cpu = 0;

    FUNC_ENTER(FUNC_LV_HELP);

    possible_cpu = num_possible_cpus(); // TODO: FIXME

    volt_square_ratio = (((p->opp_tbl[idx].cpufreq_volt * 100) / 1000) *
            ((p->opp_tbl[idx].cpufreq_volt * 100) / 1000)) / 100;
    freq_ratio = (p->opp_tbl[idx].cpufreq_khz / 1700);

    cpufreq_dbg("freq_ratio = %d, volt_square_ratio %d\n", freq_ratio, volt_square_ratio);

    multi = ((p->opp_tbl[idx].cpufreq_volt * 100) / 1000) *
        ((p->opp_tbl[idx].cpufreq_volt * 100) / 1000) *
        ((p->opp_tbl[idx].cpufreq_volt * 100) / 1000);

    switch (ncpu) {
        case 0:
            /* 1 core */
            p_dynamic = (((A_1_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 7 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            cpufreq_dbg("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;

        case 1:
            /* 2 core */
            p_dynamic = (((A_2_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 6 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            cpufreq_dbg("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;

        case 2:
            /* 3 core */
            p_dynamic = (((A_3_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 5 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            cpufreq_dbg("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;

        case 3:
            /* 4 core */
            p_dynamic = (((A_4_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 4 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            cpufreq_dbg("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;

        default:
            break;
    }

    p->power_tbl[idx * possible_cpu + ncpu].cpufreq_ncpu    = ncpu + 1;
    p->power_tbl[idx * possible_cpu + ncpu].cpufreq_khz     = p->opp_tbl[idx].cpufreq_khz;
    p->power_tbl[idx * possible_cpu + ncpu].cpufreq_power   = p_dynamic + p_leakage;

    cpufreq_dbg("p->power_tbl[%d]: cpufreq_ncpu = %d, cpufreq_khz = %d, cpufreq_power = %d\n",
            (idx * possible_cpu + ncpu),
            p->power_tbl[idx * possible_cpu + ncpu].cpufreq_ncpu,
            p->power_tbl[idx * possible_cpu + ncpu].cpufreq_khz,
            p->power_tbl[idx * possible_cpu + ncpu].cpufreq_power
            );

    FUNC_EXIT(FUNC_LV_HELP);
}

static int setup_power_table(struct mt_cpu_dvfs *p)
{
    static const unsigned int pwr_tbl_cgf[] = { 0, 0, 1, 0, 1, 0, 1, 0, };
    unsigned int pwr_eff_tbl[NR_MAX_OPP_TBL][NR_MAX_CPU];
    unsigned int pwr_eff_num;
    int possible_cpu;
    int i, j;
    int ret = 0;

    FUNC_ENTER(FUNC_LV_LOCAL);

    BUG_ON(NULL == p);

    if (p->power_tbl)
        goto out;

    cpufreq_dbg("P_MCU_D = %d\n", P_MCU_D);
    cpufreq_dbg("P_CA7_D_1_CORE = %d, P_CA7_D_2_CORE = %d, P_CA7_D_3_CORE = %d, P_CA7_D_4_CORE = %d\n", P_CA7_D_1_CORE, P_CA7_D_2_CORE, P_CA7_D_3_CORE, P_CA7_D_4_CORE);
    cpufreq_dbg("P_TOTAL_CORE_L = %d, P_EACH_CORE_L = %d\n", P_TOTAL_CORE_L, P_EACH_CORE_L);
    cpufreq_dbg("A_1_CORE = %d, A_2_CORE = %d, A_3_CORE = %d, A_4_CORE = %d\n", A_1_CORE, A_2_CORE, A_3_CORE, A_4_CORE);

    possible_cpu = num_possible_cpus(); // TODO: FIXME

    /* allocate power table */
    memset((void *)pwr_eff_tbl, 0, sizeof(pwr_eff_tbl));
    p->power_tbl = kzalloc(p->nr_opp_tbl * possible_cpu * sizeof(struct mt_cpu_power_info), GFP_KERNEL);

    if (NULL == p->power_tbl) {
        ret = -ENOMEM;
        goto out;
    }

    /* setup power efficiency array */
    for (i = 0, pwr_eff_num = 0; i < possible_cpu; i++) {
        if (1 == pwr_tbl_cgf[i])
            pwr_eff_num++;
    }

    for (i = 0; i < p->nr_opp_tbl; i++) {
        for (j = 0; j < possible_cpu; j++) {
            if (1 == pwr_tbl_cgf[j])
                pwr_eff_tbl[i][j] = 1;
        }
    }

    p->nr_power_tbl = p->nr_opp_tbl * (possible_cpu - pwr_eff_num);

    /* calc power and fill in power table */
    for (i = 0; i < p->nr_opp_tbl; i++) {
        for (j = 0; j < possible_cpu; j++) {
            if (0 == pwr_eff_tbl[i][j])
                _power_calculation(p, i, j);
        }
    }

    /* sort power table */
    for (i = p->nr_opp_tbl * possible_cpu; i > 0; i--) {
        for (j = 1; j <= i; j++) {
            if (p->power_tbl[j - 1].cpufreq_power < p->power_tbl[j].cpufreq_power) {
                struct mt_cpu_power_info tmp;

                tmp.cpufreq_khz                 = p->power_tbl[j - 1].cpufreq_khz;
                tmp.cpufreq_ncpu                = p->power_tbl[j - 1].cpufreq_ncpu;
                tmp.cpufreq_power               = p->power_tbl[j - 1].cpufreq_power;

                p->power_tbl[j - 1].cpufreq_khz   = p->power_tbl[j].cpufreq_khz;
                p->power_tbl[j - 1].cpufreq_ncpu  = p->power_tbl[j].cpufreq_ncpu;
                p->power_tbl[j - 1].cpufreq_power = p->power_tbl[j].cpufreq_power;

                p->power_tbl[j].cpufreq_khz     = tmp.cpufreq_khz;
                p->power_tbl[j].cpufreq_ncpu    = tmp.cpufreq_ncpu;
                p->power_tbl[j].cpufreq_power   = tmp.cpufreq_power;
            }
        }
    }

    /* dump power table */
    for (i = 0; i < p->nr_opp_tbl * possible_cpu; i++) {
        cpufreq_dbg("[%d] = { .khz = %d, .ncup = %d, .power = %d }\n",
                p->power_tbl[i].cpufreq_khz,
                p->power_tbl[i].cpufreq_ncpu,
                p->power_tbl[i].cpufreq_power
                );
    }

out:
    FUNC_EXIT(FUNC_LV_LOCAL);

    return ret;
}
#else
static void _power_calculation(struct mt_cpu_dvfs *p, int oppidx, int ncpu)
{
#define CA53_REF_POWER	2286	/* mW  */
#define CA53_REF_FREQ	1690000 /* KHz */
#define CA53_REF_VOLT	100000	/* mV * 100 */

    int p_dynamic = 0, p_leakage = 0, ref_freq, ref_volt;
    int possible_cpu = max_cpu_num;//num_possible_cpus(); // TODO: FIXME

    FUNC_ENTER(FUNC_LV_HELP);

    p_dynamic = CA53_REF_POWER;
    ref_freq  = CA53_REF_FREQ;
    ref_volt  = CA53_REF_VOLT;

    // TODO: should not use a hardcode value for leakage power
#if 0
    p_leakage = mt_spower_get_leakage(MT_SPOWER_CA7, p->opp_tbl[oppidx].cpufreq_volt / 100, 65);
#else
    p_leakage = 155;
#endif

    p_dynamic = p_dynamic *
        (p->opp_tbl[oppidx].cpufreq_khz / 1000) / (ref_freq / 1000) *
        p->opp_tbl[oppidx].cpufreq_volt / ref_volt *
        p->opp_tbl[oppidx].cpufreq_volt / ref_volt +
        p_leakage;

    p->power_tbl[NR_MAX_OPP_TBL * (possible_cpu - 1 - ncpu) + oppidx].cpufreq_ncpu  = ncpu + 1;
    p->power_tbl[NR_MAX_OPP_TBL * (possible_cpu - 1 - ncpu) + oppidx].cpufreq_khz   = p->opp_tbl[oppidx].cpufreq_khz;
    p->power_tbl[NR_MAX_OPP_TBL * (possible_cpu - 1 - ncpu) + oppidx].cpufreq_power = p_dynamic * (ncpu + 1) / possible_cpu;

    FUNC_EXIT(FUNC_LV_HELP);
}

static int setup_power_table(struct mt_cpu_dvfs *p)
{
    static const unsigned int pwr_tbl_cgf[NR_MAX_CPU] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned int pwr_eff_tbl[NR_MAX_OPP_TBL][NR_MAX_CPU];
    unsigned int pwr_eff_num;
    int possible_cpu = max_cpu_num;//num_possible_cpus(); // TODO: FIXME
    int i, j;
    int ret = 0;

    FUNC_ENTER(FUNC_LV_LOCAL);

    BUG_ON(NULL == p);

    if (p->power_tbl)
        goto out;

    /* allocate power table */
    memset((void *)pwr_eff_tbl, 0, sizeof(pwr_eff_tbl));
    p->power_tbl = kzalloc(p->nr_opp_tbl * possible_cpu * sizeof(struct mt_cpu_power_info), GFP_KERNEL);

    if (NULL == p->power_tbl) {
        ret = -ENOMEM;
        goto out;
    }

    /* setup power efficiency array */
    for (i = 0, pwr_eff_num = 0; i < possible_cpu; i++) {
        if (1 == pwr_tbl_cgf[i])
            pwr_eff_num++;
    }

    for (i = 0; i < p->nr_opp_tbl; i++) {
        for (j = 0; j < possible_cpu; j++) {
            if (1 == pwr_tbl_cgf[j])
                pwr_eff_tbl[i][j] = 1;
        }
    }

    p->nr_power_tbl = p->nr_opp_tbl * (possible_cpu - pwr_eff_num);

    /* calc power and fill in power table */
    for (i = 0; i < p->nr_opp_tbl; i++) {
        for (j = 0; j < possible_cpu; j++) {
            if (0 == pwr_eff_tbl[i][j])
                _power_calculation(p, i, j);
        }
    }

    /* sort power table */
    for (i = p->nr_opp_tbl * possible_cpu; i > 0; i--) {
        for (j = 1; j <= i; j++) {
            if (p->power_tbl[j - 1].cpufreq_power < p->power_tbl[j].cpufreq_power) {
                struct mt_cpu_power_info tmp;

                tmp.cpufreq_khz                 = p->power_tbl[j - 1].cpufreq_khz;
                tmp.cpufreq_ncpu                = p->power_tbl[j - 1].cpufreq_ncpu;
                tmp.cpufreq_power               = p->power_tbl[j - 1].cpufreq_power;

                p->power_tbl[j - 1].cpufreq_khz   = p->power_tbl[j].cpufreq_khz;
                p->power_tbl[j - 1].cpufreq_ncpu  = p->power_tbl[j].cpufreq_ncpu;
                p->power_tbl[j - 1].cpufreq_power = p->power_tbl[j].cpufreq_power;

                p->power_tbl[j].cpufreq_khz     = tmp.cpufreq_khz;
                p->power_tbl[j].cpufreq_ncpu    = tmp.cpufreq_ncpu;
                p->power_tbl[j].cpufreq_power   = tmp.cpufreq_power;
            }
        }
    }

    /* dump power table */
    for (i = 0; i < p->nr_opp_tbl * possible_cpu; i++) {
        cpufreq_info("[%d] = { .cpufreq_khz = %d,\t.cpufreq_ncpu = %d,\t.cpufreq_power = %d }\n",
                i,
                p->power_tbl[i].cpufreq_khz,
                p->power_tbl[i].cpufreq_ncpu,
                p->power_tbl[i].cpufreq_power
                );
    }

#if 0 // def CONFIG_THERMAL // TODO: FIXME
    mtk_cpufreq_register(p->power_tbl, p->nr_power_tbl);
#endif

out:
    FUNC_EXIT(FUNC_LV_LOCAL);

    return ret;
}
#endif

static int _mt_cpufreq_setup_freqs_table(struct cpufreq_policy *policy, struct mt_cpu_freq_info *freqs, int num)
{
    struct mt_cpu_dvfs *p;
    struct cpufreq_frequency_table *table;
    int i, ret = 0;

    FUNC_ENTER(FUNC_LV_LOCAL);

    BUG_ON(NULL == policy);
    BUG_ON(NULL == freqs);

    p = id_to_cpu_dvfs(_get_cpu_dvfs_id(policy->cpu));

    if (NULL == p->freq_tbl_for_cpufreq) {
        table = kzalloc((num + 1) * sizeof(*table), GFP_KERNEL);

        if (NULL == table) {
            ret = -ENOMEM;
            goto out;
        }

        for (i = 0; i < num; i++) {
            table[i].index = i;
            table[i].frequency = freqs[i].cpufreq_khz;
        }

        table[num].index = i; // TODO: FIXME, why need this???
        table[num].frequency = CPUFREQ_TABLE_END;

        p->opp_tbl = freqs;
        p->nr_opp_tbl = num;
        p->freq_tbl_for_cpufreq = table;
    }

#ifdef CONFIG_CPU_FREQ
    ret = cpufreq_frequency_table_cpuinfo(policy, p->freq_tbl_for_cpufreq);

    if (!ret)
        cpufreq_frequency_table_get_attr(p->freq_tbl_for_cpufreq, policy->cpu);
#endif

    if (NULL == p->power_tbl)
        p->ops->setup_power_table(p);

out:
    FUNC_EXIT(FUNC_LV_LOCAL);

    return 0;
}

void mt_cpufreq_enable_by_ptpod(enum mt_cpu_dvfs_id id)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);
    unsigned int mt6325_chip_ver = get_pmic_mt6325_cid();

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(NULL == p);

    p->dvfs_disable_by_ptpod = false;

    /* Turbo mode is enabled when:
     *  1. PMIC 6325 is E3 or later version
     *  2. CPU segment is 1.7GHz
     *  3. PTPOD is enabled
     */    
    if (mt6325_chip_ver >= PMIC6325_E3_CID_CODE && p->cpu_level == CPU_LEVEL_1 && AllowTurboMode) {
        cpufreq_info("@%s: Turbo mode enabled!\n", __func__);
        p->turbo_mode = 1;
    }

    if (!cpu_dvfs_is_availiable(p)) {
        FUNC_EXIT(FUNC_LV_API);
        return;
    }
    
    _mt_cpufreq_set(id, p->idx_opp_tbl_for_late_resume);

    FUNC_EXIT(FUNC_LV_API);
}
EXPORT_SYMBOL(mt_cpufreq_enable_by_ptpod);

unsigned int mt_cpufreq_disable_by_ptpod(enum mt_cpu_dvfs_id id)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    FUNC_ENTER(FUNC_LV_API);

    BUG_ON(NULL == p);

    p->dvfs_disable_by_ptpod = true;

    if (!cpu_dvfs_is_availiable(p)) {
        FUNC_EXIT(FUNC_LV_API);
        return 0;
    }

#if 0 // XXX: BUG_ON(irqs_disabled()) @ __cpufreq_notify_transition()
    {
        struct cpufreq_policy *policy;
        policy = cpufreq_cpu_get(p->cpu_id);

        if (policy) {
            cpufreq_driver_target(policy, cpu_dvfs_get_normal_max_freq(p), CPUFREQ_RELATION_L);
            cpufreq_cpu_put(policy);
        } else
            cpufreq_warn("can't get cpufreq policy to disable %s DVFS\n", p->name);
    }
#else
    p->idx_opp_tbl_for_late_resume = p->idx_opp_tbl;
    _mt_cpufreq_set(id, p->idx_normal_max_opp); // XXX: useless, decided @ _calc_new_opp_idx()
#endif

    FUNC_EXIT(FUNC_LV_API);

    return cpu_dvfs_get_cur_freq(p);
}
EXPORT_SYMBOL(mt_cpufreq_disable_by_ptpod);

void mt_cpufreq_thermal_protect(unsigned int limited_power)
{
    FUNC_ENTER(FUNC_LV_API);

    cpufreq_info("%s(): limited_power = %d\n", __func__, limited_power);

#ifdef CONFIG_CPU_FREQ
    {
        struct cpufreq_policy *policy;
        struct mt_cpu_dvfs *p;
        int possible_cpu;
        int ncpu;
        int found = 0;
        unsigned long flag;
        int i;
    
        policy = cpufreq_cpu_get(0);    // TODO: FIXME if it has more than one DVFS policy
        if (NULL == policy)
            goto no_policy;

        p = id_to_cpu_dvfs(_get_cpu_dvfs_id(policy->cpu));

        BUG_ON(NULL == p);

        cpufreq_lock(flag);                                     /* <- lock */

        // save current oppidx
        if (!p->thermal_protect_limited_power)            
            p->idx_opp_tbl_for_thermal_thro = p->idx_opp_tbl;

        p->thermal_protect_limited_power = limited_power;
        possible_cpu = max_cpu_num;//num_possible_cpus(); // TODO: FIXME

        /* no limited */
        if (0 == limited_power) {
            p->limited_max_ncpu = possible_cpu;
            p->limited_max_freq = cpu_dvfs_get_max_freq(p);
            // restore oppidx
            p->idx_opp_tbl = p->idx_opp_tbl_for_thermal_thro;
        } 
        else {                       
            for (ncpu = possible_cpu; ncpu > 0; ncpu--) {
                for (i = 0; i < p->nr_opp_tbl * possible_cpu; i++) {
                    if (p->power_tbl[i].cpufreq_power <= limited_power) { // p->power_tbl[i].cpufreq_ncpu == ncpu &&
                        p->limited_max_ncpu = p->power_tbl[i].cpufreq_ncpu;
                        p->limited_max_freq = p->power_tbl[i].cpufreq_khz;
                        found = 1;
                        ncpu = 0; /* for break outer loop */
                        break;
                    }
                }
            }

            /* not found and use lowest power limit */
            if (!found) {
                p->limited_max_ncpu = p->power_tbl[p->nr_power_tbl - 1].cpufreq_ncpu;
                p->limited_max_freq = p->power_tbl[p->nr_power_tbl - 1].cpufreq_khz;
            }
        }

        cpufreq_info("found = %d, limited_max_freq = %d, limited_max_ncpu = %d\n", found, p->limited_max_freq, p->limited_max_ncpu);

        cpufreq_unlock(flag);                                   /* <- unlock */
        hps_set_cpu_num_limit(LIMIT_THERMAL, p->limited_max_ncpu, 0);
        // correct opp idx will be calcualted in _thermal_limited_verify()
        _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, -1);
        cpufreq_cpu_put(policy);                                /* <- policy put */
    }
no_policy:
#endif

    FUNC_EXIT(FUNC_LV_API);
}
EXPORT_SYMBOL(mt_cpufreq_thermal_protect);

/* for ramp down */
void mt_cpufreq_set_ramp_down_count_const(enum mt_cpu_dvfs_id id, int count)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    BUG_ON(NULL == p);

    p->ramp_down_count_const = count;
}
EXPORT_SYMBOL(mt_cpufreq_set_ramp_down_count_const);

#ifdef CONFIG_CPU_DVFS_RAMP_DOWN
static int _keep_max_freq(struct mt_cpu_dvfs *p, unsigned int freq_old, unsigned int freq_new) // TODO: inline @ mt_cpufreq_target()
{
    int ret = 0;

    FUNC_ENTER(FUNC_LV_HELP);

    if (RAMP_DOWN_TIMES < p->ramp_down_count_const)
        p->ramp_down_count_const--;
    else
        p->ramp_down_count_const = RAMP_DOWN_TIMES;

    if (freq_new < freq_old && p->ramp_down_count < p->ramp_down_count_const) {
        ret = 1;
        p->ramp_down_count++;
    } else
        p->ramp_down_count = 0;

    FUNC_ENTER(FUNC_LV_HELP);

    return ret;
}
#endif

static int _search_available_freq_idx(struct mt_cpu_dvfs *p, unsigned int target_khz, unsigned int relation) /* return -1 (not found) */
{
    int new_opp_idx = -1;
    int i;

    FUNC_ENTER(FUNC_LV_HELP);

    if (CPUFREQ_RELATION_L == relation) {
        for (i = (signed)(p->nr_opp_tbl - 1); i >= 0; i--) {
            if (cpu_dvfs_get_freq_by_idx(p, i) >= target_khz) {
                new_opp_idx = i;
                break;
            }
        }
    } else { /* CPUFREQ_RELATION_H */
        for (i = 0; i < (signed)p->nr_opp_tbl; i++) {
            if (cpu_dvfs_get_freq_by_idx(p, i) <= target_khz) {
                new_opp_idx = i;
                break;
            }
        }
    }

    FUNC_EXIT(FUNC_LV_HELP);

    return new_opp_idx;
}

static int _thermal_limited_verify(struct mt_cpu_dvfs *p, int new_opp_idx)
{
    unsigned int target_khz = cpu_dvfs_get_freq_by_idx(p, new_opp_idx);
    int possible_cpu = 0;
    unsigned int online_cpu = 0;
    int found = 0;
    int i;

    FUNC_ENTER(FUNC_LV_HELP);

    possible_cpu = max_cpu_num; //num_possible_cpus(); // TODO: FIXME
    online_cpu = num_online_cpus(); // TODO: FIXME

    //cpufreq_dbg("%s(): begin, idx = %d, online_cpu = %d\n", __func__, new_opp_idx, online_cpu);

    /* no limited */
    if (0 == p->thermal_protect_limited_power)
        return new_opp_idx;

    for (i = 0; i < p->nr_opp_tbl * possible_cpu; i++) {
        if (p->power_tbl[i].cpufreq_ncpu == p->limited_max_ncpu
                && p->power_tbl[i].cpufreq_khz  == p->limited_max_freq
           )
            break;
    }

    cpufreq_info("%s(): idx = %d, limited_max_ncpu = %d, limited_max_freq = %d\n", __func__, i, p->limited_max_ncpu, p->limited_max_freq);

    for (; i < p->nr_opp_tbl * possible_cpu; i++) {
        if (p->power_tbl[i].cpufreq_ncpu == online_cpu) {
            if (target_khz >= p->power_tbl[i].cpufreq_khz) {
                found = 1;
                break;
            }
        }
    }

    if (found) {
        target_khz = p->power_tbl[i].cpufreq_khz;
        cpufreq_info("%s(): freq found, idx = %d, target_khz = %d, online_cpu = %d\n", __func__, i, target_khz, online_cpu);
    } else {
        target_khz = p->limited_max_freq;
        cpufreq_info("%s(): freq not found, set to limited_max_freq = %d\n", __func__, target_khz);
    }

    i = _search_available_freq_idx(p, target_khz, CPUFREQ_RELATION_H); // TODO: refine this function for idx searching

    FUNC_EXIT(FUNC_LV_HELP);

    return i;
}

static unsigned int _calc_new_opp_idx(struct mt_cpu_dvfs *p, int new_opp_idx)
{
    int idx;

    FUNC_ENTER(FUNC_LV_HELP);

    BUG_ON(NULL == p);

    /* for ramp down */
#ifdef CONFIG_CPU_DVFS_RAMP_DOWN
    if (_keep_max_freq(p, cpu_dvfs_get_cur_freq(p), cpu_dvfs_get_freq_by_idx(p, new_opp_idx))) {
        cpufreq_info("%s(): ramp down, idx = %d, freq_old = %d, freq_new = %d\n", 
                    __func__, 
                    new_opp_idx, 
                    cpu_dvfs_get_cur_freq(p), 
                    cpu_dvfs_get_freq_by_idx(p, new_opp_idx)
                    );
        new_opp_idx = p->idx_opp_tbl;
    }
#endif

    /* HEVC */
    if (p->limited_freq_by_hevc) {
        idx = _search_available_freq_idx(p, p->limited_freq_by_hevc, CPUFREQ_RELATION_L);

        if (idx != -1) {
            new_opp_idx = idx;
            cpufreq_info("%s(): hevc limited freq, idx = %d\n", __func__, new_opp_idx);
        }
    }

#ifdef CONFIG_CPU_DVFS_DOWNGRADE_FREQ
    if (true == p->downgrade_freq_for_ptpod) {
        if (cpu_dvfs_get_freq_by_idx(p, new_opp_idx) > p->downgrade_freq) {
            idx = _search_available_freq_idx(p, p->downgrade_freq, CPUFREQ_RELATION_H);

            if (idx != -1) {
                new_opp_idx = idx;
                cpufreq_info("%s(): downgrade freq, idx = %d\n", __func__, new_opp_idx);
            }
        }
    }
#endif /* CONFIG_CPU_DVFS_DOWNGRADE_FREQ */

    /* search thermal limited freq */
    idx = _thermal_limited_verify(p, new_opp_idx);

    if (idx != -1 && idx != new_opp_idx) {
        new_opp_idx = idx;
        cpufreq_info("%s(): thermal limited freq, idx = %d\n", __func__, new_opp_idx);
    }

    /* for early suspend */
    if (p->dvfs_disable_by_early_suspend) {
        new_opp_idx = p->idx_normal_max_opp; // (new_opp_idx < p->idx_normal_max_opp) ? p->idx_normal_max_opp : new_opp_idx;
        cpufreq_info("%s(): for early suspend, idx = %d\n", __func__, new_opp_idx);
    }

    /* for suspend */
    if (p->dvfs_disable_by_suspend)
        new_opp_idx = p->idx_normal_max_opp;

    /* for power throttling */
#ifdef  CONFIG_CPU_DVFS_POWER_THROTTLING
    if (p->pwr_thro_mode && new_opp_idx < p->idx_pwr_thro_max_opp) {
        new_opp_idx = p->idx_pwr_thro_max_opp;
        cpufreq_info("%s(): for power throttling = %d\n", __func__, new_opp_idx);
    }
#endif

    /* for ptpod init */
    if (p->dvfs_disable_by_ptpod) {
        // at least CPU_DVFS_FREQ6 will make sure VBoot >= 1V
        idx = _search_available_freq_idx(p, CPU_DVFS_FREQ6, CPUFREQ_RELATION_L);
        if (idx != -1) {
            new_opp_idx = idx;
            cpufreq_info("%s(): for ptpod init, idx = %d\n", __func__, new_opp_idx);
        }      
    }

    FUNC_EXIT(FUNC_LV_HELP);

    return new_opp_idx;
}

#ifdef  CONFIG_CPU_DVFS_POWER_THROTTLING
static void _calc_power_throttle_idx(struct mt_cpu_dvfs *p)
{
    FUNC_ENTER(FUNC_LV_HELP);

    cpufreq_dbg("%s(): original idx = %d\n", __func__, p->idx_pwr_thro_max_opp);

    if (!p->pwr_thro_mode)
        p->idx_pwr_thro_max_opp = 0;
    else if ((p->pwr_thro_mode & PWR_THRO_MODE_LBAT_936MHZ)
             || (p->pwr_thro_mode & PWR_THRO_MODE_BAT_PER_936MHZ)) {
        switch (p->cpu_level) {
            case CPU_LEVEL_0:
            case CPU_LEVEL_1:
                p->idx_pwr_thro_max_opp = 5;
                break;     
            case CPU_LEVEL_2:
                p->idx_pwr_thro_max_opp = 4;
                break;
            case CPU_LEVEL_3:
                p->idx_pwr_thro_max_opp = 2;
                break;    
            default:
                break;
        }
    } else if (p->pwr_thro_mode & PWR_THRO_MODE_BAT_OC_1170MHZ)
        p->idx_pwr_thro_max_opp = 4;    // 1.17G in CPU LV0 or LV1
    else if (p->pwr_thro_mode & PWR_THRO_MODE_BAT_OC_1287MHZ)
        p->idx_pwr_thro_max_opp = 2;    // 1.287G in CPU LV2
    else if (p->pwr_thro_mode & PWR_THRO_MODE_BAT_OC_1417MHZ) {
        switch (p->cpu_level) {
            case CPU_LEVEL_0:
                p->idx_pwr_thro_max_opp = 3;
                break;
            case CPU_LEVEL_1:
                p->idx_pwr_thro_max_opp = 2;
                break;
            default:
                break;
        }
    }

    cpufreq_dbg("%s(): new idx = %d\n", __func__, p->idx_pwr_thro_max_opp);

    FUNC_EXIT(FUNC_LV_HELP);

    return;
}

static void bat_per_protection_powerlimit(BATTERY_PERCENT_LEVEL level)
{
    struct mt_cpu_dvfs *p;
    int i;
    unsigned long flags;

    cpufreq_dbg("@%s: level: %d\n", __func__, level);

    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        cpufreq_lock(flags);

        if (!p->pwr_thro_mode)
            p->idx_opp_tbl_for_pwr_thro = p->idx_opp_tbl;

        switch (level) {
            case BATTERY_PERCENT_LEVEL_1:
                // Trigger CPU Limit to under 1G
                p->pwr_thro_mode |= PWR_THRO_MODE_BAT_PER_936MHZ;
                break;
            default:
                // Unlimit CPU
                p->pwr_thro_mode &= ~PWR_THRO_MODE_BAT_PER_936MHZ;
                break;
        }

        _calc_power_throttle_idx(p);

        if (!p->pwr_thro_mode)
            p->idx_opp_tbl = p->idx_opp_tbl_for_pwr_thro;

        cpufreq_unlock(flags);

        switch (level) {
            case BATTERY_PERCENT_LEVEL_1:
                // Limit CPU core num to 4
                hps_set_cpu_num_limit(LIMIT_LOW_BATTERY, 4, 0);
                break;
            default:
                //Unlimit CPU core num if no Lbat_protect_limit
                if (!(p->pwr_thro_mode & PWR_THRO_MODE_LBAT_936MHZ))
                    hps_set_cpu_num_limit(LIMIT_LOW_BATTERY, max_cpu_num, 0);
                break;
        }

        _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, -1);
    }
}

static void bat_oc_protection_powerlimit(BATTERY_OC_LEVEL level)
{
    struct mt_cpu_dvfs *p;
    int i;
    unsigned long flags;

    cpufreq_dbg("@%s: level: %d\n", __func__, level);

    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        cpufreq_lock(flags);

        if (!p->pwr_thro_mode)
            p->idx_opp_tbl_for_pwr_thro = p->idx_opp_tbl;

        // apply different OC power throttling mode according to CPU_LV and possible CPU core num
        switch (level) {
            case BATTERY_OC_LEVEL_1:
                if (max_cpu_num == 8) 
                    // Battery OC trigger CPU Limit to under 1.5G
                    p->pwr_thro_mode |= PWR_THRO_MODE_BAT_OC_1417MHZ;
                else if (max_cpu_num == 4 && p->cpu_level <= CPU_LEVEL_1)
                    // Battery OC trigger CPU Limit to under 1.2G
                    p->pwr_thro_mode |= PWR_THRO_MODE_BAT_OC_1170MHZ;
                else if (max_cpu_num == 4 && p->cpu_level == CPU_LEVEL_2)
                    // Battery OC trigger CPU Limit to under 1.4G
                    p->pwr_thro_mode |= PWR_THRO_MODE_BAT_OC_1287MHZ;
                break;
            default:
                // Unlimit CPU
                if (max_cpu_num == 8)
                    p->pwr_thro_mode &= ~PWR_THRO_MODE_BAT_OC_1417MHZ;
                else if (max_cpu_num == 4 && p->cpu_level <= CPU_LEVEL_1)
                    p->pwr_thro_mode &= ~PWR_THRO_MODE_BAT_OC_1170MHZ;
                else if (max_cpu_num == 4 && p->cpu_level == CPU_LEVEL_2)
                    p->pwr_thro_mode &= ~PWR_THRO_MODE_BAT_OC_1287MHZ;
                break;
        }  

        _calc_power_throttle_idx(p);

        if (!p->pwr_thro_mode)
            p->idx_opp_tbl = p->idx_opp_tbl_for_pwr_thro;

        cpufreq_unlock(flags);
        _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, -1);
    }
}

void Lbat_protection_powerlimit(LOW_BATTERY_LEVEL level)
{
    struct mt_cpu_dvfs *p;
    int i;
    unsigned long flags;

    cpufreq_dbg("@%s: level: %d\n", __func__, level);

    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        cpufreq_lock(flags);

        if (!p->pwr_thro_mode)
            p->idx_opp_tbl_for_pwr_thro = p->idx_opp_tbl;

        switch (level) {
            case LOW_BATTERY_LEVEL_1:
            case LOW_BATTERY_LEVEL_2:
                //1st and 2nd LV trigger CPU Limit to under 1.0G
                p->pwr_thro_mode |= PWR_THRO_MODE_LBAT_936MHZ;
                break;                
            default:
                //Unlimit CPU
                p->pwr_thro_mode &= ~PWR_THRO_MODE_LBAT_936MHZ;
                break;
        }

        _calc_power_throttle_idx(p);

        if (!p->pwr_thro_mode)
            p->idx_opp_tbl = p->idx_opp_tbl_for_pwr_thro;

        cpufreq_unlock(flags);

        switch (level) {
            case LOW_BATTERY_LEVEL_1:
            case LOW_BATTERY_LEVEL_2:
                //1st and 2nd LV trigger limit CPU core num to 4
                hps_set_cpu_num_limit(LIMIT_LOW_BATTERY, 4, 0);
                break;
            default:
                //Unlimit CPU core num if no bat_per_limit
                if (!(p->pwr_thro_mode & PWR_THRO_MODE_BAT_PER_936MHZ))
                    hps_set_cpu_num_limit(LIMIT_LOW_BATTERY, max_cpu_num, 0);
                break;
        }

        _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, -1);
    }
}
#endif


#define EMI_FREQ_CHECK  0
unsigned int gEMI_DFS_enable = 1;
unsigned int gMd32_alive = 0;
unsigned int gPMIC_6325_version = 0;
extern void do_DRAM_DFS(int high_freq);
extern unsigned int mt_get_emi_freq(void);
extern unsigned int is_md32_enable(void);

/* 
 * Return value definitions
 *  0: 26M, 
 *  1: 3 PLL, 
 *  2: 1 PLL
 */
unsigned int mt_get_clk_mem_sel(void)
{
	unsigned int val;

    //CLK_CFG_0(0x10000040)[9:8]
    //clk_mem_sel
    //2'b00:clk26m
    //2'b01:dmpll_ck->3PLL
    //2'b10:ddr_x1_ck->1PLL
    val = (*(volatile unsigned int *)(CLK_CFG_0));
	val = (val>>8) & 0x3;  
	
	return val;
}
EXPORT_SYMBOL(mt_get_clk_mem_sel);

void check_pmic_6325_version(void)
{
    static int first_check = 1;
    
    if (first_check == 1)
    {
        unsigned int rdata;
        
        first_check = 0;
        
        pwrap_read(0x202, &rdata);
        gPMIC_6325_version = rdata;
        printk("gPMIC_6325_version = 0x%x\n", gPMIC_6325_version);
    }
}

/*
 * cpufreq driver
 */
static int _mt_cpufreq_verify(struct cpufreq_policy *policy)
{
    struct mt_cpu_dvfs *p;
    int ret = 0; /* cpufreq_frequency_table_verify() always return 0 */

    FUNC_ENTER(FUNC_LV_MODULE);

    p = id_to_cpu_dvfs(_get_cpu_dvfs_id(policy->cpu));

    BUG_ON(NULL == p);

#ifdef CONFIG_CPU_FREQ
    ret = cpufreq_frequency_table_verify(policy, p->freq_tbl_for_cpufreq);
#endif

    FUNC_EXIT(FUNC_LV_MODULE);

    return ret;
}

static int _mt_cpufreq_target(struct cpufreq_policy *policy, unsigned int target_freq, unsigned int relation)
{
    // unsigned int cpu;							// XXX: move to _cpufreq_set_locked()
    // struct cpufreq_freqs freqs;						// XXX: move to _cpufreq_set_locked()
    unsigned int new_opp_idx;

    enum mt_cpu_dvfs_id id = _get_cpu_dvfs_id(policy->cpu);

    // unsigned long flags;							// XXX: move to _mt_cpufreq_set()
    int ret = 0; /* -EINVAL; */

    FUNC_ENTER(FUNC_LV_MODULE);

    if (policy->cpu >= max_cpu_num
            || cpufreq_frequency_table_target(policy, id_to_cpu_dvfs(id)->freq_tbl_for_cpufreq, target_freq, relation, &new_opp_idx)
            || (id_to_cpu_dvfs(id) && id_to_cpu_dvfs(id)->dvfs_disable_by_procfs)
       )
        return -EINVAL;

    // freqs.old = policy->cur;						// XXX: move to _cpufreq_set_locked()
    // freqs.new = mt_cpufreq_max_frequency_by_DVS(id, new_opp_idx);	// XXX: move to _cpufreq_set_locked()
    // freqs.cpu = policy->cpu;						// XXX: move to _cpufreq_set_locked()

    // for_each_online_cpu(cpu) { // TODO: big LITTLE issue (id mapping)	// XXX: move to _cpufreq_set_locked()
    // 	freqs.cpu = cpu;						// XXX: move to _cpufreq_set_locked()
    // 	cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);	// XXX: move to _cpufreq_set_locked()
    // }									// XXX: move to _cpufreq_set_locked()

    // cpufreq_lock(flags);							// XXX: move to _mt_cpufreq_set()

    _mt_cpufreq_set(id, new_opp_idx);

    // cpufreq_unlock(flags);						// XXX: move to _mt_cpufreq_set()

    // for_each_online_cpu(cpu) { // TODO: big LITTLE issue (id mapping)	// XXX: move to _cpufreq_set_locked()
    // 	freqs.cpu = cpu;						// XXX: move to _cpufreq_set_locked()
    // 	cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);	// XXX: move to _cpufreq_set_locked()
    // }									// XXX: move to _cpufreq_set_locked()

    FUNC_EXIT(FUNC_LV_MODULE);

    return ret;
}

static int _mt_cpufreq_init(struct cpufreq_policy *policy)
{
    int ret = -EINVAL;

    FUNC_ENTER(FUNC_LV_MODULE);

    max_cpu_num = num_possible_cpus();

    if (policy->cpu >= max_cpu_num) // TODO: FIXME
        return -EINVAL;

    cpufreq_info("@%s: max_cpu_num: %d\n", __func__, max_cpu_num);

    policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
    cpumask_setall(policy->cpus);

    /*******************************************************
     * 1 us, assumed, will be overwrited by min_sampling_rate
     ********************************************************/
    policy->cpuinfo.transition_latency = 1000;

    /*********************************************
     * set default policy and cpuinfo, unit : Khz
     **********************************************/
    {
#define DORMANT_MODE_VOLT   80000

        enum mt_cpu_dvfs_id id = _get_cpu_dvfs_id(policy->cpu);
        struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);
        unsigned int lv = _mt_cpufreq_get_cpu_level();
        struct opp_tbl_info *opp_tbl_info = &opp_tbls[CPU_LV_TO_OPP_IDX(lv)];

        BUG_ON(NULL == p);
        BUG_ON(!(lv == CPU_LEVEL_0 || lv == CPU_LEVEL_1 || lv == CPU_LEVEL_2 || lv == CPU_LEVEL_3));

        p->cpu_level = lv;

        if (!cpu_dvfs_is_extbuck_valid()) {
            p->ops = &dvfs_ops_pmic_wrap;

            // change PMIC table for 4 core
            pw.set[PMIC_WRAP_PHASE_DEEPIDLE]._[IDX_DI_VPROC_NORMAL].cmd_addr = PMIC_ADDR_VPROC_VOSEL_CTRL;
            pw.set[PMIC_WRAP_PHASE_DEEPIDLE]._[IDX_DI_VPROC_NORMAL].cmd_wdata = _BITS_(1  : 1,  1);
            pw.set[PMIC_WRAP_PHASE_DEEPIDLE]._[IDX_DI_VPROC_SLEEP].cmd_addr = PMIC_ADDR_VPROC_VOSEL_CTRL;
            pw.set[PMIC_WRAP_PHASE_DEEPIDLE]._[IDX_DI_VPROC_SLEEP].cmd_wdata = _BITS_(1  : 1,  0);
            
            // set dpidle volt for Vproc
            pmic_config_interface(PMIC_ADDR_VSRAM_VOSEL_CTRL, 0x1, 0x1, 0x1);
            pmic_config_interface(PMIC_ADDR_VPROC_VOSEL, VOLT_TO_PMIC_VAL(DORMANT_MODE_VOLT), 0x7F, 0x0); // Set VPROC_VOSEL[6:0] to 0.7v

            // VSRAM auto-tracking settings
            // turn on tracking
            pmic_config_interface(0x42E, 0x7, 0x7, 0x0);        // 0x42E[2:0] = 3'b111
            // turn on soft change for VSRAM
            pmic_config_interface(0x502, 0x84FF, 0xFFFF, 0x0);  // 0x502[15:0] = 16'h84FF
            // HW mode vosel of VSRAM as VDVFS1_VOSEL + offset
            pmic_config_interface(0x506, 0x40, 0x7F, 0x0);      // 0x506[6:0] = 7'h40
            pmic_config_interface(0x430, 0xF01, 0x7F7F, 0x0);   // 0x430[14:0] = 15'hF01
            // VOSEL_ON_HB[14:8] = 1.15v, VPSEL_ON_LB[6:0] = 0.93125v
            pmic_config_interface(0x432, 0x5835, 0x7F7F, 0x0);  // 0x432[14:0] = 15'h5F35
            // VOSEL_SLEEP_LB = 0.6v
            pmic_config_interface(0x434, 0x0, 0x7F, 0x0);   // 0x434[6:0] = 7'h0 
            // set vosel as HW mode vosel for VSRAM & DVFS1
            pmic_config_interface(0x4B0, 0x1, 0x1, 0x1);    // 0x4B0[1] = 1'b1
            pmic_config_interface(0x4FC, 0x1, 0x1, 0x1);    // 0x4FC[1] = 1'b1
            // HW mode vosel for DVFS1
            pmic_config_interface(0x4BA, 0x30, 0x7F, 0x0);  // 0x4BA[6:0] = 7'h30 
            pmic_config_interface(0x4BC, 0x10, 0x7F, 0x0);  // 0x4BC[6:0] = 7'h10
            // enhancement DVFS11 performance
            pmic_config_interface(0x4B6, 0x84FF, 0xFFFF, 0x0);  // 0x4B6[15:0] = 16'h84FF
            pmic_config_interface(0x4C6, 0x13, 0x3F, 0x0);      // 0x4C6[5:0] = 6'h13
        } else {
            // set dpidle volt for Vproc
            mt6311_set_vdvfs11_vosel_ctrl(0x1);
            mt6311_set_vdvfs11_vosel(VOLT_TO_EXTBUCK_VAL(DORMANT_MODE_VOLT));
            // set dpidle volt for Vsram
            pmic_config_interface(PMIC_ADDR_VSRAM_VOSEL_CTRL, 0x1, 0x1, 0x1);
            pmic_config_interface(PMIC_ADDR_VSRAM_VOSEL, VOLT_TO_PMIC_VAL(MIN_VSRAM_VOLT), 0x7F, 0x9); // Set RG_VSRAM_VOSEL[15:9] to 0.93125v
        }

        ret = _mt_cpufreq_setup_freqs_table(policy,
                opp_tbl_info->opp_tbl,
                opp_tbl_info->size
                );

        policy->cpuinfo.max_freq = cpu_dvfs_get_max_freq(id_to_cpu_dvfs(id));
        policy->cpuinfo.min_freq = cpu_dvfs_get_min_freq(id_to_cpu_dvfs(id));

        policy->cur = _mt_cpufreq_get_cur_phy_freq(id); /* use cur phy freq is better */
        policy->max = cpu_dvfs_get_max_freq(id_to_cpu_dvfs(id));
        policy->min = cpu_dvfs_get_min_freq(id_to_cpu_dvfs(id));

        if (_sync_opp_tbl_idx(p) >= 0) /* sync p->idx_opp_tbl first before _restore_default_volt() */
            p->idx_normal_max_opp = p->idx_opp_tbl;

        /* restore default volt, sync opp idx, set default limit */
        _restore_default_volt(p);

        _set_no_limited(p);
#ifdef CONFIG_CPU_DVFS_DOWNGRADE_FREQ
        _init_downgrade(p, _mt_cpufreq_get_cpu_level());
#endif
#ifdef  CONFIG_CPU_DVFS_POWER_THROTTLING
        register_battery_percent_notify(&bat_per_protection_powerlimit, BATTERY_PERCENT_PRIO_CPU_L);
        register_battery_oc_notify(&bat_oc_protection_powerlimit, BATTERY_OC_PRIO_CPU_L);
        register_low_battery_notify(&Lbat_protection_powerlimit, LOW_BATTERY_PRIO_CPU_L);
#endif
    }

    if (ret)
        cpufreq_err("failed to setup frequency table\n");

    FUNC_EXIT(FUNC_LV_MODULE);

    return ret;
}

static unsigned int _mt_cpufreq_get(unsigned int cpu)
{
    struct mt_cpu_dvfs *p;

    FUNC_ENTER(FUNC_LV_MODULE);

    p = id_to_cpu_dvfs(_get_cpu_dvfs_id(cpu));

    BUG_ON(NULL == p);

    FUNC_EXIT(FUNC_LV_MODULE);

    return cpu_dvfs_get_cur_freq(p);
}

/*
 * Early suspend
 */
static bool _allow_dpidle_ctrl_vproc = false;

bool mt_cpufreq_earlysuspend_status_get(void)
{
    return _allow_dpidle_ctrl_vproc;
}
EXPORT_SYMBOL(mt_cpufreq_earlysuspend_status_get);

static void _mt_cpufreq_early_suspend(struct early_suspend *h)
{
    struct cpufreq_policy *policy;
    struct mt_cpu_dvfs *p;
    int i;

    FUNC_ENTER(FUNC_LV_MODULE);

    // mt_cpufreq_state_set(0); // TODO: it is not necessary because of dvfs_disable_by_early_suspend

    // mt_cpufreq_set_pmic_phase(PMIC_WRAP_PHASE_DEEPIDLE); // TODO: move to deepidle driver

    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        p->dvfs_disable_by_early_suspend = true;

        p->idx_opp_tbl_for_late_resume = p->idx_opp_tbl;

#ifdef CONFIG_CPU_FREQ
        policy = cpufreq_cpu_get(p->cpu_id);

        if (policy) {
            cpufreq_driver_target(policy, cpu_dvfs_get_normal_max_freq(p), CPUFREQ_RELATION_L);
            cpufreq_cpu_put(policy);
        }
#endif
    }

    _allow_dpidle_ctrl_vproc = true;

    check_pmic_6325_version();
    gMd32_alive = is_md32_enable();
    if (gEMI_DFS_enable && gMd32_alive && gPMIC_6325_version >= 0x2530)
    {
        //switch to 1 PLL
        do_DRAM_DFS(0);
        
        #if EMI_FREQ_CHECK
        {
            unsigned int val;
            
            val = mt_get_emi_freq();
            if (val > (184250+18425) || val < (184250-18425))
            {
                //assert it
                BUG_ON(1);
            } 
        }
        #endif
    }

    FUNC_EXIT(FUNC_LV_MODULE);
}

static void _mt_cpufreq_late_resume(struct early_suspend *h)
{
    struct cpufreq_policy *policy;
    struct mt_cpu_dvfs *p;
    int i;

    FUNC_ENTER(FUNC_LV_MODULE);

    if (gEMI_DFS_enable && gMd32_alive && gPMIC_6325_version >= 0x2530)
    {
        //switch to 3 PLL
        do_DRAM_DFS(1);
        
        #if EMI_FREQ_CHECK
        {
            unsigned int val;
            
            val = mt_get_emi_freq();
            if (val > 440000 || val < 360000)
            {
                //assert it
                BUG_ON(1);
            } 
        }
        #endif
    }
    
    _allow_dpidle_ctrl_vproc = false;

    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        p->dvfs_disable_by_early_suspend = false;

#ifdef CONFIG_CPU_FREQ
        policy = cpufreq_cpu_get(p->cpu_id);

        if (policy) {
            cpufreq_driver_target(policy, cpu_dvfs_get_freq_by_idx(p, p->idx_opp_tbl_for_late_resume), CPUFREQ_RELATION_L);
            cpufreq_cpu_put(policy);
        }
#endif
    }

    // mt_cpufreq_set_pmic_phase(PMIC_WRAP_PHASE_NORMAL); // TODO: move to deepidle driver

    // mt_cpufreq_state_set(1); // TODO: it is not necessary because of dvfs_disable_by_early_suspend

    FUNC_EXIT(FUNC_LV_MODULE);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend _mt_cpufreq_early_suspend_handler = {
    .level    = EARLY_SUSPEND_LEVEL_DISABLE_FB + 200,
    .suspend  = _mt_cpufreq_early_suspend,
    .resume   = _mt_cpufreq_late_resume,
};
#endif /* CONFIG_HAS_EARLYSUSPEND */

#ifdef CONFIG_CPU_FREQ
static struct freq_attr *_mt_cpufreq_attr[] = {
    &cpufreq_freq_attr_scaling_available_freqs,
    NULL,
};

static struct cpufreq_driver _mt_cpufreq_driver = {
    .verify = _mt_cpufreq_verify,
    .target = _mt_cpufreq_target,
    .init   = _mt_cpufreq_init,
    .get    = _mt_cpufreq_get,
    .name   = "mt-cpufreq",
    .attr   = _mt_cpufreq_attr,
};
#endif

/*
 * Platform driver
 */
static int _mt_cpufreq_suspend(struct device *dev)
{
    //struct cpufreq_policy *policy;
    struct mt_cpu_dvfs *p;
    int i;

    FUNC_ENTER(FUNC_LV_MODULE);

    // mt_cpufreq_set_pmic_phase(PMIC_WRAP_PHASE_SUSPEND); // TODO: move to suspend driver

    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        p->dvfs_disable_by_suspend = true;

#if 0 // XXX: cpufreq_driver_target doesn't work @ suspend
        policy = cpufreq_cpu_get(p->cpu_id);

        if (policy) {
            cpufreq_driver_target(policy, cpu_dvfs_get_normal_max_freq(p), CPUFREQ_RELATION_L);
            cpufreq_cpu_put(policy);
        }
#else
        _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, p->idx_normal_max_opp); // XXX: useless, decided @ _calc_new_opp_idx()
#endif
    }

    FUNC_EXIT(FUNC_LV_MODULE);

    return 0;
}

static int _mt_cpufreq_resume(struct device *dev)
{
    struct mt_cpu_dvfs *p;
    int i;

    FUNC_ENTER(FUNC_LV_MODULE);

    // mt_cpufreq_set_pmic_phase(PMIC_WRAP_PHASE_NORMAL); // TODO: move to suspend driver

    for_each_cpu_dvfs(i, p) {
        if (!cpu_dvfs_is_availiable(p))
            continue;

        p->dvfs_disable_by_suspend = false;
    }

    FUNC_EXIT(FUNC_LV_MODULE);

    return 0;
}

static int _mt_cpufreq_pm_restore_early(struct device *dev) /* for IPO-H HW(freq) / SW(opp_tbl_idx) */ // TODO: DON'T CARE???
{
    FUNC_ENTER(FUNC_LV_MODULE);

    _mt_cpufreq_sync_opp_tbl_idx();

    FUNC_EXIT(FUNC_LV_MODULE);

    return 0;
}

static int _mt_cpufreq_pdrv_probe(struct platform_device *pdev)
{
    FUNC_ENTER(FUNC_LV_MODULE);

    // TODO: check extBuck init with James

    if (pw.addr[0].cmd_addr == 0)
        _mt_cpufreq_pmic_table_init();

# if 0 // TODO: FIXME <-- disable to avoid build error
    /* init static power table */
    mt_spower_init();
#endif

    /* register early suspend */
#ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&_mt_cpufreq_early_suspend_handler);
#endif

    /* init PMIC_WRAP & volt */
    mt_cpufreq_set_pmic_phase(PMIC_WRAP_PHASE_NORMAL);
#if 0 // TODO: FIXME
    /* restore default volt, sync opp idx, set default limit */
    {
        struct mt_cpu_dvfs *p;
        int i;

        for_each_cpu_dvfs(i, p) {
            if (!cpu_dvfs_is_availiable(p))
                continue;

            _restore_default_volt(p);

            if (_sync_opp_tbl_idx(p) >= 0)
                p->idx_normal_max_opp = p->idx_opp_tbl;

            _set_no_limited(p);

#if defined(CONFIG_CPU_DVFS_DOWNGRADE_FREQ)
            _init_downgrade(p, read_efuse_cpu_speed());
#endif
        }
    }
#endif

#ifdef CONFIG_CPU_FREQ
    cpufreq_register_driver(&_mt_cpufreq_driver);
#endif
    register_hotcpu_notifier(&turbo_mode_cpu_notifier); // <-XXX

    FUNC_EXIT(FUNC_LV_MODULE);

    return 0;
}

static int _mt_cpufreq_pdrv_remove(struct platform_device *pdev)
{
    FUNC_ENTER(FUNC_LV_MODULE);

    unregister_hotcpu_notifier(&turbo_mode_cpu_notifier); // <-XXX
#ifdef CONFIG_CPU_FREQ
    cpufreq_unregister_driver(&_mt_cpufreq_driver);
#endif

    FUNC_EXIT(FUNC_LV_MODULE);

    return 0;
}

static const struct dev_pm_ops _mt_cpufreq_pm_ops = {
    .suspend	= _mt_cpufreq_suspend,
    .resume		= _mt_cpufreq_resume,
    .restore_early	= _mt_cpufreq_pm_restore_early,
    .freeze		= _mt_cpufreq_suspend,
    .thaw		= _mt_cpufreq_resume,
    .restore	= _mt_cpufreq_resume,
};

struct platform_device _mt_cpufreq_pdev = {
    .name   = "mt-cpufreq",
    .id     = -1,
};

static struct platform_driver _mt_cpufreq_pdrv = {
    .probe      = _mt_cpufreq_pdrv_probe,
    .remove     = _mt_cpufreq_pdrv_remove,
    .driver     = {
        .name   = "mt-cpufreq",
        .pm     = &_mt_cpufreq_pm_ops,
        .owner  = THIS_MODULE,
    },
};

#ifndef __KERNEL__
/*
 * For CTP
 */
int mt_cpufreq_pdrv_probe(void)
{
    static struct cpufreq_policy policy;

    _mt_cpufreq_pdrv_probe(NULL);

    policy.cpu = cpu_dvfs[MT_CPU_DVFS_LITTLE].cpu_id;
    _mt_cpufreq_init(&policy);

    return 0;
}

int mt_cpufreq_set_opp_volt(enum mt_cpu_dvfs_id id, int idx)
{
    int ret = 0;
    static struct opp_tbl_info *info;
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    info = &opp_tbls[CPU_LV_TO_OPP_IDX(p->cpu_level)];
    if (idx >= info->size)
        return -1;

    return _set_cur_volt_locked(p, info->opp_tbl[idx].cpufreq_volt);
}

int mt_cpufreq_set_freq(enum mt_cpu_dvfs_id id, int idx)
{
    unsigned int cur_freq;
    unsigned int target_freq;
    int ret;
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    cur_freq = p->ops->get_cur_phy_freq(p);
    target_freq = cpu_dvfs_get_freq_by_idx(p, idx);

    ret = _cpufreq_set_locked(p, cur_freq, target_freq);

    if (ret < 0)
        return ret;

    return target_freq;
}

#include "dvfs.h"

static unsigned int _mt_get_cpu_freq(void)
{
    unsigned int output = 0, i = 0;
    unsigned int temp, clk26cali_0, clk_dbg_cfg, clk_misc_cfg_0, clk26cali_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);

    clk_dbg_cfg = DRV_Reg32(CLK_DBG_CFG);
    DRV_WriteReg32(CLK_DBG_CFG, 2<<16); //sel abist_cksw and enable freq meter sel abist
    
    clk_misc_cfg_0 = DRV_Reg32(CLK_MISC_CFG_0);
    DRV_WriteReg32(CLK_MISC_CFG_0, (clk_misc_cfg_0 & 0x0000FFFF) | (0x07 << 16)); // select divider

    clk26cali_1 = DRV_Reg32(CLK26CALI_1);
    DRV_WriteReg32(CLK26CALI_1, 0x00ff0000); // 

    //temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, 0x1000);
    DRV_WriteReg32(CLK26CALI_0, 0x1010);

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        mdelay(10);
        i++;
        if(i > 10)
        	break;
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = (((temp * 26000) ) / 256) * 8; // Khz

    DRV_WriteReg32(CLK_DBG_CFG, clk_dbg_cfg);
    DRV_WriteReg32(CLK_MISC_CFG_0, clk_misc_cfg_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);
    DRV_WriteReg32(CLK26CALI_1, clk26cali_1);

    cpufreq_dbg("CLK26CALI_1 = 0x%x, CPU freq = %d KHz\n", temp, output);

    if(i>10) {
        cpufreq_dbg("meter not finished!\n");
        return 0;
    }
    else
        return output;
}

unsigned int dvfs_get_cpu_freq(enum mt_cpu_dvfs_id id)
{
    //return _mt_cpufreq_get_cur_phy_freq(id);
    return _mt_get_cpu_freq();
}

void dvfs_set_cpu_freq_FH(enum mt_cpu_dvfs_id id, int freq)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);
    int idx;

    if (!p) {
        cpufreq_err("%s(%d, %d), id is wrong\n", __func__, id, freq);
        return;
    }

    idx = _search_available_freq_idx(p, freq, CPUFREQ_RELATION_H);

    if (-1 == idx) {
        cpufreq_err("%s(%d, %d), freq is wrong\n", __func__, id, freq);
        return;
    }

    mt_cpufreq_set_freq(id, idx);
}

unsigned int cpu_frequency_output_slt(enum mt_cpu_dvfs_id id)
{
    return (MT_CPU_DVFS_LITTLE == id) ? _mt_get_cpu_freq() : 0;
}

void dvfs_set_cpu_volt(enum mt_cpu_dvfs_id id, int volt)  // volt: mv * 100
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(id);

    cpufreq_dbg("%s(%d, %d)\n", __func__, id, volt);

    if (!p) {
        cpufreq_err("%s(%d, %d), id is wrong\n", __func__, id, volt);
        return;
    }

    if (_set_cur_volt_locked(p, volt))
        cpufreq_err("%s(%d, %d), set volt fail\n", __func__, id, volt);

    cpufreq_dbg("%s(%d, %d) Vproc = %d, Vsram = %d\n", 
        __func__,
        id, 
        volt,
        p->ops->get_cur_volt(p),
        get_cur_vsram(p)
        );
}

void dvfs_set_gpu_volt(int pmic_val)
{
    cpufreq_dbg("%s(%d)\n", __func__, pmic_val);
    mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_NORMAL, IDX_NM_VGPU, pmic_val);
    mt_cpufreq_apply_pmic_cmd(IDX_NM_VGPU);
}

/* NOTE: This is ONLY for PTPOD SLT. Should not adjust VCORE in other cases. */
void dvfs_set_vcore_ao_volt(int pmic_val)
{
    cpufreq_dbg("%s(%d)\n", __func__, pmic_val);
    mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_NORMAL, IDX_NM_VCORE, pmic_val);
    mt_cpufreq_apply_pmic_cmd(IDX_NM_VCORE);
}

#if 0 
void dvfs_set_vcore_pdn_volt(int pmic_val)
{ 
    cpufreq_dbg("%s(%d)\n", __func__, pmic_val);
    mt_cpufreq_set_pmic_phase(PMIC_WRAP_PHASE_DEEPIDLE);
    mt_cpufreq_set_pmic_cmd(PMIC_WRAP_PHASE_DEEPIDLE, IDX_DI_VCORE_PDN_NORMAL, pmic_val);
    mt_cpufreq_apply_pmic_cmd(IDX_DI_VCORE_PDN_NORMAL);
    mt_cpufreq_set_pmic_phase(PMIC_WRAP_PHASE_NORMAL);
}
#endif

//static unsigned int little_freq_backup;
static unsigned int vcpu_backup;
static unsigned int vgpu_backup;
static unsigned int vcore_ao_backup;
//static unsigned int vcore_pdn_backup;

void dvfs_disable_by_ptpod(void)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(MT_CPU_DVFS_LITTLE);

    cpufreq_dbg("%s()\n", __func__); // <-XXX
    
    //little_freq_backup = _mt_cpufreq_get_cur_phy_freq(MT_CPU_DVFS_LITTLE);
    vcpu_backup = cpu_dvfs_get_cur_volt(p);
    pmic_read_interface(PMIC_ADDR_VGPU_VOSEL_ON, &vgpu_backup, 0x7F, 0);
    pmic_read_interface(PMIC_ADDR_VCORE_VOSEL_ON, &vcore_ao_backup, 0x7F, 0);
    //pmic_read_interface(PMIC_ADDR_VCORE_PDN_VOSEL_ON, &vcore_pdn_backup, 0x7F, 0);

    dvfs_set_cpu_volt(MT_CPU_DVFS_LITTLE, 100000);    // 1V
    dvfs_set_gpu_volt(VOLT_TO_PMIC_VAL(100000));      // 1V
    dvfs_set_vcore_ao_volt(VOLT_TO_PMIC_VAL(100000)); // 1V
    //dvfs_set_vcore_pdn_volt(0x30);
}

void dvfs_enable_by_ptpod(void)
{
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(MT_CPU_DVFS_LITTLE);

    cpufreq_dbg("%s()\n", __func__); // <-XXX
    
    dvfs_set_cpu_volt(MT_CPU_DVFS_LITTLE, vcpu_backup);
    dvfs_set_gpu_volt(vgpu_backup);
    dvfs_set_vcore_ao_volt(vcore_ao_backup);
    //dvfs_set_vcore_pdn_volt(vcore_pdn_backup);
}
#endif /* ! __KERNEL__ */

#ifdef CONFIG_PROC_FS
/*
 * PROC
 */

static char *_copy_from_user_for_proc(const char __user *buffer, size_t count)
{
    char *buf = (char *)__get_free_page(GFP_USER);

    if (!buf)
        return NULL;

    if (count >= PAGE_SIZE)
        goto out;

    if (copy_from_user(buf, buffer, count))
        goto out;

    buf[count] = '\0';

    return buf;

out:
    free_page((unsigned long)buf);

    return NULL;
}

/* cpufreq_debug */
static int cpufreq_debug_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "cpufreq debug (log level) = %d\n", func_lv_mask);

    return 0;
}

static ssize_t cpufreq_debug_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    unsigned int dbg_lv;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &dbg_lv) == 1)
        func_lv_mask = dbg_lv;
    else
        cpufreq_err("echo dbg_lv (dec) > /proc/cpufreq/cpufreq_debug\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_downgrade_freq_info */
static int cpufreq_downgrade_freq_info_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "downgrade_freq_counter_limit = %d\n"
            "ptpod_temperature_limit_1 = %d\n"
            "ptpod_temperature_limit_2 = %d\n"
            "ptpod_temperature_time_1 = %d\n"
            "ptpod_temperature_time_2 = %d\n"
            "downgrade_freq_counter_return_limit 1 = %d\n"
            "downgrade_freq_counter_return_limit 2 = %d\n"
            "over_max_cpu = %d\n",
            p->downgrade_freq_counter_limit,
            p->ptpod_temperature_limit_1,
            p->ptpod_temperature_limit_2,
            p->ptpod_temperature_time_1,
            p->ptpod_temperature_time_2,
            p->ptpod_temperature_limit_1 * p->ptpod_temperature_time_1,
            p->ptpod_temperature_limit_2 * p->ptpod_temperature_time_2,
            p->over_max_cpu
            );

    return 0;
}

/* cpufreq_downgrade_freq_counter_limit */
static int cpufreq_downgrade_freq_counter_limit_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "%d\n", p->downgrade_freq_counter_limit);

    return 0;
}

static ssize_t cpufreq_downgrade_freq_counter_limit_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int downgrade_freq_counter_limit;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &downgrade_freq_counter_limit) == 1)
        p->downgrade_freq_counter_limit = downgrade_freq_counter_limit;
    else
        cpufreq_err("echo downgrade_freq_counter_limit (dec) > /proc/cpufreq/cpufreq_downgrade_freq_counter_limit\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_downgrade_freq_counter_return_limit */
static int cpufreq_downgrade_freq_counter_return_limit_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "%d\n", p->downgrade_freq_counter_return_limit);

    return 0;
}

static ssize_t cpufreq_downgrade_freq_counter_return_limit_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int downgrade_freq_counter_return_limit;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &downgrade_freq_counter_return_limit) == 1)
        p->downgrade_freq_counter_return_limit = downgrade_freq_counter_return_limit; // TODO: p->ptpod_temperature_limit_1 * p->ptpod_temperature_time_1 or p->ptpod_temperature_limit_2 * p->ptpod_temperature_time_2
    else
        cpufreq_err("echo downgrade_freq_counter_return_limit (dec) > /proc/cpufreq/cpufreq_downgrade_freq_counter_return_limit\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_fftt_test */
#include <asm/sched_clock.h>

static unsigned long _delay_us;
static unsigned long _delay_us_buf;

static int cpufreq_fftt_test_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%lu\n", _delay_us);

    if (_delay_us < _delay_us_buf)
        cpufreq_err("@%s(), %lu < %lu, loops_per_jiffy = %lu\n", __func__, _delay_us, _delay_us_buf, loops_per_jiffy);

    return 0;
}

static ssize_t cpufreq_fftt_test_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%lu", &_delay_us_buf) == 1) {
        unsigned long start;

        start = (unsigned long)sched_clock();
        udelay(_delay_us_buf);
        _delay_us = ((unsigned long)sched_clock() - start) / 1000;

        cpufreq_ver("@%s(%lu), _delay_us = %lu, loops_per_jiffy = %lu\n", __func__, _delay_us_buf, _delay_us, loops_per_jiffy);
    }

    free_page((unsigned int)buf);

    return count;
}

static int cpufreq_stress_test_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%d\n", do_dvfs_stress_test);

    return 0;
}

static ssize_t cpufreq_stress_test_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    unsigned int do_stress;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &do_stress) == 1)
        do_dvfs_stress_test = do_stress;
    else
        cpufreq_err("echo 0/1 > /proc/cpufreq/cpufreq_stress_test\n");

    free_page((unsigned int)buf);
    return count;
}

static int cpufreq_emi_dfs_proc_show(struct seq_file *m, void *v)
{
    check_pmic_6325_version();
    gMd32_alive = is_md32_enable();
    
    seq_printf(m, "EMI DFS: %s, %s, EMI_DFS_e=%d, MD32_a=%d, PMIC_v=0x%x\n", 
        (gEMI_DFS_enable && gMd32_alive && gPMIC_6325_version >= 0x2530)?"enable":"disable", (mt_get_clk_mem_sel()==1)?"3PLL":"1PLL", 
        gEMI_DFS_enable, gMd32_alive, gPMIC_6325_version);
    return 0;
}

static ssize_t cpufreq_emi_dfs_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char *buf = _copy_from_user_for_proc(buffer, count);
    unsigned int tmp;

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%u", &tmp) == 1) 
    {
        if (tmp == 0 || tmp == 1) 
        {
            gEMI_DFS_enable = tmp;
        }
        else 
        {
            cpufreq_err("echo [0|1] > /proc/cpufreq/cpufreq_emi_dfs\n");
        }
    }
    else
    {
        cpufreq_err("echo [0|1] > /proc/cpufreq/cpufreq_emi_dfs\n");
    }
    
    free_page((unsigned int)buf);

    return count;
}

/* cpufreq_limited_by_hevc */
static int cpufreq_limited_by_hevc_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "%d\n", p->limited_freq_by_hevc);

    return 0;
}

static ssize_t cpufreq_limited_by_hevc_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int limited_freq_by_hevc;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &limited_freq_by_hevc) == 1) {
        p->limited_freq_by_hevc = limited_freq_by_hevc;
        if (cpu_dvfs_is_availiable(p) && (p->limited_freq_by_hevc > cpu_dvfs_get_cur_freq(p))) {
            struct cpufreq_policy *policy = cpufreq_cpu_get(p->cpu_id);

            if (policy) {
                cpufreq_driver_target(policy, p->limited_freq_by_hevc, CPUFREQ_RELATION_L);
                cpufreq_cpu_put(policy);
            }
        }
    } else
        cpufreq_err("echo limited_freq_by_hevc (dec) > /proc/cpufreq/cpufreq_limited_by_hevc\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_limited_power */
static int cpufreq_limited_power_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p;
    int i;

    for_each_cpu_dvfs(i, p) {
        seq_printf(m, "[%s/%d]\n"
                "limited_max_freq = %d\n"
                "limited_max_ncpu = %d\n",
                p->name, i,
                p->limited_max_freq,
                p->limited_max_ncpu
                );
    }

    return 0;
}

static ssize_t cpufreq_limited_power_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    int limited_power;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &limited_power) == 1)
        mt_cpufreq_thermal_protect(limited_power); // TODO: specify limited_power by id???
    else
        cpufreq_err("echo limited_power (dec) > /proc/cpufreq/cpufreq_limited_power\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_over_max_cpu */
static int cpufreq_over_max_cpu_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "%d\n", p->over_max_cpu);

    return 0;
}

static ssize_t cpufreq_over_max_cpu_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int over_max_cpu;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &over_max_cpu) == 1)
        p->over_max_cpu = over_max_cpu;
    else
        cpufreq_err("echo over_max_cpu (dec) > /proc/cpufreq/cpufreq_over_max_cpu\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_power_dump */
static int cpufreq_power_dump_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p;
    int i, j;

    for_each_cpu_dvfs(i, p) {
        seq_printf(m, "[%s/%d]\n", p->name, i);

        for (j = 0; j < p->nr_power_tbl; j++) {
            seq_printf(m, "[%d] = { .cpufreq_khz = %d,\t.cpufreq_ncpu = %d,\t.cpufreq_power = %d, },\n",
                    j,
                    p->power_tbl[j].cpufreq_khz,
                    p->power_tbl[j].cpufreq_ncpu,
                    p->power_tbl[j].cpufreq_power
                    );
        }
    }

    return 0;
}

/* cpufreq_ptpod_freq_volt */
static int cpufreq_ptpod_freq_volt_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;
    int j;

    for (j = 0; j < p->nr_opp_tbl; j++) {
        seq_printf(m, "[%d] = { .cpufreq_khz = %d,\t.cpufreq_volt = %d,\t.cpufreq_volt_org = %d, },\n",
                j,
                p->opp_tbl[j].cpufreq_khz,
                p->opp_tbl[j].cpufreq_volt,
                p->opp_tbl[j].cpufreq_volt_org
                );
    }

    return 0;
}

/* cpufreq_ptpod_temperature_limit */
static int cpufreq_ptpod_temperature_limit_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "ptpod_temperature_limit_1 = %d\n"
            "ptpod_temperature_limit_2 = %d\n",
            p->ptpod_temperature_limit_1,
            p->ptpod_temperature_limit_2
            );

    return 0;
}

static ssize_t cpufreq_ptpod_temperature_limit_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int ptpod_temperature_limit_1;
    int ptpod_temperature_limit_2;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d %d", &ptpod_temperature_limit_1, &ptpod_temperature_limit_2) == 2) {
        p->ptpod_temperature_limit_1 = ptpod_temperature_limit_1;
        p->ptpod_temperature_limit_2 = ptpod_temperature_limit_2;
    } else
        cpufreq_err("echo ptpod_temperature_limit_1 (dec) ptpod_temperature_limit_2 (dec) > /proc/cpufreq/cpufreq_ptpod_temperature_limit\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_ptpod_temperature_time */
static int cpufreq_ptpod_temperature_time_proc_show(struct seq_file *m, void *v)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "ptpod_temperature_time_1 = %d\n"
            "ptpod_temperature_time_2 = %d\n",
            p->ptpod_temperature_time_1,
            p->ptpod_temperature_time_2
            );

    return 0;
}

static ssize_t cpufreq_ptpod_temperature_time_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int ptpod_temperature_time_1;
    int ptpod_temperature_time_2;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d %d", &ptpod_temperature_time_1, &ptpod_temperature_time_2) == 2) {
        p->ptpod_temperature_time_1 = ptpod_temperature_time_1;
        p->ptpod_temperature_time_2 = ptpod_temperature_time_2;
    } else
        cpufreq_err("echo ptpod_temperature_time_1 (dec) ptpod_temperature_time_2 (dec) > /proc/cpufreq/cpufreq_ptpod_temperature_time\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_ptpod_test */
static int cpufreq_ptpod_test_proc_show(struct seq_file *m, void *v)
{
    return 0;
}

static ssize_t cpufreq_ptpod_test_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    return count;
}

/* cpufreq_state */
static int cpufreq_state_proc_show(struct seq_file *m, void *v) // TODO: keep this function???
{
    struct mt_cpu_dvfs *p;
    int i;

    for_each_cpu_dvfs(i, p) {
        seq_printf(m, "[%s/%d]\n"
                "dvfs_disable_by_suspend = %d\n"
                "dvfs_disable_by_early_suspend = %d\n"
                "dvfs_disable_by_ptpod = %d\n"
                "dvfs_disable_by_procfs = %d\n",
                p->name, i,
                p->dvfs_disable_by_suspend,
                p->dvfs_disable_by_early_suspend,
                p->dvfs_disable_by_ptpod,
                p->dvfs_disable_by_procfs
                );
    }

    return 0;
}

static ssize_t cpufreq_state_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) // TODO: keep this function???
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    char *buf = _copy_from_user_for_proc(buffer, count);
    int enable;    

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &enable) == 1) {
        if (enable == 0)
            p->dvfs_disable_by_procfs = true;
        else
            p->dvfs_disable_by_procfs = false;
    }
    else
        cpufreq_err("echo 1/0 > /proc/cpufreq/cpufreq_state\n");

    free_page((unsigned int)buf);
    return count;
}

/* cpufreq_oppidx */
static int cpufreq_oppidx_proc_show(struct seq_file *m, void *v) // <-XXX
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;
    int j;

    seq_printf(m, "[%s/%d]\n"
            "cpufreq_oppidx = %d\n",
            p->name, p->cpu_id,
            p->idx_opp_tbl
            );

    for (j = 0; j < p->nr_opp_tbl; j++) {
        seq_printf(m, "\tOP(%d, %d),\n",
                cpu_dvfs_get_freq_by_idx(p, j),
                cpu_dvfs_get_volt_by_idx(p, j)
                );
    }

    return 0;
}

static ssize_t cpufreq_oppidx_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) // <-XXX
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int oppidx;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    BUG_ON(NULL == p);

    if (sscanf(buf, "%d", &oppidx) == 1
            && 0 <= oppidx && oppidx < p->nr_opp_tbl
       ) {
        p->dvfs_disable_by_procfs = true;
        _mt_cpufreq_set(MT_CPU_DVFS_LITTLE, oppidx);
    } else {
        p->dvfs_disable_by_procfs = false; // TODO: FIXME
        cpufreq_err("echo oppidx > /proc/cpufreq/cpufreq_oppidx (0 <= %d < %d)\n", oppidx, p->nr_opp_tbl);
    }

    free_page((unsigned int)buf);

    return count;
}

/* cpufreq_freq */
static int cpufreq_freq_proc_show(struct seq_file *m, void *v) // <-XXX
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    seq_printf(m, "%d KHz\n", p->ops->get_cur_phy_freq(p));

    return 0;
}

static ssize_t cpufreq_freq_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) // <-XXX
{
    unsigned long flags;
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    unsigned int cur_freq;
    int freq, i, found;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    BUG_ON(NULL == p);

    if (sscanf(buf, "%d", &freq) == 1) {
        if (freq < CPUFREQ_LAST_FREQ_LEVEL) {
            if (freq != 0)
                cpufreq_err("frequency should higher than %dKHz!\n", CPUFREQ_LAST_FREQ_LEVEL);
            p->dvfs_disable_by_procfs = false;
            goto end;
        } else {    
            for (i = 0; i < p->nr_opp_tbl; i++) {
                if(freq == p->opp_tbl[i].cpufreq_khz) {
                    found = 1;
                    break;
                }
            }

            if (found == 1) {
                p->dvfs_disable_by_procfs = true; // TODO: FIXME
                cpufreq_lock(flags);	// <-XXX
                cur_freq = p->ops->get_cur_phy_freq(p);
                if (freq != cur_freq)
                    p->ops->set_cur_freq(p, cur_freq, freq);
                cpufreq_unlock(flags);	// <-XXX
            }
            else {
                p->dvfs_disable_by_procfs = false;
                cpufreq_err("frequency %dKHz! is not found in CPU opp table\n", freq);
            }
        }
    } else {
        p->dvfs_disable_by_procfs = false; // TODO: FIXME
        cpufreq_err("echo khz > /proc/cpufreq/cpufreq_freq\n");
    }

end:
    free_page((unsigned int)buf);

    return count;
}

/* cpufreq_volt */
static int cpufreq_volt_proc_show(struct seq_file *m, void *v) // <-XXX
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;

    if (cpu_dvfs_is_extbuck_valid()) {
        seq_printf(m, "Vproc: %d mv\n", p->ops->get_cur_volt(p) / 100);  // mv
        seq_printf(m, "Vsram: %d mv\n", get_cur_vsram(p) / 100);  // mv
    } else
        seq_printf(m, "%d mv\n", p->ops->get_cur_volt(p) / 100);  // mv

    return 0;
}

static ssize_t cpufreq_volt_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) // <-XXX
{
    unsigned long flags;
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    int mv;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if (sscanf(buf, "%d", &mv) == 1) {
        p->dvfs_disable_by_procfs = true; // TODO: FIXME
        cpufreq_lock(flags);
        _set_cur_volt_locked(p, mv * 100);
        cpufreq_unlock(flags);
    } else {
        p->dvfs_disable_by_procfs = false; // TODO: FIXME
        cpufreq_err("echo mv > /proc/cpufreq/cpufreq_volt\n");
    }

    free_page((unsigned int)buf);

    return count;
}

/* cpufreq_turbo_mode */
static int cpufreq_turbo_mode_proc_show(struct seq_file *m, void *v) // <-XXX
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)m->private;
    int i;

    seq_printf(m, "turbo_mode = %d\n", p->turbo_mode);

    for (i = 0; i < NR_TURBO_MODE; i++) {
        seq_printf(m, "[%d] = { .temp = %d, .freq_delta = %d, .volt_delta = %d }\n",
                i,
                turbo_mode_cfg[i].temp,
                turbo_mode_cfg[i].freq_delta,
                turbo_mode_cfg[i].volt_delta
                );
    }

    return 0;
}

static ssize_t cpufreq_turbo_mode_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) // <-XXX
{
    struct mt_cpu_dvfs *p = (struct mt_cpu_dvfs *)PDE_DATA(file_inode(file));
    unsigned int turbo_mode;
    int temp;
    int freq_delta;
    int volt_delta;

    char *buf = _copy_from_user_for_proc(buffer, count);

    if (!buf)
        return -EINVAL;

    if ((sscanf(buf, "%d %d %d %d", &turbo_mode, &temp, &freq_delta, &volt_delta) == 4) && turbo_mode < NR_TURBO_MODE) {
        turbo_mode_cfg[turbo_mode].temp = temp;
        turbo_mode_cfg[turbo_mode].freq_delta = freq_delta;
        turbo_mode_cfg[turbo_mode].volt_delta = volt_delta;
    } else if (sscanf(buf, "%d", &turbo_mode) == 1)
        p->turbo_mode = turbo_mode; // TODO: FIXME
    else {
        cpufreq_err("echo 0/1 > /proc/cpufreq/cpufreq_turbo_mode\n");
        cpufreq_err("echo idx temp freq_delta volt_delta > /proc/cpufreq/cpufreq_turbo_mode\n");
    }

    free_page((unsigned int)buf);

    return count;
}

#define PROC_FOPS_RW(name)							\
    static int name ## _proc_open(struct inode *inode, struct file *file)	\
{									\
    return single_open(file, name ## _proc_show, PDE_DATA(inode));	\
}									\
static const struct file_operations name ## _proc_fops = {		\
    .owner          = THIS_MODULE,					\
    .open           = name ## _proc_open,				\
    .read           = seq_read,					\
    .llseek         = seq_lseek,					\
    .release        = single_release,				\
    .write          = name ## _proc_write,				\
}

#define PROC_FOPS_RO(name)							\
    static int name ## _proc_open(struct inode *inode, struct file *file)	\
{									\
    return single_open(file, name ## _proc_show, PDE_DATA(inode));	\
}									\
static const struct file_operations name ## _proc_fops = {		\
    .owner          = THIS_MODULE,					\
    .open           = name ## _proc_open,				\
    .read           = seq_read,					\
    .llseek         = seq_lseek,					\
    .release        = single_release,				\
}

#define PROC_ENTRY(name)	{__stringify(name), &name ## _proc_fops}

PROC_FOPS_RW(cpufreq_debug);
PROC_FOPS_RW(cpufreq_fftt_test);
PROC_FOPS_RW(cpufreq_stress_test);
PROC_FOPS_RW(cpufreq_limited_power);
PROC_FOPS_RO(cpufreq_power_dump);
PROC_FOPS_RW(cpufreq_ptpod_test);
PROC_FOPS_RW(cpufreq_state);
PROC_FOPS_RW(cpufreq_emi_dfs);

PROC_FOPS_RO(cpufreq_downgrade_freq_info);
PROC_FOPS_RW(cpufreq_downgrade_freq_counter_limit);
PROC_FOPS_RW(cpufreq_downgrade_freq_counter_return_limit);
PROC_FOPS_RW(cpufreq_limited_by_hevc);
PROC_FOPS_RW(cpufreq_over_max_cpu);
PROC_FOPS_RO(cpufreq_ptpod_freq_volt);
PROC_FOPS_RW(cpufreq_ptpod_temperature_limit);
PROC_FOPS_RW(cpufreq_ptpod_temperature_time);
PROC_FOPS_RW(cpufreq_oppidx); // <-XXX
PROC_FOPS_RW(cpufreq_freq); // <-XXX
PROC_FOPS_RW(cpufreq_volt); // <-XXX
PROC_FOPS_RW(cpufreq_turbo_mode); // <-XXX

static int _create_procfs(void)
{
    struct proc_dir_entry *dir = NULL;
    //struct proc_dir_entry *cpu_dir = NULL;
    struct mt_cpu_dvfs *p = id_to_cpu_dvfs(0);
    int i; //, j;

    struct pentry {
        const char *name;
        const struct file_operations *fops;
    };

    const struct pentry entries[] = {
        PROC_ENTRY(cpufreq_debug),
        PROC_ENTRY(cpufreq_fftt_test),
        PROC_ENTRY(cpufreq_stress_test),        
        PROC_ENTRY(cpufreq_limited_power),
        PROC_ENTRY(cpufreq_power_dump),
        PROC_ENTRY(cpufreq_ptpod_test),
        PROC_ENTRY(cpufreq_emi_dfs),
    };

    const struct pentry cpu_entries[] = {
        PROC_ENTRY(cpufreq_downgrade_freq_info),
        PROC_ENTRY(cpufreq_downgrade_freq_counter_limit),
        PROC_ENTRY(cpufreq_downgrade_freq_counter_return_limit),
        PROC_ENTRY(cpufreq_limited_by_hevc),
        PROC_ENTRY(cpufreq_over_max_cpu),
        PROC_ENTRY(cpufreq_ptpod_freq_volt),
        PROC_ENTRY(cpufreq_ptpod_temperature_limit),
        PROC_ENTRY(cpufreq_ptpod_temperature_time),
        PROC_ENTRY(cpufreq_state),
        PROC_ENTRY(cpufreq_oppidx), // <-XXX
        PROC_ENTRY(cpufreq_freq), // <-XXX
        PROC_ENTRY(cpufreq_volt), // <-XXX
        PROC_ENTRY(cpufreq_turbo_mode), // <-XXX
    };

    dir = proc_mkdir("cpufreq", NULL);

    if (!dir) {
        cpufreq_err("fail to create /proc/cpufreq @ %s()\n", __func__);
        return -ENOMEM;
    }

    for (i = 0; i < ARRAY_SIZE(entries); i++) {
        if (!proc_create(entries[i].name, S_IRUGO | S_IWUSR | S_IWGRP, dir, entries[i].fops))
            cpufreq_err("%s(), create /proc/cpufreq/%s failed\n", __func__, entries[i].name);
    }

    for (i = 0; i < ARRAY_SIZE(cpu_entries); i++) {
        if (!proc_create_data(cpu_entries[i].name, S_IRUGO | S_IWUSR | S_IWGRP, dir, cpu_entries[i].fops, p))
            cpufreq_err("%s(), create /proc/cpufreq/%s failed\n", __func__, cpu_entries[i].name);
    }

#if 0  // K2 has little core only
    for_each_cpu_dvfs(j, p) {
        cpu_dir = proc_mkdir(p->name, dir);

        if (!cpu_dir) {
            cpufreq_err("fail to create /proc/cpufreq/%s @ %s()\n", p->name, __func__);
            return -ENOMEM;
        }

        for (i = 0; i < ARRAY_SIZE(cpu_entries); i++) {
            if (!proc_create_data(cpu_entries[i].name, S_IRUGO | S_IWUSR | S_IWGRP, cpu_dir, cpu_entries[i].fops, p))
                cpufreq_err("%s(), create /proc/cpufreq/%s/%s failed\n", __func__, p->name, entries[i].name);
        }
    }
#endif

    return 0;
}
#endif /* CONFIG_PROC_FS */

/*
 * Module driver
 */
static int __init _mt_cpufreq_pdrv_init(void)
{
    int ret = 0;

    FUNC_ENTER(FUNC_LV_MODULE);

#ifdef CONFIG_PROC_FS
    /* init proc */
    if (_create_procfs())
        goto out;
#endif /* CONFIG_PROC_FS */

    /* register platform device/driver */
    ret = platform_device_register(&_mt_cpufreq_pdev);
    if (ret) {
        cpufreq_err("fail to register cpufreq device @ %s()\n", __func__);
        goto out;
    }
    
    ret = platform_driver_register(&_mt_cpufreq_pdrv);
    if (ret) {
        cpufreq_err("fail to register cpufreq driver @ %s()\n", __func__);
        platform_device_unregister(&_mt_cpufreq_pdev);
    }
    
out:
    FUNC_EXIT(FUNC_LV_MODULE);

    return ret;
}

static void __exit _mt_cpufreq_pdrv_exit(void)
{
    FUNC_ENTER(FUNC_LV_MODULE);

    platform_driver_unregister(&_mt_cpufreq_pdrv);
    platform_device_unregister(&_mt_cpufreq_pdev);

    FUNC_EXIT(FUNC_LV_MODULE);
}

late_initcall(_mt_cpufreq_pdrv_init);
module_exit(_mt_cpufreq_pdrv_exit);

MODULE_DESCRIPTION("MediaTek CPU DVFS Driver v0.3");
MODULE_LICENSE("GPL");

