
#ifndef _PL_MT_PMIC_H_
#define _PL_MT_PMIC_H_

//==============================================================================
// The CHIP INFO
//==============================================================================
#define PMIC6325_E1_CID_CODE    0x2510
#define PMIC6325_E2_CID_CODE    0x2520
#define PMIC6325_E3_CID_CODE    0x2530
#define PMIC6325_E4_CID_CODE    0x2540
#define PMIC6325_E5_CID_CODE    0x2550
#define PMIC6325_E6_CID_CODE    0x2560


//==============================================================================
// PMIC AUXADC defines
//==============================================================================
#define AUXADC_CHANNEL_MASK	0x1f
#define AUXADC_CHANNEL_SHIFT	0
#define AUXADC_CHIP_MASK	0x03
#define AUXADC_CHIP_SHIFT	5
#define AUXADC_USER_MASK	0x0f
#define AUXADC_USER_SHIFT	8


/* ADC Channel Number */
typedef enum {
	//MT6325
	AUX_BATSNS_AP =		0x000,
	AUX_ISENSE_AP,
	AUX_VCDT_AP,
	AUX_BATON_AP,
	AUX_TSENSE_AP,
	AUX_TSENSE_MD =		0x005,
	AUX_VACCDET_AP =	0x007,
	AUX_VISMPS_AP =		0x00B,
	AUX_ICLASSAB_AP =	0x016,
	AUX_HP_AP =		0x017,
	AUX_CH10_AP =		0x018,
	AUX_VBIF_AP =		0x019,
	
	AUX_CH0_6311 =		0x020,
	AUX_CH1_6311 =		0x021,

	AUX_ADCVIN0_MD =	0x10F,
	AUX_ADCVIN0_GPS = 	0x20C,
	AUX_CH12 = 		0x1011,
	AUX_CH13 = 		0x2011,
	AUX_CH14 = 		0x3011,
	AUX_CH15 = 		0x4011,
} upmu_adc_chl_list_enum;

typedef enum {
	AP = 0,
	MD,
	GPS,
	AUX_USER_MAX	
} upmu_adc_user_list_enum;
typedef enum {
	MT6325_CHIP = 0,
	MT6311_CHIP,
	ADC_CHIP_MAX
} upmu_adc_chip_list_enum;

//==============================================================================
// PMIC define
//==============================================================================
typedef enum {
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,          // USB : 450mA
    CHARGING_HOST,
    NONSTANDARD_CHARGER,    // AC : 450mA~1A 
    STANDARD_CHARGER,       // AC : ~1A
    APPLE_2_1A_CHARGER,     // 2.1A apple charger
    APPLE_1_0A_CHARGER,     // 1A apple charger
    APPLE_0_5A_CHARGER,     // 0.5A apple charger
} CHARGER_TYPE;


//==============================================================================
// PMIC Register Index
//==============================================================================
//register number
#include <upmu_hw.h>


//==============================================================================
// PMIC Status Code
//==============================================================================
#define PMIC_TEST_PASS               0x0000
#define PMIC_TEST_FAIL               0xB001
#define PMIC_EXCEED_I2C_FIFO_LENGTH  0xB002
#define PMIC_CHRDET_EXIST            0xB003
#define PMIC_CHRDET_NOT_EXIST        0xB004
#define PMIC_VBAT_DROP		0xB005
#define PMIC_VBAT_NOT_DROP	0xB006
//==============================================================================
// PMIC Exported Function
//==============================================================================
extern CHARGER_TYPE mt_charger_type_detection(void);
extern U32 pmic_IsUsbCableIn (void);
extern int pmic_detect_powerkey(void);
extern int pmic_detect_homekey(void);
extern void hw_set_cc(int cc_val);
extern int hw_check_battery(void);
extern void pl_charging(int en_chr);
extern void pl_kick_chr_wdt(void);
extern void pl_close_pre_chr_led(void);
extern void pl_hw_ulc_det(void);
extern U32 pmic_init (void);
extern int pmic_IsVbatDrop(void);

#endif 

