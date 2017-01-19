###################################################################
# flag
###################################################################

MOD_SRC  =
MOD_OBJ  = $(MTK_ROOT_OUT)/PRELOADER_OBJ/obj

###################################################################
# source files
# - add your file here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
###################################################################

MOD_SRC  := \
	circbuf.c \
	rtc.c \
	mmc_common_inter.c \
	mmc_core.c \
	mmc_test.c \
	msdc.c \
	msdc_dma.c \
	msdc_irq.c \
	msdc_utils.c \
	i2c.c \
	keypad.c \
	usbd.c \
	usbphy.c \
	usbtty.c \
	emi.c \
	dramc_pi_basic_api.c \
	dramc_pi_calibration_api.c \
	timer.c \
	wdt.c \
	efi.c \
	platform.c \
	pll.c \
	device_apc.c \
	gpio.c \
	uart.c \
 	pmic_wrap_init.c \
	pmic.c \
	mt6311.c \
	memory.c \
	memory_test.s \
	sync_write.c \
	spm_mtcmos.c \
	hotplug.s

#ifneq ("$(MTK_EMMC_SUPPORT)","yes")
ifndef MTK_EMMC_SUPPORT
	MOD_SRC  += nand.c
	MOD_SRC  +=	bmt.c
endif

ifeq ("$(PL_MODE)","DUMMY_AP")
	MOD_SRC  += gpio_init.c
endif

ifeq ("$(PL_MODE)","TINY")
	MOD_SRC  += gpio_init.c
endif

###################################################################
# target
###################################################################

include $(COMMON_FILE_MK)
