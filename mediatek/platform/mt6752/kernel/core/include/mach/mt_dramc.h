#ifndef __DRAMC_H__
#define __DRAMC_H__

#define SHUFFER_BY_MD32
#define DUAL_FREQ_DIFF_RLWL			// If defined, need to set MR2 in dramcinit.
//#define DDR_1866

#define DMA_BASE          IOMEM(AP_DMA_BASE)
#define DMA_SRC           IOMEM((DMA_BASE + 0x001C))
#define DMA_DST           IOMEM((DMA_BASE + 0x0020))
#define DMA_LEN1          IOMEM((DMA_BASE + 0x0024))
#define DMA_GSEC_EN       IOMEM((DMA_BASE + 0x0058))
#define DMA_INT_EN        IOMEM((DMA_BASE + 0x0004))
#define DMA_CON           IOMEM((DMA_BASE + 0x0018))
#define DMA_START         IOMEM((DMA_BASE + 0x0008))
#define DMA_INT_FLAG      IOMEM((DMA_BASE + 0x0000))

#define DMA_GDMA_LEN_MAX_MASK   (0x000FFFFF)
#define DMA_GSEC_EN_BIT         (0x00000001)
#define DMA_INT_EN_BIT          (0x00000001)
#define DMA_INT_FLAG_CLR_BIT (0x00000000)

#define CHA_DRAMCAO_BASE	0xF0004000
#define CHA_DDRPHY_BASE	0xF000F000
#define CHA_DRAMCNAO_BASE	0xF020E000
#define INFRA_BASE			0xF0001000
#define MEM_DCM_CTRL		(INFRA_BASE + 0x078)
#define DFS_MEM_DCM_CTRL	(INFRA_BASE + 0x07c)
#define CLK_CFG_0_CLR		(0xF0000048)
#define CLK_CFG_0_SET		(0xF0000044)
#define CLK_CFG_UPDATE		(0xF0000004)
#define PCM_INI_PWRON0_REG		(0xF0006010)

#define LPDDR3_MODE_REG_2_LOW		0x00140002		// RL6 WL3.
#define LPDDR2_MODE_REG_2_LOW		0x00040002		// RL6 WL3.

#ifdef DDR_1866
    #define LPDDR3_MODE_REG_2		0x001C0002
#else
    #define LPDDR3_MODE_REG_2		0x001A0002
#endif
#define LPDDR2_MODE_REG_2		0x00060002

#define DRAMC_REG_MRS 0x088
#define DRAMC_REG_PADCTL4 0x0e4
#define DRAMC_REG_LPDDR2_3 0x1e0
#define DRAMC_REG_SPCMD 0x1e4
#define DRAMC_REG_ACTIM1 0x1e8
#define DRAMC_REG_RRRATE_CTL 0x1f4
#define DRAMC_REG_MRR_CTL 0x1fc
#define DRAMC_REG_SPCMDRESP 0x3b8
//#define READ_DRAM_TEMP_TEST

#define PATTERN1 0x5A5A5A5A
#define PATTERN2 0xA5A5A5A5
/*Config*/
#define APDMA_TEST
//#define APDMAREG_DUMP
#define PHASE_NUMBER 3
#define DRAM_BASE             (0x40000000ULL)
#define BUFF_LEN   0x100
#define IOREMAP_ALIGMENT 0x1000
#define Delay_magic_num 0x295;   //We use GPT to measurement how many clk pass in 100us

extern unsigned int DMA_TIMES_RECORDER;
extern phys_addr_t get_max_DRAM_size(void);

int DFS_APDMA_Enable(void);
int DFS_APDMA_Init(void);
int DFS_APDMA_early_init(void);
void dma_dummy_read_for_vcorefs(int loops);
void get_mempll_table_info(u32 *high_addr, u32 *low_addr, u32 *num);
unsigned int get_dram_data_rate(void);
unsigned int read_dram_temperature(void);
void sync_hw_gating_value(void);
unsigned int is_one_pll_mode(void);

typedef enum
{
    TYPE_DDR1 = 1,
    TYPE_LPDDR2,
    TYPE_LPDDR3,
    TYPE_PCDDR3,
};

/************************** Common Macro *********************/
#define delay_a_while(count) \
    do {                                   \
           register unsigned int delay;        \
           asm volatile ("mov %0, %1\n\t"      \
                         "1:\n\t"              \
                         "subs %0, %0, #1\n\t" \
                         "bne 1b\n\t"          \
                         : "+r" (delay)        \
                         : "r" (count)         \
                         : "cc");              \
        } while (0)

#define mcDELAY_US(x)           delay_a_while((U32) (x*1000*10))
    
#endif   /*__WDT_HW_H__*/
