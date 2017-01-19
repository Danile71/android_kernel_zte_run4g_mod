#include <typedefs.h>
#include <platform.h>
#include <pmic_wrap_init.h>
#include <pmic.h>
#include <mt6311.h>

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
        print("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= (MASK << SHIFT);
    *val = (pmic_reg >> SHIFT);    
    //print("[pmic_read_interface] val=0x%x\n", *val);

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
        print("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= ~(MASK << SHIFT);
    pmic_reg |= (val << SHIFT);

    //2. mt_write_byte(RegNum, pmic_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic_reg, &rdata);
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic_reg);    

#if 0
    //3. Double Check    
    //mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
#endif    

    return return_value;
}

//==============================================================================
// PMIC-Charger Type Detection
//==============================================================================
CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;


#if defined(CONFIG_POWER_EXT)

int hw_charging_get_charger_type(void)
{
    g_chr_type_num = STANDARD_HOST;

    return STANDARD_HOST;
}

#else

void mt6325_upmu_set_rg_bc11_bb_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_BB_CTRL_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_BB_CTRL_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

void mt6325_upmu_set_rg_bc11_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_RST_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_RST_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

void mt6325_upmu_set_rg_bc11_vsrc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_VSRC_EN_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_VSRC_EN_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

void mt6325_upmu_set_rg_bc11_aca_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_ACA_EN_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_ACA_EN_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

kal_uint32 mt6325_upmu_get_rgs_bc11_cmp_out(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  ret=pmic_read_interface( (kal_uint32)(MT6325_CHR_CON20),
                           (&val),
                           (kal_uint32)(MT6325_PMIC_RGS_BC11_CMP_OUT_MASK),
                           (kal_uint32)(MT6325_PMIC_RGS_BC11_CMP_OUT_SHIFT)
	                       );
  if(ret!=0) print("%d", ret);

  return val;
}

void mt6325_upmu_set_rg_bc11_vref_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_VREF_VTH_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_VREF_VTH_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

void mt6325_upmu_set_rg_bc11_cmp_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_CMP_EN_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_CMP_EN_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

void mt6325_upmu_set_rg_bc11_ipd_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_IPD_EN_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_IPD_EN_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

void mt6325_upmu_set_rg_bc11_ipu_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_IPU_EN_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_IPU_EN_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

void mt6325_upmu_set_rg_bc11_bias_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(MT6325_CHR_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_BIAS_EN_MASK),
                             (kal_uint32)(MT6325_PMIC_RG_BC11_BIAS_EN_SHIFT)
	                         );
  if(ret!=0) print("%d", ret);
}

extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);
extern void mdelay (unsigned long msec);
int g_bq24261_log_en = 1;

static void hw_bc11_init(void)
{
    //mdelay(300);
    Charger_Detect_Init();
        
    //RG_bc11_BIAS_EN=1    
    mt6325_upmu_set_rg_bc11_bias_en(0x1);
    //RG_bc11_VSRC_EN[1:0]=00
    mt6325_upmu_set_rg_bc11_vsrc_en(0x0);
    //RG_bc11_VREF_VTH = [1:0]=00
    mt6325_upmu_set_rg_bc11_vref_vth(0x0);
    //RG_bc11_CMP_EN[1.0] = 00
    mt6325_upmu_set_rg_bc11_cmp_en(0x0);
    //RG_bc11_IPU_EN[1.0] = 00
    mt6325_upmu_set_rg_bc11_ipu_en(0x0);
    //RG_bc11_IPD_EN[1.0] = 00
    mt6325_upmu_set_rg_bc11_ipd_en(0x0);
    //bc11_RST=1
    mt6325_upmu_set_rg_bc11_rst(0x1);
    //bc11_BB_CTRL=1
    mt6325_upmu_set_rg_bc11_bb_ctrl(0x1);

    mdelay(50);
    
    if(g_bq24261_log_en>1)
    {
       print("hw_bc11_init() \r\n");
    }    

}
 
 
static U32 hw_bc11_DCD(void)
{
    U32 wChargerAvail = 0;

    //RG_bc11_IPU_EN[1.0] = 10
    mt6325_upmu_set_rg_bc11_ipu_en(0x2);    
    //RG_bc11_IPD_EN[1.0] = 01
    mt6325_upmu_set_rg_bc11_ipd_en(0x1);
    //RG_bc11_VREF_VTH = [1:0]=01
    mt6325_upmu_set_rg_bc11_vref_vth(0x1);    
    //RG_bc11_CMP_EN[1.0] = 10
    mt6325_upmu_set_rg_bc11_cmp_en(0x2);

    mdelay(80);

    wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();
    
    if(g_bq24261_log_en>1)
    {
        print("hw_bc11_DCD() \r\n");
    }
    
    //RG_bc11_IPU_EN[1.0] = 00
    mt6325_upmu_set_rg_bc11_ipu_en(0x0);
    //RG_bc11_IPD_EN[1.0] = 00
    mt6325_upmu_set_rg_bc11_ipd_en(0x0);
    //RG_bc11_CMP_EN[1.0] = 00
    mt6325_upmu_set_rg_bc11_cmp_en(0x0);
    //RG_bc11_VREF_VTH = [1:0]=00
    mt6325_upmu_set_rg_bc11_vref_vth(0x0);

    return wChargerAvail;
}
 
 
static U32 hw_bc11_stepA1(void)
{
   U32 wChargerAvail = 0;
     
   //RG_bc11_IPD_EN[1.0] = 01
   mt6325_upmu_set_rg_bc11_ipd_en(0x1);   
   //RG_bc11_VREF_VTH = [1:0]=00
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);   
   //RG_bc11_CMP_EN[1.0] = 01
   mt6325_upmu_set_rg_bc11_cmp_en(0x1);

   mdelay(80);

   wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();

   if(g_bq24261_log_en>1)
   {
       print("hw_bc11_stepA1() \r\n");
   }

   //RG_bc11_IPD_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_ipd_en(0x0);
   //RG_bc11_CMP_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);

   return  wChargerAvail;
}
 
 
static U32 hw_bc11_stepA2(void)
{
   U32 wChargerAvail = 0;
     
   //RG_bc11_VSRC_EN[1.0] = 10 
   mt6325_upmu_set_rg_bc11_vsrc_en(0x2);   
   //RG_bc11_IPD_EN[1:0] = 01
   mt6325_upmu_set_rg_bc11_ipd_en(0x1);   
   //RG_bc11_VREF_VTH = [1:0]=00
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);   
   //RG_bc11_CMP_EN[1.0] = 01
   mt6325_upmu_set_rg_bc11_cmp_en(0x1);

   mdelay(80);

   wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();

   if(g_bq24261_log_en>1)
   {
       print("hw_bc11_stepA2() \r\n");
   }

   //RG_bc11_VSRC_EN[1:0]=00
   mt6325_upmu_set_rg_bc11_vsrc_en(0x0);
   //RG_bc11_IPD_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_ipd_en(0x0);
   //RG_bc11_CMP_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);

   return  wChargerAvail;
}
 
 
static U32 hw_bc11_stepB2(void)
{
   U32 wChargerAvail = 0;

   //RG_bc11_IPU_EN[1:0]=10
   mt6325_upmu_set_rg_bc11_ipu_en(0x2);   
   //RG_bc11_VREF_VTH = [1:0]=01
   mt6325_upmu_set_rg_bc11_vref_vth(0x1);   
   //RG_bc11_CMP_EN[1.0] = 01
   mt6325_upmu_set_rg_bc11_cmp_en(0x1);

   mdelay(80);

   wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();

   if(g_bq24261_log_en>1)
   {
       print("hw_bc11_stepB2() \r\n");
   }

   //RG_bc11_IPU_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_ipu_en(0x0);
   //RG_bc11_CMP_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);
   //RG_bc11_VREF_VTH = [1:0]=00
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);

   return  wChargerAvail;
}
 
 
static void hw_bc11_done(void)
{
   //RG_bc11_VSRC_EN[1:0]=00
   mt6325_upmu_set_rg_bc11_vsrc_en(0x0);
   //RG_bc11_VREF_VTH = [1:0]=0
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);
   //RG_bc11_CMP_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);
   //RG_bc11_IPU_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_ipu_en(0x0);
   //RG_bc11_IPD_EN[1.0] = 00
   mt6325_upmu_set_rg_bc11_ipd_en(0x0);
   //RG_bc11_BIAS_EN=0
   mt6325_upmu_set_rg_bc11_bias_en(0x0); 

   Charger_Detect_Release();

   if(g_bq24261_log_en>1)
   {
       print("hw_bc11_done() \r\n");
   }
   
}

int hw_charging_get_charger_type(void)
{
    if(CHARGER_UNKNOWN != g_ret)
        return g_ret;
    
    /********* Step initial  ***************/         
    hw_bc11_init();
 
    /********* Step DCD ***************/  
    if(1 == hw_bc11_DCD())
    {
         /********* Step A1 ***************/
         if(1 == hw_bc11_stepA1())
         {
             g_ret = APPLE_2_1A_CHARGER;
             print("step A1 : Apple 2.1A CHARGER!\r\n");
         }
         else
         {
             g_ret = NONSTANDARD_CHARGER;
             print("step A1 : Non STANDARD CHARGER!\r\n");
         }
    }
    else
    {
         /********* Step A2 ***************/
         if(1 == hw_bc11_stepA2())
         {
             /********* Step B2 ***************/
             if(1 == hw_bc11_stepB2())
             {
                 g_ret = STANDARD_CHARGER;
                 print("step B2 : STANDARD CHARGER!\r\n");
             }
             else
             {
                 g_ret = CHARGING_HOST;
                 print("step B2 :  Charging Host!\r\n");
             }
         }
         else
         {
             g_ret = STANDARD_HOST;
             print("step A2 : Standard USB Host!\r\n");
         }
 
    }
 
    /********* Finally setting *******************************/
    hw_bc11_done();

    return g_ret;
}
#endif

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE charger_type;
    
    charger_type = hw_charging_get_charger_type();
    
    return charger_type;
}

CHARGER_TYPE mt_charger_type_detection(void)
{
    if( g_first_check == 0 )
    {
        g_first_check = 1;
        g_ret = hw_charger_type_detection();
    }
    else
    {
        printf("[mt_charger_type_detection] Got data !!, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
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

    return val;
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

    if (val==1){     
        printf("pl pmic powerkey Release\n");
        return 0;
    }else{
        printf("pl pmic powerkey Press\n");
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
	                                                    
    if (val==1){     
        printf("pl pmic FCHRKEY Release\n");
        return 0;
    }else{
        printf("pl pmic FCHRKEY Press\n");
        return 1;
    }
}

U32 pmic_IsUsbCableIn (void) 
{    
    U32 ret=0;
    U32 val=0;

#if 0//CFG_EVB_PLATFORM
    val = 1; // for bring up
    //printf("[pmic_IsUsbCableIn] have CFG_EVB_PLATFORM, %d\n", val);
#else
    ret=pmic_read_interface( (U32)(MT6325_CHR_CON0),
                           (&val),
                           (U32)(MT6325_PMIC_RGS_CHRDET_MASK),
                           (U32)(MT6325_PMIC_RGS_CHRDET_SHIFT)
	                       );
    printf("[pmic_IsUsbCableIn] %d\n", val);
#endif    

    if(val)
        return PMIC_CHRDET_EXIST;
    else
        return PMIC_CHRDET_NOT_EXIST;
}    

static int vbat_status = PMIC_VBAT_NOT_DROP;
static void pmic_DetectVbatDrop (void) 
{    
	U32 ret=0;
	U32 just_rst=0;

	pmic_read_interface( MT6325_STRUP_CON9, (&just_rst), MT6325_PMIC_JUST_PWRKEY_RST_MASK, MT6325_PMIC_JUST_PWRKEY_RST_SHIFT );
	pmic_config_interface(MT6325_STRUP_CON9, 1, MT6325_PMIC_CLR_JUST_RST_MASK, MT6325_PMIC_CLR_JUST_RST_SHIFT);

	printf("just_rst = %d\n", just_rst);
	if(just_rst)
		vbat_status = PMIC_VBAT_DROP;
	else
		vbat_status = PMIC_VBAT_NOT_DROP;
}

int pmic_IsVbatDrop(void)
{
   return vbat_status;	
}

void hw_set_cc(int cc_val)
{
    U32 ret=0;
    U32 val=0;    
    U32 hw_charger_ov_flag=0;

    printf("hw_set_cc: %d\r\n", cc_val);
    
    #if defined(MTK_PUMP_EXPRESS_PLUS_SUPPORT)
    //mt6325_upmu_set_rg_vcdt_hv_vth, 10.5V
    ret=pmic_config_interface(MT6325_CHR_CON1,0xF,MT6325_PMIC_RG_VCDT_HV_VTH_MASK,MT6325_PMIC_RG_VCDT_HV_VTH_SHIFT);
    #else
    //mt6325_upmu_set_rg_vcdt_hv_vth, 7V
    ret=pmic_config_interface(MT6325_CHR_CON1,0xB,MT6325_PMIC_RG_VCDT_HV_VTH_MASK,MT6325_PMIC_RG_VCDT_HV_VTH_SHIFT);
    #endif
    //mt6325_upmu_set_rg_vcdt_hv_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON0,0x1,MT6325_PMIC_RG_VCDT_HV_EN_MASK,MT6325_PMIC_RG_VCDT_HV_EN_SHIFT);
    //mt6325_upmu_set_rg_cs_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON2,0x1,MT6325_PMIC_RG_CS_EN_MASK,MT6325_PMIC_RG_CS_EN_SHIFT);
    //mt6325_upmu_set_rg_csdac_mode(1);
    ret=pmic_config_interface(MT6325_CHR_CON25,0x1,MT6325_PMIC_RG_CSDAC_MODE_MASK,MT6325_PMIC_RG_CSDAC_MODE_SHIFT);

    //mt6325_upmu_get_rgs_vcdt_hv_det, if yes:turn off charging and return
    ret=pmic_read_interface(MT6325_CHR_CON0,(&hw_charger_ov_flag),MT6325_PMIC_RGS_VCDT_HV_DET_MASK,MT6325_PMIC_RGS_VCDT_HV_DET_SHIFT);
    if(hw_charger_ov_flag == 1)
    {
        ret=pmic_config_interface(MT6325_CHR_CON0,0x0,MT6325_PMIC_RG_CHR_EN_MASK,MT6325_PMIC_RG_CHR_EN_SHIFT);
        printf("pl chargerov turn off charging \n"); 
        return;
    }

    //mt6325_upmu_set_rg_cs_vth, table
    switch(cc_val){        
        case 900:  ret=pmic_config_interface(MT6325_CHR_CON4,0x07,MT6325_PMIC_RG_CS_VTH_MASK,MT6325_PMIC_RG_CS_VTH_SHIFT);  break;
        case 650:  ret=pmic_config_interface(MT6325_CHR_CON4,0x0A,MT6325_PMIC_RG_CS_VTH_MASK,MT6325_PMIC_RG_CS_VTH_SHIFT);  break;
        case 450:  ret=pmic_config_interface(MT6325_CHR_CON4,0x0C,MT6325_PMIC_RG_CS_VTH_MASK,MT6325_PMIC_RG_CS_VTH_SHIFT);  break;
        case 70:   ret=pmic_config_interface(MT6325_CHR_CON4,0x0F,MT6325_PMIC_RG_CS_VTH_MASK,MT6325_PMIC_RG_CS_VTH_SHIFT);  break;          
        default:
            dbg_print("hw_set_cc: argument invalid!!\r\n");
            break;
    }

    //mt6325_upmu_set_rg_csdac_dly(0x4);
    ret=pmic_config_interface(MT6325_CHR_CON23,0x4,MT6325_PMIC_RG_CSDAC_DLY_MASK,MT6325_PMIC_RG_CSDAC_DLY_SHIFT);
    //mt6325_upmu_set_rg_csdac_stp(0x1);
    ret=pmic_config_interface(MT6325_CHR_CON23,0x1,MT6325_PMIC_RG_CSDAC_STP_MASK,MT6325_PMIC_RG_CSDAC_STP_SHIFT);
    //mt6325_upmu_set_rg_csdac_stp_inc(0x1);
    ret=pmic_config_interface(MT6325_CHR_CON22,0x1,MT6325_PMIC_RG_CSDAC_STP_INC_MASK,MT6325_PMIC_RG_CSDAC_STP_INC_SHIFT);
    //mt6325_upmu_set_rg_csdac_stp_dec(0x2);
    ret=pmic_config_interface(MT6325_CHR_CON22,0x2,MT6325_PMIC_RG_CSDAC_STP_DEC_MASK,MT6325_PMIC_RG_CSDAC_STP_DEC_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_td(0x0);
    ret=pmic_config_interface(MT6325_CHR_CON13,0x0,MT6325_PMIC_RG_CHRWDT_TD_MASK,MT6325_PMIC_RG_CHRWDT_TD_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_wr(1);
    ret=pmic_config_interface(MT6325_CHR_CON13,0x1,MT6325_PMIC_RG_CHRWDT_WR_MASK,MT6325_PMIC_RG_CHRWDT_WR_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_int_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON15,0x1,MT6325_PMIC_RG_CHRWDT_INT_EN_MASK,MT6325_PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON13,0x1,MT6325_PMIC_RG_CHRWDT_EN_MASK,MT6325_PMIC_RG_CHRWDT_EN_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_flag_wr(1);
    ret=pmic_config_interface(MT6325_CHR_CON15,0x1,MT6325_PMIC_RG_CHRWDT_FLAG_WR_MASK,MT6325_PMIC_RG_CHRWDT_FLAG_WR_SHIFT);
    //mt6325_upmu_set_rg_csdac_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON0,0x1,MT6325_PMIC_RG_CSDAC_EN_MASK,MT6325_PMIC_RG_CSDAC_EN_SHIFT);
    //mt6325_upmu_set_rg_hwcv_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON25,0x1,MT6325_PMIC_RG_HWCV_EN_MASK,MT6325_PMIC_RG_HWCV_EN_SHIFT);
    //mt6325_upmu_set_rg_chr_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON0,0x1,MT6325_PMIC_RG_CHR_EN_MASK,MT6325_PMIC_RG_CHR_EN_SHIFT);
}

void mt6325_upmu_set_baton_tdet_en(U32 val)
{
    U32 ret=0;  
    ret=pmic_config_interface( (U32)(MT6325_CHR_CON7),
                             (U32)(val),
                             (U32)(MT6325_PMIC_BATON_TDET_EN_MASK),
                             (U32)(MT6325_PMIC_BATON_TDET_EN_SHIFT)
	                         );  
}

void mt6325_upmu_set_rg_baton_en(U32 val)
{
    U32 ret=0;
    ret=pmic_config_interface( (U32)(MT6325_CHR_CON7),
                             (U32)(val),
                             (U32)(MT6325_PMIC_RG_BATON_EN_MASK),
                             (U32)(MT6325_PMIC_RG_BATON_EN_SHIFT)
	                         );  
}

U32 mt6325_upmu_get_rgs_baton_undet(void)
{
    U32 ret=0;
    U32 val=0;  
    ret=pmic_read_interface( (U32)(MT6325_CHR_CON7),
                           (&val),
                           (U32)(MT6325_PMIC_RGS_BATON_UNDET_MASK),
                           (U32)(MT6325_PMIC_RGS_BATON_UNDET_SHIFT)
	                       );
   return val;
}

int hw_check_battery(void)
{
    #ifdef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
        printf("ignore bat check !\n");
        return 1;
    #else
        #if CFG_EVB_PLATFORM
            printf("ignore bat check\n");
            return 1;
        #else
            U32 val=0;

            pmic_config_interface(0x0a08,0x010b,0xFFFF,0);
			
            mt6325_upmu_set_baton_tdet_en(1);    
            mt6325_upmu_set_rg_baton_en(1);

            mdelay(1);
            
            val = mt6325_upmu_get_rgs_baton_undet();

            if(val==0)
            {
                printf("bat is exist\n");
                return 1;
            }
            else
            {
                printf("bat NOT exist\n");
                return 0;
            }
        #endif
    #endif
}

void pl_hw_ulc_det(void)
{
    U32 ret=0;
    U32 val=0;
    
    //mt6325_upmu_set_rg_ulc_det_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON25,0x1,MT6325_PMIC_RG_ULC_DET_EN_MASK,MT6325_PMIC_RG_ULC_DET_EN_SHIFT);
    //mt6325_upmu_set_rg_low_ich_db(1);
    ret=pmic_config_interface(MT6325_CHR_CON24,0x1,MT6325_PMIC_RG_LOW_ICH_DB_MASK,MT6325_PMIC_RG_LOW_ICH_DB_SHIFT);
}

void pl_charging(int en_chr)
{
    U32 ret=0;
    U32 val=0;
    U32 i=0;

    pl_hw_ulc_det();
    
    if(en_chr == 1)
    {
        printf("pl charging en\n");
    
        hw_set_cc(450);

        //mt6325_upmu_set_rg_usbdl_set, USBDL set 1
        ret=pmic_config_interface(MT6325_CHR_CON16,0x1,MT6325_PMIC_RG_USBDL_SET_MASK,MT6325_PMIC_RG_USBDL_SET_SHIFT);
    }
    else
    {
        printf("pl charging dis\n");
    
        //mt6325_upmu_set_rg_usbdl_set, USBDL set 0
        ret=pmic_config_interface(MT6325_CHR_CON16,0x0,MT6325_PMIC_RG_USBDL_SET_MASK,MT6325_PMIC_RG_USBDL_SET_SHIFT);

        //mt6325_upmu_set_rg_hwcv_en(0);
        ret=pmic_config_interface(MT6325_CHR_CON25,0x0,MT6325_PMIC_RG_HWCV_EN_MASK,MT6325_PMIC_RG_HWCV_EN_SHIFT);
        //mt6325_upmu_set_rg_chr_en(0);
        ret=pmic_config_interface(MT6325_CHR_CON0,0x0,MT6325_PMIC_RG_CHR_EN_MASK,MT6325_PMIC_RG_CHR_EN_SHIFT);
    }

    for(i=MT6325_CHR_CON0 ; i<=MT6325_CHR_CON40 ; i+=2)
    {
        ret=pmic_read_interface(i,&val,0xFFFF,0x0);        
        print("[0x%x]=0x%x\n", i, val);
    }

    printf("pl charging done\n");
}

void pl_kick_chr_wdt(void)
{
    U32 ret=0;
    U32 val=0;

    //mt6325_upmu_set_rg_chrwdt_td(0x0);
    ret=pmic_config_interface(MT6325_CHR_CON13,0x0,MT6325_PMIC_RG_CHRWDT_TD_MASK,MT6325_PMIC_RG_CHRWDT_TD_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_wr(1);
    ret=pmic_config_interface(MT6325_CHR_CON13,0x1,MT6325_PMIC_RG_CHRWDT_WR_MASK,MT6325_PMIC_RG_CHRWDT_WR_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_int_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON15,0x1,MT6325_PMIC_RG_CHRWDT_INT_EN_MASK,MT6325_PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_en(1);
    ret=pmic_config_interface(MT6325_CHR_CON13,0x1,MT6325_PMIC_RG_CHRWDT_EN_MASK,MT6325_PMIC_RG_CHRWDT_EN_SHIFT);
    //mt6325_upmu_set_rg_chrwdt_flag_wr(1);
    ret=pmic_config_interface(MT6325_CHR_CON15,0x1,MT6325_PMIC_RG_CHRWDT_FLAG_WR_MASK,MT6325_PMIC_RG_CHRWDT_FLAG_WR_SHIFT);
}

void pl_close_pre_chr_led(void)
{
    //no charger feature
}

U32 upmu_get_reg_value(kal_uint32 reg)
{
    U32 ret=0;
    U32 reg_val=0;
    
    ret=pmic_read_interface(reg, &reg_val, 0xFFFF, 0x0);
    
    return reg_val;
}

void pmic_buck_unlimit(void)
{
    U32 reg_val=0;
    U32 i=0;
    U32 from_i=0xC1E;
    U32 to_i  =0xC5E;

    print("[pmic_buck_unlimit] Start\n");

    print("[pmic_buck_unlimit] before\n");
    print("[0x%x]=0x%x\n", 0xC14, upmu_get_reg_value(0xC14));
    for(i=from_i ; i<=0xC9C ; i+=0x2) { print("[0x%x]=0x%x\n", i, upmu_get_reg_value(i)); }

    //------------------------------------------
    for(i=from_i ; i<=0xC5C ; i+=0x2)
    {
        pmic_read_interface(i,&reg_val,0xFFFF,0);        
        pmic_config_interface(to_i,reg_val,0xFFFF,0);
        to_i+=0x2;
    }
    pmic_config_interface(0xC84,0x1,0x1,10);
    pmic_config_interface(0xC14,0x1,0xFFFF,0);
    //------------------------------------------

    print("[pmic_buck_unlimit] after\n");
    print("[0x%x]=0x%x\n", 0xC14, upmu_get_reg_value(0xC14));
    for(i=from_i ; i<=0xC9C ; i+=0x2) { print("[0x%x]=0x%x\n", i, upmu_get_reg_value(i)); }
    
    print("[pmic_buck_unlimit] Done\n");
}

//==============================================================================
// PMIC 6325 EFUSE
//==============================================================================

void get_pmic_6325_efuse_data(U32 *efuse_data)
{
    U32 ret=0;
    U32 reg_val=0;        
    int i=0;
    
    //1. enable efuse ctrl engine clock
    ret=pmic_config_interface(0x026C, 0x0040, 0xFFFF, 0);
    ret=pmic_config_interface(0x024E, 0x0004, 0xFFFF, 0);

    //2.
    ret=pmic_config_interface(0x0C16, 0x1, 0x1, 0);

    //dump
    //print("[0x%x]=0x%x,[0x%x]=0x%x,[0x%x]=0x%x\n", 
    //    0x026C,upmu_get_reg_value(0x026C),
    //    0x024C,upmu_get_reg_value(0x024C),
    //    0x0C16,upmu_get_reg_value(0x0C16)
    //    );

    for(i=0;i<=0x1F;i++)
    {
        //3. set row to read
        ret=pmic_config_interface(0x0C00, i, 0x1F, 1);

        //4. Toggle
        ret=pmic_read_interface(0xC10, &reg_val, 0x1, 0);
        if(reg_val==0)
            ret=pmic_config_interface(0xC10, 1, 0x1, 0);
        else
            ret=pmic_config_interface(0xC10, 0, 0x1, 0);

        reg_val=1;    
        while(reg_val == 1)
        {
            ret=pmic_read_interface(0xC1A, &reg_val, 0x1, 0);
            //print("5. polling Reg[0x61A][0]=0x%x\n", reg_val);
        }

        udelay(1000);

        //6. read data
        efuse_data[i] = upmu_get_reg_value(0x0C18);
        
        //print("i=%d,Reg[0x%x]=0x%x,Reg[0x%x]=0x%x,Reg[0x%x]=0x%x\n", 
        //    i,
        //    0x0C00,upmu_get_reg_value(0x0C00),
        //    0x0C18,upmu_get_reg_value(0x0C18),
        //    0x0C1C,upmu_get_reg_value(0x0C1C)
        //    );
    }

    //7. Disable efuse ctrl engine clock
    ret=pmic_config_interface(0x024C, 0x0004, 0xFFFF, 0);
    ret=pmic_config_interface(0x026A, 0x0040, 0xFFFF, 0);
}

void pmic_6325_efuse_check(void)
{
    print("[0x%x]=0x%x\n", 0x0458, upmu_get_reg_value(0x0458));
    print("[0x%x]=0x%x\n", 0x045A, upmu_get_reg_value(0x045A));
    print("[0x%x]=0x%x\n", 0x047E, upmu_get_reg_value(0x047E));
    print("[0x%x]=0x%x\n", 0x049A, upmu_get_reg_value(0x049A));
    print("[0x%x]=0x%x\n", 0x045C, upmu_get_reg_value(0x045C));
    print("[0x%x]=0x%x\n", 0x045E, upmu_get_reg_value(0x045E));
    print("[0x%x]=0x%x\n", 0x0460, upmu_get_reg_value(0x0460));
    print("[0x%x]=0x%x\n", 0x046A, upmu_get_reg_value(0x046A));
    print("[0x%x]=0x%x\n", 0x046C, upmu_get_reg_value(0x046C));    
    print("[0x%x]=0x%x\n", 0x046E, upmu_get_reg_value(0x046E));
    print("[0x%x]=0x%x\n", 0x0476, upmu_get_reg_value(0x0476));
    print("[0x%x]=0x%x\n", 0x0448, upmu_get_reg_value(0x0448));
    print("[0x%x]=0x%x\n", 0x0488, upmu_get_reg_value(0x0488));
    print("[0x%x]=0x%x\n", 0x0492, upmu_get_reg_value(0x0492));
    print("[0x%x]=0x%x\n", 0x043E, upmu_get_reg_value(0x043E));
    print("[0x%x]=0x%x\n", 0x0470, upmu_get_reg_value(0x0470));
    print("[0x%x]=0x%x\n", 0x0A3E, upmu_get_reg_value(0x0A3E));
    print("[0x%x]=0x%x\n", 0x0A40, upmu_get_reg_value(0x0A40));
    print("[0x%x]=0x%x\n", 0x0A42, upmu_get_reg_value(0x0A42));
    print("[0x%x]=0x%x\n", 0x0A46, upmu_get_reg_value(0x0A46));
    print("[0x%x]=0x%x\n", 0x0A44, upmu_get_reg_value(0x0A44));
    print("[0x%x]=0x%x\n", 0x0A4A, upmu_get_reg_value(0x0A4A));
    print("[0x%x]=0x%x\n", 0x0A48, upmu_get_reg_value(0x0A48));
    print("[0x%x]=0x%x\n", 0x0A4C, upmu_get_reg_value(0x0A4C));
    print("[0x%x]=0x%x\n", 0x0A4E, upmu_get_reg_value(0x0A4E));
    print("[0x%x]=0x%x\n", 0x0A50, upmu_get_reg_value(0x0A50));
    print("[0x%x]=0x%x\n", 0x0A5A, upmu_get_reg_value(0x0A5A));
    print("[0x%x]=0x%x\n", 0x0A56, upmu_get_reg_value(0x0A56));
    print("[0x%x]=0x%x\n", 0x045C, upmu_get_reg_value(0x045C));    
    print("[0x%x]=0x%x\n", 0x0CB8, upmu_get_reg_value(0x0CB8));
    print("[0x%x]=0x%x\n", 0x0E84, upmu_get_reg_value(0x0E84));
    print("[0x%x]=0x%x\n", 0x0E86, upmu_get_reg_value(0x0E86));
}

void pmic_6325_efuse_check_bit(void)
{
    U32 val_reg=0;

    print("pmic_6325_efuse_check_bit\n");
    
        pmic_read_interface(0x0458,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,8 ); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0458,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x045A,&val_reg,0x1,0 ); print("%d\n", val_reg);
        pmic_read_interface(0x045A,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x045A,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x045A,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x045A,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x047E,&val_reg,0x1,0 ); print("%d\n", val_reg);
        pmic_read_interface(0x047E,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x047E,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x047E,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x047E,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x047E,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x049A,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x049A,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x049A,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x049A,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x049A,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x049A,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x045C,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x045C,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x045C,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,0 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,8 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x045E,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0460,&val_reg,0x1,0 ); print("%d\n", val_reg);
        pmic_read_interface(0x0460,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x0460,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x046A,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x046A,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x046A,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x046A,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x046A,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x046A,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,0 ); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x046E,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x046E,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x046E,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x046C,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x046E,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x046E,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x046E,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0476,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0448,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0488,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,0 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,8 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0492,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x043E,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0470,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0470,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0470,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0470,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A3E,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A3E,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A3E,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A3E,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A40,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A40,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A40,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A40,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A42,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A42,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A42,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A42,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0A44,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A44,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A44,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A44,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A46,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4A,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A4A,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A4A,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A4A,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A48,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0A4C,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4C,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4C,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4C,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A50,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A5A,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A5A,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A5A,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A5A,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A4E,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0A56,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x045C,&val_reg,0x1,8 ); print("%d\n", val_reg);
        pmic_read_interface(0x045C,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x045C,&val_reg,0x1,10); print("%d\n", val_reg);
                                                     print("//--------------------\n");
        pmic_read_interface(0x0CB8,&val_reg,0x1,10); print("%d\n", val_reg);   
        pmic_read_interface(0x0CB8,&val_reg,0x1,11); print("%d\n", val_reg);   
        pmic_read_interface(0x0CB8,&val_reg,0x1,12); print("%d\n", val_reg);   
        pmic_read_interface(0x0CB8,&val_reg,0x1,13); print("%d\n", val_reg);   
        pmic_read_interface(0x0CB8,&val_reg,0x1,14); print("%d\n", val_reg);   
        pmic_read_interface(0x0CB8,&val_reg,0x1,15); print("%d\n", val_reg);   
                                                     print("//--------------------\n");          
        pmic_read_interface(0x0E84,&val_reg,0x1,0 ); print("%d\n", val_reg);            
        pmic_read_interface(0x0E84,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,8 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0E84,&val_reg,0x1,15); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,0 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,1 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,2 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,3 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,4 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,5 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,6 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,7 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,8 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,9 ); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,10); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,11); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,12); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,13); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,14); print("%d\n", val_reg);
        pmic_read_interface(0x0E86,&val_reg,0x1,15); print("%d\n", val_reg);
}

int g_ebit_990=0;
int g_ebit_991=0;

void pmic_6325_efuse_management(void)
{
    U32 efuse_data[0x20]={0};
    int i=0;
    int is_efuse_trimed=0;

    is_efuse_trimed = ((upmu_get_reg_value(0xC5C))>>15)&0x0001;

    print("[6325] is_efuse_trimed=0x%x,[0x%x]=0x%x\n", is_efuse_trimed, 0xC5C, upmu_get_reg_value(0xC5C));

    if(is_efuse_trimed == 1)
    {
        //get efuse data
        get_pmic_6325_efuse_data(efuse_data);
        
        //dump efuse data for check
        for(i=0x0;i<=0x1F;i++)
            print("[6325] efuse_data[0x%x]=0x%x\n", i, efuse_data[i]);

        //------------------------------------------
        g_ebit_990 = ((efuse_data[0x1D] >>14)&0x0001);
        g_ebit_991 = ((efuse_data[0x1D] >>15)&0x0001);
        print("[6325] g_ebit_990=%d, g_ebit_991=%d\n", g_ebit_990, g_ebit_991);
        //------------------------------------------
    
        print("Before apply pmic efuse\n");
        pmic_6325_efuse_check();

        //------------------------------------------
        pmic_config_interface(0x0458,((efuse_data[0x0] >>0 )&0x0001),0x1,4 );
        pmic_config_interface(0x0458,((efuse_data[0x0] >>1 )&0x0001),0x1,5 );
        pmic_config_interface(0x0458,((efuse_data[0x0] >>2 )&0x0001),0x1,6 );
        pmic_config_interface(0x0458,((efuse_data[0x0] >>3 )&0x0001),0x1,7 );
        pmic_config_interface(0x0458,((efuse_data[0x0] >>4 )&0x0001),0x1,8 );
        pmic_config_interface(0x0458,((efuse_data[0x0] >>5 )&0x0001),0x1,9 );
        pmic_config_interface(0x0458,((efuse_data[0x0] >>6 )&0x0001),0x1,10);
        pmic_config_interface(0x0458,((efuse_data[0x0] >>7 )&0x0001),0x1,11);
        pmic_config_interface(0x0458,((efuse_data[0x0] >>8 )&0x0001),0x1,12);
        pmic_config_interface(0x0458,((efuse_data[0x0] >>9 )&0x0001),0x1,13);
        pmic_config_interface(0x045A,((efuse_data[0x0] >>10)&0x0001),0x1,0 );
        pmic_config_interface(0x045A,((efuse_data[0x0] >>11)&0x0001),0x1,1 );
        pmic_config_interface(0x045A,((efuse_data[0x0] >>12)&0x0001),0x1,2 );
        pmic_config_interface(0x045A,((efuse_data[0x0] >>13)&0x0001),0x1,3 );
        pmic_config_interface(0x045A,((efuse_data[0x0] >>14)&0x0001),0x1,4 );
        pmic_config_interface(0x047E,((efuse_data[0x0] >>15)&0x0001),0x1,0 );
        pmic_config_interface(0x047E,((efuse_data[0x1] >>0 )&0x0001),0x1,1 );
        pmic_config_interface(0x047E,((efuse_data[0x1] >>1 )&0x0001),0x1,2 );
        pmic_config_interface(0x047E,((efuse_data[0x1] >>2 )&0x0001),0x1,3 );
        pmic_config_interface(0x047E,((efuse_data[0x1] >>3 )&0x0001),0x1,4 );
        pmic_config_interface(0x047E,((efuse_data[0x1] >>4 )&0x0001),0x1,5 );
        pmic_config_interface(0x049A,((efuse_data[0x1] >>5 )&0x0001),0x1,9 );
        pmic_config_interface(0x049A,((efuse_data[0x1] >>6 )&0x0001),0x1,10);
        pmic_config_interface(0x049A,((efuse_data[0x1] >>7 )&0x0001),0x1,11);
        pmic_config_interface(0x049A,((efuse_data[0x1] >>8 )&0x0001),0x1,12);
        pmic_config_interface(0x049A,((efuse_data[0x1] >>9 )&0x0001),0x1,13);
        pmic_config_interface(0x049A,((efuse_data[0x1] >>10)&0x0001),0x1,14);
        pmic_config_interface(0x045C,((efuse_data[0x1] >>11)&0x0001),0x1,11);
        pmic_config_interface(0x045C,((efuse_data[0x1] >>12)&0x0001),0x1,12);
        pmic_config_interface(0x045C,((efuse_data[0x1] >>13)&0x0001),0x1,13);
        pmic_config_interface(0x045E,((efuse_data[0x1] >>14)&0x0001),0x1,0 );
        pmic_config_interface(0x045E,((efuse_data[0x1] >>15)&0x0001),0x1,1 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>0 )&0x0001),0x1,2 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>1 )&0x0001),0x1,3 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>2 )&0x0001),0x1,4 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>3 )&0x0001),0x1,5 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>4 )&0x0001),0x1,6 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>5 )&0x0001),0x1,7 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>6 )&0x0001),0x1,8 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>7 )&0x0001),0x1,9 );
        pmic_config_interface(0x045E,((efuse_data[0x2] >>8 )&0x0001),0x1,10);
        pmic_config_interface(0x045E,((efuse_data[0x2] >>9 )&0x0001),0x1,11);
        pmic_config_interface(0x0460,((efuse_data[0x2] >>10)&0x0001),0x1,0 );
        pmic_config_interface(0x0460,((efuse_data[0x2] >>11)&0x0001),0x1,1 );
        pmic_config_interface(0x0460,((efuse_data[0x2] >>12)&0x0001),0x1,2 );
        pmic_config_interface(0x046A,((efuse_data[0x2] >>13)&0x0001),0x1,10);
        pmic_config_interface(0x046A,((efuse_data[0x2] >>14)&0x0001),0x1,11);
        pmic_config_interface(0x046A,((efuse_data[0x2] >>15)&0x0001),0x1,12);
        pmic_config_interface(0x046C,((efuse_data[0x3] >>0 )&0x0001),0x1,10);
        pmic_config_interface(0x046C,((efuse_data[0x3] >>1 )&0x0001),0x1,12);
        pmic_config_interface(0x046C,((efuse_data[0x3] >>2 )&0x0001),0x1,14);
        pmic_config_interface(0x046A,((efuse_data[0x3] >>3 )&0x0001),0x1,13);
        pmic_config_interface(0x046A,((efuse_data[0x3] >>4 )&0x0001),0x1,14);
        pmic_config_interface(0x046A,((efuse_data[0x3] >>5 )&0x0001),0x1,15);
        pmic_config_interface(0x046C,((efuse_data[0x3] >>6 )&0x0001),0x1,11);
        pmic_config_interface(0x046C,((efuse_data[0x3] >>7 )&0x0001),0x1,13);
        pmic_config_interface(0x046C,((efuse_data[0x3] >>8 )&0x0001),0x1,15);
        pmic_config_interface(0x046C,((efuse_data[0x3] >>9 )&0x0001),0x1,0 );
        pmic_config_interface(0x046C,((efuse_data[0x3] >>10)&0x0001),0x1,1 );
        pmic_config_interface(0x046C,((efuse_data[0x3] >>11)&0x0001),0x1,2 );
        pmic_config_interface(0x046E,((efuse_data[0x3] >>12)&0x0001),0x1,2 );
        pmic_config_interface(0x046E,((efuse_data[0x3] >>13)&0x0001),0x1,4 );
        pmic_config_interface(0x046E,((efuse_data[0x3] >>14)&0x0001),0x1,6 );
        pmic_config_interface(0x046C,((efuse_data[0x3] >>15)&0x0001),0x1,3 );
        pmic_config_interface(0x046C,((efuse_data[0x4] >>0 )&0x0001),0x1,4 );
        pmic_config_interface(0x046C,((efuse_data[0x4] >>1 )&0x0001),0x1,5 );
        pmic_config_interface(0x046E,((efuse_data[0x4] >>2 )&0x0001),0x1,3 );
        pmic_config_interface(0x046E,((efuse_data[0x4] >>3 )&0x0001),0x1,5 );
        pmic_config_interface(0x046E,((efuse_data[0x4] >>4 )&0x0001),0x1,7 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>5 )&0x0001),0x1,1 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>6 )&0x0001),0x1,2 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>7 )&0x0001),0x1,3 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>8 )&0x0001),0x1,9 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>9 )&0x0001),0x1,11);
        pmic_config_interface(0x0476,((efuse_data[0x4] >>10)&0x0001),0x1,13);
        pmic_config_interface(0x0476,((efuse_data[0x4] >>11)&0x0001),0x1,4 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>12)&0x0001),0x1,5 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>13)&0x0001),0x1,6 );
        pmic_config_interface(0x0476,((efuse_data[0x4] >>14)&0x0001),0x1,10);
        pmic_config_interface(0x0476,((efuse_data[0x4] >>15)&0x0001),0x1,12);
        pmic_config_interface(0x0476,((efuse_data[0x5] >>0 )&0x0001),0x1,14);
        pmic_config_interface(0x0448,((efuse_data[0x5] >>1 )&0x0001),0x1,1 );
        pmic_config_interface(0x0448,((efuse_data[0x5] >>2 )&0x0001),0x1,2 );
        pmic_config_interface(0x0448,((efuse_data[0x5] >>3 )&0x0001),0x1,3 );
        pmic_config_interface(0x0448,((efuse_data[0x5] >>4 )&0x0001),0x1,9 );
        pmic_config_interface(0x0448,((efuse_data[0x5] >>5 )&0x0001),0x1,11);
        pmic_config_interface(0x0448,((efuse_data[0x5] >>6 )&0x0001),0x1,13);
        pmic_config_interface(0x0448,((efuse_data[0x5] >>7 )&0x0001),0x1,4 );
        pmic_config_interface(0x0448,((efuse_data[0x5] >>8 )&0x0001),0x1,5 );
        pmic_config_interface(0x0448,((efuse_data[0x5] >>9 )&0x0001),0x1,6 );
        pmic_config_interface(0x0448,((efuse_data[0x5] >>10)&0x0001),0x1,10);
        pmic_config_interface(0x0448,((efuse_data[0x5] >>11)&0x0001),0x1,12);
        pmic_config_interface(0x0448,((efuse_data[0x5] >>12)&0x0001),0x1,14);
        pmic_config_interface(0x0488,((efuse_data[0x5] >>13)&0x0001),0x1,1 );
        pmic_config_interface(0x0488,((efuse_data[0x5] >>14)&0x0001),0x1,2 );
        pmic_config_interface(0x0488,((efuse_data[0x5] >>15)&0x0001),0x1,3 );
        pmic_config_interface(0x0488,((efuse_data[0x6] >>0 )&0x0001),0x1,9 );
        pmic_config_interface(0x0488,((efuse_data[0x6] >>1 )&0x0001),0x1,11);
        pmic_config_interface(0x0488,((efuse_data[0x6] >>2 )&0x0001),0x1,13);
        pmic_config_interface(0x0488,((efuse_data[0x6] >>3 )&0x0001),0x1,4 );
        pmic_config_interface(0x0488,((efuse_data[0x6] >>4 )&0x0001),0x1,5 );
        pmic_config_interface(0x0488,((efuse_data[0x6] >>5 )&0x0001),0x1,6 );
        pmic_config_interface(0x0488,((efuse_data[0x6] >>6 )&0x0001),0x1,10);
        pmic_config_interface(0x0488,((efuse_data[0x6] >>7 )&0x0001),0x1,12);
        pmic_config_interface(0x0488,((efuse_data[0x6] >>8 )&0x0001),0x1,14);
        pmic_config_interface(0x0492,((efuse_data[0x6] >>9 )&0x0001),0x1,0 );
        pmic_config_interface(0x0492,((efuse_data[0x6] >>10)&0x0001),0x1,1 );
        pmic_config_interface(0x0492,((efuse_data[0x6] >>11)&0x0001),0x1,2 );
        pmic_config_interface(0x0492,((efuse_data[0x6] >>12)&0x0001),0x1,8 );
        pmic_config_interface(0x0492,((efuse_data[0x6] >>13)&0x0001),0x1,10);
        pmic_config_interface(0x0492,((efuse_data[0x6] >>14)&0x0001),0x1,12);
        pmic_config_interface(0x0492,((efuse_data[0x6] >>15)&0x0001),0x1,3 );
        pmic_config_interface(0x0492,((efuse_data[0x7] >>0 )&0x0001),0x1,4 );
        pmic_config_interface(0x0492,((efuse_data[0x7] >>1 )&0x0001),0x1,5 );
        pmic_config_interface(0x0492,((efuse_data[0x7] >>2 )&0x0001),0x1,9 );
        pmic_config_interface(0x0492,((efuse_data[0x7] >>3 )&0x0001),0x1,11);
        pmic_config_interface(0x0492,((efuse_data[0x7] >>4 )&0x0001),0x1,13);
        pmic_config_interface(0x043E,((efuse_data[0x7] >>5 )&0x0001),0x1,1 );
        pmic_config_interface(0x043E,((efuse_data[0x7] >>6 )&0x0001),0x1,2 );
        pmic_config_interface(0x043E,((efuse_data[0x7] >>7 )&0x0001),0x1,3 );
        pmic_config_interface(0x043E,((efuse_data[0x7] >>8 )&0x0001),0x1,9 );
        pmic_config_interface(0x043E,((efuse_data[0x7] >>9 )&0x0001),0x1,11);
        pmic_config_interface(0x043E,((efuse_data[0x7] >>10)&0x0001),0x1,13);
        pmic_config_interface(0x043E,((efuse_data[0x7] >>11)&0x0001),0x1,4 );
        pmic_config_interface(0x043E,((efuse_data[0x7] >>12)&0x0001),0x1,5 );
        pmic_config_interface(0x043E,((efuse_data[0x7] >>13)&0x0001),0x1,6 );
        pmic_config_interface(0x043E,((efuse_data[0x7] >>14)&0x0001),0x1,10);
        pmic_config_interface(0x043E,((efuse_data[0x7] >>15)&0x0001),0x1,12);
        pmic_config_interface(0x043E,((efuse_data[0x8] >>0 )&0x0001),0x1,14);
        pmic_config_interface(0x0470,((efuse_data[0x8] >>1 )&0x0001),0x1,10);
        pmic_config_interface(0x0470,((efuse_data[0x8] >>2 )&0x0001),0x1,11);
        pmic_config_interface(0x0470,((efuse_data[0x8] >>3 )&0x0001),0x1,12);
        pmic_config_interface(0x0470,((efuse_data[0x8] >>4 )&0x0001),0x1,13);
        pmic_config_interface(0x0A3E,((efuse_data[0x8] >>5 )&0x0001),0x1,4 );
        pmic_config_interface(0x0A3E,((efuse_data[0x8] >>6 )&0x0001),0x1,5 );
        pmic_config_interface(0x0A3E,((efuse_data[0x8] >>7 )&0x0001),0x1,6 );
        pmic_config_interface(0x0A3E,((efuse_data[0x8] >>8 )&0x0001),0x1,7 );
        pmic_config_interface(0x0A40,((efuse_data[0x8] >>9 )&0x0001),0x1,4 );
        pmic_config_interface(0x0A40,((efuse_data[0x8] >>10)&0x0001),0x1,5 );
        pmic_config_interface(0x0A40,((efuse_data[0x8] >>11)&0x0001),0x1,6 );
        pmic_config_interface(0x0A40,((efuse_data[0x8] >>12)&0x0001),0x1,7 );
        pmic_config_interface(0x0A42,((efuse_data[0x8] >>13)&0x0001),0x1,4 );
        pmic_config_interface(0x0A42,((efuse_data[0x8] >>14)&0x0001),0x1,5 );
        pmic_config_interface(0x0A42,((efuse_data[0x8] >>15)&0x0001),0x1,6 );
        pmic_config_interface(0x0A42,((efuse_data[0x9] >>0 )&0x0001),0x1,7 );
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>1 )&0x0001),0x1,12);
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>2 )&0x0001),0x1,13);
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>3 )&0x0001),0x1,14);
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>4 )&0x0001),0x1,15);
        pmic_config_interface(0x0A44,((efuse_data[0x9] >>5 )&0x0001),0x1,12);
        pmic_config_interface(0x0A44,((efuse_data[0x9] >>6 )&0x0001),0x1,13);
        pmic_config_interface(0x0A44,((efuse_data[0x9] >>7 )&0x0001),0x1,14);
        pmic_config_interface(0x0A44,((efuse_data[0x9] >>8 )&0x0001),0x1,15);
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>9 )&0x0001),0x1,4 );
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>10)&0x0001),0x1,5 );
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>11)&0x0001),0x1,6 );
        pmic_config_interface(0x0A46,((efuse_data[0x9] >>12)&0x0001),0x1,7 );
        pmic_config_interface(0x0A4A,((efuse_data[0x9] >>13)&0x0001),0x1,12);
        pmic_config_interface(0x0A4A,((efuse_data[0x9] >>14)&0x0001),0x1,13);
        pmic_config_interface(0x0A4A,((efuse_data[0x9] >>15)&0x0001),0x1,14);
        pmic_config_interface(0x0A4A,((efuse_data[0xA] >>0 )&0x0001),0x1,15);
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>1 )&0x0001),0x1,4 );
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>2 )&0x0001),0x1,5 );
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>3 )&0x0001),0x1,6 );
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>4 )&0x0001),0x1,7 );
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>5 )&0x0001),0x1,12);
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>6 )&0x0001),0x1,13);
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>7 )&0x0001),0x1,14);
        pmic_config_interface(0x0A48,((efuse_data[0xA] >>8 )&0x0001),0x1,15);
        pmic_config_interface(0x0A4C,((efuse_data[0xA] >>9 )&0x0001),0x1,4 );
        pmic_config_interface(0x0A4C,((efuse_data[0xA] >>10)&0x0001),0x1,5 );
        pmic_config_interface(0x0A4C,((efuse_data[0xA] >>11)&0x0001),0x1,6 );
        pmic_config_interface(0x0A4C,((efuse_data[0xA] >>12)&0x0001),0x1,7 );
        pmic_config_interface(0x0A4E,((efuse_data[0xA] >>13)&0x0001),0x1,12);
        pmic_config_interface(0x0A4E,((efuse_data[0xA] >>14)&0x0001),0x1,13);
        pmic_config_interface(0x0A4E,((efuse_data[0xA] >>15)&0x0001),0x1,14);
        pmic_config_interface(0x0A4E,((efuse_data[0xB] >>0 )&0x0001),0x1,15);
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>1 )&0x0001),0x1,12);
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>2 )&0x0001),0x1,13);
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>3 )&0x0001),0x1,14);
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>4 )&0x0001),0x1,15);
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>5 )&0x0001),0x1,4 );
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>6 )&0x0001),0x1,5 );
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>7 )&0x0001),0x1,6 );
        pmic_config_interface(0x0A50,((efuse_data[0xB] >>8 )&0x0001),0x1,7 );
        pmic_config_interface(0x0A5A,((efuse_data[0xB] >>9 )&0x0001),0x1,12);
        pmic_config_interface(0x0A5A,((efuse_data[0xB] >>10)&0x0001),0x1,13);
        pmic_config_interface(0x0A5A,((efuse_data[0xB] >>11)&0x0001),0x1,14);
        pmic_config_interface(0x0A5A,((efuse_data[0xB] >>12)&0x0001),0x1,15);
        pmic_config_interface(0x0A56,((efuse_data[0xB] >>13)&0x0001),0x1,4 );
        pmic_config_interface(0x0A56,((efuse_data[0xB] >>14)&0x0001),0x1,5 );
        pmic_config_interface(0x0A56,((efuse_data[0xB] >>15)&0x0001),0x1,6 );
        pmic_config_interface(0x0A56,((efuse_data[0xC] >>0 )&0x0001),0x1,7 );
        pmic_config_interface(0x0A4E,((efuse_data[0xC] >>1 )&0x0001),0x1,4 );
        pmic_config_interface(0x0A4E,((efuse_data[0xC] >>2 )&0x0001),0x1,5 );
        pmic_config_interface(0x0A4E,((efuse_data[0xC] >>3 )&0x0001),0x1,6 );
        pmic_config_interface(0x0A4E,((efuse_data[0xC] >>4 )&0x0001),0x1,7 );
        pmic_config_interface(0x0A56,((efuse_data[0xC] >>5 )&0x0001),0x1,12);
        pmic_config_interface(0x0A56,((efuse_data[0xC] >>6 )&0x0001),0x1,13);
        pmic_config_interface(0x0A56,((efuse_data[0xC] >>7 )&0x0001),0x1,14);
        pmic_config_interface(0x0A56,((efuse_data[0xC] >>8 )&0x0001),0x1,15);
        pmic_config_interface(0x045C,((efuse_data[0xC] >>9 )&0x0001),0x1,8 );
        pmic_config_interface(0x045C,((efuse_data[0xC] >>10)&0x0001),0x1,9 );
        pmic_config_interface(0x045C,((efuse_data[0xC] >>11)&0x0001),0x1,10);
        
        pmic_config_interface(0x0CB8,((efuse_data[0x10]>>3 )&0x0001),0x1,10);   
        pmic_config_interface(0x0CB8,((efuse_data[0x10]>>4 )&0x0001),0x1,11);   
        pmic_config_interface(0x0CB8,((efuse_data[0x10]>>5 )&0x0001),0x1,12);   
        pmic_config_interface(0x0CB8,((efuse_data[0x10]>>6 )&0x0001),0x1,13);   
        pmic_config_interface(0x0CB8,((efuse_data[0x10]>>7 )&0x0001),0x1,14);   
        pmic_config_interface(0x0CB8,((efuse_data[0x10]>>8 )&0x0001),0x1,15);   
                              
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>0 )&0x0001),0x1,0 );            
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>1 )&0x0001),0x1,1 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>2 )&0x0001),0x1,2 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>3 )&0x0001),0x1,3 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>4 )&0x0001),0x1,4 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>5 )&0x0001),0x1,5 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>6 )&0x0001),0x1,6 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>7 )&0x0001),0x1,7 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>8 )&0x0001),0x1,8 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>9 )&0x0001),0x1,9 );
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>10)&0x0001),0x1,10);
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>11)&0x0001),0x1,11);
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>12)&0x0001),0x1,12);
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>13)&0x0001),0x1,13);
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>14)&0x0001),0x1,14);
        pmic_config_interface(0x0E84,((efuse_data[0x1E]>>15)&0x0001),0x1,15);
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>0 )&0x0001),0x1,0 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>1 )&0x0001),0x1,1 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>2 )&0x0001),0x1,2 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>3 )&0x0001),0x1,3 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>4 )&0x0001),0x1,4 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>5 )&0x0001),0x1,5 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>6 )&0x0001),0x1,6 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>7 )&0x0001),0x1,7 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>8 )&0x0001),0x1,8 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>9 )&0x0001),0x1,9 );
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>10)&0x0001),0x1,10);
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>11)&0x0001),0x1,11);
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>12)&0x0001),0x1,12);
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>13)&0x0001),0x1,13);
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>14)&0x0001),0x1,14);
        pmic_config_interface(0x0E86,((efuse_data[0x1F]>>15)&0x0001),0x1,15);
        //------------------------------------------

        print("After apply pmic efuse\n");
        pmic_6325_efuse_check();

        //pmic_6325_efuse_check_bit();
    }
}

void pmic_buck_zxc_check_log(void)
{
    print("Reg[0x%x]=0x%x\n", 0x446, upmu_get_reg_value(0x446));
    print("Reg[0x%x]=0x%x\n", 0x486, upmu_get_reg_value(0x486));
    print("Reg[0x%x]=0x%x\n", 0x474, upmu_get_reg_value(0x474));
    print("Reg[0x%x]=0x%x\n", 0x43C, upmu_get_reg_value(0x43C));
    print("Reg[0x%x]=0x%x\n", 0x494, upmu_get_reg_value(0x494));
    print("Reg[0x%x]=0x%x\n", 0x46E, upmu_get_reg_value(0x46E));

    print("Reg[0x%x]=0x%x\n", 0x644, upmu_get_reg_value(0x644));
    print("Reg[0x%x]=0x%x\n", 0x66A, upmu_get_reg_value(0x66A));
    print("Reg[0x%x]=0x%x\n", 0x61E, upmu_get_reg_value(0x61E));
    print("Reg[0x%x]=0x%x\n", 0x532, upmu_get_reg_value(0x532));
    print("Reg[0x%x]=0x%x\n", 0x694, upmu_get_reg_value(0x694));
    print("Reg[0x%x]=0x%x\n", 0x464, upmu_get_reg_value(0x464));
}

int pmic_zxc_remap_1(int val)
{
    int ret=0;
    
    switch(val){       
        case 0x01: ret=0x05; break;
        case 0x02: ret=0x06; break;
        case 0x03: ret=0x07; break;
        case 0x04: ret=0x07; break;
        case 0x05: ret=0x07; break;
        case 0x06: ret=0x07; break;
        case 0x07: ret=0x07; break;
        case 0x00: ret=0x04; break;
        case 0x08: ret=0x03; break;
        case 0x10: ret=0x02; break;
        case 0x18: ret=0x01; break;
        case 0x20: ret=0x00; break;
        case 0x28: ret=0x08; break;
        case 0x30: ret=0x10; break;
        case 0x38: ret=0x18; break;
        default:
            ret=val;    
            print("[pmic_zxc_remap_1] not match (%d)\n", val);
            break;
    }

    print("[pmic_zxc_remap_1] ret=%d\n", ret);
    return ret;
}

int pmic_zxc_remap_2(int val)
{    
    int ret=0;
    
    switch(val){       
        case 0x00: ret=0x04; break;
        
        case 0x40: ret=0x44; break;
        case 0x01: ret=0x05; break;
        case 0x41: ret=0x45; break;
        case 0x02: ret=0x06; break;
        case 0x42: ret=0x46; break;
        case 0x03: ret=0x07; break;
        case 0x43: ret=0x47; break;
        case 0x04: ret=0x47; break;
        case 0x44: ret=0x47; break;
        case 0x05: ret=0x47; break;
        case 0x45: ret=0x47; break;
        case 0x06: ret=0x47; break;
        case 0x46: ret=0x47; break;
        case 0x07: ret=0x47; break;
        case 0x47: ret=0x47; break;
                           
        case 0x80: ret=0x43; break;
        case 0x08: ret=0x03; break;
        case 0x88: ret=0x42; break;
        case 0x10: ret=0x02; break;
        case 0x90: ret=0x41; break;
        case 0x18: ret=0x01; break;
        case 0x98: ret=0x40; break;
        case 0x20: ret=0x00; break;
        case 0xA0: ret=0x80; break;
        case 0x28: ret=0x08; break;
        case 0xA8: ret=0x88; break;
        case 0x30: ret=0x10; break;
        case 0xB0: ret=0x90; break;
        case 0x38: ret=0x18; break;
        case 0xB8: ret=0x98; break;

        default:
            ret=val;
            print("[pmic_zxc_remap_2] not match (%d)\n", val);
            break;
    }

    print("[pmic_zxc_remap_2] ret=%d\n", ret);
    return ret;
}

void pmic_buck_zxc_check(void)
{
    int read_val=0;
    int write_val=0;
        
    if( (g_ebit_990==0) && (g_ebit_991==0) )
    {
        print("[pmic_buck_zxc_check] Before\n"); pmic_buck_zxc_check_log();

        //init
        pmic_config_interface(0x644,0x6,0x7,0);
        pmic_config_interface(0x66A,0x6,0x7,0);
        pmic_config_interface(0x61E,0x6,0x7,0);
        pmic_config_interface(0x532,0x6,0x7,0);
        pmic_config_interface(0x694,0x6,0x7,0);
        pmic_config_interface(0x464,0x6,0x7,6);

        pmic_read_interface(  0x446,&read_val,0x3F,9); write_val = pmic_zxc_remap_1(read_val);       
        pmic_config_interface(0x446,write_val,0x3F,9);

        pmic_read_interface(  0x486,&read_val,0x3F,9); write_val = pmic_zxc_remap_1(read_val);       
        pmic_config_interface(0x486,write_val,0x3F,9);

        pmic_read_interface(  0x474,&read_val,0x3F,9); write_val = pmic_zxc_remap_1(read_val);       
        pmic_config_interface(0x474,write_val,0x3F,9);

        pmic_read_interface(  0x43C,&read_val,0x3F,9); write_val = pmic_zxc_remap_1(read_val);       
        pmic_config_interface(0x43C,write_val,0x3F,9);

        pmic_read_interface(  0x494,&read_val,0x3F,0); write_val = pmic_zxc_remap_1(read_val);       
        pmic_config_interface(0x494,write_val,0x3F,0);

        pmic_read_interface(  0x46E,&read_val,0xFF,8); write_val = pmic_zxc_remap_2(read_val);       
        pmic_config_interface(0x46E,write_val,0xFF,8);
    
        print("[pmic_buck_zxc_check] After\n"); pmic_buck_zxc_check_log();
    }
    else
    {
        print("[pmic_buck_zxc_check] Ignore (g_ebit_990=%d, g_ebit_991=%d) \n", g_ebit_990, g_ebit_991); 
    }
}

//==============================================================================
// PMIC Init Code
//==============================================================================
void pmic_hw_pre_init(void)
{
    int ret=0;

    if(get_mt6325_pmic_chip_version()==PMIC6325_E3_CID_CODE)
    {
        ret = pmic_config_interface(0x43C,0x0,0x3,7); // [8:7]: RG_VDRAM_ZX_OS; Chihao: for E3 negative current sipke issue only
        ret = pmic_config_interface(0x446,0x0,0x3,7); // [8:7]: RG_VCORE1_ZX_OS; Johnson: for E3 negative current sipke issue only
        ret = pmic_config_interface(0x466,0x0,0x3,10); // [11:10]: RG_VDVFS11_ZX_OS; Johnson: for E3 negative current sipke issue only
        ret = pmic_config_interface(0x466,0x0,0x3,12); // [13:12]: RG_VDVFS12_ZX_OS; Johnson: for E3 negative current sipke issue only
        ret = pmic_config_interface(0x474,0x0,0x3,7); // [8:7]: RG_VGPU_ZX_OS; Chihao: for E3 negative current sipke issue only
        ret = pmic_config_interface(0x486,0x0,0x3,7); // [8:7]: RG_VCORE2_ZX_OS; Johnson: for E3 negative current sipke issue only
        ret = pmic_config_interface(0x490,0x1,0x3,7); // [8:7]: RG_VIO18_ZX_OS; Johnson: for E3 negative current sipke issue only

        print("[pmic_hw_pre_init] Reg[0x%x]=0x%x\n", 0x43C, upmu_get_reg_value(0x43C));
        print("[pmic_hw_pre_init] Reg[0x%x]=0x%x\n", 0x446, upmu_get_reg_value(0x446));
        print("[pmic_hw_pre_init] Reg[0x%x]=0x%x\n", 0x466, upmu_get_reg_value(0x466));
        print("[pmic_hw_pre_init] Reg[0x%x]=0x%x\n", 0x474, upmu_get_reg_value(0x474));
        print("[pmic_hw_pre_init] Reg[0x%x]=0x%x\n", 0x486, upmu_get_reg_value(0x486));
        print("[pmic_hw_pre_init] Reg[0x%x]=0x%x\n", 0x490, upmu_get_reg_value(0x490));
    }
    else
    {
        print("[pmic_hw_pre_init] NA\n");
    }
}

void pmic_hw_pre_init_pfm_oc(void)
{
    int ret=0;

    if( (get_mt6325_pmic_chip_version()==PMIC6325_E3_CID_CODE) ||
        (get_mt6325_pmic_chip_version()==PMIC6325_E5_CID_CODE)
    ){
        ret = pmic_config_interface(0x532,0x6,0x7,0); // [2:0]: VDRAM_BURST; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x532,0x6,0x7,4); // [6:4]: VDRAM_BURST_ON; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x532,0x6,0x7,8); // [10:8]: VDRAM_BURST_SLEEP; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x61E,0x6,0x7,0); // [2:0]: VGPU_BURST; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x61E,0x6,0x7,4); // [6:4]: VGPU_BURST_ON; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x61E,0x6,0x7,8); // [10:8]: VGPU_BURST_SLEEP; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x644,0x6,0x7,0); // [2:0]: VCORE1_BURST; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x644,0x6,0x7,4); // [6:4]: VCORE1_BURST_ON; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x644,0x6,0x7,8); // [10:8]: VCORE1_BURST_SLEEP; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x66A,0x6,0x7,0); // [2:0]: VCORE2_BURST; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x66A,0x6,0x7,4); // [6:4]: VCORE2_BURST_ON; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x66A,0x6,0x7,8); // [10:8]: VCORE2_BURST_SLEEP; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x694,0x6,0x7,0); // [2:0]: VIO18_BURST; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x694,0x6,0x7,4); // [6:4]: VIO18_BURST_ON; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        ret = pmic_config_interface(0x694,0x6,0x7,8); // [10:8]: VIO18_BURST_SLEEP; 8/19: Chihao: E3 only, to keep hystersis window between PFM an PWM
        
        print("[pmic_hw_pre_init_pfm_oc] Reg[0x%x]=0x%x\n", 0x532, upmu_get_reg_value(0x532));
        print("[pmic_hw_pre_init_pfm_oc] Reg[0x%x]=0x%x\n", 0x61E, upmu_get_reg_value(0x61E));
        print("[pmic_hw_pre_init_pfm_oc] Reg[0x%x]=0x%x\n", 0x644, upmu_get_reg_value(0x644));
        print("[pmic_hw_pre_init_pfm_oc] Reg[0x%x]=0x%x\n", 0x66A, upmu_get_reg_value(0x66A));
        print("[pmic_hw_pre_init_pfm_oc] Reg[0x%x]=0x%x\n", 0x694, upmu_get_reg_value(0x694));
    }
    else
    {
        print("[pmic_hw_pre_init_pfm_oc] NA\n");
    }
}


U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;
    int ret_val=0;
    int reg_val=0;

    print("[pmic_init] Preloader Start..................\n");    
    print("[pmic_init] MT6325 CHIP Code = 0x%x\n", get_mt6325_pmic_chip_version());

	//detect V battery Drop 
	pmic_DetectVbatDrop();
    #if 1
    ret_val = pmic_config_interface(0x1E,0x0,0x1,11); // [11:11]: RG_TESTMODE_SWEN; CC: Test mode, first command
    print("[pmic_init] Reg[0x%x]=0x%x\n", 0x1E, upmu_get_reg_value(0x1E));   

    ret_val = pmic_config_interface(0x44E,0x1,0x1,0); //RG_SMPS_TESTMODE_B[0]; Chris Huang
    print("[pmic_init] Reg[0x%x]=0x%x\n", 0x44E, upmu_get_reg_value(0x44E));        
    #endif
        
    ret_val = pmic_config_interface(0xF14,0x4,0x1F,0);  //2.7V
    ret_val = pmic_config_interface(0xF16,0x5,0x1F,0);  //2.75V
    print("[pmic_init]UVLO_L Reg[0x%x]=0x%x Reg[0x%x]=0x%x\n", 0xF14, upmu_get_reg_value(0xF14),0xF16, upmu_get_reg_value(0xF16));   

    #if 1
    //Enable PMIC RST function (depends on main chip RST function)
    ret_val=pmic_config_interface(MT6325_TOP_RST_MISC_CLR, 0x0002, 0xFFFF, 0); //[1]=0, RG_WDTRSTB_MODE
    ret_val=pmic_config_interface(MT6325_TOP_RST_MISC_SET, 0x0001, 0xFFFF, 0); //[0]=1, RG_WDTRSTB_EN
    print("[pmic_init] Reg[0x%x]=0x%x\n", MT6325_TOP_RST_MISC, upmu_get_reg_value(MT6325_TOP_RST_MISC));        
    #endif

    #if 1
    pmic_hw_pre_init(); //n-limit
    pmic_hw_pre_init_pfm_oc(); 
    #endif

    #if defined(MTK_PUMP_EXPRESS_PLUS_SUPPORT)
    ret_val=pmic_config_interface(0xEF4,0xF,0xF,4); // [7:4]: RG_VCDT_HV_VTH; Tim:VCDT_HV_th=10.5V
    #endif

    #if 1
    pmic_6325_efuse_management();
    #endif

    #if 1
    if(get_mt6325_pmic_chip_version()==PMIC6325_E1_CID_CODE)
    {        
        pmic_buck_unlimit();        
    }
    #endif

    #if 1
    if(get_mt6325_pmic_chip_version()==PMIC6325_E3_CID_CODE)
    {        
        pmic_buck_zxc_check();
    }
    #endif

    #if 1
    mt6311_driver_probe();
    #endif

    #ifdef DUMMY_AP
    //print("[pmic_init for DUMMY_AP]\n");
    #endif

    print("[pmic_init] Done...................\n");

    return ret_code;
}

//==============================================================================
// PMIC API for preloader : AUXADC
//==============================================================================

//==============================================================================
// PMIC-AUXADC related define
//==============================================================================
#define VOLTAGE_FULL_RANGE     	1800
#define ADC_PRECISE		32768 	// 15 bits

//==============================================================================
// PMIC-AUXADC global variable
//==============================================================================
kal_int32 count_time_out=10000;

void pmic_auxadc_init(void)
{
	U32 adc_busy;
    
	//mt6325_upmu_set_rg_vref18_enb(0);
	pmic_config_interface(MT6325_AUXADC_CON26, 0, MT6325_PMIC_RG_VREF18_ENB_MASK, MT6325_PMIC_RG_VREF18_ENB_SHIFT);
	//change to suspend/resume
	pmic_read_interface(MT6325_AUXADC_ADC19, &adc_busy, MT6325_PMIC_RG_ADC_BUSY_MASK, MT6325_PMIC_RG_ADC_BUSY_SHIFT); 
	if (adc_busy==0) {
		pmic_config_interface(MT6325_AUXADC_CON19, 0x0, MT6325_PMIC_RG_ADC_DECI_GDLY_MASK, MT6325_PMIC_RG_ADC_DECI_GDLY_SHIFT);
	}
	//mt6325_upmu_set_rg_adc_gps_status(1);
	pmic_config_interface(MT6325_AUXADC_CON19, 1, MT6325_PMIC_RG_ADC_GPS_STATUS_MASK, MT6325_PMIC_RG_ADC_GPS_STATUS_SHIFT);
	//mt6325_upmu_set_rg_adc_md_status(1);
	pmic_config_interface(MT6325_AUXADC_CON27, 1, MT6325_PMIC_RG_ADC_MD_STATUS_MASK, MT6325_PMIC_RG_ADC_MD_STATUS_SHIFT);
	//mt6325_upmu_set_rg_deci_gdly_vref18_selb(1);
	pmic_config_interface(MT6325_AUXADC_CON26, 1, MT6325_PMIC_RG_DECI_GDLY_VREF18_SELB_MASK, MT6325_PMIC_RG_DECI_GDLY_VREF18_SELB_SHIFT);
	//mt6325_upmu_set_rg_deci_gdly_sel_mode(1);
    pmic_config_interface(MT6325_AUXADC_CON26, 1, MT6325_PMIC_RG_DECI_GDLY_SEL_MODE_MASK, MT6325_PMIC_RG_DECI_GDLY_SEL_MODE_SHIFT);
	//*set CLK as 26MHz CLK
	//mt6325_upmu_set_rg_auxadc_ck_cksel(1);
	pmic_config_interface(MT6325_TOP_CKHWEN_CON, 1, MT6325_PMIC_RG_AUXADC_CK_CKSEL_HWEN_MASK, MT6325_PMIC_RG_AUXADC_CK_CKSEL_HWEN_SHIFT);
	//1. set AUXADC CLK HW control
	//mt6325_upmu_set_rg_auxadc_ck_pdn_hwen(1);
	pmic_config_interface(MT6325_TOP_CKHWEN_CON, 1, MT6325_PMIC_RG_AUXADC_CK_PDN_HWEN_MASK, MT6325_PMIC_RG_AUXADC_CK_PDN_HWEN_SHIFT);
	//2. turn on AP & MD
	//mt6325_upmu_set_rg_clksq_en_aux_ap(1);
	pmic_config_interface(MT6325_TOP_CLKSQ, 1, MT6325_PMIC_RG_CLKSQ_EN_AUX_AP_MASK, MT6325_PMIC_RG_CLKSQ_EN_AUX_AP_SHIFT); 
	//mt6325_upmu_set_auxadc_ck_aon(1);
	pmic_config_interface(MT6325_AUXADC_CK0, 1, MT6325_PMIC_AUXADC_CK_AON_MASK, MT6325_PMIC_AUXADC_CK_AON_SHIFT);
	print("****[pmic_auxadc_init] DONE\n");
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
			//ret=mt6325_upmu_get_rg_adc_rdy_gps();
			pmic_read_interface(MT6325_AUXADC_ADC12, &ret, MT6325_PMIC_RG_ADC_RDY_GPS_MASK, MT6325_PMIC_RG_ADC_RDY_GPS_SHIFT);
		} else if (user_num == MD ) {
			//ret=mt6325_upmu_get_rg_adc_rdy_md();
			pmic_read_interface(MT6325_AUXADC_ADC16, &ret, MT6325_PMIC_RG_ADC_RDY_MD_MASK, MT6325_PMIC_RG_ADC_RDY_MD_SHIFT);
		} else if (user_num == AP ) {
			pmic_read_interface(MT6325_AUXADC_ADC0+channel_num * 2,(&int_status_val_0),0x8000,0x0);
			ret = int_status_val_0 >> 15;
		}
	}
	//spin_unlock_irqrestore(&pmic_adc_lock, flags);

    if(ret!=0) print("%d", ret);
    
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
			//int_status_val_0 = mt6325_upmu_get_rg_adc_out_gps();
			pmic_read_interface(MT6325_AUXADC_ADC13, &int_status_val_0, MT6325_PMIC_RG_ADC_OUT_GPS_MASK, MT6325_PMIC_RG_ADC_OUT_GPS_SHIFT);
			//int_status_val_0 = (int_status_val_0 << 1) + mt6325_upmu_get_rg_adc_out_gps_lsb();
			pmic_read_interface(MT6325_AUXADC_ADC14, &ret, MT6325_PMIC_RG_ADC_OUT_GPS_LSB_MASK, MT6325_PMIC_RG_ADC_OUT_GPS_LSB_SHIFT);
			int_status_val_0 = (int_status_val_0 << 1) + ret;
		} else if (user_num == MD ) {
			//int_status_val_0 = mt6325_upmu_get_rg_adc_out_md();
			pmic_read_interface(MT6325_AUXADC_ADC15, &int_status_val_0, MT6325_PMIC_RG_ADC_OUT_MD_MASK, MT6325_PMIC_RG_ADC_OUT_MD_SHIFT);
			//int_status_val_0 = (int_status_val_0 << 1) + mt6325_upmu_get_rg_adc_out_md_lsb();
			pmic_read_interface(MT6325_AUXADC_ADC16, &ret, MT6325_PMIC_RG_ADC_OUT_MD_LSB_MASK, MT6325_PMIC_RG_ADC_OUT_MD_LSB_SHIFT);
			int_status_val_0 = (int_status_val_0 << 1) + ret;
		} else if (user_num == AP ) {
			ret=pmic_read_interface(MT6325_AUXADC_ADC0+channel_num * 2,(&int_status_val_0),0x7fff,0x0);
		}
	}

    if(ret!=0) print("%d", ret);
    
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
			//mt6325_upmu_set_rg_ap_rqst_list(1<<7);
			pmic_config_interface(MT6325_AUXADC_CON22, (1<<7), MT6325_PMIC_RG_AP_RQST_LIST_MASK, MT6325_PMIC_RG_AP_RQST_LIST_SHIFT);
			break;
		case AUX_ISENSE_AP:
			//mt6325_upmu_set_rg_ap_rqst_list(1<<6);
			pmic_config_interface(MT6325_AUXADC_CON22, (1<<6), MT6325_PMIC_RG_AP_RQST_LIST_MASK, MT6325_PMIC_RG_AP_RQST_LIST_SHIFT);
			break;
		case AUX_VCDT_AP:
			//mt6325_upmu_set_rg_ap_rqst_list(1<<4);
			pmic_config_interface(MT6325_AUXADC_CON22, (1<<4), MT6325_PMIC_RG_AP_RQST_LIST_MASK, MT6325_PMIC_RG_AP_RQST_LIST_SHIFT);
			break;          
		case AUX_BATON_AP:
			pmic_config_interface(0x0a08,0x010b, 0xffff, 0);
			pmic_config_interface(0x0f00,0x0005, 0xffff, 0);
			//upmu_set_reg_value(0x0aba,0x500f);
			//mt6325_upmu_set_rg_ap_rqst_list(1<<5);
			pmic_config_interface(MT6325_AUXADC_CON22, (1<<5), MT6325_PMIC_RG_AP_RQST_LIST_MASK, MT6325_PMIC_RG_AP_RQST_LIST_SHIFT);
			break;
		default:
		        print("[AUXADC] Invalid channel value(%d)\n", dwChannel);
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

		print("\n[PMIC_IMM_GetOneChannelValue] %d ch=%d, chip= %d, user= %d \n",dwChannel, channel_num, chip_num, user_num);

		if (chip_num == MT6325_CHIP) {
			if (user_num == AP) {
				//mt6325_upmu_set_rg_clksq_en_aux_ap(1);
				pmic_config_interface(MT6325_TOP_CLKSQ, 1, MT6325_PMIC_RG_CLKSQ_EN_AUX_AP_MASK, MT6325_PMIC_RG_CLKSQ_EN_AUX_AP_SHIFT); 
				//mt6325_upmu_set_strup_auxadc_rstb_sw(1);
				pmic_config_interface(MT6325_STRUP_CON11, 1, MT6325_PMIC_STRUP_AUXADC_RSTB_SW_MASK, MT6325_PMIC_STRUP_AUXADC_RSTB_SW_SHIFT);
				//mt6325_upmu_set_strup_auxadc_rstb_sel(1);
				pmic_config_interface(MT6325_STRUP_CON11, 1, MT6325_PMIC_STRUP_AUXADC_RSTB_SEL_MASK, MT6325_PMIC_STRUP_AUXADC_RSTB_SEL_SHIFT);
			} else if (user_num == MD ) {
				//Prerequisite settings before accessing AuxADC 
				//mt6325_upmu_set_rg_clksq_en_aux_md(1);
				pmic_config_interface(MT6325_TOP_CLKSQ, 1, MT6325_PMIC_RG_CLKSQ_EN_AUX_MD_MASK, MT6325_PMIC_RG_CLKSQ_EN_AUX_MD_SHIFT);
				//mt6325_upmu_set_rg_vref18_enb_md(0);
				pmic_config_interface(MT6325_AUXADC_CON27, 0, MT6325_PMIC_RG_VREF18_ENB_MD_MASK, MT6325_PMIC_RG_VREF18_ENB_MD_SHIFT);
			} else if (user_num == GPS) {
				//mt6325_upmu_set_rg_clksq_en_aux_gps(1);
				pmic_config_interface(MT6325_TOP_CLKSQ, 1, MT6325_PMIC_RG_CLKSQ_EN_AUX_GPS_MASK, MT6325_PMIC_RG_CLKSQ_EN_AUX_GPS_SHIFT);
			}
		}
		
		if (chip_num == MT6325_CHIP) {
			//mt6325_upmu_set_rg_ap_rqst_list(0);
			pmic_config_interface(MT6325_AUXADC_CON22, 0, MT6325_PMIC_RG_AP_RQST_LIST_MASK, MT6325_PMIC_RG_AP_RQST_LIST_SHIFT);
			//mt6325_upmu_set_rg_ap_rqst_list_rsv(0);
			pmic_config_interface(MT6325_AUXADC_CON23, 0, MT6325_PMIC_RG_AP_RQST_LIST_RSV_MASK, MT6325_PMIC_RG_AP_RQST_LIST_RSV_SHIFT);
			//mt6325_upmu_set_rg_md_rqst(0);
			pmic_config_interface(MT6325_AUXADC_CON20, 0, MT6325_PMIC_RG_MD_RQST_MASK, MT6325_PMIC_RG_MD_RQST_SHIFT);
			//mt6325_upmu_set_rg_gps_rqst(0);
			pmic_config_interface(MT6325_AUXADC_CON21, 0, MT6325_PMIC_RG_GPS_RQST_MASK, MT6325_PMIC_RG_GPS_RQST_SHIFT);
		}

		PMIC_IMM_RequestAuxadcChannel(dwChannel);
		udelay(10);
		while( pmic_is_auxadc_ready(channel_num, chip_num, user_num) != 1 ) {
			if( (count++) > count_time_out)
			{
				print("[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
				break;
			}            
		}
		ret_data = pmic_get_adc_output(channel_num, chip_num, user_num);                
		
		//clear
		
		if (chip_num == MT6325_CHIP) {
			//mt6325_upmu_set_rg_ap_rqst_list(0);
			pmic_config_interface(MT6325_AUXADC_CON22, 0, MT6325_PMIC_RG_AP_RQST_LIST_MASK, MT6325_PMIC_RG_AP_RQST_LIST_SHIFT);
			//mt6325_upmu_set_rg_ap_rqst_list_rsv(0);
			pmic_config_interface(MT6325_AUXADC_CON23, 0, MT6325_PMIC_RG_AP_RQST_LIST_RSV_MASK, MT6325_PMIC_RG_AP_RQST_LIST_RSV_SHIFT);
			//mt6325_upmu_set_rg_md_rqst(0);
			pmic_config_interface(MT6325_AUXADC_CON20, 0, MT6325_PMIC_RG_MD_RQST_MASK, MT6325_PMIC_RG_MD_RQST_SHIFT);
			//mt6325_upmu_set_rg_gps_rqst(0);
			pmic_config_interface(MT6325_AUXADC_CON21, 0, MT6325_PMIC_RG_GPS_RQST_MASK, MT6325_PMIC_RG_GPS_RQST_SHIFT);
		}
	        u4channel += ret_data;
	
	        u4Sample_times++;
	
		//debug
		//dprintf(CRITICAL, "[AUXADC] output data[%d]=%d\n", 
//			dwChannel, ret_data);
		if (chip_num == MT6325_CHIP) {
		    if (user_num == AP) {
				//mt6325_upmu_set_rg_clksq_en_aux_ap(0);
				pmic_config_interface(MT6325_TOP_CLKSQ, 0, MT6325_PMIC_RG_CLKSQ_EN_AUX_AP_MASK, MT6325_PMIC_RG_CLKSQ_EN_AUX_AP_SHIFT); 
			} else if (user_num == MD) {
				//Prerequisite settings before accessing AuxADC 
				//mt6325_upmu_set_rg_clksq_en_aux_md(0);
				pmic_config_interface(MT6325_TOP_CLKSQ, 0, MT6325_PMIC_RG_CLKSQ_EN_AUX_MD_MASK, MT6325_PMIC_RG_CLKSQ_EN_AUX_MD_SHIFT);
				//mt6325_upmu_set_rg_vref18_enb_md(1);
				pmic_config_interface(MT6325_AUXADC_CON27, 1, MT6325_PMIC_RG_VREF18_ENB_MD_MASK, MT6325_PMIC_RG_VREF18_ENB_MD_SHIFT);
			} else if (user_num == GPS) {
				//mt6325_upmu_set_rg_clksq_en_aux_gps(0);
				pmic_config_interface(MT6325_TOP_CLKSQ, 0, MT6325_PMIC_RG_CLKSQ_EN_AUX_GPS_MASK, MT6325_PMIC_RG_CLKSQ_EN_AUX_GPS_SHIFT);
			}
		}
	}while (u4Sample_times < deCount);

	/* Value averaging  */ 
	adc_result_temp = u4channel / deCount;
	
	switch(dwChannel){
	case AUX_BATSNS_AP:
		print("[AUX_BATSNS_AP]");
		r_val_temp = 4;
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	case AUX_ISENSE_AP:
		print("[AUX_ISENSE_AP]");
		r_val_temp = 4;           
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	case AUX_VCDT_AP:
		print("[AUX_VCDT_AP]");
		r_val_temp = 1;           
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	case AUX_BATON_AP:
		print("[AUX_BATON_AP]");
		r_val_temp = 2;           
		adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
		break;
	default:
	    print("[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
	    return -1;
	}

	print("outputdata=%d, transfer data=%d, r_val=%d.\n", 
	        adc_result_temp, adc_result, r_val_temp);
	
	return adc_result;
#else
	return 0;
#endif   
}

int get_bat_sense_volt(int times)
{
    int bat_vol;
	bat_vol = PMIC_IMM_GetOneChannelValue(AUX_BATSNS_AP,times,1);
	print("Check Battery Vol = %d\n", bat_vol);
	return bat_vol;
}

