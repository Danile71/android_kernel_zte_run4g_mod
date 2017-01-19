#ifndef _DRAMC_PI_API_H
#define _DRAMC_PI_API_H

#include "dramc_register.h"
/***********************************************************************/
/*              Includes                                               */
/***********************************************************************/

/***********************************************************************/
/*              Constant Define                                        */
/***********************************************************************/

// MEMPLL reference clock input config (from SYSPLL)
#define fcMEMPLL_52M_REF_IN
//#define fcMEMPLL_26M_REF_IN

// MEMPLL BW config
#define fcMEMPLL_0p5M_BW
//#define fcMEMPLL_1M_BW

#define fcWAVEFORM_MEASURE
//#define fcMEMPLL_TPTN_MON_EN

//MEMPLL config
#ifdef fcMEMPLL_26M_REF_IN
 #define Fin 26   // ref clock input in MHz
#elif defined fcMEMPLL_52M_REF_IN
 #define Fin 52
#else
 #define Fin 52
#endif

#define PREDIV 1   // pre-divider
#define POSDIV 1   // post-divider

// SSC / SYSPLL
//====== Config ======
#ifdef fcMEMPLL_26M_REF_IN
 #define SSC_SYSPLL_N 60
 #define DMSS_DIV 0xf
#elif defined fcMEMPLL_52M_REF_IN
 #define SSC_SYSPLL_N 56
 #define DMSS_DIV 0x7
#else
 #define SSC_SYSPLL_N 60
 #define DMSS_DIV 0xf
#endif

//init
#define DEFAULT_TEST2_1_CAL 0x55000000   // pattern0 and base address for test engine when we do calibration
// for testing, to separate TA4-3 address for running simultaneously
//#define DEFAULT_TEST2_1_CAL 0x55010000   // pattern0 and base address for test engine when we do calibration
#define DEFAULT_TEST2_2_CAL 0xaa000400   // pattern1 and offset address for test engine when we  do calibraion
#define DEFAULT_TEST2_1_DQSIEN 0x55000000   // pattern0 and base address for test engine when we do dqs gating window
#define DEFAULT_TEST2_2_DQSIEN 0xaa000010   // pattern1 and offset address for test engine when we  do dqs gating window
#define DEFAULT_GOLD_DQSIEN 0x20202020   // gold pattern for dqsien compare
#define DEFAULT_BL_TYPE BL_TYPE_8
#define DEFAULT_DATA_WIDTH DATA_WIDTH_32BIT
#define DEFAULT_DRAM_MODE MODE_2X

#define DEFAULT_MR1_VALUE_DDR3 0x00002000
#define DEFAULT_MR2_VALUE_LP3 0x001c0002

// timeout for TE2: (CMP_CPT_POLLING_PERIOD X MAX_CMP_CPT_WAIT_LOOP) 
// complete flag
// for testing
//#define CMP_CPT_POLLING_PERIOD 1000
#define CMP_CPT_POLLING_PERIOD 10
#define MAX_CMP_CPT_WAIT_LOOP 10000   // max loop

// jitter meter for PLL phase calibration
#define fcJMETER_COUNT 1024
// K2?? Need to check Fin frequency.
#define fcJMETER_WAIT_DONE_US (fcJMETER_COUNT/Fin+10)    // 10us for more margin

// gating window
//==========================
#define fcNEW_GATING_FINETUNE_LIMIT   // to filter fake passed window
#if defined(fcNEW_GATING_FINETUNE_LIMIT) && defined(DDR_FT_LOAD_BOARD)
 #define fcNEW_GATING_FINETUNE_LIMIT_2   // improved version for very long fake passed window in MT6595 FT (only for FT) 
#endif

#define DQS_GW_COARSE_START  6//22//18
#define DQS_GW_COARSE_END	40//32//29
#define DQS_GW_COARSE_STEP	1
#define DQS_GW_COARSE_MAX	((DQS_GW_COARSE_END-DQS_GW_COARSE_START)/DQS_GW_COARSE_STEP+1)
#define DQS_GW_FINE_START 0
#define DQS_GW_FINE_END 127
#define DQS_GW_FINE_STEP 8
#define DQS_GW_FINE_MAX ((DQS_GW_FINE_END-DQS_GW_FINE_START)/DQS_GW_FINE_STEP+1)
#define DQS_GW_LEN_PER_COARSE_ELEMENT 32   // depend on dqs_gw[] type. U32 currently
#define DQS_GW_LEN_PER_COARSE	(CEIL_A_OVER_B(DQS_GW_FINE_MAX, DQS_GW_LEN_PER_COARSE_ELEMENT))   // dqs_gw[] length per coarse tune. each bit represents one test result 
#define DQS_GW_LEN	(DQS_GW_COARSE_MAX*DQS_GW_LEN_PER_COARSE)   // dqs_gw[] total length

#define DQS_GW_TE_OFFSET 0x10
#define DQS_GW_GOLD_COUNTER_16BIT 0x40404040   // 16*16/4=64
#define DQS_GW_GOLD_COUNTER_32BIT 0x20202020

// common
#define DQ_DATA_WIDTH 32   // define max support bus width in the system (to allocate array size)
#define DQS_BIT_NUMBER 8
#define DQS_NUMBER (DQ_DATA_WIDTH /DQS_BIT_NUMBER)

// RX DQ/DQS
#define MAX_RX_DQSDLY_TAPS 64   	// 0x018, May set back to 64 if no need.
#define MAX_RX_DQDLY_TAPS 16      // 0x210~0x22c, 0~15 delay tap

// DATLAT
#define DATLAT_TAP_NUMBER 26   // DATLAT[4:0] = {0xf0[25] 0x0e4[4] 0x07c[6:4]}

// TX DQ/DQS
#define FIRST_TX_DQ_DELAY 0   // first step to DQ delay
#define FIRST_TX_DQS_DELAY 0   // first step to DQS delay
#define MAX_TX_DQDLY_TAPS 16   // max DQ TAP number
#define MAX_TX_DQSDLY_TAPS 16   // max DQS TAP number

// DRAM Driving
#define MAX_DRAM_DRIV_SET_LPDDR2 6   // LPDDR2 driving setting steps
#define MAX_DRAM_DRIV_SET_DDR3 2     // DDR3 driving setting steps
#define DEFAULT_LPDDR2_DRIV_MR3_VALUE 0x00020003   // LPDDR2 MR3 default value
#define DEFAULT_DDR3_DRIV_MR1_VALUE 0x00002000   // DDR3 MR1 default value

#ifdef DDR_FT_LOAD_BOARD
//Step
#define FLAG_PLLPHASE_CALIBRATION		0
#define FLAG_PLLGPPHASE_CALIBRATION	1
#define FLAG_IMPEDANCE_CALIBRATION		2
#define FLAG_CA_CALIBRATION				3
#define FLAG_WL_CALIBRATION			4
#define FLAG_GATING_CALIBRATION		5
#define FLAG_RX_CALIBRATION				6
#define FLAG_DATLAT_CALIBRATION		7
#define FLAG_TX_CALIBRATION				8
//Error Type
#define FLAG_CALIBRATION_PASS			0
#define FLAG_WINDOW_TOO_SMALL			1
#define FLAG_WINDOW_TOO_BIG			2
#define FLAG_CALIBRATION_FAIL			3
//Channel (the same as DRAM_CHANNEL_T)

//Complete flag
#define FLAG_NOT_COMPLETE_OR_FAIL		0
#define FLAG_COMPLETE_AND_PASS			1

// FT window size criterion (need to be updated)
#define PLL_PHCALIB_BOUND				24 // skew difference (00~h'17), from simulation
#define PLL_GPPHCALIB_BOUND			56 // skew difference (00~h'37), from simulation
#define CA_TRAINING_BOUND				5
#define RXWIN_CALIB_BOUND				10
#define DATLAT_CALIB_BOUND				1
#define TXWIN_CALIB_BOUND				7
#endif

/***********************************************************************/
/*              Defines                                                */
/***********************************************************************/
#define ENABLE  1
#define DISABLE 0

typedef enum
{
    DRAM_OK = 0, // OK
    DRAM_FAIL    // FAIL
} DRAM_STATUS_T; // DRAM status type

/*
#if fcFOR_CHIP_ID == fcMT6595
#define CHANNEL_NUMBER		2
#elif fcFOR_CHIP_ID == fcK2
#define CHANNEL_NUMBER		1
#endif
*/
#define CHANNEL_NUMBER		2

typedef enum
{
    CHANNEL_A = 0,    
    CHANNEL_B,    
} DRAM_CHANNEL_T;

typedef enum
{
    TYPE_DDR1 = 1,
    TYPE_LPDDR2,
    TYPE_LPDDR3,
    TYPE_PCDDR3
} DRAM_DRAM_TYPE_T;

typedef enum
{
    DATA_WIDTH_16BIT = 16,
    DATA_WIDTH_32BIT = 32
} DRAM_DATA_WIDTH_T;

// for A60808 DDR3
typedef enum
{
    PCB_LOC_ASIDE = 0,
    PCB_LOC_BSIDE
} DRAM_PCB_LOC_T;

typedef enum
{
    MODE_1X = 0,
    MODE_2X
} DRAM_DRAM_MODE_T;

typedef enum
{
    PACKAGE_SBS = 0,
    PACKAGE_POP
} DRAM_PACKAGE_T;

typedef enum
{
    TE_OP_WRITE_READ_CHECK = 0,
    TE_OP_READ_CHECK
} DRAM_TE_OP_T;

typedef enum
{
    TEST_ISI_PATTERN = 0,
    TEST_AUDIO_PATTERN,
    TEST_TA1_SIMPLE,
    TEST_TESTPAT4,
    TEST_TESTPAT4_3, 
    TEST_XTALK_PATTERN,
    TEST_MIX_PATTERN
} DRAM_TEST_PATTERN_T;

typedef enum
{
    BL_TYPE_4 = 0,
    BL_TYPE_8
} DRAM_BL_TYPE_T;

typedef enum
{
    DLINE_0 = 0,
    DLINE_1,
    DLINE_TOGGLE    
} PLL_PHASE_CAL_STATUS_T;

typedef enum
{
    TA43_OP_STOP,
    TA43_OP_CLEAR,
    TA43_OP_RUN,
    TA43_OP_RUNQUIET,
    TA43_OP_UNKNOWN,
} DRAM_TA43_OP_TYPE_T;

// used for record last test pattern in TA
typedef enum
{
    TA_PATTERN_IDLE,
    TA_PATTERN_TA43,
    TA_PATTERN_TA4,
    TA_PATTERN_UNKNOWM,
} DRAM_TA_PATTERN_T;

typedef enum
{
    DMA_OP_PURE_READ,
    DMA_OP_PURE_WRITE,
    DMA_OP_READ_WRITE,
} DRAM_DMA_OP_T;

////////////////////////////
typedef struct _DRAMC_CTX_T
{
    DRAM_CHANNEL_T channel;
    DRAM_DRAM_TYPE_T dram_type;
    DRAM_PACKAGE_T package;
    DRAM_DATA_WIDTH_T data_width;
    U32 test2_1;
    U32 test2_2;
    DRAM_TEST_PATTERN_T test_pattern;
    U16 frequency;
    U16 frequency_low;
    U8 fglow_freq_write_en;
    U8 ssc_en;
    //U8 dynamicODT;
    U8 en_4bitMux;  
} DRAMC_CTX_T;

typedef struct _RXDQS_PERBIT_DLY_T
{
    S8 first_dqdly_pass;
    S8 last_dqdly_pass;
    S8 first_dqsdly_pass;
    S8 last_dqsdly_pass;
    S8 best_first_dqdly_pass;
    S8 best_last_dqdly_pass;
    S8 best_first_dqsdly_pass;
    S8 best_last_dqsdly_pass;    
    U8 best_dqdly;
    U8 best_dqsdly;
} RXDQS_PERBIT_DLY_T;

typedef struct _TXDQM_DLY_T
{
    S8 first_dqmdly_pass;
    S8 last_dqmdly_pass;
    S8 best_first_dqmdly_pass;
    S8 best_last_dqmdly_pass;
} TXDQM_DLY_T;

typedef struct _TXDQS_PERBIT_DLY_T
{
    S8 first_dqdly_pass;
    S8 last_dqdly_pass;
    S8 first_dqsdly_pass;
    S8 last_dqsdly_pass;
    S8 best_first_dqdly_pass;
    S8 best_last_dqdly_pass;
    S8 best_first_dqsdly_pass;
    S8 best_last_dqsdly_pass;        
    U8 best_dqdly;
    U8 best_dqsdly;
} TXDQS_PERBIT_DLY_T;

// has defined in Common/pi_def.h
/************************ Bit Process *************************/
/*
#define mcBITL(b)               (1L << (b))
#define mcBIT(b)                (1L << (b))
#define mcMASK(w)               (mcBIT(w) - 1)
#define mcMASKS(w, b)           (mcMASK(w) << (b))
#define mcFIELD(val, msk, pos)  (((val) << (pos)) & (msk))

#define mcSET_MASK(a, b)        ((a) |= (b))
#define mcCLR_MASK(a, b)        ((a) &= (~(b)))
#define mcCHK_MASK(a, b)        ((a) & (b))
//#define mcSET_BIT(a, b)         mcSET_MASK(a, mcBIT(b))
#define mcSET_BIT(a, b)         ((a) |= (1L<<(b)))
//#define mcCLR_BIT(a, b)         mcCLR_MASK(a, mcBIT(b))
#define mcCLR_BIT(a, b)         ((a) &= (~(1L<<(b))))
#define mcCHK_BIT1(a, b)        ((a) & mcBIT(b))
#define mcCHK_BITM(a, b, m)     (((a) >> (b)) & (m))
#define mcCHK_BITS(a, b, w)     mcCHK_BITM(a, b, mcMASK(w))
//#define mcTEST_BIT(a, b)        mcCHK_BITM(a, b, 1)
#define mcTEST_BIT(a, b)        mcCHK_BIT1(a, b)
#define mcCHG_BIT(a, b)         ((a) ^= mcBIT(b))

#define mcSET_FIELD0(var, val, msk, pos)    mcSET_MASK(var, mcFIELD(val, msk, pos))

#define mcSET_FIELD(var, value, mask, pos)  \
{                                           \
    mcCLR_MASK(var, mask);                  \
    mcSET_FIELD0(var, value, mask, pos);    \
}

#define mcGET_FIELD(var, mask, pos)     (((var) & (mask)) >> (pos))
*/

#if fcFOR_CHIP_ID == fcK2
#define mcSET_DRAMC_REG_ADDR(offset)    (offset)
#define mcSET_SYS_REG_ADDR(offset)    (offset)
#endif

/***********************************************************************/
/*              External declarations                                  */
/***********************************************************************/

/***********************************************************************/
/*              Public Functions                                       */
/***********************************************************************/
// basic function

DRAM_STATUS_T DramcPreInit(DRAMC_CTX_T *p);
DRAMC_CTX_T *DramcCtxCreate(void);
void vDramcCtxDestroy(DRAMC_CTX_T *p);
void vDramcCtxInit(DRAMC_CTX_T *p);
DRAM_STATUS_T MemPllInit(DRAMC_CTX_T *p);
U32 DramOperateDataRate(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p);
U32 DramcEngine1(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, S16 loopforever, U8 period);
U32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U32 test2_1, U32 test2_2, U8 testaudpat, S16 loopforever, U8 period, U8 log2loopcount);
void DramcEnterSelfRefresh(DRAMC_CTX_T *p, U8 op);
void DramcLowFreqWrite(DRAMC_CTX_T *p);
void DramcPhyReset(DRAMC_CTX_T *p);
void DramcRunTimeConfig(DRAMC_CTX_T *p);
void DramcHWGWEnable(DRAMC_CTX_T *p);
void DramcTestPat4_3(DRAMC_CTX_T *p, DRAM_TA43_OP_TYPE_T type);
U32 DramcPat4_3(DRAMC_CTX_T *p);
U32 DramcDmaEngine(DRAMC_CTX_T *p, DRAM_DMA_OP_T op, U32 src_addr, U32 dst_addr, U32 trans_len, U8 burst_len, U8 check_result, U8 ChannelNum);
void DramcWorstPat_mem(DRAMC_CTX_T *p, U32 src_addr);
U32 DramcWorstPat_dma(DRAMC_CTX_T *p, U32 src_addr, U32 dst_addr, U32 loop, U8 check_result);

U32 DramcDmaEngineNoWait(DRAMC_CTX_T *p, DRAM_DMA_OP_T op, U32 src_addr, U32 dst_addr, U32 trans_len, U8 burst_len);
U32 DramcDmaWaitCompare(DRAMC_CTX_T *p, U32 src_addr, U32 dst_addr, U32 trans_len, U8 burst_len);
void DramcDiv2PhaseSync(DRAMC_CTX_T *p);

#ifdef DUAL_FREQ_K
void DramcDFS(DRAMC_CTX_T *p, U32 Init);
void DramcShuffle(DRAMC_CTX_T *p);
void DramcSwitchFrequency(DRAMC_CTX_T *p);
void DramcSaveToBackup(DRAMC_CTX_T *p);
void DramcRestoreBackup(DRAMC_CTX_T *p);
void DramcSaveToShuffleReg(DRAMC_CTX_T *p);
void DramcDumpFreqSetting(DRAMC_CTX_T *p);
void DramcRestorePLLSetting(DRAMC_CTX_T *p);
void DramcRestoreFreqSetting(DRAMC_CTX_T *p);
void DramcExchangeFreqSetting(DRAMC_CTX_T *p);
#endif

#ifdef LOOPBACK_TEST
void DramcLoopbackTest(DRAMC_CTX_T *p);
#endif

// mandatory calibration function
DRAM_STATUS_T DramcPllPhaseCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramCPllGroupsCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcSwImpedanceCal(DRAMC_CTX_T *p, U8 apply);
DRAM_STATUS_T DramcHwImpedanceCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcWriteLeveling(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdqsGatingCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DualRankDramcRxdqsGatingCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxWindowPerbitCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxWindowPerbitCal_RK1(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxRegPrint(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcDualRankRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcTxWindowPerbitCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcCATraining(DRAMC_CTX_T *p);
void DramcClkDutyCal(DRAMC_CTX_T *p);
#ifdef SPM_CONTROL_AFTERK
void TransferToSPMControl(void);
void TransferToRegControl(void);
#endif
#ifdef SUSPEND_TEST
void Suspend_Resume(DRAMC_CTX_T *p);
#endif

// reference function
DRAM_STATUS_T DramcRxEyeScan(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcTxEyeScan(DRAMC_CTX_T *p);

#ifdef DDR_FT_LOAD_BOARD
void LoadBoardGpioInit(void);
void LoadBoardShowResult(U8 step, U8 error_type, U8 channel, U8 complete);
#endif

// Global variables
#ifdef _WIN32
extern FILE *fp_A60808;
#endif

#ifdef DUAL_RANKS
extern unsigned int uiDualRank;
#endif
extern U8 CurrentRank;
extern U8 RXPERBIT_LOG_PRINT;

// API prototypes.
U8 ucDram_Register_Read(U32 u4reg_addr, U32 *pu4reg_value);
U8 ucDram_Register_Write(U32 u4reg_addr, U32 u4reg_value);
U8 ucDramC_Register_Read(U32 u4reg_addr, U32 *pu4reg_value);
U8 ucDramC_Register_Write(U32 u4reg_addr, U32 u4reg_value);
void DramcDmaEngine_Config(DRAMC_CTX_T *p);
void DramcGeneralPat_mem(DRAMC_CTX_T *p, U32 src_addr);
#endif // _PI_API_H
