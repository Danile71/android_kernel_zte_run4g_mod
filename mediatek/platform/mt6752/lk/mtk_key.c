#include <printf.h>
#include <platform/mt_typedefs.h>
#include <platform/mtk_key.h>
#include <platform/boot_mode.h>
#include <platform/mt_pmic.h>
#include <platform/mt_gpio.h>
#include <platform/mt_pmic_wrap_init.h>
#include <platform/sync_write.h>
#include <platform/upmu_common.h>
#include <target/cust_key.h>

extern int pmic_detect_powerkey(void);
extern int pmic_detect_homekey(void);

void set_kpd_pmic_mode(void)
{
	return;
}

void disable_PMIC_kpd_clock(void)
{
	return;
}

void enable_PMIC_kpd_clock(void)
{
	return;
}

BOOL mtk_detect_key(unsigned short key)	/* key: HW keycode */
{
#ifdef MACH_FPGA
	return FALSE;
#else

	dprintf(ALWAYS, "mtk detect key function key = %d\n", key);

	unsigned short idx, bit, din;

	if (key >= KPD_NUM_KEYS)
		return FALSE;
#if 0
	if (key % 9 == 8)
		key = 8;
#endif
	if (key == MTK_PMIC_PWR_KEY)
    	{ /* Power key */
		if (1 == pmic_detect_powerkey())
		{
			//dbg_print ("power key is pressed\n");
			return TRUE;
		}
		return FALSE;
	}

#ifdef MTK_PMIC_RST_KEY
	if (key == MTK_PMIC_RST_KEY)
	{
		dprintf(INFO, "mtk detect key function pmic_detect_homekey MTK_PMIC_RST_KEY = %d\n", MTK_PMIC_RST_KEY);
		if (1 == pmic_detect_homekey())
		{
			dprintf(ALWAYS, "mtk detect key function pmic_detect_homekey pressed\n");
			return TRUE;
		}
		return FALSE;
	}
#endif

	idx = key / 16;
	bit = key % 16;

	din = DRV_Reg16(KP_MEM1 + (idx << 2)) & (1U << bit);
	if (!din) {
		dprintf(ALWAYS, "key %d is pressed\n", key);
		return TRUE;
	}
	return FALSE;
#endif
}

BOOL mtk_detect_pmic_just_rst(void)
{
	kal_uint32 just_rst = 0;

	dprintf(ALWAYS, "detecting pmic just reset\n");

	just_rst = mt6325_upmu_get_just_pwrkey_rst();
	// pmic_read_interface(MT6325_STRUP_CON9, &just_rst, MT6325_PMIC_JUST_PWRKEY_RST_MASK, MT6325_PMIC_JUST_PWRKEY_RST_SHIFT);
	if (just_rst)
	{
		dprintf(ALWAYS, "Just recover from a reset\n");
		mt6325_upmu_set_clr_just_rst(0x1);
		// pmic_config_interface(MT6325_STRUP_CON9, 0x01, MT6325_PMIC_CLR_JUST_RST_MASK, MT6325_PMIC_CLR_JUST_RST_SHIFT);
		return TRUE;
	}
	return FALSE;
}
