/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include "print.h"
#include "string.h"
#include "boot_device.h"
#include "blkdev.h"
#include "preloader.h"

#include "mt6752.h"

#include "cust_bldr.h"
#include "partition.h"
#include "cust_rtc.h"
#include "cust_nand.h"
#include "cust_usb.h"
#include "sec_limit.h"

/*=======================================================================*/
/* Pre-Loader Internal Features                                          */
/*=======================================================================*/

/* if not defined in cust_usb.h, use default setting */
#if !defined(CFG_USB_ENUM_TIMEOUT)
#define CFG_USB_ENUM_TIMEOUT            (8000)           /* 8s */
#endif

/* if not defined in cust_usb.h, use default setting */
#if !defined(CFG_USB_HANDSHAKE_TIMEOUT)
#define CFG_USB_HANDSHAKE_TIMEOUT       (2500)           /* 2.5s */
#endif

/* support switch to modem com or not */
#if (defined( MTK_DT_SUPPORT) && !defined(EVDO_DT_SUPPORT))
#define CFG_DT_MD_DOWNLOAD              (1)
#else
#define CFG_DT_MD_DOWNLOAD              (0)
#endif

/*=======================================================================*/
/* Platform Setting                                                      */
/*=======================================================================*/
#if CFG_FPGA_PLATFORM
#define I2C_SRC_CLK_FRQ                 (12000000)
#define UART_SRC_CLK_FRQ                (12000000)
#define MSDC_SRC_CLK_FRQ                (12000000)

#else /* !CFG_FPGA_PLATFORM */
#define UART_SRC_CLK_FRQ                (0)         /* use default */
#endif

#define PART_MAX_NUM                    20

/*=======================================================================*/
/* Image Address                                                         */
/*=======================================================================*/
#define CFG_DRAM_ADDR                   (0x40000000)
#define CFG_DA_RAM_ADDR                 (CFG_DRAM_ADDR + 0x00001000)

#if CFG_TEE_SUPPORT
#define CFG_TEE_CORE_SIZE               (0x500000)
#if CFG_TRUSTONIC_TEE_SUPPORT
#define CFG_MIN_TEE_DRAM_SIZE           (0x600000)
#define CFG_MAX_TEE_DRAM_SIZE           (160 * 1024 * 1024) /* TEE max DRAM size is 160MB */
#else
#define CFG_MIN_TEE_DRAM_SIZE           (0)
#define CFG_MAX_TEE_DRAM_SIZE           (0) /* TEE max DRAM size is 0 if TEE is not enabled */
#endif
#endif

//ALPS00427972, implement the analog register formula
//Add here for eFuse, chip version checking -> analog register calibration
#define M_HW_RES3	                    0x10009170
//#define M_HW_RES3_PHY                   IO_PHYS+M_HW_RES3
#define RG_USB20_TERM_VREF_SEL_MASK     0xE000      //0b 1110,0000,0000,0000     15~13
#define RG_USB20_CLKREF_REF_MASK        0x1C00      //0b 0001,1100,0000,0000     12~10
#define RG_USB20_VRT_VREF_SEL_MASK      0x0380      //0b 0000,0011,1000,0000     9~7
//ALPS00427972, implement the analog register formula

//add abstractlayer to avoid hard code in Preloader
#define PL_PMIC_PWR_KEY	MTK_PMIC_PWR_KEY
//#define PL_PMIC_PWR_KEY	8

#if defined(PL_PROFILING)
extern U32 profiling_time;	//declare in main.c
#define BOOTING_TIME_PROFILING_LOG(stage) \
do { \
	printf("#T#%s=%d\n", stage, get_timer(profiling_time)); \
	profiling_time = get_timer(0);	\
}while(0)
#else
#define BOOTING_TIME_PROFILING_LOG(stage) do {}while(0)
#endif

typedef enum {
    NORMAL_BOOT         = 0,
    META_BOOT           = 1,
    RECOVERY_BOOT       = 2,
    SW_REBOOT           = 3,
    FACTORY_BOOT        = 4,
    ADVMETA_BOOT        = 5,
    ATE_FACTORY_BOOT    = 6,
    ALARM_BOOT          = 7,
    FASTBOOT            = 99,

    DOWNLOAD_BOOT       = 100,
    UNKNOWN_BOOT
} boot_mode_t;

typedef enum {
    BR_POWER_KEY = 0,
    BR_USB,
    BR_RTC,
    BR_WDT,
    BR_WDT_BY_PASS_PWK,
    BR_TOOL_BY_PASS_PWK,
    BR_2SEC_REBOOT,
    BR_UNKNOWN,
    BR_KERNEL_PANIC,
    BR_WDT_SW,
    BR_WDT_HW
} boot_reason_t;

typedef enum {
    META_UNKNOWN_COM = 0,
    META_UART_COM,
    META_USB_COM
} meta_com_t;

/* boot argument magic */
#define BOOT_ARGUMENT_MAGIC             (0x504c504c)
#define BA_FIELD_BYPASS_MAGIC           (0x5A5B5A5B)

#define ME_IDENTITY_LEN                 (16)
#define MAX_RECORD_SIZE                 (512)
#define MAX_BL_NUM                      (8)

typedef struct {    
    u32 bl_load_addr;
    u32 bl_length;
    u32 p_bl_desc;
    u8  bl_index;
    u8  state;
    u16 code_1;
    u16 code_2;
} bl_info_t;

typedef struct {            
    u32 raw_data[MAX_RECORD_SIZE>>2];
} hdrblk_info_t;

typedef struct {
    u32 ver;
    u32 hdr_blk_dev_addr;
    hdrblk_info_t hdr_blk_info[4];        
    bl_info_t bl_loadinfo[MAX_BL_NUM];
    u32 brom_flags;
    u32 meid[ME_IDENTITY_LEN>>2];
} bl_param_t;

typedef struct {
    u32 addr;    /* download agent address */
    u32 arg1;    /* download agent argument 1 */
    u32 arg2;    /* download agent argument 2 */
    u32 len;     /* length of DA */
    u32 sig_len; /* signature length of DA */
} da_info_t;

/* DRAM rank info */
typedef struct {
	u64 start;
	u64 size;
} mem_desc_t;

/* mblock is used by CPU */
typedef struct {
	u64 start;
	u64 size;
	u32 rank;	/* rank the mblock belongs to */
} mblock_t;

typedef struct {
	u32 mblock_num;
	mblock_t mblock[4];
} mblock_info_t;

typedef struct {
	u32 rank_num;
	mem_desc_t rank_info[4];
} dram_info_t;

enum reserve_rank {
	RANK0,	/* first rank */
	RANKMAX,	/* as high rank as possible */
};


typedef struct {
    u32 magic;
    boot_mode_t mode;
    u32 e_flag;
    u32 log_port;
    u32 log_baudrate;
    u8  log_enable;
    u8  part_num;
    u8  reserved[2];
    u32 dram_rank_num;
    u32 dram_rank_size[4];
    mblock_info_t mblock_info;	/* number of mblock should be max rank number + 1 */
    dram_info_t orig_dram_info;
    mem_desc_t lca_reserved_mem;
    mem_desc_t tee_reserved_mem;
    u32 boot_reason;
    u32 meta_com_type;
    u32 meta_com_id;
    u32 boot_time;
    da_info_t da_info;
    SEC_LIMIT sec_limit;
    part_hdr_t *part_info;
    u8  md_type[4];
    u32  ddr_reserve_enable;
    u32  ddr_reserve_success;
    u32  dram_buf_size;
    u32  meta_uart_port;
    u32  smc_boot_opt;
    u32  lk_boot_opt;
    u32  kernel_boot_opt;
    u32 non_secure_sram_addr;
    u32 non_secure_sram_size;
} boot_arg_t;

typedef enum {
    CHIP_VER_E1 = 0x0,
    CHIP_VER_E2 = 0x1,
} chip_ver_t;

typedef enum {
    COM_UART = 0,
    COM_USB
} comport_t;

/* BOOT OPTION for ARMv8 ARM Trusted Firmware Support */

#define  STR_BOOT_OPT_64S3 "64S3"
#define  STR_BOOT_OPT_64S1 "64S1"
#define  STR_BOOT_OPT_32S3 "32S3"
#define  STR_BOOT_OPT_32S1 "32S1"
#define  STR_BOOT_OPT_64N2 "64N2"
#define  STR_BOOT_OPT_64N1 "64N1"
#define  STR_BOOT_OPT_32N2 "32N2"
#define  STR_BOOT_OPT_32N1 "32N1"


typedef enum {
    BOOT_OPT_64S3 = 0,
    BOOT_OPT_64S1,
    BOOT_OPT_32S3,
    BOOT_OPT_32S1,
    BOOT_OPT_64N2,
    BOOT_OPT_64N1,
    BOOT_OPT_32N2,
    BOOT_OPT_32N1,
    BOOT_OPT_UNKNOWN
} boot_option_t;

struct comport_ops {
    int (*send)(u8 *buf, u32 len);
    int (*recv)(u8 *buf, u32 len, u32 tmo);
};

struct bldr_comport {
    u32 type;
    u32 tmo;
    struct comport_ops *ops;
};

struct bldr_command {
    u8  *data;
    u32  len;
};

struct bldr_command_handler;

typedef bool (*bldr_cmd_handler_t)(struct bldr_command_handler *handler, struct bldr_command *cmd, struct bldr_comport *comm);

struct bldr_command_handler {
    void *priv;
    u32   attr;
    bldr_cmd_handler_t cb;
};

#define CMD_HNDL_ATTR_COM_FORBIDDEN               (1<<0)

extern int usb_cable_in(void);

extern int  platform_get_mcp_id(u8 *id, u32 len, u32 *fw_id_len);
extern void platform_vusb_on(void);
extern void platform_wdt_kick(void);
extern void platform_wdt_all_kick(void);
extern void platform_pre_init(void);
extern void platform_init(void);
extern void platform_post_init(void);
extern void platform_error_handler(void);
extern void platform_assert(char *file, int line, char *expr);
extern chip_ver_t platform_chip_ver(void);
extern u32 platform_memory_size(void);

extern void apmcu_isb(void);
extern void apmcu_disable_icache(void);
extern void apmcu_icache_invalidate(void);
extern void apmcu_disable_smp(void);
extern void jump(u32 addr, u32 arg1, u32 arg2);
extern void jumparch64(u32 addr, u32 arg1, u32 arg2, u32 atf_arg);

extern boot_mode_t g_boot_mode;
extern boot_dev_t  g_boot_dev;
extern boot_reason_t g_boot_reason;
extern meta_com_t g_meta_com_type;
extern u32 g_meta_com_id;

/* emi functions */
extern void get_orig_dram_rank_info(dram_info_t *orig_dram_info);
extern u64 mblock_reserve(mblock_info_t *mblock_info, u64 size, u64 align, u64 limit,
		enum reserve_rank rank);
extern void setup_mblock_info(mblock_info_t *mblock_info, dram_info_t
		*orig_dram_info, mem_desc_t *lca_reserved_mem);


#define AARCH64_SLT_DONE_ADDRESS (0x110000)
#define AARCH64_SLT_DONE_MAGIC   (0x5A5A5A5A)
extern int aarch64_slt_done(void);

#endif /* PLATFORM_H */
