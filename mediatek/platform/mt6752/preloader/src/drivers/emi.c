// 6595_DDR.cpp : Defines the entry point for the console application.
//

#include <emi.h>
#include <typedefs.h>
#include <stdlib.h>
#include "dramc_common.h"
#include "dramc_register.h"
#include "dramc_pi_api.h"
#include "emi.h"
#include "platform.h"
#include "emi_hw.h"
#include "wdt.h"

#define DRAM_BASE 0x40000000ULL
 
int DQS_PRINT = 1;
int emi_setting_index = -1;
DRAMC_CTX_T *psCurrDramCtx;
EMI_SETTINGS *default_emi_setting;

DRAMC_CTX_T DramCtx_LPDDR3 =
{
	CHANNEL_A,		// DRAM_CHANNEL
	TYPE_LPDDR3,	// DRAM_DRAM_TYPE_T
#if fcFOR_CHIP_ID == fcK2
	PACKAGE_SBS,				// DRAM_PACKAGE_T
#endif
	DATA_WIDTH_32BIT,		// DRAM_DATA_WIDTH_T
	DEFAULT_TEST2_1_CAL, 	// test2_1;
	DEFAULT_TEST2_2_CAL,	// test2_2;
	TEST_XTALK_PATTERN,	// test_pattern;
#if defined(DUAL_FREQ_K) & !defined(FREQ_ADJUST)
	DUAL_FREQ_HIGH,
#else
	#if defined (DDR_667)
        333,
        #elif defined (DDR_800)
        400,
        #elif defined (DDR_1066)
	533,
	#elif defined (DDR_1160)	
	580,
        #elif defined (DDR_1333)
	666,
        #elif defined (DDR_1600)
	800,
        #elif defined (DDR_1780)
	890,
        #elif defined (DDR_1866)
	933,
        #elif defined (DDR_2000)
	1000,
        #elif defined (DDR_2133)
	1066,
	#elif defined (DDR_1420)
	710,
	#elif defined (DDR_2400)
	1200,
#else		
	890,
#endif	
#endif
	
	533,			// frequency_low;
	DISABLE,		// fglow_freq_write_en;
	DISABLE,	// ssc_en;
	DISABLE		// en_4bitMux;
};

DRAMC_CTX_T DramCtx_LPDDR2 =
{
	CHANNEL_A,		// DRAM_CHANNEL
	TYPE_LPDDR2,	// DRAM_DRAM_TYPE_T
	PACKAGE_SBS,	// DRAM_PACKAGE_T
	DATA_WIDTH_32BIT,		// DRAM_DATA_WIDTH_T
	DEFAULT_TEST2_1_CAL, 	// test2_1;
	DEFAULT_TEST2_2_CAL,	// test2_2;
	TEST_XTALK_PATTERN,	// test_pattern;
#if defined(DUAL_FREQ_K) & !defined(FREQ_ADJUST)
	DUAL_FREQ_HIGH,
#else
	#ifdef DDR_1066
	533,
	#elif defined (DDR_667)
	333,
	#else
	400,
	#endif
#endif
	533,			// frequency_low;
	DISABLE,		// fglow_freq_write_en;
	DISABLE,	// ssc_en;
	DISABLE		// en_4bitMux;
};

DRAMC_CTX_T DramCtx_PCDDR3 =
{
	CHANNEL_A,		// DRAM_CHANNEL
	TYPE_PCDDR3,		// DRAM_DRAM_TYPE_T
	PACKAGE_SBS,	// DRAM_PACKAGE_T
	DATA_WIDTH_32BIT,		// DRAM_DATA_WIDTH_T
	DEFAULT_TEST2_1_CAL, 	// test2_1;
	DEFAULT_TEST2_2_CAL,	// test2_2;
	TEST_XTALK_PATTERN,	// test_pattern; Audio or Xtalk.
	900, // frequency;	
	533,			// frequency_low;
	ENABLE,		// fglow_freq_write_en;
	DISABLE,	// ssc_en;
	DISABLE		// en_4bitMux;
};

#ifndef COMBO_MCP
#ifdef DDRTYPE_LPDDR2
#define    EMI_CONA_VAL     LPDDR2_EMI_CONA
#endif
#ifdef DDRTYPE_LPDDR3
#define    EMI_CONA_VAL     LPDDR3_EMI_CONA
#endif

#ifdef LPDDR3_EMI_CONH
    #define    EMI_CONH_VAL     LPDDR3_EMI_CONH
#else
    #define    EMI_CONH_VAL     0
#endif

#endif //ifndef COMBO_MCP

static int enable_combo_dis = 0;

#if fcFOR_CHIP_ID == fcK2

#define EMI_CONA		(EMI_APB_BASE + 0x0)
#define EMI_CONB		(EMI_APB_BASE + 0x8)
#define EMI_CONC		(EMI_APB_BASE + 0xc)
#define EMI_COND		(EMI_APB_BASE + 0x18)
#define EMI_CONE		(EMI_APB_BASE + 0x20)
#define EMI_CONG		(EMI_APB_BASE + 0x30)
#define EMI_CONH		(EMI_APB_BASE + 0x38)
#define EMI_CONI		(EMI_APB_BASE + 0x40)
#define EMI_MDCT		(EMI_APB_BASE + 0x78)
#define EMI_TESTB	(EMI_APB_BASE + 0xe8)
#define EMI_TESTC	(EMI_APB_BASE + 0xf0)
#define EMI_TESTD	(EMI_APB_BASE + 0xf8)
#define EMI_TEST0	(EMI_APB_BASE + 0xd0)
#define EMI_TEST1	(EMI_APB_BASE + 0xd8)
#define EMI_ARBI		(EMI_APB_BASE + 0x140)
#define EMI_ARBI_2ND		(EMI_APB_BASE + 0x144)
#define EMI_ARBJ		(EMI_APB_BASE + 0x148)
#define EMI_ARBJ_2ND		(EMI_APB_BASE + 0x14c)
#define EMI_ARBK		(EMI_APB_BASE + 0x150)
#define EMI_ARBK_2ND	(EMI_APB_BASE + 0x154)
#define EMI_SLCT		(EMI_APB_BASE + 0x158)
#define EMI_BMEN		(EMI_APB_BASE + 0x400)	
#define EMI_ARBA		(EMI_APB_BASE + 0x100)
#define EMI_ARBB		(EMI_APB_BASE + 0x108)
#define EMI_ARBC		(EMI_APB_BASE + 0x110)
#define EMI_ARBD		(EMI_APB_BASE + 0x118)
#define EMI_ARBE		(EMI_APB_BASE + 0x120)
#define EMI_ARBF		(EMI_APB_BASE + 0x128)
#define EMI_ARBG		(EMI_APB_BASE + 0x130)
#define EMI_ARBH		(EMI_APB_BASE + 0x138)
#define EMI_CONM		(EMI_APB_BASE + 0x60)

void EMI_Init(DRAMC_CTX_T *p)
{
#ifdef COMBO_MCP
    EMI_SETTINGS *emi_set;
    
    if(emi_setting_index == -1)
        emi_set = default_emi_setting;
    else
        emi_set = &emi_settings[emi_setting_index];

	*(volatile unsigned *)EMI_CONA = emi_set->EMI_CONA_VAL;
	*(volatile unsigned *)EMI_CONH = emi_set->EMI_CONH_VAL | 0x00000003;
#else    
	if (p->dram_type == TYPE_LPDDR3) 
	{
		*(volatile unsigned *)EMI_CONA = LPDDR3_EMI_CONA;
	}
	else
	{
		*(volatile unsigned *)EMI_CONA = LPDDR2_EMI_CONA;
	}

	#ifdef LPDDR3_EMI_CONH
	*(volatile unsigned *)EMI_CONH = LPDDR3_EMI_CONH;
	#else
	*(volatile unsigned *)EMI_CONH =	0x00000003;
	#endif
#endif		
	*(volatile unsigned *)EMI_CONB =	0x17283544;
	*(volatile unsigned *)EMI_CONC =	0x0a1a0b1a;
	*(volatile unsigned *)EMI_COND =	0x00000000;
	*(volatile unsigned *)EMI_CONE =	0xffff0848;
	*(volatile unsigned *)EMI_CONG =	0x2b2b2a38;
//	*(volatile unsigned *)EMI_CONH =	0x00000003;
	*(volatile unsigned *)EMI_CONI =	0x80808807;
	*(volatile unsigned *)EMI_MDCT =	0x45340e17;
	*(volatile unsigned *)EMI_TESTB = 0x00000027;
	*(volatile unsigned *)EMI_TESTC = 0x38460000;
 	*(volatile unsigned *)EMI_TESTD = 0x00000000;
	*(volatile unsigned *)EMI_TEST0 = 0xcccccccc;
	*(volatile unsigned *)EMI_TEST1 = 0xcccccccc;
	*(volatile unsigned *)EMI_ARBI = 0x20406188;
	*(volatile unsigned *)EMI_ARBI_2ND =	0x20406188;
	*(volatile unsigned *)EMI_ARBJ = 0x9719595e;
	*(volatile unsigned *)EMI_ARBJ_2ND =	0x9719595e;
	*(volatile unsigned *)EMI_ARBK = 0x64f3fc79;
	*(volatile unsigned *)EMI_ARBK_2ND = 0x64f3fc79;
	*(volatile unsigned *)EMI_SLCT =	0xff02ff00;
	*(volatile unsigned *)EMI_BMEN =	0x00ff0001;
	*(volatile unsigned *)EMI_ARBA =	0xa0a07c4b;
	*(volatile unsigned *)EMI_ARBB =	0xffff7041;
	*(volatile unsigned *)EMI_ARBC =	0xffff7041;
	*(volatile unsigned *)EMI_ARBD =	0x40047045;
	*(volatile unsigned *)EMI_ARBE =	0x18306049;
	*(volatile unsigned *)EMI_ARBF =	0xa0a070dd;
	*(volatile unsigned *)EMI_ARBG =	0xffff7046;
	*(volatile unsigned *)EMI_ARBH =	0x400a7042;
	*(volatile unsigned *)EMI_CONM =	0x000006b8;
}

#endif

int mt_get_freq_setting(DRAMC_CTX_T *p)
{
    int freq;

#if defined(DUAL_FREQ_K)
    if (p->dram_type == TYPE_LPDDR3)
	    freq = 800; //DDR_1600
	  else if(p->dram_type == TYPE_LPDDR2)
	    freq = 533; //DDR_1066
#else    
    #if defined (DDR_667)
      freq = 333;
    #elif defined (DDR_800)
      freq = 400;
    #elif defined (DDR_1066)
    	freq = 533;
    #elif defined (DDR_1333)
    	freq = 666;
    #elif defined (DDR_1600)
    	freq = 800;
    #elif defined (DDR_1780)
    	freq = 890;
    #elif defined (DDR_1866)
    	freq = 933;
    #else		
    	freq = 800;
    #endif
    
    if((p->dram_type == TYPE_LPDDR2) && (freq > 533))
      freq = 533;
      	
#endif
    return freq;
}

void CHA_HWGW_Print(DRAMC_CTX_T *p)
	{
	static U8 LowFreq_Min_R0_DQS[4] = {0xff, 0xff, 0xff, 0xff};
	static U8 LowFreq_Max_R0_DQS[4] = {0x00, 0x00, 0x00, 0x00};
	static U8 HighFreq_Min_R0_DQS[4] = {0xff, 0xff, 0xff, 0xff};
	static U8 HighFreq_Max_R0_DQS[4] = {0x00, 0x00, 0x00, 0x00};
	U8 ucstatus = 0, R0_DQS[4], Count;
	U32 u4value, u4value1, u4value2;

	p->channel = CHANNEL_A;
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x374), &u4value);
	R0_DQS[0] = (u4value >> 0) & 0x7f;
	R0_DQS[1] = (u4value >> 8) & 0x7f;
	R0_DQS[2] = (u4value >> 16) & 0x7f;
	R0_DQS[3] = (u4value >> 24) & 0x7f;
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x94), &u4value1);
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x840), &u4value2);

	mcSHOW_DBG_MSG(("[Channel %d]Clock=%d,Reg.94h=%xh,Reg.840h=%xh,Reg.374h=%xh\n", p->channel, p->frequency, u4value1, u4value2, u4value));
	mcSHOW_DBG_MSG(("mt_pll_post_init: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq()));

#ifdef DUAL_FREQ_TEST
	if (p->frequency == DUAL_FREQ_LOW)
	{
		for (Count=0; Count<4; Count++)
		{
			if (R0_DQS[Count] < LowFreq_Min_R0_DQS[Count])
			{
				LowFreq_Min_R0_DQS[Count] = R0_DQS[Count];
			}
			if (R0_DQS[Count]  > LowFreq_Max_R0_DQS[Count])
			{
				LowFreq_Max_R0_DQS[Count] = R0_DQS[Count];
			}
		}

		mcSHOW_DBG_MSG(("[Channel %d]Clock=%d,DQS0=(%d, %d),DQS1=(%d, %d),DQS2=(%d, %d),DQS3=(%d, %d)\n", 
			p->channel, p->frequency, 
			LowFreq_Min_R0_DQS[0], LowFreq_Max_R0_DQS[0], LowFreq_Min_R0_DQS[1], LowFreq_Max_R0_DQS[1], 
			LowFreq_Min_R0_DQS[2], LowFreq_Max_R0_DQS[2], LowFreq_Min_R0_DQS[3], LowFreq_Max_R0_DQS[3]));
	}
	else
	{
		for (Count=0; Count<4; Count++)
		{
			if (R0_DQS[Count] < HighFreq_Min_R0_DQS[Count])
			{
				HighFreq_Min_R0_DQS[Count] = R0_DQS[Count];
			}
			if (R0_DQS[Count]  > HighFreq_Max_R0_DQS[Count])
			{
				HighFreq_Max_R0_DQS[Count] = R0_DQS[Count];
			}
		}	
		mcSHOW_DBG_MSG(("[Channel %d]Clock=%d,DQS0=(%d, %d),DQS1=(%d, %d),DQS2=(%d, %d),DQS3=(%d, %d)\n", 
			p->channel, p->frequency, 
			HighFreq_Min_R0_DQS[0], HighFreq_Max_R0_DQS[0], HighFreq_Min_R0_DQS[1], HighFreq_Max_R0_DQS[1], 
			HighFreq_Min_R0_DQS[2], HighFreq_Max_R0_DQS[2], HighFreq_Min_R0_DQS[3], HighFreq_Max_R0_DQS[3]));

	}
	#else
	for (Count=0; Count<4; Count++)
	{
		if (R0_DQS[Count] < LowFreq_Min_R0_DQS[Count])
		{
			LowFreq_Min_R0_DQS[Count] = R0_DQS[Count];
		}
		if (R0_DQS[Count]  > LowFreq_Max_R0_DQS[Count])
		{
			LowFreq_Max_R0_DQS[Count] = R0_DQS[Count];
		}
	}
	mcSHOW_DBG_MSG(("[Channel %d]Clock=%d,DQS0=(%d, %d),DQS1=(%d, %d),DQS2=(%d, %d),DQS3=(%d, %d)\n", 
		p->channel, p->frequency, 
		LowFreq_Min_R0_DQS[0], LowFreq_Max_R0_DQS[0], LowFreq_Min_R0_DQS[1], LowFreq_Max_R0_DQS[1], 
		LowFreq_Min_R0_DQS[2], LowFreq_Max_R0_DQS[2], LowFreq_Min_R0_DQS[3], LowFreq_Max_R0_DQS[3]));

#endif

#ifdef TEMP_SENSOR_ENABLE
	p->channel = CHANNEL_A;
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x03B8), &u4value);
        mcSHOW_ERR_MSG(("[CHA] MRR(MR4) Reg.3B8h[10:8]=%x\n", (u4value & 0x700)>>8));
	p->channel = CHANNEL_B;
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x03B8), &u4value);
        mcSHOW_ERR_MSG(("[CHB] MRR(MR4) Reg.3B8h[10:8]=%x\n", (u4value & 0x700)>>8));
#endif	

}

static void Dump_EMIRegisters(DRAMC_CTX_T *p)
{
	U8 ucstatus = 0;
	U32 uiAddr;
	U32 u4value;

	for (uiAddr=0; uiAddr<=0x160; uiAddr+=4)
	{
		mcSHOW_DBG_MSG2(("EMI offset:%x, value:%x\n", uiAddr, *(volatile unsigned *)(EMI_APB_BASE+uiAddr)));
	}
}

void print_DBG_info(DRAMC_CTX_T *p)
{
    unsigned int addr = 0x0;
    U32 u4value;

#ifdef DDR_INIT_TIME_PROFILING    
    return;
#endif

    mcSHOW_DBG_MSG2(("EMI_CONA=%x\n",*(volatile unsigned *)(EMI_APB_BASE+0x00000000)));
    mcSHOW_DBG_MSG2(("EMI_CONH=%x\n",*(volatile unsigned *)(EMI_APB_BASE+0x00000038)));

    mcSHOW_DBG_MSG2(("=====================DBG=====================\n"));
    for(addr = 0x0; addr <= 0x690; addr +=4)
    {
        ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(addr), &u4value);
        mcSHOW_DBG_MSG2(("addr:%x, value:%x\n",addr, u4value));
    }
    mcSHOW_DBG_MSG2(("=============================================\n"));
}

#ifdef MEM_TEST

void pmic_Vcore1_adjust(int nAdjust)
{
}

void pmic_Vcore2_adjust(int nAdjust)
{
}

void pmic_voltage_read(void)
{
}

void static MemoryTest(DRAMC_CTX_T *p)
{
	int temp;
	int mem_start,len;
	unsigned int err_count = 0;
	unsigned int pass_count = 0;
	unsigned int count;
	unsigned int count2 = 1000;
	unsigned int delay = 1000;
	int reg_val;
	int rank0_col,rank1_col;
	U8 ucstatus = 0;
	U32 u4value;
	unsigned int uiStartAddr = 0x50000000;
	U32 uiCmpErr;
	U32 uiSrcAddr, uiDestAddr, uiLen, uiFixedAddr = 0x4f000000, uiReadCompareOK=1;
	U8 Voltage;

	uiSrcAddr = 0x50000000;
	uiLen = 0xff000;

	*(volatile unsigned int*)(0x10007000) = 0x22000000;
#ifdef READ_COMPARE_TEST
	for (count=uiFixedAddr; count<uiFixedAddr+0x10000; count+=4)
	{
		*(volatile unsigned int   *)(count) = count;
	}
#endif
	do {

#ifdef MEMPLL_RESET_TEST
    dbg_print("MEMPLL Reset\n");

    *(volatile unsigned int*)(0x10006000) = 0x0b160001; 
    *(volatile unsigned int*)(0x10006010) |= 0x08000000;  //(4) 0x10006010[27]=1  //Request MEMPLL reset/pdn mode 
    mcDELAY_US(1);
    *(volatile unsigned int*)(0x10006010) &= ~(0x08000000);  //(1) 0x10006010[27]=0 //Unrequest MEMPLL reset/pdn mode and wait settle (1us for reset)
    mcDELAY_US(1);                
#endif        

		if (uiDualRank) 
		{
			#ifdef SINGLE_RANK_DMA
			uiDestAddr = uiSrcAddr+0x10000000;
			#else
			#ifdef SINGLE_CHANNEL_ENABLE
			uiDestAddr = uiSrcAddr+0x30000000;
			#else
			uiDestAddr = uiSrcAddr+0x60000000;
			#endif
			#endif
			
		}
		else
		{
			uiDestAddr = uiSrcAddr+0x10000000;
		}
		

		CHA_HWGW_Print(p);
#ifdef DUAL_FREQ_TEST			
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif
		{
			// Switch to low frequency.
			p->frequency = DUAL_FREQ_LOW;
		}
		else
		{
			// Switch into high frequency.
    #ifndef FREQ_ADJUST			
			p->frequency = DUAL_FREQ_HIGH;
	#else
	        p->frequency = mt_get_freq_setting(p);
	#endif		
			// Adjust to HV first.
			// HV 1.125V
			for (Voltage=0x30; Voltage<0x44; Voltage++)
			{
				pmic_Vcore1_adjust(4);
				pmic_Vcore2_adjust(4);
				if (uiDualRank) 
				{
					DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, 0x100, 8, 1, 1);
					DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiDestAddr, uiSrcAddr, 0x100, 8, 1, 1);
				}
				else
				{
					DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiSrcAddr+0x10000000, 0x100, 8, 1, 1);
				}
			}
			//pmic_Vcore1_adjust(6);
			//pmic_Vcore2_adjust(6);	
			pmic_voltage_read();
		}
		RXPERBIT_LOG_PRINT = 0;

		//DramcSwitchFreq(p, 0);
		DramcDFS(p, 0);
		
		RXPERBIT_LOG_PRINT = 1;

		CHA_HWGW_Print(p);
	
    #ifndef FREQ_ADJUST
		if (p->frequency == DUAL_FREQ_HIGH)
    #else
        if (p->frequency != DUAL_FREQ_LOW)   
    #endif		    
		{
		}
		else
		{
			// Now in low freq. switch into low voltage.
			// LV 1.00V
			for (Voltage=0x44; Voltage>0x30; Voltage--)
			{
				pmic_Vcore1_adjust(5);
				pmic_Vcore2_adjust(5);
				if (uiDualRank) 
				{
					DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, 0x100, 8, 1, 1);
					DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiDestAddr, uiSrcAddr, 0x100, 8, 1, 1);
				}
				else
				{
					DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, 0x100, 8, 1, 1);
				}
			}
			pmic_voltage_read();
		}
		
	
#endif
		
#ifdef XTALK_SSO_STRESS
		{
			
			#ifdef SINGLE_CHANNEL_ENABLE
			uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, 1, 1);
			#else
			uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, 1, 2);
			#endif
			if (uiCmpErr)
			{
				err_count++;
				mcSHOW_DBG_MSG(("Fail %xh : DMA source =%xh Dest = %xh Len=%xh\n", uiCmpErr, uiSrcAddr, uiDestAddr, uiLen));
			}
			else
			{
				pass_count++;
				mcSHOW_DBG_MSG(("Pass %xh : DMA source =%xh Dest = %xh Len=%xh\n", uiCmpErr, uiSrcAddr, uiDestAddr, uiLen));
			}
			mcSHOW_DBG_MSG(("pass_count = %d, err_count = %d \n", pass_count, err_count));

			#ifdef SUSPEND_TEST
			Suspend_Resume(p);
			#endif

			#ifdef SINGLE_CHANNEL_ENABLE
			uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiDestAddr, uiSrcAddr, uiLen, 8, 1, 1);
			#else
			uiCmpErr = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiDestAddr, uiSrcAddr, uiLen, 8, 1, 2);
			#endif
			if (uiCmpErr)
			{
				err_count++;
				mcSHOW_DBG_MSG(("Fail %xh : DMA source =%xh Dest = %xh Len=%xh\n", uiCmpErr, uiDestAddr, uiSrcAddr, uiLen));
			}
			else
			{
				pass_count++;
				mcSHOW_DBG_MSG(("Pass %xh : DMA source =%xh Dest = %xh Len=%xh\n", uiCmpErr, uiDestAddr, uiSrcAddr, uiLen));
			}
			mcSHOW_DBG_MSG(("pass_count = %d, err_count = %d \n", pass_count, err_count));
		    
			uiSrcAddr+=0xff000;
			if ((uiSrcAddr+0x200000) >= 0x60000000)
			{
				uiSrcAddr = 0x50000000;
			}
		}
#endif		

#ifdef SUSPEND_TEST
		Suspend_Resume(p);
#endif

#ifdef READ_COMPARE_TEST
		uiReadCompareOK = 1;
		for (count=uiFixedAddr; count<uiFixedAddr+0x10000; count+=4)
		{
			if (*(volatile unsigned int   *)(count) != count)
			{
				mcSHOW_DBG_MSG(("[Fail] Addr %xh = %xh\n",count, *(volatile unsigned int   *)(count)));
				uiReadCompareOK = 0;
			}
		}		
		if (uiReadCompareOK)
		{
			pass_count++;
			mcSHOW_DBG_MSG(("Read compare pass..\n"));
		}
		else
		{
			err_count++;
			mcSHOW_DBG_MSG(("Read compare fail..\n"));
		}
#endif

#ifdef SUSPEND_TEST
		Suspend_Resume(p);
#endif
		
#ifdef TA2_STRESS
		p->channel = CHANNEL_A;
		//temp = DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, 0xaaffffff, 2, 0, 0, 0);
		temp = DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, 0xaa7fffff, 2, 0, 0, 0);
		if (temp < 0){
		    err_count++;
		    dbg_print("channel A TA2 failed,pass_cnt:%d,err_cnt:%d\n",pass_count,err_count);
		}
		else
		{
		    pass_count++;
		    dbg_print("channel A TA2 pass,pass_cnt:%d,err_cnt:%d\n",pass_count,err_count);
		}       	
#endif

#ifdef SUSPEND_TEST
		Suspend_Resume(p);
#endif

#ifdef CPUTEST
		temp = Memory_Read_Write_Test(uiStartAddr,0x100000);
		if (temp < 0){
		    err_count++;
		    dbg_print("mem test failed,pass_cnt:%d,err_cnt:%d\n",pass_count,err_count);
		}
		else{
		    pass_count++;
		    dbg_print("mem test pass,pass_cnt:%d,err_cnt:%d\n",pass_count,err_count);
		}
#endif
		uiStartAddr+=0x100000;
		if (uiStartAddr>=0x80000000)
		{
			uiStartAddr = 0x50000000;
		}
	}while(1);
}

#endif

#ifdef DQM_CALIBRATION
extern S32 DQMSkew[DQS_NUMBER];
#endif

void Init_DRAM(void)
{
	int temp;
	int mem_start,len;
	unsigned int err_count = 0;
	unsigned int pass_count = 0;
	unsigned int count;
	unsigned int count2 = 1000;
	unsigned int delay = 1000;
	unsigned int DDR2_MODE_REG5;
	int reg_val;
	int rank0_col,rank1_col;
	DRAMC_CTX_T * p;
	U8 ucstatus = 0;
        U32 u4value;

	p = psCurrDramCtx;

#if CFG_FPGA_PLATFORM

    *(volatile unsigned *)(EMI_APB_BASE+0x00000000) = 0x50045002; //EMI_CONA , single channel/single rank
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1ec), 0x00101001);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x07c), 0xC0063201);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x430), 0x10ff10ff);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x434), 0xffffffff);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x438), 0xffffffff);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x43c), 0x0000001f);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x400), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x404), 0x00101000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x418), 0x00000228);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x408), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x40C), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x410), 0x03555555);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x41C), 0x11111111);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x420), 0x11111111);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x424), 0x11111111);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x428), 0x0000ffff);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x42C), 0x000000ff);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1E0), 0x3601ffff);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1F8), 0x0c002ec1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x23C), 0x2201ffff);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x008), 0x00406300);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x048), 0x2200110d);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x08C), 0x00000001);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0D8), 0x40500510);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0E4), 0x00002111);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0B8), 0x99169952);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0BC), 0x99109950);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x090), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x094), 0x80000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0DC), 0x83200200);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0E0), 0x12200200);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x118), 0x00000002);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0F0), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0F4), 0x11000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x168), 0x00000080);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x130), 0x30000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0D8), 0x40700510);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x004), 0xf00485a3);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x124), 0xc0000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x138), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x094), 0x40404040);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1C0), 0x8000c8b8);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0fc), 0x07000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1f8), 0x0c002ec1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1ec), 0x00100001);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x07c), 0xc0063201);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x028), 0xf1200f01);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x158), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0e4), 0x00000015);

    mcDELAY_US(200);	// tINIT3 > 200us

    // MR63 -> Reset
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x0000003F);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
    mcDELAY_US(10);	// Wait >=10us if not check DAI.
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);        

    // MR10 -> ZQ Init
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00FF000A);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
    mcDELAY_US(1);		// tZQINIT>=1us
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
   
    // MR1             
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00330001);        
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
    mcDELAY_US(1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
    
    // MR2
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), 0x00060002);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
    mcDELAY_US(1);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);

    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x110), 0x00251180);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0e4), 0x00000011);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1ec), 0x00000001);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x084), 0x00000a56);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x080), 0x00060600);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x00c), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x000), 0x555844a3);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x044), 0xa80d0400);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e8), 0x81000d20);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x008), 0x00406360);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x010), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x100), 0x01008110);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x01c), 0x12121212);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0f8), 0x00000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0fc), 0x07000000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1dc), 0xd2623840);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1ec), 0x00100001);

    *(volatile unsigned *)(EMI_APB_BASE+0x00000060) = 0x00000400; //Enable EMI

    //Single Channel , Invert 180 degree
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0FC), 0x07020000);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x07c), 0xc0063201);
    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x0e4), 0x00000011);     

#else

#if fcFOR_CHIP_ID == fcK2
	//According to Benson & MH,  K2 with LPDDR3/2 VBianN is connected to GND directly. So no need to set.
	// Reg.648h[15:11] INTREF2_REFTUNE=01111b      [10:9]INTREF2_DS=10b      [8] EN_INTREF2=1b
	// Reg.648h[7:3] INTREF1_REFTUNE=01111b      [2:1]INTREF1_DS_DQ=10b      [0] EN_INTREF1_DQ=1b
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x648), &u4value);
	u4value &= 0xffff0000;
	u4value |= 0x00007d7d;
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x648), u4value);      	

	// Reg.644h[7:3] INTREF0_REFTUNE=01111b      [2:1]INTREF0_DS_DQ=10b      [0] EN_INTREF0_DQ=1b
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x644), &u4value);
	u4value &= 0xffffff00;
	u4value |= 0x0000007d;
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x644), u4value);      	
        
	mcDELAY_MS(50);	// According to ACD spec, need to delay 50ms in normal operation (DS1,DS0)=(1,0).
#endif

    EMI_Init(p);   	
		
#if fcFOR_CHIP_ID == fcK2
    p->channel = CHANNEL_A;
    DramcInit((DRAMC_CTX_T *) p);		
#endif

#ifdef FTTEST_ZQONLY
	while (1)
	{
	    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), LPDDR3_MODE_REG_10);
	    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000001);
	    mcDELAY_US(1);		// tZQINIT>=1us
	    ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
	}
#endif

#if defined(DDR_INIT_TIME_PROFILING)
	/* enable ARM CPU PMU */
	asm volatile(
	    "MRC p15, 0, %0, c9, c12, 0\n"
	    "BIC %0, %0, #1 << 0\n"   /* disable */
	    "ORR %0, %0, #1 << 2\n"   /* reset cycle count */
	    "BIC %0, %0, #1 << 3\n"   /* count every clock cycle */
	    "MCR p15, 0, %0, c9, c12, 0\n"
	    : "+r"(temp)
	    :
	    : "cc"
	);
	asm volatile(
	    "MRC p15, 0, %0, c9, c12, 0\n"
	    "ORR %0, %0, #1 << 0\n"   /* enable */
	    "MCR p15, 0, %0, c9, c12, 0\n"
	    "MRC p15, 0, %0, c9, c12, 1\n"
	    "ORR %0, %0, #1 << 31\n"
	    "MCR p15, 0, %0, c9, c12, 1\n"
	    : "+r"(temp)
	    :
	    : "cc"
	);

	mcDELAY_US(100);

	/* get CPU cycle count from the ARM CPU PMU */
	asm volatile(
	    "MRC p15, 0, %0, c9, c12, 0\n"
	    "BIC %0, %0, #1 << 0\n"   /* disable */
	    "MCR p15, 0, %0, c9, c12, 0\n"
	    "MRC p15, 0, %0, c9, c13, 0\n"
	    : "+r"(temp)
	    :
	    : "cc"
	);
	opt_print(" mcDELAY_US(100) takes %d CPU cycles\n\r", temp);
#endif

#if defined(DDR_INIT_TIME_PROFILING)
	/* enable ARM CPU PMU */
	asm volatile(
	    "MRC p15, 0, %0, c9, c12, 0\n"
	    "BIC %0, %0, #1 << 0\n"   /* disable */
	    "ORR %0, %0, #1 << 2\n"   /* reset cycle count */
	    "BIC %0, %0, #1 << 3\n"   /* count every clock cycle */
	    "MCR p15, 0, %0, c9, c12, 0\n"
	    : "+r"(temp)
	    :
	    : "cc"
	);
	asm volatile(
	    "MRC p15, 0, %0, c9, c12, 0\n"
	    "ORR %0, %0, #1 << 0\n"   /* enable */
	    "MCR p15, 0, %0, c9, c12, 0\n"
	    "MRC p15, 0, %0, c9, c12, 1\n"
	    "ORR %0, %0, #1 << 31\n"
	    "MCR p15, 0, %0, c9, c12, 1\n"
	    : "+r"(temp)
	    :
	    : "cc"
	);
#endif				

		    			
        p->channel = CHANNEL_A;
	DramcSwImpedanceCal((DRAMC_CTX_T *) p, 1);	//BringUp : Maybe bypass.
					
       mcSHOW_DBG_MSG2(("mt_get_mem_freq=%d\n", mt_get_mem_freq()));
        
#if 0	
// Run again here for different voltage. For preloader, if the following code is executed after voltage change, no need.
    DramcEnterSelfRefresh(p, 1);
    mcSHOW_DBG_MSG2(("mempll init again\n"));
    MemPllInit(p);		
    mcDELAY_US(1);
    mt_mempll_cali(p);							//BringUp : Maybe bypass.
    mcSHOW_DBG_MSG2(("Dramc clock=%d\n", mt_get_mem_freq()));
    DramcEnterSelfRefresh(p, 0);
#endif
	
#ifdef DUAL_FREQ_K
DDR_CALI_START:	
	// Disable MR4 for GW calibration.
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM1), &u4value);
	mcSET_FIELD(u4value, 0x0, MASK_ACTIM1_REFRCNT, POS_ACTIM1_REFRCNT);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM1), u4value);    
#endif


#ifdef MATYPE_ADAPTATION	
		// Backup here because Reg.04h may be modified based on different column address of different die or channel.
		// Default value should be the smallest number.
		ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x04), &u4Backup_Reg_04);
#endif


	p->channel = CHANNEL_A;

#ifdef CA_WR_ENABLE
    if (p->dram_type == TYPE_LPDDR3)
    {  
        #ifdef DUAL_FREQ_K
            #ifndef FREQ_ADJUST
	            if (p->frequency == DUAL_FREQ_HIGH)
            #else
                if (p->frequency != DUAL_FREQ_LOW)   
            #endif
	            {
		            // CA, clock output delay regsiters only have one set which could not be shuffled. So only do calibration in high frequency.
		            DramcCATraining((DRAMC_CTX_T *) p);
		            DramcWriteLeveling((DRAMC_CTX_T *) p);
	            }
        #else 	//DUAL_FREQ_K
	    DramcCATraining((DRAMC_CTX_T *) p);		//BringUp : Maybe bypass.
	    DramcWriteLeveling((DRAMC_CTX_T *) p);	//BringUp : Maybe bypass.
        #endif	// DUAL_FREQ_K
    }    
#endif	//CA_WR_ENABLE

	DQS_PRINT = 1;
	#ifdef DUAL_RANKS
		if (uiDualRank) 
		{
			DualRankDramcRxdqsGatingCal((DRAMC_CTX_T *) p);
		}
		else
		{
			DramcRxdqsGatingCal((DRAMC_CTX_T *) p);		
		}
	#else
		DramcRxdqsGatingCal((DRAMC_CTX_T *) p);		
	#endif

	DQS_PRINT = 0;

	if (((DRAMC_CTX_T *) p)->fglow_freq_write_en==ENABLE)
	{
		mcSHOW_DBG_MSG2(("**********************NOTICE*************************\n"));
			mcSHOW_DBG_MSG2(("Low speed write and high speed read calibration...\n"));
		mcSHOW_DBG_MSG2(("*****************************************************\n"));
			// change low frequency and use test engine2 to write data, after write, recover back to the original frequency
		CurrentRank = 0;
			DramcLowFreqWrite((DRAMC_CTX_T *) p);   
		if (uiDualRank) 
		{
			CurrentRank = 1;
			// Swap CS0 and CS1.
			ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x110), &u4value);
			u4value = u4value |0x08;
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x110), u4value);
		          		           
			// do channel A & B low frequency write simultaneously
			DramcLowFreqWrite((DRAMC_CTX_T *) p);	

			// Swap CS back.
			ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x110), &u4value);
			u4value = u4value & (~0x08);
			ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x110), u4value);
			CurrentRank = 0;
		}
	}
	
	if (uiDualRank) 
	{
		DramcDualRankRxdatlatCal((DRAMC_CTX_T *) p);
	}
	else
	{
		DramcRxdatlatCal((DRAMC_CTX_T *) p);
	}

#ifdef RX_DUTY_CALIBRATION
  #ifdef DUAL_FREQ_K
    #ifndef FREQ_ADJUST
	if (p->frequency == DUAL_FREQ_HIGH)
    #else
    if (p->frequency != DUAL_FREQ_LOW)   
    #endif
	{
		DramcClkDutyCal(p);
	}
  #else
	DramcClkDutyCal(p);
  #endif
#endif

#ifdef DQS_RISING_FALLING
	DramcRxWindowPerbitCal_RF((DRAMC_CTX_T *) p, 1);
	DramcRxWindowPerbitCal_RF((DRAMC_CTX_T *) p, 0);
	while (1);
#else

	#ifdef ENABLE_DQSI_PER_RANK
	if (uiDualRank) 
	{
		// Rank 1
		mcSHOW_DBG_MSG4(("Rank 1 RX calibration\n"));
		DramcRxWindowPerbitCal_RK1	((DRAMC_CTX_T *) p);	
		//DramcRxRegPrint((DRAMC_CTX_T *) p);
	}
	#endif
	// Rank 0
	mcSHOW_DBG_MSG4(("Rank 0 RX calibration\n"));
	DramcRxWindowPerbitCal((DRAMC_CTX_T *) p);

#endif

#ifdef DQM_CALIBRATION
	DQMSkew[0] = DQMSkew[1] = DQMSkew[2] = DQMSkew[3] = 0;
	DramcTxWindowPerbitCal((DRAMC_CTX_T *) p);
	DramcDQMCal(p);
	DramcTxWindowPerbitCal((DRAMC_CTX_T *) p);
#else
	DramcTxWindowPerbitCal((DRAMC_CTX_T *) p);  
#endif

#ifdef TDP_TDN_CAL
	DramcClkTDNTDPCal(p);
	DramcDQSTDNTDPCal(p);
#endif

#ifdef TUNE_RX_TX_FACTORS  
	ett_tune_rx_factors();
	ett_rx_factor_reset();
	ett_config_rx_set_opt_to_hw(); 
	ett_tune_tx_factors();
#endif    

  //SW workaround for DFS hang issue on single rank memory device
  ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x110), &u4value);
  ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x110), u4value | 0x1);
  
	//Put RANKINCTL setting
	DramcRunTimeConfig((DRAMC_CTX_T *) p);

#ifdef DUAL_FREQ_K
        print_DBG_info(p);
        
  #ifndef FREQ_ADJUST
	if (p->frequency == DUAL_FREQ_HIGH)
  #else
    if (p->frequency != DUAL_FREQ_LOW)   
  #endif
	{
		DramcSaveToBackup(p);

		p->frequency = DUAL_FREQ_LOW;
		DramcSwitchFreq(p);

	#if defined(DUAL_FREQ_DIFF_ACTIMING) || defined(DUAL_FREQ_DIFF_RLWL)
		#if fcFOR_CHIP_ID == fcK2
		DramcInit((DRAMC_CTX_T *) p);		
		#endif
	#endif
		goto DDR_CALI_START;
	} 
	else
	{
	#if fcFOR_CHIP_ID == fcK2
		DramcSaveToShuffleReg(p);	// Now is in low frequency with normal registers. Save to shuffle.
		p->frequency = DUAL_FREQ_LOW;	// Switch to low frequency with shuffle registers.
		//DramcDFS(p, 1);
		DramcShuffle(p);

		DramcRestoreBackup(p);			// Restore back high frequency registers to normal.
		#ifndef ONEPLL_TEST
		    #ifndef FREQ_ADJUST
		        p->frequency = DUAL_FREQ_HIGH;	// Switch to high frequency with normal registers.
            #else
                p->frequency = mt_get_freq_setting(p);	// Switch to high frequency with normal registers
            #endif		        
		DramcDFS(p, 1);
	#endif
	#endif
	}
	
	DramcDumpFreqSetting(p);
#endif

#if defined(DDR_INIT_TIME_PROFILING)
	/* get CPU cycle count from the ARM CPU PMU */
	asm volatile(
	    "MRC p15, 0, %0, c9, c12, 0\n"
	    "BIC %0, %0, #1 << 0\n"   /* disable */
	    "MCR p15, 0, %0, c9, c12, 0\n"
	    "MRC p15, 0, %0, c9, c13, 0\n"
	    : "+r"(temp)
	    :
	    : "cc"
	);
	opt_print("DRAMC calibration takes %d CPU cycles\n\r", temp);
#endif

	DramcHWGWEnable((DRAMC_CTX_T *) p);

#ifdef SPM_CONTROL_AFTERK
	TransferToSPMControl();
#endif

#ifdef RANK1_TEST
	// Swap CS0 and CS1.
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x110), &u4value);
	u4value = u4value |0x08;
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x110), u4value);
#endif
#ifdef DR_QUICKENDTEST
	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0xd8), &u4value);
	mcSET_BIT(u4value, 10);
	mcSET_BIT(u4value, 11);
	ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0xd8), u4value);
#endif	

	print("\n\nSettings after calibration ...\n\n");
        print_DBG_info(p);
        Dump_EMIRegisters(p); 

#ifdef EYE_SCAN
	*(volatile unsigned int*)(0x10007000) = 0x22000000;
	mcSHOW_DBG_MSG2(("Eye scan.\n\n"));
	DramcEyeScan(p);
        while(1);
#endif

        #ifdef DISABLE_WDT
	*(volatile unsigned int*)(0x10007000) = 0x22000000;
        #endif

	#ifdef MEM_TEST
	#ifdef MEM_TEST_IDLE
	while (1);
	#endif
        MemoryTest(p);
        #endif        
#endif
}

unsigned int is_one_pll_mode(void)
{  
   int data;
   
   data = Reg_Readl((CHA_DRAMCAO_BASE + (0x00a << 2)));
   if(data & 0x10000)
   {
      print("It is 1-PLL mode (value = 0x%x)\n", data);
      return 1;
   }
   else
   {
      print("It is 3-PLL mode (value = 0x%x)\n", data);
      return 0;
   }
}

#ifdef DDR_RESERVE_MODE
extern u32 g_ddr_reserve_enable;
extern u32 g_ddr_reserve_success;
#define TIMEOUT 3
#endif

void release_dram(void)
{
#ifdef DDR_RESERVE_MODE  
    int i;
    int counter = TIMEOUT;
    int one_pll_mode = 0;
      
#ifdef FRCLK_26M_DIV8
     //in DDR reserved mode, need to set dramc_f26m_sel because it will be clear in reset
	 *(volatile unsigned *)(0x10001088) |= 0x80000000;	// MODULE_SW_CG_1_SET[31] = 1 ==> dramc_f26m_cg_set
	 *(volatile unsigned *)(0x10001098) |= (0x01 <<7);		// dramc_f26m_sel : 0-div 1, 1-div8
	 *(volatile unsigned *)(0x1000108C) |= 0x80000000;	// MODULE_SW_CG_1_CLR[31] = 1 ==> dramc_f26m_cg_set
#endif
    
    one_pll_mode = is_one_pll_mode();
    if(one_pll_mode == 1)
    {
		   do_DFS(0); //shuffer to low freq		   
    }  
        
    rgu_release_rg_dramc_conf_iso();
    rgu_release_rg_dram_setting();
    rgu_release_rg_dramc_iso();
    rgu_release_rg_dramc_sref();

    // setup for EMI
    DRV_WriteReg32(EMI_MPUP, 0x200);
    for (i=0;i<10;i++);
    
    while(counter)
    {
      if(rgu_is_dram_slf() == 0) /* expect to exit dram-self-refresh */
        break;
      counter--;
    }
    if(counter == 0)
    {
      if(g_ddr_reserve_enable==1 && g_ddr_reserve_success==1)
      {
        print("[DDR Reserve] release dram from self-refresh FAIL!\n");
        g_ddr_reserve_success = 0;
      }
    }
    else
    {
         print("[DDR Reserve] release dram from self-refresh PASS!\n");
    }

    if(one_pll_mode == 1)
    {
		   do_DFS(1); //shuffer to high freq
		   //do_memory_test();
    } 
        
    //for (i = 0x410; i <= 0x48C; i +=4)
    //{
    //    print("[DDR Reserve] 0x%x:0x%x\n",(0x10200000+i),*(volatile unsigned int *)(0x10200000+i));
    //}
#endif    
}

void ddr_reserve_sample_flow(void)
{
    int read_data;
    int i;
    *(volatile unsigned *)(0x10007040) = (0x59000000 | *(volatile unsigned *)(0x10007040)&~0x400);

    if(*(volatile unsigned *)(0x10007040) &0x10000==1)
    {
        print("DDR RESERVE Before RST Fail\n");
    }
#if 0
    while ((*(volatile unsigned *)(0x10007040) & 0x200000)==0x0){
        print("dram is not in self refresh\n");
    }
#endif
    
    read_data= *(unsigned int *)(0x10004000+(0x77<<2));
    *(unsigned int *)(0x10004000+(0x77<<2))= read_data & ~(0xc0000000);

    for (i=0;i<10;i++);

    /* read fifo reset */
    read_data= *(unsigned int *)(0x100040f4);
    *(unsigned int *)(0x100040f4) = read_data | (0x02000000);
    read_data= *(unsigned int *)(0x100040f4);
    *(unsigned int *)(0x100040f4) = read_data & (0xfdffffff);

    /* phase sync reset */
    read_data= *(unsigned int *)(0x1000f5cc);
    *(unsigned int *)(0x1000f5cc) = read_data | (0x00001100);
    *(unsigned int *)(0x1000f640) = 0x00020091;
    *(unsigned int *)(0x1000f640) = 0x000200b1;

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_PHYCTL1), 0x10000000);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_PHYCTL1), 0x0);

    read_data= *(unsigned int *)(0x1000f640);
    *(unsigned int *)(0x1000f640) = read_data & 0xfffffffe;
    *(unsigned int *)(0x1000f640) = read_data | 0x1;

    for (i=0;i<10;i++);

    read_data= *(unsigned int *)(0x10004000+(0x77<<2));
    *(unsigned int *)(0x10004000+(0x77<<2))= read_data | (0xc0000000);

    *(volatile unsigned *)(0x10007040) = (0x59000000 | *(volatile unsigned *)(0x10007040)&~0x200);
    for (i=0;i<10;i++);
    //exit self refresh control
    *(volatile unsigned *)(0x10007040) = (0x59000000 | *(volatile unsigned *)(0x10007040)&~0x100);

    for (i=0;i<10;i++);    
#if 0
    while (*(volatile unsigned *)(0x10007040) & 0x200000){
        print("dram in self refresh\n");
    }
#endif
   
    for (i=0;i<10;i++);
    
    DRV_WriteReg32(EMI_MPUP, 0x200);
    for (i=0;i<10;i++);
       print("[DDR Reserve] ddr_reserve_sample_flow\n");
    for (i = 0x410; i <= 0x48C; i +=4)
    {
        print("[DDR Reserve] 0x%x:0x%x\n",(0x10200000+i),*(volatile unsigned int *)(0x10200000+i));
    }

#if 0
    for (i = 0x0; i <= 0x1000; i +=4)
    {
        print("[DDR Reserve:DRAMC] 0x%x:0x%x\n",(0x10004000+i),*(volatile unsigned int *)(0x10004000+i));
    }
    for (i = 0x0; i <= 0x1000; i +=4)
    {
        print("[DDR Reserve:DRAMC_NAO] 0x%x:0x%x\n",(0x1020E000+i),*(volatile unsigned int *)(0x1020E000+i));
    }
    for (i = 0x0; i <= 0x1000; i +=4)
    {
        print("[DDR Reserve:DDRPHY] 0x%x:0x%x\n",(0x1000F000+i),*(volatile unsigned int *)(0x1000F000+i));
    }
#endif

    //while(1);
}

void check_ddr_reserve_status(void)
{
    //unsigned int wdt_mode;
    //unsigned int wdt_dbg_ctrl;
    //wdt_mode = READ_REG(MTK_WDT_MODE);
    //wdt_dbg_ctrl = READ_REG(MTK_WDT_DEBUG_CTL);

    //print("before test, wdt_mode = 0x%x, wdt_dbg_ctrl = 0x%x\n", wdt_mode, wdt_dbg_ctrl);     
    //if(((wdt_mode & MTK_WDT_MODE_DDR_RESERVE) !=0) && ((wdt_dbg_ctrl & MTK_DDR_RESERVE_RTA) != 0) )
    //{
    //    print("go sample reserved flow\n");
    //    print("[DDR Reserve_sp:DRAMC] 0x%x:0x%x\n",(0x10004080),*(volatile unsigned int *)(0x10004080));
    //    ddr_reserve_sample_flow();
    //    print("[DDR Reserve_sp:DRAMC] 0x%x:0x%x\n",(0x10004080),*(volatile unsigned int *)(0x10004080));
    //}

#ifdef DDR_RESERVE_MODE  
    int counter = TIMEOUT;
    if(rgu_is_reserve_ddr_enabled())
    {
      g_ddr_reserve_enable = 1;
      if(rgu_is_reserve_ddr_mode_success())
      {
        while(counter)
        {
          if(rgu_is_dram_slf())
          {
            g_ddr_reserve_success = 1;
            break;
          }
          counter--;
        }
        if(counter == 0)
        {
          print("[DDR Reserve] ddr reserve mode success but DRAM not in self-refresh!\n");
          g_ddr_reserve_success = 0;
        }
      }
    else
      {
        print("[DDR Reserve] ddr reserve mode FAIL!\n");
        g_ddr_reserve_success = 0;
      }
    }
    else
    {
      print("[DDR Reserve] ddr reserve mode not be enabled yet\n");
      g_ddr_reserve_enable = 0;
    }
    
    /* release dram, no matter success or failed */
    release_dram();
#endif    
}

#ifdef COMBO_MCP
EMI_SETTINGS emi_setting_default_lpddr2 =
{

        //default
                0x0,            /* sub_version */
                0x0002,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x000250A2,             /* EMI_CONA_VAL */
                0x00000003,             /* EMI_CONH_VAL */
                //0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                //0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x11000000,             /* DRAMC_GDDR3CTL1_VAL */
                0x00048483,             /* DRAMC_CONF1_VAL */
                0x000032B1,             /* DRAMC_DDR2CTL_VAL */
                0xBFC80401,             /* DRAMC_TEST2_3_VAL */
                0x00000040,             /* DRAMC_CONF2_VAL */
                0xC10B2342,             /* DRAMC_PD_CTRL_VAL */
                //0x00008888,             /* DRAMC_PADCTL3_VAL */
                //0xEEEEEEEE,             /* DRAMC_DQODLY_VAL */
                //0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                //0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x81000510,             /* DRAMC_ACTIM1_VAL*/
                0x07000000,             /* DRAMC_MISCTL0_VAL*/
                0x00002232,             /* DRAMC_ACTIM05T_VAL*/
                0x00613541,             /* DRAMC_RKCFG_VAL*/
                0x1701110D,             /* DRAMC_TEST2_4_VAL*/                 
                {0x40000000,40000000,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */                
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000006,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
};

EMI_SETTINGS emi_setting_default_lpddr3 =
{

        //default
                0x0,            /* sub_version */
                0x0003,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x00009052,             /* EMI_CONA_VAL */
                0x00000003,             /* EMI_CONH_VAL */
                //0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                //0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x66cB4619,             /* DRAMC_ACTIM_VAL */
                0x11000000,             /* DRAMC_GDDR3CTL1_VAL */
                0x000484C3,             /* DRAMC_CONF1_VAL */
                0x000053B1,             /* DRAMC_DDR2CTL_VAL */
                0xBF090401,             /* DRAMC_TEST2_3_VAL */
                0x00800F60,             /* DRAMC_CONF2_VAL */
                0xD10B3642,             /* DRAMC_PD_CTRL_VAL */
                //0x00008888,             /* DRAMC_PADCTL3_VAL */
                //0xEEEEEEEE,             /* DRAMC_DQODLY_VAL */
                //0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                //0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x91000D21,             /* DRAMC_ACTIM1_VAL*/
                0x17000000,             /* DRAMC_MISCTL0_VAL*/
                0x000030F7,             /* DRAMC_ACTIM05T_VAL*/
                0x00615641,             /* DRAMC_RKCFG_VAL*/
                0x2301110D,             /* DRAMC_TEST2_4_VAL*/                 
                {0x10000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00630001,             /* LPDDR3_MODE_REG1 */
                0x001A0002,             /* LPDDR3_MODE_REG2 */
                0x00020003,             /* LPDDR3_MODE_REG3 */
                0x00000006,             /* LPDDR3_MODE_REG5 */
                0x00FF000A,             /* LPDDR3_MODE_REG10 */
                0x0000003F,             /* LPDDR3_MODE_REG63 */
};

static int mt_get_dram_type_for_dis(void)
{
    int i;
    int type = 2;
    type = (emi_settings[0].type & 0xF);
    for (i = 0 ; i < num_of_emi_records; i++)
    {
      //print("[EMI][%d] type%d\n",i,type);
      if (type != (emi_settings[0].type & 0xF))
      {
          print("It's not allow to combine two type dram when combo discrete dram enable\n");
          ASSERT(0);
          break;
      }
    }
    return type;
}

extern const U32 uiLPDDR3_PHY_Mapping_POP_CHA[32];
extern const U32 uiLPDDR2_PHY_Mapping_POP_CHA[32];
unsigned int DRAM_MRR(int MRR_num)
{
    unsigned int MRR_value = 0x0;
    unsigned int dram_type, ucstatus, u4value;
    DRAMC_CTX_T *p = psCurrDramCtx; 
          
    if ((p->dram_type == TYPE_LPDDR3) || (p->dram_type == TYPE_LPDDR2))
    {
        // set DQ bit 0, 1, 2 pinmux
        if (p->channel == CHANNEL_A)
        {
            if (p->dram_type == TYPE_LPDDR3)
            {                
                // refer to CA training pinmux array
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), &u4value);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[0], MASK_RRRATE_CTL_BIT0_SEL, POS_RRRATE_CTL_BIT0_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[1], MASK_RRRATE_CTL_BIT1_SEL, POS_RRRATE_CTL_BIT1_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[2], MASK_RRRATE_CTL_BIT2_SEL, POS_RRRATE_CTL_BIT2_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[3], MASK_RRRATE_CTL_BIT3_SEL, POS_RRRATE_CTL_BIT3_SEL);
                ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), u4value);                
                
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRR_CTL), &u4value);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[4], MASK_MRR_CTL_BIT4_SEL, POS_MRR_CTL_BIT4_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[5], MASK_MRR_CTL_BIT5_SEL, POS_MRR_CTL_BIT5_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[6], MASK_MRR_CTL_BIT6_SEL, POS_MRR_CTL_BIT6_SEL);
                mcSET_FIELD(u4value, uiLPDDR3_PHY_Mapping_POP_CHA[7], MASK_MRR_CTL_BIT7_SEL, POS_MRR_CTL_BIT7_SEL);  
                ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRR_CTL), u4value);                                                                     
            }
            else // LPDDR2
            {
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), &u4value);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[0], MASK_RRRATE_CTL_BIT0_SEL, POS_RRRATE_CTL_BIT0_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[1], MASK_RRRATE_CTL_BIT1_SEL, POS_RRRATE_CTL_BIT1_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[2], MASK_RRRATE_CTL_BIT2_SEL, POS_RRRATE_CTL_BIT2_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[3], MASK_RRRATE_CTL_BIT3_SEL, POS_RRRATE_CTL_BIT3_SEL);        
                ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RRRATE_CTL), u4value);       
                
                ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRR_CTL), &u4value);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[4], MASK_MRR_CTL_BIT4_SEL, POS_MRR_CTL_BIT4_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[5], MASK_MRR_CTL_BIT5_SEL, POS_MRR_CTL_BIT5_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[6], MASK_MRR_CTL_BIT6_SEL, POS_MRR_CTL_BIT6_SEL);
                mcSET_FIELD(u4value, uiLPDDR2_PHY_Mapping_POP_CHA[7], MASK_MRR_CTL_BIT7_SEL, POS_MRR_CTL_BIT7_SEL);  
                ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRR_CTL), u4value);                               
            }
        }

        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x088), MRR_num);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000002);
        mcDELAY_US(1);
        ucstatus |= ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x1e4), 0x00000000);
        ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(0x03B8), &u4value);
        MRR_value = (u4value >> 20) & 0xFF;
    }    

    return MRR_value;
}

static int mt_get_dram_density(void)
{
    int value, density, io_width;
    long long size;
    
    value = DRAM_MRR(8);
    print("[EMI]MRR(8) = 0x%x\n", value);
    
    io_width = ((value & 0xC0) >> 6)? 2 : 4; //0:32bit(4byte), 1:16bit(2byte)
    //print("[EMI]DRAM IO width = %d bit\n", io_width*8);
        
    density = (value & 0x3C) >> 2;
    switch(density)
    {
        case 0x6:
            size = 0x20000000;  //4Gb
            //print("[EMI]DRAM density = 4Gb\n");
            break;
        case 0xE:
            size = 0x30000000;  //6Gb
            //print("[EMI]DRAM density = 6Gb\n");
            break;
        case 0x7:
            size = 0x40000000;  //8Gb
            //print("[EMI]DRAM density = 8Gb\n");
            break;
        case 0xD:
            size = 0x60000000;  //12Gb
            //print("[EMI]DRAM density = 12Gb\n");
            break;
        case 0x8:
            size = 0x80000000;  //16Gb
            //print("[EMI]DRAM density = 16Gb\n");
            break;
        //case 0x9:
            //size = 0x100000000L; //32Gb
            //print("[EMI]DRAM density = 32Gb\n");
            //break;
        default:
            size = 0; //reserved
     }  
     
     if(io_width == 2)
        size = size << 1;
        
     return size; 
}

static char id[22];
static int emmc_nand_id_len=16;
static int fw_id_len;
static int mt_get_mdl_number (void)
{
    static int found = 0;
    static int mdl_number = -1;
    int i;
    int j;
    int has_emmc_nand = 0;
    int discrete_dram_num = 0;
    int mcp_dram_num = 0;

    unsigned int mode_reg_5, dram_density, dram_channel_nr, dram_rank_nr;
    unsigned int dram_type;
    
    if (!(found))
    {
        int result=0;
        platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);      
        for (i = 0 ; i < num_of_emi_records; i++)
        {         
            if ((emi_settings[i].type & 0x0F00) == 0x0000) 
            {
                discrete_dram_num ++; 
            }
            else
            {
                mcp_dram_num ++; 
            }
        }
        /*If the number >=2  &&
         * one of them is discrete DRAM
         * enable combo discrete dram parse flow
         * */
        if ((discrete_dram_num > 0) && (num_of_emi_records >= 2))
        {
            /* if we enable combo discrete dram
             * check all dram are all same type and not DDR3
             * */
            enable_combo_dis = 1;
            dram_type = emi_settings[0].type & 0x000F;
            for (i = 0 ; i < num_of_emi_records; i++)
            {
                if (dram_type != (emi_settings[i].type & 0x000F))
                {
                    printf("[EMI] Combo discrete dram only support when combo lists are all same dram type.");
                    ASSERT(0);
                }
                if ((emi_settings[i].type & 0x000F) == TYPE_PCDDR3) 
                {
                    // has PCDDR3, disable combo discrete drame, no need to check others setting 
                    enable_combo_dis = 0; 
                    break;
                }
                dram_type = emi_settings[i].type & 0x000F;
            }
            
        } 
        printf("[EMI] mcp_dram_num:%d,discrete_dram_num:%d,enable_combo_dis:%d\r\n",mcp_dram_num,discrete_dram_num,enable_combo_dis);
        /*
         *
         * 0. if there is only one discrete dram, use index=0 emi setting and boot it.
         * */
        if ((0 == mcp_dram_num) && (1 == discrete_dram_num))
        {
            mdl_number = 0;
            found = 1;
            return mdl_number;
        }
            

        /* 1.
         * if there is MCP dram in the list, we try to find emi setting by emmc ID
         * */
        if (mcp_dram_num > 0)
        {
        result = platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
    
        print("[EMI] eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8],id[9],id[10],id[11],id[12],id[13],id[14],id[15]);
    
        for (i = 0; i < num_of_emi_records; i++)
        {
            if (emi_settings[i].type != 0)
            {
                if ((emi_settings[i].type & 0xF00) != 0x000)
                {
                    if (result == 0)
                    {   /* valid ID */

                        if ((emi_settings[i].type & 0xF00) == 0x100)
                        {
                            /* NAND */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0){
                                memset(id + emi_settings[i].id_length, 0, sizeof(id) - emi_settings[i].id_length);                                
                                mdl_number = i;
                                found = 1;
                                break; /* found */
                            }
                        }
                        else
                        {
                            
                            /* eMMC */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0)
                            {
#if 1
                                printf("fw id len:%d\n",emi_settings[i].fw_id_length);
                                if (emi_settings[i].fw_id_length > 0)
                                {
                                    char fw_id[6];
                                    memset(fw_id, 0, sizeof(fw_id));
                                    memcpy(fw_id,id+emmc_nand_id_len,fw_id_len);
                                    for (j = 0; j < fw_id_len;j ++){
                                        printf("0x%x, 0x%x ",fw_id[j],emi_settings[i].fw_id[j]); 
                                    }
                                    if(memcmp(fw_id,emi_settings[i].fw_id,fw_id_len) == 0)
                                    {
                                        mdl_number = i;
                                        found = 1;
                                        break; /* found */
                                    }
                                    else
                                    {
                                        printf("[EMI] fw id match failed\n");
                                    }
                                }
                                else
                                {
                                    mdl_number = i;
                                    found = 1;
                                    break; /* found */
                                }
#else
                                    mdl_number = i;
                                    found = 1;
                                    break; /* found */
#endif
                            }
                            else{
                                  printf("[EMI] index(%d) emmc id match failed\n",i);
                            }
                            
                        }
                    }
                }
            }
        }
        }
#if 1
        /* 2. find emi setting by MODE register 5
         * */
        // if we have found the index from by eMMC ID checking, we can boot android by the setting
        // if not, we try by vendor ID
        if ((0 == found) && (1 == enable_combo_dis))
        {
            //print_DBG_info();
            //print("-->%x,%x,%x\n",emi_set->DRAMC_ACTIM_VAL,emi_set->sub_version,emi_set->fw_id_length); 
            //print("-->%x,%x,%x\n",emi_setting_default.DRAMC_ACTIM_VAL,emi_setting_default.sub_version,emi_setting_default.fw_id_length); 
            dram_type = mt_get_dram_type_for_dis();
            if (TYPE_LPDDR2 == dram_type)
            {
                print("[EMI] LPDDR2 discrete dram init\r\n");
                default_emi_setting = &emi_setting_default_lpddr2;
                psCurrDramCtx = &DramCtx_LPDDR2;                  
            }
            else if (TYPE_LPDDR3 == dram_type)
            {
                print("[EMI] LPDDR3 discrete dram init\r\n");
                default_emi_setting = &emi_setting_default_lpddr3;
                psCurrDramCtx = &DramCtx_LPDDR3;  
            }

#if defined(DUAL_FREQ_K) & defined(FREQ_ADJUST)
                psCurrDramCtx->frequency = mt_get_freq_setting(psCurrDramCtx); 
#endif

            Init_DRAM();

            unsigned int manu_id = DRAM_MRR(0x5);
            print("[EMI]MR5:%x\n",manu_id);
            //try to find discrete dram by DDR2_MODE_REG5(vendor ID)
            for (i = 0; i < num_of_emi_records; i++)
            {
                if (TYPE_LPDDR2 == dram_type)
                    mode_reg_5 = emi_settings[i].iLPDDR2_MODE_REG_5; 
                else if (TYPE_LPDDR3 == dram_type)
                    mode_reg_5 = emi_settings[i].iLPDDR3_MODE_REG_5; 
                print("emi_settings[%d].MODE_REG_5:%x,emi_settings[%d].type:%x\n",i,mode_reg_5,i,emi_settings[i].type);
                //only check discrete dram type
                if ((emi_settings[i].type & 0x0F00) == 0x0000) 
                {
                    //support for compol discrete dram 
                    if ((mode_reg_5 == manu_id) )
                    {
                        dram_density = mt_get_dram_density();                       
                        print("emi_settings[%d].DRAM_RANK_SIZE[0]:0x%x, dram_density:0x%x\n",i,emi_settings[i].DRAM_RANK_SIZE[0], dram_density);                            
                        if(emi_settings[i].DRAM_RANK_SIZE[0] == dram_density)
                        {  
                            mdl_number = i;
                            found = 1;
                            break;
                        } 
                    }
                }
            }
        }
#endif
        printf("found:%d,i:%d\n",found,i);    
    }
    return mdl_number;
}

#endif //#ifdef COMBO_MCP 

int get_dram_rank_nr (void)
{

    int index;
    int emi_cona;
#ifdef COMBO_MCP    
    index = mt_get_mdl_number ();
    if (index < 0 || index >=  num_of_emi_records)
    {
        return -1;
    }

    emi_cona = emi_settings[index].EMI_CONA_VAL;
#else
    emi_cona = EMI_CONA_VAL;
#if CFG_FPGA_PLATFORM
    return 1;
#endif
#endif
    return (emi_cona & 0x20000) ? 2 : 1;

}

#ifdef COMBO_MCP
int is_combo_dis_enable(void)
{
    int i;
    int discrete_dram_num = 0;
    int mcp_dram_num = 0;
    unsigned int dram_type;
    int combo_dis = 0;   
      
    for (i = 0 ; i < num_of_emi_records; i++)
    {         
        if ((emi_settings[i].type & 0x0F00) == 0x0000) 
        {
            discrete_dram_num ++; 
        }
        else
        {
            mcp_dram_num ++; 
        }
    }
    /*If the number >=2  &&
     * one of them is discrete DRAM
     * then combo discrete enable is true
     * */
     
    if ((discrete_dram_num > 0) && (num_of_emi_records >= 2))
    {
        /* if we enable combo discrete dram
         * check all dram are all same type and not DDR3
         * */
        combo_dis = 1;
        dram_type = emi_settings[0].type & 0x000F;
        for (i = 0 ; i < num_of_emi_records; i++)
        {
            if (dram_type != (emi_settings[i].type & 0x000F))
            {
                printf("[EMI] Combo discrete dram only support when combo lists are all same dram type.");
                ASSERT(0);
            }
            if ((emi_settings[i].type & 0x000F) == TYPE_PCDDR3) 
            {
                // has PCDDR3, disable combo discrete drame, no need to check others setting 
                combo_dis = 0; 
                break;
            }
            dram_type = emi_settings[i].type & 0x000F;
        }       
    } 
    return combo_dis;
}
#endif

int mt_get_dram_type (void)
{
    int n;
#ifdef COMBO_MCP     
   /* if combo discrete is enabled, the dram_type is LPDDR2 or LPDDR4, depend on the emi_setting list*/
    if ( is_combo_dis_enable() == 1)
    return mt_get_dram_type_for_dis();

    n = mt_get_mdl_number ();

    if (n < 0  || n >= num_of_emi_records)
    {
        return 0; /* invalid */
    }

    return (emi_settings[n].type & 0xF);
#else
    #if defined(DDRTYPE_LPDDR3)
    return TYPE_LPDDR3;
    #else 
        return TYPE_LPDDR2;
    #endif
#endif

}

/*
 * reserve a memory from mblock
 * @mblock_info: address of mblock_info 
 * @size: size of memory
 * @align: alignment, not implemented
 * @limit: address limit. Must higher than return address + size 
 * @rank: preferable rank, the returned address is in rank or lower ranks
 * It returns as high rank and high address as possible. (consider rank first)
 */
u64 mblock_reserve(mblock_info_t *mblock_info, u64 size, u64 align, u64 limit,
		enum reserve_rank rank)
{
	int i, max_rank, target = -1;
	u64 start, sz, max_addr = 0;

	if (size & (0x200000 - 1)) {
		printf("warning: size is not 2MB aligned\n");
	}

	if (rank == RANK0) {
		/* reserve memory from rank 0 */
		max_rank = 0;
	} else {
		/* reserve memory from any possible rank */
		/* mblock_num >= nr_ranks is true */
		max_rank = mblock_info->mblock_num - 1;
	}

	for (i = 0; i < mblock_info->mblock_num; i++) {
		start = mblock_info->mblock[i].start;
		sz = mblock_info->mblock[i].size;
		printf("mblock[%d].start: 0x%llx, sz: 0x%llx, limit: 0x%llx, "
				"max_addr: 0x%llx, max_rank: %d, target: %d, "
				"mblock[].rank: %d\n",
				i, start, sz, limit, max_addr, max_rank,
				target, mblock_info->mblock[i].rank);
		printf("mblock_reserve dbg[%d]: %d, %d, %d, %d\n",
				i, (start + sz <= limit),
				(mblock_info->mblock[i].rank <= max_rank),
				(start + sz > max_addr),
				 (sz >= size));
		if ((start + sz <= limit) &&
			(mblock_info->mblock[i].rank <= max_rank) &&
			(start + sz > max_addr) &&
			(sz >= size)) {
			max_addr = start + sz;
			target = i;
		}
	}

	if (target < 0) {
		printf("mblock_reserve error\n");
		return 0;
	} 

	mblock_info->mblock[target].size -= size;

	printf("mblock_reserve: %llx - %llx from mblock %d\n",
			(mblock_info->mblock[target].start
			+ mblock_info->mblock[target].size),
			(mblock_info->mblock[target].start
			+ mblock_info->mblock[target].size + size),
			target);


	return mblock_info->mblock[target].start + 
		mblock_info->mblock[target].size;
}

/* 
 * setup block correctly, we should hander both 4GB mode and 
 * non-4GB mode.
 */
void setup_mblock_info(mblock_info_t *mblock_info, dram_info_t *orig_dram_info,
		mem_desc_t *lca_reserved_mem)
{
	int i;
	u64 max_dram_size = -1; /* MAX value */
	u64 size = 0;
	u64 total_dram_size = 0;

	for (i = 0; i < orig_dram_info->rank_num; i++) {
		total_dram_size += 
			orig_dram_info->rank_info[i].size;
	}
#ifdef CUSTOM_CONFIG_MAX_DRAM_SIZE
	max_dram_size = atoll(CUSTOM_CONFIG_MAX_DRAM_SIZE);
	printf("CUSTOM_CONFIG_MAX_DRAM_SIZE: 0x%llx\n", max_dram_size);
#endif 
	lca_reserved_mem->start = lca_reserved_mem->size = 0;
	memset(mblock_info, 0, sizeof(mblock_info_t));

	/* 
	 * non-4GB mode case 
	 */
	/* we do some DRAM size fixup here base on orig_dram_info */
	for (i = 0; i < orig_dram_info->rank_num; i++) {
		size += orig_dram_info->rank_info[i].size;
		mblock_info->mblock[i].start = 
			orig_dram_info->rank_info[i].start;
		mblock_info->mblock[i].rank = i;	/* setup rank */
		if (size <= max_dram_size) {
			mblock_info->mblock[i].size = 
				orig_dram_info->rank_info[i].size;
		} else {
			/* max dram size reached */
			size -= orig_dram_info->rank_info[i].size;
			mblock_info->mblock[i].size = 
				max_dram_size - size;
			/* get lca_reserved_mem info */
			lca_reserved_mem->start = mblock_info->mblock[i].start
				+ mblock_info->mblock[i].size;
			if (mblock_info->mblock[i].size) {
				mblock_info->mblock_num++;
			}
			break;
		}

		if (mblock_info->mblock[i].size) {
			mblock_info->mblock_num++;
		}
	}
	
	printf("total_dram_size: 0x%llx, max_dram_size: 0x%llx\n",
			total_dram_size, max_dram_size);
	if (total_dram_size > max_dram_size) {
		/* add left unused memory to lca_reserved memory */
		lca_reserved_mem->size = total_dram_size - max_dram_size;
		printf("lca_reserved_mem start: 0x%llx, size: 0x%llx\n",
				lca_reserved_mem->start,
				lca_reserved_mem->size);
	}

	/*
	 * TBD
	 * for 4GB mode, we fixup the start address of every mblock
	 */
}

/* 
 * setup block correctly, we should hander both 4GB mode and 
 * non-4GB mode.
 */
void get_orig_dram_rank_info(dram_info_t *orig_dram_info)
{
	int i, j;
	u64 base = DRAM_BASE;
	unsigned int rank_size[4];

	orig_dram_info->rank_num = get_dram_rank_nr();
	get_dram_rank_size(rank_size);
  
	orig_dram_info->rank_info[0].start = base;
	for (i = 0; i < orig_dram_info->rank_num; i++) {

		orig_dram_info->rank_info[i].size = (u64)rank_size[i];

		if (i > 0) {
			orig_dram_info->rank_info[i].start =
				orig_dram_info->rank_info[i - 1].start +
				orig_dram_info->rank_info[i - 1].size;
		}
		printf("orig_dram_info[%d] start: 0x%llx, size: 0x%llx\n",
				i, orig_dram_info->rank_info[i].start,
				orig_dram_info->rank_info[i].size);
	}
	
	for(j=i; j<4; j++)
	{
	  		orig_dram_info->rank_info[j].start = 0;
	  		orig_dram_info->rank_info[j].size = 0;	
	}
}

void get_dram_rank_size (unsigned int dram_rank_size[])
{
#ifdef COMBO_MCP
    int index, rank_nr, i;

    index = mt_get_mdl_number ();

    if (index < 0 || index >=  num_of_emi_records)
    {
        return;
    }

    rank_nr = get_dram_rank_nr();

    for(i = 0; i < rank_nr; i++){
        dram_rank_size[i] = emi_settings[index].DRAM_RANK_SIZE[i];

        printf("%d:dram_rank_size:%x\n",i,dram_rank_size[i]);
    }

    return;
#else

    unsigned col_bit, row_bit, ch0_rank0_size, ch0_rank1_size, ch1_rank0_size, ch1_rank1_size;
    unsigned emi_cona = EMI_CONA_VAL, emi_conh = EMI_CONH_VAL;
 
    dram_rank_size[0] = 0;
    dram_rank_size[1] = 0;
    
    ch0_rank0_size = (emi_conh >> 16) & 0xf;
    ch0_rank1_size = (emi_conh >> 20) & 0xf;
    
    //Channel 0
    {   
        if(ch0_rank0_size == 0)
        {
            //rank 0 setting
            col_bit = ((emi_cona >> 4) & 0x03) + 9;
            row_bit = ((emi_cona >> 12) & 0x03) + 13;
            dram_rank_size[0] = (1 << (row_bit + col_bit)) * 4 * 8; // 4 byte * 8 banks
        }
        else
        {
            dram_rank_size[0] = (ch0_rank0_size * 256 << 20);
        }
 
        if (0 != (emi_cona &  (1 << 17)))   //rank 1 exist
        {
            if(ch0_rank1_size == 0)
            {
                col_bit = ((emi_cona >> 6) & 0x03) + 9;
                row_bit = ((emi_cona >> 14) & 0x03) + 13;
                dram_rank_size[1] = ((1 << (row_bit + col_bit)) * 4 * 8); // 4 byte * 8 banks
            }
            else
            {
                dram_rank_size[1] = (ch0_rank1_size * 256 << 20);
            }                
        }                                        
    }
        
    printf("DRAM rank0 size:0x%x,\nDRAM rank1 size=0x%x\n", dram_rank_size[0], dram_rank_size[1]);    

    return;      
#endif
}

DRAMC_CTX_T* mt_get_dramc_context(void)
{
    int index = 0;
    EMI_SETTINGS *emi_set;

    index = emi_setting_index = mt_get_mdl_number ();
    print("[Check]mt_get_mdl_number 0x%x\n", index);
    //print("[EMI] eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8],id[9],id[10],id[11],id[12],id[13],id[14],id[15]);
    if (index < 0 || index >=  num_of_emi_records)
    {
        print("[EMI] setting failed 0x%x\r\n", index);
        ASSERT(0);
    }
  
    print("[EMI] MDL number = %d\r\n", index);
    emi_set = &emi_settings[index];

    print("[EMI] emi_set eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", emi_set->ID[0], emi_set->ID[1], emi_set->ID[2], emi_set->ID[3], emi_set->ID[4], emi_set->ID[5], emi_set->ID[6], emi_set->ID[7], emi_set->ID[8],emi_set->ID[9],emi_set->ID[10],emi_set->ID[11],emi_set->ID[12],emi_set->ID[13],emi_set->ID[14],emi_set->ID[15]);
    if ((emi_set->type & 0xF) == TYPE_LPDDR3)
    {
        return &DramCtx_LPDDR3;
    }
    else if ((emi_set->type & 0xF) == TYPE_LPDDR2)
    {
        return &DramCtx_LPDDR2;
    }
    else
    {
        print("The DRAM type is not supported");
        ASSERT(0);
        return -1;
    }    
}

void do_DFS(high_freq)
{
    DRAMC_CTX_T *p;
     
#ifdef COMBO_MCP
    p = mt_get_dramc_context();
#else

    #ifdef DDRTYPE_LPDDR3
	  p = &DramCtx_LPDDR3;
    #endif

    #ifdef DDRTYPE_LPDDR2
	  p = &DramCtx_LPDDR2;
    #endif

#endif

    if(high_freq == 1)
    {
#if defined(DUAL_FREQ_K) & defined(FREQ_ADJUST)     
      p->frequency = mt_get_freq_setting(p);
#endif      
    }
    else
    {
      p->frequency = DUAL_FREQ_LOW;      
    }

    DramcDFS(p, 1);    
}

void do_memory_test(void)
{
    DRAMC_CTX_T *p;
     
#ifdef COMBO_MCP
    p = mt_get_dramc_context();
#else
    #ifdef DDRTYPE_LPDDR3
	  p = &DramCtx_LPDDR3;
    #endif

    #ifdef DDRTYPE_LPDDR2
	  p = &DramCtx_LPDDR2;
    #endif
#endif
    MemoryTest(p);
}

#ifdef DRAM_HQA

#define Vdram_HV (0x70) //1.30V
#define Vdram_NV (0x63) //1.22V
#define Vdram_LV (0x5A) //1.16V

#ifndef ONEPLL_TEST
#define Vcore_HV 0x50 //1.10V
#define Vcore_NV 0x40 //1.00V
//#define Vcore_LV 0x30 //0.90V
#define Vcore_LV 0x38 //0.95V
#else
#define Vcore_HV 0x3E //0.99V
#define Vcore_NV 0x30 //0.90V
//#define Vcore_LV 0x22 //0.81V
#define Vcore_LV 0x29 //0.855V
#endif

#define Vio18_HV 0x28 //1.9V
#define Vio18_NV 0x20 //1.8V
#define Vio18_LV 0x18 //1.7V

void dram_HQA_adjust_voltage()
{
#ifdef HVcore2  
    pmic_config_interface(0x662, Vcore_HV, 0x7F,0); 
    pmic_config_interface(0x664, Vcore_HV, 0x7F,0); 
    pmic_config_interface(0x52A, Vdram_HV, 0x7F,0);
    pmic_config_interface(0x52C, Vdram_HV, 0x7F,0);
    print("[HQA]Set HVcore2 setting: Vcore1=1.1V(0x%x, should be 0x%x), Vdram=1.3V(0x%x, should be 0x%x)\n", 
          upmu_get_reg_value(0x662),Vcore_HV, upmu_get_reg_value(0x52A), Vdram_HV);                                    
#endif

#ifdef NV 
    pmic_config_interface(0x662, Vcore_NV, 0x7F,0); 
    pmic_config_interface(0x664, Vcore_NV, 0x7F,0); 
    pmic_config_interface(0x52A, Vdram_NV, 0x7F,0);
    pmic_config_interface(0x52C, Vdram_NV, 0x7F,0);
    print("[HQA]Set NV setting: Vcore1=1.0V(0x%x, should be 0x%x), Vdram=1.22V(0x%x, should be 0x%x)\n", 
          upmu_get_reg_value(0x662),Vcore_NV, upmu_get_reg_value(0x52A), Vdram_NV);     
#endif

#ifdef LVcore2
    pmic_config_interface(0x662, Vcore_LV, 0x7F,0); 
    pmic_config_interface(0x664, Vcore_LV, 0x7F,0); 
    pmic_config_interface(0x52A, Vdram_LV, 0x7F,0);
    pmic_config_interface(0x52C, Vdram_LV, 0x7F,0);
    //print("[HQA]Set LVcore2 setting: Vcore1=0.9V(0x%x, should be 0x%x), Vdram=1.16V(0x%x, should be 0x%x)\n",
    print("[HQA]Set LVcore2 setting: Vcore1=0.95V(0x%x, should be 0x%x), Vdram=1.16V(0x%x, should be 0x%x)\n", 
          upmu_get_reg_value(0x662),Vcore_LV, upmu_get_reg_value(0x52A), Vdram_LV);             
#endif
        
#ifdef HVcore2_LVdram
    pmic_config_interface(0x662, Vcore_HV, 0x7F,0); 
    pmic_config_interface(0x664, Vcore_HV, 0x7F,0); 
    pmic_config_interface(0x52A, Vdram_LV, 0x7F,0);
    pmic_config_interface(0x52C, Vdram_LV, 0x7F,0);    
    print("[HQA]Set HVcore2_LVdram setting: Vcore1=1.1V(0x%x, should be 0x%x), Vdram=1.16V(0x%x, should be 0x%x)\n", 
          upmu_get_reg_value(0x662),Vcore_HV, upmu_get_reg_value(0x52A), Vdram_LV);              
#endif
        
#ifdef LVcore2_HVdram
    pmic_config_interface(0x662, Vcore_LV, 0x7F,0);
    pmic_config_interface(0x664, Vcore_LV, 0x7F,0);  
    pmic_config_interface(0x52A, Vdram_HV, 0x7F,0);
    pmic_config_interface(0x52C, Vdram_HV, 0x7F,0);
    //print("[HQA]Set LVcore2_HVdram setting: Vcore1=0.9V(0x%x, should be 0x%x), Vdram=1.3V(0x%x, should be 0x%x)\n",
    print("[HQA]Set LVcore2_HVdram setting: Vcore1=0.95V(0x%x, should be 0x%x), Vdram=1.3V(0x%x, should be 0x%x)\n",      
          upmu_get_reg_value(0x662),Vcore_LV, upmu_get_reg_value(0x52A), Vdram_HV);             
#endif   
}
#endif

void mt_set_emi(void)
{  
    //VCORE1 force PWM mode
    pmic_config_interface(0x446,0x1,0x1,15);
    //VCORE2 force PWM mode 
    pmic_config_interface(0x486,0x1,0x1,15);

#ifdef COMBO_MCP
    psCurrDramCtx = mt_get_dramc_context();
#else

    #ifdef DDRTYPE_LPDDR3
	psCurrDramCtx = &DramCtx_LPDDR3;
    #endif

    #ifdef DDRTYPE_LPDDR2
	psCurrDramCtx = &DramCtx_LPDDR2;
    #endif

#endif

#if defined(DUAL_FREQ_K) & defined(FREQ_ADJUST)
    psCurrDramCtx->frequency = mt_get_freq_setting(psCurrDramCtx); 
#endif
       
    Init_DRAM();

#if 0
    int i=0;
    for (i = 0x0; i <= 0x1000; i +=4)
    {
        print("[DDR Reserve:DRAMC] 0x%x:0x%x\n",(0x10004000+i),*(volatile unsigned int *)(0x10004000+i));
    }
    for (i = 0x0; i <= 0x1000; i +=4)
    {
        print("[DDR Reserve:DRAMC_NAO] 0x%x:0x%x\n",(0x1020E000+i),*(volatile unsigned int *)(0x1020E000+i));
    }
    for (i = 0x0; i <= 0x1000; i +=4)
    {
        print("[DDR Reserve:DDRPHY] 0x%x:0x%x\n",(0x1000F000+i),*(volatile unsigned int *)(0x1000F000+i));
    }
#endif

    //VCORE1 change to auto mode
    pmic_config_interface(0x446,0x0,0x1,15);
    //VCORE2 change auto mode 
    pmic_config_interface(0x486,0x0,0x1,15);	
}
