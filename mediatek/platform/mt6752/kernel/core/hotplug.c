#include <linux/errno.h>    //EPERM
#include <asm/cacheflush.h> //flush_cache_all
#include <mach/hotplug.h>
#ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
#include <mach/mt_spm_mtcmos.h>
#endif
#include <mach/mt_spm_idle.h>
#include <mach/wd_api.h>
#include <mach/mt_secure_api.h>


/* 
 * extern function 
 */
extern void __disable_dcache(void);         //definition in mt_cache_v7.S
extern void __enable_dcache(void);          //definition in mt_cache_v7.S
extern void __inner_clean_dcache_L2(void);  //definition in mt_cache_v7.S
extern void inner_dcache_flush_L1(void);    //definition in inner_cache.c
extern void inner_dcache_flush_L2(void);    //definition in inner_cache.c
extern void __switch_to_smp(void);          //definition in mt_hotplug.S
extern void __switch_to_amp(void);          //definition in mt_hotplug.S
extern void __disable_dcache__inner_flush_dcache_L1(void);                          //definition in mt_cache_v7.S
extern void __disable_dcache__inner_flush_dcache_L1__inner_clean_dcache_L2(void);   //definition in mt_cache_v7.S
extern void __disable_dcache__inner_flush_dcache_L1__inner_flush_dcache_L2(void);   //definition in mt_cache_v7.S



/* 
 * global variable 
 */
atomic_t hotplug_cpu_count = ATOMIC_INIT(1);



/*
 * static function
 */
static inline void cpu_enter_lowpower(unsigned int cpu)
{
    //HOTPLUG_INFO("cpu_enter_lowpower\n");

    if (((cpu == 4) && (cpu_online(5) == 0) && (cpu_online(6) == 0) && (cpu_online(7) == 0)) ||
        ((cpu == 5) && (cpu_online(4) == 0) && (cpu_online(6) == 0) && (cpu_online(7) == 0)) ||
        ((cpu == 6) && (cpu_online(4) == 0) && (cpu_online(5) == 0) && (cpu_online(7) == 0)) ||
        ((cpu == 7) && (cpu_online(4) == 0) && (cpu_online(5) == 0) && (cpu_online(6) == 0)))
    {
    #if 0
        /* Clear the SCTLR C bit to prevent further data cache allocation */
        __disable_dcache();

        /* Clean and invalidate all data from the L1/L2 data cache */
        inner_dcache_flush_L1();
        //flush_cache_all();

        /* Execute a CLREX instruction */
        __asm__ __volatile__("clrex");

        /* Clean all data from the L2 data cache */
        inner_dcache_flush_L2();
    #else
        __disable_dcache__inner_flush_dcache_L1__inner_flush_dcache_L2();
    #endif

        /* Switch the processor from SMP mode to AMP mode by clearing the ACTLR SMP bit */
        __switch_to_amp();

        /* Execute an ISB instruction to ensure that all of the CP15 register changes from the previous steps have been committed */
        isb();

        /* Execute a DSB instruction to ensure that all cache, TLB and branch predictor maintenance operations issued by any processor in the multiprocessor device before the SMP bit was cleared have completed */
        dsb();

        /* Disable snoop requests and DVM message requests */
        REG_WRITE(CCI400_SI3_SNOOP_CONTROL, REG_READ(CCI400_SI3_SNOOP_CONTROL) & ~(SNOOP_REQ | DVM_MSG_REQ));
        while (REG_READ(CCI400_STATUS) & CHANGE_PENDING);

        /* Disable CA15L snoop function */
        mcusys_smc_write(MP1_AXI_CONFIG, REG_READ(MP1_AXI_CONFIG) | ACINACTM);
    }
    else
    {
    #if 0
        /* Clear the SCTLR C bit to prevent further data cache allocation */
        __disable_dcache();

        /* Clean and invalidate all data from the L1 data cache */
        inner_dcache_flush_L1();
        //Just flush the cache.
        //flush_cache_all();

        /* Clean all data from the L2 data cache */
        //__inner_clean_dcache_L2();
    #else
        //FIXME: why __disable_dcache__inner_flush_dcache_L1 fail but 2 steps ok?
        //__disable_dcache__inner_flush_dcache_L1();
        __disable_dcache__inner_flush_dcache_L1__inner_clean_dcache_L2();
    #endif

        /* Execute a CLREX instruction */
        __asm__ __volatile__("clrex");

        /* Switch the processor from SMP mode to AMP mode by clearing the ACTLR SMP bit */
        __switch_to_amp();
    }
}

static inline void cpu_leave_lowpower(unsigned int cpu)
{
    //HOTPLUG_INFO("cpu_leave_lowpower\n");

    if (((cpu == 4) && (cpu_online(5) == 0) && (cpu_online(6) == 0) && (cpu_online(7) == 0)) ||
        ((cpu == 5) && (cpu_online(4) == 0) && (cpu_online(6) == 0) && (cpu_online(7) == 0)) ||
        ((cpu == 6) && (cpu_online(4) == 0) && (cpu_online(5) == 0) && (cpu_online(7) == 0)) ||
        ((cpu == 7) && (cpu_online(4) == 0) && (cpu_online(5) == 0) && (cpu_online(6) == 0)))
    {
        /* Enable CA15L snoop function */
        mcusys_smc_write(MP1_AXI_CONFIG, REG_READ(MP1_AXI_CONFIG) & ~ACINACTM);

        /* Enable snoop requests and DVM message requests */
        REG_WRITE(CCI400_SI3_SNOOP_CONTROL, REG_READ(CCI400_SI3_SNOOP_CONTROL) | (SNOOP_REQ | DVM_MSG_REQ));
        while (REG_READ(CCI400_STATUS) & CHANGE_PENDING);
    }

    /* Set the ACTLR.SMP bit to 1 for SMP mode */
    __switch_to_smp();

    /* Enable dcache */
    __enable_dcache();
}

static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
    /* Just enter wfi for now. TODO: Properly shut off the cpu. */
    for (;;) {

        /* Execute an ISB instruction to ensure that all of the CP15 register changes from the previous steps have been committed */
        isb();

        /* Execute a DSB instruction to ensure that all cache, TLB and branch predictor maintenance operations issued by any processor in the multiprocessor device before the SMP bit was cleared have completed */
        dsb();

        /*
         * here's the WFI
         */
        __asm__ __volatile__("wfi");

        if (pen_release == cpu) {
            /*
             * OK, proper wakeup, we're done
             */
            break;
        }

        /*
         * Getting here, means that we have come out of WFI without
         * having been woken up - this shouldn't happen
         *
         * Just note it happening - when we're woken, we can report
         * its occurrence.
         */
        (*spurious)++;
    }
}



/*
 * mt_cpu_kill:
 * @cpu:
 * Return TBD.
 */
int mt_cpu_kill(unsigned int cpu)
{
    HOTPLUG_INFO("mt_cpu_kill, cpu: %d\n", cpu);

#ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
    switch(cpu)
    {
        case 1:
            spm_mtcmos_ctrl_cpu1(STA_POWER_DOWN, 1);
            break;
        case 2:
            spm_mtcmos_ctrl_cpu2(STA_POWER_DOWN, 1);
            break;
        case 3:
            spm_mtcmos_ctrl_cpu3(STA_POWER_DOWN, 1);
            break;
        case 4:
            spm_mtcmos_ctrl_cpu4(STA_POWER_DOWN, 1);
            break;
        case 5:
            spm_mtcmos_ctrl_cpu5(STA_POWER_DOWN, 1);
            break;
        case 6:
            spm_mtcmos_ctrl_cpu6(STA_POWER_DOWN, 1);
            break;
        case 7:
            spm_mtcmos_ctrl_cpu7(STA_POWER_DOWN, 1);
            break;
        default:
            break;
    }
#endif    
    atomic_dec(&hotplug_cpu_count);

    #if 0
        pr_emerg("SPM_CA7_CPU0_PWR_CON: 0x%08x\n", REG_READ(SPM_CA7_CPU0_PWR_CON));
        pr_emerg("SPM_CA7_CPU1_PWR_CON: 0x%08x\n", REG_READ(SPM_CA7_CPU1_PWR_CON));
        pr_emerg("SPM_CA7_CPU2_PWR_CON: 0x%08x\n", REG_READ(SPM_CA7_CPU2_PWR_CON));
        pr_emerg("SPM_CA7_CPU3_PWR_CON: 0x%08x\n", REG_READ(SPM_CA7_CPU3_PWR_CON));
        pr_emerg("SPM_CA7_DBG_PWR_CON: 0x%08x\n", REG_READ(SPM_CA7_DBG_PWR_CON));
        pr_emerg("SPM_CA7_CPUTOP_PWR_CON: 0x%08x\n", REG_READ(SPM_CA7_CPUTOP_PWR_CON));
        pr_emerg("SPM_CA15_CPU0_PWR_CON: 0x%08x\n", REG_READ(SPM_CA15_CPU0_PWR_CON));
        pr_emerg("SPM_CA15_CPU1_PWR_CON: 0x%08x\n", REG_READ(SPM_CA15_CPU1_PWR_CON));
        pr_emerg("SPM_CA15_CPU2_PWR_CON: 0x%08x\n", REG_READ(SPM_CA15_CPU2_PWR_CON));
        pr_emerg("SPM_CA15_CPU3_PWR_CON: 0x%08x\n", REG_READ(SPM_CA15_CPU3_PWR_CON));
        pr_emerg("SPM_CA15_CPUTOP_PWR_CON: 0x%08x\n", REG_READ(SPM_CA15_CPUTOP_PWR_CON));
    #endif

    return 1;
}

/*
 * mt_cpu_die: shutdown a CPU
 * @cpu:
 */
void mt_cpu_die(unsigned int cpu)
{
    int spurious = 0;
#if 0
    //fix for build error
    struct wd_api *wd_api = NULL;

    get_wd_api(&wd_api);
    if (wd_api)
        wd_api->wd_cpu_hot_plug_off_notify(cpu);
#endif

    HOTPLUG_INFO("mt_cpu_die, cpu: %d\n", cpu);

    /*
     * we're ready for shutdown now, so do it
     */
    cpu_enter_lowpower(cpu);
    platform_do_lowpower(cpu, &spurious);

    /*
     * bring this CPU back into the world of cache
     * coherency, and then restore interrupts
     */
    cpu_leave_lowpower(cpu);

    if (spurious)
        HOTPLUG_INFO("platform_do_lowpower, spurious wakeup call, cpu: %d, spurious: %d\n", cpu, spurious);
}

/*
 * mt_cpu_disable:
 * @cpu:
 * Return error code.
 */
int mt_cpu_disable(unsigned int cpu)
{
    /*
    * we don't allow CPU 0 to be shutdown (it is still too special
    * e.g. clock tick interrupts)
    */
    HOTPLUG_INFO("mt_cpu_disable, cpu: %d\n", cpu);
    return cpu == 0 ? -EPERM : 0;
}
