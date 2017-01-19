#
#  Copyright Statement:
#  ---------------------------
#  This software/firmware and related documentation ("MediaTek Software") are
#  protected under relevant copyright laws. The information contained herein 
#  is confidential and proprietary to MediaTek Inc. and/or its licensors.  
#  Without the prior written permission of MediaTek inc. and/or its licensors,
#  any reproduction,modification, use or disclosure of MediaTek Software, and
#  information contained herein, in whole or in part, shall be strictly prohibited.
#   
#  MediaTek Inc.(C)2011.All rights reserved.
#
#  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
#  AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK 
#  SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED 
#  TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL 
#  WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
#  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR 
#  NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER 
#  WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
#  INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER 
#  AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING 
#  THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE 
#  RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES 
#  CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR 
#  ANY MEDIATEK SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO 
#  CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND 
#  EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT 
#  TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,AT MEDIATEK'S OPTION, 
#  TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,OR REFUND ANY SOFTWARE 
#  LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK 
#  SOFTWARE AT ISSUE. 
#
# *************************************************************************

# Generated at 2014-10-20 10:05:30

# ------------------------------ Modem specification
MODEM_SPEC = MTK_MODEM_LWG
#
#  Copyright Statement:
#  ---------------------------
#  This software/firmware and related documentation ("MediaTek Software") are
#  protected under relevant copyright laws. The information contained herein 
#  is confidential and proprietary to MediaTek Inc. and/or its licensors.  
#  Without the prior written permission of MediaTek inc. and/or its licensors,
#  any reproduction,modification, use or disclosure of MediaTek Software, and
#  information contained herein, in whole or in part, shall be strictly prohibited.
#   
#  MediaTek Inc.(C)2011.All rights reserved.
#
#  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
#  AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK 
#  SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED 
#  TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL 
#  WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
#  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR 
#  NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER 
#  WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
#  INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER 
#  AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING 
#  THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE 
#  RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES 
#  CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR 
#  ANY MEDIATEK SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO 
#  CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND 
#  EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT 
#  TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,AT MEDIATEK'S OPTION, 
#  TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,OR REFUND ANY SOFTWARE 
#  LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK 
#  SOFTWARE AT ISSUE. 
#
# *************************************************************************

# Generated at 2012-11-29 14:23:02
# ------------------------------ Modem specification
GERAN_MODE_SUPPORT = GERAN_EGPRS_MODE
  # Description:
  #   2G modem capability
  # Option Values:
  #   GERAN_GSM_MODE: Support GSM only
  #   GERAN_GPRS_MODE: Support GPRS
  #   GERAN_EGPRS_MODE: Support EGPRS
  #   NONE: NONE
  # Switch-ability:
  #   Non-switchable

R4_SUPPORT = TRUE
  # Description:
  #   R4 and R5 protocol features,maily NACC and ExtULTBF for cusotmers
  # Option Values:
  #   TRUE: Enable this feature
  #   FALSE: Disable this feature
  # Switch-ability:
  #   Non-switchable

R5_SUPPORT = TRUE
  # Description:
  #   R4 and R5 protocol features,maily NACC and ExtULTBF for cusotmers
  # Option Values:
  #   TRUE: Enable this feature
  #   FALSE: Disable this feature
  # Switch-ability:
  #   Non-switchable

R99_SUPPORT = TRUE
  # Description:
  #   to enable R99 features to be R99 capable MS.
  # Option Values:
  #   TRUE: Enable this feature
  #   FALSE: Disable this feature
  # Switch-ability:
  #   Non-switchable

L1_GPRS = TRUE
  # Description:
  #   Turn on the GPRS support of Layer 1
  #   This is used for internal test
  # Option Values:
  #   TRUE: GPRS enable
  #   FALSE: GPRS disable
  # Switch-ability:
  #   Non-switchable

L1_EGPRS = TRUE
  # Description:
  #   this compile option is only used for L1 Stand alone to turn on/off EDGE function
  # Option Values:
  #   TRUE: EGPRS enable
  #   FALSE: EGPRS disable
  # Switch-ability:
  #   Non-switchable

UTRAN_MODE_SUPPORT = UTRAN_FDD_MODE
  # Description:
  #   3G modem capability
  # Option Values:
  #   UTRAN_FDD_MODE: Support 3G FDD
  #   UTRAN_TDD128_MODE: Support 3G TDD128
  #   NONE: NONE
  # Switch-ability:
  #   Non-switchable

UMTS_MODE_SUPPORT = UMTS_FDD_MODE_SUPPORT
  # Description:
  #   The option is to switch the FDD or TDD mode for 3G modem
  # Option Values:
  #   UMTS_FDD_MODE_SUPPORT: 3G modem in FDD mode
  #   UMTS_TDD128_MODE_SUPPORT: 3G modem in TDD mode
  #   NONE: None of 3G modem mode is supported.
  # Switch-ability:
  #   Non-switchable

UMTS_RELEASE_SUPPORT = UMTS_R9_SUPPORT
  # Description:
  #   The option is to switch different version of release for 3G modem
  # Option Values:
  #   UMTS_R6_SUPPORT: 3G modem supports Rel6
  #   UMTS_R4_SUPPORT: 3G modem supports Rel4
  #   UMTS_R5_SUPPORT: 3G modem supports Rel5
  # Switch-ability:
  #   Non-switchable

HSDPA_SUPPORT = TRUE
  # Description:
  #   Define if UE supports HSDPA feature. (TRUE/FALSE)
  # Option Values:
  #   TRUE: HSDPA is support
  #   FALSE: HSDPA is not support
  # Switch-ability:
  #   Non-switchable

HSUPA_SUPPORT = TRUE
  # Description:
  #   Define if UE supports HSUPA feature. (TRUE/FALSE)
  # Option Values:
  #   TRUE: HSUPA is support
  #   FALSE: HSUPA is not support
  # Switch-ability:
  #   Non-switchable

L1_WCDMA = TRUE
  # Description:
  #   Add an option to enable WCDMA L1 support
  # Option Values:
  #   TRUE: Support UMTS L1
  #   FALSE: Not support UMTS L1
  # Switch-ability:
  #   Non-switchable

L2_HSDPA_COPRO = TRUE
  # Description:
  #   Enable using L2 HSDPA Coprocessor
  # Option Values:
  #   TRUE: Enable L2 HSDPA Copro
  #   FALSE: Disable L2 HSDPA Copro
  # Switch-ability:
  #   Non-switchable

L2_HSUPA_COPRO = TRUE
  # Description:
  #   Enable using L2 HSUPA Coprocessor
  # Option Values:
  #   TRUE: Enable L2 HSUPA Copro
  #   FALSE: Disable L2 HSUPA Copro
  # Switch-ability:
  #   Non-switchable

UL2_HSPA_PLUS_RX_COPRO = TRUE
  # Description:
  #   The option is to enable R7R8 L2 RX copro and bytecopy
  # Option Values:
  #   TRUE: enable R7R8 L2 RX copro and bytecopy
  #   FALSE: disable R7R8 L2 RX copro and bytecopy
  # Switch-ability:
  #   Non-switchable

UL2_HSPA_PLUS_TX_COPRO = TRUE
  # Description:
  #   The option is to enable R7R8 L2 TX copro
  # Option Values:
  #   TRUE: enable R7R8 L2 TX copro
  #   FALSE: disable R7R8 L2 RX copro
  # Switch-ability:
  #   Non-switchable

EUTRAN_MODE_SUPPORT = EUTRAN_MODE
  # Description:
  #   4G modem capability
  # Option Values:
  #   EUTRAN_MODE: Support EUTRAN mode
  #   NONE: NONE
  # Switch-ability:
  #   Non-switchable

LTE_RELEASE_SUPPORT = LTE_R9C0_RELEASE
  # Description:
  #   The option is to switch different asn1 version of release for TS36.331
  # Option Values:
  #   LTE_R9_RELEASE: TS 36.331 9.8.0
  #   LTE_R9C0_RELEASE: TS 36.331 9.c.0
  #   NONE: NONE
  # Switch-ability:
  #   Non-switchable

FDD_LTE_SUPPORT = TRUE
  # Description:
  #   FDD_LTE support capability
  # Option Values:
  #   TRUE: Support FDD-LTE
  #   FALSE: Do NOT support FDD-LTE
  #   NONE: NONE
  # Switch-ability:
  #   Non-switchable

TDD_LTE_SUPPORT = TRUE
  # Description:
  #   TDD_LTE support capability
  # Option Values:
  #   TRUE: Support TDD-LTE
  #   FALSE: Do NOT support TDD-LTE
  #   NONE: NONE
  # Switch-ability:
  #   Non-switchable

# ------------------------------ Configurable Features
AFC_VCXO_TYPE = VCTCXO

OTP_SUPPORT = FALSE

BAND_SUPPORT = QUAD

RF_MODULE = K52_2G_MT6169_CUSTOM

LTE_RF_MODULE = K52_LTE_MT6169_CUSTOM

UMTS_RF_MODULE = K52_UMTS_FDD_MT6169_CUSTOM

# ------------------------------ Verno information
VERNO = MOLY.LR9.W1423.MD.LWTG.MP.V8
BUILD = BUILD_NO
BRANCH = LR9.W1423.MD.LWTG.MP
# ------------------------------ System configurations
PLATFORM = MT6752

CHIP_VER = S00

BOARD_VER = MT6752_K52_MP


#==============
  
CUSTOM_OPTION += 
# if you want to ture off L1_EPSK_TX please add following custom option
#CUSTOM_OPTION += __EPSK_TX_SW_SWITCH_OFF

# internal configuration
PROJECT_MAKEFILE_EXT = NB6752_LWT_KK_MD1(LWG)_EXT
#
#  Copyright Statement:
#  ---------------------------
#  This software/firmware and related documentation ("MediaTek Software") are
#  protected under relevant copyright laws. The information contained herein 
#  is confidential and proprietary to MediaTek Inc. and/or its licensors.  
#  Without the prior written permission of MediaTek inc. and/or its licensors,
#  any reproduction,modification, use or disclosure of MediaTek Software, and
#  information contained herein, in whole or in part, shall be strictly prohibited.
#   
#  MediaTek Inc.(C)2011.All rights reserved.
#
#  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
#  AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK 
#  SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED 
#  TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL 
#  WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
#  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR 
#  NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER 
#  WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
#  INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER 
#  AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING 
#  THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE 
#  RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES 
#  CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR 
#  ANY MEDIATEK SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO 
#  CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND 
#  EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT 
#  TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,AT MEDIATEK'S OPTION, 
#  TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,OR REFUND ANY SOFTWARE 
#  LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK 
#  SOFTWARE AT ISSUE. 
#
# *************************************************************************

# Generated at 2014-10-20 10:05:30
CUSTOM_CFLAGS = #-gdwarf-2
  # Description:
  #   Add custom cflag
  # Option Values:
  #   --debug --no_debug_macros: .
  # Switch-ability:
  #   Non-switchable

DUAL_MODEM_SUPPORT = FALSE
  # Description:
  #   This option is used for support dual modem.
  # Option Values:
  #   TRUE: Support dual modem
  #   FALSE: Not support dual modem
  # Switch-ability:
  #   N/A

TX_POWER_OFFSET_SUPPORT = FALSE
  # Description:
  #   Depend on this option to distinguish whether TX POWER OFFSET is supported or not. 
  # Option Values:
  #   TRUE: TX POWER OFFSET is supported
  #   FALSE: TX POWER OFFSET is NOT supported
  # Switch-ability:
  #   Non-switchable


# =========================================================================
# Feature Options which customer can NOT modify
# =========================================================================
MTK_BT_CHIP = MTK_CONSYS_MT6752
  # Description:
  #   Bluetooth chip type for BT phone call
  # Option Values:
  #   MTK_CONSYS_MT6571: Need to support software BT codec (CVSD / mSBC).
  #   MTK_CONSYS_MT6572: Need to support software BT codec (CVSD / mSBC).
  #   MTK_CONSYS_MT6582: Need to support software BT codec (CVSD / mSBC).
  #   MTK_CONSYS_MT6592: Need to support software BT codec (CVSD / mSBC).
  #   MTK_CONSYS_MT6752: Need to support software BT codec (CVSD / mSBC).
  #   MTK_MT6620: Need to support BT phone call PCM interface.
  #   MTK_MT6622: Need to support BT phone call PCM interface.
  #   MTK_MT6626: Need to support BT phone call PCM interface.
  #   MTK_MT6628: Need to support BT phone call PCM interface.
  #   MTK_MT6630: Need to support BT phone call PCM interface.
  #   NONE: No BT phone call.
  # Switch-ability:
  #   Non-switchable


#[MTK internal used]# ------------------------------ 
ORIGINAL_VERNO = MOLY.LR9.W1423.MD.LWTG.MP.V8
DSP_IMG_ON_EMI = TRUE
  # Description:
  #   DSP image is loaded by AP and placed on EMI before MD ungated
  # Option Values:
  #   TRUE: DSP image is loaded by AP and placed on EMI before MD ungated
  #   FALSE: DSP is on-demand loading during MD runtime
  # Switch-ability:
  #   Non-switchable


HW_VER = NB6752_LWT_KK_MD1_HW

CCCI_RELEASE_SRC = TRUE

HANDOVER_NON3GPP_SUPPORT = FALSE

IMS_SUPPORT = TRUE

VOLTE_SUPPORT = TRUE

VOHSPA_SUPPORT = FALSE

SET_IMEI_BY_PLMN = TRUE

2G_MIPI_INTERSLOT_RAMPING_OPTIMIZE_SUPPORT = TRUE

LTE_ECID_SUPPORT = TRUE

LTE_OTDOA_SUPPORT = TRUE

LPP_SUPPORT = CONTROL_PLANE

CTA_ECC_SUPPORT = TRUE

DCS_TX_NOTCH_SWITCH_SUPPORT = FALSE

OPTR_SPEC_MODEM = NONE

DMA_EMI_TYPE_RESTRICT = NON_CACHEABLE

ORIGINAL_PROJECT_NAME = NB6752_LWT_KK_MD1

ORIGINAL_FLAVOR = LWG

CUSTOM_FLAVOR = LWG

DUALTALK = FALSE

GEMINI = 2

DEMO_PROJECT = FALSE

TEST_LOAD_TYPE = NONE

2G_BPI_PT3A_SUPPORT = FALSE

2G_MIPI_SUPPORT = TRUE

2G_RF_CUSTOM_TOOL_SUPPORT = TRUE

2G_TX_VOLTAGE_COMPENSATION_SUPPORT = TRUE

32K_XOSC_REMOVE = FALSE

3G_DATA_PLANE_MEMORY_SHRINK = FALSE

3G_NEW_DATA_PLANE_ARCH = TRUE

3G_VIDEO_CALL = FALSE

3SET_F8F9_ENABLE = TRUE

AEC_ENABLE = TRUE

AGPS_CP_SIB15_SUPPORT = TRUE

AGPS_SUPPORT = CP_AGPS_AGLONASS

AMR_LINK_SUPPORT = TRUE

AMRWB_LINK_SUPPORT = TRUE

AOC_SUPPORT = TRUE

AT_COMMAND_SET = FULL

BACKGROUND_SOUND = FALSE

BIP_SCWS_SUPPORT = FALSE

BOOT_CERT_SUPPORT = FALSE

BYTECOPY_SUPPORT = TRUE

CCBS_SUPPORT = FALSE

CCCI_CCMNI_SUPPORT = TRUE

CCCI_DEV_SUPPORT = TRUE

CCCI_FS_SUPPORT = TRUE

CENTRALIZED_SLEEP_MANAGER = TRUE

CHIP_VERSION_CHECK = FALSE

CHK_ENV_FLAG = TRUE

CMUX_SUPPORT = TRUE

CNAP_SUPPORT = FALSE

COMBO_MEMORY_SUPPORT = FALSE

COMPILER = GCC

CSD_SUPPORT = NONE

CTM_SUPPORT = TRUE

CUSTOM_DEBUG_MODULES = NVRAM SYS_SVC SYS_DRV CONFIG DEVDRV

CUSTOM_NON_DEBUG_MODULES = NONE

CUSTOMER_SPECIFIC_FACTORY_DETECTION = NONE

DATA_CARD_SPEECH = FALSE

DHL_DNT_LOGGING = FALSE

DHL_MALMO_SUPPORT = FALSE

DHL_SUPPORT = TRUE

DISABLE_A5_2 = TRUE

DMA_UART_VFIFO_TUNNEL_SUPPORT = FALSE

DMA_UART_VIRTUAL_FIFO = FALSE

DRV_CUSTOM_TOOL_SUPPORT = FALSE

DRV_DEBUG_MEMORY_TRACE_SUPPORT = NONE

DSP_SOLUTION = NONE

DSPIRDBG = FALSE

DT_SUPPORT = FALSE

DUAL_EXT_BL = FALSE

DUAL_LTE_DSP = FALSE

DUAL_PRIMARY_ROM = FALSE

DUAL_TALK_RX_GAIN_TABLE_CO_BAND_SUPPORT = FALSE

DUMMY_SCATTER_ENABLE = FALSE

ECC_RETRY_ENHANCEMENT = TRUE

EDGE_CLASS_10 = FALSE

EDGE_SUPPORT = TRUE

EES_ENABLE = TRUE

EMMC_BOOTING = NONE

ENHANCED_SINGLE_BANK_NOR_FLASH_SUPPORT = FALSE

ETSTM_EN = FALSE

EXT_BL_UPDATE_SUPPORT = FALSE

EXT_CLOCK = EXT_26M

EXT_MODEM_SUPPORT = FALSE

FAST_DORMANCY_SUPPORT = TRUE

FAX_SUPPORT = FALSE

FDD_EDCH_PHYSICAL_CATEGORY = 7

FDD_HSDSCH_PHYSICAL_CATEGORY = 24

FEATURE_OVERLOAD = FALSE

FLASH_DRV_IN_BL = NONE

FOTA_ENABLE = NONE

FOTA_UPDATE_PACKAGE_ON_NAND = FALSE

FS_DEDICATED_BUFFER = FALSE

FS_OPEN_HINT_SUPPORT = FALSE

FS_RAMDISK = FALSE

FS_TRACE_SUPPORT = FALSE

GCC_WARN_AS_ERROR = FALSE

GDB_STUB_SUPPORT = FALSE

GEMINI_VERSION = V2

GERAN_RELEASE_SUPPORT = GERAN_R9_SUPPORT

GPRS_CLASS_10 = FALSE

GPRS_DIALUP_PPP_DROP_PACKETS_WHEN_2G_PS_SUSPEND = FALSE

GPRS_DIALUP_PPP_SUPPORT_ESCAPE_ATO = FALSE

GPRS_MAX_PDP_SUPPORT = 6

GPS_SUPPORT = SMART_PHONE_MODEM

HIF_AOMGR_SUPPORT = FALSE

HIF_CCCI_SUPPORT = TRUE

HIF_CCIF_SUPPORT = FALSE

HIF_CLDMA_SUPPORT = TRUE

HIF_SDIO_SUPPORT = FALSE

HIF_UART_SUPPORT = TRUE

HIF_USB_SUPPORT = FALSE

HIF_USB30_SUPPORT = FALSE

HIF_WCCIF_SUPPORT = TRUE

HSUPA_CAPABILITY_NOT_SUPPORT = FALSE

HW_PFC_SUPPORT = FALSE

IC_TEST_TYPE = NONE

ICUSB_SUPPORT = TRUE

IPV6_PDP_SUPPORT = TRUE

KAL_ASSERT_LEVEL = KAL_ASSERT_LEVEL3

KAL_DEBUG_LEVEL = NORMAL_DEBUG_KAL

KAL_RECORD_BOOTUP_LOG = FALSE

KAL_TRACE_OUTPUT = FULL

L1_3GSOLUTION = MTK_UL1_FDD

L1_CATCHER = TRUE

L1_EPSK_TX = TRUE

L1_GPS_CO_CLOCK_SUPPORT = FALSE

L1_GPS_REF_TIME_SUPPORT = TRUE

L1_LOCK_AFCDAC_AT_STARTUP_SUPPORT = FALSE

L1D_LOOPBACK = 0

L1L2_TX_NEW_ARCH = TRUE

LTE_DSP_PROJECT = MT6752

LTE_DSP_FLAVOR = MP

LTE_TX_PATH_SWITCH_SUPPORT = FALSE

MAX_NUM_OF_NDIS_SUPPORT = 3

MCD_CODESET_SHIFT_SUPPORT = 440000

MCD_SUPPORT = TRUE

MCU_CLOCK = MCU_600M

MCU_DCM = DCM_ENABLE

MCU_DORMANT_MODE = TRUE

MD_OFFLOAD_COPRO = NONE

MDSYS = MD1

MM_RF_MODULE = MT6752_MMRF_CUSTOM

MOBILE_BROADBAND = NONE

MODEM_3G_LOGGING = FALSE

MODEM_CARD = NONE

MODEM_NFC_SUPPORT = TRUE

MODIS_FDM = OFF

MSDC_CARD_SUPPORT_TYPE = NONE

MTK_SLEEP_ENABLE = TRUE

MULTIPLE_NDIS_SUPPORT = FALSE

MULTIPLE_PPP_DIALUP_SUPPORT = FALSE

MULTIPLE_TBF = FALSE

NAND_FLASH_BOOTING = NONE

NAND_SUPPORT = FALSE

NAND_SUPPORT_RAW_DISK = FALSE

NDIS_SUPPORT = NONE

NOR_SUPPORT_RAW_DISK = FALSE

NOT_BENEFIT_FROM_BATTERY_CONSUMPTION_OPTIMISATION = FALSE

NVRAM_PSEUDO_MERGE = OFF

NVRAM_SUPPORT = TRUE

OS_TICK_PERIOD_IN_US = 5000

PACKAGE_SEG = NONE

PARTIAL_TRACE_LIB = NONE

PBCCH_SUPPORT = FALSE

PHB_ADDITIONAL_SUPPORT = TRUE

PHB_FDN_ENTRY = 50

PHB_LN_ENTRY = 20

PHB_PHONE_ENTRY = 10

PHB_SIM_ENTRY = 1000

PHB_SYNC = ON

PLATFORM_FOR_CHECK = NONE

PLMN_LIST_PREF_SUPPORT = DEFAULT

PMIC = MT6325

PMIC_INIT_TYPE = PMIC_INIT_NONE

PPP_SUPPORT = NONE

PPP_TYPE_PDP_DIALUP_SUPPORT = FALSE

PS_HANDOVER = FALSE

R7R8_FULL_SET_SUPPORT = FALSE

RAM_SUPPORT_TYPE = NONE

REDUCE_HEADER_DEPTH = TRUE

RES_PROT = FALSE

RFC2507_SUPPORT = TRUE

RRC_PAYLOAD_FOR_3G_UP_SUPPORT = FALSE

RRLP_VER_SUPPORT = R7

RSAT_SUPPORT = MODEM_WITH_RSAT

RTOS = NUCLEUS_V2

RTOS_DEBUG = TRUE

RVCT_PARTIAL_LINK = FALSE

RVCT_VERSION = NONE

SDIO_DEVICE_CONNECTIVITY = DISABLE

SDS_SUPPORT = FALSE

SECURE_CUSTOM_NAME = MTK

SECURE_JTAG_ENABLE = TRUE

SECURE_PORT_SUPPORT = FALSE

SECURE_RO_ENABLE = FALSE

SECURE_SUPPORT = FALSE

SECURE_VERSION = 1

SERIAL_FLASH_SUPPORT = FALSE

SGLTE_DSDS_SUPPORT = FALSE

SIM_HOT_SWAP = SIM_SLOT_2

SIM_RECOVERY_ENHANCEMENT = TRUE

SIM_SWTICH_CONTROLLER_MT6302 = FALSE

SIM_SWTICH_CONTROLLER_MT6306 = FALSE

SIP_RAM_SIZE = NONE

SIP_SERIAL_FLASH_SIZE = NONE

SMART_PHONE_CORE = ANDROID_MODEM

SML_SUPPORT = TRUE

SMS_OVER_PS_SUPPORT = TRUE

SMS_PHONE_ENTRY = 10

SMS_R8_NATION_LANGUAGE = FALSE

SMS_TOTAL_ENTRY = 100

SP_VIDEO_CALL_SUPPORT = TRUE

SSS_SUPPORT = SSS_LIB

SUB_BOARD_VER = PCB01

SW_BINDING_SUPPORT = NONE

SYSDRV_BACKUP_DISK_SUPPORT = NONE

TDD_EDCH_PHYSICAL_CATEGORY = 6

TST_DNT_LOGGING = FALSE

TST_LOGACC_SUPPORT = FALSE

TST_MALMO_SUPPORT = FALSE

TST_SET_LOG_BUF_SIZ = NONE

TST_SUPPORT = FALSE

TST_WRITE_TO_FILE = FALSE

UART3_SUPPORT = FALSE

ULCS_ASN_SUPPORT_VERSION = ULCS_ASN_SUPPORT_R99

UNIFIED_ISR_LEVEL = TRUE

USB_ACM_SUPPORT = FALSE

USB_BOOTUP_TRACE = FALSE

USB_COM_PORT_SUPPORT = FALSE

USB_COMPORT_PC_DRIVER_SUPPORT = MTK

USB_DOWNLOAD_IN_BL = NONE

USB_HIGH_SPEED_COM_PORT_SUPPORT = FALSE

USB_HS_SUPPORT = FALSE

USB_IN_META_SUPPORT = FALSE

USB_IN_NORMAL_MODE_SUPPORT = FALSE

USB_MASS_STORAGE_CDROM_SUPPORT = FALSE

USB_MASS_STORAGE_SUPPORT = FALSE

USB_MBIM_SUPPORT = FALSE

USB_MSD_SUPPORT = FALSE

USB_MULTIPLE_COMPORT_ENABLE = FALSE

USB_RNDIS_SUPPORT = FALSE

USB_ECM_SUPPORT = FALSE

USB_SUPPORT = FALSE

USB_TETHERING = FALSE

USIM_SUPPORT = TRUE

UT_FLUSH_LOG_ON_ASSERT = FALSE

USB_FW_DL_SUPPORT = FALSE

UUS_SUPPORT = FALSE

VAMOS_SUPPORT = TRUE

VM_CODEC = TRUE

WAV_CODEC = FALSE

WCDMA_PREFERRED = FALSE

X_BOOTING = NONE

ZIMAGE_FAVOR_ROM = FALSE

ZIMAGE_SUPPORT = FALSE

VENDOR = NONE

LOW_COST_SUPPORT = NONE

ACC_NC_AFC_DB_UPDATE_SUPPORT = FALSE

AFC_VCXO_TYPE_2G = VCTCXO

DUAL_SIM_HOT_SWAP_CO_DECK = FALSE

DUAL_TALK_RFIC2_SUPPORT = FALSE

PRODUCTION_RELEASE = TRUE

BTT_AGENT_ENABLE = TRUE

MODEMRESERVEDSIZE_AUTOCONFIG = TRUE

LTE_MAX_EPSB_SUPPORT = 8


# *************************************************************************
# Release Setting Section
# *************************************************************************
RELEASE_PACKAGE        = REL_CR_BASIC	# REL_CR_MMI_GPRS, REL_CR_MMI_GSM, REL_CR_L4_GPRS, REL_CR_L4_GSM REL_SUB_UAS_UMTS
RELEASE_PACKAGE_SUB_PS = REL_SUB_PROTOCOL

RELEASE_$(strip $(INPUT_METHOD)) = SRC	# MTK/SRC, only works when INPUT_METHOD is turning on
RELEASE_INPUT_METHODS_SRC =		# MMI_ZI, MMI_T9, MMI_ITAP,
RELEASE_TYPE = NONE           # NONE, INTERNAL
############################################################
COM_DEFS_FOR_K52_LTE_MT6169_CUSTOM  = MT6169_RF MT6169_LTE_RF K52_LTE_MT6169_CUSTOM
COM_DEFS_FOR_K52_2G_MT6169_CUSTOM   = MT6169_2G_RF  K52_2G_MT6169_CUSTOM
COM_DEFS_FOR_K52_UMTS_FDD_MT6169_CUSTOM  = MT6169_RF MT6169_UMTS_FDD K52_UMTS_FDD_MT6169_CUSTOM
# *************************************************************************
# Include MODEM.mak
# *************************************************************************
include make/MODEM.mak

# *************************************************************************
# Common preprocessor definitions
# *************************************************************************
CUSTOM_OPTION += __FLIGHT_MODE_SUPPORT__ __MANUAL_MODE_NW_SEL__ \
                 __USIM_DRV__ __SATC3__  \
                 __DISABLE_A5_2__ \
                 MSDC_MMC40_SUPPORT __DRV_MSDC_LAYOUT_DEFECT__
CUSTOM_OPTION += __HIGH_SPEED_USB_MODEM__

CUSTOM_OPTION += __SMS_OVER_PS_SUPPORT__ __R6_OOS__

CUSTOM_OPTION += __USB_HIGH_SPEED_COM_PORT_ENABLE__ __THREE_PORT_MODULE__

CUSTOM_OPTION += __DRV_NO_USB_CHARGER__

CUSTOM_OPTION += __MSDC_NO_WRITE_PROTECT__

CUSTOM_OPTION += __USB_MODEM_CARD_EINT_RESET__

CUSTOM_OPTION += __NVRAM_TURN_OFF_MSP__

CUSTOM_OPTION += DRV_USB_FORCE_TRIGGER_ONE_EINT

CUSTOM_OPTION += __NBR_CELL_INFO__ 

CUSTOM_OPTION += __23G_PRI_RESEL_SUPPORT__

CUSTOM_OPTION += __SATCC__ __SATCE__

CUSTOM_OPTION += __BT_SIM_PROFILE__

CUSTOM_OPTION += __CP_HW_CIPHER_ENABLE__ 
CUSTOM_OPTION += __CP_HW_INTEGRITY_ENABLE__
CUSTOM_OPTION += TK6268_FPGA1
CUSTOM_OPTION +=  __HSPA_DATA_PATH_OPT__
CUSTOM_OPTION += __DYNAMIC_HSPA_PREFERENCE__
CUSTOM_OPTION += __HSPA_PREFERENCE_SETTING__
CUSTOM_OPTION += __SIM_RESET_BY_SIM__
CUSTOM_OPTION += __4G_IDC__ 
CUSTOM_OPTION += __VSIM__
CUSTOM_OPTION += __L1DMA_DFEDMA_WORKAROUND__
CUSTOM_OPTION += __AT_EPBW_SUPPORT__
# ************************************************************************* 
# Component trace definition header files for custom release only 
# ************************************************************************* 
# Customer can add new trace headers here for new modules 
NEW_CUS_REL_TRACE_DEFS_MODEM = 


# *************************************************************************
# Custom Release Component Configuration
# *************************************************************************
include make/rel/$(strip $(RELEASE_PACKAGE)).mak

ifeq ($(strip $(CUSTOM_RELEASE)),TRUE)
  ifneq ($(findstring REL_SUB_, $(strip $(RELEASE_PACKAGE))),)
    -include make/rel/sub_ps/$(strip $(RELEASE_PACKAGE_SUB_PS)).mak
  endif
endif
