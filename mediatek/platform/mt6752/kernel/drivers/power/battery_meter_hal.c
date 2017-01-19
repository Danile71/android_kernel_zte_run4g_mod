#include <linux/xlog.h>
#include <linux/delay.h> 

#include <mach/upmu_common.h>
#include <mach/upmu_hw.h>
#include <mach/upmu_sw.h>

#include <mach/battery_meter_hal.h>
#include <cust_battery_meter.h>
#include <cust_pmic.h>

//============================================================ //
//define
//============================================================ //
#define STATUS_OK    0
#define STATUS_UNSUPPORTED    -1
#define VOLTAGE_FULL_RANGE    1800
#define ADC_PRECISE           32768  // 12 bits

#define UNIT_FGCURRENT     (158122)     // 158.122 uA

//============================================================ //
//global variable
//============================================================ //
kal_int32 chip_diff_trim_value_4_0 = 0;
kal_int32 chip_diff_trim_value = 0; // unit = 0.1

kal_int32 g_hw_ocv_tune_value = 0;

kal_bool g_fg_is_charging = 0;

//============================================================ //
//function prototype
//============================================================ //

//============================================================ //
//extern variable
//============================================================ //
 
//============================================================ //
//extern function
//============================================================ //
extern int PMIC_IMM_GetOneChannelValue(upmu_adc_chl_list_enum dwChannel, int deCount, int trimd);
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
extern int IMM_IsAdcInitReady(void);
extern U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT);
extern U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern unsigned int get_pmic_mt6325_cid(void);

//============================================================ // 
void get_hw_chip_diff_trim_value(void)
{
#if defined(CONFIG_POWER_EXT)
#else

    #if 1
    kal_int32 reg_val = 0;

    reg_val = upmu_get_reg_value(0xCB8);
    chip_diff_trim_value_4_0 = (reg_val>>10)&0x001F;
    
    bm_print(BM_LOG_CRTI, "[Chip_Trim] Reg[0xCB8]=0x%x, chip_diff_trim_value_4_0=%d\n", reg_val, chip_diff_trim_value_4_0);
    #else
    bm_print(BM_LOG_FULL, "[Chip_Trim] need check reg number\n");
    #endif

    switch(chip_diff_trim_value_4_0){       
        case 0:    chip_diff_trim_value = 1000; break;
        case 1:    chip_diff_trim_value = 1005; break;
        case 2:    chip_diff_trim_value = 1010; break;
        case 3:    chip_diff_trim_value = 1015; break;
        case 4:    chip_diff_trim_value = 1020; break;
        case 5:    chip_diff_trim_value = 1025; break;
        case 6:    chip_diff_trim_value = 1030; break;
        case 7:    chip_diff_trim_value = 1036; break;
        case 8:    chip_diff_trim_value = 1041; break;
        case 9:    chip_diff_trim_value = 1047; break;
        case 10:   chip_diff_trim_value = 1052; break;
        case 11:   chip_diff_trim_value = 1058; break;
        case 12:   chip_diff_trim_value = 1063; break;
        case 13:   chip_diff_trim_value = 1069; break;
        case 14:   chip_diff_trim_value = 1075; break;
        case 15:   chip_diff_trim_value = 1081; break;
        case 31:   chip_diff_trim_value = 995; break; 
        case 30:   chip_diff_trim_value = 990; break; 
        case 29:   chip_diff_trim_value = 985; break; 
        case 28:   chip_diff_trim_value = 980; break; 
        case 27:   chip_diff_trim_value = 975; break; 
        case 26:   chip_diff_trim_value = 970; break; 
        case 25:   chip_diff_trim_value = 966; break; 
        case 24:   chip_diff_trim_value = 961; break; 
        case 23:   chip_diff_trim_value = 956; break; 
        case 22:   chip_diff_trim_value = 952; break; 
        case 21:   chip_diff_trim_value = 947; break; 
        case 20:   chip_diff_trim_value = 943; break; 
        case 19:   chip_diff_trim_value = 938; break; 
        case 18:   chip_diff_trim_value = 934; break; 
        case 17:   chip_diff_trim_value = 930; break; 
        default:
            bm_print(BM_LOG_FULL, "[Chip_Trim] Invalid value(%d)\n", chip_diff_trim_value_4_0);
            break;
    }

    bm_print(BM_LOG_CRTI, "[Chip_Trim] chip_diff_trim_value=%d\n", chip_diff_trim_value);
#endif
}

kal_int32 use_chip_trim_value(kal_int32 not_trim_val)
{
#if defined(CONFIG_POWER_EXT)
    return not_trim_val;
#else
    kal_int32 ret_val=0;

    ret_val=((not_trim_val*chip_diff_trim_value)/1000);

    bm_print(BM_LOG_FULL, "[use_chip_trim_value] %d -> %d\n", not_trim_val, ret_val);
    
    return ret_val;
#endif    
}

int get_hw_ocv(void)
{
#if defined(CONFIG_POWER_EXT)
    return 4001;    
    bm_print(BM_LOG_CRTI, "[get_hw_ocv] TBD\n");
#else
    kal_int32 adc_result_reg=0;
    kal_int32 adc_result=0;
    kal_int32 r_val_temp=4; //MT6325 use 2, old chip use 4    

    #if defined(SWCHR_POWER_PATH)
    adc_result_reg = mt6325_upmu_get_rg_adc_out_wakeup_swchr();
    adc_result = (adc_result_reg*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
    bm_print(BM_LOG_CRTI, "[oam] get_hw_ocv (swchr) : adc_result_reg=%d, adc_result=%d\n", 
        adc_result_reg, adc_result);
    #else
    adc_result_reg = mt6325_upmu_get_rg_adc_out_wakeup_pchr();
    adc_result = (adc_result_reg*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;        
    bm_print(BM_LOG_CRTI, "[oam] get_hw_ocv (pchr) : adc_result_reg=%d, adc_result=%d\n", 
        adc_result_reg, adc_result);
    #endif

    adc_result += g_hw_ocv_tune_value;
    return adc_result;
#endif    
}


//============================================================//

#if defined(CONFIG_POWER_EXT) 
//
#else
static void dump_nter(void)
{
    bm_print(BM_LOG_CRTI, "[dump_nter] mt6325_upmu_get_fg_nter_29_16 = 0x%x\r\n", 
        mt6325_upmu_get_fg_nter_29_16());
    bm_print(BM_LOG_CRTI, "[dump_nter] mt6325_upmu_get_fg_nter_15_00 = 0x%x\r\n", 
        mt6325_upmu_get_fg_nter_15_00());
}

static void dump_car(void)
{
    bm_print(BM_LOG_CRTI, "[dump_car] upmu_get_fg_car_31_16 = 0x%x\r\n", 
        mt6325_upmu_get_fg_car_31_16());
    bm_print(BM_LOG_CRTI, "[dump_car] upmu_get_fg_car_15_00 = 0x%x\r\n", 
        mt6325_upmu_get_fg_car_15_00());
}

static kal_uint32 fg_get_data_ready_status(void)
{
    kal_uint32 ret=0;
    kal_uint32 temp_val=0;
    
    ret=pmic_read_interface(MT6325_FGADC_CON0, &temp_val, 0xFFFF, 0x0);
    
    bm_print(BM_LOG_FULL, "[fg_get_data_ready_status] Reg[0x%x]=0x%x\r\n", MT6325_FGADC_CON0, temp_val);
    
    temp_val = (temp_val & (MT6325_PMIC_FG_LATCHDATA_ST_MASK << MT6325_PMIC_FG_LATCHDATA_ST_SHIFT)) >> MT6325_PMIC_FG_LATCHDATA_ST_SHIFT;

    return temp_val;    
}
#endif    

static kal_int32 fgauge_read_current(void *data);
static kal_int32 fgauge_initialization(void *data)
{
#if defined(CONFIG_POWER_EXT)
    //
#else
    kal_uint32 ret=0;
    kal_int32 current_temp = 0;
    int m = 0;

    get_hw_chip_diff_trim_value();

// 1. HW initialization
    //FGADC clock is 32768Hz from RTC
    //Enable FGADC in current mode at 32768Hz with auto-calibration
    
    //(1)    Enable VA2
    //(2)    Enable FGADC clock for digital
    mt6325_upmu_set_rg_fgadc_ana_ck_pdn(0);
    mt6325_upmu_set_rg_fgadc_dig_ck_pdn(0);
    //(3)    Set current mode, auto-calibration mode and 32KHz clock source
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0028, 0x00FF, 0x0);
    //(4)    Enable FGADC
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0029, 0x00FF, 0x0);

    //reset HW FG
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x7100, 0xFF00, 0x0);
    bm_print(BM_LOG_CRTI, "******** [fgauge_initialization] reset HW FG!\n" );
    
    //set FG_OSR
    ret=pmic_config_interface(MT6325_FGADC_CON11, 0x8, 0xF, 0x0);
    bm_print(BM_LOG_CRTI, "[fgauge_initialization] Reg[0x%x]=0x%x\n",MT6325_FGADC_CON11, upmu_get_reg_value(MT6325_FGADC_CON11));
    
    //make sure init finish
    m = 0;
    while (current_temp == 0)
    {
        fgauge_read_current(&current_temp);
        m++;
        if (m>1000)
        {            
            bm_print(BM_LOG_CRTI, "[fgauge_initialization] timeout!\r\n");
            break;
        }    
    }
    
    bm_print(BM_LOG_CRTI, "******** [fgauge_initialization] Done!\n" );
#endif    

    return STATUS_OK;
}

static kal_int32 fgauge_read_current(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 0;
#else    
    kal_uint16 uvalue16 = 0;
    kal_int32 dvalue = 0; 
    int m = 0;
    kal_int64 Temp_Value = 0;
    kal_int32 Current_Compensate_Value=0;
    kal_uint32 ret = 0;

// HW Init
    //(1)    i2c_write (0x60, 0xC8, 0x01); // Enable VA2    
    //(2)    i2c_write (0x61, 0x15, 0x00); // Enable FGADC clock for digital
    //(3)    i2c_write (0x61, 0x69, 0x28); // Set current mode, auto-calibration mode and 32KHz clock source
    //(4)    i2c_write (0x61, 0x69, 0x29); // Enable FGADC

//Read HW Raw Data
    //(1)    Set READ command
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0200, 0xFF00, 0x0);
    //(2)     Keep i2c read when status = 1 (0x06)
    m=0;
    while ( fg_get_data_ready_status() == 0 )
    {        
        m++;
        if(m>1000)
        {            
            bm_print(BM_LOG_FULL, "[fgauge_read_current] fg_get_data_ready_status timeout 1 !\r\n");            
            break;
        }
    }
    //(3)    Read FG_CURRENT_OUT[15:08]
    //(4)    Read FG_CURRENT_OUT[07:00]
    uvalue16 = mt6325_upmu_get_fg_current_out();
    bm_print(BM_LOG_FULL, "[fgauge_read_current] : FG_CURRENT = %x\r\n", uvalue16);
    //(5)    (Read other data)
    //(6)    Clear status to 0
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0800, 0xFF00, 0x0);
    //(7)    Keep i2c read when status = 0 (0x08)
	//while ( fg_get_sw_clear_status() != 0 )
    m=0;
    while ( fg_get_data_ready_status() != 0 )
    {         
        m++;
        if(m>1000)
        {
            bm_print(BM_LOG_FULL, "[fgauge_read_current] fg_get_data_ready_status timeout 2 !\r\n");
            break;
        }
    }    
    //(8)    Recover original settings
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0000, 0xFF00, 0x0);

//calculate the real world data    
    dvalue = (kal_uint32) uvalue16;
    if( dvalue == 0 )
    {
        Temp_Value = (kal_int64) dvalue;
        g_fg_is_charging = KAL_FALSE;
    }
    else if( dvalue > 32767 ) // > 0x8000
    {
        Temp_Value = (kal_int64)(dvalue - 65535);
        Temp_Value = Temp_Value - (Temp_Value*2);
        g_fg_is_charging = KAL_FALSE;
    }
    else
    {
        Temp_Value = (kal_int64) dvalue;
        g_fg_is_charging = KAL_TRUE;
    }    
    
    Temp_Value = Temp_Value * UNIT_FGCURRENT;    
    do_div(Temp_Value, 100000);
    dvalue = (kal_uint32)Temp_Value;
    
    if( g_fg_is_charging == KAL_TRUE )
    {
        bm_print(BM_LOG_FULL, "[fgauge_read_current] current(charging) = %d mA\r\n", dvalue);
    }
    else
    {
        bm_print(BM_LOG_FULL, "[fgauge_read_current] current(discharging) = %d mA\r\n", dvalue);
    }

// Auto adjust value
    if(R_FG_VALUE != 20)
    {
        bm_print(BM_LOG_FULL, "[fgauge_read_current] Auto adjust value due to the Rfg is %d\n Ori current=%d, ", R_FG_VALUE, dvalue);

        dvalue = (dvalue*20)/R_FG_VALUE;
     
        bm_print(BM_LOG_FULL, "[fgauge_read_current] new current=%d\n", dvalue);
    }

// K current 
    if(R_FG_BOARD_SLOPE != R_FG_BOARD_BASE)
    {
        dvalue = ( (dvalue*R_FG_BOARD_BASE) + (R_FG_BOARD_SLOPE/2) ) / R_FG_BOARD_SLOPE;
    }

// current compensate
    if(g_fg_is_charging == KAL_TRUE)
    {
        dvalue = dvalue + Current_Compensate_Value;
    }
    else
    {
        dvalue = dvalue - Current_Compensate_Value;
    }

    bm_print(BM_LOG_FULL, "[fgauge_read_current] ori current=%d\n", dvalue);
    
    dvalue = ((dvalue*CAR_TUNE_VALUE)/100);

    dvalue = use_chip_trim_value(dvalue);
    
    bm_print(BM_LOG_FULL, "[fgauge_read_current] final current=%d (ratio=%d)\n", dvalue, CAR_TUNE_VALUE);

    *(kal_int32*)(data) = dvalue;
#endif
    
    return STATUS_OK;
}

static kal_int32 fgauge_read_current_sign(void *data)
{
    *(kal_bool*)(data) = g_fg_is_charging;
    
    return STATUS_OK;
}

static kal_int32 fgauge_read_columb_internal(void *data, int reset)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 0;
#else    
    kal_uint32 uvalue32_CAR = 0;
    kal_uint32 uvalue32_CAR_MSB = 0;
    kal_int32 dvalue_CAR = 0;
    int m = 0;
    kal_int64 Temp_Value = 0;
    kal_uint32 ret = 0;

// HW Init
    //(1)    i2c_write (0x60, 0xC8, 0x01); // Enable VA2
    //(2)    i2c_write (0x61, 0x15, 0x00); // Enable FGADC clock for digital
    //(3)    i2c_write (0x61, 0x69, 0x28); // Set current mode, auto-calibration mode and 32KHz clock source
    //(4)    i2c_write (0x61, 0x69, 0x29); // Enable FGADC

//Read HW Raw Data
    //(1)    Set READ command
    if (reset == 0)
    {
        ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0200, 0xFF00, 0x0);
    }
    else
    {
        ret=pmic_config_interface(MT6325_FGADC_CON0, 0x7300, 0xFF00, 0x0);
    }
    
    //(2)    Keep i2c read when status = 1 (0x06)
    m=0;
    while ( fg_get_data_ready_status() == 0 )
    {        
        m++;
        if(m>1000)
        {            
            bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] fg_get_data_ready_status timeout 1 !\r\n");
            break;
        }
    }
    //(3)    Read FG_CURRENT_OUT[28:14]
    //(4)    Read FG_CURRENT_OUT[31]
    uvalue32_CAR =  ( mt6325_upmu_get_fg_car_15_00() ) >> 14;
    uvalue32_CAR |= ( (mt6325_upmu_get_fg_car_31_16())&0x7FFF ) << 2;    
    
    uvalue32_CAR_MSB = (mt6325_upmu_get_fg_car_31_16() & 0x8000)>>15;
    
    bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] FG_CAR = 0x%x\r\n", uvalue32_CAR);
    bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] uvalue32_CAR_MSB = 0x%x\r\n", uvalue32_CAR_MSB);
    
    //(5)    (Read other data)
    //(6)    Clear status to 0
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0800, 0xFF00, 0x0);
    //(7)    Keep i2c read when status = 0 (0x08)
    //while ( fg_get_sw_clear_status() != 0 ) 
    m=0;
    while ( fg_get_data_ready_status() != 0 )
    {         
        m++;
        if(m>1000)
        {            
            bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] fg_get_data_ready_status timeout 2 !\r\n");
            break;
        }
    }    
    //(8)    Recover original settings
    ret=pmic_config_interface(MT6325_FGADC_CON0, 0x0000, 0xFF00, 0x0);    

//calculate the real world data    
    dvalue_CAR = (kal_int32) uvalue32_CAR;    

    if(uvalue32_CAR == 0)
    {
        Temp_Value = 0;
    }
    else if(uvalue32_CAR == 0x1ffff)
    {
        Temp_Value = 0;
    }
    else if(uvalue32_CAR_MSB == 0x1)
    {
        //dis-charging
        Temp_Value = (kal_int64)(dvalue_CAR - 0x1ffff); // keep negative value
        Temp_Value = Temp_Value - (Temp_Value*2);   
    }
    else
    {
        //charging
        Temp_Value = (kal_int64) dvalue_CAR;
    }

    #if 0
    Temp_Value = ( ((Temp_Value*35986)/10) + (5) )/10; //[28:14]'s LSB=359.86 uAh
    #else
    Temp_Value = Temp_Value * 35986;
    do_div(Temp_Value, 10);
    Temp_Value = Temp_Value + 5;
    do_div(Temp_Value, 10);
    #endif

    #if 0
    dvalue_CAR = Temp_Value / 1000; //mAh
    #else
    //dvalue_CAR = (Temp_Value/8)/1000; //mAh, due to FG_OSR=0x8
    do_div(Temp_Value, 8);
    do_div(Temp_Value, 1000);

    if(uvalue32_CAR_MSB == 0x1)
        dvalue_CAR = (kal_int32)(Temp_Value - (Temp_Value*2));  // keep negative value
    else
        dvalue_CAR = (kal_int32)Temp_Value;
    #endif
    
    bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] dvalue_CAR = %d\r\n", dvalue_CAR);    

    //#if (OSR_SELECT_7 == 1)
    #if 0
        dvalue_CAR = dvalue_CAR * 8;
        if (Enable_FGADC_LOG == 1) {
            xlog_printk(ANDROID_LOG_DEBUG, "[fgauge_read_columb_internal] : dvalue_CAR update to %d\r\n", dvalue_CAR);
        }
    #endif        
   
    
//Auto adjust value
    if(R_FG_VALUE != 20)
    {
        bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] Auto adjust value deu to the Rfg is %d\n Ori CAR=%d, ", R_FG_VALUE, dvalue_CAR);            
        
        dvalue_CAR = (dvalue_CAR*20)/R_FG_VALUE;
        
        bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] new CAR=%d\n", dvalue_CAR);            
    }

    dvalue_CAR = ((dvalue_CAR*CAR_TUNE_VALUE)/100);

    dvalue_CAR = use_chip_trim_value(dvalue_CAR);
    
    bm_print(BM_LOG_FULL, "[fgauge_read_columb_internal] final dvalue_CAR = %d\r\n", dvalue_CAR);

    dump_nter();
    dump_car();

    *(kal_int32*)(data) = dvalue_CAR;
#endif    

    return STATUS_OK;
}

static kal_int32 fgauge_read_columb(void *data)
{
    return fgauge_read_columb_internal(data, 0);
}

static kal_int32 fgauge_hw_reset(void *data)
{
#if defined(CONFIG_POWER_EXT)
    //
#else
    volatile kal_uint32 val_car = 1;
    kal_uint32 val_car_temp = 1;
    kal_uint32 ret = 0;

    bm_print(BM_LOG_FULL, "[fgauge_hw_reset] : Start \r\n");
    
    while(val_car != 0x0)
    {        
        ret=pmic_config_interface(MT6325_FGADC_CON0, 0x7100, 0xFF00, 0x0);        
        fgauge_read_columb_internal(&val_car_temp, 1);
        val_car = val_car_temp;
        bm_print(BM_LOG_FULL, "#");
    }
    
    bm_print(BM_LOG_FULL, "[fgauge_hw_reset] : End \r\n");
#endif    
    
    return STATUS_OK;
}


static kal_int32 read_adc_v_bat_sense(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 4201;
#else
#if defined(SWCHR_POWER_PATH)
	*(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(AUX_ISENSE_AP,*(kal_int32*)(data),1);
#else
    *(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(AUX_BATSNS_AP,*(kal_int32*)(data),1);
#endif
#endif

    return STATUS_OK;
}



static kal_int32 read_adc_v_i_sense(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 4202;
#else
#if defined(SWCHR_POWER_PATH)
	*(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(AUX_BATSNS_AP,*(kal_int32*)(data),1);
#else
    *(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(AUX_ISENSE_AP,*(kal_int32*)(data),1);
#endif
#endif

    return STATUS_OK;
}

static kal_int32 read_adc_v_bat_temp(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 0;
#else
    #if defined(MTK_PCB_TBAT_FEATURE)
        //no HW support
    #else
        bm_print(BM_LOG_FULL, "[read_adc_v_bat_temp] return PMIC_IMM_GetOneChannelValue(4,times,1);\n");
        *(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(AUX_BATON_AP,*(kal_int32*)(data),1);
    #endif
#endif

    return STATUS_OK;
}

static kal_int32 read_adc_v_charger(void *data)
{    
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 5001;
#else
    kal_int32 val;
    val = PMIC_IMM_GetOneChannelValue(AUX_VCDT_AP,*(kal_int32*)(data),1);
    val = (((R_CHARGER_1+R_CHARGER_2)*100*val)/R_CHARGER_2)/100;
    *(kal_int32*)(data) = val;
#endif

    return STATUS_OK;
}

static kal_int32 read_hw_ocv(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 3999;
#else
    #if 0
    *(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(AUX_BATSNS_AP,5,1);
    bm_print(BM_LOG_CRTI, "[read_hw_ocv] By SW AUXADC for bring up\n");
    #else
    *(kal_int32*)(data) = get_hw_ocv();
    #endif
#endif

    return STATUS_OK;
}

static kal_int32 dump_register_fgadc(void *data)
{
    return STATUS_OK;
}

static kal_int32 (* const bm_func[BATTERY_METER_CMD_NUMBER])(void *data)=
{
    fgauge_initialization		//hw fuel gague used only
    
    ,fgauge_read_current		//hw fuel gague used only
    ,fgauge_read_current_sign	//hw fuel gague used only
    ,fgauge_read_columb			//hw fuel gague used only

    ,fgauge_hw_reset			//hw fuel gague used only
    
    ,read_adc_v_bat_sense
    ,read_adc_v_i_sense
    ,read_adc_v_bat_temp
    ,read_adc_v_charger

    ,read_hw_ocv
    ,dump_register_fgadc		//hw fuel gague used only
};

kal_int32 bm_ctrl_cmd(BATTERY_METER_CTRL_CMD cmd, void *data)
{
    kal_int32 status;

    if(cmd < BATTERY_METER_CMD_NUMBER)
        status = bm_func[cmd](data);
    else
        return STATUS_UNSUPPORTED;
    
    return status;
}

