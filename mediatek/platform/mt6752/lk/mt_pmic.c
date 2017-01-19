#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/mt_gpt.h>
#include <platform/mt_pmic_wrap_init.h>
#include <printf.h>
#include <platform/upmu_hw.h>
#include <platform/upmu_common.h>
#include <platform/mt6311.h>

//==============================================================================
// Global variable
//==============================================================================
int Enable_PMIC_LOG = 1;

CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

extern int g_R_BAT_SENSE;    
extern int g_R_I_SENSE;
extern int g_R_CHARGER_1;
extern int g_R_CHARGER_2;

//==============================================================================
// PMIC-AUXADC related define
//==============================================================================
#define VOLTAGE_FULL_RANGE     	1800
#define ADC_PRECISE		32768 	// 15 bits

//==============================================================================
// PMIC-AUXADC global variable
//==============================================================================
kal_int32 count_time_out=10000;

//==============================================================================
// PMIC access API
//==============================================================================
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;    

    //mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //dprintf(INFO, "[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= (MASK << SHIFT);
    *val = (pmic_reg >> SHIFT);    
    //dprintf(INFO, "[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;

    //1. mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //dprintf(INFO, "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= ~(MASK << SHIFT);
    pmic_reg |= (val << SHIFT);

    //2. mt_write_byte(RegNum, pmic_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic_reg, &rdata);
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //dprintf(INFO, "[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic_reg);    

#if 0
    //3. Double Check    
    //mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    dprintf(INFO, "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
#endif    

    return return_value;
}

//==============================================================================
// PMIC Usage APIs
//==============================================================================
U32 get_mt6325_pmic_chip_version (void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(MT6325_SWCID),
                           (&val),
                           (U32)(MT6325_PMIC_SWCID_MASK),
                           (U32)(MT6325_PMIC_SWCID_SHIFT)
	                       );                                                      
    if(ret!=0) dprintf(INFO, "%d", ret);
    return val;
}

kal_bool upmu_is_chr_det(void)
{
    U32 tmp32=0;
    
    #if 0
    tmp32 = 1; // for bring up
    #else
    tmp32 = mt6325_upmu_get_rgs_chrdet();
    #endif
    
    dprintf(CRITICAL, "[upmu_is_chr_det] %d\n", tmp32);
    
    if(tmp32 == 0)
    {        
        return KAL_FALSE;
    }
    else
    {    
        return KAL_TRUE;
    }
}

kal_bool pmic_chrdet_status(void)
{
    if( upmu_is_chr_det() == KAL_TRUE )    
    {
        #ifndef USER_BUILD
        dprintf(INFO, "[pmic_chrdet_status] Charger exist\r\n");
        #endif
        
        return KAL_TRUE;
    }
    else
    {
        #ifndef USER_BUILD
        dprintf(INFO, "[pmic_chrdet_status] No charger\r\n");
        #endif
        
        return KAL_FALSE;
    }
}

int pmic_detect_powerkey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(MT6325_TOPSTATUS),
                           (&val),
                           (U32)(MT6325_PMIC_PWRKEY_DEB_MASK),
                           (U32)(MT6325_PMIC_PWRKEY_DEB_SHIFT)
	                       );                                                         

    if(Enable_PMIC_LOG>1) 
        dprintf(INFO, "%d", ret);

    if (val==1){
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic powerkey Release\n");
        #endif
        
        return 0;
    }else{
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic powerkey Press\n");
        #endif
        
        return 1;
    }
}

int pmic_detect_homekey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(MT6325_TOPSTATUS),
                           (&val),
                           (U32)(MT6325_PMIC_HOMEKEY_DEB_MASK),
                           (U32)(MT6325_PMIC_HOMEKEY_DEB_SHIFT)
	                       );                                                        

    if(Enable_PMIC_LOG>1) 
        dprintf(INFO, "%d", ret);

    if (val==1){     
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic HOMEKEY Release\n");
        #endif
        
        return 0;
    }else{
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic HOMEKEY Press\n");
        #endif
        
        return 1;
    }
}

kal_uint32 upmu_get_reg_value(kal_uint32 reg)
{
    U32 ret=0;
    U32 temp_val=0;

    ret=pmic_read_interface(reg, &temp_val, 0xFFFF, 0x0);

    if(Enable_PMIC_LOG>1) 
        dprintf(INFO, "%d", ret);

    return temp_val;
}

//==============================================================================
// PMIC Init Code
//==============================================================================
void PMIC_INIT_SETTING_V1(void)
{
    //dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Done\n");
}

void PMIC_CUSTOM_SETTING_V1(void)
{
    //dprintf(INFO, "[LK_PMIC_CUSTOM_SETTING_V1] Done\n");
}

U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;

    dprintf(CRITICAL, "[pmic_init] LK Start..................\n");    
    dprintf(CRITICAL, "[pmic_init] MT6325 CHIP Code = 0x%x\n", get_mt6325_pmic_chip_version());    

    PMIC_INIT_SETTING_V1();
    PMIC_CUSTOM_SETTING_V1();

    #if 1
    mt6311_driver_probe();
    #endif

    dprintf(CRITICAL, "[pmic_init] Done\n");
    
    return ret_code;
}

//==============================================================================
// PMIC API for LK : AUXADC
//==============================================================================

void pmic_auxadc_init(void)
{
	U32 adc_busy;
    
	mt6325_upmu_set_rg_vref18_enb(0);
	//change to suspend/resume
	pmic_read_interface(MT6325_AUXADC_ADC19, &adc_busy, MT6325_PMIC_RG_ADC_BUSY_MASK, MT6325_PMIC_RG_ADC_BUSY_SHIFT); 
	if (adc_busy==0) {
		pmic_config_interface(MT6325_AUXADC_CON19, 0x0, MT6325_PMIC_RG_ADC_DECI_GDLY_MASK, MT6325_PMIC_RG_ADC_DECI_GDLY_SHIFT);
	}
	mt6325_upmu_set_rg_adc_gps_status(1);
	mt6325_upmu_set_rg_adc_md_status(1);
	mt6325_upmu_set_rg_deci_gdly_vref18_selb(1);
	mt6325_upmu_set_rg_deci_gdly_sel_mode(1);

	//*set CLK as 26MHz CLK
	mt6325_upmu_set_rg_auxadc_ck_cksel(1);
	//1. set AUXADC CLK HW control
	mt6325_upmu_set_rg_auxadc_ck_pdn_hwen(1);
	//2. turn on AP & MD
	mt6325_upmu_set_rg_clksq_en_aux_ap(1);
	mt6325_upmu_set_auxadc_ck_aon(1);
	dprintf(CRITICAL, "****[pmic_auxadc_init] DONE\n");
}

kal_uint32  pmic_is_auxadc_ready(kal_int32 channel_num, upmu_adc_chip_list_enum chip_num, upmu_adc_user_list_enum user_num)
{
#if 1	
	kal_uint32 ret=0;
	kal_uint32 int_status_val_0=0;
	//unsigned long flags;
	
	//spin_lock_irqsave(&pmic_adc_lock, flags);
	if (chip_num ==MT6325_CHIP) {
		if (user_num == GPS ) {
			ret=mt6325_upmu_get_rg_adc_rdy_gps();
		} else if (user_num == MD ) {
			ret=mt6325_upmu_get_rg_adc_rdy_md();
		} else if (user_num == AP ) {
			pmic_read_interface(MT6325_AUXADC_ADC0+channel_num * 2,(&int_status_val_0),0x8000,0x0);
			ret = int_status_val_0 >> 15;
		}
	}
	//spin_unlock_irqrestore(&pmic_adc_lock, flags);

    if(ret!=0) dprintf(INFO, "%d", ret);
    
	return ret;
#else
    return 0;
#endif    
}

kal_uint32  pmic_get_adc_output(kal_int32 channel_num, upmu_adc_chip_list_enum chip_num, upmu_adc_user_list_enum user_num)
{
#if 1	
	kal_uint32 ret=0;
	kal_uint32 int_status_val_0=0;
	//unsigned long flags;
	
	//spin_lock_irqsave(&pmic_adc_lock, flags);
	if (chip_num ==MT6325_CHIP) {
		if (user_num == GPS ) {
			int_status_val_0 = mt6325_upmu_get_rg_adc_out_gps();
			int_status_val_0 = (int_status_val_0 << 1) + mt6325_upmu_get_rg_adc_out_gps_lsb();
			dprintf(CRITICAL, "adc_out_gps 16_1 = %d, 0 = %d\n", mt6325_upmu_get_rg_adc_out_gps(), mt6325_upmu_get_rg_adc_out_gps_lsb());
		} else if (user_num == MD ) {
			int_status_val_0 = mt6325_upmu_get_rg_adc_out_md();
			int_status_val_0 = (int_status_val_0 << 1) + mt6325_upmu_get_rg_adc_out_md_lsb();
			dprintf(CRITICAL, "adc_out_md 16_1 = %d, 0 = %d\n", mt6325_upmu_get_rg_adc_out_md(), mt6325_upmu_get_rg_adc_out_md_lsb());
		} else if (user_num == AP ) {
			ret=pmic_read_interface(MT6325_AUXADC_ADC0+channel_num * 2,(&int_status_val_0),0x7fff,0x0);
		}
	}

    if(ret!=0) dprintf(INFO, "%d", ret);
    
	//spin_unlock_irqrestore(&pmic_adc_lock, flags);
	return int_status_val_0;
#else
    return 0;
#endif    
}

kal_uint32 PMIC_IMM_RequestAuxadcChannel(upmu_adc_chl_list_enum dwChannel)
{
#if 1
	//unsigned long flags;
	kal_uint32 ret = 0;
	
		switch(dwChannel){ 
		case AUX_BATSNS_AP:
			mt6325_upmu_set_rg_ap_rqst_list(1<<7);
			break;
		case AUX_ISENSE_AP:
			mt6325_upmu_set_rg_ap_rqst_list(1<<6);
			break;
		case AUX_VCDT_AP:
			mt6325_upmu_set_rg_ap_rqst_list(1<<4);
			break;          
		case AUX_BATON_AP:
			pmic_config_interface(0x0a08,0x010b, 0xffff, 0);
			pmic_config_interface(0x0f00,0x0005, 0xffff, 0);
			//upmu_set_reg_value(0x0aba,0x500f);
			mt6325_upmu_set_rg_ap_rqst_list(1<<5);
			break;
		default:
		        dprintf(CRITICAL, "[AUXADC] Invalid channel value(%d)\n", dwChannel);
		        return -1;
		}

	return ret;
#else
	return 0;
#endif   
}

int PMIC_IMM_GetChannelNumber(upmu_adc_chl_list_enum dwChannel)
{
	kal_int32 channel_num;
	channel_num = (dwChannel & (AUXADC_CHANNEL_MASK << AUXADC_CHANNEL_SHIFT)) >> AUXADC_CHANNEL_SHIFT ;
	
	return channel_num;	
}

upmu_adc_chip_list_enum PMIC_IMM_GetChipNumber(upmu_adc_chl_list_enum dwChannel)
{
	upmu_adc_chip_list_enum chip_num;
	chip_num = (upmu_adc_chip_list_enum)(dwChannel & (AUXADC_CHIP_MASK << AUXADC_CHIP_SHIFT)) >> AUXADC_CHIP_SHIFT ;
	
	return chip_num;	
}

upmu_adc_user_list_enum PMIC_IMM_GetUserNumber(upmu_adc_chl_list_enum dwChannel)
{
	upmu_adc_user_list_enum user_num;
	user_num = (upmu_adc_user_list_enum)(dwChannel & (AUXADC_USER_MASK << AUXADC_USER_SHIFT)) >> AUXADC_USER_SHIFT ;
	
	return user_num;		
}

//==============================================================================
// PMIC-AUXADC 
//==============================================================================
int PMIC_IMM_GetOneChannelValue(upmu_adc_chl_list_enum dwChannel, int deCount, int trimd)
{
#if 1
	kal_int32 ret_data;    
	kal_int32 count=0;
	kal_int32 u4Sample_times = 0;
	kal_int32 u4channel=0;    
	kal_int32 adc_result_temp=0;
	kal_int32 r_val_temp=0;   
	kal_int32 adc_result=0;   
	kal_int32 channel_num;
	upmu_adc_chip_list_enum chip_num;
	upmu_adc_user_list_enum user_num;
	
	/*
	AUX_BATSNS_AP =		0x000,
	AUX_ISENSE_AP,
	AUX_VCDT_AP,
	AUX_BATON_AP,
	*/
	
	do
	{
		count=0;
	        ret_data=0;

		channel_num 	= PMIC_IMM_GetChannelNumber(dwChannel);
		chip_num 	= PMIC_IMM_GetChipNumber(dwChannel);
		user_num 	= PMIC_IMM_GetUserNumber(dwChannel);

		dprintf(CRITICAL, "\n[PMIC_IMM_GetOneChannelValue] %d ch=%d, chip= %d, user= %d \n",dwChannel, channel_num, chip_num, user_num);

		if (chip_num == MT6325_CHIP) {
			if (user_num == AP) {
				mt6325_upmu_set_rg_clksq_en_aux_ap(1);
				//mt6325_upmu_set_rg_clksq_en_aux_rsv(1);
				mt6325_upmu_set_strup_auxadc_rstb_sw(1);
				mt6325_upmu_set_strup_auxadc_rstb_sel(1);
			} else if (user_num == MD ) {
				//Prerequisite settings before accessing AuxADC 
				mt6325_upmu_set_rg_clksq_en_aux_md(1);
				mt6325_upmu_set_rg_vref18_enb_md(0);	
			} else if (user_num == GPS) {
				mt6325_upmu_set_rg_clksq_en_aux_gps(1);
			}
		}
		
		if (chip_num == MT6325_CHIP) {
			mt6325_upmu_set_rg_ap_rqst_list(0);
			mt6325_upmu_set_rg_ap_rqst_list_rsv(0);
			mt6325_upmu_set_rg_md_rqst(0);
			mt6325_upmu_set_rg_gps_rqst(0);
		}

		PMIC_IMM_RequestAuxadcChannel(dwChannel);
		udelay(10);
		while( pmic_is_auxadc_ready(channel_num, chip_num, user_num) != 1 ) {
			if( (count++) > count_time_out)
			{
			dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			break;
			}            
		}
		ret_data = pmic_get_adc_output(channel_num, chip_num, user_num);                
		
		//clear
		
		if (chip_num == MT6325_CHIP) {
			mt6325_upmu_set_rg_ap_rqst_list(0);
			mt6325_upmu_set_rg_ap_rqst_list_rsv(0);
			mt6325_upmu_set_rg_md_rqst(0);
			mt6325_upmu_set_rg_gps_rqst(0);
		}
	        u4channel += ret_data;
	
	        u4Sample_times++;
	
		//debug
		//dprintf(CRITICAL, "[AUXADC] output data[%d]=%d\n", 
//			dwChannel, ret_data);
		if (chip_num == MT6325_CHIP) {
		        if (user_num == AP) {
				mt6325_upmu_set_rg_clksq_en_aux_ap(0);
			} else if (user_num == MD) {
				//Prerequisite settings before accessing AuxADC 
				mt6325_upmu_set_rg_clksq_en_aux_md(0);
				mt6325_upmu_set_rg_vref18_enb_md(1);	
			} else if (user_num == GPS) {
				mt6325_upmu_set_rg_clksq_en_aux_gps(0);
			}
		}
	}while (u4Sample_times < deCount);

	/* Value averaging  */ 
	adc_result_temp = u4channel / deCount;
	
	switch(dwChannel){
	case AUX_BATSNS_AP:
		dprintf(CRITICAL, "[AUX_BATSNS_AP]");
		r_val_temp = 4;
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	case AUX_ISENSE_AP:
		dprintf(CRITICAL, "[AUX_ISENSE_AP]");
		r_val_temp = 4;           
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	case AUX_VCDT_AP:
		dprintf(CRITICAL, "[AUX_VCDT_AP]");
		r_val_temp = 1;           
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	case AUX_BATON_AP:
		dprintf(CRITICAL, "[AUX_BATON_AP]");
		r_val_temp = 2;           
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	default:
	    dprintf(CRITICAL, "[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
	    return -1;
	}

	dprintf(CRITICAL, "outputdata=%d, transfer data=%d, r_val=%d.\n", 
	        adc_result_temp, adc_result, r_val_temp);
	
	return adc_result;
#else
	return 0;
#endif   
}

int get_bat_sense_volt(int times)
{
	return PMIC_IMM_GetOneChannelValue(AUX_BATSNS_AP,times,1);
}

int get_i_sense_volt(int times)
{
	return PMIC_IMM_GetOneChannelValue(AUX_ISENSE_AP,times,1);
}

int get_charger_volt(int times)
{
	return PMIC_IMM_GetOneChannelValue(AUX_VCDT_AP,times,1);
}

int get_tbat_volt(int times)
{
	return PMIC_IMM_GetOneChannelValue(AUX_BATON_AP,times,1);
}

void vibr_Enable_HW(void)
{
	mt6325_upmu_set_rg_vibr_vosel(0x5); // 0x5: 2.8V, 0x6: 3V, 0x7: 3.3V
	mt6325_upmu_set_rg_vibr_sw_mode(0);
	mt6325_upmu_set_rg_vibr_fr_ori(1); 
	mt6325_upmu_set_rg_vibr_en(1);     
}

void vibr_Disable_HW(void)
{
	mt6325_upmu_set_rg_vibr_en(0);     
}

