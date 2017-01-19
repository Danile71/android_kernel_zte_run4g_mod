#include <target/board.h>
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#define CFG_POWER_CHARGING
#endif
#ifdef CFG_POWER_CHARGING
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/upmu_hw.h>
#include <platform/upmu_common.h>
#include <platform/boot_mode.h>
#include <platform/mt_gpt.h>
#include <platform/mt_rtc.h>
//#include <platform/mt_disp_drv.h>
//#include <platform/mtk_wdt.h>
//#include <platform/mtk_key.h>
//#include <platform/mt_logo.h>
#include <platform/mt_leds.h>
#include <printf.h>
#include <sys/types.h>
#include <target/cust_battery.h>

#if defined(MTK_BQ24261_SUPPORT)
#include <platform/bq24261.h>
#endif

#if defined(MTK_NCP1854_SUPPORT)
#include <platform/ncp1854.h>
#endif

#if defined(MTK_BQ24196_SUPPORT)
#include <platform/bq24196.h>
#endif

#undef printf


/*****************************************************************************
 *  Type define
 ****************************************************************************/
#if defined(CUST_BATTERY_LOWVOL_THRESOLD)
#define BATTERY_LOWVOL_THRESOLD CUST_BATTERY_LOWVOL_THRESOLD
#else
#define BATTERY_LOWVOL_THRESOLD             3450
#endif

/*****************************************************************************
 *  Global Variable
 ****************************************************************************/
bool g_boot_reason_change = false;

#if defined(STD_AC_LARGE_CURRENT)
int g_std_ac_large_current_en=1;
#else
int g_std_ac_large_current_en=0;
#endif

#if defined(MTK_NCP1854_SUPPORT)
#if defined(NCP1854_PWR_PATH)
int g_ncp1854_power_path_en=1;
#else
int g_ncp1854_power_path_en=0;
#endif
#endif

/*****************************************************************************
 *  Externl Variable
 ****************************************************************************/
extern bool g_boot_menu;
extern void mtk_wdt_restart(void);

void kick_charger_wdt(void)
{
    //mt6325_upmu_set_rg_chrwdt_td(0x0);           // CHRWDT_TD, 4s
    mt6325_upmu_set_rg_chrwdt_td(0x3);           // CHRWDT_TD, 32s for keep charging for lk to kernel
    mt6325_upmu_set_rg_chrwdt_wr(1);             // CHRWDT_WR
    mt6325_upmu_set_rg_chrwdt_int_en(1);         // CHRWDT_INT_EN
    mt6325_upmu_set_rg_chrwdt_en(1);             // CHRWDT_EN
    mt6325_upmu_set_rg_chrwdt_flag_wr(1);        // CHRWDT_WR
}

#if defined(MTK_BATLOWV_NO_PANEL_ON_EARLY)
kal_bool is_low_battery(kal_int32  val)
{
    static UINT8 g_bat_low = 0xFF;

    //low battery only justice once in lk
    if(0xFF != g_bat_low)
        return g_bat_low;
    else
        g_bat_low = FALSE;

    #if defined(SWCHR_POWER_PATH)
    if(0 == val)
        val = get_i_sense_volt(1);
    #else
    if(0 == val)
        val = get_bat_sense_volt(1);
    #endif

    if (val < BATTERY_LOWVOL_THRESOLD)
    {
        printf("%s, TRUE\n", __FUNCTION__);
        g_bat_low = 0x1;
    }

    if(FALSE == g_bat_low)
        printf("%s, FALSE\n", __FUNCTION__);

    return g_bat_low;
}
#endif

void pchr_turn_on_charging(kal_bool bEnable)
{
    mt6325_upmu_set_rg_usbdl_rst(1);       //force leave USBDL mode
	
    kick_charger_wdt();
	
    mt6325_upmu_set_rg_cs_vth(0xC);             // CS_VTH, 450mA
    mt6325_upmu_set_rg_csdac_en(1);				// CSDAC_EN
    mt6325_upmu_set_rg_chr_en(1);				// CHR_EN

    #if defined(MTK_BQ24261_SUPPORT)
    bq24261_hw_init();
    bq24261_charging_enable(bEnable);
    bq24261_dump_register();
    #endif
	
    #if defined(MTK_NCP1854_SUPPORT)
    ncp1854_hw_init();
    ncp1854_charging_enable(bEnable);
    ncp1854_dump_register();
    #endif

    #if defined(MTK_BQ24196_SUPPORT)
    bq24196_hw_init();
    bq24196_charging_enable(bEnable);
    bq24196_dump_register();
    #endif
}

//enter this function when low battery with charger
void check_bat_protect_status()
{
    kal_int32 bat_val = 0;
    
    #if defined(SWCHR_POWER_PATH)
    bat_val = get_i_sense_volt(5);
    #else
    bat_val = get_bat_sense_volt(5);
    #endif
    
    dprintf(CRITICAL, "[%s]: check VBAT=%d mV with %d mV, start charging... \n", __FUNCTION__, bat_val, BATTERY_LOWVOL_THRESOLD);

    if (bat_val < BATTERY_LOWVOL_THRESOLD)
        mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);

    while (bat_val < BATTERY_LOWVOL_THRESOLD)
    {
        mtk_wdt_restart();
        if(upmu_is_chr_det() == KAL_FALSE)
        {
            dprintf(CRITICAL, "[BATTERY] No Charger, Power OFF !\n");
            mt6575_power_off();
            while(1);
        }
    
        pchr_turn_on_charging(KAL_TRUE);

        mdelay(5000);

        #if defined(SWCHR_POWER_PATH)
        #ifndef MTK_NCP1854_SUPPORT
        pchr_turn_on_charging(KAL_FALSE);
        mdelay(100);
        #endif
        bat_val = get_i_sense_volt(5);
        #else
        bat_val = get_bat_sense_volt(5);
        #endif
    }

    dprintf(CRITICAL, "[%s]: check VBAT=%d mV with %d mV, stop charging... \n", __FUNCTION__, bat_val, BATTERY_LOWVOL_THRESOLD);
}

void mt65xx_bat_init(void)
{    
    kal_int32 bat_vol;
    
    // Low Battery Safety Booting
    
    #if defined(SWCHR_POWER_PATH)
    bat_vol = get_i_sense_volt(1);
    #else
    bat_vol = get_bat_sense_volt(1);
    #endif

    pchr_turn_on_charging(KAL_TRUE);
    dprintf(CRITICAL, "[mt65xx_bat_init] check VBAT=%d mV with %d mV\n", bat_vol, BATTERY_LOWVOL_THRESOLD);    

    if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && (mt6325_upmu_get_pwrkey_deb()==0) ) {
            dprintf(CRITICAL, "[mt65xx_bat_init] KPOC+PWRKEY => change boot mode\n");        
    
            g_boot_reason_change = true;
    }
    rtc_boot_check(false);

    #ifndef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
    #ifndef MTK_BATLOWV_NO_PANEL_ON_EARLY
    if (bat_vol < BATTERY_LOWVOL_THRESOLD)
    #else
    if (is_low_battery(bat_vol))
    #endif
    {
        if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && upmu_is_chr_det() == KAL_TRUE)
        {
            dprintf(CRITICAL, "[%s] Kernel Low Battery Power Off Charging Mode\n", __func__);
            g_boot_mode = LOW_POWER_OFF_CHARGING_BOOT;
            #if defined(SWCHR_POWER_PATH)
            check_bat_protect_status();
            #endif
            return;
        }
        else
        {
            dprintf(CRITICAL, "[BATTERY] battery voltage(%dmV) <= CLV ! Can not Boot Linux Kernel !! \n\r",bat_vol);
    #ifndef NO_POWER_OFF
            mt6575_power_off();
    #endif            
            while(1)
            {
                dprintf(CRITICAL, "If you see the log, please check with RTC power off API\n\r");
            }
        }
    }
    #endif
    return;
}

#else

#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <printf.h>

void mt65xx_bat_init(void)
{
    dprintf(CRITICAL, "[BATTERY] Skip mt65xx_bat_init !!\n\r");
    dprintf(CRITICAL, "[BATTERY] If you want to enable power off charging, \n\r");
    dprintf(CRITICAL, "[BATTERY] Please #define CFG_POWER_CHARGING!!\n\r");
}

#endif
