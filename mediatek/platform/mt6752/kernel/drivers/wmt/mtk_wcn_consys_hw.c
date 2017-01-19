/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/




/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG "[WMT-CONSYS-HW]"


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include <mach/mt_clkmgr.h>
#include <linux/delay.h>
#include <mach/upmu_common.h>

#include "osal_typedef.h"
#include "mtk_wcn_consys_hw.h"

#if CONSYS_EMI_MPU_SETTING
#include <mach/emi_mpu.h>
#endif
#if CONSYS_PMIC_CTRL_ENABLE 
#include <mach/mt_pm_ldo.h>
#endif
#include <mach/mtk_hibernate_dpm.h>
#include <asm/memblock.h>

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
UINT8 __iomem *pEmibaseaddr = NULL;
#ifdef BT_WIFI_SHARE_V33
extern BT_WIFI_V33_STATUS gBtWifiV33;
#endif
phys_addr_t gConEmiPhyBase;
struct CONSYS_BASE_ADDRESS conn_reg;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/





/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/



/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#if CONSYS_ENALBE_SET_JTAG
	UINT32 gJtagCtrl = 0;

#define JTAG_ADDR1_BASE 0x10002000

char *jtag_addr1 = (char *)JTAG_ADDR1_BASE;

#define JTAG1_REG_WRITE(addr, value)	\
writel(value, ((volatile UINT32 *)(jtag_addr1+(addr-JTAG_ADDR1_BASE))))
#define JTAG1_REG_READ(addr)			\
readl(((volatile UINT32 *)(jtag_addr1+(addr-JTAG_ADDR1_BASE))))

static INT32 mtk_wcn_consys_jtag_set_for_mcu(VOID)
{
#if 1
    int iRet = -1;

	WMT_PLAT_INFO_FUNC("WCN jtag_set_for_mcu start...\n");
    jtag_addr1 = ioremap(JTAG_ADDR1_BASE, 0x5000);
    if (jtag_addr1 == 0) {
        printk("remap jtag_addr1 fail!\n");
        return iRet;
    }
    printk("jtag_addr1 = 0x%p\n", jtag_addr1);

	JTAG1_REG_WRITE(0x100053c4, 0x11111100);
	JTAG1_REG_WRITE(0x100053d4, 0x00111111);

			/*Enable IES of all pins*/
	JTAG1_REG_WRITE(0x10002014, 0x00000003);
	JTAG1_REG_WRITE(0x10005334, 0x55000000);
	JTAG1_REG_WRITE(0x10005344, 0x00555555);
	JTAG1_REG_WRITE(0x10005008, 0xc0000000);
	JTAG1_REG_WRITE(0x10005018, 0x0000000d);
	JTAG1_REG_WRITE(0x10005014, 0x00000032);
	JTAG1_REG_WRITE(0x100020a4, 0x000000ff);
	JTAG1_REG_WRITE(0x100020d4, 0x000000b4);
	JTAG1_REG_WRITE(0x100020d8, 0x0000004b);


#else
	WMT_PLAT_INFO_FUNC("WCN jtag set for mcu start...\n");
		kal_int32 iRet = 0;
		kal_uint32 tmp = 0;
		kal_int32 addr = 0;
		kal_int32 remap_addr1 = 0;
		kal_int32 remap_addr2 = 0;
		
		remap_addr1 = ioremap(JTAG_ADDR1_BASE, 0x1000);
		if (remap_addr1 == 0) {
			printk("remap jtag_addr1 fail!\n");
			return -1;
		}
		
		remap_addr2 = ioremap(JTAG_ADDR2_BASE, 0x100);
		if (remap_addr2 == 0) {
			printk("remap jtag_addr2 fail!\n");
			return -1;
		}

	/*Pinmux setting for MT6625 I/F*/
		addr = remap_addr1 + 0x03C0;
		tmp = DRV_Reg32(addr);
		tmp = tmp & 0xff;
		tmp = tmp | 0x11111100;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr)); 

		addr = remap_addr1 + 0x03D0;
		tmp = DRV_Reg32(addr);
		tmp = tmp & 0xff000000;
		tmp = tmp | 0x00111111;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr)); 

	/*AP GPIO Setting 1 <default use>*/
		/*Enable IES*/
		//addr = 0x10002014;
		addr = remap_addr2 + 0x0014;
		tmp = 0x00000003;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr)); 
		/*GPIO mode setting*/
		//addr = 0x10005334;
		addr = remap_addr1 + 0x0334;
		tmp = 0x55000000;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
	
		//addr = 0x10005344;
		addr = remap_addr1 + 0x0344;
		tmp = 0x00555555;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
		/*GPIO direction control*/
		//addr = 0x10005008;
		addr = remap_addr1 + 0x0008;
		tmp = 0xc0000000;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
	
		//addr = 0x10005018;
		addr = remap_addr1 + 0x0018;
		tmp = 0x0000000d;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
	
		//addr = 0x10005014;
		addr = remap_addr1 + 0x0014;
		tmp = 0x00000032;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
	
		/*PULL Enable*/
		//addr = 0x100020a4;
		addr = remap_addr2 + 0x00a4;
		tmp = 0x000000ff;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
	
		/*PULL select enable*/
		//addr = 0x100020d4;
		addr = remap_addr2 + 0x00d4;
		tmp = 0x000000b4;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
	
		//addr = 0x100020d8;
		addr = remap_addr2 + 0x00d8;
		tmp = 0x0000004b;
		DRV_WriteReg32(addr, tmp);
		WMT_PLAT_INFO_FUNC("(RegAddr, RegVal):(0x%08x, 0x%08x)", addr, DRV_Reg32(addr));
#endif

	return 0;
}

UINT32 mtk_wcn_consys_jtag_flag_ctrl(UINT32 en)
{
	WMT_PLAT_INFO_FUNC("%s jtag set for MCU\n",en ? "enable" : "disable");
	gJtagCtrl = en;

	return 0;
}

#endif


#if CONFIG_OF

#define PWR_ON_OFF_API_AVALIABLE 1

INT32 mtk_wcn_consys_hw_reg_ctrl(UINT32 on,UINT32 co_clock_en)
{
	
#if PWR_ON_OFF_API_AVALIABLE
	INT32 iRet = -1;
#endif

	UINT32 retry = 10;
	UINT32 consysHwChipId = 0;

	WMT_PLAT_INFO_FUNC("CONSYS-HW-REG-CTRL(0x%08x),start\n",on);
	WMT_PLAT_DBG_FUNC("CONSYS_EMI_MAPPING dump before power on/off(0x%08x)\n",CONSYS_REG_READ(CONSYS_EMI_MAPPING));

	if(on)
	{
#if CONSYS_PMIC_CTRL_ENABLE 
		/*need PMIC driver provide new API protocol */
	    /*1.AP power on MT6325 VCN_1V8 LDO (with PMIC_WRAP API) VCN_1V8	"0x0512[1] "1'b0   0x0512[14]"	1'b1"*/
		mt6325_upmu_set_rg_vcn18_on_ctrl(0);
		/* VOL_DEFAULT, VOL_1200, VOL_1300, VOL_1500, VOL_1800, VOL_2500, VOL_2800, VOL_3000, VOL_3300*/
		hwPowerOn(MT6325_POWER_LDO_VCN18, VOL_1800, "wcn_drv"); 

		udelay(150);
		
		if(co_clock_en)
		{
			/*if co-clock mode:*/
			/*2.set VCN28 to SW control mode (with PMIC_WRAP API) VCN28_ON_CTRL 	0x041C[14]	1'b0*/
			/*turn on VCN28 LDO only when FMSYS is activated"  */
			mt6325_upmu_set_rg_vcn28_on_ctrl(0);
		}
		else
		{
			/*if NOT co-clock:*/
			/*2.1.switch VCN28 to HW control mode (with PMIC_WRAP API) VCN28_ON_CTRL  0x041C[14] 1'b1*/
			mt6325_upmu_set_rg_vcn28_on_ctrl(1);
			/*2.2.turn on VCN28 LDO (with PMIC_WRAP API)"  RG_VCN28_EN"	 0x041C[12] 1'b1	*/
			hwPowerOn(MT6325_POWER_LDO_VCN28, VOL_2800, "wcn_drv"); 
		}
#endif

		/*3.assert CONNSYS CPU SW reset  0x10007018	"[12]=1'b1  [31:24]=8'h88 (key)"*/
		CONSYS_REG_WRITE((conn_reg.ap_rgu_base + CONSYS_CPU_SW_RST_OFFSET), CONSYS_REG_READ(conn_reg.ap_rgu_base + CONSYS_CPU_SW_RST_OFFSET) | CONSYS_CPU_SW_RST_BIT | CONSYS_CPU_SW_RST_CTRL_KEY);
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_CPU_SW_RST_REG(0x%x)\n",CONSYS_REG_READ(conn_reg.ap_rgu_base + CONSYS_CPU_SW_RST_OFFSET));
		/*turn on top clock gating enable	TOP_CLKCG_CLR	0x10000084[26]	1'b1	*/
		CONSYS_REG_WRITE((conn_reg.topckgen_base + CONSYS_TOP_CLKCG_CLR_OFFSET), CONSYS_REG_READ(conn_reg.topckgen_base + CONSYS_TOP_CLKCG_CLR_OFFSET) | CONSYS_TOP_CLKCG_BIT);
		/*turn on SPM clock gating enable	PWRON_CONFG_EN	0x10006000	32'h0b160001	*/
		CONSYS_REG_WRITE((conn_reg.spm_base + CONSYS_PWRON_CONFG_EN_OFFSET), CONSYS_PWRON_CONFG_EN_VALUE);		

#if PWR_ON_OFF_API_AVALIABLE
		iRet = conn_power_on(); //consult clkmgr owner. 
		if(iRet)
		{
			WMT_PLAT_ERR_FUNC("conn_power_on fail(%d)\n",iRet);
		}else
		{
			WMT_PLAT_INFO_FUNC("conn_power_on ok\n");
		}
#else
		/*2.write conn_top1_pwr_on=1, power on conn_top1 (MT6572 with the same addr)	conn_spm_pwr_on 0x10006280 [2]	1'b1		*/			
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) | CONSYS_SPM_PWR_ON_BIT);
		/*3.read conn_top1_pwr_on_ack =1, power on ack ready (MT6572 with the same addr)	pwr_conn_ack	0x1000660C [1]			*/		
		while (0 == (CONSYS_PWR_ON_ACK_BIT & CONSYS_REG_READ(conn_reg.spm_base + CONSYS_PWR_CONN_ACK_OFFSET)));
		/*4.write conn_top1_mem_PD=0, power on MCU memory (MT6572 with the same addr)	sram_conn_pd	0x10006280 [8]	1'b1	*/		
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) | CONSYS_SRAM_CONN_PD_BIT);
		/*5.write conn_top1_pwr_on_s=1, power on conn_top1 (MT6572 with the same addr)	conn_spm_pwr_on_s	0x10006280 [3]	1'b1	*/					
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) | CONSYS_SPM_PWR_ON_S_BIT);
		/*6.write conn_clk_dis=0, enable connsys clock	(MT6572 with the same addr) conn_clk_dis	0x10006280 [4]	1'b0				*/		
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) & ~CONSYS_CLK_CTRL_BIT);
		/*7.wait 1us	*/	
		udelay(1);
		/*8.read conn_top1_pwr_on_ack_s =1, power on ack ready (MT6572 with the same addr)	pwr_conn_ack_s	0x10006610 [1]	*/						
		while (0 == (CONSYS_PWR_CONN_ACK_S_BIT & CONSYS_REG_READ(conn_reg.spm_base + CONSYS_PWR_CONN_ACK_S_OFFSET)));
		/*9.release connsys ISO, conn_top1_iso_en=0 (MT6572 with the same addr) conn_spm_pwr_iso	0x10006280 [1]	1'b0		*/				
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) & ~CONSYS_SPM_PWR_ISO_S_BIT);
		/*10.release SW reset of connsys, conn_ap_sw_rst_b=1 (MT6572 with the same addr) conn_spm_pwr_rst_b	0x10006280[0]	1'b1	*/					
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) | CONSYS_SPM_PWR_RST_BIT);

#endif
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_PWR_CONN_ACK_REG(0x%x)\n",CONSYS_REG_READ(conn_reg.spm_base + CONSYS_PWR_CONN_ACK_OFFSET));
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_PWR_CONN_ACK_S_REG(0x%x)\n",CONSYS_REG_READ(conn_reg.spm_base + CONSYS_PWR_CONN_ACK_S_OFFSET));
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_TOP1_PWR_CTRL_REG(0x%x)\n",CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET));
		/*11.26M is ready now, delay 10us for mem_pd de-assert*/
		udelay(10);
		/*enable AP bus clock : connmcu_bus_pd  API: enable_clock() ++??*/
		
		/*12.poll CONNSYS CHIP ID until 6752 is returned		0x18070008	32'h6752 */
		while (retry-- > 0)
		{
			consysHwChipId = CONSYS_REG_READ(conn_reg.mcu_base + CONSYS_CHIP_ID_OFFSET);
			if((consysHwChipId == 0x6752) || (consysHwChipId == 0x6582) || (consysHwChipId == 0x6572) || (consysHwChipId == 0x6571)) {
				WMT_PLAT_INFO_FUNC("retry(%d)consys chipId(0x%08x)\n", retry,consysHwChipId);
				break;
			} else {
				WMT_PLAT_INFO_FUNC("Read CONSYS chipId(0x%08x)",consysHwChipId); 
			}
			msleep(20);
		}

		/*13.{default no need}update ROMDEL/PATCH RAM DELSEL if needed		0x18070114	"[3:0] ROM DELSEL [7:$] RAM4Kx32 DELSEL"	??*/
		
		/*14.write 1 to conn_mcu_confg ACR[1] if real speed MBIST (default write "1")	ACR	0x18070110[18]	1'b1*/
		/*if this bit is 0, HW will do memory auto test under low CPU frequence (26M Hz)*/
		/*if this bit is 0, HW will do memory auto test under high CPU frequence(138M Hz) inclulding low CPU frequence*/
		CONSYS_REG_WRITE(conn_reg.mcu_base + CONSYS_MCU_CFG_ACR_OFFSET, CONSYS_REG_READ(conn_reg.mcu_base + CONSYS_MCU_CFG_ACR_OFFSET) | CONSYS_MCU_CFG_ACR_MBIST_BIT);

		/*update ANA_WBG(AFE) CR. AFE setting file:  MT6752_AFE_SW_patch_REG_xxxx.xlsx ??"		AP Offset = 0x180B2000	??*/

#if 0
		/*15.{default no need, Analog HW will inform if this need to be update or not 1 week after IC sample back}
		update ANA_WBG(AFE) CR if needed, CONSYS_AFE_REG */
		CONSYS_REG_WRITE(CONSYS_AFE_REG_DIG_RCK_01,CONSYS_AFE_REG_DIG_RCK_01_VALUE);
		CONSYS_REG_WRITE(CONSYS_AFE_REG_WBG_PLL_02,CONSYS_AFE_REG_WBG_PLL_02_VALUE);
		CONSYS_REG_WRITE(CONSYS_AFE_REG_WBG_WB_TX_01,CONSYS_AFE_REG_WBG_WB_TX_01_VALUE);
#endif
		/*16.deassert CONNSYS CPU SW reset (need to check MT6752)		 0x10007018	"[12]=1'b0 [31:24] =8'h88 (key)"*/
		CONSYS_REG_WRITE(conn_reg.ap_rgu_base + CONSYS_CPU_SW_RST_OFFSET,
			(CONSYS_REG_READ(conn_reg.ap_rgu_base + CONSYS_CPU_SW_RST_OFFSET) & ~CONSYS_CPU_SW_RST_BIT) | CONSYS_CPU_SW_RST_CTRL_KEY);

		msleep(5);

	}else{

#if PWR_ON_OFF_API_AVALIABLE
		WMT_PLAT_INFO_FUNC("\n conn_power_off begin\n");
		/*power off connsys by API (MT6582, MT6572 are different) API: conn_power_off()	*/
		iRet = conn_power_off();//consult clkmgr owner
		WMT_PLAT_INFO_FUNC("\n conn_power_off end\n");
		if(iRet)
		{
			WMT_PLAT_ERR_FUNC("conn_power_off fail(%d)\n",iRet);
		}else
		{
			WMT_PLAT_INFO_FUNC("conn_power_off ok\n");
		}
#else
		/*assert SW reset of connsys, conn_ap_sw_rst_b=0 (MT6572 with the same addr)		0x10006280[0]	1'b0		*/		
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) & ~CONSYS_SPM_PWR_RST_BIT);
		/*release connsys ISO, conn_top1_iso_en=1 (MT6572 with the same addr) 	0x10006280 [1]	1'b1	*/
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) | CONSYS_SPM_PWR_ISO_S_BIT);
		/*DA_WBG_EN_XBUF=0		*/										
		/*wait 1us	*/
		udelay(1);
		/*write conn_top1_mem_PD=1, power off MCU memory (MT6572 with the same addr)		0x10006280 [8]	1'b0	  */	
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) & ~CONSYS_SRAM_CONN_PD_BIT);
		/*write conn_top1_pwr_on=0, power off conn_top1 (MT6572 with the same addr)		0x10006280 [3:2]	2'b00 */	
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) & ~(CONSYS_SPM_PWR_ON_BIT | CONSYS_SPM_PWR_ON_S_BIT));
		/*write conn_clk_dis=1, disable connsys clock  (MT6572 with the same addr)	conn_clk_dis	0x10006280 [4]	1'b1 	*/
		CONSYS_REG_WRITE(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET, CONSYS_REG_READ(conn_reg.spm_base + CONSYS_TOP1_PWR_CTRL_OFFSET) |CONSYS_CLK_CTRL_BIT);

#endif

#if CONSYS_PMIC_CTRL_ENABLE

		/*set VCN_28 to SW control mode (with PMIC_WRAP API) "1. VCN28_ON_CTRL "0x041C[14] 1'b0*/
		mt6325_upmu_set_rg_vcn28_on_ctrl(0);
		/*turn off VCN28 LDO (with PMIC_WRAP API)"	 2. RG_VCN28_EN"  0x041C[12]" 1'b0*/
		hwPowerDown(MT6325_POWER_LDO_VCN28, "wcn_drv");

		/*AP power off MT6625L VCN_1V8 LDO (with PMIC_WRAP API)  "0x0512[1]1'b0  0x0512[14]" 1'b0*/
		mt6325_upmu_set_rg_vcn18_mode_set(0);
		hwPowerDown(MT6325_POWER_LDO_VCN18, "wcn_drv"); 

#endif

	}
	WMT_PLAT_INFO_FUNC("CONSYS-HW-REG-CTRL(0x%08x),finish\n",on);
	return 0;
}

#else
INT32 mtk_wcn_consys_hw_reg_ctrl(UINT32 on,UINT32 co_clock_en)
{

#if PWR_ON_OFF_API_AVALIABLE
	INT32 iRet = -1;
#endif

	UINT32 retry = 10;
	UINT32 consysHwChipId = 0;
	
	WMT_PLAT_INFO_FUNC("CONSYS-HW-REG-CTRL(0x%08x),start\n",on);
	WMT_PLAT_DBG_FUNC("CONSYS_EMI_MAPPING dump before power on/off(0x%08x)\n",CONSYS_REG_READ(CONSYS_EMI_MAPPING));

	if(on)
	{
#if CONSYS_PMIC_CTRL_ENABLE 
		/*need PMIC driver provide new API protocol */
	    /*1.AP power on MT6325 VCN_1V8 LDO (with PMIC_WRAP API) VCN_1V8	"0x0512[1] "1'b0   0x0512[14]"	1'b1"*/
		mt6325_upmu_set_rg_vcn18_on_ctrl(0);
		/* VOL_DEFAULT, VOL_1200, VOL_1300, VOL_1500, VOL_1800, VOL_2500, VOL_2800, VOL_3000, VOL_3300*/
		hwPowerOn(MT6325_POWER_LDO_VCN18, VOL_1800, "wcn_drv"); 

		udelay(150);
		
		if(co_clock_en)
		{
			/*if co-clock mode:*/
			/*2.set VCN28 to SW control mode (with PMIC_WRAP API) VCN28_ON_CTRL 	0x041C[14]	1'b0*/
			/*turn on VCN28 LDO only when FMSYS is activated"  */
			mt6325_upmu_set_rg_vcn28_on_ctrl(0);
		}
		else
		{
			/*if NOT co-clock:*/
			/*2.1.switch VCN28 to HW control mode (with PMIC_WRAP API) VCN28_ON_CTRL  0x041C[14] 1'b1*/
			mt6325_upmu_set_rg_vcn28_on_ctrl(1);
			/*2.2.turn on VCN28 LDO (with PMIC_WRAP API)"  RG_VCN28_EN"	 0x041C[12] 1'b1	*/
			hwPowerOn(MT6325_POWER_LDO_VCN28, VOL_2800, "wcn_drv"); 
		}
#endif

		/*3.assert CONNSYS CPU SW reset  0x10007018	"[12]=1'b1  [31:24]=8'h88 (key)"*/
		CONSYS_REG_WRITE(CONSYS_CPU_SW_RST_REG, (CONSYS_REG_READ(CONSYS_CPU_SW_RST_REG) | CONSYS_CPU_SW_RST_BIT | CONSYS_CPU_SW_RST_CTRL_KEY));
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_CPU_SW_RST_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_CPU_SW_RST_REG));
		/*turn on top clock gating enable	TOP_CLKCG_CLR	0x10000084[26]	1'b1	*/
		CONSYS_REG_WRITE(CONSYS_TOP_CLKCG_CLR_REG,CONSYS_REG_READ(CONSYS_TOP_CLKCG_CLR_REG) | CONSYS_TOP_CLKCG_BIT);
		/*turn on SPM clock gating enable	PWRON_CONFG_EN	0x10006000	32'h0b160001	*/
		CONSYS_REG_WRITE(CONSYS_PWRON_CONFG_EN_REG, CONSYS_PWRON_CONFG_EN_VALUE);		

#if PWR_ON_OFF_API_AVALIABLE
		iRet = conn_power_on(); //consult clkmgr owner
		if(iRet)
		{
			WMT_PLAT_ERR_FUNC("conn_power_on fail(%d)\n",iRet);
		}else
		{
			WMT_PLAT_INFO_FUNC("conn_power_on ok\n");
		}
#else
		/*2.write conn_top1_pwr_on=1, power on conn_top1 (MT6572 with the same addr)	conn_spm_pwr_on 0x10006280 [2]	1'b1		*/			
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) | CONSYS_SPM_PWR_ON_BIT);
		/*3.read conn_top1_pwr_on_ack =1, power on ack ready (MT6572 with the same addr)	pwr_conn_ack	0x1000660C [1]			*/		
		while (0 == (CONSYS_PWR_ON_ACK_BIT & CONSYS_REG_READ(CONSYS_PWR_CONN_ACK_REG)));
		/*4.write conn_top1_mem_PD=0, power on MCU memory (MT6572 with the same addr)	sram_conn_pd	0x10006280 [8]	1'b1	*/		
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) | CONSYS_SRAM_CONN_PD_BIT);
		/*5.write conn_top1_pwr_on_s=1, power on conn_top1 (MT6572 with the same addr)	conn_spm_pwr_on_s	0x10006280 [3]	1'b1	*/					
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) | CONSYS_SPM_PWR_ON_S_BIT);
		/*6.write conn_clk_dis=0, enable connsys clock	(MT6572 with the same addr) conn_clk_dis	0x10006280 [4]	1'b0				*/		
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) & ~CONSYS_CLK_CTRL_BIT);
		/*7.wait 1us	*/	
		udelay(1);
		/*8.read conn_top1_pwr_on_ack_s =1, power on ack ready (MT6572 with the same addr)	pwr_conn_ack_s	0x10006610 [1]	*/						
		while (0 == (CONSYS_PWR_CONN_ACK_S_BIT & CONSYS_REG_READ(CONSYS_PWR_CONN_ACK_S_REG)));
		/*9.release connsys ISO, conn_top1_iso_en=0 (MT6572 with the same addr) conn_spm_pwr_iso	0x10006280 [1]	1'b0		*/				
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) & ~CONSYS_SPM_PWR_ISO_S_BIT);
		/*10.release SW reset of connsys, conn_ap_sw_rst_b=1 (MT6572 with the same addr) conn_spm_pwr_rst_b 0x10006280[0]	1'b1	*/					
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) | CONSYS_SPM_PWR_RST_BIT);
#endif

		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_PWR_CONN_ACK_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_PWR_CONN_ACK_REG));
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_PWR_CONN_ACK_S_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_PWR_CONN_ACK_S_REG));
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_TOP1_PWR_CTRL_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG));
		/*11.26M is ready now, delay 10us for mem_pd de-assert*/
		udelay(10);
		/*enable AP bus clock : connmcu_bus_pd  API: enable_clock() ++??*/
		
		/*12.poll CONNSYS CHIP ID until 6752 is returned		0x18070008	32'h6752 */
		while (retry-- > 0)
		{
			WMT_PLAT_DBG_FUNC("CONSYS_CHIP_ID_REG(0x%08x)",CONSYS_REG_READ(CONSYS_CHIP_ID_REG));
			consysHwChipId = CONSYS_REG_READ(CONSYS_CHIP_ID_REG);
			if((consysHwChipId == 0x6752) || (consysHwChipId == 0x6582) || (consysHwChipId == 0x6572) || (consysHwChipId == 0x6571))
			{
				WMT_PLAT_INFO_FUNC("retry(%d)consys chipId(0x%08x)\n", retry,consysHwChipId);
				break;
			}
			msleep(20);
		}

		/*13.{default no need}update ROMDEL/PATCH RAM DELSEL if needed		0x18070114	"[3:0] ROM DELSEL [7:$] RAM4Kx32 DELSEL"	??*/
		
		/*14.write 1 to conn_mcu_confg ACR[1] if real speed MBIST (default write "1")	ACR	0x18070110[18]	1'b1*/
		/*if this bit is 0, HW will do memory auto test under low CPU frequence (26M Hz)*/
		/*if this bit is 0, HW will do memory auto test under high CPU frequence(138M Hz) inclulding low CPU frequence*/
		CONSYS_REG_WRITE(CONSYS_MCU_CFG_ACR_REG, CONSYS_REG_READ(CONSYS_MCU_CFG_ACR_REG) | CONSYS_MCU_CFG_ACR_MBIST_BIT);

		/*update ANA_WBG(AFE) CR. AFE setting file:  MT6752_AFE_SW_patch_REG_xxxx.xlsx ??"		AP Offset = 0x180B2000	??*/

#if 0
		/*15.{default no need, Analog HW will inform if this need to be update or not 1 week after IC sample back}
		update ANA_WBG(AFE) CR if needed, CONSYS_AFE_REG */
		CONSYS_REG_WRITE(CONSYS_AFE_REG_DIG_RCK_01,CONSYS_AFE_REG_DIG_RCK_01_VALUE);
		CONSYS_REG_WRITE(CONSYS_AFE_REG_WBG_PLL_02,CONSYS_AFE_REG_WBG_PLL_02_VALUE);
		CONSYS_REG_WRITE(CONSYS_AFE_REG_WBG_WB_TX_01,CONSYS_AFE_REG_WBG_WB_TX_01_VALUE);
#endif
		/*16.deassert CONNSYS CPU SW reset (need to check MT6752)		 0x10007018	"[12]=1'b0 [31:24] =8'h88 (key)"*/
		CONSYS_REG_WRITE(CONSYS_CPU_SW_RST_REG,
			(CONSYS_REG_READ(CONSYS_CPU_SW_RST_REG) & ~CONSYS_CPU_SW_RST_BIT) | CONSYS_CPU_SW_RST_CTRL_KEY);

		msleep(5);

	}else{

#if PWR_ON_OFF_API_AVALIABLE

		WMT_PLAT_INFO_FUNC("\n conn_power_off begin\n");
		/*power off connsys by API (MT6582, MT6572 are different) API: conn_power_off()	*/
		iRet = conn_power_off();//consult clkmgr owner 
		WMT_PLAT_INFO_FUNC("\n conn_power_off end\n");
		if(iRet)
		{
			WMT_PLAT_ERR_FUNC("conn_power_off fail(%d)\n",iRet);
		}else
		{
			WMT_PLAT_INFO_FUNC("conn_power_off ok\n");
		}
#else
		/*assert SW reset of connsys, conn_ap_sw_rst_b=0 (MT6572 with the same addr)		0x10006280[0]	1'b0		*/		
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) & ~CONSYS_SPM_PWR_RST_BIT);
		/*release connsys ISO, conn_top1_iso_en=1 (MT6572 with the same addr)	0x10006280 [1]	1'b1	*/
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) | CONSYS_SPM_PWR_ISO_S_BIT);
		/*DA_WBG_EN_XBUF=0		*/										
		/*wait 1us	*/
		udelay(1);
		/*write conn_top1_mem_PD=1, power off MCU memory (MT6572 with the same addr)		0x10006280 [8]	1'b0	  */	
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) & ~CONSYS_SRAM_CONN_PD_BIT);
		/*write conn_top1_pwr_on=0, power off conn_top1 (MT6572 with the same addr) 	0x10006280 [3:2]	2'b00 */	
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) & ~(CONSYS_SPM_PWR_ON_BIT | CONSYS_SPM_PWR_ON_S_BIT));
		/*write conn_clk_dis=1, disable connsys clock  (MT6572 with the same addr)	conn_clk_dis	0x10006280 [4]	1'b1	*/
		CONSYS_REG_WRITE(CONSYS_TOP1_PWR_CTRL_REG, CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG) |CONSYS_CLK_CTRL_BIT);
#endif

#if CONSYS_PMIC_CTRL_ENABLE

		/*set VCN_28 to SW control mode (with PMIC_WRAP API) "1. VCN28_ON_CTRL "0x041C[14] 1'b0*/
		mt6325_upmu_set_rg_vcn28_on_ctrl(0);
		/*turn off VCN28 LDO (with PMIC_WRAP API)"	 2. RG_VCN28_EN"  0x041C[12]" 1'b0*/
		hwPowerDown(MT6325_POWER_LDO_VCN28, "wcn_drv");

		/*AP power off MT6625L VCN_1V8 LDO (with PMIC_WRAP API)  "0x0512[1]1'b0  0x0512[14]" 1'b0*/
		mt6325_upmu_set_rg_vcn18_mode_set(0);
		hwPowerDown(MT6325_POWER_LDO_VCN18, "wcn_drv"); 

#endif

	}
	WMT_PLAT_INFO_FUNC("CONSYS-HW-REG-CTRL(0x%08x),finish\n",on);
	return 0;
}
#endif

INT32
mtk_wcn_consys_hw_gpio_ctrl (UINT32 on)
{
    INT32 iRet = 0;
	WMT_PLAT_INFO_FUNC("CONSYS-HW-GPIO-CTRL(0x%08x), start\n",on);

	if(on)
	{

		/*if external modem used,GPS_SYNC still needed to control*/
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_INIT);
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_INIT);

	    iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP,PIN_STA_INIT);

	    /*set EINT< -ommited-> move this to WMT-IC module, where common sdio interface will be identified and do proper operation*/
	    // TODO: [FixMe][GeorgeKuo] double check if BGF_INT is implemented ok
	    //iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_MUX);
	    iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_INIT);
		iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
	    WMT_PLAT_INFO_FUNC("CONSYS-HW, BGF IRQ registered and disabled \n");

	}else{

	    /* set bgf eint/all eint to deinit state, namely input low state*/
	    iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
		iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_DEINIT);
	    WMT_PLAT_INFO_FUNC("CONSYS-HW, BGF IRQ unregistered and disabled\n");
	    //iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_DEINIT);

		/*if external modem used,GPS_SYNC still needed to control*/
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_DEINIT);
	   	iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP,PIN_STA_DEINIT);
	    /* deinit gps_lna*/
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_DEINIT);
	    

	}
	WMT_PLAT_INFO_FUNC("CONSYS-HW-GPIO-CTRL(0x%08x), finish\n",on);
    return iRet;

}

INT32 mtk_wcn_consys_hw_pwr_on(UINT32 co_clock_en)
{
	INT32 iRet = 0;

	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-ON, start\n");

	iRet += mtk_wcn_consys_hw_reg_ctrl(1,co_clock_en);
	iRet += mtk_wcn_consys_hw_gpio_ctrl(1);
#if CONSYS_ENALBE_SET_JTAG
	if(gJtagCtrl)
	{
		mtk_wcn_consys_jtag_set_for_mcu();
	}
#endif
	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-ON, finish(%d)\n",iRet);
	return iRet;
}

INT32 mtk_wcn_consys_hw_pwr_off (VOID)
{
	INT32 iRet = 0;

	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-OFF, start\n");

	iRet += mtk_wcn_consys_hw_reg_ctrl(0,0);
	iRet += mtk_wcn_consys_hw_gpio_ctrl(0);

	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-OFF, finish(%d)\n",iRet);
	return iRet;
}


INT32
mtk_wcn_consys_hw_rst (UINT32 co_clock_en)
{
    INT32 iRet = 0;
    WMT_PLAT_INFO_FUNC("CONSYS-HW, hw_rst start, eirq should be disabled before this step\n");

    /*1. do whole hw power off flow*/
    iRet += mtk_wcn_consys_hw_reg_ctrl(0,co_clock_en);

    /*2. do whole hw power on flow*/
    iRet += mtk_wcn_consys_hw_reg_ctrl(1,co_clock_en);

    WMT_PLAT_INFO_FUNC("CONSYS-HW, hw_rst finish, eirq should be enabled after this step\n");
    return iRet;
}
#ifdef BT_WIFI_SHARE_V33
INT32 mtk_wcn_consys_hw_bt_paldo_ctrl(UINT32 enable)
{
	//spin_lock_irqsave(&gBtWifiV33.lock,gBtWifiV33.flags);
	if(enable){
		if(1 == gBtWifiV33.counter) {
			gBtWifiV33.counter++;
			WMT_PLAT_DBG_FUNC("V33 has been enabled,counter(%d)\n",gBtWifiV33.counter);
		} else if (2 == gBtWifiV33.counter) {
			WMT_PLAT_DBG_FUNC("V33 has been enabled,counter(%d)\n",gBtWifiV33.counter);
		} else {
#if CONSYS_PMIC_CTRL_ENABLE 
			/*do BT PMIC on,depenency PMIC API ready*/
			/*switch BT PALDO control from SW mode to HW mode:0x416[5]-->0x1*/
			/* VOL_DEFAULT, VOL_3300, VOL_3400, VOL_3500, VOL_3600*/
			hwPowerOn(MT6325_POWER_LDO_VCN33, VOL_3300, "wcn_drv"); 
			mt6325_upmu_set_rg_vcn33_on_ctrl(1); 
#endif
			WMT_PLAT_INFO_FUNC("WMT do BT/WIFI v3.3 on\n");
			gBtWifiV33.counter++;
		}

	}else{
		if(1 == gBtWifiV33.counter) {
			/*do BT PMIC off*/
			/*switch BT PALDO control from HW mode to SW mode:0x416[5]-->0x0*/
#if CONSYS_PMIC_CTRL_ENABLE 
			mt6325_upmu_set_rg_vcn33_on_ctrl(0);
			hwPowerDown(MT6325_POWER_LDO_VCN33,"wcn_drv");
#endif
			WMT_PLAT_INFO_FUNC("WMT do BT/WIFI v3.3 off\n");
			gBtWifiV33.counter--;
		} else if(2 == gBtWifiV33.counter){
			gBtWifiV33.counter--;
			WMT_PLAT_DBG_FUNC("V33 no need disabled,counter(%d)\n",gBtWifiV33.counter);
		} else {
			WMT_PLAT_DBG_FUNC("V33 has been disabled,counter(%d)\n",gBtWifiV33.counter);			
		}

	}
	//spin_unlock_irqrestore(&gBtWifiV33.lock,gBtWifiV33.flags);
	return 0;
}

INT32 mtk_wcn_consys_hw_wifi_paldo_ctrl(UINT32 enable)
{
	mtk_wcn_consys_hw_bt_paldo_ctrl(enable);
	return 0;
}

#else
INT32 mtk_wcn_consys_hw_bt_paldo_ctrl(UINT32 enable)
{
	if(enable){
		/*do BT PMIC on,depenency PMIC API ready*/
		/*switch BT PALDO control from SW mode to HW mode:0x416[5]-->0x1*/
#if CONSYS_PMIC_CTRL_ENABLE 
		/* VOL_DEFAULT, VOL_3300, VOL_3400, VOL_3500, VOL_3600*/
		hwPowerOn(MT6325_POWER_LDO_VCN33, VOL_3300, "wcn_drv"); 
		mt6325_upmu_set_rg_vcn33_on_ctrl(1); 
		
#endif
		WMT_PLAT_INFO_FUNC("WMT do BT PMIC on\n");
	}else{
		/*do BT PMIC off*/
		/*switch BT PALDO control from HW mode to SW mode:0x416[5]-->0x0*/
#if CONSYS_PMIC_CTRL_ENABLE 
		mt6325_upmu_set_rg_vcn33_on_ctrl(0);
		hwPowerDown(MT6325_POWER_LDO_VCN33,"wcn_drv");
#endif
		WMT_PLAT_INFO_FUNC("WMT do BT PMIC off\n");
	}
	return 0;
}

INT32 mtk_wcn_consys_hw_wifi_paldo_ctrl(UINT32 enable)
{
	if(enable){
		/*do WIFI PMIC on,depenency PMIC API ready*/
		/*switch WIFI PALDO control from SW mode to HW mode:0x418[14]-->0x1*/
#if CONSYS_PMIC_CTRL_ENABLE 
		hwPowerOn(MT6325_POWER_LDO_VCN33, VOL_3300, "wcn_drv"); 
		mt6325_upmu_set_rg_vcn33_on_ctrl(1);
#endif
		WMT_PLAT_INFO_FUNC("WMT do WIFI PMIC on\n");
	}else{
		/*do WIFI PMIC off*/
		/*switch WIFI PALDO control from HW mode to SW mode:0x418[14]-->0x0*/
#if CONSYS_PMIC_CTRL_ENABLE 
		mt6325_upmu_set_rg_vcn33_on_ctrl(0);
		hwPowerDown(MT6325_POWER_LDO_VCN33,"wcn_drv");
#endif
		WMT_PLAT_INFO_FUNC("WMT do WIFI PMIC off\n");
	}
	return 0;
}

#endif
INT32 mtk_wcn_consys_hw_vcn28_ctrl(UINT32 enable)
{
	if(enable){
		/*in co-clock mode,need to turn on vcn28 when fm on*/
#if CONSYS_PMIC_CTRL_ENABLE 
		hwPowerOn(MT6325_POWER_LDO_VCN28, VOL_2800, "wcn_drv");
#endif
		WMT_PLAT_INFO_FUNC("turn on vcn28 for fm/gps usage in co-clock mode\n");
	}else{
		/*in co-clock mode,need to turn off vcn28 when fm off*/
#if CONSYS_PMIC_CTRL_ENABLE 
		hwPowerDown(MT6325_POWER_LDO_VCN28, "wcn_drv");
#endif
		WMT_PLAT_INFO_FUNC("turn off vcn28 for fm/gps usage in co-clock mode\n");
	}
	return 0;
}

INT32 mtk_wcn_consys_hw_state_show(VOID)
{
	return 0;
}

#if CONSYS_WMT_REG_SUSPEND_CB_ENABLE
UINT32 mtk_wcn_consys_hw_osc_en_ctrl(UINT32 en)
{
	if(en)
	{
		WMT_PLAT_INFO_FUNC("enable consys sleep mode(turn off 26M)\n");
		CONSYS_REG_WRITE(CONSYS_AP2CONN_OSC_EN_REG, CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG) & ~CONSYS_AP2CONN_OSC_EN_BIT);
	}else
	{
		WMT_PLAT_INFO_FUNC("disable consys sleep mode\n");
		CONSYS_REG_WRITE(CONSYS_AP2CONN_OSC_EN_REG, CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG) | CONSYS_AP2CONN_OSC_EN_BIT);	
	}

	WMT_PLAT_INFO_FUNC("dump CONSYS_AP2CONN_OSC_EN_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG));

	return 0;
}
#endif

INT32 mtk_wcn_consys_hw_restore(struct device *device)
{
	UINT32 addrPhy = 0;
	
	if(gConEmiPhyBase)
	{
	
#if CONSYS_EMI_MPU_SETTING
		/*set MPU for EMI share Memory*/
		WMT_PLAT_INFO_FUNC("setting MPU for EMI share memory\n");

		emi_mpu_set_region_protection(gConEmiPhyBase + SZ_1M/2,
			gConEmiPhyBase + SZ_1M - 1,
			13,
			SET_ACCESS_PERMISSON(FORBIDDEN,FORBIDDEN,FORBIDDEN,FORBIDDEN,FORBIDDEN,NO_PROTECTION,FORBIDDEN,NO_PROTECTION));

#endif
		/*consys to ap emi remapping register:10001310, cal remapping address*/
		addrPhy = (gConEmiPhyBase & 0xFFF00000) >> 20;

		/*enable consys to ap emi remapping bit12*/
		addrPhy -= 0x400;/*Gavin ??*/
		addrPhy = addrPhy | 0x1000;

		CONSYS_REG_WRITE(conn_reg.topckgen_base + CONSYS_EMI_MAPPING_OFFSET, \
							CONSYS_REG_READ(conn_reg.topckgen_base + CONSYS_EMI_MAPPING_OFFSET) | addrPhy);

		WMT_PLAT_INFO_FUNC("CONSYS_EMI_MAPPING dump in restore cb(0x%08x)\n",CONSYS_REG_READ(conn_reg.topckgen_base + CONSYS_EMI_MAPPING_OFFSET));

#if 1
		pEmibaseaddr = ioremap_nocache(gConEmiPhyBase + SZ_1M/2, CONSYS_EMI_MEM_SIZE);
#else
		pEmibaseaddr = ioremap_nocache(CONSYS_EMI_AP_PHY_BASE,CONSYS_EMI_MEM_SIZE);
#endif
		if(pEmibaseaddr)
		{
			WMT_PLAT_INFO_FUNC("EMI mapping OK(0x%p)\n",pEmibaseaddr);
			memset(pEmibaseaddr,0,CONSYS_EMI_MEM_SIZE);
		}else{
			WMT_PLAT_ERR_FUNC("EMI mapping fail\n");
		}
	}
	else
	{
		WMT_PLAT_ERR_FUNC("consys emi memory address gConEmiPhyBase invalid\n");
	}

	return 0;
}

VOID __init mtk_wcn_consys_memory_reserve(VOID)
{
	gConEmiPhyBase = arm_memblock_steal(SZ_2M,SZ_2M);

	if(gConEmiPhyBase)
	{
		WMT_PLAT_INFO_FUNC("memblock done: 0x%x\n",gConEmiPhyBase);
	}else
	{
		WMT_PLAT_ERR_FUNC("memblock fail\n");
	}
}

INT32 mtk_wcn_consys_hw_init()
{

	INT32 iRet = -1;
	UINT32 addrPhy = 0;
	struct device_node *node = NULL;

 	node = of_find_compatible_node(NULL, NULL, "mediatek,CONSYS");
	if(node){
		/* registers base address*/
		conn_reg.mcu_base = (unsigned long)of_iomap(node, 0);
		WMT_PLAT_INFO_FUNC("Get mcu register base(0x%lx)\n",conn_reg.mcu_base);
		conn_reg.ap_rgu_base = (unsigned long)of_iomap(node, 1);
		WMT_PLAT_INFO_FUNC("Get ap_rgu register base(0x%lx)\n",conn_reg.ap_rgu_base);
		conn_reg.topckgen_base = (unsigned long)of_iomap(node, 2);
		WMT_PLAT_INFO_FUNC("Get topckgen register base(0x%lx)\n",conn_reg.topckgen_base);
		conn_reg.spm_base = (unsigned long)of_iomap(node, 3);
		WMT_PLAT_INFO_FUNC("Get spm register base(0x%lx)\n",conn_reg.spm_base);
	} else {
		WMT_PLAT_ERR_FUNC("[%s] can't find CONSYS compatible node\n",__func__);
		return iRet;
	}
	
	if(gConEmiPhyBase)
	{
#if CONSYS_EMI_MPU_SETTING
		/*set MPU for EMI share Memory*/
		WMT_PLAT_INFO_FUNC("setting MPU for EMI share memory\n");

		emi_mpu_set_region_protection(gConEmiPhyBase + SZ_1M/2,
			gConEmiPhyBase + SZ_1M - 1,
			13,
			SET_ACCESS_PERMISSON(FORBIDDEN,FORBIDDEN,FORBIDDEN,FORBIDDEN,FORBIDDEN,NO_PROTECTION,FORBIDDEN,NO_PROTECTION));

#endif
		WMT_PLAT_INFO_FUNC("get consys start phy address(0x%x)\n",gConEmiPhyBase);

		/*consys to ap emi remapping register:10001310, cal remapping address*/
		addrPhy = (gConEmiPhyBase & 0xFFF00000) >> 20;

		/*enable consys to ap emi remapping bit12*/
		addrPhy -= 0x400;/*Gavin ??*/
		addrPhy = addrPhy | 0x1000;

		CONSYS_REG_WRITE(conn_reg.topckgen_base + CONSYS_EMI_MAPPING_OFFSET, \
							CONSYS_REG_READ(conn_reg.topckgen_base + CONSYS_EMI_MAPPING_OFFSET) | addrPhy);

		WMT_PLAT_INFO_FUNC("CONSYS_EMI_MAPPING dump(0x%08x)\n",CONSYS_REG_READ(conn_reg.topckgen_base + CONSYS_EMI_MAPPING_OFFSET));

#if 1
		pEmibaseaddr = ioremap_nocache(gConEmiPhyBase + SZ_1M/2, CONSYS_EMI_MEM_SIZE);
#else
		pEmibaseaddr = ioremap_nocache(CONSYS_EMI_AP_PHY_BASE,CONSYS_EMI_MEM_SIZE);
#endif
		//pEmibaseaddr = ioremap_nocache(0x80090400,270*KBYTE);
		if(pEmibaseaddr)
		{
			WMT_PLAT_INFO_FUNC("EMI mapping OK(0x%p)\n",pEmibaseaddr);
			memset(pEmibaseaddr,0,CONSYS_EMI_MEM_SIZE);
			iRet = 0;
		}else{
			WMT_PLAT_ERR_FUNC("EMI mapping fail\n");
		}
	}
	else
	{
		WMT_PLAT_ERR_FUNC("consys emi memory address gConEmiPhyBase invalid\n");
	}

	WMT_PLAT_INFO_FUNC("register connsys restore cb for complying with IPOH function\n");
	register_swsusp_restore_noirq_func(ID_M_CONNSYS,mtk_wcn_consys_hw_restore,NULL);

	return iRet;

}

INT32 mtk_wcn_consys_hw_deinit()
{
	if(pEmibaseaddr)
	{
		iounmap(pEmibaseaddr);
		pEmibaseaddr = NULL;
	}
	unregister_swsusp_restore_noirq_func(ID_M_CONNSYS);
	return 0;
}


UINT8  *mtk_wcn_consys_emi_virt_addr_get(UINT32 ctrl_state_offset)
{
	UINT8 *p_virtual_addr = NULL;

	if(!pEmibaseaddr)
	{
		WMT_PLAT_ERR_FUNC("EMI base address is NULL\n");
		return NULL;
	}
	WMT_PLAT_DBG_FUNC("ctrl_state_offset(%08x)\n",ctrl_state_offset);
	p_virtual_addr = pEmibaseaddr + ctrl_state_offset;

	return p_virtual_addr;
}

UINT32 mtk_wcn_consys_soc_chipid()
{
	return PLATFORM_SOC_CHIP;
}

